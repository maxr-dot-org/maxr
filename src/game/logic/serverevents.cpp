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
#include "game/logic/serverevents.h"

#include "game/data/units/building.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
//#include "menuevents.h"
#include "game/logic/movejobs.h"
#include "netmessage.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/logic/upgradecalculator.h"
#include "game/data/units/vehicle.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/game/gameguistate.h"
#include "game/data/report/savedreport.h"

//------------------------------------------------------------------------------
void sendAddUnit (cServer& server, const cPosition& position, int id, bool isVehicle, sID unitID, const cPlayer& player, bool isInit, bool shouldAddToMap)
{
	std::unique_ptr<cNetMessage> message;

	if (isVehicle) message = std::make_unique<cNetMessage> (GAME_EV_ADD_VEHICLE);
	else message = std::make_unique<cNetMessage> (GAME_EV_ADD_BUILDING);

	message->pushBool (shouldAddToMap);
	message->pushInt16 (id);
	message->pushPosition (position);
	message->pushID (unitID);
	message->pushInt16 (player.getId());
	message->pushBool (isInit);

	server.sendNetMessage (std::move (message), &player);
}

//------------------------------------------------------------------------------
void sendAddRubble (cServer& server, const cBuilding& building, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_ADD_RUBBLE);

	message->pushPosition (building.getPosition());
	message->pushInt16 (building.iID);
	message->pushInt16 (building.RubbleValue);
	message->pushInt16 (building.RubbleTyp);
	message->pushBool (building.getIsBig());

	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendDeleteUnitMessage (cServer& server, const cUnit& unit, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (unit.isABuilding() ? GAME_EV_DEL_BUILDING : GAME_EV_DEL_VEHICLE);
	message->pushInt16 (unit.iID);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendDeleteUnit (cServer& server, const cUnit& unit, const cPlayer* receiver)
{
	if (receiver == nullptr)
	{
		for (unsigned int i = 0; i < unit.seenByPlayerList.size(); i++)
			sendDeleteUnitMessage (server, unit, *unit.seenByPlayerList[i]);

		//send message to owner, since he is not in the seenByPlayerList
		if (unit.getOwner() != 0)
			sendDeleteUnitMessage (server, unit, *unit.getOwner());
	}
	else
		sendDeleteUnitMessage (server, unit, *receiver);
}

//------------------------------------------------------------------------------
void sendAddEnemyUnit (cServer& server, const cUnit& unit, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (unit.isABuilding() ? GAME_EV_ADD_ENEM_BUILDING : GAME_EV_ADD_ENEM_VEHICLE);

	message->pushInt16 (unit.data.getVersion());
	message->pushInt16 (unit.iID);
	if (unit.isAVehicle())
		message->pushInt16 (unit.dir);
	message->pushPosition (unit.getPosition());
	message->pushID (unit.data.getId());
	message->pushInt16 (unit.getOwner()->getId());

	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendMakeTurnEnd (cServer& server, const cPlayer* receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_MAKE_TURNEND);

	server.sendNetMessage (std::move (message), receiver);
}

//------------------------------------------------------------------------------
void sendTurnFinished (cServer& server, const cPlayer& playerWhoEndedTurn, const cPlayer* nextPlayer, const cPlayer* receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_FINISHED_TURN);

	if (nextPlayer)
	{
		message->pushInt16 (nextPlayer->getId());
		message->pushBool (true);
	}
	else
	{
		message->pushBool (false);
	}
	message->pushInt16 (playerWhoEndedTurn.getId());

	server.sendNetMessage (std::move (message), receiver);
}

//------------------------------------------------------------------------------
void sendTurnStartTime (cServer& server, unsigned int gameTime)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_TURN_START_TIME);

	message->pushInt32 (gameTime);

	server.sendNetMessage (std::move (message));
}

//------------------------------------------------------------------------------
void sendTurnEndDeadlineStartTime (cServer& server, unsigned int gameTime)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_TURN_END_DEADLINE_START_TIME);

	message->pushInt32 (gameTime);

	server.sendNetMessage (std::move (message));
}

//------------------------------------------------------------------------------
void sendSpecificUnitData (cServer& server, const cVehicle& vehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_SPECIFIC_UNIT_DATA);
	message->pushPosition (vehicle.bandPosition);
	message->pushBool (vehicle.BuildPath);
	message->pushID (vehicle.getBuildingType());
	message->pushInt16 (vehicle.dir);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (std::move (message), vehicle.getOwner());
}

