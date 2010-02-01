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

sMessage::sMessage(std::string const& s, unsigned int const age_)
{
	chars = (int)s.length();
	msg = new char[chars + 1];
	strcpy(msg, s.c_str());
	if (chars > 500) msg[500] = '\0';
	len = font->getTextWide(s);
	age = age_;
}


sMessage::~sMessage()
{
	delete [] msg;
}

sFX::sFX( eFXTyps typ, int x, int y )
{
	this->typ = typ;
	PosX = x;
	PosY = y;
	StartTime = Client->iTimerTime;
	param = 0;
	rocketInfo = NULL;
	smokeInfo = NULL;
	trackInfo = NULL;
	param = 0;

	switch ( typ )
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
	}
}

sFX::~sFX()
{
	if ( rocketInfo ) delete rocketInfo;
	if ( smokeInfo ) delete smokeInfo;
	if ( trackInfo ) delete trackInfo;
}

Uint32 TimerCallback(Uint32 interval, void *arg)
{
	((cClient *)arg)->Timer();
	return interval;
}

cClient::cClient(cMap* const Map, cList<cPlayer*>* const PlayerList) : gameGUI ( NULL, Map )
{
	this->Map = Map;

	this->PlayerList = PlayerList;

	TimerID = SDL_AddTimer ( 50, TimerCallback, this );
	iTimerTime = 0;
	neutralBuildings = NULL;
	iObjectStream = -1;
	bDefeated = false;
	iMsgCoordsX = -1;
	iMsgCoordsY = -1;
	iTurn = 1;
	bWantToEnd = false;
	bUpShowTank = true;
	bUpShowPlane = true;
	bUpShowShip = true;
	bUpShowBuild = true;
	bUpShowTNT = false;
	bAlienTech = false;
	bWaitForOthers = false;
	iTurnTime = 0;
	scoreLimit = turnLimit = 0;
}

cClient::~cClient()
{
	SDL_RemoveTimer ( TimerID );
	StopFXLoop ( iObjectStream );
	while (messages.Size())
	{
		delete messages[0];
		messages.Delete ( 0 );
	}
	while (FXList.Size())
	{
		delete FXList[0];
		FXList.Delete ( 0 );
	}
	while (FXListBottom.Size())
	{
		delete FXListBottom[0];
		FXListBottom.Delete ( 0 );
	}

	for (unsigned int i = 0; i < attackJobs.Size(); i++)
	{
		delete attackJobs[i];
	}

	while ( neutralBuildings )
	{
		cBuilding* nextBuilding = neutralBuildings->next;
		delete neutralBuildings;
		neutralBuildings = nextBuilding;
	}
}

void cClient::sendNetMessage(cNetMessage *message)
{
	message->iPlayerNr = ActivePlayer->Nr;

	Log.write("Client: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	if (!network || network->isHost() )
	{
		//push an event to the lokal server in singleplayer, HotSeat or if this machine is the host
		Server->pushEvent(message->getGameEvent() );
		delete message;
		//Server->pushNetMessage( message );
	}
	else // else send it over the net
	{
		//the client is only connected to one socket
		//so netwwork->send() only sends to the server
		network->send( message->iLength, message->serialize() );
		delete message;
	}
}

void cClient::Timer()
{
	iTimerTime++;
}

void cClient::initPlayer( cPlayer *Player )
{
	ActivePlayer = Player;
	gameGUI.setPlayer ( Player );

	// generate subbase for enemy players
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		if ( (*PlayerList)[i] == ActivePlayer ) continue;
		(*PlayerList)[i]->base.SubBases.Add ( new sSubBase ((*PlayerList)[i]) );
	}
}

void cClient::addMoveJob(cVehicle* vehicle, int iDestOffset, cList<cVehicle*> *group)
{
	if ( vehicle->bIsBeeingAttacked ) return;

	cClientMoveJob *MoveJob = new cClientMoveJob ( vehicle->PosX+vehicle->PosY*Map->size, iDestOffset, vehicle->data.factorAir > 0, vehicle );
	if ( MoveJob->calcPath( group ) )
	{
		sendMoveJob ( MoveJob );
		Log.write(" Client: Added new movejob: VehicleID: " + iToStr ( vehicle->iID ) + ", SrcX: " + iToStr ( vehicle->PosX ) + ", SrcY: " + iToStr ( vehicle->PosY ) + ", DestX: " + iToStr ( MoveJob->DestX ) + ", DestY: " + iToStr ( MoveJob->DestY ), cLog::eLOG_TYPE_NET_DEBUG);
	}
	else
	{
		if ( !vehicle || !vehicle->autoMJob ) //automoving suveyors must not tell this
		{
			if ( random(2) ) PlayVoice(VoiceData.VOINoPath1);
			else PlayVoice ( VoiceData.VOINoPath2 );
		}

		if ( MoveJob->Vehicle )
		{
			MoveJob->Vehicle->ClientMoveJob = NULL;
		}
		delete MoveJob;
	}
}

void cClient::startGroupMove()
{
	int mainPosX = (*gameGUI.getSelVehiclesGroup())[0]->PosX;
	int mainPosY = (*gameGUI.getSelVehiclesGroup())[0]->PosY;
	int mainDestX = mouse->GetKachelOff()%Map->size;
	int mainDestY = mouse->GetKachelOff()/Map->size;

	// copy the selected-units-list
	cList<cVehicle*> group;
	for ( unsigned int i = 0; i < gameGUI.getSelVehiclesGroup()->Size(); i++ ) group.Add( (*gameGUI.getSelVehiclesGroup())[i] );

	// go trough all vehicles in the list
	while ( group.Size() )
	{
		// we will start moving the vehicles in the list with the vehicle that is the closesed to the destination.
		// this will avoid that the units will crash into each other becouse the one infront of them has started
		// his move and the next field is free.
		int shortestWayLength = 0xFFFF;
		int shortestWayVehNum = 0;
		for ( unsigned int i = 0; i < group.Size(); i++ )
		{
			cVehicle *vehicle = group[i];
			int deltaX = vehicle->PosX-mainDestX+vehicle->PosX-mainPosX;
			int deltaY = vehicle->PosY-mainDestY+vehicle->PosY-mainPosY;
			int wayLength = Round ( sqrt ( (double)deltaX*deltaX + deltaY*deltaY ) );

			if ( wayLength < shortestWayLength )
			{
				shortestWayLength = wayLength;
				shortestWayVehNum = i;
			}
		}
		cVehicle *vehicle = group[shortestWayVehNum];
		// add the movejob to the destination of the unit.
		// the formation of the vehicle group will stay as destination formation.
		int destOffset = mainDestX+vehicle->PosX-mainPosX+(mainDestY+vehicle->PosY-mainPosY)*Map->size;
		addMoveJob ( vehicle, destOffset, gameGUI.getSelVehiclesGroup() );
		// delete the unit from the copyed list
		group.Delete ( shortestWayVehNum );
	}
}

void cClient::handleTimer()
{
	//timer50ms: 50ms
	//timer100ms: 100ms
	//timer400ms: 400ms

	static unsigned int iLast = 0;
	timer50ms = false;
	timer100ms = false;
	timer400ms = false;
	if ( iTimerTime != iLast )
	{
		iLast = iTimerTime;
		timer50ms = true;
		if (   iTimerTime & 0x1 ) timer100ms = true;
		if ( ( iTimerTime & 0x3 ) == 3 ) timer400ms = true;
	}
}

void cClient::runFX()
{
	if ( !timer100ms ) return;
	for ( unsigned int i = 0; i < FXList.Size(); i++ )
	{
		sFX* fx = FXList[i];
		switch ( fx->typ )
		{
			case fxRocket:
				{
					sFXRocketInfos *ri= fx->rocketInfo;
					if ( abs ( fx->PosX - ri->DestX ) <64&&abs ( fx->PosY-ri->DestY ) <64 )
					{
						ri->aj->state = cClientAttackJob::FINISHED;
						delete fx;
						FXList.Delete ( i );
						return;
					}

					for ( int k=0; k < 64; k += 8 )
					{
						if ( SettingsData.bAlphaEffects )
						{
							addFX ( fxSmoke, ( int ) ri->fpx, ( int ) ri->fpy,0 );
							gameGUI.drawFX((int)FXList.Size() - 1);
						}
						ri->fpx+=ri->mx*8;
						ri->fpy-=ri->my*8;
					}

					fx->PosX= ( int ) ri->fpx;
					fx->PosY= ( int ) ri->fpy;
				}
				break;
			default:
				break;
		}
	}

	for ( unsigned int i = 0; i < FXListBottom.Size(); i++ )
	{
		sFX* fx = FXListBottom[i];
		switch ( fx->typ )
		{
			case fxTorpedo:
			{
				sFXRocketInfos *ri= fx->rocketInfo;
				if ( abs ( fx->PosX-ri->DestX ) <64&&abs ( fx->PosY-ri->DestY ) <64 )
				{
					ri->aj->state = cClientAttackJob::FINISHED;
					delete fx;
					FXListBottom.Delete ( i );
					return;
				}

				for ( int k=0; k < 64; k += 8 )
				{
					if ( SettingsData.bAlphaEffects )
					{
						addFX ( fxBubbles, ( int ) ri->fpx, ( int ) ri->fpy,0 );
						gameGUI.drawBottomFX( (int)FXListBottom.Size() - 1 );
					}
					ri->fpx+=ri->mx*8;
					ri->fpy-=ri->my*8;
				}

				fx->PosX= ( int ) ( ri->fpx );
				fx->PosY= ( int ) ( ri->fpy );
			}
			default:
				break;
		}

	}
}

// F¸gt einen FX-Effekt ein:
void cClient::addFX ( eFXTyps typ,int x,int y, cClientAttackJob* aj, int iDestOff, int iFireDir )
{
	sFX* n = new sFX(typ, x, y);
	sFXRocketInfos* ri = n->rocketInfo;
	ri->ScrX = x;
	ri->ScrY = y;
	ri->DestX = (iDestOff % Client->Map->size) * 64;
	ri->DestY = (iDestOff / Client->Map->size) * 64;
	ri->aj = aj;
	ri->dir = iFireDir;
	addFX( n );
}

