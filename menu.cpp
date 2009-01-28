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
#include "unifonts.h"
#include "mouse.h"
#include "sound.h"
#include "dialog.h"
#include "log.h"
#include "files.h"
#include "loaddata.h"
#include "events.h"
#include "client.h"
#include "server.h"
#include "serverevents.h"
#include "upgradecalculator.h"
#include "loaddata.h"
#include "input.h"

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

sClientLandData::sClientLandData()
{
	iLandX = 0;
	iLandY = 0;
	landingState = LANDING_STATE_UNKNOWN;
	iLastLandX = 0;
	iLastLandY = 0;
};

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
	font->showTextCentered(DIALOG_X+320,DIALOG_Y+465, lngPack.i18n ( "Text~Main~Credits_Reloaded" )+ " "+PACKAGE_VERSION);
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
			iUnitShow = random((int)UnitsData.building.Size());
		}
		while ( iUnitShow == s_iLastUnitShown );	//make sure we don't show same unit twice
		SDL_BlitSurface ( UnitsData.building[iUnitShow].info,NULL,buffer,&rDest );
	}
	else //and a 66% chance to show a vehicle on 0 or 2
	{
		do
		{
			iUnitShow = random((int)UnitsData.vehicle.Size());
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
	SDL_Rect dest = { DIALOG_X , DIALOG_Y, DIALOG_W, DIALOG_H};
	int len;
	len = font->getTextWide(sText);

	if ( center )
	{
		font->showTextCentered(x+DIALOG_X, y+DIALOG_Y, sText);
	}
	else
	{
		font->showText(x+DIALOG_X, y+DIALOG_Y, sText);
		x+=len/2;
	}
	r.x=x-len/2-4;
	r.w=len+8;
	r.h=1;
	r.y=y-2;
	dest = r;
	dest.x += DIALOG_X;
	dest.y += DIALOG_Y;

	if ( checked ) SDL_FillRect ( buffer,&dest,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&dest );
	r.y+=14;
	r.w++;
	dest = r;
	dest.x += DIALOG_X;
	dest.y += DIALOG_Y;
	if ( checked ) SDL_FillRect ( buffer,&dest,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&dest );
	r.y-=14;
	r.w=1;
	r.h=14;
	dest = r;
	dest.x += DIALOG_X;
	dest.y += DIALOG_Y;
	if ( checked ) SDL_FillRect ( buffer,&dest,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&dest );
	r.x+=len+8;
	dest = r;
	dest.x += DIALOG_X;
	dest.y += DIALOG_Y;
	if ( checked ) SDL_FillRect ( buffer,&dest,0xE3DACF );else SDL_BlitSurface ( surface,&r,buffer,&dest );
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
#endif
	MenuButton btn_preferences(BTN_4_X, BTN_4_Y, "Text~Settings~Preferences");
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
#endif
			btn_preferences.Draw();
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
#endif
		if (btn_preferences.CheckClick(x, y, down, up))
		{

				showPreferences();

			EscHot = false;
			redraw = true;
		}
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
			cMultiPlayerMenu m(true);
			MultiPlayerMenu = &m;
			m.runNetworkMenu();
			MultiPlayerMenu = 0;
			break;
		}
		if (btn_client.CheckClick(x, y, down, up))
		{
			cMultiPlayerMenu m(false);
			MultiPlayerMenu = &m;
			m.runNetworkMenu();
			MultiPlayerMenu = 0;
			break;
		}
		if (btn_new_hot.CheckClick(x, y, down, up))
		{
#ifdef RELEASE
			ShowOK(lngPack.i18n("Text~Error_Messages~INFO_Not_Implemented"), true);
#else
			HeatTheSeat();
			break;
#endif
		}
		if (btn_load_hot.CheckClick(x, y, down, up))
		{
#ifdef RELEASE
			ShowOK(lngPack.i18n("Text~Error_Messages~INFO_Not_Implemented"), true);
#else
			if (ShowDateiMenu(false) != -1)
			{
				if ( !SaveLoadFile.empty() )
				{
					/*cSavegame Savegame ( SaveLoadNumber );
					Savegame.load();
					ExitMenu();*/
				}
				break;
			}
			prepareMenu();
			RunSPMenu();
			break;
#endif
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
				ServerMap.NewMap( Map.size, Map.iNumberOfTerrains );
				ServerMap.MapName = Map.MapName;
				memcpy ( ServerMap.Kacheln, Map.Kacheln, sizeof ( int )*Map.size*Map.size );
				memcpy ( ServerMap.Resources, Map.Resources, sizeof ( sResources )*Map.size*Map.size );
				for ( int i = 0; i < Map.iNumberOfTerrains; i++ )
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

				// init client and his player
				Client = new cClient(&Map, &ClientPlayerList);
				Client->isInMenu = true;
				Client->initPlayer ( Player );
				for ( unsigned int i = 0; i < ClientPlayerList.Size(); i++ )
				{
					ClientPlayerList[i]->InitMaps(Map.size, &Map);
					ClientPlayerList[i]->Credits = options.credits;
				}

				// run the hangar
				cList<sLanding> landingList;
				RunHangar ( Player, &landingList );

				// playerlist for server
				cList<cPlayer*> ServerPlayerList;
				ServerPlayerList.Add ( new cPlayer ( (*Player) ) ); // Socketnumber MAX_CLIENTS for lokal client

				// init the players of playerlist
				for ( unsigned int i = 0; i < ServerPlayerList.Size(); i++ )
				{
					ServerPlayerList[i]->InitMaps(ServerMap.size, &ServerMap);
				}
				// init server
				Server = new cServer(&ServerMap, &ServerPlayerList, GAME_TYPE_SINGLE, false);

				sClientLandData c;
				cSelectLandingMenu landingMenu( &Map, &c, 1 );
				landingMenu.run();

				Server->makeLanding(c.iLandX, c.iLandY, ServerPlayerList[0], landingList, options.FixedBridgeHead);

				// exit menu and start game
				ExitMenu();

				Server->bStarted = true;
				Client->isInMenu = false;
				Client->run();

				SettingsData.sPlayerName = Player->name;
				while ( ClientPlayerList.Size() )
				{
					delete ClientPlayerList[0];
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
				ExitMenu();
				if (!SaveLoadFile.empty())
				{
					cSavegame Savegame ( SaveLoadNumber );
					if ( Savegame.load() == 1 )
					{
						// copy map for client
						cMap ClientMap;
						ClientMap.LoadMap ( Server->Map->MapName );

						cList<cPlayer*> ClientPlayerList;

						// copy players for client
						for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
						{
							ClientPlayerList.Add( new cPlayer( *(*Server->PlayerList)[i] ) );
							// reinit unit values
							for ( unsigned int j = 0; j < UnitsData.vehicle.Size(); j++) ClientPlayerList[i]->VehicleData[j] = UnitsData.vehicle[j].data;
							for ( unsigned int j = 0; j < UnitsData.building.Size(); j++) ClientPlayerList[i]->BuildingData[j] = UnitsData.building[j].data;
						}
						// reinit unitvalues
						// init client and his player
						Client = new cClient( &ClientMap, &ClientPlayerList );
						Client->isInMenu = true;
						Client->initPlayer ( ClientPlayerList[0] );
						for ( unsigned int i = 0; i < ClientPlayerList.Size(); i++ )
						{
							ClientPlayerList[i]->InitMaps( ClientMap.size, &ClientMap );
						}

						// in singleplayer only the first player is important
						(*Server->PlayerList)[0]->iSocketNum = MAX_CLIENTS;
						Server->resyncPlayer ( (*Server->PlayerList)[0] );

						// exit menu and start game
						Server->bStarted = true;
						Client->isInMenu = false;
						Client->run();

						delete Client;
						Client = NULL;
						delete Server;
						Server = NULL;

						reloadUnitValues();
					}
				}
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
	SDL_Rect dest = { DIALOG_X , DIALOG_Y, DIALOG_W, DIALOG_H};

	//need a tmpsf since I can't tell LoadPCXtoSF any dest
	//what is vital for resolutions > 640*480
	sfTmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, DIALOG_W, DIALOG_H, SettingsData.iColourDepth,0,0,0,0 );
	LoadPCXtoSF ( GFXOD_OPTIONS,sfTmp );

 	//some menus don't support bigger resolutions yet and to
 	// prevent old graphic garbage in the background we refill
 	// with black -- beko
	SDL_FillRect(buffer, NULL, 0x0000);

	//blit sfTmp to buffer
	SDL_BlitSurface (sfTmp, NULL, buffer, &dest);
	font->showTextCentered(DIALOG_X + 320, DIALOG_Y + 11, GAMEOPTIONS);

	// Ressourcen:
	font->showTextCentered(DIALOG_X + 110,DIALOG_Y + 56, lngPack.i18n ( "Text~Title~Resource" ));

	font->showText(DIALOG_X + 17,DIALOG_Y + 86, METAL);
	placeSelectableText ( LOW,38,86+16,options.metal==0, sfTmp );
	placeSelectableText ( MIDDLE,38+45,86+16,options.metal==1, sfTmp);
	placeSelectableText ( MUCH,38+45*2,86+16,options.metal==2, sfTmp );
	placeSelectableText ( MOST,38+45*3,86+16,options.metal==3, sfTmp );

	font->showText(DIALOG_X + 17, DIALOG_Y + 124, OIL);
	placeSelectableText ( LOW,38,124+16,options.oil==0, sfTmp );
	placeSelectableText ( MIDDLE,38+45,124+16,options.oil==1 , sfTmp);
	placeSelectableText ( MUCH,38+45*2,124+16,options.oil==2, sfTmp );
	placeSelectableText ( MOST,38+45*3,124+16,options.oil==3, sfTmp );

	font->showText(DIALOG_X + 17, DIALOG_Y + 162, GOLD);
	placeSelectableText ( LOW,38,162+16,options.gold==0, sfTmp );
	placeSelectableText ( MIDDLE,38+45,162+16,options.gold==1, sfTmp );
	placeSelectableText ( MUCH,38+45*2,162+16,options.gold==2 , sfTmp);
	placeSelectableText ( MOST,38+45*3,162+16,options.gold==3, sfTmp );

	// Credits:
	font->showTextCentered(DIALOG_X + 110+211, DIALOG_Y + 56, CREDITS);

	placeSelectableText ( LOWEST,110+130,86,options.credits==25, sfTmp,false );
	placeSelectableText ( LOWER,110+130,86+20,options.credits==50, sfTmp,false );
	placeSelectableText ( LOW,110+130,86+20*2,options.credits==100, sfTmp,false );
	placeSelectableText ( MIDDLE,110+130,86+20*3,options.credits==150, sfTmp,false );
	placeSelectableText ( MUCH,110+130,86+20*4,options.credits==200, sfTmp,false );
	placeSelectableText ( MORE,110+130,86+20*5,options.credits==250, sfTmp,false );
	placeSelectableText ( MOST,110+130,86+20*6,options.credits==300, sfTmp,false );

	// Brückenkopf:
	font->showTextCentered(DIALOG_X + 110+211*2, DIALOG_Y + 56, HEAD);

	placeSelectableText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
	placeSelectableText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );

	// AlienTechs:
	font->showTextCentered(DIALOG_X + 110, DIALOG_Y + 251, ALIEN);

	placeSelectableText ( ON,38,281,options.AlienTech, sfTmp );
	placeSelectableText ( OFF,38,281+20,!options.AlienTech, sfTmp );

	// Ressourcendichte:
	font->showTextCentered(DIALOG_X + 110+211, DIALOG_Y + 251, RESOURCE);

	placeSelectableText ( THIN,110+130,281,options.dichte==0, sfTmp,false );
	placeSelectableText ( MIDDLE,110+130,281+20,options.dichte==1, sfTmp,false );
	placeSelectableText ( THICK,110+130,281+20*2,options.dichte==2, sfTmp,false );
	placeSelectableText ( MOST,110+130,281+20*3,options.dichte==3, sfTmp,false );

	// Spielart:
	font->showTextCentered(DIALOG_X + 110+211*2, DIALOG_Y + 251, GAMETYPE);


	placeSelectableText ( SIMU,452,281,!options.PlayRounds, sfTmp,false );
	placeSelectableText ( TURNS,452,281+20,options.PlayRounds, sfTmp,false );

	MenuButton btn_back( DIALOG_X + 50, DIALOG_Y + 440, "Text~Button~Back");
	MenuButton btn_ok(  DIALOG_X + 390, DIALOG_Y + 440, "Text~Button~OK");

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
		if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20&&mouse->x< DIALOG_X + 38+20&&mouse->y>=DIALOG_Y + 86+16-4&&mouse->y<DIALOG_Y + 86+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45&&mouse->x<DIALOG_X + 38+20+45&&mouse->y>=DIALOG_Y + 86+16-4&&mouse->y<DIALOG_Y + 86+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45*2&&mouse->x<DIALOG_X + 38+20+45*2&&mouse->y>=DIALOG_Y + 86+16-4&&mouse->y<DIALOG_Y + 86+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45*3&&mouse->x<DIALOG_X + 38+20+45*3&&mouse->y>=DIALOG_Y + 86+16-4&&mouse->y<DIALOG_Y + 86+16-4+14 )
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
		if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20&&mouse->x<DIALOG_X + 38+20&&mouse->y>=DIALOG_Y + 124+16-4&&mouse->y<DIALOG_Y + 124+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45&&mouse->x<DIALOG_X + 38+20+45&&mouse->y>=DIALOG_Y + 124+16-4&&mouse->y<DIALOG_Y + 124+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45*2&&mouse->x<DIALOG_X + 38+20+45*2&&mouse->y>=DIALOG_Y + 124+16-4&&mouse->y<DIALOG_Y + 124+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45*3&&mouse->x<DIALOG_X + 38+20+45*3&&mouse->y>=DIALOG_Y + 124+16-4&&mouse->y<DIALOG_Y + 124+16-4+14 )
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
		if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20&&mouse->x<DIALOG_X + 38+20&&mouse->y>=DIALOG_Y + 162+16-4&&mouse->y<DIALOG_Y + 162+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45&&mouse->x<DIALOG_X + 38+20+45&&mouse->y>=DIALOG_Y + 162+16-4&&mouse->y<DIALOG_Y + 162+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45*2&&mouse->x<DIALOG_X + 38+20+45*2&&mouse->y>=DIALOG_Y + 162+16-4&&mouse->y<DIALOG_Y + 162+16-4+14 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 38-20+45*3&&mouse->x<DIALOG_X + 38+20+45*3&&mouse->y>=DIALOG_Y + 162+16-4&&mouse->y<DIALOG_Y + 162+16-4+14 )
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
		if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4&&mouse->y<DIALOG_Y + 86-4+20 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4+20&&mouse->y<DIALOG_Y + 86-4+20+20 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4+20*2&&mouse->y<DIALOG_Y + 86-4+20+20*2 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4+20*3&&mouse->y<DIALOG_Y + 86-4+20+20*3 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4+20*4&&mouse->y<DIALOG_Y + 86-4+20+20*4 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4+20*5&&mouse->y<DIALOG_Y + 86-4+20+20*5 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 86-4+20*6&&mouse->y<DIALOG_Y + 86-4+20+20*6 )
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
		if ( b&&!lb&&mouse->x>=DIALOG_X + 452&&mouse->x<DIALOG_X + 452+100&&mouse->y>=DIALOG_Y + 86-4&&mouse->y<DIALOG_Y + 86-4+14 )
		{
			options.FixedBridgeHead=false;
			placeSelectableText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
			placeSelectableText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 452&&mouse->x<DIALOG_X + 452+100&&mouse->y>=DIALOG_Y + 86-4+20&&mouse->y<DIALOG_Y + 86-4+14+20 )
		{
			options.FixedBridgeHead=true;
			placeSelectableText ( MOBILE,452,86,!options.FixedBridgeHead, sfTmp,false );
			placeSelectableText ( DEFINITE,452,86+20,options.FixedBridgeHead, sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// AlienTech:
		if ( b&&!lb&&mouse->x>=DIALOG_X + 30&&mouse->x<DIALOG_X + 38+100&&mouse->y>=DIALOG_Y + 281-4&&mouse->y<DIALOG_Y + 281-4+14 )
		{
			options.AlienTech=true;
			placeSelectableText ( ON,38,281,options.AlienTech, sfTmp );
			placeSelectableText ( OFF,38,281+20,!options.AlienTech, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 30&&mouse->x<DIALOG_X + 38+100&&mouse->y>=DIALOG_Y + 281-4+20&&mouse->y<DIALOG_Y + 281-4+14+20 )
		{
			options.AlienTech=false;
			placeSelectableText ( ON,38,281,options.AlienTech, sfTmp );
			placeSelectableText ( OFF,38,281+20,!options.AlienTech, sfTmp );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Ressourcendichte:
		if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 281-4&&mouse->y<DIALOG_Y + 281-4+20 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 281+20-4&&mouse->y<DIALOG_Y + 281+20-4+20 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 281+20*2-4&&mouse->y<DIALOG_Y + 281+20*2-4+20 )
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
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 110+130&&mouse->x<DIALOG_X + 110+130+100&&mouse->y>=DIALOG_Y + 281+20*3-4&&mouse->y<DIALOG_Y + 281+20*3-4+20 )
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
		if ( b&&!lb&&mouse->x>=DIALOG_X + 452&&mouse->x<DIALOG_X + 452+100&&mouse->y>=DIALOG_Y + 281-4&&mouse->y<DIALOG_Y + 281-4+20 )
		{
			options.PlayRounds=false;
			placeSelectableText ( SIMU,452,281,!options.PlayRounds,sfTmp,false );
			placeSelectableText ( TURNS,452,281+20,options.PlayRounds,sfTmp,false );
			PlayFX ( SoundData.SNDObjectMenu );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&mouse->x>=DIALOG_X + 452&&mouse->x<DIALOG_X + 452+100&&mouse->y>=DIALOG_Y + 281+20-4&&mouse->y<DIALOG_Y + 281+20-4+20 )
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


static void ShowPlanets(cList<string>* files, int offset, int selected, SDL_Surface* surface);


// Startet die Planetenauswahl (gibt den Namen des Planeten zurück):
string RunPlanetSelect ( void )
{
	Uint8 *keystate;
	int b,lb=0,offset=0,selected=-1,i,lx=-1,ly=-1;
	cList<string> *files;
	SDL_Rect scr;
	SDL_Rect dest = { DIALOG_X , DIALOG_Y, DIALOG_W, DIALOG_H};

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
	SDL_BlitSurface (sfTmp, NULL, buffer, &dest); 

	font->showTextCentered(DIALOG_X + 320, DIALOG_Y + 11, lngPack.i18n ( "Text~Title~Choose_Planet" ));

	MenuButton btn_back( DIALOG_X + 50, DIALOG_Y + 440, "Text~Button~Back");
	MenuButton btn_ok(  DIALOG_X + 390, DIALOG_Y + 440, "Text~Button~OK");

	btn_back.Draw();
	btn_ok.Lock();

	files = getFilesOfDirectory ( SettingsData.sMapsPath );
	for ( unsigned int i = 0; i < files->Size(); i++ )
	{
		string const& f = (*files)[i];
		if (f.substr(f.length() - 3, 3).compare("WRL") != 0 && f.substr(f.length() - 3, 3).compare("wrl") != 0)
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
			string name = (*files)[selected];
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
		if ( mouse->x>=DIALOG_X + 293&&mouse->x<DIALOG_X + 293+24&&mouse->y>=DIALOG_Y + 440&&mouse->y<DIALOG_Y + 440+24&&b&&!lb&&offset>0 )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			offset-=4;
			ShowPlanets ( files,offset,selected, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Down:
		if ( mouse->x>=DIALOG_X + 321&&mouse->x<DIALOG_X + 321+24&&mouse->y>=DIALOG_Y + 440&&mouse->y<DIALOG_Y + 440+24&&b&&!lb&&files->Size()-8-offset>0 )
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
			scr.x=DIALOG_X + 25;
			scr.y=DIALOG_Y + 90;
			for ( i=0;i<8;i++ )
			{
				if ( i+offset>=(int)files->Size() ) break;

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
					scr.x=DIALOG_X + 25;
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
static void ShowPlanets(cList<string>* const files, int const offset, int const selected, SDL_Surface* const surface)
{
	SDL_Surface *sf;
	SDL_Rect dest = { DIALOG_X , DIALOG_Y + 38, DIALOG_W, DIALOG_H};
	SDL_Rect scr={0, 38, 640, 390};
	string sMap;
	string sPath;
	int size;
	SDL_RWops *fp;

	SDL_BlitSurface ( surface,&scr,buffer,&dest );

	//scr.x=25; scr.y=90; scr.h=scr.w=112;
	dest.x=DIALOG_X+25; dest.y=DIALOG_Y+90;

	Log.write ( "Loading Maps", cLog::eLOG_TYPE_INFO );

	for ( int i=0;i<8;i++ ) //only 8 maps on one screen
	{
		if ( i+offset>=(int)files->Size() ) break;
		sMap = (*files)[i + offset];
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
				SDL_BlitSurface ( sf,NULL,buffer,&dest );
			}

			SDL_Rect r = dest;
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

				if ( font->getTextWide ( ">" + sMap.substr( 0, sMap.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
				{
					while ( font->getTextWide ( ">" + sMap + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
					{
						sMap.erase ( sMap.length()-1, sMap.length() );
					}
					sMap = ">" + sMap + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<";
				}
				else sMap = ">" + sMap.substr( 0, sMap.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<";
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

				if ( font->getTextWide ( ">" + sMap.substr( 0, sMap.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
				{
					while ( font->getTextWide ( ">" + sMap + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
					{
						sMap.erase ( sMap.length()-1, sMap.length() );
					}
					sMap = sMap + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")";
				}
				else sMap = sMap.substr( 0, sMap.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")";
			}
			font->showTextCentered(dest.x+77-21,dest.y-42, sMap);

			dest.x+=158;
			if ( i==3 )
			{
				dest.x=DIALOG_X+25;
				dest.y+=197;
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
	Log.mark();

	// Die Up-Down Buttons machen:

	dest.x=DIALOG_X + 293;
	dest.y=DIALOG_Y + 440;
	scr.h=scr.w=25;

	if ( offset )
	{
		scr.x=130;scr.y=452;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		scr.x=293;scr.y=440;
		SDL_BlitSurface ( surface,&scr,buffer,&dest );

	}

	dest.x=DIALOG_X + 321;
	dest.y=DIALOG_Y + 440;

	if ( files->Size()-8-offset>0 )
	{
		scr.x=103; scr.y=452;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		scr.x=321; scr.y=440;
		SDL_BlitSurface ( surface,&scr,buffer,&dest );
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

//FIXME: this menu is aged. Don't waste your time on this. Settings should be combined with the menu from hot seat games. To save some time for test games startup we simply return with 1 player and 1 (not implemented) AI -- beko
	return players;

	SDL_FillRect(buffer, NULL, 0x0000);
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
	norm1.w = norm2.w = 35;
	norm1.h = norm2.h = 35;
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
	norm1.w = norm2.w = 55;
	norm1.h = norm2.h = 71;
	dest.x = 394;
	dest.y = 67;
	for ( int i = 0; i<4;i++ )
	{
		dest.y = norm1.y = norm2.y = 67 + 92*i;
		switch ( players.what[i] )
		{
			// Nichts
			case PLAYER_N :
				norm1.x = 394 - 110;
				norm2.x = 394 - 219;
				dest.x = 394;
				SDL_BlitSurface ( GraphicsData.gfx_player_none,NULL,buffer,&dest );
			break;
			// Spieler
			case PLAYER_H :
				norm1.x = 394 - 110;
				norm2.x = 394;
				dest.x = 394 - 219;
				SDL_BlitSurface ( GraphicsData.gfx_player_human,NULL,buffer,&dest );
			break;
			// Computer
			case PLAYER_AI :
				norm1.x = 394;
				norm2.x = 394 - 219;
				dest.x = 394 - 110;
				SDL_BlitSurface ( GraphicsData.gfx_player_pc,NULL,buffer,&dest );
			break;
		}
		SDL_BlitSurface ( GraphicsData.gfx_player_select,&norm1,buffer,&norm1 );
		SDL_BlitSurface ( GraphicsData.gfx_player_select,&norm2,buffer,&norm2 );
	}
}

// Zeigt den Hngar an:
void ShowBars ( int credits,int StartCredits,cList<sLanding> *landing,int selected, SDL_Surface *surface );
void MakeUpgradeSliderVehicle ( sUpgrades *u,int nr,cPlayer *p );
void MakeUpgradeSliderBuilding ( sUpgrades *u,int nr,cPlayer *p );
void MakeUpgradeSubButtons ( bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf, SDL_Surface *surface );

static void ShowSelectionList(cList<sHUp*>& list, int selected, int offset, bool beschreibung, int credits, cPlayer* p);
static void CreateSelectionList(cList<sHUp*>& selection, cList<sHUp*>& images, int* selected, int* offset, bool tank, bool plane, bool ship, bool build, bool tnt, bool kauf);

void RunHangar ( cPlayer *player,cList<sLanding> *LandingList )
{
	bool tank=true,plane=false,ship=false,build=false,tnt=false,kauf=true;
	bool Beschreibung=SettingsData.bShowDescription;
	bool DownPressed=false,UpPressed=false;
	bool Down2Pressed=false,Up2Pressed=false;
	bool LadungUpPressed=false,LadungDownPressed=false;
	int b,lb=0,selected=0,offset=0,x,y,StartCredits=player->Credits,lx=-1,ly=-1;
	int LandingOffset=0,LandingSelected=0;
	SDL_Rect scr;

	#define BUTTON_W 77
	#define BUTTON_H 23

	SDL_Rect rTxtDescription = {DIALOG_X + 141, DIALOG_Y + 266,150,13};

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

	SDL_BlitSurface (sfTmp, NULL, buffer, &dest); //FIXME: making this working > 640x480 breaks some other ingame menus like upgrading from gold raffinery - those have to be fixed for higher resolutions too -- beko

	NormalButton btn_done ( DIALOG_X + 447, DIALOG_Y + 452, "Text~Button~Done" );
	NormalButton btn_buy ( DIALOG_X + 561, DIALOG_Y +388, "Text~Button~Buy" );
	NormalButton btn_delete ( DIALOG_X + 388, DIALOG_Y +240, "Text~Button~Delete" );

	btn_done.Draw();
	btn_buy.Draw();
	btn_delete.Draw();
	font->showTextCentered(rTxtDescription.x+rTxtDescription.w/2, rTxtDescription.y, lngPack.i18n( "Text~Comp~Description" ));

	//blit sfTmp to buffer



	// Die Liste erstellen:
	cList<sHUp*> list;
	for (unsigned int i = 0; i < UnitsData.vehicle.Size(); ++i)
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
		n->UnitID=UnitsData.vehicle[i].data.ID;
		n->costs=UnitsData.vehicle[i].data.iBuilt_Costs;
		n->vehicle=true;
		MakeUpgradeSliderVehicle ( n->upgrades,(int)i,player );
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
		n->UnitID = UnitsData.building[i].data.ID;
		n->costs=UnitsData.building[i].data.iBuilt_Costs;
		n->vehicle=false;
		MakeUpgradeSliderBuilding ( n->upgrades,(int)i,player );
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

		bool const down = b > lb;
		bool const up   = b < lb;

		if (btn_done.CheckClick(x, y, down, up))
		{
			break;
		}
		// Beschreibung Haken:
		if ( x>=DIALOG_X +292&&x<DIALOG_X +292+16&&y>=DIALOG_Y +265&&y<DIALOG_Y +265+15&&b&&!lb )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Beschreibung=!Beschreibung;
			SettingsData.bShowDescription=Beschreibung;
			if ( Beschreibung )
			{
				scr.x=291;
				dest.x=scr.x + DIALOG_X;
				scr.y=264;
				dest.y=scr.y + DIALOG_Y;
				scr.w=17;
				scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			}
			else
			{
				scr.x=393;
				scr.y=46;
				dest.x=291+ DIALOG_X;
				dest.y=264+ DIALOG_Y;
				scr.w=18;
				scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			}
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Down-Button:
		if ( x>=DIALOG_X +491&&x<DIALOG_X +491+18&&y>=DIALOG_Y +386&&y<DIALOG_Y +386+17&&b&&!DownPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 491;
			dest.y=DIALOG_Y + 386;
			if ( offset<(int)selection.Size()-9 )
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
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 491;
			dest.y=DIALOG_Y + 386;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=false;
		}
		// Up-Button:
		if ( x>=DIALOG_X +470&&x<DIALOG_X +470+18&&y>=DIALOG_Y +386&&y<DIALOG_Y +386+17&&b&&!UpPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 470;
			dest.y=DIALOG_Y + 386;
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
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 470;
			dest.y=DIALOG_Y + 386;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=false;
		}
		// Klick in die Liste:
		if ( x>=DIALOG_X +490&&x<DIALOG_X +490+70&&y>=DIALOG_Y +60&&y<DIALOG_Y +60+315&&b&&!lb )
		{
			int nr;
			nr= ( y-60-DIALOG_Y ) / ( 32+2 );
			if ( selection.Size()<9 )
			{
				if ( nr>=(int)selection.Size() ) nr=-1;
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
				if (last_selected == nr && selection[selected]->costs <= player->Credits)
				{
					// Don't add buildings, humans, planes, etc...
					sHUp * const s = selection[selected];
					if (s->vehicle &&
						!s->UnitID.getUnitData()->is_human &&
							!s->UnitID.getUnitData()->is_alien &&
							s->UnitID.getUnitData()->can_drive != DRIVE_AIR &&
							s->UnitID.getUnitData()->can_drive != DRIVE_SEA)
					{
						sLanding n;
						n.cargo=0;
						n.sf = s->sf;
						n.UnitID = s->UnitID;
						n.costs = s->costs;
						LandingList->Add ( n );
						LandingSelected=(int)LandingList->Size()-1;
						while ( LandingSelected>=LandingOffset+5 ) LandingOffset++;

						if ( LandingSelected<0 ) LandingSelected=0;
						ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
						player->Credits -= s->costs;
						ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
						ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
					}
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Klick auf einen Upgrade-Slider:
		if ( b&&!lb&&x>=DIALOG_X +283&&x<DIALOG_X +301+18&&selection.Size() )
		{
			sHUp* ptr = selection[selected];
			for (int i=0;i<8;i++ )
			{
				if ( !ptr->upgrades[i].active ) 
					continue;
				if (ptr->upgrades[i].Purchased 
					&& x < DIALOG_X + 283 + 18 
					&& y >= DIALOG_Y + 293 + i*19 
					&& y < DIALOG_Y + 293 + i*19 + 19)
				{
					int upgradeType = -1;
					if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Hitpoints")) == 0)
						upgradeType = cUpgradeCalculator::kHitpoints;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Armor")) == 0)
						upgradeType = cUpgradeCalculator::kArmor;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Ammo")) == 0)
						upgradeType = cUpgradeCalculator::kAmmo;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Damage")) == 0)
						upgradeType = cUpgradeCalculator::kAttack;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Speed")) == 0)
						upgradeType = cUpgradeCalculator::kSpeed;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Shots")) == 0)
						upgradeType = cUpgradeCalculator::kShots;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Range")) == 0)
						upgradeType = cUpgradeCalculator::kRange;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Scan")) == 0)
						upgradeType = cUpgradeCalculator::kScan;

					cUpgradeCalculator& uc = cUpgradeCalculator::instance();
					if (upgradeType != cUpgradeCalculator::kSpeed)
					{
						*(ptr->upgrades[i].value) -= uc.calcIncreaseByUpgrade (ptr->upgrades[i].StartValue);
						ptr->upgrades[i].NextPrice = uc.calcPrice (*(ptr->upgrades[i].value), ptr->upgrades[i].StartValue, upgradeType);
					}
					else
					{
						*(ptr->upgrades[i].value) -= 4 * uc.calcIncreaseByUpgrade (ptr->upgrades[i].StartValue / 4);
						ptr->upgrades[i].NextPrice = uc.calcPrice (*(ptr->upgrades[i].value) / 4, ptr->upgrades[i].StartValue / 4, upgradeType);
					}

					player->Credits += ptr->upgrades[i].NextPrice;
					ptr->upgrades[i].Purchased--;

					PlayFX ( SoundData.SNDObjectMenu );
					ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
					ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					break;
				}
				else if (ptr->upgrades[i].NextPrice <= player->Credits
						 && x >= DIALOG_X + 301 
						 && y >= DIALOG_Y + 293 + i*19 
						 && y < DIALOG_Y + 293 + i*19 + 19)
				{
					player->Credits -= ptr->upgrades[i].NextPrice;

					int upgradeType = -1;
					if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Hitpoints")) == 0)
						upgradeType = cUpgradeCalculator::kHitpoints;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Armor")) == 0)
						upgradeType = cUpgradeCalculator::kArmor;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Ammo")) == 0)
						upgradeType = cUpgradeCalculator::kAmmo;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Damage")) == 0)
						upgradeType = cUpgradeCalculator::kAttack;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Speed")) == 0)
						upgradeType = cUpgradeCalculator::kSpeed;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Shots")) == 0)
						upgradeType = cUpgradeCalculator::kShots;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Range")) == 0)
						upgradeType = cUpgradeCalculator::kRange;
					else if (ptr->upgrades[i].name.compare (lngPack.i18n ("Text~Vehicles~Scan")) == 0)
						upgradeType = cUpgradeCalculator::kScan;

					cUpgradeCalculator& uc = cUpgradeCalculator::instance();
					if (upgradeType != cUpgradeCalculator::kSpeed)
					{
						*(ptr->upgrades[i].value) += uc.calcIncreaseByUpgrade (ptr->upgrades[i].StartValue);
						ptr->upgrades[i].NextPrice = uc.calcPrice (*(ptr->upgrades[i].value), ptr->upgrades[i].StartValue, upgradeType);
					}
					else
					{
						*(ptr->upgrades[i].value) += 4 * uc.calcIncreaseByUpgrade (ptr->upgrades[i].StartValue / 4);
						ptr->upgrades[i].NextPrice = uc.calcPrice (*(ptr->upgrades[i].value) / 4, ptr->upgrades[i].StartValue / 4, upgradeType);
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
		if ( b&&!lb&&x>=DIALOG_X +467&&x<DIALOG_X +467+32&&y>=DIALOG_Y +411&&y<DIALOG_Y +411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			tank=!tank;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=DIALOG_X +467+33&&x<DIALOG_X +467+32+33&&y>=DIALOG_Y +411&&y<DIALOG_Y +411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			plane=!plane;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=DIALOG_X +467+33*2&&x<DIALOG_X +467+32+33*2&&y>=DIALOG_Y +411&&y<DIALOG_Y +411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			ship=!ship;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=DIALOG_X +467+33*3&&x<DIALOG_X +467+32+33*3&&y>=DIALOG_Y +411&&y<DIALOG_Y +411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			build=!build;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=DIALOG_X +467+33*4&&x<DIALOG_X +467+32+33*4&&y>=DIALOG_Y +411&&y<DIALOG_Y +411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			tnt=!tnt;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=DIALOG_X +542&&x<DIALOG_X +542+16&&y>=DIALOG_Y +459 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			kauf=false;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!lb&&x>=DIALOG_X +542&&x<DIALOG_X +542+16&&y>=DIALOG_Y +445 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			kauf=true;
			CreateSelectionList ( selection,list,&selected,&offset,tank,plane,ship,build,tnt,kauf );      ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			MakeUpgradeSubButtons ( tank,plane,ship,build,tnt,kauf, sfTmp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		if (btn_buy.CheckClick(x, y, down, up) && selection[selected]->costs <= player->Credits && kauf)
		{
			sLanding n;
			n.cargo=0;
			n.sf = selection[selected]->sf;
			n.UnitID = selection[selected]->UnitID;
			n.costs = selection[selected]->costs;
			LandingList->Add ( n );
			LandingSelected=(int)LandingList->Size()-1;
			while ( LandingSelected>=LandingOffset+5 )
			{
				LandingOffset++;
			}
			ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			player->Credits -= n.costs;
			ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Down2-Button:
		if ( x>=DIALOG_X +327&&x<DIALOG_X +327+18&&y>=DIALOG_Y +240&&y<DIALOG_Y +240+17&&b&&!Down2Pressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 327;
			dest.y=DIALOG_Y + 240;
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
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 327;
			dest.y=DIALOG_Y + 240;
			SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Down2Pressed=false;
		}
		// Up2-Button:
		if ( x>=DIALOG_X +347&&x<DIALOG_X +347+18&&y>=DIALOG_Y +240&&y<DIALOG_Y +240+17&&b&&!Up2Pressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 347;
			dest.y=DIALOG_Y + 240;
			if ( LandingOffset<(int)LandingList->Size()-5 )
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
			scr.w=18;
			scr.h=17;
			dest.x=DIALOG_X + 347;
			dest.y=DIALOG_Y + 240;
			SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Up2Pressed=false;
		}

		if (btn_delete.CheckClick(x, y, down, up) && LandingList->Size() && (int)LandingList->Size() > LandingSelected&& LandingSelected >= 0)
		{
			// Vehicle aus der Liste entfernen:
			const sLanding& sel = (*LandingList)[LandingSelected];
			player->Credits += sel.costs;
			player->Credits += sel.cargo / 5;

			LandingList->Delete ( LandingSelected );
			ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp  );
			ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

			if ( LandingSelected>=(int)LandingList->Size() )
			{
				LandingSelected--;
			}
			if ( LandingList->Size()-LandingOffset<5&&LandingOffset>0 )
			{
				LandingOffset--;
			}
			if ( LandingSelected<0 ) LandingSelected=0;
			ShowLandingList ( LandingList,LandingSelected,LandingOffset, sfTmp  );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Klick in die LandingListe:
		if ( x>=DIALOG_X +330&&x<DIALOG_X +330+128&&y>=DIALOG_Y +22&&y<DIALOG_Y +22+210&&b&&!lb )
		{
			int nr;
			nr= ( y-22-DIALOG_Y ) / ( 32+10 );
			if ( LandingList->Size()<5 )
			{
				if ( nr>=(int)LandingList->Size() ) nr=-1;
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
					if ( LandingList->Size()&&(int)LandingList->Size()>LandingSelected&&LandingSelected>=0 )
					{
						const sLanding& sel = (*LandingList)[LandingSelected];
						player->Credits += sel.costs;
						player->Credits += sel.cargo / 5;
						LandingList->Delete ( LandingSelected );
						ShowBars ( player->Credits,StartCredits,LandingList,LandingSelected, sfTmp );
						ShowSelectionList ( selection,selected,offset,Beschreibung,player->Credits,player );

						if ( LandingSelected>=(int)LandingList->Size() )
						{
							LandingSelected--;
						}
						if ( LandingList->Size()-LandingOffset<5&&LandingOffset>0 )
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

		if ( LandingSelected>=0&&LandingList->Size()&&LandingSelected<(int)LandingList->Size() )
		{
			sLanding& ptr = (*LandingList)[LandingSelected];
			if ( ptr.UnitID.getUnitData()->can_transport==TRANS_METAL||ptr.UnitID.getUnitData()->can_transport==TRANS_OIL||ptr.UnitID.getUnitData()->can_transport==TRANS_GOLD )
			{
				//FIXME: this is not a good way since numbers of vehicles can change becouse of modifications in vehicles.xml
				// Prevent players from buying Gold cargo into a GoldTruck in the beginning of the game, as in org MAX (&&ptr->id!=32)

				// LadungUp-Button: 
				if ( x >= DIALOG_X + 413 && x < DIALOG_X + 413 + 18 && y >= DIALOG_Y + 424 && y < DIALOG_Y + 424 + 17 && b && !LadungDownPressed && ptr.cargo < ptr.UnitID.getUnitData()->max_cargo && player->Credits > 0 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					scr.x=249;
					scr.y=151;
					scr.w=18;
					scr.h=17;
					dest.x=DIALOG_X + 413;
					dest.y=DIALOG_Y + 424;

					ptr.cargo += 5;
					if ( ptr.cargo > ptr.UnitID.getUnitData()->max_cargo )
					{
						ptr.cargo = ptr.UnitID.getUnitData()->max_cargo;
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
					scr.w=18;
					scr.h=17;
					dest.x=DIALOG_X + 413;
					dest.y=DIALOG_Y + 424;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					LadungDownPressed=false;
				}
				//FIXME: this is not a good way since numbers of vehicles can change becouse of modifications in vehicles.xml
				// Prevent players from buying Gold cargo into a GoldTruck in the beginning of the game, as in org MAX (&&ptr->id!=32)

				// LadungDown-Button:
				if ( x >= DIALOG_X + 433 && x < DIALOG_X + 433 + 18 && y >= DIALOG_Y + 424 && y < DIALOG_Y + 424 + 17 && b && !LadungUpPressed && ptr.cargo > 0 )
				{
					PlayFX ( SoundData.SNDObjectMenu );
					scr.x=230;
					scr.y=151;
					scr.w=18;
					scr.h=17;
					dest.x=DIALOG_X + 433;
					dest.y=DIALOG_Y + 424;

					ptr.cargo -= 5;
					if ( ptr.cargo < 0 )
					{
						ptr.cargo = 0;
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
					scr.w=18;
					scr.h=17;
					dest.x=DIALOG_X + 433;
					dest.y=DIALOG_Y + 424;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&dest );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					LadungUpPressed=false;
				}//FIXME: this is not a good way since numbers of vehicles can change becouse of modifications in vehicles.xml
				// Prevent players from buying Gold cargo into a GoldTruck in the beginning of the game, as in org MAX (&&ptr->id!=32)

				// Klick auf den Ladungsbalken:
				if ( b&&!lb&&x>=DIALOG_X +422&&x<DIALOG_X +422+20&&y>=DIALOG_Y +301&&y<DIALOG_Y +301+115 )
				{
					int value;
					value= ( ( ( int ) ( ( 115- ( y-301-DIALOG_Y ) ) * ( ptr.UnitID.getUnitData()->max_cargo/115.0 ) ) ) /5 ) *5;
					PlayFX ( SoundData.SNDObjectMenu );

					if ( ( 115- ( y-301-DIALOG_Y ) ) >=110 ) value=ptr.UnitID.getUnitData()->max_cargo;

					if ( value < ptr.cargo )
					{
						player->Credits+= ( ptr.cargo-value ) /5;
						ptr.cargo=value;
					}
					else if ( value>ptr.cargo&&player->Credits>0 )
					{
						value-=ptr.cargo;
						while ( value>0&&player->Credits>0&&ptr.cargo<ptr.UnitID.getUnitData()->max_cargo )
						{
							ptr.cargo+=5;
							player->Credits--;
							value-=5;
						}
						if ( ptr.cargo > ptr.UnitID.getUnitData()->max_cargo )
						{
							ptr.cargo = ptr.UnitID.getUnitData()->max_cargo;
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

	while ( list.Size() )
	{
		sHUp *ptr;
		ptr = list[0];
		for (size_t i=0;i<8;i++ )
		{
			if ( ptr->upgrades[i].active&&ptr->upgrades[i].Purchased )
			{
				if ( ptr->vehicle )
				{
					ptr->UnitID.getUnitData( player )->version++;
				}
				else
				{
					ptr->UnitID.getUnitData( player )->version++;
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

// TODO ALERT: DUPLICATE CODE with cBuilding::MakeUpgradeSliderVehicle
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
		u[i].active = true;
		u[i].value = & ( d->damage );
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.damage, cUpgradeCalculator::kAttack);
		u[i].name = lngPack.i18n ("Text~Vehicles~Damage");
		i++;
		// Shots:
		u[i].active = true;
		u[i].value = & ( d->max_shots );
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.max_shots, cUpgradeCalculator::kShots);
		u[i].name = lngPack.i18n ("Text~Vehicles~Shots");
		i++;
		// Range:
		u[i].active = true;
		u[i].value = & ( d->range );
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.range, cUpgradeCalculator::kRange);
		u[i].name = lngPack.i18n ("Text~Vehicles~Range");
		i++;
		// Ammo:
		u[i].active = true;
		u[i].value = & ( d->max_ammo );
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.max_ammo, cUpgradeCalculator::kAmmo);
		u[i].name = lngPack.i18n ("Text~Vehicles~Ammo");
		i++;
	}
	if ( d->can_transport==TRANS_METAL||d->can_transport==TRANS_OIL||d->can_transport==TRANS_GOLD )
	{
		i++;
	}

	// Armor:
	u[i].active = true;
	u[i].value = & ( d->armor );
	u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.armor, cUpgradeCalculator::kArmor);
	u[i].name = lngPack.i18n ("Text~Vehicles~Armor");
	i++;
	// Hitpoints:
	u[i].active = true;
	u[i].value = & ( d->max_hit_points );
	u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.max_hit_points, cUpgradeCalculator::kHitpoints);
	u[i].name = lngPack.i18n ("Text~Vehicles~Hitpoints");
	i++;
	// Scan:
	u[i].active = true;
	u[i].value = & ( d->scan );
	u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.vehicle[nr].data.scan, cUpgradeCalculator::kScan);
	u[i].name = lngPack.i18n ("Text~Vehicles~Scan");
	i++;
	// Speed:
	u[i].active = true;
	u[i].value = & ( d->max_speed );
	u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value) / 4, UnitsData.vehicle[nr].data.max_speed / 4, cUpgradeCalculator::kSpeed);
	u[i].name = lngPack.i18n ("Text~Vehicles~Speed");
	i++;
	// Costs:
	i++;

	for ( i=0;i<8;i++ )
	{
		if ( u[i].value==NULL ) continue;
		u[i].StartValue=* ( u[i].value );
	}
}

// TODO ALERT: DUPLICATE CODE with cBuilding::MakeUpgradeSliderVehicle
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
		u[i].active = true;
		u[i].value = & ( d->damage );
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.damage, cUpgradeCalculator::kAttack);
		u[i].name = lngPack.i18n ("Text~Vehicles~Damage");
		i++;

		if ( !d->is_expl_mine )
		{
			// Shots:
			u[i].active = true;
			u[i].value = & ( d->max_shots );
			u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.max_shots, cUpgradeCalculator::kShots);
			u[i].name = lngPack.i18n ("Text~Vehicles~Shots");
			i++;
			// Range:
			u[i].active = true;
			u[i].value = & ( d->range );
			u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.range, cUpgradeCalculator::kRange);
			u[i].name = lngPack.i18n ("Text~Vehicles~Range");
			i++;
			// Ammo:
			u[i].active = true;
			u[i].value = & ( d->max_ammo );
			u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.max_ammo, cUpgradeCalculator::kAmmo);
			u[i].name = lngPack.i18n ("Text~Vehicles~Ammo");
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
	if (d->armor != 1)
	{
		u[i].active = true;
		u[i].value = &(d->armor);
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.armor, cUpgradeCalculator::kArmor);
		u[i].name = lngPack.i18n ("Text~Vehicles~Armor");
	}
	i++;
	// Hitpoints:
	u[i].active = true;
	u[i].value = & ( d->max_hit_points );
	u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.max_hit_points, cUpgradeCalculator::kHitpoints);
	u[i].name = lngPack.i18n ("Text~Vehicles~Hitpoints");
	i++;
	// Scan:
	if ( d->scan && d->scan != 1 )
	{
		u[i].active = true;
		u[i].value = & ( d->scan );
		u[i].NextPrice = cUpgradeCalculator::instance().calcPrice (*(u[i].value), UnitsData.building[nr].data.scan, cUpgradeCalculator::kScan);
		u[i].name = lngPack.i18n ("Text~Vehicles~Scan");
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

// Malt die SubButtons im Upgradefenster:
void MakeUpgradeSubButtons ( bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf, SDL_Surface *surface )
{
	SDL_Rect scr,dest, orig_dest;
	scr.x=152;scr.y=479;
	dest.x=orig_dest.x=467;
	dest.y=orig_dest.y=411;
	scr.w=orig_dest.w=32;
	scr.h=orig_dest.h=31;

	dest.x += DIALOG_X;
	dest.y += DIALOG_Y;

	// Tank:
	if ( !tank )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&orig_dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	orig_dest.x+=33;
	// Plane:
	if ( !plane )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&orig_dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	orig_dest.x+=33;
	// Ship:
	if ( !ship )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&orig_dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	orig_dest.x+=33;
	// Building:
	if ( !build )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&orig_dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	orig_dest.x+=33;
	// TNT:
	if ( !tnt )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&orig_dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	// Kauf:
	scr.x=54;scr.y=352;
	scr.w=scr.h=orig_dest.h=orig_dest.w=16;
	dest.x=orig_dest.x=542;
	dest.y=orig_dest.y=446;
	dest.x += DIALOG_X;
	dest.y += DIALOG_Y;
	
	if ( !kauf )
	{
		SDL_BlitSurface ( surface,&orig_dest,buffer,&dest );
		dest.y=DIALOG_Y + 462;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		dest.y=DIALOG_Y + 462;
		SDL_BlitSurface ( surface,&orig_dest,buffer,&dest );
	}
}

// Zeigt die Bars an:
void ShowBars ( int credits,int StartCredits,cList<sLanding> *landing,int selected, SDL_Surface *surface )
{
	SDL_Rect scr,dest;
	scr.x=dest.x=371;
	dest.x += DIALOG_X;
	scr.y=dest.y=301;
	dest.y += DIALOG_Y;
	scr.w=22;
	scr.h=115;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );
	scr.x=dest.x=312;
	dest.x +=DIALOG_X;
	scr.y=dest.y=265;
	dest.y +=DIALOG_Y;
	scr.w=150;
	scr.h=30;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );
	font->showTextCentered(DIALOG_X +381,DIALOG_Y + 275, lngPack.i18n ( "Text~Title~Gold" ));
	font->showTextCentered(DIALOG_X +381,DIALOG_Y +275+10, iToStr(credits));

	scr.x=118;
	scr.y=336;
	scr.w=16;
	scr.h= ( int ) ( 115 * ( credits / ( float ) StartCredits ) );
	dest.x=DIALOG_X +375;
	dest.y=DIALOG_Y +301+115-scr.h;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );

	scr.x=dest.x=422;
	dest.x += DIALOG_X;
	scr.y=dest.y=301;
	dest.y += DIALOG_Y;
	scr.w=20;
	scr.h=115;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );

	if ( selected>=0&&landing->Size()&&selected<(int)landing->Size() )
	{
		sLanding& ptr = (*landing)[selected];
		if ( ptr.UnitID.getUnitData()->can_transport==TRANS_METAL||ptr.UnitID.getUnitData()->can_transport==TRANS_OIL||ptr.UnitID.getUnitData()->can_transport==TRANS_GOLD )
		{
			font->showTextCentered(DIALOG_X +430,DIALOG_Y +275, lngPack.i18n ( "Text~Title~Cargo" ));
			font->showTextCentered(DIALOG_X +430,DIALOG_Y +275+10, iToStr(ptr.cargo));


			scr.x=133;
			scr.y=336;
			scr.w=20;
			scr.h= ( int ) ( 115 * ( ptr.cargo / ( float ) ptr.UnitID.getUnitData()->max_cargo ) );
			dest.x=DIALOG_X +422;
			dest.y=DIALOG_Y +301+115-scr.h;
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

cSelectLandingMenu::cSelectLandingMenu(cMap *map, sClientLandData* clientLandData, int iClients, int iLocalClient )
{
	this->map = map;
	this->clientLandData = clientLandData;
	this->iClients = iClients;
	this->iLocalClient = iLocalClient;

	iLandedClients = 0;
	bAllLanded = false;
	
}


void cSelectLandingMenu::run()
{
	int lx = 0, ly = 0,	lb = 0,	b  = 0;

	if ( !MultiPlayerMenu )	 //single player
	{
		selectLandingSite();
		clientLandData[0].iLandX = c.iLandX;
		clientLandData[0].iLandY = c.iLandY;
	}
	else
	{	
		if ( MultiPlayerMenu->bHost )  //menu is running on a network host
		{
			while ( !bAllLanded )
			{
				//get coords from the local client
				if ( c.landingState != LANDING_POSITION_OK )
				{
					selectLandingSite();
					Log.write("Server: received landing coords from Player " + iToStr( iLocalClient ), cLog::eLOG_TYPE_NET_DEBUG);
					iLandedClients++;
					clientLandData[iLocalClient].iLastLandX = clientLandData[iLocalClient].iLandX;
					clientLandData[iLocalClient].iLastLandY = clientLandData[iLocalClient].iLandY;
					clientLandData[iLocalClient].iLandX = c.iLandX;
					clientLandData[iLocalClient].iLandY = c.iLandY;
				}

				//wait for all other clients
				font->showTextCentered( 320, 235, lngPack.i18n ( "Text~Multiplayer~Waiting" ) ,FONT_LATIN_BIG );
				SHOW_SCREEN
				mouse->draw( false, screen );

				while ( iLandedClients < iClients )
				{
					EventHandler->HandleEvents();
					mouse->GetPos();

					if ( mouse->x != lx || mouse->y != ly )
					{
						mouse->draw ( true, screen );
					}

					handleMessages();

					lx = mouse->x;
					ly = mouse->y;
					lb = b;
					SDL_Delay ( 1 );
				}

				Log.write("Server: all clients have selected a position. Checking...", cLog::eLOG_TYPE_NET_DEBUG); 

				//now check the landing positions
				for ( int playerNr = 0; playerNr < iClients; playerNr++ )
				{
					eLandingState state = checkLandingState( playerNr );
					if ( state == LANDING_POSITION_WARNING || state == LANDING_POSITION_TOO_CLOSE )
					{
						iLandedClients--;
						if ( playerNr == iLocalClient )
						{
							c.landingState = state;
						}
						else
						{
							cNetMessage* message = new cNetMessage(MU_MSG_RESELECT_LANDING);
							message->pushChar( state );
							MultiPlayerMenu->sendMessage( message, playerNr );
						}
					}

					if ( state == LANDING_POSITION_TOO_CLOSE )
						Log.write("Server: Player " + iToStr(playerNr) + " has state LANDING_POSITION_TOO_CLOSE, Position: " + iToStr(clientLandData[playerNr].iLandX) + "," + iToStr(clientLandData[playerNr].iLandY), cLog::eLOG_TYPE_NET_DEBUG);
					else if ( state == LANDING_POSITION_WARNING )
						Log.write("Server: Player " + iToStr(playerNr) + " has state LANDING_POSITION_WARNING, Position: " + iToStr(clientLandData[playerNr].iLandX) + "," + iToStr(clientLandData[playerNr].iLandY), cLog::eLOG_TYPE_NET_DEBUG);
					else if ( state == LANDING_POSITION_OK )
						Log.write("Server: Player " + iToStr(playerNr) + " has state LANDING_POSITION_OK, Position: " + iToStr(clientLandData[playerNr].iLandX) + "," + iToStr(clientLandData[playerNr].iLandY), cLog::eLOG_TYPE_NET_DEBUG);
					else if ( state == LANDING_POSITION_CONFIRMED )
						Log.write("Server: Player " + iToStr(playerNr) + " has state LANDING_POSITION_COMFIRMED, Position: " + iToStr(clientLandData[playerNr].iLandX) + "," + iToStr(clientLandData[playerNr].iLandY), cLog::eLOG_TYPE_NET_DEBUG);
					else if ( state == LANDING_STATE_UNKNOWN )
						Log.write("Server: Player " + iToStr(playerNr) + " has state LANDING_STATE_UNKNOWN, Position: " + iToStr(clientLandData[playerNr].iLandX) + "," + iToStr(clientLandData[playerNr].iLandY), cLog::eLOG_TYPE_NET_DEBUG);
					else
						Log.write("Server: Player " + iToStr(playerNr) + " has an unknown landing state, Position: " + iToStr(clientLandData[playerNr].iLandX) + "," + iToStr(clientLandData[playerNr].iLandY), cLog::eLOG_TYPE_NET_DEBUG);

				}

				Log.write("Server: waiting for " + iToStr(iClients - iLandedClients) + " clients to select a position", cLog::eLOG_TYPE_NET_DEBUG);

				if ( iLandedClients >= iClients )
				{
					bAllLanded = true;
				}
			}				
		}
		else	//menu is running on a multiplayer client
		{
			while ( !bAllLanded )
			{
				if ( c.landingState != LANDING_POSITION_OK )
				{
					selectLandingSite();
					sendLandingCoords( c );
					font->showTextCentered( 320, 235, lngPack.i18n ( "Text~Multiplayer~Waiting" ), FONT_LATIN_BIG );
					SHOW_SCREEN
					mouse->draw( false, screen );
					c.landingState = LANDING_POSITION_OK;
				}
	
				EventHandler->HandleEvents();
				mouse->GetPos();

				if ( mouse->x != lx || mouse->y != ly )
				{
					mouse->draw ( true, screen );
				}

				handleMessages();

				lx = mouse->x;
				ly = mouse->y;
				lb = b;
				SDL_Delay ( 1 );
			}
		}
	}
}

void cSelectLandingMenu::sendLandingCoords( sClientLandData& c )
{
	Log.write("Client: sending landing coords", cLog::eLOG_TYPE_NET_DEBUG);
	cNetMessage* message = new cNetMessage( MU_MSG_LANDING_COORDS );
	message->pushInt16( c.iLandY );
	message->pushInt16( c.iLandX );
	message->pushChar( iLocalClient );
	MultiPlayerMenu->sendMessage( message );
}

eLandingState cSelectLandingMenu::checkLandingState(int playerNr )
{
	int posX = clientLandData[playerNr].iLandX;
	int posY = clientLandData[playerNr].iLandY;
	int lastPosX = clientLandData[playerNr].iLastLandX;
	int lastPosY = clientLandData[playerNr].iLastLandY;
	bool bPositionTooClose = false;
	bool bPositionWarning = false;

	//check distances to all other players
	for ( int i = 0; i < iClients; i++ )
	{
		const sClientLandData& c = clientLandData[i];
		if ( i == playerNr ) continue;

		int distance = (int) sqrt( pow( (float) c.iLandX - posX, 2) + pow( (float) c.iLandY - posY, 2) );

		if ( distance < LANDING_DISTANCE_TOO_CLOSE ) 
		{
			bPositionTooClose = true;
		}
		if ( distance < LANDING_DISTANCE_WARNING )
		{
			bPositionWarning = true;
		}
	}
	
	//now set the new landing state, 
	//depending on the last state, the last position, the current position, bPositionTooClose and bPositionWarning 
	eLandingState lastState = clientLandData[playerNr].landingState;
	eLandingState newState = LANDING_STATE_UNKNOWN;

	if ( bPositionTooClose )
	{
		newState = LANDING_POSITION_TOO_CLOSE;
	}
	else if ( bPositionWarning )
	{
		if ( lastState == LANDING_POSITION_WARNING )
		{
			int delta = (int) sqrt( pow( (float) posX - lastPosX, 2) + pow( (float) posY - lastPosY, 2) );
			if ( delta <= LANDING_DISTANCE_TOO_CLOSE )
			{
				//the player has choosen the same position after a warning
				//so further warnings will be ignored
				newState = LANDING_POSITION_CONFIRMED;
			}
			else
			{
				newState = LANDING_POSITION_WARNING;
			}
		}
		else if ( lastState == LANDING_POSITION_CONFIRMED )
		{
			//player is in state LANDING_POSITION_CONFIRMED, so ignore the warning
			newState = LANDING_POSITION_CONFIRMED;
		}
		else
		{
			newState = LANDING_POSITION_WARNING;
		}
	}
	else
	{
		if ( lastState == LANDING_POSITION_CONFIRMED )
		{
			newState = LANDING_POSITION_CONFIRMED;
		}
		else
		{
			newState = LANDING_POSITION_OK;
		}
	}

	clientLandData[playerNr].landingState = newState;
	return newState;
}

void cSelectLandingMenu::handleMessages()
{
	if (!MultiPlayerMenu) return;

	MultiPlayerMenu->HandleMessages();

	while ( MultiPlayerMenu->landingSelectionMessageList.Size() > 0 )
	{
		cNetMessage* message = MultiPlayerMenu->landingSelectionMessageList[0];
		
		switch ( message->iType )
		{
		case MU_MSG_LANDING_COORDS:
			{
				int playerNr = message->popChar();
				Log.write("Server: received landing coords from Player " + iToStr( playerNr ), cLog::eLOG_TYPE_NET_DEBUG);

				if ( playerNr >= iClients ) break;
			
				iLandedClients++;
				if ( iLandedClients > iClients )
				{
					Log.write("Server: received too much landing coords", cLog::eLOG_TYPE_NET_DEBUG);
				}

				sClientLandData& c = clientLandData[playerNr];
				//save last coords, so that a player can confirm his position after a warning about nearby players
				c.iLastLandX = c.iLandX;
				c.iLastLandY = c.iLandY;
				c.iLandX = message->popInt16();
				c.iLandY = message->popInt16();
			}
			break;
		case MU_MSG_RESELECT_LANDING:
			{
				Log.write("Client: received MU_MSG_RESELECT_LANDING", cLog::eLOG_TYPE_NET_DEBUG);
				c.landingState = (eLandingState) message->popChar();
			}
			break;
		case MU_MSG_ALL_LANDED:
			{
				bAllLanded = true;
			}
			break;
		}

		delete message;
		MultiPlayerMenu->landingSelectionMessageList.Delete(0);
	}

}

void cSelectLandingMenu::selectLandingSite()
{
	int b,lx=-1,ly=-1;
	sTerrain *t;
	SDL_Rect rTextArea;
	rTextArea.x = 220;
	rTextArea.y = 235;
	rTextArea.w = SettingsData.iScreenW - rTextArea.x - 20;
	float fakx= (float)( ( SettingsData.iScreenW-192.0 ) / map->size ); //pixel per field in x direction
	float faky= (float)( ( SettingsData.iScreenH-32.0 ) / map->size );  //pixel per field in y direction
	
	drawMap();

	if ( c.landingState != LANDING_STATE_UNKNOWN )
	{
		int posX = (int)(180 + c.iLandX * fakx);
		int posY = (int)(18  + c.iLandY * faky);
		//for non 4:3 screen resolutions, the size of the circles is
		//only correct in x dimension, because I don't draw an ellipse
		drawCircle( posX, posY, (int)((LANDING_DISTANCE_WARNING/2)*fakx), SCAN_COLOR, buffer );
		drawCircle( posX, posY, (int)((LANDING_DISTANCE_TOO_CLOSE/2)*fakx), RANGE_GROUND_COLOR, buffer );
		if( c.landingState == LANDING_POSITION_TOO_CLOSE)
		{
			font->showTextAsBlock( rTextArea, lngPack.i18n("Text~Comp~Landing_Too_Close"),FONT_LATIN_BIG );
		}
		else if ( c.landingState == LANDING_POSITION_WARNING)
		{
			font->showTextAsBlock( rTextArea, lngPack.i18n("Text~Comp~Landing_Warning"),FONT_LATIN_BIG );

		}
	}

	drawHud();
	SHOW_SCREEN
	mouse->SetCursor ( CNo );
	mouse->draw ( false,screen );

	while ( 1 )
	{
		//get events
		EventHandler->HandleEvents();
		if ( MultiPlayerMenu ) MultiPlayerMenu->HandleMessages();
		
		//handle mouse
		mouse->GetPos();
		b = mouse->GetMouseButton();
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
		}

		if ( b&&mouse->cur==GraphicsData.gfx_Cmove )
		{
			c.iLandX = (int) ( ( mouse->x-180 ) / ( 448.0/map->size ) * ( 448.0/ ( SettingsData.iScreenW-192 )));
			c.iLandY = (int) ( ( mouse->y-18  ) / ( 448.0/map->size ) * ( 448.0/ ( SettingsData.iScreenH-32 )));
			c.landingState = LANDING_POSITION_OK;

			drawMap();
			int posX = (int)(180 + c.iLandX * fakx);
			int posY = (int)(18  + c.iLandY * faky);
			//for non 4:3 screen resolutions, the size of the circles is
			//only correct in x dimension, because I don't draw an ellipse
			drawCircle( posX, posY, (int)((LANDING_DISTANCE_WARNING/2)*fakx), SCAN_COLOR, buffer );
			drawCircle( posX, posY, (int)((LANDING_DISTANCE_TOO_CLOSE/2)*fakx), RANGE_GROUND_COLOR, buffer );
			drawHud();
			SHOW_SCREEN
			mouse->SetCursor ( CHand );
			mouse->draw ( false,screen );
			return;
		}

		lx=mouse->x;
		ly=mouse->y;
		SDL_Delay ( 1 );
	}
}

void cSelectLandingMenu::drawMap()
{
	unsigned int nr,off;
	sTerrain *t;

	int fakx= ( int ) ( ( SettingsData.iScreenW-192.0 ) / map->size ); //pixel per field in x direction
	int faky= ( int ) ( ( SettingsData.iScreenH-32.0 ) / map->size );  //pixel per field in y direction

	// Die Karte malen:
	SDL_LockSurface ( buffer );
	for ( int i=0; i<SettingsData.iScreenW-192; i++ )
	{
		for ( int k=0; k<SettingsData.iScreenH-32; k++ )
		{
			nr=GetKachelBig ( ( i/fakx ) *fakx, ( k/faky ) *faky, map );
			t=map->terrain+nr;
			off= ( i%fakx ) * ( t->sf_org->h/fakx ) + ( k%faky ) * ( t->sf_org->h/faky ) *t->sf_org->w;
			nr= *( ( unsigned char* ) ( t->sf_org->pixels ) +off );

			unsigned char* pixel = (unsigned char*) &( ( Sint32* ) ( buffer->pixels ) ) [i+180+ ( k+18 ) *buffer->w];
			pixel[0] = map->palette[nr].b;
			pixel[1] = map->palette[nr].g;
			pixel[2] = map->palette[nr].r;
		}
	}
	SDL_UnlockSurface ( buffer );
}

void cSelectLandingMenu::drawHud()
{
	Client->Hud.DoAllHud();
	Client->Hud.EndeButton(false);
	Client->Hud.showTurnTime(NULL);
	
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	
	SDL_Rect top, bottom;
	top.x=0;
	top.y= ( SettingsData.iScreenH/2 )-479;

	bottom.x=0;
	bottom.y= ( SettingsData.iScreenH/2 );

	SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&top );
	SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&bottom );
}

// shows list of selected units for landing (box upper middle)
void ShowLandingList ( cList<sLanding> *list,int selected,int offset, SDL_Surface *surface )
{
	SDL_Rect scr = {375,32,80,font->getFontHeight(FONT_LATIN_SMALL_WHITE)};
	SDL_Rect dest,text = {DIALOG_X + 375,DIALOG_Y + 32,80,font->getFontHeight(FONT_LATIN_SMALL_WHITE)};
	scr.x=330;scr.y=11;
	scr.w=128;scr.h=222;
	dest.x = scr.x + DIALOG_X;
	dest.y = scr.y + DIALOG_Y;
	SDL_BlitSurface ( surface,&scr,buffer,&dest );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=DIALOG_X + 340;
	dest.y=DIALOG_Y + 20;
	dest.w=32;dest.h=32;
	for ( unsigned int i=offset;i<list->Size();i++ )
	{
		if ( (int)i>=offset+5 ) break;
		sLanding &ptr = (*list)[i];
		// Das Bild malen:
		SDL_BlitSurface ( ptr.sf,&scr,buffer,&dest );
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

		if ( font->getTextWide ( ptr.UnitID.getUnitData()->name, FONT_LATIN_SMALL_WHITE ) > text.w )
		{
			text.y -= font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2;
			font->showTextAsBlock ( text, ptr.UnitID.getUnitData()->name, FONT_LATIN_SMALL_WHITE);
			text.y += font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2;
		}
		else
		{
			font->showText ( text, ptr.UnitID.getUnitData()->name, FONT_LATIN_SMALL_WHITE);
		}



		if ( ptr.UnitID.getUnitData()->can_transport==TRANS_METAL||ptr.UnitID.getUnitData()->can_transport==TRANS_OIL||ptr.UnitID.getUnitData()->can_transport==TRANS_GOLD )
		{
			int value = ptr.cargo;
			int maxval = ptr.UnitID.getUnitData()->max_cargo;

			if(value == 0)
			{
				font->showText(text.x,text.y+10, "(empty)", FONT_LATIN_SMALL_WHITE);
			}
			else if(value <= maxval / 4)
			{
				font->showText(text.x,text.y+10, " ("+iToStr(value)+"/"+iToStr(maxval)+")", FONT_LATIN_SMALL_RED);
			}
			else if(value <= maxval / 2)
			{
				font->showText(text.x,text.y+10, " ("+iToStr(value)+"/"+iToStr(maxval)+")", FONT_LATIN_SMALL_YELLOW);
			}
			else
			{
				font->showText(text.x,text.y+10, " ("+iToStr(value)+"/"+iToStr(maxval)+")", FONT_LATIN_SMALL_GREEN);
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
	while ( selection.Size() )
	{
		selection.Delete ( 0 );
	}
	if ( kauf )
	{
		plane=false;
		ship=false;
		build=false;
	}
	for ( unsigned int i=0;i<images.Size();i++ )
	{
		sHUp* const s = images[i];
		if (s->vehicle)
		{
			if ( ! ( tank||ship||plane ) ) continue;
			vd = s->UnitID.getUnitData();
			if ( vd->is_alien&&kauf ) continue;
			if ( vd->is_human&&kauf ) continue;
			if ( tnt&&!vd->can_attack ) continue;
			if ( vd->can_drive==DRIVE_AIR&&!plane ) continue;
			if ( vd->can_drive==DRIVE_SEA&&!ship ) continue;
			if ( ( vd->can_drive==DRIVE_LAND||vd->can_drive==DRIVE_LANDnSEA ) &&!tank ) continue;
			selection.Add(s);
		}
		else
		{
			if ( !build ) continue;
			bd = s->UnitID.getUnitData();
			if ( tnt&&!bd->can_attack ) continue;
			selection.Add(s);
		}
	}
	if ( *offset>=(int)selection.Size()-9 )
	{
		*offset=(int)selection.Size()-9;
		if ( *offset<0 ) *offset=0;
	}
	if ( *selected>=(int)selection.Size() )
	{
		*selected=(int)selection.Size()-1;
		if ( *selected<0 ) *selected=0;
	}
}

// shows the units of the selection list (box left middle)
static void ShowSelectionList(cList<sHUp*>& list, int const selected, int const offset, bool const beschreibung, int const credits, cPlayer* const p)
{
	sHUp *ptr;
	SDL_Rect dest,text = {DIALOG_X + 530, DIALOG_Y +  70, 72, font->getFontHeight(FONT_LATIN_SMALL_WHITE)};
	SDL_Rect scr = {530, 70, 72, font->getFontHeight(FONT_LATIN_SMALL_WHITE)};
	scr.x=479;scr.y=52;
	scr.w=150;scr.h=330;
	dest.x = scr.x + DIALOG_X;
	dest.y = scr.y + DIALOG_Y;
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=DIALOG_X + 490;dest.y=DIALOG_Y + 58;
	if ( list.Size()==0 )
	{
		scr.x=0;scr.y=0;
		scr.w=316;
		scr.h=256;
		dest.x = scr.x + DIALOG_X;
		dest.y = scr.y + DIALOG_Y;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
		scr.x=11;scr.y=290;
		scr.w=346;
		scr.h=176;
		dest.x = scr.x + DIALOG_X;
		dest.y = scr.y + DIALOG_Y;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
		return;
	}
	for ( unsigned int i=offset;i<list.Size();i++ )
	{
		if ( (int)i>=offset+9 ) break;
		// Das Bild malen:
		ptr = list[i];
		SDL_BlitSurface ( ptr->sf,&scr,buffer,&dest );
		// Ggf noch Rahmen drum:
		if ( selected==i )
		{
			SDL_Rect tmp = dest;
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
			tmp.x=DIALOG_X +11;tmp.y=DIALOG_Y +13;
			if ( ptr->vehicle )
			{
				tmp.w=ptr->UnitID.getVehicle()->info->w;
				tmp.h=ptr->UnitID.getVehicle()->info->h;
				SDL_BlitSurface ( ptr->UnitID.getVehicle()->info,NULL,buffer,&tmp );
			}
			else
			{
				tmp.w=ptr->UnitID.getBuilding()->info->w;
				tmp.h=ptr->UnitID.getBuilding()->info->h;
				SDL_BlitSurface ( ptr->UnitID.getBuilding()->info,NULL,buffer,&tmp );
			}
			// Ggf die Beschreibung ausgeben:
			if ( beschreibung )
			{
				tmp.x+=10;tmp.y+=10;
				tmp.w-=20;tmp.h-=20;
				if ( ptr->vehicle )
				{
					font->showTextAsBlock(tmp, ptr->UnitID.getVehicle()->text);
				}
				else
				{
					font->showTextAsBlock(tmp, ptr->UnitID.getBuilding()->text);
				}
			}
			// Die Details anzeigen:
			{
				tmp.x=11;
				tmp.y=290;
				SDL_Rect temp2;
				temp2.x = tmp.x + DIALOG_X;
				temp2.y = tmp.y + DIALOG_Y;
				tmp.w = temp2.w = 346;
				tmp.h = temp2.h = 176;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&tmp,buffer,&temp2 );
				if ( ptr->vehicle )
				{
					cVehicle tv(ptr->UnitID.getVehicle(), p);
					tv.ShowBigDetails();
				}
				else
				{
					cBuilding tb(ptr->UnitID.getBuilding(), p, NULL);
					tb.ShowBigDetails();
				}
			}
			// Die Texte anzeigen/Slider machen:
			for ( int k=0;k<8;k++ )
			{
				SDL_Rect temp2;
				if ( !ptr->upgrades[k].active ) continue;
				font->showText(DIALOG_X +322,DIALOG_Y +296+k*19, iToStr(ptr->upgrades[k].NextPrice));
				SDL_Rect srcTemp;
				if ( ptr->upgrades[k].Purchased )
				{
					srcTemp.x = 380;
					srcTemp.y = 256;
					temp2.w = srcTemp.w = 18;
					temp2.h = srcTemp.h = 17;
					temp2.x = DIALOG_X +283;
					temp2.y = DIALOG_Y + 293 + k*19;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&srcTemp,buffer,&temp2 );
				}
				if ( ptr->upgrades[k].NextPrice<=credits )
				{
					srcTemp.x = 399;
					srcTemp.y = 256;
					temp2.w = srcTemp.w = 18;
					temp2.h = srcTemp.h = 17;
					temp2.x = DIALOG_X + 301;
					temp2.y = DIALOG_Y + 293 + k*19;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&srcTemp,buffer,&temp2 );
				}
			}
		}
		// Text ausgeben:
		string sTmp;

		if ( ptr->vehicle )
		{
			sTmp = ptr->UnitID.getUnitData()->name;
			font->showTextCentered(DIALOG_X +616, text.y, iToStr(ptr->UnitID.getUnitData()->iBuilt_Costs), FONT_LATIN_SMALL_YELLOW);
		}
		else
		{
			sTmp = ptr->UnitID.getUnitData()->name;
		}


		if ( font->getTextWide ( sTmp, FONT_LATIN_SMALL_WHITE ) > text.w )
		{
			text.y -= font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2;
			font->showTextAsBlock ( text, sTmp, FONT_LATIN_SMALL_WHITE);
			text.y += font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2;
		}
		else
		{
			font->showText ( text, sTmp, FONT_LATIN_SMALL_WHITE);
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
	ReadyList = (bool *) malloc ( sizeof( bool* ) );
	ReadyList[0] = false;
	bOptions = false;
	bStartSelecting = false;
	bExit = false;
	Savegame = NULL;
	menuPressedReturn = false;

	if ( bHost ) sIP = "-";
	else sIP = SettingsData.sIP;
	iPort = SettingsData.iPort;

	network = new cTCP;
}

cMultiPlayerMenu::~cMultiPlayerMenu()
{
	delete network;
	network = NULL;

	while ( PlayerList.Size() ) PlayerList.Delete ( PlayerList.Size()-1 );

	while ( ChatLog.Size() ) ChatLog.Delete ( ChatLog.Size()-1 );

	if ( Savegame )
	{
		if ( Server ) delete Server;
		Server = NULL;
		delete Savegame;
	}
	free ( ReadyList );
}

void cMultiPlayerMenu::addChatLog( string sMsg )
{
	// Delete old chat messages
	while( ChatLog.Size() > 7 )
	{
		ChatLog.Delete( 0 );
	}
	// Add the new message
	ChatLog.Add ( sMsg );
	bRefresh = true;
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
	for( unsigned int i = 0; i < ChatLog.Size(); i++ )
	{
		sMsg = ChatLog[ChatLog.Size() - 1 - i];
		while ( font->getTextWide( sMsg ) > 410 )
		{
			sMsg.erase( sMsg.length()-1 );
		}
		font->showText( 25, 387-12*i, sMsg, FONT_LATIN_NORMAL, buffer );
	}
	SHOW_SCREEN
	mouse->draw( false, screen );
}

void cMultiPlayerMenu::runNetworkMenu()
{
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
	InputHandler->setInputStr ( ActualPlayer->name );

	ChatStr = "";
	font->showText(20,245, lngPack.i18n ( "Text~Title~IP" ));
	font->showText(20,260, sIP);

	font->showText(228,245, lngPack.i18n ( "Text~Title~Port" ));
	font->showText(228,260, iToStr( iPort ) );

	font->showText(352, 245, lngPack.i18n ( "Text~Title~Player_Name" ));
	font->showText(352,260, ActualPlayer->name);

	font->showText(500,245, lngPack.i18n ( "Text~Title~Color" ));
	dest.x = 505; dest.y = 260; scr.w = 83; scr.h = 10; scr.x = 0; scr.y=  0;
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
		btn_load.Draw();
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

	while ( !bExit )
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
		if ( InputHandler->checkHasBeenInput() )
		{
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
					i_tmpRedrawLength += font->getTextWide( InputHandler->getInputStr( CURSOR_SHOW ) );
					InputHandler->cutToLength ( 176 );
					stmp = InputHandler->getInputStr();

					sIP = InputHandler->getInputStr( CURSOR_DISABLED );
					scr.x=20;scr.y=260;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					font->showText(20,260, stmp);
					break;
				case FOCUS_PORT:
					i_tmpRedrawLength += font->getTextWide(InputHandler->getInputStr( CURSOR_SHOW ));
					if ( atoi ( InputHandler->getInputStr( CURSOR_DISABLED ).c_str() ) > 65535 ) //ports over 65535 are impossible
					{
						iPort = 58600; //default Port 58600 - why is this our default Port? -- beko
						stmp = "58600";
					}
					else
					{
						stmp = InputHandler->getInputStr();
						iPort = atoi ( InputHandler->getInputStr( CURSOR_DISABLED ).c_str() );
					}
					scr.x=228;scr.y=260;
					scr.w=i_tmpRedrawLength;scr.h=16;
					SDL_BlitSurface ( sfTmp,&scr,buffer,&scr );
					font->showText(228,260, stmp);
					break;
				case FOCUS_NAME:
					i_tmpRedrawLength  += font->getTextWide(InputHandler->getInputStr( CURSOR_SHOW ));
					InputHandler->cutToLength ( 98 );
					stmp = InputHandler->getInputStr();

					if ( strcmp ( ActualPlayer->name.c_str(),InputHandler->getInputStr( CURSOR_DISABLED ).c_str() ) )
					{
						ActualPlayer->name=InputHandler->getInputStr( CURSOR_DISABLED );
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
					i_tmpRedrawLength += font->getTextWide( InputHandler->getInputStr() );
					InputHandler->cutToLength ( 410-font->getTextWide ( ( ActualPlayer->name+": " ) ) );
					stmp = InputHandler->getInputStr();

					ChatStr = InputHandler->getInputStr( CURSOR_DISABLED );
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
			SettingsData.iPort = iPort;
			SaveOption(SAVETYPE_NAME);
			SaveOption(SAVETYPE_PORT);
			if ( !bHost )
			{
				SettingsData.sIP = sIP;
				SaveOption(SAVETYPE_IP);
			}
			break;
		}
		if (bHost && btn_ok.CheckClick(x, y, down, up))
		{
			int iPlayerNum;
			if ((!bOptions || sMap.empty()) && !Savegame )
			{
				addChatLog(lngPack.i18n("Text~Multiplayer~Missing_Settings"));
			}
			else if ((iPlayerNum = testAllReady()) != -1)
			{
				addChatLog(PlayerList[iPlayerNum]->name + " " + lngPack.i18n("Text~Multiplayer~Not_Ready"));
			}
			else
			{
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
			dest.x = 505; dest.y = 260; scr.w = 83; scr.h = 10; scr.x = 0; scr.y = 0;
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
			dest.x = 505; dest.y = 260; scr.w = 83; scr.h = 10; scr.x = 0; scr.y = 0;
			SDL_BlitSurface ( ActualPlayer->color, &scr, buffer, &dest );
			displayPlayerList();
			sendIdentification();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Host buttons:
		if ( bHost )
		{
			if (btn_planet.CheckClick(x, y, down, up) && !Savegame)
			{
				sMap = RunPlanetSelect();
				SDL_FillRect(buffer, NULL, 0x0000);
				SDL_BlitSurface ( sfTmp, NULL, buffer, NULL );
				displayGameSettings();
				displayPlayerList();
				showChatLog();

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
				scr.w = 83;
				scr.h = 10;
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
			if (btn_options.CheckClick(x, y, down, up) && !Savegame)
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
				SDL_FillRect(buffer, NULL, 0x0000);
				SDL_BlitSurface ( sfTmp, NULL, buffer, NULL );
				showChatLog();
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
				scr.w = 83;
				scr.h = 10;
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
				if ( ShowDateiMenu ( false ) != -1 && !SaveLoadFile.empty() )
				{
					if ( Savegame ) delete Savegame;
					Savegame = new cSavegame ( SaveLoadNumber );
					Savegame->loadHeader ( &savegameString, NULL, NULL );
					savegameString += "\n\n" + lngPack.i18n ( "Text~Title~Players" ) +"\n" + Savegame->getPlayerNames();
					sMap = Savegame->getMapName();
					sendOptions ();
				}
				else
				{
					if ( Savegame ) delete Savegame;
					Savegame = NULL;
					savegameString.clear();
				}
				// reset InputStr
				switch ( iFocus )
				{
					case FOCUS_IP:
						InputHandler->setInputStr ( sIP );
						break;
					case FOCUS_PORT:
						InputHandler->setInputStr ( iToStr ( iPort ) );
						break;
					case FOCUS_NAME:
						InputHandler->setInputStr ( ActualPlayer->name );
						break;
					case FOCUS_CHAT:
						InputHandler->setInputStr ( ChatStr );
						break;
				}

				SDL_FillRect(buffer, NULL, 0x0000);
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
				scr.w = 83;
				scr.h = 10;
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
						Log.write("Error opening socket", cLog::eLOG_TYPE_WARNING);
					}
					else
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n("Text~Title~Port") + ": "  + iToStr(iPort) + ")");
						Log.write("Game open (Port: " + iToStr(iPort) + ")", cLog::eLOG_TYPE_INFO);
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
					Log.write(("Connecting to " + sIP + ":" + iToStr(iPort)), cLog::eLOG_TYPE_INFO);

					if (network->connect() == -1)
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Error_Connect") + sIP + ":" + iToStr(iPort));
						Log.write("Error on connecting " + sIP + ":" + iToStr(iPort), cLog::eLOG_TYPE_WARNING);
					}
					else
					{
						addChatLog(lngPack.i18n("Text~Multiplayer~Network_Connected"));
						Log.write("Connected", cLog::eLOG_TYPE_INFO);
					}
				}

				SHOW_SCREEN
				mouse->draw(false, screen);
			}
		}

		if ( ( menuPressedReturn && iFocus == FOCUS_CHAT) ||
				btn_send.CheckClick(x, y, down, up))
		{
			if ( !ChatStr.empty() )
			{
				PlayFX ( SoundData.SNDChat );
				if ( ChatStr.compare( "/ready" ) == 0 )
				{
					int iPlayerIndex;
					for ( iPlayerIndex = 0; iPlayerIndex < (int)PlayerList.Size(); iPlayerIndex++ )
					{
						if (PlayerList[iPlayerIndex] == ActualPlayer) break;
					}
					ReadyList[iPlayerIndex] = !ReadyList[iPlayerIndex];
					displayPlayerList();
					sendIdentification();
				}
				else
				{
					ChatStr.insert ( 0,": " );
					ChatStr.insert ( 0, ActualPlayer->name );

					if ( ChatStr.length() > PACKAGE_LENGTH-8 )
					{
						ChatStr.erase ( PACKAGE_LENGTH-8 );
					}
					cNetMessage *Message = new cNetMessage ( MU_MSG_CHAT );
					Message->pushString ( ChatStr );
					sendMessage ( Message );

					if ( network->isHost() ) addChatLog ( ChatStr );
				}
				ChatStr="";
				if ( iFocus==FOCUS_CHAT ) InputHandler->setInputStr ( "" );
				scr.x = 20;
				scr.y = 423;
				scr.w = 430;
				scr.h = 16;
				SDL_BlitSurface ( sfTmp, &scr, buffer, &scr );

				font->showText(20, 423, ChatStr);
				bRefresh = true;
			}

			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		menuPressedReturn = false;

		// Set new Focus
		if ( ( !bHost && b && !lb && mouse->x >= 20 && mouse->x < 20+188 && mouse->y >= 250 && mouse->y < 250+30 ) ||
				( b && !lb && mouse->x >= 228 && mouse->x < 228+108 && mouse->y >= 250 && mouse->y < 250+30 ) ||
				( b && !lb && mouse->x >= 352 && mouse->x < 352+108 && mouse->y >= 250 && mouse->y < 250+30 ) ||
				( b && !lb && mouse->x >= 20 && mouse->x < 20+425 && mouse->y >= 420 && mouse->y < 420+30 ) )
		{
			if ( !bHost && b && !lb && mouse->x >= 20 && mouse->x < 20+188 && mouse->y >= 250 && mouse->y < 250+30 )
			{
				iFocus = FOCUS_IP;
				InputHandler->setInputStr ( sIP );
			}
			else if( b && !lb && mouse->x >= 228 && mouse->x < 228+108 && mouse->y >= 250 && mouse->y < 250+30 )
			{
				iFocus = FOCUS_PORT;
				InputHandler->setInputStr ( iToStr( iPort ) );
			}
			else if( b && !lb && mouse->x >= 352 && mouse->x < 352+108 && mouse->y >= 250 && mouse->y < 250+30 )
			{
				iFocus = FOCUS_NAME;
				InputHandler->setInputStr ( ActualPlayer->name );
			}
			else if( b && !lb && mouse->x >= 20 && mouse->x < 20+425 && mouse->y >= 420 && mouse->y < 420+30 )
			{
				iFocus = FOCUS_CHAT;
				InputHandler->setInputStr ( ChatStr );
			}
			InputHandler->setInputState ( true );

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

		// click on point for ready?
		int iPlayerIndex;
		for ( iPlayerIndex = 0; iPlayerIndex < (int)PlayerList.Size(); iPlayerIndex++ )
		{
			if (PlayerList[iPlayerIndex] == ActualPlayer) break;
		}
		if ( b && !lb && mouse->x >= 611 && mouse->x < 611+10 && mouse->y >= 297 && mouse->y < 297+10+16*iPlayerIndex )
		{
			PlayFX ( SoundData.SNDMenuButton );

			//check, if the selected map is available
			if ( sMap.empty() || FileExists( (SettingsData.sMapsPath + PATH_DELIMITER + sMap).c_str() ) )
			{
				ReadyList[iPlayerIndex] = !ReadyList[iPlayerIndex];
				displayPlayerList();
				sendIdentification();
			}
		}
		// show data if necessary
		if ( bRefresh )
		{
			showChatLog();
			displayGameSettings();
			displayPlayerList();
			bRefresh = false;
			SHOW_SCREEN
			mouse->draw ( false, screen );
		}

		if ( bStartSelecting )
		{
			// Save changed name, port or ip to max.xml
			SettingsData.sPlayerName = ActualPlayer->name;
			SettingsData.iPort = iPort;
			SaveOption(SAVETYPE_NAME);
			SaveOption(SAVETYPE_PORT);
			if ( !bHost )
			{
				SettingsData.sIP = sIP;
				SaveOption(SAVETYPE_IP);
			}

			if( !savegameString.empty() )
			{
				if ( bHost )
				{
					if ( !runSavedGame() )
					{
						bStartSelecting = false;
						continue;
					}
				}
				else
				{
					Map = new cMap;
					Map->LoadMap ( sMap );

					Client = new cClient( Map, &PlayerList );
					Client->isInMenu = true;
					Client->initPlayer ( ActualPlayer );
					for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
					{
						PlayerList[i]->InitMaps( Map->size, Map );
					}

					Client->isInMenu = false;
					Client->run();

					delete Client;
					Client = NULL;
					delete Server;
					Server = NULL;
				}
			}
			else runNewGame();
			break;
		}

		HandleMessages();

		lx = mouse->x;
		ly = mouse->y;
		lb = b;
		SDL_Delay ( 1 );
	}
	SDL_FreeSurface(sfTmp);
}

void cMultiPlayerMenu::runNewGame ()
{
	if ( bHost )
	{
		cNetMessage *Message = new cNetMessage ( MU_MSG_GO );
		sendMessage ( Message );
	}
	Map = new cMap;
	cMap *ServerMap = new cMap;
	if ( !Map->LoadMap ( sMap ) ) 
	{
		delete Map;
		return;
	}

	if ( bHost )
	{
		Map->PlaceRessources ( Options.metal, Options.oil, Options.gold, Options.dichte );
		// copy map for server
		ServerMap->NewMap( Map->size, Map->iNumberOfTerrains );
		ServerMap->MapName = Map->MapName;
		memcpy ( ServerMap->Kacheln, Map->Kacheln, sizeof ( int )*Map->size*Map->size );
		memcpy ( ServerMap->Resources, Map->Resources, sizeof ( sResources )*Map->size*Map->size );
		for ( int i = 0; i < Map->iNumberOfTerrains; i++ )
		{
			ServerMap->terrain[i].blocked = Map->terrain[i].blocked;
			ServerMap->terrain[i].coast = Map->terrain[i].coast;
			ServerMap->terrain[i].water = Map->terrain[i].water;
		}
	}
	else memset ( Map->Resources, 0, Map->size*Map->size*sizeof( sResources ) );

	ActualPlayer->Credits = Options.credits;
	if ( bHost ) ActualPlayer->InitMaps ( ServerMap->size, ServerMap );
	else ActualPlayer->InitMaps ( Map->size, Map );
	cList<sLanding> landingList;
	RunHangar ( ActualPlayer, &landingList );

	if ( !bHost ) 
	{
		sendUpgrades();
		sendLandingVehicles( landingList );
	}

	// copy playerlist for client
	cList<cPlayer*> *ClientPlayerList = new cList<cPlayer*>;
	cPlayer *ActualPlayerClient;
	for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
	{
		ClientPlayerList->Add(new cPlayer( *PlayerList[i] ) );
		if ((*ClientPlayerList)[i]->Nr == ActualPlayer->Nr) ActualPlayerClient = (*ClientPlayerList)[i];
	}
	// init client and his player
	Client = new cClient(Map, ClientPlayerList);
	Client->isInMenu = true;
	Client->initPlayer ( ActualPlayerClient );
	for ( unsigned int i = 0; i < ClientPlayerList->Size(); i++ )
	{
		(*ClientPlayerList)[i]->InitMaps(Map->size, Map);
	}

	if ( bHost )
	{
		// init the players of playerlist
		for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
		{
			if ( PlayerList[i] != ActualPlayer ) PlayerList[i]->InitMaps(ServerMap->size, ServerMap);
		}

		// init server
		Server = new cServer(ServerMap, &PlayerList, GAME_TYPE_TCPIP, Options.PlayRounds);
	}

	if ( bHost )
	{
		clientLandingCoordsList = new sClientLandData[PlayerList.Size()];
		clientLandingVehicleList = new cList<sLanding>[PlayerList.Size()];

		cSelectLandingMenu landingMenu( Map, clientLandingCoordsList, (int) PlayerList.Size(), ActualPlayer->Nr );
		landingMenu.run();

		// finished selecting landing positions
		// make all landings
		for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
		{
			cPlayer *Player = PlayerList[i];
			sClientLandData& c = clientLandingCoordsList[Player->Nr];
			cList<sLanding>& clientLandingList = clientLandingVehicleList[Player->Nr];

			if ( i == ActualPlayer->Nr )
				Server->makeLanding(c.iLandX, c.iLandY, Player, landingList, Options.FixedBridgeHead);
			else
				Server->makeLanding(c.iLandX, c.iLandY, Player, clientLandingList, Options.FixedBridgeHead);
		}

		delete[] clientLandingCoordsList;
		delete[] clientLandingVehicleList;

		// copy changed resources from server map to client map
		memcpy ( Map->Resources, ServerMap->Resources, sizeof ( sResources )*Map->size*Map->size );

		// send clients that all players have been landed
		cNetMessage *Message = new cNetMessage ( MU_MSG_ALL_LANDED );
		sendMessage ( Message );
	}
	else
	{
		cSelectLandingMenu landingMenu( Map, NULL, 1, ActualPlayer->Nr );
		landingMenu.run();
	}

	ExitMenu();

	if ( Options.PlayRounds && Client->ActivePlayer->Nr != 0 ) Client->bWaitForOthers = true;
	if ( bHost ) Server->bStarted = true;
	Client->isInMenu = false;
	Client->run();

	SettingsData.sPlayerName = ActualPlayerClient->name;
	
	delete Client;
	Client = NULL;
	if ( bHost )
	{
		delete Server;
		Server = NULL;
	}
	while ( PlayerList.Size() )
	{
		delete PlayerList[0];
		PlayerList.Delete ( 0 );
	}
	return;
}
int cMultiPlayerMenu::runSavedGame()
{
	if ( Savegame && Savegame->load() == 1 )
	{
		// set sockets
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			if ( (*Server->PlayerList)[i]->name == ActualPlayer->name ) (*Server->PlayerList)[i]->iSocketNum = MAX_CLIENTS;
			else
			{
				for ( unsigned int j = 0; j < PlayerList.Size(); j++ )
				{
					if ( (*Server->PlayerList)[i]->name == PlayerList[j]->name )
					{
						(*Server->PlayerList)[i]->iSocketNum = PlayerList[j]->iSocketNum;
						break;
					}
					// stop when a player is missing
					if ( j == PlayerList.Size()-1 )
					{
						addChatLog ( lngPack.i18n ( "Text~Multiplayer~Player_Wrong" ) );
						return 0;
					}
				}
			}
		}
		sendPlayerList ( Server->PlayerList );
		// send client that the game has to be started
		cNetMessage *Message = new cNetMessage ( MU_MSG_GO );
		sendMessage ( Message );

		// copy map for client
		cMap ClientMap;
		ClientMap.LoadMap ( Server->Map->MapName );

		cList<cPlayer*> ClientPlayerList;

		// copy players for client
		cPlayer *LocalPlayer;
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			ClientPlayerList.Add( new cPlayer( *(*Server->PlayerList)[i] ) );
			if ( (*Server->PlayerList)[i]->iSocketNum == MAX_CLIENTS ) LocalPlayer = ClientPlayerList[i];
			// reinit unit values
			for ( unsigned int j = 0; j < UnitsData.vehicle.Size(); j++) ClientPlayerList[i]->VehicleData[j] = UnitsData.vehicle[j].data;
			for ( unsigned int j = 0; j < UnitsData.building.Size(); j++) ClientPlayerList[i]->BuildingData[j] = UnitsData.building[j].data;
		}
		// init client and his player
		Client = new cClient( &ClientMap, &ClientPlayerList );
		Client->isInMenu = true;
		Client->initPlayer ( LocalPlayer );
		for ( unsigned int i = 0; i < ClientPlayerList.Size(); i++ )
		{
			ClientPlayerList[i]->InitMaps( ClientMap.size, &ClientMap );
		}

		// send data to all players
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			Server->resyncPlayer ( (*Server->PlayerList)[i] );
		}

		// exit menu and start game
		Server->bStarted = true;
		Client->isInMenu = false;
		Client->run();

		delete Client;
		Client = NULL;
		delete Server;
		Server = NULL;

		reloadUnitValues();
	}
	return 1;
}


void cMultiPlayerMenu::HandleMessages()
{
	while ( MessageList.Size()> 0 )
	{
		cNetMessage* Message = MessageList[0];
		Log.write("Menu: --> " + Message->getTypeAsString() + ", Hexdump: " + Message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

		switch ( Message->iType )
		{
		case MU_MSG_LANDING_COORDS:
		case MU_MSG_RESELECT_LANDING:
		case MU_MSG_ALL_LANDED:
			{
				//forward messages for the landing selection menu
				landingSelectionMessageList.Add( Message );
				MessageList.Delete(0);
				continue;
			}
			break;
		case MU_MSG_CHAT:
			{
				cNetMessage *SendMessage = new cNetMessage( MU_MSG_CHAT );
				string sChatString = Message->popString();
				SendMessage->pushString( sChatString );
				if ( network->isHost() ) sendMessage ( SendMessage, -1 );
				addChatLog( sChatString );
			}
			break;
		case MU_MSG_NEW_PLAYER:
			{
				cPlayer *Player = new cPlayer ( "unidentified", OtherData.colors[0], iNextPlayerNr, Message->popInt16() );
				PlayerList.Add ( Player );
				ReadyList = (bool *)realloc ( ReadyList, sizeof (bool*)*PlayerList.Size() );
				ReadyList[PlayerList.Size()-1] = false;
				cNetMessage *SendMessage = new cNetMessage ( MU_MSG_REQ_IDENTIFIKATION );
				SendMessage->pushInt16( iNextPlayerNr );
				iNextPlayerNr++;
				sendMessage ( SendMessage, Player->iSocketNum );
			}
			break;
		case MU_MSG_DEL_PLAYER:
			{
				int iClientNum = Message->popInt16();
				for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
				{
					if (PlayerList[i]->iSocketNum == iClientNum)
					{
						PlayerList.Delete ( i );
						for (unsigned int j = i; j < PlayerList.Size(); j++ ) ReadyList[j] = ReadyList[j+1];
						ReadyList = (bool *)realloc ( ReadyList, sizeof (bool*)*PlayerList.Size() );
					}
				}
				displayPlayerList();
				sendPlayerList();
			}
			break;
		case MU_MSG_REQ_IDENTIFIKATION:
			ActualPlayer->Nr = Message->popInt16();
			sendIdentification();
			break;
		case MU_MSG_IDENTIFIKATION:
			{
				unsigned int iPlayerNum;
				int iPlayerNr = Message->popInt16();
				for ( iPlayerNum = 0; iPlayerNum < PlayerList.Size(); iPlayerNum++ )
				{
					if (PlayerList[iPlayerNum]->Nr == iPlayerNr) break;
				}
				PlayerList[iPlayerNum]->color = OtherData.colors[Message->popInt16()];
				PlayerList[iPlayerNum]->name  = Message->popString();
				ReadyList[iPlayerNum] = Message->popBool();
				displayPlayerList();
				sendPlayerList();
				sendOptions();
			}
			break;
		case MU_MSG_PLAYERLIST:
			{
				string str = Message->getHexDump();
				int iPlayerCount = Message->popInt16();
				while ( PlayerList.Size() > 0 ) PlayerList.Delete ( 0 );
				ReadyList = (bool *)realloc( ReadyList, sizeof (bool*)*iPlayerCount );
				for ( int i = 0; i < iPlayerCount; i++ )
				{
					string sName = Message->popString();
					int iColor = Message->popInt16();
					int iNr = Message->popInt16();
					cPlayer *Player = new cPlayer ( sName, OtherData.colors[iColor], iNr );
					PlayerList.Add ( Player );
					// check if it's the own player by name if there has not been send the menu-plaer-list
					if ( !Message->popBool() )
					{
						if ( Player->name == ActualPlayer->name ) ActualPlayer = Player;
					}
					else
					{
						// else check by number and get the ready status
						if ( Player->Nr == ActualPlayer->Nr ) ActualPlayer = Player;
						ReadyList[i] = Message->popBool();
					}
				}
				displayPlayerList();
				break;
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
					savegameString = Message->popString();
				}
				bRefresh = true;
			}
			break;
		case MU_MSG_GO:
			bStartSelecting = true;
			break;
		case MU_MSG_LANDING_VEHICLES:
			{
				unsigned int playerNr = Message->popInt16();
				if ( playerNr >= PlayerList.Size() ) break;

				cList<sLanding>& landingList = clientLandingVehicleList[playerNr];

				int iCount = Message->popInt16();
				for ( int i = 0; i < iCount; i++ )
				{
					sLanding Landing;
					Landing.cargo = Message->popInt16();
					Landing.UnitID.iFirstPart = Message->popInt16();
					Landing.UnitID.iSecondPart = Message->popInt16();
					landingList.Add ( Landing );
				}
			}
			break;
		
		case MU_MSG_UPGRADES:
			{
				cPlayer *Player;
				int iPlayerNr = Message->popInt16();
				for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
				{
					if (PlayerList[i]->Nr == iPlayerNr)
					{
						Player = PlayerList[i];
						break;
					}
				}
				int iCount = Message->popInt16();
				for ( int i = 0; i < iCount; i++ )
				{
					bool bVehicle = Message->popBool();
					sID ID;
					ID.iFirstPart = Message->popInt16();
					ID.iSecondPart = Message->popInt16();

					ID.getUnitData ( Player )->damage = Message->popInt16();
					ID.getUnitData ( Player )->max_shots = Message->popInt16();
					ID.getUnitData ( Player )->range = Message->popInt16();
					ID.getUnitData ( Player )->max_ammo = Message->popInt16();
					ID.getUnitData ( Player )->armor = Message->popInt16();
					ID.getUnitData ( Player )->max_hit_points = Message->popInt16();
					ID.getUnitData ( Player )->scan = Message->popInt16();
					if ( bVehicle ) ID.getUnitData ( Player )->max_speed = Message->popInt16();
					ID.getUnitData ( Player )->version++;
				}
			}
			break;
		case GAME_EV_REQ_IDENT:
			{
				if ( ShowYesNo ( lngPack.i18n ( "Text~Multiplayer~Reconnect" ), false ) )
				{
					cNetMessage *NewMessage = new cNetMessage ( GAME_EV_IDENTIFICATION );
					NewMessage->pushInt16 ( Message->popInt16() );
					NewMessage->pushString ( ActualPlayer->name );
					sendMessage ( NewMessage );
				}
			}
			break;
		case GAME_EV_OK_RECONNECT:
			{
				ActualPlayer->Nr = Message->popInt16();
				ActualPlayer->color = OtherData.colors[Message->popInt16()];
				Map = new cMap;
				if ( !Map->LoadMap ( Message->popString() ) ) break;
				ActualPlayer->InitMaps ( Map->size, Map );
				int iPlayerCount = Message->popInt16();
				while ( iPlayerCount > 1 )
				{
					string playername = Message->popString();
					int playercolor = Message->popInt16();
					int playernr = Message->popInt16();
					PlayerList.Add ( new cPlayer ( playername, OtherData.colors[playercolor], playernr ) );
					PlayerList[PlayerList.Size()-1]->InitMaps ( Map->size, Map );
					iPlayerCount--;
				}

				Client = new cClient( Map, &PlayerList );
				Client->isInMenu = true;
				Client->initPlayer ( ActualPlayer );

				ExitMenu();

				cNetMessage *NewMessage = new cNetMessage ( GAME_EV_RECON_SUCESS );
				NewMessage->pushInt16 ( ActualPlayer->Nr );
				sendMessage ( NewMessage );

				Client->isInMenu = false;
				Client->run();

				delete Client; Client = NULL;

				bExit = true;
			}
		default:
			break;
		}

		delete Message;
		MessageList.Delete ( 0 );
	}
}

void cMultiPlayerMenu::sendLandingVehicles( const cList<sLanding>& landingList )
{
	cNetMessage *Message = new cNetMessage ( MU_MSG_LANDING_VEHICLES );

	for ( unsigned int i = 0; i < landingList.Size(); i++ )
	{
		Message->pushInt16( landingList[i].UnitID.iSecondPart);
		Message->pushInt16( landingList[i].UnitID.iFirstPart);
		Message->pushInt16( landingList[i].cargo);
	}
	Message->pushInt16( (int)landingList.Size() );
	Message->pushInt16( ActualPlayer->Nr );

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
			Message->pushInt16( ActualPlayer->VehicleData[i].ID.iSecondPart );
			Message->pushInt16( ActualPlayer->VehicleData[i].ID.iFirstPart );
			Message->pushBool( true ); // true for vehciles

			iCount++;
		}

		if ( Message->iLength+38 > PACKAGE_LENGTH )
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
			Message->pushInt16( ActualPlayer->BuildingData[i].ID.iSecondPart );
			Message->pushInt16( ActualPlayer->BuildingData[i].ID.iFirstPart );
			Message->pushBool( false ); // false for buildings

			iCount++;
		}

		if ( Message->iLength+34 > PACKAGE_LENGTH )
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
	for ( unsigned int i = 0; i < PlayerList.Size(); i++ )
	{
		if (!ReadyList[i]) return i;
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

	unsigned int iPlayerNum;
	for ( iPlayerNum = 0; iPlayerNum < PlayerList.Size(); iPlayerNum++ )
	{
		if (PlayerList[iPlayerNum] == ActualPlayer) break;
	}
	cNetMessage *Message = new cNetMessage ( MU_MSG_IDENTIFIKATION );
	Message->pushBool ( ReadyList[iPlayerNum] );
	Message->pushString ( ActualPlayer->name );
	Message->pushInt16( GetColorNr( ActualPlayer->color ) );
	Message->pushInt16( ActualPlayer->Nr );
	sendMessage ( Message );
}

void cMultiPlayerMenu::sendPlayerList( cList<cPlayer*> *SendPlayerList )
{
	if ( SendPlayerList == NULL ) SendPlayerList = &PlayerList;
	cNetMessage *Message = new cNetMessage ( MU_MSG_PLAYERLIST );

	for ( unsigned int i = 0; i < SendPlayerList->Size(); i++ )
	{
		// if the menulist is send, the players need the readystate, else not
		if ( SendPlayerList == &PlayerList )
		{
			Message->pushBool ( ReadyList[i] );
			Message->pushBool ( true );
		}
		else Message->pushBool ( false );
		cPlayer const* const p = (*SendPlayerList)[i];
		Message->pushInt16(p->Nr);
		Message->pushInt16(GetColorNr(p->color));
		Message->pushString(p->name);
	}
	Message->pushInt16 ( (int)SendPlayerList->Size() );
	sendMessage ( Message );
}

void cMultiPlayerMenu::sendOptions()
{
	cNetMessage *Message = new cNetMessage ( MU_MSG_OPTINS );

	if ( Savegame )
	{
		Message->pushString ( savegameString );
	}
	Message->pushBool ( Savegame != NULL );

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

	OptionString = lngPack.i18n ( "Text~Main~Version", PACKAGE_VERSION ) + "\n";
	OptionString += "Checksum: " + iToStr( SettingsData.Checksum ) + "\n\n";

	if( !bHost && !network )
	{
		OptionString += lngPack.i18n ( "Text~Multiplayer~Network_Connected_Not" );
		font->showTextAsBlock( rect, OptionString );
	}

	if( !savegameString.empty() )
	{
		OptionString += lngPack.i18n ( "Text~Title~Savegame" ) + ":\n  " + savegameString + "\n";
	}

	if ( !sMap.empty() )
	{
		string sMapName = sMap;
		sMapName.erase( sMapName.length() - 4 );
		if ( !FileExists( (SettingsData.sMapsPath + PATH_DELIMITER + sMap).c_str() ) )
		{
			OptionString += lngPack.i18n ( "Text~Error_Messages~ERROR_File_Not_Found", sMap );

			//blank map preview
			SDL_Rect rect;
			rect.w = rect.h = 112;
			rect.x = 33;
			rect.y = 106;
			SDL_BlitSurface ( sfTmp, &rect, buffer, &rect );

			//remove Player from readylist
			int iPlayerIndex;
			for ( iPlayerIndex = 0; iPlayerIndex < (int)PlayerList.Size(); iPlayerIndex++ )
			{
				if (PlayerList[iPlayerIndex] == ActualPlayer) break;
			}
			if ( ReadyList[iPlayerIndex] )
			{
				ReadyList[iPlayerIndex] = false;
				displayPlayerList();
				sendIdentification();
			}

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
				dest.x = 33;
				dest.y = 106;
				SDL_BlitSurface ( sfMapPic, NULL, buffer, &dest );
			}
			SDL_FreeSurface ( sfMapPic );
			font->showTextCentered( 90, 65, sMapName + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")" );
		}
	}
	else if ( savegameString.empty() )
	{
		OptionString += lngPack.i18n ( "Text~Multiplayer~Map_NoSet" ) + "\n";
	}
	OptionString += "\n";

	if ( savegameString.empty() )
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
	scr.w = scr.h = 10;
	dest.x = 476;
	dest.y = 297;
	for( unsigned int i = 0; i < PlayerList.Size(); i++ )
	{
		SDL_BlitSurface(PlayerList[i]->color, &scr, buffer, &dest);
		font->showText(dest.x + 16, dest.y, PlayerList[i]->name);

		if (!ReadyList[i]) scr.x = 0; // red if not ready
		else scr.x = 10; // green if ready

		dest.x += 135;
		SDL_BlitSurface( GraphicsData.gfx_player_ready, &scr, buffer, &dest );
		dest.x = 476;

		dest.y+=16;
	}
	bRefresh = true;
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

	cPlayer *Player;

	cMap Map;
	if ( !Map.LoadMap ( sMapName ) )
	{
		return;
	}
	Map.PlaceRessources ( Options.metal, Options.oil, Options.gold, Options.dichte );
	// copy map for server
	cMap ServerMap;
	ServerMap.NewMap( Map.size, Map.iNumberOfTerrains );
	ServerMap.MapName = Map.MapName;
	memcpy ( ServerMap.Kacheln, Map.Kacheln, sizeof ( int )*Map.size*Map.size );
	memcpy ( ServerMap.Resources, Map.Resources, sizeof ( sResources )*Map.size*Map.size );
	for ( int i = 0; i < Map.iNumberOfTerrains; i++ )
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
	Client->isInMenu = true;
	Client->initPlayer ( ClientPlayerList[0] );
	for ( unsigned int i = 0; i < ClientPlayerList.Size(); i++ )
	{
		ClientPlayerList[i]->InitMaps(Map.size, &Map);
		ClientPlayerList[i]->Credits = Options.credits;
		ClientPlayerList[i]->HotHud  = Client->Hud;
	}

	// init server
	Server = new cServer(&ServerMap, &ServerPlayerList, GAME_TYPE_HOTSEAT, false);

	// land the players
	for ( unsigned int i = 0; i < ServerPlayerList.Size(); i++ )
	{
		Player = ServerPlayerList[i];
		Player->InitMaps ( Map.size, &ServerMap );
		Player->Credits = Options.credits;

		ShowOK ( Player->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn" ), true );

		cList<sLanding> landingList;
		RunHangar(ClientPlayerList[i], &landingList);

		sClientLandData c;
		cSelectLandingMenu landingMenu( &Map, &c, 1 );
		landingMenu.run();

		Server->makeLanding ( c.iLandX, c.iLandY, Player, landingList, Options.FixedBridgeHead );
	}

	// exit menu and start game
	ExitMenu();

	Client->Hud = Player->HotHud;
	ShowOK ( Player->name + lngPack.i18n ( "Text~Multiplayer~Player_Turn" ), true );
	Server->bStarted = true;
	Client->isInMenu = false;
	Client->run();

	while ( ClientPlayerList.Size() )
	{
		delete ClientPlayerList[0];
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
	bool UpPressed=false, DownPressed=false;
	Uint8 *keystate;
	cList<string> *files;
	cList<sSaveFile*> savefiles;
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

	BigButton btn_back(rDialog.x + 353, rDialog.y + 438, "Text~Button~Back");
	BigButton btn_save(rDialog.x + 132, rDialog.y + 438, "Text~Button~Save");
	BigButton btn_exit(rDialog.x + 246, rDialog.y + 438, "Text~Button~Exit");
	BigButton btn_load(rDialog.x + 514, rDialog.y + 438, "Text~Button~Load");

	btn_back.Draw();
	if ( bSave )
	{
		btn_save.Draw();
		btn_exit.Draw();
	}
	else
	{
		btn_load.Draw();
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
	loadFiles ( files, savefiles, 0 );
	displayFiles ( savefiles, offset, selected, false, false, rDialog);
	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	int timer2 = 0;
	while ( 1 )
	{
		// Events holen:
		EventHandler->HandleEvents();

		// Tasten prüfen:
		if ( Client )
		{
			Client->handleTimer();
			timer2 = Client->iTimer2;
			Client->doGameActions();
		}
		keystate = SDL_GetKeyState( NULL );
		if ( keystate[SDLK_ESCAPE] ) break;

		if ( InputHandler->checkHasBeenInput() )
		{
			displayFiles ( savefiles, offset, selected, bSave, false, rDialog );
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
			if ( b )
			{
				InputHandler->setInputStr ( "" );
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
				displayFiles ( savefiles, offset, selected, bSave, true, rDialog );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				if ( bSave ) InputHandler->setInputState ( true );
			}
		}

		bool const down = b > LastB;
		bool const up   = b < LastB;

		if (btn_back.CheckClick(x, y, down, up))
		{
			delete files;
			InputHandler->setInputState ( false );
			return -1;
		}

		if (bSave && btn_exit.CheckClick(x, y, down, up))
		{
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Client->bExit = true;
			delete files;
			InputHandler->setInputState ( false );
			return -1;
		}

		if (bSave && btn_save.CheckClick(x, y, down, up))
		{
			// TODO: make sure the game is halted befor saving
			if ( selected != -1 )
			{
				displayFiles ( savefiles, offset, selected, true, false, rDialog );
				if ( !Server ) ShowOK ( lngPack.i18n ( "Text~Multiplayer~Save_Only_Host" ) );
				else
				{
					cSavegame Save( SaveLoadNumber );
					if ( Save.save( SaveLoadFile ) )
					{
						delete files;
						files = getFilesOfDirectory ( SettingsData.sSavesPath );
						for ( unsigned int i = 0; i < savefiles.Size(); i++ )
						{
							if ( savefiles[i]->number == SaveLoadNumber )
							{
								delete savefiles[i];
								savefiles.Delete ( i );
								break;
							}
						}
						loadFiles ( files, savefiles, offset );
						selected = -1;
					}
					displayFiles ( savefiles, offset, selected, true, false, rDialog );
				}
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
			InputHandler->setInputState ( false );
		}

		if (!bSave && btn_load.CheckClick(x, y, down, up))
		{
			InputHandler->setInputState ( false );
			if ( selected != -1 )
			{
				displayFiles ( savefiles, offset, selected, false, false, rDialog );
				delete files;
				return 1;
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
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
				displayFiles ( savefiles, offset, selected, false, false, rDialog );
				scr.x=96;
				SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowUp );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				UpPressed=false;
			}
			InputHandler->setInputState ( false );
		}
		else if ( UpPressed )
		{
			scr.x=96;
			SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowUp );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=false;
			InputHandler->setInputState ( false );
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
					loadFiles ( files, savefiles, offset );
					selected=-1;
				}
				displayFiles ( savefiles, offset, selected, false, false, rDialog );
				scr.x=96+28*2;
				SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowDown );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				DownPressed=false;
			}
			InputHandler->setInputState ( false );
		}
		else if ( DownPressed )
		{
			scr.x=96+28*2;
			SDL_BlitSurface ( GraphicsData.gfx_menu_buttons,&scr,buffer,&rArrowDown );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=false;
			InputHandler->setInputState ( false );
		}

		LastMouseX=x;LastMouseY=y;
		LastB=b;
		SDL_Delay ( 10 );
	}
	delete files;
	while ( savefiles.Size() > 0 )
	{
		delete savefiles[0];
		savefiles.Delete( 0 );
	}
	InputHandler->setInputState ( false );
	return -1;
}

void loadFiles ( cList<string> *filesList, cList<sSaveFile*> &savesList, int offset )
{
	for ( unsigned int i = 0; i < filesList->Size(); i++ )
	{
		// only check for xml files and numbers for this offset
		string const& file = (*filesList)[i];
		if ( file.substr( file.length() - 3, 3 ).compare( "xml" ) != 0 )
		{
			filesList->Delete ( i );
			i--;
			continue;
		}
		int number;
		if ( ( number = atoi( file.substr( file.length() - 7, 3 ).c_str() ) ) < offset || number > offset+10 ) continue;
		// don't add files twice
		bool found = false;
		for ( unsigned int j = 0; j < savesList.Size(); j++ )
		{
			if ( savesList[j]->number == number )
			{
				found = true;
				break;
			}
		}
		if ( found ) continue;
		// read the information and add it to the saveslist
		sSaveFile *savefile = new sSaveFile;
		savefile->number = number;
		savefile->filename = file;
		cSavegame Savegame ( number );
		Savegame.loadHeader ( &savefile->gamename, &savefile->type, &savefile->time );
		savesList.Add ( savefile );
	}
}

void displayFiles ( cList<sSaveFile*> &savesList, int offset, int selected, bool bSave, bool bFirstSelect, SDL_Rect rDialog )
{
	SDL_Rect rect, src;
	int i, x = rDialog.x + 35, y = rDialog.y + 72;
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
			font->showTextCentered(x, y, iToStr ( offset + i + 1 ), FONT_LATIN_BIG_GOLD);
		}
		else
		{
			font->showTextCentered(x, y, iToStr ( offset + i + 1 ), FONT_LATIN_BIG);
		}

		rect.y += 76;
		src.y += 76;
		y += 76;
	}

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
			bool found = false;
			for ( unsigned int j = 0; j < savesList.Size() || j == 0; j++ )
			{
				if ( savesList.Size() > 0 && savesList[j]->number == i )
				{
					string gamename = savesList[j]->gamename;
					// cut filename and display it
					if ( i == selected )
					{
						if ( bFirstSelect )
						{
							InputHandler->setInputStr ( gamename );
							InputHandler->cutToLength ( 145 );
						}
						else gamename = InputHandler->getInputStr();

						SaveLoadFile = InputHandler->getInputStr( CURSOR_DISABLED );
						SaveLoadNumber = i;
					}
					while ( font->getTextWide ( gamename ) > 145 )
					{
						gamename.erase ( gamename.length()-1 );
					}
					font->showText(x, y, gamename);
					// display time and gametype
					font->showText(x, y-23, savesList[j]->time);
					font->showText(x+113,y-23, savesList[j]->type);
					found = true;
					break;
				}
			}
			if ( !found && i == selected )
			{
				string gamename;
				InputHandler->cutToLength ( 145 );
				gamename = InputHandler->getInputStr();
				SaveLoadFile = InputHandler->getInputStr( CURSOR_DISABLED );
				SaveLoadNumber = i;
				font->showText(x, y, gamename);
			}
		}
		else
		{
			for ( unsigned int j = 0; j < savesList.Size(); j++ )
			{
				if ( savesList[j]->number == i )
				{
					string gamename = savesList[j]->gamename;
					// cut filename and display it
					if ( gamename.length() > 15 ) gamename.erase ( 15 );

					if ( i == selected ) SaveLoadFile = savesList[j]->filename;
					font->showText(x, y, savesList[j]->gamename);

					// display time and gametype
					font->showText(x, y - 23, savesList[j]->time);
					font->showText(x + 113, y - 23, savesList[j]->type);
					break;
				}
			}
			SaveLoadNumber = selected;
		}
		y += 76;
	}
}

void cMultiPlayerMenu::sendMessage( cNetMessage *Message, int iPlayer )
{
	Message->iPlayerNr = iPlayer;
	Log.write("Menu: <-- " + Message->getTypeAsString() + ", Hexdump: " + Message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	if ( iPlayer == -1 )
	{
		network->send ( Message->iLength, Message->serialize() );
	}
	else
	{
		network->sendTo ( iPlayer, Message->iLength, Message->serialize() );
	}
	delete Message;
}
