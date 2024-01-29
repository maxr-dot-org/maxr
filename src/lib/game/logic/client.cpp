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

#include "game/logic/client.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/data/savegame.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/action.h"
#include "game/logic/action/actionactivate.h"
#include "game/logic/action/actionattack.h"
#include "game/logic/action/actionbuyupgrades.h"
#include "game/logic/action/actionchangebuildlist.h"
#include "game/logic/action/actionchangemanualfire.h"
#include "game/logic/action/actionchangeresearch.h"
#include "game/logic/action/actionchangesentry.h"
#include "game/logic/action/actionchangeunitname.h"
#include "game/logic/action/actionclear.h"
#include "game/logic/action/actionendturn.h"
#include "game/logic/action/actionfinishbuild.h"
#include "game/logic/action/actioninitnewgame.h"
#include "game/logic/action/actionload.h"
#include "game/logic/action/actionminelayerstatus.h"
#include "game/logic/action/actionrepairreload.h"
#include "game/logic/action/actionresourcedistribution.h"
#include "game/logic/action/actionresumemove.h"
#include "game/logic/action/actionselfdestroy.h"
#include "game/logic/action/actionsetautomove.h"
#include "game/logic/action/actionstartbuild.h"
#include "game/logic/action/actionstartmove.h"
#include "game/logic/action/actionstartturn.h"
#include "game/logic/action/actionstartwork.h"
#include "game/logic/action/actionstealdisable.h"
#include "game/logic/action/actionstop.h"
#include "game/logic/action/actiontransfer.h"
#include "game/logic/action/actionupgradebuilding.h"
#include "game/logic/action/actionupgradevehicle.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/fxeffects.h"
#include "game/logic/gametimer.h"
#include "game/logic/server.h"
#include "game/logic/surveyorai.h"
#include "game/logic/turntimeclock.h"
#include "game/protocol/netmessage.h"
#include "game/startup/lobbypreparationdata.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/ranges.h"
#include "utility/serialization/jsonarchive.h"

//------------------------------------------------------------------------------
cClient::cClient (std::shared_ptr<cConnectionManager> connectionManager) :
	connectionManager (connectionManager),
	gameTimer (std::make_shared<cGameTimerClient>())
{
	gameTimer->start();
}

//------------------------------------------------------------------------------
cClient::~cClient()
{
	connectionManager->setLocalClient (nullptr, -1);
	gameTimer->stop();
}

//------------------------------------------------------------------------------
void cClient::setMap (std::shared_ptr<cStaticMap> staticMap)
{
	model.setMap (staticMap);
}

//------------------------------------------------------------------------------
void cClient::setPreparationData (const sLobbyPreparationData& preparationData)
{
	model.setUnitsData (std::make_shared<cUnitsData> (*preparationData.unitsData));
	model.setGameSettings (*preparationData.gameSettings);
	model.setMap (preparationData.staticMap);
}

//------------------------------------------------------------------------------
void cClient::setPlayers (const std::vector<cPlayerBasicData>& splayers, size_t activePlayerNr)
{
	model.setPlayerList (splayers);
	activePlayer = model.getPlayer (activePlayerNr);
}

