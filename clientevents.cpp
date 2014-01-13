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

#include "clientevents.h"

#include "buildings.h"
#include "client.h"
#include "events.h"
#include "hud.h"
#include "log.h"
#include "netmessage.h"
#include "network.h"
#include "player.h"
#include "vehicles.h"

using namespace std;

void sendClan (const cClient& client)
{
	cNetMessage* message = new cNetMessage (MU_MSG_CLAN);
	const cPlayer& player = *client.getActivePlayer();

	message->pushInt16 (player.getClan());
	message->pushInt16 (player.getNr());

	client.sendNetMessage (message);
}

void sendLandingUnits (const cClient& client, const std::vector<sLandingUnit>& landingList)
{
	cNetMessage* message = new cNetMessage (MU_MSG_LANDING_VEHICLES);

	for (size_t i = 0; i != landingList.size(); ++i)
	{
		message->pushID (landingList[i].unitID);
		message->pushInt16 (landingList[i].cargo);
	}
	message->pushInt16 ((int) landingList.size());
	message->pushInt16 (client.getActivePlayer()->getNr());

	client.sendNetMessage (message);
}

void sendUnitUpgrades (const cClient& client)
{
	const cPlayer& player = *client.getActivePlayer();
	cNetMessage* message = NULL;
	int count = 0;

	// send vehicles
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); ++i)
	{
		const sUnitData& playerData = player.VehicleData[i];
		const sUnitData& originalData = UnitsData.getVehicle (i, player.getClan());
		if (playerData.damage == originalData.damage &&
			playerData.shotsMax == originalData.shotsMax &&
			playerData.range == originalData.range &&
			playerData.ammoMax == originalData.ammoMax &&
			playerData.armor == originalData.armor &&
			playerData.hitpointsMax == originalData.hitpointsMax &&
			playerData.scan == originalData.scan &&
			playerData.speedMax == originalData.speedMax)
		{
			continue;
		}
		if (message == NULL)
		{
			message = new cNetMessage (MU_MSG_UPGRADES);
		}
		message->pushInt16 (playerData.speedMax);
		message->pushInt16 (playerData.scan);
		message->pushInt16 (playerData.hitpointsMax);
		message->pushInt16 (playerData.armor);
		message->pushInt16 (playerData.ammoMax);
		message->pushInt16 (playerData.range);
		message->pushInt16 (playerData.shotsMax);
		message->pushInt16 (playerData.damage);
		message->pushID (playerData.ID);

		count++;

		if (message->iLength + 38 > PACKAGE_LENGTH)
		{
			message->pushInt16 (count);
			message->pushInt16 (player.getNr());
			client.sendNetMessage (message);
			message = NULL;
			count = 0;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 (count);
		message->pushInt16 (player.getNr());
		client.sendNetMessage (message);
		message = NULL;
		count = 0;
	}

	// send buildings
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		const sUnitData& playerData = player.BuildingData[i];
		const sUnitData& originalData = UnitsData.getBuilding (i, player.getClan());
		if (playerData.damage == originalData.damage &&
			playerData.shotsMax == originalData.shotsMax &&
			playerData.range == originalData.range &&
			playerData.ammoMax == originalData.ammoMax &&
			playerData.armor == originalData.armor &&
			playerData.hitpointsMax == originalData.hitpointsMax &&
			playerData.scan == originalData.scan)
		{
			continue;
		}
		if (message == NULL)
		{
			message = new cNetMessage (MU_MSG_UPGRADES);
		}
		message->pushInt16 (playerData.scan);
		message->pushInt16 (playerData.hitpointsMax);
		message->pushInt16 (playerData.armor);
		message->pushInt16 (playerData.ammoMax);
		message->pushInt16 (playerData.range);
		message->pushInt16 (playerData.shotsMax);
		message->pushInt16 (playerData.damage);
		message->pushID (playerData.ID);

		count++;

		if (message->iLength + 34 > PACKAGE_LENGTH)
		{
			message->pushInt16 (count);
			message->pushInt16 (player.getNr());
			client.sendNetMessage (message);
			message = NULL;
			count = 0;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 (count);
		message->pushInt16 (player.getNr());
		client.sendNetMessage (message);
	}
}

void sendReconnectionSuccess (const cClient& client)
{
	cNetMessage* message = new cNetMessage (GAME_EV_RECON_SUCCESS);

	message->pushInt16 (client.getActivePlayer()->getNr());
	client.sendNetMessage (message);
}

