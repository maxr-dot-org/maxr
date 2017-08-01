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

#include "game/logic/attackjob.h"
#include "game/logic/automjobs.h"
#include "game/data/units/building.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "game/logic/fxeffects.h"
#include "game/logic/gametimer.h"
#include "game/logic/jobs.h"
#include "game/logic/automjobs.h"
#include "utility/log.h"
#include "main.h"
#include "netmessage.h"
#include "netmessage2.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/logic/server2.h"
#include "game/logic/serverevents.h"
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

using namespace std;

//------------------------------------------------------------------------
// cClient implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cClient::cClient (std::shared_ptr<cConnectionManager> connectionManager) :
	server (nullptr),
	connectionManager(connectionManager),
	gameTimer (std::make_shared<cGameTimerClient> ()),
	activePlayer (nullptr),
	casualtiesTracker (std::make_shared<cCasualtiesTracker> ()),
	effectsList (new cFxContainer)
{
	gameTimer->start();
}

cClient::~cClient()
{
	connectionManager->setLocalClient(nullptr, -1);
	connectionManager->disconnectAll();
	gameTimer->stop();

	for (unsigned int i = 0; i < attackJobs.size(); i++)
	{
		delete attackJobs[i];
	}
	neutralBuildings.clear();
	helperJobs.clear();
}

void cClient::setMap (std::shared_ptr<cStaticMap> staticMap)
{
	model.setMap(staticMap);
}

void cClient::setGameSettings (const cGameSettings& gameSettings)
{
	model.setGameSettings(gameSettings);
}

class LessByNr
{
public:
	bool operator() (const cPlayer* lhs, const cPlayer* rhs) const
	{
		return lhs->getId() < rhs->getId();
	}
};

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

void cClient::sendNetMessage (std::unique_ptr<cNetMessage> message) const
{

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

void cClient::runFx()
{
	effectsList->run();
}

void cClient::addFx (std::shared_ptr<cFx> fx, bool playSound)
{
	effectsList->push_back (fx);
	addedEffect (fx, playSound);
}

void cClient::HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_BUILDING);

	cBuilding* Building = getBuildingFromID (message.popInt16());

	if (Building)
	{
		//deleteUnit (Building);
	}
}

void cClient::HandleNetMessage_GAME_EV_DEL_VEHICLE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_VEHICLE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());

	//if (Vehicle) deleteUnit (Vehicle);
}

void cClient::HandleNetMessage_GAME_EV_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_DATA);