//------------------------------------------------------------------------------
void cClient::pushMessage (std::unique_ptr<cNetMessage> message)
{
	if (message->getType() == eNetMessageType::GAMETIME_SYNC_SERVER)
	{
		// This is a preview for the client to know
		// how many sync messages are in queue.
		// Used to detect a growing lag behind the server time
		const cNetMessageSyncServer* syncMessage = static_cast<const cNetMessageSyncServer*> (message.get());
		gameTimer->setReceivedTime (syncMessage->gameTime);
	}
	eventQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
void cClient::sendNetMessage (cNetMessage&& message) const
{
	message.playerNr = activePlayer->getId();

	if (message.getType() != eNetMessageType::GAMETIME_SYNC_CLIENT)
	{
		nlohmann::json json;
		cJsonArchiveOut jsonarchive (json);
		jsonarchive << message;
		NetLog.debug (getActivePlayer().getName() + ": --> " + json.dump (-1) + " @" + std::to_string (model.getGameTime()));
	}
	connectionManager->sendToServer (message);
}

//------------------------------------------------------------------------------
void cClient::sendSyncMessage (unsigned int gameTime, bool crcOK, unsigned int timeBuffer, unsigned int ticksPerFrame, unsigned int eventCounter) const
{
	cNetMessageSyncClient message;
	message.gameTime = gameTime;
	message.crcOK = crcOK;
	message.timeBuffer = timeBuffer;
	message.ticksPerFrame = ticksPerFrame;
	message.queueSize = static_cast<unsigned int> (eventQueue.safe_size());
	message.eventCounter = eventCounter;

	sendNetMessage (std::move (message));
}

//------------------------------------------------------------------------------
void cClient::runClientJobs (const cModel& model)
{
	// run surveyor AI
	handleSurveyorMoveJobs();
}

void cClient::handleNetMessages()
{
	std::unique_ptr<cNetMessage> message;
	while (eventQueue.try_pop (message))
	{
		if (message->getType() != eNetMessageType::GAMETIME_SYNC_SERVER && message->getType() != eNetMessageType::RESYNC_MODEL)
		{
			nlohmann::json json;
			cJsonArchiveOut jsonarchive (json);
			jsonarchive << *message;
			NetLog.debug (getActivePlayer().getName() + ": <-- " + json.dump (-1) + " @" + std::to_string (model.getGameTime()));
		}

		switch (message->getType())
		{
			case eNetMessageType::REPORT:
			{
				if (message->playerNr != -1 && model.getPlayer (message->playerNr) == nullptr) continue;

				cNetMessageReport* chatMessage = static_cast<cNetMessageReport*> (message.get());
				reportMessageReceived (chatMessage->playerNr, chatMessage->report, activePlayer->getId());
			}
			break;
			case eNetMessageType::ACTION:
			{
				if (model.getPlayer (message->playerNr) == nullptr) continue;

				const cAction* action = static_cast<cAction*> (message.get());
				action->execute (model);
			}
			break;
			case eNetMessageType::GAMETIME_SYNC_SERVER:
			{
				const cNetMessageSyncServer* syncMessage = static_cast<cNetMessageSyncServer*> (message.get());
				gameTimer->handleSyncMessage (*syncMessage, model.getGameTime());
				return; //stop processing messages after receiving a sync message. Gametime needs to be increased before handling the next message.
			}
			break;
			case eNetMessageType::RANDOM_SEED:
			{
				const cNetMessageRandomSeed* msg = static_cast<cNetMessageRandomSeed*> (message.get());
				model.randomGenerator.seed (msg->seed);
			}
			break;
			case eNetMessageType::REQUEST_GUI_SAVE_INFO:
			{
				const cNetMessageRequestGUISaveInfo* msg = static_cast<cNetMessageRequestGUISaveInfo*> (message.get());
				guiSaveInfoRequested (msg->slot, msg->savingID);
			}
			break;
			case eNetMessageType::GUI_SAVE_INFO:
			{
				const cNetMessageGUISaveInfo* msg = static_cast<cNetMessageGUISaveInfo*> (message.get());
				if (msg->playerNr != activePlayer->getId()) continue;
				guiSaveInfoReceived (*msg);
			}
			break;
			case eNetMessageType::RESYNC_MODEL:
			{
				NetLog.debug (" Client: Received model data for resynchronization");
				const cNetMessageResyncModel* msg = static_cast<cNetMessageResyncModel*> (message.get());
				try
				{
					msg->apply (model);
					recreateSurveyorMoveJobs();
					gameTimer->sendSyncMessage (*this, model.getGameTime(), 0, 0);
				}
				catch (const std::runtime_error& e)
				{
					NetLog.error (std::string (" Client: error loading received model data: ") + e.what());
				}

				//FIXME: deserializing model does not trigger signals on changed data members. Use this signal to trigger some gui updates
				freezeModeChanged();
				resynced();
			}
			break;
			case eNetMessageType::FREEZE_MODES:
			{
				const cNetMessageFreezeModes* msg = static_cast<cNetMessageFreezeModes*> (message.get());

				// don't overwrite waitForServer flag
				bool waitForServer = freezeModes.isEnabled (eFreezeMode::WaitForServer);
				freezeModes = msg->freezeModes;
				if (waitForServer) freezeModes.enable (eFreezeMode::WaitForServer);

				for (const auto& [playerId, conn] : msg->playerStates)
				{
					if (model.getPlayer (playerId) == nullptr)
					{
						NetLog.error (" Client: Invalid player id: " + std::to_string (playerId));
						break;
					}
				}
				if (msg->playerStates.size() != model.getPlayerList().size())
				{
					NetLog.error (" Client: Wrong size of playerState map " + std::to_string (msg->playerStates.size()));
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
				NetLog.warn (" Client: received unknown net message type");
				break;
		}
	}
}

//------------------------------------------------------------------------------
void cClient::handleSurveyorMoveJobs()
{
	for (auto& job : surveyorAiJobs)
	{
		job->run (*this, surveyorAiJobs);
	}
	EraseIf (surveyorAiJobs, [] (auto& job) { return job->isFinished(); });
}

//------------------------------------------------------------------------------
void cClient::enableFreezeMode (eFreezeMode mode)
{
	NetLog.debug (" Client: enabled freeze mode: " + serialization::enumToString (mode));
	const auto wasEnabled = freezeModes.isEnabled (mode);

	freezeModes.enable (mode);

	if (!wasEnabled) freezeModeChanged();
}

//------------------------------------------------------------------------------
void cClient::disableFreezeMode (eFreezeMode mode)
{
	NetLog.debug (" Client: disabled freeze mode: " + serialization::enumToString (mode));
	const auto wasDisabled = !freezeModes.isEnabled (mode);

	freezeModes.disable (mode);

	if (!wasDisabled) freezeModeChanged();
}

//------------------------------------------------------------------------------
const cFreezeModes& cClient::getFreezeModes() const
{
	return freezeModes;
}

//------------------------------------------------------------------------------
const std::map<int, ePlayerConnectionState>& cClient::getPlayerConnectionStates() const
{
	return playerConnectionStates;
}

//------------------------------------------------------------------------------
void cClient::addSurveyorMoveJob (const cVehicle& vehicle)
{
	if (!vehicle.getStaticData().canSurvey) return;

	sendNetMessage (cActionSetAutoMove (vehicle, true));

	//don't add new job, if there is already one for this vehicle
	auto it = ranges::find_if (surveyorAiJobs, [&] (const std::unique_ptr<cSurveyorAi>& job) {
		return job->getVehicle().getId() == vehicle.getId();
	});
	if (it != surveyorAiJobs.end())
	{
		return;
	}
	surveyorAiJobs.push_back (std::make_unique<cSurveyorAi> (vehicle));
}

//------------------------------------------------------------------------------
void cClient::removeSurveyorMoveJob (const cVehicle& vehicle)
{
	sendNetMessage (cActionSetAutoMove (vehicle, false));

	auto it = ranges::find_if (surveyorAiJobs, [&] (const std::unique_ptr<cSurveyorAi>& job) {
		return job->getVehicle().getId() == vehicle.getId();
	});
	if (it != surveyorAiJobs.end())
	{
		surveyorAiJobs.erase (it);
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
			surveyorAiJobs.push_back (std::make_unique<cSurveyorAi> (*vehicle));
		}
	}
}

//------------------------------------------------------------------------------
void cClient::loadModel (int saveGameNumber, int playerNr)
{
	cSavegame savegame;
	savegame.loadModel (model, saveGameNumber);

	activePlayer = model.getPlayerList()[playerNr].get();

	recreateSurveyorMoveJobs();

	NetLog.debug (" Client: loaded model. GameId: " + std::to_string (model.getGameId()));
}
//------------------------------------------------------------------------------
void cClient::run()
{
	gameTimer->run (*this, model);
}

//------------------------------------------------------------------------------
void cClient::activateUnit (const cUnit& containingUnit, const cVehicle& activatedVehicle, const cPosition& position)
{
	sendNetMessage (cActionActivate (containingUnit, activatedVehicle, position));
}

//------------------------------------------------------------------------------
void cClient::attack (const cUnit& aggressor, const cPosition& targetPosition, const cUnit* targetUnit)
{
	sendNetMessage (cActionAttack (aggressor, targetPosition, targetUnit));
}

//------------------------------------------------------------------------------
void cClient::buyUpgrades (const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
{
	sendNetMessage (cActionBuyUpgrades (unitUpgrades));
}

//------------------------------------------------------------------------------
void cClient::changeBuildList (const cBuilding& building, const std::vector<sID>& buildList, int buildSpeed, bool repeat)
{
	sendNetMessage (cActionChangeBuildList (building, buildList, buildSpeed, repeat));
}

//------------------------------------------------------------------------------
void cClient::changeManualFire (const cUnit& unit)
{
	sendNetMessage (cActionChangeManualFire (unit));
}

//------------------------------------------------------------------------------
void cClient::changeResearch (const std::array<int, cResearch::kNrResearchAreas>& researchAreas)
{
	sendNetMessage (cActionChangeResearch (researchAreas));
}

//------------------------------------------------------------------------------
void cClient::changeSentry (const cUnit& unit)
{
	sendNetMessage (cActionChangeSentry (unit));
}

//------------------------------------------------------------------------------
void cClient::changeUnitName (const cUnit& unit, const std::string& name)
{
	sendNetMessage (cActionChangeUnitName (unit, name));
}

//------------------------------------------------------------------------------
void cClient::startClearRubbles (const cVehicle& vehicle)
{
	sendNetMessage (cActionClear (vehicle));
}

//------------------------------------------------------------------------------
void cClient::endTurn()
{
	if (!getFreezeModes().isFreezed()) sendNetMessage (cActionEndTurn());
}

//------------------------------------------------------------------------------
void cClient::finishBuild (const cUnit& unit, const cPosition& escapePosition)
{
	sendNetMessage (cActionFinishBuild (unit, escapePosition));
}

//------------------------------------------------------------------------------
void cClient::initNewGame (const sInitPlayerData& initPlayerData)
{
	sendNetMessage (cActionInitNewGame (initPlayerData));
}

//------------------------------------------------------------------------------
void cClient::load (const cUnit& loadingUnit, const cVehicle& loadedVehicle)
{
	sendNetMessage (cActionLoad (loadingUnit, loadedVehicle));
}

//------------------------------------------------------------------------------
void cClient::toggleLayMines (const cVehicle& vehicle)
{
	sendNetMessage (cActionMinelayerStatus (vehicle, !vehicle.isUnitLayingMines(), false));
}

//------------------------------------------------------------------------------
void cClient::toggleCollectMines (const cVehicle& vehicle)
{
	sendNetMessage (cActionMinelayerStatus (vehicle, false, !vehicle.isUnitClearingMines()));
}

//------------------------------------------------------------------------------
void cClient::rearm (const cUnit& sourceUnit, const cUnit& destUnit)
{
	sendNetMessage (cActionRepairReload (sourceUnit, destUnit, eSupplyType::REARM));
}

//------------------------------------------------------------------------------
void cClient::repair (const cUnit& sourceUnit, const cUnit& destUnit)
{
	sendNetMessage (cActionRepairReload (sourceUnit, destUnit, eSupplyType::REPAIR));
}

//------------------------------------------------------------------------------
void cClient::changeResourceDistribution (const cBuilding& building, const sMiningResource& miningResource)
{
	sendNetMessage (cActionResourceDistribution (building, miningResource));
}

//------------------------------------------------------------------------------
void cClient::resumeMoveJob (const cVehicle& vehicle)
{
	sendNetMessage (cActionResumeMove (vehicle));
}

//------------------------------------------------------------------------------
void cClient::resumeAllMoveJobs()
{
	sendNetMessage (cActionResumeMove());
}

//------------------------------------------------------------------------------
void cClient::selfDestroy (const cBuilding& building)
{
	sendNetMessage (cActionSelfDestroy (building));
}

//------------------------------------------------------------------------------
void cClient::setAutoMove (const cVehicle& vehicle, bool value)
{
	sendNetMessage (cActionSetAutoMove (vehicle, value));
}

//------------------------------------------------------------------------------
void cClient::startBuild (const cVehicle& vehicle, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition)
{
	sendNetMessage (cActionStartBuild (vehicle, buildingTypeID, buildSpeed, buildPosition));
}

//------------------------------------------------------------------------------
void cClient::startBuildPath (const cVehicle& vehicle, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition, const cPosition& pathEndPosition)
{
	sendNetMessage (cActionStartBuild (vehicle, buildingTypeID, buildSpeed, buildPosition, pathEndPosition));
}

//------------------------------------------------------------------------------
void cClient::startMove (const cVehicle& vehicle, const std::forward_list<cPosition>& path, eStart start, eStopOn stopOn, cEndMoveAction emat)
{
	sendNetMessage (cActionStartMove (vehicle, path, start, stopOn, emat));
}

//------------------------------------------------------------------------------
void cClient::startTurn()
{
	sendNetMessage (cActionStartTurn{});
}

//------------------------------------------------------------------------------
void cClient::startWork (const cBuilding& building)
{
	sendNetMessage (cActionStartWork (building));
}

//------------------------------------------------------------------------------
void cClient::disable (const cVehicle& infiltrator, const cUnit& target)
{
	sendNetMessage (cActionStealDisable (infiltrator, target, false));
}

//------------------------------------------------------------------------------
void cClient::steal (const cVehicle& infiltrator, const cUnit& target)
{
	sendNetMessage (cActionStealDisable (infiltrator, target, true));
}

//------------------------------------------------------------------------------
void cClient::stopWork (const cUnit& unit)
{
	sendNetMessage (cActionStop (unit));
}

//------------------------------------------------------------------------------
void cClient::transfer (const cUnit& sourceUnit, const cUnit& destinationUnit, int transferValue, eResourceType resourceType)
{
	if (transferValue != 0)
	{
		sendNetMessage (cActionTransfer (sourceUnit, destinationUnit, transferValue, resourceType));
	}
}

//------------------------------------------------------------------------------
void cClient::upgradeAllBuildings (const cBuilding& building)
{
	sendNetMessage (cActionUpgradeBuilding (building, true));
}

//------------------------------------------------------------------------------
void cClient::upgradeBuilding (const cBuilding& building)
{
	sendNetMessage (cActionUpgradeBuilding (building, false));
}

//------------------------------------------------------------------------------
void cClient::upgradeAllVehicles (const cBuilding& containingBuilding)
{
	sendNetMessage (cActionUpgradeVehicle (containingBuilding));
}

//------------------------------------------------------------------------------
void cClient::upgradeVehicle (const cBuilding& containingBuilding, const cVehicle& vehicle)
{
	sendNetMessage (cActionUpgradeVehicle (containingBuilding, &vehicle));
}

//------------------------------------------------------------------------------
void cClient::report (std::unique_ptr<cSavedReport> report)
{
	sendNetMessage (cNetMessageReport (std::move (report)));
}

//------------------------------------------------------------------------------
void cClient::sendGUISaveInfo (int slot, int savingId, const sPlayerGuiInfo& guiInfo, std::optional<cGameGuiState> gameGuiState)
{
	cNetMessageGUISaveInfo message (slot, savingId);
	message.guiInfo = guiInfo;

	if (gameGuiState)
	{
		message.guiInfo.gameGuiState = *gameGuiState;
	}
	sendNetMessage (std::move (message));
}
