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

#include "game/logic/clientevents.h"

#include "game/data/units/building.h"
#include "game/data/units/landingunit.h"
#include "game/logic/client.h"
#include "utility/log.h"
#include "netmessage.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "utility/position.h"
#include "game/data/report/savedreport.h"
#include "ui/graphical/game/gameguistate.h"
#include "../data/report/savedreportchat.h"

using namespace std;

void sendClan (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (MU_MSG_CLAN);
	const cPlayer& player = client.getActivePlayer();

	message->pushInt16 (player.getClan());
	message->pushInt16 (player.getId());

	client.sendNetMessage (std::move (message));
}

void sendLandingUnits (const cClient& client, const std::vector<sLandingUnit>& landingList)
{
	auto message = std::make_unique<cNetMessage> (MU_MSG_LANDING_VEHICLES);

	for (size_t i = 0; i != landingList.size(); ++i)
	{
		message->pushID (landingList[i].unitID);
		message->pushInt16 (landingList[i].cargo);
	}
	message->pushInt16 ((int) landingList.size());
	message->pushInt16 (client.getActivePlayer().getId());

	client.sendNetMessage (std::move (message));
}

void sendUnitUpgrades (const cClient& client)
{
	const cPlayer& player = client.getActivePlayer();
	std::unique_ptr<cNetMessage> message = nullptr;
	int count = 0;

	// send vehicles
	/*for (unsigned int i = 0; i < UnitsData.getNrVehicles(); ++i)
	{
		const sUnitData& playerData = player.VehicleData[i];
		const sUnitData& originalData = UnitsData.getVehicle (i, player.getClan());
		if (playerData.getDamage() == originalData.getDamage() &&
			playerData.getShotsMax() == originalData.getShotsMax() &&
			playerData.getRange() == originalData.getRange() &&
			playerData.getAmmoMax() == originalData.getAmmoMax() &&
			playerData.getArmor() == originalData.getArmor() &&
			playerData.getHitpointsMax() == originalData.getHitpointsMax() &&
			playerData.getScan() == originalData.getScan() &&
			playerData.getSpeedMax() == originalData.getSpeedMax())
		{
			continue;
		}
		if (message == nullptr)
		{
			message = std::make_unique<cNetMessage> (MU_MSG_UPGRADES);
		}
		message->pushInt16 (playerData.getSpeedMax());
		message->pushInt16 (playerData.getScan());
		message->pushInt16 (playerData.getHitpointsMax());
		message->pushInt16 (playerData.getArmor());
		message->pushInt16 (playerData.getAmmoMax());
		message->pushInt16 (playerData.getRange());
		message->pushInt16 (playerData.getShotsMax());
		message->pushInt16 (playerData.getDamage());
		message->pushID (playerData.ID);


		count++;

		if (message->iLength + 38 > PACKAGE_LENGTH)
		{
			message->pushInt16 (count);
			message->pushInt16 (player.getId());
			client.sendNetMessage (std::move (message));
			count = 0;
		}
	}
	if (message != nullptr)
	{
		message->pushInt16 (count);
		message->pushInt16 (player.getId());
		client.sendNetMessage (std::move (message));
		count = 0;
	}

	// send buildings
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		const sUnitData& playerData = player.BuildingData[i];
		const sUnitData& originalData = UnitsData.getBuilding (i, player.getClan());
		if (playerData.getDamage() == originalData.getDamage() &&
			playerData.getShotsMax() == originalData.getShotsMax() &&
			playerData.getRange() == originalData.getRange() &&
			playerData.getAmmoMax() == originalData.getAmmoMax() &&
			playerData.getArmor() == originalData.getArmor() &&
			playerData.getHitpointsMax() == originalData.getHitpointsMax() &&
			playerData.getScan() == originalData.getScan())
		{
			continue;
		}
		if (message == nullptr)
		{
			message = std::make_unique<cNetMessage> (MU_MSG_UPGRADES);
		}
		message->pushInt16 (playerData.getScan());
		message->pushInt16 (playerData.getHitpointsMax());
		message->pushInt16 (playerData.getArmor());
		message->pushInt16 (playerData.getAmmoMax());
		message->pushInt16 (playerData.getRange());
		message->pushInt16 (playerData.getShotsMax());
		message->pushInt16 (playerData.getDamage());
		message->pushID (playerData.ID);

		count++;

		if (message->iLength + 34 > PACKAGE_LENGTH)
		{
			message->pushInt16 (count);
			message->pushInt16 (player.getId());
			client.sendNetMessage (std::move (message));
			count = 0;
		}
	}
	if (message != nullptr)
	{
		message->pushInt16 (count);
		message->pushInt16 (player.getId());
		client.sendNetMessage (std::move (message));
	}*/
}

