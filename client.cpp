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
#include "dialog.h"
#include "events.h"
#include "fxeffects.h"
#include "gametimer.h"
#include "hud.h"
#include "jobs.h"
#include "log.h"
#include "main.h"
#include "menus.h"
#include "mouse.h"
#include "netmessage.h"
#include "player.h"
#include "server.h"
#include "serverevents.h"
#include "settings.h"
#include "vehicles.h"

using namespace std;

sMessage::sMessage (const std::string& s, unsigned int const age_)
{
	chars = (int) s.length();
	msg = new char[chars + 1];
	strcpy (msg, s.c_str());
	if (chars > 500) msg[500] = '\0';
	len = font->getTextWide (s);
	age = age_;
}

sMessage::~sMessage()
{
	delete [] msg;
}

//------------------------------------------------------------------------
// cClient implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cClient::cClient (cServer* server_, cTCP* network_, cEventHandling& eventHandling_, cStaticMap& staticMap, std::vector<cPlayer*>* const playerList) :
	server (server_),
	network (network_),
	eventHandling (&eventHandling_),
	Map (new cMap (staticMap)),
	PlayerList (playerList),
	gameGUI (new cGameGUI ()),
	gameTimer(),
	FxList (new cFxContainer)
{
	gameTimer.setClient (this);
	if (server) server->setLocalClient (*this);
	else network->setMessageReceiver (this);
	neutralBuildings = NULL;
	bDefeated = false;
	iTurn = 1;
	bWantToEnd = false;
	iEndTurnTime = 0;
	iStartTurnTime = 0;
	scoreLimit = 0;
	turnLimit = 0;

	casualtiesTracker = new cCasualtiesTracker();

	gameTimer.start();
	for (unsigned int i = 0; i < PlayerList->size(); i++)
	{
		(*PlayerList) [i]->initMaps (*Map);
	}
}

cClient::~cClient()
{
	gameTimer.stop();

	delete casualtiesTracker;

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
	delete Map;
}

/*virtual*/ void cClient::pushEvent (cNetMessage* message)
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
	eventHandling->pushEvent (message);
}

void cClient::sendNetMessage (cNetMessage* message) const
{
	message->iPlayerNr = ActivePlayer->getNr();

	if (message->iType != NET_GAME_TIME_CLIENT)
		Log.write ("Client: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	if (server)
	{
		// push an event to the local server in singleplayer, HotSeat or
		// if this machine is the host
		server->pushEvent (message);
	}
	else // else send it over the net
	{
		//the client is only connected to one socket
		//so network->send() only sends to the server
		network->send (message->iLength, message->serialize());
		delete message;
	}
}

void cClient::initPlayer (cPlayer* Player)
{
	ActivePlayer = Player;
	gameGUI->setClient (this);

	// generate subbase for enemy players
	for (unsigned int i = 0; i < PlayerList->size(); i++)
	{
		if ( (*PlayerList) [i] == ActivePlayer) continue;
		(*PlayerList) [i]->base.SubBases.push_back (new sSubBase ( (*PlayerList) [i]));
	}
}

int cClient::addMoveJob (cVehicle* vehicle, int DestX, int DestY, const std::vector<cVehicle*>* group)
{
	sWaypoint* path = cClientMoveJob::calcPath (*getMap(), vehicle->PosX, vehicle->PosY, DestX, DestY, *vehicle, group);
	if (path)
	{
		sendMoveJob (*this, path, vehicle->iID);
		Log.write (" Client: Added new movejob: VehicleID: " + iToStr (vehicle->iID) + ", SrcX: " + iToStr (vehicle->PosX) + ", SrcY: " + iToStr (vehicle->PosY) + ", DestX: " + iToStr (DestX) + ", DestY: " + iToStr (DestY), cLog::eLOG_TYPE_NET_DEBUG);
		return 1;
	}
	else
	{
		// automoving surveyors must not tell this
		if (!vehicle->autoMJob)
		{
			PlayRandomVoice (VoiceData.VOINoPath);
		}
		return 0;
	}
}

void cClient::startGroupMove (const std::vector<cVehicle*>& group_, int mainDestX, int mainDestY)
{
	const int mainPosX = group_[0]->PosX;
	const int mainPosY = group_[0]->PosY;

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
			const int deltaX = vehicle->PosX - mainDestX + vehicle->PosX - mainPosX;
			const int deltaY = vehicle->PosY - mainDestY + vehicle->PosY - mainPosY;
			const int waySquareLength = Square (deltaX) + Square (deltaY);

			if (waySquareLength < shortestWaySquareLength)
			{
				shortestWaySquareLength = waySquareLength;
				shortestWayVehNum = i;
			}
		}
		cVehicle* vehicle = group[shortestWayVehNum];
		// add the movejob to the destination of the unit.
		// the formation of the vehicle group will stay as destination formation.
		const int destX = mainDestX + vehicle->PosX - mainPosX;
		const int destY = mainDestY + vehicle->PosY - mainPosY;
		addMoveJob (vehicle, destX, destY, &group_);
		// delete the unit from the copyed list
		group.erase (group.begin() + shortestWayVehNum);
	}
}

void cClient::runFx()
{
	FxList->run();
}

void cClient::addFx (cFx* fx)
{
	FxList->push_back (fx);
	fx->playSound (*gameGUI);
}

void cClient::HandleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	network->close (message.popInt16());
	const string msgString = lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server");
	gameGUI->addMessage (msgString);
	ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
	//TODO: ask user for reconnect
}

void cClient::HandleNetMessage_GAME_EV_CHAT_SERVER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CHAT_SERVER);

	switch (message.popChar())
	{
		case USER_MESSAGE:
		{
			PlayFX (SoundData.SNDChat);
			const string msgString = message.popString();
			gameGUI->addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_CHAT);
			break;
		}
		case SERVER_ERROR_MESSAGE:
		{
			PlayFX (SoundData.SNDQuitsch);
			const string msgString = lngPack.i18n (message.popString());
			gameGUI->addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
			break;
		}
		case SERVER_INFO_MESSAGE:
		{
			const string translationpath = message.popString();
			const string inserttext = message.popString();
			string msgString;
			if (!inserttext.compare ("")) msgString = lngPack.i18n (translationpath);
			else msgString = lngPack.i18n (translationpath, inserttext);
			gameGUI->addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
			break;
		}
	}
}

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
	const int PosY = message.popInt16();
	const int PosX = message.popInt16();
	unsigned int ID = message.popInt16();
	cBuilding* AddedBuilding = Player->addBuilding (PosX, PosY, *UnitID.getBuilding (Player), ID);

	addUnit (PosX, PosY, AddedBuilding, Init);

	Player->base.addBuilding (AddedBuilding, NULL);

	// play placesound if it is a mine
	if (UnitID == UnitsData.specialIDLandMine && Player == ActivePlayer) PlayFX (SoundData.SNDLandMinePlace);
	else if (UnitID == UnitsData.specialIDSeaMine && Player == ActivePlayer) PlayFX (SoundData.SNDSeaMinePlace);
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
	const sID UnitID = message.popID();
	const int PosY = message.popInt16();
	const int PosX = message.popInt16();
	const unsigned int ID = message.popInt16();
	const bool bAddToMap = message.popBool();

	cVehicle* AddedVehicle = Player->addVehicle (PosX, PosY, *UnitID.getVehicle (Player), ID);
	addUnit (PosX, PosY, AddedVehicle, Init, bAddToMap);
}

