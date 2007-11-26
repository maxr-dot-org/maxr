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
#include <time.h>
#include <math.h>
#include "menu.h"
#include "pcx.h"
#include "fonts.h"
#include "mouse.h"
#include "keyinp.h"
#include "sound.h"
#include "dialog.h"
#include "game.h"
#include "log.h"
#include "files.h"
#include "loaddata.h"

#define DIALOG_W 640
#define DIALOG_H 480
#define DIALOG_X (SettingsData.iScreenW / 2 - DIALOG_W / 2)
#define DIALOG_Y (SettingsData.iScreenH / 2 - DIALOG_H / 2)	
#define TITLE_X DIALOG_X+320
#define TITLE_Y DIALOG_Y+147
#define INFO_IMG_X DIALOG_X+16
#define INFO_IMG_Y DIALOG_Y+182
#define INFO_IMG_WIDTH 320
#define INFO_IMG_HEIGHT 240

#define BTN_SPACE 35
#define BTN_WIDTH 200
#define BTN_HEIGHT 29
#define BTN_1_X DIALOG_X+390
#define BTN_1_Y DIALOG_Y+190
#define BTN_2_X BTN_1_X
#define BTN_2_Y BTN_1_Y+BTN_SPACE
#define BTN_3_X BTN_1_X
#define BTN_3_Y BTN_1_Y+BTN_SPACE*2
#define BTN_4_X BTN_1_X
#define BTN_4_Y BTN_1_Y+BTN_SPACE*3
#define BTN_5_X BTN_1_X
#define BTN_5_Y BTN_1_Y+BTN_SPACE*4
#define BTN_6_X BTN_1_X
#define BTN_6_Y BTN_1_Y+BTN_SPACE*5
	
/** int for showUnitPicture to prevent same graphic shown twice on click*/
static int s_iLastUnitShown = 0;

// Menü vorbereiten:
void prepareMenu ( bool bIAmMain )
{
	//BEGIN MENU REDRAW
	SDL_Rect dest = { DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H};
	SDL_Surface *sfTmp;
	
	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_MAIN,sfTmp );
	
 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);
	
	//blit sfTmp to buffer and delete it
	SDL_BlitSurface (sfTmp, NULL, buffer, &dest);
	SDL_FreeSurface(sfTmp);	

	//draw infostring with maxversion at the bottom
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Main~Credits_Reloaded" )+ " "+MAX_VERSION,DIALOG_X+320,DIALOG_Y+465,buffer );
	//END MENU REDRAW
		
	//we came back from a submenu so we have to redraw main menu
	if(bIAmMain)
	{
		fonts->OutTextCenter ( lngPack.Translate ( "Text~Menu_Main~Title_Main_Menu" ), TITLE_X, TITLE_Y,buffer );
		drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Single_Player" ),false,BTN_1_X,BTN_1_Y );
		drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Multi_Player" ),false,BTN_2_X,BTN_2_Y );
		//uncommented since no need for right now -- beko
		//drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Map_Editor" ),false,BTN_3_X,BTN_3_Y );
		//drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Credits" ),false,BTN_4_X,BTN_4_Y );
		drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Mani" ),false,BTN_5_X,BTN_5_Y );
		drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),false,BTN_6_X,BTN_6_Y );
	}

	//display random unit
	showUnitPicture();
	
	//show mouse
	mouse->Show();	
	mouse->SetCursor ( CHand );
}

// shows the randomized unit picture
void showUnitPicture ( void )
{
	/**To randomize whether to show a vehicles or a building*/
	int iShowBuilding = random ( 3,1 );
	/*I want 3 possible random numbers
	since a chance of 50:50 is boring (and
	vehicles are way more cool so I prefer
	them to be shown) -- beko */
	/**Unit to show*/
	int iUnitShow;
	/**Destinationrect for unit picture in main menu*/
	SDL_Rect rDest = {INFO_IMG_X, INFO_IMG_Y, INFO_IMG_WIDTH, INFO_IMG_HEIGHT};

	if ( iShowBuilding == 1 ) //that's a 33% chance that we show a building on 1
	{
		do
		{
			iUnitShow = random ( UnitsData.building_anz,0 );
		}
		while ( iUnitShow == s_iLastUnitShown );	//make sure we don't show same unit twice
		SDL_BlitSurface ( UnitsData.building[iUnitShow].info,NULL,buffer,&rDest );
	}
	else //and a 66% chance to show a vehicle on 0 or 2
	{
		do
		{
			iUnitShow = random ( UnitsData.vehicle_anz,0 );
		}
		while ( iUnitShow == s_iLastUnitShown );	//make sure we don't show same unit twice
		SDL_BlitSurface ( UnitsData.vehicle[iUnitShow].info,NULL,buffer,&rDest );
	}
	s_iLastUnitShown = iUnitShow; //store shown unit
}

// Menü aufräumen:
void ExitMenu ( void )
{
	SDL_FillRect ( GraphicsData.gfx_shadow,NULL,(buffer->format, 0, 0, 0)  );
	SDL_SetAlpha ( GraphicsData.gfx_shadow,SDL_SRCALPHA,50 );
}


// Platziert einen kleinen Button:
void PlaceSmallButton (string sText,int x,int y,bool pressed )
{
	SDL_Rect scr,dest;
	scr.w=dest.w=150;
	scr.h=dest.h=29;
	scr.x=0;
	if ( pressed ) scr.y=90;else scr.y=60;
	dest.x=x;
	dest.y=y;
	SDL_BlitSurface ( GraphicsData.gfx_menu_stuff,&scr,buffer,&dest );

	fonts->OutTextBigCenter( sText,x+150/2,y+8,buffer );
}

// Platziert einen spezielen Menübutton:
void PlaceMenuButton (string sText,int x,int y, int darkness, bool pressed )
{
	SDL_Rect scr,dest;
	scr.w=dest.w=109;
	scr.h=dest.h=40;
	scr.y=0;
	scr.x=218 * darkness;
	if ( pressed ) scr.x+=110;
	if ( pressed ) scr.x--;
	dest.x=x;
	dest.y=y;
	SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&dest );

	fonts->OutTextBigCenter ( sText,x+109/2,y+12,buffer );
}

// Platziert einen kleinen spezielen Menübutton:
void PlaceSmallMenuButton ( string sText,int x,int y,bool pressed )
{
	SDL_Rect scr,dest;
	scr.w=dest.w=48;
	scr.h=dest.h=40;
	scr.y=40;
	if ( pressed ) scr.x=49;else scr.x=0;
	dest.x=x;
	dest.y=y;
	SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&dest );

	fonts->OutTextBigCenter ( sText,x+48/2,y+12,buffer );
}

// Platziert einen auswählbaren Text (zentriert):
void PlaceSelectText ( const char *str,int x,int y,bool checked, SDL_Surface *surface,bool center )
{
	SDL_Rect r;
	int len;
	len=fonts->GetTextLen ( str );
	if ( center )
	{
		fonts->OutTextCenter ( str,x,y,buffer );
	}
	else
	{
		fonts->OutText ( str,x,y,buffer );
		x+=len/2;
	}
	r.x=x-len/2-4;
	r.w=len+8;
	r.h=1;
	r.y=y-2;
	if ( checked ) SDL_FillRect ( buffer,&r,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&r );
	r.y+=14;
	r.w++;
	if ( checked ) SDL_FillRect ( buffer,&r,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&r );
	r.y-=14;
	r.w=1;
	r.h=14;
	if ( checked ) SDL_FillRect ( buffer,&r,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&r );
	r.x+=len+8;
	if ( checked ) SDL_FillRect ( buffer,&r,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&r );
}

// Zeigt das Hauptmenü an:
void RunMainMenu ( void )
{
	bool SPPressed=false,MPPRessed=false,MEPressed=false,CrPressed=false,BePressed=false,LiPressed=false;
	bool EscHot=true;
	Uint8 *keystate;
	int b,lb=0,lx=-1,ly=-1;
	// start main musicfile
	PlayMusic ( ( char * ) ( SettingsData.sMusicPath + PATH_DELIMITER + "main.ogg" ).c_str() );

	prepareMenu(true);	
	SHOW_SCREEN

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Tasten prüfen:
		keystate=SDL_GetKeyState ( NULL );
		if ( keystate[SDLK_ESCAPE]&&EscHot ) break;else if ( !keystate[SDLK_ESCAPE] ) EscHot=true;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		// Den Focus machen:
		if ( DoKeyInp ( keystate ) )
		{
			//ShowCursor=true;
		}

		// Klick aufs Bild:
		if ( b && !lb && mouse->x >= INFO_IMG_X && mouse->x < INFO_IMG_X + INFO_IMG_WIDTH && mouse->y >= INFO_IMG_Y && mouse->y < INFO_IMG_Y + INFO_IMG_HEIGHT )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			showUnitPicture();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Einzelspieler:
		if ( mouse->x >= BTN_1_X && mouse->x < BTN_1_X + BTN_WIDTH && mouse->y >= BTN_1_Y && mouse->y < BTN_1_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				SPPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Single_Player" ),true,BTN_1_X,BTN_1_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&SPPressed )
			{
				RunSPMenu();
				prepareMenu(true);
				SHOW_SCREEN
				SPPressed=false;
				EscHot=false;
			}
		}
		else if ( SPPressed )
		{
			SPPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Single_Player" ),false,BTN_1_X,BTN_1_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Mehrspieler:
		if ( mouse->x >= BTN_2_X && mouse->x < BTN_2_X + BTN_WIDTH && mouse->y >= BTN_2_Y && mouse->y < BTN_2_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				MPPRessed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Multi_Player" ), true,BTN_2_X,BTN_2_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&MPPRessed )
			{
				RunMPMenu();
				prepareMenu(true);
				SHOW_SCREEN
				SPPressed=false;
				EscHot=false;
			}
		}
		else if ( MPPRessed )
		{
			MPPRessed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Multi_Player" ),false,BTN_2_X,BTN_2_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Map-Editor:
		/*
		if ( mouse->x >= BTN_3_X && mouse->x < BTN_3_X + BTN_WIDTH && mouse->y >= BTN_3_Y && mouse->y < BTN_3_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				MEPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Map_Editor" ),true,BTN_3_X,BTN_3_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&MEPressed )
			{
				//cMapEditor *me;
				//ExitMenu();
				//
				//me=new cMapEditor();
				//me->Run();
				//delete me;

				prepareMenu(true);
				SHOW_SCREEN
				MEPressed=false;
				EscHot=false;
			}
		}
		else if ( MEPressed )
		{
			MEPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Map_Editor" ),false,BTN_3_X,BTN_3_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		} */
		// Credits:
		/*
		if ( mouse->x >= BTN_4_X && mouse->x < BTN_4_X + BTN_WIDTH && mouse->y >= BTN_4_Y && mouse->y < BTN_4_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				CrPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Credits" ),true,BTN_4_X,BTN_4_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&CrPressed )
			{
// 				cCredits *cred;
// 				cred=new cCredits();
// 				cred->Run();
// 				delete cred;
				prepareMenu(true);
				SHOW_SCREEN
				EscHot=false;
				CrPressed=false;
			}
		}
		else if ( CrPressed )
		{
			CrPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Credits" ),false,BTN_4_X,BTN_4_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
*/
		//Licence
		if ( mouse->x >= BTN_5_X && mouse->x < BTN_5_X + BTN_WIDTH && mouse->y >= BTN_5_Y  && mouse->y < BTN_5_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				LiPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Mani" ),true,BTN_5_X,BTN_5_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&LiPressed )
			{
				mouse->draw ( false,screen );
				//SHOW_SCREEN
				showLicence();
				prepareMenu(true);
				SHOW_SCREEN
				LiPressed=false;
				CrPressed=false;
			}
		}
		else if ( CrPressed )
		{
			CrPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Mani" ),false,BTN_5_X,BTN_5_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		
		// Beenden:
		if ( mouse->x >= BTN_6_X && mouse->x < BTN_6_X + BTN_WIDTH && mouse->y >= BTN_6_Y && mouse->y < BTN_6_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				BePressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),true,BTN_6_X,BTN_6_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&BePressed )
			{
				break;
			}
		}
		else if ( BePressed )
		{
			BePressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),false,BTN_6_X,BTN_6_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}

	ExitMenu();
	StopMusic();
}

