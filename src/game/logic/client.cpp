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
#include <cmath>
#include <sstream>

#include "game/logic/client.h"

#include "game/data/units/building.h"
#include "game/logic/casualtiestracker.h"
#include "utility/listhelpers.h"
#include "game/logic/fxeffects.h"
#include "game/logic/gametimer.h"
#include "utility/log.h"
#include "protocol/netmessage.h"
#include "game/data/player/player.h"
#include "game/logic/server2.h"
#include "settings.h"
#include "game/data/units/vehicle.h"
#include "video.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/game/gameguistate.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/unit/savedreportdisabled.h"
#include "game/data/report/unit/savedreportpathinterrupted.h"
#include "game/data/report/unit/savedreportcapturedbyenemy.h"
#include "game/data/report/unit/savedreportdetected.h"
#include "game/data/report/unit/savedreportpathinterrupted.h"
#include "game/data/report/special/savedreportplayerendedturn.h"
#include "game/data/report/special/savedreportplayerdefeated.h"
#include "game/data/report/special/savedreportplayerleft.h"
#include "game/data/report/special/savedreportupgraded.h"
#include "game/logic/turntimeclock.h"
#include "game/logic/action/action.h"
#include "game/data/savegame.h"
#include "utility/serialization/textarchive.h"
#include "utility/string/toString.h"
#include "surveyorai.h"
#include "action/actionsetautomove.h"

using namespace std;

//------------------------------------------------------------------------
// cClient implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cClient::cClient (std::shared_ptr<cConnectionManager> connectionManager) :
	connectionManager(connectionManager),
	gameTimer (std::make_shared<cGameTimerClient> ()),
	activePlayer (nullptr)
{
	gameTimer->start();
}

cClient::~cClient()
{
	connectionManager->setLocalClient(nullptr, -1);
	gameTimer->stop();
}

void cClient::setMap (std::shared_ptr<cStaticMap> staticMap)
{
	model.setMap(staticMap);
}

void cClient::setGameSettings (const cGameSettings& gameSettings)
{
	model.setGameSettings(gameSettings);
}

void cClient::setPlayers (const std::vector<cPlayerBasicData>& splayers, size_t activePlayerNr)
{
	model.setPlayerList(splayers);
	activePlayer = model.getPlayer(activePlayerNr);
}

void cClient::pushMessage(std::unique_ptr<cNetMessage2> message)
{
	if (message->getType() == eNetMessageType::GAMETIME_SYNC_SERVER)
	{
		// This is a preview for the client to know
		// how many sync messages are in queue.
		// Used to detect a growing lag behind the server time
		const cNetMessageSyncServer* syncMessage = static_cast<const cNetMessageSyncServer*>(message.get());
		gameTimer->setReceivedTime(syncMessage->gameTime);
	}
	eventQueue2.push(std::move(message));
}

void cClient::sendNetMessage(cNetMessage2& message) const
{
	message.playerNr = activePlayer->getId();

	if (message.getType() != eNetMessageType::GAMETIME_SYNC_CLIENT)
	{
		cTextArchiveIn archive;
		archive << message;
		Log.write(getActivePlayer().getName() + ": --> " + archive.data() + " @" + iToStr(model.getGameTime()), cLog::eLOG_TYPE_NET_DEBUG);
	}

	connectionManager->sendToServer(message);
}

void cClient::sendNetMessage(cNetMessage2&& message) const
{
	sendNetMessage(static_cast<cNetMessage2&>(message));
}

//------------------------------------------------------------------------------
void cClient::runClientJobs(const cModel& model)
{
	// run surveyor AI
	handleSurveyorMoveJobs();

}

void cClient::setUnitsData(std::shared_ptr<const cUnitsData> unitsData)
{
	model.setUnitsData(std::make_shared<cUnitsData>(*unitsData));
}