void cClient::HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message, cMenu* activeMenu)
{
	assert (message.iType == GAME_EV_DEL_BUILDING);

	cBuilding* Building = getBuildingFromID (message.popInt16());

	if (Building)
	{
		// play clearsound if it is a mine
		if (Building->owner && Building->data.ID == UnitsData.specialIDLandMine && Building->owner == ActivePlayer) PlayFX (SoundData.SNDLandMineClear);
		else if (Building->owner && Building->data.ID == UnitsData.specialIDSeaMine && Building->owner == ActivePlayer) PlayFX (SoundData.SNDSeaMineClear);

		deleteUnit (Building, activeMenu);
	}
}

void cClient::HandleNetMessage_GAME_EV_DEL_VEHICLE (cNetMessage& message, cMenu* activeMenu)
{
	assert (message.iType == GAME_EV_DEL_VEHICLE);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());

	if (Vehicle) deleteUnit (Vehicle, activeMenu);
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
	const sID UnitID = message.popID();
	const int iPosY = message.popInt16();
	const int iPosX = message.popInt16();
	const int dir = message.popInt16();
	const int ID = message.popInt16();
	const int version = message.popInt16();
	cVehicle* AddedVehicle = Player->addVehicle (iPosX, iPosY, *UnitID.getVehicle (Player), ID);

	AddedVehicle->dir = dir;
	AddedVehicle->data.version = version;
	addUnit (iPosX, iPosY, AddedVehicle, false);
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
	const sID UnitID = message.popID();
	const int iPosY = message.popInt16();
	const int iPosX = message.popInt16();
	const int ID = message.popInt16();
	const int version = message.popInt16();
	cBuilding* AddedBuilding = Player->addBuilding (iPosX, iPosY, *UnitID.getBuilding (Player), ID);

	AddedBuilding->data.version = version;
	addUnit (iPosX, iPosY, AddedBuilding, false);

	if (AddedBuilding->data.connectsToBase)
	{
		Player->base.SubBases[0]->buildings.push_back (AddedBuilding);
		AddedBuilding->SubBase = Player->base.SubBases[0];

		AddedBuilding->updateNeighbours (getMap());
	}
}

void cClient::HandleNetMessage_GAME_EV_WAIT_FOR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WAIT_FOR);

	const int nextPlayerNum = message.popInt16();

	if (nextPlayerNum != ActivePlayer->getNr())
	{
		enableFreezeMode (FREEZE_WAIT_FOR_OTHERS, nextPlayerNum);
		gameGUI->setEndButtonLock (true);
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
		gameGUI->updateTurn (iTurn);
		if (!bWaitForNextPlayer) gameGUI->setEndButtonLock (false);
		bWantToEnd = false;
		gameGUI->updateTurnTime (-1);
		ActivePlayer->clearDone();
		Log.write ("######### Round " + iToStr (iTurn) + " ###########", cLog::eLOG_TYPE_NET_DEBUG);
		for (unsigned int i = 0; i < getPlayerList().size(); i++)
		{
			getPlayerList() [i]->bFinishedTurn = false;
		}
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
			gameGUI->setEndButtonLock (false);
		}
	}
	else if (iNextPlayerNum != -1)
	{
		makeHotSeatEnd (iNextPlayerNum);
	}
}

void cClient::HandleNetMessage_GAME_EV_FINISHED_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FINISHED_TURN);

	const int iPlayerNum = message.popInt16();
	const int iTimeDelay = message.popInt16();
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
			string msgString = lngPack.i18n ("Text~Multiplayer~Player_Turn_End", Player->getName()) + ". " + lngPack.i18n ("Text~Multiplayer~Deadline", iToStr (iTimeDelay));
			gameGUI->addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
		}
		iEndTurnTime = gameTimer.gameTime + 100 * iTimeDelay;
	}
	else if (iPlayerNum != ActivePlayer->getNr() && iPlayerNum != -1)
	{
		string msgString = lngPack.i18n ("Text~Multiplayer~Player_Turn_End", Player->getName());
		gameGUI->addMessage (msgString);
		ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
	}
}