// Zeigt das Multiplayermenü an:
void RunMPMenu ( void )
{
	//defines for translation since I'm very lazy --beko
#define TCPIPHOST lngPack.Translate("Text~Menu_Main~Button_TCPIP_Host")
#define TCPIPCLIENT lngPack.Translate( "Text~Menu_Main~Button_TCPIP_Client")
#define NEWHOTSEAT lngPack.Translate( "Text~Menu_Main~Button_HotSeat_New")
#define LOADHOTSEAT lngPack.Translate( "Text~Menu_Main~Button_HotSeat_Load")
#define BACK lngPack.Translate( "Text~Menu_Main~Button_Back")
	bool TCPHostPressed=false,TCPClientPressed=false,BackPressed=false,HotSeatPressed=false,LoadHotSeatPressed=false;
	Uint8 *keystate;
	int b,lb=0,lx=-1,ly=-1;

	prepareMenu();
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Menu_Main~Button_Multi_Player" ).c_str(),TITLE_X, TITLE_Y,buffer );

	drawMenuButton ( TCPIPHOST, false, BTN_1_X, BTN_1_Y );
	drawMenuButton ( TCPIPCLIENT, false, BTN_2_X, BTN_2_Y );
	drawMenuButton ( NEWHOTSEAT, false, BTN_3_X, BTN_3_Y );
	drawMenuButton ( LOADHOTSEAT, false, BTN_4_X, BTN_4_Y );
	drawMenuButton ( BACK, false, BTN_6_X, BTN_6_Y );

	SHOW_SCREEN

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Tasten prüfen:
		keystate=SDL_GetKeyState ( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		// Klick aufs Bild:
		if ( b && !lb && mouse->x >= INFO_IMG_X && mouse->x < INFO_IMG_X + INFO_IMG_WIDTH && mouse->y >= INFO_IMG_Y && mouse->y < INFO_IMG_Y + INFO_IMG_HEIGHT )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			showUnitPicture();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// TCP Host:
		if ( mouse->x >= BTN_1_X && mouse->x < BTN_1_X + BTN_WIDTH && mouse->y >= BTN_1_Y && mouse->y < BTN_1_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				TCPHostPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( TCPIPHOST,true,BTN_1_X,BTN_1_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&TCPHostPressed )
			{
				MultiPlayer=new cMultiPlayer ( true,true );
				MultiPlayer->RunMenu();
				delete MultiPlayer;
				break;
			}
		}
		else if ( TCPHostPressed )
		{
			TCPHostPressed=false;
			drawMenuButton ( TCPIPHOST,false,BTN_1_X,BTN_1_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// TCP Client:
		if ( mouse->x >= BTN_2_X && mouse->x < BTN_2_X + BTN_WIDTH && mouse->y >= BTN_2_Y && mouse->y < BTN_2_Y + BTN_HEIGHT)
		{
			if ( b&&!lb )
			{
				TCPClientPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( TCPIPCLIENT,true,BTN_2_X,BTN_2_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&TCPClientPressed )
			{
				MultiPlayer=new cMultiPlayer ( false,true );
				MultiPlayer->RunMenu();
				delete MultiPlayer;
				break;
			}
		}
		else if ( TCPClientPressed )
		{
			TCPClientPressed=false;
			drawMenuButton ( TCPIPCLIENT,false,BTN_2_X,BTN_2_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Hot Seat:
		if ( mouse->x >= BTN_3_X && mouse->x < BTN_3_X + BTN_WIDTH && mouse->y >= BTN_3_Y && mouse->y < BTN_3_Y + BTN_HEIGHT)
		{
			if ( b&&!lb )
			{
				HotSeatPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( NEWHOTSEAT,true,BTN_3_X, BTN_3_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&HotSeatPressed )
			{
				HeatTheSeat();
				break;
			}
		}
		else if ( HotSeatPressed )
		{
			HotSeatPressed=false;
			drawMenuButton ( NEWHOTSEAT,false,BTN_3_X, BTN_3_Y);
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Hot Seat laden:
		if ( mouse->x >= BTN_4_X && mouse->x < BTN_4_X + BTN_WIDTH && mouse->y >= BTN_4_Y && mouse->y < BTN_4_Y + BTN_HEIGHT)
		{
			if ( b&&!lb )
			{
				LoadHotSeatPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( LOADHOTSEAT,true,BTN_4_X,BTN_4_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&LoadHotSeatPressed )
			{
				if ( ShowDateiMenu( false ) != -1 )
				{
					cMap *map;
					map=new cMap();
					game=new cGame ( NULL, map );
					ExitMenu();
					if ( !SaveLoadFile.empty() )
					{
						game->HotSeat=true;
						game->Load ( SaveLoadFile,0 );
					}
					delete game;game=NULL;
					delete map;
					break;
				}
				prepareMenu();
				RunSPMenu();
				break;
			}
		}
		else if ( LoadHotSeatPressed )
		{
			LoadHotSeatPressed=false;
			drawMenuButton ( LOADHOTSEAT,false,BTN_4_X,BTN_4_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Zurück:
		if ( mouse->x >= BTN_6_X && mouse->x < BTN_6_X + BTN_WIDTH && mouse->y >= BTN_6_Y && mouse->y < BTN_6_Y + BTN_HEIGHT)
		{
			if ( b&&!lb )
			{
				BackPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( BACK,true,BTN_6_X, BTN_6_Y);
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&BackPressed )
			{
				break;
			}
		}
		else if ( BackPressed )
		{
			BackPressed=false;
			drawMenuButton ( BACK,false,BTN_6_X, BTN_6_Y);
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}
}

void RunSPMenu ( void )
{
	//defines for translation since I'm very lazy --beko
#define SINGLEPLAYER lngPack.Translate("Text~Menu_Main~Button_Single_Player")
#define TRAINING lngPack.Translate( "Text~Menu_Main~Button_Training")
#define NEWGAME lngPack.Translate( "Text~Menu_Main~Button_Game_New")
#define LOADGAME lngPack.Translate( "Text~Menu_Main~Button_Game_Load")
#define BACK lngPack.Translate( "Text~Menu_Main~Button_Back")
	bool StartTrainingPressed=false, StartNewPressed=false, LoadPressed=false, BackPressed=false;
	Uint8 *keystate;
	int b,lb=0,lx=-1,ly=-1;

	prepareMenu();
	fonts->OutTextCenter ( SINGLEPLAYER,TITLE_X, TITLE_Y,buffer );

	drawMenuButton ( TRAINING,false,BTN_1_X, BTN_1_Y );
	drawMenuButton ( NEWGAME,false,BTN_2_X, BTN_2_Y );
	drawMenuButton ( LOADGAME,false,BTN_3_X, BTN_3_Y );
	drawMenuButton ( BACK,false,BTN_6_X, BTN_6_Y );

	SHOW_SCREEN

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Tasten prüfen:
		keystate=SDL_GetKeyState ( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}
		// Klick aufs Bild:
		if ( b && !lb && mouse->x >= INFO_IMG_X && mouse->x < INFO_IMG_X + INFO_IMG_WIDTH && mouse->y >= INFO_IMG_Y && mouse->y < INFO_IMG_Y + INFO_IMG_HEIGHT )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			showUnitPicture();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Training starten:
		if ( mouse->x >= BTN_1_X && mouse->x < BTN_1_X + BTN_WIDTH && mouse->y >= BTN_1_Y && mouse->y < BTN_1_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				StartTrainingPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( TRAINING,true,BTN_1_X, BTN_1_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&StartTrainingPressed )
			{
				StartTrainingPressed=false;
				drawMenuButton ( TRAINING,false,BTN_1_X, BTN_1_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		else if ( StartTrainingPressed )
		{
			StartTrainingPressed=false;
			drawMenuButton ( TRAINING,false,BTN_1_X,BTN_1_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Neues Spiel starten:
		if ( mouse->x >= BTN_2_X && mouse->x < BTN_2_X + BTN_WIDTH && mouse->y >= BTN_2_Y && mouse->y < BTN_2_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				StartNewPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( NEWGAME,true,BTN_2_X,BTN_2_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&StartNewPressed )
			{
				sOptions options;
				string MapName = "";
				options = RunOptionsMenu ( NULL );
				if ( options.metal == -1 ) break;
				MapName = RunPlanetSelect();

				if ( !MapName.empty() )
				{
					int i,LandX,LandY;
					TList *list, *LandingList;
					cMap *map;
					cPlayer *p;
					map=new cMap;
					sPlayer players;
					if ( !map->LoadMap ( MapName ) )
					{
						delete map;
						break;
					}
					map->PlaceRessources ( options.metal,options.oil,options.gold,options.dichte );
					players = RunPlayerSelect();

					list=new TList;
					list->AddPlayer ( p=new cPlayer ( SettingsData.sPlayerName.c_str(),OtherData.colors[cl_red],1 ) );
					list->AddPlayer ( new cPlayer ( "Player 2",OtherData.colors[cl_green],2 ) );

					game = new cGame ( NULL, map );
					game->AlienTech = options.AlienTech;
					game->PlayRounds = options.PlayRounds;
					game->ActiveRoundPlayerNr = p->Nr;
					game->Init ( list,0 );

					for ( i=0;i<list->Count;i++ )
						list->PlayerItems[i]->InitMaps ( map->size );

					p->Credits=options.credits;

					LandingList = new TList;
					RunHangar ( p, LandingList );
					SelectLanding ( &LandX, &LandY, map );
					game->MakeLanding ( LandX,LandY,p,LandingList,options.FixedBridgeHead );

					ExitMenu();

					game->Run();

					SettingsData.sPlayerName=p->name;
					while ( list->Count )
					{
						delete ( ( cPlayer* ) ( list->PlayerItems[0] ) );
						list->DeletePlayer ( 0 );
					}
					delete game; game=NULL;
					delete map;
					delete LandingList;
					delete list;
					break;
				}
				break;
			}
		}
		else if ( StartNewPressed )
		{
			StartNewPressed=false;
			drawMenuButton ( NEWGAME,false,BTN_2_X,BTN_2_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Spiel laden:
		if ( mouse->x >= BTN_3_X && mouse->x < BTN_3_X + BTN_WIDTH && mouse->y >= BTN_3_Y && mouse->y < BTN_3_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				LoadPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( LOADGAME,false,BTN_3_X, BTN_3_Y );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&LoadPressed )
			{
				if ( ShowDateiMenu( false ) != -1 )
				{
					cMap *map;
					map=new cMap();
					game=new cGame ( NULL, map );
					ExitMenu();
					if ( !SaveLoadFile.empty() )
					{
						game->Load ( SaveLoadFile,0 );
					}
					delete game; game=NULL;
					delete map;
					break;
				}
				prepareMenu();
				RunSPMenu();
				break;
			}
		}
		else if ( LoadPressed )
		{
			LoadPressed=false;
			drawMenuButton ( LOADGAME,false,BTN_3_X, BTN_3_Y );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Zurück:
		if ( mouse->x >= BTN_6_X && mouse->x < BTN_6_X + BTN_WIDTH && mouse->y >= BTN_6_Y && mouse->y < BTN_6_Y + BTN_HEIGHT )
		{
			if ( b&&!lb )
			{
				BackPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( BACK,true,BTN_6_X, BTN_6_Y);
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&BackPressed )
			{
				break;
			}
		}
		else if ( BackPressed )
		{
			BackPressed=false;
			drawMenuButton ( BACK,false,BTN_6_X, BTN_6_Y);
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}
}

// Zeigt die Optionen an:
sOptions RunOptionsMenu ( sOptions *init )
{
	//defines for translation since I'm very lazy --beko
#define GAMEOPTIONS lngPack.Translate("Text~Menu_Main~Button_Game_Options")
#define OK lngPack.Translate( "Text~Menu_Main~Button_OK")
#define BACK lngPack.Translate( "Text~Menu_Main~Button_Back")

#define LOWEST lngPack.Translate( "Text~Game_Options~Option_Lowest").c_str()
#define LOWER lngPack.Translate( "Text~Game_Options~Option_Lower").c_str()
#define LOW lngPack.Translate( "Text~Game_Options~Option_Low").c_str()
#define MIDDLE lngPack.Translate( "Text~Game_Options~Option_Normal").c_str()
#define MUCH lngPack.Translate( "Text~Game_Options~Option_Much").c_str()
#define MORE lngPack.Translate( "Text~Game_Options~Option_More").c_str()
#define MOST lngPack.Translate( "Text~Game_Options~Option_Most").c_str()
#define THIN lngPack.Translate( "Text~Game_Options~Option_Thin").c_str()
#define THICK lngPack.Translate( "Text~Game_Options~Option_Thick").c_str()
#define ON lngPack.Translate( "Text~Game_Options~Option_On").c_str()
#define OFF lngPack.Translate( "Text~Game_Options~Option_Off").c_str()
#define DEFINITE lngPack.Translate( "Text~Game_Options~Option_Definite").c_str()
#define MOBILE lngPack.Translate( "Text~Game_Options~Option_Mobile").c_str()
#define TURNS lngPack.Translate( "Text~Game_Options~Option_Type_Turns").c_str()
#define SIMU lngPack.Translate( "Text~Game_Options~Option_Type_Simu").c_str()

#define METAL lngPack.Translate( "Text~Game_Options~Title_Metal")
#define OIL lngPack.Translate( "Text~Game_Options~Title_Oil")
#define GOLD lngPack.Translate( "Text~Game_Options~Title_Gold")
#define RESOURCE lngPack.Translate( "Text~Game_Options~Title_Resource_Density")
#define HEAD lngPack.Translate( "Text~Game_Options~Title_BridgeHead")
#define CREDITS lngPack.Translate( "Text~Game_Options~Title_Credits")
#define ALIEN lngPack.Translate( "Text~Game_Options~Title_Alien_Tech")
#define GAMETYPE lngPack.Translate( "Text~Game_Options~Title_Game_Type")
	//beko IS lazy. fact is.

	bool OKPressed=false, BackPressed=false;
	sOptions options;
	int b,lb=0,lx=-1,ly=-1;

	if ( init==NULL )
	{
		options.metal=1;
		options.oil=1;
		options.gold=1;
		options.dichte=1;
		options.credits=100;
		options.FixedBridgeHead=true;
		options.AlienTech=false;
		options.PlayRounds=false;
	}
	else
	{
		options=*init;
	}

	SDL_Rect dest = { DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H};
	SDL_Surface *sfTmp;
	
	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_OPTIONS,sfTmp );
	
 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);
	
	//blit sfTmp to buffer
	SDL_BlitSurface (sfTmp, NULL, buffer, NULL); //FIXME: use dest and make this working > 640x480
	fonts->OutTextCenter ( GAMEOPTIONS,320,11,buffer );

	// Ressourcen:
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Options~Title_Resource" ).c_str(),110,56,buffer );

	fonts->OutText ( METAL.c_str(),17,86,buffer );
	PlaceSelectText ( LOW,38,86+16,options.metal==0, sfTmp );
	PlaceSelectText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp);
	PlaceSelectText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
	PlaceSelectText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );

	fonts->OutText ( OIL.c_str(),17,124,buffer );
	PlaceSelectText ( LOW,38,124+16,options.oil==0, sfTmp );
	PlaceSelectText ( MIDDLE,38+45,124+16,options.oil==1 , sfTmp);
	PlaceSelectText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
	PlaceSelectText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );

	fonts->OutText ( GOLD.c_str(),17,162,buffer );
	PlaceSelectText ( LOW,38,162+16,options.gold==0, sfTmp );
	PlaceSelectText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
	PlaceSelectText ( MUCH,38+45*2,162+16,options.gold==2 , sfTmp);
	PlaceSelectText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );

	// Credits:
	fonts->OutTextCenter ( CREDITS.c_str(),110+211,56,buffer );

	PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
	PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
	PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
	PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
	PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
	PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
	PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );

	// Brückenkopf:
	fonts->OutTextCenter ( HEAD.c_str(),110+211*2,56,buffer );

	PlaceSelectText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
	PlaceSelectText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );

	// AlienTechs:
	fonts->OutTextCenter ( ALIEN.c_str(),110,251,buffer );

	PlaceSelectText ( ON,38,281,options.AlienTech, sfTmp );
	PlaceSelectText ( OFF,38,281+20,!options.AlienTech, sfTmp );

	// Ressourcendichte:
	fonts->OutTextCenter ( RESOURCE.c_str(),110+211,251,buffer );

	PlaceSelectText ( THIN,110+130,281,options.dichte==0, sfTmp,false );
	PlaceSelectText ( MIDDLE,110+130,281+20,options.dichte==1, sfTmp,false );
	PlaceSelectText ( THICK,110+130,281+20*2,options.dichte==2, sfTmp,false );
	PlaceSelectText ( MOST,110+130,281+20*3,options.dichte==3, sfTmp,false );

	// Spielart:
	fonts->OutTextCenter ( GAMETYPE.c_str(),110+211*2,251,buffer );

	PlaceSelectText ( SIMU,452,281,!options.PlayRounds, sfTmp,false );
	PlaceSelectText ( TURNS,452,281+20,options.PlayRounds, sfTmp,false );

	drawMenuButton ( OK,false,390,440);
	drawMenuButton ( BACK,false,50,440);
	SHOW_SCREEN
	mouse->draw ( false,screen );

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		// Klick aufs Metall:
		if ( b&&!lb&&mouse->x>=38-20&&mouse->x<38+20&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=0;
			PlaceSelectText ( LOW,38,86+16,options.metal==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45&&mouse->x<38+20+45&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=1;
			PlaceSelectText ( LOW,38,86+16,options.metal==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*2&&mouse->x<38+20+45*2&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=2;
			PlaceSelectText ( LOW,38,86+16,options.metal==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*3&&mouse->x<38+20+45*3&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=3;
			PlaceSelectText ( LOW,38,86+16,options.metal==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick aufs Öl:
		if ( b&&!lb&&mouse->x>=38-20&&mouse->x<38+20&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=0;
			PlaceSelectText ( LOW,38,124+16,options.oil==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45&&mouse->x<38+20+45&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=1;
			PlaceSelectText ( LOW,38,124+16,options.oil==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*2&&mouse->x<38+20+45*2&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=2;
			PlaceSelectText ( LOW,38,124+16,options.oil==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*3&&mouse->x<38+20+45*3&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=3;
			PlaceSelectText ( LOW,38,124+16,options.oil==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick aufs Gold:
		if ( b&&!lb&&mouse->x>=38-20&&mouse->x<38+20&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=0;
			PlaceSelectText ( LOW,38,162+16,options.gold==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45&&mouse->x<38+20+45&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=1;
			PlaceSelectText ( LOW,38,162+16,options.gold==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*2&&mouse->x<38+20+45*2&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=2;
			PlaceSelectText ( LOW,38,162+16,options.gold==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*3&&mouse->x<38+20+45*3&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=3;
			PlaceSelectText ( LOW,38,162+16,options.gold==0, sfTmp );
			PlaceSelectText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			PlaceSelectText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			PlaceSelectText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick auf die Credits:
		if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4&&mouse->y<86-4+20 )
		{
			options.credits=25;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20&&mouse->y<86-4+20+20 )
		{
			options.credits=50;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*2&&mouse->y<86-4+20+20*2 )
		{
			options.credits=100;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*3&&mouse->y<86-4+20+20*3 )
		{
			options.credits=150;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*4&&mouse->y<86-4+20+20*4 )
		{
			options.credits=200;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*5&&mouse->y<86-4+20+20*5 )
		{
			options.credits=250;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*6&&mouse->y<86-4+20+20*6 )
		{
			options.credits=300;
			PlaceSelectText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			PlaceSelectText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			PlaceSelectText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			PlaceSelectText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			PlaceSelectText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			PlaceSelectText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Brückenkopf:
		if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=86-4&&mouse->y<86-4+14 )
		{
			options.FixedBridgeHead=false;
			PlaceSelectText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
			PlaceSelectText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=86-4+20&&mouse->y<86-4+14+20 )
		{
			options.FixedBridgeHead=true;
			PlaceSelectText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
			PlaceSelectText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// AlienTech:
		if ( b&&!lb&&mouse->x>=30&&mouse->x<38+100&&mouse->y>=281-4&&mouse->y<281-4+14 )
		{
			options.AlienTech=true;
			PlaceSelectText ( ON,38,281,options.AlienTech, sfTmp );
			PlaceSelectText ( OFF,38,281+20,!options.AlienTech, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=30&&mouse->x<38+100&&mouse->y>=281-4+20&&mouse->y<281-4+14+20 )
		{
			options.AlienTech=false;
			PlaceSelectText ( ON,38,281,options.AlienTech, sfTmp );
			PlaceSelectText ( OFF,38,281+20,!options.AlienTech, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Ressourcendichte:
		if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281-4&&mouse->y<281-4+20 )
		{
			options.dichte=0;
			PlaceSelectText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			PlaceSelectText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			PlaceSelectText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281+20-4&&mouse->y<281+20-4+20 )
		{
			options.dichte=1;
			PlaceSelectText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			PlaceSelectText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			PlaceSelectText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281+20*2-4&&mouse->y<281+20*2-4+20 )
		{
			options.dichte=2;
			PlaceSelectText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			PlaceSelectText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			PlaceSelectText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281+20*3-4&&mouse->y<281+20*3-4+20 )
		{
			options.dichte=3;
			PlaceSelectText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			PlaceSelectText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			PlaceSelectText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			PlaceSelectText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Spielart:
		if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=281-4&&mouse->y<281-4+20 )
		{
			options.PlayRounds=false;
			PlaceSelectText ( SIMU,452,281,!options.PlayRounds,sfTmp,false );
			PlaceSelectText ( TURNS,452,281+20,options.PlayRounds,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=281+20-4&&mouse->y<281+20-4+20 )
		{
			options.PlayRounds=true;
			PlaceSelectText ( SIMU,452,281,!options.PlayRounds,sfTmp,false );
			PlaceSelectText ( TURNS,452,281+20,options.PlayRounds,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Zurück:
		if ( mouse->x>=50&&mouse->x<50+200&&mouse->y>=440&&mouse->y<440+29 )
		{
			if ( b&&!lb )
			{
				BackPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( BACK,true,50,440);
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&BackPressed )
			{
				options.metal=-1;
				break;
			}
		}
		else if ( BackPressed )
		{
			BackPressed=false;
			drawMenuButton ( BACK,false,50,440);
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Ok:
		if ( mouse->x>=390&&mouse->x<390+200&&mouse->y>=440&&mouse->y<440+29 )
		{
			if ( b&&!lb )
			{
				OKPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( OK,true,390,440 );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&OKPressed )
			{
				break;
			}
		}
		else if ( OKPressed )
		{
			OKPressed=false;
			drawMenuButton ( OK,false,390,440 );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}

	SDL_FreeSurface(sfTmp);
	return options;
}

// Startet die Planetenauswahl (gibt den Namen des Planeten zurück):
string RunPlanetSelect ( void )
{
	bool OKPressed=false;
	Uint8 *keystate;
	int b,lb=0,offset=0,selected=-1,i,lx=-1,ly=-1;
	TList *files;
	SDL_Rect scr;
	TiXmlDocument doc;
	TiXmlNode* rootnode;
	TiXmlNode* node;

	SDL_Rect dest = { DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H};
	SDL_Surface *sfTmp;
	
	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_PLANET_SELECT,sfTmp );
	
 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);
	
	//blit sfTmp to buffer
	SDL_BlitSurface (sfTmp, NULL, buffer, NULL); //FIXME: use dest and make this working > 640x480

	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Choose_Planet" ).c_str() ,320,11,buffer );

	drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,440 );

	files = new TList;
	int k = 1;

	if ( !doc.LoadFile ( "maps//maps.xml" ) )
	{
		cLog::write ( "Could not load maps.xml",1 );
		return "";
	}
	rootnode=doc.FirstChildElement ( "MapData" )->FirstChildElement ( "MapList" );

	node=rootnode->FirstChildElement();
	if ( node )
		files->Add ( node->ToElement()->Attribute ( "file" ) );
	while ( node )
	{
		node=node->NextSibling();
		if ( node && node->Type() ==1 )
			files->Add ( node->ToElement()->Attribute ( "file" ) );
	}

	ShowPlanets ( files,offset,selected, sfTmp );
	mouse->Show();
	mouse->SetCursor ( CHand );
	SHOW_SCREEN

#define TEAR_DOWN while(files->Count){/*delete (files->Items(0));*/files->DeleteString(0);}delete files;

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Tasten prüfen:
		keystate=SDL_GetKeyState ( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}
		// Ok:
		if ( mouse->x>=390&&mouse->x<390+200&&mouse->y>=440&&mouse->y<440+29&&selected>=0 )
		{
			if ( b&&!lb )
			{
				OKPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), true, 390,440 );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&OKPressed )
			{
				string name;
				name = files->Items[selected];
				name.replace ( name.length()-3,3,"map" );
				TEAR_DOWN
				return name;
			}
		}
		else if ( OKPressed )
		{
			OKPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,440 );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Up:
		if ( mouse->x>=293&&mouse->x<293+24&&mouse->y>=440&&mouse->y<440+24&&b&&!lb&&offset>0 )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			offset-=4;
			ShowPlanets ( files,offset,selected, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Down:
		if ( mouse->x>=321&&mouse->x<321+24&&mouse->y>=440&&mouse->y<440+24&&b&&!lb&&files->Count-8-offset>0 )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			offset+=4;
			ShowPlanets ( files,offset,selected, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick auf eine Map:
		if ( b&&!lb )
		{
			scr.x=25;
			scr.y=90;
			for ( i=0;i<8;i++ )
			{
				if ( i+offset>=files->Count ) break;

				if ( mouse->x>=scr.x&&mouse->x<scr.x+112&&mouse->y>=scr.y&&mouse->y<scr.y+112 )
				{
					selected=i+offset;
					PlayFX ( SoundData.SNDObjectMenu );
					ShowPlanets ( files,offset,selected, sfTmp );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					break;
				}

				scr.x+=158;
				if ( i==3 )
				{
					scr.x=25;
					scr.y+=197;
				}
			}
		}

		lx=mouse->x;
		ly=mouse->y;
		lb=b;
		SDL_Delay ( 1 );
	}

	TEAR_DOWN
	SDL_FreeSurface(sfTmp);
	return "";
}

// Zeigt die Planeten an:
void ShowPlanets ( TList *files,int offset,int selected, SDL_Surface *surface )
{
	SDL_Surface *sf;
	SDL_Rect scr={640, 390, 0, 38},dest;
	string sMap;
	string sPath;
	int size;
	FILE *fp;

	SDL_BlitSurface ( surface,&scr,buffer,&scr );

	scr.x=25; scr.y=90; scr.h=scr.w=112;

	cLog::write ( "Loading Maps", cLog::eLOG_TYPE_INFO );

	for ( int i=0;i<8;i++ ) //only 8 maps on one screen
	{
		if ( i+offset>=files->Count ) break;
		sMap = files->Items[i+offset];
		sPath = sMap;
		sPath.insert ( 0, PATH_DELIMITER );
		sPath.insert ( 0,SettingsData.sMapsPath );

		if ( FileExists ( sPath.c_str() ) )
		{

			sf = SDL_LoadBMP ( sPath.c_str() );
			if ( sf!=NULL )
			{
				SDL_BlitSurface ( sf,NULL,buffer,&scr );
			}
			sPath.replace ( sPath.length()-3, 3, "map" );
			cLog::write ( sPath.c_str(), cLog::eLOG_TYPE_DEBUG );
			fp=fopen ( sPath.c_str(),"rb" );
			fseek ( fp,21,SEEK_CUR );
			fread ( &size,sizeof ( int ),1,fp );
			fclose ( fp );

			SDL_Rect r;
			r=scr;
			//borders
#define SELECTED 0x00C000
#define UNSELECTED 0x000000
			//TOFIX: remove old offset on old selection after selecting new map - painting other borders just black for now -- beko
			if ( selected==i+offset ) //draw offset "green border"
			{

				r.x-=4; r.y-=4; r.w+=8; r.h=4;
				SDL_FillRect ( buffer,&r,SELECTED );
				r.w=4; r.h=112+8;
				SDL_FillRect ( buffer,&r,SELECTED );
				r.x+=112+4;
				SDL_FillRect ( buffer,&r,SELECTED );
				r.x-=112+4; r.y+=112+4; r.w=112+8; r.h=4;
				SDL_FillRect ( buffer,&r,SELECTED );

				char tmp[16];
				sprintf ( tmp,"%d",size );
				sMap.insert ( 0,"> " );
				sMap.replace ( sMap.length()-4, 2, " (" );
				sMap.replace ( sMap.length()-2, 3, tmp );
				sMap.insert ( sMap.length(),"x" );
				sMap.replace ( sMap.length(), 3, tmp );
				sMap.insert ( sMap.length(),") <" );
			}
			else
			{
				r.x-=4; r.y-=4; r.w+=8; r.h=4;
				SDL_FillRect ( buffer,&r,UNSELECTED );
				r.w=4; r.h=112+8;
				SDL_FillRect ( buffer,&r,UNSELECTED );
				r.x+=112+4;
				SDL_FillRect ( buffer,&r,UNSELECTED );
				r.x-=112+4; r.y+=112+4; r.w=112+8; r.h=4;
				SDL_FillRect ( buffer,&r,UNSELECTED );

				char tmp[16];
				sprintf ( tmp,"%d",size );
				sMap.replace ( sMap.length()-4, 2, " (" );
				sMap.replace ( sMap.length()-2, 3, tmp );
				sMap.insert ( sMap.length(),"x" );
				sMap.replace ( sMap.length(), 3, tmp );
				sMap.insert ( sMap.length(),")" );
			}
			fonts->OutTextCenter ( ( char * ) sMap.c_str(),scr.x+77-21,scr.y-42,buffer );

			scr.x+=158;
			if ( i==3 )
			{
				scr.x=25;
				scr.y+=197;
			}


		}
		else
		{
			//error - do nothing.
		}

		if ( sf!=NULL )
		{
			SDL_FreeSurface ( sf );
		}
	}
	cLog::mark();

	// Die Up-Down Buttons machen:
	if ( offset )
	{
		scr.x=130;scr.y=452;
		dest.h=scr.h=dest.w=scr.w=25;
		dest.x=293;
		dest.y=440;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		dest.h=dest.w=25;
		dest.x=293;
		dest.y=440;
		SDL_BlitSurface ( surface,&dest,buffer,&dest );
	}
	if ( files->Count-8-offset>0 )
	{
		scr.x=103;scr.y=452;
		dest.h=scr.h=dest.w=scr.w=25;
		dest.x=321;
		dest.y=440;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		dest.h=dest.w=25;
		dest.x=321;
		dest.y=440;
		SDL_BlitSurface ( surface,&dest,buffer,&dest );
	}
}

// Startet die Spielerauswahl (gibt die Spielereinstellungen):
sPlayer RunPlayerSelect ( void )
{
	bool OKPressed=false;
	int b,lb=0,offset=0,lx=-1,ly=-1;
	sPlayer players;

	for ( int i = 0; i < 4; i++ )
	{
		players.clan[i] = "NONE";
		players.what[i] = 0;
	}
	players.what[0] = 1;
	players.what[1] = 2;

	SDL_BlitSurface ( GraphicsData.gfx_player_select,NULL,buffer,NULL );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Player_Select" ) ,320,11,buffer );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Team" ) ,100,35,buffer );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Human" ) ,200,35,buffer );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Team" ) ,310,35,buffer );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Nobody" ) ,420,35,buffer );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Clan" ) ,535,35,buffer );

	drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false ,390,440 );
	ShowPlayerStates ( players );

	mouse->Show();
	mouse->SetCursor ( CHand );
	SHOW_SCREEN
	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}
		// Änderungen:
		if ( b&&!lb )
		{
			int x = 175;
			int y = 67;
			for ( int i = 0; i < 12; i++ )
			{
				if ( mouse->x>=x&&mouse->x<x+55&&mouse->y>=y&&mouse->y<y+71 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					if ( i == 0 ) players.what[0] = 1;
					if ( i == 1 ) players.what[0] = 2;
					if ( i == 2 ) players.what[0] = 0;

					if ( i == 3 ) players.what[1] = 1;
					if ( i == 4 ) players.what[1] = 2;
					if ( i == 5 ) players.what[1] = 0;

					if ( i == 6 ) players.what[2] = 1;
					if ( i == 7 ) players.what[2] = 2;
					if ( i == 8 ) players.what[2] = 0;

					if ( i == 9 ) players.what[3] = 1;
					if ( i == 10 ) players.what[3] = 2;
					if ( i == 11 ) players.what[3] = 0;
					ShowPlayerStates ( players );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				if ( x == 395 )
				{
					x = 175;
					y += 92;
				}
				else x += 110;
			}
		}
		// Ok:
		if ( mouse->x>=390&&mouse->x<390+200&&mouse->y>=440&&mouse->y<440+29 && ( players.what[0] == 1 || players.what[1] == 1 || players.what[2] == 1 || players.what[3] == 1 ) )
		{
			if ( b&&!lb )
			{
				OKPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), true, 390,440 );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&OKPressed )
			{
				return players;
			}
		}
		else if ( OKPressed )
		{
			OKPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,440 );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		lx=mouse->x;
		ly=mouse->y;
		lb=b;
		SDL_Delay ( 1 );
	}
	return players;
}

void ShowPlayerStates ( sPlayer players )
{
	SDL_Rect dest,norm1,norm2;
	dest.w = norm1.w = norm2.w = 55;
	dest.h = norm1.h = norm2.h = 71;
	dest.x = 394;
	dest.y = 67;
	for ( int i = 0; i<4;i++ )
	{
		dest.y = norm1.y = norm2.y = 67 + 92*i;
		// Nichts
		if ( players.what[i] == 0 )
		{
			dest.x = 394;
			SDL_BlitSurface ( GraphicsData.gfx_player_none,NULL,buffer,&dest );
			norm1.x = 394 - 110;
			norm2.x = 394 - 219;
		}
		// Spieler
		if ( players.what[i] == 1 )
		{
			norm1.x = 394 - 110;
			norm2.x = 394;
			dest.x = 394 - 219;
			SDL_BlitSurface ( GraphicsData.gfx_player_human,NULL,buffer,&dest );
		}
		// Computer
		if ( players.what[i] == 2 )
		{
			norm1.x = 394;
			norm2.x = 394 - 219;
			dest.x = 394 - 110;
			SDL_BlitSurface ( GraphicsData.gfx_player_pc,NULL,buffer,&dest );
		}
		SDL_BlitSurface ( GraphicsData.gfx_player_select,&norm1,buffer,&norm1 );
		SDL_BlitSurface ( GraphicsData.gfx_player_select,&norm2,buffer,&norm2 );
	}
}

// Zeigt den Hngar an:
void ShowBars ( int credits,int StartCredits,TList *landing,int selected, SDL_Surface *surface );
void MakeUpgradeSliderVehicle ( sUpgrades *u,int nr,cPlayer *p );
void MakeUpgradeSliderBuilding ( sUpgrades *u,int nr,cPlayer *p );
int CalcSteigerung ( int org, int variety );
int CalcPrice ( int value,int org, int variety );
void MakeUpgradeSubButtons ( bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf, SDL_Surface *surface );
void ShowSelectionList ( TList *list,int selected,int offset,bool beschreibung,int credits,cPlayer *p );

void RunHangar ( cPlayer *player,TList *LandingList )
{
	bool tank=true,plane=false,ship=false,build=false,tnt=false,kauf=true;
	bool FertigPressed=false,Beschreibung=SettingsData.bShowDescription;
	bool DownPressed=false,UpPressed=false,KaufPressed=false;
	bool Down2Pressed=false,Up2Pressed=false,EntfernenPressed=false;
	bool LadungUpPressed=false,LadungDownPressed=false;
	int b,lb=0,i,selected=0,offset=0,x,y,StartCredits=player->Credits,lx=-1,ly=-1;
	int LandingOffset=0,LandingSelected=0;
	SDL_Rect scr;
	TList *list,*selection;
	
	#define BUTTON_W 77
	#define BUTTON_H 23	
	
	SDL_Rect rBtnDone = {447, 452, BUTTON_W, BUTTON_H};
	SDL_Rect rBtnBuy = {561, 388, BUTTON_W, BUTTON_H};
	SDL_Rect rBtnDel = {388, 240, BUTTON_W, BUTTON_H};
	SDL_Rect rTxtDescription = {141,266,150,13};

	SDL_Rect dest = { DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H};
	SDL_Surface *sfTmp;
	
	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_HANGAR,sfTmp );
	
 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);

	SDL_BlitSurface (sfTmp, NULL, buffer, NULL); //FIXME: use dest and make this working > 640x480

	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Done"), false, rBtnDone.x, rBtnDone.y, buffer);
	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Buy"), false, rBtnBuy.x, rBtnBuy.y, buffer);
	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Delete"), false, rBtnDel.x, rBtnDel.y, buffer);
	fonts->OutTextCenter(lngPack.Translate( "Text~Comp~Description" ).c_str(), rTxtDescription.x+rTxtDescription.w/2, rTxtDescription.y, buffer);
	
	//blit sfTmp to buffer



	// Die Liste erstellen:
	list=new TList;
	for ( i=0;i<UnitsData.vehicle_anz;i++ )
	{
		sHUp *n;
		SDL_Surface *sf;
		ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[0],UnitsData.vehicle[i].img[0],UnitsData.vehicle[i].img_org[0]->w/2,UnitsData.vehicle[i].img_org[0]->h/2 );
		sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,UnitsData.vehicle[i].img[0]->w,UnitsData.vehicle[i].img[0]->h,32,0,0,0,0 );
		SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
		SDL_BlitSurface ( OtherData.colors[cl_grey],NULL,sf,NULL );
		SDL_BlitSurface ( UnitsData.vehicle[i].img[0],NULL,sf,NULL );
		ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[0],UnitsData.vehicle[i].img[0],UnitsData.vehicle[i].img_org[0]->w,UnitsData.vehicle[i].img_org[0]->h );
		n=new sHUp;
		n->sf=sf;
		n->id=i;
		n->costs=UnitsData.vehicle[i].data.costs;
		n->vehicle=true;
		MakeUpgradeSliderVehicle ( n->upgrades,i,player );
		list->AddHUp ( n );
	}
	for ( i=0;i<UnitsData.building_anz;i++ )
	{
		sHUp *n;
		SDL_Surface *sf;
		if ( UnitsData.building[i].data.is_big )
		{
			ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img,UnitsData.building[i].img_org->w/4,UnitsData.building[i].img_org->h/4 );
		}
		else
		{
			ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img,UnitsData.building[i].img_org->w/2,UnitsData.building[i].img_org->h/2 );
		}
		sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,UnitsData.building[i].img->w,UnitsData.building[i].img->h,32,0,0,0,0 );
		SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
		if ( !UnitsData.building[i].data.is_connector&&!UnitsData.building[i].data.is_road )
		{
			SDL_BlitSurface ( OtherData.colors[cl_grey],NULL,sf,NULL );
		}
		else
		{
			SDL_FillRect ( sf,NULL,0xFF00FF );
		}
		SDL_BlitSurface ( UnitsData.building[i].img,NULL,sf,NULL );
		ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img,UnitsData.building[i].img_org->w,UnitsData.building[i].img_org->h );
		n=new sHUp;
		n->sf=sf;
		n->id=i;
		n->costs=UnitsData.building[i].data.costs;
		n->vehicle=false;
		MakeUpgradeSliderBuilding ( n->upgrades,i,player );
		list->AddHUp ( n );
	}
	// Die Selection erstellen:
	selection=new TList;
	CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );
	ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
	MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );

	// Die Landeliste anzeigen:
	ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );

	// Die Bars anzeigen:
	ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );

	SHOW_SCREEN
	mouse->draw ( false,screen );

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Die Maus machen:
		mouse->GetPos();
		b = mouse->GetMouseButton();
		x = mouse->x;
		y = mouse->y;

		if ( x != lx || y != ly )
		{
			mouse->draw ( true,screen );
		}

		// Fertig:
		if ( x >= rBtnDone.x && x < rBtnDone.x + rBtnDone.w && y >= rBtnDone.y && y < rBtnDone.y + rBtnDone.h )
		{
			if ( b&&!lb )
			{
				FertigPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawButton(lngPack.Translate( "Text~Menu_Main~Button_Done"), true, rBtnDone.x, rBtnDone.y, buffer);
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&FertigPressed )
			{
				break;
			}
		}
		else if ( FertigPressed )
		{
			FertigPressed=false;
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Done"), false, rBtnDone.x, rBtnDone.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Beschreibung Haken:
		if ( x>=292&&x<292+16&&y>=265&&y<265+15&&b&&!lb )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Beschreibung=!Beschreibung;
			SettingsData.bShowDescription=Beschreibung;
			if ( Beschreibung )
			{
				dest.x=scr.x=291;
				dest.y=scr.y=264;
				dest.w=scr.w=17;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			}
			else
			{
				scr.x=393;
				scr.y=46;
				dest.x=291;
				dest.y=264;
				dest.w=scr.w=18;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			}
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Down-Button:
		if ( x>=491&&x<491+18&&y>=386&&y<386+17&&b&&!DownPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=491;
			dest.y=386;
			if ( offset<selection->Count-9 )
			{
				offset++;
				if ( selected<offset ) selected=offset;
				ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=true;
		}
		else if ( !b&&DownPressed )
		{
			scr.x=491;
			scr.y=386;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=491;
			dest.y=386;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=false;
		}
		// Up-Button:
		if ( x>=470&&x<470+18&&y>=386&&y<386+17&&b&&!UpPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=470;
			dest.y=386;
			if ( offset!=0 )
			{
				offset--;
				if ( selected>=offset+9 ) selected=offset+8;
				ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=true;
		}
		else if ( !b&&UpPressed )
		{
			scr.x=470;
			scr.y=386;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=470;
			dest.y=386;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=false;
		}
		// Klick in die Liste:
		if ( x>=490&&x<490+70&&y>=60&&y<60+315&&b&&!lb )
		{
			int nr;
			nr= ( y-60 ) / ( 32+2 );
			if ( selection->Count<9 )
			{
				if ( nr>=selection->Count ) nr=-1;
			}
			else
			{
				if ( nr>=10 ) nr=-1;
				nr+=offset;
			}
			if ( nr!=-1 )
			{
				int last_selected=selected;
				PlayFX ( SoundData.SNDObjectMenu );
				selected=nr;
				ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
				// Doppelklick prüfen:
				if ( last_selected==nr&&selection->HUpItems[selected]->costs<=player->Credits )
				{
					// Don't add buildings, humans, planes, etc...
					if ( selection->HUpItems[selected]->vehicle &&
						!UnitsData.vehicle[selection->HUpItems[selected]->id].data.is_human &&
						!UnitsData.vehicle[selection->HUpItems[selected]->id].data.is_alien &&
						!(UnitsData.vehicle[selection->HUpItems[selected]->id].data.can_drive == DRIVE_AIR) &&
						!(UnitsData.vehicle[selection->HUpItems[selected]->id].data.can_drive == DRIVE_SEA) )
					{
						sLanding *n;
						n=new sLanding;
						n->cargo=0;
						n->sf=selection->HUpItems[selected]->sf;
						n->id=selection->HUpItems[selected]->id;
						n->costs=selection->HUpItems[selected]->costs;
						LandingList->AddLanding ( n );
						LandingSelected=LandingList->Count-1;
						while ( LandingSelected>=LandingOffset+5 ) LandingOffset++;

						if ( LandingSelected<0 ) LandingSelected=0;
						ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
						player->Credits-=selection->HUpItems[selected]->costs;
						ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
						ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
					}
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Klick auf einen Upgrade-Slider:
		if ( b&&!lb&&x>=283&&x<301+18&&selection->Count )
		{
			sHUp *ptr=selection->HUpItems[selected];
			for ( i=0;i<8;i++ )
			{
				if ( !ptr->upgrades[i].active ) continue;
				if ( ptr->upgrades[i].Purchased&&x<283+18&&y>=293+i*19&&y<293+i*19+19 )
				{
					int variety;
					if ( ptr->upgrades[i].name.compare( lngPack.Translate ( "Text~Vehicles~Title_Hitpoints" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Armor" ) ) == 0 || ptr->upgrades[i].name.compare( lngPack.Translate ( "Text~Vehicles~Title_Ammo" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Damage" ) ) == 0 )
						variety = 0;
					else if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Speed" ) ) == 0 )
						variety = 1;
					else if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Shots" ) ) == 0 )
						variety = 2;
					else if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Range" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Scan" ) ) == 0 )
						variety = 3;
					else variety = -1;
					* ( ptr->upgrades[i].value )-=CalcSteigerung ( ptr->upgrades[i].StartValue,variety );
					// double price for damage
					if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Damage" ) ) == 0 )
					{
						ptr->upgrades[i].NextPrice = 2*CalcPrice ( * ( ptr->upgrades[i].value ),ptr->upgrades[i].StartValue,variety );
					}
					else
					{
						ptr->upgrades[i].NextPrice = CalcPrice ( * ( ptr->upgrades[i].value ),ptr->upgrades[i].StartValue,variety );
					}
					player->Credits+=ptr->upgrades[i].NextPrice;
					ptr->upgrades[i].Purchased--;

					PlayFX ( SoundData.SNDObjectMenu );
					ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
					ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					break;
				}
				else if ( ptr->upgrades[i].NextPrice<=player->Credits&&x>=301&&y>=293+i*19&&y<293+i*19+19 )
				{
					int variety;
					player->Credits-=ptr->upgrades[i].NextPrice;
					if ( ptr->upgrades[i].name.compare( lngPack.Translate ( "Text~Vehicles~Title_Hitpoints" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Armor" ) ) == 0 || ptr->upgrades[i].name.compare( lngPack.Translate ( "Text~Vehicles~Title_Ammo" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Damage" ) ) == 0 )
						variety = 0;
					else if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Speed" ) ) == 0 )
						variety = 1;
					else if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Shots" ) ) == 0 )
						variety = 2;
					else if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Range" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Scan" ) ) == 0 )
						variety = 3;
					else variety = -1;
					* ( ptr->upgrades[i].value ) +=CalcSteigerung ( ptr->upgrades[i].StartValue,variety );
					// double price for damage
					if ( ptr->upgrades[i].name.compare ( lngPack.Translate ( "Text~Vehicles~Title_Damage" ) ) == 0 )
					{
						ptr->upgrades[i].NextPrice = 2*CalcPrice ( * ( ptr->upgrades[i].value ),ptr->upgrades[i].StartValue,variety );
					}
					else
					{
						ptr->upgrades[i].NextPrice = CalcPrice ( * ( ptr->upgrades[i].value ),ptr->upgrades[i].StartValue,variety );
					}
					ptr->upgrades[i].Purchased++;

					PlayFX ( SoundData.SNDObjectMenu );
					ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
					ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					break;
				}
			}
		}

		// Klick auf einen der SubSelctionButtons:
		if ( b&&!lb&&x>=467&&x<467+32&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			tank=!tank;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=467+33&&x<467+32+33&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			plane=!plane;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=467+33*2&&x<467+32+33*2&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			ship=!ship;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=467+33*3&&x<467+32+33*3&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			build=!build;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=467+33*4&&x<467+32+33*4&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			tnt=!tnt;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=542&&x<542+16&&y>=459 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			kauf=false;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=542&&x<542+16&&y>=445 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			kauf=true;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Kauf-Button:
		if ( x >= rBtnBuy.x && x < rBtnBuy.x + rBtnBuy.w && y >= rBtnBuy.y && y < rBtnBuy.y + rBtnBuy.h && b && !KaufPressed && selection->HUpItems[selected]->costs <= player->Credits && kauf )
		{
			PlayFX ( SoundData.SNDMenuButton );

			sLanding *n;
			n=new sLanding;
			n->cargo=0;
			n->sf=selection->HUpItems[selected]->sf;
			n->id=selection->HUpItems[selected]->id;
			n->costs=selection->HUpItems[selected]->costs;
			LandingList->AddLanding ( n );
			LandingSelected=LandingList->Count-1;
			while ( LandingSelected>=LandingOffset+5 )
			{
				LandingOffset++;
			}
			ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			player->Credits-=n->costs;
			ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Buy"), true, rBtnBuy.x, rBtnBuy.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			KaufPressed=true;
		}
		else if ( !b&&KaufPressed )
		{
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Buy"), false, rBtnBuy.x, rBtnBuy.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			KaufPressed=false;
		}
		// Down2-Button:
		if ( x>=327&&x<327+18&&y>=240&&y<240+17&&b&&!Down2Pressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=327;
			dest.y=240;
			if ( LandingOffset!=0 )
			{
				LandingOffset--;
				if ( LandingSelected>=LandingOffset+5 ) LandingSelected=LandingOffset+4;
				ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Down2Pressed=true;
		}
		else if ( !b&&Down2Pressed )
		{
			scr.x=327;
			scr.y=240;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=327;
			dest.y=240;
			SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Down2Pressed=false;
		}
		// Up2-Button:
		if ( x>=347&&x<347+18&&y>=240&&y<240+17&&b&&!Up2Pressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=347;
			dest.y=240;
			if ( LandingOffset<LandingList->Count-5 )
			{
				LandingOffset++;
				if ( LandingSelected<LandingOffset ) LandingSelected=LandingOffset;
				ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Up2Pressed=true;
		}
		else if ( !b&&Up2Pressed )
		{
			scr.x=347;
			scr.y=240;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=347;
			dest.y=240;
			SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Up2Pressed=false;
		}
		// Entfernen-Button:
		if ( x >= rBtnDel.x && x < rBtnDel.x + rBtnDel.w && y >= rBtnDel.y && y < rBtnDel.y + rBtnDel.h && b && !EntfernenPressed )
		{
			PlayFX ( SoundData.SNDMenuButton );
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Delete"), true, rBtnDel.x, rBtnDel.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			EntfernenPressed=true;
		}
		else if ( EntfernenPressed&&!b&&lb )
		{
			// Vehicle aus der Liste entfernen:
			if ( LandingList->Count&&LandingList->Count>LandingSelected&&LandingSelected>=0 )
			{
				player->Credits+=LandingList->LandItems[LandingSelected]->costs;
				player->Credits+=LandingList->LandItems[LandingSelected]->cargo/5;

				delete LandingList->LandItems[LandingSelected];
				LandingList->DeleteLanding ( LandingSelected );
				ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
				ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

				if ( LandingSelected>=LandingList->Count )
				{
					LandingSelected--;
				}
				if ( LandingList->Count-LandingOffset<5&&LandingOffset>0 )
				{
					LandingOffset--;
				}
				if ( LandingSelected<0 ) LandingSelected=0;
				ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			}

			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Delete"), false, rBtnDel.x, rBtnDel.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			EntfernenPressed=false;
		}
		// Klick in die LandingListe:
		if ( x>=330&&x<330+128&&y>=22&&y<22+210&&b&&!lb )
		{
			int nr;
			nr= ( y-22 ) / ( 32+10 );
			if ( LandingList->Count<5 )
			{
				if ( nr>=LandingList->Count ) nr=-1;
			}
			else
			{
				if ( nr>=10 ) nr=-1;
				nr+=LandingOffset;
			}
			if ( nr!=-1 )
			{
				int last_selected=LandingSelected;
				PlayFX ( SoundData.SNDObjectMenu );
				LandingSelected=nr;
				ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
				ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
				// Doppelklick prüfen:
				if ( last_selected==nr )
				{
					if ( LandingList->Count&&LandingList->Count>LandingSelected&&LandingSelected>=0 )
					{
						player->Credits+=LandingList->LandItems[LandingSelected]->costs;
						player->Credits+=LandingList->LandItems[LandingSelected]->cargo/5;
						delete LandingList->LandItems[LandingSelected];
						LandingList->DeleteLanding ( LandingSelected );
						ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
						ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

						if ( LandingSelected>=LandingList->Count )
						{
							LandingSelected--;
						}
						if ( LandingList->Count-LandingOffset<5&&LandingOffset>0 )
						{
							LandingOffset--;
						}
						if ( LandingSelected<0 ) LandingSelected=0;
						ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp );
					}
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}

		if ( LandingSelected>=0&&LandingList->Count&&LandingSelected<LandingList->Count )
		{
			sLanding *ptr;
			ptr=LandingList->LandItems[LandingSelected];
			if ( UnitsData.vehicle[ptr->id].data.can_transport==TRANS_METAL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_OIL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_GOLD )
			{

				// LadungUp-Button:
				if ( x>=413&&x<413+18&&y>=424&&y<424+17&&b&&!LadungDownPressed&&ptr->cargo<UnitsData.vehicle[ptr->id].data.max_cargo&&player->Credits>0 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					scr.x=249;
					scr.y=151;
					dest.w=scr.w=18;
					dest.h=scr.h=17;
					dest.x=413;
					dest.y=424;

					ptr->cargo+=5;
					player->Credits--;
					ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
					ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp );

					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					LadungDownPressed=true;
				}
				else if ( !b&&LadungDownPressed )
				{
					scr.x=413;
					scr.y=424;
					dest.w=scr.w=18;
					dest.h=scr.h=17;
					dest.x=413;
					dest.y=424;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					LadungDownPressed=false;
				}
				// LadungDown-Button:
				if ( x>=433&&x<433+18&&y>=424&&y<424+17&&b&&!LadungUpPressed&&ptr->cargo>0 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					scr.x=230;
					scr.y=151;
					dest.w=scr.w=18;
					dest.h=scr.h=17;
					dest.x=433;
					dest.y=424;

					ptr->cargo-=5;
					player->Credits++;
					ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
					ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp );

					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					LadungUpPressed=true;
				}
				else if ( !b&&LadungUpPressed )
				{
					scr.x=433;
					scr.y=424;
					dest.w=scr.w=18;
					dest.h=scr.h=17;
					dest.x=433;
					dest.y=424;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					LadungUpPressed=false;
				}
				// Klick auf den Ladungsbalken:
				if ( b&&!lb&&x>=422&&x<422+20&&y>=301&&y<301+115 )
				{
					int value;
					value= ( ( ( int ) ( ( 115- ( y-301 ) ) * ( UnitsData.vehicle[ptr->id].data.max_cargo/115.0 ) ) ) /5 ) *5;
					PlayFX ( SoundData.SNDObjectMenu );

					if ( ( 115- ( y-301 ) ) >=110 ) value=UnitsData.vehicle[ptr->id].data.max_cargo;

					if ( value<ptr->cargo )
					{
						player->Credits+= ( ptr->cargo-value ) /5;
						ptr->cargo=value;
					}
					else if ( value>ptr->cargo&&player->Credits>0 )
					{
						value-=ptr->cargo;
						while ( value>0&&player->Credits>0&&ptr->cargo<UnitsData.vehicle[ptr->id].data.max_cargo )
						{
							ptr->cargo+=5;
							player->Credits--;
							value-=5;
						}
						if ( ptr->cargo>UnitsData.vehicle[ptr->id].data.max_cargo )
						{
							ptr->cargo=UnitsData.vehicle[ptr->id].data.max_cargo;
						}
					}

					ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
					ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
			}
		}

		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}

	while ( list->Count )
	{
		sHUp *ptr;
		ptr = list->HUpItems[0];
		for ( i=0;i<8;i++ )
		{
			if ( ptr->upgrades[i].active&&ptr->upgrades[i].Purchased )
			{
				if ( ptr->vehicle )
				{
					player->VehicleData[ptr->id].version++;
				}
				else
				{
					player->BuildingData[ptr->id].version++;
				}
				break;
			}
		}
		SDL_FreeSurface ( ptr->sf );
		delete ptr;
		list->DeleteHUp ( 0 );
	}
	delete list;
	delete selection;
	SDL_FreeSurface(sfTmp);
	
}

// Macht die Upgradeschieber für Vehicle:
void MakeUpgradeSliderVehicle ( sUpgrades *u,int nr,cPlayer *p )
{
	sUnitData *d;
	int i;
	for ( i=0;i<8;i++ )
	{
		u[i].active=false;
		u[i].Purchased=0;
		u[i].value=NULL;
	}
	d=p->VehicleData+nr;
	i=0;

	if ( d->can_attack )
	{
		// Damage:
		u[i].active=true;
		u[i].value=& ( d->damage );
		u[i].NextPrice=2*CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.damage, 0 );
		u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Damage" );
		i++;
		// Shots:
		u[i].active=true;
		u[i].value=& ( d->max_shots );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_shots, 2 );
		u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Shoots" );
		i++;
		// Range:
		u[i].active=true;
		u[i].value=& ( d->range );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.range, 3 );
		u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Range" );
		i++;
		// Ammo:
		u[i].active=true;
		u[i].value=& ( d->max_ammo );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_ammo, 0 );
		u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Ammo" );
		i++;
	}
	if ( d->can_transport==TRANS_METAL||d->can_transport==TRANS_OIL||d->can_transport==TRANS_GOLD )
	{
		i++;
	}
	// Armor:
	u[i].active=true;
	u[i].value=& ( d->armor );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.armor, 0 );
	u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Armor" );
	i++;
	// Hitpoints:
	u[i].active=true;
	u[i].value=& ( d->max_hit_points );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_hit_points, 0 );
	u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Hitpoints" );
	i++;
	// Scan:
	u[i].active=true;
	u[i].value=& ( d->scan );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.scan, 3 );
	u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Scan" );
	i++;
	// Speed:
	u[i].active=true;
	u[i].value=& ( d->max_speed );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_speed, 1 );
	u[i].name = lngPack.Translate ( "Text~Vehicles~Title_Speed" );
	i++;
	// Costs:
	i++;

	for ( i=0;i<8;i++ )
	{
		if ( u[i].value==NULL ) continue;
		u[i].StartValue=* ( u[i].value );
	}
}