/*	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	(void) Player;  // TODO use me
	const int iID = message.popInt16();
	const bool bVehicle = message.popBool();
	const auto position = message.popPosition();

	Log.write (" Client: Received Unit Data: Vehicle: " + iToStr ((int) bVehicle) + ", ID: " + iToStr (iID) + ", XPos: " + iToStr (position.x()) + ", YPos: " + iToStr (position.y()), cLog::eLOG_TYPE_NET_DEBUG);
	cVehicle* Vehicle = nullptr;
	sUnitData* Data = nullptr;
	// unit is a vehicle
	if (bVehicle)
	{
		bool bBig = message.popBool();
		Vehicle = getVehicleFromID (iID);

		if (!Vehicle)
		{
			Log.write (" Client: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of vehicle
			return;
		}
		if (Vehicle->getPosition() != position || Vehicle->data.isBig != bBig)
		{
			// should never happen
			cLog::eLogType iLogType = cLog::eLOG_TYPE_NET_WARNING;
			if (Vehicle->isUnitBuildingABuilding() || Vehicle->isUnitClearing() || Vehicle->isUnitMoving()) iLogType = cLog::eLOG_TYPE_NET_DEBUG;
			Log.write (" Client: Vehicle identificated by ID (" + iToStr (iID) + ") but has wrong position [IS: X" + iToStr (Vehicle->getPosition().x()) + " Y" + iToStr (Vehicle->getPosition().y()) + "; SHOULD: X" + iToStr (position.x()) + " Y" + iToStr (position.y()) + "]", iLogType);

			// set to server position if vehicle is not moving
			if (!Vehicle->MoveJobActive)
			{
//				getMap()->moveVehicle (*Vehicle, position);
//				if (bBig) getMap()->moveVehicleBig (*Vehicle, position);
				Vehicle->getOwner()->doScan();
			}
		}

		if (message.popBool()) Vehicle->changeName (message.popString());

		Vehicle->setAttacking (message.popBool());
		Vehicle->setIsBeeinAttacked (message.popBool());

		const bool bWasDisabled = Vehicle->isDisabled();
		Vehicle->setDisabledTurns (message.popInt16());
		Vehicle->setCommandoRank (message.popInt16());
		Vehicle->setClearing (message.popBool());
		Vehicle->setBuildingABuilding (message.popBool());
		Vehicle->setBuildTurns (message.popInt16());
		Vehicle->setClearingTurns (message.popInt16());
		Vehicle->setSentryActive (message.popBool());
		Vehicle->setManualFireActive (message.popBool());

		if (Vehicle->isDisabled() != bWasDisabled && Vehicle->getOwner() == activePlayer)
		{
			if (Vehicle->isDisabled())
			{
				activePlayer->addSavedReport (std::make_unique<cSavedReportDisabled> (*Vehicle));
				unitDisabled (*Vehicle);
			}
			Vehicle->getOwner()->doScan();
		}
		Data = &Vehicle->data;
	}
	else
	{
		cBuilding* Building = getBuildingFromID (iID);
		if (!Building)
		{
			Log.write (" Client: Unknown building with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of building
			return;
		}

		if (message.popBool()) Building->changeName (message.popString());

		Building->setAttacking (message.popBool());
		Building->setIsBeeinAttacked (message.popBool());

		const bool bWasDisabled = Building->isDisabled();
		Building->setDisabledTurns (message.popInt16());
		Building->setResearchArea ((cResearch::ResearchArea)message.popInt16());
		Building->setWorking (message.popBool());
		Building->setSentryActive (message.popBool());
		Building->setManualFireActive (message.popBool());
		Building->points = message.popInt16();

		if (Building->isDisabled() != bWasDisabled && Building->getOwner() == activePlayer)
		{
			if (Building->isDisabled())
			{
				activePlayer->addSavedReport (std::make_unique<cSavedReportDisabled> (*Building));
				unitDisabled (*Building);
			}
			Building->getOwner()->doScan();
		}
		Data = &Building->data;
	}

	Data->buildCosts = message.popInt16();
	Data->setAmmo (message.popInt16());
	Data->setAmmoMax (message.popInt16());
	Data->setStoredResources (message.popInt16());
	Data->storageResMax = message.popInt16();
	Data->setStoredUnits (message.popInt16());
	Data->storageUnitsMax = message.popInt16();
	Data->setDamage (message.popInt16());
	Data->setShots (message.popInt16());
	Data->setShotsMax (message.popInt16());
	Data->setRange (message.popInt16());
	Data->setScan (message.popInt16());
	Data->setArmor (message.popInt16());
	Data->setHitpoints (message.popInt16());
	Data->setHitpointsMax (message.popInt16());
	Data->setVersion (message.popInt16());

	if (bVehicle)
	{
		if (Data->canPlaceMines)
		{
			if (Data->getStoredResources() <= 0) Vehicle->setLayMines (false);
			if (Data->getStoredResources() >= Data->storageResMax) Vehicle->setClearMines (false);
		}
		Data->setSpeed (message.popInt16());
		Data->setSpeedMax (message.popInt16());

		Vehicle->setFlightHeight (message.popInt16());
	}*/
}

void cClient::HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SPECIFIC_UNIT_DATA);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (!Vehicle) return;
	Vehicle->dir = message.popInt16();
	Vehicle->setBuildingType (message.popID());
	Vehicle->BuildPath = message.popBool();
	Vehicle->bandPosition = message.popPosition();
}

void cClient::HandleNetMessage_GAME_EV_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESOURCES);

	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		const auto position = message.popPosition();

		activePlayer->exploreResource (position);

//		sResources& res = getMap()->getResource (position);
//		res.typ = (unsigned char) message.popInt16();
//		res.value = (unsigned char) message.popInt16();
	}
}

void cClient::HandleNetMessage_GAME_EV_BUILD_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_BUILD_ANSWER);

