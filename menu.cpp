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
#include "buttons.h"
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
#include "events.h"
#include "client.h"
#include "server.h"

#define DIALOG_W 640
#define DIALOG_H 480
#define DIALOG_X (SettingsData.iScreenW / 2 - DIALOG_W / 2)
#define DIALOG_Y (SettingsData.iScreenH / 2 - DIALOG_H / 2)
#define TITLE_X DIALOG_X+320
#define TITLE_Y DIALOG_Y+147
#define INFO_IMG_X DIALOG_X+16
#define INFO_IMG_Y DIALOG_Y+182
#define INFO_IMG_WIDTH  300
#define INFO_IMG_HEIGHT 240

#define BTN_SPACE 35
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


/**
 * Shows a vehicle or a building in the mainscreen. There's a 33% chance that
 * it shows a building and a 66% chance to show a vehicle. A unit won't be
 * shown twice in order.
 * @author beko
 */
static void showUnitPicture(void);

// Menü vorbereiten:
static void prepareMenu()
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
	font->showTextCentered(DIALOG_X+320,DIALOG_Y+465, lngPack.i18n ( "Text~Main~Credits_Reloaded" )+ " "+MAX_VERSION);
	//END MENU REDRAW

	//display random unit
	showUnitPicture();

	//show mouse
	mouse->Show();
	mouse->SetCursor ( CHand );
}

