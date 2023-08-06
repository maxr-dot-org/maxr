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

#include "crashreporter/debug.h"
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

#include <SDL_thread.h>
#include <cassert>

//------------------------------------------------------------------------------
cServer::cServer (std::shared_ptr<cConnectionManager> connectionManager) :
	model(),
	gameTimer(),
	connectionManager (connectionManager),
	serverThread (nullptr),
	exit (false)
{
	model.turnEnded.connect ([this]() {
		enableFreezeMode (eFreezeMode::WaitForTurnend);
	});
	model.newTurnStarted.connect ([this] (const sNewTurnReport&) {
		if (cSettings::getInstance().shouldAutosave())
		{
			saveGameState (10, lngPack.i18n ("Comp~Turn_5") + " " + std::to_string (model.getTurnCounter()->getTurn()) + " - " + lngPack.i18n ("Settings~Autosave"));
		}
		disableFreezeMode (eFreezeMode::WaitForTurnend);
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

	result << "Map: " << model.getMap()->getFilename().u8string() << std::endl;
	result << "Turn: " << model.getTurnCounter()->getTurn() << std::endl;
	const auto turnTimeClockPtr = model.getTurnTimeClock();
	const auto& turnTimeClock = *turnTimeClockPtr;
	const auto time = turnTimeClock.hasDeadline() ? turnTimeClock.getTimeTillFirstDeadline() : turnTimeClock.getTimeSinceStart();
	result << "Time: " << to_MM_ss (time) << (turnTimeClock.hasDeadline() ? " (deadline)" : "") << std::endl;

	result << "Players:" << std::endl;
	for (auto player : model.getPlayerList())
	{
		result << " " << player->getName() << " (" << serialization::enumToString (playerConnectionStates.at (player->getId())) << ")" << std::endl;
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

	NetLog.debug (" Server: writing gamestate to save file " + std::to_string (saveGameNumber) + ", Modelcrc: " + std::to_string (model.getChecksum()));

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
	NetLog.debug (" Server: loading game state from save file " + std::to_string (saveGameNumber));
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
		NetLog.error ((std::string) " Server: Loading GuiInfo from savegame failed: " + e.what());
	}
}

//------------------------------------------------------------------------------
void cServer::resyncClientModel (int playerNr /*= -1*/) const
{
	assert (SDL_ThreadID() == SDL_GetThreadID (serverThread));

	NetLog.debug (" Server: Resynchronize client model " + std::to_string (playerNr));
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
		NetLog.debug ("Server: --> " + json.dump (-1) + " @" + std::to_string (model.getGameTime()));
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
				NetLog.debug ("Server: <-- " + json.dump (-1) + " @" + std::to_string (model.getGameTime()));
			}

			if (model.getPlayer (message->playerNr) == nullptr && message->getType() != eNetMessageType::TCP_WANT_CONNECT) continue;

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
							NetLog.warn (" Server: Discarding action, because game is freezed.");
							break;
						}
						if (model.getGameSettings()->gameType == eGameSettingsGameType::Turns && message->playerNr != model.getActiveTurnPlayer()->getId())
						{
							NetLog.warn (" Server: Discarding action, because it's another players turn.");
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
						NetLog.warn ("Received GuiSaveInfo with wrong savingID");
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
						NetLog.warn (" Server: Connecting player " + connectMessage.player.name + " is not part of the game");
						connectionManager->declineConnection (*connectMessage.socket, eDeclineConnectionReason::NotPartOfTheGame);
						break;
					}
					if (connectionManager->isPlayerConnected (player->getId()))
					{
						NetLog.warn (" Server: Connecting player " + connectMessage.player.name + " is already connected");
						connectionManager->declineConnection (*connectMessage.socket, eDeclineConnectionReason::AlreadyConnected);
						break;
					}

					connectionManager->acceptConnection (*connectMessage.socket, player->getId());

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
					const cNetMessageWantRejoinGame& msg = *static_cast<cNetMessageWantRejoinGame*> (message.get());

					const auto player = model.getPlayer (msg.playerNr);
					if (player == nullptr)
					{
						NetLog.error (" Server: Invalid player id: " + std::to_string (msg.playerNr));
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
					NetLog.error (" Server: Can not handle net message!");
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
	if (playerConnectionStates[playerId] != ePlayerConnectionState::Connected) return;

	playerConnectionStates[playerId] = ePlayerConnectionState::NotResponding;
	NetLog.debug (" Server: Player " + std::to_string (playerId) + " not responding");
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::clearPlayerNotResponding (int playerId)
{
	if (playerConnectionStates[playerId] != ePlayerConnectionState::NotResponding) return;

	playerConnectionStates[playerId] = ePlayerConnectionState::Connected;
	NetLog.debug (" Server: Player " + std::to_string (playerId) + " responding again");
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::playerDisconnected (int playerId)
{
	const auto player = model.getPlayer (playerId);
	if (player->isDefeated)
	{
		playerConnectionStates[playerId] = ePlayerConnectionState::Inactive;
	}
	else
	{
		//TODO: set to INACTIVE when running in dedicated mode
		playerConnectionStates[playerId] = ePlayerConnectionState::Disconnected;
	}
	NetLog.debug (" Server: Player " + std::to_string (playerId) + " disconnected");
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::playerConnected (int playerId)
{
	playerConnectionStates[playerId] = ePlayerConnectionState::Connected;
	NetLog.debug (" Server: Player " + std::to_string (playerId) + " connected");
	updateWaitForClientFlag();
}

//------------------------------------------------------------------------------
void cServer::updateWaitForClientFlag()
{
	const bool freeze = ranges::any_of (playerConnectionStates, [] (const auto& state) {
		return state.second == ePlayerConnectionState::Disconnected || state.second == ePlayerConnectionState::NotResponding;
	});

	if (freeze)
	{
		enableFreezeMode (eFreezeMode::WaitForClient);
	}
	else
	{
		disableFreezeMode (eFreezeMode::WaitForClient);
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
	for (const auto& player : model.getPlayerList())
	{
		if (connectionManager->isPlayerConnected (player->getId()))
		{
			playerConnectionStates[player->getId()] = ePlayerConnectionState::Connected;
		}
		else
		{
			// assume that game always start with all necessary players connected.
			// So set the disconnected players to INACTIVE
			playerConnectionStates[player->getId()] = ePlayerConnectionState::Inactive;
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
		Log.error (std::string ("Exception: ") + ex.what());
	}
	return 0;
}
