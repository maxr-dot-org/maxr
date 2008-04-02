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
#include "client.h"
#include "server.h"
#include "events.h"
#include "serverevents.h"
#include "pcx.h"
#include "mouse.h"
#include "keyinp.h"
#include "keys.h"
#include "fonts.h"

Uint32 TimerCallback(Uint32 interval, void *arg)
{
	((cClient *)arg)->Timer();
	return interval;
}

void cClient::init( cMap *Map, cList<cPlayer*> *PlayerList )
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
	messages=new cList<sMessage*>;
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
	bDebugBase = false;
	bDebugWache = false;
	bDebugFX = false;
	bDebugTrace = false;

	SDL_Rect rSrc = {0,0,170,224};
	SDL_Surface *SfTmp = LoadPCX( (char*) (SettingsData.sGfxPath + PATH_DELIMITER + "hud_left.pcx").c_str() );
	SDL_BlitSurface( SfTmp, &rSrc, GraphicsData.gfx_hud, NULL );
	SDL_FreeSurface( SfTmp );

	setWind ( random ( 360,0 ) );
}

void cClient::kill()
{
	Hud->Zoom = 64;
	Hud->ScaleSurfaces();
	SDL_RemoveTimer ( TimerID );
	StopFXLoop ( iObjectStream );
	while ( messages->iCount )
	{
		free ( messages->Items[0]->msg );
		free ( messages->Items[0] );
		messages->Delete ( 0 );
	}
	delete messages;
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

	while( DirtList )
	{
		cBuilding *ptr;
		ptr = DirtList->next;
		delete DirtList;
		DirtList=ptr;
	}
}

