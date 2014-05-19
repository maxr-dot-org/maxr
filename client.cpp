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

#include "client.h"

#include "attackJobs.h"
#include "automjobs.h"
#include "buildings.h"
#include "casualtiestracker.h"
#include "clientevents.h"
#include "clist.h"
#include "fxeffects.h"
#include "gametimer.h"
#include "hud.h"
#include "jobs.h"
#include "log.h"
#include "main.h"
#include "netmessage.h"
#include "player.h"
#include "server.h"
#include "serverevents.h"
#include "settings.h"
#include "vehicles.h"
#include "video.h"
#include "gui/menu/windows/windowgamesettings/gamesettings.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreporttranslated.h"
#include "game/data/report/savedreportunit.h"

using namespace std;

//------------------------------------------------------------------------
// cClient implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cClient::cClient (cServer* server_, std::shared_ptr<cTCP> network_) :
	server (server_),
	network (std::move(network_)),
	ActivePlayer (NULL),
	casualtiesTracker (new cCasualtiesTracker()),
	gameTimer(),
	effectsList (new cFxContainer)
{
	assert (server != nullptr || network != nullptr);

	gameTimer.setClient (this);
	if (server) server->addLocalClient (*this);
	else network->setMessageReceiver (this);
	neutralBuildings = NULL;
	bDefeated = false;
	iTurn = 1;
	bWantToEnd = false;
	iEndTurnTime = 0;
	iStartTurnTime = 0;

	gameTimer.start();
}

cClient::~cClient()
{
	if (network != nullptr) network->setMessageReceiver (nullptr);

	gameTimer.stop();

	for (unsigned int i = 0; i < attackJobs.size(); i++)
	{
		delete attackJobs[i];
	}
	while (neutralBuildings)
	{
		cBuilding* nextBuilding = neutralBuildings->next;
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}

	// since currently the vehicles do own movejobs and other stuff that
	// have non owning references to the client, we delete all units in the players
	// and hopefully eliminate all those references by doing so...
	for (size_t i = 0; i < playerList.size (); ++i)
	{
		playerList[i]->deleteAllUnits ();
	}
}

void cClient::setMap (std::shared_ptr<cStaticMap> staticMap)
{
	Map = std::make_shared<cMap>(staticMap);
	initPlayersWithMap();
}

void cClient::setGameSetting (const cGameSettings& gameSetting_)
{
	gameSetting = new cGameSettings (gameSetting_);
}

class LessByNr
{
public:
	bool operator() (const cPlayer* lhs, const cPlayer* rhs) const
	{
		return lhs->getNr() < rhs->getNr();
	}
};

void cClient::setPlayers (const std::vector<sPlayer>& splayers, size_t activePlayerIndex)
{
	assert (activePlayerIndex < splayers.size ());

	for (size_t i = 0, size = splayers.size(); i != size; ++i)
	{
		playerList.push_back(std::make_shared<cPlayer>(splayers[i]));
	}

	ActivePlayer = playerList[activePlayerIndex].get();

	//std::sort(PlayerList.begin(), PlayerList.end(), LessByNr());

	initPlayersWithMap();
}

void cClient::initPlayersWithMap()
{
	if (playerList.empty() || Map == NULL) return;

	for (size_t i = 0; i != playerList.size (); ++i)
	{
		playerList[i]->initMaps (*Map);
	}

	// generate subbase for enemy players
	for (size_t i = 0; i != playerList.size (); ++i)
	{
		if (playerList[i].get() == ActivePlayer) continue;
		playerList[i]->base.SubBases.push_back (new sSubBase (playerList[i].get()));
	}
}

/*virtual*/ void cClient::pushEvent (std::unique_ptr<cNetMessage> message)
{
	if (message->iType == NET_GAME_TIME_SERVER)
	{
		// this is a preview for the client to know
		// how many sync messages are in queue
		// used to detect a growing lag behind the server time
		message->popInt32();
		unsigned int receivedTime = message->popInt32();
		message->rewind();

		gameTimer.setReceivedTime (receivedTime);
	}
	eventQueue.push (std::move(message));
}

void cClient::sendNetMessage (cNetMessage* message_) const
{
	// FIXME: sendNetMessage should take a unique_ptr directly!
	std::unique_ptr<cNetMessage> message (message_);

	message->iPlayerNr = ActivePlayer->getNr();

	if (message->iType != NET_GAME_TIME_CLIENT)
	{
		Log.write ("Client: " + getActivePlayer ().getName () + " --> "
				   + message->getTypeAsString ()
				   + ", gameTime:" + iToStr (this->gameTimer.gameTime)
				   + ", Hexdump: " + message->getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);
	}

	if (server)
	{
		// push an event to the local server in singleplayer, HotSeat or
		// if this machine is the host
		server->pushEvent (std::move(message));
	}
	else // else send it over the net
	{
		//the client is only connected to one socket
		//so network->send() only sends to the server
		network->send (message->iLength, message->serialize());
	}
}

int cClient::addMoveJob (cVehicle& vehicle, const cPosition& destination, const std::vector<cVehicle*>* group)
{
	sWaypoint* path = cClientMoveJob::calcPath (*getMap(), vehicle.getPosition(), destination, vehicle, group);
	if (path)
	{
		sendMoveJob (*this, path, vehicle.iID);
		Log.write (" Client: Added new movejob: VehicleID: " + iToStr (vehicle.iID) + ", SrcX: " + iToStr (vehicle.getPosition().x()) + ", SrcY: " + iToStr (vehicle.getPosition().y()) + ", DestX: " + iToStr (destination.x()) + ", DestY: " + iToStr (destination.y()), cLog::eLOG_TYPE_NET_DEBUG);
		return 1;
	}
	else
	{
		moveJobBlocked (vehicle);
		return 0;
	}
}

void cClient::startGroupMove(const std::vector<cVehicle*>& group_, const cPosition& mainDestination)
{
	const auto& mainPosition = group_[0]->getPosition();

	// copy the selected-units-list
	std::vector<cVehicle*> group = group_;

	// go trough all vehicles in the list
	while (group.size())
	{
		// we will start moving the vehicles in the list with the vehicle
		// that is the closest to the destination.
		// this will avoid that the units will crash into each other
		// because the one infront of them has started
		// his move and the next field is free.
		// TODO: sort group by ditance, then proceed.
		int shortestWaySquareLength = 0x7FFFFFFF;
		int shortestWayVehNum = 0;
		for (unsigned int i = 0; i < group.size(); i++)
		{
			cVehicle* vehicle = group[i];
			const auto delta = vehicle->getPosition() + mainDestination + vehicle->getPosition() - mainPosition;
			const int waySquareLength = delta.l2NormSquared();

			if (waySquareLength < shortestWaySquareLength)
			{
				shortestWaySquareLength = waySquareLength;
				shortestWayVehNum = i;
			}
		}
		cVehicle& vehicle = *group[shortestWayVehNum];
		// add the movejob to the destination of the unit.
		// the formation of the vehicle group will stay as destination formation.
		auto destination = mainDestination + vehicle.getPosition() - mainPosition;
		addMoveJob(vehicle, destination, &group_);
		// delete the unit from the copyed list
		group.erase (group.begin() + shortestWayVehNum);
	}
}

void cClient::runFx()
{
	effectsList->run ();
}

void cClient::addFx (std::shared_ptr<cFx> fx)
{
	effectsList->push_back (fx);
	addedEffect (fx);
}

void cClient::HandleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	network->close (message.popInt16());
	//FIXME: gameGUI
	//gameGUI->onLostConnection();
}

//void cClient::HandleNetMessage_GAME_EV_CHAT_SERVER (cNetMessage& message)
//{
//	assert (message.iType == GAME_EV_CHAT_SERVER);
//
//	switch (message.popChar())
//	{
//		case USER_MESSAGE:
//		{
//			const string msg = message.popString ();
//			//getActivePlayer ().addSavedReport (std::make_unique<cSavedReportChat>(msg, sSavedReportMessage::REPORT_TYPE_CHAT);
//			//FIXME: gameGUI
//			//PlayFX (SoundData.SNDChat);
//			break;
//		}
//		case SERVER_ERROR_MESSAGE:
//		{
//			const string msg = lngPack.i18n (message.popString ());
//			//getActivePlayer ().addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_COMP);
//			//FIXME: gameGUI
//			//PlayFX (SoundData.SNDQuitsch);
//			break;
//		}
//		case SERVER_INFO_MESSAGE:
//		{
//			const string translationpath = message.popString();
//			const string inserttext = message.popString();
//			string msgString;
//			if (inserttext.empty()) msgString = lngPack.i18n (translationpath);
//			else msgString = lngPack.i18n (translationpath, inserttext);
//			getActivePlayer ().addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
//			break;
//		}
//	}
//}

