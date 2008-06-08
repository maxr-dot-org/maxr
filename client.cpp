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
#include "keyinp.h"
#include "keys.h"
#include "fonts.h"
#include "netmessage.h"
#include "main.h"
#include "attackJobs.h"

Uint32 TimerCallback(Uint32 interval, void *arg)
{
	((cClient *)arg)->Timer();
	return interval;
}

cClient::cClient(cMap* const Map, cList<cPlayer*>* const PlayerList)
{
	this->Map = Map;
	bExit = false;

	this->PlayerList = PlayerList;

	TimerID = SDL_AddTimer ( 50, TimerCallback, this );
	iTimerTime = 0;
	iFrame = 0;
	SelectedVehicle = NULL;
	SelectedBuilding = NULL;
	DirtList = NULL;
	iObjectStream = -1;
	OverObject = NULL;
	iBlinkColor = 0xFFFFFF;
	FLC = NULL;
	sFLCname = "";
	video = NULL;
	Hud = new cHud;
	bHelpActive = false;
	bChangeObjectName = false;
	bChatInput = false;
	bDefeated = false;
	iMsgCoordsX = -1;
	iMsgCoordsY = -1;
	iTurn = 1;
	bWantToEnd = false;
	FXList = new cList<sFX*>;
	FXListBottom = new cList<sFX*>;
	bUpShowTank = true;
	bUpShowPlane = true;
	bUpShowShip = true;
	bUpShowBuild = true;
	bUpShowTNT = false;
	bAlienTech = false;
	bDebugBaseServer = false;
	bDebugBaseClient = false;
	bDebugWache = false;
	bDebugFX = false;
	bDebugTraceServer = false;
	bDebugTraceClient = false;
	bWaitForOthers = false;
	iTurnTime = 0;
	attackJobs = new cList<cClientAttackJob*>;

	SDL_Rect rSrc = {0,0,170,224};
	SDL_Surface *SfTmp = LoadPCX((SettingsData.sGfxPath + PATH_DELIMITER + "hud_left.pcx").c_str());
	SDL_BlitSurface( SfTmp, &rSrc, GraphicsData.gfx_hud, NULL );
	SDL_FreeSurface( SfTmp );

	setWind ( random ( 360,0 ) );
}

cClient::~cClient()
{
	Hud->Zoom = 64;
	Hud->ScaleSurfaces();
	SDL_RemoveTimer ( TimerID );
	StopFXLoop ( iObjectStream );
	while ( messages.iCount )
	{
		free ( messages.Items[0]->msg );
		free ( messages.Items[0] );
		messages.Delete ( 0 );
	}
	if ( FLC ) FLI_Close ( FLC );
	delete Hud;
	while ( FXList->iCount )
	{
		if ( FXList->Items[0]->typ == fxRocket )
		{
			delete FXList->Items[0]->rocketInfo;
		}
		else if ( FXList->Items[0]->typ == fxDarkSmoke )
		{
			delete FXList->Items[0]->smokeInfo;
		}
		delete FXList->Items[0];
		FXList->Delete ( 0 );
	}
	delete FXList;
	while ( FXListBottom->iCount )
	{
		if ( FXListBottom->Items[0]->typ == fxTorpedo )
		{
			delete FXListBottom->Items[0]->rocketInfo;
		}
		else if ( FXListBottom->Items[0]->typ == fxTracks )
		{
			delete FXListBottom->Items[0]->trackInfo;
		}
		delete FXListBottom->Items[0];
		FXListBottom->Delete ( 0 );
	}
	delete FXListBottom;

	for (int i = 0; i < attackJobs->iCount; i++ )
	{
		delete attackJobs->Items[i];
	}
	delete attackJobs;

	while( DirtList )
	{
		cBuilding *ptr;
		ptr = DirtList->next;
		delete DirtList;
		DirtList=ptr;
	}
}

void cClient::sendNetMessage(cNetMessage *message)
{
	message->iPlayerNr = ActivePlayer->Nr;

	cLog::write("Client: sending message,  type: " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

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
		network->send( message->iLength, message->serialize( true ) );
		delete message;
	}
}

void cClient::Timer()
{
	iTimerTime++;
}

void cClient::initPlayer( cPlayer *Player )
{
	bFlagDrawHud = true;
	bFlagDrawMap = true;
	bFlagDraw = true;
	bFlagDrawMMap = true;
	ActivePlayer = Player;
}

