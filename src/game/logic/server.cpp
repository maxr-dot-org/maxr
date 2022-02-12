/***************************************************************************
*      Mechanized Assault and Exploration Reloaded Projectfile            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "server.h"

#include "debug.h"
#include "game/connectionmanager.h"
#include "game/data/player/playerbasicdata.h"
#include "game/data/report/special/savedreportlostconnection.h"
#include "game/data/savegame.h"
#include "game/logic/action/action.h"
#include "game/logic/turntimeclock.h"
#include "game/protocol/netmessage.h"
#include "game/startup/lobbypreparationdata.h"
#include "settings.h"
#include "utility/language.h"
#include "utility/log.h"
#include "utility/random.h"

#include <cassert>
#include <SDL_thread.h>

//------------------------------------------------------------------------------
cServer::cServer (std::shared_ptr<cConnectionManager> connectionManager) :
	model(),
	gameTimer(),
	connectionManager (connectionManager),
	serverThread (nullptr),
	exit (false)
{
	model.turnEnded.connect ([this]()
	{
		enableFreezeMode (eFreezeMode::WAIT_FOR_TURNEND);
	});
	model.newTurnStarted.connect ([this](const sNewTurnReport&)
	{
		if (cSettings::getInstance().shouldAutosave())
		{
			saveGameState (10, lngPack.i18n ("Text~Comp~Turn_5") + " " + std::to_string (model.getTurnCounter()->getTurn()) + " - " + lngPack.i18n ("Text~Settings~Autosave"));
		}
		disableFreezeMode (eFreezeMode::WAIT_FOR_TURNEND);
	});
}

//------------------------------------------------------------------------------
cServer::~cServer()
{
	stop();
}

//------------------------------------------------------------------------------
std::string cServer::getGameState() const
{
	std::stringstream result;
	result << "GameState: Game is active" << std::endl;

	result << "Map: " << model.getMap()->getName() << std::endl;
	result << "Turn: " << model.getTurnCounter()->getTurn() << std::endl;
	const auto& turnTimeClock = *model.getTurnTimeClock();
	const auto time = turnTimeClock.hasDeadline() ? turnTimeClock.getTimeTillFirstDeadline() : turnTimeClock.getTimeSinceStart();
	result << "Time: " << to_MM_ss (time) << (turnTimeClock.hasDeadline() ? " (deadline)" : "") << std::endl;

	result << "Players:" << std::endl;
	for (auto player : model.getPlayerList())
	{
		result << " " << player->getName() << " (" << serialization::enumToString (playerConnectionStates.at (player->getId())) << ")" <<  std::endl;
	}
	return result.str();
}

//------------------------------------------------------------------------------
void cServer::setPreparationData (const sLobbyPreparationData& preparationData)
{
	model.setUnitsData (std::make_shared<cUnitsData> (*preparationData.unitsData));
	model.setGameSettings (*preparationData.gameSettings);
	model.setMap (preparationData.staticMap);
}

//------------------------------------------------------------------------------
void cServer::setPlayers (const std::vector<cPlayerBasicData>& splayers)
{
	model.setPlayerList (splayers);
	gameTimer.setPlayerNumbers (model.getPlayerList());
}

//------------------------------------------------------------------------------
const cModel& cServer::getModel() const
{
	return model;
}

//------------------------------------------------------------------------------
void cServer::saveGameState (int saveGameNumber, const std::string& saveName) const
{
	if (SDL_ThreadID() != SDL_GetThreadID (serverThread))
	{
		//allow save writing of the server model from the main thread
		exit = true;
		SDL_WaitThread (serverThread, nullptr);
		serverThread = nullptr;
	}

	Log.write (" Server: writing gamestate to save file " + std::to_string (saveGameNumber) + ", Modelcrc: " + std::to_string (model.getChecksum()), cLog::eLOG_TYPE_NET_DEBUG);

	cSavegame savegame;
	savegame.save (model, saveGameNumber, saveName);
	cNetMessageRequestGUISaveInfo message (saveGameNumber, ++savingID);
	sendMessageToClients (message);

	if (!serverThread)
	{
		exit = false;
		serverThread = SDL_CreateThread (serverThreadCallback, "server", const_cast<cServer*> (this));
	}
}
//------------------------------------------------------------------------------
void cServer::loadGameState (int saveGameNumber)
{
	Log.write (" Server: loading game state from save file " + std::to_string (saveGameNumber), cLog::eLOG_TYPE_NET_DEBUG);
	cSavegame savegame;
	savegame.loadModel (model, saveGameNumber);

	gameTimer.setPlayerNumbers (model.getPlayerList());
}
//------------------------------------------------------------------------------
void cServer::sendGuiInfoToClients (int saveGameNumber, int playerNr /*= -1*/)
{
	try
	{
		cSavegame savegame;
		savegame.loadGuiInfo (this, saveGameNumber);
	}
	catch (std::runtime_error& e)
	{
		Log.write ((std::string) " Server: Loading GuiInfo from savegame failed: " + e.what(), cLog::eLOG_TYPE_NET_ERROR);
	}
}