void cClient::HandleNetMessage_GAME_EV_PLAYER_CLANS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_PLAYER_CLANS);

	for (unsigned int i = 0; i < getPlayerList().size(); i++)
	{
		const int playerNr = message.popChar();
		const int clan = message.popChar();

		cPlayer* player = getPlayerFromNumber (playerNr);
		player->setClan (clan);
	}
}

void cClient::HandleNetMessage_GAME_EV_ADD_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ADD_BUILDING);

	const bool Init = message.popBool();
	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	if (!Player)
	{
		Log.write ("Player not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sID UnitID = message.popID();
	const auto position = message.popPosition();
	unsigned int ID = message.popInt16();
	cBuilding* AddedBuilding = Player->addBuilding(position, UnitID, ID);

	addUnit(position, *AddedBuilding, Init);

	Player->base.addBuilding (AddedBuilding, NULL);
}

void cClient::HandleNetMessage_GAME_EV_ADD_VEHICLE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ADD_VEHICLE);

	const bool Init = message.popBool();
	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	if (!Player)
	{
		Log.write ("Player not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sID UnitID = message.popID ();
	const auto position = message.popPosition ();
	const unsigned int ID = message.popInt16();
	const bool bAddToMap = message.popBool();

	cVehicle* AddedVehicle = Player->addVehicle(position, UnitID, ID);
	addUnit(position, *AddedVehicle, Init, bAddToMap);
}

void cClient::HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_BUILDING);

	cBuilding* Building = getBuildingFromID (message.popInt16());

	if (Building)
	{
		deleteUnit (Building);
	}
}

void cClient::HandleNetMessage_GAME_EV_DEL_VEHICLE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_VEHICLE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());

	if (Vehicle) deleteUnit (Vehicle);
}

void cClient::HandleNetMessage_GAME_EV_ADD_ENEM_VEHICLE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ADD_ENEM_VEHICLE);

	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	if (!Player)
	{
		Log.write ("Player not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sID UnitID = message.popID ();
	const auto position = message.popPosition ();
	const int dir = message.popInt16();
	const int ID = message.popInt16();
	const int version = message.popInt16();
	cVehicle* AddedVehicle = Player->addVehicle (position, UnitID, ID);

	AddedVehicle->dir = dir;
	AddedVehicle->data.setVersion(version);
	addUnit (position, *AddedVehicle, false);
}

void cClient::HandleNetMessage_GAME_EV_ADD_ENEM_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ADD_ENEM_BUILDING);

	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	if (!Player)
	{
		Log.write ("Player not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const sID UnitID = message.popID ();
	const auto position = message.popPosition ();
	const int ID = message.popInt16();
	const int version = message.popInt16();
	cBuilding* AddedBuilding = Player->addBuilding (position, UnitID, ID);

	AddedBuilding->data.setVersion(version);
	addUnit (position, *AddedBuilding, false);

	if (AddedBuilding->data.connectsToBase)
	{
		Player->base.SubBases[0]->buildings.push_back (AddedBuilding);
		AddedBuilding->SubBase = Player->base.SubBases[0];

		AddedBuilding->updateNeighbours (*getMap());
	}
}

void cClient::HandleNetMessage_GAME_EV_WAIT_FOR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WAIT_FOR);

	const int nextPlayerNum = message.popInt16();

	if (nextPlayerNum != ActivePlayer->getNr())
	{
		enableFreezeMode (FREEZE_WAIT_FOR_OTHERS, nextPlayerNum);
	}
}

void cClient::HandleNetMessage_GAME_EV_MAKE_TURNEND (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MAKE_TURNEND);

	const int iNextPlayerNum = message.popInt16();
	const bool bWaitForNextPlayer = message.popBool();
	const bool bEndTurn = message.popBool();

	iEndTurnTime = gameTimer.gameTime;
	if (bEndTurn)
	{
		iStartTurnTime = gameTimer.gameTime;
		iTurn++;
		bWantToEnd = false;
		ActivePlayer->clearDone();
		Log.write ("######### Round " + iToStr (iTurn) + " ###########", cLog::eLOG_TYPE_NET_DEBUG);
		for (unsigned int i = 0; i < getPlayerList().size(); i++)
		{
			getPlayerList() [i]->bFinishedTurn = false;
		}
		turnChanged ();
		finishedTurnEndProcess ();
	}

	if (bWaitForNextPlayer)
	{
		if (iNextPlayerNum != ActivePlayer->getNr())
		{
			enableFreezeMode (FREEZE_WAIT_FOR_OTHERS, iNextPlayerNum);
		}
		else
		{
			disableFreezeMode (FREEZE_WAIT_FOR_OTHERS);
		}
	}
	else if (iNextPlayerNum != -1)
	{
		//makeHotSeatEnd (iNextPlayerNum);
	}
}

void cClient::HandleNetMessage_GAME_EV_FINISHED_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FINISHED_TURN);

	const int iPlayerNum = message.popInt16();
	const int iTimeDelay = message.popInt32();
	cPlayer* Player = getPlayerFromNumber (iPlayerNum);

	if (Player == NULL && iPlayerNum != -1)
	{
		Log.write (" Client: Player with nr " + iToStr (iPlayerNum) + " has finished turn, but can't find him", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	if (Player) Player->bFinishedTurn = true;

	if (iTimeDelay != -1)
	{
		if (iPlayerNum != ActivePlayer->getNr() && iPlayerNum != -1)
		{
			string msgString = lngPack.i18n ("Text~Multiplayer~Player_Turn_End", Player->getName()) + ". " + lngPack.i18n ("Text~Multiplayer~Deadline", iToStr (iTimeDelay / 100));
			ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (msgString));
		}
		iEndTurnTime = gameTimer.gameTime + iTimeDelay;
	}
	else if (iPlayerNum != ActivePlayer->getNr() && iPlayerNum != -1)
	{
		ActivePlayer->addSavedReport (std::make_unique<cSavedReportTranslated> ("Text~Multiplayer~Player_Turn_End", Player->getName()));
	}
}