void cClient::HandleNetMessage_GAME_EV_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_DATA);

	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	(void) Player;  // TODO use me
	const int iID = message.popInt16();
	const bool bVehicle = message.popBool();
	const int iPosY = message.popInt16();
	const int iPosX = message.popInt16();

	Log.write (" Client: Received Unit Data: Vehicle: " + iToStr ( (int) bVehicle) + ", ID: " + iToStr (iID) + ", XPos: " + iToStr (iPosX) + ", YPos: " + iToStr (iPosY), cLog::eLOG_TYPE_NET_DEBUG);
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
		if (Vehicle->PosX != iPosX || Vehicle->PosY != iPosY || Vehicle->data.isBig != bBig)
		{
			// if the vehicle is moving it is normal that
			// the positions may not be the same,
			// when the vehicle was building it is also normal that
			// the position should be changed
			// so the log message will just be a debug one
			int iLogType = cLog::eLOG_TYPE_NET_WARNING;
			if (Vehicle->IsBuilding || Vehicle->IsClearing || Vehicle->moving) iLogType = cLog::eLOG_TYPE_NET_DEBUG;
			Log.write (" Client: Vehicle identificated by ID (" + iToStr (iID) + ") but has wrong position [IS: X" + iToStr (Vehicle->PosX) + " Y" + iToStr (Vehicle->PosY) + "; SHOULD: X" + iToStr (iPosX) + " Y" + iToStr (iPosY) + "]", iLogType);

			// set to server position if vehicle is not moving
			if (!Vehicle->MoveJobActive)
			{
				getMap()->moveVehicle (*Vehicle, iPosX, iPosY);
				if (bBig) getMap()->moveVehicleBig (*Vehicle, iPosX, iPosY);
				Vehicle->owner->doScan();
			}
		}

		if (message.popBool()) Vehicle->changeName (message.popString());

		Vehicle->isBeeingAttacked = message.popBool();
		const bool bWasDisabled = Vehicle->isDisabled();
		Vehicle->turnsDisabled = message.popInt16();
		Vehicle->CommandoRank = message.popInt16();
		Vehicle->IsClearing = message.popBool();
		bWasBuilding = Vehicle->IsBuilding;
		Vehicle->IsBuilding = message.popBool();
		Vehicle->BuildRounds = message.popInt16();
		Vehicle->ClearingRounds = message.popInt16();
		Vehicle->sentryActive = message.popBool();
		Vehicle->manualFireActive = message.popBool();

		if (Vehicle->isDisabled() != bWasDisabled && Vehicle->owner == ActivePlayer)
		{
			if (Vehicle->isDisabled())
			{
				const std::string msg = Vehicle->getDisplayName() + " " + lngPack.i18n ("Text~Comp~Disabled");
				const sSavedReportMessage& report = ActivePlayer->addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_UNIT, Vehicle->data.ID, Vehicle->PosX, Vehicle->PosY);
				gameGUI->addCoords (report);
				PlayVoice (VoiceData.VOIUnitDisabled);
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
		Building->turnsDisabled = message.popInt16();
		Building->researchArea = message.popInt16();
		Building->IsWorking = message.popBool();
		Building->sentryActive = message.popBool();
		Building->manualFireActive = message.popBool();
		Building->points = message.popInt16();

		if (Building->isDisabled() != bWasDisabled && Building->owner == ActivePlayer)
		{
			if (Building->isDisabled())
			{
				const std::string msg = Building->getDisplayName() + " " + lngPack.i18n ("Text~Comp~Disabled");
				const sSavedReportMessage& report = ActivePlayer->addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_UNIT, Building->data.ID, Building->PosX, Building->PosY);
				gameGUI->addCoords (report);
				PlayVoice (VoiceData.VOIUnitDisabled);
			}
			Building->owner->doScan();
		}
		Data = &Building->data;
	}

	Data->buildCosts = message.popInt16();
	Data->ammoCur = message.popInt16();
	Data->ammoMax = message.popInt16();
	Data->storageResCur = message.popInt16();
	Data->storageResMax = message.popInt16();
	Data->storageUnitsCur = message.popInt16();
	Data->storageUnitsMax = message.popInt16();
	Data->damage = message.popInt16();
	Data->shotsCur = message.popInt16();
	Data->shotsMax = message.popInt16();
	Data->range = message.popInt16();
	Data->scan = message.popInt16();
	Data->armor = message.popInt16();
	Data->hitpointsCur = message.popInt16();
	Data->hitpointsMax = message.popInt16();
	Data->version = message.popInt16();

	if (bVehicle)
	{
		if (Data->canPlaceMines)
		{
			if (Data->storageResCur <= 0) Vehicle->LayMines = false;
			if (Data->storageResCur >= Data->storageResMax) Vehicle->ClearMines = false;
		}
		Data->speedCur = message.popInt16();
		Data->speedMax = message.popInt16();

		if (bWasBuilding && !Vehicle->IsBuilding && Vehicle == gameGUI->getSelectedUnit()) gameGUI->stopFXLoop();

		Vehicle->FlightHigh = message.popInt16();
	}
}

void cClient::HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SPECIFIC_UNIT_DATA);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (!Vehicle) return;
	Vehicle->dir = message.popInt16();
	Vehicle->BuildingTyp = message.popID();
	Vehicle->BuildPath = message.popBool();
	Vehicle->BandX = message.popInt16();
	Vehicle->BandY = message.popInt16();
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
	building->ClientStartWork (*gameGUI);
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
	building->ClientStopWork (*gameGUI);
}

void cClient::HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MOVE_JOB_SERVER);

	const int iVehicleID = message.popInt32();
	const int iSrcOff = message.popInt32();
	const int iDestOff = message.popInt32();
	const int iSavedSpeed = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iVehicleID);

	if (Vehicle == NULL)
	{
		Log.write (" Client: Can't find vehicle with id " + iToStr (iVehicleID) + " for movejob from " + iToStr (iSrcOff % getMap()->getSize()) + "x" + iToStr (iSrcOff / getMap()->getSize()) + " to " + iToStr (iDestOff % getMap()->getSize()) + "x" + iToStr (iDestOff / getMap()->getSize()), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: request sync of vehicle
		return;
	}

	cClientMoveJob* MoveJob = new cClientMoveJob (*this, iSrcOff, iDestOff, Vehicle);
	MoveJob->iSavedSpeed = iSavedSpeed;
	if (!MoveJob->generateFromMessage (&message)) return;
	Log.write (" Client: Added received movejob at time " + iToStr (gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);
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

	cClientAttackJob::lockTarget (*this, &message);
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_FIRE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_FIRE);

	cClientAttackJob* job = new cClientAttackJob (this, &message);
	attackJobs.push_back (job);
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_IMPACT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_IMPACT);

	const int id = message.popInt16();
	const int remainingHP = message.popInt16();
	const int offset = message.popInt32();
	cClientAttackJob::makeImpact (*this, offset, remainingHP, id);
}

void cClient::HandleNetMessage_GAME_EV_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESOURCES);

	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		const int iOff = message.popInt32();
		ActivePlayer->exploreResource (iOff);

		sResources& res = getMap()->getResource (iOff);
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
				gameGUI->addMessage (msgString);
				ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
			}
			else if (Vehicle->BandX != Vehicle->PosX || Vehicle->BandY != Vehicle->PosY)
			{
				const string msgString = lngPack.i18n ("Text~Comp~Path_interrupted");
				const sSavedReportMessage& report = ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_UNIT, Vehicle->data.ID, Vehicle->PosX, Vehicle->PosY);
				gameGUI->addCoords (report);
			}
		}
		Vehicle->BuildRounds = 0;
		Vehicle->BuildingTyp.iFirstPart = 0;
		Vehicle->BuildingTyp.iSecondPart = 0;
		Vehicle->BuildPath = false;
		Vehicle->BandX = 0;
		Vehicle->BandY = 0;
		return;
	}

	if (Vehicle->IsBuilding) Log.write (" Client: Vehicle is already building", cLog::eLOG_TYPE_NET_ERROR);

	const int iBuildX = message.popInt16();
	const int iBuildY = message.popInt16();
	const bool buildBig = message.popBool();
	const int oldPosX = Vehicle->PosX;
	const int oldPosY = Vehicle->PosY;

	if (buildBig)
	{
		getMap()->moveVehicleBig (*Vehicle, iBuildX, iBuildY);
		Vehicle->owner->doScan();

		Vehicle->BigBetonAlpha = 10;
	}
	else
	{
		getMap()->moveVehicle (*Vehicle, iBuildX, iBuildY);
		Vehicle->owner->doScan();
	}

	if (Vehicle->owner == ActivePlayer)
	{
		Vehicle->BuildingTyp = message.popID();
		Vehicle->BuildRounds = message.popInt16();
		Vehicle->BuildPath = message.popBool();
		Vehicle->BandX = message.popInt16();
		Vehicle->BandY = message.popInt16();
	}

	Vehicle->IsBuilding = true;
	addJob (new cStartBuildJob (*Vehicle, oldPosX, oldPosY, buildBig));

	if (Vehicle == gameGUI->getSelectedUnit())
	{
		gameGUI->stopFXLoop();
		gameGUI->playStream (*Vehicle);
	}

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

	const int iNewPos = message.popInt32();

	if (Vehicle->data.isBig)
	{
		getMap()->moveVehicle (*Vehicle, iNewPos % getMap()->getSize(), iNewPos / getMap()->getSize());
		Vehicle->owner->doScan();
	}

	Vehicle->IsBuilding = false;
	Vehicle->BuildPath = false;

	if (gameGUI->getSelectedUnit() == Vehicle)
	{
		gameGUI->stopFXLoop();
		gameGUI->playStream (*Vehicle);
	}
}