void cClient::run()
{
	int iLastMouseX, iLastMouseY;
	bool bStartup = true;

	mouse->Show();
	mouse->SetCursor ( CHand );
	mouse->MoveCallback = true;
	Hud->DoAllHud();

	waitForOtherPlayer( 0 );

	while ( 1 )
	{
		// check defeat
		if ( bDefeated ) break;
		// check user
		if ( checkUser() == -1 )
		{
			drawMap();
			SHOW_SCREEN
			makePanel ( false );
			break;
		}
		// end truth save/load menu
		if ( bExit )
		{
			drawMap();
			SHOW_SCREEN
			makePanel ( false );
			break;
		}
		// Die Map malen:
		if ( bFlagDrawMap )
		{
			drawMap();
			displayFX();
		}

		// Ggf das Objekt deselektieren:
		if ( SelectedVehicle&&SelectedVehicle->owner!=ActivePlayer&&!ActivePlayer->ScanMap[SelectedVehicle->PosX+SelectedVehicle->PosY*Map->size] )
		{
			SelectedVehicle->Deselct();
			SelectedVehicle=NULL;
		}
		if ( SelectedBuilding&&SelectedBuilding->owner!=ActivePlayer&&!ActivePlayer->ScanMap[SelectedBuilding->PosX+SelectedBuilding->PosY*Map->size] )
		{
			SelectedBuilding->Deselct();
			SelectedBuilding=NULL;
		}
		// Die Objekt-Kreise malen:
		if ( SelectedVehicle )
		{
			int spx,spy;
			spx = SelectedVehicle->GetScreenPosX();
			spy = SelectedVehicle->GetScreenPosY();
			if ( Hud->Scan )
			{
				drawCircle ( spx+Hud->Zoom/2,
				             spy+Hud->Zoom/2,
				             SelectedVehicle->data.scan*Hud->Zoom,SCAN_COLOR,buffer );
			}
			if ( Hud->Reichweite&& ( SelectedVehicle->data.can_attack==ATTACK_LAND||SelectedVehicle->data.can_attack==ATTACK_SUB_LAND||SelectedVehicle->data.can_attack==ATTACK_AIRnLAND ) )
			{
				drawCircle ( spx+Hud->Zoom/2,
				             spy+Hud->Zoom/2,
				             SelectedVehicle->data.range*Hud->Zoom+1,RANGE_GROUND_COLOR,buffer );
			}
			if ( Hud->Reichweite&&SelectedVehicle->data.can_attack==ATTACK_AIR )
			{
				drawCircle ( spx+Hud->Zoom/2,
				             spy+Hud->Zoom/2,
				             SelectedVehicle->data.range*Hud->Zoom+2,RANGE_AIR_COLOR,buffer );
			}
			if ( Hud->Munition&&SelectedVehicle->data.can_attack )
			{
				SelectedVehicle->DrawMunBar();
			}
			if ( Hud->Treffer )
			{
				SelectedVehicle->DrawHelthBar();
			}
			if ( ( ( SelectedVehicle->IsBuilding&&SelectedVehicle->BuildRounds==0 ) || ( SelectedVehicle->IsClearing&&SelectedVehicle->ClearingRounds==0 ) ) &&SelectedVehicle->owner==ActivePlayer )
			{
				if ( SelectedVehicle->data.can_build==BUILD_BIG||SelectedVehicle->ClearBig )
				{
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+1+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+2+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom*2,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+2+ ( SelectedVehicle->PosY ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom*2,spy );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY+1 ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy+Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+2+ ( SelectedVehicle->PosY+1 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom*2,spy+Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY+2 ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy+Hud->Zoom*2 );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+ ( SelectedVehicle->PosY+2 ) *Map->size ) ) drawExitPoint ( spx,spy+Hud->Zoom*2 );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+1+ ( SelectedVehicle->PosY+2 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom,spy+Hud->Zoom*2 );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+2+ ( SelectedVehicle->PosY+2 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom*2,spy+Hud->Zoom*2 );
				}
				else
				{
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+1+ ( SelectedVehicle->PosY-1 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom,spy-Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+1+ ( SelectedVehicle->PosY ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom,spy );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX-1+ ( SelectedVehicle->PosY+1 ) *Map->size ) ) drawExitPoint ( spx-Hud->Zoom,spy+Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+ ( SelectedVehicle->PosY+1 ) *Map->size ) ) drawExitPoint ( spx,spy+Hud->Zoom );
					if ( SelectedVehicle->CanDrive ( SelectedVehicle->PosX+1+ ( SelectedVehicle->PosY+1 ) *Map->size ) ) drawExitPoint ( spx+Hud->Zoom,spy+Hud->Zoom );
				}
			}
			if ( SelectedVehicle->PlaceBand )
			{
				if ( SelectedVehicle->data.can_build==BUILD_BIG )
				{
					SDL_Rect dest;
					dest.x=180- ( ( int ) ( ( Hud->OffX ) / ( 64.0/Hud->Zoom ) ) ) +Hud->Zoom*SelectedVehicle->BandX;
					dest.y=18- ( ( int ) ( ( Hud->OffY ) / ( 64.0/Hud->Zoom ) ) ) +Hud->Zoom*SelectedVehicle->BandY;
					dest.h=dest.w=GraphicsData.gfx_band_big->h;
					SDL_BlitSurface ( GraphicsData.gfx_band_big,NULL,buffer,&dest );
				}
				else
				{
					SDL_Rect dest;
					int x,y;
					mouse->GetKachel ( &x,&y );
					if ( x==SelectedVehicle->PosX||y==SelectedVehicle->PosY )
					{
						dest.x=180- ( ( int ) ( ( Hud->OffX ) / ( 64.0/Hud->Zoom ) ) ) +Hud->Zoom*x;
						dest.y=18- ( ( int ) ( ( Hud->OffY ) / ( 64.0/Hud->Zoom ) ) ) +Hud->Zoom*y;
						dest.h=dest.w=GraphicsData.gfx_band_small->h;
						SDL_BlitSurface ( GraphicsData.gfx_band_small,NULL,buffer,&dest );
						SelectedVehicle->BandX=x;
						SelectedVehicle->BandY=y;
						SelectedVehicle->BuildPath=true;
					}
					else
					{
						SelectedVehicle->BandX=SelectedVehicle->PosX;
						SelectedVehicle->BandY=SelectedVehicle->PosY;
					}
				}
			}
			if ( SelectedVehicle->ActivatingVehicle&&SelectedVehicle->owner==ActivePlayer )
			{
				SelectedVehicle->DrawExitPoints ( SelectedVehicle->StoredVehicles->Items[SelectedVehicle->VehicleToActivate]->typ );
			}
		}
		else if ( SelectedBuilding )
		{
			int spx,spy;
			spx=SelectedBuilding->GetScreenPosX();
			spy=SelectedBuilding->GetScreenPosY();
			if ( Hud->Scan )
			{
				if ( SelectedBuilding->data.is_big )
				{
					drawCircle ( spx+Hud->Zoom,
					             spy+Hud->Zoom,
					             SelectedBuilding->data.scan*Hud->Zoom,SCAN_COLOR,buffer );
				}
				else
				{
					drawCircle ( spx+Hud->Zoom/2,
					             spy+Hud->Zoom/2,
					             SelectedBuilding->data.scan*Hud->Zoom,SCAN_COLOR,buffer );
				}
			}
			if ( Hud->Reichweite&& ( SelectedBuilding->data.can_attack==ATTACK_LAND||SelectedBuilding->data.can_attack==ATTACK_SUB_LAND ) &&!SelectedBuilding->data.is_expl_mine )
			{
				drawCircle ( spx+Hud->Zoom/2,
				             spy+Hud->Zoom/2,
				             SelectedBuilding->data.range*Hud->Zoom+2,RANGE_GROUND_COLOR,buffer );
			}
			if ( Hud->Reichweite&&SelectedBuilding->data.can_attack==ATTACK_AIR )
			{
				drawCircle ( spx+Hud->Zoom/2,
				             spy+Hud->Zoom/2,
				             SelectedBuilding->data.range*Hud->Zoom+2,RANGE_AIR_COLOR,buffer );
			}

			if ( Hud->Munition&&SelectedBuilding->data.can_attack&&!SelectedBuilding->data.is_expl_mine )
			{
				SelectedBuilding->DrawMunBar();
			}
			if ( Hud->Treffer )
			{
				SelectedBuilding->DrawHelthBar();
			}
			if ( SelectedBuilding->BuildList && SelectedBuilding->BuildList->iCount && !SelectedBuilding->IsWorking && SelectedBuilding->BuildList->Items[0]->metall_remaining <= 0 && SelectedBuilding->owner == ActivePlayer )
			{
				SelectedBuilding->DrawExitPoints ( SelectedBuilding->BuildList->Items[0]->typ );
			}
			if ( SelectedBuilding->ActivatingVehicle&&SelectedBuilding->owner==ActivePlayer )
			{
				SelectedBuilding->DrawExitPoints ( SelectedBuilding->StoredVehicles->Items[SelectedBuilding->VehicleToActivate]->typ );
			}
		}
		ActivePlayer->DrawLockList ( Hud );
		// draw the minimap:
		if ( bFlagDrawMMap )
		{
			bFlagDrawMMap = false;
			drawMiniMap();
			bFlagDrawHud = true;
		}
		// Debugausgaben machen:
		iDebugOff = 30;
		if ( bDebugBaseClient && bFlagDrawMap )
		{
			font->showText(550, iDebugOff, "subbases: " + iToStr(ActivePlayer->base->SubBases.iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight ( LATIN_SMALL_WHITE );
		}

		if ( bDebugBaseServer && bFlagDrawMap )
		{
			cPlayer* serverPlayer = Server->getPlayerFromNumber(ActivePlayer->Nr);
			font->showText(550, iDebugOff, "subbases: " + iToStr(serverPlayer->base->SubBases.iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight ( LATIN_SMALL_WHITE );
		}

		if ( bDebugWache && bFlagDrawMap )
		{
			font->showText(550,iDebugOff, "w-air: " + iToStr(ActivePlayer->WachpostenAir.iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
			font->showText(550,iDebugOff, "w-ground: " + iToStr(ActivePlayer->WachpostenGround.iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
		}

		if ( bDebugFX && bFlagDrawMap )
		{
			font->showText(550, iDebugOff, "fx-count: " + iToStr(FXList->iCount + FXListBottom->iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
			font->showText(550, iDebugOff, "wind-dir: " + iToStr(( int ) ( fWindDir*57.29577 )), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
		}
		if ( ( bDebugTraceServer || bDebugTraceClient ) && bFlagDrawMap )
		{
			trace();
		}
		// check whether the hud has to be drawn:
		if ( bFlagDrawHud || bFlagDrawMap )
		{
			SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );
			mouse->GetBack ( buffer );
			bFlagDraw = true;
		}
		// draw the video:
		if ( bFlagDraw || bFlagDrawHud )
		{
			drawFLC();
		}
		// display the object menu:
		if ( bFlagDrawMap && SelectedVehicle && SelectedVehicle->MenuActive )
		{
			SelectedVehicle->DrawMenu();
		}
		if ( bFlagDrawMap && SelectedBuilding && SelectedBuilding->MenuActive )
		{
			SelectedBuilding->DrawMenu();
		}
		// display the chatinput:
		if ( bChatInput && bFlagDrawMap )
		{
			string OutTxt = ">";
			OutTxt += InputStr;
			if ( iFrame%2 )
			{

			}
			else
			{
				OutTxt += "_";
			}
			font->showText(185,440, OutTxt);
		}
		// display the messages:
		if ( bFlagDrawMap )
		{
			handleMessages();
		}
		// draw the mouse:
		if ( bFlagDraw )
		{
			iLastMouseX = mouse->x;
			iLastMouseY = mouse->y;
			mouse->draw ( false, buffer );
		}
		else if ( mouse->x != iLastMouseX || mouse->y != iLastMouseY )
		{
			iLastMouseX = mouse->x;
			iLastMouseY = mouse->y;
			mouse->draw ( true, screen );
		}
		// display the buffer:
		if ( bFlagDraw )
		{
			//iFrames++;
			if ( bStartup )
			{
				Hud->SetZoom(1);
				drawMap();
				SHOW_SCREEN
				makePanel ( true );

				Hud->SetZoom(64);
				if ( ActivePlayer->BuildingList) ActivePlayer->BuildingList->Center();
				else if (ActivePlayer->VehicleList) ActivePlayer->VehicleList->Center();
				drawMap();
				SHOW_SCREEN

				bStartup = false;
			}
			SHOW_SCREEN
			bFlagDraw = false;
			bFlagDrawHud = false;
			bFlagDrawMap = false;
		}
		else if ( !SettingsData.bFastMode )
		{
			SDL_Delay ( 10 ); // theres northing to do.
		}
		// handle the timers and do game actions:
		doGameActions();
		if ( iTimer1 )
		{
			iFrame++;
			bFlagDrawMap = true;
			rotateBlinkColor();
			if ( FLC != NULL && Hud->PlayFLC )
			{
				FLI_NextFrame ( FLC );
			}
		}
		handleTurnTime();
		// change the wind direction:
		if ( iTimer2 && SettingsData.bDamageEffects )
		{
			static int iNextChange = 25, iNextDirChange = 25, iDir = 90, iChange = 3;
			if ( iNextChange == 0 )
			{
				iNextChange = 10+random ( 20,0 );
				iDir += iChange;
				setWind ( iDir );
				if ( iDir >= 360 ) iDir -= 360;
				else if ( iDir < 0 ) iDir += 360;

				if ( iNextDirChange==0 )
				{
					iNextDirChange = 10+random ( 25,0 );
					iChange = random ( 11,0 )-5;
				}
				else iNextDirChange--;

			}
			else iNextChange--;
		}
	}
	mouse->MoveCallback = false;
}

int cClient::checkUser()
{
	static int iLastMouseButton = 0, iMouseButton;
	static bool bLastReturn = false;
	Uint8 *keystate;
	// get events:
	EventHandler->HandleEvents();

	// check the keys:
	keystate = SDL_GetKeyState( NULL );
	if ( bChangeObjectName )
	{
		DoKeyInp ( keystate );
		if ( InputEnter )
		{
			bChangeObjectName = false;
			// TODO: no engine: Change names of units
			/*if ( SelectedVehicle )
			{
				engine->ChangeVehicleName ( SelectedVehicle->PosX,SelectedVehicle->PosY,InputStr,false,SelectedVehicle->data.can_drive==DRIVE_AIR );
			}
			else if ( SelectedBuilding )
			{
				engine->ChangeBuildingName ( SelectedBuilding->PosX,SelectedBuilding->PosY,InputStr,false,SelectedBuilding->data.is_base );
			}*/
		}
		else
		{
			if ( font->getTextWide(InputStr) >=128 )
			{
				InputStr.erase ( InputStr.length()-1 );
			}
		}
	}
	else if ( bChatInput )
	{
		DoKeyInp ( keystate );
		if ( InputEnter )
		{
			bChatInput=false;
			if ( !InputStr.empty() && !doCommand ( InputStr ) )
			{
				sendChatMessageToServer( ActivePlayer->name+": " + InputStr);
			}
		}
		else
		{
			string OutTxt = ">"; OutTxt += InputStr; OutTxt += "_";
			if ( font->getTextWide(OutTxt) >=440 )
			{
				InputStr.erase ( InputStr.length()-1 );
			}
		}
	}
	else
	{

		//TODO: read keystates more sensitive - e.g. take care of ALT and STRG modifiers
		//TODO: add more keys:
		/*
		e end turn
		f center on selected unit
		-/+ zoom bigger/smaller
		g grid
		PG DOWN center on selected unit
		ARROWS scroll on map
		ALT L load menu
		ALT S save menu
		ALT X end game
		ALT F5 save window pos
		ALT F6 save window pos
		ALT F7 save window pos
		ALT F8 save window pos
		F5 jump to saved window pos
		F6 jump to saved window pos
		F7 jump to saved window pos
		F8 jump to saved window pos
		/ or ? activate help cursor
		ALT C screenshot
		Space + Enter + ESC cancel demo sequence
		Shift + mouseclick groupcommand
		*/
		//TODO: additional hotkeys
		/*
		mousewheel UP zoom out
		mousewheel DOWN zoom in

		*/

		if ( keystate[KeysList.KeyExit]&&ShowYesNo ( lngPack.i18n( "Text~Comp~End_Game") ) )
		{
			drawMap ( false );
			SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
			return -1;
		}
		if ( keystate[KeysList.KeyJumpToAction]&&iMsgCoordsX!=-1 )
		{
			Hud->OffX = iMsgCoordsX*64- ( ( int ) ( ( ( float ) 224/Hud->Zoom ) *64 ) ) +32;
			Hud->OffY = iMsgCoordsY*64- ( ( int ) ( ( ( float ) 224/Hud->Zoom ) *64 ) ) +32;
			bFlagDrawMap=true;
			Hud->DoScroll ( 0 );
			iMsgCoordsX=-1;
		}
		if ( keystate[KeysList.KeyEndTurn]&&!bLastReturn&&!Hud->Ende )
		{
			Hud->EndeButton ( true );
			handleEnd();
			bLastReturn = true;
		}
		else if ( !keystate[KeysList.KeyEndTurn] ) bLastReturn=false;
		if ( keystate[KeysList.KeyChat]&&!keystate[SDLK_RALT]&&!keystate[SDLK_LALT] )
		{
			bChatInput = true;
			InputStr = "";
		}
		if ( keystate[KeysList.KeyScroll8a]||keystate[KeysList.KeyScroll8b] ) Hud->DoScroll ( 8 );
		if ( keystate[KeysList.KeyScroll2a]||keystate[KeysList.KeyScroll2b] ) Hud->DoScroll ( 2 );
		if ( keystate[KeysList.KeyScroll6a]||keystate[KeysList.KeyScroll6b] ) Hud->DoScroll ( 6 );
		if ( keystate[KeysList.KeyScroll4a]||keystate[KeysList.KeyScroll4b] ) Hud->DoScroll ( 4 );
		if ( keystate[KeysList.KeyScroll7] ) Hud->DoScroll ( 7 );
		if ( keystate[KeysList.KeyScroll9] ) Hud->DoScroll ( 9 );
		if ( keystate[KeysList.KeyScroll1] ) Hud->DoScroll ( 1 );
		if ( keystate[KeysList.KeyScroll3] ) Hud->DoScroll ( 3 );
		if ( keystate[KeysList.KeyZoomIna]||keystate[KeysList.KeyZoomInb] ) Hud->SetZoom ( Hud->Zoom+1 );
		if ( keystate[KeysList.KeyZoomOuta]||keystate[KeysList.KeyZoomOutb] ) Hud->SetZoom ( Hud->Zoom-1 );

		{
			static SDLKey last_key=SDLK_UNKNOWN;
			if ( keystate[KeysList.KeyFog] ) {if ( last_key!=KeysList.KeyFog ) {Hud->SwitchNebel ( !Hud->Nebel );last_key=KeysList.KeyFog;}}
			else if ( keystate[KeysList.KeyGrid] ) {if ( last_key!=KeysList.KeyGrid ) {Hud->SwitchGitter ( !Hud->Gitter );last_key=KeysList.KeyGrid;}}
			else if ( keystate[KeysList.KeyScan] ) {if ( last_key!=KeysList.KeyScan ) {Hud->SwitchScan ( !Hud->Scan );last_key=KeysList.KeyScan;}}
			else if ( keystate[KeysList.KeyRange] ) {if ( last_key!=KeysList.KeyRange ) {Hud->SwitchReichweite ( !Hud->Reichweite );last_key=KeysList.KeyRange;}}
			else if ( keystate[KeysList.KeyAmmo] ) {if ( last_key!=KeysList.KeyAmmo ) {Hud->SwitchMunition ( !Hud->Munition );last_key=KeysList.KeyAmmo;}}
			else if ( keystate[KeysList.KeyHitpoints] ) {if ( last_key!=KeysList.KeyHitpoints ) {Hud->SwitchTreffer ( !Hud->Treffer );last_key=KeysList.KeyHitpoints;}}
			else if ( keystate[KeysList.KeyColors] ) {if ( last_key!=KeysList.KeyColors ) {Hud->SwitchFarben ( !Hud->Farben );last_key=KeysList.KeyColors;}}
			else if ( keystate[KeysList.KeyStatus] ) {if ( last_key!=KeysList.KeyStatus ) {Hud->SwitchStatus ( !Hud->Status );last_key=KeysList.KeyStatus;}}
			else if ( keystate[KeysList.KeySurvey] ) {if ( last_key!=KeysList.KeySurvey ) {Hud->SwitchStudie ( !Hud->Studie );last_key=KeysList.KeySurvey;}}
			else last_key=SDLK_UNKNOWN;
		}
	}
	// handle the mouse:
	mouse->GetPos();
	iMouseButton = mouse->GetMouseButton();
	if ( MouseStyle == OldSchool && iMouseButton == 4 && iLastMouseButton != 4 && OverObject )
	{
		if ( OverObject->vehicle ) OverObject->vehicle->ShowHelp();
		else if ( OverObject->plane ) OverObject->plane->ShowHelp();
		else if ( OverObject->base ) OverObject->base->ShowHelp();
		else if ( OverObject->top ) OverObject->top->ShowHelp();
		iMouseButton = 0;
	}
	if ( MouseStyle == OldSchool && iMouseButton == 5 ) iMouseButton = 4;
	if ( iMouseButton == 4 && iLastMouseButton != 4 )
	{
		if ( bHelpActive )
		{
			bHelpActive = false;
		}
		else
		{
			if ( OverObject&& (
			            ( SelectedVehicle&& ( OverObject->vehicle==SelectedVehicle||OverObject->plane==SelectedVehicle ) ) ||
			            ( SelectedBuilding&& ( OverObject->base==SelectedBuilding||OverObject->top==SelectedBuilding ) ) ) )
			{
				int next = -1;

				if ( SelectedVehicle )
				{
					if ( OverObject->plane == SelectedVehicle )
					{
						if ( OverObject->vehicle ) next='v';
						else if ( OverObject->top ) next='t';
						else if ( OverObject->base&&OverObject->base->owner ) next='b';
					}
					else
					{
						if ( OverObject->top ) next='t';
						else if ( OverObject->base&&OverObject->base->owner ) next='b';
						else if ( OverObject->plane ) next='p';
					}

					SelectedVehicle->Deselct();
					SelectedVehicle=NULL;
					bChangeObjectName=false;
					StopFXLoop ( iObjectStream );
				}
				else if ( SelectedBuilding )
				{
					if ( OverObject->top==SelectedBuilding )
					{
						if ( OverObject->base&&OverObject->base->owner ) next='b';
						else if ( OverObject->plane ) next='p';
						else if ( OverObject->vehicle ) next='v';
					}
					else
					{
						if ( OverObject->plane ) next='p';
						else if ( OverObject->vehicle ) next='v';
						else if ( OverObject->top ) next='t';
					}

					SelectedBuilding->Deselct();
					SelectedBuilding=NULL;
					bChangeObjectName=false;
					StopFXLoop ( iObjectStream );
				}
				switch ( next )
				{
					case 't':
						SelectedBuilding = OverObject->top;
						SelectedBuilding->Select();
						iObjectStream=SelectedBuilding->PlayStram();
						break;
					case 'b':
						SelectedBuilding = OverObject->base;
						SelectedBuilding->Select();
						iObjectStream=SelectedBuilding->PlayStram();
						break;
					case 'v':
						SelectedVehicle = OverObject->vehicle;
						SelectedVehicle->Select();
						iObjectStream=SelectedVehicle->PlayStram();
						break;
					case 'p':
						SelectedVehicle = OverObject->plane;
						SelectedVehicle->Select();
						iObjectStream=SelectedVehicle->PlayStram();
						break;
				}
			}
			else if ( SelectedVehicle != NULL )
			{
				SelectedVehicle->Deselct();
				SelectedVehicle = NULL;
				bChangeObjectName = false;
				StopFXLoop ( iObjectStream );
			}
			else if ( SelectedBuilding!=NULL )
			{
				SelectedBuilding->Deselct();
				SelectedBuilding=NULL;
				bChangeObjectName=false;
				StopFXLoop ( iObjectStream );
			}
		}
	}
	if ( iMouseButton && !iLastMouseButton && iMouseButton != 4 )
	{
		if ( OverObject && Hud->Lock ) ActivePlayer->ToggelLock ( OverObject );
		if ( SelectedVehicle && mouse->cur == GraphicsData.gfx_Ctransf )
		{
			//TODO: transfer
			//SelectedVehicle->ShowTransfer ( Map->GO+mouse->GetKachelOff() );
		}
		else if ( SelectedBuilding&&mouse->cur==GraphicsData.gfx_Ctransf )
		{
			//TODO: transfer
			//SelectedBuilding->ShowTransfer ( Map->GO+mouse->GetKachelOff() );
		}
		else if ( SelectedVehicle && SelectedVehicle->PlaceBand && mouse->cur == GraphicsData.gfx_Cband )
		{
			// TODO: Build big
		}
		else if ( mouse->cur == GraphicsData.gfx_Cactivate && SelectedBuilding && SelectedBuilding->ActivatingVehicle )
		{
			SelectedBuilding->ExitVehicleTo ( SelectedBuilding->VehicleToActivate, mouse->GetKachelOff(), false );
			PlayFX ( SoundData.SNDActivate );
			mouseMoveCallback ( true );
		}
		else if ( mouse->cur == GraphicsData.gfx_Cactivate && SelectedVehicle && SelectedVehicle->ActivatingVehicle )
		{
			SelectedVehicle->ExitVehicleTo ( SelectedVehicle->VehicleToActivate,mouse->GetKachelOff(),false );
			PlayFX ( SoundData.SNDActivate );
			mouseMoveCallback ( true );
		}
		else if ( mouse->cur == GraphicsData.gfx_Cactivate && SelectedBuilding && SelectedBuilding->BuildList && SelectedBuilding->BuildList->iCount )
		{
			// TODO: Building vehicle finished
		}
		else if ( mouse->cur == GraphicsData.gfx_Cload && SelectedBuilding && SelectedBuilding->LoadActive )
		{
			// TODO: Load vehcile
		}
		else if ( mouse->cur == GraphicsData.gfx_Cload && SelectedVehicle && SelectedVehicle->LoadActive )
		{
			// TODO: Load vehcile
		}
		else if ( mouse->cur == GraphicsData.gfx_Cmuni && SelectedVehicle && SelectedVehicle->MuniActive )
		{
			// TODO: rearm
		}
		else if ( mouse->cur == GraphicsData.gfx_Crepair && SelectedVehicle && SelectedVehicle->RepairActive )
		{
			// TODO: repair
		}
		else if ( mouse->cur == GraphicsData.gfx_Cmove && SelectedVehicle && !SelectedVehicle->moving && !SelectedVehicle->rotating && !Hud->Ende && !SelectedVehicle->Attacking )
		{
			addMoveJob( SelectedVehicle, mouse->GetKachelOff() );
			/*cMJobs *MJob = new cMJobs ( Map, SelectedVehicle->PosX+SelectedVehicle->PosY*Map->size, mouse->GetKachelOff(), SelectedVehicle->data.can_drive == DRIVE_AIR, SelectedVehicle->iID, PlayerList, false );
			if ( !MJob->finished )
			{
				if ( MJob->CalcPath() )
				{
					MJob->CalcNextDir();
					sendMoveJob ( MJob );
					addActiveMoveJob ( MJob );
					cLog::write("(Client) Added new movejob: VehicleID: " + iToStr ( SelectedVehicle->iID ) + ", SrcX: " + iToStr ( SelectedVehicle->PosX ) + ", SrcY: " + iToStr ( SelectedVehicle->PosY ) + ", DestX: " + iToStr ( MJob->DestX ) + ", DestY: " + iToStr ( MJob->DestY ), cLog::eLOG_TYPE_NET_DEBUG);
				}
				else
				{
					MJob->finished = true;
					if ( random ( 2,0 ) ) PlayVoice ( VoiceData.VOINoPath1 );
					else PlayVoice ( VoiceData.VOINoPath2 );
				}
			}
			if ( MJob->finished )
			{
				if ( MJob->vehicle )
				{
					MJob->vehicle->mjob = NULL;
				}
				delete MJob;
			}*/
		}
		else if ( !bHelpActive )
		{
			Hud->CheckButtons();
			// check whether the mouse is over an unit menu:
			if ( ( SelectedVehicle&&SelectedVehicle->MenuActive&&SelectedVehicle->MouseOverMenu ( mouse->x,mouse->y ) ) ||
			        ( SelectedBuilding&&SelectedBuilding->MenuActive&&SelectedBuilding->MouseOverMenu ( mouse->x,mouse->y ) ) )
			{
			}
			else
				// check, if the player wants to attack:
				if ( mouse->cur==GraphicsData.gfx_Cattack&&SelectedVehicle&&!SelectedVehicle->Attacking&&!SelectedVehicle->MoveJobActive )
				{
					//find target ID
					int targetId = 0;
					if (SelectedVehicle->data.can_attack == ATTACK_AIR || ATTACK_AIRnLAND )
					{
						cVehicle* target = Map->GO[mouse->GetKachelOff()].plane;
						if (target) targetId = target->iID;
					}
					else if (SelectedVehicle->data.can_attack == ATTACK_LAND || ATTACK_AIRnLAND || ATTACK_SUB_LAND )
					{
						cVehicle* target = Map->GO[mouse->GetKachelOff()].vehicle;
						if (target) targetId = target->iID;
					}
					sendWantAttack( targetId, mouse->GetKachelOff(), SelectedVehicle->iID, true );
				}
				else if ( mouse->cur == GraphicsData.gfx_Cattack && SelectedBuilding && !SelectedBuilding->Attacking )
				{
					//find target ID
					int targetId = 0;
					if (SelectedBuilding->data.can_attack == ATTACK_AIR || ATTACK_AIRnLAND )
					{
						cVehicle* target = Map->GO[mouse->GetKachelOff()].plane;
						if (target) targetId = target->iID;
					}
					else if (SelectedVehicle->data.can_attack == ATTACK_LAND || ATTACK_AIRnLAND || ATTACK_SUB_LAND )
					{
						cVehicle* target = Map->GO[mouse->GetKachelOff()].vehicle;
						if (target) targetId = target->iID;
					}
					int offset = SelectedBuilding->PosX + SelectedBuilding->PosY * Map->size;
					sendWantAttack( targetId, mouse->GetKachelOff(), offset, false );
				}
				else if ( mouse->cur == GraphicsData.gfx_Csteal && SelectedVehicle )
				{
					// TODO: add commando steal
				}
				else if ( mouse->cur == GraphicsData.gfx_Cdisable && SelectedVehicle )
				{
					// TODO: add commando disable
				}
				else
					// select the unit:
					if ( OverObject && !Hud->Ende )
					{
						if ( SelectedVehicle && ( OverObject->plane == SelectedVehicle || OverObject->vehicle == SelectedVehicle ) )
						{
							if ( !SelectedVehicle->moving && !SelectedVehicle->rotating&&SelectedVehicle->owner == ActivePlayer )
							{
								SelectedVehicle->MenuActive = true;
								PlayFX ( SoundData.SNDHudButton );
							}
						}
						else if ( SelectedBuilding&& ( OverObject->base == SelectedBuilding || OverObject->top == SelectedBuilding ) )
						{
							if ( SelectedBuilding->owner == ActivePlayer )
							{
								SelectedBuilding->MenuActive = true;
								PlayFX ( SoundData.SNDHudButton );
							}
						}
						else if ( OverObject->plane && !OverObject->plane->moving && !OverObject->plane->rotating )
						{
							bChangeObjectName = false;
							if ( SelectedVehicle == OverObject->plane )
							{
								if ( SelectedVehicle->owner == ActivePlayer )
								{
									SelectedVehicle->MenuActive = true;
									PlayFX ( SoundData.SNDHudButton );
								}
							}
							else
							{
								if ( SelectedVehicle )
								{
									SelectedVehicle->Deselct();
									SelectedVehicle = NULL;
									StopFXLoop ( iObjectStream );
								}
								else if ( SelectedBuilding )
								{
									SelectedBuilding->Deselct();
									SelectedBuilding = NULL;
									StopFXLoop ( iObjectStream );
								}
								SelectedVehicle = OverObject->plane;
								SelectedVehicle->Select();
								iObjectStream = SelectedVehicle->PlayStram();
							}
						}
						else if ( OverObject->vehicle && !OverObject->vehicle->moving && !OverObject->vehicle->rotating && !( OverObject->plane && ( OverObject->vehicle->MenuActive || OverObject->vehicle->owner != ActivePlayer ) ) )
						{
							bChangeObjectName = false;
							if ( SelectedVehicle == OverObject->vehicle )
							{
								if ( SelectedVehicle->owner == ActivePlayer )
								{
									SelectedVehicle->MenuActive = true;
									PlayFX ( SoundData.SNDHudButton );
								}
							}
							else
							{
								if ( SelectedVehicle )
								{
									SelectedVehicle->Deselct();
									SelectedVehicle = NULL;
									StopFXLoop ( iObjectStream );
								}
								else if ( SelectedBuilding )
								{
									SelectedBuilding->Deselct();
									SelectedBuilding = NULL;
									StopFXLoop ( iObjectStream );
								}
								SelectedVehicle = OverObject->vehicle;
								SelectedVehicle->Select();
								iObjectStream = SelectedVehicle->PlayStram();
							}
						}
						else if ( OverObject->top )
						{
							bChangeObjectName = false;
							if ( SelectedBuilding == OverObject->top )
							{
								if ( SelectedBuilding->owner == ActivePlayer )
								{
									SelectedBuilding->MenuActive = true;
									PlayFX ( SoundData.SNDHudButton );
								}
							}
							else
							{
								if ( SelectedVehicle )
								{
									SelectedVehicle->Deselct();
									SelectedVehicle = NULL;
									StopFXLoop ( iObjectStream );
								}
								else if ( SelectedBuilding )
								{
									SelectedBuilding->Deselct();
									SelectedBuilding = NULL;
									StopFXLoop ( iObjectStream );
								}
								SelectedBuilding = OverObject->top;
								SelectedBuilding->Select();
								iObjectStream = SelectedBuilding->PlayStram();
							}
						}
						else if ( OverObject->base && OverObject->base->owner )
						{
							bChangeObjectName = false;
							if ( SelectedBuilding == OverObject->base )
							{
								if ( SelectedBuilding->owner == ActivePlayer )
								{
									SelectedBuilding->MenuActive = true;
									PlayFX ( SoundData.SNDHudButton );
								}
							}
							else
							{
								if ( SelectedVehicle )
								{
									SelectedVehicle->Deselct();
									SelectedVehicle = NULL;
									StopFXLoop ( iObjectStream );
								}
								else if ( SelectedBuilding )
								{
									SelectedBuilding->Deselct();
									SelectedBuilding = NULL;
									StopFXLoop ( iObjectStream );
								}
								SelectedBuilding = OverObject->base;
								SelectedBuilding->Select();
								iObjectStream = SelectedBuilding->PlayStram();
							}
						}

					}
			// check whether the name of a unit has to be changed:
			if ( SelectedVehicle&&SelectedVehicle->owner==ActivePlayer&&mouse->x>=10&&mouse->y>=29&&mouse->x<10+128&&mouse->y<29+10 )
			{
				InputStr = SelectedVehicle->name;
				bChangeObjectName=true;
			}
			else if ( SelectedBuilding&&SelectedBuilding->owner==ActivePlayer&&mouse->x>=10&&mouse->y>=29&&mouse->x<10+128&&mouse->y<29+10 )
			{
				InputStr = SelectedBuilding->name;
				bChangeObjectName=true;
			}
		}
		else if ( OverObject )
		{
			if ( OverObject->plane )
			{
				OverObject->plane->ShowHelp();
			}
			else if ( OverObject->vehicle )
			{
				OverObject->vehicle->ShowHelp();
			}
			else if ( OverObject->top )
			{
				OverObject->top->ShowHelp();
			}
			else if ( OverObject->base )
			{
				OverObject->base->ShowHelp();
			}
			bHelpActive=false;
		}
	}
	if ( iMouseButton && !bHelpActive )
	{
		Hud->CheckOneClick();
	}
	Hud->CheckMouseOver();
	Hud->CheckScroll();
	iLastMouseButton = iMouseButton;
	return 0;
}

void cClient::addMoveJob(cVehicle* vehicle, int iDestOffset)
{
	cMJobs *MJob = new cMJobs ( Map, vehicle->PosX+vehicle->PosY*Map->size, iDestOffset, vehicle->data.can_drive == DRIVE_AIR, vehicle->iID, PlayerList, false );
	if ( !MJob->finished )
	{
		if ( MJob->CalcPath() )
		{
			MJob->CalcNextDir();
			sendMoveJob ( MJob );
			addActiveMoveJob ( MJob );
			cLog::write("(Client) Added new movejob: VehicleID: " + iToStr ( vehicle->iID ) + ", SrcX: " + iToStr ( vehicle->PosX ) + ", SrcY: " + iToStr ( vehicle->PosY ) + ", DestX: " + iToStr ( MJob->DestX ) + ", DestY: " + iToStr ( MJob->DestY ), cLog::eLOG_TYPE_NET_DEBUG);
		}
		else
		{
			MJob->finished = true;
			if (!vehicle || !vehicle->autoMJob) //automoving suveyors must not tell this
			{
				if ( random ( 2,0 ) ) PlayVoice ( VoiceData.VOINoPath1 );
				else PlayVoice ( VoiceData.VOINoPath2 );
			}
		}
	}
	if ( MJob->finished )
	{
		if ( MJob->vehicle )
		{
			MJob->vehicle->mjob = NULL;
		}
		delete MJob;
	}
}

void cClient::handleTimer()
{
	//iTimer0: 50ms
	//iTimer1: 100ms
	//iTimer2: 400ms

	static unsigned int iLast = 0, i = 0;
	iTimer0 = 0 ;
	iTimer1 = 0;
	iTimer2 = 0;
	if ( iTimerTime != iLast )
	{
		iLast = iTimerTime;
		i++;
		iTimer0 = 1;
		if ( i&0x1 ) iTimer1 = 1;
		if ( ( i&0x3 ) == 3 ) iTimer2 = 1;
	}
}

void cClient::drawMap( bool bPure )
{
	int iX, iY, iPos, iZoom, iOffX, iOffY, iStartX, iStartY, iEndX, iEndY;
	struct sTerrain *terr, *defwater;
	SDL_Rect dest, tmp, scr;
	iZoom = Hud->Zoom;
	float f = 64.0;
	iOffX = ( int ) ( Hud->OffX/ ( f/iZoom ) );
	iOffY = ( int ) ( Hud->OffY/ ( f/iZoom ) );
	scr.y = 0;
	scr.h = scr.w = dest.w = dest.h = iZoom;
	dest.y = 18-iOffY;
	defwater = Map->terrain + Map->DefaultWater;
	for ( iY=0;iY<Map->size;iY++ )
	{
		dest.x=180-iOffX;
		if ( dest.y>=18-iZoom )
		{
			iPos=iY*Map->size;
			for ( iX=0;iX<Map->size;iX++ )
			{
				if ( dest.x>=180-iZoom )
				{
					// draw thr terrain:
					tmp=dest;
					terr=Map->terrain+Map->Kacheln[iPos];
					// check whether it is a coast:
					if ( terr->overlay )
					{
						scr.x= ( iFrame%defwater->frames ) *iZoom;
						if ( Hud->Nebel&&!ActivePlayer->ScanMap[iPos] )
						{
							SDL_BlitSurface ( defwater->shw,&scr,buffer,&tmp );
						}
						else
						{
							SDL_BlitSurface ( defwater->sf,&scr,buffer,&tmp );
						}
						tmp=dest;
					}
					// draw the fog:
					if ( Hud->Nebel&&!ActivePlayer->ScanMap[iPos] )
					{
						if ( terr->sf_org->w>64 )
						{
							scr.x= ( iFrame%terr->frames ) *iZoom;
							SDL_BlitSurface ( terr->shw,&scr,buffer,&tmp );
						}
						else
						{
							SDL_BlitSurface ( terr->shw,NULL,buffer,&tmp );
						}
					}
					else
					{
						if ( terr->frames>1 )
						{
							scr.x= ( iFrame%terr->frames ) *iZoom;
							SDL_BlitSurface ( terr->sf,&scr,buffer,&tmp );
						}
						else
						{
							SDL_BlitSurface ( terr->sf,NULL,buffer,&tmp );
						}
					}
				}
				iPos++;
				dest.x+=iZoom;
				if ( dest.x>SettingsData.iScreenW-13 ) break;
			}
		}
		dest.y+=iZoom;
		if ( dest.y>SettingsData.iScreenH-15 ) break;
	}
	// draw the grid:
	if ( Hud->Gitter )
	{
		dest.x=180;
		dest.y=18+iZoom- ( iOffY%iZoom );
		dest.w=SettingsData.iScreenW-192;
		dest.h=1;
		for ( iY=0;iY< ( SettingsData.iScreenH-32 ) /iZoom+1;iY++ )
		{
			SDL_FillRect ( buffer,&dest,GRID_COLOR );
			dest.y+=iZoom;
		}
		dest.x=180+iZoom- ( iOffX%iZoom );
		dest.y=18;
		dest.w=1;
		dest.h=SettingsData.iScreenH-32;
		for ( iX = 0; iX < ( SettingsData.iScreenW-192 ) /iZoom+1;iX++ )
		{
			SDL_FillRect ( buffer,&dest,GRID_COLOR );
			dest.x+=iZoom;
		}
	}
	if ( bPure ) return;

	// display the FX-Bottom-Effects:
	displayFXBottom();

	// draw sub- and basebuildings:
	iStartX= ( Hud->OffX-1 ) /64;if ( iStartX<0 ) iStartX=0;
	iStartY= ( Hud->OffY-1 ) /64;if ( iStartY<0 ) iStartY=0;
	iStartX-=1;if ( iStartX<0 ) iStartX=0;
	iStartY-=1;if ( iStartY<0 ) iStartY=0;
	iEndX=Hud->OffX/64+ ( SettingsData.iScreenW-192 ) /Hud->Zoom+1;
	if ( iEndX>=Map->size ) iEndX=Map->size-1;
	iEndY=Hud->OffY/64+ ( SettingsData.iScreenH-32 ) /Hud->Zoom+1;
	if ( iEndY>=Map->size ) iEndY=Map->size-1;
	dest.y=18-iOffY+iZoom*iStartY;
	for ( iY=iStartY;iY<=iEndY;iY++ )
	{
		dest.x=180-iOffX+iZoom*iStartX;
		iPos=iY*Map->size+iStartX;
		for ( iX=iStartX;iX<=iEndX;iX++ )
		{
			if ( ActivePlayer->ScanMap[iPos] )
			{
				if ( Map->GO[iPos].subbase&&Map->GO[iPos].subbase->PosX==iX&&Map->GO[iPos].subbase->PosY==iY )
				{
					Map->GO[iPos].subbase->Draw ( &dest );
				}
				if ( Map->GO[iPos].base&&Map->GO[iPos].base->PosX==iX&&Map->GO[iPos].base->PosY==iY )
				{
					Map->GO[iPos].base->Draw ( &dest );
				}
			}
			iPos++;
			dest.x+=iZoom;
		}
		dest.y+=iZoom;
	}
	// draw vehicles:
	dest.y=18-iOffY+iZoom*iStartY;
	for ( iY=iStartY;iY<=iEndY;iY++ )
	{
		dest.x=180-iOffX+iZoom*iStartX;
		iPos=iY*Map->size+iStartX;
		for ( iX=iStartX;iX<=iEndX;iX++ )
		{
			if ( ActivePlayer->ScanMap[iPos] )
			{
				if ( Map->GO[iPos].vehicle&&Map->GO[iPos].vehicle->PosX==iX&&Map->GO[iPos].vehicle->PosY==iY )
				{
					Map->GO[iPos].vehicle->Draw ( &dest );
				}
			}
			iPos++;
			dest.x+=iZoom;
		}
		dest.y+=iZoom;
	}
	// draw top buildings:
	iStartY-=1;if ( iStartY<0 ) iStartY=0;
	iStartX-=1;if ( iStartX<0 ) iStartX=0;
	dest.y=18-iOffY+iZoom*iStartY;
	for ( iY=iStartY;iY<=iEndY;iY++ )
	{
		dest.x=180-iOffX+iZoom*iStartX;
		iPos=iY*Map->size+iStartX;
		for ( iX=iStartX;iX<=iEndX;iX++ )
		{
			if ( ActivePlayer->ScanMap[iPos]||
			        ( Map->GO[iPos].top&&Map->GO[iPos].top->data.is_big&& ( ( iX<iEndX&&ActivePlayer->ScanMap[iPos+1] ) || ( iY<iEndY&&ActivePlayer->ScanMap[iPos+Map->size] ) || ( iX<iEndX&&iY<iEndY&&ActivePlayer->ScanMap[iPos+Map->size+1] ) ) ) )
			{
				if ( Map->GO[iPos].top&&Map->GO[iPos].top->PosX==iX&&Map->GO[iPos].top->PosY==iY )
				{
					Map->GO[iPos].top->Draw ( &dest );
					if ( bDebugBaseClient )
					{
						sSubBase *sb;
						tmp=dest;
						if ( tmp.h>8 ) tmp.h=8;
						sb=Map->GO[iPos].top->SubBase;
						SDL_FillRect ( buffer,&tmp, (long int) sb );
						font->showText(dest.x+1,dest.y+1, iToStr(( long int ) ( sb )), LATIN_SMALL_WHITE);
						string sTmp = "m "+iToStr(sb->Metal)+"/"+iToStr(sb->MaxMetal)+" +"+iToStr(sb->MetalProd-sb->MetalNeed);
						font->showText(dest.x+1,dest.y+1+8, sTmp, LATIN_SMALL_WHITE);

						sTmp = "o "+iToStr(sb->Oil)+"/"+iToStr(sb->MaxOil)+" +"+iToStr(sb->OilProd-sb->OilNeed);
						font->showText(dest.x+1,dest.y+1+16, sTmp, LATIN_SMALL_WHITE);

						sTmp = "g "+iToStr(sb->Gold)+"/"+iToStr(sb->MaxGold)+" +"+iToStr(sb->GoldProd-sb->GoldNeed);
						font->showText(dest.x+1,dest.y+1+24, sTmp, LATIN_SMALL_WHITE);
					}
					if ( bDebugBaseServer )
					{
						sSubBase *sb;
						tmp=dest;
						if ( tmp.h>8 ) tmp.h=8;
						sb=Server->Map->GO[iPos].top->SubBase;
						SDL_FillRect ( buffer,&tmp, (long int) sb );
						font->showText(dest.x+1,dest.y+1, iToStr(( long int ) ( sb )), LATIN_SMALL_WHITE);
						string sTmp = "m "+iToStr(sb->Metal)+"/"+iToStr(sb->MaxMetal)+" +"+iToStr(sb->MetalProd-sb->MetalNeed);
						font->showText(dest.x+1,dest.y+1+8, sTmp, LATIN_SMALL_WHITE);

						sTmp = "o "+iToStr(sb->Oil)+"/"+iToStr(sb->MaxOil)+" +"+iToStr(sb->OilProd-sb->OilNeed);
						font->showText(dest.x+1,dest.y+1+16, sTmp, LATIN_SMALL_WHITE);

						sTmp = "g "+iToStr(sb->Gold)+"/"+iToStr(sb->MaxGold)+" +"+iToStr(sb->GoldProd-sb->GoldNeed);
						font->showText(dest.x+1,dest.y+1+24, sTmp, LATIN_SMALL_WHITE);
					}
				}
			}
			iPos++;
			dest.x+=iZoom;
		}
		dest.y+=iZoom;
	}
	// draw the planes:
	dest.y=18-iOffY+iZoom*iStartY;
	scr.x=0;scr.y=0;
	scr.h=scr.w=iZoom;

	for ( iY=iStartY;iY<=iEndY;iY++ )
	{
		dest.x=180-iOffX+iZoom*iStartX;
		iPos=iY*Map->size+iStartX;
		for ( iX=iStartX;iX<=iEndX;iX++ )
		{
			if ( ActivePlayer->ScanMap[iPos] )
			{
				if ( Map->GO[iPos].plane )
				{
					Map->GO[iPos].plane->Draw ( &dest );
				}
			}
			iPos++;
			dest.x+=iZoom;
		}
		dest.y+=iZoom;
	}
	// draw the resources:
	if ( Hud->Studie|| ( SelectedVehicle&&SelectedVehicle->owner==ActivePlayer&&SelectedVehicle->data.can_survey ) )
	{
		scr.y=0;
		scr.h=scr.w=iZoom;
		dest.y=18-iOffY+iZoom*iStartY;
		for ( iY=iStartY;iY<=iEndY;iY++ )
		{
			dest.x=180-iOffX+iZoom*iStartX;
			iPos=iY*Map->size+iStartX;
			for ( iX=iStartX;iX<=iEndX;iX++ )
			{
				if ( ActivePlayer->ResourceMap[iPos] )
				{
					if ( Map->Resources[iPos].typ==RES_NONE )
					{
						scr.x=0;
						tmp=dest;
						SDL_BlitSurface ( ResourceData.res_metal,&scr,buffer,&tmp );
					}
					else
					{
						scr.x=Map->Resources[iPos].value*iZoom;
						tmp=dest;
						if ( Map->Resources[iPos].typ==RES_METAL )
						{
							SDL_BlitSurface ( ResourceData.res_metal,&scr,buffer,&tmp );
						}
						else if ( Map->Resources[iPos].typ==RES_OIL )
						{
							SDL_BlitSurface ( ResourceData.res_oil,&scr,buffer,&tmp );
						}
						else
						{
							SDL_BlitSurface ( ResourceData.res_gold,&scr,buffer,&tmp );
						}
					}
				}
				iPos++;
				dest.x+=iZoom;
			}
			dest.y+=iZoom;
		}
	}
	// draw the path:
	if ( SelectedVehicle&& ( ( SelectedVehicle->mjob&&SelectedVehicle->mjob->Suspended ) ||SelectedVehicle->BuildPath ) )
	{
		SelectedVehicle->DrawPath();
	}
	// debug sentry:
	if ( bDebugWache )
	{
		scr.y=0;
		scr.h=scr.w=iZoom;
		dest.y=18-iOffY+iZoom*iStartY;
		for ( iY=iStartY;iY<=iEndY;iY++ )
		{
			dest.x=180-iOffX+iZoom*iStartX;
			iPos=iY*Map->size+iStartX;
			for ( iX=iStartX;iX<=iEndX;iX++ )
			{
				if ( ActivePlayer->WachMapAir[iPos] )
				{
					if ( ActivePlayer->ScanMap[iPos] )
					{
						font->showText(dest.x+1,dest.y+1, "A+", LATIN_SMALL_YELLOW);
					}
					else
					{
						font->showText(dest.x+1,dest.y+1, "A-", LATIN_SMALL_YELLOW);
					}
				}
				if ( ActivePlayer->WachMapGround[iPos] )
				{
					if ( ActivePlayer->ScanMap[iPos] )
					{
						font->showText(dest.x+10,dest.y+1, "G+", LATIN_SMALL_YELLOW);
					}
					else
					{
						font->showText(dest.x+10,dest.y+1, "G-", LATIN_SMALL_YELLOW);
					}
				}
				iPos++;
				dest.x+=iZoom;
			}
			dest.y+=iZoom;
		}
	}
}

void cClient::drawMiniMap()
{
	unsigned int cl,*ptr;
	int x, y, tx, ty, ex, ey;
	sGameObjects *GO;

	GO=Map->GO;
	SDL_LockSurface ( GraphicsData.gfx_hud );
	ptr= ( ( unsigned int* ) GraphicsData.gfx_hud->pixels );
	// draw the background:
	for ( y=0;y<112;y++ )
	{
		ty= ( int ) ( ( Map->size/112.0 ) *y );
		ty*=Map->size;
		for ( x=0;x<112;x++ )
		{
			tx= ( int ) ( ( Map->size/112.0 ) *x );
			if ( Hud->Radar&&!ActivePlayer->ScanMap[tx+ty] )
			{
				cl=* ( unsigned int* ) Map->terrain[Map->Kacheln[tx+ty]].shw_org->pixels;
			}
			else if ( ActivePlayer->ScanMap[tx+ty]&&GO[tx+ty].base&&GO[tx+ty].base->detected&&ActivePlayer->ScanMap[tx+ty]&&GO[tx+ty].base->owner&& ( !Hud->TNT|| ( GO[tx+ty].base->data.can_attack ) ) )
			{
				cl=* ( unsigned int* ) GO[tx+ty].base->owner->color->pixels;
			}
			else if ( ActivePlayer->ScanMap[tx+ty]&&GO[tx+ty].top&& ( !Hud->TNT|| ( GO[tx+ty].top->data.can_attack ) ) )
			{
				cl=* ( unsigned int* ) GO[tx+ty].top->owner->color->pixels;
			}
			else if ( ActivePlayer->ScanMap[tx+ty]&&GO[tx+ty].plane&& ( !Hud->TNT|| ( GO[tx+ty].plane->data.can_attack ) ) )
			{
				cl=* ( unsigned int* ) GO[tx+ty].plane->owner->color->pixels;
			}
			else if ( ActivePlayer->ScanMap[tx+ty]&&GO[tx+ty].vehicle&&GO[tx+ty].vehicle->detected&& ( !Hud->TNT|| ( GO[tx+ty].vehicle->data.can_attack ) ) )
			{
				cl=* ( unsigned int* ) GO[tx+ty].vehicle->owner->color->pixels;
			}
			else
			{
				cl=* ( unsigned int* ) Map->terrain[Map->Kacheln[tx+ty]].sf_org->pixels;
			}

			if ( cl==0xFF00FF )
			{
				cl=* ( unsigned int* ) Map->terrain[Map->DefaultWater].sf_org->pixels;
			}
			else if ( ( cl&0xFFFFFF ) ==0xCD00CD )
			{
				cl=* ( unsigned int* ) Map->terrain[Map->DefaultWater].shw_org->pixels;
			}

			ptr[15+356*GraphicsData.gfx_hud->w+x+y*GraphicsData.gfx_hud->w]=cl;
		}
	}
	// draw the borders:
	tx= ( int ) ( ( Hud->OffX/64.0 ) * ( 112.0/Map->size ) );
	ty= ( int ) ( ( Hud->OffY/64.0 ) * ( 112.0/Map->size ) );
	ex= ( int ) ( 112/ ( Map->size/ ( ( ( SettingsData.iScreenW-192.0 ) /Hud->Zoom ) ) ) );
	ey= ( int ) ( ty+112/ ( Map->size/ ( ( ( SettingsData.iScreenH-32.0 ) /Hud->Zoom ) ) ) );
	ex+=tx;
	for ( y=ty;y<ey;y++ )
	{
		ptr[y*GraphicsData.gfx_hud->w+15+356*GraphicsData.gfx_hud->w+tx]=MINIMAP_COLOR;
		ptr[y*GraphicsData.gfx_hud->w+15+356*GraphicsData.gfx_hud->w+tx+ex-tx-1]=MINIMAP_COLOR;
	}
	for ( x=tx;x<ex;x++ )
	{
		ptr[x+15+356*GraphicsData.gfx_hud->w+ty*GraphicsData.gfx_hud->w]=MINIMAP_COLOR;
		ptr[x+15+356*GraphicsData.gfx_hud->w+ ( ty+ey-1-ty ) *GraphicsData.gfx_hud->w]=MINIMAP_COLOR;
	}
	SDL_UnlockSurface ( GraphicsData.gfx_hud );
}

void cClient::drawFLC()
{
	SDL_Rect dest;
	string stmp;
	if (  FLC == NULL || ( SelectedVehicle == NULL && SelectedBuilding == NULL ) ) return;
	// draw the video:
	dest.x=10;
	dest.y=29;
	dest.w=128;
	dest.h=128;
	if ( FLC )
	{
		SDL_BlitSurface ( FLC->surface, NULL, buffer, &dest );
	}
	else
	{
		SDL_BlitSurface ( video, NULL, buffer, &dest );
	}
	// display the name of the unit:
	if ( SelectedVehicle )
	{
		if ( bChangeObjectName )
		{
			dest.y+=2;
			dest.h=6;
			SDL_FillRect ( buffer,&dest,0x404040 );
			if ( iFrame%2 )
			{
				stmp = InputStr; stmp += "_";
				font->showText(10, 32, stmp, LATIN_SMALL_GREEN);
			}
			else
			{
				font->showText(10, 32, InputStr, LATIN_SMALL_GREEN);
			}
		}
		else
		{
			font->showText(10, 32, SelectedVehicle->name, LATIN_SMALL_GREEN);
		}
		font->showText(10, 40, SelectedVehicle->GetStatusStr(), LATIN_SMALL_WHITE);
	}
	else if ( SelectedBuilding )
	{
		if ( bChangeObjectName )
		{
			dest.y+=2;
			dest.h=6;
			SDL_FillRect ( buffer,&dest,0x404040 );
			if ( iFrame%2 )
			{
				stmp = InputStr; stmp += "_";
				font->showText(10, 32, stmp, LATIN_SMALL_GREEN);
			}
			else
			{
				font->showText(10, 32, InputStr, LATIN_SMALL_GREEN);
			}
		}
		else
		{
			font->showText(10, 32, SelectedBuilding->name, LATIN_SMALL_GREEN);
		}
		font->showText(10, 40, SelectedBuilding->GetStatusStr(), LATIN_SMALL_WHITE);
	}
}

void cClient::displayFX()
{
	if ( !FXList->iCount ) return;

	for ( int i = FXList->iCount-1; i >= 0; i-- )
	{
		drawFX ( i );
	}
}

void cClient::displayFXBottom()
{
	if ( !FXListBottom->iCount ) return;

	for ( int i = FXListBottom->iCount-1; i >= 0; i-- )
	{
		drawFXBottom ( i );
	}
}

void cClient::drawFX( int iNum )
{
	SDL_Rect scr,dest;
	sFX *fx;

	fx=FXList->Items[iNum];
	if ( ( !ActivePlayer->ScanMap[fx->PosX/64+fx->PosY/64*Map->size] ) &&fx->typ!=fxRocket ) return;
	switch ( fx->typ )
	{
		case fxMuzzleBig:
			if ( iFrame - fx->StartFrame > 2 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x=Hud->Zoom*fx->param;
			scr.y=0;
			dest.w=scr.w=Hud->Zoom;
			dest.h=scr.h=Hud->Zoom;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_big[1],&scr,buffer,&dest );
			break;
		case fxMuzzleSmall:
			if ( iFrame - fx->StartFrame > 2 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x=Hud->Zoom*fx->param;
			scr.y=0;
			dest.w=scr.w=Hud->Zoom;
			dest.h=scr.h=Hud->Zoom;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_small[1],&scr,buffer,&dest );
			break;
		case fxMuzzleMed:
			if ( iFrame - fx->StartFrame > 2 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x=Hud->Zoom*fx->param;
			scr.y=0;
			dest.w=scr.w=Hud->Zoom;
			dest.h=scr.h=Hud->Zoom;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_med[1],&scr,buffer,&dest );
			break;
		case fxMuzzleMedLong:
			if ( iFrame - fx->StartFrame > 5 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x=Hud->Zoom*fx->param;
			scr.y=0;
			dest.w=scr.w=Hud->Zoom;
			dest.h=scr.h=Hud->Zoom;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_med[1],&scr,buffer,&dest );
			break;
		case fxHit:
			if ( iFrame - fx->StartFrame > 5 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x=Hud->Zoom* ( iFrame-fx->StartFrame );
			scr.y=0;
			dest.w=scr.w=Hud->Zoom;
			dest.h=scr.h=Hud->Zoom;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_hit[1],&scr,buffer,&dest );
			break;
		case fxExploSmall:
			if ( iFrame - fx->StartFrame > 14 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x = (int) Hud->Zoom * 114 * ( iFrame - fx->StartFrame ) / 64.0;
			scr.y = 0;
			scr.w = (int) Hud->Zoom * 114 / 64.0;
			scr.h = (int) Hud->Zoom * 108 / 64.0;
			dest.x = 180 - ( (int) ( ( Hud->OffX- ( fx->PosX - 57 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y = 18 -  ( (int) ( ( Hud->OffY- ( fx->PosY - 54 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_explo_small[1], &scr, buffer, &dest );
			break;
		case fxExploBig:
			if ( iFrame - fx->StartFrame > 28 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x = (int) Hud->Zoom * 307 * ( iFrame - fx->StartFrame ) / 64.0;
			scr.y = 0;
			scr.w = (int) Hud->Zoom * 307 / 64.0;
			scr.h = (int) Hud->Zoom * 194 / 64.0;
			dest.x = 180- ( (int) ( ( Hud->OffX- ( fx->PosX - 134 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y = 18-  ( (int) ( ( Hud->OffY- ( fx->PosY - 85 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_explo_big[1], &scr, buffer, &dest );
			break;
		case fxExploWater:
			if ( iFrame - fx->StartFrame > 14 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x = (int) Hud->Zoom * 114 * ( iFrame - fx->StartFrame ) / 64.0;
			scr.y = 0;
			scr.w = (int) Hud->Zoom * 114 / 64.0;
			scr.h = (int) Hud->Zoom * 108 / 64.0;
			dest.x = 180- ( (int) ( ( Hud->OffX- ( fx->PosX - 57 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y = 18-  ( (int) ( ( Hud->OffY- ( fx->PosY - 54 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_explo_water[1],&scr,buffer,&dest );
			break;
		case fxExploAir:
			if ( iFrame - fx->StartFrame > 14 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x = (int) Hud->Zoom * 137 * ( iFrame - fx->StartFrame ) / 64.0;
			scr.y = 0;
			scr.w = (int) Hud->Zoom * 137 / 64.0;
			scr.h = (int) Hud->Zoom * 121 / 64.0;
			dest.x = 180- ( ( int ) ( ( Hud->OffX- ( fx->PosX - 61 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y = 18-  ( ( int ) ( ( Hud->OffY- ( fx->PosY - 68 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_explo_air[1],&scr,buffer,&dest );
			break;
		case fxSmoke:
			if ( iFrame-fx->StartFrame>100/4 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			SDL_SetAlpha ( EffectsData.fx_smoke[1],SDL_SRCALPHA,100- ( iFrame-fx->StartFrame ) *4 );
			scr.y=scr.x=0;
			dest.w=scr.w=EffectsData.fx_smoke[1]->h;
			dest.h=scr.h=EffectsData.fx_smoke[1]->h;
			dest.x=180- ( ( int ) ( ( Hud->OffX- ( fx->PosX-EffectsData.fx_smoke[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY- ( fx->PosY-EffectsData.fx_smoke[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_smoke[1],&scr,buffer,&dest );
			break;
		case fxRocket:
		{
			sFXRocketInfos *ri;
			ri= fx->rocketInfo;
			if ( abs ( fx->PosX-ri->DestX ) <64&&abs ( fx->PosY-ri->DestY ) <64 )
			{
				ri->aj->MuzzlePlayed=true;
				delete ri;
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			if ( iTimer0 )
			{
				int k;
				for ( k=0;k<64;k+=8 )
				{
					if ( SettingsData.bAlphaEffects ) addFX ( fxSmoke, ( int ) ri->fpx, ( int ) ri->fpy,0 );
					ri->fpx+=ri->mx*8;
					ri->fpy-=ri->my*8;
					drawFX ( FXList->iCount-1 );
				}
			}

			fx->PosX= ( int ) ri->fpx;
			fx->PosY= ( int ) ri->fpy;
			scr.x=ri->dir*EffectsData.fx_rocket[1]->h;
			scr.y=0;
			scr.h=scr.w=dest.h=dest.w=EffectsData.fx_rocket[1]->h;
			dest.x=180- ( ( int ) ( ( Hud->OffX- ( fx->PosX-EffectsData.fx_rocket[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY- ( fx->PosY-EffectsData.fx_rocket[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_rocket[1],&scr,buffer,&dest );
			break;
		}
		case fxDarkSmoke:
		{
			sFXDarkSmoke *dsi;
			dsi = fx->smokeInfo;
			if ( iFrame-fx->StartFrame>50||dsi->alpha<=1 )
			{
				delete fx;
				delete dsi;
				FXList->Delete ( iNum );
				return;
			}
			scr.x= ( int ) ( 0.375*Hud->Zoom ) * ( iFrame-fx->StartFrame );
			scr.y=0;
			dest.w=scr.w=EffectsData.fx_dark_smoke[1]->h;
			dest.h=scr.h=EffectsData.fx_dark_smoke[1]->h;
			dest.x=180- ( ( int ) ( ( Hud->OffX- ( ( int ) dsi->fx ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY- ( ( int ) dsi->fy ) ) / ( 64.0/Hud->Zoom ) ) );

			SDL_SetAlpha ( EffectsData.fx_dark_smoke[1],SDL_SRCALPHA,dsi->alpha );
			SDL_BlitSurface ( EffectsData.fx_dark_smoke[1],&scr,buffer,&dest );

			if ( iTimer0 )
			{
				dsi->fx+=dsi->dx;
				dsi->fy+=dsi->dy;
				dsi->alpha-=3;
				if ( dsi->alpha<=0 ) dsi->alpha=1;
			}
			break;
		}
		case fxAbsorb:
		{
			if ( iFrame-fx->StartFrame>10 )
			{
				delete fx;
				FXList->Delete ( iNum );
				return;
			}
			scr.x=Hud->Zoom* ( iFrame-fx->StartFrame );
			scr.y=0;
			dest.w=scr.w=Hud->Zoom;
			dest.h=scr.h=Hud->Zoom;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_absorb[1],&scr,buffer,&dest );
			break;
		}
	}
}

void cClient::drawFXBottom( int iNum )
{
	SDL_Rect scr,dest;
	sFX *fx;

	fx=FXListBottom->Items[iNum];
	if ( ( !ActivePlayer->ScanMap[fx->PosX/64+fx->PosY/64*Map->size] ) &&fx->typ!=fxTorpedo ) return;
	switch ( fx->typ )
	{
		case fxTorpedo:
		{
			sFXRocketInfos *ri;
			ri = fx->rocketInfo;
			int x,y;
			if ( abs ( fx->PosX-ri->DestX ) <64&&abs ( fx->PosY-ri->DestY ) <64 )
			{
				ri->aj->MuzzlePlayed=true;
				delete ri;
				delete fx;
				FXListBottom->Delete ( iNum );
				return;
			}

			if ( iTimer0 )
			{
				int k;
				for ( k=0;k<64;k+=8 )
				{
					if ( SettingsData.bAlphaEffects ) addFX ( fxBubbles, ( int ) ri->fpx, ( int ) ri->fpy,0 );
					ri->fpx+=ri->mx*8;
					ri->fpy-=ri->my*8;
					drawFXBottom ( FXListBottom->iCount-1 );
				}
			}

			fx->PosX= ( int ) ( ri->fpx );
			fx->PosY= ( int ) ( ri->fpy );
			scr.x=ri->dir*EffectsData.fx_rocket[1]->h;
			scr.y=0;
			scr.h=scr.w=dest.h=dest.w=EffectsData.fx_rocket[1]->h;
			dest.x=180- ( ( int ) ( ( Hud->OffX- ( fx->PosX-EffectsData.fx_rocket[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY- ( fx->PosY-EffectsData.fx_rocket[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_rocket[1],&scr,buffer,&dest );

			x= ( ( int ) ( ( ( dest.x-180 ) +Hud->OffX/ ( 64.0/Hud->Zoom ) ) /Hud->Zoom ) );
			y= ( ( int ) ( ( ( dest.y-18 ) +Hud->OffY/ ( 64.0/Hud->Zoom ) ) /Hud->Zoom ) );

			if ( !Map->IsWater ( x+y*Map->size,false ) &&
			        ! ( abs ( fx->PosX-ri->DestX ) <64&&abs ( fx->PosY-ri->DestY ) <64 ) &&
			        ! ( Map->GO[x+y*Map->size].base&&Map->GO[x+y*Map->size].base->owner&& ( Map->GO[x+y*Map->size].base->data.is_bridge||Map->GO[x+y*Map->size].base->data.is_platform ) ) )
			{
				ri->aj->DestX=ri->aj->ScrX;
				ri->aj->DestY=ri->aj->ScrY;
				ri->aj->MuzzlePlayed=true;
				delete ri;
				delete fx;
				FXListBottom->Delete ( iNum );
				return;
			}
			break;
		}
		case fxTracks:
		{
			sFXTracks *tri;
			tri = fx->trackInfo;
			if ( tri->alpha<=1 )
			{
				delete fx;
				delete tri;
				FXListBottom->Delete ( iNum );
				return;
			}
			scr.y=0;
			dest.w=scr.w=dest.h=scr.h=EffectsData.fx_tracks[1]->h;
			scr.x=tri->dir*scr.w;
			dest.x=180- ( ( int ) ( ( Hud->OffX- ( fx->PosX ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY- ( fx->PosY ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_SetAlpha ( EffectsData.fx_tracks[1],SDL_SRCALPHA,tri->alpha );
			SDL_BlitSurface ( EffectsData.fx_tracks[1],&scr,buffer,&dest );

			if ( iTimer0 )
			{
				tri->alpha--;
			}
			break;
		}
		case fxBubbles:
			if ( iFrame-fx->StartFrame>100/4 )
			{
				delete fx;
				FXListBottom->Delete ( iNum );
				return;
			}
			SDL_SetAlpha ( EffectsData.fx_smoke[1],SDL_SRCALPHA,100- ( iFrame-fx->StartFrame ) *4 );
			scr.y=scr.x=0;
			dest.w=scr.w=EffectsData.fx_smoke[1]->h;
			dest.h=scr.h=EffectsData.fx_smoke[1]->h;
			dest.x=180- ( ( int ) ( ( Hud->OffX- ( fx->PosX-EffectsData.fx_smoke[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY- ( fx->PosY-EffectsData.fx_smoke[0]->h/2+32 ) ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_smoke[1],&scr,buffer,&dest );
			break;
		case fxCorpse:
			SDL_SetAlpha ( EffectsData.fx_corpse[1],SDL_SRCALPHA,fx->param-- );
			scr.y=scr.x=0;
			dest.w=scr.w=EffectsData.fx_corpse[1]->h;
			dest.h=scr.h=EffectsData.fx_corpse[1]->h;
			dest.x=180- ( ( int ) ( ( Hud->OffX-fx->PosX ) / ( 64.0/Hud->Zoom ) ) );
			dest.y=18- ( ( int ) ( ( Hud->OffY-fx->PosY ) / ( 64.0/Hud->Zoom ) ) );
			SDL_BlitSurface ( EffectsData.fx_corpse[1],&scr,buffer,&dest );

			if ( fx->param<=0 )
			{
				delete fx;
				FXListBottom->Delete ( iNum );
				return;
			}
			break;
	}
}

void cClient::drawCircle( int iX, int iY, int iRadius, int iColor, SDL_Surface *surface )
{
	int d,da,db,xx,yy,bry;
	unsigned int *ptr;
	if ( iX + iRadius < 0 || iX - iRadius > SettingsData.iScreenW || iY + iRadius < 0 || iY - iRadius > SettingsData.iScreenH ) return;
	SDL_LockSurface ( surface );
	ptr = ( unsigned int* ) surface->pixels;
	iY *= SettingsData.iScreenW;

	d = 0;
	xx = 0;
	yy = iRadius;
	bry = ( int ) Round ( 0.70710678*iRadius,0 );
	while ( yy > bry )
	{
		da=d+ ( xx<<1 ) +1;
		db=da- ( yy<<1 ) +1;
		if ( abs ( da ) <abs ( db ) )
		{
			d=da;
			xx++;
		}
		else
		{
			d=db;
			xx++;
			yy--;
		}
#define PUTC(xxx,yyy) if((xxx)+iX>=0&&(xxx)+iX<SettingsData.iScreenW&&(yyy)*SettingsData.iScreenW+iY>=0&&(yyy)*SettingsData.iScreenW+iY<SettingsData.iScreenH*SettingsData.iScreenW)ptr[(xxx)+iX+(yyy)*SettingsData.iScreenW+iY] = iColor;
		PUTC ( xx,yy )
		PUTC ( yy,xx )
		PUTC ( yy,-xx )
		PUTC ( xx,-yy )
		PUTC ( -xx,yy )
		PUTC ( -yy,xx )
		PUTC ( -yy,-xx )
		PUTC ( -xx,-yy )
	}
	SDL_UnlockSurface ( surface );
}

void cClient::drawExitPoint( int iX, int iY )
{
	SDL_Rect dest, scr;
	int iNr;
	int iZoom;
	iNr = iFrame%5;
	iZoom = Hud->Zoom;
	scr.y = 0;
	scr.h = scr.w = iZoom;
	scr.x = iZoom*iNr;
	dest.y = iY;
	dest.x = iX;
	dest.w = dest.h = iZoom;
	SDL_BlitSurface ( GraphicsData.gfx_exitpoints, &scr, buffer, &dest );
}

void cClient::setWind( int iDir )
{
	fWindDir = iDir/57.29577;
}

void cClient::makePanel( bool bOpen )
{
	SDL_Rect top,bottom,tmp;
	if ( bOpen )
	{
		PlayFX ( SoundData.SNDPanelOpen );
		top.x=0;top.y= ( SettingsData.iScreenH/2 )-479;
		top.w=bottom.w=171;
		top.h=479;bottom.h=481;
		bottom.x=0;bottom.y= ( SettingsData.iScreenH/2 );
		tmp=top;
		SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&tmp );
		tmp=bottom;
		SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&tmp );
		while ( top.y>-479 )
		{
			SHOW_SCREEN
			SDL_Delay ( 10 );
			top.y-=10;
			bottom.y+=10;
			SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
			tmp=top;
			SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&tmp );
			SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&bottom );
		}
	}
	else
	{
		PlayFX ( SoundData.SNDPanelClose );
		top.x=0;top.y=-480;
		top.w=bottom.w=171;
		top.h=479;bottom.h=481;
		bottom.x=0;bottom.y=SettingsData.iScreenH;
		while ( bottom.y>SettingsData.iScreenH/2 )
		{
			SHOW_SCREEN
			SDL_Delay ( 10 );
			top.y+=10;
			if ( top.y> ( SettingsData.iScreenH/2 )-479-9 ) top.y= ( SettingsData.iScreenH/2 )-479;
			bottom.y-=10;
			if ( bottom.y<SettingsData.iScreenH/2+9 ) bottom.y=SettingsData.iScreenH/2;
			SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
			tmp=top;
			SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&tmp );
			tmp=bottom;
			SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&tmp );
		}
		SHOW_SCREEN
		SDL_Delay ( 100 );
	}
}

void cClient::rotateBlinkColor()
{
	static bool bDec = true;
	if ( bDec )
	{
		iBlinkColor -= 0x0A0A0A;
		if ( iBlinkColor <= 0xA0A0A0 ) bDec = false;
	}
	else
	{
		iBlinkColor += 0x0A0A0A;
		if ( iBlinkColor >= 0xFFFFFF ) bDec = true;
	}
}

// Fügt einen FX-Effekt ein:
void cClient::addFX ( eFXTyps typ,int x,int y, sFXRocketInfos* param )
{
	sFX* n = new sFX;
	n->typ = typ;
	n->PosX = x;
	n->PosY = y;
	n->StartFrame = iFrame;
	n->param = 0;
	n->rocketInfo= param;
	n->smokeInfo = NULL;
	n->trackInfo = NULL;
	addFX( n );
}

// Fügt einen FX-Effekt ein:
void cClient::addFX ( eFXTyps typ,int x,int y,int param )
{
	sFX* n = new sFX;
	n->typ = typ;
	n->PosX = x;
	n->PosY = y;
	n->StartFrame = iFrame;
	n->param = param;
	n->rocketInfo= NULL;
	n->smokeInfo = NULL;
	n->trackInfo = NULL;
	addFX( n );
}

// Fügt einen FX-Effekt ein:
void cClient::addFX ( sFX* n )
{
	if ( (n->typ == fxRocket || n->typ == fxTorpedo ) && n->rocketInfo == NULL )
	{
		//invalid effect
		//rocketInfo is missing
		delete n;
		return;
	}

	if ( (n->typ != fxRocket && n->typ != fxTorpedo ) && n->rocketInfo != NULL )
	{
		//invalid effect
		//an sFXRocketInfos has been passed, but is not needed for this effect type
		delete n->rocketInfo;
		delete n;
		return;
	}

	if ( n->typ==fxTracks||n->typ==fxTorpedo||n->typ==fxBubbles||n->typ==fxCorpse )
	{
		FXListBottom->Add ( n );
	}
	else
	{
		FXList->Add ( n );
	}
	switch ( n->typ )
	{
		case fxExploAir:
			int nr;
			nr = random ( 3,0 );
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
				nr=random ( 3,0 );
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
				nr=random ( 3,0 );
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
				if ( random ( 2,0 ) )
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
				nr=random ( 4,0 );
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
		case fxSmoke:
		case fxBubbles:
			n->param=0;
			break;
		case fxRocket:
		case fxTorpedo:
		{
			sFXRocketInfos *ri;
			int dx,dy;
			ri= n->rocketInfo;
			ri->fpx=n->PosX;
			ri->fpy=n->PosY;
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
			sFXDarkSmoke *dsi;
			dsi=new sFXDarkSmoke;
			dsi->alpha=n->param;
			if ( dsi->alpha>150 ) dsi->alpha=150;
			n->smokeInfo = dsi;
			dsi->fx=n->PosX;
			dsi->fy=n->PosY;

			ax=x=sin ( fWindDir );
			ay=y=cos ( fWindDir );
			if ( ax<0 ) ax=-ax;
			if ( ay<0 ) ay=-ay;
			if ( ax>ay )
			{
				dsi->dx=x*2+ ( random ( 5,0 ) /10.0 );
				dsi->dy=y*2+ ( ( random ( 15,0 )-7 ) /14.0 );
			}
			else
			{
				dsi->dx=x*2+ ( ( random ( 15,0 )-7 ) /14.0 );
				dsi->dy=y*2+ ( random ( 5,0 ) /10.0 );
			}
			break;
		}
		case fxTracks:
		{
			sFXTracks *tri;
			tri=new sFXTracks;
			tri->alpha=100;
			tri->dir=n->param;
			n->trackInfo = tri;
			break;
		}
		case fxCorpse:
			n->param=255;
			break;
		case fxAbsorb:
			PlayFX ( SoundData.SNDAbsorb );
			break;
	}
}


bool cClient::doCommand ( string sCmd )
{
	/*if ( sCmd.compare( "fps on" ) == 0 ) {DebugFPS=true;FPSstart=SDL_GetTicks();frames=0;cycles=0;return true;}
	if ( sCmd.compare( "fps off" ) == 0 ) {DebugFPS=false;return true;}*/
	if ( sCmd.compare( "base client" ) == 0 ) { bDebugBaseClient = true; bDebugBaseServer = false; return true; }
	if ( sCmd.compare( "base server" ) == 0 ) { if (Server) bDebugBaseServer = true; bDebugBaseClient = false; return true; }
	if ( sCmd.compare( "base off" ) == 0 ) { bDebugBaseServer = false; bDebugBaseClient = false; return true; }
	if ( sCmd.compare( "wache on" ) == 0 ) { bDebugWache =true; return true; }
	if ( sCmd.compare( "wache off" ) == 0 ) { bDebugWache =false; return true; }
	if ( sCmd.compare( "fx on" ) == 0 ) { bDebugFX = true; return true; }
	if ( sCmd.compare( "fx off" ) == 0 ) { bDebugFX = false; return true; }
	if ( sCmd.compare( "trace server" ) == 0 ) { if ( Server ) bDebugTraceServer = true; bDebugTraceClient = false; return true; }
	if ( sCmd.compare( "trace client" ) == 0 ) { bDebugTraceClient = true; bDebugTraceServer = false; return true; }
	if ( sCmd.compare( "trace off" ) == 0 ) { bDebugTraceServer = false; bDebugTraceClient = false; return true; }

	if ( sCmd.substr( 0, 6 ).compare( "color " ) == 0 ) {int cl=0;sscanf ( sCmd.c_str(),"color %d",&cl );cl%=8;ActivePlayer->color=OtherData.colors[cl];return true;}
	if ( sCmd.compare( "fog off" ) == 0 )
	{
		//memset ( ActivePlayer->ScanMap,1,Map->size*Map->size );
		sPlayerCheat = ActivePlayer->name + " " + lngPack.i18n( "Text~Comp~Cheat");
		sPlayerCheat+=" \"Fog Off\"";
		return true;
	}

	if ( sCmd.compare( "survey" ) == 0 )
	{
		//memset ( ActivePlayer->ResourceMap,1,Map->size*Map->size );
		sPlayerCheat=ActivePlayer->name + " " + lngPack.i18n( "Text~Comp~Cheat");
		sPlayerCheat+=" \"Survey\"";
		return true;
	}

	if ( sCmd.compare( "credits" ) == 0 )
	{
		//ActivePlayer->Credits+=1000;
		sPlayerCheat = ActivePlayer->name + " " + lngPack.i18n( "Text~Comp~Cheat");
		sPlayerCheat+=" \"Credits\"";
		return true;
	}
	if ( sCmd.substr( 0, 5 ).compare( "kill " ) == 0 )
	{
		int x,y;
		sscanf ( sCmd.c_str(),"kill %d,%d",&x,&y );
		/*engine->DestroyObject ( x+y*Map->size,false );
		engine->DestroyObject ( x+y*Map->size,true );*/
		sPlayerCheat=ActivePlayer->name + " " + lngPack.i18n( "Text~Comp~Cheat");
		sPlayerCheat+=" \"Kill\"";
		return true;
	}
	if ( sCmd.compare( "god off" ) == 0 )
	{
		int i;
		for ( i=0;i<Map->size*Map->size;i++ )
		{
			if ( Map->GO[i].plane )
			{
				//engine->DestroyObject ( i,true );
			}
			if ( Map->GO[i].vehicle || ( Map->GO[i].base&&Map->GO[i].base->owner ) ||Map->GO[i].top )
			{
				//engine->DestroyObject ( i,false );
			}
			memset ( ActivePlayer->ScanMap,1,Map->size*Map->size );
		}
		sPlayerCheat = ActivePlayer->name + " " + lngPack.i18n( "Text~Comp~Cheat");
		sPlayerCheat += " \"God Off\"";
		return true;
	}
	if ( sCmd.compare( "load" ) == 0 )
	{
		sPlayerCheat = ActivePlayer->name + " " + lngPack.i18n( "Text~Comp~Cheat");
		sPlayerCheat += " \"Load\"";

		/*if ( SelectedVehicle ) {SelectedVehicle->data.cargo=SelectedVehicle->data.max_cargo;SelectedVehicle->data.ammo=SelectedVehicle->data.max_ammo;SelectedVehicle->ShowDetails();}
		else if ( SelectedBuilding )
		{
			if ( SelectedBuilding->data.can_load==TRANS_METAL )
			{
				SelectedBuilding->SubBase->Metal-=SelectedBuilding->data.cargo;
				SelectedBuilding->data.cargo=SelectedBuilding->data.max_cargo;
				SelectedBuilding->SubBase->Metal+=SelectedBuilding->data.cargo;
			}
			else if ( SelectedBuilding->data.can_load==TRANS_OIL )
			{
				SelectedBuilding->SubBase->Oil-=SelectedBuilding->data.cargo;
				SelectedBuilding->data.cargo=SelectedBuilding->data.max_cargo;
				SelectedBuilding->SubBase->Oil+=SelectedBuilding->data.cargo;
			}
			else if ( SelectedBuilding->data.can_load==TRANS_GOLD )
			{
				SelectedBuilding->SubBase->Gold-=SelectedBuilding->data.cargo;
				SelectedBuilding->data.cargo=SelectedBuilding->data.max_cargo;
				SelectedBuilding->SubBase->Gold+=SelectedBuilding->data.cargo;
			}
			SelectedBuilding->data.ammo=SelectedBuilding->data.max_ammo;SelectedBuilding->ShowDetails();
		}*/
		return true;
	}
	return false;
}

void cClient::mouseMoveCallback ( bool bForce )
{
	static int iLastX = -1, iLastY = -1;
	SDL_Rect scr, dest;
	sGameObjects *GO;

	int iX, iY;
	mouse->GetKachel ( &iX, &iY );
	if ( iX == iLastX && iY == iLastY && !bForce ) return;
	iLastX = iX;
	iLastY = iY;
	// re-establish the hud:
	scr.x=262;
	scr.y=25;
	scr.w=64;
	scr.h=16;
	dest.x=265;
	dest.y=SettingsData.iScreenH-21;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	scr.x=64;
	scr.y=198;
	scr.w=215;
	dest.x=342;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	if ( iX == -1 )
	{
		return;
	}
	// draw the coordinates:
	/*array to get map coords in sceme XXX-YYY\0 = 8 characters
	a case where I accept an array since I don't know a better
	method to format x and y easily with leading 0 -- beko */
	char str[8];
	sprintf ( str, "%0.3d-%0.3d", iX, iY );
	font->showTextCentered(265+32, ( SettingsData.iScreenH-21 ) +4, str, LATIN_NORMAL, GraphicsData.gfx_hud);

	if ( !ActivePlayer->ScanMap[iX+iY*Map->size] )
	{
		OverObject=NULL;
		if ( mouse->cur==GraphicsData.gfx_Cattack )
		{
			SDL_Rect r;
			r.x=1;r.y=29;
			r.h=3;r.w=35;
			SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0 );
		}
		return;
	}
	// check wether there is a Go under the mouse:
	GO=Map->GO+ ( Map->size*iY+iX );
	if ( mouse->cur == GraphicsData.gfx_Csteal && SelectedVehicle )
	{
		SelectedVehicle->DrawCommandoCursor ( GO,true );
	}
	else if ( mouse->cur == GraphicsData.gfx_Cdisable && SelectedVehicle )
	{
		SelectedVehicle->DrawCommandoCursor ( GO,false );
	}
	if ( GO->vehicle != NULL&& ( GO->vehicle->detected || GO->vehicle->owner == ActivePlayer ) )
	{
		OverObject=GO;
		font->showTextCentered(343+106, ( SettingsData.iScreenH-21 ) +4, GO->vehicle->name, LATIN_NORMAL, GraphicsData.gfx_hud);
		if ( mouse->cur==GraphicsData.gfx_Cattack )
		{
			if ( SelectedVehicle )
			{
				SelectedVehicle->DrawAttackCursor ( GO,SelectedVehicle->data.can_attack );
			}
			else if ( SelectedBuilding )
			{
				SelectedBuilding->DrawAttackCursor ( GO,SelectedBuilding->data.can_attack );
			}
		}
	}
	else if ( GO->plane!=NULL )
	{
		OverObject=GO;
		font->showTextCentered(343+106, ( SettingsData.iScreenH-21 ) +4, GO->plane->name, LATIN_NORMAL, GraphicsData.gfx_hud);
		if ( mouse->cur==GraphicsData.gfx_Cattack )
		{
			if ( SelectedVehicle )
			{
				SelectedVehicle->DrawAttackCursor ( GO, SelectedVehicle->data.can_attack );
			}
			else if ( SelectedBuilding )
			{
				SelectedBuilding->DrawAttackCursor ( GO, SelectedBuilding->data.can_attack );
			}
		}
	}
	else if ( GO->top!=NULL )
	{
		OverObject=GO;
		font->showTextCentered(343+106, ( SettingsData.iScreenH-21 ) +4, GO->top->name, LATIN_NORMAL, GraphicsData.gfx_hud);
		if ( mouse->cur==GraphicsData.gfx_Cattack )
		{
			if ( SelectedVehicle )
			{
				SelectedVehicle->DrawAttackCursor ( GO, SelectedVehicle->data.can_attack );
			}
			else if ( SelectedBuilding )
			{
				SelectedBuilding->DrawAttackCursor ( GO, SelectedBuilding->data.can_attack );
			}
		}
	}
	else if ( GO->base!=NULL&&GO->base->owner&&(GO->base->owner == ActivePlayer || GO->base->detected) )
	{
		OverObject=GO;
		font->showTextCentered(343+106, ( SettingsData.iScreenH-21 ) +4, GO->base->name, LATIN_NORMAL, GraphicsData.gfx_hud);
		if ( mouse->cur==GraphicsData.gfx_Cattack )
		{
			if ( SelectedVehicle )
			{
				SelectedVehicle->DrawAttackCursor ( GO, SelectedVehicle->data.can_attack );
			}
			else if ( SelectedBuilding )
			{
				SelectedBuilding->DrawAttackCursor ( GO, SelectedBuilding->data.can_attack );
			}
		}
	}
	else
	{
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			SDL_Rect r;
			r.x=1;r.y=29;
			r.h=3;r.w=35;
			SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0 );
		}
		OverObject=NULL;
	}
	// place band:
	if ( SelectedVehicle && SelectedVehicle->PlaceBand )
	{
		SelectedVehicle->FindNextband();
	}
}

// Adds an message to be displayed in the game
void cClient::addMessage ( string sMsg )
{
	sMessage *Message;
	Message = new sMessage;
	Message->chars = (int)sMsg.length();
	Message->msg = ( char* ) malloc ( Message->chars+1 );
	strcpy ( Message->msg, sMsg.c_str() );
	if ( Message->chars > 500 ) Message->msg[500]=0;
	Message->len = font->getTextWide( sMsg );
	Message->age = iFrame;
	messages.Add ( Message );
	if(SettingsData.bDebug) cLog::write(Message->msg, cLog::eLOG_TYPE_DEBUG);
}

// displays a message with 'goto' coordinates
void cClient::addCoords (const string msg,int x,int y )
{
 	stringstream strStream;
 	//e.g. [85,22] missel MK I is under attack (F1)
 	strStream << "[" << x << "," << y << "] " << msg << " (" << GetKeyString ( KeysList.KeyJumpToAction ) << ")";
	Client->addMessage ( strStream.str() );
	iMsgCoordsX=x;
	iMsgCoordsY=y;
}

void cClient::handleMessages()
{
	SDL_Rect scr, dest;
	int iHeight;
	sMessage *message;
	if ( messages.iCount == 0 ) return;
	iHeight = 0;
	// Alle alten Nachrichten löschen:
	for ( int i = messages.iCount-1; i >= 0; i-- )
	{
		message = messages.Items[i];
		if ( message->age+MSG_FRAMES < iFrame || iHeight > 200 )
		{
			free ( message->msg );
			free ( message );
			messages.Delete ( i );
			continue;
		}
		iHeight += 14+11*message->len/296;
	}
	if ( messages.iCount == 0 ) return;
	if ( SettingsData.bAlphaEffects )
	{
		scr.x = 0; scr.y = 0;
		dest.x = 180; dest.y = 30;
		dest.w = scr.w = 250;
		dest.h = scr.h = iHeight+6;
		SDL_BlitSurface ( GraphicsData.gfx_shadow, &scr, buffer, &dest );
	}
	dest.x = 180+2; dest.y = 34;
	dest.w = 250-4;
	dest.h = iHeight;
	for ( int i = 0; i < messages.iCount; i++ )
	{
		message = messages.Items[i];
		font->showTextAsBlock( dest, message->msg );
		dest.y += 14+11*message->len/300;
	}
}

int cClient::HandleNetMessage( cNetMessage* message )
{
	cLog::write("Client: received message, type: " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	switch ( message->iType )
	{
	case GAME_EV_LOST_CONNECTION:
		// This is just temporary so doesn't need to be translated
		addMessage( "Lost connection to Server" );
		break;
	case GAME_EV_CHAT_SERVER:
		switch (message->popChar())
		{
		case USER_MESSAGE:
			//TODO: play sound for incoming user chat message
			addMessage( message->popString() );
			break;
		case SERVER_ERROR_MESSAGE:
			PlayFX ( SoundData.SNDQuitsch );
			addMessage( lngPack.i18n( message->popString() ) );
			break;
		case SERVER_INFO_MESSAGE:
			addMessage( lngPack.i18n( message->popString() ) );
			break;
		}
		break;
	case GAME_EV_ADD_BUILDING:
		{
			cBuilding *AddedBuilding;
			bool Init = message->popBool();
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			int UnitNum = message->popInt16();
			int PosY = message->popInt16();
			int PosX = message->popInt16();

			AddedBuilding = Player->AddBuilding ( PosX, PosY, UnitsData.building + UnitNum );
			AddedBuilding->iID = message->popInt16();

			addUnit ( PosX, PosY, AddedBuilding, Init );

			// play placesound if it is a mine
			if ( UnitNum == BNrLandMine && Player == ActivePlayer ) PlayFX ( SoundData.SNDLandMinePlace );
			else if ( UnitNum == BNrSeaMine && Player == ActivePlayer ) PlayFX ( SoundData.SNDSeaMinePlace );
		}
		break;
	case GAME_EV_ADD_VEHICLE:
		{
			cVehicle *AddedVehicle;
			bool Init = message->popBool();
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			int UnitNum = message->popInt16();
			int PosY = message->popInt16();
			int PosX = message->popInt16();

			AddedVehicle = Player->AddVehicle ( PosX, PosY, UnitsData.vehicle + UnitNum );
			AddedVehicle->iID = message->popInt16();

			addUnit ( PosX, PosY, AddedVehicle, Init );
		}
		break;
	case GAME_EV_DEL_BUILDING:
		{
			cBuilding *Building = NULL;
			bool bSubBase = message->popBool ();
			bool bBase = message->popBool ();
			bool bPlane = message->popBool ();
			int iPlayer = message->popInt16 ();
			int iPosY = message->popInt16 ();
			int iPosX = message->popInt16 ();

			cPlayer *Player = getPlayerFromNumber ( iPlayer );

			int iOff = iPosX + iPosY*Map->size;

			if ( bBase ) Building = Map->GO[iOff].base;
			else if ( bSubBase ) Building = Map->GO[iOff].subbase;
			else Building = Map->GO[iOff].top;

			// play clearsound if it is a mine
			if ( Building->typ->nr == BNrLandMine && Player == ActivePlayer ) PlayFX ( SoundData.SNDLandMineClear );
			else if ( Building->typ->nr == BNrSeaMine && Player == ActivePlayer ) PlayFX ( SoundData.SNDSeaMineClear );

			if ( Building && Building->owner == Player )
			{
				deleteUnit ( Building );
			}
		}
		break;
	case GAME_EV_DEL_VEHICLE:
		{
			cVehicle *Vehicle = NULL;
			bool bSubBase = message->popBool ();
			bool bBase = message->popBool ();
			bool bPlane = message->popBool ();
			int iPlayer = message->popInt16 ();
			int iPosY = message->popInt16 ();
			int iPosX = message->popInt16 ();

			cPlayer *Player = getPlayerFromNumber ( iPlayer );
			int iOff = iPosX + iPosY*Map->size;

			if ( bPlane ) Vehicle = Map->GO[iOff].plane;
			else Vehicle = Map->GO[iOff].vehicle;

			if ( Vehicle && Vehicle->owner == Player )
			{
				deleteUnit ( Vehicle );
			}
		}
		break;
	case GAME_EV_ADD_ENEM_VEHICLE:
		{
			cVehicle *AddedVehicle;
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );

			int iUnitNumber = message->popInt16();
			int iPosY = message->popInt16();
			int iPosX = message->popInt16();
			AddedVehicle = Player->AddVehicle ( iPosX, iPosY, UnitsData.vehicle + iUnitNumber );

			AddedVehicle->dir = message->popInt16();
			AddedVehicle->iID = message->popInt16();

			addUnit ( iPosX, iPosY, AddedVehicle, false );
		}
		break;
	case GAME_EV_ADD_ENEM_BUILDING:
		{
			cBuilding *AddedBuilding;
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			int iUnitNumber = message->popInt16();
			int iPosY = message->popInt16();
			int iPosX = message->popInt16();

			AddedBuilding = Player->AddBuilding ( iPosX, iPosY, UnitsData.building + iUnitNumber );
			AddedBuilding->iID = message->popInt16();
			addUnit ( iPosX, iPosY, AddedBuilding, false );
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
				Hud->ShowRunde();
				Hud->EndeButton ( false );
				Hud->showTurnTime ( -1 );
			}

			if ( bWaitForNextPlayer )
			{
				if ( iNextPlayerNum != ActivePlayer->Nr )
				{
					if ( bWaitForOthers == true )
					{
						drawMap();
						SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );
						drawMiniMap();
						drawFLC();
					}
					else bWaitForOthers = true;
					waitForOtherPlayer( iNextPlayerNum );
				}
				else
				{
					bWaitForOthers = false;
				}
			}
			else if ( iNextPlayerNum != -1 )
			{
				makeHotSeatEnd( iNextPlayerNum );
			}

			if ( iNextPlayerNum == -1 || iNextPlayerNum == ActivePlayer->Nr )
			{
				addMessage( lngPack.i18n( "Text~Comp~Turn_Start") + " " + iToStr( iTurn ) );

				switch ( message->popInt16() )
				{
				case 0:
					PlayVoice ( VoiceData.VOIStartNone );
					break;
				case 1:
					addMessage( message->popString() );
					PlayVoice ( VoiceData.VOIStartOne );
					break;
				case 2:
					addMessage( message->popString() );
					PlayVoice ( VoiceData.VOIStartMore );
					break;
				}
			}
		}
		break;
	case GAME_EV_FINISHED_TURN:
		{
			int iPlayerNum = message->popInt16();
			int iTimeDelay = message->popInt16();

			if ( iTimeDelay != -1 )
			{
				if ( iPlayerNum != ActivePlayer->Nr ) addMessage( getPlayerFromNumber( iPlayerNum )->name + " " + lngPack.i18n( "Text~Multiplayer~Player_Turn_End") + ". " + lngPack.i18n( "Text~Multiplayer~Deadline", iToStr( iTimeDelay ) ) );
				iTurnTime = iTimeDelay;
				iStartTurnTime = SDL_GetTicks();
			}
			else if ( iPlayerNum != ActivePlayer->Nr ) addMessage( getPlayerFromNumber( iPlayerNum )->name + " " + lngPack.i18n( "Text~Multiplayer~Player_Turn_End") );
		}
		break;
	case GAME_EV_UNIT_DATA:
		{
			cPlayer *Player = getPlayerFromNumber ( message->popInt16() );
			sUnitData *Data;

			int iID = message->popInt16();
			bool bVehicle = message->popBool();
			int iPosY = message->popInt16();
			int iPosX = message->popInt16();

			cVehicle *Vehicle = NULL;
			cBuilding *Building = NULL;

			cLog::write("Received Unit Data: Vehicle: " + iToStr ( (int)bVehicle ) + ", ID: " + iToStr ( iID ) + ", XPos: " + iToStr ( iPosX ) + ", YPos: " +iToStr ( iPosY ), cLog::eLOG_TYPE_NET_DEBUG);
			// unit is a vehicle
			if ( bVehicle )
			{
				bool bPlane = message->popBool();
				Vehicle = getVehicleFromID ( iID );

				if ( !Vehicle )
				{
					cLog::write("Unknown vehicle with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_WARNING);
					break;
				}
				if ( Vehicle->PosX != iPosX || Vehicle->PosY != iPosY )
				{
					// if the vehicle is moving it is normal that the positions are not the same,
					// so the log message will just be an debug one
					int iLogType = cLog::eLOG_TYPE_NET_DEBUG;

					// set to server position if vehicle is not moving
					if ( !Vehicle->moving )
					{
						iLogType = cLog::eLOG_TYPE_NET_WARNING;

						if ( bPlane ) Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
						else Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;

						if ( bPlane ) Map->GO[iPosX+iPosY*Map->size].plane = Vehicle;
						else Map->GO[iPosX+iPosY*Map->size].vehicle = Vehicle;

						Vehicle->PosX = iPosX;
						Vehicle->PosY = iPosY;
					}
					cLog::write("Vehicle identificated by ID (" + iToStr( iID ) + ") but has wrong position [IS: X" + iToStr( Vehicle->PosX ) + " Y" + iToStr( Vehicle->PosY ) + "; SHOULD: X" + iToStr( iPosX ) + " Y" + iToStr( iPosY ) + "]", iLogType );
				}

				Vehicle->name = message->popString();
				Vehicle->Disabled = message->popInt16();
				Vehicle->IsClearing = message->popBool();
				Vehicle->IsBuilding = message->popBool();
				Vehicle->Wachposten = message->popBool();

				Data = &Vehicle->data;
			}
			else
			{
				bool bBase = message->popBool();
				bool bSubBase = message->popBool();
				Building = getBuildingFromID ( iID );

				if ( !Building )
				{
					cLog::write("Unknown building with ID: "  + iToStr( iID ) , cLog::eLOG_TYPE_NET_ERROR);
					break;
				}
				if ( Building->PosX != iPosX || Building->PosY != iPosY )
				{
					cLog::write("Building identificated by ID (" + iToStr( iID ) + ") but has wrong position [IS: X" + iToStr( Vehicle->PosX ) + " Y" + iToStr( Vehicle->PosY ) + "; SHOULD: X" + iToStr( iPosX ) + " Y" + iToStr( iPosY ) + "]", cLog::eLOG_TYPE_NET_WARNING);
					// set to server position
					if ( bBase ) Map->GO[iPosX+iPosY*Map->size].base = NULL;
					else if ( bSubBase ) Map->GO[iPosX+iPosY*Map->size].subbase = NULL;
					else Map->GO[iPosX+iPosY*Map->size].top = NULL;

					if ( bBase ) Map->GO[Building->PosX+Building->PosY*Map->size].base = Building;
					else if ( bSubBase ) Map->GO[Building->PosX+Building->PosY*Map->size].subbase = Building;
					else Map->GO[Building->PosX+Building->PosY*Map->size].top = Building;

					Building->PosX = iPosX;
					Building->PosY = iPosY;
				}

				Building->name = message->popString();
				Building->Disabled = message->popInt16();
				Building->IsWorking = message->popBool();
				Building->Wachposten = message->popBool();

				Data = &Building->data;
			}

			Data->costs = message->popInt16();
			Data->ammo = message->popInt16();
			Data->max_ammo = message->popInt16();
			Data->cargo = message->popInt16();
			Data->max_cargo = message->popInt16();
			Data->damage = message->popInt16();
			Data->shots = message->popInt16();
			Data->max_shots = message->popInt16();
			Data->range = message->popInt16();
			Data->scan = message->popInt16();
			Data->armor = message->popInt16();
			Data->hit_points = message->popInt16();
			Data->max_hit_points = message->popInt16();
			Data->version = message->popInt16();

			if ( bVehicle )
			{
				if ( Data->can_lay_mines )
				{
					if ( Data->cargo <= 0 ) Vehicle->LayMines = false;
					if ( Data->cargo >= Data->max_cargo ) Vehicle->ClearMines = false;
				}
				Data->speed = message->popInt16();
				Data->max_speed = message->popInt16();
				if ( Vehicle == SelectedVehicle ) Vehicle->ShowDetails();
			}
			else if ( Building == SelectedBuilding ) Building->ShowDetails();
		}
		break;
	case GAME_EV_DO_START_WORK:
		{
			int offset = message->popInt32();
			if (offset < 0 || offset > Client->Map->size * Client->Map->size ) break;

			cBuilding* building = Client->Map->GO[offset].top;
			if ( building == NULL ) break;

			building->ClientStartWork();

			//if the message is not for the owner of the building, no subbase data follows
			if ( message->iPlayerNr != building->owner->Nr ) break;

			building->SubBase->GoldProd = message->popInt16();
			building->SubBase->OilProd = message->popInt16();
			building->SubBase->MetalProd = message->popInt16();
			building->SubBase->GoldNeed = message->popInt16();
			building->SubBase->MetalNeed = message->popInt16();
			building->SubBase->EnergyNeed = message->popInt16();
			building->SubBase->OilNeed = message->popInt16();
			building->SubBase->EnergyProd = message->popInt16();
			building->SubBase->HumanNeed = message->popInt16();
		}
		break;
	case GAME_EV_DO_STOP_WORK:
		{
			int offset = message->popInt32();
			if (offset < 0 || offset > Client->Map->size * Client->Map->size ) break;

			cBuilding* building = Client->Map->GO[offset].top;
			if ( building == NULL ) break;

			building->ClientStopWork();

			//if the message is not for the owner of the building, no subbase data follows
			if ( message->iPlayerNr != building->owner->Nr ) break;

			building->SubBase->GoldProd = message->popInt16();
			building->SubBase->OilProd = message->popInt16();
			building->SubBase->MetalProd = message->popInt16();
			building->SubBase->GoldNeed = message->popInt16();
			building->SubBase->MetalNeed = message->popInt16();
			building->SubBase->EnergyNeed = message->popInt16();
			building->SubBase->OilNeed = message->popInt16();
			building->SubBase->EnergyProd = message->popInt16();
			building->SubBase->HumanNeed = message->popInt16();
		}
		break;
	case GAME_EV_MOVE_JOB_SERVER:
		{
			cMJobs *MJob = NULL;
			int iCount = 0;
			int iWaypointOff;

			int iID = message->popInt16();
			int iSrcOff = message->popInt16();
			int iDestOff = message->popInt16();
			bool bPlane = message->popBool();
			int iReceivedCount = message->popInt16();

			cLog::write("(Client) Received MoveJob: VehicleID: " + iToStr( iID ) + ", SrcX: " + iToStr( iSrcOff%Map->size ) + ", SrcY: " + iToStr( iSrcOff/Map->size ) + ", DestX: " + iToStr( iDestOff%Map->size ) + ", DestY: " + iToStr( iDestOff/Map->size ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);
			// Add the waypoints to the movejob
			sWaypoint *Waypoint = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
			while ( iCount < iReceivedCount )
			{
				iWaypointOff = message->popInt16();
				Waypoint->X = iWaypointOff%Map->size;
				Waypoint->Y = iWaypointOff/Map->size;
				Waypoint->Costs = message->popInt16();
				Waypoint->next = NULL;

				if ( iCount == 0 )
				{
					if ( iWaypointOff == iSrcOff )
					{
						MJob = new cMJobs( Map, iSrcOff, iDestOff, bPlane, iID, PlayerList, false );
						if ( MJob->vehicle == NULL )
						{
							// warning, here is something wrong! ( out of sync? )
							cLog::write("(Client) Created new movejob but no vehicle found!", cLog::eLOG_TYPE_NET_WARNING);
							break;
						}
						MJob->waypoints = Waypoint;
					}
					else
					{
						cVehicle *Vehicle = getVehicleFromID ( iID );
						if ( Vehicle == NULL || Vehicle->mjob == NULL )
						{
							// warning, here is something wrong! ( out of sync? )
							cLog::write("(Client) Error while adding waypoints: Can't find vehicle or movejob", cLog::eLOG_TYPE_NET_WARNING);
							break;
						}
						MJob = Vehicle->mjob;
						sWaypoint *LastWaypoint = MJob->waypoints;
						while ( LastWaypoint->next )
						{
							LastWaypoint = LastWaypoint->next;
						}
						LastWaypoint->next = Waypoint;
					}
				}
				iCount++;

				if ( iCount < iReceivedCount )
				{
					Waypoint->next = ( sWaypoint* ) malloc ( sizeof ( sWaypoint ) );
					Waypoint = Waypoint->next;
				}
			}
			// is the last waypoint in this message?
			if ( iWaypointOff == iDestOff && MJob )
			{
				MJob->CalcNextDir();
				addActiveMoveJob ( MJob );
				cLog::write("(Client) Added received movejob", cLog::eLOG_TYPE_NET_DEBUG);
			}
		}
		break;
	case GAME_EV_NEXT_MOVE:
		{
			int iID = message->popInt16();
			int iDestOff = message->popInt16();
			int iType = message->popInt16();

			cLog::write("(Client) Received information for next move: ID: " + iToStr ( iID ) + ", SrcX: " + iToStr( iDestOff%Map->size ) + ", SrcY: " + iToStr( iDestOff/Map->size ) + ", Type: " + iToStr ( iType ), cLog::eLOG_TYPE_NET_DEBUG);

			cVehicle *Vehicle = getVehicleFromID ( iID );
			if ( Vehicle && Vehicle->mjob )
			{
				// set vehicle to server position
				int iDestX = iDestOff%Map->size;
				int iDestY = iDestOff/Map->size;
				if ( Vehicle->PosX != iDestX || Vehicle->PosY != iDestY )
				{
					// the client is faster then the server and has already
					// reached the last field or the next will be the last,
					// then stop the vehicle
					if ( Vehicle->mjob->waypoints == NULL || Vehicle->mjob->waypoints->next == NULL )
					{
						cLog::write ( "(Client) Client has already reached the last field", cLog::eLOG_TYPE_NET_DEBUG );
						Vehicle->mjob->finished = true;
						Vehicle->OffX = Vehicle->OffY = 0;
					}
					else
					{
						// server is one field faster then client
						if ( iDestX == Vehicle->mjob->waypoints->next->X && iDestY == Vehicle->mjob->waypoints->next->Y )
						{
							cLog::write ( "(Client) Server is one field faster then client", cLog::eLOG_TYPE_NET_DEBUG );
							doEndMoveVehicle( Vehicle );
						}
						else
						{
							// check whether the destination field is one of the next in the waypointlist
							// if not it must have been one that has been deleted already
							bool bServerIsFaster = false;
							sWaypoint *Waypoint = Vehicle->mjob->waypoints->next->next;
							while ( Waypoint )
							{
								if ( Waypoint->X == iDestX && Waypoint->Y == iDestY )
								{
									bServerIsFaster = true;
									break;
								}
								Waypoint = Waypoint->next;
							}
							// the server is more then one field faster
							if ( bServerIsFaster )
							{
								cLog::write ( "(Client) Server is more then one field faster", cLog::eLOG_TYPE_NET_DEBUG );
								if ( Vehicle->data.can_drive == DRIVE_AIR )
								{
									Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
									Map->GO[iDestX+iDestY*Map->size].plane = Vehicle;
								}
								else
								{
									Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
									Map->GO[iDestX+iDestY*Map->size].vehicle = Vehicle;
								}
								Vehicle->PosX = iDestOff%Map->size;
								Vehicle->PosY = iDestOff/Map->size;
								Vehicle->OffX = Vehicle->OffY = 0;

								Waypoint = Vehicle->mjob->waypoints;
								while ( Waypoint )
								{
									if ( Waypoint->next->X != iDestX && Waypoint->next->Y != iDestY )
									{
										Vehicle->mjob->waypoints = Waypoint->next;
										free ( Waypoint );
										Waypoint = Vehicle->mjob->waypoints;
									}
									else
									{
										Vehicle->mjob->waypoints = Waypoint;
										break;
									}
								}
							}
							// the client is faster
							else
							{
								cLog::write ( "(Client) Client is faster (one or more fields)", cLog::eLOG_TYPE_NET_DEBUG );
								// just stop the vehicle and wait for the next commando of the server
								Vehicle->mjob->EndForNow = true;
								break;
							}
						}
					}
				}
				if ( iType == MJOB_OK )	// move is OK
				{
					Vehicle->moving = true;
					if ( !Vehicle->MoveJobActive ) Vehicle->StartMoveSound();
					Vehicle->MoveJobActive = true;
					cLog::write("(Client) The movejob is ok: DestX: " + iToStr ( Vehicle->mjob->waypoints->next->X ) + ", DestY: " + iToStr ( Vehicle->mjob->waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
				}
				else if ( iType == MJOB_STOP )	// move has to be stoped
				{
					if ( Vehicle->mjob->waypoints->X == Vehicle->mjob->DestX && Vehicle->mjob->waypoints->Y == Vehicle->mjob->DestY )
					{
						cLog::write("(Client) The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
						Vehicle->mjob->finished = true;
					}
					else
					{
						cLog::write("(Client) The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
						Vehicle->mjob->Suspended = true;
						Vehicle->mjob->EndForNow = true;
					}
				}
				else if ( iType == MJOB_BLOCKED ) // next field is blocked
				{
					cLog::write("(Client) Movejob is finished becouse the next field is blocked: DestX: " + iToStr ( Vehicle->mjob->waypoints->next->X ) + ", DestY: " + iToStr ( Vehicle->mjob->waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
					Vehicle->mjob->finished = true;
					// TODO: Calc a new path and start the new job
				}
			}
			else
			{
				if ( Vehicle == NULL ) cLog::write("(Client) Can't find vehicle", cLog::eLOG_TYPE_NET_WARNING);
				else cLog::write("(Client) Vehicle has no movejob", cLog::eLOG_TYPE_NET_WARNING);
			}
		}
		break;
	case GAME_EV_ATTACKJOB_LOCK_TARGET:
		{
			cClientAttackJob::clientLockTarget( message );
		}
		break;
	case GAME_EV_ATTACKJOB_FIRE:
		{
			cClientAttackJob* job = new cClientAttackJob( message );
			Client->attackJobs->Add( job );
		}
		break;
	case GAME_EV_RESOURCES:
		{
			int iCount = message->popInt16();
			for ( int i = 0; i < iCount; i++ )
			{
				int iOff = message->popInt32();
				ActivePlayer->ResourceMap[iOff] = 1;

				Map->Resources[iOff].typ = message->popInt16();
				Map->Resources[iOff].value = message->popInt16();
			}
		}
		break;
	default:
		cLog::write("Client: Can not handle message type " + iToStr(message->iType), cLog::eLOG_TYPE_NET_ERROR);
		break;
	}
	return 0;
}

void cClient::addUnit( int iPosX, int iPosY, cVehicle *AddedVehicle, bool bInit )
{
	// place the vehicle:
	if ( AddedVehicle->data.can_drive != DRIVE_AIR )
	{
		int iOff = iPosX+Map->size*iPosY;
		Map->GO[iOff].vehicle = AddedVehicle;
	}
	else
	{
		int iOff = iPosX+Map->size*iPosY;
		Map->GO[iOff].plane = AddedVehicle;
	}
	// startup:
	if ( !bInit ) AddedVehicle->StartUp = 10;
}

void cClient::addUnit( int iPosX, int iPosY, cBuilding *AddedBuilding, bool bInit )
{
	// place the building:
	int iOff = iPosX + Map->size*iPosY;
	if ( AddedBuilding->data.is_base )
	{
		if(Map->GO[iOff].base)
		{
			Map->GO[iOff].subbase = Map->GO[iOff].base;
			Map->GO[iOff].base = AddedBuilding;
		}
		else
		{
			Map->GO[iOff].base = AddedBuilding;
		}
	}
	else
	{
		if ( AddedBuilding->data.is_big )
		{
			Map->GO[iOff].top;
			Map->GO[iOff+1].top;
			Map->GO[iOff+Map->size].top;
			Map->GO[iOff+Map->size+1].top;
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				Map->GO[iOff].base = NULL;
			}
			iOff++;
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				Map->GO[iOff].base=NULL;
			}
			iOff+=Map->size;
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				Map->GO[iOff].base=NULL;
			}
			iOff--;
			Map->GO[iOff].top=AddedBuilding;
			if ( Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				Map->GO[iOff].base=NULL;
			}
		}
		else
		{
			Map->GO[iOff].top=AddedBuilding;
			if ( !AddedBuilding->data.is_connector&&Map->GO[iOff].base&&(Map->GO[iOff].base->data.is_road || Map->GO[iOff].base->data.is_expl_mine) )
			{
				Map->GO[iOff].base=NULL;
			}
		}
	}
	if ( !bInit ) AddedBuilding->StartUp=10;
	// integrate the building to the base:
	AddedBuilding->owner->base->AddBuilding ( AddedBuilding );
}

cPlayer *cClient::getPlayerFromNumber ( int iNum )
{
	cPlayer *Player = NULL;
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		if ( PlayerList->Items[i]->Nr == iNum )
		{
			Player = PlayerList->Items[i];
			break;
		}
	}
	return Player;
}

void cClient::deleteUnit( cBuilding *Building )
{
	if( Building )
	{
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
		if ( Map->GO[Building->PosX+Building->PosY*Map->size].top == Building ) Map->GO[Building->PosX+Building->PosY*Map->size].top = NULL;
		else if ( Map->GO[Building->PosX+Building->PosY*Map->size].base == Building ) Map->GO[Building->PosX+Building->PosY*Map->size].base = NULL;
		else  Map->GO[Building->PosX+Building->PosY*Map->size].subbase = NULL;
		if ( SelectedBuilding == Building ) SelectedBuilding = NULL;
		delete Building;
	}
}

void cClient::deleteUnit( cVehicle *Vehicle )
{
	if( Vehicle )
	{
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
		if ( Vehicle->data.can_drive == DRIVE_AIR ) Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
		else  Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
		if ( SelectedVehicle == Vehicle ) SelectedVehicle = NULL;
		delete Vehicle;
	}
}

void cClient::handleEnd()
{
	if ( ActiveMJobs.iCount > 0 )
	{
		addMessage( lngPack.i18n( "Text~Comp~Turn_Wait") );
	}
	else
	{
		sendWantToEndTurn();
	}
}

void cClient::makeHotSeatEnd( int iNextPlayerNum )
{
	// clear the messages
	sMessage *Message;
	while ( messages.iCount )
	{
		Message = messages.Items[0];
		free ( Message->msg );
		free ( Message );
		messages.Delete ( 0 );
	}

	// save information and set next player
	int iZoom, iX, iY;
	ActivePlayer->HotHud = *Hud;
	iZoom = Hud->LastZoom;
	ActivePlayer = getPlayerFromNumber( iNextPlayerNum );	// TODO: maybe here must be done more than just set the next player!
	*Hud = ActivePlayer->HotHud;
	iX = Hud->OffX;
	iY = Hud->OffY;
	if ( Hud->LastZoom != iZoom )
	{
		Hud->LastZoom = -1;
		Hud->ScaleSurfaces();
	}
	Hud->DoAllHud();
	Hud->EndeButton ( false );
	Hud->OffX = iX;
	Hud->OffY = iY;

	// reset the screen
	if ( SelectedBuilding ) { SelectedBuilding->Deselct(); SelectedBuilding = NULL; }
	if ( SelectedVehicle ) { SelectedVehicle->Deselct(); SelectedVehicle = NULL; }
	SDL_Surface *sf;
	SDL_Rect scr;
	sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,SettingsData.iScreenW,SettingsData.iScreenH,32,0,0,0,0 );
	scr.x=15;
	scr.y=356;
	scr.w=scr.h=112;
	SDL_BlitSurface ( sf,NULL,buffer,NULL );
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	SDL_BlitSurface ( sf,&scr,buffer,&scr );

	ShowOK ( ActivePlayer->name + lngPack.i18n( "Text~Multiplayer~Player_Turn"), true );
}

void cClient::waitForOtherPlayer( int iPlayerNum )
{
	if ( !bWaitForOthers ) return;
	int iLastX = -1, iLastY = -1;
	Uint8 *keystate;

	font->showTextCentered( 320, 235, lngPack.i18n ( "Text~Multiplayer~Wait_Until", getPlayerFromNumber( iPlayerNum )->name ), LATIN_BIG );
	while ( bWaitForOthers )
	{
		EventHandler->HandleEvents();

		keystate = SDL_GetKeyState( NULL );
		mouse->GetPos();
		if ( mouse->x != iLastX || mouse->y != iLastY )
		{
			mouse->draw ( true, screen );
		}
		iLastX = mouse->x;
		iLastY = mouse->y;

		if ( keystate[KeysList.KeyExit] && ShowYesNo ( lngPack.i18n( "Text~Comp~End_Game") ) )
		{
			bExit = true;
			bWaitForOthers = false;
		}

		//handleTimer();
		Client->doGameActions();
	}
}

void cClient::handleTurnTime()
{
	if ( iTurnTime > 0 )
	{
		int iRestTime = iTurnTime - Round( ( SDL_GetTicks() - iStartTurnTime )/1000 );
		if ( iRestTime < 0 ) iRestTime = 0;
		Hud->showTurnTime ( iRestTime );
	}
}

void cClient::addActiveMoveJob ( cMJobs *MJob )
{
	ActiveMJobs.Add ( MJob );
	MJob->Suspended = false;
}

void cClient::handleMoveJobs ()
{
	for ( int i = 0; i < ActiveMJobs.iCount; i++ )
	{
		cMJobs *MJob;
		cVehicle *Vehicle;

		MJob = ActiveMJobs.Items[i];
		Vehicle = MJob->vehicle;

		if ( MJob->finished || MJob->EndForNow )
		{
			// Stop the soundstream
			if ( Vehicle && Vehicle == SelectedVehicle && Vehicle->MoveJobActive )
			{
				StopFXLoop ( iObjectStream );
				if ( Map->IsWater ( Vehicle->PosX+Vehicle->PosY*Map->size ) && Vehicle->data.can_drive != DRIVE_AIR ) PlayFX ( Vehicle->typ->StopWater );
				else PlayFX ( Vehicle->typ->Stop );
				iObjectStream = Vehicle->PlayStram();
			}

			if ( MJob->finished )
			{
				if ( Vehicle && Vehicle->mjob == MJob )
				{
					cLog::write("(Client) Movejob is finished an will be deleted now", cLog::eLOG_TYPE_NET_DEBUG);
					Vehicle->mjob = NULL;
					Vehicle->moving = false;
					Vehicle->rotating = false;
					Vehicle->MoveJobActive = false;
				}
				else cLog::write("(Client) Delete movejob with nonactive vehicle (released one)", cLog::eLOG_TYPE_NET_DEBUG);
				ActiveMJobs.Delete ( i );
				delete MJob;
				continue;
			}
			if ( MJob->EndForNow )
			{
				cLog::write("(Client) Movejob has end for now and will be stoped (delete from active ones)", cLog::eLOG_TYPE_NET_DEBUG);
				if ( Vehicle )
				{
					Vehicle->MoveJobActive = false;
					Vehicle->rotating = false;
					// save speed
					if ( Vehicle->data.speed < MJob->waypoints->next->Costs )
					{
						MJob->SavedSpeed += Vehicle->data.speed;
						Vehicle->data.speed = 0;
						Vehicle->ShowDetails();
					}
				}
				ActiveMJobs.Delete ( i );
				continue;
			}
		}

		if ( MJob->vehicle == NULL ) continue;

		// rotate vehicle
		if ( MJob->next_dir != Vehicle->dir && Vehicle->data.speed )
		{
			Vehicle->rotating = true;
			if ( iTimer1 )
			{
				Vehicle->RotateTo ( MJob->next_dir );
			}
			continue;
		}
		else
		{
			Vehicle->rotating = false;
		}

		if ( Vehicle->MoveJobActive )
		{
			moveVehicle( MJob->vehicle );
			bFlagDrawMap = true;
		}
	}
}

void cClient::moveVehicle( cVehicle *Vehicle )
{
	if ( Vehicle == NULL || Vehicle->mjob == NULL ) return;
	cMJobs *MJob = Vehicle->mjob;

	int iSpeed;
	if ( !Vehicle ) return;
	if ( Vehicle->data.is_human )
	{
		Vehicle->WalkFrame++;
		if ( Vehicle->WalkFrame >= 13 ) Vehicle->WalkFrame = 0;
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.can_drive == DRIVE_AIR) && !(Vehicle->data.can_drive == DRIVE_SEA) )
	{
		iSpeed = MOVE_SPEED;
		if ( MJob->waypoints && MJob->waypoints->next && Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].base&& ( Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].base->data.is_road || Map->GO[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size].base->data.is_bridge ) ) iSpeed*=2;
	}
	else if ( Vehicle->data.can_drive == DRIVE_AIR ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if ( SettingsData.bMakeTracks && Vehicle->data.make_tracks && !Map->IsWater ( Vehicle->PosX+Vehicle->PosY*Map->size,false ) &&!
	        ( MJob->waypoints && MJob->waypoints->next && Map->terrain[Map->Kacheln[MJob->waypoints->next->X+MJob->waypoints->next->Y*Map->size]].water ) &&
	        ( Vehicle->owner == Client->ActivePlayer || ActivePlayer->ScanMap[Vehicle->PosX+Vehicle->PosY*Map->size] ) )
	{
		if ( !Vehicle->OffX && !Vehicle->OffY )
		{
			switch ( Vehicle->dir )
			{
				case 0:
					addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64-10,0 );
					break;
				case 4:
					addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64+10,0 );
					break;
				case 2:
					addFX ( fxTracks,Vehicle->PosX*64+10,Vehicle->PosY*64,2 );
					break;
				case 6:
					addFX ( fxTracks,Vehicle->PosX*64-10,Vehicle->PosY*64,2 );
					break;
				case 1:
					addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,1 );
					break;
				case 5:
					addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,1 );
					break;
				case 3:
					addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,3 );
					break;
				case 7:
					addFX ( fxTracks,Vehicle->PosX*64,Vehicle->PosY*64,3 );
					break;
			}
		}
		else if ( abs ( Vehicle->OffX ) == iSpeed*2 || abs ( Vehicle->OffY ) == iSpeed*2 )
		{
			switch ( Vehicle->dir )
			{
				case 1:
					addFX ( fxTracks,Vehicle->PosX*64+26,Vehicle->PosY*64-26,1 );
					break;
				case 5:
					addFX ( fxTracks,Vehicle->PosX*64-26,Vehicle->PosY*64+26,1 );
					break;
				case 3:
					addFX ( fxTracks,Vehicle->PosX*64+26,Vehicle->PosY*64+26,3 );
					break;
				case 7:
					addFX ( fxTracks,Vehicle->PosX*64-26,Vehicle->PosY*64-26,3 );
					break;
			}
		}
	}

	switch ( MJob->next_dir )
	{
		case 0:
			Vehicle->OffY -= iSpeed;
			break;
		case 1:
			Vehicle->OffY -= iSpeed;
			Vehicle->OffX += iSpeed;
			break;
		case 2:
			Vehicle->OffX += iSpeed;
			break;
		case 3:
			Vehicle->OffX += iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 4:
			Vehicle->OffY += iSpeed;
			break;
		case 5:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY += iSpeed;
			break;
		case 6:
			Vehicle->OffX -= iSpeed;
			break;
		case 7:
			Vehicle->OffX -= iSpeed;
			Vehicle->OffY -= iSpeed;
			break;
	}

	// check whether the point has been reached:
	if ( Vehicle->OffX >= 64 || Vehicle->OffY >= 64 || Vehicle->OffX <= -64 || Vehicle->OffY <= -64 )
	{
		cLog::write("(Client) Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Vehicle->mjob->waypoints->next->X ) + ", Y: " + iToStr ( Vehicle->mjob->waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
		doEndMoveVehicle ( Vehicle );
	}
}

void cClient::doEndMoveVehicle ( cVehicle *Vehicle )
{
	if ( Vehicle == NULL || Vehicle->mjob == NULL ) return;
	cMJobs *MJob = Vehicle->mjob;

	if ( MJob->waypoints->next == NULL )
	{
		// this is just to avoid errors, this should normaly never happen.
		MJob->finished = true;
		return;
	}

	Vehicle->data.speed += MJob->SavedSpeed;
	MJob->SavedSpeed = 0;
	Vehicle->DecSpeed ( MJob->waypoints->next->Costs );
	Vehicle->moving = false;
	Vehicle->WalkFrame = 0;

	sWaypoint *wp;
	wp = MJob->waypoints->next;
	free ( MJob->waypoints );
	MJob->waypoints = wp;

	Vehicle->moving = false;

	if ( !MJob->plane )
	{
		if ( Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].vehicle != NULL || Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].top != NULL )
		{
			cLog::write ( "(Client) Next waypoint for vehicle with ID \"" + iToStr( Vehicle->iID )  + "\" is blocked by an other unit", cLog::eLOG_TYPE_NET_ERROR );
			MJob->finished = true;
		}
		else
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].vehicle = NULL;
			Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].vehicle = Vehicle;
			Vehicle->PosX = MJob->waypoints->X;
			Vehicle->PosY = MJob->waypoints->Y;
		}
	}
	else
	{
		Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
		if ( Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].plane != NULL )
		{
			cLog::write ( "(Client) Next waypoint for plane with ID \"" + iToStr( Vehicle->iID )  + "\" is blocked by an other plane", cLog::eLOG_TYPE_NET_ERROR );
			MJob->finished = true;
		}
		else
		{
			Map->GO[Vehicle->PosX+Vehicle->PosY*Map->size].plane = NULL;
			Map->GO[MJob->waypoints->X+MJob->waypoints->Y*Map->size].plane = Vehicle;
			Vehicle->PosX = MJob->waypoints->X;
			Vehicle->PosY = MJob->waypoints->Y;
		}
	}
	Vehicle->OffX = 0;
	Vehicle->OffY = 0;

	Vehicle->owner->DoScan();
	bFlagDrawMMap = true;

	mouseMoveCallback ( true );
	MJob->CalcNextDir();
}

cVehicle *cClient::getVehicleFromID ( int iID )
{
	cVehicle *Vehicle;
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		Vehicle = PlayerList->Items[i]->VehicleList;
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
	for ( int i = 0; i < PlayerList->iCount; i++ )
	{
		Building = PlayerList->Items[i]->BuildingList;
		while ( Building )
		{
			if ( Building->iID == iID ) return Building;
			Building = Building->next;
		}
	}
	return NULL;
}

void cClient::trace ()
{
	int iY, iX;
	sGameObjects *GO;

	mouse->GetKachel ( &iX, &iY );
	if ( iX < 0 || iY < 0 ) return;
	if ( bDebugTraceServer ) GO = Server->Map->GO + ( Server->Map->size*iY+iX );
	else GO = Map->GO + ( Map->size*iY+iX );

	if ( GO->reserviert ) font->showText(180+5,18+5, "reserviert", LATIN_SMALL_WHITE);
	if ( GO->air_reserviert ) font->showText(180+5+100,18+5, "air-reserviert", LATIN_SMALL_WHITE);
	iY = 18+5+8;
	iX = 180+5;

	if ( GO->vehicle ) { traceVehicle ( GO->vehicle, &iY, iX ); iY += 20; }
	if ( GO->plane ) { traceVehicle ( GO->plane, &iY, iX ); iY += 20; }
	if ( GO->top ) { traceBuilding ( GO->top, &iY, iX ); iY += 20; }
	if ( GO->base ) traceBuilding ( GO->base, &iY, iX );
}

void cClient::traceVehicle ( cVehicle *Vehicle, int *iY, int iX )
{
	string sTmp;

	sTmp = "name: \"" + Vehicle->name + "\" id: \"" + iToStr ( Vehicle->iID ) + "\" owner: \"" + Vehicle->owner->name + "\" posX: +" + iToStr ( Vehicle->PosX ) + " posY: " + iToStr ( Vehicle->PosY ) + " offX: " + iToStr ( Vehicle->OffX ) + " offY: " + iToStr ( Vehicle->OffY );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "dir: " + iToStr ( Vehicle->dir ) + " selected: " + iToStr ( Vehicle->selected ) + " moving: +" + iToStr ( Vehicle->moving ) + " rotating: " + iToStr ( Vehicle->rotating ) + " mjob: "  + iToStr ((long int) Vehicle->mjob ) + " speed: " + iToStr ( Vehicle->data.speed ) + " mj_active: " + iToStr ( Vehicle->MoveJobActive ) + " menu_active: " + iToStr ( Vehicle->MenuActive );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "attack_mode: " + iToStr ( Vehicle->AttackMode ) + " attacking: " + iToStr ( Vehicle->Attacking ) + " wachpost: +" + iToStr ( Vehicle->Wachposten ) + " transfer: " + iToStr ( Vehicle->Transfer ) + " ditherx: " + iToStr (Vehicle->ditherX ) + " dithery: " + iToStr ( Vehicle->ditherY );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "is_building: " + iToStr ( Vehicle->IsBuilding ) + " building_typ: " + iToStr ( Vehicle->BuildingTyp ) + " build_costs: +" + iToStr ( Vehicle->BuildCosts ) + " build_rounds: " + iToStr ( Vehicle->BuildRounds ) + " build_round_start: " + iToStr (Vehicle->BuildRoundsStart );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "place_band: " + iToStr ( Vehicle->PlaceBand ) + " bandx: " + iToStr ( Vehicle->BandX ) + " bandy: +" + iToStr ( Vehicle->BandY ) + " build_big_saved_pos: " + iToStr ( Vehicle->BuildBigSavedPos ) + " build_path: " + iToStr (Vehicle->BuildPath );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "build_override: " + iToStr ( Vehicle->BuildOverride ) + " is_clearing: " + iToStr ( Vehicle->IsClearing ) + " clearing_rounds: +" + iToStr ( Vehicle->ClearingRounds ) + " clear_big: " + iToStr ( Vehicle->ClearBig ) + " loaded: " + iToStr (Vehicle->Loaded );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "commando_rank: " + iToStr ( Vehicle->CommandoRank ) + " steal_active: " + iToStr ( Vehicle->StealActive ) + " disable_active: +" + iToStr ( Vehicle->DisableActive ) + " disabled: " + iToStr ( Vehicle->Disabled ) + " detection_override: " + iToStr (Vehicle->detection_override );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "is_locked: " + iToStr ( Vehicle->IsLocked ) + " detected: " + iToStr ( Vehicle->detected ) + " clear_mines: +" + iToStr ( Vehicle->ClearMines ) + " lay_mines: " + iToStr ( Vehicle->LayMines ) + " repair_active: " + iToStr (Vehicle->RepairActive ) + " muni_active: " + iToStr (Vehicle->MuniActive );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "load_active: " + iToStr ( Vehicle->LoadActive ) + " activating_vehicle: " + iToStr ( Vehicle->ActivatingVehicle ) + " vehicle_to_activate: +" + iToStr ( Vehicle->VehicleToActivate ) + " stored_vehicles_count: " + iToStr ( ( Vehicle->StoredVehicles?Vehicle->StoredVehicles->iCount:0 ) );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	if ( Vehicle->StoredVehicles && Vehicle->StoredVehicles->iCount )
	{
		cVehicle *StoredVehicle;
		for ( int i = 0; i < Vehicle->StoredVehicles->iCount; i++ )
		{
			StoredVehicle = Vehicle->StoredVehicles->Items[i];
			font->showText(iX, *iY, " store " + iToStr(i)+": \""+StoredVehicle->name+"\"", LATIN_SMALL_WHITE);
			*iY += 8;
		}
	}

	if ( bDebugTraceServer )
	{
		sTmp = "seen by players: owner";
		for ( int i = 0; i < Vehicle->SeenByPlayerList.iCount; i++ )
		{
			sTmp += ", \"" + getPlayerFromNumber ( *Vehicle->SeenByPlayerList.Items[i] )->name + "\"";
		}
		font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
		*iY+=8;
	}
}

void cClient::traceBuilding ( cBuilding *Building, int *iY, int iX )
{
	string sTmp;

	sTmp = "name: \"" + Building->name + "\" id: \"" + iToStr ( Building->iID ) + "\" owner: \"" + ( Building->owner?Building->owner->name:"<null>" ) + "\" posX: +" + iToStr ( Building->PosX ) + " posY: " + iToStr ( Building->PosY ) + " selected: " + iToStr ( Building->selected );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "dir: " + iToStr ( Building->dir ) + " menu_active: " + iToStr ( Building->MenuActive ) + " wachpost: +" + iToStr ( Building->Wachposten ) + " attacking_mode: +" + iToStr ( Building->AttackMode ) + " base: " + iToStr ( (long int)Building->base ) + " sub_base: " + iToStr ((long int)Building->SubBase );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "attacking: " + iToStr ( Building->Attacking ) + " UnitsData.dirt_typ: " + iToStr ( Building->DirtTyp ) + " UnitsData.dirt_value: +" + iToStr ( Building->DirtValue ) + " big_dirt: " + iToStr ( Building->BigDirt ) + " is_working: " + iToStr (Building->IsWorking ) + " transfer: " + iToStr (Building->Transfer );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "metal_prod: " + iToStr ( Building->MetalProd ) + " oil_prod: " + iToStr ( Building->OilProd ) + " gold_prod: +" + iToStr ( Building->GoldProd ) + " max_metal_p: " + iToStr ( Building->MaxMetalProd ) + " max_oil_p: " + iToStr (Building->MaxOilProd ) + " max_gold_p: " + iToStr (Building->MaxGoldProd );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "is_locked: " + iToStr ( Building->IsLocked ) + " disabled: " + iToStr ( Building->Disabled ) + " detected: +" + iToStr ( Building->detected ) + " activating_vehicle: " + iToStr ( Building->ActivatingVehicle ) + " vehicle_to_activate: " + iToStr (Building->VehicleToActivate );
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	sTmp = "load_active: " + iToStr ( Building->LoadActive ) + " stored_vehicles_count: " + iToStr (( Building->StoredVehicles?Building->StoredVehicles->iCount:0 ));
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	if ( Building->StoredVehicles&&Building->StoredVehicles->iCount )
	{
		cVehicle *StoredVehicle;
		for ( int i = 0; i < Building->StoredVehicles->iCount; i++ )
		{
			StoredVehicle = Building->StoredVehicles->Items[i];
			font->showText(iX, *iY, " store " + iToStr(i)+": \""+StoredVehicle->name+"\"", LATIN_SMALL_WHITE);
			*iY+=8;
		}
	}

	sTmp = "build_speed: " + iToStr ( Building->BuildSpeed ) + " repeat_build: " + iToStr ( Building->RepeatBuild ) + " build_list_count: +" + iToStr (( Building->BuildList?Building->BuildList->iCount:0 ));
	font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
	*iY+=8;

	if ( Building->BuildList&&Building->BuildList->iCount )
	{
		sBuildList *BuildingList;
		for ( int i = 0; i < Building->BuildList->iCount; i++ )
		{
			BuildingList = Building->BuildList->Items[i];
			font->showText(iX, *iY, "  build "+iToStr(i)+": "+iToStr(BuildingList->typ->nr)+" \""+UnitsData.vehicle[BuildingList->typ->nr].data.name+"\"", LATIN_SMALL_WHITE);
			*iY+=8;
		}
	}

	if ( bDebugTraceServer )
	{
		sTmp = "seen by players: owner";
		for ( int i = 0; i < Building->SeenByPlayerList.iCount; i++ )
		{
			sTmp += ", \"" + getPlayerFromNumber ( *Building->SeenByPlayerList.Items[i] )->name + "\"";
		}
		font->showText(iX,*iY, sTmp, LATIN_SMALL_WHITE);
		*iY+=8;
	}
}

void cClient::releaseMoveJob ( cMJobs *MJob )
{
	cLog::write ( "(Client) Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for ( int i = 0; i < ActiveMJobs.iCount; i++ )
	{
		if ( MJob == ActiveMJobs.Items[i] ) return;
	}
	addActiveMoveJob ( MJob );
	cLog::write ( "(Client) Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

void cClient::doGameActions()
{
	handleTimer();
	if ( !iTimer0 ) return;

	//run attackJobs
	cClientAttackJob::handleAttackJobs();
	//run moveJobs - this has to be called before handling the auto movejobs
	handleMoveJobs();
	//run surveyor ai
	cAutoMJob::handleAutoMoveJobs();
}