/*	const bool bOK = message.popBool();
	const int iID = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle == nullptr)
	{
		Log.write (" Client: Vehicle can't start building: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of vehicle
		return;
	}

	if (!bOK)
	{
		if (Vehicle->getOwner() == activePlayer)
		{
			if (!Vehicle->BuildPath)
			{
				activePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (eSavedReportType::ProducingError));
			}
			else if (Vehicle->bandPosition != Vehicle->getPosition())
			{
				activePlayer->addSavedReport (std::make_unique<cSavedReportPathInterrupted> (*Vehicle));
			}
		}
		Vehicle->setBuildTurns (0);
		Vehicle->BuildPath = false;
		Vehicle->bandPosition = cPosition (0, 0);
		return;
	}

	if (Vehicle->isUnitBuildingABuilding()) Log.write (" Client: Vehicle is already building", cLog::eLOG_TYPE_NET_ERROR);

	const auto buildPosition = message.popPosition();
	const bool buildBig = message.popBool();
	const auto oldPosition = Vehicle->getPosition();

	if (buildBig)
	{
//		getMap()->moveVehicleBig (*Vehicle, buildPosition);
		Vehicle->getOwner()->doScan();
	}
	else
	{
//		getMap()->moveVehicle (*Vehicle, buildPosition);
		Vehicle->getOwner()->doScan();
	}

	if (Vehicle->getOwner() == activePlayer)
	{
		Vehicle->setBuildingType (message.popID());
		Vehicle->setBuildTurns (message.popInt16());
		Vehicle->BuildPath = message.popBool();
		Vehicle->bandPosition = message.popPosition();
	}

	Vehicle->setBuildingABuilding (true);
	addJob (new cStartBuildJob (*Vehicle, oldPosition, buildBig));

	if (Vehicle->getClientMoveJob()) Vehicle->getClientMoveJob()->release(); */
}


void cClient::HandleNetMessage_GAME_EV_STOP_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_STOP_BUILD);

	const int iID = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle == nullptr)
	{
		Log.write (" Client: Can't stop building: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of vehicle
		return;
	}

	const auto newPosition = message.popPosition();

	if (Vehicle->getIsBig())
	{
//		getMap()->moveVehicle (*Vehicle, newPosition);
		Vehicle->getOwner()->doScan();
	}

	Vehicle->setBuildingABuilding (false);
	Vehicle->BuildPath = false;
}

void cClient::HandleNetMessage_GAME_EV_BUILDLIST (cNetMessage& message)
{
	assert (message.iType == GAME_EV_BUILDLIST);

	const int iID = message.popInt16();
	cBuilding* Building = getBuildingFromID (iID);
	if (Building == nullptr)
	{
		Log.write (" Client: Can't set buildlist: Unknown building with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of building
		return;
	}

	std::vector<cBuildListItem> newBuildList;
	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		cBuildListItem buildListItem;
		buildListItem.setType (message.popID());
		buildListItem.setRemainingMetal (message.popInt16());
		newBuildList.push_back (std::move (buildListItem));
	}
	Building->setBuildList (std::move (newBuildList));

	Building->setMetalPerRound(message.popInt16());
	Building->setBuildSpeed(message.popInt16());
	Building->setRepeatBuild(message.popBool());
}

void cClient::HandleNetMessage_GAME_EV_MARK_LOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MARK_LOG);

	Log.write ("=============================================================================================", cLog::eLOG_TYPE_NET_DEBUG);
	Log.write (message.popString(), cLog::eLOG_TYPE_NET_DEBUG);
	Log.write ("=============================================================================================", cLog::eLOG_TYPE_NET_DEBUG);
}

void cClient::HandleNetMessage_GAME_EV_SUPPLY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SUPPLY);

	const int iType = message.popChar();
	cUnit* DestUnit = nullptr;
	if (message.popBool())
	{
		const int iID = message.popInt16();
		cVehicle* DestVehicle = getVehicleFromID (iID);
		DestUnit = DestVehicle;
		if (!DestVehicle)
		{
			Log.write (" Client: Can't supply vehicle: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of vehicle
			return;
		}
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.setAmmo (message.popInt16());
		else DestVehicle->data.setHitpoints (message.popInt16());

		//if (DestVehicle->isUnitLoaded ())
		//{
		//	// get the building which has loaded the unit
		//	cBuilding* Building = DestVehicle->owner->BuildingList;
		//	for (; Building; Building = Building->next)
		//	{
		//		if (Contains (Building->storedUnits, DestVehicle)) break;
		//	}
		//}
	}
	else
	{
		const int iID = message.popInt16();
		cBuilding* DestBuilding = getBuildingFromID (iID);
		DestUnit = DestBuilding;
		if (!DestBuilding)
		{
			Log.write (" Client: Can't supply building: Unknown building with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of building
			return;
		}
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.setAmmo (message.popInt16());
		else DestBuilding->data.setHitpoints (message.popInt16());
	}
	assert (DestUnit != nullptr);
	if (iType == SUPPLY_TYPE_REARM)
	{
		unitSuppliedWithAmmo (*DestUnit);
	}
	else
	{
		unitRepaired (*DestUnit);
	}
}

void cClient::HandleNetMessage_GAME_EV_ADD_RUBBLE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ADD_RUBBLE);

	const bool big = message.popBool();
	const int typ = message.popInt16();
	const int value = message.popInt16();
	const unsigned int ID = message.popInt16();
	auto rubble = std::make_shared<cBuilding> (nullptr, nullptr, nullptr, ID);

	rubble->setIsBig(big);
