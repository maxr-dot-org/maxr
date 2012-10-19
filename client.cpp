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
#include <math.h>
#include <sstream>
#include "client.h"
#include "server.h"
#include "events.h"
#include "serverevents.h"
#include "clientevents.h"
#include "pcx.h"
#include "mouse.h"
#include "keys.h"
#include "unifonts.h"
#include "netmessage.h"
#include "main.h"
#include "attackJobs.h"
#include "menus.h"
#include "dialog.h"
#include "settings.h"
#include "hud.h"
#include "video.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "casualtiestracker.h"
#include "gametimer.h"
#include "jobs.h"

using namespace std;

sMessage::sMessage (std::string const& s, unsigned int const age_)
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

sFX::sFX (eFXTyps typ, int x, int y)
{
	this->typ = typ;
	PosX = x;
	PosY = y;
	StartTime = Client->gameGUI.iTimerTime;
	param = 0;
	rocketInfo = NULL;
	smokeInfo = NULL;
	trackInfo = NULL;
	param = 0;

	switch (typ)
	{
		case fxRocket:
		case fxTorpedo:
			rocketInfo = new sFXRocketInfos();
			rocketInfo->ScrX = 0;
			rocketInfo->ScrY = 0;
			rocketInfo->DestX = 0;
			rocketInfo->DestY = 0;
			rocketInfo->dir = 0;
			rocketInfo->fpx = 0;
			rocketInfo->fpy = 0;
			rocketInfo->mx = 0;
			rocketInfo->my = 0;
			rocketInfo->aj = NULL;
			break;
		case fxDarkSmoke:
			smokeInfo = new sFXDarkSmoke();
			smokeInfo->alpha = 0;
			smokeInfo->fx = 0;
			smokeInfo->fy = 0;
			smokeInfo->dx = 0;
			smokeInfo->dy = 0;
			break;
		case fxTracks:
			trackInfo = new sFXTracks();
			trackInfo->alpha = 0;
			trackInfo->dir = 0;
			break;
		default:
			break;
	}
}

sFX::~sFX()
{
	delete rocketInfo;
	delete smokeInfo;
	delete trackInfo;
}

//------------------------------------------------------------------------
// cClient implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cClient* Client = 0; // global instance

cClient::cClient (cMap* const Map, cList<cPlayer*>* const playerList) :
	Map (Map),
	PlayerList (playerList),
	gameGUI (NULL, Map, playerList),
	gameTimer ()
{
	gameGUI.setClient (this);
	neutralBuildings = NULL;
	iObjectStream = -1;
	bDefeated = false;
	iTurn = 1;
	bWantToEnd = false;
	bUpShowTank = true;
	bUpShowPlane = true;
	bUpShowShip = true;
	bUpShowBuild = true;
	bUpShowTNT = false;
	bAlienTech = false;
	iTurnTime = 0;
	scoreLimit = turnLimit = 0;

	casualtiesTracker = new cCasualtiesTracker();

	gameTimer.start ();
}

cClient::~cClient()
{
	gameTimer.stop ();

	delete casualtiesTracker;

	StopFXLoop (iObjectStream);
	for (size_t i = 0; i != FXList.Size(); ++i)
	{
		delete FXList[i];
	}
	for (size_t i = 0; i != FXListBottom.Size(); ++i)
	{
		delete FXListBottom[i];
	}
	for (unsigned int i = 0; i < attackJobs.Size(); i++)
	{
		delete attackJobs[i];
	}
	while (neutralBuildings)
	{
		cBuilding* nextBuilding = static_cast<cBuilding*> (neutralBuildings->next);
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}
}

void cClient::sendNetMessage (cNetMessage* message)
{
	message->iPlayerNr = ActivePlayer->Nr;

	if (message->iType != NET_GAME_TIME_CLIENT)
		Log.write ("Client: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	if (!network || network->isHost())
	{
		//push an event to the lokal server in singleplayer, HotSeat or if this machine is the host
		Server->pushEvent (message);
	}
	else // else send it over the net
	{
		//the client is only connected to one socket
		//so netwwork->send() only sends to the server
		network->send (message->iLength, message->serialize());
		delete message;
	}
}

void cClient::initPlayer (cPlayer* Player)
{
	ActivePlayer = Player;
	gameGUI.setPlayer (Player);

	// generate subbase for enemy players
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		if ( (*PlayerList) [i] == ActivePlayer) continue;
		(*PlayerList) [i]->base.SubBases.Add (new sSubBase ( (*PlayerList) [i]));
	}
}

int cClient::addMoveJob (cVehicle* vehicle, int DestX, int DestY, cList<cVehicle*>* group)
{
	sWaypoint* path = cClientMoveJob::calcPath (vehicle->PosX, vehicle->PosY, DestX, DestY, vehicle, group);
	if (path)
	{
		sendMoveJob (path, vehicle->iID);
		Log.write (" Client: Added new movejob: VehicleID: " + iToStr (vehicle->iID) + ", SrcX: " + iToStr (vehicle->PosX) + ", SrcY: " + iToStr (vehicle->PosY) + ", DestX: " + iToStr (DestX) + ", DestY: " + iToStr (DestY), cLog::eLOG_TYPE_NET_DEBUG);
		return 1;
	}
	else
	{
		if (!vehicle || !vehicle->autoMJob)   //automoving suveyors must not tell this
		{
			if (random (2)) PlayVoice (VoiceData.VOINoPath1);
			else PlayVoice (VoiceData.VOINoPath2);
		}
		return 0;
	}
}

void cClient::startGroupMove()
{
	int mainPosX = (*gameGUI.getSelVehiclesGroup()) [0]->PosX;
	int mainPosY = (*gameGUI.getSelVehiclesGroup()) [0]->PosY;
	int mainDestX = mouse->getKachelX();
	int mainDestY = mouse->getKachelY();

	// copy the selected-units-list
	cList<cVehicle*> group;
	for (unsigned int i = 0; i < gameGUI.getSelVehiclesGroup()->Size(); i++) group.Add ( (*gameGUI.getSelVehiclesGroup()) [i]);

	// go trough all vehicles in the list
	while (group.Size())
	{
		// we will start moving the vehicles in the list with the vehicle that is the closesed to the destination.
		// this will avoid that the units will crash into each other because the one infront of them has started
		// his move and the next field is free.
		int shortestWayLength = 0xFFFF;
		int shortestWayVehNum = 0;
		for (unsigned int i = 0; i < group.Size(); i++)
		{
			cVehicle* vehicle = group[i];
			int deltaX = vehicle->PosX - mainDestX + vehicle->PosX - mainPosX;
			int deltaY = vehicle->PosY - mainDestY + vehicle->PosY - mainPosY;
			int wayLength = Round (sqrt ( (double) deltaX * deltaX + deltaY * deltaY));

			if (wayLength < shortestWayLength)
			{
				shortestWayLength = wayLength;
				shortestWayVehNum = i;
			}
		}
		cVehicle* vehicle = group[shortestWayVehNum];
		// add the movejob to the destination of the unit.
		// the formation of the vehicle group will stay as destination formation.
		int destX = mainDestX + vehicle->PosX - mainPosX;
		int destY = mainDestY + vehicle->PosY - mainPosY ;
		addMoveJob (vehicle, destX, destY, gameGUI.getSelVehiclesGroup());
		// delete the unit from the copyed list
		group.Delete (shortestWayVehNum);
	}
}

void cClient::runFX()
{
	//TODO: add flag "gameTimeSynchronous" for fx
	//TODO: timerIntervall prüfen

	if (!gameGUI.timer100ms) return;
	for (unsigned int i = 0; i < FXList.Size(); i++)
	{
		sFX* fx = FXList[i];
		switch (fx->typ)
		{
			case fxRocket:
			{
				sFXRocketInfos* ri = fx->rocketInfo;
				if (abs (fx->PosX - ri->DestX) < 64 && abs (fx->PosY - ri->DestY) < 64)
				{
					ri->aj->state = cClientAttackJob::FINISHED;
					delete fx;
					FXList.Delete (i);
					return;
				}

				for (int k = 0; k < 64; k += 8)
				{
					if (cSettings::getInstance().isAlphaEffects())
					{
						addFX (fxSmoke, (int) ri->fpx, (int) ri->fpy, 0);
						gameGUI.drawFX ( (int) FXList.Size() - 1);
					}
					ri->fpx += ri->mx * 8;
					ri->fpy -= ri->my * 8;
				}

				fx->PosX = (int) ri->fpx;
				fx->PosY = (int) ri->fpy;
			}
			break;
			default:
				break;
		}
	}

	for (unsigned int i = 0; i < FXListBottom.Size(); i++)
	{
		sFX* fx = FXListBottom[i];
		switch (fx->typ)
		{
			case fxTorpedo:
			{
				sFXRocketInfos* ri = fx->rocketInfo;
				if (abs (fx->PosX - ri->DestX) < 64 && abs (fx->PosY - ri->DestY) < 64)
				{
					ri->aj->state = cClientAttackJob::FINISHED;
					delete fx;
					FXListBottom.Delete (i);
					return;
				}

				for (int k = 0; k < 64; k += 8)
				{
					if (cSettings::getInstance().isAlphaEffects())
					{
						addFX (fxBubbles, (int) ri->fpx, (int) ri->fpy, 0);
						gameGUI.drawBottomFX ( (int) FXListBottom.Size() - 1);
					}
					ri->fpx += ri->mx * 8;
					ri->fpy -= ri->my * 8;
				}

				fx->PosX = (int) (ri->fpx);
				fx->PosY = (int) (ri->fpy);
			}
			default:
				break;
		}
	}
}