void sendTakenUpgrades (const cClient& client, const std::vector<cUnitUpgrade>& unitUpgrades)
{
	const cPlayer* player = client.getActivePlayer();
	cNetMessage* msg = NULL;
	int iCount = 0;

	for (unsigned int i = 0; i < UnitsData.getNrVehicles() + UnitsData.getNrBuildings(); ++i)
	{
		const cUnitUpgrade& curUpgrade = unitUpgrades[i];

		if (!curUpgrade.hasBeenPurchased()) continue;

		if (msg == NULL)
		{
			msg = new cNetMessage (GAME_EV_WANT_BUY_UPGRADES);
			iCount = 0;
		}

		const sUnitData* currentVersion;
		if (i < UnitsData.getNrVehicles()) currentVersion = &player->VehicleData[i];
		else currentVersion = &player->BuildingData[i - UnitsData.getNrVehicles()];

		if (i < UnitsData.getNrVehicles()) msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_SPEED, currentVersion->speedMax));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_SCAN, currentVersion->scan));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_HITS, currentVersion->hitpointsMax));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_ARMOR, currentVersion->armor));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_AMMO, currentVersion->ammoMax));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_RANGE, currentVersion->range));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_SHOTS, currentVersion->shotsMax));
		msg->pushInt16 (curUpgrade.getValueOrDefault (sUnitUpgrade::UPGRADE_TYPE_DAMAGE, currentVersion->damage));
		msg->pushID (currentVersion->ID);

		iCount++; // msg contains one more upgrade struct

		// the msg would be too long,
		// if another upgrade would be written into it.
		// So send it and put the next upgrades in a new message.
		if (msg->iLength + 38 > PACKAGE_LENGTH)
		{
			msg->pushInt16 (iCount);
			msg->pushInt16 (player->getNr());
			client.sendNetMessage (msg);
			msg = NULL;
		}
	}

	if (msg != NULL)
	{
		msg->pushInt16 (iCount);
		msg->pushInt16 (player->getNr());
		client.sendNetMessage (msg);
	}
}

void sendLandingCoords (const cClient& client, const sClientLandData& c)
{
	Log.write ("Client: sending landing coords", cLog::eLOG_TYPE_NET_DEBUG);
	cNetMessage* message = new cNetMessage (MU_MSG_LANDING_COORDS);
	message->pushInt16 (c.iLandY);
	message->pushInt16 (c.iLandX);
	message->pushChar (client.getActivePlayer()->getNr());

	client.sendNetMessage (message);
}

void sendChatMessageToServer (const cClient& client, const string& sMsg)
{
	cNetMessage* message = new cNetMessage (GAME_EV_CHAT_CLIENT);
	message->pushString (sMsg);
	client.sendNetMessage (message);
}

void sendWantToEndTurn (const cClient& client)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_TO_END_TURN);
	client.sendNetMessage (message);
}

void sendWantStartWork (const cClient& client, const cUnit& building)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_START_WORK);
	message->pushInt32 (building.iID);
	client.sendNetMessage (message);
}

void sendWantStopWork (const cClient& client, const cUnit& building)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_STOP_WORK);
	message->pushInt32 (building.iID);
	client.sendNetMessage (message);
}

void sendMoveJob (const cClient& client, sWaypoint* path, int vehicleID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_MOVE_JOB_CLIENT);

	const sWaypoint* waypoint = path;
	int iCount = 0;
	while (waypoint)
	{
		message->pushInt16 (waypoint->Costs);
		message->pushInt16 (waypoint->X);
		message->pushInt16 (waypoint->Y);

		if (message->iLength > PACKAGE_LENGTH - 19)
		{
			Log.write (" Client: Error sending movejob: message too long", cLog::eLOG_TYPE_NET_ERROR);
			delete message;
			return; // don't send movejobs that are to long
		}

		waypoint = waypoint->next;
		iCount++;
	}

	message->pushInt16 (iCount);
	message->pushInt32 (vehicleID);

	client.sendNetMessage (message);

	while (path)
	{
		waypoint = path;
		path = path->next;
		delete waypoint;
	}
}

void sendWantStopMove (const cClient& client, int iVehicleID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_STOP_MOVE);
	message->pushInt16 (iVehicleID);
	client.sendNetMessage (message);
}

void sendMoveJobResume (const cClient& client, int unitId)
{
	cNetMessage* message = new cNetMessage (GAME_EV_MOVEJOB_RESUME);
	message->pushInt32 (unitId);
	client.sendNetMessage (message);
}

void sendWantAttack (const cClient& client, int targetID, int targetOffset, int aggressor, bool isVehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_ATTACK);
	message->pushInt32 (targetID);
	message->pushInt32 (targetOffset);
	message->pushInt32 (aggressor);
	message->pushBool (isVehicle);
	client.sendNetMessage (message);
}

void sendMineLayerStatus (const cClient& client, const cVehicle& vehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_MINELAYERSTATUS);
	message->pushBool (vehicle.LayMines);
	message->pushBool (vehicle.ClearMines);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (message);
}