//	rubble->RubbleTyp = typ;
//	rubble->RubbleValue = value;
	const auto position = message.popPosition();

	rubble->setPosition (position);

//	getMap()->addBuilding (*rubble, rubble->getPosition());

	auto result = neutralBuildings.insert (std::move (rubble));
	assert (result.second);
}

void cClient::HandleNetMessage_GAME_EV_DETECTION_STATE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DETECTION_STATE);

	const int id = message.popInt32();
	cVehicle* vehicle = getVehicleFromID (id);
	if (vehicle == nullptr)
	{
		Log.write (" Client: Vehicle (ID: " + iToStr (id) + ") not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const bool detected = message.popBool();
	if (detected)
	{
		//mark vehicle as detected with size of detectedByPlayerList > 0
		vehicle->detectedByPlayerList.push_back (nullptr);
	}
	else
	{
		vehicle->detectedByPlayerList.clear();
	}
}

void cClient::HandleNetMessage_GAME_EV_CLEAR_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CLEAR_ANSWER);

	switch (message.popInt16())
	{
		case 0:
		{
			const int id = message.popInt16();
			cVehicle* Vehicle = getVehicleFromID (id);
			if (Vehicle == nullptr)
			{
				Log.write ("Client: Can not find vehicle with id " + iToStr (id) + " for clearing", cLog::eLOG_TYPE_NET_WARNING);
				break;
			}
			const auto orgiginalPosition = Vehicle->getPosition();

			Vehicle->setClearingTurns (message.popInt16());
			const auto bigPosition = message.popPosition();
			if (bigPosition.x() >= 0 && bigPosition.y() >= 0)
			{
//				getMap()->moveVehicleBig (*Vehicle, bigPosition);
				Vehicle->getOwner()->doScan();
			}
			Vehicle->setClearing (true);
			//addJob (new cStartBuildJob (*Vehicle, orgiginalPosition, (bigPosition.x() >= 0 && bigPosition.y() >= 0)));
		}
		break;
		case 1:
			// TODO: add blocked message
			// gameGUI->addMessage ("blocked");
			break;
		case 2:
			Log.write ("Client: warning on start of clearing", cLog::eLOG_TYPE_NET_WARNING);
			break;
		default:
			break;
	}
}

void cClient::HandleNetMessage_GAME_EV_STOP_CLEARING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_STOP_CLEARING);

	const int id = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (id);
	if (Vehicle == nullptr)
	{
		Log.write ("Client: Can not find vehicle with id " + iToStr (id) + " for stop clearing", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	const auto bigPosition = message.popPosition();
	if (bigPosition.x() >= 0 && bigPosition.y() >= 0)
	{
//		getMap()->moveVehicle (*Vehicle, bigPosition);
		Vehicle->getOwner()->doScan();
	}
	Vehicle->setClearing (false);
	Vehicle->setClearingTurns (0);
}

void cClient::HandleNetMessage_GAME_EV_NOFOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NOFOG);

	activePlayer->revealMap();
}

void cClient::HandleNetMessage_GAME_EV_DEFEATED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEFEATED);

/*	const int iTmp = message.popInt16();
	cPlayer* Player = getPlayerFromNumber (iTmp);
	if (Player == nullptr)
	{
		Log.write ("Client: Cannot find defeated player!", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	Player->isDefeated = true;
	activePlayer->addSavedReport (std::make_unique<cSavedReportPlayerDefeated> (*Player));
#if 0
	for (unsigned int i = 0; i < getPlayerList()->size(); i++)
	{
		if (Player == (*getPlayerList()) [i])
		{
			Hud.ExtraPlayers (Player->getName() + " (d)", Player->getColor(), i, Player->bFinishedTurn, false);
			return;
		}
	}
#endif*/
}
void cClient::HandleNetMessage_GAME_EV_DEL_PLAYER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_PLAYER);

/*	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	if (Player == activePlayer)
	{
		Log.write ("Client: Cannot delete own player!", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}
	if (Player->hasUnits())
	{
		Log.write ("Client: Player to be deleted has some units left !", cLog::eLOG_TYPE_NET_ERROR);
	}
	activePlayer->addSavedReport (std::make_unique<cSavedReportPlayerLeft> (*Player));

	deletePlayer (*Player);*/
}