// F¸gt einen FX-Effekt ein:
void cClient::addFX (eFXTyps typ, int x, int y, cClientAttackJob* aj, int iDestOff, int iFireDir)
{
	sFX* n = new sFX (typ, x, y);
	sFXRocketInfos* ri = n->rocketInfo;
	ri->ScrX = x;
	ri->ScrY = y;
	ri->DestX = (iDestOff % getMap()->size) * 64;
	ri->DestY = (iDestOff / getMap()->size) * 64;
	ri->aj = aj;
	ri->dir = iFireDir;
	addFX (n);
}

// F¸gt einen FX-Effekt ein:
void cClient::addFX (eFXTyps typ, int x, int y, int param)
{
	sFX* n = new sFX (typ, x, y);
	n->param = param;
	addFX (n);
}

// F¸gt einen FX-Effekt ein:
void cClient::addFX (sFX* n)
{
	if (n->typ == fxTracks || n->typ == fxTorpedo || n->typ == fxBubbles || n->typ == fxCorpse)
	{
		FXListBottom.Add (n);
	}
	else
	{
		FXList.Add (n);
	}
	switch (n->typ)
	{
		case fxExploAir:
			int nr;
			nr = random (3);
			if (nr == 0)
			{
				PlayFX (SoundData.EXPSmall0);
			}
			else if (nr == 1)
			{
				PlayFX (SoundData.EXPSmall1);
			}
			else
			{
				PlayFX (SoundData.EXPSmall2);
			}
			break;
		case fxExploSmall:
		case fxExploWater:
			if (getMap()->isWater (n->PosX / 64, n->PosY / 64))
			{
				int nr;
				nr = random (3);
				if (nr == 0)
				{
					PlayFX (SoundData.EXPSmallWet0);
				}
				else if (nr == 1)
				{
					PlayFX (SoundData.EXPSmallWet1);
				}
				else
				{
					PlayFX (SoundData.EXPSmallWet2);
				}
			}
			else
			{
				int nr;
				nr =  random (3);
				if (nr == 0)
				{
					PlayFX (SoundData.EXPSmall0);
				}
				else if (nr == 1)
				{
					PlayFX (SoundData.EXPSmall1);
				}
				else
				{
					PlayFX (SoundData.EXPSmall2);
				}
			}
			break;
		case fxExploBig:
			if (getMap()->isWater (n->PosX / 64, n->PosY / 64))
			{
				if (random (2))
				{
					PlayFX (SoundData.EXPBigWet0);
				}
				else
				{
					PlayFX (SoundData.EXPBigWet1);
				}
			}
			else
			{
				int nr;
				nr = random (4);
				if (nr == 0)
				{
					PlayFX (SoundData.EXPBig0);
				}
				else if (nr == 1)
				{
					PlayFX (SoundData.EXPBig1);
				}
				else if (nr == 2)
				{
					PlayFX (SoundData.EXPBig2);
				}
				else
				{
					PlayFX (SoundData.EXPBig3);
				}
			}
			break;
		case fxRocket:
		case fxTorpedo:
		{
			sFXRocketInfos* ri;
			int dx, dy;
			ri = n->rocketInfo;
			ri->fpx = (float) n->PosX;
			ri->fpy = (float) n->PosY;
			dx = ri->ScrX - ri->DestX;
			dy = ri->ScrY - ri->DestY;
			if (abs (dx) > abs (dy))
			{
				if (ri->ScrX > ri->DestX) ri->mx = -1;
				else ri->mx = 1;
				ri->my = dy / (float) dx * (-ri->mx);
			}
			else
			{
				if (ri->ScrY < ri->DestY) ri->my = -1;
				else ri->my = 1;
				ri->mx = dx / (float) dy * (-ri->my);
			}
			break;
		}
		case fxDarkSmoke:
		{
			float x, y, ax, ay;
			sFXDarkSmoke* dsi = n->smokeInfo;
			dsi->alpha = n->param;
			if (dsi->alpha > 150) dsi->alpha = 150;
			dsi->fx = (float) n->PosX;
			dsi->fy = (float) n->PosY;

			ax = x = sin (gameGUI.getWindDir());
			ay = y = cos (gameGUI.getWindDir());
			if (ax < 0) ax = -ax;
			if (ay < 0) ay = -ay;
			if (ax > ay)
			{
				dsi->dx = (float) (x * 2 + random (5)        / 10.0);
				dsi->dy = (float) (y * 2 + (random (15) - 7) / 14.0);
			}
			else
			{
				dsi->dx = (float) (x * 2 + (random (15) - 7) / 14.0);
				dsi->dy = (float) (y * 2 + random (5)        / 10.0);
			}
			break;
		}
		case fxTracks:
		{
			sFXTracks* tri = n->trackInfo;
			tri->alpha = 100;
			tri->dir = n->param;
			break;
		}
		case fxCorpse:
			n->param = 255;
			break;
		case fxAbsorb:
			PlayFX (SoundData.SNDAbsorb);
			break;
		default:
			break;
	}
}

void cClient::HandleNetMessage_TCP_CLOSE (cNetMessage& message)
{
	assert (message.iType == TCP_CLOSE);

	network->close (message.popInt16());
	string msgString = lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server");
	gameGUI.addMessage (msgString);
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
			gameGUI.addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_CHAT);
			break;
		}
		case SERVER_ERROR_MESSAGE:
		{
			PlayFX (SoundData.SNDQuitsch);
			const string msgString = lngPack.i18n (message.popString());
			gameGUI.addMessage (msgString);
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
			gameGUI.addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
			break;
		}
	}
}

void cClient::HandleNetMessage_GAME_EV_PLAYER_CLANS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_PLAYER_CLANS);

	for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
	{
		int playerNr = message.popChar();
		int clan = message.popChar();

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
	sID UnitID;
	UnitID.iFirstPart = message.popInt16();
	UnitID.iSecondPart = message.popInt16();
	int PosY = message.popInt16();
	int PosX = message.popInt16();

	cBuilding* AddedBuilding = Player->addBuilding (PosX, PosY, UnitID.getBuilding (Player));
	AddedBuilding->iID = message.popInt16();

	addUnit (PosX, PosY, AddedBuilding, Init);

	Player->base.addBuilding (AddedBuilding, false);

	// play placesound if it is a mine
	if (UnitID == specialIDLandMine && Player == ActivePlayer) PlayFX (SoundData.SNDLandMinePlace);
	else if (UnitID == specialIDSeaMine && Player == ActivePlayer) PlayFX (SoundData.SNDSeaMinePlace);
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
	sID UnitID;

	UnitID.iFirstPart = message.popInt16();
	UnitID.iSecondPart = message.popInt16();
	int PosY = message.popInt16();
	int PosX = message.popInt16();

	cVehicle* AddedVehicle = Player->AddVehicle (PosX, PosY, UnitID.getVehicle (Player));
	AddedVehicle->iID = message.popInt16();
	bool bAddToMap = message.popBool();

	addUnit (PosX, PosY, AddedVehicle, Init, bAddToMap);
}

void cClient::HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEL_BUILDING);

	cBuilding* Building = getBuildingFromID (message.popInt16());

	if (Building)
	{
		// play clearsound if it is a mine
		if (Building->owner && Building->data.ID == specialIDLandMine && Building->owner == ActivePlayer) PlayFX (SoundData.SNDLandMineClear);
		else if (Building->owner && Building->data.ID == specialIDSeaMine && Building->owner == ActivePlayer) PlayFX (SoundData.SNDSeaMineClear);

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
	sID UnitID;

	UnitID.iFirstPart = message.popInt16();
	UnitID.iSecondPart = message.popInt16();
	int iPosY = message.popInt16();
	int iPosX = message.popInt16();
	cVehicle* AddedVehicle = Player->AddVehicle (iPosX, iPosY, UnitID.getVehicle (Player));

	AddedVehicle->dir = message.popInt16();
	AddedVehicle->iID = message.popInt16();
	AddedVehicle->data.version = message.popInt16();

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
	sID UnitID;

	UnitID.iFirstPart = message.popInt16();
	UnitID.iSecondPart = message.popInt16();
	int iPosY = message.popInt16();
	int iPosX = message.popInt16();

	cBuilding* AddedBuilding = Player->addBuilding (iPosX, iPosY, UnitID.getBuilding (Player));
	AddedBuilding->iID = message.popInt16();
	AddedBuilding->data.version = message.popInt16();
	addUnit (iPosX, iPosY, AddedBuilding, false);

	if (AddedBuilding->data.connectsToBase)
	{
		Player->base.SubBases[0]->buildings.Add (AddedBuilding);
		AddedBuilding->SubBase = Player->base.SubBases[0];

		AddedBuilding->updateNeighbours (getMap());
	}
}