// F¸gt einen FX-Effekt ein:
void cClient::addFX ( eFXTyps typ,int x,int y,int param )
{
	sFX* n = new sFX(typ, x, y);
	n->param = param;
	addFX( n );
}

// F¸gt einen FX-Effekt ein:
void cClient::addFX ( sFX* n )
{

	if ( n->typ==fxTracks||n->typ==fxTorpedo||n->typ==fxBubbles||n->typ==fxCorpse )
	{
		FXListBottom.Add ( n );
	}
	else
	{
		FXList.Add ( n );
	}
	switch ( n->typ )
	{
		case fxExploAir:
			int nr;
			nr = random(3);
			if ( nr==0 )
			{
				PlayFX ( SoundData.EXPSmall0 );
			}
			else if ( nr==1 )
			{
				PlayFX ( SoundData.EXPSmall1 );
			}
			else
			{
				PlayFX ( SoundData.EXPSmall2 );
			}
			break;
		case fxExploSmall:
		case fxExploWater:
			if ( Map->IsWater ( n->PosX/64+ ( n->PosY/64 ) *Map->size ) )
			{
				int nr;
				nr = random(3);
				if ( nr==0 )
				{
					PlayFX ( SoundData.EXPSmallWet0 );
				}
				else if ( nr==1 )
				{
					PlayFX ( SoundData.EXPSmallWet1 );
				}
				else
				{
					PlayFX ( SoundData.EXPSmallWet2 );
				}
			}
			else
			{
				int nr;
				nr =  random(3);
				if ( nr==0 )
				{
					PlayFX ( SoundData.EXPSmall0 );
				}
				else if ( nr==1 )
				{
					PlayFX ( SoundData.EXPSmall1 );
				}
				else
				{
					PlayFX ( SoundData.EXPSmall2 );
				}
			}
			break;
		case fxExploBig:
			if ( Map->IsWater ( n->PosX/64+ ( n->PosY/64 ) *Map->size ) )
			{
				if (random(2))
				{
					PlayFX ( SoundData.EXPBigWet0 );
				}
				else
				{
					PlayFX ( SoundData.EXPBigWet1 );
				}
			}
			else
			{
				int nr;
				nr = random(4);
				if ( nr==0 )
				{
					PlayFX ( SoundData.EXPBig0 );
				}
				else if ( nr==1 )
				{
					PlayFX ( SoundData.EXPBig1 );
				}
				else if ( nr==2 )
				{
					PlayFX ( SoundData.EXPBig2 );
				}
				else
				{
					PlayFX ( SoundData.EXPBig3 );
				}
			}
			break;
		case fxRocket:
		case fxTorpedo:
		{
			sFXRocketInfos *ri;
			int dx,dy;
			ri= n->rocketInfo;
			ri->fpx=(float)n->PosX;
			ri->fpy=(float)n->PosY;
			dx=ri->ScrX-ri->DestX;
			dy=ri->ScrY-ri->DestY;
			if ( abs ( dx ) >abs ( dy ) )
			{
				if ( ri->ScrX>ri->DestX ) ri->mx=-1;
				else ri->mx=1;
				ri->my=dy/ ( float ) dx* ( -ri->mx );
			}
			else
			{
				if ( ri->ScrY<ri->DestY ) ri->my=-1;
				else ri->my=1;
				ri->mx=dx/ ( float ) dy* ( -ri->my );
			}
			break;
		}
		case fxDarkSmoke:
		{
			float x,y,ax,ay;
			sFXDarkSmoke *dsi = n->smokeInfo;
			dsi->alpha=n->param;
			if ( dsi->alpha>150 ) dsi->alpha=150;
			dsi->fx=(float)n->PosX;
			dsi->fy=(float)n->PosY;

			ax=x=sin ( gameGUI.getWindDir() );
			ay=y=cos ( gameGUI.getWindDir() );
			if ( ax<0 ) ax=-ax;
			if ( ay<0 ) ay=-ay;
			if ( ax>ay )
			{
				dsi->dx = (float)(x * 2 + random(5)        / 10.0);
				dsi->dy = (float)(y * 2 + (random(15) - 7) / 14.0);
			}
			else
			{
				dsi->dx = (float)(x * 2 + (random(15) - 7) / 14.0);
				dsi->dy = (float)(y * 2 + random(5)        / 10.0);
			}
			break;
		}
		case fxTracks:
		{
			sFXTracks *tri = n->trackInfo;
			tri->alpha = 100;
			tri->dir = n->param;
			break;
		}
		case fxCorpse:
			n->param=255;
			break;
		case fxAbsorb:
			PlayFX ( SoundData.SNDAbsorb );
			break;
		default:
			break;
	}
}

// Adds an message to be displayed in the game
void cClient::addMessage ( string sMsg )
{
	sMessage* const Message = new sMessage(sMsg, SDL_GetTicks() );
	messages.Add(Message);
	if(SettingsData.bDebug) Log.write(Message->msg, cLog::eLOG_TYPE_DEBUG);
}

// displays a message with 'goto' coordinates
string cClient::addCoords (const string msg,int x,int y )
{
 	stringstream strStream;
 	//e.g. [85,22] missel MK I is under attack (F1)
 	strStream << "[" << x << "," << y << "] " << msg << " (" << GetKeyString ( KeysList.KeyJumpToAction ) << ")";
	Client->addMessage ( strStream.str() );
	iMsgCoordsX=x;
	iMsgCoordsY=y;
	return strStream.str();
}

void cClient::handleMessages()
{
	int iHeight = 0;
	sMessage *message;
	if (messages.Size() == 0) return;
	// Alle alten Nachrichten lˆschen:
	for (int i = (int)messages.Size() - 1; i >= 0; i--)
	{
		message = messages[i];
		if ( message->age+MSG_TICKS < SDL_GetTicks() || iHeight > 200 )
		{
			delete message;
			messages.Delete ( i );
			continue;
		}
		iHeight += 17 + font->getFontHeight() * ( message->len  / (SettingsData.iScreenW - 300) );
	}
}