void cClient::HandleNetMessage_GAME_EV_STORE_UNIT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_STORE_UNIT);

	cVehicle* storedVehicle = getVehicleFromID (message.popInt16());
	if (!storedVehicle) return;

	if (message.popBool())
	{
		cVehicle* storingVehicle = getVehicleFromID (message.popInt16());
		if (!storingVehicle) return;
//		storingVehicle->storeVehicle (*storedVehicle, *getMap());
		unitStored (*storingVehicle, *storedVehicle);
	}
	else
	{
		cBuilding* storingBuilding = getBuildingFromID (message.popInt16());
		if (!storingBuilding) return;
//		storingBuilding->storeVehicle (*storedVehicle, *getMap());
		unitStored (*storingBuilding, *storedVehicle);
	}
}

void cClient::HandleNetMessage_GAME_EV_EXIT_UNIT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_EXIT_UNIT);

	cVehicle* StoredVehicle = getVehicleFromID (message.popInt16());
	if (!StoredVehicle) return;

	if (message.popBool())
	{
		cVehicle* StoringVehicle = getVehicleFromID (message.popInt16());
		if (!StoringVehicle) return;

		const auto position = message.popPosition();
//		StoringVehicle->exitVehicleTo (*StoredVehicle, position, *getMap());
		unitActivated (*StoringVehicle, *StoredVehicle);
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		const auto position = message.popPosition();
//		StoringBuilding->exitVehicleTo (*StoredVehicle, position, *getMap());
		unitActivated (*StoringBuilding, *StoredVehicle);
	}
}

void cClient::HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_UPGRADE_VALUES);

	const sID ID = message.popID();
	cDynamicUnitData* Data = activePlayer->getUnitDataCurrentVersion (ID);
	if (Data == nullptr) return;

	Data->setVersion (message.popInt16());
	Data->setScan (message.popInt16());
	Data->setRange (message.popInt16());
	Data->setDamage (message.popInt16());
	Data->setBuildCost(message.popInt16());
	Data->setArmor (message.popInt16());
	Data->setSpeedMax (message.popInt16());
	Data->setShotsMax (message.popInt16());
	Data->setAmmoMax (message.popInt16());
	Data->setHitpointsMax (message.popInt16());
}

void cClient::HandleNetMessage_GAME_EV_CREDITS_CHANGED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CREDITS_CHANGED);

	activePlayer->setCredits (message.popInt32());
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UPGRADED_BUILDINGS);

	const int buildingsInMsg = message.popInt16();
	const int totalCosts = message.popInt16();
	if (buildingsInMsg <= 0) return;

	const cDynamicUnitData* unitData = nullptr;
	bool scanNecessary = false;
	for (int i = 0; i < buildingsInMsg; i++)
	{
		const int buildingID = message.popInt32();
		cBuilding* building = getBuildingFromID (buildingID);
		if (!building)
		{
			Log.write (" Client: Unknown building with ID: " + iToStr (buildingID), cLog::eLOG_TYPE_NET_ERROR);
			break;
		}
		const cDynamicUnitData& upgraded = *activePlayer->getUnitDataCurrentVersion (building->data.getId());
		if (building->data.getScan() < upgraded.getScan())
			scanNecessary = true; // Scan range was upgraded. So trigger a scan.
		building->upgradeToCurrentVersion();
		if (i == 0)
		{
			unitData = &building->data;
		}
	}
	assert (unitData != nullptr);
	//activePlayer->addSavedReport (std::make_unique<cSavedReportUpgraded> (unitData->ID, buildingsInMsg, totalCosts));
	if (scanNecessary)
		activePlayer->doScan();
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UPGRADED_VEHICLES);

	const int vehiclesInMsg = message.popInt16();
	const int totalCosts = message.popInt16();
	/*const unsigned int storingBuildingID =*/ message.popInt32();
	if (vehiclesInMsg <= 0) return;

	const cDynamicUnitData* unitData = nullptr;
	for (int i = 0; i < vehiclesInMsg; i++)
	{
		const int vehicleID = message.popInt32();
		cVehicle* vehicle = getVehicleFromID (vehicleID);
		if (!vehicle)
		{
			Log.write (" Client: Unknown vehicle with ID: " + iToStr (vehicleID), cLog::eLOG_TYPE_NET_ERROR);
			break;
		}
		vehicle->upgradeToCurrentVersion();
		if (i == 0)
		{
			unitData = &vehicle->data;
		}
	}
	assert (unitData != nullptr);
	//activePlayer->addSavedReport (std::make_unique<cSavedReportUpgraded> (unitData->ID, vehiclesInMsg, totalCosts));
}