void cClient::HandleNetMessage_GAME_EV_SUBBASE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SUBBASE_VALUES);

	const int iID = message.popInt16();
	sSubBase* SubBase = getSubBaseFromID (iID);
	if (SubBase == NULL)
	{
		Log.write (" Client: Can't add subbase values: Unknown subbase with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of subbases
		return;
	}

	SubBase->HumanProd = message.popInt16();
	SubBase->MaxHumanNeed = message.popInt16();
	SubBase->HumanNeed = message.popInt16();
	SubBase->OilProd = message.popInt16();
	SubBase->MaxOilNeed = message.popInt16();
	SubBase->OilNeed = message.popInt16();
	SubBase->MaxOil = message.popInt16();
	SubBase->Oil = message.popInt16();
	SubBase->GoldProd = message.popInt16();
	SubBase->MaxGoldNeed = message.popInt16();
	SubBase->GoldNeed = message.popInt16();
	SubBase->MaxGold = message.popInt16();
	SubBase->Gold = message.popInt16();
	SubBase->MetalProd = message.popInt16();
	SubBase->MaxMetalNeed = message.popInt16();
	SubBase->MetalNeed = message.popInt16();
	SubBase->MaxMetal = message.popInt16();
	SubBase->Metal = message.popInt16();
	SubBase->MaxEnergyNeed = message.popInt16();
	SubBase->MaxEnergyProd = message.popInt16();
	SubBase->EnergyNeed = message.popInt16();
	SubBase->EnergyProd = message.popInt16();

	//temporary debug check
	if (SubBase->getGoldProd() < SubBase->getMaxAllowedGoldProd() ||
		SubBase->getMetalProd() < SubBase->getMaxAllowedMetalProd() ||
		SubBase->getOilProd() < SubBase->getMaxAllowedOilProd())
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

	while (Building->BuildList->size())
	{
		delete (*Building->BuildList) [0];
		Building->BuildList->erase (Building->BuildList->begin());
	}
	const int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		sBuildList* BuildListItem = new sBuildList;
		BuildListItem->type = message.popID();
		BuildListItem->metall_remaining = message.popInt16();
		Building->BuildList->push_back (BuildListItem);
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

	const int nrResearchAreasFinished = message.popChar();
	const bool bFinishedResearch = (nrResearchAreasFinished > 0);
	if ( (iCount == 0 || !playVoice) && !bFinishedResearch) PlayVoice (VoiceData.VOIStartNone);
	if (iCount == 1)
	{
		sReportMsg += " " + lngPack.i18n ("Text~Comp~Finished") + ".";
		if (!bFinishedResearch && playVoice) PlayVoice (VoiceData.VOIStartOne);
	}
	else if (iCount > 1)
	{
		sReportMsg += " " + lngPack.i18n ("Text~Comp~Finished2") + ".";
		if (!bFinishedResearch && playVoice) PlayVoice (VoiceData.VOIStartMore);
	}
	gameGUI->addMessage (lngPack.i18n ("Text~Comp~Turn_Start") + " " + iToStr (iTurn));
	if (sReportMsg.length() > 0) gameGUI->addMessage (sReportMsg);
	string researchMsgString = "";
	if (bFinishedResearch)
	{
		ActivePlayer->researchFinished = true;
		PlayVoice (VoiceData.VOIResearchComplete);

		// build research finished string
		const string themeNames[8] =
		{
			lngPack.i18n ("Text~Vehicles~Damage"),
			lngPack.i18n ("Text~Hud~Shots"),
			lngPack.i18n ("Text~Hud~Range"),
			lngPack.i18n ("Text~Hud~Armor"),
			lngPack.i18n ("Text~Hud~Hitpoints"),
			lngPack.i18n ("Text~Hud~Speed"),
			lngPack.i18n ("Text~Hud~Scan"),
			lngPack.i18n ("Text~Vehicles~Costs")
		};

		researchMsgString = lngPack.i18n ("Text~Context~Research") + " " + lngPack.i18n ("Text~Comp~Finished") + ": ";
		for (int i = 0; i < nrResearchAreasFinished; i++)
		{
			const int area = message.popChar();
			if (0 <= area && area < 8)
			{
				researchMsgString += themeNames[area];
				if (i + 1 < nrResearchAreasFinished)
					researchMsgString += ", ";
			}
		}
		gameGUI->addMessage (researchMsgString);
	}

	// Save the report
	string msgString = lngPack.i18n ("Text~Comp~Turn_Start") + " " + iToStr (iTurn) + "\n";
	if (sReportMsg.length() > 0) msgString += sReportMsg + "\n";
	if (bFinishedResearch) msgString += researchMsgString + "\n";
	ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
}

void cClient::HandleNetMessage_GAME_EV_MARK_LOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MARK_LOG);

	Log.write ("=============================================================================================", cLog::eLOG_TYPE_NET_DEBUG);
	Log.write (message.popString(), cLog::eLOG_TYPE_NET_DEBUG);
	Log.write ("=============================================================================================", cLog::eLOG_TYPE_NET_DEBUG);
}