int cClient::HandleNetMessage( cNetMessage* message )
{
	if ( message->iType != DEBUG_CHECK_VEHICLE_POSITIONS )		//do not pollute log file with debug events
		Log.write("Client: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	switch ( message->iType )
	{
	case DEBUG_CHECK_VEHICLE_POSITIONS:
		checkVehiclePositions( message );
		break;
	case GAME_EV_LOST_CONNECTION:
		{
			string msgString = lngPack.i18n ( "Text~Multiplayer~Lost_Connection", "server" );
			addMessage( msgString );
			ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
		}
		break;
	case GAME_EV_CHAT_SERVER:
		switch (message->popChar())
		{
		case USER_MESSAGE:
			{
				PlayFX ( SoundData.SNDChat );
				string msgString = message->popString();
				addMessage( msgString );
				ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_CHAT );
			}
			break;
		case SERVER_ERROR_MESSAGE:
			{
				PlayFX ( SoundData.SNDQuitsch );
				string msgString = lngPack.i18n( message->popString() );
				addMessage( msgString );
				ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
			}
			break;
		case SERVER_INFO_MESSAGE:
			{
				string translationpath = message->popString();
				string inserttext = message->popString();
				string msgString;
				if ( !inserttext.compare ( "" ) ) msgString = lngPack.i18n( translationpath );
				else msgString = lngPack.i18n( translationpath, inserttext );
				addMessage( msgString );
				ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
			}
			break;
		}
		break;
	case GAME_EV_PLAYER_CLANS:
			for (unsigned int i = 0; i < PlayerList->Size (); i++)
			{
				int playerNr = message->popChar();
				int clan = message->popChar ();

				cPlayer* player = getPlayerFromNumber( playerNr );
				player->setClan (clan);
			}
		break;
	case GAME_EV_ADD_BUILDING:
		{
			cBuilding *AddedBuilding;
			sID UnitID;
			bool Init = message->popBool();
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			if ( !Player )
			{
				Log.write("Player not found", cLog::eLOG_TYPE_NET_ERROR );
				break;
			}
			UnitID.iFirstPart = message->popInt16();
			UnitID.iSecondPart = message->popInt16();
			int PosY = message->popInt16();
			int PosX = message->popInt16();

			AddedBuilding = Player->addBuilding( PosX, PosY, UnitID.getBuilding(Player) );
			AddedBuilding->iID = message->popInt16();

			addUnit ( PosX, PosY, AddedBuilding, Init );

			Player->base.addBuilding( AddedBuilding, false );

			// play placesound if it is a mine
			if ( UnitID == specialIDLandMine && Player == ActivePlayer ) PlayFX ( SoundData.SNDLandMinePlace );
			else if ( UnitID == specialIDSeaMine && Player == ActivePlayer ) PlayFX ( SoundData.SNDSeaMinePlace );
		}
		break;
	case GAME_EV_ADD_VEHICLE:
		{
			cVehicle *AddedVehicle;
			sID UnitID;
			bool Init = message->popBool();
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			if ( !Player )
			{
				Log.write("Player not found", cLog::eLOG_TYPE_NET_ERROR );
				break;
			}

			UnitID.iFirstPart = message->popInt16();
			UnitID.iSecondPart = message->popInt16();
			int PosY = message->popInt16();
			int PosX = message->popInt16();

			AddedVehicle = Player->AddVehicle(PosX, PosY, UnitID.getVehicle(Player));
			AddedVehicle->iID = message->popInt16();
			bool bAddToMap = message->popBool();

			addUnit ( PosX, PosY, AddedVehicle, Init, bAddToMap );
		}
		break;
	case GAME_EV_DEL_BUILDING:
		{
			cBuilding *Building;

			Building = getBuildingFromID ( message->popInt16() );

			if ( Building )
			{
				// play clearsound if it is a mine
				if ( Building->owner && Building->data.ID == specialIDLandMine && Building->owner == ActivePlayer ) PlayFX ( SoundData.SNDLandMineClear );
				else if ( Building->owner && Building->data.ID == specialIDSeaMine && Building->owner == ActivePlayer ) PlayFX ( SoundData.SNDSeaMineClear );

				deleteUnit ( Building );
			}
		}
		break;
	case GAME_EV_DEL_VEHICLE:
		{
			cVehicle *Vehicle;

			Vehicle = getVehicleFromID ( message->popInt16() );

			if ( Vehicle )
			{
				deleteUnit ( Vehicle );
			}
		}
		break;
	case GAME_EV_ADD_ENEM_VEHICLE:
		{
			cVehicle *AddedVehicle;
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			if ( !Player )
			{
				Log.write("Player not found", cLog::eLOG_TYPE_NET_ERROR );
				break;
			}
			sID UnitID;

			UnitID.iFirstPart = message->popInt16();
			UnitID.iSecondPart = message->popInt16();
			int iPosY = message->popInt16();
			int iPosX = message->popInt16();
			AddedVehicle = Player->AddVehicle(iPosX, iPosY, UnitID.getVehicle(Player) );

			AddedVehicle->dir = message->popInt16();
			AddedVehicle->iID = message->popInt16();

			addUnit ( iPosX, iPosY, AddedVehicle, false );

			// make report
			string message = AddedVehicle->getDisplayName() + " (" + Player->name + ") " + lngPack.i18n ( "Text~Comp~Detected" );
			Client->ActivePlayer->addSavedReport ( Client->addCoords( message, iPosX, iPosY ), sSavedReportMessage::REPORT_TYPE_UNIT, UnitID, iPosX, iPosY );
			if ( random( 2 ) == 0 ) PlayVoice ( VoiceData.VOIDetected1 );
			else PlayVoice ( VoiceData.VOIDetected2 );
		}
		break;
	case GAME_EV_ADD_ENEM_BUILDING:
		{
			cBuilding *AddedBuilding;
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			if ( !Player )
			{
				Log.write("Player not found", cLog::eLOG_TYPE_NET_ERROR );
				break;
			}
			sID UnitID;

			UnitID.iFirstPart = message->popInt16();
			UnitID.iSecondPart = message->popInt16();
			int iPosY = message->popInt16();
			int iPosX = message->popInt16();

			AddedBuilding = Player->addBuilding( iPosX, iPosY, UnitID.getBuilding(Player) );
			AddedBuilding->iID = message->popInt16();
			addUnit ( iPosX, iPosY, AddedBuilding, false );

			Player->base.SubBases[0]->buildings.Add ( AddedBuilding );
			AddedBuilding->SubBase = Player->base.SubBases[0];

			AddedBuilding->updateNeighbours( Map );
		}
		break;
	case GAME_EV_WAIT_FOR:
		{
			int nextPlayerNum = message->popInt16();

			if ( nextPlayerNum != ActivePlayer->Nr )
			{
				bWaitForOthers = true;
				gameGUI.setInfoTexts ( lngPack.i18n ( "Text~Multiplayer~Wait_Until", getPlayerFromNumber( nextPlayerNum )->name ), "" );
				gameGUI.setEndButtonLock ( true );
			}
		}
		break;
	case GAME_EV_MAKE_TURNEND:
		{
			int iNextPlayerNum = message->popInt16();
			bool bWaitForNextPlayer = message->popBool();
			bool bEndTurn = message->popBool();

			if ( bEndTurn )
			{
				iTurn++;
				iTurnTime = 0;
				gameGUI.updateTurn( iTurn );
				if (!bWaitForNextPlayer ) gameGUI.setEndButtonLock( false );
				bWantToEnd = false;
				gameGUI.updateTurnTime ( -1 );
				Log.write("######### Round " + iToStr( iTurn ) + " ###########", cLog::eLOG_TYPE_NET_DEBUG );
			}

			if ( bWaitForNextPlayer )
			{
				if ( iNextPlayerNum != ActivePlayer->Nr )
				{
					bWaitForOthers = true;
					gameGUI.setInfoTexts ( lngPack.i18n ( "Text~Multiplayer~Wait_Until", getPlayerFromNumber( iNextPlayerNum )->name ), "" );
				}
				else
				{
					bWaitForOthers = false;
					gameGUI.setInfoTexts ( "", "" );
					gameGUI.setEndButtonLock( false );
				}
			}
			else if ( iNextPlayerNum != -1 )
			{
				makeHotSeatEnd( iNextPlayerNum );
			}
		}
		break;
	case GAME_EV_FINISHED_TURN:
		{
			int iPlayerNum = message->popInt16();
			int iTimeDelay = message->popInt16();

			cPlayer *Player = getPlayerFromNumber( iPlayerNum );
			if ( Player == NULL && iPlayerNum != -1 )
			{
				Log.write(" Client: Player with nr " + iToStr(iPlayerNum) + " has finished turn, but can't find him", cLog::eLOG_TYPE_NET_WARNING );
				break;
			}

			//HACK SHOWFINISHEDPLAYERS player finished his turn
			/*if ( Player )
			{
				Player->bFinishedTurn=true;
				for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
				{
					if ( Player == (*PlayerList)[i] )
					{
						Hud.ExtraPlayers(Player->name, GetColorNr(Player->color), i, Player->bFinishedTurn);
						break;
					}
				}
			}*/


			if ( iTimeDelay != -1 )
			{
				if ( iPlayerNum != ActivePlayer->Nr && iPlayerNum != -1  )
				{
					string msgString = lngPack.i18n( "Text~Multiplayer~Player_Turn_End", Player->name) + ". " + lngPack.i18n( "Text~Multiplayer~Deadline", iToStr( iTimeDelay ) );
					addMessage( msgString );
					ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
				}
				iTurnTime = iTimeDelay;
				iStartTurnTime = SDL_GetTicks();
			}
			else if ( iPlayerNum != ActivePlayer->Nr && iPlayerNum != -1  )
			{
				string msgString = lngPack.i18n( "Text~Multiplayer~Player_Turn_End", Player->name);
				addMessage( msgString );
				ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
			}
		}
		break;
	case GAME_EV_UNIT_DATA:
		{
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			(void)Player; // TODO use me
			sUnitData *Data;

			bool bWasBuilding = false;
			int iID = message->popInt16();

			bool bVehicle = message->popBool();
			int iPosY = message->popInt16();
			int iPosX = message->popInt16();

			cVehicle *Vehicle = NULL;
			cBuilding *Building = NULL;

			Log.write(" Client: Received Unit Data: Vehicle: " + iToStr ( (int)bVehicle ) + ", ID: " + iToStr ( iID ) + ", XPos: " + iToStr ( iPosX ) + ", YPos: " +iToStr ( iPosY ), cLog::eLOG_TYPE_NET_DEBUG);
			// unit is a vehicle
			if ( bVehicle )
			{
				bool bBig = message->popBool();
				Vehicle = getVehicleFromID ( iID );

				if ( !Vehicle )
				{
					Log.write(" Client: Unknown vehicle with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
					// TODO: Request sync of vehicle
					break;
				}
				if ( Vehicle->PosX != iPosX || Vehicle->PosY != iPosY || Vehicle->data.isBig != bBig )
				{
					// if the vehicle is moving it is normal that the positions are not the same,
					// when the vehicle was building it is also normal that the position should be changed
					// so the log message will just be an debug one
					int iLogType = cLog::eLOG_TYPE_NET_WARNING;
					if ( Vehicle->IsBuilding || Vehicle->IsClearing || Vehicle->moving ) iLogType = cLog::eLOG_TYPE_NET_DEBUG;
					Log.write(" Client: Vehicle identificated by ID (" + iToStr( iID ) + ") but has wrong position [IS: X" + iToStr( Vehicle->PosX ) + " Y" + iToStr( Vehicle->PosY ) + "; SHOULD: X" + iToStr( iPosX ) + " Y" + iToStr( iPosY ) + "]", iLogType );

					// set to server position if vehicle is not moving
					if ( !Vehicle->MoveJobActive )
					{
						Map->moveVehicle( Vehicle, iPosX, iPosY );
						if ( bBig ) Map->moveVehicleBig( Vehicle, iPosX, iPosY );
						Vehicle->owner->DoScan();
					}
				}

				if ( message->popBool() ) Vehicle->changeName ( message->popString() );

				Vehicle->bIsBeeingAttacked = message->popBool();
				bool bWasDisabled = Vehicle->Disabled > 0;
				Vehicle->Disabled = message->popInt16();
				Vehicle->CommandoRank = message->popInt16();
				Vehicle->IsClearing = message->popBool();
				bWasBuilding = Vehicle->IsBuilding;
				Vehicle->IsBuilding = message->popBool();
				Vehicle->BuildRounds = message->popInt16();
				Vehicle->ClearingRounds = message->popInt16();
				Vehicle->bSentryStatus = message->popBool();

				if ( Vehicle->Disabled > 0 != bWasDisabled ) Vehicle->owner->DoScan();
				Data = &Vehicle->data;
			}
			else
			{
				Building = getBuildingFromID ( iID );
				if ( !Building )
				{
					Log.write(" Client: Unknown building with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
					// TODO: Request sync of building
					break;
				}

				if ( message->popBool() ) Building->changeName ( message->popString() );

				bool bWasDisabled = Building->Disabled > 0;
				Building->Disabled = message->popInt16();
				Building->researchArea = message->popInt16();
				Building->IsWorking = message->popBool();
				Building->bSentryStatus = message->popBool();
				Building->points = message->popInt16();

				if ( Building->Disabled > 0 != bWasDisabled ) Building->owner->DoScan();
				Data = &Building->data;
			}

			Data->buildCosts = message->popInt16();
			Data->ammoCur = message->popInt16();
			Data->ammoMax = message->popInt16();
			Data->storageResCur = message->popInt16();
			Data->storageResMax = message->popInt16();
			Data->storageUnitsCur = message->popInt16();
			Data->storageUnitsMax = message->popInt16();
			Data->damage = message->popInt16();
			Data->shotsCur = message->popInt16();
			Data->shotsMax = message->popInt16();
			Data->range = message->popInt16();
			Data->scan = message->popInt16();
			Data->armor = message->popInt16();
			Data->hitpointsCur = message->popInt16();
			Data->hitpointsMax = message->popInt16();
			Data->version = message->popInt16();

			if ( bVehicle )
			{
				if ( Data->canPlaceMines )
				{
					if ( Data->storageResCur <= 0 ) Vehicle->LayMines = false;
					if ( Data->storageResCur >= Data->storageResMax ) Vehicle->ClearMines = false;
				}
				Data->speedCur = message->popInt16();
				Data->speedMax = message->popInt16();

				if ( bWasBuilding && !Vehicle->IsBuilding && Vehicle == Client->gameGUI.getSelVehicle() ) StopFXLoop ( iObjectStream );
			}
		}
		break;
	case GAME_EV_SPECIFIC_UNIT_DATA:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( !Vehicle ) break;
			Vehicle->dir = message->popInt16();
			Vehicle->BuildingTyp.iFirstPart = message->popInt16();
			Vehicle->BuildingTyp.iSecondPart = message->popInt16();
			Vehicle->BuildPath = message->popBool();
			Vehicle->BandX = message->popInt16();
			Vehicle->BandY = message->popInt16();
		}
		break;
	case GAME_EV_DO_START_WORK:
		{
			int iID = message->popInt32();

			cBuilding* building = getBuildingFromID( iID);
			if ( building == NULL )
			{
				Log.write(" Client: Can't start work of building: Unknown building with id: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_ERROR);
				// TODO: Request sync of building
				break;
			}

			building->ClientStartWork();
		}
		break;
	case GAME_EV_DO_STOP_WORK:
		{
			int iID = message->popInt32();
			cBuilding* building = getBuildingFromID(iID);
			if ( building == NULL )
			{
				Log.write(" Client: Can't stop work of building: Unknown building with id: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
				// TODO: Request sync of building
				break;
			}

			building->ClientStopWork();
		}
		break;
	case GAME_EV_MOVE_JOB_SERVER:
		{
			int iVehicleID = message->popInt16();
			int iSrcOff = message->popInt32();
			int iDestOff = message->popInt32();
			bool bPlane = message->popBool();

			cVehicle *Vehicle = getVehicleFromID ( iVehicleID );
			if ( Vehicle == NULL )
			{
				Log.write(" Client: Can't find vehicle with id " + iToStr ( iVehicleID ) + " for movejob from " +  iToStr (iSrcOff%Map->size) + "x" + iToStr (iSrcOff/Map->size) + " to " + iToStr (iDestOff%Map->size) + "x" + iToStr (iDestOff/Map->size), cLog::eLOG_TYPE_NET_WARNING);
				// TODO: request sync of vehicle
				break;
			}

			cClientMoveJob *MoveJob = new cClientMoveJob ( iSrcOff, iDestOff, bPlane, Vehicle );
			if ( !MoveJob->generateFromMessage ( message ) ) break;
			Log.write(" Client: Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
		}
		break;
	case GAME_EV_NEXT_MOVE:
		{
			int iID = message->popInt16();
			int iDestX = message->popInt16();
			int iDestY = message->popInt16();
			int iType = message->popChar();
			int iSavedSpeed = -1;
			if ( iType == MJOB_STOP ) iSavedSpeed = message->popChar();

			Log.write(" Client: Received information for next move: ID: " + iToStr ( iID ) + ", SrcX: " + iToStr( iDestX ) + ", SrcY: " + iToStr( iDestY ) + ", Type: " + iToStr ( iType ), cLog::eLOG_TYPE_NET_DEBUG);

			cVehicle *Vehicle = getVehicleFromID ( iID );
			if ( Vehicle && Vehicle->ClientMoveJob )
			{
				Vehicle->ClientMoveJob->handleNextMove( iDestX, iDestY, iType, iSavedSpeed );
			}
			else
			{
				if ( Vehicle == NULL ) Log.write(" Client: Can't find vehicle with ID " + iToStr(iID), cLog::eLOG_TYPE_NET_WARNING);
				else Log.write(" Client: Vehicle with ID " + iToStr(iID) + "has no movejob", cLog::eLOG_TYPE_NET_WARNING);
				// TODO: request sync of vehicle
			}
		}
		break;
	case GAME_EV_ATTACKJOB_LOCK_TARGET:
		{
			cClientAttackJob::lockTarget( message );
		}
		break;
	case GAME_EV_ATTACKJOB_FIRE:
		{
			cClientAttackJob* job = new cClientAttackJob( message );
			Client->attackJobs.Add( job );
		}
		break;
	case GAME_EV_ATTACKJOB_IMPACT:
		{
			int id = message->popInt16();
			int remainingHP = message->popInt16();
			int offset = message->popInt32();
			cClientAttackJob::makeImpact( offset, remainingHP, id );
			break;
		}
	case GAME_EV_RESOURCES:
		{
			int iCount = message->popInt16();
			for ( int i = 0; i < iCount; i++ )
			{
				int iOff = message->popInt32();
				ActivePlayer->ResourceMap[iOff] = 1;

				Map->Resources[iOff].typ = (unsigned char)message->popInt16();
				Map->Resources[iOff].value = (unsigned char)message->popInt16();
			}
		}
		break;
	case GAME_EV_BUILD_ANSWER:
		{
			cVehicle *Vehicle;
			bool bOK = message->popBool();
			int iID = message->popInt16();

			Vehicle = getVehicleFromID ( iID );
			if ( Vehicle == NULL )
			{
				Log.write(" Client: Vehicle can't start building: Unknown vehicle with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
				// TODO: Request sync of vehicle
				break;
			}

			if ( !bOK )
			{
				if ( !Vehicle->BuildPath && Vehicle->owner == ActivePlayer )
				{
					string msgString = lngPack.i18n( "Text~Comp~Producing_Err");
					addMessage( msgString );
					ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
				}
				Vehicle->BuildRounds = 0;
				Vehicle->BuildingTyp.iFirstPart = 0;
				Vehicle->BuildingTyp.iSecondPart = 0;
				Vehicle->BuildPath = false;
				Vehicle->BandX = 0;
				Vehicle->BandY = 0;
				break;
			}

			if ( Vehicle->IsBuilding ) Log.write(" Client: Vehicle is already building", cLog::eLOG_TYPE_NET_ERROR );

			int iBuildX = message->popInt16();
			int iBuildY = message->popInt16();
			bool bBuildBig = message->popBool();

			if ( bBuildBig )
			{
				Map->moveVehicleBig(Vehicle, iBuildX, iBuildY );
				Vehicle->owner->DoScan();

				Vehicle->BigBetonAlpha = 10;
			}
			else
			{
				Map->moveVehicle(Vehicle, iBuildX, iBuildY );
				Vehicle->owner->DoScan();
			}

			if ( Vehicle->owner == ActivePlayer )
			{
				Vehicle->BuildingTyp.iFirstPart = message->popInt16();
				Vehicle->BuildingTyp.iSecondPart = message->popInt16();
				Vehicle->BuildRounds = message->popInt16();
				Vehicle->BuildPath = message->popBool();
				Vehicle->BandX = message->popInt16();
				Vehicle->BandY = message->popInt16();
			}

			Vehicle->IsBuilding = true;

			if ( Vehicle == gameGUI.getSelVehicle() )
			{
				StopFXLoop ( iObjectStream );
				iObjectStream = Vehicle->playStream();
			}

			if ( Vehicle->ClientMoveJob ) Vehicle->ClientMoveJob->release();
		}
		break;
	case GAME_EV_STOP_BUILD:
		{
			int iID = message->popInt16();
			cVehicle *Vehicle = getVehicleFromID ( iID );
			if ( Vehicle == NULL )
			{
				Log.write(" Client: Can't stop building: Unknown vehicle with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
				// TODO: Request sync of vehicle
				break;
			}

			int iNewPos = message->popInt32();

			if ( Vehicle->data.isBig )
			{
				Map->moveVehicle(Vehicle, iNewPos );
				Vehicle->owner->DoScan();
			}

			Vehicle->IsBuilding = false;
			Vehicle->BuildPath = false;

			if ( gameGUI.getSelVehicle() == Vehicle )
			{
				StopFXLoop ( iObjectStream );
				iObjectStream = Vehicle->playStream();
			}
		}
		break;
	case GAME_EV_SUBBASE_VALUES:
		{
			int iID = message->popInt16();
			sSubBase *SubBase = getSubBaseFromID ( iID );
			if ( SubBase == NULL )
			{
				Log.write(" Client: Can't add subbase values: Unknown subbase with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
				// TODO: Request sync of subbases
				break;
			}

			SubBase->HumanProd = message->popInt16();
			SubBase->MaxHumanNeed = message->popInt16();
			SubBase->HumanNeed = message->popInt16();
			SubBase->OilProd = message->popInt16();
			SubBase->MaxOilNeed = message->popInt16();
			SubBase->OilNeed = message->popInt16();
			SubBase->MaxOil = message->popInt16();
			SubBase->Oil = message->popInt16();
			SubBase->GoldProd = message->popInt16();
			SubBase->MaxGoldNeed = message->popInt16();
			SubBase->GoldNeed = message->popInt16();
			SubBase->MaxGold = message->popInt16();
			SubBase->Gold = message->popInt16();
			SubBase->MetalProd = message->popInt16();
			SubBase->MaxMetalNeed = message->popInt16();
			SubBase->MetalNeed = message->popInt16();
			SubBase->MaxMetal = message->popInt16();
			SubBase->Metal = message->popInt16();
			SubBase->MaxEnergyNeed  = message->popInt16();
			SubBase->MaxEnergyProd = message->popInt16();
			SubBase->EnergyNeed = message->popInt16();
			SubBase->EnergyProd = message->popInt16();

			//temporary debug check
			if ( SubBase->getGoldProd() < SubBase->getMaxAllowedGoldProd() ||
				 SubBase->getMetalProd() < SubBase->getMaxAllowedMetalProd() ||
				 SubBase->getOilProd() < SubBase->getMaxAllowedOilProd() )
			{
				Log.write(" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
			}

		}
		break;
	case GAME_EV_BUILDLIST:
		{
			int iID = message->popInt16();
			cBuilding *Building = getBuildingFromID ( iID );
			if ( Building == NULL )
			{
				Log.write(" Client: Can't set buildlist: Unknown building with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
				// TODO: Request sync of building
				break;
			}

			while (Building->BuildList->Size())
			{
				delete (*Building->BuildList)[0];
				Building->BuildList->Delete( 0 );
			}
			int iCount = message->popInt16();
			for ( int i = 0; i < iCount; i++ )
			{
				sBuildList *BuildListItem = new sBuildList;
				BuildListItem->type.iFirstPart = message->popInt16();
				BuildListItem->type.iSecondPart = message->popInt16();
				BuildListItem->metall_remaining = message->popInt16();
				Building->BuildList->Add ( BuildListItem );
			}

			Building->MetalPerRound = message->popInt16();
			Building->BuildSpeed = message->popInt16();
			Building->RepeatBuild = message->popBool();
		}
		break;
	case GAME_EV_MINE_PRODUCE_VALUES:
		{
			int iID = message->popInt16();
			cBuilding *Building = getBuildingFromID ( iID );
			if ( Building == NULL )
			{
				Log.write(" Client: Can't set produce values of building: Unknown building with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
				// TODO: Request sync of building
				break;
			}

			Building->MaxMetalProd = message->popInt16();
			Building->MaxOilProd = message->popInt16();
			Building->MaxGoldProd = message->popInt16();
		}
		break;
	case GAME_EV_TURN_REPORT:
		{
			string sReportMsg = "";
			string sTmp;
			int iCount = 0;
			bool playVoice = false;

			int iReportAnz = message->popInt16();
			while ( iReportAnz )
			{
				sID Type;
				Type.iFirstPart = message->popInt16();
				Type.iSecondPart = message->popInt16();
				int iAnz = message->popInt16();
				if ( iCount ) sReportMsg += ", ";
				iCount += iAnz;
				sTmp = iToStr( iAnz ) + " " + Type.getUnitDataOriginalVersion()->name;
				sReportMsg += iAnz > 1 ? sTmp : Type.getUnitDataOriginalVersion()->name;
				if ( Type.getUnitDataOriginalVersion()->surfacePosition == sUnitData::SURFACE_POS_GROUND ) playVoice = true;
				iReportAnz--;
			}

			bool bFinishedResearch = message->popBool();
			ActivePlayer->reportResearchFinished = bFinishedResearch;
			if ( ( iCount == 0  || !playVoice ) && !bFinishedResearch ) PlayVoice ( VoiceData.VOIStartNone );
			if ( iCount == 1 )
			{
				sReportMsg += " " + lngPack.i18n( "Text~Comp~Finished") + ".";
				if ( !bFinishedResearch && playVoice ) PlayVoice ( VoiceData.VOIStartOne );
			}
			else if ( iCount > 1 )
			{
				sReportMsg += " " + lngPack.i18n( "Text~Comp~Finished2") + ".";
				if ( !bFinishedResearch && playVoice ) PlayVoice ( VoiceData.VOIStartMore );
			}
			addMessage( lngPack.i18n( "Text~Comp~Turn_Start") + " " + iToStr( iTurn ) );
			if ( sReportMsg.length() > 0 ) addMessage( sReportMsg.c_str() );
			if ( bFinishedResearch )
			{
				PlayVoice ( VoiceData.VOIResearchComplete );
				//FIXME: Ticket #196
				addMessage (lngPack.i18n( "Text~Context~Research") + " " + lngPack.i18n( "Text~Comp~Finished"));
			}

			// Save the report
			string msgString = lngPack.i18n( "Text~Comp~Turn_Start") + " " + iToStr( iTurn ) + "\n";
			if ( sReportMsg.length() > 0 ) msgString += sReportMsg + "\n";
			if ( bFinishedResearch ) msgString += lngPack.i18n( "Text~Context~Research") + " " + lngPack.i18n( "Text~Comp~Finished") + "\n";
			ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );

			//HACK SHOWFINISHEDPLAYERS reset finished turn for all players since a new turn started right now
			/*for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				cPlayer *Player = (*Client->PlayerList)[i];
				if(Player)
				{
					Player->bFinishedTurn=false;
					Hud.ExtraPlayers(Player->name, GetColorNr(Player->color), i, Player->bFinishedTurn);
				}
			}*/
		}
		break;
	case GAME_EV_MARK_LOG:
		{
			Log.write("=============================================================================================", cLog::eLOG_TYPE_NET_DEBUG);
			Log.write( message->popString(), cLog::eLOG_TYPE_NET_DEBUG );
			Log.write("=============================================================================================", cLog::eLOG_TYPE_NET_DEBUG);
		}
		break;
	case GAME_EV_SUPPLY:
		{
			int iType = message->popChar ();
			if ( message->popBool () )
			{
				int iID = message->popInt16();
				cVehicle *DestVehicle = getVehicleFromID ( iID );
				if ( !DestVehicle )
				{
					Log.write(" Client: Can't supply vehicle: Unknown vehicle with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
					// TODO: Request sync of vehicle
					break;
				}
				if ( iType == SUPPLY_TYPE_REARM ) DestVehicle->data.ammoCur = message->popInt16();
				else DestVehicle->data.hitpointsCur = message->popInt16();
				if ( DestVehicle->Loaded )
				{
					// get the building which has loaded the unit
					cBuilding *Building = DestVehicle->owner->BuildingList;
					while ( Building )
					{
						bool found = false;
						for ( unsigned int i = 0; i < Building->StoredVehicles.Size(); i++ )
						{
							if ( Building->StoredVehicles[i] == DestVehicle )
							{
								found = true;
								break;
							}
						}
						if ( found ) break;
						Building = Building->next;
					}
					if ( Building != NULL && ActiveMenu != NULL )
					{
						cStorageMenu *storageMenu = dynamic_cast<cStorageMenu*>(ActiveMenu);
						if ( storageMenu )
						{
							storageMenu->resetInfos();
							storageMenu->draw();
						}
					}
				}
			}
			else
			{
				int iID = message->popInt16();
				cBuilding *DestBuilding = getBuildingFromID ( iID );
				if ( !DestBuilding )
				{
					Log.write(" Client: Can't supply building: Unknown building with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
					// TODO: Request sync of building
					break;
				}
				if ( iType == SUPPLY_TYPE_REARM ) DestBuilding->data.ammoCur = message->popInt16();
				else DestBuilding->data.hitpointsCur = message->popInt16();
			}
			if ( iType == SUPPLY_TYPE_REARM )
			{
				PlayVoice ( VoiceData.VOILoaded );
				PlayFX ( SoundData.SNDReload );
			}
			else
			{
				PlayVoice ( VoiceData.VOIRepaired );
				PlayFX ( SoundData.SNDRepair );
			}
		}
		break;
	case GAME_EV_ADD_RUBBLE:
		{
			cBuilding* rubble = new cBuilding( NULL, NULL, NULL );
			rubble->next = neutralBuildings;
			if ( neutralBuildings ) neutralBuildings->prev = rubble;
			neutralBuildings = rubble;
			rubble->prev = NULL;

			rubble->data.isBig = message->popBool();
			rubble->RubbleTyp = message->popInt16();
			rubble->RubbleValue = message->popInt16();
			rubble->iID = message->popInt16();
			rubble->PosY = message->popInt16();
			rubble->PosX = message->popInt16();

			Map->addBuilding( rubble, rubble->PosX, rubble->PosY);
		}
		break;
	case GAME_EV_DETECTION_STATE:
		{
			int id = message->popInt32();
			cVehicle* vehicle = getVehicleFromID( id );
			if ( vehicle == NULL )
			{
				Log.write(" Client: Vehicle (ID: " + iToStr(id) + ") not found", cLog::eLOG_TYPE_NET_ERROR);
				break;
			}
			bool detected = message->popBool();
			if ( detected )
			{
				//mark vehicle as detected with size of DetectedByPlayerList > 0
				vehicle->DetectedByPlayerList.Add(NULL);
			}
			else
			{
				while ( vehicle->DetectedByPlayerList.Size() > 0 ) vehicle->DetectedByPlayerList.Delete(0);
			}
		}
		break;
	case GAME_EV_CLEAR_ANSWER:
		{
			switch ( message->popInt16() )
			{
			case 0:
				{
					int id = message->popInt16();
					cVehicle *Vehicle = getVehicleFromID ( id );
					if ( Vehicle == NULL )
					{
						Log.write("Client: Can not find vehicle with id " + iToStr ( id ) + " for clearing", LOG_TYPE_NET_WARNING);
						break;
					}

					Vehicle->ClearingRounds = message->popInt16();
					int bigoffset = message->popInt16();
					if ( bigoffset >= 0 ) Map->moveVehicleBig ( Vehicle, bigoffset );
					Vehicle->IsClearing = true;

					if ( gameGUI.getSelVehicle() == Vehicle )
					{
						StopFXLoop( Client->iObjectStream );
						Client->iObjectStream = Vehicle->playStream();
					}
				}
				break;
			case 1:
				// TODO: add blocked message
				// addMessage ( "blocked" );
				break;
			case 2:
				Log.write("Client: warning on start of clearing", LOG_TYPE_NET_WARNING);
				break;
			default:
				break;
			}
		}
		break;
	case GAME_EV_STOP_CLEARING:
		{
			int id = message->popInt16();
			cVehicle *Vehicle = getVehicleFromID ( id );
			if ( Vehicle == NULL )
			{
				Log.write("Client: Can not find vehicle with id " + iToStr ( id ) + " for stop clearing", LOG_TYPE_NET_WARNING);
				break;
			}

			int bigoffset = message->popInt16();
			if ( bigoffset >= 0 ) Map->moveVehicle ( Vehicle, bigoffset );
			Vehicle->IsClearing = false;
			Vehicle->ClearingRounds = 0;

			if ( gameGUI.getSelVehicle() == Vehicle )
			{
				StopFXLoop( Client->iObjectStream );
				Client->iObjectStream = Vehicle->playStream();
			}
		}
		break;
	case GAME_EV_NOFOG:
		memset ( ActivePlayer->ScanMap, 1, Map->size*Map->size );
		break;
	case GAME_EV_DEFEATED:
		{
			int iTmp = message->popInt16();
			cPlayer *Player = getPlayerFromNumber ( iTmp );
			if ( Player == NULL )
			{
				Log.write ( "Client: Cannot find defeated player!", LOG_TYPE_NET_WARNING );
				break;
			}
			Player->isDefeated = true;
			string msgString = lngPack.i18n( "Text~Multiplayer~Player") + " " + Player->name + " " + lngPack.i18n( "Text~Comp~Defeated");
			addMessage ( msgString );
			ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
			/*for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				if ( Player == (*PlayerList)[i] )
				{
					Hud.ExtraPlayers(Player->name + " (d)", GetColorNr(Player->color), i, Player->bFinishedTurn, false);
					break;
				}
			}*/
		}
		break;
	case GAME_EV_FREEZE:
		freeze();
		if ( message->popBool() ) gameGUI.setInfoTexts ( lngPack.i18n ( "Text~Multiplayer~Frozen" ), "" );
		break;
	case GAME_EV_UNFREEZE:
		unfreeze();
		gameGUI.setInfoTexts ( "", "" );
		break;
	case GAME_EV_WAIT_RECON:
		freeze();
		gameGUI.setInfoTexts ( lngPack.i18n ( "Text~Multiplayer~Wait_Reconnect" ), lngPack.i18n ( "Text~Multiplayer~Abort_Waiting" ) );
		break;
	case GAME_EV_ABORT_WAIT_RECON:
		gameGUI.setInfoTexts ( "", "" );
		break;
	case GAME_EV_DEL_PLAYER:
		{
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			if ( Player == ActivePlayer )
			{
				Log.write ( "Client: Cannot delete own player!", LOG_TYPE_NET_WARNING );
				break;
			}
			if ( Player->VehicleList || Player->BuildingList )
			{
				Log.write ( "Client: Player to be deleted has some units left !", LOG_TYPE_NET_ERROR );
			}
			string msgString = lngPack.i18n ( "Text~Multiplayer~Player_Left", Player->name);
			addMessage ( msgString );
			ActivePlayer->addSavedReport ( msgString, sSavedReportMessage::REPORT_TYPE_COMP );
			/*for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				if ( Player == (*PlayerList)[i] )
				{
					Hud.ExtraPlayers(Player->name + " (-)", GetColorNr(Player->color), i, false, false);
					delete (*PlayerList)[i];
					PlayerList->Delete ( i );
				}
			}*/
		}
		break;
	case GAME_EV_TURN:
		iTurn = message->popInt16();
		gameGUI.updateTurn ( iTurn );
		break;
	case GAME_EV_HUD_SETTINGS:
		{
			int unitID = message->popInt16();
			cBuilding *building = NULL;
			cVehicle *vehicle = getVehicleFromID ( unitID );
			if ( !vehicle ) building = getBuildingFromID ( unitID );
			
			if ( vehicle )
			{
				gameGUI.selectUnit( vehicle );
			}
			else if ( building )
			{
				gameGUI.selectUnit( building );
			}

			int x = message->popInt16();
			int y = message->popInt16();
			gameGUI.setOffsetPosition ( x, y );
			gameGUI.setZoom ( message->popFloat(), true );
			gameGUI.setColor ( message->popBool() );
			gameGUI.setGrid ( message->popBool() );
			gameGUI.setAmmo ( message->popBool() );
			gameGUI.setFog ( message->popBool() );
			gameGUI.setTwoX ( message->popBool() );
			gameGUI.setRange ( message->popBool() );
			gameGUI.setScan ( message->popBool() );
			gameGUI.setStatus ( message->popBool() );
			gameGUI.setSurvey ( message->popBool() );
			gameGUI.setLock ( message->popBool() );
			gameGUI.setHits ( message->popBool() );
			gameGUI.setTNT ( message->popBool() );

			gameGUI.setStartup ( false );
		}
		break;
	case GAME_EV_STORE_UNIT:
		{
			cVehicle *StoredVehicle = getVehicleFromID ( message->popInt16() );
			if ( !StoredVehicle ) break;

			if ( message->popBool() )
			{
				cVehicle *StoringVehicle = getVehicleFromID ( message->popInt16() );
				if ( !StoringVehicle ) break;
				StoringVehicle->storeVehicle ( StoredVehicle, Map );
			}
			else
			{
				cBuilding *StoringBuilding = getBuildingFromID ( message->popInt16() );
				if ( !StoringBuilding ) break;
				StoringBuilding->storeVehicle ( StoredVehicle, Map );
			}

			int mouseX, mouseY;
			mouse->GetKachel ( &mouseX, &mouseY );
			if ( StoredVehicle->PosX == mouseX && StoredVehicle->PosY == mouseY ) gameGUI.updateMouseCursor();

			gameGUI.checkMouseInputMode();

			if ( StoredVehicle == gameGUI.getSelVehicle() ) gameGUI.deselectUnit();

			PlayFX ( SoundData.SNDLoad );
		}
		break;
	case GAME_EV_EXIT_UNIT:
		{
			cVehicle *StoredVehicle = getVehicleFromID ( message->popInt16() );
			if ( !StoredVehicle ) break;

			if ( message->popBool() )
			{
				cVehicle *StoringVehicle = getVehicleFromID ( message->popInt16() );
				if ( !StoringVehicle ) break;

				int x = message->popInt16 ();
				int y = message->popInt16 ();
				StoringVehicle->exitVehicleTo ( StoredVehicle, x+y*Map->size, Map );
				if ( gameGUI.getSelVehicle() == StoringVehicle && gameGUI.mouseInputMode == activateVehicle )
				{
					gameGUI.mouseInputMode = normalInput;
				}
			}
			else
			{
				cBuilding *StoringBuilding = getBuildingFromID ( message->popInt16() );
				if ( !StoringBuilding ) break;

				int x = message->popInt16 ();
				int y = message->popInt16 ();
				StoringBuilding->exitVehicleTo ( StoredVehicle, x+y*Map->size, Map );

				if ( gameGUI.getSelBuilding() == StoringBuilding && gameGUI.mouseInputMode == activateVehicle )
				{
					gameGUI.mouseInputMode = normalInput;
				}
			}
			PlayFX ( SoundData.SNDActivate );
			gameGUI.updateMouseCursor();
		}
		break;
	case GAME_EV_DELETE_EVERYTHING:
		{
			for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
			{
				cPlayer *const Player = (*PlayerList)[i];

				cVehicle *vehicle = Player->VehicleList;
				while ( vehicle )
				{
					if ( vehicle->StoredVehicles.Size() ) vehicle->DeleteStored();
					vehicle = vehicle->next;
				}

				while ( Player->VehicleList )
				{
					vehicle = Player->VehicleList->next;
					Player->VehicleList->bSentryStatus = false;
					Map->deleteVehicle ( Player->VehicleList );
					delete Player->VehicleList;
					Player->VehicleList = vehicle;
				}
				while ( Player->BuildingList )
				{
					cBuilding *building;
					building = Player->BuildingList->next;
					Player->BuildingList->bSentryStatus = false;


					while( Player->BuildingList->StoredVehicles.Size() > 0 )
					{
						Player->BuildingList->StoredVehicles.Delete( 0 );
					}

					Map->deleteBuilding ( Player->BuildingList );
					delete Player->BuildingList;
					Player->BuildingList = building;
				}
			}

			//delete subbases
			while ( ActivePlayer->base.SubBases.Size() )
			{
				ActivePlayer->base.SubBases.Delete(0);
			}

			while ( neutralBuildings )
			{
				cBuilding* nextBuilding = neutralBuildings->next;
				Map->deleteBuilding( neutralBuildings );
				delete neutralBuildings;
				neutralBuildings = nextBuilding;
			}

			//delete attack jobs
			while ( attackJobs.Size() )
			{
				delete attackJobs[0];
				attackJobs.Delete(0);
			}

			//delete FX effects, because a finished rocked animations would do a callback on an attackjob
			while ( FXList.Size() )
			{
				delete FXList[0];
				FXList.Delete(0);
			}
			while ( FXList.Size() )
			{
				delete FXList[0];
				FXList.Delete(0);
			}

			// delete all eventually remaining pointers on the map, to prevent crashes after a resync.
			// Normally there shouldn't be any pointers left after deleting all units, but a resync is not
			// executed in normal situations and there are situations, when this happens.
			Map->reset();
		}
		break;
	case GAME_EV_UNIT_UPGRADE_VALUES:
		{
			sID ID;
			sUnitData *Data;
			ID.iFirstPart = message->popInt16();
			ID.iSecondPart = message->popInt16();
			Data = ID.getUnitDataCurrentVersion ( ActivePlayer );
			if ( Data != NULL )
			{
				Data->version = message->popInt16();
				Data->scan = message->popInt16();
				Data->range = message->popInt16();
				Data->damage = message->popInt16();
				Data->buildCosts = message->popInt16();
				Data->armor = message->popInt16();
				Data->speedMax = message->popInt16();
				Data->shotsMax = message->popInt16();
				Data->ammoMax = message->popInt16();
				Data->hitpointsMax = message->popInt16();
			}
		}
		break;
	case GAME_EV_CREDITS_CHANGED:
		{
			ActivePlayer->Credits = message->popInt16();
		}
		break;
	case GAME_EV_UPGRADED_BUILDINGS:
		{
			int buildingsInMsg = message->popInt16();
			int totalCosts = message->popInt16();
			if (buildingsInMsg > 0)
			{
				string buildingName;
				bool scanNecessary = false;
				for (int i = 0; i < buildingsInMsg; i++)
				{
					int buildingID = message->popInt32();
					cBuilding* building = getBuildingFromID(buildingID);
					if (!scanNecessary && building->data.scan < ActivePlayer->BuildingData[building->typ->nr].scan)
						scanNecessary = true; // Scan range was upgraded. So trigger a scan.
					building->upgradeToCurrentVersion();
					if (i == 0)
					{
						buildingName = building->data.name;
					}
				}
				ostringstream os;
				os << lngPack.i18n ( "Text~Comp~Upgrades_Done") << " " << buildingsInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", buildingName)  << " (" << lngPack.i18n ("Text~Vehicles~Costs") << ": " << totalCosts << ")"; 
				string sTmp(os.str());
				addMessage (sTmp);
				ActivePlayer->addSavedReport ( sTmp, sSavedReportMessage::REPORT_TYPE_COMP );
				if (scanNecessary)
					ActivePlayer->DoScan();
			}
		}
		break;
	case GAME_EV_UPGRADED_VEHICLES:
		{
			int vehiclesInMsg = message->popInt16();
			int totalCosts = message->popInt16();
			unsigned int storingBuildingID = message->popInt32();
			if (vehiclesInMsg > 0)
			{
				string vehicleName;
				for (int i = 0; i < vehiclesInMsg; i++)
				{
					int vehicleID = message->popInt32();
					cVehicle* vehicle = getVehicleFromID(vehicleID);
					vehicle->upgradeToCurrentVersion();
					if (i == 0)
					{
						vehicleName = vehicle->data.name;
					}
				}
				cBuilding* storingBuilding = getBuildingFromID(storingBuildingID);
				if (storingBuilding && ActiveMenu )
				{
					cStorageMenu *storageMenu = dynamic_cast<cStorageMenu*>(ActiveMenu);
					if ( storageMenu )
					{
						storageMenu->resetInfos();
						storageMenu->draw();
					}
				}
				ostringstream os;
				os << lngPack.i18n ( "Text~Comp~Upgrades_Done") << " " << vehiclesInMsg << " " << lngPack.i18n ("Text~Comp~Upgrades_Done2", vehicleName)  << " (" << lngPack.i18n ("Text~Vehicles~Costs") << ": " << totalCosts << ")";

				string printStr(os.str());
				addMessage (printStr);
				ActivePlayer->addSavedReport ( printStr, sSavedReportMessage::REPORT_TYPE_COMP );
			}
		}
		break;
	case GAME_EV_RESEARCH_SETTINGS:
		{
			int buildingsInMsg = message->popInt16();
			if (buildingsInMsg > 0)
			{
				for (int i = 0; i < buildingsInMsg; i++)
				{
					int buildingID = message->popInt32();
					int newArea = message->popChar();
					cBuilding* building = getBuildingFromID(buildingID);
					if (building && building->data.canResearch && 0 <= newArea && newArea <= cResearch::kNrResearchAreas)
						building->researchArea = newArea;
				}
			}
			// now update the research center count for the areas
			ActivePlayer->refreshResearchCentersWorkingOnArea();
		}
		break;
	case GAME_EV_RESEARCH_LEVEL:
		{
			for (int area = cResearch::kNrResearchAreas - 1; area >= 0; area--)
			{
				int newCurPoints = message->popInt16();
				int newLevel = message->popInt16();
				ActivePlayer->researchLevel.setCurResearchLevel(newLevel, area);
				ActivePlayer->researchLevel.setCurResearchPoints(newCurPoints, area);
			}
		}
		break;
	case GAME_EV_REFRESH_RESEARCH_COUNT: // sent, when the player was resynced (or a game was loaded)
		ActivePlayer->refreshResearchCentersWorkingOnArea();
		break;
	case GAME_EV_SET_AUTOMOVE:
		{
			cVehicle *Vehicle = getVehicleFromID ( message->popInt16() );
			if ( Vehicle )
			{
				if ( Vehicle->autoMJob )
				{
					delete Vehicle->autoMJob;
					Vehicle->autoMJob = NULL;
				}
				Vehicle->autoMJob = new cAutoMJob ( Vehicle );
			}
		}
		break;
	case GAME_EV_COMMANDO_ANSWER:
		{
			if ( message->popBool() )
			{
				if ( message->popBool() ) PlayVoice ( VoiceData.VOIUnitStolen );
				else PlayVoice ( VoiceData.VOIUnitDisabled );
			}
			else PlayVoice ( VoiceData.VOICommandoDetected );
			cVehicle *srcUnit = getVehicleFromID ( message->popInt16() );

			gameGUI.checkMouseInputMode();
		}
		break;
	case GAME_EV_REQ_SAVE_INFO:
		{
			int saveingID = message->popInt16();
			if ( gameGUI.getSelVehicle() ) sendSaveHudInfo ( gameGUI.getSelVehicle()->iID, ActivePlayer->Nr, saveingID );
			else if ( gameGUI.getSelBuilding() ) sendSaveHudInfo ( gameGUI.getSelBuilding()->iID, ActivePlayer->Nr, saveingID );
			else sendSaveHudInfo ( -1, ActivePlayer->Nr, saveingID );

			for ( int i = ActivePlayer->savedReportsList.Size()-50; i < (int)ActivePlayer->savedReportsList.Size(); i++ )
			{
				if ( i < 0 ) continue;
				sendSaveReportInfo ( &ActivePlayer->savedReportsList[i], ActivePlayer->Nr, saveingID );
			}
			sendFinishedSendSaveInfo ( ActivePlayer->Nr, saveingID );
		}
		break;
	case GAME_EV_SAVED_REPORT:
		{
			sSavedReportMessage savedReport;
			savedReport.message = message->popString();
			savedReport.type = (sSavedReportMessage::eReportTypes)message->popInt16();
			savedReport.xPos = message->popInt16();
			savedReport.yPos = message->popInt16();
			savedReport.unitID.iFirstPart = message->popInt16();
			savedReport.unitID.iSecondPart = message->popInt16();
			savedReport.colorNr = message->popInt16();
			ActivePlayer->savedReportsList.Add ( savedReport );
		}
		break;
	case GAME_EV_SCORE:
		{
			int pn = message->popInt16();
			int turn = message->popInt16();
			int n = message->popInt16();
			
			getPlayerFromNumber(pn)->setScore(n, turn);
		}
		break;
	case GAME_EV_NUM_ECOS:
		{
			int pn = message->popInt16();
			int n = message->popInt16();
			
			getPlayerFromNumber(pn)->numEcos = n;
		}
		break;
	case GAME_EV_UNIT_SCORE:
		{
			cBuilding *b = getBuildingFromID(message->popInt16());
			b->points = message->popInt16();
		}
		break;
	case GAME_EV_VICTORY_CONDITIONS:
		{
			scoreLimit = message->popInt16();
			turnLimit = message->popInt16();
		}
		break;
	case GAME_EV_SELFDESTROY:
		{
			cBuilding* building = getBuildingFromID(message->popInt16());
			if ( !building ) break;

			destroyUnit(building);
		}
		break;
	default:
		Log.write("Client: Can not handle message type " + message->getTypeAsString(), cLog::eLOG_TYPE_NET_ERROR);
		break;
	}

	CHECK_MEMORY;
	return 0;
}

void cClient::addUnit( int iPosX, int iPosY, cVehicle *AddedVehicle, bool bInit, bool bAddToMap )
{
	// place the vehicle
	if ( bAddToMap ) Map->addVehicle( AddedVehicle, iPosX, iPosY );

	if ( !bInit ) AddedVehicle->StartUp = 10;

	gameGUI.updateMouseCursor();
	gameGUI.callMiniMapDraw();
}

void cClient::addUnit( int iPosX, int iPosY, cBuilding *AddedBuilding, bool bInit )
{
	// place the building
	Map->addBuilding( AddedBuilding, iPosX, iPosY);


	if ( !bInit ) AddedBuilding->StartUp = 10;

	gameGUI.updateMouseCursor();
	gameGUI.callMiniMapDraw();
}

cPlayer *cClient::getPlayerFromNumber ( int iNum )
{
	for ( unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		cPlayer* const p = (*PlayerList)[i];
		if (p->Nr == iNum) return p;
	}
	return NULL;
}

void cClient::deleteUnit( cBuilding *Building )
{
	if( !Building ) return;
	gameGUI.callMiniMapDraw();

	if ( ActiveMenu ) ActiveMenu->handleDestroyUnit ( Building );
	Map->deleteBuilding( Building );

	if ( !Building->owner )
	{
		if ( !Building->prev )
		{
			neutralBuildings = Building->next;
			if ( Building->next )
				Building->next->prev = NULL;
		}
		else
		{
			Building->prev->next = Building->next;
			if ( Building->next )
				Building->next->prev = Building->prev;
		}
		delete Building;
		return;
	}

	for ( unsigned int i = 0; i < attackJobs.Size(); i++)
	{
		if ( attackJobs[i]->building == Building )
		{
			attackJobs[i]->building = NULL;
		}
	}

	if( Building->prev )
	{
		Building->prev->next = Building->next;
		if( Building->next )
		{
			Building->next->prev = Building->prev;
		}
	}
	else
	{
		Building->owner->BuildingList = Building->next;
		if( Building->next )
		{
			Building->next->prev = NULL;
		}
	}

	if ( Building->owner == ActivePlayer )
		Building->owner->base.deleteBuilding(Building, false );

	cPlayer* owner = Building->owner;
	delete Building;

	owner->DoScan();

}

void cClient::deleteUnit( cVehicle *Vehicle )
{
	if( !Vehicle ) return;

	if ( ActiveMenu ) ActiveMenu->handleDestroyUnit ( NULL, Vehicle );
	Map->deleteVehicle( Vehicle );

	for ( unsigned int i = 0; i < attackJobs.Size(); i++)
	{
		if ( attackJobs[i]->vehicle == Vehicle )
		{
			attackJobs[i]->vehicle = NULL;
		}
	}

	gameGUI.callMiniMapDraw();

	if( Vehicle->prev )
	{
		Vehicle->prev->next = Vehicle->next;
		if( Vehicle->next )
		{
			Vehicle->next->prev = Vehicle->prev;
		}
	}
	else
	{
		Vehicle->owner->VehicleList = Vehicle->next;
		if( Vehicle->next )
		{
			Vehicle->next->prev = NULL;
		}
	}

	cPlayer* owner = Vehicle->owner;
	delete Vehicle;

	if ( owner ) owner->DoScan();
}

void cClient::handleEnd()
{
	if ( bWaitForOthers ) return;
	bWantToEnd = true;
	sendWantToEndTurn();
}

void cClient::makeHotSeatEnd( int iNextPlayerNum )
{
	// clear the messages
	sMessage *Message;
	while (messages.Size())
	{
		Message = messages[0];
		delete Message;
		messages.Delete ( 0 );
	}

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
	SDL_Surface *sf;
	SDL_Rect scr;
	sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,SettingsData.iScreenW,SettingsData.iScreenH,32,0,0,0,0 );
	scr.x=15;
	scr.y=356;
	scr.w=scr.h=112;
	SDL_BlitSurface ( sf,NULL,buffer,NULL );
	SDL_BlitSurface ( sf,&scr,buffer,&scr );

	cDialogOK okDialog ( lngPack.i18n( "Text~Multiplayer~Player_Turn", ActivePlayer->name) );
	okDialog.show();
}

void cClient::handleTurnTime()
{
	static int lastCheckTime = SDL_GetTicks();
	if ( !timer50ms ) return;
	// stop time when waiting for reconnection
	if ( bWaitForOthers  )
	{
		iStartTurnTime += SDL_GetTicks()-lastCheckTime;
	}
	if ( iTurnTime > 0 )
	{
		int iRestTime = iTurnTime - Round( ( SDL_GetTicks() - iStartTurnTime )/1000 );
		if ( iRestTime < 0 ) iRestTime = 0;
		gameGUI.updateTurnTime ( iRestTime );
	}
	lastCheckTime = SDL_GetTicks();
}

void cClient::addActiveMoveJob ( cClientMoveJob *MoveJob )
{
	MoveJob->bSuspended = false;
	if ( MoveJob->Vehicle) MoveJob->Vehicle->MoveJobActive = true;
	for ( unsigned int i = 0; i < ActiveMJobs.Size(); i++ )
	{
		if ( ActiveMJobs[i] == MoveJob ) return;
	}
	ActiveMJobs.Add ( MoveJob );
}

void cClient::handleMoveJobs ()
{
	for (unsigned int i = 0; i < ActiveMJobs.Size(); i++)
	{
		cClientMoveJob *MoveJob;
		cVehicle *Vehicle;

		MoveJob = ActiveMJobs[i];
		Vehicle = MoveJob->Vehicle;

		//suspend movejobs of attacked vehicles
		if ( Vehicle && Vehicle->bIsBeeingAttacked ) continue;

		if ( MoveJob->bFinished || MoveJob->bEndForNow )
		{
			if ( Vehicle && Vehicle->ClientMoveJob == MoveJob ) MoveJob->stopMoveSound();
		}

		if ( MoveJob->bFinished )
		{
			if ( Vehicle && Vehicle->ClientMoveJob == MoveJob )
			{
				Log.write(" Client: Movejob is finished and will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
				Vehicle->ClientMoveJob = NULL;
				Vehicle->moving = false;
				Vehicle->MoveJobActive = false;
				if ( MoveJob->endMoveAction ) MoveJob->endMoveAction->execute();
			}
			else Log.write(" Client: Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
			ActiveMJobs.Delete ( i );
			delete MoveJob;
			if ( Vehicle == gameGUI.getSelVehicle() ) gameGUI.updateMouseCursor();
			continue;
		}
		if ( MoveJob->bEndForNow )
		{
			Log.write(" Client: Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
			if ( Vehicle )
			{
				Vehicle->MoveJobActive = false;
				Vehicle->moving = false;
			}
			ActiveMJobs.Delete ( i );
			if ( Vehicle == gameGUI.getSelVehicle() ) gameGUI.updateMouseCursor();
			continue;
		}

		if ( Vehicle == NULL ) continue;


		if ( MoveJob->iNextDir != Vehicle->dir && Vehicle->data.speedCur )
		{
			// rotate vehicle
			if ( timer100ms ) Vehicle->RotateTo ( MoveJob->iNextDir );
		}
		else if ( Vehicle->MoveJobActive )
		{
			// move vehicle
			if ( timer50ms ) MoveJob->moveVehicle();
		}
	}
}

cVehicle *cClient::getVehicleFromID ( int iID )
{
	cVehicle *Vehicle;
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		Vehicle = (*PlayerList)[i]->VehicleList;
		while ( Vehicle )
		{
			if ( Vehicle->iID == iID ) return Vehicle;
			Vehicle = Vehicle->next;
		}
	}
	return NULL;
}

cBuilding *cClient::getBuildingFromID ( int iID )
{
	cBuilding *Building;
	for (unsigned int i = 0; i < PlayerList->Size(); i++)
	{
		Building = (*PlayerList)[i]->BuildingList;
		while ( Building )
		{
			if ( Building->iID == iID ) return Building;
			Building = Building->next;
		}
	}

	Building = neutralBuildings;
	while ( Building )
	{
		if ( Building->iID == iID ) return Building;
		Building = Building->next;
	}

	return NULL;
}

void cClient::doGameActions()
{
	handleMessages();

	handleTimer();
	if ( !timer50ms ) return;

	//run attackJobs
	cClientAttackJob::handleAttackJobs();
	//run moveJobs - this has to be called before handling the auto movejobs
	handleMoveJobs();
	//run surveyor ai
	cAutoMJob::handleAutoMoveJobs();
	//run effects
	runFX();

	handleTurnTime();
}

sSubBase *cClient::getSubBaseFromID ( int iID )
{
	cBuilding* building = Client->getBuildingFromID( iID );
	if ( building )
		return building->SubBase;

	return NULL;
}

void cClient::destroyUnit( cVehicle* vehicle )
{
	//play explosion
	if ( vehicle->data.isBig )
	{
		Client->addFX( fxExploBig, vehicle->PosX * 64 + 64, vehicle->PosY * 64 + 64, 0);
	}
	else if ( vehicle->data.factorAir > 0 && vehicle->FlightHigh != 0 )
	{
		Client->addFX( fxExploAir, vehicle->PosX*64 + vehicle->OffX + 32, vehicle->PosY*64 + vehicle->OffY + 32, 0);
	}
	else if ( Map->IsWater(vehicle->PosX + vehicle->PosY*Map->size) )
	{
		Client->addFX( fxExploWater, vehicle->PosX*64 + vehicle->OffX + 32, vehicle->PosY*64 + vehicle->OffY + 32, 0);
	}
	else
	{
		Client->addFX( fxExploSmall, vehicle->PosX*64 + vehicle->OffX + 32, vehicle->PosY*64 + vehicle->OffY + 32, 0);
	}

	if ( vehicle->data.hasCorpse )
	{
		//add corpse
		Client->addFX( fxCorpse,  vehicle->PosX*64 + vehicle->OffX, vehicle->PosY*64 + vehicle->OffY, 0);
	}
}

void cClient::destroyUnit(cBuilding *building)
{
	//play explosion animation
	cBuilding* topBuilding = Map->fields[building->PosX + building->PosY*Map->size].getBuildings();
	if ( topBuilding && topBuilding->data.isBig )
	{
		Client->addFX( fxExploBig, topBuilding->PosX * 64 + 64, topBuilding->PosY * 64 + 64, 0);
	}
	else
	{
		Client->addFX( fxExploSmall, building->PosX * 64 + 32, building->PosY * 64 + 32, 0);
	}
}

void cClient::checkVehiclePositions(cNetMessage *message)
{

	static cList<cVehicle*>  vehicleList;
	bool lastMessagePart = message->popBool();

	if ( vehicleList.Size() == 0 && !lastMessagePart )
	{
		//generate list with all vehicles
		for ( unsigned int i = 0; i < Client->PlayerList->Size(); i++ )
		{
			cVehicle* vehicle = (*Client->PlayerList)[i]->VehicleList;
			while ( vehicle )
			{
				vehicleList.Add( vehicle );
				vehicle = vehicle->next;
			}
		}
	}

	//check all sent positions
	while ( message->iLength > 6 )
	{
		int id = message->popInt32();
		int PosY = message->popInt16();
		int PosX = message->popInt16();
		cVehicle* vehicle = getVehicleFromID(id);
		if ( vehicle == NULL )
		{
			Log.write("   --Vehicle not present, ID: " + iToStr(id), cLog::eLOG_TYPE_NET_ERROR );
			continue;
		}

		if ( PosX != -1 && PosY != -1 && ( vehicle->PosX != PosX || vehicle->PosY != PosY ) && !vehicle->ClientMoveJob )
		{
			Log.write("   --wrong position, ID: " + iToStr(id) + ", is: "+iToStr(vehicle->PosX)+":"+iToStr(vehicle->PosY)+", should: "+iToStr(PosX)+":"+iToStr(PosY) , cLog::eLOG_TYPE_NET_ERROR);
		}

		//remove vehicle from list
		for ( unsigned int i = 0; i < vehicleList.Size(); i++)
		{
			if ( vehicleList[i] == vehicle )
			{
				vehicleList.Delete(i);
				break;
			}
		}
	}

	if ( lastMessagePart )
	{
		//check remaining vehicles
		while ( vehicleList.Size() > 0 )
		{
			Log.write("   --vehicle should not exist, ID: "+iToStr(vehicleList[0]->iID), cLog::eLOG_TYPE_NET_ERROR );
			vehicleList.Delete(0);
		}
	}
}

//-------------------------------------------------------------------------------------
int cClient::getTurn() const
{
	return iTurn;
}

//-------------------------------------------------------------------------------------
void cClient::getVictoryConditions(int *turnLimit, int *scoreLimit) const
{
	*turnLimit = this->turnLimit;
	*scoreLimit = this->scoreLimit;
}

//-------------------------------------------------------------------------------------
void cClient::freeze()
{
	bWaitForOthers = true;
	waitReconnect = true;
}

//-------------------------------------------------------------------------------------
void cClient::unfreeze()
{
	bWaitForOthers = false;
	waitReconnect = false;
}