// Macht die Upgradeschieber für Buildings:
void MakeUpgradeSliderBuilding ( sUpgrades *u,int nr,cPlayer *p )
{
	sUnitData *d;
	int i;
	for ( i=0;i<8;i++ )
	{
		u[i].active=false;
		u[i].Purchased=0;
		u[i].value=NULL;
	}
	d=p->BuildingData+nr;
	i=0;

	if ( d->can_attack )
	{
		// Damage:
		u[i].active=true;
		u[i].value=& ( d->damage );
		u[i].NextPrice=2*CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.damage, 0 );
		u[i].name = "damage";
		i++;
		if ( !d->is_expl_mine )
		{
			// Shots:
			u[i].active=true;
			u[i].value=& ( d->max_shots );
			u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.max_shots, 2 );
			u[i].name = "shots";
			i++;
			// Range:
			u[i].active=true;
			u[i].value=& ( d->range );
			u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.range, 3 );
			u[i].name = "range";
			i++;
			// Ammo:
			u[i].active=true;
			u[i].value=& ( d->max_ammo );
			u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.max_ammo, 0 );
			u[i].name = "ammo";
			i++;
		}
	}
	if ( d->max_shield )
	{
		// Range:
		u[i].active=true;
		u[i].value=& ( d->range );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.range, 3 );
		u[i].name = "range";
		i++;
	}
	if ( d->can_load==TRANS_METAL||d->can_load==TRANS_OIL||d->can_load==TRANS_GOLD )
	{
		i++;
	}
	if ( d->energy_prod )
	{
		i+=2;
	}
	if ( d->human_prod )
	{
		i++;
	}
	// Armor:
	if ( d->armor!=1 )
	{
		u[i].active=true;
		u[i].value=& ( d->armor );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.armor, 0 );
		u[i].name = "armor";
	}
	i++;
	// Hitpoints:
	u[i].active=true;
	u[i].value=& ( d->max_hit_points );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.max_hit_points, 0 );
	u[i].name = "hitpoints";
	i++;
	// Scan:
	if ( d->scan && d->scan!=1 )
	{
		u[i].active=true;
		u[i].value=& ( d->scan );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.scan, 3 );
		u[i].name = "scan";
		i++;
	}
	// Energieverbrauch:
	if ( d->energy_need )
	{
		i++;
	}
	// Humanverbrauch:
	if ( d->human_need )
	{
		i++;
	}
	// Metallverbrauch:
	if ( d->metal_need )
	{
		i++;
	}
	// Goldverbrauch:
	if ( d->gold_need )
	{
		i++;
	}
	// Costs:
	i++;

	for ( i=0;i<8;i++ )
	{
		if ( u[i].value==NULL ) continue;
		u[i].StartValue=* ( u[i].value );
	}
}