void sendWantBuild (const cClient& client, int iVehicleID, sID buildingTypeID, int iBuildSpeed, int iBuildOff, bool bBuildPath, int iPathOff)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_BUILD);
	message->pushInt32 (iPathOff);
	message->pushBool (bBuildPath);
	message->pushInt32 (iBuildOff);
	message->pushInt16 (iBuildSpeed);
	message->pushID (buildingTypeID);
	message->pushInt16 (iVehicleID);
	client.sendNetMessage (message);
}

void sendWantEndBuilding (const cClient& client, const cVehicle& vehicle, int escapeX, int escapeY)
{
	cNetMessage* message = new cNetMessage (GAME_EV_END_BUILDING);
	message->pushInt16 (escapeY);
	message->pushInt16 (escapeX);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (message);
}

void sendWantStopBuilding (const cClient& client, int iVehicleID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_STOP_BUILDING);
	message->pushInt16 (iVehicleID);
	client.sendNetMessage (message);
}

void sendWantTransfer (const cClient& client, bool bSrcVehicle, int iSrcID, bool bDestVehicle, int iDestID, int iTransferValue, int iType)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_TRANSFER);
	message->pushInt16 (iType);
	message->pushInt16 (iTransferValue);
	message->pushInt16 (iDestID);
	message->pushBool (bDestVehicle);
	message->pushInt16 (iSrcID);
	message->pushBool (bSrcVehicle);
	client.sendNetMessage (message);
}

void sendWantBuildList (const cClient& client, const cBuilding& building, const std::vector<sBuildList>& buildList, bool bRepeat, int buildSpeed)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_BUILDLIST);
	message->pushBool (bRepeat);
	for (int i = (int) buildList.size() - 1; i >= 0; i--)
	{
		message->pushID (buildList[i].type);
	}
	message->pushInt16 ((int) buildList.size());
	message->pushInt16 (buildSpeed);
	message->pushInt16 (building.iID);
	client.sendNetMessage (message);
}

void sendWantExitFinishedVehicle (const cClient& client, const cBuilding& building, int iX, int iY)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_EXIT_FIN_VEH);
	message->pushInt16 (iY);
	message->pushInt16 (iX);
	message->pushInt16 (building.iID);
	client.sendNetMessage (message);
}

void sendChangeResources (const cClient& client, const cBuilding& building, int iMetalProd, int iOilProd, int iGoldProd)
{
	cNetMessage* message = new cNetMessage (GAME_EV_CHANGE_RESOURCES);
	message->pushInt16 (iGoldProd);
	message->pushInt16 (iOilProd);
	message->pushInt16 (iMetalProd);
	message->pushInt16 (building.iID);
	client.sendNetMessage (message);
}

void sendChangeManualFireStatus (const cClient& client, int iUnitID, bool bVehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_CHANGE_MANUAL_FIRE);
	message->pushInt16 (iUnitID);
	message->pushBool (bVehicle);
	client.sendNetMessage (message);
}

void sendChangeSentry (const cClient& client, int iUnitID, bool bVehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_CHANGE_SENTRY);
	message->pushInt16 (iUnitID);
	message->pushBool (bVehicle);
	client.sendNetMessage (message);
}

void sendWantSupply (const cClient& client, int iDestID, bool bDestVehicle, int iSrcID, bool bSrcVehicle, int iType)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_SUPPLY);
	message->pushInt16 (iDestID);
	message->pushBool (bDestVehicle);
	message->pushInt16 (iSrcID);
	message->pushBool (bSrcVehicle);
	message->pushChar (iType);
	client.sendNetMessage (message);
}

void sendWantStartClear (const cClient& client, const cVehicle& vehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_START_CLEAR);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (message);
}

void sendWantStopClear (const cClient& client, const cVehicle& vehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_STOP_CLEAR);
	message->pushInt16 (vehicle.iID);
	client.sendNetMessage (message);
}

void sendAbortWaiting (const cClient& client)
{
	cNetMessage* message = new cNetMessage (GAME_EV_ABORT_WAITING);
	client.sendNetMessage (message);
}

void sendWantLoad (const cClient& client, int unitid, bool vehicle, int loadedunitid)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_LOAD);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (loadedunitid);
	client.sendNetMessage (message);
}

void sendWantActivate (const cClient& client, int unitid, bool vehicle, int activatunitid, int x, int y)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_EXIT);
	message->pushInt16 (y);
	message->pushInt16 (x);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (activatunitid);
	client.sendNetMessage (message);
}

void sendRequestResync (const cClient& client, char playerNr)
{
	cNetMessage* message = new cNetMessage (GAME_EV_REQUEST_RESYNC);
	message->pushChar (playerNr);
	client.sendNetMessage (message);
}