void cClient::HandleNetMessage_GAME_EV_WAIT_FOR (cNetMessage& message)
{
	assert (message.iType == GAME_EV_WAIT_FOR);

	int nextPlayerNum = message.popInt16();

	if (nextPlayerNum != ActivePlayer->Nr)
	{
		enableFreezeMode(FREEZE_WAIT_FOR_OTHERS, nextPlayerNum);
		gameGUI.setEndButtonLock (true);
	}
}

void cClient::HandleNetMessage_GAME_EV_MAKE_TURNEND (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MAKE_TURNEND);

	int iNextPlayerNum = message.popInt16();
	bool bWaitForNextPlayer = message.popBool();
	bool bEndTurn = message.popBool();

	if (bEndTurn)
	{
		iTurn++;
		iTurnTime = 0;
		gameGUI.updateTurn (iTurn);
		if (!bWaitForNextPlayer) gameGUI.setEndButtonLock (false);
		bWantToEnd = false;
		gameGUI.updateTurnTime (-1);
		ActivePlayer->clearDone();
		Log.write ("######### Round " + iToStr (iTurn) + " ###########", cLog::eLOG_TYPE_NET_DEBUG);
		for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
		{
			(*getPlayerList()) [i]->bFinishedTurn = false;
		}
	}

	if (bWaitForNextPlayer)
	{
		if (iNextPlayerNum != ActivePlayer->Nr)
		{
			enableFreezeMode (FREEZE_WAIT_FOR_OTHERS, iNextPlayerNum);
		}
		else
		{
			disableFreezeMode (FREEZE_WAIT_FOR_OTHERS);
			gameGUI.setEndButtonLock (false);
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

	int iPlayerNum = message.popInt16();
	int iTimeDelay = message.popInt16();

	cPlayer* Player = getPlayerFromNumber (iPlayerNum);
	if (Player == NULL && iPlayerNum != -1)
	{
		Log.write (" Client: Player with nr " + iToStr (iPlayerNum) + " has finished turn, but can't find him", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	if (Player) Player->bFinishedTurn = true;

	if (iTimeDelay != -1)
	{
		if (iPlayerNum != ActivePlayer->Nr && iPlayerNum != -1)
		{
			string msgString = lngPack.i18n ("Text~Multiplayer~Player_Turn_End", Player->name) + ". " + lngPack.i18n ("Text~Multiplayer~Deadline", iToStr (iTimeDelay));
			gameGUI.addMessage (msgString);
			ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
		}
		iTurnTime = iTimeDelay;
		iStartTurnTime = SDL_GetTicks();
	}
	else if (iPlayerNum != ActivePlayer->Nr && iPlayerNum != -1)
	{
		string msgString = lngPack.i18n ("Text~Multiplayer~Player_Turn_End", Player->name);
		gameGUI.addMessage (msgString);
		ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
	}
}

void cClient::HandleNetMessage_GAME_EV_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_DATA);

	cPlayer* Player = getPlayerFromNumber (message.popInt16());
	(void) Player;  // TODO use me
	sUnitData* Data;

	bool bWasBuilding = false;
	int iID = message.popInt16();

	bool bVehicle = message.popBool();
	int iPosY = message.popInt16();
	int iPosX = message.popInt16();

	cVehicle* Vehicle = NULL;
	cBuilding* Building = NULL;

	Log.write (" Client: Received Unit Data: Vehicle: " + iToStr ( (int) bVehicle) + ", ID: " + iToStr (iID) + ", XPos: " + iToStr (iPosX) + ", YPos: " + iToStr (iPosY), cLog::eLOG_TYPE_NET_DEBUG);
	// unit is a vehicle
	if (bVehicle)
	{
		bool bBig = message.popBool();
		Vehicle = getVehicleFromID (iID);

		if (!Vehicle)
		{
			Log.write (" Client: Unknown vehicle with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of vehicle
			return;
		}
		if (Vehicle->PosX != iPosX || Vehicle->PosY != iPosY || Vehicle->data.isBig != bBig)
		{
			// if the vehicle is moving it is normal that the positions are not the same,
			// when the vehicle was building it is also normal that the position should be changed
			// so the log message will just be an debug one
			int iLogType = cLog::eLOG_TYPE_NET_WARNING;
			if (Vehicle->IsBuilding || Vehicle->IsClearing || Vehicle->moving) iLogType = cLog::eLOG_TYPE_NET_DEBUG;
			Log.write (" Client: Vehicle identificated by ID (" + iToStr (iID) + ") but has wrong position [IS: X" + iToStr (Vehicle->PosX) + " Y" + iToStr (Vehicle->PosY) + "; SHOULD: X" + iToStr (iPosX) + " Y" + iToStr (iPosY) + "]", iLogType);

			// set to server position if vehicle is not moving
			if (!Vehicle->MoveJobActive)
			{
				getMap()->moveVehicle (Vehicle, iPosX, iPosY);
				if (bBig) getMap()->moveVehicleBig (Vehicle, iPosX, iPosY);
				Vehicle->owner->DoScan();
			}
		}

		if (message.popBool()) Vehicle->changeName (message.popString());

		Vehicle->isBeeingAttacked = message.popBool();
		bool bWasDisabled = Vehicle->turnsDisabled > 0;
		Vehicle->turnsDisabled = message.popInt16();
		Vehicle->CommandoRank = message.popInt16();
		Vehicle->IsClearing = message.popBool();
		bWasBuilding = Vehicle->IsBuilding;
		Vehicle->IsBuilding = message.popBool();
		Vehicle->BuildRounds = message.popInt16();
		Vehicle->ClearingRounds = message.popInt16();
		Vehicle->sentryActive = message.popBool();
		Vehicle->manualFireActive = message.popBool();

		if ( (Vehicle->turnsDisabled > 0) != bWasDisabled && Vehicle->owner == ActivePlayer)
		{
			if (Vehicle->turnsDisabled > 0) ActivePlayer->addSavedReport (gameGUI.addCoords (Vehicle->getDisplayName() + " " + lngPack.i18n ("Text~Comp~Disabled"), Vehicle->PosX, Vehicle->PosY), sSavedReportMessage::REPORT_TYPE_UNIT, Vehicle->data.ID, Vehicle->PosX, Vehicle->PosY);
			Vehicle->owner->DoScan();
		}
		Data = &Vehicle->data;
	}
	else
	{
		Building = getBuildingFromID (iID);
		if (!Building)
		{
			Log.write (" Client: Unknown building with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of building
			return;
		}

		if (message.popBool()) Building->changeName (message.popString());

		bool bWasDisabled = Building->turnsDisabled > 0;
		Building->turnsDisabled = message.popInt16();
		Building->researchArea = message.popInt16();
		Building->IsWorking = message.popBool();
		Building->sentryActive = message.popBool();
		Building->manualFireActive = message.popBool();
		Building->points = message.popInt16();

		if ( (Building->turnsDisabled > 0) != bWasDisabled && Building->owner == ActivePlayer)
		{
			if (Building->turnsDisabled > 0) ActivePlayer->addSavedReport (gameGUI.addCoords (Building->getDisplayName() + " " + lngPack.i18n ("Text~Comp~Disabled"), Building->PosX, Building->PosY), sSavedReportMessage::REPORT_TYPE_UNIT, Building->data.ID, Building->PosX, Building->PosY);
			Building->owner->DoScan();
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

		if (bWasBuilding && !Vehicle->IsBuilding && Vehicle == gameGUI.getSelVehicle()) StopFXLoop (iObjectStream);

		Vehicle->FlightHigh = message.popInt16();
	}
}

void cClient::HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SPECIFIC_UNIT_DATA);

	cVehicle* Vehicle = getVehicleFromID (message.popInt16());
	if (!Vehicle) return;
	Vehicle->dir = message.popInt16();
	Vehicle->BuildingTyp.iFirstPart = message.popInt16();
	Vehicle->BuildingTyp.iSecondPart = message.popInt16();
	Vehicle->BuildPath = message.popBool();
	Vehicle->BandX = message.popInt16();
	Vehicle->BandY = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_DO_START_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DO_START_WORK);

	int iID = message.popInt32();

	cBuilding* building = getBuildingFromID (iID);
	if (building == NULL)
	{
		Log.write (" Client: Can't start work of building: Unknown building with id: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_ERROR);
		// TODO: Request sync of building
		return;
	}

	building->ClientStartWork (gameGUI);
}

void cClient::HandleNetMessage_GAME_EV_DO_STOP_WORK (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DO_STOP_WORK);

	int iID = message.popInt32();
	cBuilding* building = getBuildingFromID (iID);
	if (building == NULL)
	{
		Log.write (" Client: Can't stop work of building: Unknown building with id: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of building
		return;
	}

	building->ClientStopWork (gameGUI);
}

void cClient::HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MOVE_JOB_SERVER);

	int iVehicleID = message.popInt32();
	int iSrcOff = message.popInt32();
	int iDestOff = message.popInt32();
	int iSavedSpeed = message.popInt16();

	cVehicle* Vehicle = getVehicleFromID (iVehicleID);
	if (Vehicle == NULL)
	{
		Log.write (" Client: Can't find vehicle with id " + iToStr (iVehicleID) + " for movejob from " +  iToStr (iSrcOff % getMap()->size) + "x" + iToStr (iSrcOff / getMap()->size) + " to " + iToStr (iDestOff % getMap()->size) + "x" + iToStr (iDestOff / getMap()->size), cLog::eLOG_TYPE_NET_WARNING);
		// TODO: request sync of vehicle
		return;
	}

	cClientMoveJob* MoveJob = new cClientMoveJob (iSrcOff, iDestOff, Vehicle);
	MoveJob->iSavedSpeed = iSavedSpeed;
	if (!MoveJob->generateFromMessage (&message)) return;
	Log.write (" Client: Added received movejob at time "+ iToStr(gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);
}

void cClient::HandleNetMessage_GAME_EV_NEXT_MOVE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NEXT_MOVE);

	int iID = message.popInt16();
	int iDestX = message.popInt16();
	int iDestY = message.popInt16();
	int iType = message.popChar();
	int height = message.popChar();
	int iSavedSpeed = -1;
	if (iType == MJOB_STOP) iSavedSpeed = message.popChar();

	Log.write (" Client: Received information for next move: ID: " + iToStr (iID) + ", SrcX: " + iToStr (iDestX) + ", SrcY: " + iToStr (iDestY) + ", Type: " + iToStr (iType) + ", Time: " + iToStr(gameTimer.gameTime), cLog::eLOG_TYPE_NET_DEBUG);

	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle && Vehicle->ClientMoveJob)
	{
		Vehicle->ClientMoveJob->handleNextMove (iDestX, iDestY, iType, iSavedSpeed, height);
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

	cClientAttackJob::lockTarget (&message);
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_FIRE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_FIRE);

	cClientAttackJob* job = new cClientAttackJob (&message);
	attackJobs.Add (job);
}