// Berechnet den Preis für ein Upgrade:
int CalcPrice ( int value,int org, int variety )
{
	int tmp;
	double a, b, c;
	switch ( variety )
	{
			// Treffer, Panzerung, Munition & Angriff
		case 0:
			switch ( org )
			{
				case 2:
					if ( value==2 ) return 39;
					if ( value==3 ) return 321;
					break;
				case 4:
					a=0.0016091639;
					b=-0.073815318;
					c=6.0672869;
					break;
				case 6:
					a=0.000034548596;
					b=-0.27217472;
					c=6.3695123;
					break;
				case 8:
					a=0.00037219059;
					b=2.5148748;
					c=5.0938608;
					break;
				case 9:
					a=0.000059941694;
					b=1.3962889;
					c=4.6045196;
					break;
				case 10:
					a=0.000033736018;
					b=1.4674423;
					c=5.5606209;
					break;
				case 12:
					a=0.0000011574058;
					b=0.23439586;
					c=6.113616;
					break;
				case 14:
					a=0.0000012483447;
					b=1.4562373;
					c=5.8250952;
					break;
				case 15:
					a=0.00000018548742;
					b=-0.33519669;
					c=6.3333527;
					break;
				case 16:
					a=0.000010898263;
					b=5.0297434;
					c=5.0938627;
					break;
				case 18:
					a=0.00000017182818;
					b=2.0009536;
					c=5.8937153;
					break;
				case 20:
					a=0.00000004065782;
					b=1.6533066;
					c=6.0601538;
					break;
				case 22:
					a=0.0000000076942857;
					b=-0.45461813;
					c=6.4148588;
					break;
				case 24:
					a=0.00000076484313;
					b=8.0505377;
					c=5.1465019;
					break;
				case 28:
					a=0.00000015199858;
					b=5.1528048;
					c=5.4700225;
					break;
				case 32:
					a=0.00000030797077;
					b=8.8830596;
					c=5.1409486;
					break;
				case 56:
					a=0.000000004477053;
					b=11.454622;
					c=5.4335099;
					break;
				default:
					return 0;
					break;
			}
			break;
			// Geschwindgigkeit
		case 1:
			org=org/2;
			value=value/2;
			switch ( org )
			{
				case 5:
					a=0.00040716128;
					b=-0.16662054;
					c=6.2234362;
					break;
				case 6:
					a=0.00038548127;
					b=0.48236948;
					c=5.827724;
					break;
				case 7:
					a=0.000019798772;
					b=-0.31204765;
					c=6.3982628;
					break;
				case 9:
					a=0.0000030681294;
					b=-0.25372812;
					c=6.3995668;
					break;
				case 10:
					a=0.0000062019158;
					b=-0.23774407;
					c=6.1901333;
					break;
				case 12:
					a=0.0000064901101;
					b=0.93320705;
					c=5.8395847;
					break;
				case 14:
					a=0.0000062601892;
					b=2.1588132;
					c=5.5866699;
					break;
				case 15:
					a=0.00000027748628;
					b=-0.0031671959;
					c=6.2349744;
					break;
				case 16:
					a=0.0000011401659;
					b=1.8660343;
					c=5.7884287;
					break;
				case 18:
					a=0.00000093928003;
					b=2.9224069;
					c=5.6503159;
					break;
				case 20:
					a=0.00000003478867;
					b=0.44735558;
					c=6.2388156;
					break;
				case 24:
					a=0.0000000038623391;
					b=-0.4486039;
					c=6.4245686;
					break;
				case 28:
					a=0.000000039660207;
					b=1.6425505;
					c=5.8842817;
					break;
				default:
					return 0;
					break;
			}
			break;
			// Schüsse
		case 2:
			switch ( org )
			{
				case 1:
					return 720;
					break;
				case 2:
					if ( value==2 ) return 79;
					if ( value==3 ) return 641;
					break;
				default:
					return 0;
					break;
			}
			break;
			// Reichweite, Scan
		case 3:
			switch ( org )
			{
				case 3:
					if ( value==3 ) return 61;
					if ( value==4 ) return 299;
					break;
				case 4:
					a=0.010226741;
					b=-0.001141961;
					c=5.8477272;
					break;
				case 5:
					a=0.00074684696;
					b=-0.24064936;
					c=6.2377712;
					break;
				case 6:
					a=0.0000004205569;
					b=-2.5074874;
					c=8.1868728;
					break;
				case 7:
					a=0.00018753949;
					b=0.42735532;
					c=5.9259322;
					break;
				case 8:
					a=0.000026278484;
					b=0.0026600724;
					c=6.2281618;
					break;
				case 9:
					a=0.000017724816;
					b=0.35087138;
					c=6.1028354;
					break;
				case 10:
					a=0.000011074461;
					b=-0.41358078;
					c=6.2067919;
					break;
				case 11:
					a=0.0000022011968;
					b=-0.97456761;
					c=6.4502985;
					break;
				case 12:
					a=0.0000000034515189;
					b=-4.4597674;
					c=7.9715326;
					break;
				case 14:
					a=0.0000028257552;
					b=0.78730358;
					c=5.9483863;
					break;
				case 18:
					a=0.00000024289322;
					b=0.64536566;
					c=6.11706;
					break;
				default:
					return 0;
					break;
			}
			break;
		default:
			return 0;
	}

	tmp= ( int ) ( Round ( ( a*pow ( ( value-b ),c ) ), 0 ) );
	return tmp;
}

// Berechnet die Steigerung bei eim Upgrade:
int CalcSteigerung ( int org, int variety )
{
	int tmp = 0;
	switch ( variety )
	{
		case 0:
		{
			if ( org == 2 || org == 4 || org == 6 || org == 8 )
				tmp = 1;
			if ( org == 9 || org == 10 || org == 12 || org == 14 || org == 15 || org == 16 || org == 18 || org == 20 || org == 22 || org == 24 )
				tmp = 2;
			if ( org == 28 || org == 32 )
				tmp = 5;
			if ( org == 56 )
				tmp = 10;
			break;
		}
		case 1:
		{
			org=org/2;
			if ( org == 5 || org == 6 || org == 7 || org == 9 )
				tmp = 1;
			if ( org == 10 || org == 12 || org ==14 || org == 15 || org == 16 || org == 18 || org == 20 )
				tmp = 2;
			if ( org == 28 )
				tmp = 5;
			tmp=tmp*2;
			break;
		}
		case 2:
			tmp = 1;
			break;
		case 3:
		{
			if ( org == 3 || org ==4 || org == 5 || org == 6 || org == 7 || org == 8 || org == 9 )
				tmp = 1;
			if ( org == 10 || org ==11 || org == 12 || org == 14 || org == 18 )
				tmp = 2;
			break;
		}
		default:
			tmp = 1;
			break;
	}
	return tmp;
}

// Malt die SubButtons im Upgradefenster:
void MakeUpgradeSubButtons ( bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf, SDL_Surface *surface )
{
	SDL_Rect scr,dest;
	scr.x=152;scr.y=479;
	dest.x=467;dest.y=411;
	dest.w=scr.w=32;dest.h=scr.h=31;
	// Tank:
	if ( !tank )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// Plane:
	if ( !plane )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// Ship:
	if ( !ship )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// Building:
	if ( !build )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// TNT:
	if ( !tnt )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	// Kauf:
	scr.x=54;scr.y=352;
	scr.w=scr.h=dest.w=dest.h=16;
	dest.x=542;
	dest.y=446;
	if ( !kauf )
	{
		SDL_BlitSurface ( surface,&dest,buffer,&dest );
		dest.y=462;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		dest.y=462;
		SDL_BlitSurface ( surface,&dest,buffer,&dest );
	}
}

// Zeigt die Bars an:
void ShowBars ( int credits,int StartCredits,TList *landing,int selected, SDL_Surface *surface )
{
	char str[50];
	SDL_Rect scr,dest;
	scr.x=dest.x=371;
	scr.y=dest.y=301;
	scr.w=dest.w=22;
	scr.h=dest.h=115;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );
	scr.x=dest.x=312;
	scr.y=dest.y=265;
	scr.w=dest.w=150;
	scr.h=dest.h=30;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );
	sprintf ( str,"%d",credits );
	fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Options~Title_Gold" ).c_str() ,381,275,buffer );
	fonts->OutTextCenter ( str,381,275+10,buffer );

	scr.x=118;
	scr.y=336;
	scr.w=dest.w=16;
	scr.h=dest.h= ( int ) ( 115 * ( credits / ( float ) StartCredits ) );
	dest.x=375;
	dest.y=301+115-dest.h;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );

	scr.x=dest.x=422;
	scr.y=dest.y=301;
	scr.w=dest.w=20;
	scr.h=dest.h=115;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );

	if ( selected>=0&&landing->Count&&selected<landing->Count )
	{
		sLanding *ptr;
		ptr=landing->LandItems[selected];
		if ( UnitsData.vehicle[ptr->id].data.can_transport==TRANS_METAL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_OIL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_GOLD )
		{
			sprintf ( str,"%d",ptr->cargo );
			fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Cargo" ).c_str(),430,275,buffer );
			fonts->OutTextCenter ( str,430,275+10,buffer );

			scr.x=133;
			scr.y=336;
			scr.w=dest.w=20;
			scr.h=dest.h= ( int ) ( 115 * ( ptr->cargo / ( float ) UnitsData.vehicle[ptr->id].data.max_cargo ) );
			dest.x=422;
			dest.y=301+115-dest.h;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}
	}
}

// Liefert die Kachel, die auf der großen Karte unter den Koordinaten liegt:
int GetKachelBig ( int x,int y, cMap *map )
{
	double fak;
	int nr;
	if ( x<0||x>=SettingsData.iScreenW-192||y<0||y>=SettingsData.iScreenH-32 ) return 0;

	x= ( int ) ( x* ( 448.0/ ( SettingsData.iScreenW-192 ) ) );
	y= ( int ) ( y* ( 448.0/ ( SettingsData.iScreenH-32 ) ) );

	if ( map->size<448 )
	{
		fak=448.0/map->size;
		x= ( int ) ( x/fak );
		y= ( int ) ( y/fak );
		nr=map->Kacheln[x+y*map->size];
	}
	else
	{
		fak=map->size/448.0;
		x= ( int ) ( x*fak );
		y= ( int ) ( y*fak );
		nr=map->Kacheln[x+y*map->size];
	}
	return nr;
}

// Wählt die Landestelle aus:
void SelectLanding ( int *x,int *y,cMap *map )
{
	SDL_Rect top,bottom;
	int b,lx=-1,ly=-1,i,k,nr,fakx,faky,off;
	sTerrain *t;

	fakx= ( int ) ( ( SettingsData.iScreenW-192.0 ) /game->map->size );
	faky= ( int ) ( ( SettingsData.iScreenH-32.0 ) /game->map->size );

	// Die Karte malen:
	SDL_LockSurface ( buffer );
	for ( i=0;i<SettingsData.iScreenW-192;i++ )
	{
		for ( k=0;k<SettingsData.iScreenH-32;k++ )
		{
			nr=GetKachelBig ( ( i/fakx ) *fakx, ( k/faky ) *faky, map );
			t=TerrainData.terrain+nr;
			off= ( i%fakx ) * ( t->sf_org->h/fakx ) + ( k%faky ) * ( t->sf_org->h/faky ) *t->sf_org->w;
			nr=* ( ( int* ) ( t->sf_org->pixels ) +off );
			if ( nr==0xFF00FF )
			{
				t=TerrainData.terrain+map->DefaultWater;
				off= ( i%fakx ) * ( t->sf_org->h/fakx ) + ( k%faky ) * ( t->sf_org->h/faky ) *t->sf_org->w;
				nr=* ( ( int* ) ( t->sf_org->pixels ) +off );
			}
			( ( int* ) ( buffer->pixels ) ) [i+180+ ( k+18 ) *buffer->w]=nr;
		}
	}
	SDL_UnlockSurface ( buffer );

	// Hud drüber legen:
	game->hud->DoAllHud();
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );


	top.x=0;top.y= ( SettingsData.iScreenH/2 )-479;
	top.w=bottom.w=171;

	top.h=479;bottom.h=481;
	bottom.x=0;bottom.y= ( SettingsData.iScreenH/2 );
	SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&top );
	SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&bottom );

	SHOW_SCREEN

	t=TerrainData.terrain+GetKachelBig ( mouse->x-180,mouse->y-18, map );
	if ( mouse->x>=180&&mouse->x<SettingsData.iScreenW-12&&mouse->y>=18&&mouse->y<SettingsData.iScreenH-14&&! ( t->water||t->coast||t->blocked ) )
	{
		mouse->SetCursor ( CMove );
	}
	else
	{
		mouse->SetCursor ( CNo );
	}
	mouse->draw ( false,screen );

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		t=TerrainData.terrain+GetKachelBig ( mouse->x-180,mouse->y-18, map );
		if ( mouse->x>=180&&mouse->x<SettingsData.iScreenW-12&&mouse->y>=18&&mouse->y<SettingsData.iScreenH-14&&! ( t->water||t->coast||t->blocked ) )
		{
			mouse->SetCursor ( CMove );
		}
		else
		{
			mouse->SetCursor ( CNo );
		}

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
//      SDL_UpdateRect(screen,0,0,0,0);
//      SHOW_SCREEN
		}

		if ( b&&mouse->cur==GraphicsData.gfx_Cmove )
		{
			*x= ( int ) ( ( mouse->x-180 ) / ( 448.0/game->map->size ) * ( 448.0/ ( SettingsData.iScreenW-192 ) ) );
			*y= ( int ) ( ( mouse->y-18 ) / ( 448.0/game->map->size ) * ( 448.0/ ( SettingsData.iScreenH-32 ) ) );
			break;
		}

		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}
}