void cClient::HandleNetMessage_GAME_EV_SUPPLY (cNetMessage& message, cMenu* activeMenu)
{
	assert (message.iType == GAME_EV_SUPPLY);

	bool storageMenuActive = false;
	const int iType = message.popChar();
	if (message.popBool())
	{
		const int iID = message.popInt16();
		cVehicle* DestVehicle = getVehicleFromID (iID);
		if (!DestVehicle)
		{
			Log.write (" Client: Can't supply vehicle: Unknown vehicle with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of vehicle
			return;
		}
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.ammoCur = message.popInt16();
		else DestVehicle->data.hitpointsCur = message.popInt16();
		if (DestVehicle->Loaded)
		{
			// get the building which has loaded the unit
			cBuilding* Building = DestVehicle->owner->BuildingList;
			for (; Building; Building = Building->next)
			{
				if (Contains (Building->storedUnits, DestVehicle)) break;
			}
			if (Building != NULL && activeMenu != NULL)
			{
				//FIXME: Is activeMenu really an instance of cStorageMenu?
				cStorageMenu* storageMenu = dynamic_cast<cStorageMenu*> (activeMenu);
				if (storageMenu)
				{
					storageMenuActive = true;
					storageMenu->resetInfos();
					storageMenu->draw();
					storageMenu->playVoice (iType);
				}
			}
		}
	}
	else
	{
		const int iID = message.popInt16();
		cBuilding* DestBuilding = getBuildingFromID (iID);
		if (!DestBuilding)
		{
			Log.write (" Client: Can't supply building: Unknown building with ID: " + iToStr (iID), cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of building
			return;
		}
		if (iType == SUPPLY_TYPE_REARM) DestBuilding->data.ammoCur = message.popInt16();
		else DestBuilding->data.hitpointsCur = message.popInt16();
	}
	if (!storageMenuActive)
	{
		if (iType == SUPPLY_TYPE_REARM)
		{
			PlayFX (SoundData.SNDReload);// play order changed else no VOIReammo-sound - nonsinn
			PlayVoice (VoiceData.VOIReammo);
		}
		else
		{
			PlayFX (SoundData.SNDRepair);// play order changed else no VOIRepaired-sound - nonsinn
			PlayRandomVoice (VoiceData.VOIRepaired);
		}
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
	rubble->PosY = message.popInt16();
	rubble->PosX = message.popInt16();

	getMap()->addBuilding (*rubble, rubble->PosX, rubble->PosY);
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
			const int orgX = Vehicle->PosX;
			const int orgY = Vehicle->PosY;

			Vehicle->ClearingRounds = message.popInt16();
			const int bigoffset = message.popInt16();
			if (bigoffset >= 0)
			{
				getMap()->moveVehicleBig (*Vehicle, bigoffset % getMap()->getSize(), bigoffset / getMap()->getSize());
				Vehicle->owner->doScan();
			}
			Vehicle->IsClearing = true;
			addJob (new cStartBuildJob (*Vehicle, orgX, orgY, (bigoffset > 0)));

			if (gameGUI->getSelectedUnit() == Vehicle)
			{
				gameGUI->stopFXLoop();
				gameGUI->playStream (*Vehicle);
			}
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

	const int bigoffset = message.popInt16();
	if (bigoffset >= 0)
	{
		getMap()->moveVehicle (*Vehicle, bigoffset % getMap()->getSize(), bigoffset / getMap()->getSize());
		Vehicle->owner->doScan();
	}
	Vehicle->IsClearing = false;
	Vehicle->ClearingRounds = 0;

	if (gameGUI->getSelectedUnit() == Vehicle)
	{
		gameGUI->stopFXLoop();
		gameGUI->playStream (*Vehicle);
	}
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
	string msgString = lngPack.i18n ("Text~Multiplayer~Player") + " " + Player->getName() + " " + lngPack.i18n ("Text~Comp~Defeated");
	gameGUI->addMessage (msgString);
	ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
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
	gameGUI->addMessage (msgString);
	ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);

	deletePlayer (Player);
}

void cClient::HandleNetMessage_GAME_EV_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_TURN);

	iTurn = message.popInt16();
	gameGUI->updateTurn (iTurn);
}

void cClient::HandleNetMessage_GAME_EV_HUD_SETTINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_HUD_SETTINGS);

	const int unitID = message.popInt16();
	cUnit* unit = getVehicleFromID (unitID);
	if (!unit) unit = getBuildingFromID (unitID);

	if (unit) gameGUI->selectUnit (*unit);

	const int x = message.popInt16();
	const int y = message.popInt16();
	gameGUI->setOffsetPosition (x, y);
	gameGUI->setZoom (message.popFloat(), true, false);
	gameGUI->setColor (message.popBool());
	gameGUI->setGrid (message.popBool());
	gameGUI->setAmmo (message.popBool());
	gameGUI->setFog (message.popBool());
	gameGUI->setTwoX (message.popBool());
	gameGUI->setRange (message.popBool());
	gameGUI->setScan (message.popBool());
	gameGUI->setStatus (message.popBool());
	gameGUI->setSurvey (message.popBool());
	gameGUI->setLock (message.popBool());
	gameGUI->setHits (message.popBool());
	gameGUI->setTNT (message.popBool());

	gameGUI->setStartup (false);
}

void cClient::HandleNetMessage_GAME_EV_STORE_UNIT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_STORE_UNIT);

	cVehicle* StoredVehicle = getVehicleFromID (message.popInt16());
	if (!StoredVehicle) return;

	if (message.popBool())
	{
		cVehicle* StoringVehicle = getVehicleFromID (message.popInt16());
		if (!StoringVehicle) return;
		StoringVehicle->storeVehicle (StoredVehicle, getMap());
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;
		StoringBuilding->storeVehicle (StoredVehicle, getMap());
	}

	const int mouseX = mouse->getKachelX (*gameGUI);
	const int mouseY = mouse->getKachelY (*gameGUI);
	if (StoredVehicle->PosX == mouseX && StoredVehicle->PosY == mouseY) gameGUI->updateMouseCursor();

	gameGUI->checkMouseInputMode();

	if (StoredVehicle == gameGUI->getSelectedUnit()) gameGUI->deselectUnit();

	PlayFX (SoundData.SNDLoad);
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

		const int x = message.popInt16();
		const int y = message.popInt16();
		StoringVehicle->exitVehicleTo (StoredVehicle, getMap()->getOffset (x, y), getMap());
		if (gameGUI->getSelectedUnit() == StoringVehicle && gameGUI->mouseInputMode == activateVehicle)
		{
			gameGUI->mouseInputMode = normalInput;
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		const int x = message.popInt16();
		const int y = message.popInt16();
		StoringBuilding->exitVehicleTo (StoredVehicle, getMap()->getOffset (x, y), getMap());

		if (gameGUI->getSelectedUnit() == StoringBuilding && gameGUI->mouseInputMode == activateVehicle)
		{
			gameGUI->mouseInputMode = normalInput;
		}
	}
	PlayFX (SoundData.SNDActivate);
	gameGUI->updateMouseCursor();
}