void cClient::HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESEARCH_SETTINGS);

	const int buildingsInMsg = message.popInt16();
	for (int i = 0; i < buildingsInMsg; ++i)
	{
		const int buildingID = message.popInt32();
		const cResearch::ResearchArea newArea = (cResearch::ResearchArea)message.popChar();
		cBuilding* building = getBuildingFromID (buildingID);
		if (building && building->getStaticUnitData().canResearch && 0 <= newArea && newArea <= cResearch::kNrResearchAreas)
			building->setResearchArea (newArea);
	}
	// now update the research center count for the areas
	activePlayer->refreshResearchCentersWorkingOnArea();
}

void cClient::HandleNetMessage_GAME_EV_RESEARCH_LEVEL (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESEARCH_LEVEL);

	for (int area = cResearch::kNrResearchAreas - 1; area >= 0; area--)
	{
		const int newCurPoints = message.popInt16();
		const int newLevel = message.popInt16();
		activePlayer->getResearchState().setCurResearchLevel (newLevel, area);
		activePlayer->getResearchState().setCurResearchPoints (newCurPoints, area);
	}
}

void cClient::HandleNetMessage_GAME_EV_FINISHED_RESEARCH_AREAS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FINISHED_RESEARCH_AREAS);

	std::vector<int> areas (message.popInt32());
	for (size_t i = 0; i < areas.size(); ++i)
	{
		areas[i] = message.popInt32();
	}
	activePlayer->setCurrentTurnResearchAreasFinished (std::move (areas));
}

void cClient::HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REFRESH_RESEARCH_COUNT);

	activePlayer->refreshResearchCentersWorkingOnArea();
}

void cClient::HandleNetMessage_GAME_EV_SET_AUTOMOVE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SET_AUTOMOVE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle)
	{
		Vehicle->startAutoMoveJob (*this);
	}
}

void cClient::HandleNetMessage_GAME_EV_COMMANDO_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_COMMANDO_ANSWER);

	bool success = message.popBool();
	bool steal = false;
	if (success) steal = message.popBool();
	cVehicle* vehicle = getVehicleFromID (message.popInt16());

	if (vehicle)
	{
		if (success)
		{
			if (steal) unitHasStolenSuccessfully (*vehicle);
			else unitHasDisabledSuccessfully (*vehicle);
		}
		else
		{
			unitStealDisableFailed (*vehicle);
		}
	}
}

void cClient::HandleNetMessage_GAME_EV_SCORE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SCORE);

	const int pn = message.popInt16();
	const int turn = message.popInt16();
	const int n = message.popInt16();

//	getPlayerFromNumber (pn)->setScore (n, turn);
}

void cClient::HandleNetMessage_GAME_EV_NUM_ECOS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NUM_ECOS);

	const int pn = message.popInt16();
	const int n = message.popInt16();

//	getPlayerFromNumber (pn)->numEcos = n;
}

void cClient::HandleNetMessage_GAME_EV_UNIT_SCORE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_SCORE);

	cBuilding* b = getBuildingFromID (message.popInt16());
	b->points = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building) return;

	addDestroyFx (*building);
}

void cClient::HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_MOVE_ACTION_SERVER);

	cVehicle* vehicle = getVehicleFromID (message.popInt32());
	if (!vehicle || !vehicle->getMoveJob()) return;

	const int destID = message.popInt32();
	eEndMoveActionType type = (eEndMoveActionType) message.popChar();
	//vehicle->getMoveJob()->endMoveAction = new cEndMoveAction (vehicle, destID, type);
}


void cClient::HandleNetMessage_GAME_EV_REVEAL_MAP (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REVEAL_MAP);

	activePlayer->revealMap();
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