// Zeigt die Liste mit den ausgewählten Landefahrzeugen an:
void ShowLandingList ( TList *list,int selected,int offset, SDL_Surface *surface )
{
	sLanding *ptr;
	SDL_Rect scr,dest,text;
	char str[100];
	int i,t;
	scr.x=330;scr.y=11;
	scr.w=128;scr.h=233;
	SDL_BlitSurface ( surface,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=340;dest.y=20;
	dest.w=32;dest.h=32;
	text.x=375;text.y=32;
	for ( i=offset;i<list->Count;i++ )
	{
		if ( i>=offset+5 ) break;
		ptr=list->LandItems[i];
		// Das Bild malen:
		SDL_BlitSurface ( ptr->sf,&scr,buffer,&dest );
		// Ggf noch Rahmen drum:
		if ( selected==i )
		{
			SDL_Rect tmp;
			tmp=dest;
			tmp.x-=4;
			tmp.y-=4;
			tmp.h=1;
			tmp.w=8;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x+=30;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y+=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x-=30;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y=dest.y-4;
			tmp.w=1;
			tmp.h=8;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x+=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y+=31;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x-=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
		}
		// Text ausgeben:
		t=0;
		str[0]=0;
		while ( UnitsData.vehicle[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <70 )
		{
			str[t]=UnitsData.vehicle[ptr->id].data.name[t];str[++t]=0;
		}
		str[t]='.';
		str[t+1]=0;
		fonts->OutText ( str,text.x,text.y,buffer );
		if ( UnitsData.vehicle[ptr->id].data.can_transport==TRANS_METAL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_OIL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_GOLD )
		{
			sprintf ( str," (%d/%d)",ptr->cargo,UnitsData.vehicle[ptr->id].data.max_cargo );
			fonts->OutText ( str,text.x,text.y+10,buffer );
		}
		text.y+=32+10;
		dest.y+=32+10;
	}
}

// Stellt die Selectionlist zusammen:
void CreateSelectionList ( TList *selection,TList *images,int *selected,int *offset,bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf )
{
	sUnitData *bd;
	sUnitData *vd;
	int i;
	while ( selection->Count )
	{
		selection->Delete ( 0 );
	}
	if ( kauf )
	{
		plane=false;
		ship=false;
		build=false;
	}
	for ( i=0;i<images->Count;i++ )
	{
		if ( images->HUpItems[i]->vehicle )
		{
			if ( ! ( tank||ship||plane ) ) continue;
			vd=& ( UnitsData.vehicle[images->HUpItems[i]->id].data );
			if ( vd->is_alien&&kauf ) continue;
			if ( vd->is_human&&kauf ) continue;
			if ( tnt&&!vd->can_attack ) continue;
			if ( vd->can_drive==DRIVE_AIR&&!plane ) continue;
			if ( vd->can_drive==DRIVE_SEA&&!ship ) continue;
			if ( ( vd->can_drive==DRIVE_LAND||vd->can_drive==DRIVE_LANDnSEA ) &&!tank ) continue;
			selection->AddHUp ( images->HUpItems[i] );
		}
		else
		{
			if ( !build ) continue;
			bd=& ( UnitsData.building[ ( ( sHUp* ) ( images->HUpItems[i] ) )->id].data );
			if ( tnt&&!bd->can_attack ) continue;
			selection->AddHUp ( images->HUpItems[i] );
		}
	}
	if ( *offset>=selection->Count-9 )
	{
		*offset=selection->Count-9;
		if ( *offset<0 ) *offset=0;
	}
	if ( *selected>=selection->Count )
	{
		*selected=selection->Count-1;
		if ( *selected<0 ) *selected=0;
	}
}

// Zeigt die Liste mit den Images an:
void ShowSelectionList ( TList *list,int selected,int offset,bool beschreibung,int credits, cPlayer *p )
{
	sHUp *ptr;
	SDL_Rect dest,scr,text;
	char str[100];
	int i,t,k;
	scr.x=479;scr.y=52;
	scr.w=150;scr.h=330;
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=490;dest.y=58;
	dest.w=32;dest.h=32;
	text.x=530;text.y=70;
	if ( list->Count==0 )
	{
		scr.x=0;scr.y=0;
		scr.w=316;scr.h=256;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
		scr.x=11;scr.y=290;
		scr.w=346;scr.h=176;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
		return;
	}
	for ( i=offset;i<list->Count;i++ )
	{
		if ( i>=offset+9 ) break;
		// Das Bild malen:
		ptr = list->HUpItems[i];
		SDL_BlitSurface ( ptr->sf,&scr,buffer,&dest );
		// Ggf noch Rahmen drum:
		if ( selected==i )
		{
			SDL_Rect tmp;
			tmp=dest;
			tmp.x-=4;
			tmp.y-=4;
			tmp.h=1;
			tmp.w=8;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x+=30;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y+=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x-=30;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y=dest.y-4;
			tmp.w=1;
			tmp.h=8;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x+=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y+=31;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x-=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			// Das Bild neu malen:
			tmp.x=11;tmp.y=13;
			if ( ptr->vehicle )
			{
				tmp.w=UnitsData.vehicle[ptr->id].info->w;
				tmp.h=UnitsData.vehicle[ptr->id].info->h;
				SDL_BlitSurface ( UnitsData.vehicle[ptr->id].info,NULL,buffer,&tmp );
			}
			else
			{
				tmp.w=UnitsData.building[ptr->id].info->w;
				tmp.h=UnitsData.building[ptr->id].info->h;
				SDL_BlitSurface ( UnitsData.building[ptr->id].info,NULL,buffer,&tmp );
			}
			// Ggf die Beschreibung ausgeben:
			if ( beschreibung )
			{
				tmp.x+=10;tmp.y+=10;
				tmp.w-=20;tmp.h-=20;
				if ( ptr->vehicle )
				{
					fonts->OutTextBlock ( UnitsData.vehicle[ptr->id].text,tmp,buffer );
				}
				else
				{
					fonts->OutTextBlock ( UnitsData.building[ptr->id].text,tmp,buffer );
				}
			}
			// Die Details anzeigen:
			{
				cVehicle *tv;
				cBuilding *tb;
				tmp.x=11;
				tmp.y=290;
				tmp.w=346;
				tmp.h=176;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&tmp,buffer,&tmp );
				if ( ptr->vehicle )
				{
					tv=new cVehicle ( UnitsData.vehicle+ptr->id,p );
					tv->ShowBigDetails();
					delete tv;
				}
				else
				{
					tb=new cBuilding ( UnitsData.building+ptr->id,p,NULL );
					tb->ShowBigDetails();
					delete tb;
				}
			}
			// Die Texte anzeigen/Slider machen:
			for ( k=0;k<8;k++ )
			{
				SDL_Rect scr,dest;
				if ( !ptr->upgrades[k].active ) continue;
				sprintf ( str,"%d",ptr->upgrades[k].NextPrice );
				fonts->OutText ( str,322,296+k*19,buffer );

				if ( ptr->upgrades[k].Purchased )
				{
					scr.x=380;scr.y=256;
					dest.w=scr.w=18;dest.h=scr.h=17;
					dest.x=283;dest.y=293+k*19;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				}
				if ( ptr->upgrades[k].NextPrice<=credits )
				{
					scr.x=399;scr.y=256;
					dest.w=scr.w=18;dest.h=scr.h=17;
					dest.x=301;dest.y=293+k*19;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				}
			}
		}
		// Text ausgeben:
		t=0;
		if ( ptr->vehicle )
		{
			sprintf ( str,"%d",UnitsData.vehicle[ptr->id].data.costs );
			fonts->OutTextCenter ( str,616,text.y,buffer );
			str[0]=0;
			while ( UnitsData.vehicle[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <70 )
			{
				str[t]=UnitsData.vehicle[ptr->id].data.name[t];str[++t]=0;
			}
		}
		else
		{
			str[0]=0;
			while ( UnitsData.building[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <85 )
			{
				str[t]=UnitsData.building[ptr->id].data.name[t];str[++t]=0;
			}
		}
		str[t]='.';
		str[t+1]=0;
		fonts->OutText ( str,text.x,text.y,buffer );
		text.y+=32+2;
		dest.y+=32+2;
	}
}

// Liefert die Numemr der Farbe zurück:
int GetColorNr ( SDL_Surface *sf )
{
	if ( sf==OtherData.colors[cl_red] ) return cl_red;
	if ( sf==OtherData.colors[cl_blue] ) return cl_blue;
	if ( sf==OtherData.colors[cl_green] ) return cl_green;
	if ( sf==OtherData.colors[cl_grey] ) return cl_grey;
	if ( sf==OtherData.colors[cl_orange] ) return cl_orange;
	if ( sf==OtherData.colors[cl_yellow] ) return cl_yellow;
	if ( sf==OtherData.colors[cl_purple] ) return cl_purple;
	if ( sf==OtherData.colors[cl_aqua] ) return cl_aqua;
	return cl_red;
}

cMultiPlayer::cMultiPlayer ( bool host,bool tcp )
{
	NextPlayerID=0;
	PlayerList=new TList;
	PlayerList->AddPlayer ( MyPlayer=new cPlayer ( SettingsData.sPlayerName,OtherData.colors[cl_red],NextPlayerID++ ) );

	ChatList=new TList;
	this->host=host;
	this->tcp=tcp;

	if ( tcp )
	{
		if ( host )
		{
			fstcpip=new cFSTcpIp ( true );
			Titel=lngPack.Translate ( "Text~Menu_Main~Button_TCPIP_Host" );
			IP="-";
		}
		else
		{
			fstcpip=new cFSTcpIp ( false );
			Titel=lngPack.Translate ( "Text~Menu_Main~Button_TCPIP_Client" );
			IP=SettingsData.sIP;
		}
	}
//  fstcpip->FSTcpIpMessageFuntion=ReceiveMenuMessage;
	Port=SettingsData.iPort;

	map="";
	no_options=true;
	WaitForGo=false;
	LetsGo=false;
	game=NULL;
	map_obj=NULL;
}

cMultiPlayer::~cMultiPlayer ( void )
{
	while ( ChatList->Count )
	{
		ChatList->DeleteString ( 0 );
	}
	delete ChatList;
	while ( PlayerList->Count )
	{
		delete PlayerList->PlayerItems[0];
		PlayerList->DeletePlayer ( 0 );
	}
	delete PlayerList;
	if ( game )
	{
		while ( game->PlayerList->Count )
		{
			delete game->PlayerList->PlayerItems[0];
			game->PlayerList->DeletePlayer ( 0 );
		}
		delete game;game=NULL;
	}
	if ( map_obj )
	{
		delete map_obj;map_obj=NULL;
	}
	if ( strcmp ( IP.c_str(),"-" ) ) SettingsData.sIP=IP;
	SettingsData.iPort=Port;
}

// Zeigt das Chatmenü an:
void cMultiPlayer::RunMenu ( void )
{
	bool PlanetPressed=false,OptionsPressed=false,StartHostConnect=false,SendenPressed=false;
	bool OKPressed=false,BackPressed=false,ShowCursor=true,LadenPressed=false;
	int b, lb=0,lx=-1,ly=-1;
	string ChatStr, stmp;
	char sztmp[256];
	SDL_Rect scr;
	Uint8 *keystate;
	unsigned int time;
	int Focus;
	int LastStatus=STAT_CLOSED;
	int LastConnectionCount=0;

#define FOCUS_IP   0
#define FOCUS_PORT 1
#define FOCUS_NAME 2
#define FOCUS_CHAT 3

	SDL_Rect dest = { DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H};
	SDL_Surface *sfTmp;
	
	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_MULT,sfTmp );
	
 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);
	
	//blit sfTmp to buffer
	SDL_BlitSurface (sfTmp, NULL, buffer, NULL); //FIXME: use dest and make this working > 640x480


	Focus=FOCUS_NAME;
	ChatStr="";
	fonts->OutTextCenter ( Titel,320,11,buffer );

	fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_IP" ),20,245,buffer );
	fonts->OutText ( IP,20,260,buffer );
	fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Port" ),228,245,buffer );
	sprintf ( sztmp,"%d",Port );
	fonts->OutText ( sztmp,228,260,buffer );
	fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Player_Name" ),352,245,buffer );
	fonts->OutText ( MyPlayer->name,352,260,buffer );
	fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Color" ),500,245,buffer );
	dest.x=505;dest.y=260;scr.w=dest.w=83;scr.h=dest.h=10;scr.x=0;scr.y=0;
	SDL_BlitSurface ( MyPlayer->color,&scr,buffer,&dest );

	if ( host )
	{
		PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Choose_Planet" ) ,470,42,false );
		PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Options" ) ,470,42+35,false );
		PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Game_Load" ) ,470,42+35*2,false );
		PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Button_Host_Start" ),470,200,false );
		drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,450 );
	}
	else
	{
		PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Connect" ), 470,200,false );
	}
	PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Send" ), 470,416,false );

	drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Back" ), false, 50,450 );

	// Den Focus vorbereiten:
	switch ( Focus )
	{
		case FOCUS_IP:
			InputStr=IP;
			break;
		case FOCUS_PORT:
			InputStr=Port;
			break;
		case FOCUS_NAME:
			InputStr=MyPlayer->name;
			break;
		case FOCUS_CHAT:
			InputStr=ChatStr;
			break;
	}

	mouse->SetCursor ( CHand );
	DisplayGameSettings(sfTmp);
	DisplayPlayerList(sfTmp);
	SHOW_SCREEN
	mouse->draw ( false,screen );
	time=SDL_GetTicks();
	Refresh=false;

	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Tasten prüfen:
		keystate=SDL_GetKeyState ( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		// Den Focus machen:
		if ( DoKeyInp ( keystate ) )
		{
			ShowCursor=true;
		}
		if ( ShowCursor )
		{
			static bool CursorOn=false;
			ShowCursor=false;
			CursorOn=!CursorOn;
			int i_tmpRedrawLength=20; //20 choosen by random to make sure we erase _all_ the old garbage on screen - should be calculated in a better way when fonts come from ttf and not from jpg -- beko
			switch ( Focus )
			{
					/*okej, what we are trying to to here
					*is that: first we get a focus. Then
					*we store the actual length of string
					*in textfield of focus for a cleaner
					*redraw later. Now we do some sanity
					*checks on the input we got. Shorten
					*strings and recognize impossible
					*portsnumbers. Stuff like that. Then
					*we draw new string back on screen
					*adding "_" so the user sees where the
					*focus is while the original data is
					*safed to their proper vals.
					*
					*			-- beko
					*/
				case FOCUS_IP:
					i_tmpRedrawLength += fonts->GetTextLen ( ( char * ) InputStr.c_str() );
					while ( fonts->GetTextLen ( ( char * ) InputStr.c_str() ) >176 )
					{
						InputStr.erase ( InputStr.end()-1 );
					}
					stmp = InputStr; stmp += "_";

					IP=InputStr;
					scr.x=20;scr.y=260;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					fonts->OutText ( ( char * ) stmp.c_str(),20,260,buffer );
					break;
				case FOCUS_PORT:
					i_tmpRedrawLength += fonts->GetTextLen ( ( char * ) InputStr.c_str() );
					if ( atoi ( InputStr.c_str() ) > 65535 ) //ports over 65535 are impossible
					{
						Port = 58600; //default Port 58600 - why is this our default Port? -- beko
						stmp = "58600_";
					}
					else
					{
						stmp = InputStr; stmp += "_";
						Port= atoi ( InputStr.c_str() );
					}
					scr.x=228;scr.y=260;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					fonts->OutText ( ( char * ) stmp.c_str(),228,260,buffer );
					break;
				case FOCUS_NAME:
					i_tmpRedrawLength += fonts->GetTextLen ( ( char * ) InputStr.c_str() );
					while ( fonts->GetTextLen ( ( char * ) InputStr.c_str() ) >98 )
					{
						InputStr.erase ( InputStr.end()-1 );
					}
					stmp = InputStr;  stmp += "_";

					if ( strcmp ( MyPlayer->name.c_str(),InputStr.c_str() ) )
					{
						MyPlayer->name=InputStr;
						DisplayPlayerList(sfTmp);
						ChangeFarbeName();
					}
					scr.x=352;
					scr.y=260;
					scr.w=i_tmpRedrawLength;
					scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					fonts->OutText ( ( char * ) stmp.c_str(),352,260,buffer );
					break;
				case FOCUS_CHAT:
					i_tmpRedrawLength += fonts->GetTextLen ( ( char * ) InputStr.c_str() );
					while ( fonts->GetTextLen ( ( char * ) InputStr.c_str() ) >410 -fonts->GetTextLen ( ( MyPlayer->name+": " ).c_str() ) ) //keeping playername lenght in mind
					{
						InputStr.erase ( InputStr.end()-1 );
					}
					stmp = InputStr; stmp += "_";

					ChatStr=InputStr;
					scr.x=20;scr.y=423;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					fonts->OutText ( ( char * ) stmp.c_str(),20,423,buffer );
					break;
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else
		{
			/*unsigned short hour,min,sec,msec;
			int t;
			(time.CurrentTime()-time).DecodeTime(&hour,&min,&sec,&msec);
			t=(((int)hour*24+min)*60+sec)*1000+msec;
			if(t>500){
			  ShowCursor=true;
			  time=time.CurrentTime();
			}*/
		}

		// Zurück:
		if ( mouse->x>=50&&mouse->x<50+200&&mouse->y>=440&&mouse->y<440+29 )
		{
			if ( b&&!lb )
			{
				BackPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Back" ), true, 50,450);
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&BackPressed )
			{
				if ( fstcpip )
					delete fstcpip;
				// Save changed name, port or ip to max.xml
				SettingsData.sPlayerName = MyPlayer->name;
				SaveOption ( SAVETYPE_NAME );
				SaveOption ( SAVETYPE_IP );
				SaveOption ( SAVETYPE_PORT );
				break;
			}
		}
		else if ( BackPressed )
		{
			BackPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Back" ), false, 50,450 );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Ok:
		if ( host&&mouse->x>=390&&mouse->x<390+200&&mouse->y>=440&&mouse->y<440+29&& ( ( !no_options&&!map.empty() ) || ( !SaveGame.empty() ) ) &&!WaitForGo )
		{
			if ( b&&!lb )
			{
				OKPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), true, 390,450 );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( !b&&OKPressed )
			{
				// Save changed name, port or ip to max.xml
				SettingsData.sPlayerName = MyPlayer->name;
				SaveOption ( SAVETYPE_NAME );
				SaveOption ( SAVETYPE_IP );
				SaveOption ( SAVETYPE_PORT );
				if ( TestPlayerList() && ( TestPlayerListLoad() ||SaveGame.empty() ) )
				{
					/*if(!SaveGame.empty()){
					         unsigned char *msg,*ptr;
					         int file_size,blocks,half,buffer_size;
					         bool start=true;
					         FILE *fp;
					         // Savegame übertragen:
					         fp=fopen((SavePath+SaveGame).c_str(),"rb");
					         if(!fp){
					           break;
					         }
					         fseek(fp,0,SEEK_END);
					         file_size=ftell(fp);
					         fseek(fp,0,SEEK_SET);
					         blocks=(file_size/240);
					         half=file_size-blocks*240;
					         buffer_size=blocks*243+half+3;

					         ptr=msg=(unsigned char*)malloc(buffer_size);
					         while(blocks--){
					           ptr[0]='#';
					           ptr[1]=243;
					           if(start){
					             start=false;
					             ptr[2]=MSG_SAVEGAME_START;
					           }else{
					             ptr[2]=MSG_SAVEGAME_PART;
					           }
					           fread(ptr+3,1,240,fp);
					           ptr+=243;
					         }
					         if(half){
					           ptr[0]='#';
					           ptr[1]=half+3;
					           if(start){
					             start=false;
					             ptr[2]=MSG_SAVEGAME_START;
					           }else{
					             ptr[2]=MSG_SAVEGAME_PART;
					           }
					           fread(ptr+3,1,half,fp);
					         }

					         fstcpip->Send(msg,buffer_size);

					         fclose(fp);
					         free(msg);
					       }*/
					char sztmp[256];
					string msg;
					sprintf ( sztmp,"%d",SettingsData.Checksum );
					msg=sztmp; msg+="#";
					msg+=MAX_VERSION;
					AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Go_Check" ) );

					WaitForGo=true;
					ClientsToGo=fstcpip->GetConnectionCount();
					fstcpip->FSTcpIpSend ( MSG_CHECK_FOR_GO,msg.c_str());
				}
				OKPressed=false;
				drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,450 );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		else if ( OKPressed )
		{
			OKPressed=false;
			drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,450 );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Farbe-Next:
		if ( b&&!lb&&mouse->x>=596&&mouse->x<596+18&&mouse->y>=256&&mouse->y<256+18&&!WaitForGo )
		{
			int nr;
			PlayFX ( SoundData.SNDObjectMenu );
			nr=GetColorNr ( MyPlayer->color ) +1;
			if ( nr>7 ) nr=0;
			MyPlayer->color=OtherData.colors[nr];
			fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Color" ).c_str(),500,245,buffer );
			dest.x=505;dest.y=260;scr.w=dest.w=83;scr.h=dest.h=10;scr.x=0;scr.y=0;
			SDL_BlitSurface ( MyPlayer->color,&scr,buffer,&dest );
			DisplayPlayerList(sfTmp);
			ChangeFarbeName();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Farbe-Prev:
		if ( b&&!lb&&mouse->x>=478&&mouse->x<478+18&&mouse->y>=256&&mouse->y<256+18&&!WaitForGo )
		{
			int nr;
			PlayFX ( SoundData.SNDObjectMenu );
			nr=GetColorNr ( MyPlayer->color )-1;
			if ( nr<0 ) nr=7;
			MyPlayer->color=OtherData.colors[nr];
			fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Color" ).c_str(),500,245,buffer );
			dest.x=505;dest.y=260;scr.w=dest.w=83;scr.h=dest.h=10;scr.x=0;scr.y=0;
			SDL_BlitSurface ( MyPlayer->color,&scr,buffer,&dest );
			DisplayPlayerList(sfTmp);
			ChangeFarbeName();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Host-Buttons:
		if ( host&&SaveGame.empty() )
		{
			// Planet wählen:
			if ( mouse->x>=470&&mouse->x<470+150&&mouse->y>=42&&mouse->y<42+29 )
			{
				if ( b&&!lb )
				{
					PlanetPressed=true;
					PlayFX ( SoundData.SNDMenuButton );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Choose_Planet" ).c_str(),470,42,true );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( !b&&PlanetPressed )
				{
					map=RunPlanetSelect();
					SaveGame="";
					LoadPCXtoSF ( GFXOD_MULT,sfTmp );
					SDL_BlitSurface ( sfTmp,NULL,buffer,NULL );
					DisplayGameSettings(sfTmp);
					DisplayPlayerList(sfTmp);
					fonts->OutTextCenter ( ( char * ) Titel.c_str(),320,11,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_IP" ).c_str(),20,245,buffer );
					fonts->OutText ( ( char * ) IP.c_str(),20,260,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Port" ).c_str(),228,245,buffer );
					sprintf ( sztmp,"%d",Port );
					fonts->OutText ( sztmp,228,260,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Player_Name" ).c_str(),352,245,buffer );
					fonts->OutText ( ( char * ) MyPlayer->name.c_str(),352,260,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Color" ).c_str(),500,245,buffer );
					dest.x=505;dest.y=260;scr.w=dest.w=83;scr.h=dest.h=10;scr.x=0;scr.y=0;
					SDL_BlitSurface ( MyPlayer->color,&scr,buffer,&dest );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Choose_Planet" ).c_str(),470,42,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Options" ).c_str(),470,42+35,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Game_Load" ).c_str(),470,42+35*2,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Button_Host_Start" ).c_str(),470,200,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Send" ).c_str(), 470,416,false );
					drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Back" ),false, 50,450 );
					drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,450);
					SHOW_SCREEN
					mouse->draw ( false,screen );
					SendOptions();
				}
			}
			else if ( PlanetPressed )
			{
				PlanetPressed=false;
				PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Choose_Planet" ).c_str() ,470,42,false );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			// Optionen:
			if ( mouse->x>=470&&mouse->x<470+150&&mouse->y>=42+35&&mouse->y<42+29+35&&!WaitForGo )
			{
				if ( b&&!lb )
				{
					OptionsPressed=true;
					PlayFX ( SoundData.SNDMenuButton );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Options" ).c_str(),470,42+35,true );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( !b&&OptionsPressed )
				{
					if ( no_options )
					{
						options=RunOptionsMenu ( NULL );
					}
					else
					{
						options=RunOptionsMenu ( &options );
					}
					no_options=false;
					SaveGame="";
					LoadPCXtoSF ( GFXOD_MULT,sfTmp );
					SDL_BlitSurface ( sfTmp,NULL,buffer,NULL );
					DisplayGameSettings(sfTmp);
					DisplayPlayerList(sfTmp);
					fonts->OutTextCenter ( ( char * ) Titel.c_str(),320,11,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_IP" ).c_str(),20,245,buffer );
					fonts->OutText ( ( char * ) IP.c_str(),20,260,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Port" ).c_str(),228,245,buffer );
					sprintf ( sztmp,"%d",Port );
					fonts->OutText ( sztmp,228,260,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Player_Name" ).c_str(),352,245,buffer );
					fonts->OutText ( ( char * ) MyPlayer->name.c_str(),352,260,buffer );
					fonts->OutText ( lngPack.Translate ( "Text~Game_Start~Title_Color" ).c_str(),500,245,buffer );
					dest.x=505;dest.y=260;scr.w=dest.w=83;scr.h=dest.h=10;scr.x=0;scr.y=0;
					SDL_BlitSurface ( MyPlayer->color,&scr,buffer,&dest );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Choose_Planet" ).c_str(),470,42,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Options" ).c_str(),470,42+35,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Game_Load" ).c_str(), 470,42+35*2,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Button_Host_Start" ).c_str(),470,200,false );
					PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Send" ).c_str(), 470,416,false );
					drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_Back" ), false,50,450 );
					drawMenuButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ), false, 390,450 );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					SendOptions();
				}
			}
			else if ( OptionsPressed )
			{
				OptionsPressed=false;
				PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Options" ).c_str(),470,42+35,false );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			// Spiel laden:
			/*	  if(mouse->x>=470&&mouse->x<470+150&&mouse->y>=42+35*2&&mouse->y<42+29+32*2){
			        if(b&&!lb){
			          LadenPressed=true;
			          PlayFX(SoundData.SNDMenuButton);
			          PlaceSmallButton(lngPack.Translate( "Text~Menu_Main~Button_Game_Load").c_str(), 470,42+35*2,true);
			          SHOW_SCREEN
			          mouse->draw(false,screen);
					}else if(!b&&LadenPressed){
			          string tmp;
			          tmp=InputStr;
			          ShowDialog("Dateiname:",true,SavePath,1);
			          if(!InputStr.IsEmpty()){
			            SaveGame=InputStr;

			            map_obj=new cMap;
			            game=new cGame(fstcpip,map_obj);
			            game->Load(SaveGame,0,true);
			          }
			          {
			            InputStr=tmp;
			            TmpSf=GraphicsData.gfx_shadow;
			            SDL_SetAlpha(TmpSf,SDL_SRCALPHA,255);
			            if(FileExists(GFXOD_MULT))
			            {
			            	LoadPCXtoSF(GfxODPath+GFXOD_MULT,TmpSf);
			            }
			            SDL_BlitSurface(TmpSf,NULL,buffer,NULL);
			            DisplayGameSettings();
			            DisplayPlayerList();
			            fonts->OutTextCenter(Titel.c_str(),320,11,buffer);
			            fonts->OutText(lngPack.Translate( "Text~Game_Start~Title_IP").c_str(),20,245,buffer);
			            fonts->OutText(IP.c_str(),20,260,buffer);
			            fonts->OutText(lngPack.Translate( "Text~Game_Start~Title_Port").c_str(),228,245,buffer);
			            fonts->OutText(((AnsiString)Port).c_str(),228,260,buffer);
			            fonts->OutText(lngPack.Translate( "Text~Game_Start~Title_Player_Name").c_str(),352,245,buffer);
			            fonts->OutText(MyPlayer->name.c_str(),352,260,buffer);
			            fonts->OutText(lngPack.Translate( "Text~Game_Start~Title_Color").c_str(),500,245,buffer);
			            dest.x=505;dest.y=260;scr.w=dest.w=83;scr.h=dest.h=10;scr.x=0;scr.y=0;
			            SDL_BlitSurface(MyPlayer->color,&scr,buffer,&dest);
			            if(SaveGame.IsEmpty())PlaceSmallButton("Planet wählen",470,42,false);
			            if(SaveGame.IsEmpty())PlaceSmallButton(lngPack.Translate( "Text~Game_Start~Title_Options").c_str(),470,42+35,false);
			            if(SaveGame.IsEmpty())PlaceSmallButton(lngPack.Translate( "Text~Menu_Main~Button_Game_Load").c_str(), 470,42+35*2,false);
			            PlaceSmallButton(lngPack.Translate( "Text~Game_Start~Button_Host_Start").c_str(),470,200,false);
			            PlaceSmallButton(lngPack.Translate( "Text~Game_Start~Title_Send").c_str(), 470,416,false);
			            drawMenuButton(lngPack.Translate( "Text~Menu_Main~Button_Back"),false, 50,450);
			            drawMenuButton(lngPack.Translate( "Text~Menu_Main~Button_OK"),false, 390,450);
			            SHOW_SCREEN
			            mouse->draw(false,screen);
			            SendOptions();
			          }
			        }
			      }else if(LadenPressed){
			        LadenPressed=false;
			        if(SaveGame.empty())PlaceSmallButton(lngPack.Translate( "Text~Menu_Main~Button_Game_Load").c_str(), 470,42+35*2,false);
			        SHOW_SCREEN
			        mouse->draw(false,screen);
			      }*/
		}
		// Host/Connect:
		if ( b&&mouse->x>=470&&mouse->x<470+150&&mouse->y>=200&&mouse->y<200+29&&!WaitForGo )
		{
			if ( !lb )
			{
				StartHostConnect=true;
				PlayFX ( SoundData.SNDMenuButton );
				if ( host ) PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Button_Host_Start" ).c_str(),470,200,true );
				else PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Connect" ).c_str(), 470,200,true );
//FIXME: error opening socket when we choose map and options before starting host -- beko
				if ( host )
				{
					fstcpip->FSTcpIpClose();
					fstcpip->SetTcpIpPort ( Port );
					fstcpip->FSTcpIpReceiveThread = SDL_CreateThread ( Open,NULL );
					if ( fstcpip->iStatus==STAT_OPENED )
					{
						AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Network_Error_Socket" ) );
						cLog::write ( "Error opening socket", cLog::eLOG_TYPE_WARNING );
					}
					else
					{
						sprintf ( sztmp,"%d",Port );
						stmp=lngPack.Translate ( "Text~Game_MP~Comp_Network_Open" );
						stmp+=" (";
						stmp+=lngPack.Translate ( "Text~Game_Start~Title_Port" );
						stmp+=": ";
						stmp+=sztmp;
						stmp+=")";
						AddChatLog ( stmp );
						stmp="Game open (Port: ";stmp+=sztmp; stmp+=")";
						cLog::write ( stmp, cLog::eLOG_TYPE_INFO );
					}
				}
				else
				{
					fstcpip->FSTcpIpClose();
					fstcpip->SetIp ( IP );
					fstcpip->SetTcpIpPort ( Port );

					AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Network_Connecting" ) +IP+":"+sztmp ); // e.g. Connecting to 127.0.0.1:55800
					cLog::write ( ( "Connecting to "+IP+":"+sztmp ), cLog::eLOG_TYPE_INFO );

					fstcpip->FSTcpIpReceiveThread = SDL_CreateThread ( Open,NULL );
					for ( int i=0;i<5;i++ ) //wait 5 seconds for connection - break in case we got one earlier
					{
						SDL_Delay ( 1000 );
						if ( fstcpip->iStatus==STAT_CONNECTED ) continue;

					}
					if( ! fstcpip->bReceiveThreadFinished )
					{
						SDL_KillThread ( fstcpip->FSTcpIpReceiveThread ) ;
					}
					sprintf ( sztmp,"%d",Port );

					if ( fstcpip->iStatus!=STAT_CONNECTED )
					{
						AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Network_Error_Connect" ) +IP+":"+sztmp );
						cLog::write ( "Error on connecting "+IP+":"+sztmp, cLog::eLOG_TYPE_WARNING );
					}
				}

				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		else if ( StartHostConnect )
		{
			StartHostConnect=false;
			if ( host ) PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Button_Host_Start" ).c_str(),470,200,false );
			else PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Connect" ).c_str(), 470,200,false );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Senden:
		if ( ( b&&mouse->x>=470&&mouse->x<470+150&&mouse->y>=416&&mouse->y<416+29 ) || ( InputEnter&&Focus==FOCUS_CHAT ) )
		{
			if ( !lb|| ( InputEnter&&Focus==FOCUS_CHAT ) )
			{
				SendenPressed=true;
				PlayFX ( SoundData.SNDMenuButton );
				PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Send" ).c_str(), 470,416,true );

				if ( !ChatStr.empty() )
				{
					PlayFX ( SoundData.SNDChat );
					ChatStr.insert ( 0,": " );
					ChatStr.insert ( 0,MyPlayer->name );

					if ( ChatStr.length() >=200 )
					{
						ChatStr.erase ( 200 );
					}
					fstcpip->FSTcpIpSend ( MSG_CHAT, ( char * ) ChatStr.c_str());

					AddChatLog ( ChatStr );
					ChatStr="";
					if ( Focus==FOCUS_CHAT ) InputStr="";
					scr.x=20;scr.y=423;scr.w=430;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) ChatStr.c_str(),20,423,buffer );
				}

				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		else if ( SendenPressed )
		{
			SendenPressed=false;
			PlaceSmallButton ( lngPack.Translate ( "Text~Game_Start~Title_Send" ).c_str(), 470,416,false );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick auf die IP:
		if ( !host&&b&&!lb&&mouse->x>=20&&mouse->x<20+188&&mouse->y>=250&&mouse->y<250+30 )
		{
			Focus=FOCUS_IP;
			InputStr=IP;
			ShowCursor=true;
			sprintf ( sztmp,"%d",Port );
			scr.x=20;scr.y=260;scr.w=188;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) IP.c_str(),20,260,buffer );scr.x=228;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( sztmp,228,260,buffer );scr.x=352;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) MyPlayer->name.c_str(),352,260,buffer );scr.x=20;scr.y=423;scr.w=430;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) ChatStr.c_str(),20,423,buffer );
			// Klick auf den Port:
		}
		else if ( b&&!lb&&mouse->x>=228&&mouse->x<228+108&&mouse->y>=250&&mouse->y<250+30 )
		{
			Focus=FOCUS_PORT;
			sprintf ( sztmp,"%d",Port );
			InputStr=sztmp;
			ShowCursor=true;
			scr.x=20;scr.y=260;scr.w=188;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) IP.c_str(),20,260,buffer );scr.x=228;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( sztmp,228,260,buffer );scr.x=352;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) MyPlayer->name.c_str(),352,260,buffer );scr.x=20;scr.y=423;scr.w=430;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) ChatStr.c_str(),20,423,buffer );
			// Klick auf den Namen:
		}
		else if ( b&&!lb&&mouse->x>=352&&mouse->x<352+108&&mouse->y>=250&&mouse->y<250+30 )
		{
			Focus=FOCUS_NAME;
			InputStr=MyPlayer->name;
			ShowCursor=true;
			sprintf ( sztmp,"%d",Port );
			scr.x=20;scr.y=260;scr.w=188;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) IP.c_str(),20,260,buffer );scr.x=228;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( sztmp,228,260,buffer );scr.x=352;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) MyPlayer->name.c_str(),352,260,buffer );scr.x=20;scr.y=423;scr.w=430;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) ChatStr.c_str(),20,423,buffer );
			// Klick auf den ChatStr:
		}
		else if ( b&&!lb&&mouse->x>=20&&mouse->x<20+425&&mouse->y>=420&&mouse->y<420+30 )
		{
			Focus=FOCUS_CHAT;
			InputStr=ChatStr;
			ShowCursor=true;
			sprintf ( sztmp,"%d",Port );
			scr.x=20;scr.y=260;scr.w=188;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) IP.c_str(),20,260,buffer );scr.x=228;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( sztmp,228,260,buffer );scr.x=352;scr.y=260;scr.w=108;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) MyPlayer->name.c_str(),352,260,buffer );scr.x=20;scr.y=423;scr.w=430;scr.h=16;SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );fonts->OutText ( ( char * ) ChatStr.c_str(),20,423,buffer );
		}

		// Das WaitForGo machen:
		if ( host&&WaitForGo )
		{
			if ( ClientsToGo>fstcpip->GetConnectionCount() )
			{
				AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Go_Abort" ) );
				WaitForGo=false;
			}
			else if ( ClientsToGo<=0&&SaveGame.empty() )
			{
				ClientSettingsList=new TList;

				AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Go" ) );
				fstcpip->FSTcpIpSend ( MSG_LETS_GO,"");

				// Das Spiel machen:
				TList *LandingList;
				int i,LandX,LandY;
				map_obj=new cMap();
				if ( map_obj->LoadMap ( map ) )
				{

					map_obj->PlaceRessources ( options.metal,options.oil,options.gold,options.dichte );
					game=new cGame ( fstcpip,map_obj );
					game->AlienTech=options.AlienTech;
					game->PlayRounds=options.PlayRounds;
					game->ActiveRoundPlayerNr=MyPlayer->Nr;
					game->Init ( PlayerList,0 );

					for ( i=0;i<PlayerList->Count;i++ )
					{
						PlayerList->PlayerItems[i]->InitMaps ( map_obj->size );
					}
					MyPlayer->Credits=options.credits;

					LandingList=new TList;
					RunHangar ( MyPlayer,LandingList );

					SelectLanding ( &LandX,&LandY,map_obj );

					ServerWait ( LandX,LandY,LandingList );

					while ( LandingList->Count )
					{
						delete LandingList->LandItems[0];
						LandingList->DeleteLanding ( 0 );
					}
					delete LandingList;

					ExitMenu();

					fstcpip->iMin_clients=PlayerList->Count-1;
					game->Run();
					SettingsData.sPlayerName=MyPlayer->name;

					while ( PlayerList->Count )
					{
						delete PlayerList->PlayerItems[0];
						PlayerList->DeletePlayer ( 0 );
					}
					delete game;game=NULL;
					break;
				}
				else
				{
					AddChatLog ( lngPack.Translate ( "Text~Error_Messages~ERROR_Map_Loading" ) );
					cLog::write ( "Error loading map", cLog::eLOG_TYPE_WARNING );
					delete ClientSettingsList;
				}
			}
			else if ( ClientsToGo<=0&&!SaveGame.empty() )
			{
				/*unsigned char msg[3];
				int i;

				msg[0]='#';
				msg[1]=3;
				msg[2]=MSG_LETS_GO;
				fstcpip->Send(msg,3);

				ExitMenu();
				fstcpip->RxFunc=game->engine->ReceiveNetMsg;
				TmpSf=NULL;

				for(i=0;i<game->PlayerList->Count;i++){
				  if(((cPlayer*)(game->PlayerList->Items[i]))->name==MyPlayer->name){
				    game->ActivePlayer=(cPlayer*)(game->PlayerList->Items[i]);
				    break;
				  }
				}

				fstcpip->MinConnections=PlayerList->Count-1;
				*(game->hud)=game->ActivePlayer->HotHud;
				if(game->hud->Zoom!=64){
				  game->hud->LastZoom=-1;
				  game->hud->ScaleSurfaces();
				}
				game->Run();
				sPlayerName=MyPlayer->name;

				break;*/
			}
		}

		// Das LetsGo machen:
		if ( !host&&LetsGo&&SaveGame.empty() )
		{
			// Das Spiel machen:
			int i,LandX,LandY,nr;
			TList *LandingList;
			map_obj=new cMap();
			LetsGo=false;
			if ( map_obj->LoadMap ( map ) )
			{

				for ( i=0;i<PlayerList->Count;i++ )
				{
					if ( PlayerList->PlayerItems[i]==MyPlayer ) {nr=i;break;}
				}

				game=new cGame ( fstcpip,map_obj );
				game->AlienTech=options.AlienTech;
				game->PlayRounds=options.PlayRounds;
				game->ActiveRoundPlayerNr=-1;
				game->Init ( PlayerList,nr );

				for ( i=0;i<PlayerList->Count;i++ )
				{
					PlayerList->PlayerItems[i]->InitMaps ( map_obj->size );
				}

				MyPlayer->Credits=options.credits;

				LandingList=new TList;
				RunHangar ( MyPlayer,LandingList );

				SelectLanding ( &LandX,&LandY,map_obj );

				// Settings übertragen:
				ClientWait ( LandX,LandY,LandingList );

				while ( LandingList->Count )
				{
					delete LandingList->LandItems[0];
					LandingList->DeleteLanding ( 0 );
				}
				delete LandingList;

				ExitMenu();

				fstcpip->iMax_clients = fstcpip->iMin_clients = PlayerList->Count-1; // set maximal and minimal players for this game
				game->Run();
				SettingsData.sPlayerName=MyPlayer->name;

				while ( PlayerList->Count )
				{
					delete PlayerList->PlayerItems[0];
					PlayerList->DeletePlayer ( 0 );
				}
				delete game;game=NULL;
				break;
			}
			else
			{
				AddChatLog ( lngPack.Translate ( "Text~Error_Messages~ERROR_Map_Loading" ) );
				cLog::write ( "Error loading map", cLog::eLOG_TYPE_WARNING );
				delete map_obj;map_obj=NULL;
			}
		}
		else if ( !host&&LetsGo&&!SaveGame.empty() )
		{
			/*int i;

			map_obj=new cMap;
			game=new cGame(fstcpip,map_obj);
			game->Load(SaveGame,0,true);

			ExitMenu();
			TmpSf=NULL;
			fstcpip->RxFunc=game->engine->ReceiveNetMsg;

			for(i=0;i<game->PlayerList->Count;i++){
			  if(((cPlayer*)(game->PlayerList->Items[i]))->name==MyPlayer->name){
			    game->ActivePlayer=(cPlayer*)(game->PlayerList->Items[i]);
			    break;
			  }
			}

			fstcpip->MinConnections=PlayerList->Count-1;
			*(game->hud)=game->ActivePlayer->HotHud;
			if(game->hud->Zoom!=64){
			  game->hud->LastZoom=-1;
			  game->hud->ScaleSurfaces();
			}
			game->Run();
			sPlayerName=MyPlayer->name;

			break;*/
		}

		// Ggf Chatlogs anzeigen:
		ShowChatLog(sfTmp);
		// Ggf weitere Daten anzeigen:
		if ( Refresh )
		{
			Refresh=false;
			DisplayGameSettings(sfTmp);
			DisplayPlayerList(sfTmp);
		}

		// Ggf Meldung über Statusänderung machen:
		if ( LastStatus!=fstcpip->iStatus )
		{
			LastStatus=fstcpip->iStatus;
			switch ( LastStatus )
			{
				case STAT_CONNECTED:
					if ( host )
					{
						AddChatLog ( "fstcpip: "+lngPack.Translate ( "Text~Game_MP~Comp_Network_New" ) );
						cLog::write ( "New connection", cLog::eLOG_TYPE_DEBUG );
					}
					else
					{
						AddChatLog ( "fstcpip: "+lngPack.Translate ( "Text~Game_MP~Comp_Network_Connected" ) );
						cLog::write ( "Connected", cLog::eLOG_TYPE_DEBUG );
						ClientConnectedCallBack();
					}
					break;
				case STAT_CLOSED:
					AddChatLog ( "fstcpip: "+lngPack.Translate ( "Text~Game_MP~Comp_Network_Closed" ) );
					cLog::write ( "Connection closed", cLog::eLOG_TYPE_DEBUG );
					if ( !host ) ClientDistconnect();
					break;
			}
		}
		if ( host )
		{
			if ( LastConnectionCount>fstcpip->GetConnectionCount() )
			{
				ServerDisconnect();
			}
			LastConnectionCount=fstcpip->GetConnectionCount();
		}

		lx=mouse->x;
		ly=mouse->y;
		lb=b;
		if ( fstcpip->iStatus==STAT_CONNECTED && fstcpip->bReceiveThreadFinished )
		{
			SDL_WaitThread ( fstcpip->FSTcpIpReceiveThread, NULL ); // free the last memory allocated by the thread. If not done so, SDL_CreateThread will hang after about 1010 successfully created threads
			fstcpip->FSTcpIpReceiveThread = SDL_CreateThread ( Receive,NULL );
		}
		HandleMenuMessages();
		SDL_Delay ( 1 );
	}
	SDL_FreeSurface(sfTmp);
}