void cClient::HandleNetMessage_GAME_EV_ATTACKJOB_IMPACT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ATTACKJOB_IMPACT);

	int id = message.popInt16();
	int remainingHP = message.popInt16();
	int offset = message.popInt32();
	cClientAttackJob::makeImpact (offset, remainingHP, id);
}

void cClient::HandleNetMessage_GAME_EV_RESOURCES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESOURCES);

	int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		int iOff = message.popInt32();
		ActivePlayer->ResourceMap[iOff] = 1;

		getMap()->Resources[iOff].typ = (unsigned char) message.popInt16();
		getMap()->Resources[iOff].value = (unsigned char) message.popInt16();
	}
}

void cClient::HandleNetMessage_GAME_EV_BUILD_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_BUILD_ANSWER);

	cVehicle* Vehicle;
	bool bOK = message.popBool();
	int iID = message.popInt16();

	Vehicle = getVehicleFromID (iID);
	if (Vehicle == NULL)
	{
		Log.write (" Client: Vehicle can't start building: Unknown vehicle with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of vehicle
		return;
	}

	if (!bOK)
	{
		if (Vehicle->owner == ActivePlayer)
		{
			string msgString;
			if (!Vehicle->BuildPath)
			{
				msgString = lngPack.i18n ("Text~Comp~Producing_Err");
				gameGUI.addMessage (msgString);
				ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
			}
			else if (Vehicle->BandX != Vehicle->PosX || Vehicle->BandY != Vehicle->PosY)
			{
				msgString =  lngPack.i18n ("Text~Comp~Path_interrupted");
				gameGUI.addCoords (msgString, Vehicle->PosX, Vehicle->PosY);
				ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_UNIT, Vehicle->data.ID , Vehicle->PosX, Vehicle->PosY);
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

	int iBuildX = message.popInt16();
	int iBuildY = message.popInt16();
	bool buildBig = message.popBool();
	int oldPosX = Vehicle->PosX;
	int oldPosY = Vehicle->PosY;

	if (buildBig)
	{
		getMap()->moveVehicleBig (Vehicle, iBuildX, iBuildY);
		Vehicle->owner->DoScan();

		Vehicle->BigBetonAlpha = 10;
	}
	else
	{
		getMap()->moveVehicle (Vehicle, iBuildX, iBuildY);
		Vehicle->owner->DoScan();
	}

	if (Vehicle->owner == ActivePlayer)
	{
		Vehicle->BuildingTyp.iFirstPart = message.popInt16();
		Vehicle->BuildingTyp.iSecondPart = message.popInt16();
		Vehicle->BuildRounds = message.popInt16();
		Vehicle->BuildPath = message.popBool();
		Vehicle->BandX = message.popInt16();
		Vehicle->BandY = message.popInt16();
	}

	Vehicle->IsBuilding = true;
	addJob (new cStartBuildJob(Vehicle, oldPosX, oldPosY, buildBig));

	if (Vehicle == gameGUI.getSelVehicle())
	{
		StopFXLoop (iObjectStream);
		iObjectStream = Vehicle->playStream();
	}

	if (Vehicle->ClientMoveJob) Vehicle->ClientMoveJob->release();
}


void cClient::HandleNetMessage_GAME_EV_STOP_BUILD (cNetMessage& message)
{
	assert (message.iType == GAME_EV_STOP_BUILD);

	int iID = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (iID);
	if (Vehicle == NULL)
	{
		Log.write (" Client: Can't stop building: Unknown vehicle with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of vehicle
		return;
	}

	int iNewPos = message.popInt32();

	if (Vehicle->data.isBig)
	{
		getMap()->moveVehicle (Vehicle, iNewPos % getMap()->size, iNewPos / getMap()->size);
		Vehicle->owner->DoScan();
	}

	Vehicle->IsBuilding = false;
	Vehicle->BuildPath = false;

	if (gameGUI.getSelVehicle() == Vehicle)
	{
		StopFXLoop (iObjectStream);
		iObjectStream = Vehicle->playStream();
	}
}

void cClient::HandleNetMessage_GAME_EV_SUBBASE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SUBBASE_VALUES);

	int iID = message.popInt16();
	sSubBase* SubBase = getSubBaseFromID (iID);
	if (SubBase == NULL)
	{
		Log.write (" Client: Can't add subbase values: Unknown subbase with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
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
	SubBase->MaxEnergyNeed  = message.popInt16();
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

	int iID = message.popInt16();
	cBuilding* Building = getBuildingFromID (iID);
	if (Building == NULL)
	{
		Log.write (" Client: Can't set buildlist: Unknown building with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
		// TODO: Request sync of building
		return;
	}

	while (Building->BuildList->Size())
	{
		delete (*Building->BuildList) [0];
		Building->BuildList->Delete (0);
	}
	int iCount = message.popInt16();
	for (int i = 0; i < iCount; i++)
	{
		sBuildList* BuildListItem = new sBuildList;
		BuildListItem->type.iFirstPart = message.popInt16();
		BuildListItem->type.iSecondPart = message.popInt16();
		BuildListItem->metall_remaining = message.popInt16();
		Building->BuildList->Add (BuildListItem);
	}

	Building->MetalPerRound = message.popInt16();
	Building->BuildSpeed = message.popInt16();
	Building->RepeatBuild = message.popBool();
}

void cClient::HandleNetMessage_GAME_EV_MINE_PRODUCE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_MINE_PRODUCE_VALUES);

	int iID = message.popInt16();
	cBuilding* Building = getBuildingFromID (iID);
	if (Building == NULL)
	{
		Log.write (" Client: Can't set produce values of building: Unknown building with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
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
	string sTmp;
	int iCount = 0;
	bool playVoice = false;

	int iReportAnz = message.popInt16();
	while (iReportAnz)
	{
		sID Type;
		Type.iFirstPart = message.popInt16();
		Type.iSecondPart = message.popInt16();
		int iAnz = message.popInt16();
		if (iCount) sReportMsg += ", ";
		iCount += iAnz;
		sTmp = iToStr (iAnz) + " " + Type.getUnitDataOriginalVersion()->name;
		sReportMsg += iAnz > 1 ? sTmp : Type.getUnitDataOriginalVersion()->name;
		if (Type.getUnitDataOriginalVersion()->surfacePosition == sUnitData::SURFACE_POS_GROUND) playVoice = true;
		iReportAnz--;
	}

	int nrResearchAreasFinished = message.popChar();
	bool bFinishedResearch = (nrResearchAreasFinished > 0);
	if ( (iCount == 0  || !playVoice) && !bFinishedResearch) PlayVoice (VoiceData.VOIStartNone);
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
	gameGUI.addMessage (lngPack.i18n ("Text~Comp~Turn_Start") + " " + iToStr (iTurn));
	if (sReportMsg.length() > 0) gameGUI.addMessage (sReportMsg);
	string researchMsgString = "";
	if (bFinishedResearch)
	{
		ActivePlayer->researchFinished = true;
		PlayVoice (VoiceData.VOIResearchComplete);

		// build research finished string
		string themeNames[8] =
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
			int area = message.popChar();
			if (0 <= area && area < 8)
			{
				researchMsgString += themeNames[area];
				if (i + 1 < nrResearchAreasFinished)
					researchMsgString += ", ";
			}
		}
		gameGUI.addMessage (researchMsgString);
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

void cClient::HandleNetMessage_GAME_EV_SUPPLY (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SUPPLY);

	bool storageMenuActive = false;
	int iType = message.popChar();
	if (message.popBool())
	{
		int iID = message.popInt16();
		cVehicle* DestVehicle = getVehicleFromID (iID);
		if (!DestVehicle)
		{
			Log.write (" Client: Can't supply vehicle: Unknown vehicle with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
			// TODO: Request sync of vehicle
			return;
		}
		if (iType == SUPPLY_TYPE_REARM) DestVehicle->data.ammoCur = message.popInt16();
		else DestVehicle->data.hitpointsCur = message.popInt16();
		if (DestVehicle->Loaded)
		{
			// get the building which has loaded the unit
			cBuilding* Building = DestVehicle->owner->BuildingList;
			while (Building)
			{
				bool found = false;
				for (unsigned int i = 0; i < Building->storedUnits.Size(); i++)
				{
					if (Building->storedUnits[i] == DestVehicle)
					{
						found = true;
						break;
					}
				}
				if (found) break;
				Building = static_cast<cBuilding*> (Building->next);
			}
			if (Building != NULL && ActiveMenu != NULL)
			{
				//FIXME: Is ActiveMenu really an instance of cStorageMenu?
				cStorageMenu* storageMenu = dynamic_cast<cStorageMenu*> (ActiveMenu);
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
		int iID = message.popInt16();
		cBuilding* DestBuilding = getBuildingFromID (iID);
		if (!DestBuilding)
		{
			Log.write (" Client: Can't supply building: Unknown building with ID: "  + iToStr (iID) , cLog::eLOG_TYPE_NET_WARNING);
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
			PlayVoice (VoiceData.VOILoaded);
			PlayFX (SoundData.SNDReload);
		}
		else
		{
			PlayVoice (VoiceData.VOIRepaired);
			PlayFX (SoundData.SNDRepair);
		}
	}
}

void cClient::HandleNetMessage_GAME_EV_ADD_RUBBLE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_ADD_RUBBLE);

	cBuilding* rubble = new cBuilding (NULL, NULL, NULL);
	rubble->next = neutralBuildings;
	if (neutralBuildings) neutralBuildings->prev = rubble;
	neutralBuildings = rubble;
	rubble->prev = NULL;

	rubble->data.isBig = message.popBool();
	rubble->RubbleTyp = message.popInt16();
	rubble->RubbleValue = message.popInt16();
	rubble->iID = message.popInt16();
	rubble->PosY = message.popInt16();
	rubble->PosX = message.popInt16();

	getMap()->addBuilding (rubble, rubble->PosX, rubble->PosY);
}

void cClient::HandleNetMessage_GAME_EV_DETECTION_STATE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DETECTION_STATE);

	int id = message.popInt32();
	cVehicle* vehicle = getVehicleFromID (id);
	if (vehicle == NULL)
	{
		Log.write (" Client: Vehicle (ID: " + iToStr (id) + ") not found", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	bool detected = message.popBool();
	if (detected)
	{
		//mark vehicle as detected with size of detectedByPlayerList > 0
		vehicle->detectedByPlayerList.Add (NULL);
	}
	else
	{
		vehicle->detectedByPlayerList.Clear();
	}
}

void cClient::HandleNetMessage_GAME_EV_CLEAR_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CLEAR_ANSWER);

	switch (message.popInt16())
	{
		case 0:
		{
			int id = message.popInt16();
			cVehicle* Vehicle = getVehicleFromID (id);
			if (Vehicle == NULL)
			{
				Log.write ("Client: Can not find vehicle with id " + iToStr (id) + " for clearing", LOG_TYPE_NET_WARNING);
				break;
			}
			int orgX = Vehicle->PosX;
			int orgY = Vehicle->PosY;

			Vehicle->ClearingRounds = message.popInt16();
			int bigoffset = message.popInt16();
			if (bigoffset >= 0) 
			{
				getMap()->moveVehicleBig (Vehicle, bigoffset % getMap()->size, bigoffset / getMap()->size);
				Vehicle->owner->DoScan ();
			}
			Vehicle->IsClearing = true;
			addJob (new cStartBuildJob(Vehicle, orgX, orgY, (bigoffset > 0)));

			if (gameGUI.getSelVehicle() == Vehicle)
			{
				StopFXLoop (iObjectStream);
				iObjectStream = Vehicle->playStream();
			}
		}
		break;
		case 1:
			// TODO: add blocked message
			// gameGUI.addMessage ( "blocked" );
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

	int id = message.popInt16();
	cVehicle* Vehicle = getVehicleFromID (id);
	if (Vehicle == NULL)
	{
		Log.write ("Client: Can not find vehicle with id " + iToStr (id) + " for stop clearing", LOG_TYPE_NET_WARNING);
		return;
	}

	int bigoffset = message.popInt16();
	if (bigoffset >= 0) 
	{
		getMap()->moveVehicle (Vehicle, bigoffset % getMap()->size, bigoffset / getMap()->size);
		Vehicle->owner->DoScan ();
	}
	Vehicle->IsClearing = false;
	Vehicle->ClearingRounds = 0;

	if (gameGUI.getSelVehicle() == Vehicle)
	{
		StopFXLoop (iObjectStream);
		iObjectStream = Vehicle->playStream();
	}
}

void cClient::HandleNetMessage_GAME_EV_NOFOG (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NOFOG);

	memset (ActivePlayer->ScanMap, 1, getMap()->size * getMap()->size);
}

void cClient::HandleNetMessage_GAME_EV_DEFEATED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DEFEATED);

	int iTmp = message.popInt16();
	cPlayer* Player = getPlayerFromNumber (iTmp);
	if (Player == NULL)
	{
		Log.write ("Client: Cannot find defeated player!", LOG_TYPE_NET_WARNING);
		return;
	}
	Player->isDefeated = true;
	string msgString = lngPack.i18n ("Text~Multiplayer~Player") + " " + Player->name + " " + lngPack.i18n ("Text~Comp~Defeated");
	gameGUI.addMessage (msgString);
	ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);
#if 0
	for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
	{
		if (Player == (*getPlayerList()) [i])
		{
			Hud.ExtraPlayers (Player->name + " (d)", GetColorNr (Player->color), i, Player->bFinishedTurn, false);
			return;
		}
	}
#endif
}