// shows the randomized unit picture
static void showUnitPicture(void)
{
	/**To randomize whether to show a vehicles or a building*/
	int const iShowBuilding = random(3);
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
			iUnitShow = random(UnitsData.building.Size());
		}
		while ( iUnitShow == s_iLastUnitShown );	//make sure we don't show same unit twice
		SDL_BlitSurface ( UnitsData.building[iUnitShow].info,NULL,buffer,&rDest );
	}
	else //and a 66% chance to show a vehicle on 0 or 2
	{
		do
		{
			iUnitShow = random(UnitsData.vehicle.Size());
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


// Platziert einen auswählbaren Text (zentriert):
void placeSelectableText ( string sText,int x,int y,bool checked, SDL_Surface *surface,bool center )
{
	SDL_Rect r;
	int len;
	len = font->getTextWide(sText);
	if ( center )
	{
		font->showTextCentered(x, y, sText);
	}
	else
	{
		font->showText(x, y, sText);
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


static void CheckUnitImageClick(int const x, int const y, bool const down)
{
	if (!down) return;
	if (x < INFO_IMG_X || INFO_IMG_X + INFO_IMG_WIDTH  <= x) return;
	if (y < INFO_IMG_Y || INFO_IMG_Y + INFO_IMG_HEIGHT <= y) return;
	PlayFX(SoundData.SNDObjectMenu);
	showUnitPicture();
	SHOW_SCREEN
	mouse->draw(false, screen);
}


// Zeigt das Hauptmenü an:
void RunMainMenu ( void )
{
	bool EscHot=true;
	Uint8 *keystate;
	int b,lb=0,lx=-1,ly=-1;
	// start main musicfile
	PlayMusic((SettingsData.sMusicPath + PATH_DELIMITER + "main.ogg").c_str());

	MenuButton btn_single( BTN_1_X, BTN_1_Y, "Text~Button~Single_Player");
	MenuButton btn_multi(  BTN_2_X, BTN_2_Y, "Text~Button~Multi_Player");
#if 0
	MenuButton btn_editor( BTN_3_X, BTN_3_Y, "Text~Button~Map_Editor");
	MenuButton btn_credits(BTN_4_X, BTN_4_Y, "Text~Button~Credits");
#endif
	MenuButton btn_license(BTN_5_X, BTN_5_Y, "Text~Button~Mani");
	MenuButton btn_exit(   BTN_6_X, BTN_6_Y, "Text~Button~Exit");

	bool redraw = true;
	while ( 1 )
	{
		if (redraw)
		{
			prepareMenu();
			font->showTextCentered(TITLE_X, TITLE_Y, lngPack.i18n ("Text~Title~MainMenu"));
			btn_single.Draw();
			btn_multi.Draw();
#if 0
			btn_editor.Draw();
			btn_credits.Draw();
#endif
			btn_license.Draw();
			btn_exit.Draw();
			SHOW_SCREEN
			redraw = false;
		}

		// Events holen:
		EventHandler->HandleEvents();
		// Tasten prüfen:
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE]&&EscHot ) break;else if ( !keystate[SDLK_ESCAPE] ) EscHot=true;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		CheckUnitImageClick(x, y, down);

		if (btn_single.CheckClick(x, y, down, up))
		{
			RunSPMenu();
			EscHot = false;
			redraw = true;
		}
		if (btn_multi.CheckClick(x, y, down, up))
		{
			RunMPMenu();
			EscHot = false;
			redraw = true;
		}
#if 0
		if (btn_editor.CheckClick(x, y, down, up))
		{
			ExitMenu();
			{ cMapEditor me; me.Run(); }
			EscHot = false;
			redraw = true;
		}
		if (btn_credits.CheckClick(x, y, down, up))
		{
			{ cCredits cred; cred.Run(); }
			EscHot = false;
			redraw = true;
		}
#endif
		if (btn_license.CheckClick(x, y, down, up))
		{
			mouse->draw(false, screen);
			showLicence();
			redraw = true;
		}
		if (btn_exit.CheckClick(x, y, down, up))
		{
			break;
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
	Uint8 *keystate;
	int b,lb=0,lx=-1,ly=-1;

	prepareMenu();
	font->showTextCentered(TITLE_X, TITLE_Y, lngPack.i18n ( "Text~Button~Multi_Player" ));

	MenuButton btn_host(    BTN_1_X, BTN_1_Y, "Text~Button~TCPIP_Host");
	MenuButton btn_client(  BTN_2_X, BTN_2_Y, "Text~Button~TCPIP_Client");
	MenuButton btn_new_hot( BTN_3_X, BTN_3_Y, "Text~Button~HotSeat_New");
	MenuButton btn_load_hot(BTN_4_X, BTN_4_Y, "Text~Button~HotSeat_Load");
	MenuButton btn_back    (BTN_6_X, BTN_6_Y, "Text~Button~Back");

	btn_host.Draw();
	btn_client.Draw();
	btn_new_hot.Draw();
	btn_load_hot.Draw();
	btn_back.Draw();
	SHOW_SCREEN

	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();
		// Tasten prüfen:
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		CheckUnitImageClick(x, y, down);

		if (btn_host.CheckClick(x, y, down, up))
		{
#ifdef RELEASE
			ShowOK(lngPack.i18n("Text~Error_Messages~INFO_Not_Implemented"), true);
#else
			cMultiPlayerMenu m(true);
			MultiPlayerMenu = &m;
			m.runNetworkMenu();
			MultiPlayerMenu = 0;
			break;
#endif
		}
		if (btn_client.CheckClick(x, y, down, up))
		{
#ifdef RELEASE
			ShowOK(lngPack.i18n("Text~Error_Messages~INFO_Not_Implemented"), true);
#else
			cMultiPlayerMenu m(false);
			MultiPlayerMenu = &m;
			m.runNetworkMenu();
			MultiPlayerMenu = 0;
			break;
#endif
		}
		if (btn_new_hot.CheckClick(x, y, down, up))
		{
			HeatTheSeat();
			break;
		}
		if (btn_load_hot.CheckClick(x, y, down, up))
		{
			if (ShowDateiMenu(false) != -1)
			{
				cMap *map;
				map  = new cMap();
				game = new cGame(map);
				ExitMenu();
				if (!SaveLoadFile.empty())
				{
					game->HotSeat = true;
					game->Load(SaveLoadFile, 0);
				}
				delete game; game = NULL;
				delete map;
				break;
			}
			prepareMenu();
			RunSPMenu();
			break;
		}
		if (btn_back.CheckClick(x, y, down, up))
		{
			break;
		}

		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}
}

void RunSPMenu ( void )
{
	Uint8 *keystate;
	int b,lb=0,lx=-1,ly=-1;

	prepareMenu();
	font->showTextCentered(TITLE_X, TITLE_Y, lngPack.i18n("Text~Button~Single_Player"));

	MenuButton btn_train(BTN_1_X, BTN_1_Y, "Text~Button~Training");
	MenuButton btn_new(  BTN_2_X, BTN_2_Y, "Text~Button~Game_New");
	MenuButton btn_load( BTN_3_X, BTN_3_Y, "Text~Button~Game_Load");
	MenuButton btn_back( BTN_6_X, BTN_6_Y, "Text~Button~Back");

	btn_train.Draw();
	btn_new.Draw();
	btn_load.Draw();
	btn_back.Draw();
	SHOW_SCREEN

	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();
		// Tasten prüfen:
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		CheckUnitImageClick(x, y, down);

		if (btn_train.CheckClick(x, y, down, up))
		{
			ShowOK(lngPack.i18n("Text~Error_Messages~INFO_Not_Implemented"), true);
		}
		if (btn_new.CheckClick(x, y, down, up))
		{
			sOptions options;
			string sMapName = "";
			options = RunOptionsMenu ( NULL );
			if ( options.metal == -1 ) break;
			sMapName = RunPlanetSelect();

			if ( !sMapName.empty() )
			{
				int iLandX, iLandY;
				cPlayer *Player;
				cMap Map;
				sPlayer players;
				if ( !Map.LoadMap ( sMapName ) )
				{
					break;
				}
				Map.PlaceRessources ( options.metal,options.oil,options.gold,options.dichte );
				// copy map for server
				cMap ServerMap;
				ServerMap.NewMap( Map.size, Map.TerrainInUse.iCount );
				ServerMap.DefaultWater = Map.DefaultWater;
				ServerMap.MapName = Map.MapName;
				memcpy ( ServerMap.Kacheln, Map.Kacheln, sizeof ( int )*Map.size*Map.size );
				memcpy ( ServerMap.Resources, Map.Resources, sizeof ( sResources )*Map.size*Map.size );
				for ( int i = 0; i < Map.TerrainInUse.iCount; i++ )
				{
					ServerMap.terrain[i].blocked = Map.terrain[i].blocked;
					ServerMap.terrain[i].coast = Map.terrain[i].coast;
					ServerMap.terrain[i].water = Map.terrain[i].water;
				}

				players = runPlayerSelection();

				bool bHavePlayer = false;
				for ( int i = 0; i < 4; i++ ) //check for players
				{
					if( players.what[i] != PLAYER_N )
					{
						bHavePlayer = true;
					}
				}
				if(!bHavePlayer) //no players - break
				{
					ExitMenu();
					break;
				}
				// player for client
				Player = new cPlayer ( SettingsData.sPlayerName.c_str(), OtherData.colors[cl_red], 1, MAX_CLIENTS ); // Socketnumber MAX_CLIENTS for lokal client
				cList<cPlayer*> ClientPlayerList;
				ClientPlayerList.Add ( Player );
				ClientPlayerList.Add ( new cPlayer ( "Player 2", OtherData.colors[cl_green], 2 ) );

				// playerlist for server
				cList<cPlayer*> ServerPlayerList;
				ServerPlayerList.Add ( new cPlayer ( SettingsData.sPlayerName.c_str(), OtherData.colors[cl_red], 1, MAX_CLIENTS ) ); // Socketnumber MAX_CLIENTS for lokal client
				ServerPlayerList.Add ( new cPlayer ( "Player 2", OtherData.colors[cl_green], 2 ) );

				// init client and his player
				Client = new cClient(&Map, &ClientPlayerList);
				Client->initPlayer ( Player );
				for ( int i = 0; i < ClientPlayerList.iCount; i++ )
				{
					ClientPlayerList.Items[i]->InitMaps ( Map.size, &Map );
					ClientPlayerList.Items[i]->Credits = options.credits;
				}

				// init the players of playerlist
				for ( int i = 0; i < ServerPlayerList.iCount; i++ )
				{
					ServerPlayerList.Items[i]->InitMaps ( ServerMap.size, &ServerMap );
					ServerPlayerList.Items[i]->Credits = options.credits;
				}
				// init server
				Server = new cServer(&ServerMap, &ServerPlayerList, GAME_TYPE_SINGLE, false);

				// land the player
				cList<sLanding*> LandingList;
				RunHangar ( Player, &LandingList );
				SelectLanding ( &iLandX, &iLandY, &Map );

				Server->makeLanding ( iLandX, iLandY, ServerPlayerList.Items[0], &LandingList, options.FixedBridgeHead );

				while ( LandingList.iCount )
				{
					delete LandingList.Items[LandingList.iCount - 1];
					LandingList.Delete( LandingList.iCount - 1);
				}

				// exit menu and start game
				ExitMenu();

				Server->bStarted = true;
				Client->run();

				SettingsData.sPlayerName = Player->name;
				while ( ClientPlayerList.iCount )
				{
					delete ( ClientPlayerList.Items[0] );
					ClientPlayerList.Delete ( 0 );
				}

				delete Client; Client = NULL;
				delete Server; Server = NULL;

				break;
			}
			break;
		}
		if (btn_load.CheckClick(x, y, down, up))
		{
			if (ShowDateiMenu(false) != -1)
			{
				cMap *map;
				map  = new cMap();
				game = new cGame(map);
				ExitMenu();
				if (!SaveLoadFile.empty())
				{
					game->Load(SaveLoadFile, 0);
				}
				delete game; game = NULL;
				delete map;
				break;
			}
			prepareMenu();
			RunSPMenu();
			break;
		}
		if (btn_back.CheckClick(x, y, down, up))
		{
			break;
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
#define GAMEOPTIONS lngPack.i18n("Text~Button~Game_Options")

#define LOWEST lngPack.i18n( "Text~Option~Lowest")
#define LOWER lngPack.i18n( "Text~Option~Lower")
#define LOW lngPack.i18n( "Text~Option~Low")
#define MIDDLE lngPack.i18n( "Text~Option~Normal")
#define MUCH lngPack.i18n( "Text~Option~Much")
#define MORE lngPack.i18n( "Text~Option~More")
#define MOST lngPack.i18n( "Text~Option~Most")
#define THIN lngPack.i18n( "Text~Option~Thin")
#define THICK lngPack.i18n( "Text~Option~Thick")
#define ON lngPack.i18n( "Text~Option~On")
#define OFF lngPack.i18n( "Text~Option~Off")
#define DEFINITE lngPack.i18n( "Text~Option~Definite")
#define MOBILE lngPack.i18n( "Text~Option~Mobile")
#define TURNS lngPack.i18n( "Text~Option~Type_Turns")
#define SIMU lngPack.i18n( "Text~Option~Type_Simu")

#define METAL lngPack.i18n( "Text~Title~Metal")
#define OIL lngPack.i18n( "Text~Title~Oil")
#define GOLD lngPack.i18n( "Text~Title~Gold")
#define RESOURCE lngPack.i18n( "Text~Title~Resource_Density")
#define HEAD lngPack.i18n( "Text~Title~BridgeHead")
#define CREDITS lngPack.i18n( "Text~Title~Credits")
#define ALIEN lngPack.i18n( "Text~Title~Alien_Tech")
#define GAMETYPE lngPack.i18n( "Text~Title~Game_Type")
	//beko IS lazy. fact is.

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
	font->showTextCentered(320, 11, GAMEOPTIONS);

	// Ressourcen:
	font->showTextCentered(110,56, lngPack.i18n ( "Text~Title~Resource" ));

	font->showText(17,86, METAL);
	placeSelectableText ( LOW,38,86+16,options.metal==0, sfTmp );
	placeSelectableText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp);
	placeSelectableText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
	placeSelectableText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );

	font->showText(17,124, OIL);
	placeSelectableText ( LOW,38,124+16,options.oil==0, sfTmp );
	placeSelectableText ( MIDDLE,38+45,124+16,options.oil==1 , sfTmp);
	placeSelectableText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
	placeSelectableText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );

	font->showText(17,162, GOLD);
	placeSelectableText ( LOW,38,162+16,options.gold==0, sfTmp );
	placeSelectableText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
	placeSelectableText ( MUCH,38+45*2,162+16,options.gold==2 , sfTmp);
	placeSelectableText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );

	// Credits:
	font->showTextCentered(110+211,56, CREDITS);

	placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
	placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
	placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
	placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
	placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
	placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
	placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );

	// Brückenkopf:
	font->showTextCentered(110+211*2,56, HEAD);

	placeSelectableText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
	placeSelectableText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );

	// AlienTechs:
	font->showTextCentered(110,251, ALIEN);

	placeSelectableText ( ON,38,281,options.AlienTech, sfTmp );
	placeSelectableText ( OFF,38,281+20,!options.AlienTech, sfTmp );

	// Ressourcendichte:
	font->showTextCentered(110+211,251, RESOURCE);

	placeSelectableText ( THIN,110+130,281,options.dichte==0, sfTmp,false );
	placeSelectableText ( MIDDLE,110+130,281+20,options.dichte==1, sfTmp,false );
	placeSelectableText ( THICK,110+130,281+20*2,options.dichte==2, sfTmp,false );
	placeSelectableText ( MOST,110+130,281+20*3,options.dichte==3, sfTmp,false );

	// Spielart:
	font->showTextCentered(110+211*2,251, GAMETYPE);


	placeSelectableText ( SIMU,452,281,!options.PlayRounds, sfTmp,false );
	placeSelectableText ( TURNS,452,281+20,options.PlayRounds, sfTmp,false );

	MenuButton btn_back( 50, 440, "Text~Button~Back");
	MenuButton btn_ok(  390, 440, "Text~Button~OK");

	btn_back.Draw();
	btn_ok.Draw();
	SHOW_SCREEN
	mouse->draw ( false,screen );

	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();
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
			placeSelectableText ( LOW,38,86+16,options.metal==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45&&mouse->x<38+20+45&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=1;
			placeSelectableText ( LOW,38,86+16,options.metal==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*2&&mouse->x<38+20+45*2&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=2;
			placeSelectableText ( LOW,38,86+16,options.metal==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*3&&mouse->x<38+20+45*3&&mouse->y>=86+16-4&&mouse->y<86+16-4+14 )
		{
			options.metal=3;
			placeSelectableText ( LOW,38,86+16,options.metal==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick aufs Öl:
		if ( b&&!lb&&mouse->x>=38-20&&mouse->x<38+20&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=0;
			placeSelectableText ( LOW,38,124+16,options.oil==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45&&mouse->x<38+20+45&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=1;
			placeSelectableText ( LOW,38,124+16,options.oil==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*2&&mouse->x<38+20+45*2&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=2;
			placeSelectableText ( LOW,38,124+16,options.oil==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*3&&mouse->x<38+20+45*3&&mouse->y>=124+16-4&&mouse->y<124+16-4+14 )
		{
			options.oil=3;
			placeSelectableText ( LOW,38,124+16,options.oil==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,124+16,options.oil==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick aufs Gold:
		if ( b&&!lb&&mouse->x>=38-20&&mouse->x<38+20&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=0;
			placeSelectableText ( LOW,38,162+16,options.gold==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45&&mouse->x<38+20+45&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=1;
			placeSelectableText ( LOW,38,162+16,options.gold==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*2&&mouse->x<38+20+45*2&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=2;
			placeSelectableText ( LOW,38,162+16,options.gold==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=38-20+45*3&&mouse->x<38+20+45*3&&mouse->y>=162+16-4&&mouse->y<162+16-4+14 )
		{
			options.gold=3;
			placeSelectableText ( LOW,38,162+16,options.gold==0, sfTmp );
			placeSelectableText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
			placeSelectableText ( MUCH,38+45*2,162+16,options.gold==2, sfTmp );
			placeSelectableText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick auf die Credits:
		if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4&&mouse->y<86-4+20 )
		{
			options.credits=25;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20&&mouse->y<86-4+20+20 )
		{
			options.credits=50;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*2&&mouse->y<86-4+20+20*2 )
		{
			options.credits=100;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*3&&mouse->y<86-4+20+20*3 )
		{
			options.credits=150;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*4&&mouse->y<86-4+20+20*4 )
		{
			options.credits=200;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*5&&mouse->y<86-4+20+20*5 )
		{
			options.credits=250;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=86-4+20*6&&mouse->y<86-4+20+20*6 )
		{
			options.credits=300;
			placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
			placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
			placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
			placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
			placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
			placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Brückenkopf:
		if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=86-4&&mouse->y<86-4+14 )
		{
			options.FixedBridgeHead=false;
			placeSelectableText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
			placeSelectableText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=86-4+20&&mouse->y<86-4+14+20 )
		{
			options.FixedBridgeHead=true;
			placeSelectableText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
			placeSelectableText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// AlienTech:
		if ( b&&!lb&&mouse->x>=30&&mouse->x<38+100&&mouse->y>=281-4&&mouse->y<281-4+14 )
		{
			options.AlienTech=true;
			placeSelectableText ( ON,38,281,options.AlienTech, sfTmp );
			placeSelectableText ( OFF,38,281+20,!options.AlienTech, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=30&&mouse->x<38+100&&mouse->y>=281-4+20&&mouse->y<281-4+14+20 )
		{
			options.AlienTech=false;
			placeSelectableText ( ON,38,281,options.AlienTech, sfTmp );
			placeSelectableText ( OFF,38,281+20,!options.AlienTech, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Ressourcendichte:
		if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281-4&&mouse->y<281-4+20 )
		{
			options.dichte=0;
			placeSelectableText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			placeSelectableText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			placeSelectableText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281+20-4&&mouse->y<281+20-4+20 )
		{
			options.dichte=1;
			placeSelectableText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			placeSelectableText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			placeSelectableText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281+20*2-4&&mouse->y<281+20*2-4+20 )
		{
			options.dichte=2;
			placeSelectableText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			placeSelectableText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			placeSelectableText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=110+130&&mouse->x<110+130+100&&mouse->y>=281+20*3-4&&mouse->y<281+20*3-4+20 )
		{
			options.dichte=3;
			placeSelectableText ( THIN,110+130,281,options.dichte==0,sfTmp,false );
			placeSelectableText ( MIDDLE,110+130,281+20,options.dichte==1,sfTmp,false );
			placeSelectableText ( THICK,110+130,281+20*2,options.dichte==2,sfTmp,false );
			placeSelectableText ( MOST,110+130,281+20*3,options.dichte==3,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Spielart:
		if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=281-4&&mouse->y<281-4+20 )
		{
			options.PlayRounds=false;
			placeSelectableText ( SIMU,452,281,!options.PlayRounds,sfTmp,false );
			placeSelectableText ( TURNS,452,281+20,options.PlayRounds,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=452&&mouse->x<452+100&&mouse->y>=281+20-4&&mouse->y<281+20-4+20 )
		{
			options.PlayRounds=true;
			placeSelectableText ( SIMU,452,281,!options.PlayRounds,sfTmp,false );
			placeSelectableText ( TURNS,452,281+20,options.PlayRounds,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		if (btn_back.CheckClick(x, y, down, up))
		{
			options.metal = -1;
			break;
		}
		if (btn_ok.CheckClick(x, y, down, up))
		{
			break;
		}

		lb=b;
		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}

	SDL_FreeSurface(sfTmp);
	return options;
}


static void ShowPlanets(cList<string>* const files, int const offset, int const selected, SDL_Surface* const surface);


// Startet die Planetenauswahl (gibt den Namen des Planeten zurück):
string RunPlanetSelect ( void )
{
	Uint8 *keystate;
	int b,lb=0,offset=0,selected=-1,i,lx=-1,ly=-1;
	cList<string> *files;
	SDL_Rect scr;

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

	font->showTextCentered(320,11, lngPack.i18n ( "Text~Title~Choose_Planet" ));

	MenuButton btn_back( 50, 440, "Text~Button~Back");
	MenuButton btn_ok(  390, 440, "Text~Button~OK");

	btn_back.Draw();
	btn_ok.Lock();

	files = getFilesOfDirectory ( SettingsData.sMapsPath );
	for ( int i = 0; i < files->iCount; i++ )
	{
		if( files->Items[i].substr( files->Items[i].length() -3, 3 ).compare ( "WRL" ) != 0 && files->Items[i].substr( files->Items[i].length() -3, 3 ).compare ( "wrl" ) != 0 )
		{
			files->Delete ( i );
			i--;
		}
	}

	ShowPlanets ( files,offset,selected, sfTmp );
	mouse->Show();
	mouse->SetCursor ( CHand );
	SHOW_SCREEN


	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();
		// Tasten prüfen:
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x!=lx||mouse->y!=ly )
		{
			mouse->draw ( true,screen );
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		if (btn_ok.CheckClick(x, y, down, up))
		{
			string name = files->Items[selected];
			if (name.substr(name.length() - 4, name.length()).compare(".WRL") != 0 &&
					name.substr(name.length() - 4, name.length()).compare(".wrl") != 0)
			{
				name.replace(name.length() - 3, 3, "map");
			}

			SDL_FreeSurface(sfTmp);
			delete files;
			return name;
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
		if ( mouse->x>=321&&mouse->x<321+24&&mouse->y>=440&&mouse->y<440+24&&b&&!lb&&files->iCount-8-offset>0 )
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
				if ( i+offset>=files->iCount ) break;

				if ( mouse->x>=scr.x&&mouse->x<scr.x+112&&mouse->y>=scr.y&&mouse->y<scr.y+112 )
				{
					selected=i+offset;
					btn_ok.Unlock();
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

		if (btn_back.CheckClick(x, y, down, up))
		{
			break;
		}

		lx=mouse->x;
		ly=mouse->y;
		lb=b;
		SDL_Delay ( 1 );
	}

	delete files;
	SDL_FreeSurface(sfTmp);
	return "";
}


// Zeigt die Planeten an:
static void ShowPlanets(cList<string>* files, int offset, int selected, SDL_Surface* surface)
{
	SDL_Surface *sf;
	//SDL_Rect scr={640, 390, 0, 38},dest;
	SDL_Rect scr={0, 38, 640, 390},dest;
	string sMap;
	string sPath;
	int size;
	SDL_RWops *fp;

	SDL_BlitSurface ( surface,&scr,buffer,&scr );

	scr.x=25; scr.y=90; scr.h=scr.w=112;

	cLog::write ( "Loading Maps", cLog::eLOG_TYPE_INFO );

	for ( int i=0;i<8;i++ ) //only 8 maps on one screen
	{
		if ( i+offset>=files->iCount ) break;
		sMap = files->Items[i+offset];
		sPath = sMap;
		sPath.insert ( 0, PATH_DELIMITER );
		sPath.insert ( 0,SettingsData.sMapsPath );

		if ( FileExists ( sPath.c_str() ) )
		{
			fp = SDL_RWFromFile ( sPath.c_str(),"rb" );
			if ( fp != NULL )
			{
				SDL_RWseek ( fp, 5, SEEK_SET );
				size = SDL_ReadLE16( fp );

				sColor Palette[256];
				short sGraphCount;
				SDL_RWseek ( fp, 2 + size*size*3, SEEK_CUR );
				sGraphCount = SDL_ReadLE16( fp );
				SDL_RWseek ( fp, 64*64*sGraphCount, SEEK_CUR );
				SDL_RWread ( fp, &Palette, 1, 768 );

				sf = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size,8,0,0,0,0);
				sf->pitch = sf->w;

				sf->format->palette->ncolors = 256;
				for (int j = 0; j < 256; j++ )
				{
					sf->format->palette->colors[j].r = Palette[j].cBlue;
					sf->format->palette->colors[j].g = Palette[j].cGreen;
					sf->format->palette->colors[j].b = Palette[j].cRed;
				}
				SDL_RWseek ( fp, 9, SEEK_SET );
				for( int iY = 0; iY < size; iY++ )
				{
					for( int iX = 0; iX < size; iX++ )
					{
						unsigned char cColorOffset;
						SDL_RWread ( fp, &cColorOffset, 1, 1 );
						Uint8 *pixel = (Uint8*) sf->pixels  + (iY * size + iX);
						*pixel = cColorOffset;
					}
				}
				SDL_RWclose ( fp );
			}
			if ( sf!=NULL )
			{
				SDL_BlitSurface ( sf,NULL,buffer,&scr );
			}

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

				sMap.insert ( 0,"> " );
				sMap.replace ( sMap.length()-4, 2, " (" );
				sMap.replace ( sMap.length()-2, 3, iToStr(size) );
				sMap.insert ( sMap.length(),"x" );
				sMap.replace ( sMap.length(), 3, iToStr(size) );
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

				sMap.replace ( sMap.length()-4, 2, " (" );
				sMap.replace ( sMap.length()-2, 3, iToStr(size) );
				sMap.insert ( sMap.length(),"x" );
				sMap.replace ( sMap.length(), 3, iToStr(size) );
				sMap.insert ( sMap.length(),")" );
			}
			font->showTextCentered(scr.x+77-21,scr.y-42, sMap);

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
	if ( files->iCount-8-offset>0 )
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


sPlayerHS runPlayerSelectionHotSeat ( void )
{
	//TODO: add support to define player names
	//TODO: add clan support
	//TODO: interpret _all_ playersettings (not just amount) later
	//TODO: readjust background picture - last box-combos are misaligned on background gfx

	sPlayerHS players;
	#define FIELD1 DIALOG_X+194
	#define FIELD2 DIALOG_X+304
	#define FIELD3 DIALOG_X+414
	int b,lb=0,lx=-1,ly=-1;
	Uint8 *keystate;
	SDL_Rect dest = { DIALOG_X , DIALOG_Y, DIALOG_W, DIALOG_H};
	SDL_Surface *sfTmp = NULL;

	//BEGIN INIT DEFAULT PLAYERS
	for ( int i = 0; i < 8; i++ )
	{
		players.clan[i] = "NONE";
		players.name[i] = lngPack.i18n ( "Text~Multiplayer~Player" ) + " " + iToStr(i+1);
		players.iColor[i] = i;
		players.what[i] = PLAYER_N;
	}
	players.what[0] = PLAYER_H;
	players.name[0] = SettingsData.sPlayerName;
	players.what[1] = PLAYER_H;
	//END INIT DEFAULT PLAYERS

	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_PLAYERHS_SELECT,sfTmp );

 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);

	SDL_BlitSurface (sfTmp, NULL, buffer, &dest);

	font->showTextCentered(DIALOG_X + 320, DIALOG_Y + 11, lngPack.i18n ( "Text~Title~Player_Select" ));
	font->showTextCentered(DIALOG_X + 100,DIALOG_Y + 35, lngPack.i18n ( "Text~Title~Player_Name" ));
	font->showTextCentered(DIALOG_X + 210,DIALOG_Y + 35, lngPack.i18n ( "Text~Title~Human" ));
	font->showTextCentered(DIALOG_X + 320,DIALOG_Y + 35, lngPack.i18n ( "Text~Title~Computer" ));
	font->showTextCentered(DIALOG_X + 430,DIALOG_Y + 35, lngPack.i18n ( "Text~Title~Nobody" ));
	font->showTextCentered(DIALOG_X + 535,DIALOG_Y + 35, lngPack.i18n ( "Text~Title~Clan" ));

	MenuButton btn_back(DIALOG_X +  50, DIALOG_Y + 440, "Text~Button~Back");
	MenuButton btn_ok(  DIALOG_X + 390, DIALOG_Y + 440, "Text~Button~OK");

	btn_back.Draw();
	btn_ok.Draw();

	showPlayerStatesHotSeat ( players );

	mouse->Show();
	mouse->SetCursor ( CHand );
	SHOW_SCREEN
	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] )
		{
			for ( int i = 0; i < 8; i++ ) //reset players and exit
			{
				players.what[i] = PLAYER_N;
			}
			break;
		}
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
			int x = FIELD1;
			int y = DIALOG_Y + 66;

			for ( int i = 0; i < 24; i++ ) //playermatrix 3 * 8 fields
			{
				if ( mouse->x >= x && mouse->x < x + 35 && mouse->y >= y && mouse->y < y + 35 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					if ( i == 0 ) players.what[0] = PLAYER_H;
					if ( i == 1 ) players.what[0] = PLAYER_AI;
					if ( i == 2 ) players.what[0] = PLAYER_N;

					if ( i == 3 ) players.what[1] = PLAYER_H;
					if ( i == 4 ) players.what[1] = PLAYER_AI;
					if ( i == 5 ) players.what[1] = PLAYER_N;

					if ( i == 6 ) players.what[2] = PLAYER_H;
					if ( i == 7 ) players.what[2] = PLAYER_AI;
					if ( i == 8 ) players.what[2] = PLAYER_N;

					if ( i == 9 ) players.what[3] = PLAYER_H;
					if ( i == 10 ) players.what[3] = PLAYER_AI;
					if ( i == 11 ) players.what[3] = PLAYER_N;

					if ( i == 12 ) players.what[4] = PLAYER_H;
					if ( i == 13 ) players.what[4] = PLAYER_AI;
					if ( i == 14 ) players.what[4] = PLAYER_N;

					if ( i == 15 ) players.what[5] = PLAYER_H;
					if ( i == 16 ) players.what[5] = PLAYER_AI;
					if ( i == 17 ) players.what[5] = PLAYER_N;

					if ( i == 18 ) players.what[6] = PLAYER_H;
					if ( i == 19 ) players.what[6] = PLAYER_AI;
					if ( i == 20 ) players.what[6] = PLAYER_N;

					if ( i == 21 ) players.what[7] = PLAYER_H;
					if ( i == 22 ) players.what[7] = PLAYER_AI;
					if ( i == 23 ) players.what[7] = PLAYER_N;
					showPlayerStatesHotSeat( players );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}

				if(x == FIELD1)  //go through columns and/or go to next row on end
					x = FIELD2;
				else if(x == FIELD2)
					x = FIELD3;
				else
				{
					x = FIELD1;
					y += 45;
				}
			}
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		if (btn_ok.CheckClick(x, y, down, up))
		{
			int iPlayers = 0;
			bool bAiFound = false;
			for ( int i = 0; i < 8; i++ ) //check if we have at least two players
			{
				if(players.what[i] == PLAYER_AI)
				{
					players.what[i] = PLAYER_H;
					bAiFound = true;

				}
				showPlayerStatesHotSeat( players );

				if(players.what[i] == PLAYER_H)
				{
					iPlayers++;
				}
			}
			if(iPlayers >= 2)
			{
				if(bAiFound)
				{
					ShowOK ( "AI is currently not supported\n\nChanged AI slot(s) to human players!", true );
				}
				break;
			}
			else
			{
				ShowOK ( lngPack.i18n ( "Text~Multiplayer~Player_Amount" ), true );
			}
		}
		if (btn_back.CheckClick(x, y, down, up))
		{
			for (int i = 0; i < 8; ++i) //reset players to nobody
			{
				players.what[i] = PLAYER_N;
			}
			break;
		}

		lx=mouse->x;
		ly=mouse->y;
		lb=b;
		SDL_Delay ( 1 );
	}

	SDL_FreeSurface(sfTmp);
	return players;
}

// Startet die Spielerauswahl (gibt die Spielereinstellungen):
sPlayer runPlayerSelection ( void )
{
	int b,lb=0,lx=-1,ly=-1;
	sPlayer players;
	Uint8 *keystate;

	for ( int i = 0; i < 4; i++ )
	{
		players.clan[i] = "NONE";
		players.what[i] = PLAYER_N;
	}
	players.what[0] = PLAYER_H;
	players.what[1] = PLAYER_AI;

	SDL_BlitSurface ( GraphicsData.gfx_player_select,NULL,buffer,NULL );
	font->showTextCentered(320,11, lngPack.i18n ( "Text~Title~Player_Select" ));
	font->showTextCentered(100,35, lngPack.i18n ( "Text~Title~Team" ));
	font->showTextCentered(200,35, lngPack.i18n ( "Text~Title~Human" ));
	font->showTextCentered(310,35, lngPack.i18n ( "Text~Title~Computer" ));
	font->showTextCentered(420,35, lngPack.i18n ( "Text~Title~Nobody" ));
	font->showTextCentered(535,35, lngPack.i18n ( "Text~Title~Clan" ));

	MenuButton btn_back( 50, 440, "Text~Button~Back");
	MenuButton btn_ok(  390, 440, "Text~Button~OK");

	btn_back.Draw();
	btn_ok.Draw();

	ShowPlayerStates ( players );

	mouse->Show();
	mouse->SetCursor ( CHand );
	SHOW_SCREEN
	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] )
		{
			for ( int i = 0; i < 4; i++ ) //reset players and exit
			{
				players.what[i] = PLAYER_N;
			}
			break;
		}
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
				if ( mouse->x >= x && mouse->x < x + 55 && mouse->y >= y && mouse->y < y + 71 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					if ( i == 0 ) players.what[0] = PLAYER_H;
					if ( i == 1 ) players.what[0] = PLAYER_AI;
					if ( i == 2 ) players.what[0] = PLAYER_N;

					if ( i == 3 ) players.what[1] = PLAYER_H;
					if ( i == 4 ) players.what[1] = PLAYER_AI;
					if ( i == 5 ) players.what[1] = PLAYER_N;

					if ( i == 6 ) players.what[2] = PLAYER_H;
					if ( i == 7 ) players.what[2] = PLAYER_AI;
					if ( i == 8 ) players.what[2] = PLAYER_N;

					if ( i == 9 ) players.what[3] = PLAYER_H;
					if ( i == 10 ) players.what[3] = PLAYER_AI;
					if ( i == 11 ) players.what[3] = PLAYER_N;
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

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		if (btn_ok.CheckClick(x, y, down, up))
		{
			return players;
		}
		if (btn_back.CheckClick(x, y, down, up))
		{
			for (int i = 0; i < 4; ++i) //reset players to nobody
			{
				players.what[i] = PLAYER_N;
			}
			break;
		}

		lx=mouse->x;
		ly=mouse->y;
		lb=b;
		SDL_Delay ( 1 );
	}
	return players;
}

void showPlayerStatesHotSeat ( sPlayerHS players )
{
	SDL_Rect dest,norm1,norm2;
	SDL_Rect rSrc = { 0, 0, 35, 35 };
	dest.w = norm1.w = norm2.w = 35;
	dest.h = norm1.h = norm2.h = 35;
	#define FIELD1 DIALOG_X+194
	#define FIELD2 DIALOG_X+304
	#define FIELD3 DIALOG_X+414
	dest.x = DIALOG_X+304;
	dest.y = DIALOG_Y+66;
	for ( int i = 0; i< 8;i++ )
	{

		dest.y = norm1.y = norm2.y = DIALOG_Y+66 + 45*i;
		font->showText(DIALOG_X + 45, dest.y + 10 + i, players.name[i]); //FIXME: adjust backgroundgraphicbars for names proper
		// Nichts
		if ( players.what[i] == PLAYER_N )
		{
			dest.x = FIELD3;
			//SDL_FillRect(buffer, &dest, 0xFC0000);
			SDL_BlitSurface ( OtherData.colors[i],&rSrc,buffer,&dest );
			norm1.x = FIELD1;
			norm2.x = FIELD2;
		}
		// Spieler
		if ( players.what[i] == PLAYER_H )
		{
			norm1.x = FIELD2;
			norm2.x = FIELD3;
			dest.x = FIELD1;
			//SDL_FillRect(buffer, &dest, 0x00FF00);
			SDL_BlitSurface ( OtherData.colors[i],&rSrc,buffer,&dest );
		}
		// Computer
		if ( players.what[i] == PLAYER_AI )
		{
			norm1.x = FIELD1;
			norm2.x = FIELD3;
			dest.x = FIELD2;
			//SDL_FillRect(buffer, &dest, 0x0000FF);
			SDL_BlitSurface ( OtherData.colors[i],&rSrc,buffer,&dest );
		}
		SDL_FillRect(buffer, &norm1, 0x000000);
		SDL_FillRect(buffer, &norm2, 0x000000);
		//SDL_BlitSurface ( GraphicsData.gfx_player_select,&norm1,buffer,&norm1 );
		//SDL_BlitSurface ( GraphicsData.gfx_player_select,&norm2,buffer,&norm2 );
	}
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
		if ( players.what[i] == PLAYER_N )
		{
			dest.x = 394;
			SDL_BlitSurface ( GraphicsData.gfx_player_none,NULL,buffer,&dest );
			norm1.x = 394 - 110;
			norm2.x = 394 - 219;
		}
		// Spieler
		if ( players.what[i] == PLAYER_H )
		{
			norm1.x = 394 - 110;
			norm2.x = 394;
			dest.x = 394 - 219;
			SDL_BlitSurface ( GraphicsData.gfx_player_human,NULL,buffer,&dest );
		}
		// Computer
		if ( players.what[i] == PLAYER_AI )
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
void ShowBars ( int credits,int StartCredits,cList<sLanding*> *landing,int selected, SDL_Surface *surface );
void MakeUpgradeSliderVehicle ( sUpgrades *u,int nr,cPlayer *p );
void MakeUpgradeSliderBuilding ( sUpgrades *u,int nr,cPlayer *p );
int CalcSteigerung ( int org, int variety );
int CalcPrice ( int value,int org, int variety );
void MakeUpgradeSubButtons ( bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf, SDL_Surface *surface );

static void ShowSelectionList(cList<sHUp*>& list, int selected, int offset, bool beschreibung, int credits, cPlayer* p);
static void CreateSelectionList(cList<sHUp*>& selection, cList<sHUp*>& images, int* selected, int* offset, bool tank, bool plane, bool ship, bool build, bool tnt, bool kauf);

void RunHangar ( cPlayer *player,cList<sLanding*> *LandingList )
{
	bool tank=true,plane=false,ship=false,build=false,tnt=false,kauf=true;
	bool FertigPressed=false,Beschreibung=SettingsData.bShowDescription;
	bool DownPressed=false,UpPressed=false,KaufPressed=false;
	bool Down2Pressed=false,Up2Pressed=false,EntfernenPressed=false;
	bool LadungUpPressed=false,LadungDownPressed=false;
	int b,lb=0,selected=0,offset=0,x,y,StartCredits=player->Credits,lx=-1,ly=-1;
	int LandingOffset=0,LandingSelected=0;
	SDL_Rect scr;

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

	drawButton(lngPack.i18n( "Text~Button~Done"), false, rBtnDone.x, rBtnDone.y, buffer);
	drawButton(lngPack.i18n( "Text~Button~Buy"), false, rBtnBuy.x, rBtnBuy.y, buffer);
	drawButton(lngPack.i18n( "Text~Button~Delete"), false, rBtnDel.x, rBtnDel.y, buffer);
	font->showTextCentered(rTxtDescription.x+rTxtDescription.w/2, rTxtDescription.y, lngPack.i18n( "Text~Comp~Description" ));

	//blit sfTmp to buffer



	// Die Liste erstellen:
	cList<sHUp*> list;
	for (size_t i = 0; i < UnitsData.vehicle.Size(); ++i)
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
		n->costs=UnitsData.vehicle[i].data.iBuilt_Costs;
		n->vehicle=true;
		MakeUpgradeSliderVehicle ( n->upgrades,i,player );
		list.Add ( n );
	}
	for (size_t i = 0; i < UnitsData.building.Size(); ++i)
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
		n->costs=UnitsData.building[i].data.iBuilt_Costs;
		n->vehicle=false;
		MakeUpgradeSliderBuilding ( n->upgrades,i,player );
		list.Add ( n );
	}
	// Die Selection erstellen:
	cList<sHUp*> selection;
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
		EventHandler->HandleEvents();
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
				drawButton(lngPack.i18n( "Text~Button~Done"), true, rBtnDone.x, rBtnDone.y, buffer);
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
			drawButton(lngPack.i18n( "Text~Button~Done"), false, rBtnDone.x, rBtnDone.y, buffer);
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
			if ( offset<selection.iCount-9 )
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
			if ( selection.iCount<9 )
			{
				if ( nr>=selection.iCount ) nr=-1;
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
				if ( last_selected==nr&&selection.Items[selected]->costs<=player->Credits )
				{
					// Don't add buildings, humans, planes, etc...
					if ( selection.Items[selected]->vehicle &&
						!UnitsData.vehicle[selection.Items[selected]->id].data.is_human &&
						!UnitsData.vehicle[selection.Items[selected]->id].data.is_alien &&
						!(UnitsData.vehicle[selection.Items[selected]->id].data.can_drive == DRIVE_AIR) &&
						!(UnitsData.vehicle[selection.Items[selected]->id].data.can_drive == DRIVE_SEA) )
					{
						sLanding *n;
						n=new sLanding;
						n->cargo=0;
						n->sf=selection.Items[selected]->sf;
						n->id=selection.Items[selected]->id;
						n->costs=selection.Items[selected]->costs;
						LandingList->Add ( n );
						LandingSelected=LandingList->iCount-1;
						while ( LandingSelected>=LandingOffset+5 ) LandingOffset++;

						if ( LandingSelected<0 ) LandingSelected=0;
						ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
						player->Credits-=selection.Items[selected]->costs;
						ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
						ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
					}
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Klick auf einen Upgrade-Slider:
		if ( b&&!lb&&x>=283&&x<301+18&&selection.iCount )
		{
			sHUp *ptr=selection.Items[selected];
			for (size_t i=0;i<8;i++ )
			{
				if ( !ptr->upgrades[i].active ) continue;
				if ( ptr->upgrades[i].Purchased&&x<283+18&&y>=293+i*19&&y<293+i*19+19 )
				{
					int variety;
					if ( ptr->upgrades[i].name.compare( lngPack.i18n ( "Text~Vehicles~Hitpoints" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Armor" ) ) == 0 || ptr->upgrades[i].name.compare( lngPack.i18n ( "Text~Vehicles~Ammo" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Damage" ) ) == 0 )
						variety = 0;
					else if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Speed" ) ) == 0 )
						variety = 1;
					else if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Shots" ) ) == 0 )
						variety = 2;
					else if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Range" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Scan" ) ) == 0 )
						variety = 3;
					else variety = -1;
					* ( ptr->upgrades[i].value )-=CalcSteigerung ( ptr->upgrades[i].StartValue,variety );
					// double price for damage
					if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Damage" ) ) == 0 )
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
					if ( ptr->upgrades[i].name.compare( lngPack.i18n ( "Text~Vehicles~Hitpoints" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Armor" ) ) == 0 || ptr->upgrades[i].name.compare( lngPack.i18n ( "Text~Vehicles~Ammo" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Damage" ) ) == 0 )
						variety = 0;
					else if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Speed" ) ) == 0 )
						variety = 1;
					else if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Shots" ) ) == 0 )
						variety = 2;
					else if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Range" ) ) == 0 || ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Scan" ) ) == 0 )
						variety = 3;
					else variety = -1;
					* ( ptr->upgrades[i].value ) +=CalcSteigerung ( ptr->upgrades[i].StartValue,variety );
					// double price for damage
					if ( ptr->upgrades[i].name.compare ( lngPack.i18n ( "Text~Vehicles~Damage" ) ) == 0 )
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
		if ( x >= rBtnBuy.x && x < rBtnBuy.x + rBtnBuy.w && y >= rBtnBuy.y && y < rBtnBuy.y + rBtnBuy.h && b && !KaufPressed && selection.Items[selected]->costs <= player->Credits && kauf )
		{
			PlayFX ( SoundData.SNDMenuButton );

			sLanding *n;
			n=new sLanding;
			n->cargo=0;
			n->sf=selection.Items[selected]->sf;
			n->id=selection.Items[selected]->id;
			n->costs=selection.Items[selected]->costs;
			LandingList->Add ( n );
			LandingSelected=LandingList->iCount-1;
			while ( LandingSelected>=LandingOffset+5 )
			{
				LandingOffset++;
			}
			ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			player->Credits-=n->costs;
			ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

			drawButton(lngPack.i18n( "Text~Button~Buy"), true, rBtnBuy.x, rBtnBuy.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			KaufPressed=true;
		}
		else if ( !b&&KaufPressed )
		{
			drawButton(lngPack.i18n( "Text~Button~Buy"), false, rBtnBuy.x, rBtnBuy.y, buffer);
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
			if ( LandingOffset<LandingList->iCount-5 )
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
			drawButton(lngPack.i18n( "Text~Button~Delete"), true, rBtnDel.x, rBtnDel.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			EntfernenPressed=true;
		}
		else if ( EntfernenPressed&&!b&&lb )
		{
			// Vehicle aus der Liste entfernen:
			if ( LandingList->iCount&&LandingList->iCount>LandingSelected&&LandingSelected>=0 )
			{
				player->Credits+=LandingList->Items[LandingSelected]->costs;
				player->Credits+=LandingList->Items[LandingSelected]->cargo/5;

				delete LandingList->Items[LandingSelected];
				LandingList->Delete ( LandingSelected );
				ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
				ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

				if ( LandingSelected>=LandingList->iCount )
				{
					LandingSelected--;
				}
				if ( LandingList->iCount-LandingOffset<5&&LandingOffset>0 )
				{
					LandingOffset--;
				}
				if ( LandingSelected<0 ) LandingSelected=0;
				ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			}

			drawButton(lngPack.i18n( "Text~Button~Delete"), false, rBtnDel.x, rBtnDel.y, buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			EntfernenPressed=false;
		}
		// Klick in die LandingListe:
		if ( x>=330&&x<330+128&&y>=22&&y<22+210&&b&&!lb )
		{
			int nr;
			nr= ( y-22 ) / ( 32+10 );
			if ( LandingList->iCount<5 )
			{
				if ( nr>=LandingList->iCount ) nr=-1;
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
					if ( LandingList->iCount&&LandingList->iCount>LandingSelected&&LandingSelected>=0 )
					{
						player->Credits+=LandingList->Items[LandingSelected]->costs;
						player->Credits+=LandingList->Items[LandingSelected]->cargo/5;
						delete LandingList->Items[LandingSelected];
						LandingList->Delete ( LandingSelected );
						ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
						ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

						if ( LandingSelected>=LandingList->iCount )
						{
							LandingSelected--;
						}
						if ( LandingList->iCount-LandingOffset<5&&LandingOffset>0 )
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

		if ( LandingSelected>=0&&LandingList->iCount&&LandingSelected<LandingList->iCount )
		{
			sLanding *ptr;
			ptr=LandingList->Items[LandingSelected];
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
					if ( ptr->cargo > UnitsData.vehicle[ptr->id].data.max_cargo )
					{
						ptr->cargo = UnitsData.vehicle[ptr->id].data.max_cargo;
					}
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
					if ( ptr->cargo < 0 )
					{
						ptr->cargo = 0;
					}
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

	while ( list.iCount )
	{
		sHUp *ptr;
		ptr = list.Items[0];
		for (size_t i=0;i<8;i++ )
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
		list.Delete ( 0 );
	}
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
		u[i].name = lngPack.i18n ( "Text~Vehicles~Damage" );
		i++;
		// Shots:
		u[i].active=true;
		u[i].value=& ( d->max_shots );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_shots, 2 );
		u[i].name = lngPack.i18n ( "Text~Vehicles~Shoots" );
		i++;
		// Range:
		u[i].active=true;
		u[i].value=& ( d->range );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.range, 3 );
		u[i].name = lngPack.i18n ( "Text~Vehicles~Range" );
		i++;
		// Ammo:
		u[i].active=true;
		u[i].value=& ( d->max_ammo );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_ammo, 0 );
		u[i].name = lngPack.i18n ( "Text~Vehicles~Ammo" );
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
	u[i].name = lngPack.i18n ( "Text~Vehicles~Armor" );
	i++;
	// Hitpoints:
	u[i].active=true;
	u[i].value=& ( d->max_hit_points );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_hit_points, 0 );
	u[i].name = lngPack.i18n ( "Text~Vehicles~Hitpoints" );
	i++;
	// Scan:
	u[i].active=true;
	u[i].value=& ( d->scan );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.scan, 3 );
	u[i].name = lngPack.i18n ( "Text~Vehicles~Scan" );
	i++;
	// Speed:
	u[i].active=true;
	u[i].value=& ( d->max_speed );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_speed, 1 );
	u[i].name = lngPack.i18n ( "Text~Vehicles~Speed" );
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
			org=org/4;
			value=value/4;
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
			org=org/4;
			if ( org == 5 || org == 6 || org == 7 || org == 9 )
				tmp = 1;
			if ( org == 10 || org == 12 || org ==14 || org == 15 || org == 16 || org == 18 || org == 20 )
				tmp = 2;
			if ( org == 28 )
				tmp = 5;
			tmp=tmp*4;
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
void ShowBars ( int credits,int StartCredits,cList<sLanding*> *landing,int selected, SDL_Surface *surface )
{
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
	font->showTextCentered(381,275, lngPack.i18n ( "Text~Title~Gold" ));
	font->showTextCentered(381,275+10, iToStr(credits));

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

	if ( selected>=0&&landing->iCount&&selected<landing->iCount )
	{
		sLanding *ptr;
		ptr=landing->Items[selected];
		if ( UnitsData.vehicle[ptr->id].data.can_transport==TRANS_METAL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_OIL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_GOLD )
		{
			font->showTextCentered(430,275, lngPack.i18n ( "Text~Title~Cargo" ));
			font->showTextCentered(430,275+10, iToStr(ptr->cargo));


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

	fakx= ( int ) ( ( SettingsData.iScreenW-192.0 ) / map->size );
	faky= ( int ) ( ( SettingsData.iScreenH-32.0 ) / map->size );

	// Die Karte malen:
	SDL_LockSurface ( buffer );
	for ( i=0;i<SettingsData.iScreenW-192;i++ )
	{
		for ( k=0;k<SettingsData.iScreenH-32;k++ )
		{
			nr=GetKachelBig ( ( i/fakx ) *fakx, ( k/faky ) *faky, map );
			t=map->terrain+nr;
			off= ( i%fakx ) * ( t->sf_org->h/fakx ) + ( k%faky ) * ( t->sf_org->h/faky ) *t->sf_org->w;
			nr=* ( ( int* ) ( t->sf_org->pixels ) +off );
			if ( nr==0xFF00FF )
			{
				t=map->terrain+map->DefaultWater;
				off= ( i%fakx ) * ( t->sf_org->h/fakx ) + ( k%faky ) * ( t->sf_org->h/faky ) *t->sf_org->w;
				nr=* ( ( int* ) ( t->sf_org->pixels ) +off );
			}
			( ( int* ) ( buffer->pixels ) ) [i+180+ ( k+18 ) *buffer->w]=nr;
		}
	}
	SDL_UnlockSurface ( buffer );

	// Hud drüber legen:
	Client->Hud.DoAllHud();
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );


	top.x=0;top.y= ( SettingsData.iScreenH/2 )-479;
	top.w=bottom.w=171;

	top.h=479;bottom.h=481;
	bottom.x=0;bottom.y= ( SettingsData.iScreenH/2 );
	SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&top );
	SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&bottom );

	SHOW_SCREEN

	t=map->terrain+GetKachelBig ( mouse->x-180,mouse->y-18, map );
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
		EventHandler->HandleEvents();
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		t=map->terrain+GetKachelBig ( mouse->x-180,mouse->y-18, map );
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
			*x= ( int ) ( ( mouse->x-180 ) / ( 448.0/map->size ) * ( 448.0/ ( SettingsData.iScreenW-192 ) ) );
			*y= ( int ) ( ( mouse->y-18 ) / ( 448.0/map->size ) * ( 448.0/ ( SettingsData.iScreenH-32 ) ) );
			break;
		}

		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}
}

// Zeigt die Liste mit den ausgewählten Landefahrzeugen an:
void ShowLandingList ( cList<sLanding*> *list,int selected,int offset, SDL_Surface *surface )
{
	sLanding *ptr;
	SDL_Rect scr,dest,text = {375,32,80,font->getFontHeight(LATIN_SMALL_WHITE)};
	int i;
	scr.x=330;scr.y=11;
	scr.w=128;scr.h=222;
	SDL_BlitSurface ( surface,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=340;dest.y=20;
	dest.w=32;dest.h=32;
	for ( i=offset;i<list->iCount;i++ )
	{
		if ( i>=offset+5 ) break;
		ptr=list->Items[i];
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

		if ( font->getTextWide ( UnitsData.vehicle[ptr->id].data.name, LATIN_SMALL_WHITE ) > text.w )
		{
			text.y -= font->getFontHeight(LATIN_SMALL_WHITE) / 2;
			font->showTextAsBlock ( text, UnitsData.vehicle[ptr->id].data.name, LATIN_SMALL_WHITE);
			text.y += font->getFontHeight(LATIN_SMALL_WHITE) / 2;
		}
		else
		{
			font->showText ( text, UnitsData.vehicle[ptr->id].data.name, LATIN_SMALL_WHITE);
		}



		if ( UnitsData.vehicle[ptr->id].data.can_transport==TRANS_METAL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_OIL||UnitsData.vehicle[ptr->id].data.can_transport==TRANS_GOLD )
		{
			int value = ptr->cargo;
			int maxval = UnitsData.vehicle[ptr->id].data.max_cargo;

			if(value == 0)
			{
				font->showText(text.x,text.y+10, "(empty)", LATIN_SMALL_WHITE);
			}
			else if(value <= maxval / 4)
			{
				font->showText(text.x,text.y+10, " ("+iToStr(value)+"/"+iToStr(maxval)+")", LATIN_SMALL_RED);
			}
			else if(value <= maxval / 2)
			{
				font->showText(text.x,text.y+10, " ("+iToStr(value)+"/"+iToStr(maxval)+")", LATIN_SMALL_YELLOW);
			}
			else
			{
				font->showText(text.x,text.y+10, " ("+iToStr(value)+"/"+iToStr(maxval)+")", LATIN_SMALL_GREEN);
			}


		}
		text.y+=32+10;
		dest.y+=32+10;
	}
}

// Stellt die Selectionlist zusammen:
static void CreateSelectionList(cList<sHUp*>& selection, cList<sHUp*>& images, int* const selected, int* const offset, bool const tank, bool plane, bool ship, bool build, bool const tnt, bool const kauf)
{
	sUnitData *bd;
	sUnitData *vd;
	int i;
	while ( selection.iCount )
	{
		selection.Delete ( 0 );
	}
	if ( kauf )
	{
		plane=false;
		ship=false;
		build=false;
	}
	for ( i=0;i<images.iCount;i++ )
	{
		if ( images.Items[i]->vehicle )
		{
			if ( ! ( tank||ship||plane ) ) continue;
			vd=& ( UnitsData.vehicle[images.Items[i]->id].data );
			if ( vd->is_alien&&kauf ) continue;
			if ( vd->is_human&&kauf ) continue;
			if ( tnt&&!vd->can_attack ) continue;
			if ( vd->can_drive==DRIVE_AIR&&!plane ) continue;
			if ( vd->can_drive==DRIVE_SEA&&!ship ) continue;
			if ( ( vd->can_drive==DRIVE_LAND||vd->can_drive==DRIVE_LANDnSEA ) &&!tank ) continue;
			selection.Add ( images.Items[i] );
		}
		else
		{
			if ( !build ) continue;
			bd=& ( UnitsData.building[ ( ( sHUp* ) ( images.Items[i] ) )->id].data );
			if ( tnt&&!bd->can_attack ) continue;
			selection.Add ( images.Items[i] );
		}
	}
	if ( *offset>=selection.iCount-9 )
	{
		*offset=selection.iCount-9;
		if ( *offset<0 ) *offset=0;
	}
	if ( *selected>=selection.iCount )
	{
		*selected=selection.iCount-1;
		if ( *selected<0 ) *selected=0;
	}
}

// Zeigt die Liste mit den Images an:
static void ShowSelectionList(cList<sHUp*>& list, int const selected, int const offset, bool const beschreibung, int const credits, cPlayer* const p)
{
	sHUp *ptr;
	SDL_Rect dest,scr,text = {530, 70, 72, font->getFontHeight(LATIN_SMALL_WHITE)};
	int i,k;
	scr.x=479;scr.y=52;
	scr.w=150;scr.h=330;
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=490;dest.y=58;
	dest.w=32;dest.h=32;
	if ( list.iCount==0 )
	{
		scr.x=0;scr.y=0;
		scr.w=316;scr.h=256;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
		scr.x=11;scr.y=290;
		scr.w=346;scr.h=176;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
		return;
	}
	for ( i=offset;i<list.iCount;i++ )
	{
		if ( i>=offset+9 ) break;
		// Das Bild malen:
		ptr = list.Items[i];
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
					font->showTextAsBlock(tmp, UnitsData.vehicle[ptr->id].text);
				}
				else
				{
					font->showTextAsBlock(tmp, UnitsData.building[ptr->id].text);
				}
			}
			// Die Details anzeigen:
			{
				tmp.x=11;
				tmp.y=290;
				tmp.w=346;
				tmp.h=176;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&tmp,buffer,&tmp );
				if ( ptr->vehicle )
				{
					cVehicle tv(&UnitsData.vehicle[ptr->id], p);
					tv.ShowBigDetails();
				}
				else
				{
					cBuilding tb(&UnitsData.building[ptr->id], p, NULL);
					tb.ShowBigDetails();
				}
			}
			// Die Texte anzeigen/Slider machen:
			for ( k=0;k<8;k++ )
			{
				SDL_Rect scr,dest;
				if ( !ptr->upgrades[k].active ) continue;
				font->showText(322,296+k*19, iToStr(ptr->upgrades[k].NextPrice));

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
		string sTmp;

		if ( ptr->vehicle )
		{
			sTmp = UnitsData.vehicle[ptr->id].data.name;
			font->showTextCentered(616, text.y, iToStr(UnitsData.vehicle[ptr->id].data.iBuilt_Costs), LATIN_SMALL_YELLOW);
		}
		else
		{
			sTmp = UnitsData.building[ptr->id].data.name;
		}


		if ( font->getTextWide ( sTmp, LATIN_SMALL_WHITE ) > text.w )
		{
			text.y -= font->getFontHeight(LATIN_SMALL_WHITE) / 2;
			font->showTextAsBlock ( text, sTmp, LATIN_SMALL_WHITE);
			text.y += font->getFontHeight(LATIN_SMALL_WHITE) / 2;
		}
		else
		{
			font->showText ( text, sTmp, LATIN_SMALL_WHITE);
		}


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

cMultiPlayerMenu::cMultiPlayerMenu(bool const bHost)
{
	this->bHost = bHost;
	ActualPlayer = new cPlayer ( SettingsData.sPlayerName, OtherData.colors[cl_red], 0, MAX_CLIENTS ); // Socketnumber MAX_CLIENTS for lokal client
	PlayerList.Add ( ActualPlayer );
	iNextPlayerNr = 1;
	ReadyList = (bool *) malloc ( sizeof( bool ) );
	ReadyList[0] = false;
	bOptions = false;
	bStartSelecting = false;

	bAllLanded = false;

	if ( bHost ) sIP = "-";
	else sIP = SettingsData.sIP;
	iPort = SettingsData.iPort;

	network = new cTCP;
}

cMultiPlayerMenu::~cMultiPlayerMenu()
{
	delete network;
	network = NULL;

	while ( PlayerList.iCount ) PlayerList.Delete ( PlayerList.iCount-1 );

	while ( ChatLog.iCount ) ChatLog.Delete ( ChatLog.iCount-1 );
}

void cMultiPlayerMenu::addChatLog( string sMsg )
{
	// Delete old chat messages
	while( ChatLog.iCount > 7 )
	{
		ChatLog.Delete( 0 );
	}
	// Add the new message
	ChatLog.Add ( sMsg );
}

void cMultiPlayerMenu::showChatLog()
{
	string sMsg;
	SDL_Rect rect;
	// clear chat window
	rect.x = 20;
	rect.y = 290;
	rect.w = 425;
	rect.h = 110;
	SDL_BlitSurface( sfTmp, &rect, buffer, &rect );
	// now fill chat window new
	for( int i = 0; i < ChatLog.iCount; i++ )
	{
		sMsg = ChatLog.Items[ChatLog.iCount-1-i];
		while ( font->getTextWide( sMsg ) > 410 )
		{
			sMsg.erase( sMsg.length()-1 );
		}
		font->showText( 25, 387-12*i, sMsg, LATIN_NORMAL, buffer );
	}
	SHOW_SCREEN
	mouse->draw( false, screen );
}

void cMultiPlayerMenu::runNetworkMenu()
{
	bool bShowCursor = true;
	int b, lb = 0, lx = -1, ly = -1;
	string ChatStr, stmp;
	SDL_Rect scr;
	Uint8 *keystate;

#define FOCUS_IP   0
#define FOCUS_PORT 1
#define FOCUS_NAME 2
#define FOCUS_CHAT 3

	SDL_Rect dest = { DIALOG_X, DIALOG_Y, DIALOG_W, DIALOG_H};

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

	// prepare the focus
	iFocus = FOCUS_NAME;
	InputStr = ActualPlayer->name;

	ChatStr = "";
	font->showText(20,245, lngPack.i18n ( "Text~Title~IP" ));
	font->showText(20,260, sIP);

	font->showText(228,245, lngPack.i18n ( "Text~Title~Port" ));
	font->showText(228,260, iToStr( iPort ) );

	font->showText(352, 245, lngPack.i18n ( "Text~Title~Player_Name" ));
	font->showText(352,260, ActualPlayer->name);

	font->showText(500,245, lngPack.i18n ( "Text~Title~Color" ));
	dest.x = 505; dest.y = 260; scr.w = dest.w = 83; scr.h = dest.h = 10; scr.x = 0; scr.y=  0;
	SDL_BlitSurface ( ActualPlayer->color,&scr,buffer,&dest );

	SmallButton btn_send(   470, 416, "Text~Title~Send");
	MenuButton  btn_back(    50, 450, "Text~Button~Back");
	MenuButton  btn_ok(     390, 450, "Text~Button~OK");
	SmallButton btn_planet( 470,  42, "Text~Title~Choose_Planet");
	SmallButton btn_options(470,  77, "Text~Title~Options");
	SmallButton btn_load(   470, 112, "Text~Button~Game_Load");
	SmallButton btn_start(  470, 200, "Text~Button~Host_Start");
	SmallButton btn_connect(470, 200, "Text~Title~Connect");

	if ( bHost )
	{
		font->showTextCentered( 320, 11, lngPack.i18n ( "Text~Button~TCPIP_Host" ) );
		btn_planet.Draw();
		btn_options.Draw();
		btn_load.Lock();
		btn_start.Draw();
		btn_ok.Draw();
	}
	else
	{
		font->showTextCentered( 320, 11, lngPack.i18n ( "Text~Button~TCPIP_Client" ) );
		btn_connect.Draw();
	}

	btn_send.Draw();
	btn_back.Draw();

	mouse->SetCursor ( CHand );
	displayGameSettings();
	displayPlayerList();
	SHOW_SCREEN
	mouse->draw ( false, screen );
	bRefresh = false;

	while ( 1 )
	{
		// get Events
		EventHandler->HandleEvents();
		// check keyboard input
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;
		// do the mouse stuff
		mouse->GetPos();
		b=mouse->GetMouseButton();

		if ( mouse->x != lx || mouse->y != ly )
		{
			mouse->draw ( true, screen );
		}

		// do the focus
		if ( DoKeyInp ( keystate ) )
		{
			bShowCursor = true;
		}
		if ( bShowCursor )
		{
			static bool CursorOn = false;
			bShowCursor = false;
			CursorOn = !CursorOn;
			int i_tmpRedrawLength = 20; //20 choosen by random to make sure we erase _all_ the old garbage on screen - should be calculated in a better way when fonts come from ttf and not from jpg -- beko
			switch ( iFocus )
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
					i_tmpRedrawLength += font->getTextWide(InputStr);
					while ( font->getTextWide(InputStr) > 176 )
					{
						InputStr.erase ( InputStr.end()-1 );
					}
					stmp = InputStr; stmp += "_";

					sIP = InputStr;
					scr.x=20;scr.y=260;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					font->showText(20,260, stmp);
					break;
				case FOCUS_PORT:
					i_tmpRedrawLength += font->getTextWide(InputStr);
					if ( atoi ( InputStr.c_str() ) > 65535 ) //ports over 65535 are impossible
					{
						iPort = 58600; //default Port 58600 - why is this our default Port? -- beko
						stmp = "58600_";
					}
					else
					{
						stmp = InputStr + "_";
						iPort = atoi ( InputStr.c_str() );
					}
					scr.x=228;scr.y=260;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					font->showText(228,260, stmp);
					break;
				case FOCUS_NAME:
					i_tmpRedrawLength  += font->getTextWide(InputStr);
					while ( font->getTextWide(InputStr) > 98 )
					{
						InputStr.erase ( InputStr.end()-1 );
					}
					stmp = InputStr + "_";

					if ( strcmp ( ActualPlayer->name.c_str(),InputStr.c_str() ) )
					{
						ActualPlayer->name=InputStr;
						displayPlayerList();
						sendIdentification();
					}
					scr.x=352;
					scr.y=260;
					scr.w=i_tmpRedrawLength;
					scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					font->showText(352,260, stmp);
					break;
				case FOCUS_CHAT:
					i_tmpRedrawLength += font->getTextWide(InputStr);
					while ( font->getTextWide(InputStr) > 410 - font->getTextWide ( ( ActualPlayer->name+": " ) ) ) //keeping playername lenght in mind
					{
						InputStr.erase ( InputStr.end()-1 );
					}
					stmp = InputStr + "_";

					ChatStr = InputStr;
					scr.x=20;scr.y=423;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					font->showText(20,423, stmp);
					break;
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		int  const x    = mouse->x;
		int  const y    = mouse->y;
		bool const down = b > lb;
		bool const up   = b < lb;

		if (btn_back.CheckClick(x, y, down, up))
		{
			// Save changed name, port or ip to max.xml
			SettingsData.sPlayerName = ActualPlayer->name;
			SaveOption(SAVETYPE_NAME);
			SaveOption(SAVETYPE_IP);
			SaveOption(SAVETYPE_PORT);
			break;
		}
		if (bHost && btn_ok.CheckClick(x, y, down, up))
		{
			int iPlayerNum;
			if ((!bOptions || sMap.empty()) && sSaveGame.empty())
			{
				addChatLog(lngPack.i18n("Text~Multiplayer~Missing_Settings"));
			}
			else if ((iPlayerNum = testAllReady()) != -1)
			{
				addChatLog(PlayerList.Items[iPlayerNum]->name + " " + lngPack.i18n("Text~Multiplayer~Not_Ready"));
			}
			else
			{
				// Save changed name, port or ip to max.xml
				SettingsData.sPlayerName = ActualPlayer->name;
				SaveOption(SAVETYPE_NAME);
				SaveOption(SAVETYPE_IP);
				SaveOption(SAVETYPE_PORT);

				bStartSelecting = true;
			}

			SHOW_SCREEN
			mouse->draw(false, screen);
		}

		// Next color:
		if ( b && !lb && mouse->x >= 596 && mouse->x < 596+18 && mouse->y >= 256 && mouse->y < 256+18 )
		{
			int nr;
			PlayFX ( SoundData.SNDObjectMenu );
			nr = GetColorNr ( ActualPlayer->color )+1;
			if ( nr > 7 ) nr = 0;
			ActualPlayer->color = OtherData.colors[nr];
			font->showText( 500, 245, lngPack.i18n ( "Text~Title~Color" ) );
			dest.x = 505; dest.y = 260; scr.w = dest.w = 83; scr.h = dest.h = 10; scr.x = 0; scr.y = 0;
			SDL_BlitSurface ( ActualPlayer->color, &scr, buffer, &dest );
			displayPlayerList();
			sendIdentification();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// prev Color:
		if ( b && !lb && mouse->x >= 478 && mouse->x < 478+18 && mouse->y >= 256 && mouse->y < 256+18 )
		{
			int nr;
			PlayFX ( SoundData.SNDObjectMenu );
			nr = GetColorNr ( ActualPlayer->color )-1;
			if ( nr < 0 ) nr = 7;
			ActualPlayer->color = OtherData.colors[nr];
			font->showText( 500, 245, lngPack.i18n ( "Text~Title~Color" ) );
			dest.x = 505; dest.y = 260; scr.w = dest.w = 83; scr.h = dest.h = 10; scr.x = 0; scr.y = 0;
			SDL_BlitSurface ( ActualPlayer->color, &scr, buffer, &dest );
			displayPlayerList();
			sendIdentification();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Host buttons:
		if ( bHost && sSaveGame.empty() )
		{
			if (btn_planet.CheckClick(x, y, down, up))
			{
				sMap = RunPlanetSelect();
				sSaveGame = "";
				SDL_BlitSurface ( sfTmp, NULL, buffer, NULL );
				displayGameSettings();
				displayPlayerList();

				font->showTextCentered( 320, 11, lngPack.i18n ( "Text~Button~TCPIP_Host" ) );
				font->showText( 20, 245, lngPack.i18n ( "Text~Title~IP" ) );
				font->showText( 20, 260, sIP);
				font->showText( 228, 245, lngPack.i18n ( "Text~Title~Port" ) );
				font->showText( 228, 260, iToStr( iPort ) );
				font->showText( 352, 245, lngPack.i18n ( "Text~Title~Player_Name" ) );
				font->showText( 352, 260, ActualPlayer->name );
				font->showText( 500, 245, lngPack.i18n ( "Text~Title~Color" ) );

				dest.x = 505;
				dest.y = 260;
				scr.w = dest.w = 83;
				scr.h = dest.h = 10;
				scr.x = 0;
				scr.y = 0;
				SDL_BlitSurface ( ActualPlayer->color, &scr, buffer, &dest );
				btn_planet.Draw();
				btn_options.Draw();
				btn_load.Draw();
				btn_start.Draw();
				btn_send.Draw();
				btn_back.Draw();
				btn_ok.Draw();
				SHOW_SCREEN
				mouse->draw ( false, screen );
				sendOptions();
			}
			if (btn_options.CheckClick(x, y, down, up))
			{
				if ( !bOptions )
				{
					Options = RunOptionsMenu ( NULL );
				}
				else
				{
					Options = RunOptionsMenu ( &Options );
				}
				bOptions = true;
				sSaveGame = "";
				SDL_BlitSurface ( sfTmp, NULL, buffer, NULL );
				displayGameSettings();
				displayPlayerList();

				font->showTextCentered( 320, 11, lngPack.i18n ( "Text~Button~TCPIP_Host" ) );
				font->showText( 20, 245, lngPack.i18n ( "Text~Title~IP" ) );
				font->showText( 20, 260, sIP);
				font->showText( 228, 245, lngPack.i18n ( "Text~Title~Port" ) );
				font->showText( 228, 260, iToStr( iPort ) );
				font->showText( 352, 245, lngPack.i18n ( "Text~Title~Player_Name" ) );
				font->showText( 352, 260, ActualPlayer->name );
				font->showText( 500, 245, lngPack.i18n ( "Text~Title~Color" ) );

				dest.x = 505;
				dest.y = 260;
				scr.w = dest.w = 83;
				scr.h = dest.h = 10;
				scr.x = 0;
				scr.y = 0;
				SDL_BlitSurface ( ActualPlayer->color, &scr, buffer, &dest );
				btn_planet.Draw();
				btn_options.Draw();
				btn_load.Draw();
				btn_start.Draw();
				btn_send.Draw();
				btn_back.Draw();
				btn_ok.Draw();
				SHOW_SCREEN
				mouse->draw ( false, screen );
				sendOptions();
			}
			if (btn_load.CheckClick(x, y, down, up))
			{
				// TODO implement
			}
		}

		if (bHost)
		{
			if (btn_start.CheckClick(x, y, down, up))
			{
				if (network->getConnectionStatus() == 0) // Connect only if there isn't a connection jet
				{
					network->setPort(iPort);

					if (network->create() == -1)
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Error_Socket"));
						cLog::write("Error opening socket", cLog::eLOG_TYPE_WARNING);
					}
					else
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n("Text~Title~Port") + ": "  + iToStr(iPort) + ")");
						cLog::write("Game open (Port: " + iToStr(iPort) + ")", cLog::eLOG_TYPE_INFO);
					}
				}

				SHOW_SCREEN
				mouse->draw(false, screen);
			}
		}
		else
		{
			if (btn_connect.CheckClick(x, y, down, up))
			{
				if (network->getConnectionStatus() == 0) // Connect only if there isn't a connection jet
				{
					network->setPort(iPort);
					network->setIP(sIP);

					addChatLog(lngPack.i18n("Text~Multiplayer~Network_Connecting") + sIP + ":" + iToStr(iPort)); // e.g. Connecting to 127.0.0.1:55800
					cLog::write(("Connecting to " + sIP + ":" + iToStr(iPort)), cLog::eLOG_TYPE_INFO);

					if (network->connect() == -1)
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Error_Connect") + sIP + ":" + iToStr(iPort));
						cLog::write("Error on connecting " + sIP + ":" + iToStr(iPort), cLog::eLOG_TYPE_WARNING);
					}
					else
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Connected"));
						cLog::write("Connected", cLog::eLOG_TYPE_INFO);
					}
				}

				SHOW_SCREEN
				mouse->draw(false, screen);
			}
		}

		if ((InputEnter && iFocus == FOCUS_CHAT) ||
				btn_send.CheckClick(x, y, down, up))
		{
			if ( !ChatStr.empty() )
			{
				PlayFX ( SoundData.SNDChat );
				if ( ChatStr.compare( "/ready" ) == 0 )
				{
					int iPlayerIndex;
					for ( iPlayerIndex = 0; iPlayerIndex < PlayerList.iCount; iPlayerIndex++ )
					{
						if ( PlayerList.Items[iPlayerIndex] == ActualPlayer ) break;
					}
					ReadyList[iPlayerIndex] = !ReadyList[iPlayerIndex];
					displayPlayerList();
					sendIdentification();
				}
				else
				{
					ChatStr.insert ( 0,": " );
					ChatStr.insert ( 0, ActualPlayer->name );

					if ( ChatStr.length() > PACKAGE_LENGHT-7 )
					{
						ChatStr.erase ( PACKAGE_LENGHT-7 );
					}
					cNetMessage *Message = new cNetMessage ( MU_MSG_CHAT );
					Message->pushString ( ChatStr );
					sendMessage ( Message );

					if ( network->isHost() ) addChatLog ( ChatStr );
				}
				ChatStr="";
				if ( iFocus==FOCUS_CHAT ) InputStr = "";
				scr.x = 20;
				scr.y = 423;
				scr.w = 430;
				scr.h = 16;
				SDL_BlitSurface ( sfTmp, &scr, buffer, &scr );

				font->showText(20, 423, ChatStr);
			}

			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Set new Focus
		if ( ( !bHost && b && !lb && mouse->x >= 20 && mouse->x < 20+188 && mouse->y >= 250 && mouse->y < 250+30 ) ||
				( b && !lb && mouse->x >= 228 && mouse->x < 228+108 && mouse->y >= 250 && mouse->y < 250+30 ) ||
				( b && !lb && mouse->x >= 352 && mouse->x < 352+108 && mouse->y >= 250 && mouse->y < 250+30 ) ||
				( b && !lb && mouse->x >= 20 && mouse->x < 20+425 && mouse->y >= 420 && mouse->y < 420+30 ) )
		{
			if ( !bHost && b && !lb && mouse->x >= 20 && mouse->x < 20+188 && mouse->y >= 250 && mouse->y < 250+30 )
			{
				iFocus = FOCUS_IP;
				InputStr = sIP;
			}
			else if( b && !lb && mouse->x >= 228 && mouse->x < 228+108 && mouse->y >= 250 && mouse->y < 250+30 )
			{
				iFocus = FOCUS_PORT;
				InputStr = iToStr( iPort );
			}
			else if( b && !lb && mouse->x >= 352 && mouse->x < 352+108 && mouse->y >= 250 && mouse->y < 250+30 )
			{
				iFocus = FOCUS_NAME;
				InputStr = ActualPlayer->name;
			}
			else if( b && !lb && mouse->x >= 20 && mouse->x < 20+425 && mouse->y >= 420 && mouse->y < 420+30 )
			{
				iFocus = FOCUS_CHAT;
				InputStr = ChatStr;
			}

			bShowCursor = true;

			scr.x = 20;
			scr.y = 260;
			scr.w = 188;
			scr.h = 16;
			SDL_BlitSurface ( sfTmp, &scr, buffer, &scr );

			font->showText(20, 260, sIP);
			scr.x = 228;
			scr.y = 260;
			scr.w = 108;
			scr.h = 16;
			SDL_BlitSurface ( sfTmp, &scr, buffer, &scr );

			font->showText(228, 260, iToStr(iPort));
			scr.x = 352;
			scr.y = 260;
			scr.w = 108;
			scr.h = 16;
			SDL_BlitSurface ( sfTmp, &scr, buffer, &scr );

			font->showText(352, 260, ActualPlayer->name);
			scr.x = 20;
			scr.y = 423;
			scr.w = 430;
			scr.h = 16;
			SDL_BlitSurface ( sfTmp, &scr, buffer, &scr );

			font->showText(20, 423, ChatStr);
		}

		// show chat log if necessary
		showChatLog();
		// show other data if necessary
		if ( bRefresh )
		{
			bRefresh = false;
			displayGameSettings();
			displayPlayerList();
		}

		if ( bStartSelecting )
		{
			if( !sSaveGame.empty() )
			{
				if ( bHost )
				{
					// Send savegame
				}
				else
				{
					// Wait for savegame
				}
			}
			else
			{
				if ( bHost )
				{
					cNetMessage *Message = new cNetMessage ( MU_MSG_GO );
					sendMessage ( Message );
				}
				int iLandX, iLandY;
				Map = new cMap;
				cMap *ServerMap = new cMap;
				if ( Map->LoadMap ( sMap ) )
				{
					if ( bHost )
					{
						Map->PlaceRessources ( Options.metal, Options.oil, Options.gold, Options.dichte );
						// copy map for server
						ServerMap->NewMap( Map->size, Map->TerrainInUse.iCount );
						ServerMap->DefaultWater = Map->DefaultWater;
						ServerMap->MapName = Map->MapName;
						memcpy ( ServerMap->Kacheln, Map->Kacheln, sizeof ( int )*Map->size*Map->size );
						memcpy ( ServerMap->Resources, Map->Resources, sizeof ( sResources )*Map->size*Map->size );
						for ( int i = 0; i < Map->TerrainInUse.iCount; i++ )
						{
							ServerMap->terrain[i].blocked = Map->terrain[i].blocked;
							ServerMap->terrain[i].coast = Map->terrain[i].coast;
							ServerMap->terrain[i].water = Map->terrain[i].water;
						}
					}
					else memset ( Map->Resources, 0, Map->size*Map->size*sizeof( sResources ) );

					// copy playerlist for client
					cList<cPlayer*> *ClientPlayerList = new cList<cPlayer*>;
					cPlayer *ActualPlayerClient;
					for ( int i = 0; i < PlayerList.iCount; i++ )
					{
						ClientPlayerList->Add ( new cPlayer( PlayerList.Items[i]->name, PlayerList.Items[i]->color, PlayerList.Items[i]->Nr, PlayerList.Items[i]->iSocketNum ) );
						if ( ClientPlayerList->Items[i]->Nr == ActualPlayer->Nr ) ActualPlayerClient = ClientPlayerList->Items[i];
					}
					// init client and his player
					Client = new cClient(Map, ClientPlayerList);
					Client->initPlayer ( ActualPlayerClient );
					for ( int i = 0; i < ClientPlayerList->iCount; i++ )
					{
						ClientPlayerList->Items[i]->InitMaps ( Map->size, Map );
						ClientPlayerList->Items[i]->Credits = Options.credits;
					}

					if ( bHost )
					{
						// init the players of playerlist
						for ( int i = 0; i < PlayerList.iCount; i++ )
						{
							PlayerList.Items[i]->InitMaps ( ServerMap->size, ServerMap );
							PlayerList.Items[i]->Credits = Options.credits;
						}

						// init server
						Server = new cServer(ServerMap, &PlayerList, GAME_TYPE_TCPIP, Options.PlayRounds);
					}

					cList<sLanding*> LandingList;
					RunHangar ( ActualPlayerClient, &LandingList );

					if ( !bHost ) sendUpgrades();

					SelectLanding ( &iLandX, &iLandY, Map );

					if ( bHost )
					{
						// wait for other players
						font->showTextCentered( 320, 235, lngPack.i18n ( "Text~Multiplayer~Waiting" ) ,LATIN_BIG );
						SHOW_SCREEN
						ClientDataList = new cList<sClientLandData*>;
						while ( ClientDataList->iCount < PlayerList.iCount-1 )
						{
							if ( network->getSocketCount() < PlayerList.iCount-1 )
							{
								;// Should not happen oO :P
							}
							EventHandler->HandleEvents();
							mouse->GetPos();

							if ( mouse->x != lx || mouse->y != ly )
							{
								mouse->draw ( true, screen );
							}

							HandleMessages();

							lx = mouse->x;
							ly = mouse->y;
							lb = b;
							SDL_Delay ( 1 );
						}

						// make all landings
						Server->makeLanding ( iLandX, iLandY, ActualPlayer, &LandingList, Options.FixedBridgeHead );
						for ( int i = 0; i < ClientDataList->iCount; i++ )
						{
							cPlayer *Player;
							for ( int n = 0; n < PlayerList.iCount; n++ )
							{
								if ( PlayerList.Items[n]->Nr == ClientDataList->Items[i]->iNr )
								{
									Player = PlayerList.Items[n];
									break;
								}
							}
							Server->makeLanding( ClientDataList->Items[i]->iLandX, ClientDataList->Items[i]->iLandY, Player, ClientDataList->Items[i]->LandingList, Options.FixedBridgeHead );
						}
						// copy changed resources from server map to client map
						memcpy ( Map->Resources, ServerMap->Resources, sizeof ( sResources )*Map->size*Map->size );

						// send clients that all players have been landed
						cNetMessage *Message = new cNetMessage ( MU_MSG_ALL_LANDED );
						sendMessage ( Message );
					}
					else
					{
						sendLandingInfo( iLandX, iLandY, &LandingList );
						// wait for other players
						font->showTextCentered( 320, 235, lngPack.i18n ( "Text~Multiplayer~Waiting" ), LATIN_BIG );
						SHOW_SCREEN
						while ( !bAllLanded )
						{
							EventHandler->HandleEvents();
							mouse->GetPos();

							if ( mouse->x != lx || mouse->y != ly )
							{
								mouse->draw ( true, screen );
							}

							HandleMessages();

							lx = mouse->x;
							ly = mouse->y;
							lb = b;
							SDL_Delay ( 1 );
						}
					}

					while ( LandingList.iCount )
					{
						delete LandingList.Items[LandingList.iCount - 1];
						LandingList.Delete(LandingList.iCount - 1);
					}

					ExitMenu();

					if ( Options.PlayRounds && Client->ActivePlayer->Nr != 0 ) Client->bWaitForOthers = true;
					if ( bHost ) Server->bStarted = true;
					Client->run();

					SettingsData.sPlayerName = ActualPlayerClient->name;
					while ( PlayerList.iCount )
					{
						delete ( PlayerList.Items[0] );
						PlayerList.Delete ( 0 );
					}
					delete Client;
					Client = NULL;
					if ( bHost )
					{
						delete Server;
						Server = NULL;
					}
					break;
				}
			}
		}

		HandleMessages();

		lx = mouse->x;
		ly = mouse->y;
		lb = b;
		SDL_Delay ( 1 );
	}
	SDL_FreeSurface(sfTmp);
}

void cMultiPlayerMenu::HandleMessages()
{
#define SWITCH_MESSAGE_END { delete Message; MessageList.Delete ( iMsgNum ); iMsgNum--; break; }

	for ( int iMsgNum = 0; iMsgNum < MessageList.iCount; iMsgNum++ )
	{
		cNetMessage *Message = MessageList.Items[iMsgNum];

		switch ( Message->iType )
		{
		case MU_MSG_CHAT:
			{
				cNetMessage *SendMessage = new cNetMessage( MU_MSG_CHAT );
				string sChatString = Message->popString();
				SendMessage->pushString( sChatString );
				if ( network->isHost() ) sendMessage ( SendMessage, -1 );
				addChatLog( sChatString );
			}
			SWITCH_MESSAGE_END
		case MU_MSG_NEW_PLAYER:
			{
				cPlayer *Player = new cPlayer ( "unidentified", OtherData.colors[0], iNextPlayerNr, Message->popInt16() );
				PlayerList.Add ( Player );
				ReadyList = (bool *)realloc ( ReadyList, PlayerList.iCount );
				ReadyList[PlayerList.iCount-1] = false;
				cNetMessage *SendMessage = new cNetMessage ( MU_MSG_REQ_IDENTIFIKATION );
				SendMessage->pushInt16( iNextPlayerNr );
				iNextPlayerNr++;
				sendMessage ( SendMessage, Player->iSocketNum );
			}
			SWITCH_MESSAGE_END
		case MU_MSG_DEL_PLAYER:
			{
				int iClientNum = Message->popInt16();
				for ( int i = 0; i < PlayerList.iCount; i++ )
				{
					if ( PlayerList.Items[i]->iSocketNum == iClientNum )
					{
						PlayerList.Delete ( i );
						for (int j = i; j < PlayerList.iCount; j++ ) ReadyList[j] = ReadyList[j+1];
						ReadyList = (bool *)realloc ( ReadyList, PlayerList.iCount );
					}
				}
				displayPlayerList();
				sendPlayerList();
			}
			SWITCH_MESSAGE_END
		case MU_MSG_REQ_IDENTIFIKATION:
			ActualPlayer->Nr = Message->popInt16();
			sendIdentification();
			SWITCH_MESSAGE_END
		case MU_MSG_IDENTIFIKATION:
			{
				int iPlayerNum;
				int iPlayerNr = Message->popInt16();
				for ( iPlayerNum = 0; iPlayerNum < PlayerList.iCount; iPlayerNum++ )
				{
					if ( PlayerList.Items[iPlayerNum]->Nr == iPlayerNr ) break;
				}
				PlayerList.Items[iPlayerNum]->color = OtherData.colors[Message->popInt16()];
				PlayerList.Items[iPlayerNum]->name = Message->popString();
				ReadyList[iPlayerNum] = Message->popBool();
				displayPlayerList();
				sendPlayerList();
				sendOptions();
			}
			SWITCH_MESSAGE_END
		case MU_MSG_PLAYERLIST:
			{
				string str = Message->getHexDump();
				int iPlayerCount = Message->popInt16();
				while ( PlayerList.iCount > 0 ) PlayerList.Delete ( 0 );
				ReadyList = (bool *)realloc( ReadyList, iPlayerCount );
				for ( int i = 0; i < iPlayerCount; i++ )
				{
					string sName = Message->popString();
					int iColor = Message->popInt16();
					int iNr = Message->popInt16();
					cPlayer *Player = new cPlayer ( sName, OtherData.colors[iColor], iNr );
					PlayerList.Add ( Player );
					if ( Player->Nr == ActualPlayer->Nr ) ActualPlayer = Player;
					ReadyList[i] = Message->popBool();
				}
				displayPlayerList();
				SWITCH_MESSAGE_END
			}
		case MU_MSG_OPTINS:
			{
				if ( bOptions = Message->popBool() )
				{
					Options.metal = Message->popInt16();
					Options.oil = Message->popInt16();
					Options.gold = Message->popInt16();
					Options.dichte = Message->popInt16();
					Options.credits = Message->popInt16();
					Options.FixedBridgeHead = Message->popBool();
					Options.AlienTech = Message->popBool();
					Options.PlayRounds = Message->popBool();
				}
				if ( Message->popBool() )
				{
					sMap = Message->popString();
				}
				if ( Message->popBool() )
				{
					sSaveGame = Message->popString();
				}
				bRefresh = true;
			}
			SWITCH_MESSAGE_END
		case MU_MSG_GO:
			bStartSelecting = true;
			SWITCH_MESSAGE_END
		case MU_MSG_WT_LAND:
			{
				sClientLandData *ClientData;
				ClientData = new sClientLandData;
				ClientData->LandingList = new cList<sLanding*>;
				ClientData->iLandX = Message->popInt16();
				ClientData->iLandY = Message->popInt16();
				ClientData->iNr = Message->popInt16();
				int iCount = Message->popInt16();
				for ( int i = 0; i < iCount; i++ )
				{
					sLanding *Landing;
					Landing = new sLanding;
					Landing->cargo = Message->popInt16();
					Landing->id = Message->popInt16();
					ClientData->LandingList->Add ( Landing );
				}

				ClientDataList->Add ( ClientData );
			}
			SWITCH_MESSAGE_END
		case MU_MSG_ALL_LANDED:
			bAllLanded = true;
			SWITCH_MESSAGE_END
		case MU_MSG_UPGRADES:
			{
				cPlayer *Player;
				int iPlayerNr = Message->popInt16();
				for ( int i = 0; i < PlayerList.iCount; i++ )
				{
					if ( PlayerList.Items[i]->Nr == iPlayerNr )
					{
						Player = PlayerList.Items[i];
						break;
					}
				}
				int iCount = Message->popInt16();
				for ( int i = 0; i < iCount; i++ )
				{
					bool bVehicle = Message->popBool();
					int iNum = Message->popInt16();
					if ( bVehicle )
					{
						Player->VehicleData[iNum].damage = Message->popInt16();
						Player->VehicleData[iNum].max_shots = Message->popInt16();
						Player->VehicleData[iNum].range = Message->popInt16();
						Player->VehicleData[iNum].max_ammo = Message->popInt16();
						Player->VehicleData[iNum].armor = Message->popInt16();
						Player->VehicleData[iNum].max_hit_points = Message->popInt16();
						Player->VehicleData[iNum].scan = Message->popInt16();
						Player->VehicleData[iNum].max_speed = Message->popInt16();
						Player->VehicleData[iNum].version++;
					}
					else
					{
						Player->BuildingData[iNum].damage = Message->popInt16();
						Player->BuildingData[iNum].max_shots = Message->popInt16();
						Player->BuildingData[iNum].range = Message->popInt16();
						Player->BuildingData[iNum].max_ammo = Message->popInt16();
						Player->BuildingData[iNum].armor = Message->popInt16();
						Player->BuildingData[iNum].max_hit_points = Message->popInt16();
						Player->BuildingData[iNum].scan = Message->popInt16();
						Player->BuildingData[iNum].version++;
					}
				}
			}
			SWITCH_MESSAGE_END
		default:
			SWITCH_MESSAGE_END
		}
	}
}

void cMultiPlayerMenu::sendLandingInfo( int iLandX, int iLandY, cList<sLanding*> *LandingList )
{
	cNetMessage *Message = new cNetMessage ( MU_MSG_WT_LAND );

	for ( int i = 0; i < LandingList->iCount; i++ )
	{
		// break if package will be to big
		// this is very improbable becouse player must have selected over 50 units
		// at the standard PACKAGE_LENGHT of 256bytes
		if ( Message->iLength+16 > PACKAGE_LENGHT-2 ) break;

		Message->pushInt16( LandingList->Items[i]->id );
		Message->pushInt16( LandingList->Items[i]->cargo );
	}
	Message->pushInt16( LandingList->iCount );
	Message->pushInt16( ActualPlayer->Nr );
	Message->pushInt16( iLandY );
	Message->pushInt16( iLandX );

	sendMessage ( Message );
}

void cMultiPlayerMenu::sendUpgrades()
{

	cNetMessage *Message = NULL;
	int iCount = 0;

	// send vehicles
	for (size_t i = 0; i < UnitsData.vehicle.Size(); ++i)
	{
		if ( Message == NULL )
		{
			Message = new cNetMessage ( MU_MSG_UPGRADES );
		}
		if ( ActualPlayer->VehicleData[i].damage != UnitsData.vehicle[i].data.damage ||
			ActualPlayer->VehicleData[i].max_shots != UnitsData.vehicle[i].data.max_shots ||
			ActualPlayer->VehicleData[i].range != UnitsData.vehicle[i].data.range ||
			ActualPlayer->VehicleData[i].max_ammo != UnitsData.vehicle[i].data.max_ammo ||
			ActualPlayer->VehicleData[i].armor != UnitsData.vehicle[i].data.armor ||
			ActualPlayer->VehicleData[i].max_hit_points != UnitsData.vehicle[i].data.max_hit_points ||
			ActualPlayer->VehicleData[i].scan != UnitsData.vehicle[i].data.scan ||
			ActualPlayer->VehicleData[i].max_speed != UnitsData.vehicle[i].data.max_speed )
		{
			Message->pushInt16( ActualPlayer->VehicleData[i].max_speed );
			Message->pushInt16( ActualPlayer->VehicleData[i].scan );
			Message->pushInt16( ActualPlayer->VehicleData[i].max_hit_points );
			Message->pushInt16( ActualPlayer->VehicleData[i].armor );
			Message->pushInt16( ActualPlayer->VehicleData[i].max_ammo );
			Message->pushInt16( ActualPlayer->VehicleData[i].range );
			Message->pushInt16( ActualPlayer->VehicleData[i].max_shots );
			Message->pushInt16( ActualPlayer->VehicleData[i].damage );
			Message->pushInt16( i );
			Message->pushBool( true ); // true for vehciles

			iCount++;
		}

		if ( Message->iLength+35 > PACKAGE_LENGHT-2 )
		{
			Message->pushInt16 ( iCount );
			Message->pushInt16 ( ActualPlayer->Nr );
			sendMessage ( Message );
			Message = NULL;
			iCount = 0;
		}
	}
	if ( Message != NULL )
	{
		Message->pushInt16 ( iCount );
		Message->pushInt16 ( ActualPlayer->Nr );
		sendMessage ( Message );
		Message = NULL;
		iCount = 0;
	}

	// send buildings
	for (size_t i = 0; i < UnitsData.building.Size(); ++i)
	{
		if ( Message == NULL )
		{
			Message = new cNetMessage ( MU_MSG_UPGRADES );
		}
		if ( ActualPlayer->BuildingData[i].damage != UnitsData.building[i].data.damage ||
			ActualPlayer->BuildingData[i].max_shots != UnitsData.building[i].data.max_shots ||
			ActualPlayer->BuildingData[i].range != UnitsData.building[i].data.range ||
			ActualPlayer->BuildingData[i].max_ammo != UnitsData.building[i].data.max_ammo ||
			ActualPlayer->BuildingData[i].armor != UnitsData.building[i].data.armor ||
			ActualPlayer->BuildingData[i].max_hit_points != UnitsData.building[i].data.max_hit_points ||
			ActualPlayer->BuildingData[i].scan != UnitsData.building[i].data.scan )
		{
			Message->pushInt16( ActualPlayer->BuildingData[i].scan );
			Message->pushInt16( ActualPlayer->BuildingData[i].max_hit_points );
			Message->pushInt16( ActualPlayer->BuildingData[i].armor );
			Message->pushInt16( ActualPlayer->BuildingData[i].max_ammo );
			Message->pushInt16( ActualPlayer->BuildingData[i].range );
			Message->pushInt16( ActualPlayer->BuildingData[i].max_shots );
			Message->pushInt16( ActualPlayer->BuildingData[i].damage );
			Message->pushInt16( i );
			Message->pushBool( false ); // false for buildings

			iCount++;
		}

		if ( Message->iLength+32 > PACKAGE_LENGHT-2 )
		{
			Message->pushInt16 ( iCount );
			Message->pushInt16 ( ActualPlayer->Nr );
			sendMessage ( Message );
			Message = NULL;
			iCount = 0;
		}
	}
	if ( Message != NULL )
	{
		Message->pushInt16 ( iCount );
		Message->pushInt16 ( ActualPlayer->Nr );
		sendMessage ( Message );
	}
}

int cMultiPlayerMenu::testAllReady()
{
	for ( int i = 0; i < PlayerList.iCount; i++ )
	{
		if ( ReadyList[i] != true ) return i;
	}
	return -1;
}

void cMultiPlayerMenu::sendIdentification()
{
	if ( bHost )
	{
		sendPlayerList();
		return;
	}

	int iPlayerNum;
	for ( iPlayerNum = 0; iPlayerNum < PlayerList.iCount; iPlayerNum++ )
	{
		if ( PlayerList.Items[iPlayerNum] == ActualPlayer ) break;
	}
	cNetMessage *Message = new cNetMessage ( MU_MSG_IDENTIFIKATION );
	Message->pushBool ( ReadyList[iPlayerNum] );
	Message->pushString ( ActualPlayer->name );
	Message->pushInt16( GetColorNr( ActualPlayer->color ) );
	Message->pushInt16( ActualPlayer->Nr );
	sendMessage ( Message );
}

void cMultiPlayerMenu::sendPlayerList()
{
	cNetMessage *Message = new cNetMessage ( MU_MSG_PLAYERLIST );

	for ( int i = 0; i < PlayerList.iCount; i++ )
	{
		Message->pushBool ( ReadyList[i] );
		Message->pushInt16 ( PlayerList.Items[i]->Nr );
		Message->pushInt16 ( GetColorNr( PlayerList.Items[i]->color ) );
		Message->pushString ( PlayerList.Items[i]->name );
	}
	Message->pushInt16 ( PlayerList.iCount );
	sendMessage ( Message );
}

void cMultiPlayerMenu::sendOptions()
{
	cNetMessage *Message = new cNetMessage ( MU_MSG_OPTINS );

	if ( !sSaveGame.empty() ) Message->pushString ( sSaveGame );
	Message->pushBool ( !sSaveGame.empty() );

	if ( !sMap.empty() ) Message->pushString ( sMap );
	Message->pushBool ( !sMap.empty() );

	if ( bOptions )
	{
		Message->pushBool ( Options.PlayRounds );
		Message->pushBool ( Options.AlienTech );
		Message->pushBool ( Options.FixedBridgeHead );
		Message->pushInt16 ( Options.credits );
		Message->pushInt16 ( Options.dichte );
		Message->pushInt16 ( Options.gold );
		Message->pushInt16 ( Options.oil );
		Message->pushInt16 ( Options.metal );
	}
	Message->pushBool ( bOptions );

	sendMessage ( Message );
}

void cMultiPlayerMenu::displayGameSettings()
{
	string OptionString;
	SDL_Rect rect;

	rect.x = 192; rect.y = 52;
	rect.w = 246; rect.h = 176;

	if( !bHost )
	{
		SDL_BlitSurface( sfTmp, &rect, buffer, &rect );
	}

	OptionString = lngPack.i18n ( "Text~Main~Version", MAX_VERSION ) + "\n";
	OptionString += "Checksum: " + iToStr( SettingsData.Checksum ) + "\n\n";

	if( !bHost && !network )
	{
		OptionString += lngPack.i18n ( "Text~Multiplayer~Network_Connected_Not" );
		font->showTextAsBlock( rect, OptionString );
	}

	if( !sSaveGame.empty() )
	{
		;
	}

	if ( !sMap.empty() )
	{
		string sMapName = sMap;
		sMapName.erase( sMapName.length() - 4 );
		if ( !FileExists( (SettingsData.sMapsPath + PATH_DELIMITER + sMap).c_str() ) )
		{
			OptionString += lngPack.i18n ( "Text~Error_Messages~ERROR_File_Not_Found", sMap );
		}
		else
		{
			SDL_Surface *sfMapPic;
			int size;
			SDL_RWops *fp = SDL_RWFromFile ( (SettingsData.sMapsPath + PATH_DELIMITER + sMap).c_str(),"rb" );
			if ( fp != NULL )
			{
				OptionString += lngPack.i18n ( "Text~Title~Map" ) + ": " + sMapName;
				SDL_RWseek ( fp, 5, SEEK_SET );
				size = SDL_ReadLE16( fp );
				OptionString += " (" + iToStr( size ) + "x" + iToStr( size ) + ")\n";

				sColor Palette[256];
				short sGraphCount;
				SDL_RWseek ( fp, 2 + size*size*3, SEEK_CUR );
				sGraphCount = SDL_ReadLE16( fp );
				SDL_RWseek ( fp, 64*64*sGraphCount, SEEK_CUR );
				SDL_RWread ( fp, &Palette, 1, 768 );

				sfMapPic = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size,8,0,0,0,0);
				sfMapPic->pitch = sfMapPic->w;

				sfMapPic->format->palette->ncolors = 256;
				for (int j = 0; j < 256; j++ )
				{
					sfMapPic->format->palette->colors[j].r = Palette[j].cBlue;
					sfMapPic->format->palette->colors[j].g = Palette[j].cGreen;
					sfMapPic->format->palette->colors[j].b = Palette[j].cRed;
				}
				SDL_RWseek ( fp, 9, SEEK_SET );
				for( int iY = 0; iY < size; iY++ )
				{
					for( int iX = 0; iX < size; iX++ )
					{
						unsigned char cColorOffset;
						SDL_RWread ( fp, &cColorOffset, 1, 1 );
						Uint8 *pixel = (Uint8*) sfMapPic->pixels  + (iY * size + iX);
						*pixel = cColorOffset;
					}
				}
				SDL_RWclose ( fp );
			}
			if ( sfMapPic != NULL )
			{
				SDL_Rect dest;
				dest.w = dest.h = 112;
				dest.x = 33;
				dest.y = 106;
				SDL_BlitSurface ( sfMapPic, NULL, buffer, &dest );
			}
			SDL_FreeSurface ( sfMapPic );
			font->showTextCentered( 90, 65, sMapName + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")" );
		}
	}
	else if ( sSaveGame.empty() )
	{
		OptionString += lngPack.i18n ( "Text~Multiplayer~Map_NoSet" ) + "\n";
	}
	OptionString += "\n";

	if ( sSaveGame.empty() )
	{
		if ( bOptions )
		{
			OptionString += lngPack.i18n ( "Text~Title~Metal" ) + ": " + ( Options.metal < 2 ? ( Options.metal < 1 ? lngPack.i18n ( "Text~Option~Low" ) : lngPack.i18n ( "Text~Option~Normal" ) ) : ( Options.metal < 3 ? lngPack.i18n ( "Text~Option~Much" ) : lngPack.i18n ( "Text~Option~Most" ) ) ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~Oil" ) + ": " + ( Options.oil < 2 ? ( Options.oil < 1 ? lngPack.i18n ( "Text~Option~Low" ) : lngPack.i18n ( "Text~Option~Normal" ) ) : ( Options.oil < 3 ? lngPack.i18n ( "Text~Option~Much" ) : lngPack.i18n ( "Text~Option~Most" ) ) ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~Gold" ) + ": " + ( Options.gold < 2 ? ( Options.gold < 1 ? lngPack.i18n ( "Text~Option~Low" ) : lngPack.i18n ( "Text~Option~Normal" ) ) : ( Options.gold < 3 ? lngPack.i18n ( "Text~Option~Much" ) : lngPack.i18n ( "Text~Option~Most" ) ) ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~Resource_Density" ) + ": " + ( Options.dichte < 2 ? ( Options.dichte < 1 ? lngPack.i18n ( "Text~Option~Thin" ) : lngPack.i18n ( "Text~Option~Normal" ) ) : ( Options.dichte < 3 ? lngPack.i18n ( "Text~Option~Thick" ) : lngPack.i18n ( "Text~Option~Most" ) ) ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~Credits" )  + ": " + iToStr( Options.credits ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~BridgeHead" ) + ": " + ( Options.FixedBridgeHead ? lngPack.i18n ( "Text~Option~Definite" ) : lngPack.i18n ( "Text~Option~Mobile" ) ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~Alien_Tech" ) + ": " + ( Options.AlienTech ? lngPack.i18n ( "Text~Option~On" ) : lngPack.i18n ( "Text~Option~Off" ) ) + "\n";
			OptionString += lngPack.i18n ( "Text~Title~Game_Type" ) + ": " + ( Options.PlayRounds ? lngPack.i18n ( "Text~Option~Type_Turns" ) : lngPack.i18n ( "Text~Option~Type_Simu" ) ) + "\n";
		}
		else
		{
			OptionString += lngPack.i18n ( "Text~Multiplayer~Option_NoSet" ) + "\n";
		}
	}

	font->showTextAsBlock ( rect, OptionString );
}

void cMultiPlayerMenu::displayPlayerList()
{
	SDL_Rect scr, dest;

	scr.x = 465;
	scr.y = 287;
	scr.w = 162;
	scr.h = 116;
	SDL_BlitSurface( sfTmp, &scr, buffer, &scr);

	scr.x = 0;
	scr.y = 0;
	dest.w = dest.h = scr.w = scr.h = 10;
	dest.x = 476;
	dest.y = 297;
	for( int i = 0; i < PlayerList.iCount; i++ )
	{
		SDL_BlitSurface( PlayerList.Items[i]->color, &scr, buffer, &dest );
		font->showText( dest.x+16, dest.y, PlayerList.Items[i]->name );

		if ( ReadyList[i] == false ) scr.x = 0; // red if not ready
		else scr.x = 10; // green if ready

		dest.x += 135;
		SDL_BlitSurface( GraphicsData.gfx_player_ready, &scr, buffer, &dest );
		dest.x = 476;

		dest.y+=16;
	}
}

// Startet ein Hot-Seat-Spiel:
void HeatTheSeat ( void )
{
	// get number of players:
	int iPlayerAnz = 0;
	sPlayerHS Players;

	Players = runPlayerSelectionHotSeat();

	for ( int i = 0; i < 8; i++ )
	{
		if( Players.what[i] != PLAYER_N )
		{
			iPlayerAnz ++;
		}
	}
	if( iPlayerAnz == 0 ) //got no players - cancel
	{
		return;
	}

	// generate the game:
	string sMapName;
	sOptions Options;

	Options = RunOptionsMenu ( NULL );
	if( Options.metal == -1 ) return;

	sMapName = RunPlanetSelect();
	if ( sMapName.empty() ) return;

	cList<sLanding*> *LandingList;
	int iLandX, iLandY;
	cPlayer *Player;

	cMap Map;
	if ( !Map.LoadMap ( sMapName ) )
	{
		return;
	}
	Map.PlaceRessources ( Options.metal, Options.oil, Options.gold, Options.dichte );
	// copy map for server
	cMap ServerMap;
	ServerMap.NewMap( Map.size, Map.TerrainInUse.iCount );
	ServerMap.DefaultWater = Map.DefaultWater;
	ServerMap.MapName = Map.MapName;
	memcpy ( ServerMap.Kacheln, Map.Kacheln, sizeof ( int )*Map.size*Map.size );
	memcpy ( ServerMap.Resources, Map.Resources, sizeof ( sResources )*Map.size*Map.size );
	for ( int i = 0; i < Map.TerrainInUse.iCount; i++ )
	{
		ServerMap.terrain[i].blocked = Map.terrain[i].blocked;
		ServerMap.terrain[i].coast = Map.terrain[i].coast;
		ServerMap.terrain[i].water = Map.terrain[i].water;
	}

	// player for client
	cList<cPlayer*> ClientPlayerList;
	int iPlayerNumber = 1;
	for ( int i = 0; i < 8; i++ )
	{
		if( Players.what[i] == PLAYER_H )
		{
			ClientPlayerList.Add ( Player = new cPlayer ( Players.name[i], OtherData.colors[Players.iColor[i]], iPlayerNumber, MAX_CLIENTS ) );
			Player->Credits = Options.credits;
			iPlayerNumber++;
		}
	}

	// playerlist for server
	cList<cPlayer*> ServerPlayerList;
	iPlayerNumber = 1;
	for ( int i = 0; i < 8; i++ )
	{
		if( Players.what[i] == PLAYER_H )
		{
			ServerPlayerList.Add ( Player = new cPlayer ( Players.name[i], OtherData.colors[Players.iColor[i]], iPlayerNumber, MAX_CLIENTS ) );
			Player->Credits = Options.credits;
			iPlayerNumber++;
		}
	}

	// init client
	Client = new cClient(&Map, &ClientPlayerList);
	for ( int i = 0; i < ClientPlayerList.iCount; i++ )
	{
		ClientPlayerList.Items[i]->InitMaps ( Map.size, &Map );
		ClientPlayerList.Items[i]->Credits = Options.credits;
		ClientPlayerList.Items[i]->HotHud = Client->Hud;
	}

	// init server
	Server = new cServer(&ServerMap, &ServerPlayerList, GAME_TYPE_HOTSEAT, false);

	// land the players
	for ( int i = 0; i < ServerPlayerList.iCount; i++ )
	{
		Player = ServerPlayerList.Items[i];
		Player->InitMaps ( Map.size, &ServerMap );
		Player->Credits = Options.credits;

		ShowOK ( Player->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn" ), true );

		LandingList = new cList<sLanding*>;
		RunHangar ( ClientPlayerList.Items[i], LandingList );

		SelectLanding ( &iLandX, &iLandY, &Map );

		Server->makeLanding ( iLandX, iLandY, Player, LandingList, Options.FixedBridgeHead );

		while ( LandingList->iCount )
		{
			delete LandingList->Items[0];
			LandingList->Delete ( 0 );
		}
		delete LandingList;
	}

	// exit menu and start game
	ExitMenu();

	Player = ClientPlayerList.Items[0];
	Client->initPlayer ( Player );
	Client->Hud = Player->HotHud;
	ShowOK ( Player->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn" ), true );
	Server->bStarted = true;
	Client->run();

	while ( ClientPlayerList.iCount )
	{
		delete ( ClientPlayerList.Items[0] );
		ClientPlayerList.Delete ( 0 );
	}
	delete Client; Client = NULL;
	delete Server; Server = NULL;
}

// Zeigt das Laden Menü an:
int ShowDateiMenu ( bool bSave )
{
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOG_W / 2, SettingsData.iScreenH / 2 - DIALOG_H / 2, DIALOG_W, DIALOG_H };
	SDL_Rect scr;
	int LastMouseX=0,LastMouseY=0,LastB=0,x,b,y,offset=0,selected=-1;
	bool SpeichernPressed=false, FertigPressed=false, UpPressed=false, DownPressed=false;
	bool  BeendenPressed=false, LadenPressed=false, Cursor=true;
	Uint8 *keystate;
	cList<string> *files;
	SDL_Rect rBtnBack = {rDialog.x+353,rDialog.y+438, 106, 40};
	SDL_Rect rBtnExit = {rDialog.x+246,rDialog.y+438, 106, 40};
	SDL_Rect rBtnLoad = {rDialog.x+514,rDialog.y+438, 106, 40};
	SDL_Rect rBtnSave = {rDialog.x+132,rDialog.y+438, 106, 40};
	SDL_Rect rArrowUp = {rDialog.x+33, rDialog.y+438, 28, 29};
	SDL_Rect rArrowDown = {rDialog.x+63, rDialog.y+438, 28, 29};
	SDL_Rect rTitle = { rDialog.x+320, rDialog.y+12, 150, 12 };

	PlayFX ( SoundData.SNDHudButton );
	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	// Den Bildschirm blitten:
	SDL_BlitSurface ( GraphicsData.gfx_load_save_menu,NULL,buffer,&rDialog );
	// Den Text anzeigen:
	if ( bSave )
		font->showTextCentered(rTitle.x, rTitle.y, lngPack.i18n ( "Text~Title~LoadSave" ));
	else
		font->showTextCentered(rTitle.x, rTitle.y, lngPack.i18n ( "Text~Title~Load" ));

	// Buttons setzen;
	drawButtonBig( lngPack.i18n ( "Text~Button~Back" ),false,rBtnBack.x,rBtnBack.y,buffer );
	if ( bSave )
	{
		drawButtonBig( lngPack.i18n ( "Text~Button~Save" ),false,rBtnSave.x,rBtnSave.y,buffer );
		drawButtonBig( lngPack.i18n ( "Text~Button~Exit" ),false,rBtnExit.x,rBtnExit.y,buffer );
	}
	else
	{
		drawButtonBig( lngPack.i18n ( "Text~Button~Load" ),false,rBtnLoad.x,rBtnLoad.y,buffer );
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
	for ( int i = 0; i < files->iCount; i++ )
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
		EventHandler->HandleEvents();

		// Tasten prüfen:
		if ( Client )
		{
			Client->handleTimer();
			Client->doGameActions();
		}
		keystate = SDL_GetKeyState( NULL );
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
				drawButtonBig( lngPack.i18n ( "Text~Button~Back" ),true,rBtnBack.x,rBtnBack.y,buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				delete files;
				return -1;
			}
		}
		else if ( FertigPressed )
		{
			drawButtonBig( lngPack.i18n ( "Text~Button~Back" ),false,rBtnBack.x,rBtnBack.y,buffer );
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
				drawButtonBig( lngPack.i18n ( "Text~Button~Exit" ),true,rBtnExit.x,rBtnExit.y,buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				BeendenPressed=true;
			}
			else if ( !b&&LastB )
			{
				drawButtonBig( lngPack.i18n ( "Text~Button~Exit" ),false,rBtnExit.x,rBtnExit.y,buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				BeendenPressed=false;
				Client->bExit = true;
				delete files;
				return -1;
			}
		}
		else if ( BeendenPressed )
		{
			drawButtonBig( lngPack.i18n ( "Text~Button~Exit" ),false,rBtnExit.x,rBtnExit.y,buffer );
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
				drawButtonBig( lngPack.i18n ( "Text~Button~Save" ),true,rBtnSave.x,rBtnSave.y,buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SpeichernPressed=true;
			}
			else if ( !b&&LastB )
			{
				drawButtonBig( lngPack.i18n ( "Text~Button~Save" ),false,rBtnSave.x,rBtnSave.y,buffer );
				if ( selected != -1 )
				{
					ShowFiles ( files,offset,selected,true,false,false, rDialog );
					if ( game->Save ( SaveLoadFile, SaveLoadNumber ) )
					{
						delete files;
						files = getFilesOfDirectory ( SettingsData.sSavesPath );
						for ( int i = 0; i < files->iCount; i++ )
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
			drawButtonBig( lngPack.i18n ( "Text~Button~Save" ),false,rBtnSave.x,rBtnSave.y,buffer );
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
				drawButtonBig( lngPack.i18n ( "Text~Button~Load" ),true,rBtnLoad.x,rBtnLoad.y,buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				LadenPressed=true;
			}
			else if ( !b&&LastB )
			{
				drawButtonBig( lngPack.i18n ( "Text~Button~Load" ),false,rBtnLoad.x,rBtnLoad.y,buffer );
				if ( selected != -1 )
				{
					ShowFiles ( files,offset,selected,false,false,false, rDialog );
					delete files;
					return 1;
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
				LadenPressed=false;
			}
		}
		else if ( LadenPressed )
		{
			drawButtonBig( lngPack.i18n ( "Text~Button~Load" ),false,rBtnLoad.x,rBtnLoad.y,buffer );
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
	// Ignore version
	SDL_RWread(pFile, &iLenght, sizeof ( int ), 1);
	SDL_RWseek(pFile, iLenght, SEEK_CUR);

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
void ShowFiles ( cList<string> *files, int offset, int selected, bool bSave, bool bCursor, bool bFirstSelect, SDL_Rect rDialog )

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
			font->showTextCentered(x, y, iToStr ( offset + i + 1 ), LATIN_BIG_GOLD);
		}
		else
		{
			font->showTextCentered(x, y, iToStr ( offset + i + 1 ), LATIN_BIG);
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
			for ( int j = 0; j < files->iCount || j == 0; j++ )
			{
				int iSaveNumber;
				string sFilename, sTime, sMode;
				if ( files->iCount > 0 )
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
					font->showText(x, y, sFilename);
					// Zeit und Modus ausgeben
					font->showText(x, y-23, sTime);
					font->showText(x+113,y-23, sMode);
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
					font->showText(x, y, sFilename);
				}
			}
		}
		else
		{
			for ( int j = 0; j < files->iCount; j++ )
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

					font->showText(x, y, sFilename);

					// Zeit und Modus ausgeben
					font->showText(x, y - 23, sTime);
					font->showText(x + 113, y - 23, sMode);
					break;
				}
			}
		}
		y += 76;
	}
}

void cMultiPlayerMenu::sendMessage( cNetMessage *Message, int iPlayer )
{
	if ( iPlayer == -1 )
	{
		network->send ( Message->iLength, Message->serialize( true ) );
	}
	else
	{
		network->sendTo ( iPlayer, Message->iLength, Message->serialize( true ) );
	}
	delete Message;
}