// Empfängt eine Nachricht fürs Menü:
void cMultiPlayer::HandleMenuMessages()
{
	cNetMessage *msg;
	string msgstring;
	for ( int i=0;i<fstcpip->NetMessageList->iCount;i++ )
	{
		msg = (cNetMessage *) fstcpip->NetMessageList->Items[i];
		msgstring = ( char * ) msg->msg;
		switch ( msg->typ )
		{
				// Chatnachricht:
			case MSG_CHAT:
				AddChatLog ( msgstring );
				PlayFX ( SoundData.SNDChat );
				fstcpip->NetMessageList->Delete ( i );
				break;
				// Neuer Spieler meldet sich an:
			case MSG_SIGNING_IN:
			{
				cPlayer *p;
				char sztmp[256];
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				p=new cPlayer ( Strings->Items[0],OtherData.colors[atoi ( Strings->Items[1].c_str() ) ],NextPlayerID++ );
				PlayerList->AddPlayer ( p );
				Refresh=true;
				string smsg;
				sprintf ( sztmp,"%d",p->Nr );
				smsg=Strings->Items[2]; smsg+="#"; smsg+=sztmp;
				fstcpip->FSTcpIpSend ( MSG_YOUR_ID_IS, ( char * ) smsg.c_str());
				SendPlayerList();
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Mitteilung über die eigene ID:
			case MSG_YOUR_ID_IS:
			{
				cPlayer *p;
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				if ( MyPlayer->Nr!=atoi ( Strings->Items[0].c_str() ) )
				{
					fstcpip->NetMessageList->Delete ( i );
					break;
				}
				for ( i=0;i<PlayerList->Count;i++ )
				{
					p=PlayerList->PlayerItems[i];
					if ( p==MyPlayer )
					{
						p->Nr=atoi ( Strings->Items[1].c_str() );
						break;
					}
				}
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Ein Client ändert seinen Namen:
			case MSG_MY_NAME_CHANGED:
			{
				cPlayer *p;
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				int i;
				for ( i=0;i<PlayerList->Count;i++ )
				{
					p=PlayerList->PlayerItems[i];
					if ( p->Nr!=atoi ( Strings->Items[0].c_str() ) ) continue;
					p->color=OtherData.colors[atoi ( Strings->Items[1].c_str() ) ];
					p->name=Strings->Items[2];
					Refresh=true;
					SendPlayerList();
					break;
				}
				if ( i==PlayerList->Count )
				{
					p=new cPlayer ( Strings->Items[2],OtherData.colors[atoi ( Strings->Items[1].c_str() ) ],atoi ( Strings->Items[0].c_str() ) );
					PlayerList->AddPlayer ( p );
					SendPlayerList();
					Refresh=true;
				}
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Bekommt die Liste mit den Spielern:
			case MSG_PLAYER_LIST:
			{
				int count,myID;
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				myID=MyPlayer->Nr;
				while ( PlayerList->Count )
				{
					delete PlayerList->PlayerItems[0];
					PlayerList->DeletePlayer ( 0 );
				}
				count=atoi ( Strings->Items[0].c_str() );
				for ( int k = 0;count--;k++ )
				{
					cPlayer *p;
					int id,color;
					id=atoi ( Strings->Items[k*3+1].c_str() );
					color=atoi ( Strings->Items[k*3+2].c_str() );

					p=new cPlayer ( Strings->Items[k*3+3],OtherData.colors[color],id );
					if ( id==myID ) MyPlayer=p;
					PlayerList->AddPlayer ( p );
				}
				Refresh=true;
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Überträgt die Optionen:
			case MSG_OPTIONS:
			{
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				no_options=atoi ( Strings->Items[0].c_str() );

				SaveGame=Strings->Items[1];

				if ( !no_options )
				{
					options.AlienTech=atoi ( Strings->Items[2].c_str() );
					options.credits=atoi ( Strings->Items[3].c_str() );
					options.dichte=atoi ( Strings->Items[4].c_str() );
					options.FixedBridgeHead=atoi ( Strings->Items[5].c_str() );
					options.gold=atoi ( Strings->Items[6].c_str() );
					options.metal=atoi ( Strings->Items[7].c_str() );
					options.oil=atoi ( Strings->Items[8].c_str() );
					options.PlayRounds=atoi ( Strings->Items[9].c_str() );
					map=Strings->Items[10];
				}
				else
					map=Strings->Items[4];
				Refresh=true;
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Fordert einen Client auf sich zu identifizieren:
			case MSG_WHO_ARE_YOU:
			{
				ChangeFarbeName();
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Prüfen, ob der Client bereit ist zum Go:
			case MSG_CHECK_FOR_GO:
			{
				char sztmp[256];
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				FILE *fp;
				string mapstr;
				mapstr=SettingsData.sMapsPath; mapstr+=PATH_DELIMITER; mapstr+=map;
				if ( FileExists ( mapstr.c_str() ) )
				{
					fp=fopen ( mapstr.c_str(),"rb" );
				}
				if ( atoi ( Strings->Items[0].c_str() ) ==SettingsData.Checksum && strcmp ( Strings->Items[1].c_str(),MAX_VERSION ) ==0 && fp )
				{
					string new_msg;
					sprintf ( sztmp,"%d",MyPlayer->Nr );
					new_msg=sztmp;
					fstcpip->FSTcpIpSend ( MSG_READY_TO_GO,new_msg.c_str() );
					AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Go_Host" ) );
				}
				else
				{
					string new_msg;
					sprintf ( sztmp,"%d",MyPlayer->Nr );
					new_msg=sztmp;
					fstcpip->FSTcpIpSend ( MSG_NO_GO,new_msg.c_str() );
					AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Go_Host_No" ) );
				}
				if ( fp ) fclose ( fp );
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Benachrichtigung über einen nicht bereiten Client:
			case MSG_NO_GO:
			{
				cPlayer *p;
				int i;
				for ( i=0;i<PlayerList->Count;i++ )
				{
					p=PlayerList->PlayerItems[i];
					if ( p->Nr==atoi ( msgstring.c_str() ) )
					{
						string log;
						log=p->name; log+=": "+lngPack.Translate ( "Text~Game_MP~Comp_Go_Ready_No" );
						AddChatLog ( log );
						break;
					}
				}
				WaitForGo=false;
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Benachrichtigung über einen bereiten Client:
			case MSG_READY_TO_GO:
			{
				cPlayer *p;
				int i;
				for ( i=0;i<PlayerList->Count;i++ )
				{
					p=PlayerList->PlayerItems[i];
					if ( p->Nr==atoi ( msgstring.c_str() ) )
					{
						string log;
						log=p->name; log+=": "+lngPack.Translate ( "Text~Game_MP~Comp_Go_Ready" );
						AddChatLog ( log );
						break;
					}
				}
				ClientsToGo--;
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Benachrichtigung, dass es jetzt los geht:
			case MSG_LETS_GO:
				LetsGo=true;
				fstcpip->NetMessageList->Delete ( i );
				break;
				// Die Ressourcen:
			case MSG_RESSOURCES:
			{
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				int off;
				if ( map_obj==NULL ) break;
				for ( int k=0;k<Strings->Count;k++ )
				{
					off=atoi ( Strings->Items[k].c_str() );
					map_obj->Resources[off].typ= ( unsigned char ) atoi ( Strings->Items[k++].c_str() );
					map_obj->Resources[off].value= ( unsigned char ) atoi ( Strings->Items[k++].c_str() );
				}
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Empfang der Upgrades eines Players:
			case MSG_PLAYER_UPGRADES:
			{
				cPlayer *p;
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				int nr,i;
				nr=atoi ( Strings->Items[0].c_str() );
				for ( i=0;i<PlayerList->Count;i++ )
				{
					p=PlayerList->PlayerItems[i];
					if ( p->Nr==nr )
						break;
				}
				if ( p==MyPlayer )
				{
					fstcpip->NetMessageList->Delete ( i );
					break;
				}
				for ( int i=1;i<Strings->Count;i++ )
				{
					if ( atoi ( Strings->Items[i].c_str() ) ==0 )
					{
						p->VehicleData[i].damage=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].max_shots=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].range=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].max_ammo=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].armor=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].max_hit_points=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].scan=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].max_speed=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].version++;
					}
					else
					{
						p->BuildingData[i].damage=atoi ( Strings->Items[i++].c_str() );
						p->BuildingData[i].max_shots=atoi ( Strings->Items[i++].c_str() );
						p->BuildingData[i].range=atoi ( Strings->Items[i++].c_str() );
						p->BuildingData[i].max_ammo=atoi ( Strings->Items[i++].c_str() );
						p->BuildingData[i].armor=atoi ( Strings->Items[i++].c_str() );
						p->BuildingData[i].max_hit_points=atoi ( Strings->Items[i++].c_str() );
						p->BuildingData[i].scan=atoi ( Strings->Items[i++].c_str() );
						p->VehicleData[i].version++;
					}
				}
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			// Landedaten eines Players:
			case MSG_PLAYER_LANDING:
			{
				TList *Strings;
				Strings = SplitMessage ( msgstring );
				sClientSettings *cs=NULL;
				int nr,k,max;

				nr=atoi ( Strings->Items[2].c_str() );
				if ( nr==MyPlayer->Nr )
				{
					fstcpip->NetMessageList->Delete ( i );
					break;
				}

				for ( k=0;k<ClientSettingsList->Count;k++ )
				{
					cs=ClientSettingsList->ClientSettingsItems[k];
					if ( cs->nr==nr )
					{
						break;
					}
				}

				if ( cs==NULL||k==ClientSettingsList->Count )
				{
					cs=new sClientSettings;
					cs->LandX=atoi ( Strings->Items[0].c_str() );
					cs->LandY=atoi ( Strings->Items[1].c_str() );
					cs->nr=nr;
					cs->LandingList=new TList;
					ClientSettingsList->AddClientSettings ( cs );
				}
				max=atoi ( Strings->Items[3].c_str() );

				for ( k=4;k< ( max*2+4 );k++ )
				{
					sLanding *l;
					l=new sLanding;
					l->id=atoi ( Strings->Items[k].c_str() );
					l->cargo=atoi ( Strings->Items[k++].c_str() );
					cs->LandingList->AddLanding ( l );
				}
				fstcpip->NetMessageList->Delete ( i );
				break;
			}
			default:
				fstcpip->NetMessageList->Delete ( i );
				break;
		}
	}
}

// Zeigt die Settings für das Spiel an:
void cMultiPlayer::DisplayGameSettings ( SDL_Surface *surface )
{
	string str;
	char sztmp[256];
	SDL_Rect r;

	r.x=192;r.y=52;
	r.w=246;r.h=176;

	if ( !host )
	{
		SDL_BlitSurface ( surface,&r,buffer,&r );
	}

	str="Version: "; str+=MAX_VERSION; str+="\n";
	sprintf ( sztmp,"%d",SettingsData.Checksum );
	str+="Checksum: "; str+=sztmp; str+="\n";
	str+="\n";

	if ( !host&&fstcpip->iStatus!=STAT_CONNECTED )
	{
		str+=lngPack.Translate ( "Text~Game_MP~Comp_Network_Connected_Not" );
		fonts->OutTextBlock ( ( char * ) str.c_str(),r,buffer );
		return;
	}

	if ( !SaveGame.empty() )
	{
		char *tmpstr;
		FILE *fp;
		int len;
		str+="Savegame: "; str+=SaveGame; str+="\n";
		if ( host )
		{
			fp=fopen ( ( SaveGame ).c_str(),"rb" );
			if ( fp==NULL )
			{
				str+="Fehler beim Öffnen\n";
			}
			else
			{
				fread ( &len,sizeof ( int ),1,fp );
				tmpstr= ( char* ) malloc ( len );
				fread ( tmpstr,1,len,fp );
				map=tmpstr;
				free ( tmpstr );
				sprintf ( sztmp,"%d",game->Runde );
				str+="Runde: "; str+=sztmp; str+="\n";

				if ( host&&game )
				{
					int i;
					str+=lngPack.Translate ( "Text~Game_MP~Title_Player" ) +": ";
					for ( i=0;i<game->PlayerList->Count;i++ )
					{
						str+=game->PlayerList->PlayerItems[i]->name;
						if ( i<game->PlayerList->Count-1 ) str+=",";
					}
					str+="\n";
				}

				fclose ( fp );
			}
		}
		str+="\n";
	}

	if ( !map.empty() )
	{
		FILE *fp;
		/**mappath*/
		string sMapPath;
		/**eyecandy mapname*/
		string sNameNice;
		/**mapimage*/
		string sMapImage;

		//set absolute mapname
		sMapPath=SettingsData.sMapsPath;
		sMapPath+=PATH_DELIMITER;
		sMapPath+=map;

		//remove ".map" for eyecandy
		sNameNice = map;
		sNameNice.erase ( sNameNice.length()-4,4 );

		//set abolute mapimagename
		sMapImage=sMapPath;
		//replace "ma" from ".map" with "bm" so our new ending is .bmp
		sMapImage.replace ( sMapImage.length()-3,2,"bm" );

		if ( !FileExists ( sMapPath.c_str() ) )
		{
			//d'oh, somebody doesn't have the map we've choosen here
			str+=lngPack.Translate ( "Text~Error_Messages~ERROR_Map_Loading" ) +" "+map+"\n";
			cLog::write ( "Couldn't load map "+sMapPath, cLog::eLOG_TYPE_WARNING );
		}
		else
		{
			//draw mapinfo in game infobox
			fp=fopen ( sMapPath.c_str(),"rb" );
			/**rect to draw mapname*/
			SDL_Rect r = {20,60,150,20};
			/**Mapsize*/
			int iSize=0;

			str+=lngPack.Translate ( "Text~Game_Start~Title_Map" ) +": "+sNameNice;
			if ( fp )
			{
				fseek ( fp,21,SEEK_SET );
				fread ( &iSize,sizeof ( int ),1,fp );
				fclose ( fp );
			}
			sprintf ( sztmp,"%d",iSize );
			str+=" ("; str+=sztmp; str+="x"; str+=sztmp; str+=")\n";

			SDL_BlitSurface ( surface,&r,buffer,&r );

			//draw mapname to infobox map
			fonts->OutTextCenter ( ( char * ) sNameNice.c_str(),90,65,buffer );

			//load mapimage (if exists)
			if ( FileExists ( sMapImage.c_str() ) )
			{
				//draw map in infobox for map
				fp=fopen ( sMapImage.c_str(),"rb" );
				SDL_Surface *sf;
				sf=SDL_LoadBMP ( sMapImage.c_str() );
				if ( sf!=NULL )
				{
					/**rect to draw mapimage*/
					SDL_Rect dest = {33,106,112,112};
					SDL_BlitSurface ( sf,NULL,buffer,&dest );
				}
				SDL_FreeSurface ( sf );
			}
		}
	}
	else
	{
		if ( SaveGame.empty() ) str+=lngPack.Translate ( "Text~Game_MP~Comp_Map_NoSet" ) +"\n";
	}
	str+="\n";
	if ( SaveGame.empty() )
	{
		if ( !no_options )
		{
			str+=METAL+": ";
			str+= ( options.metal<2? ( options.metal<1?LOW:MIDDLE ) : ( options.metal<3?MUCH:MOST ) ); str+="\n";

			str+=OIL+": ";
			str+= ( options.oil<2? ( options.oil<1?LOW:MIDDLE ) : ( options.oil<3?MUCH:MOST ) ); str+="\n";

			str+=GOLD+": ";
			str+= ( options.gold<2? ( options.gold<1?LOW:MIDDLE ) : ( options.gold<3?MUCH:MOST ) ); str+="\n";

			str+=RESOURCE+": ";
			str+= ( options.dichte<2? ( options.dichte<1?THIN:MIDDLE ) : ( options.gold<3?THICK:MOST ) ); str+="\n";

			str+=CREDITS+": "; sprintf ( sztmp,"%d",options.credits ); str+=sztmp; str+="\n";

			str+=HEAD+": "; str+= ( options.FixedBridgeHead?DEFINITE:MOBILE ); str+="\n";

			str+=ALIEN+": "; str+= ( options.AlienTech?ON:OFF ); str+="\n";

			str+=GAMETYPE+": "; str+= ( options.PlayRounds?TURNS:SIMU ); str+="\n";

		}
		else
		{
			str+=lngPack.Translate ( "Text~Game_MP~Comp_Option_NoSet" ) +"\n";
		}
	}

	fonts->OutTextBlock ( ( char * ) str.c_str(),r,buffer );
}

TList* cMultiPlayer::SplitMessage ( string msg )
{
	TList *Strings;
	Strings = new TList;
	int npos=0;
	for ( int i=0; npos!=string::npos; i++ )
	{
		Strings->Items[i]=msg.substr ( npos, ( msg.find ( "#",npos )-npos ) );
		npos= ( int ) msg.find ( "#",npos );
		if ( npos!=string::npos )
			npos++;
	}
	return Strings;
}


void cMultiPlayer::ShowChatLog ( SDL_Surface *surface )
{
	string str;
	int i;
	if ( !ChatList->Count ) return;
	for ( i=ChatList->Count-1;i>=0;i-- )
	{
		str=ChatList->Items[i];
		while ( fonts->GetTextLen ( ( char * ) str.c_str() ) >410 )
		{
			str.erase ( str.end()-1 );
		}
		SDL_Rect scr,dest;
		scr.x=27;scr.y=298+11;
		dest.w=scr.w=420;dest.h=scr.h=8*11;
		dest.x=27;dest.y=298;
		SDL_BlitSurface ( buffer,&scr,buffer,&dest );
		dest.y=298+8*11;
		dest.h=11;
		SDL_BlitSurface ( surface,&dest,buffer,&dest );
		fonts->OutText ( ( char * ) str.c_str(),dest.x,dest.y,buffer );
	}
	while ( ChatList->Count>0 )
		ChatList->DeleteString ( 0 );
	SHOW_SCREEN
	mouse->draw ( false,screen );
}

// Fügt einen ChatLogEintrag hinzu:
void cMultiPlayer::AddChatLog ( string str )
{
	ChatList->Add ( str );
	if ( SettingsData.bDebug ) cLog::write ( str.c_str(), cLog::eLOG_TYPE_DEBUG );
}

// Zeigt die Liste ,it den Spielern an:
void cMultiPlayer::DisplayPlayerList ( SDL_Surface *surface )
{
	SDL_Rect scr,dest;
	cPlayer *p;
	int i;
	scr.x=465;scr.y=287;
	scr.w=162;scr.h=116;
	SDL_BlitSurface ( surface,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	dest.w=dest.h=scr.w=scr.h=10;
	dest.x=476;dest.y=297;

	for ( i=0;i<PlayerList->Count;i++ )
	{
		p=PlayerList->PlayerItems[i];

		SDL_BlitSurface ( p->color,&scr,buffer,&dest );
		fonts->OutText ( p->name,dest.x+16,dest.y,buffer );
		dest.y+=16;
	}
	SHOW_SCREEN
}

// Callback für einen Client, der eine Connection bekommt:
void cMultiPlayer::ClientConnectedCallBack ( void )
{
	char sztmp[32];
	string msg;
	MyPlayer->Nr=100+random ( 1000000,1 );
	msg=MyPlayer->name; msg+="#";
	sprintf ( sztmp,"%d",GetColorNr ( MyPlayer->color ) );
	msg+=sztmp; msg+="#";
	sprintf ( sztmp,"%d",MyPlayer->Nr );
	msg+=sztmp;
	fstcpip->FSTcpIpSend ( MSG_SIGNING_IN, ( char * ) msg.c_str() );
}

// Meldet einen Disconnect, wenn man Client ist:
void cMultiPlayer::ClientDistconnect ( void )
{
	Refresh=true;
	while ( PlayerList->Count )
	{
		if ( PlayerList->PlayerItems[0]!=MyPlayer )
		{
			delete PlayerList->PlayerItems[0];
		}
		PlayerList->DeletePlayer ( 0 );
	}
	PlayerList->AddPlayer ( MyPlayer );
	MyPlayer->Nr=0;
}

// Meldet einen Disconnect, wenn man Server ist:
void cMultiPlayer::ServerDisconnect ( void )
{
	while ( PlayerList->Count )
	{
		if ( PlayerList->PlayerItems[0]!=MyPlayer )
		{
			delete PlayerList->PlayerItems[0];
		}
		PlayerList->DeletePlayer ( 0 );
	}
	PlayerList->AddPlayer ( MyPlayer );

	fstcpip->FSTcpIpSend ( MSG_WHO_ARE_YOU,"" );
	Refresh=true;
}

// Wird aufgerufen, wenn die Farbe/Name geändert wurden:
void cMultiPlayer::ChangeFarbeName ( void )
{
	string msg;
	char sztmp[256];
	if ( fstcpip->bServer && !fstcpip->GetConnectionCount() ) return;
	if ( !fstcpip->bServer && fstcpip->iStatus!=STAT_CONNECTED ) return;

	if ( fstcpip->bServer )
	{
		SendPlayerList();
		return;
	}
	sprintf ( sztmp,"%d",MyPlayer->Nr );
	msg=sztmp; msg+="#";
	sprintf ( sztmp,"%d",GetColorNr ( MyPlayer->color ) );
	msg+=sztmp; msg+="#";
	msg+=MyPlayer->name;
	fstcpip->FSTcpIpSend ( MSG_MY_NAME_CHANGED, ( char * ) msg.c_str() );
}

// Versendet eine Liste mit allen Spielern:
void cMultiPlayer::SendPlayerList ( void )
{
	char sztmp[256];
	string msg;
	cPlayer *p;
	sprintf ( sztmp,"%d",PlayerList->Count );
	msg=sztmp; msg+="#";
	for ( int i=0; i<PlayerList->Count; i++ )
	{
		p=PlayerList->PlayerItems[i];
		sprintf ( sztmp,"%d",p->Nr );
		msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",GetColorNr ( p->color ) );
		msg+=sztmp; msg+="#";
		msg+=p->name; if ( i!=PlayerList->Count-1 ) msg+="#";
	}
	fstcpip->FSTcpIpSend ( MSG_PLAYER_LIST, ( char * ) msg.c_str() );
}

// Überträgt die Spieloptionen:
void cMultiPlayer::SendOptions ( void )
{
	char sztmp[256];
	string msg;
	sprintf ( sztmp,"%d",no_options );
	msg=sztmp; msg+="#";
	msg+=SaveGame; msg+="#";
	if ( !no_options )
	{
		sprintf ( sztmp,"%d",options.AlienTech ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.credits ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.dichte ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.FixedBridgeHead ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.gold ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.metal ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.oil ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",options.PlayRounds ); msg+=sztmp; msg+="#";
	}
	msg+=map;

	fstcpip->FSTcpIpSend ( MSG_OPTIONS, msg.c_str() );
}

// Sendet die Ressourcmap an alle Clients:
void cMultiPlayer::TransmitRessources ( void )
{
	string msg;
	char sztmp[256];
	int i;
	for ( i=0;i<map_obj->size*map_obj->size;i++ )
	{
		if ( !map_obj->Resources[i].typ ) continue;
		if ( msg.length() >0 ) msg+="#";
		sprintf ( sztmp,"%d",no_options ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",map_obj->Resources[i].typ ); msg+=sztmp; msg+="#";
		sprintf ( sztmp,"%d",map_obj->Resources[i].value ); msg+=sztmp;
		if ( msg.length() >200 )
		{
			fstcpip->FSTcpIpSend ( MSG_RESSOURCES,msg.c_str() );
//      SDL_Delay(10);
			msg="";
		}
	}
	if ( msg.length() >0 )
	{
		fstcpip->FSTcpIpSend ( MSG_RESSOURCES,msg.c_str() );
//    SDL_Delay(10);
	}
}

// Wartet auf alle anderen Spieler (als Server):
void cMultiPlayer::ServerWait ( int LandX,int LandY,TList *LandingList )
{
	int lx=-1,ly=-1;
	int i;
	fonts->OutTextBigCenter ( lngPack.Translate ( "Text~Game_MP~Comp_Waiting" ).c_str(),320,235,buffer );
	SHOW_SCREEN
	mouse->SetCursor ( CHand );
	mouse->draw ( false,screen );

	while ( ClientSettingsList->Count<PlayerList->Count-1 )
	{
		// Events holen:
		SDL_PumpEvents();
		// Die Maus machen:
		mouse->GetPos();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
		HandleMenuMessages();
	}

	// Alle Upgrades übertragen:
	for ( i=0;i<PlayerList->Count;i++ )
	{
		TransmitPlayerUpgrades ( PlayerList->PlayerItems[i] );
	}

	TransmitPlayerLanding ( MyPlayer->Nr,LandX,LandY,LandingList );
	game->MakeLanding ( LandX,LandY,MyPlayer,LandingList,options.FixedBridgeHead );

	// Alle Landungen übertragen:
	while ( ClientSettingsList->Count )
	{
		sClientSettings *cs;
		cPlayer *p;
		cs=ClientSettingsList->ClientSettingsItems[0];

		TransmitPlayerLanding ( cs->nr,cs->LandX,cs->LandY,cs->LandingList );

		for ( i=0;i<PlayerList->Count;i++ )
		{
			p=PlayerList->PlayerItems[i];
			if ( p->Nr==cs->nr ) break;
		}
		game->MakeLanding ( cs->LandX,cs->LandY,p,cs->LandingList,options.FixedBridgeHead );

		while ( cs->LandingList->Count )
		{
			sLanding *l;
			l=cs->LandingList->LandItems[0];
			delete l;
			cs->LandingList->DeleteLanding ( 0 );
		}
		delete cs;
		ClientSettingsList->DeleteClientSettings ( 0 );
	}
	delete ClientSettingsList;

	// Die Ressourcen übertragen:
	TransmitRessources();

	fstcpip->FSTcpIpSend ( MSG_LETS_GO,"" );
}

// Überträgt alle Settings und wartet auf die Daten des Servers:
void cMultiPlayer::ClientWait ( int LandX,int LandY,TList *LandingList )
{
	int lx=-1,ly=-1;
	int i;
	fonts->OutTextBigCenter ( lngPack.Translate ( "Text~Game_MP~Comp_Waiting" ).c_str(),320,235,buffer );
	SHOW_SCREEN
	mouse->SetCursor ( CHand );
	mouse->draw ( false,screen );

	ClientSettingsList=new TList;
	TransmitPlayerUpgrades ( MyPlayer );
	TransmitPlayerLanding ( MyPlayer->Nr,LandX,LandY,LandingList );

	while ( !LetsGo )
	{
		// Events holen:
		SDL_PumpEvents();
		// Die Maus machen:
		mouse->GetPos();
		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
		// Look for messages and handle them
		if ( fstcpip->iStatus==STAT_CONNECTED && fstcpip->bReceiveThreadFinished )
		{
			SDL_WaitThread ( fstcpip->FSTcpIpReceiveThread, NULL ); // free the last memory allocated by the thread. If not done so, SDL_CreateThread will hang after about 1010 successfully created threads
			fstcpip->FSTcpIpReceiveThread = SDL_CreateThread ( Receive,NULL );
		}
		HandleMenuMessages();
	}

	// Alle Landungen durchführen:
	game->MakeLanding ( LandX,LandY,MyPlayer,LandingList,options.FixedBridgeHead );
	while ( ClientSettingsList->Count )
	{
		sClientSettings *cs;
		cPlayer *p;
		cs=ClientSettingsList->ClientSettingsItems[0];

		for ( i=0;i<PlayerList->Count;i++ )
		{
			p=PlayerList->PlayerItems[i];
			if ( p->Nr==cs->nr ) break;
		}
		game->MakeLanding ( cs->LandX,cs->LandY,p,cs->LandingList,options.FixedBridgeHead );

		while ( cs->LandingList->Count )
		{
			sLanding *l;
			l=cs->LandingList->LandItems[0];
			delete l;
			cs->LandingList->DeleteLanding ( 0 );
		}
		delete cs;
		ClientSettingsList->DeleteClientSettings ( 0 );
	}
	delete ClientSettingsList;
}

// Überträgt alle Upgrades dieses Players:
void cMultiPlayer::TransmitPlayerUpgrades ( cPlayer *p )
{
	string msg;
	char sztmp[256];
	int i;
	sprintf ( sztmp,"%d",p->Nr );
	msg=sztmp;

	for ( i=0;i<UnitsData.vehicle_anz;i++ )
	{
		if ( p->VehicleData[i].damage!=UnitsData.vehicle[i].data.damage||
		        p->VehicleData[i].max_shots!=UnitsData.vehicle[i].data.max_shots||
		        p->VehicleData[i].range!=UnitsData.vehicle[i].data.range||
		        p->VehicleData[i].max_ammo!=UnitsData.vehicle[i].data.max_ammo||
		        p->VehicleData[i].armor!=UnitsData.vehicle[i].data.armor||
		        p->VehicleData[i].max_hit_points!=UnitsData.vehicle[i].data.max_hit_points||
		        p->VehicleData[i].scan!=UnitsData.vehicle[i].data.scan||
		        p->VehicleData[i].max_speed!=UnitsData.vehicle[i].data.max_speed )
		{
			if ( msg.length() >0 ) msg+="#";
			msg+="0";msg+="#";
			sprintf ( sztmp,"%d",i ); msg+=sztmp; msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].damage ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].max_shots ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].range ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].max_ammo ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].armor ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].max_hit_points ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].scan ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->VehicleData[i].max_speed ); msg+=sztmp;
		}
		if ( msg.length() >200 )
		{
			fstcpip->FSTcpIpSend ( MSG_PLAYER_UPGRADES,msg.c_str() );
			SDL_Delay ( 1 );
			msg="";
		}
	}

	for ( i=0;i<UnitsData.building_anz;i++ )
	{
		if ( p->BuildingData[i].damage!=UnitsData.building[i].data.damage||
		        p->BuildingData[i].max_shots!=UnitsData.building[i].data.max_shots||
		        p->BuildingData[i].range!=UnitsData.building[i].data.range||
		        p->BuildingData[i].max_ammo!=UnitsData.building[i].data.max_ammo||
		        p->BuildingData[i].armor!=UnitsData.building[i].data.armor||
		        p->BuildingData[i].max_hit_points!=UnitsData.building[i].data.max_hit_points||
		        p->BuildingData[i].scan!=UnitsData.building[i].data.scan )
		{
			if ( msg.length() >0 ) msg+="#";
			msg+="1";msg+="#";
			sprintf ( sztmp,"%d",i ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].damage ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].max_shots ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].range ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].max_ammo ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].armor ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].max_hit_points ); msg+=sztmp;msg+="#";
			sprintf ( sztmp,"%d",p->BuildingData[i].scan ); msg+=sztmp;msg+="#";
		}
		if ( msg.length() >200 )
		{
			fstcpip->FSTcpIpSend ( MSG_PLAYER_UPGRADES,msg.c_str() );
			SDL_Delay ( 1 );
			msg="";
		}
	}
	if ( msg.length() >1 )
	{
		fstcpip->FSTcpIpSend ( MSG_PLAYER_UPGRADES,msg.c_str() );
		SDL_Delay ( 1 );
	}
}