void cClient::HandleNetMessage_GAME_EV_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_DATA);

	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	(void) Player;  // TODO use me
	const int iID = message.popInt16();
	const bool bVehicle = message.popBool ();
	const auto position = message.popPosition ();

	Log.write (" Client: Received Unit Data: Vehicle: " + iToStr ((int) bVehicle) + ", ID: " + iToStr (iID) + ", XPos: " + iToStr (position.x()) + ", YPos: " + iToStr (position.y()), cLog::eLOG_TYPE_NET_DEBUG);
	cVehicle* Vehicle = NULL;
	sUnitData* Data = NULL;
	bool bWasBuilding = false;
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
			// if the vehicle is moving it is normal that
			// the positions may not be the same,
			// when the vehicle was building it is also normal that
			// the position should be changed
			// so the log message will just be a debug one
			int iLogType = cLog::eLOG_TYPE_NET_WARNING;
			if (Vehicle->isUnitBuildingABuilding () || Vehicle->isUnitClearing () || Vehicle->moving) iLogType = cLog::eLOG_TYPE_NET_DEBUG;
			Log.write (" Client: Vehicle identificated by ID (" + iToStr (iID) + ") but has wrong position [IS: X" + iToStr (Vehicle->getPosition().x()) + " Y" + iToStr (Vehicle->getPosition().y()) + "; SHOULD: X" + iToStr (position.x()) + " Y" + iToStr (position.y()) + "]", iLogType);

			// set to server position if vehicle is not moving
			if (!Vehicle->MoveJobActive)
			{
				getMap()->moveVehicle (*Vehicle, position);
				if (bBig) getMap()->moveVehicleBig (*Vehicle, position);
				Vehicle->owner->doScan();
			}
		}

		if (message.popBool()) Vehicle->changeName (message.popString());

		Vehicle->setIsBeeinAttacked(message.popBool());
		const bool bWasDisabled = Vehicle->isDisabled();
		Vehicle->setDisabledTurns(message.popInt16 ());
		Vehicle->setCommandoRank(message.popInt16());
		Vehicle->setClearing(message.popBool());
		bWasBuilding = Vehicle->isUnitBuildingABuilding ();
		Vehicle->setBuildingABuilding(message.popBool());
		Vehicle->setBuildTurns(message.popInt16());
		Vehicle->setClearingTurns(message.popInt16());
		Vehicle->setSentryActive(message.popBool());
		Vehicle->setManualFireActive(message.popBool());

		if (Vehicle->isDisabled() != bWasDisabled && Vehicle->owner == ActivePlayer)
		{
			if (Vehicle->isDisabled())
			{
				const std::string msg = Vehicle->getDisplayName () + " " + lngPack.i18n ("Text~Comp~Disabled");
				ActivePlayer->addSavedReport (std::make_unique<cSavedReportUnit>(*Vehicle, msg));
				unitDisabled (*Vehicle);
			}
			Vehicle->owner->doScan();
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

		const bool bWasDisabled = Building->isDisabled();
		Building->setDisabledTurns (message.popInt16 ());
		Building->researchArea = message.popInt16();
		Building->setWorking(message.popBool());
		Building->setSentryActive( message.popBool ());
		Building->setManualFireActive( message.popBool ());
		Building->points = message.popInt16();

		if (Building->isDisabled() != bWasDisabled && Building->owner == ActivePlayer)
		{
			if (Building->isDisabled())
			{
				const std::string msg = Building->getDisplayName () + " " + lngPack.i18n ("Text~Comp~Disabled");
				ActivePlayer->addSavedReport (std::make_unique<cSavedReportUnit> (*Building, msg));
				unitDisabled (*Building);
			}
			Building->owner->doScan();
		}
		Data = &Building->data;
	}

	Data->buildCosts = message.popInt16();
	Data->setAmmo(message.popInt16());
	Data->ammoMax = message.popInt16();
	Data->storageResCur = message.popInt16();
	Data->storageResMax = message.popInt16();
	Data->storageUnitsCur = message.popInt16();
	Data->storageUnitsMax = message.popInt16();
	Data->damage = message.popInt16();
	Data->setShots(message.popInt16());
	Data->shotsMax = message.popInt16();
	Data->range = message.popInt16();
	Data->scan = message.popInt16();
	Data->armor = message.popInt16();
	Data->setHitpoints(message.popInt16());
	Data->hitpointsMax = message.popInt16();
	Data->setVersion(message.popInt16());

	if (bVehicle)
	{
		if (Data->canPlaceMines)
		{
			if (Data->storageResCur <= 0) Vehicle->setLayMines(false);
			if (Data->storageResCur >= Data->storageResMax) Vehicle->setClearMines(false);
		}
		Data->speedCur = message.popInt16();
		Data->speedMax = message.popInt16();

		Vehicle->FlightHigh = message.popInt16 ();

		if (bWasBuilding && !Vehicle->isUnitBuildingABuilding ())
		{
			unitStoppedBuilding (*Vehicle);
		}
	}
}

void cClient::HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SPECIFIC_UNIT_DATA);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (!Vehicle) return;
	Vehicle->dir = message.popInt16();
	Vehicle->setBuildingType (message.popID ());
	Vehicle->BuildPath = message.popBool();
	Vehicle->bandPosition = message.popPosition();
}

void cClient::HandleNetMessage_GAME_EV_DO_START_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DO_START_WORK);

	const int iID = message.popInt32();

	cBuilding* building = getBuildingFromID (iID);
	if (building == NULL)
	{
		Log.write (" Client: Can't start work of building: Unknown building with id: " + iToStr (iID), cLog::eLOG_TYPE_NET_ERROR);
		// TODO: Request sync of building
		return;
	}
	building->clientStartWork ();

	unitStartedWorking (*building);
}

void cClient::HandleNetMessage_GAME_EV_DO_STOP_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DO_STOP_WORK);

	const int iID = message.popInt32();
	cBuilding* building = getBuildingFromID (iID);
	if (building == NULL)
	{
		Log.write (" Client: Can't stop work of building: Unknown building with id: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of building
		return;
	}
	building->clientStopWork ();

	unitStoppedWorking (*building);
}

void cClient::HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MOVE_JOB_SERVER);

	const int iVehicleID = message.popInt32();
	const auto srcPosition = message.popPosition ();
	const auto destPosition = message.popPosition ();
	const int iSavedSpeed = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iVehicleID);

	if (Vehicle == NULL)
	{
		Log.write (" Client: Can't find vehicle with id " + iToStr (iVehicleID) + " for movejob from " + iToStr (srcPosition.x()) + "x" + iToStr (srcPosition.y()) + " to " + iToStr (destPosition.x()) + "x" + iToStr (destPosition.y()), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: request sync of vehicle
		return;
	}

	cClientMoveJob* MoveJob = new cClientMoveJob(*this, srcPosition, destPosition, Vehicle);
	MoveJob->iSavedSpeed = iSavedSpeed;
	if (!MoveJob->generateFromMessage (message)) return;
	Log.write (" Client: Added received movejob at time " + iToStr (gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);

	moveJobCreated (*Vehicle);
}

void cClient::HandleNetMessage_GAME_EV_NEXT_MOVE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NEXT_MOVE);

	const int iID = message.popInt16();
	const int iType = message.popChar();
	int iSavedSpeed = -1;
	if (iType == MJOB_STOP) iSavedSpeed = message.popChar();

	Log.write (" Client: Received information for next move: ID: " + iToStr (iID) + ", Type: " + iToStr (iType) + ", Time: " + iToStr (gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);

	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle && Vehicle->ClientMoveJob)
	{
		Vehicle->ClientMoveJob->handleNextMove (iType, iSavedSpeed);
	}
	else
	{
		if (Vehicle == NULL) Log.write (" Client: Can't find vehicle with ID " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		else Log.write (" Client: Vehicle with ID " + iToStr (iID) + "has no movejob", cLog::eLOG_TYPE_NET_WARNING);
		// TODO: request sync of vehicle
	}
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_LOCK_TARGET (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_LOCK_TARGET);

	cClientAttackJob::lockTarget (*this, message);
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_FIRE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_FIRE);

	cClientAttackJob* job = new cClientAttackJob (this, message);
	attackJobs.push_back (job);
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_IMPACT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_IMPACT);

	const int id = message.popInt16();
	const int remainingHP = message.popInt16();
	const auto position = message.popPosition();
	cClientAttackJob::makeImpact (*this, position, remainingHP, id);
}

void cClient::HandleNetMessage_GAME_EV_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESOURCES);

	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		const auto position = message.popPosition();

		ActivePlayer->exploreResource (position);

		sResources& res = getMap ()->getResource (position);
		res.typ = (unsigned char) message.popInt16();
		res.value = (unsigned char) message.popInt16();
	}
}

void cClient::HandleNetMessage_GAME_EV_BUILD_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_BUILD_ANSWER);

	const bool bOK = message.popBool();
	const int iID = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle == NULL)
	{
		Log.write (" Client: Vehicle can't start building: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of vehicle
		return;
	}

	if (!bOK)
	{
		if (Vehicle->owner == ActivePlayer)
		{
			if (!Vehicle->BuildPath)
			{
				const string msgString = lngPack.i18n ("Text~Comp~Producing_Err");
				ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (msgString));
			}
			else if (Vehicle->bandPosition != Vehicle->getPosition())
			{
				const string msgString = lngPack.i18n ("Text~Comp~Path_interrupted");
				ActivePlayer->addSavedReport (std::make_unique<cSavedReportUnit> (*Vehicle, msgString));
			}
		}
		Vehicle->setBuildTurns(0);
		Vehicle->BuildPath = false;
		Vehicle->bandPosition = cPosition(0,0);
		return;
	}

	if (Vehicle->isUnitBuildingABuilding ()) Log.write (" Client: Vehicle is already building", cLog::eLOG_TYPE_NET_ERROR);

	const auto buildPosition = message.popPosition ();
	const bool buildBig = message.popBool();
	const auto oldPosition = Vehicle->getPosition();

	if (buildBig)
	{
		getMap()->moveVehicleBig(*Vehicle, buildPosition);
		Vehicle->owner->doScan();

		Vehicle->BigBetonAlpha = 10;
	}
	else
	{
		getMap()->moveVehicle(*Vehicle, buildPosition);
		Vehicle->owner->doScan();
	}

	if (Vehicle->owner == ActivePlayer)
	{
		Vehicle->setBuildingType (message.popID());
		Vehicle->setBuildTurns (message.popInt16 ());
		Vehicle->BuildPath = message.popBool();
		Vehicle->bandPosition = message.popPosition();
	}

	Vehicle->setBuildingABuilding(true);
	addJob (new cStartBuildJob (*Vehicle, oldPosition, buildBig));

	unitStartedBuilding (*Vehicle);

	if (Vehicle->ClientMoveJob) Vehicle->ClientMoveJob->release();
}