void cClient::HandleNetMessage_GAME_EV_DELETE_EVERYTHING (cNetMessage& message, cMenu* activeMenu)
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
			deleteUnit (Player.VehicleList, activeMenu);
		}
		while (Player.BuildingList)
		{
			deleteUnit (Player.BuildingList, activeMenu);
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

	Data->version = message.popInt16();
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
		if (!scanNecessary && building->data.scan < ActivePlayer->BuildingData[building->typ->nr].scan)
			scanNecessary = true; // Scan range was upgraded. So trigger a scan.
		building->upgradeToCurrentVersion();
		if (i == 0)
		{
			buildingName = building->data.name;
		}
	}
	ostringstream os;
	os << lngPack.i18n ("Text~Comp~Upgrades_Done") << " " << buildingsInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", buildingName) << " (" << lngPack.i18n ("Text~Vehicles~Costs") << ": " << totalCosts << ")";
	string sTmp (os.str());
	gameGUI->addMessage (sTmp);
	ActivePlayer->addSavedReport (sTmp, sSavedReportMessage::REPORT_TYPE_COMP);
	if (scanNecessary)
		ActivePlayer->doScan();
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message, cMenu* activeMenu)
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
	cBuilding* storingBuilding = getBuildingFromID (storingBuildingID);
	if (storingBuilding && activeMenu)
	{
		cStorageMenu* storageMenu = dynamic_cast<cStorageMenu*> (activeMenu);
		if (storageMenu)
		{
			storageMenu->resetInfos();
			storageMenu->draw();
		}
	}
	ostringstream os;
	os << lngPack.i18n ("Text~Comp~Upgrades_Done") << " " << vehiclesInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", vehicleName) << " (" << lngPack.i18n ("Text~Vehicles~Costs") << ": " << totalCosts << ")";

	string printStr (os.str());
	gameGUI->addMessage (printStr);
	ActivePlayer->addSavedReport (printStr, sSavedReportMessage::REPORT_TYPE_COMP);
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

	if (message.popBool())	//success?
	{
		if (message.popBool())
		{
			PlayRandomVoice (VoiceData.VOIUnitStolen);
		}
		else
		{
			PlayVoice (VoiceData.VOIUnitDisabled);
		}
	}
	else
	{
		PlayRandomVoice (VoiceData.VOICommandoFailed);
	}

	/* Ignore vehicle ID. */
	message.popInt16();

	gameGUI->checkMouseInputMode();
}

void cClient::HandleNetMessage_GAME_EV_REQ_SAVE_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQ_SAVE_INFO);

	const int saveingID = message.popInt16();
	if (gameGUI->getSelectedUnit()) sendSaveHudInfo (*this, gameGUI->getSelectedUnit()->iID, ActivePlayer->getNr(), saveingID);
	else sendSaveHudInfo (*this, -1, ActivePlayer->getNr(), saveingID);

	const std::vector<sSavedReportMessage>& savedReports = ActivePlayer->savedReportsList;
	for (size_t i = std::max<int> (0, savedReports.size() - 50); i != savedReports.size(); ++i)
	{
		sendSaveReportInfo (*this, savedReports[i], ActivePlayer->getNr(), saveingID);
	}
	sendFinishedSendSaveInfo (*this, ActivePlayer->getNr(), saveingID);
}

void cClient::HandleNetMessage_GAME_EV_SAVED_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVED_REPORT);

	sSavedReportMessage savedReport;
	savedReport.message = message.popString();
	savedReport.type = (sSavedReportMessage::eReportTypes) message.popInt16();
	savedReport.xPos = message.popInt16();
	savedReport.yPos = message.popInt16();
	savedReport.unitID = message.popID();
	savedReport.colorNr = message.popInt16();
	ActivePlayer->savedReportsList.push_back (savedReport);
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

void cClient::HandleNetMessage_GAME_EV_VICTORY_CONDITIONS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_VICTORY_CONDITIONS);

	scoreLimit = message.popInt16();
	turnLimit = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_SELFDESTROY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SELFDESTROY);

	cBuilding* building = getBuildingFromID (message.popInt16());
	if (!building) return;

	destroyUnit (building);
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