void cClient::handleNetMessages()
{
	std::unique_ptr<cNetMessage2> message;
	while (eventQueue2.try_pop(message))
	{

		if (message->getType() != eNetMessageType::GAMETIME_SYNC_SERVER && message->getType() != eNetMessageType::RESYNC_MODEL)
		{
			cTextArchiveIn archive;
			archive << *message;
			Log.write(getActivePlayer().getName() + ": <-- " + archive.data() + " @" + iToStr(model.getGameTime()), cLog::eLOG_TYPE_NET_DEBUG);
		}

		switch (message->getType())
		{
		case eNetMessageType::REPORT:
			{
				if (message->playerNr != -1 && model.getPlayer(message->playerNr) == nullptr) continue;

				cNetMessageReport* chatMessage = static_cast<cNetMessageReport*>(message.get());
				reportMessageReceived(chatMessage->playerNr, chatMessage->report, activePlayer->getId());
			}
			break;
		case eNetMessageType::ACTION:
			{
				if (model.getPlayer(message->playerNr) == nullptr) continue;

				const cAction* action = static_cast<cAction*>(message.get());
				action->execute(model);
			}
			break;
		case eNetMessageType::GAMETIME_SYNC_SERVER:
			{
				const cNetMessageSyncServer* syncMessage = static_cast<cNetMessageSyncServer*>(message.get());
				gameTimer->handleSyncMessage(*syncMessage, model.getGameTime());
				return; //stop processing messages after receiving a sync message. Gametime needs to be increased before handling the next message.
			}
			break;
		case eNetMessageType::RANDOM_SEED:
			{
				const cNetMessageRandomSeed* msg = static_cast<cNetMessageRandomSeed*>(message.get());
				model.randomGenerator.seed(msg->seed);
			}
			break;
		case eNetMessageType::REQUEST_GUI_SAVE_INFO:
			{
				const cNetMessageRequestGUISaveInfo* msg = static_cast<cNetMessageRequestGUISaveInfo*>(message.get());
				guiSaveInfoRequested(msg->savingID);
			}
			break;
		case eNetMessageType::GUI_SAVE_INFO:
			{
				const cNetMessageGUISaveInfo* msg = static_cast<cNetMessageGUISaveInfo*>(message.get());
				if (msg->playerNr != activePlayer->getId()) continue;
				guiSaveInfoReceived(*msg);
			}
			break;
		case eNetMessageType::RESYNC_MODEL:
			{
				Log.write(" Client: Received model data for resynchronization", cLog::eLOG_TYPE_NET_DEBUG);
				const cNetMessageResyncModel* msg = static_cast<cNetMessageResyncModel*>(message.get());
				try
				{
					msg->apply(model);
					recreateSurveyorMoveJobs();
					gameTimer->sendSyncMessage(*this, model.getGameTime(), 0, 0);
				}
				catch (std::runtime_error& e)
				{
					Log.write(std::string(" Client: error loading received model data: ") + e.what(), cLog::eLOG_TYPE_NET_ERROR);
				}

				//FIXME: deserializing model does not trigger signals on changed data members. Use this signal to trigger some gui updates
				freezeModeChanged(); 
			}
			break;
		case eNetMessageType::FREEZE_MODES:
			{
				const cNetMessageFreezeModes* msg = static_cast<cNetMessageFreezeModes*>(message.get());
				
				// don't overwrite waitForServer flag
				bool waitForServer = freezeModes.isEnabled(eFreezeMode::WAIT_FOR_SERVER);
				freezeModes = msg->freezeModes;
				if (waitForServer) freezeModes.enable(eFreezeMode::WAIT_FOR_SERVER);
				
				for (auto state : msg->playerStates)
				{
					if (model.getPlayer(state.first) == nullptr)
					{
						Log.write(" Client: Invalid player id: " + toString(state.first), cLog::eLOG_TYPE_NET_ERROR);
						break;
					}
				}
				if (msg->playerStates.size() != model.getPlayerList().size())
				{
					Log.write(" Client: Wrong size of playerState map " + toString(msg->playerStates.size()), cLog::eLOG_TYPE_NET_ERROR);
					break;
				}
				playerConnectionStates = msg->playerStates;

				freezeModeChanged();
			}
			break;
		case eNetMessageType::TCP_CLOSE:
			{
				connectionToServerLost();
			}
			break;
		default:
			Log.write(" Client: received unknown net message type", cLog::eLOG_TYPE_NET_WARNING);
			break;
		}
	}

}