void sendSetAutoStatus (const cClient& client, int vehicleID, bool set)
{
	cNetMessage* message = new cNetMessage (GAME_EV_AUTOMOVE_STATUS);
	message->pushBool (set);
	message->pushInt16 (vehicleID);
	client.sendNetMessage (message);
}

void sendWantComAction (const cClient& client, int srcUnitID, int destUnitID, bool destIsVehicle, bool steal)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_COM_ACTION);
	message->pushBool (steal);
	message->pushInt16 (destUnitID);
	message->pushBool (destIsVehicle);
	message->pushInt16 (srcUnitID);
	client.sendNetMessage (message);
}

//------------------------------------------------------------------------
void sendUpgradeBuilding (const cClient& client, const cBuilding& building, bool upgradeAll)
{
	if (building.owner == 0)
		return;

	const sUnitData& currentVersion = building.data;
	const sUnitData& upgradedVersion = *building.owner->getUnitDataCurrentVersion (building.data.ID);
	if (currentVersion.version >= upgradedVersion.version)
		return; // already uptodate

	cNetMessage* msg = new cNetMessage (GAME_EV_WANT_BUILDING_UPGRADE);
	msg->pushBool (upgradeAll);
	msg->pushInt32 (building.iID);

	client.sendNetMessage (msg);
}

void sendWantUpgrade (const cClient& client, int buildingID, int storageSlot, bool upgradeAll)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_VEHICLE_UPGRADE);
	message->pushInt32 (buildingID);
	if (!upgradeAll) message->pushInt16 (storageSlot);
	message->pushBool (upgradeAll);
	client.sendNetMessage (message);
}

void sendWantResearchChange (const cClient& client, int (&newResearchSettings) [cResearch::kNrResearchAreas], int ownerNr)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_RESEARCH_CHANGE);
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		message->pushInt16 (newResearchSettings[i]);
	}
	message->pushInt16 (ownerNr);
	client.sendNetMessage (message);
}

void sendSaveHudInfo (const cClient& client, int selectedUnitID, int ownerNr, int savingID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SAVE_HUD_INFO);
	const cGameGUI& gameGUI = client.getGameGUI();
	message->pushBool (gameGUI.tntChecked());
	message->pushBool (gameGUI.hitsChecked());
	message->pushBool (gameGUI.lockChecked());
	message->pushBool (gameGUI.surveyChecked());
	message->pushBool (gameGUI.statusChecked());
	message->pushBool (gameGUI.scanChecked());
	message->pushBool (gameGUI.rangeChecked());
	message->pushBool (gameGUI.twoXChecked());
	message->pushBool (gameGUI.fogChecked());
	message->pushBool (gameGUI.ammoChecked());
	message->pushBool (gameGUI.gridChecked());
	message->pushBool (gameGUI.colorChecked());
	message->pushFloat (gameGUI.getZoom());
	message->pushInt16 (gameGUI.getOffsetY());
	message->pushInt16 (gameGUI.getOffsetX());
	message->pushInt16 (selectedUnitID);
	message->pushInt16 (ownerNr);
	message->pushInt16 (savingID);
	client.sendNetMessage (message);
}

void sendSaveReportInfo (const cClient& client, const sSavedReportMessage& savedReport, int ownerNr, int savingID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SAVE_REPORT_INFO);
	savedReport.pushInto (*message);
	message->pushInt16 (ownerNr);
	message->pushInt16 (savingID);
	client.sendNetMessage (message);
}

void sendFinishedSendSaveInfo (const cClient& client, int ownerNr, int savingID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_FIN_SEND_SAVE_INFO);
	message->pushInt16 (ownerNr);
	message->pushInt16 (savingID);
	client.sendNetMessage (message);
}

void sendRequestCasualtiesReport (const cClient& client)
{
	cNetMessage* message = new cNetMessage (GAME_EV_REQUEST_CASUALTIES_REPORT);
	client.sendNetMessage (message);
}

void sendWantSelfDestroy (const cClient& client, const cBuilding& building)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_SELFDESTROY);
	message->pushInt16 (building.iID);
	client.sendNetMessage (message);
}

void sendWantChangeUnitName (const cClient& client, const string& newName, int unitID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WANT_CHANGE_UNIT_NAME);
	message->pushString (newName);
	message->pushInt16 (unitID);
	client.sendNetMessage (message);
}

void sendEndMoveAction (const cClient& client, int vehicleID, int destID, eEndMoveActionType type)
{
	cNetMessage* message = new cNetMessage (GAME_EV_END_MOVE_ACTION);
	message->pushChar (type);
	message->pushInt32 (destID);
	message->pushInt32 (vehicleID);
	client.sendNetMessage (message);
}