//------------------------------------------------------------------------------
void sendNextMove (cServer& server, const cVehicle& vehicle, int iType, int iSavedSpeed)
{
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
	{
		auto message = std::make_unique<cNetMessage> (GAME_EV_NEXT_MOVE);
		if (iSavedSpeed >= 0) message->pushChar (iSavedSpeed);
		message->pushChar (iType);
		message->pushInt16 (vehicle.iID);
		server.sendNetMessage (std::move (message), vehicle.seenByPlayerList[i]);
	}

	auto message = std::make_unique<cNetMessage> (GAME_EV_NEXT_MOVE);
	if (iSavedSpeed >= 0) message->pushChar (iSavedSpeed);
	message->pushChar (iType);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (std::move (message), vehicle.getOwner());
}

//------------------------------------------------------------------------------
void sendMoveJobServer (cServer& server, const cServerMoveJob& moveJob, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_MOVE_JOB_SERVER);

	const sWaypoint* waypoint = moveJob.Waypoints;
	int iCount = 0;
	while (waypoint)
	{
		message->pushInt16 (waypoint->Costs);
		message->pushPosition (waypoint->position);

		if (message->iLength > PACKAGE_LENGTH - 19)
		{
			Log.write (" Server: Error sending movejob: message too long", cLog::eLOG_TYPE_NET_ERROR);
			return; // don't send movejobs that are to long
		}

		waypoint = waypoint->next;
		iCount++;
	}

	message->pushInt16 (iCount);
	message->pushInt16 (moveJob.iSavedSpeed);
	message->pushPosition (moveJob.destination);
	message->pushPosition (moveJob.source);
	message->pushInt32 (moveJob.Vehicle->iID);

	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendVehicleResources (cServer& server, const cVehicle& vehicle)
{
	int iCount = 0;
	auto message = std::make_unique<cNetMessage> (GAME_EV_RESOURCES);
	const cMap& map = *server.Map;
	// TODO: only send new scaned resources

	const int minx = std::max (vehicle.getPosition().x() - 1, 0);
	const int maxx = std::min (vehicle.getPosition().x() + 1, map.getSize().x() - 1);
	const int miny = std::max (vehicle.getPosition().y() - 1, 0);
	const int maxy = std::min (vehicle.getPosition().y() + 1, map.getSize().y() - 1);
	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const cPosition position (x, y);
			if (vehicle.getOwner()->hasResourceExplored (position)) continue;

			const sResources& resource = map.getResource (position);
			message->pushInt16 (resource.value);
			message->pushInt16 (resource.typ);
			message->pushPosition (position);
			iCount++;
		}
	}
	message->pushInt16 (iCount);

	server.sendNetMessage (std::move (message), vehicle.getOwner());
}

//------------------------------------------------------------------------------
void sendResources (cServer& server, const cPlayer& player)
{
	int iCount = 0;
	auto message = std::make_unique<cNetMessage> (GAME_EV_RESOURCES);
	for (int x = 0; x != server.Map->getSize().x(); ++x)
	{
		for (int y = 0; y != server.Map->getSize().y(); ++y)
		{
			const cPosition position (x, y);

			if (!player.hasResourceExplored (position)) continue;

			const sResources& resource = server.Map->getResource (position);
			message->pushInt16 (resource.value);
			message->pushInt16 (resource.typ);
			message->pushPosition (position);
			iCount++;

			if (message->iLength >= PACKAGE_LENGTH - 10)
			{
				message->pushInt16 (iCount);
				server.sendNetMessage (std::move (message), &player);
				message = std::make_unique<cNetMessage> (GAME_EV_RESOURCES);
				iCount = 0;
			}
		}
	}
	if (iCount > 0)
	{
		message->pushInt16 (iCount);
		server.sendNetMessage (std::move (message), &player);
	}
}

//------------------------------------------------------------------------------
void sendScore (cServer& server, const cPlayer& subject, int turn, const cPlayer* receiver)
{
	if (!receiver)
	{
		const auto& playerList = server.playerList;
		for (unsigned int n = 0; n < playerList.size(); n++)
			sendScore (server, subject, turn, playerList[n].get());
	}
	else
	{
		auto message = std::make_unique<cNetMessage> (GAME_EV_SCORE);
		message->pushInt16 (subject.getScore (turn));
		message->pushInt16 (turn);
		message->pushInt16 (subject.getId());

		server.sendNetMessage (std::move (message), receiver);
	}
}