void sendReconnectionSuccess (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_RECON_SUCCESS);

	message->pushInt16 (client.getActivePlayer().getId());
	client.sendNetMessage (std::move (message));
}

void sendTakenUpgrades (const cClient& client, const std::vector<std::pair<sID, cUnitUpgrade>>& unitUpgrades)
{
	const cPlayer& player = client.getActivePlayer();
	std::unique_ptr<cNetMessage> msg = nullptr;
	int iCount = 0;

	for (size_t i = 0; i < unitUpgrades.size(); ++i)
	{
		const auto& unitId = unitUpgrades[i].first;
		const auto& curUpgrade = unitUpgrades[i].second;

		if (!curUpgrade.hasBeenPurchased()) continue;

		if (msg == nullptr)
		{
			msg = std::make_unique<cNetMessage> (GAME_EV_WANT_BUY_UPGRADES);
			iCount = 0;
		}

		const auto currentVersion = player.getUnitDataCurrentVersion (unitId);

		if (unitId.isAVehicle()) msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_SPEED, currentVersion->getSpeedMax()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_SCAN, currentVersion->getScan()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_HITS, currentVersion->getHitpointsMax()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_ARMOR, currentVersion->getArmor()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_AMMO, currentVersion->getAmmoMax()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_RANGE, currentVersion->getRange()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_SHOTS, currentVersion->getShotsMax()));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_DAMAGE, currentVersion->getDamage()));
		msg->pushID (currentVersion->getId());

		iCount++; // msg contains one more upgrade struct

		// the msg would be too long,
		// if another upgrade would be written into it.
		// So send it and put the next upgrades in a new message.
		if (msg->iLength + 38 > PACKAGE_LENGTH)
		{
			msg->pushInt16 (iCount);
			msg->pushInt16 (player.getId());
			client.sendNetMessage (std::move (msg));
		}
	}

	if (msg != nullptr)
	{
		msg->pushInt16 (iCount);
		msg->pushInt16 (player.getId());
		client.sendNetMessage (std::move (msg));
	}
}

void sendLandingCoords (const cClient& client, const cPosition& coords)
{
	Log.write ("Client: sending landing coords", cLog::eLOG_TYPE_NET_DEBUG);
	auto message = std::make_unique<cNetMessage> (MU_MSG_LANDING_COORDS);
	message->pushPosition (coords);
	message->pushChar (client.getActivePlayer().getId());

	client.sendNetMessage (std::move (message));
}

void sendReadyToStart (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (MU_MSG_READY_TO_START);
	message->pushChar (client.getActivePlayer().getId());

	client.sendNetMessage (std::move (message));
}

void sendChatMessageToServer (const cClient& client, const string& msg, const cPlayer& player)
{
	cNetMessageReport netMsg(std::make_unique<cSavedReportChat>(player, msg));
	client.sendNetMessage (netMsg);
}

void sendWantToEndTurn (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_TO_END_TURN);
	client.sendNetMessage (std::move (message));
}

void sendMoveJob (const cClient& client, sWaypoint* path, int vehicleID)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_MOVE_JOB_CLIENT);

	const sWaypoint* waypoint = path;
	int iCount = 0;
	while (waypoint)
	{

		if (message->iLength + 10 > PACKAGE_LENGTH - 19)
		{
			Log.write (" Client: Error sending movejob: message too long, sending partial path", cLog::eLOG_TYPE_NET_ERROR);
			break; // send part of movejob
 		}

		message->pushInt16 (waypoint->Costs);
		message->pushPosition (waypoint->position);

		waypoint = waypoint->next;
		iCount++;
	}

	message->pushInt16 (iCount);
	message->pushInt32 (vehicleID);

	client.sendNetMessage (std::move (message));

	while (path)
	{
		waypoint = path;
		path = path->next;
		delete waypoint;
	}
}