int cClient::handleNetMessage (cNetMessage& message)
{


	switch (message.iType)
	{
		case GAME_EV_DEL_BUILDING: HandleNetMessage_GAME_EV_DEL_BUILDING (message); break;
		case GAME_EV_DEL_VEHICLE: HandleNetMessage_GAME_EV_DEL_VEHICLE (message); break;
		case GAME_EV_ATTACKJOB: attackJobs.push_back (new cAttackJob (this, message)); break;
		case GAME_EV_UNIT_DATA: HandleNetMessage_GAME_EV_UNIT_DATA (message); break;
		case GAME_EV_SPECIFIC_UNIT_DATA: HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (message); break;
		case GAME_EV_RESOURCES: HandleNetMessage_GAME_EV_RESOURCES (message); break;
		case GAME_EV_BUILD_ANSWER: HandleNetMessage_GAME_EV_BUILD_ANSWER (message); break;
		case GAME_EV_STOP_BUILD: HandleNetMessage_GAME_EV_STOP_BUILD (message); break;
		case GAME_EV_BUILDLIST: HandleNetMessage_GAME_EV_BUILDLIST (message); break;
		case GAME_EV_MARK_LOG: HandleNetMessage_GAME_EV_MARK_LOG (message); break;
		case GAME_EV_SUPPLY: HandleNetMessage_GAME_EV_SUPPLY (message); break;
		case GAME_EV_ADD_RUBBLE: HandleNetMessage_GAME_EV_ADD_RUBBLE (message); break;
		case GAME_EV_DETECTION_STATE: HandleNetMessage_GAME_EV_DETECTION_STATE (message); break;
		case GAME_EV_CLEAR_ANSWER: HandleNetMessage_GAME_EV_CLEAR_ANSWER (message); break;
		case GAME_EV_STOP_CLEARING: HandleNetMessage_GAME_EV_STOP_CLEARING (message); break;
		case GAME_EV_NOFOG: HandleNetMessage_GAME_EV_NOFOG (message); break;
		case GAME_EV_DEFEATED: HandleNetMessage_GAME_EV_DEFEATED (message); break;
		case GAME_EV_DEL_PLAYER: HandleNetMessage_GAME_EV_DEL_PLAYER (message); break;
		case GAME_EV_STORE_UNIT: HandleNetMessage_GAME_EV_STORE_UNIT (message); break;
		case GAME_EV_EXIT_UNIT: HandleNetMessage_GAME_EV_EXIT_UNIT (message); break;
		case GAME_EV_UNIT_UPGRADE_VALUES: HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (message); break;
		case GAME_EV_CREDITS_CHANGED: HandleNetMessage_GAME_EV_CREDITS_CHANGED (message); break;
		case GAME_EV_UPGRADED_BUILDINGS: HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (message); break;
		case GAME_EV_UPGRADED_VEHICLES: HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (message); break;
		case GAME_EV_RESEARCH_SETTINGS: HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (message); break;
		case GAME_EV_RESEARCH_LEVEL: HandleNetMessage_GAME_EV_RESEARCH_LEVEL (message); break;
		case GAME_EV_FINISHED_RESEARCH_AREAS: HandleNetMessage_GAME_EV_FINISHED_RESEARCH_AREAS (message); break;
		case GAME_EV_REFRESH_RESEARCH_COUNT: // sent, when the player was resynced (or a game was loaded)
			HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (message); break;
		case GAME_EV_SET_AUTOMOVE: HandleNetMessage_GAME_EV_SET_AUTOMOVE (message); break;
		case GAME_EV_COMMANDO_ANSWER: HandleNetMessage_GAME_EV_COMMANDO_ANSWER (message); break;
		case GAME_EV_SCORE: HandleNetMessage_GAME_EV_SCORE (message); break;
		case GAME_EV_NUM_ECOS: HandleNetMessage_GAME_EV_NUM_ECOS (message); break;
		case GAME_EV_UNIT_SCORE: HandleNetMessage_GAME_EV_UNIT_SCORE (message); break;
		case GAME_EV_SELFDESTROY: HandleNetMessage_GAME_EV_SELFDESTROY (message); break;
		case GAME_EV_END_MOVE_ACTION_SERVER: HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (message); break;
		case GAME_EV_REVEAL_MAP: HandleNetMessage_GAME_EV_REVEAL_MAP (message); break;

		default:
			Log.write ("Client: Can not handle message type " + message.getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}

	return 0;
}

void cClient::addAutoMoveJob (std::weak_ptr<cAutoMJob> autoMoveJob)
{
	autoMoveJobs.push_back (std::move (autoMoveJob));
}

void cClient::handleAutoMoveJobs()
{
	std::vector<cAutoMJob*> activeAutoMoveJobs;

	// clean up deleted and finished move jobs
	for (auto i = autoMoveJobs.begin(); i != autoMoveJobs.end(); /*erase in loop*/)
	{
		if (i->expired())
		{
			i = autoMoveJobs.erase (i);
		}
		else
		{
			auto job = i->lock();
			if (job->isFinished())
			{
				job->getVehicle().stopAutoMoveJob();
				if (job.use_count() == 1)
				{
					i = autoMoveJobs.erase (i);
				}
			}
			else
			{
				activeAutoMoveJobs.push_back (job.get());
				++i;
			}
		}
	}

	// We do not want to execute all auto move jobs at the same time.
	// instead we do a round-robin like scheduling for the auto move jobs:
	// we do executed only a limited number of auto move jobs at a time.
	// To do so we execute only the first N auto move jobs in the list
	// and afterwards push them to the end of the list so that they will only be executed
	// when all the other jobs have been executed once as well.
	const size_t maxConcurrentJobs = 2;

	auto jobsToExecute = std::min (activeAutoMoveJobs.size(), maxConcurrentJobs);

	while (jobsToExecute > 0)
	{
		auto job = autoMoveJobs.front().lock();
		job->doAutoMove (activeAutoMoveJobs);

		autoMoveJobs.pop_front();
		autoMoveJobs.push_back (job);

		--jobsToExecute;
	}
}

cVehicle* cClient::getVehicleFromID (unsigned int id) const
{
	for (unsigned int i = 0; i < model.getPlayerList().size(); i++)
	{
		cPlayer& player = *model.getPlayerList()[i];
		auto unit = player.getVehicleFromId (id);
		if (unit) return unit;
	}
	return nullptr;
}

cBuilding* cClient::getBuildingFromID (unsigned int id) const
{
	for (unsigned int i = 0; i < model.getPlayerList().size(); i++)
	{
		cPlayer& player = *model.getPlayerList() [i];
		auto unit = player.getBuildingFromId (id);
		if (unit) return unit;
	}
	auto iter = neutralBuildings.find (id);
	return iter == neutralBuildings.end() ? nullptr : iter->get();
}

cUnit* cClient::getUnitFromID (unsigned int id) const
{
	cUnit* result = getVehicleFromID (id);

	if (result == nullptr)
		result = getBuildingFromID (id);
	return result;
}

void cClient::doGameActions()
{
	runFx();

	// run attackJobs
	cAttackJob::runAttackJobs (attackJobs);

	// run surveyor ai
	//if (gameTimer->timer50ms)
	{
		handleAutoMoveJobs();
	}

	runJobs();
}

cSubBase* cClient::getSubBaseFromID (int iID)
{
	cBuilding* building = getBuildingFromID (iID);
	if (building)
		return building->subBase;
	return nullptr;
}

void cClient::addDestroyFx (cVehicle& vehicle)
{
	// play explosion
	if (vehicle.getIsBig())
	{
		addFx (std::make_shared<cFxExploBig> (vehicle.getPosition() * 64 + 64, model.getMap()->isWaterOrCoast (vehicle.getPosition())));
	}
	else if (vehicle.getStaticUnitData().factorAir > 0 && vehicle.getFlightHeight() != 0)
	{
		addFx (std::make_shared<cFxExploAir> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
	else if (model.getMap()->isWaterOrCoast(vehicle.getPosition()))
	{
		addFx (std::make_shared<cFxExploWater> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
	else
	{
		addFx (std::make_shared<cFxExploSmall> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}

	if (vehicle.uiData->hasCorpse)
	{
		// add corpse
		addFx (std::make_shared<cFxCorpse> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
}

void cClient::addDestroyFx (cBuilding& building)
{
	// play explosion animation
	cBuilding* topBuilding = model.getMap()->getField(building.getPosition()).getBuilding();
	if (topBuilding && topBuilding->getIsBig())
	{
		addFx(std::make_shared<cFxExploBig>(topBuilding->getPosition() * 64 + 64, model.getMap()->isWaterOrCoast(topBuilding->getPosition())));
	}
	else
	{
		addFx (std::make_shared<cFxExploSmall> (building.getPosition() * 64 + 32));
	}
}

//------------------------------------------------------------------------------
void cClient::deletePlayer (cPlayer& player)
{
	player.setIsRemovedFromGame (true);
	auto playerList = model.getPlayerList();
	playerList.erase (std::remove_if (playerList.begin(), playerList.end(), [&player] (const std::shared_ptr<cPlayer>& entry) { return entry.get() == &player; }), playerList.end());
}

//------------------------------------------------------------------------------
void cClient::addJob (cJob* job)
{
	helperJobs.addJob (*job);
}

//------------------------------------------------------------------------------
void cClient::runJobs()
{
	helperJobs.run (*gameTimer);
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
void cClient::loadModel(int saveGameNumber)
{
	cSavegame savegame;
	savegame.loadModel(model, saveGameNumber);

	Log.write(" Client: loaded model. GameId: " + toString(model.getGameId()), cLog::eLOG_TYPE_NET_DEBUG);
}
//------------------------------------------------------------------------------
void cClient::run()
{
	gameTimer->run(*this, model);
}