void sendUnitScore (cServer& server, const cBuilding& building)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_UNIT_SCORE);
	message->pushInt16 (building.points);
	message->pushInt16 (building.iID);
	server.sendNetMessage (std::move (message), building.getOwner());
}

void sendNumEcos (cServer& server, cPlayer& subject, const cPlayer* receiver)
{
	subject.countEcoSpheres();

	if (!receiver)
	{
		const auto& playerList = server.playerList;
		for (unsigned int n = 0; n < playerList.size(); n++)
			sendNumEcos (server, subject, playerList[n].get());
	}
	else
	{
		auto message = std::make_unique<cNetMessage> (GAME_EV_NUM_ECOS);
		message->pushInt16 (subject.numEcos);
		message->pushInt16 (subject.getId());

		server.sendNetMessage (std::move (message), receiver);
	}
}

//------------------------------------------------------------------------------
void sendBuildAnswer (cServer& server, bool bOK, const cVehicle& vehicle)
{
	//message for the owner
	auto message = std::make_unique<cNetMessage> (GAME_EV_BUILD_ANSWER);
	if (bOK)
	{
		message->pushPosition (vehicle.bandPosition);
		message->pushBool (vehicle.BuildPath);
		message->pushInt16 (vehicle.getBuildTurns());
		message->pushID (vehicle.getBuildingType());
	//	message->pushBool (vehicle.getBuildingType().getUnitDataOriginalVersion()->isBig);
		message->pushPosition (vehicle.getPosition());
	}

	message->pushInt16 (vehicle.iID);
	message->pushBool (bOK);
	server.sendNetMessage (std::move (message), vehicle.getOwner());

	//message for the enemys
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
	{
		auto message = std::make_unique<cNetMessage> (GAME_EV_BUILD_ANSWER);
		if (bOK)
		{
	//		message->pushBool (vehicle.getBuildingType().getUnitDataOriginalVersion()->isBig);
			message->pushPosition (vehicle.getPosition());
		}
		message->pushInt16 (vehicle.iID);
		message->pushBool (bOK);
		server.sendNetMessage (std::move (message), vehicle.seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void sendStopBuild (cServer& server, int iVehicleID, const cPosition& newPosition, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_STOP_BUILD);
	message->pushPosition (newPosition);
	message->pushInt16 (iVehicleID);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendBuildList (cServer& server, const cBuilding& building)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_BUILDLIST);
	message->pushBool (building.getRepeatBuild());
	message->pushInt16 (building.getBuildSpeed());
	message->pushInt16 (building.getMetalPerRound());
	for (int i = (int) building.getBuildListSize() - 1; i >= 0; i--)
	{
		message->pushInt16 (building.getBuildListItem (i).getRemainingMetal());
		message->pushID (building.getBuildListItem (i).getType());
	}
	message->pushInt16 ((int)building.getBuildListSize());
	message->pushInt16 (building.iID);
	server.sendNetMessage (std::move (message), building.getOwner());
}

//------------------------------------------------------------------------------
void sendSupply (cServer& server, int iDestID, bool bDestVehicle, int iValue, int iType, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_SUPPLY);
	message->pushInt16 (iValue);
	message->pushInt16 (iDestID);
	message->pushBool (bDestVehicle);
	message->pushChar (iType);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendDetectionState (cServer& server, const cVehicle& vehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_DETECTION_STATE);
	message->pushBool (!vehicle.detectedByPlayerList.empty());
	message->pushInt32 (vehicle.iID);
	server.sendNetMessage (std::move (message), vehicle.getOwner());
}

//------------------------------------------------------------------------------
void sendClearAnswer (cServer& server, int answertype, const cVehicle& vehicle, int turns, const cPosition& bigPosition, const cPlayer* receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_CLEAR_ANSWER);
	message->pushPosition (bigPosition);
	message->pushInt16 (turns);
	message->pushInt16 (vehicle.iID);
	message->pushInt16 (answertype);
	server.sendNetMessage (std::move (message), receiver);
}

//------------------------------------------------------------------------------
void sendStopClear (cServer& server, const cVehicle& vehicle, const cPosition& bigPosition, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_STOP_CLEARING);
	message->pushPosition (bigPosition);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendNoFog (cServer& server, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_NOFOG);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendDefeated (cServer& server, const cPlayer& player, const cPlayer* receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_DEFEATED);
	message->pushInt16 (player.getId());
	server.sendNetMessage (std::move (message), receiver);
}