void sendWantStopMove (const cClient& client, int iVehicleID)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_STOP_MOVE);
	message->pushInt16 (iVehicleID);
	client.sendNetMessage (std::move (message));
}

void sendMoveJobResume (const cClient& client, int unitId)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_MOVEJOB_RESUME);
	message->pushInt32 (unitId);
	client.sendNetMessage (std::move (message));
}

void sendWantAttack (const cClient& client, int aggressorID, const cPosition& targetPosition, int targetID)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_ATTACK);
	message->pushInt32 (aggressorID);
	message->pushPosition (targetPosition);
	message->pushInt32 (targetID);
	client.sendNetMessage (std::move (message));
}

void sendMineLayerStatus (const cClient& client, const cVehicle& vehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_MINELAYERSTATUS);
	message->pushBool (vehicle.isUnitLayingMines());
	message->pushBool (vehicle.isUnitClearingMines());
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (std::move (message));
}

void sendWantBuild (const cClient& client, int iVehicleID, sID buildingTypeID, int iBuildSpeed, const cPosition& buildPosition, bool bBuildPath, const cPosition& pathEndPosition)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_BUILD);
	message->pushPosition (pathEndPosition);
	message->pushBool (bBuildPath);
	message->pushPosition (buildPosition);
	message->pushInt16 (iBuildSpeed);
	message->pushID (buildingTypeID);
	message->pushInt16 (iVehicleID);
	client.sendNetMessage (std::move (message));
}

void sendWantEndBuilding (const cClient& client, const cVehicle& vehicle, const cPosition& escapePosition)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_END_BUILDING);
	message->pushPosition (escapePosition);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (std::move (message));
}

void sendWantStopBuilding (const cClient& client, int iVehicleID)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_STOP_BUILDING);
	message->pushInt16 (iVehicleID);
	client.sendNetMessage (std::move (message));
}

void sendWantBuildList (const cClient& client, const cBuilding& building, const std::vector<cBuildListItem>& buildList, bool bRepeat, int buildSpeed)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_BUILDLIST);
	message->pushBool (bRepeat);
	for (int i = (int) buildList.size() - 1; i >= 0; i--)
	{
		message->pushID (buildList[i].getType());
	}
	message->pushInt16 ((int) buildList.size());
	message->pushInt16 (buildSpeed);
	message->pushInt16 (building.iID);
	client.sendNetMessage (std::move (message));
}

void sendWantExitFinishedVehicle (const cClient& client, const cBuilding& building, const cPosition& position)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_EXIT_FIN_VEH);
	message->pushPosition (position);
	message->pushInt16 (building.iID);
	client.sendNetMessage (std::move (message));
}

void sendChangeResources (const cClient& client, const cBuilding& building, int iMetalProd, int iOilProd, int iGoldProd)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_CHANGE_RESOURCES);
	message->pushInt16 (iGoldProd);
	message->pushInt16 (iOilProd);
	message->pushInt16 (iMetalProd);
	message->pushInt16 (building.iID);
	client.sendNetMessage (std::move (message));
}

void sendChangeManualFireStatus (const cClient& client, int iUnitID, bool bVehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_CHANGE_MANUAL_FIRE);
	message->pushInt16 (iUnitID);
	message->pushBool (bVehicle);
	client.sendNetMessage (std::move (message));
}

void sendChangeSentry (const cClient& client, int iUnitID, bool bVehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_CHANGE_SENTRY);
	message->pushInt16 (iUnitID);
	message->pushBool (bVehicle);
	client.sendNetMessage (std::move (message));
}

void sendWantSupply (const cClient& client, int iDestID, bool bDestVehicle, int iSrcID, bool bSrcVehicle, int iType)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_SUPPLY);
	message->pushInt16 (iDestID);
	message->pushBool (bDestVehicle);
	message->pushInt16 (iSrcID);
	message->pushBool (bSrcVehicle);
	message->pushChar (iType);
	client.sendNetMessage (std::move (message));
}