void cClient::HandleNetMessage_GAME_EV_FREEZE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_FREEZE);

	eFreezeMode mode =  (eFreezeMode) message.popInt16 ();
	int playerNumber = message.popInt16 ();
	enableFreezeMode (mode, playerNumber);
}

void cClient::HandleNetMessage_GAME_EV_UNFREEZE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNFREEZE);

	eFreezeMode mode = (eFreezeMode) message.popInt16 ();
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
	string msgString = lngPack.i18n ("Text~Multiplayer~Player_Left", Player->name);
	gameGUI.addMessage (msgString);
	ActivePlayer->addSavedReport (msgString, sSavedReportMessage::REPORT_TYPE_COMP);

	deletePlayer (Player);
}

void cClient::HandleNetMessage_GAME_EV_TURN (cNetMessage& message)
{
	assert (message.iType == GAME_EV_TURN);

	iTurn = message.popInt16();
	gameGUI.updateTurn (iTurn);
}

void cClient::HandleNetMessage_GAME_EV_HUD_SETTINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_HUD_SETTINGS);

	int unitID = message.popInt16();
	cBuilding* building = NULL;
	cVehicle* vehicle = getVehicleFromID (unitID);
	if (!vehicle) building = getBuildingFromID (unitID);

	if (vehicle)
	{
		gameGUI.selectUnit (vehicle);
	}
	else if (building)
	{
		gameGUI.selectUnit (building);
	}

	int x = message.popInt16();
	int y = message.popInt16();
	gameGUI.setOffsetPosition (x, y);
	gameGUI.setZoom (message.popFloat(), true, false);
	gameGUI.setColor (message.popBool());
	gameGUI.setGrid (message.popBool());
	gameGUI.setAmmo (message.popBool());
	gameGUI.setFog (message.popBool());
	gameGUI.setTwoX (message.popBool());
	gameGUI.setRange (message.popBool());
	gameGUI.setScan (message.popBool());
	gameGUI.setStatus (message.popBool());
	gameGUI.setSurvey (message.popBool());
	gameGUI.setLock (message.popBool());
	gameGUI.setHits (message.popBool());
	gameGUI.setTNT (message.popBool());

	gameGUI.setStartup (false);
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

	int mouseX = mouse->getKachelX();
	int mouseY = mouse->getKachelY();
	if (StoredVehicle->PosX == mouseX && StoredVehicle->PosY == mouseY) gameGUI.updateMouseCursor();

	gameGUI.checkMouseInputMode();

	if (StoredVehicle == gameGUI.getSelVehicle()) gameGUI.deselectUnit();

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

		int x = message.popInt16();
		int y = message.popInt16();
		StoringVehicle->exitVehicleTo (StoredVehicle, x + y * getMap()->size, getMap());
		if (gameGUI.getSelVehicle() == StoringVehicle && gameGUI.mouseInputMode == activateVehicle)
		{
			gameGUI.mouseInputMode = normalInput;
		}
	}
	else
	{
		cBuilding* StoringBuilding = getBuildingFromID (message.popInt16());
		if (!StoringBuilding) return;

		int x = message.popInt16();
		int y = message.popInt16();
		StoringBuilding->exitVehicleTo (StoredVehicle, x + y * getMap()->size, getMap());

		if (gameGUI.getSelBuilding() == StoringBuilding && gameGUI.mouseInputMode == activateVehicle)
		{
			gameGUI.mouseInputMode = normalInput;
		}
	}
	PlayFX (SoundData.SNDActivate);
	gameGUI.updateMouseCursor();
}