//------------------------------------------------------------------------------
void sendWaitFor (cServer& server, const cPlayer& player, const cPlayer* receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WAIT_FOR);
	message->pushInt32 (player.getId());
	server.sendNetMessage (std::move (message), receiver);
}

//------------------------------------------------------------------------------
void sendDeletePlayer (cServer& server, const cPlayer& player, const cPlayer* receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_DEL_PLAYER);
	message->pushInt16 (player.getId());
	server.sendNetMessage (std::move (message), receiver);
}

//------------------------------------------------------------------------------
void sendTurn (cServer& server, int turn, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_TURN);
	message->pushInt16 (turn);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendStoreVehicle (cServer& server, int unitid, bool vehicle, int storedunitid, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_STORE_UNIT);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (storedunitid);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendActivateVehicle (cServer& server, int unitid, bool vehicle, int activatunitid, const cPosition& position, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_EXIT_UNIT);
	message->pushPosition (position);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (activatunitid);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendDeleteEverything (cServer& server, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_DELETE_EVERYTHING);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendResearchLevel (cServer& server, const cResearch& researchLevel, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_RESEARCH_LEVEL);
	for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	{
		message->pushInt16 (researchLevel.getCurResearchLevel (area));
		message->pushInt16 (researchLevel.getCurResearchPoints (area));
	}
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendFinishedResearchAreas (cServer& server, const std::vector<int>& areas, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_FINISHED_RESEARCH_AREAS);
	for (size_t i = 0; i < areas.size(); ++i)
	{
		message->pushInt32 (areas[i]);
	}
	message->pushInt32 (areas.size());
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendRefreshResearchCount (cServer& server, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_REFRESH_RESEARCH_COUNT);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendUnitUpgrades (cServer& server, const cDynamicUnitData& unitData, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_UNIT_UPGRADE_VALUES);
	message->pushInt16 (unitData.getHitpointsMax());
	message->pushInt16 (unitData.getAmmoMax());
	message->pushInt16 (unitData.getShotsMax());
	message->pushInt16 (unitData.getSpeedMax());
	message->pushInt16 (unitData.getArmor());
	message->pushInt16 (unitData.getBuildCost());
	message->pushInt16 (unitData.getDamage());
	message->pushInt16 (unitData.getRange());
	message->pushInt16 (unitData.getScan());
	message->pushInt16 (unitData.getVersion());
	message->pushID (unitData.getId());
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendCredits (cServer& server, int newCredits, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_CREDITS_CHANGED);
	message->pushInt32 (newCredits);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendUpgradeBuildings (cServer& server, const std::vector<cBuilding*>& upgradedBuildings, int totalCosts, const cPlayer& receiver)
{
	// send to owner
	std::unique_ptr<cNetMessage> message;
	int buildingsInMsg = 0;
	for (unsigned int i = 0; i < upgradedBuildings.size(); i++)
	{
		if (message == nullptr)
		{
			message = std::make_unique<cNetMessage> (GAME_EV_UPGRADED_BUILDINGS);
			buildingsInMsg = 0;
		}

		message->pushInt32 (upgradedBuildings[i]->iID);
		buildingsInMsg++;
		if (message->iLength + 8 > PACKAGE_LENGTH)
		{
			message->pushInt16 ((totalCosts * buildingsInMsg) / (int) upgradedBuildings.size());
			message->pushInt16 (buildingsInMsg);
			server.sendNetMessage (std::move (message), &receiver);
		}
	}
	if (message != nullptr)
	{
		message->pushInt16 ((int) (totalCosts * buildingsInMsg) / (int) upgradedBuildings.size());
		message->pushInt16 (buildingsInMsg);
		server.sendNetMessage (std::move (message), &receiver);
	}

	// send to other players
	const auto& playerList = server.playerList;
	for (unsigned int n = 0; n < playerList.size(); n++)
	{
		const auto curPlayer = playerList[n].get();
		// don't send to the owner of the buildings
		if (curPlayer == 0 || curPlayer->getId() == receiver.getId()) continue;

		for (unsigned int buildingIdx = 0; buildingIdx < upgradedBuildings.size(); buildingIdx++)
		{
			//if (Contains (upgradedBuildings[buildingIdx]->seenByPlayerList, curPlayer)) // that player can see the building
				//sendUnitData (server, *upgradedBuildings[buildingIdx], *curPlayer);
		}
	}
}