void cClient::sendEvent ( SDL_Event *event, int iLenght )
{
	// push an event to the lokal server in singleplayer or if this machine is the host
	if ( !network || network->isHost() ) Server->pushEvent ( event );
	// else send it over the net
	else if ( network ) 
	{
		network->sendEvent ( event, iLenght );
		delete event;
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

	while ( 1 )
	{
		// check defeat
		if ( bDefeated ) break;
		// check user
		if ( checkUser() == -1 )
		{
			makePanel ( false );
			break;
		}
		// end truth save/load menu
		if ( bExit )
		{
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
			if ( Hud->Reichweite&&SelectedBuilding->data.max_shield )
			{
				if ( SelectedBuilding->data.is_big )
				{
					drawCircle ( spx+Hud->Zoom,
					             spy+Hud->Zoom,
					             SelectedBuilding->data.range*Hud->Zoom+3,RANGE_SHIELD_COLOR,buffer );
				}
				else
				{
					drawCircle ( spx+Hud->Zoom/2,
					             spy+Hud->Zoom/2,
					             SelectedBuilding->data.range*Hud->Zoom+3,RANGE_SHIELD_COLOR,buffer );
				}
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
		if ( bDebugBase && bFlagDrawMap )
		{
			font->showText(550,iDebugOff, "subbases: " + iToStr(ActivePlayer->base->SubBases->iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight ( LATIN_SMALL_WHITE );
		}
	
		if ( bDebugWache && bFlagDrawMap )
		{
			font->showText(550,iDebugOff, "w-air: " + iToStr(ActivePlayer->WachpostenAir->iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
			font->showText(550,iDebugOff, "w-ground: " + iToStr(ActivePlayer->WachpostenGround->iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
		}
		
		if ( bDebugFX && bFlagDrawMap )
		{
			font->showText(550, iDebugOff, "fx-count: " + iToStr(FXList->iCount + FXListBottom->iCount), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
			font->showText(550, iDebugOff, "wind-dir: " + iToStr(( int ) ( fWindDir*57.29577 )), LATIN_SMALL_WHITE);
			iDebugOff += font->getFontHeight(LATIN_SMALL_WHITE);
		}
		if ( bDebugTrace && bFlagDrawMap )
		{
			//TODO: implement
			//Trace();
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
				makePanel ( true );
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
		// handle the timers:
		handleTimer();
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
				// TODO: no engine: Chat input
				//engine->SendChatMessage((ActivePlayer->name+": "+InputStr).c_str());
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
			Hud->MakeMeMyEnd();
			bLastReturn=true;
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
			SelectedVehicle->ShowTransfer ( Map->GO+mouse->GetKachelOff() );
		}
		else if ( SelectedBuilding&&mouse->cur==GraphicsData.gfx_Ctransf )
		{
			SelectedBuilding->ShowTransfer ( Map->GO+mouse->GetKachelOff() );
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
			// TODO: add movejob
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
				// Prüfen, ob geschossen werden soll:
				if ( mouse->cur==GraphicsData.gfx_Cattack&&SelectedVehicle&&!SelectedVehicle->Attacking&&!SelectedVehicle->MoveJobActive )
				{
					// TODO: add attack job
				}
				else if ( mouse->cur == GraphicsData.gfx_Cattack && SelectedBuilding && !SelectedBuilding->Attacking )
				{
					// TODO: add attack job
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
					if ( bDebugBase )
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
	if ( SettingsData.bAlphaEffects )
	{
		SDL_SetAlpha ( ActivePlayer->ShieldColor,SDL_SRCALPHA,150 );
	}
	else
	{
		SDL_SetAlpha ( ActivePlayer->ShieldColor,SDL_SRCALPHA,255 );
	}
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
				if ( Hud->Status&&ActivePlayer->ShieldMap&&ActivePlayer->ShieldMap[iPos] )
				{
					tmp=dest;
					SDL_BlitSurface ( ActivePlayer->ShieldColor,&scr,buffer,&tmp );
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
	if ( ( FLC == NULL && video == NULL ) || ( SelectedVehicle == NULL && SelectedBuilding == NULL ) ) return;
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

void cClient::drawSpecialCircle( int iX, int iY, int iRadius, char *map )
{
	float w=0.017453*45,step;
	int rx,ry,x1,x2;
	if ( iRadius ) iRadius--;
	if ( !iRadius ) return;
	iRadius *= 10;
	step=0.017453*90-acos ( 1.0/iRadius );
	step/=2;
	for ( float i=0;i<=w;i+=step )
	{
		rx= ( int ) ( cos ( i ) *iRadius );
		ry= ( int ) ( sin ( i ) *iRadius);
		rx/=10;ry/=10;

		x1=rx+iX;x2=-rx+iX;
		for ( int k=x2;k<=x1+1;k++ )
		{
			if ( k<0 ) continue;
			if ( k>=Map->size ) break;
			if ( iY+ry>=0&&iY+ry<Map->size )
				map[k+ ( iY+ry ) *Map->size]|=1;
			if ( iY-ry>=0&&iY-ry<Map->size )
				map[k+ ( iY-ry ) *Map->size]|=1;

			if ( iY+ry+1>=0&&iY+ry+1<Map->size )
				map[k+ ( iY+ry+1 ) *Map->size]|=1;
			if ( iY-ry+1>=0&&iY-ry+1<Map->size )
				map[k+ ( iY-ry+1 ) *Map->size]|=1;
		}

		x1=ry+iX;x2=-ry+iX;
		for ( int k=x2;k<=x1+1;k++ )
		{
			if ( k<0 ) continue;
			if ( k>=Map->size ) break;
			if ( iY+rx>=0&&iY+rx<Map->size )
				map[k+ ( iY+rx ) *Map->size]|=1;
			if ( iY-rx>=0&&iY-rx<Map->size )
				map[k+ ( iY-rx ) *Map->size]|=1;

			if ( iY+rx+1>=0&&iY+rx+1<Map->size )
				map[k+ ( iY+rx+1 ) *Map->size]|=1;
			if ( iY-rx+1>=0&&iY-rx+1<Map->size )
				map[k+ ( iY-rx+1 ) *Map->size]|=1;
		}
	}
}

void cClient::drawSpecialCircleBig( int iX, int iY, int iRadius, char *map )
{
	float w=0.017453*45,step;
	int rx,ry,x1,x2;
	if ( iRadius ) iRadius--;
	if ( !iRadius ) return;
	iRadius *= 10;
	step=0.017453*90-acos ( 1.0/iRadius );
	step/=2;
	for ( float i=0;i<=w;i+=step )
	{
		rx= ( int ) ( cos ( i ) *iRadius );
		ry= ( int ) ( sin ( i ) *iRadius);
		rx/=10;ry/=10;

		x1=rx+iX;x2=-rx+iX;
		for ( int k=x2;k<=x1+1;k++ )
		{
			if ( k<0 ) continue;
			if ( k>=Map->size ) break;
			if ( iY+ry>=0&&iY+ry<Map->size )
				map[k+ ( iY+ry ) *Map->size]|=1;
			if ( iY-ry>=0&&iY-ry<Map->size )
				map[k+ ( iY-ry ) *Map->size]|=1;
		}

		x1=ry+iX;x2=-ry+iX;
		for ( int k=x2;k<=x1+1;k++ )
		{
			if ( k<0 ) continue;
			if ( k>=Map->size ) break;
			if ( iY+rx>=0&&iY+rx<Map->size )
				map[k+ ( iY+rx ) *Map->size]|=1;
			if ( iY-rx>=0&&iY-rx<Map->size )
				map[k+ ( iY-rx ) *Map->size]|=1;
		}

	}
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

void cClient::addFX( eFXTyps typ, int iX, int iY, int iParam )
{
}

void cClient::addFX( eFXTyps typ, int iX, int iY, sFXRocketInfos* param )
{
}

void cClient::addFX( sFX* iNum )
{
}

bool cClient::doCommand ( string sCmd )
{
	/*if ( sCmd.compare( "fps on" ) == 0 ) {DebugFPS=true;FPSstart=SDL_GetTicks();frames=0;cycles=0;return true;}
	if ( sCmd.compare( "fps off" ) == 0 ) {DebugFPS=false;return true;}*/
	if ( sCmd.compare( "base on" ) == 0 ) { bDebugBase = true; return true; }
	if ( sCmd.compare( "base off" ) == 0 ) { bDebugBase = false; return true; }
	if ( sCmd.compare( "wache on" ) == 0 ) { bDebugWache =true; return true; }
	if ( sCmd.compare( "wache off" ) == 0 ) { bDebugWache =false; return true; }
	if ( sCmd.compare( "fx on" ) == 0 ) { bDebugFX = true; return true; }
	if ( sCmd.compare( "fx off" ) == 0 ) { bDebugFX = false; return true; }
	if ( sCmd.compare( "trace on" ) == 0 ) { bDebugTrace = true; return true; }
	if ( sCmd.compare( "trace off" ) == 0 ) { bDebugTrace = false; return true; }

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

void cClient::handleMessages()
{
	SDL_Rect scr, dest;
	int iHeight;
	sMessage *message;
	if ( messages->iCount == 0 ) return;
	iHeight = 0;
	// Alle alten Nachrichten löschen:
	for ( int i = messages->iCount-1; i >= 0; i-- )
	{
		message = messages->Items[i];
		if ( message->age+MSG_FRAMES < iFrame || iHeight > 200 )
		{
			free ( message->msg );
			free ( message );
			messages->Delete ( i );
			continue;
		}
		iHeight += 14+11*message->len/296;
	}
	if ( messages->iCount == 0 ) return;
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
	for ( int i = 0; i < messages->iCount; i++ )
	{
		message = messages->Items[i];
		font->showTextAsBlock( dest, message->msg );
		dest.y += 14+11*message->len/300;
	}
}

int cClient::HandleEvent( SDL_Event *event )
{
	void *data = event->user.data1;
	switch ( event->user.code )
	{
	case GAME_EV_CHAT:
		break;
	case GAME_EV_ADD_BUILDING:
		{
			cBuilding *AddedBuilding;
			AddedBuilding = ActivePlayer->AddBuilding ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), UnitsData.building+SDL_SwapLE16 ( ((Sint16*)data)[2] ) );

			addUnit ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), AddedBuilding, ((bool*)data)[8] );
		}
		break;
	case GAME_EV_ADD_VEHICLE:
		{
			cVehicle *AddedVehicle;
			AddedVehicle = ActivePlayer->AddVehicle ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), UnitsData.vehicle+SDL_SwapLE16 ( ((Sint16*)data)[2] ) );

			addUnit ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), AddedVehicle, ((bool*)data)[6] );
		}
		break;
	case GAME_EV_DEL_BUILDING:
		{
			cBuilding *Building;
			cPlayer *Player = GetPlayerFromNumber ( SDL_SwapLE16 ( ((Sint16*)data)[2] ) );
			int iOff = ((Sint16*)data)[0]+((Sint16*)data)[1]*Map->size;

			if ( ((bool *)data)[7] ) Building = Map->GO[iOff].base;
			else if ( ((bool *)data)[8] ) Building = Map->GO[iOff].subbase;
			else Building = Map->GO[iOff].top;

			if ( Building && Building->owner == Player )
			{
				deleteUnit ( Building );
			}
		}
		break;
	case GAME_EV_DEL_VEHICLE:
		{
			cVehicle *Vehicle;
			cPlayer *Player = GetPlayerFromNumber ( SDL_SwapLE16 ( ((Sint16*)data)[2] ) );
			int iOff = ((Sint16*)data)[0]+((Sint16*)data)[1]*Map->size;

			if ( ((bool *)data)[6] ) Vehicle = Map->GO[iOff].plane;
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
			cPlayer *Player = GetPlayerFromNumber ( SDL_SwapLE16 ( ((Sint16*)data)[2] ) );
			AddedVehicle = Player->AddVehicle ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), UnitsData.vehicle+SDL_SwapLE16 ( ((Sint16*)data)[3] ) );

			AddedVehicle->data.max_hit_points = SDL_SwapLE16 ( ((Sint16*)data)[4] );
			AddedVehicle->data.max_ammo = SDL_SwapLE16 ( ((Sint16*)data)[5] );
			AddedVehicle->data.max_speed = SDL_SwapLE16 ( ((Sint16*)data)[6] );
			AddedVehicle->data.max_shots = SDL_SwapLE16 ( ((Sint16*)data)[7] );
			AddedVehicle->data.damage = SDL_SwapLE16 ( ((Sint16*)data)[8] );
			AddedVehicle->data.range = SDL_SwapLE16 ( ((Sint16*)data)[9] );
			AddedVehicle->data.scan = SDL_SwapLE16 ( ((Sint16*)data)[10] );
			AddedVehicle->data.armor = SDL_SwapLE16 ( ((Sint16*)data)[11] );
			AddedVehicle->data.costs = SDL_SwapLE16 ( ((Sint16*)data)[12] );
			AddedVehicle->data.hit_points = SDL_SwapLE16 ( ((Sint16*)data)[13] );
			AddedVehicle->data.shots = SDL_SwapLE16 ( ((Sint16*)data)[14] );
			AddedVehicle->data.speed = SDL_SwapLE16 ( ((Sint16*)data)[15] );
			AddedVehicle->dir = SDL_SwapLE16 ( ((Sint16*)data)[16] );
			AddedVehicle->Wachposten = ((bool *)data)[34];
			AddedVehicle->IsBuilding = ((bool *)data)[35];
			AddedVehicle->IsClearing = ((bool *)data)[36];

			addUnit ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), AddedVehicle, false );
		}
		break;
	case GAME_EV_ADD_ENEM_BUILDING:
		{
			cBuilding *AddedBuilding;
			cPlayer *Player = GetPlayerFromNumber ( SDL_SwapLE16 ( ((Sint16*)data)[2] ) );
			AddedBuilding = Player->AddBuilding ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), UnitsData.building+SDL_SwapLE16 ( ((Sint16*)data)[3] ) );

			AddedBuilding->data.max_hit_points = SDL_SwapLE16 ( ((Sint16*)data)[4] );
			AddedBuilding->data.max_ammo = SDL_SwapLE16 ( ((Sint16*)data)[5] );
			AddedBuilding->data.max_shots = SDL_SwapLE16 ( ((Sint16*)data)[6] );
			AddedBuilding->data.damage = SDL_SwapLE16 ( ((Sint16*)data)[7] );
			AddedBuilding->data.range = SDL_SwapLE16 ( ((Sint16*)data)[8] );
			AddedBuilding->data.scan = SDL_SwapLE16 ( ((Sint16*)data)[9] );
			AddedBuilding->data.armor = SDL_SwapLE16 ( ((Sint16*)data)[10] );
			AddedBuilding->data.costs = SDL_SwapLE16 ( ((Sint16*)data)[11] );
			AddedBuilding->data.hit_points = SDL_SwapLE16 ( ((Sint16*)data)[12] );
			AddedBuilding->data.shots = SDL_SwapLE16 ( ((Sint16*)data)[13] );
			AddedBuilding->Wachposten = ((bool *)data)[28];

			addUnit ( SDL_SwapLE16 ( ((Sint16*)data)[0] ), SDL_SwapLE16 ( ((Sint16*)data)[1] ), AddedBuilding, false );
		}
		break;
	}
	free ( data );
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
	// scan with surveyor:
	if ( AddedVehicle->data.can_survey )
	{
		AddedVehicle->DoSurvey();
	}
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
	// intigrate the building to the base:
	ActivePlayer->base->AddBuilding ( AddedBuilding );
}

cPlayer *cClient::GetPlayerFromNumber ( int iNum )
{
	cPlayer *Player;
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
		delete Vehicle;
	}
}