void cClient::HandleNetMessage_GAME_EV_STOP_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_STOP_BUILD);

	const int iID = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle == NULL)
	{
		Log.write (" Client: Can't stop building: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of vehicle
		return;
	}

	const auto newPosition = message.popPosition();

	if (Vehicle->data.isBig)
	{
		getMap()->moveVehicle (*Vehicle, newPosition);
		Vehicle->owner->doScan();
	}

	Vehicle->setBuildingABuilding(false);
	Vehicle->BuildPath = false;

	unitStoppedBuilding (*Vehicle);
}

void cClient::HandleNetMessage_GAME_EV_SUBBASE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SUBBASE_VALUES);

	const int iID = message.popInt16();
	sSubBase* subBase = getSubBaseFromID (iID);
	if (subBase == NULL)
	{
		Log.write (" Client: Can't add subbase values: Unknown subbase with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of subbases
		return;
	}

	subBase->popFrom (message);

	// temporary debug check
	if (subBase->isDitributionMaximized() == false)
	{
		Log.write (" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}
}

void cClient::HandleNetMessage_GAME_EV_BUILDLIST (cNetMessage& message)
{
	assert (message.iType == GAME_EV_BUILDLIST);

	const int iID = message.popInt16();
	cBuilding* Building = getBuildingFromID (iID);
	if (Building == NULL)
	{
		Log.write (" Client: Can't set buildlist: Unknown building with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of building
		return;
	}

	Building->BuildList.clear();
	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		sBuildList BuildListItem;
		BuildListItem.type = message.popID();
		BuildListItem.metall_remaining = message.popInt16();
		Building->BuildList.push_back (BuildListItem);
	}

	Building->MetalPerRound = message.popInt16();
	Building->BuildSpeed = message.popInt16();
	Building->RepeatBuild = message.popBool();
}

void cClient::HandleNetMessage_GAME_EV_MINE_PRODUCE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MINE_PRODUCE_VALUES);

	const int iID = message.popInt16();
	cBuilding* Building = getBuildingFromID (iID);
	if (Building == NULL)
	{
		Log.write (" Client: Can't set produce values of building: Unknown building with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of building
		return;
	}

	Building->MaxMetalProd = message.popInt16();
	Building->MaxOilProd = message.popInt16();
	Building->MaxGoldProd = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_TURN_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_TURN_REPORT);

	string sReportMsg = "";
	int iCount = 0;
	bool playVoice = false;

	const int iReportAnz = message.popInt16();
	for (int i = 0; i != iReportAnz; ++i)
	{
		const sID Type = message.popID();
		const int iAnz = message.popInt16();
		if (iCount) sReportMsg += ", ";
		iCount += iAnz;
		const string sTmp = iToStr (iAnz) + " " + Type.getUnitDataOriginalVersion()->name;
		sReportMsg += iAnz > 1 ? sTmp : Type.getUnitDataOriginalVersion()->name;
		if (Type.getUnitDataOriginalVersion()->surfacePosition == sUnitData::SURFACE_POS_GROUND) playVoice = true;
	}

	//FIXME: gameGUI - voices

	const int nrResearchAreasFinished = message.popChar();
	const bool bFinishedResearch = (nrResearchAreasFinished > 0);
	//if ((iCount == 0 || !playVoice) && !bFinishedResearch) PlayVoice (VoiceData.VOIStartNone);
	if (iCount == 1)
	{
		sReportMsg += " " + lngPack.i18n ("Text~Comp~Finished") + ".";
		//if (!bFinishedResearch && playVoice) PlayVoice (VoiceData.VOIStartOne);
	}
	else if (iCount > 1)
	{
		sReportMsg += " " + lngPack.i18n ("Text~Comp~Finished2") + ".";
		//if (!bFinishedResearch && playVoice) PlayVoice (VoiceData.VOIStartMore);
	}
	string researchMsgString = "";
	if (bFinishedResearch)
	{
		ActivePlayer->researchFinished = true;
		//PlayVoice (VoiceData.VOIResearchComplete);

		// build research finished string
		const string themeNames[8] =
		{
			lngPack.i18n ("Text~Others~Attack"),
			lngPack.i18n ("Text~Others~Shots"),
			lngPack.i18n ("Text~Others~Range"),
			lngPack.i18n ("Text~Others~Armor"),
			lngPack.i18n ("Text~Others~Hitpoints"),
			lngPack.i18n ("Text~Others~Speed"),
			lngPack.i18n ("Text~Others~Scan"),
			lngPack.i18n ("Text~Others~Costs")
		};

		researchMsgString = lngPack.i18n ("Text~Others~Research") + " " + lngPack.i18n ("Text~Comp~Finished") + ": ";
		for (int i = 0; i < nrResearchAreasFinished; i++)
		{
			const int area = message.popChar();
			if (0 <= area && area < 8)
			{
				researchMsgString += themeNames[area];
				if (i + 1 < nrResearchAreasFinished) researchMsgString += ", ";
			}
		}
	}

	// Save the report
	string msgString = lngPack.i18n ("Text~Comp~Turn_Start") + " " + iToStr (iTurn);
	if (sReportMsg.length () > 0) msgString += "\n" + sReportMsg;
	if (bFinishedResearch) msgString += "\n" + researchMsgString;
	ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (msgString));
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
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.setAmmo(message.popInt16());
		else DestVehicle->data.setHitpoints(message.popInt16());
		if (DestVehicle->isUnitLoaded ())
		{
			// get the building which has loaded the unit
			cBuilding* Building = DestVehicle->owner->BuildingList;
			for (; Building; Building = Building->next)
			{
				if (Contains (Building->storedUnits, DestVehicle)) break;
			}
		}
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
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.setAmmo(message.popInt16());
		else DestBuilding->data.setHitpoints(message.popInt16());
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
	cBuilding* rubble = new cBuilding (NULL, NULL, ID);
	push_front_into_intrusivelist (neutralBuildings, *rubble);

	rubble->data.isBig = big;
	rubble->RubbleTyp = typ;
	rubble->RubbleValue = value;
	const auto position = message.popPosition ();

	rubble->setPosition (position);

	getMap()->addBuilding (*rubble, rubble->getPosition());
}

void cClient::HandleNetMessage_GAME_EV_DETECTION_STATE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DETECTION_STATE);

	const int id = message.popInt32();
	cVehicle* vehicle = getVehicleFromID (id);
	if (vehicle == NULL)
	{
		Log.write (" Client: Vehicle (ID: " + iToStr (id) + ") not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	const bool detected = message.popBool();
	if (detected)
	{
		//mark vehicle as detected with size of detectedByPlayerList > 0
		vehicle->detectedByPlayerList.push_back (NULL);
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
			if (Vehicle == NULL)
			{
				Log.write ("Client: Can not find vehicle with id " + iToStr (id) + " for clearing", LOG_TYPE_NET_WARNING);
				break;
			}
			const auto orgiginalPosition = Vehicle->getPosition();

			Vehicle->setClearingTurns(message.popInt16());
			const auto bigPosition = message.popPosition ();
			if (bigPosition.x () >= 0 && bigPosition.y() >= 0)
			{
				getMap()->moveVehicleBig (*Vehicle, bigPosition);
				Vehicle->owner->doScan();
			}
			Vehicle->setClearing(true);
			addJob (new cStartBuildJob (*Vehicle, orgiginalPosition, (bigPosition.x () >= 0 && bigPosition.y () >= 0)));

			unitStartedClearing (*Vehicle);
		}
		break;
		case 1:
			// TODO: add blocked message
			// gameGUI->addMessage ("blocked");
			break;
		case 2:
			Log.write ("Client: warning on start of clearing", LOG_TYPE_NET_WARNING);
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
	if (Vehicle == NULL)
	{
		Log.write ("Client: Can not find vehicle with id " + iToStr (id) + " for stop clearing", LOG_TYPE_NET_WARNING);
		return;
	}

	const auto bigPosition = message.popPosition ();
	if (bigPosition.x () >= 0 && bigPosition.y () >= 0)
	{
		getMap()->moveVehicle(*Vehicle, bigPosition);
		Vehicle->owner->doScan();
	}
	Vehicle->setClearing(false);
	Vehicle->setClearingTurns(0);

	unitStoppedClearing (*Vehicle);
}

void cClient::HandleNetMessage_GAME_EV_NOFOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NOFOG);

	ActivePlayer->revealMap();
}