//------------------------------------------------------------------------------
void sendUpgradeVehicles (cServer& server, const std::vector<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, const cPlayer& receiver)
{
	if (upgradedVehicles.size() * 4 > PACKAGE_LENGTH - 50)
	{
		Log.write ("Server: sendUpgradeVehicles: Message would exceed messagesize!!!", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	// send to owner
	auto message = std::make_unique<cNetMessage> (GAME_EV_UPGRADED_VEHICLES);
	for (unsigned int i = 0; i < upgradedVehicles.size(); i++)
		message->pushInt32 (upgradedVehicles[i]->iID);

	message->pushInt32 (storingBuildingID);
	message->pushInt16 (totalCosts);
	message->pushInt16 ((int) upgradedVehicles.size());
	server.sendNetMessage (std::move (message), &receiver);

	//TODO: send to other players as well?
}

//------------------------------------------------------------------------------
void sendResearchSettings (cServer& server, const std::vector<cBuilding*>& researchCentersToChangeArea, const std::vector<cResearch::ResearchArea>& newAreasForResearchCenters, const cPlayer& receiver)
{
	if (researchCentersToChangeArea.size() != newAreasForResearchCenters.size())
		return;

	std::unique_ptr<cNetMessage> message;
	int buildingsInMsg = 0;
	for (unsigned int i = 0; i < researchCentersToChangeArea.size(); i++)
	{
		if (message == nullptr)
		{
			message = std::make_unique<cNetMessage> (GAME_EV_RESEARCH_SETTINGS);
			buildingsInMsg = 0;
		}

		message->pushChar (newAreasForResearchCenters[i]);
		message->pushInt32 (researchCentersToChangeArea[i]->iID);
		buildingsInMsg++;
		if (message->iLength + 7 > PACKAGE_LENGTH)
		{
			message->pushInt16 (buildingsInMsg);
			server.sendNetMessage (std::move (message), &receiver);
		}
	}
	if (message != nullptr)
	{
		message->pushInt16 (buildingsInMsg);
		server.sendNetMessage (std::move (message), &receiver);
	}
}

//------------------------------------------------------------------------------
void sendClans (cServer& server, const std::vector<std::unique_ptr<cPlayer>>& playerList, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_PLAYER_CLANS);
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		message->pushChar (playerList[i]->getClan());
		message->pushChar (playerList[i]->getId());
	}
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendClansToClients (cServer& server, const std::vector<std::unique_ptr<cPlayer>>& playerList)
{
	for (unsigned int n = 0; n < playerList.size(); n++)
		sendClans (server, playerList, *playerList[n]);
}

//------------------------------------------------------------------------------
void sendSetAutomoving (cServer& server, const cVehicle& vehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_SET_AUTOMOVE);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (std::move (message), vehicle.getOwner());
}

//------------------------------------------------------------------------------
void sendCommandoAnswer (cServer& server, bool success, bool steal, const cVehicle& srcUnit, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_COMMANDO_ANSWER);
	message->pushInt16 (srcUnit.iID);
	message->pushBool (steal);
	message->pushBool (success);
	server.sendNetMessage (std::move (message), &receiver);
}

//------------------------------------------------------------------------------
void sendSelfDestroy (cServer& server, const cBuilding& building)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_SELFDESTROY);
	message->pushInt16 (building.iID);
	server.sendNetMessage (std::move (message), building.getOwner());

	for (unsigned int i = 0; i < building.seenByPlayerList.size(); i++)
	{
		message = std::make_unique<cNetMessage> (GAME_EV_SELFDESTROY);
		message->pushInt16 (building.iID);
		server.sendNetMessage (std::move (message), building.seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void sendEndMoveActionToClient (cServer& server, const cVehicle& vehicle, int destID, eEndMoveActionType type)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_END_MOVE_ACTION_SERVER);
	message->pushChar (type);
	message->pushInt32 (destID);
	message->pushInt32 (vehicle.iID);

	server.sendNetMessage (std::move (message), vehicle.getOwner());
}

//------------------------------------------------------------------------------
void sendRevealMap (cServer& server, const cPlayer& receiver)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_REVEAL_MAP);

	server.sendNetMessage (std::move (message), &receiver);
}