// Überträgt die Landungsdaten des Players:
void cMultiPlayer::TransmitPlayerLanding ( int nr,int x,int y,TList *ll )
{
	string msg;
	char sztmp[256];
	int i;
	sprintf ( sztmp,"%d",x );msg=sztmp;msg+="#";
	sprintf ( sztmp,"%d",y );msg+=sztmp;msg+="#";
	sprintf ( sztmp,"%d",nr );msg+=sztmp;msg+="#";
	sprintf ( sztmp,"%d",ll->Count );msg+=sztmp;
	for ( i=0;i<ll->Count;i++ )
	{
		sLanding *l;
		l=ll->LandItems[i];
		if ( msg.length() >0 ) msg+="#";
		sprintf ( sztmp,"%d",l->id );msg+=sztmp;msg+="#";
		sprintf ( sztmp,"%d",l->cargo );msg+=sztmp;
		if ( msg.length() >200 )
		{
			fstcpip->FSTcpIpSend ( MSG_PLAYER_LANDING,msg.c_str() );
			SDL_Delay ( 1 );
			msg="";
		}
	}
	if ( msg.length() >0 )
	{
		fstcpip->FSTcpIpSend ( MSG_PLAYER_LANDING,msg.c_str() );
		SDL_Delay ( 1 );
	}
}