void cClient::HandleNetMessage_GAME_EV_DEFEATED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEFEATED);

	const int iTmp = message.popInt16();
	cPlayer* Player = getPlayerFromNumber (iTmp);
	if (Player == NULL)
	{
		Log.write ("Client: Cannot find defeated player!", LOG_TYPE_NET_WARNING);
		return;
	}
	Player->isDefeated = true;
	string msgString = lngPack.i18n ("Text~Multiplayer~Player") + " " + Player->getName () + " " + lngPack.i18n ("Text~Comp~Defeated");
	ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (msgString));
#if 0
	for (unsigned int i = 0; i < getPlayerList()->size(); i++)
	{
		if (Player == (*getPlayerList()) [i])
		{
			Hud.ExtraPlayers (Player->getName() + " (d)", Player->getColor(), i, Player->bFinishedTurn, false);
			return;
		}
	}
#endif
}

void cClient::HandleNetMessage_GAME_EV_FREEZE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FREEZE);

	const eFreezeMode mode = (eFreezeMode) message.popInt16();
	const int playerNumber = message.popInt16();
	enableFreezeMode (mode, playerNumber);
}

void cClient::HandleNetMessage_GAME_EV_UNFREEZE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNFREEZE);

	eFreezeMode mode = (eFreezeMode) message.popInt16();
	disableFreezeMode (mode);
}

void cClient::HandleNetMessage_GAME_EV_DEL_PLAYER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_PLAYER);

	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	if (Player == ActivePlayer)
	{
		Log.write ("Client: Cannot delete own player!", LOG_TYPE_NET_WARNING);
		return;
	}
	if (Player->VehicleList || Player->BuildingList)
	{
		Log.write ("Client: Player to be deleted has some units left !", LOG_TYPE_NET_ERROR);
	}
	const string msgString = lngPack.i18n ("Text~Multiplayer~Player_Left", Player->getName());
	ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (msgString));

	deletePlayer (Player);
}

void cClient::HandleNetMessage_GAME_EV_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_TURN);

	iTurn = message.popInt16();
	iStartTurnTime = message.popInt32();
	iEndTurnTime = iStartTurnTime;
	turnChanged ();
}

void cClient::HandleNetMessage_GAME_EV_HUD_SETTINGS (cNetMessage& message)
{
	//FIXME: gameGUI
	//assert (message.iType == GAME_EV_HUD_SETTINGS);

	//const int unitID = message.popInt16();
	//cUnit* unit = getVehicleFromID (unitID);
	//if (!unit) unit = getBuildingFromID (unitID);

	//if (unit) gameGUI->selectUnit (*unit);

	//const int x = message.popInt16();
	//const int y = message.popInt16();
	//gameGUI->setOffsetPosition (x, y);
	//gameGUI->setZoom (message.popFloat(), true, false);
	//gameGUI->setColor (message.popBool());
	//gameGUI->setGrid (message.popBool());
	//gameGUI->setAmmo (message.popBool());
	//gameGUI->setFog (message.popBool());
	//gameGUI->setTwoX (message.popBool());
	//gameGUI->setRange (message.popBool());
	//gameGUI->setScan (message.popBool());
	//gameGUI->setStatus (message.popBool());
	//gameGUI->setSurvey (message.popBool());
	//gameGUI->setLock (message.popBool());
	//gameGUI->setHits (message.popBool());
	//gameGUI->setTNT (message.popBool());

	//gameGUI->setStartup (false);
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
		storingVehicle->storeVehicle (*storedVehicle, *getMap());
		unitStored (*storingVehicle, *storedVehicle);
	}
	else
	{
		cBuilding* storingBuilding = getBuildingFromID (message.popInt16());
		if (!storingBuilding) return;
		storingBuilding->storeVehicle (*storedVehicle, *getMap ());
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

		const auto position = message.popPosition ();
		StoringVehicle->exitVehicleTo (*StoredVehicle, position, *getMap ());
		unitActivated (*StoringVehicle, *StoredVehicle);
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		const auto position = message.popPosition ();
		StoringBuilding->exitVehicleTo (*StoredVehicle, position, *getMap ());
		unitActivated (*StoringBuilding, *StoredVehicle);
	}
}

void cClient::HandleNetMessage_GAME_EV_DELETE_EVERYTHING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DELETE_EVERYTHING);

	for (unsigned int i = 0; i < getPlayerList().size(); i++)
	{
		cPlayer& Player = *getPlayerList() [i];

		for (cVehicle* vehicle = Player.VehicleList; vehicle; vehicle = vehicle->next)
		{
			vehicle->deleteStoredUnits();
		}
		for (cBuilding* building = Player.BuildingList; building; building = building->next)
		{
			building->deleteStoredUnits();
		}

		while (Player.VehicleList)
		{
			deleteUnit (Player.VehicleList);
		}
		while (Player.BuildingList)
		{
			deleteUnit (Player.BuildingList);
		}
	}

	//delete subbases
	// TODO: check that each subbase is deleted
	ActivePlayer->base.SubBases.clear();

	while (neutralBuildings)
	{
		cBuilding* nextBuilding = neutralBuildings->next;
		getMap()->deleteBuilding (*neutralBuildings);
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}

	//delete attack jobs
	for (size_t i = 0; i != attackJobs.size(); ++i)
	{
		delete attackJobs[i];
	}
	attackJobs.clear();

	//TODO: delete fx effects???

	// delete all eventually remaining pointers on the map, to prevent crashes after a resync.
	// Normally there shouldn't be any pointers left after deleting all units, but a resync is not
	// executed in normal situations and there are situations, when this happens.
	getMap()->reset();
}

void cClient::HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_UPGRADE_VALUES);

	const sID ID = message.popID();
	sUnitData* Data = ActivePlayer->getUnitDataCurrentVersion (ID);
	if (Data == NULL) return;

	Data->setVersion(message.popInt16());
	Data->scan = message.popInt16();
	Data->range = message.popInt16();
	Data->damage = message.popInt16();
	Data->buildCosts = message.popInt16();
	Data->armor = message.popInt16();
	Data->speedMax = message.popInt16();
	Data->shotsMax = message.popInt16();
	Data->ammoMax = message.popInt16();
	Data->hitpointsMax = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_CREDITS_CHANGED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CREDITS_CHANGED);

	ActivePlayer->Credits = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UPGRADED_BUILDINGS);

	const int buildingsInMsg = message.popInt16();
	const int totalCosts = message.popInt16();
	if (buildingsInMsg <= 0) return;

	string buildingName;
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
		const sUnitData& upgraded = *ActivePlayer->getUnitDataCurrentVersion (building->data.ID);
		if (building->data.scan < upgraded.scan)
			scanNecessary = true; // Scan range was upgraded. So trigger a scan.
		building->upgradeToCurrentVersion();
		if (i == 0)
		{
			buildingName = building->data.name;
		}
	}
	ostringstream os;
	os << lngPack.i18n ("Text~Comp~Upgrades_Done") << " " << buildingsInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", buildingName) << " (" << lngPack.i18n ("Text~Others~Costs") << ": " << totalCosts << ")";
	string sTmp (os.str());
	ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (sTmp));
	if (scanNecessary)
		ActivePlayer->doScan();
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UPGRADED_VEHICLES);

	const int vehiclesInMsg = message.popInt16();
	const int totalCosts = message.popInt16();
	const unsigned int storingBuildingID = message.popInt32();
	if (vehiclesInMsg <= 0) return;

	string vehicleName;
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
			vehicleName = vehicle->data.name;
		}
	}
	ostringstream os;
	os << lngPack.i18n ("Text~Comp~Upgrades_Done") << " " << vehiclesInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", vehicleName) << " (" << lngPack.i18n ("Text~Others~Costs") << ": " << totalCosts << ")";

	string printStr (os.str());
	ActivePlayer->addSavedReport (std::make_unique<cSavedReportSimple> (printStr));
}