int cClient::HandleNetMessage (cNetMessage* message, cMenu* activeMenu)
{
	if (message->iType != NET_GAME_TIME_SERVER)
		Log.write ("Client: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message->iType)
	{
		case TCP_ACCEPT:
			//should not happen
			break;
		case TCP_CLOSE: HandleNetMessage_TCP_CLOSE (*message); break;
		case GAME_EV_CHAT_SERVER: HandleNetMessage_GAME_EV_CHAT_SERVER (*message); break;
		case GAME_EV_PLAYER_CLANS: HandleNetMessage_GAME_EV_PLAYER_CLANS (*message); break;
		case GAME_EV_ADD_BUILDING: HandleNetMessage_GAME_EV_ADD_BUILDING (*message); break;
		case GAME_EV_ADD_VEHICLE: HandleNetMessage_GAME_EV_ADD_VEHICLE (*message); break;
		case GAME_EV_DEL_BUILDING: HandleNetMessage_GAME_EV_DEL_BUILDING (*message, activeMenu); break;
		case GAME_EV_DEL_VEHICLE: HandleNetMessage_GAME_EV_DEL_VEHICLE (*message, activeMenu); break;
		case GAME_EV_ADD_ENEM_VEHICLE: HandleNetMessage_GAME_EV_ADD_ENEM_VEHICLE (*message); break;
		case GAME_EV_ADD_ENEM_BUILDING: HandleNetMessage_GAME_EV_ADD_ENEM_BUILDING (*message); break;
		case GAME_EV_WAIT_FOR: HandleNetMessage_GAME_EV_WAIT_FOR (*message); break;
		case GAME_EV_MAKE_TURNEND: HandleNetMessage_GAME_EV_MAKE_TURNEND (*message); break;
		case GAME_EV_FINISHED_TURN: HandleNetMessage_GAME_EV_FINISHED_TURN (*message); break;
		case GAME_EV_UNIT_DATA: HandleNetMessage_GAME_EV_UNIT_DATA (*message); break;
		case GAME_EV_SPECIFIC_UNIT_DATA: HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (*message); break;
		case GAME_EV_DO_START_WORK: HandleNetMessage_GAME_EV_DO_START_WORK (*message); break;
		case GAME_EV_DO_STOP_WORK: HandleNetMessage_GAME_EV_DO_STOP_WORK (*message); break;
		case GAME_EV_MOVE_JOB_SERVER: HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (*message); break;
		case GAME_EV_NEXT_MOVE: HandleNetMessage_GAME_EV_NEXT_MOVE (*message); break;
		case GAME_EV_ATTACKJOB_LOCK_TARGET: HandleNetMessage_GAME_EV_ATTACKJOB_LOCK_TARGET (*message); break;
		case GAME_EV_ATTACKJOB_FIRE: HandleNetMessage_GAME_EV_ATTACKJOB_FIRE (*message); break;
		case GAME_EV_ATTACKJOB_IMPACT: HandleNetMessage_GAME_EV_ATTACKJOB_IMPACT (*message); break;
		case GAME_EV_RESOURCES: HandleNetMessage_GAME_EV_RESOURCES (*message); break;
		case GAME_EV_BUILD_ANSWER: HandleNetMessage_GAME_EV_BUILD_ANSWER (*message); break;
		case GAME_EV_STOP_BUILD: HandleNetMessage_GAME_EV_STOP_BUILD (*message); break;
		case GAME_EV_SUBBASE_VALUES: HandleNetMessage_GAME_EV_SUBBASE_VALUES (*message); break;
		case GAME_EV_BUILDLIST: HandleNetMessage_GAME_EV_BUILDLIST (*message); break;
		case GAME_EV_MINE_PRODUCE_VALUES: HandleNetMessage_GAME_EV_MINE_PRODUCE_VALUES (*message); break;
		case GAME_EV_TURN_REPORT: HandleNetMessage_GAME_EV_TURN_REPORT (*message); break;
		case GAME_EV_MARK_LOG: HandleNetMessage_GAME_EV_MARK_LOG (*message); break;
		case GAME_EV_SUPPLY: HandleNetMessage_GAME_EV_SUPPLY (*message, activeMenu); break;
		case GAME_EV_ADD_RUBBLE: HandleNetMessage_GAME_EV_ADD_RUBBLE (*message); break;
		case GAME_EV_DETECTION_STATE: HandleNetMessage_GAME_EV_DETECTION_STATE (*message); break;
		case GAME_EV_CLEAR_ANSWER: HandleNetMessage_GAME_EV_CLEAR_ANSWER (*message); break;
		case GAME_EV_STOP_CLEARING: HandleNetMessage_GAME_EV_STOP_CLEARING (*message); break;
		case GAME_EV_NOFOG: HandleNetMessage_GAME_EV_NOFOG (*message); break;
		case GAME_EV_DEFEATED: HandleNetMessage_GAME_EV_DEFEATED (*message); break;
		case GAME_EV_FREEZE: HandleNetMessage_GAME_EV_FREEZE (*message); break;
		case GAME_EV_UNFREEZE: HandleNetMessage_GAME_EV_UNFREEZE (*message); break;
		case GAME_EV_DEL_PLAYER: HandleNetMessage_GAME_EV_DEL_PLAYER (*message); break;
		case GAME_EV_TURN: HandleNetMessage_GAME_EV_TURN (*message); break;
		case GAME_EV_HUD_SETTINGS: HandleNetMessage_GAME_EV_HUD_SETTINGS (*message); break;
		case GAME_EV_STORE_UNIT: HandleNetMessage_GAME_EV_STORE_UNIT (*message); break;
		case GAME_EV_EXIT_UNIT: HandleNetMessage_GAME_EV_EXIT_UNIT (*message); break;
		case GAME_EV_DELETE_EVERYTHING: HandleNetMessage_GAME_EV_DELETE_EVERYTHING (*message, activeMenu); break;
		case GAME_EV_UNIT_UPGRADE_VALUES: HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (*message); break;
		case GAME_EV_CREDITS_CHANGED: HandleNetMessage_GAME_EV_CREDITS_CHANGED (*message); break;
		case GAME_EV_UPGRADED_BUILDINGS: HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (*message); break;
		case GAME_EV_UPGRADED_VEHICLES: HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (*message, activeMenu); break;
		case GAME_EV_RESEARCH_SETTINGS: HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (*message); break;
		case GAME_EV_RESEARCH_LEVEL: HandleNetMessage_GAME_EV_RESEARCH_LEVEL (*message); break;
		case GAME_EV_REFRESH_RESEARCH_COUNT: // sent, when the player was resynced (or a game was loaded)
			HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (*message); break;
		case GAME_EV_SET_AUTOMOVE: HandleNetMessage_GAME_EV_SET_AUTOMOVE (*message); break;
		case GAME_EV_COMMANDO_ANSWER: HandleNetMessage_GAME_EV_COMMANDO_ANSWER (*message); break;
		case GAME_EV_REQ_SAVE_INFO: HandleNetMessage_GAME_EV_REQ_SAVE_INFO (*message); break;
		case GAME_EV_SAVED_REPORT: HandleNetMessage_GAME_EV_SAVED_REPORT (*message); break;
		case GAME_EV_CASUALTIES_REPORT: HandleNetMessage_GAME_EV_CASUALTIES_REPORT (*message); break;
		case GAME_EV_SCORE: HandleNetMessage_GAME_EV_SCORE (*message); break;
		case GAME_EV_NUM_ECOS: HandleNetMessage_GAME_EV_NUM_ECOS (*message); break;
		case GAME_EV_UNIT_SCORE: HandleNetMessage_GAME_EV_UNIT_SCORE (*message); break;
		case GAME_EV_VICTORY_CONDITIONS: HandleNetMessage_GAME_EV_VICTORY_CONDITIONS (*message); break;
		case GAME_EV_SELFDESTROY: HandleNetMessage_GAME_EV_SELFDESTROY (*message); break;
		case GAME_EV_END_MOVE_ACTION_SERVER: HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (*message); break;
		case GAME_EV_SET_GAME_TIME: HandleNetMessage_GAME_EV_SET_GAME_TIME (*message); break;
		case NET_GAME_TIME_SERVER: gameTimer.handleSyncMessage (*message); break;

		default:
			Log.write ("Client: Can not handle message type " + message->getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
			break;
	}

	return 0;
}

void cClient::addUnit (int iPosX, int iPosY, cVehicle* AddedVehicle, bool bInit, bool bAddToMap)
{
	// place the vehicle
	if (bAddToMap) getMap()->addVehicle (*AddedVehicle, iPosX, iPosY);

	if (!bInit) AddedVehicle->StartUp = 10;

	gameGUI->updateMouseCursor();
	gameGUI->callMiniMapDraw();

	if (AddedVehicle->owner != ActivePlayer && AddedVehicle->iID == ActivePlayer->lastDeletedUnit)
	{
		//this unit was captured by an infiltrator
		PlayVoice (VoiceData.VOIUnitStolenByEnemy);
		const std::string msg = lngPack.i18n ("Text~Comp~CapturedByEnemy", AddedVehicle->getDisplayName());
		const sSavedReportMessage& report = getActivePlayer()->addSavedReport (msg, sSavedReportMessage::REPORT_TYPE_UNIT, AddedVehicle->data.ID, AddedVehicle->PosX, AddedVehicle->PosY);
		gameGUI->addCoords (report);
	}
	else if (AddedVehicle->owner != ActivePlayer)
	{
		// make report
		const string message = AddedVehicle->getDisplayName() + " (" + AddedVehicle->owner->getName() + ") " + lngPack.i18n ("Text~Comp~Detected");
		const sSavedReportMessage& report = getActivePlayer()->addSavedReport (message, sSavedReportMessage::REPORT_TYPE_UNIT, AddedVehicle->data.ID, iPosX, iPosY);
		gameGUI->addCoords (report);

		if (AddedVehicle->data.isStealthOn & TERRAIN_SEA && AddedVehicle->data.canAttack)
			PlayVoice (VoiceData.VOISubDetected);
		else
			PlayRandomVoice (VoiceData.VOIDetected);
	}
}

void cClient::addUnit (int iPosX, int iPosY, cBuilding* AddedBuilding, bool bInit)
{
	// place the building
	getMap()->addBuilding (*AddedBuilding, iPosX, iPosY);

	if (!bInit) AddedBuilding->StartUp = 10;

	gameGUI->updateMouseCursor();
	gameGUI->callMiniMapDraw();
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
	for (unsigned int i = 0; i < PlayerList->size(); i++)
	{
		if ( (*PlayerList) [i]->getName().compare (playerID) == 0) return (*PlayerList) [i];
	}

	return NULL;
}

void cClient::deleteUnit (cBuilding* Building, cMenu* activeMenu)
{
	if (!Building) return;
	gameGUI->onRemoveUnit (*Building);

	if (activeMenu) activeMenu->handleDestroyUnit (*Building);
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

void cClient::deleteUnit (cVehicle* Vehicle, cMenu* activeMenu)
{
	if (!Vehicle) return;

	if (activeMenu) activeMenu->handleDestroyUnit (*Vehicle);
	getMap()->deleteVehicle (*Vehicle);

	for (unsigned int i = 0; i < attackJobs.size(); i++)
	{
		attackJobs[i]->onRemoveUnit (*Vehicle);
	}
	helperJobs.onRemoveUnit (Vehicle);

	gameGUI->onRemoveUnit (*Vehicle);

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
}

void cClient::makeHotSeatEnd (int iNextPlayerNum)
{
#if 0
	// clear the messages
	for (size_t i = 0; i != messages.size(); ++i)
	{
		delete messages[i];
	}
	messages.Clear();
#endif
#if 0
	// save information and set next player
	//ActivePlayer->HotHud = Hud;
	const int iZoom = Hud.LastZoom;
	// TODO: maybe here must be done more than just set the next player!
	ActivePlayer = getPlayerFromNumber (iNextPlayerNum);
	//Hud = ActivePlayer->HotHud;
	const int iX = Hud.OffX;
	const int iY = Hud.OffY;
	if (Hud.LastZoom != iZoom)
	{
		Hud.LastZoom = -1;
		Hud.ScaleSurfaces();
	}
	Hud.OffX = iX;
	Hud.OffY = iY;
#endif
	// reset the screen
	gameGUI->deselectUnit();
	SDL_Surface* sf = SDL_CreateRGBSurface (SDL_SRCCOLORKEY, Video.getResolutionX(), Video.getResolutionY(), 32, 0, 0, 0, 0);
	SDL_Rect scr = { 15, 356, 112u, 112u};
	SDL_BlitSurface (sf, NULL, buffer, NULL);
	SDL_BlitSurface (sf, &scr, buffer, &scr);

	cDialogOK okDialog (lngPack.i18n ("Text~Multiplayer~Player_Turn", ActivePlayer->getName()));
	okDialog.show (this);
}

unsigned int cClient::getRemainingTimeInSecond() const
{
	if (iEndTurnTime <= iStartTurnTime) return ~0; // max value
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
		gameGUI->updateTurnTime (getRemainingTimeInSecond());
	}
	else
	{
		gameGUI->updateTurnTime (getElapsedTimeInSecond());
	}
}

void cClient::addActiveMoveJob (cClientMoveJob* MoveJob)
{
	MoveJob->bSuspended = false;
	if (MoveJob->Vehicle) MoveJob->Vehicle->MoveJobActive = true;
	if (Contains (ActiveMJobs, MoveJob)) return;
	ActiveMJobs.push_back (MoveJob);
}

void cClient::handleMoveJobs()
{
	for (int i = ActiveMJobs.size() - 1; i >= 0; i--)
	{
		cClientMoveJob* MoveJob = ActiveMJobs[i];
		cVehicle* Vehicle = MoveJob->Vehicle;

		// suspend movejobs of attacked vehicles
		if (Vehicle && Vehicle->isBeeingAttacked) continue;

		if (MoveJob->bFinished || MoveJob->bEndForNow)
		{
			if (Vehicle && Vehicle->ClientMoveJob == MoveJob) MoveJob->stopMoveSound();
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
			if (Vehicle == gameGUI->getSelectedUnit()) gameGUI->updateMouseCursor();
			continue;
		}
		if (MoveJob->bEndForNow)
		{
			Log.write (" Client: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
			if (Vehicle)
			{
				Vehicle->MoveJobActive = false;
				Vehicle->moving = false;
			}
			ActiveMJobs.erase (ActiveMJobs.begin() + i);
			if (Vehicle == gameGUI->getSelectedUnit()) gameGUI->updateMouseCursor();
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

void cClient::doGameActions (cMenu* activeMenu)
{
	runFx();

	// run attackJobs
	if (gameTimer.timer50ms)
		cClientAttackJob::handleAttackJobs (*this, activeMenu);

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

void cClient::destroyUnit (cVehicle* vehicle)
{
	// play explosion
	if (vehicle->data.isBig)
	{
		addFx (new cFxExploBig (vehicle->PosX * 64 + 64, vehicle->PosY * 64 + 64));
	}
	else if (vehicle->data.factorAir > 0 && vehicle->FlightHigh != 0)
	{
		addFx (new cFxExploAir (vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32));
	}
	else if (getMap()->isWaterOrCoast (vehicle->PosX, vehicle->PosY))
	{
		addFx (new cFxExploWater (vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32));
	}
	else
	{
		addFx (new cFxExploSmall (vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32));
	}

	if (vehicle->data.hasCorpse)
	{
		// add corpse
		addFx (new cFxCorpse (vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32));
	}
}

void cClient::destroyUnit (cBuilding* building)
{
	// play explosion animation
	cBuilding* topBuilding = getMap()->fields[getMap()->getOffset (building->PosX, building->PosY)].getBuilding();
	if (topBuilding && topBuilding->data.isBig)
	{
		addFx (new cFxExploBig (topBuilding->PosX * 64 + 64, topBuilding->PosY * 64 + 64));
	}
	else
	{
		addFx (new cFxExploSmall (building->PosX * 64 + 32, building->PosY * 64 + 32));
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
	gameGUI->updateInfoTexts();
}

void cClient::disableFreezeMode (eFreezeMode mode)
{
	freezeModes.disable (mode);
	gameGUI->updateInfoTexts();
}

bool cClient::isFreezed() const
{
	return freezeModes.isFreezed();
}

bool cClient::getFreezeMode (eFreezeMode mode) const
{
	return freezeModes.isEnable (mode);
}