// Prüft, ob die Spielerliste ok ist:
bool cMultiPlayer::TestPlayerList ( void )
{
	int i,k;
	for ( i=0;i<PlayerList->Count;i++ )
	{
		for ( k=0;k<PlayerList->Count;k++ )
		{
			if ( i==k ) continue;
			if ( strcmp ( PlayerList->PlayerItems[i]->name.c_str(),PlayerList->PlayerItems[k]->name.c_str() ) ==0 )
			{
				string log;
				log=lngPack.Translate ( "Text~Game_MP~Comp_Player_Twice" );
				log+=PlayerList->PlayerItems[i]->name;
				AddChatLog ( log );
				return false;
			}
		}
	}
	return true;
}

// Prüft, ob alle Spieler aus dem Savegame da sind:
bool cMultiPlayer::TestPlayerListLoad ( void )
{
	int i,k,found;
	if ( SaveGame.empty() ) return false;

	if ( PlayerList->Count>game->PlayerList->Count )
	{
		AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Player_Many" ) );
		return false;
	}
	if ( PlayerList->Count<game->PlayerList->Count )
	{
		AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Player_Few" ) );
		return false;
	}

	found=0;
	for ( i=0;i<PlayerList->Count;i++ )
	{
		cPlayer *a;
		a=PlayerList->PlayerItems[i];
		for ( k=0;k<game->PlayerList->Count;k++ )
		{
			cPlayer *b;
			b=game->PlayerList->PlayerItems[k];
			if ( strcmp ( b->name.c_str(),a->name.c_str() ) ==0 ) {found++;break;}
		}
	}
	if ( found!=PlayerList->Count )
	{
		AddChatLog ( lngPack.Translate ( "Text~Game_MP~Comp_Player_Wrong" ) );
		return false;
	}

	if ( game->PlayRounds && strcmp ( game->PlayerList->PlayerItems[0]->name.c_str(),MyPlayer->name.c_str() ) )
	{
		string log;
		lngPack.Translate ( "Text~Game_MP~Title_Player" );
		log+=" ";
		log+=game->PlayerList->PlayerItems[0]->name.c_str();
		log+=" ";
		log+=lngPack.Translate ( "Text~Game_MP~Comp_Player_MustHost" );
		AddChatLog ( log );
		return false;
	}

	return true;
}

// Startet ein Hot-Seat-Spiel:
void HeatTheSeat ( void )
{
	string stmp;
	char sztmp[32];
	// Anzahl der Spieler holen:
	int PlayerAnz;
	PlayerAnz=ShowNumberInput ( lngPack.Translate ( "Text~Game_MP~Comp_Player_Amount" ), MAXPLAYER_HOTSEAT, 2 );
	if ( PlayerAnz<2 ) PlayerAnz=2;

	// Spiel erstellen:
	string MapName;
	MapName=RunPlanetSelect();
	if ( MapName.empty() ) return;

	TList *list,*LandingList;
	int i,LandX,LandY;
	sOptions options;
	cPlayer *p;
	cMap *map;

	map=new cMap;
	if ( !map->LoadMap ( MapName ) )
	{
		delete map;
		return;
	}
	options=RunOptionsMenu ( NULL );

	map->PlaceRessources ( options.metal,options.oil,options.gold,options.dichte );

	list=new TList;
	for ( i=1;i<=PlayerAnz;i++ )
	{
		stmp = lngPack.Translate ( "Text~Game_MP~Title_Player" );
		sprintf ( sztmp,"%d",i );
		stmp+=sztmp;
		list->AddPlayer ( p=new cPlayer ( stmp,OtherData.colors[ ( i-1 ) %8],i ) );
		p->Credits=options.credits;
	}

	game=new cGame ( NULL, map );
	game->AlienTech=options.AlienTech;
	game->PlayRounds=options.PlayRounds;
	game->ActiveRoundPlayerNr=p->Nr;
	game->Init ( list,0 );

	for ( i=0;i<list->Count;i++ )
	{
		p=list->PlayerItems[i];
		p->InitMaps ( map->size );

		stmp=p->name; stmp+=lngPack.Translate ( "Text~Game_MP~Comp_Player_Turn" );
		ShowOK ( stmp,true );

		LandingList=new TList;
		RunHangar ( p,LandingList );

		SelectLanding ( &LandX,&LandY,map );
		game->ActivePlayer=p;
		game->MakeLanding ( LandX,LandY,p,LandingList,options.FixedBridgeHead );
		p->HotHud=* ( game->hud );

		while ( LandingList->Count )
		{
			delete LandingList->LandItems[0];
			LandingList->DeleteLanding ( 0 );
		}
		delete LandingList;
	}

	ExitMenu();

	p=list->PlayerItems[0];
	game->ActivePlayer=p;
	* ( game->hud ) =p->HotHud;
	stmp=p->name; stmp+=lngPack.Translate ( "Text~Game_MP~Comp_Player_Turn" );
	ShowOK ( stmp,true );
	game->HotSeat=true;
	game->HotSeatPlayer=0;
	game->Run();

	if(!game->HotSeat) //don't store playername in hotseat games (would crash, too)
	{
		SettingsData.sPlayerName=p->name; 
	}
	while ( list->Count )
	{
		list->Delete ( 0 );
	}
	delete game;game=NULL;
	delete map;
	delete list;
}

// Zeigt das Laden Menü an:
int ShowDateiMenu ( bool bSave )
{
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOG_W / 2, SettingsData.iScreenH / 2 - DIALOG_H / 2, DIALOG_W, DIALOG_H };
	SDL_Rect scr;
	int LastMouseX=0,LastMouseY=0,LastB=0,x,b,y,offset=0,selected=-1;
	bool SpeichernPressed=false, FertigPressed=false, UpPressed=false, DownPressed=false;
	bool  BeendenPressed=false, HilfePressed=false, LadenPressed=false, Cursor=true;
	Uint8 *keystate;
	TList *files;
	SDL_Rect rBtnBack = {rDialog.x+353,rDialog.y+438, 106, 40};
	SDL_Rect rBtnExit = {rDialog.x+246,rDialog.y+438, 106, 40};
	SDL_Rect rBtnLoad = {rDialog.x+514,rDialog.y+438, 106, 40};
	SDL_Rect rBtnSave = {rDialog.x+132,rDialog.y+438, 106, 40};
	SDL_Rect rBtnHelp = {rDialog.x+464,rDialog.y+438, 40, 40};
	SDL_Rect rArrowUp = {rDialog.x+33, rDialog.y+438, 28, 29};
	SDL_Rect rArrowDown = {rDialog.x+63, rDialog.y+438, 28, 29};
	SDL_Rect rTitle = { rDialog.x+320, rDialog.y+12, 150, 12 };

	PlayFX ( SoundData.SNDHudButton );
	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	// Den Bildschirm blitten:
	SDL_BlitSurface ( GraphicsData.gfx_load_save_menu,NULL,buffer,&rDialog );
	// Den Text anzeigen:
	if ( bSave ) fonts->OutTextCenter ( lngPack.Translate( "Text~Game_Start~Title_LoadSave").c_str(),rTitle.x,rTitle.y,buffer );
	else fonts->OutTextCenter ( lngPack.Translate ( "Text~Game_Start~Title_Load" ).c_str(),rTitle.x,rTitle.y,buffer );
	// Buttons setzen;
	drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Back" ),false,rBtnBack.x,rBtnBack.y,buffer );	
	// PlaceSmallMenuButton ( "? ",rBtnHelp.x,rBtnHelp.y,false ); //TODO: move this to dialog.cpp and rewrite it
	if ( bSave )
	{
		drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Save" ),false,rBtnSave.x,rBtnSave.y,buffer );
		drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),false,rBtnExit.x,rBtnExit.y,buffer );
	}
	else
	{
		drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Load" ),false,rBtnLoad.x,rBtnLoad.y,buffer );
	}
	//BEGIN ARROW CODE
	scr.y=40;
	scr.w=28;
	scr.h=29;
	scr.x=96;
	SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowUp );
	scr.x=96+28*2;
	SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowDown );
	//END ARROW CODE
	// Dateien suchen und Anzeigen:
	files = getFilesOfDirectory ( SettingsData.sSavesPath );
	for ( int i = 0; i < files->Count; i++ )
	{
		if( files->Items[i].substr( files->Items[i].length() -3, 3 ).compare ( "sav" ) != 0 )
		{
			files->Delete ( i );
			i--;
		}
	}
	ShowFiles ( files,offset,selected,false,false,false, rDialog );
	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );
	while ( 1 )
	{
		// Events holen:
		SDL_PumpEvents();

		// Tasten prüfen:
		if ( bSave ) game->HandleTimer();
		keystate=SDL_GetKeyState ( NULL );
		if ( keystate[SDLK_ESCAPE] )
		{
			InputStr="";
			break;
		}

		if ( DoKeyInp ( keystate ) || timer2 )
		{
			if ( Cursor )
			{
				Cursor=false;
				ShowFiles ( files,offset,selected,true,true,false, rDialog );
			}
			else
			{
				Cursor=true;
				ShowFiles ( files,offset,selected,true,false,false, rDialog );
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}
		// Klick auf einen Speicher:
		if ( ( x >= rDialog.x+15 && x < rDialog.x+15 + 205 && y > rDialog.y+45 && y <  rDialog.y+45 + 375 ) || ( x >=  rDialog.x+417 && x <  rDialog.x+417 + 205 && y >  rDialog.y+45 && y <  rDialog.y+45 + 375 ) )
		{
			if ( b&&!LastB )
			{
				InputStr = "";
				int checkx = rDialog.x+15, checky = rDialog.y+45;
				for ( int i = 0; i<10; i++ )
				{
					if ( i==5 )
					{
						checkx=rDialog.x+418;
						checky=rDialog.y+45;
					}
					if ( x>=checkx&&x<checkx+205&&y>checky&&y<checky+73 )
						selected = i+offset;
					checky+=75;
				}
				ShowFiles ( files,offset,selected,bSave,false,true, rDialog );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Fertig-Button:
		if ( x >= rBtnBack.x && x < rBtnBack.x + rBtnBack.w && y >= rBtnBack.y && y < rBtnBack.y + rBtnBack.h )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Back" ),true,rBtnBack.x,rBtnBack.y,buffer );	
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				return -1;
			}
		}
		else if ( FertigPressed )
		{
			drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Back" ),false,rBtnBack.x,rBtnBack.y,buffer );	
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Beenden-Button:
		if (  bSave && ( x >= rBtnExit.x && x < rBtnExit.x + rBtnExit.w && y >= rBtnExit.y && y < rBtnExit.y + rBtnExit.h ) )
		{
			if ( b&&!BeendenPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),true,rBtnExit.x,rBtnExit.y,buffer );	
				SHOW_SCREEN
				mouse->draw ( false,screen );
				BeendenPressed=true;
			}
			else if ( !b&&LastB )
			{
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),false,rBtnExit.x,rBtnExit.y,buffer );	
				SHOW_SCREEN
				mouse->draw ( false,screen );
				BeendenPressed=false;
				game->End = true;
				return -1;
			}
		}
		else if ( BeendenPressed )
		{
			drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Exit" ),false,rBtnExit.x,rBtnExit.y,buffer );	
			SHOW_SCREEN
			mouse->draw ( false,screen );
			BeendenPressed=false;
		}
		// Speichern-Button:
		if ( bSave && ( x >= rBtnSave.x && x < rBtnSave.x + rBtnSave.w && y >= rBtnSave.y && y < rBtnSave.y + rBtnSave.h ) )
		{
			if ( b&&!SpeichernPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Save" ),true,rBtnSave.x,rBtnSave.y,buffer );	
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SpeichernPressed=true;
			}
			else if ( !b&&LastB )
			{
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Save" ),false,rBtnSave.x,rBtnSave.y,buffer );	
				if ( selected != -1 )
				{
					ShowFiles ( files,offset,selected,true,false,false, rDialog );
					if ( game->Save ( SaveLoadFile, SaveLoadNumber ) )
					{
						files = getFilesOfDirectory ( SettingsData.sSavesPath );
						for ( int i = 0; i < files->Count; i++ )
						{
							if( files->Items[i].substr( files->Items[i].length() -3, 3 ).compare ( "sav" ) != 0 )
							{
								files->Delete ( i );
								i--;
							}
						}
						selected = -1;
					}
					ShowFiles ( files,offset,selected,true,false,false, rDialog );
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SpeichernPressed=false;
			}
		}
		else if ( SpeichernPressed )
		{
			drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Save" ),false,rBtnSave.x,rBtnSave.y,buffer );	
			SHOW_SCREEN
			mouse->draw ( false,screen );
			SpeichernPressed=false;
		}
		// Laden-Button:
		if ( !bSave && ( x >= rBtnLoad.x && x < rBtnLoad.x + rBtnLoad.w && y >= rBtnLoad.y && y < rBtnLoad.y + rBtnLoad.h ) )
		{
			if ( b&&!LadenPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Load" ),true,rBtnLoad.x,rBtnLoad.y,buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				LadenPressed=true;
			}
			else if ( !b&&LastB )
			{
				drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Load" ),false,rBtnLoad.x,rBtnLoad.y,buffer );
				if ( selected != -1 )
				{
					ShowFiles ( files,offset,selected,false,false,false, rDialog );
					return 1;
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
				LadenPressed=false;
			}
		}
		else if ( LadenPressed )
		{
			drawButtonBig( lngPack.Translate ( "Text~Menu_Main~Button_Load" ),false,rBtnLoad.x,rBtnLoad.y,buffer );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			LadenPressed=false;
		}
		// Up-Button:
		if ( x >= rArrowUp.x && x < rArrowUp.x + rArrowUp.w && y >= rArrowUp.y && y < rArrowUp.y + rArrowUp.h )
		{
			if ( b&&!UpPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=96+28;
				SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowUp );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				UpPressed=true;
			}
			else if ( !b&&LastB )
			{
				if ( offset>0 )
				{
					offset-=10;
					selected=-1;
				}
				ShowFiles ( files,offset,selected,false,false,false, rDialog );
				scr.x=96;
				SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowUp );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				UpPressed=false;
			}
		}
		else if ( UpPressed )
		{
			scr.x=96;
			SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowUp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=false;
		}
		// Down-Button:
		if ( x >= rArrowDown.x && x < rArrowDown.x + rArrowDown.w && y >= rArrowDown.y && y < rArrowDown.y + rArrowDown.h )
		{
			if ( b&&!DownPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=96+28*3;
				SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowDown );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				DownPressed=true;
			}
			else if ( !b&&LastB )
			{
				if ( offset<90 )
				{
					offset+=10;
					selected=-1;
				}
				ShowFiles ( files,offset,selected,false,false,false, rDialog );
				scr.x=96+28*2;
				SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowDown );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				DownPressed=false;
			}
		}
		else if ( DownPressed )
		{
			scr.x=96+28*2;
			SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowDown );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=false;
		}

		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
	delete files;
	return -1;
}

void loadMenudatasFromSave ( string sFileName, string *sTime, string *sSavegameName, string *sMode )
{
	SDL_RWops *pFile;
	int iLenght;
	char *szBuffer;
	string sTmp;

	if ( ( pFile = SDL_RWFromFile( ( SettingsData.sSavesPath + PATH_DELIMITER + sFileName ).c_str(),"rb" ) ) == NULL )
	{
		cLog::write ( "Can't open Savegame: " + sFileName, LOG_TYPE_WARNING );
		return ;
	}

	// Read time
	if ( sTime != NULL )
	{
		SDL_RWread(pFile, &iLenght, sizeof ( int ), 1);
		szBuffer = ( char* ) malloc ( iLenght );
		SDL_RWread(pFile, szBuffer, sizeof ( char ), iLenght);
		sTmp = szBuffer;
		*sTime = sTmp;
		free ( szBuffer );
	}
	else
	{
		SDL_RWread(pFile, &iLenght, sizeof ( int ), 1);
		SDL_RWseek(pFile, iLenght, SEEK_CUR);
	}

	// Read name
	if ( sSavegameName != NULL )
	{
		SDL_RWread(pFile, &iLenght, sizeof ( int ), 1);
		szBuffer = ( char* ) malloc ( iLenght );
		SDL_RWread(pFile, szBuffer, sizeof ( char ), iLenght);
		sTmp = szBuffer;
		*sSavegameName = sTmp;
		free ( szBuffer );
	}
	else
	{
		SDL_RWread(pFile, &iLenght, sizeof ( int ), 1);
		SDL_RWseek(pFile, iLenght, SEEK_CUR);
	}

	// Read mode
	if ( sMode != NULL )
	{
		szBuffer = ( char* ) malloc ( 4 );
		SDL_RWread(pFile, szBuffer, sizeof ( char ), 4);
		sTmp = szBuffer;
		*sMode = sTmp;
		free ( szBuffer );
	}

	SDL_RWclose ( pFile );
}

// Zeigt die Saves an
void ShowFiles ( TList *files, int offset, int selected, bool bSave, bool bCursor, bool bFirstSelect, SDL_Rect rDialog )

{
	SDL_Rect rect, src;
	int i, x = rDialog.x + 35, y = rDialog.y + 72;
	// Save Nummern ausgeben
	rect.x = rDialog.x + (src.x = 25);
	rect.y = rDialog.y + (src.y = 70);
	rect.w = src.w = 26;
	rect.h = src.h = 16;

	//redraw numbers
	for ( i = 0; i < 10; i++ )
	{
		if ( i == 5 )
		{
			rect.x += 398;
			src.x += 398;
			rect.y = rDialog.y + (src.y=70);
			x = rDialog.x + 435;
			y = rDialog.y + 72;
		}

		SDL_BlitSurface ( GraphicsData.gfx_load_save_menu, &src, buffer, &rect );
		if ( i + offset == selected )
		{
			fonts->OutTextBigCenterGold ( iToStr ( offset + i + 1 ), x, y, buffer );
		}
		else
		{
			fonts->OutTextBigCenter ( iToStr ( offset + i + 1 ), x, y, buffer );
		}

		rect.y += 76;
		src.y += 76;
		y += 76;
	}

	// Savenamen mit evtl. Auswahl ausgeben
	rect.x = rDialog.x + (src.x=55);
	rect.y = rDialog.y + (src.y=59);
	rect.w = src.w = 153;
	rect.h = src.h = 41;

	//redraw black bars in save slots
	for ( i = 0; i < 10; i++ )
	{
		if ( i == 5 )
		{
			rect.x += 402;
			src.x += 402;
			rect.y = rDialog.y + (src.y=59);
		}

		SDL_BlitSurface ( GraphicsData.gfx_load_save_menu, &src, buffer, &rect );

		rect.y += 76;
		src.y += 76;
	}

	x = rDialog.x + 60;
	y = rDialog.y + 87;
	selected++;

	for ( i = offset + 1; i <= 10 + offset; i++ )
	{
		if ( i == offset + 6 )
		{
			x += 402;
			y = rDialog.y + 87;
		}
		if ( bSave )
		{
			for ( int j = 0; j < files->Count || j == 0; j++ )
			{
				int iSaveNumber;
				string sFilename, sTime, sMode;
				if ( files->Count > 0 )
				{
					iSaveNumber = atoi( files->Items[j].substr ( files->Items[j].length() - 7, 3 ).c_str() );
					loadMenudatasFromSave ( files->Items[j], &sTime, &sFilename, &sMode );
				}
				else
				{
					iSaveNumber = -1;
				}
				if ( iSaveNumber == i )
				{
					// Dateinamen anpassen und ausgeben
					if ( sFilename.length() > 15 )
					{
						sFilename.erase ( 15 );
					}
					if ( i == selected )
					{
						if ( bFirstSelect )
						{
							InputStr = sFilename;
						}
						else
						{
							sFilename = InputStr;
						}
						if ( bCursor )
						{
							sFilename += "_";
						}
						SaveLoadFile = InputStr;
						SaveLoadNumber = i;
					}
					fonts->OutText ( ( char * ) sFilename.c_str(),x,y,buffer );
					// Zeit und Modus ausgeben
					fonts->OutText ( ( char * ) sTime.c_str(),x,y-23,buffer );
					fonts->OutText ( ( char * ) sMode.c_str(),x+113,y-23,buffer );
					break;
				}
				else if ( i == selected )
				{
					if ( InputStr.length() > 15 )
					{
						InputStr.erase ( 15 );
					}
					sFilename = InputStr;
					if ( bCursor )
					{
						sFilename += "_";
					}
					SaveLoadFile = InputStr;
					SaveLoadNumber = i;
					fonts->OutText ( ( char * ) sFilename.c_str(),x,y,buffer );
				}
			}
		}
		else
		{
			for ( int j = 0; j < files->Count; j++ )
			{
				int iSaveNumber;
				string sFilename, sTime, sMode;
				iSaveNumber = atoi ( files->Items[j].substr ( files->Items[j].length() - 7, 3 ).c_str() );
				loadMenudatasFromSave ( files->Items[j], &sTime, &sFilename, &sMode );

				if ( iSaveNumber == i )
				{
					// Dateinamen anpassen und ausgeben
					if ( sFilename.length() > 15 )
					{
						sFilename.erase ( 15 );
					}

					if ( i == selected )
					{
						SaveLoadFile = files->Items[j];
					}

					fonts->OutText ( sFilename, x, y, buffer );

					// Zeit und Modus ausgeben
					fonts->OutText ( sTime, x, y - 23, buffer );
					fonts->OutText ( sMode, x + 113, y - 23, buffer );
					break;
				}
			}
		}
		y += 76;
	}
}