//------------------------------------------------------------------------------
void cClient::handleSurveyorMoveJobs()
{
	for (auto& job : surveyorAiJobs)
	{
		job->run(*this, surveyorAiJobs);

		if (job->isFinished())
		{
			job = nullptr;
		}
	}
	Remove(surveyorAiJobs, nullptr);
}

//------------------------------------------------------------------------------
void cClient::enableFreezeMode(eFreezeMode mode)
{
	Log.write(" Client: enabled freeze mode: " + enumToString(mode), cLog::eLOG_TYPE_NET_DEBUG);
	const auto wasEnabled = freezeModes.isEnabled (mode);

	freezeModes.enable (mode);

	if (!wasEnabled) freezeModeChanged ();
}

//------------------------------------------------------------------------------
void cClient::disableFreezeMode (eFreezeMode mode)
{
	Log.write(" Client: disabled freeze mode: " + enumToString(mode), cLog::eLOG_TYPE_NET_DEBUG);
	const auto wasDisabled = !freezeModes.isEnabled (mode);

	freezeModes.disable (mode);

	if (!wasDisabled) freezeModeChanged ();
}

//------------------------------------------------------------------------------
const cFreezeModes& cClient::getFreezeModes () const
{
	return freezeModes;
}

//------------------------------------------------------------------------------
const std::map<int, ePlayerConnectionState>& cClient::getPlayerConnectionStates() const
{
	return playerConnectionStates;
}

//------------------------------------------------------------------------------
void cClient::addSurveyorMoveJob(const cVehicle& vehicle)
{
	if (!vehicle.getStaticUnitData().canSurvey) return;
	
	sendNetMessage(cActionSetAutoMove(vehicle, true));

	//don't add new job, if there is already one for this vehicle
	auto it = std::find_if(surveyorAiJobs.begin(), surveyorAiJobs.end(), [&](const std::unique_ptr<cSurveyorAi>& job)
	{
		return job->getVehicle().getId() == vehicle.getId();
	});
	if (it != surveyorAiJobs.end())
	{
		return;
	}
	
	surveyorAiJobs.push_back(std::make_unique<cSurveyorAi>(vehicle));
}

//------------------------------------------------------------------------------
void cClient::removeSurveyorMoveJob(const cVehicle& vehicle)
{
	sendNetMessage(cActionSetAutoMove(vehicle, false));

	auto it = std::find_if(surveyorAiJobs.begin(), surveyorAiJobs.end(), [&](const std::unique_ptr<cSurveyorAi>& job)
	{
		return job->getVehicle().getId() == vehicle.getId();
	});
	if (it != surveyorAiJobs.end())
	{
		surveyorAiJobs.erase(it);
	}
}

//------------------------------------------------------------------------------
void cClient::recreateSurveyorMoveJobs()
{
	surveyorAiJobs.clear();

	for (const auto& vehicle : activePlayer->getVehicles())
	{
		if (vehicle->isSurveyorAutoMoveActive())
		{
			surveyorAiJobs.push_back(std::make_unique<cSurveyorAi>(*vehicle));
		}
	}
}

//------------------------------------------------------------------------------
void cClient::loadModel(int saveGameNumber, int playerNr)
{
	cSavegame savegame;
	savegame.loadModel(model, saveGameNumber);

	activePlayer = model.getPlayerList()[playerNr].get();

	recreateSurveyorMoveJobs();

	Log.write(" Client: loaded model. GameId: " + toString(model.getGameId()), cLog::eLOG_TYPE_NET_DEBUG);
}
//------------------------------------------------------------------------------
void cClient::run()
{
	gameTimer->run(*this, model);
}