void cClient::HandleNetMessage_GAME_EV_DELETE_EVERYTHING (cNetMessage& message)
{
	assert (message.iType == GAME_EV_DELETE_EVERYTHING);

	for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
	{
		cPlayer& Player = * (*getPlayerList()) [i];

		for (cVehicle* vehicle = Player.VehicleList; vehicle; vehicle = static_cast<cVehicle*> (vehicle->next))
		{
			vehicle->deleteStoredUnits();
		}

		while (Player.VehicleList)
		{
			cVehicle* vehicle = static_cast<cVehicle*> (Player.VehicleList->next);
			Player.VehicleList->sentryActive = false;
			getMap()->deleteVehicle (Player.VehicleList);
			delete Player.VehicleList;
			Player.VehicleList = vehicle;
		}
		while (Player.BuildingList)
		{
			cBuilding* building = static_cast<cBuilding*> (Player.BuildingList->next);
			Player.BuildingList->sentryActive = false;
			Player.BuildingList->deleteStoredUnits();

			getMap()->deleteBuilding (Player.BuildingList);
			delete Player.BuildingList;
			Player.BuildingList = building;
		}
	}

	//delete subbases
	ActivePlayer->base.SubBases.Clear();

	while (neutralBuildings)
	{
		cBuilding* nextBuilding = static_cast<cBuilding*> (neutralBuildings->next);
		getMap()->deleteBuilding (neutralBuildings);
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}

	//delete attack jobs
	for (size_t i = 0; i != attackJobs.Size(); ++i)
	{
		delete attackJobs[i];
	}
	attackJobs.Clear();

	//delete FX effects, because a finished rocked animations would do a callback on an attackjob
	for (size_t i = 0; i != FXList.Size(); ++i)
	{
		delete FXList[i];
	}
	FXList.Clear();
	for (size_t i = 0; i != FXListBottom.Size(); ++i)
	{
		delete FXListBottom[i];
	}
	FXListBottom.Clear();
	// delete all eventually remaining pointers on the map, to prevent crashes after a resync.
	// Normally there shouldn't be any pointers left after deleting all units, but a resync is not
	// executed in normal situations and there are situations, when this happens.
	getMap()->reset();
}

void cClient::HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UNIT_UPGRADE_VALUES);

	sID ID;
	sUnitData* Data;
	ID.iFirstPart = message.popInt16();
	ID.iSecondPart = message.popInt16();
	Data = ID.getUnitDataCurrentVersion (ActivePlayer);
	if (Data != NULL)
	{
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
}

void cClient::HandleNetMessage_GAME_EV_CREDITS_CHANGED (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CREDITS_CHANGED);

	ActivePlayer->Credits = message.popInt16();
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UPGRADED_BUILDINGS);

	int buildingsInMsg = message.popInt16();
	int totalCosts = message.popInt16();
	if (buildingsInMsg > 0)
	{
		string buildingName;
		bool scanNecessary = false;
		for (int i = 0; i < buildingsInMsg; i++)
		{
			int buildingID = message.popInt32();
			cBuilding* building = getBuildingFromID (buildingID);
			if (!building)
			{
				Log.write (" Client: Unknown building with ID: "  + iToStr (buildingID) , cLog::eLOG_TYPE_NET_ERROR);
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
		os << lngPack.i18n ("Text~Comp~Upgrades_Done") << " " << buildingsInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", buildingName)  << " (" << lngPack.i18n ("Text~Vehicles~Costs") << ": " << totalCosts << ")";
		string sTmp (os.str());
		gameGUI.addMessage (sTmp);
		ActivePlayer->addSavedReport (sTmp, sSavedReportMessage::REPORT_TYPE_COMP);
		if (scanNecessary)
			ActivePlayer->DoScan();
	}
}

void cClient::HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message)
{
	assert (message.iType == GAME_EV_UPGRADED_VEHICLES);

	int vehiclesInMsg = message.popInt16();
	int totalCosts = message.popInt16();
	unsigned int storingBuildingID = message.popInt32();
	if (vehiclesInMsg > 0)
	{
		string vehicleName;
		for (int i = 0; i < vehiclesInMsg; i++)
		{
			int vehicleID = message.popInt32();
			cVehicle* vehicle = getVehicleFromID (vehicleID);
			if (!vehicle)
			{
				Log.write (" Client: Unknown vehicle with ID: "  + iToStr (vehicleID) , cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			vehicle->upgradeToCurrentVersion();
			if (i == 0)
			{
				vehicleName = vehicle->data.name;
			}
		}
		cBuilding* storingBuilding = getBuildingFromID (storingBuildingID);
		if (storingBuilding && ActiveMenu)
		{
			cStorageMenu* storageMenu = dynamic_cast<cStorageMenu*> (ActiveMenu);
			if (storageMenu)
			{
				storageMenu->resetInfos();
				storageMenu->draw();
			}
		}
		ostringstream os;
		os << lngPack.i18n ("Text~Comp~Upgrades_Done") << " " << vehiclesInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", vehicleName)  << " (" << lngPack.i18n ("Text~Vehicles~Costs") << ": " << totalCosts << ")";

		string printStr (os.str());
		gameGUI.addMessage (printStr);
		ActivePlayer->addSavedReport (printStr, sSavedReportMessage::REPORT_TYPE_COMP);
	}
}

void cClient::HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESEARCH_SETTINGS);

	int buildingsInMsg = message.popInt16();
	if (buildingsInMsg > 0)
	{
		for (int i = 0; i < buildingsInMsg; i++)
		{
			int buildingID = message.popInt32();
			int newArea = message.popChar();
			cBuilding* building = getBuildingFromID (buildingID);
			if (building && building->data.canResearch && 0 <= newArea && newArea <= cResearch::kNrResearchAreas)
				building->researchArea = newArea;
		}
	}
	// now update the research center count for the areas
	ActivePlayer->refreshResearchCentersWorkingOnArea();
}

void cClient::HandleNetMessage_GAME_EV_RESEARCH_LEVEL (cNetMessage& message)
{
	assert (message.iType == GAME_EV_RESEARCH_LEVEL);

	for (int area = cResearch::kNrResearchAreas - 1; area >= 0; area--)
	{
		int newCurPoints = message.popInt16();
		int newLevel = message.popInt16();
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
		if (Vehicle->autoMJob)
		{
			delete Vehicle->autoMJob;
			Vehicle->autoMJob = NULL;
		}
		Vehicle->autoMJob = new cAutoMJob (Vehicle);
	}
}

void cClient::HandleNetMessage_GAME_EV_COMMANDO_ANSWER (cNetMessage& message)
{
	assert (message.iType == GAME_EV_COMMANDO_ANSWER);

	if (message.popBool())	//success?
	{
		if (message.popBool()) PlayVoice (VoiceData.VOIUnitStolen);
		else PlayVoice (VoiceData.VOIUnitDisabled);
	}
	else
	{
		int i = random (3);
		if (i == 0)
			PlayVoice (VoiceData.VOICommandoFailed1);
		else if (i == 1)
			PlayVoice (VoiceData.VOICommandoFailed2);
		else
			PlayVoice (VoiceData.VOICommandoFailed3);
	}

	/* Ignore vehicle ID. */
	message.popInt16();

	gameGUI.checkMouseInputMode();
}

void cClient::HandleNetMessage_GAME_EV_REQ_SAVE_INFO (cNetMessage& message)
{
	assert (message.iType == GAME_EV_REQ_SAVE_INFO);

	int saveingID = message.popInt16();
	if (gameGUI.getSelVehicle()) sendSaveHudInfo (gameGUI.getSelVehicle()->iID, ActivePlayer->Nr, saveingID);
	else if (gameGUI.getSelBuilding()) sendSaveHudInfo (gameGUI.getSelBuilding()->iID, ActivePlayer->Nr, saveingID);
	else sendSaveHudInfo (-1, ActivePlayer->Nr, saveingID);

	for (int i = ActivePlayer->savedReportsList.Size() - 50; i < (int) ActivePlayer->savedReportsList.Size(); i++)
	{
		if (i < 0) continue;
		sendSaveReportInfo (&ActivePlayer->savedReportsList[i], ActivePlayer->Nr, saveingID);
	}
	sendFinishedSendSaveInfo (ActivePlayer->Nr, saveingID);
}