void cClient::HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESEARCH_SETTINGS);

	const int buildingsInMsg = message.popInt16();
	for (int i = 0; i < buildingsInMsg; ++i)
	{
		const int buildingID = message.popInt32();
		const int newArea = message.popChar();
		cBuilding* building = getBuildingFromID (buildingID);
		if (building && building->data.canResearch && 0 <= newArea && newArea <= cResearch::kNrResearchAreas)
			building->researchArea = newArea;
	}
	// now update the research center count for the areas
	ActivePlayer->refreshResearchCentersWorkingOnArea();
}

void cClient::HandleNetMessage_GAME_EV_RESEARCH_LEVEL (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESEARCH_LEVEL);

	for (int area = cResearch::kNrResearchAreas - 1; area >= 0; area--)
	{
		const int newCurPoints = message.popInt16();
		const int newLevel = message.popInt16();
		ActivePlayer->researchLevel.setCurResearchLevel (newLevel, area);
		ActivePlayer->researchLevel.setCurResearchPoints (newCurPoints, area);
	}
}

void cClient::HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REFRESH_RESEARCH_COUNT);

	ActivePlayer->refreshResearchCentersWorkingOnArea();
}

void cClient::HandleNetMessage_GAME_EV_SET_AUTOMOVE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SET_AUTOMOVE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (Vehicle)
	{
		delete Vehicle->autoMJob;
		Vehicle->autoMJob = new cAutoMJob (*this, Vehicle);
	}
}

void cClient::HandleNetMessage_GAME_EV_COMMANDO_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_COMMANDO_ANSWER);

	bool success = message.popBool ();
	bool steal = false;
	if (success) steal = message.popBool ();
	cVehicle* vehicle = getVehicleFromID (message.popInt16 ());

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

void cClient::HandleNetMessage_GAME_EV_REQ_SAVE_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQ_SAVE_INFO);

	const int saveingID = message.popInt16();
	//FIXME: gameGUI
	//if (gameGUI->getSelectedUnit()) sendSaveHudInfo (*this, gameGUI->getSelectedUnit()->iID, ActivePlayer->getNr(), saveingID);
	//else sendSaveHudInfo (*this, -1, ActivePlayer->getNr(), saveingID);

	const auto& savedReports = ActivePlayer->savedReportsList;
	for (size_t i = std::max<int> (0, savedReports.size() - 50); i != savedReports.size(); ++i)
	{
		sendSaveReportInfo (*this, *savedReports[i], ActivePlayer->getNr(), saveingID);
	}
	sendFinishedSendSaveInfo (*this, ActivePlayer->getNr(), saveingID);
}

void cClient::HandleNetMessage_GAME_EV_SAVED_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVED_REPORT);

	ActivePlayer->addSavedReport (cSavedReport::createFrom(message));
}

void cClient::HandleNetMessage_GAME_EV_CASUALTIES_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CASUALTIES_REPORT);

	casualtiesTracker->updateCasualtiesFromNetMessage (&message);
}

void cClient::HandleNetMessage_GAME_EV_SCORE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SCORE);

	const int pn = message.popInt16();
	const int turn = message.popInt16();
	const int n = message.popInt16();

	getPlayerFromNumber (pn)->setScore (n, turn);
}

void cClient::HandleNetMessage_GAME_EV_NUM_ECOS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NUM_ECOS);

	const int pn = message.popInt16();
	const int n = message.popInt16();

	getPlayerFromNumber (pn)->numEcos = n;
}

void cClient::HandleNetMessage_GAME_EV_UNIT_SCORE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_SCORE);

	cBuilding* b = getBuildingFromID (message.popInt16());
	b->points = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_GAME_SETTINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_GAME_SETTINGS);

	if (message.popBool() == false)
	{
		gameSetting = NULL;
		return;
	}
	AutoPtr<cGameSettings> gameSetting_ (new cGameSettings());

	gameSetting_->popFrom (message);
	gameSetting = gameSetting_.Release();
}

void cClient::HandleNetMessage_GAME_EV_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building) return;

	destroyUnit (*building);
}

void cClient::HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_END_MOVE_ACTION_SERVER);

	cVehicle* vehicle = getVehicleFromID (message.popInt32());
	if (!vehicle || !vehicle->ClientMoveJob) return;

	const int destID = message.popInt32();
	eEndMoveActionType type = (eEndMoveActionType) message.popChar();
	vehicle->ClientMoveJob->endMoveAction = new cEndMoveAction (vehicle, destID, type);
}

void cClient::HandleNetMessage_GAME_EV_SET_GAME_TIME (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SET_GAME_TIME);

	const unsigned int newGameTime = message.popInt32();
	gameTimer.gameTime = newGameTime;

	//confirm new time to the server
	cNetMessage* response = new cNetMessage (NET_GAME_TIME_CLIENT);
	response->pushInt32 (gameTimer.gameTime);
	sendNetMessage (response);
}

void cClient::HandleNetMessage_GAME_EV_REVEAL_MAP (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REVEAL_MAP);

	ActivePlayer->revealMap ();
}

void cClient::handleNetMessages ()
{
	if (gameTimer.nextMsgIsNextGameTime) return;

	std::unique_ptr<cNetMessage> message;
	while (eventQueue.try_pop(message))
	{
		handleNetMessage (*message);
		if (gameTimer.nextMsgIsNextGameTime) break;
	}
}