void sendWantStartClear (const cClient& client, const cVehicle& vehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_START_CLEAR);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (std::move (message));
}

void sendWantStopClear (const cClient& client, const cVehicle& vehicle)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_STOP_CLEAR);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (std::move (message));
}

void sendAbortWaiting (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_ABORT_WAITING);
	client.sendNetMessage (std::move (message));
}

void sendWantLoad (const cClient& client, int unitid, bool vehicle, int loadedunitid)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_LOAD);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (loadedunitid);
	client.sendNetMessage (std::move (message));
}

void sendWantActivate (const cClient& client, int unitid, bool vehicle, int activatunitid, const cPosition& position)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_EXIT);
	message->pushPosition (position);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (activatunitid);
	client.sendNetMessage (std::move (message));
}

void sendRequestResync (const cClient& client, char playerNumber, bool newGame)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_REQUEST_RESYNC);
	message->pushBool (newGame);
	message->pushInt32 (playerNumber);
	client.sendNetMessage (std::move (message));
}

void sendSetAutoStatus (const cClient& client, int vehicleID, bool set)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_AUTOMOVE_STATUS);
	message->pushBool (set);
	message->pushInt16 (vehicleID);
	client.sendNetMessage (std::move (message));
}

void sendWantComAction (const cClient& client, int srcUnitID, int destUnitID, bool destIsVehicle, bool steal)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_COM_ACTION);
	message->pushBool (steal);
	message->pushInt16 (destUnitID);
	message->pushBool (destIsVehicle);
	message->pushInt16 (srcUnitID);
	client.sendNetMessage (std::move (message));
}

//------------------------------------------------------------------------
void sendUpgradeBuilding (const cClient& client, const cBuilding& building, bool upgradeAll)
{
	if (building.getOwner() == 0)
		return;

	const cDynamicUnitData& currentVersion = building.data;
	const cDynamicUnitData& upgradedVersion = *building.getOwner()->getUnitDataCurrentVersion (building.data.getId());
	if (currentVersion.getVersion() >= upgradedVersion.getVersion())
		return; // already uptodate

	auto msg = std::make_unique<cNetMessage> (GAME_EV_WANT_BUILDING_UPGRADE);
	msg->pushBool (upgradeAll);
	msg->pushInt32 (building.iID);

	client.sendNetMessage (std::move (msg));
}

void sendWantUpgrade (const cClient& client, int buildingID, int storageSlot, bool upgradeAll)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_VEHICLE_UPGRADE);
	message->pushInt32 (buildingID);
	if (!upgradeAll) message->pushInt16 (storageSlot);
	message->pushBool (upgradeAll);
	client.sendNetMessage (std::move (message));
}

void sendWantResearchChange (const cClient& client, const std::array<int, cResearch::kNrResearchAreas>& newResearchSettings)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_RESEARCH_CHANGE);
	const cPlayer& player = client.getActivePlayer();
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		message->pushInt16 (newResearchSettings[i]);
	}
	message->pushInt16 (player.getId());
	client.sendNetMessage (std::move (message));
}

void sendRequestCasualtiesReport (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_REQUEST_CASUALTIES_REPORT);
	client.sendNetMessage (std::move (message));
}

void sendWantSelfDestroy (const cClient& client, int unitId)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_SELFDESTROY);
	message->pushInt16 (unitId);
	client.sendNetMessage (std::move (message));
}

void sendWantChangeUnitName (const cClient& client, const string& newName, int unitID)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_CHANGE_UNIT_NAME);
	message->pushString (newName);
	message->pushInt16 (unitID);
	client.sendNetMessage (std::move (message));
}

void sendEndMoveAction (const cClient& client, int vehicleID, int destID, eEndMoveActionType type)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_END_MOVE_ACTION);
	message->pushChar (type);
	message->pushInt32 (destID);
	message->pushInt32 (vehicleID);
	client.sendNetMessage (std::move (message));
}

void sentWantKickPlayer (const cClient& client, const cPlayer& player)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_KICK_PLAYER);
	message->pushInt32 (player.getId());
	client.sendNetMessage (std::move (message));
}