//------------------------------------------------------------------------------
void cServer::resyncClientModel (int playerNr /*= -1*/) const
{
	assert (SDL_ThreadID() == SDL_GetThreadID (serverThread));

	Log.write (" Server: Resynchronize client model " + std::to_string (playerNr), cLog::eLOG_TYPE_NET_DEBUG);
	cNetMessageResyncModel msg (model);
	sendMessageToClients (msg, playerNr);
}

//------------------------------------------------------------------------------
void cServer::pushMessage (std::unique_ptr<cNetMessage> message)
{
	eventQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
void cServer::sendMessageToClients (const cNetMessage& message, int playerNr /* = -1 */) const
{
	if (message.getType() != eNetMessageType::GAMETIME_SYNC_SERVER && message.getType() != eNetMessageType::RESYNC_MODEL)
	{
		nlohmann::json json;
		cJsonArchiveOut jsonarchive (json);
		jsonarchive << message;
		Log.write ("Server: --> " + json.dump (-1) + " @" + std::to_string (model.getGameTime()), cLog::eLOG_TYPE_NET_DEBUG);
	}

	if (playerNr == -1)
	{
		connectionManager->sendToPlayers (message);
	}
	else if (connectionManager->isPlayerConnected (playerNr))
	{
		connectionManager->sendToPlayer (message, playerNr);
	}
}

//------------------------------------------------------------------------------
void cServer::start()
{
	if (serverThread) return;

	initRandomGenerator();
	initPlayerConnectionState();
	updateWaitForClientFlag();

	serverThread = SDL_CreateThread (serverThreadCallback, "server", this);
	gameTimer.maxEventQueueSize = MAX_SERVER_EVENT_COUNTER;
	gameTimer.start();
}

//------------------------------------------------------------------------------
void cServer::stop()
{
	exit = true;
	gameTimer.stop();

	if (serverThread)
	{
		SDL_WaitThread (serverThread, nullptr);
		serverThread = nullptr;
	}
}

//------------------------------------------------------------------------------
void cServer::run()
{
	while (!exit)
	{
		std::unique_ptr<cNetMessage> message;
		while (eventQueue.try_pop (message))
		{
			if (message->getType() != eNetMessageType::GAMETIME_SYNC_CLIENT)
			{
				nlohmann::json json = nlohmann::json::object();
				cJsonArchiveOut jsonarchive (json);
				jsonarchive << *message;
				Log.write ("Server: <-- " + json.dump (-1) + " @" + std::to_string (model.getGameTime()), cLog::eLOG_TYPE_NET_DEBUG);
			}

			if (model.getPlayer (message->playerNr) == nullptr &&
				message->getType() != eNetMessageType::TCP_WANT_CONNECT) continue;

			switch (message->getType())
			{
			case eNetMessageType::ACTION:
			{
				const cAction& action = *static_cast<cAction*> (message.get());

				// filter disallowed actions
				if (action.getType() != cAction::eActiontype::InitNewGame)
				{
					if (freezeModes.isFreezed())
					{
						Log.write (" Server: Discarding action, because game is freezed.", cLog::eLOG_TYPE_NET_WARNING);
						break;
					}
					if (model.getGameSettings()->gameType == eGameSettingsGameType::Turns && message->playerNr != model.getActiveTurnPlayer()->getId())
					{
						Log.write (" Server: Discarding action, because it's another players turn.", cLog::eLOG_TYPE_NET_WARNING);
						break;
					}
				}

				action.execute (model);

				sendMessageToClients (*message);
				break;
			}
			case eNetMessageType::GAMETIME_SYNC_CLIENT:
			{
				const cNetMessageSyncClient& syncMessage = *static_cast<cNetMessageSyncClient*> (message.get());
				gameTimer.handleSyncMessage (syncMessage, model.getGameTime());
				break;
			}
			case eNetMessageType::REPORT:
			{
				sendMessageToClients (*message);
				break;
			}
			case eNetMessageType::GUI_SAVE_INFO:
			{
				const cNetMessageGUISaveInfo& saveInfo = *static_cast<cNetMessageGUISaveInfo*> (message.get());

				if (savingID != saveInfo.savingID)
				{
					Log.write ("Received GuiSaveInfo with wrong savingID", cLog::eLOG_TYPE_NET_WARNING);
				}
				else
				{
					cSavegame savegame;
					savegame.saveGuiInfo (saveInfo);
				}
				break;
			}
			case eNetMessageType::REQUEST_RESYNC_MODEL:
			{
				const cNetMessageRequestResync& requestMessage = *static_cast<cNetMessageRequestResync*> (message.get());
				resyncClientModel (requestMessage.playerToSync);
				if (requestMessage.saveNumberForGuiInfo != -1)
				{
					sendGuiInfoToClients (requestMessage.saveNumberForGuiInfo, requestMessage.playerToSync);
				}
				break;
			}
			case eNetMessageType::TCP_WANT_CONNECT:
			{
				const cNetMessageTcpWantConnect& connectMessage = *static_cast<cNetMessageTcpWantConnect*> (message.get());
				const cPlayer* player = model.getPlayer (connectMessage.player.name);
				if (player == nullptr)
				{
					Log.write (" Server: Connecting player " + connectMessage.player.name + " is not part of the game", cLog::eLOG_TYPE_NET_WARNING);
					connectionManager->declineConnection (connectMessage.socket, eDeclineConnectionReason::NotPartOfTheGame);
					break;
				}
				if (connectionManager->isPlayerConnected (player->getId()))
				{
					Log.write (" Server: Connecting player " + connectMessage.player.name + " is already connected", cLog::eLOG_TYPE_NET_WARNING);
					connectionManager->declineConnection (connectMessage.socket, eDeclineConnectionReason::AlreadyConnected);
					break;
				}

				connectionManager->acceptConnection (connectMessage.socket, player->getId());

				sendMessageToClients (cNetMessageGameAlreadyRunning (model), player->getId());
				break;
			}
			case eNetMessageType::TCP_CLOSE:
			{
				const cNetMessageTcpClose& msg = *static_cast<cNetMessageTcpClose*> (message.get());
				sendMessageToClients (cNetMessageReport (std::make_unique<cSavedReportLostConnection> (*model.getPlayer (msg.playerNr))));
				playerDisconnected (msg.playerNr);
				break;
			}
			case eNetMessageType::WANT_REJOIN_GAME:
			{
				const cNetMessageWantRejoinGame&  msg = *static_cast<cNetMessageWantRejoinGame*> (message.get());

				const auto player = model.getPlayer (msg.playerNr);
				if (player == nullptr)
				{
					Log.write (" Server: Invalid player id: " + std::to_string (msg.playerNr), cLog::eLOG_TYPE_NET_ERROR);
					break;
				}

				resyncClientModel (message->playerNr);
				playerConnected (msg.playerNr);

#if 0 // Restore gamegui
				if (savegame.getLastUsedSaveSlot() != -1)
				{
					savegame.loadGuiInfo (this, savegame.getLastUsedSaveSlot(), message->playerNr);
				}
#endif
				break;
			}
			default:
				Log.write (" Server: Can not handle net message!", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
		}

		//TODO: gameinit: start timer, when all clients are ready
		gameTimer.run (model, *this);

		SDL_Delay (10);
	}
}


//------------------------------------------------------------------------------
void cServer::initRandomGenerator()
{
	uint64_t t = random (UINT64_MAX);
	model.randomGenerator.seed (t);
	cNetMessageRandomSeed msg (t);
	sendMessageToClients (msg);
}

//------------------------------------------------------------------------------
void cServer::enableFreezeMode (eFreezeMode mode)
{
	freezeModes.enable (mode);
	updateGameTimerstate();

	sendMessageToClients (cNetMessageFreezeModes (freezeModes, playerConnectionStates));
}

//------------------------------------------------------------------------------
void cServer::disableFreezeMode (eFreezeMode mode)
{
	freezeModes.disable (mode);
	updateGameTimerstate();

	sendMessageToClients (cNetMessageFreezeModes (freezeModes, playerConnectionStates));
}

//------------------------------------------------------------------------------
void cServer::setPlayerNotResponding (int playerId)
{
	if (playerConnectionStates[playerId] != ePlayerConnectionState::CONNECTED) return;

	playerConnectionStates[playerId] = ePlayerConnectionState::NOT_RESPONDING;
	Log.write (" Server: Player " + std::to_string (playerId) + " not responding", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::clearPlayerNotResponding (int playerId)
{
	if (playerConnectionStates[playerId] != ePlayerConnectionState::NOT_RESPONDING) return;

	playerConnectionStates[playerId] = ePlayerConnectionState::CONNECTED;
	Log.write (" Server: Player " + std::to_string (playerId) + " responding again", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::playerDisconnected (int playerId)
{
	const auto player = model.getPlayer (playerId);
	if (player->isDefeated)
	{
		playerConnectionStates[playerId] = ePlayerConnectionState::INACTIVE;
	}
	else
	{
		//TODO: set to INACTIVE when running in dedicated mode
		playerConnectionStates[playerId] = ePlayerConnectionState::DISCONNECTED;
	}
	Log.write (" Server: Player " + std::to_string (playerId) + " disconnected", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::playerConnected (int playerId)
{
	playerConnectionStates[playerId] = ePlayerConnectionState::CONNECTED;
	Log.write (" Server: Player " + std::to_string (playerId) + " connected", cLog::eLOG_TYPE_NET_DEBUG);
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::updateWaitForClientFlag()
{
	bool freeze = false;
	for (const auto& state : playerConnectionStates)
	{
		if (state.second == ePlayerConnectionState::DISCONNECTED || state.second == ePlayerConnectionState::NOT_RESPONDING)
		{
			freeze = true;
		}
	}

	if (freeze)
	{
		enableFreezeMode (eFreezeMode::WAIT_FOR_CLIENT);
	}
	else
	{
		disableFreezeMode (eFreezeMode::WAIT_FOR_CLIENT);
	}
}

//------------------------------------------------------------------------------
void cServer::updateGameTimerstate()
{
	if (freezeModes.gameTimePaused())
	{
		gameTimer.stop();
	}
	else if (serverThread != nullptr)
	{
		gameTimer.start();
	}
}

//------------------------------------------------------------------------------
void cServer::initPlayerConnectionState()
{
	for (const auto player : model.getPlayerList())
	{
		if (connectionManager->isPlayerConnected (player->getId()))
		{
			playerConnectionStates[player->getId()] = ePlayerConnectionState::CONNECTED;
		}
		else
		{
			// assume that game always start with all necessary players connected.
			// So set the disconnected players to INACTIVE
			playerConnectionStates[player->getId()] = ePlayerConnectionState::INACTIVE;
		}
	}
}

//------------------------------------------------------------------------------
int cServer::serverThreadCallback (void* arg)
{
	try
	{
		CR_ENABLE_CRASH_RPT_CURRENT_THREAD();

		cServer* server = reinterpret_cast<cServer*> (arg);
		server->run();
	}
	catch (const std::exception& ex)
	{
		Log.write (std::string ("Exception: ") + ex.what(), cLog::eLOG_TYPE_ERROR);
	}
	return 0;
}