int cClient::handleNetMessage (cNetMessage& message)
{
	if (message.iType != NET_GAME_TIME_SERVER)
	{
		Log.write ("Client: " + getActivePlayer ().getName () + " <-- "
				   + message.getTypeAsString ()
				   + ", gameTime:" + iToStr (this->gameTimer.gameTime)
				   + ", Hexdump: " + message.getHexDump (), cLog::eLOG_TYPE_NET_DEBUG);
	}

	switch (message.iType)
	{
		case TCP_ACCEPT:
			//should not happen
			break;
		case TCP_CLOSE: HandleNetMessage_TCP_CLOSE (message); break;
		case GAME_EV_PLAYER_CLANS: HandleNetMessage_GAME_EV_PLAYER_CLANS (message); break;
		case GAME_EV_ADD_BUILDING: HandleNetMessage_GAME_EV_ADD_BUILDING (message); break;
		case GAME_EV_ADD_VEHICLE: HandleNetMessage_GAME_EV_ADD_VEHICLE (message); break;
		case GAME_EV_DEL_BUILDING: HandleNetMessage_GAME_EV_DEL_BUILDING (message); break;
		case GAME_EV_DEL_VEHICLE: HandleNetMessage_GAME_EV_DEL_VEHICLE (message); break;
		case GAME_EV_ADD_ENEM_VEHICLE: HandleNetMessage_GAME_EV_ADD_ENEM_VEHICLE (message); break;
		case GAME_EV_ADD_ENEM_BUILDING: HandleNetMessage_GAME_EV_ADD_ENEM_BUILDING (message); break;
		case GAME_EV_WAIT_FOR: HandleNetMessage_GAME_EV_WAIT_FOR (message); break;
		case GAME_EV_MAKE_TURNEND: HandleNetMessage_GAME_EV_MAKE_TURNEND (message); break;
		case GAME_EV_FINISHED_TURN: HandleNetMessage_GAME_EV_FINISHED_TURN (message); break;
		case GAME_EV_UNIT_DATA: HandleNetMessage_GAME_EV_UNIT_DATA (message); break;
		case GAME_EV_SPECIFIC_UNIT_DATA: HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (message); break;
		case GAME_EV_DO_START_WORK: HandleNetMessage_GAME_EV_DO_START_WORK (message); break;
		case GAME_EV_DO_STOP_WORK: HandleNetMessage_GAME_EV_DO_STOP_WORK (message); break;
		case GAME_EV_MOVE_JOB_SERVER: HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (message); break;
		case GAME_EV_NEXT_MOVE: HandleNetMessage_GAME_EV_NEXT_MOVE (message); break;
		case GAME_EV_ATTACKJOB_LOCK_TARGET: HandleNetMessage_GAME_EV_ATTACKJOB_LOCK_TARGET (message); break;
		case GAME_EV_ATTACKJOB_FIRE: HandleNetMessage_GAME_EV_ATTACKJOB_FIRE (message); break;
		case GAME_EV_ATTACKJOB_IMPACT: HandleNetMessage_GAME_EV_ATTACKJOB_IMPACT (message); break;
		case GAME_EV_RESOURCES: HandleNetMessage_GAME_EV_RESOURCES (message); break;
		case GAME_EV_BUILD_ANSWER: HandleNetMessage_GAME_EV_BUILD_ANSWER (message); break;
		case GAME_EV_STOP_BUILD: HandleNetMessage_GAME_EV_STOP_BUILD (message); break;
		case GAME_EV_SUBBASE_VALUES: HandleNetMessage_GAME_EV_SUBBASE_VALUES (message); break;
		case GAME_EV_BUILDLIST: HandleNetMessage_GAME_EV_BUILDLIST (message); break;
		case GAME_EV_MINE_PRODUCE_VALUES: HandleNetMessage_GAME_EV_MINE_PRODUCE_VALUES (message); break;
		case GAME_EV_TURN_REPORT: HandleNetMessage_GAME_EV_TURN_REPORT (message); break;
		case GAME_EV_MARK_LOG: HandleNetMessage_GAME_EV_MARK_LOG (message); break;
		case GAME_EV_SUPPLY: HandleNetMessage_GAME_EV_SUPPLY (message); break;
		case GAME_EV_ADD_RUBBLE: HandleNetMessage_GAME_EV_ADD_RUBBLE (message); break;
		case GAME_EV_DETECTION_STATE: HandleNetMessage_GAME_EV_DETECTION_STATE (message); break;
		case GAME_EV_CLEAR_ANSWER: HandleNetMessage_GAME_EV_CLEAR_ANSWER (message); break;
		case GAME_EV_STOP_CLEARING: HandleNetMessage_GAME_EV_STOP_CLEARING (message); break;
		case GAME_EV_NOFOG: HandleNetMessage_GAME_EV_NOFOG (message); break;
		case GAME_EV_DEFEATED: HandleNetMessage_GAME_EV_DEFEATED (message); break;
		case GAME_EV_FREEZE: HandleNetMessage_GAME_EV_FREEZE (message); break;
		case GAME_EV_UNFREEZE: HandleNetMessage_GAME_EV_UNFREEZE (message); break;
		case GAME_EV_DEL_PLAYER: HandleNetMessage_GAME_EV_DEL_PLAYER (message); break;
		case GAME_EV_TURN: HandleNetMessage_GAME_EV_TURN (message); break;
		case GAME_EV_HUD_SETTINGS: HandleNetMessage_GAME_EV_HUD_SETTINGS (message); break;
		case GAME_EV_STORE_UNIT: HandleNetMessage_GAME_EV_STORE_UNIT (message); break;
		case GAME_EV_EXIT_UNIT: HandleNetMessage_GAME_EV_EXIT_UNIT (message); break;
		case GAME_EV_DELETE_EVERYTHING: HandleNetMessage_GAME_EV_DELETE_EVERYTHING (message); break;
		case GAME_EV_UNIT_UPGRADE_VALUES: HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (message); break;
		case GAME_EV_CREDITS_CHANGED: HandleNetMessage_GAME_EV_CREDITS_CHANGED (message); break;
		case GAME_EV_UPGRADED_BUILDINGS: HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (message); break;
		case GAME_EV_UPGRADED_VEHICLES: HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (message); break;
		case GAME_EV_RESEARCH_SETTINGS: HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (message); break;
		case GAME_EV_RESEARCH_LEVEL: HandleNetMessage_GAME_EV_RESEARCH_LEVEL (message); break;
		case GAME_EV_REFRESH_RESEARCH_COUNT: // sent, when the player was resynced (or a game was loaded)
			HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (message); break;
		case GAME_EV_SET_AUTOMOVE: HandleNetMessage_GAME_EV_SET_AUTOMOVE (message); break;
		case GAME_EV_COMMANDO_ANSWER: HandleNetMessage_GAME_EV_COMMANDO_ANSWER (message); break;
		case GAME_EV_REQ_SAVE_INFO: HandleNetMessage_GAME_EV_REQ_SAVE_INFO (message); break;
		case GAME_EV_SAVED_REPORT: HandleNetMessage_GAME_EV_SAVED_REPORT (message); break;
		case GAME_EV_CASUALTIES_REPORT: HandleNetMessage_GAME_EV_CASUALTIES_REPORT (message); break;
		case GAME_EV_SCORE: HandleNetMessage_GAME_EV_SCORE (message); break;
		case GAME_EV_NUM_ECOS: HandleNetMessage_GAME_EV_NUM_ECOS (message); break;
		case GAME_EV_UNIT_SCORE: HandleNetMessage_GAME_EV_UNIT_SCORE (message); break;
		case GAME_EV_GAME_SETTINGS: HandleNetMessage_GAME_EV_GAME_SETTINGS (message); break;
		case GAME_EV_SELFDESTROY: HandleNetMessage_GAME_EV_SELFDESTROY (message); break;
		case GAME_EV_END_MOVE_ACTION_SERVER: HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (message); break;
		case GAME_EV_SET_GAME_TIME: HandleNetMessage_GAME_EV_SET_GAME_TIME (message); break;
		case GAME_EV_REVEAL_MAP: HandleNetMessage_GAME_EV_REVEAL_MAP (message); break;
		case NET_GAME_TIME_SERVER: gameTimer.handleSyncMessage (message); break;

		default:
			Log.write ("Client: Can not handle message type " + message.getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}

	return 0;
}

void cClient::addUnit(const cPosition& position, cVehicle& addedVehicle, bool bInit, bool bAddToMap)
{
	// place the vehicle
	if (bAddToMap) getMap()->addVehicle (addedVehicle, position);

	if (!bInit) addedVehicle.StartUp = 10;

	if (addedVehicle.owner != ActivePlayer && addedVehicle.iID == ActivePlayer->lastDeletedUnit)
	{
		const std::string msg = lngPack.i18n ("Text~Comp~CapturedByEnemy", addedVehicle.getDisplayName ());
		ActivePlayer->addSavedReport (std::make_unique<cSavedReportUnit> (addedVehicle, msg));
		unitStolen (addedVehicle);
	}
	else if (addedVehicle.owner != ActivePlayer)
	{
		const string message = addedVehicle.getDisplayName () + " (" + addedVehicle.owner->getName () + ") " + lngPack.i18n ("Text~Comp~Detected");
		ActivePlayer->addSavedReport (std::make_unique<cSavedReportUnit> (addedVehicle, message));
		unitDetected (addedVehicle);
	}
}

void cClient::addUnit(const cPosition& position, cBuilding& addedBuilding, bool bInit)
{
	// place the building
	getMap()->addBuilding (addedBuilding, position);

	if (!bInit) addedBuilding.StartUp = 10;
}

cPlayer* cClient::getPlayerFromNumber (int iNum)
{
	for (unsigned int i = 0; i < getPlayerList().size(); i++)
	{
		cPlayer& p = *getPlayerList() [i];
		if (p.getNr() == iNum) return &p;
	}
	return NULL;
}

cPlayer* cClient::getPlayerFromString (const string& playerID)
{
	// first try to find player by number
	const int playerNr = atoi (playerID.c_str());
	if (playerNr != 0 || playerID[0] == '0')
	{
		return getPlayerFromNumber (playerNr);
	}

	// try to find plyer by name
	for (unsigned int i = 0; i < playerList.size (); i++)
	{
		if (playerList[i]->getName () == playerID) return playerList[i].get();
	}
	return NULL;
}

void cClient::deleteUnit (cBuilding* Building)
{
	if (!Building) return;

	//if (activeMenu) activeMenu->handleDestroyUnit (*Building);
	getMap()->deleteBuilding (*Building);

	if (!Building->owner)
	{
		remove_from_intrusivelist (neutralBuildings, *Building);
		delete Building;
		return;
	}

	for (unsigned int i = 0; i < attackJobs.size(); i++)
	{
		attackJobs[i]->onRemoveUnit (*Building);
	}
	remove_from_intrusivelist (Building->owner->BuildingList, *Building);

	if (Building->owner == ActivePlayer)
		Building->owner->base.deleteBuilding (Building, NULL);

	cPlayer* owner = Building->owner;
	delete Building;

	owner->doScan();
}

void cClient::deleteUnit (cVehicle* Vehicle)
{
	if (!Vehicle) return;

	//if (activeMenu) activeMenu->handleDestroyUnit (*Vehicle);
	getMap()->deleteVehicle (*Vehicle);

	for (unsigned int i = 0; i < attackJobs.size(); i++)
	{
		attackJobs[i]->onRemoveUnit (*Vehicle);
	}
	helperJobs.onRemoveUnit (Vehicle);

	cPlayer* owner = Vehicle->owner;
	remove_from_intrusivelist (Vehicle->owner->VehicleList, *Vehicle);

	owner->lastDeletedUnit = Vehicle->iID;

	delete Vehicle;

	owner->doScan();
}

void cClient::handleEnd()
{
	if (isFreezed()) return;
	bWantToEnd = true;
	sendWantToEndTurn (*this);
	startedTurnEndProcess ();
}

unsigned int cClient::getRemainingTimeInSecond() const
{
	if (iEndTurnTime <= iStartTurnTime) return ~0u; // max value
	return (iEndTurnTime - gameTimer.gameTime) / 100;
}

unsigned int cClient::getElapsedTimeInSecond() const
{
	if (gameTimer.gameTime <= iStartTurnTime) return 0;
	return (gameTimer.gameTime - iStartTurnTime) / 100;
}

void cClient::handleTurnTime()
{
	if (iEndTurnTime > iStartTurnTime)
	{
		//FIXME: gameGUI
		//gameGUI->updateTurnTime (getRemainingTimeInSecond());
	}
	else
	{
		//FIXME: gameGUI
		//gameGUI->updateTurnTime (getElapsedTimeInSecond());
	}
}

void cClient::addActiveMoveJob (cClientMoveJob& MoveJob)
{
	MoveJob.bSuspended = false;
	if (MoveJob.Vehicle) MoveJob.Vehicle->MoveJobActive = true;
	if (Contains (ActiveMJobs, &MoveJob)) return;
	ActiveMJobs.push_back (&MoveJob);
}

void cClient::handleMoveJobs()
{
	for (int i = ActiveMJobs.size() - 1; i >= 0; i--)
	{
		cClientMoveJob* MoveJob = ActiveMJobs[i];
		cVehicle* Vehicle = MoveJob->Vehicle;

		// suspend movejobs of attacked vehicles
		if (Vehicle && Vehicle->isBeeingAttacked ()) continue;

		if (MoveJob->bFinished || MoveJob->bEndForNow)
		{
			MoveJob->stopped(*Vehicle);
		}

		if (MoveJob->bFinished)
		{
			if (Vehicle && Vehicle->ClientMoveJob == MoveJob)
			{
				Log.write (" Client: Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
				Vehicle->ClientMoveJob = NULL;
				Vehicle->moving = false;
				Vehicle->MoveJobActive = false;
			}
			else Log.write (" Client: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
			ActiveMJobs.erase (ActiveMJobs.begin() + i);
			delete MoveJob;
			unitFinishedMoveJob (*Vehicle);
			continue;
		}
		if (MoveJob->bEndForNow)
		{
			Log.write (" Client: Movejob has end for now and will be stopped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle)
			{
				Vehicle->MoveJobActive = false;
				Vehicle->moving = false;
			}
			ActiveMJobs.erase (ActiveMJobs.begin () + i);
			unitPausedMoveJob (*Vehicle);
			continue;
		}

		if (Vehicle == NULL) continue;


		if (MoveJob->iNextDir != Vehicle->dir && Vehicle->data.speedCur)
		{
			// rotate vehicle
			if (gameTimer.timer100ms)
			{
				Vehicle->rotateTo (MoveJob->iNextDir);
			}
		}
		else if (Vehicle->MoveJobActive)
		{
			// move vehicle
			if (gameTimer.timer10ms)
			{
				MoveJob->moveVehicle();
			}
		}
	}
}

cVehicle* cClient::getVehicleFromID (unsigned int iID)
{
	for (unsigned int i = 0; i < getPlayerList().size(); i++)
	{
		cPlayer& player = *getPlayerList() [i];
		for (cVehicle* vehicle = player.VehicleList; vehicle; vehicle = vehicle->next)
		{
			if (vehicle->iID == iID) return vehicle;
		}
	}
	return NULL;
}

cBuilding* cClient::getBuildingFromID (unsigned int iID)
{
	for (unsigned int i = 0; i < getPlayerList().size(); i++)
	{
		cPlayer& player = *getPlayerList() [i];

		for (cBuilding* building = player.BuildingList; building; building = building->next)
		{
			if (building->iID == iID) return building;
		}
	}
	for (cBuilding* building = neutralBuildings; building; building = building->next)
	{
		if (building->iID == iID) return building;
	}
	return NULL;
}

void cClient::doGameActions ()
{
	runFx();

	// run attackJobs
	if (gameTimer.timer50ms)
		cClientAttackJob::handleAttackJobs (*this);

	// run moveJobs - this has to be called before handling the auto movejobs
	handleMoveJobs();

	// run surveyor ai
	if (gameTimer.timer50ms)
		cAutoMJob::handleAutoMoveJobs();

	runJobs();
}

sSubBase* cClient::getSubBaseFromID (int iID)
{
	cBuilding* building = getBuildingFromID (iID);
	if (building)
		return building->SubBase;
	return NULL;
}

void cClient::destroyUnit (cVehicle& vehicle)
{
	// play explosion
	if (vehicle.data.isBig)
	{
		addFx (std::make_shared<cFxExploBig> (vehicle.getPosition() * 64 + 64, getMap ()->isWaterOrCoast (vehicle.getPosition())));
	}
	else if (vehicle.data.factorAir > 0 && vehicle.FlightHigh != 0)
	{
		addFx(std::make_shared<cFxExploAir>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
	else if (getMap()->isWaterOrCoast (vehicle.getPosition()))
	{
		addFx (std::make_shared<cFxExploWater> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
	else
	{
		addFx (std::make_shared<cFxExploSmall> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}

	if (vehicle.data.hasCorpse)
	{
		// add corpse
		addFx (std::make_shared<cFxCorpse> (vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
	}
}

void cClient::destroyUnit (cBuilding& building)
{
	// play explosion animation
	cBuilding* topBuilding = getMap()->getField(building.getPosition()).getBuilding();
	if (topBuilding && topBuilding->data.isBig)
	{
		addFx (std::make_shared<cFxExploBig> (topBuilding->getPosition() * 64 + 64, getMap ()->isWaterOrCoast (topBuilding->getPosition())));
	}
	else
	{
		addFx (std::make_shared<cFxExploSmall> (building.getPosition() * 64 + 32));
	}
}
//------------------------------------------------------------------------------
int cClient::getTurn() const
{
	return iTurn;
}

//------------------------------------------------------------------------------
void cClient::deletePlayer (cPlayer* player)
{
	player->isRemovedFromGame = true;

	// TODO: We do not really delete the player
	// because he may is still referenced somewhere
	// (e.g. in the playersInfo in the gameGUI)
	// or we may need him for some statistics.
	// uncomment this if we can make sure all references have been removed or
	// at least been set to NULL.
#if 0
	for (unsigned int i = 0; i < getPlayerList()->size(); i++)
	{
		if (player == (*getPlayerList()) [i])
		{
			delete (*getPlayerList()) [i];
			getPlayerList()->Delete (i);
		}
	}
#endif
}

void cClient::addJob (cJob* job)
{
	helperJobs.addJob (*job);
}

void cClient::runJobs()
{
	helperJobs.run (gameTimer);
}

void cClient::enableFreezeMode (eFreezeMode mode, int playerNumber)
{
	freezeModes.enable (mode, playerNumber);
	freezeModeChanged ();
}

void cClient::disableFreezeMode (eFreezeMode mode)
{
	freezeModes.disable (mode);
	freezeModeChanged ();
}

bool cClient::isFreezed() const
{
	return freezeModes.isFreezed();
}

bool cClient::getFreezeMode (eFreezeMode mode) const
{
	return freezeModes.isEnable (mode);
}

void cClient::handleChatMessage (const std::string& message)
{
    if (message.empty ()) return;

    sendChatMessageToServer (*this, getActivePlayer (), message);
}