void cClient::HandleNetMessage_GAME_EV_SAVED_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SAVED_REPORT);

	sSavedReportMessage savedReport;
	savedReport.message = message.popString();
	savedReport.type = (sSavedReportMessage::eReportTypes) message.popInt16();
	savedReport.xPos = message.popInt16();
	savedReport.yPos = message.popInt16();
	savedReport.unitID.iFirstPart = message.popInt16();
	savedReport.unitID.iSecondPart = message.popInt16();
	savedReport.colorNr = message.popInt16();
	ActivePlayer->savedReportsList.Add (savedReport);
}

void cClient::HandleNetMessage_GAME_EV_CASUALTIES_REPORT (cNetMessage& message)
{
	assert (message.iType == GAME_EV_CASUALTIES_REPORT);

	if (casualtiesTracker != NULL)
		casualtiesTracker->updateCasualtiesFromNetMessage (&message);
}

void cClient::HandleNetMessage_GAME_EV_SCORE (cNetMessage& message)
{
	assert (message.iType == GAME_EV_SCORE);

	int pn = message.popInt16();
	int turn = message.popInt16();
	int n = message.popInt16();

	getPlayerFromNumber (pn)->setScore (n, turn);
}

void cClient::HandleNetMessage_GAME_EV_NUM_ECOS (cNetMessage& message)
{
	assert (message.iType == GAME_EV_NUM_ECOS);

	int pn = message.popInt16();
	int n = message.popInt16();

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

	int destID = message.popInt32();
	eEndMoveActionType type = (eEndMoveActionType) message.popChar();
	vehicle->ClientMoveJob->endMoveAction = new cEndMoveAction (vehicle, destID, type);
}



int cClient::HandleNetMessage (cNetMessage* message)
{
	if ( message->iType != NET_GAME_TIME_SERVER )
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
		case GAME_EV_DEL_BUILDING: HandleNetMessage_GAME_EV_DEL_BUILDING (*message); break;
		case GAME_EV_DEL_VEHICLE: HandleNetMessage_GAME_EV_DEL_VEHICLE (*message); break;
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
		case GAME_EV_SUPPLY: HandleNetMessage_GAME_EV_SUPPLY (*message); break;
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
		case GAME_EV_DELETE_EVERYTHING: HandleNetMessage_GAME_EV_DELETE_EVERYTHING (*message); break;
		case GAME_EV_UNIT_UPGRADE_VALUES: HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (*message); break;
		case GAME_EV_CREDITS_CHANGED: HandleNetMessage_GAME_EV_CREDITS_CHANGED (*message); break;
		case GAME_EV_UPGRADED_BUILDINGS: HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (*message); break;
		case GAME_EV_UPGRADED_VEHICLES: HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (*message); break;
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
	if (bAddToMap) getMap()->addVehicle (AddedVehicle, iPosX, iPosY);

	if (!bInit) AddedVehicle->StartUp = 10;

	gameGUI.updateMouseCursor();
	gameGUI.callMiniMapDraw();

	if (AddedVehicle->owner != ActivePlayer && AddedVehicle->iID == ActivePlayer->lastDeletedUnit)
	{
		//this unit was captured by an infiltrator
		PlayVoice (VoiceData.VOIUnitStolenByEnemy);
		getActivePlayer()->addSavedReport (gameGUI.addCoords (lngPack.i18n ("Text~Comp~CapturedByEnemy", AddedVehicle->getDisplayName()), AddedVehicle->PosX, AddedVehicle->PosY), sSavedReportMessage::REPORT_TYPE_UNIT, AddedVehicle->data.ID, AddedVehicle->PosX, AddedVehicle->PosY);
	}
	else if (AddedVehicle->owner != ActivePlayer)
	{
		// make report
		string message = AddedVehicle->getDisplayName() + " (" + AddedVehicle->owner->name + ") " + lngPack.i18n ("Text~Comp~Detected");
		getActivePlayer()->addSavedReport (gameGUI.addCoords (message, iPosX, iPosY), sSavedReportMessage::REPORT_TYPE_UNIT, AddedVehicle->data.ID, iPosX, iPosY);

		if (AddedVehicle->data.isStealthOn & TERRAIN_SEA && AddedVehicle->data.canAttack)
			PlayVoice (VoiceData.VOISubDetected);
		else if (random (2))
			PlayVoice (VoiceData.VOIDetected1);
		else
			PlayVoice (VoiceData.VOIDetected2);
	}
}

void cClient::addUnit (int iPosX, int iPosY, cBuilding* AddedBuilding, bool bInit)
{
	// place the building
	getMap()->addBuilding (AddedBuilding, iPosX, iPosY);


	if (!bInit) AddedBuilding->StartUp = 10;

	gameGUI.updateMouseCursor();
	gameGUI.callMiniMapDraw();
}

cPlayer* cClient::getPlayerFromNumber (int iNum)
{
	for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
	{
		cPlayer* const p = (*getPlayerList()) [i];
		if (p->Nr == iNum) return p;
	}
	return NULL;
}

void cClient::deleteUnit (cBuilding* Building)
{
	if (!Building) return;
	gameGUI.callMiniMapDraw();

	if (ActiveMenu) ActiveMenu->handleDestroyUnit (Building);
	getMap()->deleteBuilding (Building);

	if (!Building->owner)
	{
		if (!Building->prev)
		{
			neutralBuildings = static_cast<cBuilding*> (Building->next);
			if (Building->next)
				Building->next->prev = NULL;
		}
		else
		{
			Building->prev->next = Building->next;
			if (Building->next)
				Building->next->prev = Building->prev;
		}
		delete Building;
		return;
	}

	for (unsigned int i = 0; i < attackJobs.Size(); i++)
	{
		if (attackJobs[i]->building == Building)
		{
			attackJobs[i]->building = NULL;
		}
	}

	if (Building->prev)
	{
		Building->prev->next = Building->next;
		if (Building->next)
		{
			Building->next->prev = Building->prev;
		}
	}
	else
	{
		Building->owner->BuildingList = static_cast<cBuilding*> (Building->next);
		if (Building->next)
		{
			Building->next->prev = NULL;
		}
	}

	if (gameGUI.getSelBuilding() == Building)
	{
		gameGUI.deselectUnit();
	}

	if (Building->owner == ActivePlayer)
		Building->owner->base.deleteBuilding (Building, false);

	cPlayer* owner = Building->owner;
	delete Building;

	owner->DoScan();

}

void cClient::deleteUnit (cVehicle* Vehicle)
{
	if (!Vehicle) return;

	if (ActiveMenu) ActiveMenu->handleDestroyUnit (NULL, Vehicle);
	getMap()->deleteVehicle (Vehicle);

	for (unsigned int i = 0; i < attackJobs.Size(); i++)
	{
		if (attackJobs[i]->vehicle == Vehicle)
		{
			attackJobs[i]->vehicle = NULL;
		}
	}

	gameGUI.callMiniMapDraw();

	cPlayer* owner = Vehicle->owner;
	if (Vehicle->prev)
	{
		Vehicle->prev->next = Vehicle->next;
		if (Vehicle->next)
		{
			Vehicle->next->prev = Vehicle->prev;
		}
	}
	else
	{
		owner->VehicleList = static_cast<cVehicle*> (Vehicle->next);
		if (Vehicle->next)
		{
			Vehicle->next->prev = NULL;
		}
	}

	if (gameGUI.getSelVehicle() == Vehicle)
	{
		gameGUI.deselectUnit();
	}
	cList<cVehicle*>& selGroup = *gameGUI.getSelVehiclesGroup();
	for (size_t i = 0; i < selGroup.Size(); i++)
	{
		if (selGroup[i] == Vehicle) selGroup.Delete (i);
	}

	owner->lastDeletedUnit = Vehicle->iID;

	delete Vehicle;

	if (owner) owner->DoScan();
}

void cClient::handleEnd()
{
	if (isFreezed ()) return;
	bWantToEnd = true;
	sendWantToEndTurn();
}


void cClient::makeHotSeatEnd (int iNextPlayerNum)
{
	// clear the messages
	/*for (size_t i = 0; i != messages.Size(); ++i)
	{
		delete messages[i];
	}
	messages.Clear();
	*/
	// save information and set next player
	/*int iZoom, iX, iY;
	//ActivePlayer->HotHud = Hud;
	iZoom = Hud.LastZoom;
	ActivePlayer = getPlayerFromNumber( iNextPlayerNum );	// TODO: maybe here must be done more than just set the next player!
	//Hud = ActivePlayer->HotHud;
	iX = Hud.OffX;
	iY = Hud.OffY;
	if ( Hud.LastZoom != iZoom )
	{
		Hud.LastZoom = -1;
		Hud.ScaleSurfaces();
	}
	Hud.OffX = iX;
	Hud.OffY = iY;*/

	// reset the screen
	gameGUI.deselectUnit();
	SDL_Surface* sf;
	SDL_Rect scr;
	sf = SDL_CreateRGBSurface (SDL_SRCCOLORKEY, Video.getResolutionX(), Video.getResolutionY(), 32, 0, 0, 0, 0);
	scr.x = 15;
	scr.y = 356;
	scr.w = scr.h = 112;
	SDL_BlitSurface (sf, NULL, buffer, NULL);
	SDL_BlitSurface (sf, &scr, buffer, &scr);

	cDialogOK okDialog (lngPack.i18n ("Text~Multiplayer~Player_Turn", ActivePlayer->name));
	okDialog.show();
}

void cClient::handleTurnTime()
{
	//TODO: rewrite to gameTime, instead of SDL_Ticks
	static int lastCheckTime = SDL_GetTicks();
	if (!gameGUI.timer50ms) return;
	// stop time when waiting for reconnection
	if (isFreezed ())
	{
		iStartTurnTime += SDL_GetTicks() - lastCheckTime;
	}
	if (iTurnTime > 0)
	{
		int iRestTime = iTurnTime - Round ( (SDL_GetTicks() - iStartTurnTime) / 1000);
		if (iRestTime < 0) iRestTime = 0;
		gameGUI.updateTurnTime (iRestTime);
	}
	lastCheckTime = SDL_GetTicks();
}

void cClient::addActiveMoveJob (cClientMoveJob* MoveJob)
{
	MoveJob->bSuspended = false;
	if (MoveJob->Vehicle) MoveJob->Vehicle->MoveJobActive = true;
	for (unsigned int i = 0; i < ActiveMJobs.Size(); i++)
	{
		if (ActiveMJobs[i] == MoveJob) return;
	}
	ActiveMJobs.Add (MoveJob);
}

void cClient::handleMoveJobs()
{
	for (int i = ActiveMJobs.Size() - 1; i >= 0; i--)
	{
		cClientMoveJob* MoveJob;
		cVehicle* Vehicle;

		MoveJob = ActiveMJobs[i];
		Vehicle = MoveJob->Vehicle;

		//suspend movejobs of attacked vehicles
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
			ActiveMJobs.Delete (i);
			delete MoveJob;
			if (Vehicle == gameGUI.getSelVehicle()) gameGUI.updateMouseCursor();
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
			ActiveMJobs.Delete (i);
			if (Vehicle == gameGUI.getSelVehicle()) gameGUI.updateMouseCursor();
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
	cVehicle* Vehicle;
	for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
	{
		Vehicle = (*getPlayerList()) [i]->VehicleList;
		while (Vehicle)
		{
			if (Vehicle->iID == iID) return Vehicle;
			Vehicle = static_cast<cVehicle*> (Vehicle->next);
		}
	}
	return NULL;
}

cBuilding* cClient::getBuildingFromID (unsigned int iID)
{
	cBuilding* Building;
	for (unsigned int i = 0; i < getPlayerList()->Size(); i++)
	{
		Building = (*getPlayerList()) [i]->BuildingList;
		while (Building)
		{
			if (Building->iID == iID) return Building;
			Building = static_cast<cBuilding*> (Building->next);
		}
	}

	Building = neutralBuildings;
	while (Building)
	{
		if (Building->iID == iID) return Building;
		Building = static_cast<cBuilding*> (Building->next);
	}

	return NULL;
}

void cClient::doGameActions()
{
	//TODO: hud actions in menus?
	//TODO: gameSynchronous actions here

	//run attackJobs
	if (gameTimer.timer50ms)
		cClientAttackJob::handleAttackJobs();

	//run moveJobs - this has to be called before handling the auto movejobs
	if (gameTimer.timer10ms)
		handleMoveJobs();
	
	//run surveyor ai
	if (gameTimer.timer50ms)
		cAutoMJob::handleAutoMoveJobs();

	runJobs ();
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
	//play explosion
	if (vehicle->data.isBig)
	{
		addFX (fxExploBig, vehicle->PosX * 64 + 64, vehicle->PosY * 64 + 64, 0);
	}
	else if (vehicle->data.factorAir > 0 && vehicle->FlightHigh != 0)
	{
		addFX (fxExploAir, vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32, 0);
	}
	else if (getMap()->isWater (vehicle->PosX, vehicle->PosY))
	{
		addFX (fxExploWater, vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32, 0);
	}
	else
	{
		addFX (fxExploSmall, vehicle->PosX * 64 + vehicle->OffX + 32, vehicle->PosY * 64 + vehicle->OffY + 32, 0);
	}

	if (vehicle->data.hasCorpse)
	{
		//add corpse
		addFX (fxCorpse,  vehicle->PosX * 64 + vehicle->OffX, vehicle->PosY * 64 + vehicle->OffY, 0);
	}
}

void cClient::destroyUnit (cBuilding* building)
{
	//play explosion animation
	cBuilding* topBuilding = getMap()->fields[building->PosX + building->PosY * getMap()->size].getBuildings();
	if (topBuilding && topBuilding->data.isBig)
	{
		addFX (fxExploBig, topBuilding->PosX * 64 + 64, topBuilding->PosY * 64 + 64, 0);
	}
	else
	{
		addFX (fxExploSmall, building->PosX * 64 + 32, building->PosY * 64 + 32, 0);
	}
}
//-------------------------------------------------------------------------------------
int cClient::getTurn() const
{
	return iTurn;
}

//-------------------------------------------------------------------------------------
void cClient::getVictoryConditions (int* turnLimit, int* scoreLimit) const
{
	*turnLimit = this->turnLimit;
	*scoreLimit = this->scoreLimit;
}

//-------------------------------------------------------------------------------------
void cClient::deletePlayer (cPlayer* player)
{
	player->isRemovedFromGame = true;

	// TODO: We do not really delete the player because he may is still referenced somewhere
	// (e.g. in the playersInfo in the gameGUI)
	// or we may need him for some statistics.
	// uncomment this if we can make sure all references have been removed or at least been set to NULL.
	/*for ( unsigned int i = 0; i < getPlayerList()->Size(); i++ )
	{
		if ( player == (*getPlayerList())[i] )
		{
			delete (*getPlayerList())[i];
			getPlayerList()->Delete ( i );
		}
	}*/
}

void cClient::addJob (cJob* job)
{
	//only one job per unit
	releaseJob (job->unit);

	helperJobs.Add (job);
	job->unit->job = job;
}

void cClient::runJobs ()
{
	for (unsigned int i = 0; i < helperJobs.Size(); i++)
	{
		if (!helperJobs[i]->finished)
		{
			helperJobs[i]->run (gameTimer);
		}
		if (helperJobs[i]->finished)
		{
			if (helperJobs[i]->unit)
				helperJobs[i]->unit->job = NULL;
			delete helperJobs[i];
			helperJobs.Delete(i);
			i--;
		}
	}
}

void cClient::releaseJob (cUnit* unit)
{
	if (unit->job)
	{
		unit->job->unit = NULL;
		unit->job->finished = true;
	}
}

void cClient::enableFreezeMode (eFreezeMode mode, int playerNumber)
{
	switch (mode)
	{
	case FREEZE_WAIT_FOR_SERVER:
		freezeModes.waitForServer = true;
		break;
	case FREEZE_WAIT_FOR_OTHERS:
		freezeModes.waitForOthers = true;
		break;
	case FREEZE_PAUSE:
		freezeModes.pause = true;
		break;
	case FREEZE_WAIT_FOR_RECONNECT:
		freezeModes.waitForReconnect = true;
		break;
	case FREEZE_WAIT_FOR_TURNEND:
		freezeModes.waitForTurnEnd = true;
		break;
	case FREEZE_WAIT_FOR_PLAYER:
		freezeModes.waitForPlayer = true;		
		break;
	}

	freezeModes.playerNumber = playerNumber;

	gameGUI.updateInfoTexts();
}

void cClient::disableFreezeMode (eFreezeMode mode)
{
	switch (mode)
	{
	case FREEZE_WAIT_FOR_SERVER:
		freezeModes.waitForServer = false;
		break;
	case FREEZE_WAIT_FOR_OTHERS:
		freezeModes.waitForOthers = false;
		break;
	case FREEZE_PAUSE:
		freezeModes.pause = false;
		break;
	case FREEZE_WAIT_FOR_RECONNECT:
		freezeModes.waitForReconnect = false;
		break;
	case FREEZE_WAIT_FOR_TURNEND:
		freezeModes.waitForTurnEnd = false;
		break;
	case FREEZE_WAIT_FOR_PLAYER:
		freezeModes.waitForPlayer = false;		
		break;
	}

	gameGUI.updateInfoTexts ();
}

bool cClient::isFreezed ()
{
	return	freezeModes.pause			||
			freezeModes.waitForOthers	||
			freezeModes.waitForPlayer	||
			freezeModes.waitForReconnect||
			freezeModes.waitForServer	||
			freezeModes.waitForTurnEnd;
}

bool cClient::getFreezeMode(eFreezeMode mode)
{
	switch (mode)
	{
	case FREEZE_PAUSE:
		return freezeModes.pause;
	case FREEZE_WAIT_FOR_RECONNECT:
		return freezeModes.waitForReconnect;
	case FREEZE_WAIT_FOR_OTHERS:
		return freezeModes.waitForOthers;
	case FREEZE_WAIT_FOR_TURNEND:
		return freezeModes.waitForTurnEnd;
	case FREEZE_WAIT_FOR_PLAYER:
		return freezeModes.waitForPlayer;
	case FREEZE_WAIT_FOR_SERVER:
		return freezeModes.waitForServer;
	default:
		return false;
	}
}

int cClient::getFreezeInfoPlayerNumber ()
{
	return freezeModes.playerNumber;
}

