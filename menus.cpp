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

#include "menus.h"
#include "mouse.h"
#include "pcx.h"
#include "events.h"
#include "buildings.h"
#include "files.h"
#include "server.h"
#include "client.h"
#include "menuevents.h"
#include "loaddata.h"
#include "clientevents.h"
#include "clans.h"
#include "serverevents.h"
#include "dialog.h"

#define MAIN_MENU_BTN_SPACE 35

int GetColorNr ( SDL_Surface *sf )
{
	if ( sf == OtherData.colors[cl_red] )		return cl_red;
	if ( sf == OtherData.colors[cl_blue] )		return cl_blue;
	if ( sf == OtherData.colors[cl_green] )		return cl_green;
	if ( sf == OtherData.colors[cl_grey] )		return cl_grey;
	if ( sf == OtherData.colors[cl_orange] )	return cl_orange;
	if ( sf == OtherData.colors[cl_yellow] )	return cl_yellow;
	if ( sf == OtherData.colors[cl_purple] )	return cl_purple;
	if ( sf == OtherData.colors[cl_aqua] )		return cl_aqua;
	return cl_red;
}

string sSettings::getResValString ( eSettingResourceValue type )
{
	switch ( type )
	{
	case SETTING_RESVAL_LOW:
		return lngPack.i18n( "Text~Option~Low");
	case SETTING_RESVAL_NORMAL:
		return lngPack.i18n( "Text~Option~Normal");
	case SETTING_RESVAL_MUCH:
		return lngPack.i18n( "Text~Option~Much");
	case SETTING_RESVAL_MOST:
		return lngPack.i18n( "Text~Option~Most");
	}
	return "";
}

string sSettings::getResFreqString()
{
	switch ( resFrequency )
	{
	case SETTING_RESFREQ_THIN:
		return lngPack.i18n( "Text~Option~Thin");
	case SETTING_RESFREQ_NORMAL:
		return lngPack.i18n( "Text~Option~Normal");
	case SETTING_RESFREQ_THICK:
		return lngPack.i18n( "Text~Option~Thick");
	case SETTING_RESFREQ_MOST:
		return lngPack.i18n( "Text~Option~Most");
	}
	return "";
}

cGameDataContainer::~cGameDataContainer()
{
	if ( settings ) delete settings;
	if ( map ) delete map;
}

void cGameDataContainer::runGame( int player, bool reconnect )
{
	if ( savegameNum >= 0 )
	{
		runSavedGame ( player );
		return;
	}
	cMap serverMap;
	cList<cPlayer*> serverPlayers;
	if ( player == 0 )
	{
		// copy map for server
		serverMap.NewMap( map->size, map->iNumberOfTerrains );
		serverMap.MapName = map->MapName;
		memcpy ( serverMap.Kacheln, map->Kacheln, sizeof ( int )*map->size*map->size );
		serverMap.PlaceRessources ( settings->metal, settings->oil, settings->gold, settings->resFrequency );
		for ( int i = 0; i < map->iNumberOfTerrains; i++ )
		{
			serverMap.terrain[i].blocked = map->terrain[i].blocked;
			serverMap.terrain[i].coast = map->terrain[i].coast;
			serverMap.terrain[i].water = map->terrain[i].water;
		}

		// playerlist for server
		for ( unsigned int i = 0; i < players.Size(); i++ )
		{
			serverPlayers.Add ( new cPlayer ( (*players[i]) ) );

			serverPlayers[i]->InitMaps ( serverMap.size, &serverMap );
		}

		// init server
		Server = new cServer( &serverMap, &serverPlayers, type, false );
	}

	// init client and his players
	Client = new cClient( map, &players );
	Client->initPlayer ( players[player] );
	for ( unsigned int i = 0; i < players.Size(); i++ )
	{
		players[i]->InitMaps ( map->size, map );
	}

	if ( player == 0 )
	{
		if (settings->clans == SETTING_CLANS_ON)
		{
			cList<int> clans;
			for (unsigned int i =  0; i < players.Size (); i++)
				clans.Add ( players[i]->getClan () );
			
			sendClansToClients ( &clans );
		}
		for ( unsigned int i = 0; i < players.Size(); i++ )
		{
			Server->makeLanding( landData[i]->iLandX, landData[i]->iLandY, serverPlayers[i], landingUnits[i], settings->bridgeHead == SETTING_BRIDGEHEAD_DEFINITE );
			delete landData[i];
			delete landingUnits[i];
		}

		Server->bStarted = true;
	}

	if ( reconnect ) sendReconnectionSuccess ( player );
	Client->run();

	for ( unsigned int i = 0; i < players.Size(); i++ )
	{
		delete players[i];
	}

	delete Client;
	Client = NULL;

	if ( player == 0 )
	{
		delete Server;
		Server = NULL;
	}
}

void cGameDataContainer::runSavedGame( int player )
{
	cSavegame savegame ( savegameNum );
	if ( savegame.load() != 1 ) return;
	if ( player >= (int)Server->PlayerList->Size() ) return;

	// copy map for client
	cMap clientMap;
	clientMap.LoadMap ( Server->Map->MapName );

	cList<cPlayer*> clientPlayerList;

	// copy players for client
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		clientPlayerList.Add( new cPlayer( *(*Server->PlayerList)[i] ) );
		// reinit unit values
		for ( unsigned int j = 0; j < UnitsData.getNrVehicles (); j++) clientPlayerList[i]->VehicleData[j] = UnitsData.vehicle[j].data;
		for ( unsigned int j = 0; j < UnitsData.getNrBuildings (); j++) clientPlayerList[i]->BuildingData[j] = UnitsData.building[j].data;
	}
	// reinit unitvalues
	// init client and his player
	Client = new cClient( &clientMap, &clientPlayerList );
	Client->isInMenu = true;
	Client->initPlayer ( clientPlayerList[player] );
	for ( unsigned int i = 0; i < clientPlayerList.Size(); i++ )
	{
		clientPlayerList[i]->InitMaps( clientMap.size, &clientMap );
	}

	// in singleplayer only the first player is important
	(*Server->PlayerList)[player]->iSocketNum = MAX_CLIENTS;
	sendRequestResync( (*Server->PlayerList)[player]->Nr );

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

void cGameDataContainer::receiveClan ( cNetMessage *message )
{
	if ( message->iType != MU_MSG_CLAN ) 
		return;
	unsigned int playerNr = message->popInt16();
	int clanNr = message->popInt16 (); // -1 = no clan
	players[playerNr]->setClan (clanNr);
}

void cGameDataContainer::receiveLandingUnits ( cNetMessage *message )
{
	if ( message->iType != MU_MSG_LANDING_VEHICLES ) return;

	unsigned int playerNr = message->popInt16();

	for ( unsigned int i = (unsigned int)landingUnits.Size(); i < players.Size(); i++ )
	{
		landingUnits.Add ( NULL );
	}

	if ( landingUnits[playerNr] == NULL ) landingUnits[playerNr] = new cList<sLandingUnit>;
	cList<sLandingUnit> *playerLandingUnits = landingUnits[playerNr];

	int iCount = message->popInt16();
	for ( int i = 0; i < iCount; i++ )
	{
		sLandingUnit unit;
		unit.cargo = message->popInt16();
		unit.unitID.iFirstPart = message->popInt16();
		unit.unitID.iSecondPart = message->popInt16();
		playerLandingUnits->Add ( unit );
	}
}

void cGameDataContainer::receiveUnitUpgrades ( cNetMessage *message )
{
	int playerNr = message->popInt16();
	int count = message->popInt16();
	for ( int i = 0; i < count; i++ )
	{
		bool isVehicle = message->popBool();
		sID ID;
		ID.iFirstPart = message->popInt16();
		ID.iSecondPart = message->popInt16();

		ID.getUnitDataCurrentVersion ( players[playerNr] )->damage = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->shotsMax = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->range = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->ammoMax = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->armor = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->hitpointsMax = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->scan = message->popInt16();
		if ( isVehicle ) ID.getUnitDataCurrentVersion ( players[playerNr] )->speedMax = message->popInt16();
		ID.getUnitDataCurrentVersion ( players[playerNr] )->version++;
	}
}

void cGameDataContainer::receiveLandingPosition ( cNetMessage *message )
{
	if ( message->iType != MU_MSG_LANDING_COORDS ) return;

	int playerNr = message->popChar();
	Log.write("Server: received landing coords from Player " + iToStr( playerNr ), cLog::eLOG_TYPE_NET_DEBUG);

	for ( unsigned int i = (unsigned int)landData.Size(); i < players.Size(); i++ )
	{
		landData.Add ( NULL );
	}

	if ( landData[playerNr] == NULL ) landData[playerNr] = new sClientLandData;
	sClientLandData *c = landData[playerNr];
	//save last coords, so that a player can confirm his position after a warning about nearby players
	c->iLastLandX = c->iLandX;
	c->iLastLandY = c->iLandY;
	c->iLandX = message->popInt16();
	c->iLandY = message->popInt16();
	c->receivedOK = true;

	for ( int player = 0; player < (int)landData.Size(); player++ )
	{
		if ( landData[player] == NULL || !landData[player]->receivedOK ) return;
	}

	//now check the landing positions
	for ( int player = 0; player < (int)landData.Size(); player++ )
	{
		eLandingState state = checkLandingState( player );

		if ( state == LANDING_POSITION_TOO_CLOSE )
			Log.write("Server: Player " + iToStr(player) + " has state LANDING_POSITION_TOO_CLOSE, Position: " + iToStr(landData[player]->iLandX) + "," + iToStr(landData[player]->iLandY), cLog::eLOG_TYPE_NET_DEBUG);
		else if ( state == LANDING_POSITION_WARNING )
			Log.write("Server: Player " + iToStr(player) + " has state LANDING_POSITION_WARNING, Position: " + iToStr(landData[player]->iLandX) + "," + iToStr(landData[player]->iLandY), cLog::eLOG_TYPE_NET_DEBUG);
		else if ( state == LANDING_POSITION_OK )
			Log.write("Server: Player " + iToStr(player) + " has state LANDING_POSITION_OK, Position: " + iToStr(landData[player]->iLandX) + "," + iToStr(landData[player]->iLandY), cLog::eLOG_TYPE_NET_DEBUG);
		else if ( state == LANDING_POSITION_CONFIRMED )
			Log.write("Server: Player " + iToStr(player) + " has state LANDING_POSITION_COMFIRMED, Position: " + iToStr(landData[player]->iLandX) + "," + iToStr(landData[player]->iLandY), cLog::eLOG_TYPE_NET_DEBUG);
		else if ( state == LANDING_STATE_UNKNOWN )
			Log.write("Server: Player " + iToStr(player) + " has state LANDING_STATE_UNKNOWN, Position: " + iToStr(landData[player]->iLandX) + "," + iToStr(landData[player]->iLandY), cLog::eLOG_TYPE_NET_DEBUG);
		else
			Log.write("Server: Player " + iToStr(player) + " has an unknown landing state, Position: " + iToStr(landData[player]->iLandX) + "," + iToStr(landData[player]->iLandY), cLog::eLOG_TYPE_NET_DEBUG);

		if ( state == LANDING_POSITION_WARNING || state == LANDING_POSITION_TOO_CLOSE )
		{
			sMenuPlayer *menuPlayer = new sMenuPlayer ( players[player]->name, 0, false, players[player]->Nr, players[player]->iSocketNum ); 
			sendReselectLanding ( state, menuPlayer );
			delete menuPlayer;
		}
	}

	// now remove all players with warning
	bool ok = true;
	for ( int player = 0; player < (int)landData.Size(); player++ )
	{
		if ( landData[player]->landingState != LANDING_POSITION_OK && landData[player]->landingState != LANDING_POSITION_CONFIRMED )
		{
			landData[player]->receivedOK = false;
			ok = false;
		}
	}
	if ( !ok ) return;

	sendAllLanded();
}

eLandingState cGameDataContainer::checkLandingState( int playerNr )
{
	int posX = landData[playerNr]->iLandX;
	int posY = landData[playerNr]->iLandY;
	int lastPosX = landData[playerNr]->iLastLandX;
	int lastPosY = landData[playerNr]->iLastLandY;
	bool bPositionTooClose = false;
	bool bPositionWarning = false;

	//check distances to all other players
	for ( int i = 0; i < (int)players.Size(); i++ )
	{
		const sClientLandData *c = landData[i];
		if ( c == NULL ) continue;
		if ( i == playerNr ) continue;

		int distance = (int) sqrt( pow( (float) c->iLandX - posX, 2) + pow( (float) c->iLandY - posY, 2) );

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
	eLandingState lastState = landData[playerNr]->landingState;
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

	landData[playerNr]->landingState = newState;
	return newState;
}

cMenu::cMenu( SDL_Surface *background_, eMenuBackgrounds backgroundType_ ) : background (background_), backgroundType(backgroundType_)
{
	end = false;
	terminate = false;
	activeItem = NULL;

	if ( background )
	{
		position.w = background->w;
		position.h = background->h;
		position.x = (SettingsData.iScreenW / 2 - position.w / 2);
		position.y = (SettingsData.iScreenH / 2 - position.h / 2);
	}
	else
	{
		position.x = position.y = 0;
		position.w = SettingsData.iScreenW;
		position.h = SettingsData.iScreenH;
	}
}

cMenu::~cMenu()
{
	if ( background ) SDL_FreeSurface ( background );
}

void cMenu::draw( bool firstDraw )
{
	switch ( backgroundType )
	{
	case MNU_BG_BLACK:
		// fill the hole screen with black to prevent old garbage from menus
		// that don't support resolutions > 640x480
		SDL_FillRect( buffer, NULL, 0x000000 );
		break;
	case MNU_BG_ALPHA:
		if ( SettingsData.bAlphaEffects && firstDraw ) SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
		break;
	case MNU_BG_TRANSPARENT:
		// do nothing here
		break;
	}

	// draw the menu background
	if ( background ) SDL_BlitSurface ( background, NULL, buffer, &position );

	//show mouse
	mouse->Show();
	mouse->SetCursor ( CHand );

	// draw all menu items
	for ( unsigned int i = 0; i < menuItems.Size(); i++ )
	{
		menuItems[i]->draw();
	}

	SHOW_SCREEN

	mouse->draw ( false, screen );
}

int cMenu::show()
{
	draw( true );

	cMenu *lastActiveMenu = ActiveMenu;
	ActiveMenu = this;

	int lastMouseX = 0, lastMouseY = 0;

	while ( !end )
	{
		EventHandler->HandleEvents();

		mouse->GetPos();
		if ( mouse->moved() )
		{
			mouse->draw ( true, screen );

			for ( unsigned int i = 0; i < menuItems.Size(); i++ )
			{
				cMenuItem *menuItem = menuItems[i];
				if ( menuItem->overItem( lastMouseX, lastMouseY ) && !menuItem->overItem( mouse->x, mouse->y ) ) menuItem->hoveredAway( this );
				else if ( !menuItem->overItem( lastMouseX, lastMouseY ) && menuItem->overItem( mouse->x, mouse->y ) ) menuItem->hoveredOn( this );
				else if ( menuItem->overItem( lastMouseX, lastMouseY ) && menuItem->overItem( mouse->x, mouse->y ) ) menuItem->movedMouseOver( lastMouseX, lastMouseY, this );
			}
		}

		lastMouseX = mouse->x;
		lastMouseY = mouse->y;
		SDL_Delay ( 1 );

		if ( terminate )
		{
			ActiveMenu = lastActiveMenu;
			return 1;
		}
	}

	ActiveMenu = lastActiveMenu;
	return 0;
}

void cMenu::handleMouseInput( sMouseState mouseState )
{
	mouse->GetPos();
	mouse->isDoubleClick = mouseState.isDoubleClick;

	for ( unsigned int i = 0; i < menuItems.Size(); i++ )
	{
		cMenuItem *menuItem = menuItems[i];
		if ( menuItem->overItem( mouse->x, mouse->y ) && mouseState.leftButtonPressed )
		{
			menuItem->clicked( this );
			if ( activeItem ) activeItem->setActivity ( false );
			activeItem = menuItem;
			activeItem->setActivity ( true );
			draw();
		}
		if ( mouseState.leftButtonReleased )
		{
			if ( menuItem->overItem( mouse->x, mouse->y ) ) menuItem->released( this );
			else menuItem->somewhereReleased();
		}
	}
}

void cMenu::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	if ( activeItem && key.state == SDL_PRESSED ) activeItem->handleKeyInput ( key.keysym, ch, this );
}

void cMenu::sendMessage ( cNetMessage *message, sMenuPlayer *player, int fromPlayerNr )
{
	if ( !network ) return;
	
	// Attention: The playernumber will only be the real player number when it is passed to this function explicitly.
	//			Otherwise it is only -1!
	message->iPlayerNr = fromPlayerNr;

	if ( player == NULL ) network->send ( message->iLength, message->serialize() );
	else network->sendTo ( player->socket, message->iLength, message->serialize() );

	Log.write("Menu: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );
	delete message;
}

cMainMenu::cMainMenu(): cMenu ( LoadPCX ( GFXOD_MAIN ) )
{
	infoImage = new cMenuImage( position.x+16, position.y+182, getRandomInfoImage() );
	infoImage->setReleasedFunction ( &infoImageReleased );
	infoImage->setClickSound ( SoundData.SNDHudButton );
	menuItems.Add ( infoImage );

	creditsLabel = new cMenuLabel ( position.x+position.w/2, position.y+465, lngPack.i18n ( "Text~Main~Credits_Reloaded" )+ " "+PACKAGE_VERSION );
	creditsLabel->setCentered( true );
	menuItems.Add ( creditsLabel );
}

cMainMenu::~cMainMenu()
{
	delete infoImage;
	delete creditsLabel;
}

SDL_Surface *cMainMenu::getRandomInfoImage()
{
	int const showBuilding = random(3);
	// I want 3 possible random numbers
	// since a chance of 50:50 is boring (and
	// vehicles are way more cool so I prefer
	// them to be shown) -- beko 
	static int lastUnitShow = -1;
	int unitShow;
	SDL_Surface *surface = NULL;

	if ( showBuilding == 1 && UnitsData.getNrBuildings () > 0 ) //that's a 33% chance that we show a building on 1
	{
		do
		{
			unitShow = random((int)UnitsData.getNrBuildings ()-1);
		}
		while ( unitShow == lastUnitShow && UnitsData.getNrBuildings () > 1 );	//make sure we don't show same unit twice
		surface = UnitsData.building[unitShow].info;
	}
	else if ( UnitsData.getNrVehicles () > 0 ) //and a 66% chance to show a vehicle on 0 or 2
	{
		do
		{
			unitShow = random((int)UnitsData.getNrVehicles ()-1);
		}
		while ( unitShow == lastUnitShow && UnitsData.getNrVehicles () > 1 );	//make sure we don't show same unit twice
		surface = UnitsData.vehicle[unitShow].info;
	}
	else surface = NULL;
	lastUnitShow = unitShow; //store shown unit
	return surface;
}

void cMainMenu::infoImageReleased( void* parent )
{
	cMainMenu *menu = dynamic_cast<cMainMenu*>((cMenu*)parent);
	// get a new random info image
	SDL_Surface *surface = ((cMainMenu*)parent)->getRandomInfoImage();
	// draw the new image
	menu->infoImage->setImage ( surface );
	menu->infoImage->draw();
	SHOW_SCREEN
	mouse->draw ( false, screen );
}

cStartMenu::cStartMenu()
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+147, lngPack.i18n ("Text~Title~MainMenu") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	singleButton = new cMenuButton ( position.x+390, position.y+190, lngPack.i18n ("Text~Button~Single_Player") );
	singleButton->setReleasedFunction ( &singlePlayerReleased );
	menuItems.Add ( singleButton );

	multiButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE, lngPack.i18n ("Text~Button~Multi_Player") );
	multiButton->setReleasedFunction ( &multiPlayerReleased );
	menuItems.Add ( multiButton );

	preferenceButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE*2, lngPack.i18n ("Text~Settings~Preferences") );
	preferenceButton->setReleasedFunction ( &preferencesReleased );
	menuItems.Add ( preferenceButton );

	licenceButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE*3, lngPack.i18n ("Text~Button~Mani") );
	licenceButton->setReleasedFunction ( &licenceReleased );
	menuItems.Add ( licenceButton );

	exitButton = new cMenuButton ( position.x+415, position.y+190+MAIN_MENU_BTN_SPACE*6, lngPack.i18n ("Text~Button~Exit"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL, FONT_LATIN_BIG, SoundData.SNDMenuButton );
	exitButton->setReleasedFunction ( &exitReleased );
	menuItems.Add ( exitButton );

	PlayMusic((SettingsData.sMusicPath + PATH_DELIMITER + "main.ogg").c_str());
}

cStartMenu::~cStartMenu()
{
	delete titleLabel;
	delete singleButton;
	delete multiButton;
	delete preferenceButton;
	delete licenceButton;
	delete exitButton;
}

void cStartMenu::singlePlayerReleased( void* parent )
{
	cStartMenu *menu = static_cast<cStartMenu*>((cMenu*)parent);
	cSinglePlayerMenu singlePlayerMenu;
	singlePlayerMenu.show();
	menu->draw();
}

void cStartMenu::multiPlayerReleased( void* parent )
{
	cStartMenu *menu = static_cast<cStartMenu*>((cMenu*)parent);
	cMultiPlayersMenu multiPlayerMenu;
	multiPlayerMenu.show();
	menu->draw();
}

void cStartMenu::preferencesReleased( void* parent )
{
	cStartMenu *menu = static_cast<cStartMenu*>((cMenu*)parent);
	cDialogPreferences preferencesDialog;
	preferencesDialog.show();
	menu->draw();
}

void cStartMenu::licenceReleased( void* parent )
{
	cStartMenu *menu = static_cast<cStartMenu*>((cMenu*)parent);
	cDialogLicence licenceDialog;
	licenceDialog.show();
	menu->draw();
}

void cStartMenu::exitReleased( void* parent )
{
	cStartMenu *menu = static_cast<cStartMenu*>((cMenu*)parent);
	menu->end = true;
}

cSinglePlayerMenu::cSinglePlayerMenu()
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+147, lngPack.i18n ("Text~Button~Single_Player") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	newGameButton = new cMenuButton ( position.x+390, position.y+190, lngPack.i18n ("Text~Button~Game_New") );
	newGameButton->setReleasedFunction ( &newGameReleased );
	menuItems.Add ( newGameButton );

	loadGameButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE, lngPack.i18n ("Text~Button~Game_Load") );
	loadGameButton->setReleasedFunction ( &loadGameReleased );
	menuItems.Add ( loadGameButton );

	backButton = new cMenuButton ( position.x+415, position.y+190+MAIN_MENU_BTN_SPACE*6, lngPack.i18n ("Text~Button~Back"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	backButton->setReleasedFunction ( &backReleased );
	menuItems.Add ( backButton );
}

cSinglePlayerMenu::~cSinglePlayerMenu()
{
	delete titleLabel;
	delete newGameButton;
	delete loadGameButton;
	delete backButton;
}

void cSinglePlayerMenu::newGameReleased( void* parent )
{	
	cSinglePlayerMenu *menu = static_cast<cSinglePlayerMenu*>((cMenu*)parent);
	cGameDataContainer gameDataContainer;
	cSettingsMenu settingsMenu ( &gameDataContainer );
	settingsMenu.show();
	menu->draw();
}

void cSinglePlayerMenu::loadGameReleased( void* parent )
{
	cSinglePlayerMenu *menu = static_cast<cSinglePlayerMenu*>((cMenu*)parent);
	cGameDataContainer gameDataContainer;
	cLoadMenu loadMenu ( &gameDataContainer );
	loadMenu.show();
	if ( !gameDataContainer.savegame.empty() )
	{
		ActiveMenu = NULL;
		gameDataContainer.runGame( 0 );
		menu->end = true;
	}
	else menu->draw();
}

void cSinglePlayerMenu::backReleased( void* parent )
{
	cSinglePlayerMenu *menu = static_cast<cSinglePlayerMenu*>((cMenu*)parent);
	menu->end = true;
}

cMultiPlayersMenu::cMultiPlayersMenu()
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+147, lngPack.i18n ("Text~Button~Multi_Player") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	tcpHostButton = new cMenuButton ( position.x+390, position.y+190, lngPack.i18n ("Text~Button~TCPIP_Host") );
	tcpHostButton->setReleasedFunction ( &tcpHostReleased );
	menuItems.Add ( tcpHostButton );

	tcpClientButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE, lngPack.i18n ("Text~Button~TCPIP_Client") );
	tcpClientButton->setReleasedFunction ( &tcpClientReleased );
	menuItems.Add ( tcpClientButton );

	newHotseatButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE*2, lngPack.i18n ("Text~Button~HotSeat_New") );
	newHotseatButton->setReleasedFunction ( &newHotseatReleased );
	menuItems.Add ( newHotseatButton );

	loadHotseatButton = new cMenuButton ( position.x+390, position.y+190+MAIN_MENU_BTN_SPACE*3, lngPack.i18n ("Text~Button~HotSeat_Load") );
	loadHotseatButton->setReleasedFunction ( &loadHotseatReleased );
	menuItems.Add ( loadHotseatButton );

	backButton = new cMenuButton ( position.x+415, position.y+190+MAIN_MENU_BTN_SPACE*6, lngPack.i18n ("Text~Button~Back"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	backButton->setReleasedFunction ( &backReleased );
	menuItems.Add ( backButton );
}

cMultiPlayersMenu::~cMultiPlayersMenu()
{
	delete titleLabel;
	delete tcpHostButton;
	delete tcpClientButton;
	delete newHotseatButton;
	delete loadHotseatButton;
	delete backButton;
}

void cMultiPlayersMenu::tcpHostReleased( void* parent )
{
	cMultiPlayersMenu *menu = static_cast<cMultiPlayersMenu *>((cMenu*)parent);
	cNetworkHostMenu networkMenu;
	if ( networkMenu.show() == 1 )
	{
		menu->draw();
		return;
	}
	menu->end = true;
}

void cMultiPlayersMenu::tcpClientReleased( void* parent )
{
	cMultiPlayersMenu *menu = static_cast<cMultiPlayersMenu *>((cMenu*)parent);
	cNetworkClientMenu networkMenu;
	if ( networkMenu.show() == 1 )
	{
		menu->draw();
		return;
	}
	menu->end = true;
}

void cMultiPlayersMenu::newHotseatReleased( void* parent )
{
}

void cMultiPlayersMenu::loadHotseatReleased( void* parent )
{
}

void cMultiPlayersMenu::backReleased( void* parent )
{
	cMultiPlayersMenu *menu = static_cast<cMultiPlayersMenu*>((cMenu*)parent);
	menu->end = true;
}

cSettingsMenu::cSettingsMenu( cGameDataContainer *gameDataContainer_ ) : cMenu ( LoadPCX(GFXOD_OPTIONS) ), gameDataContainer(gameDataContainer_)
{
	if ( gameDataContainer->settings ) settings = (*gameDataContainer->settings);

	int iCurrentLine = 57;
	int iLineHeight = 16; //pixels after we start a new line
	//black window screen on gfx is 510 width. calculation for most option fields starts at px 240x. and is 347 width. 

	// Titel
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+13, lngPack.i18n ("Text~Button~Game_Options") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	// OK button
	okButton = new cMenuButton ( position.x+390, position.y+440, lngPack.i18n ("Text~Button~OK") );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );

	// Back button
	backButton = new cMenuButton ( position.x+50, position.y+440, lngPack.i18n ("Text~Button~Back") );
	backButton->setReleasedFunction ( &backReleased );
	menuItems.Add ( backButton );


	// Resources field
	metalLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Metal") +":" );
	menuItems.Add ( metalLabel );
	metalGroup = new cMenuRadioGroup();
	metalGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Low"), settings.metal == SETTING_RESVAL_LOW, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	metalGroup->addButton ( new cMenuCheckButton ( position.x+240+86, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Normal"), settings.metal == SETTING_RESVAL_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	metalGroup->addButton ( new cMenuCheckButton ( position.x+240+86*2, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Much"), settings.metal == SETTING_RESVAL_MUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	metalGroup->addButton ( new cMenuCheckButton ( position.x+240+86*3, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Most"), settings.metal == SETTING_RESVAL_MOST, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( metalGroup );
	iCurrentLine += iLineHeight;

	oilLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Oil") +":" );
	menuItems.Add ( oilLabel );
	oilGroup = new cMenuRadioGroup();
	oilGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Low"), settings.oil == SETTING_RESVAL_LOW, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	oilGroup->addButton ( new cMenuCheckButton ( position.x+240+86, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Normal"), settings.oil == SETTING_RESVAL_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	oilGroup->addButton ( new cMenuCheckButton ( position.x+240+86*2, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Much"), settings.oil == SETTING_RESVAL_MUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	oilGroup->addButton ( new cMenuCheckButton ( position.x+240+86*3, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Most"), settings.oil == SETTING_RESVAL_MOST, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( oilGroup );
	iCurrentLine += iLineHeight;

	goldLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Gold") +":" );
	menuItems.Add ( goldLabel );
	goldGroup = new cMenuRadioGroup();
	goldGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Low"), settings.gold == SETTING_RESVAL_LOW, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	goldGroup->addButton ( new cMenuCheckButton ( position.x+240+86, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Normal"), settings.gold == SETTING_RESVAL_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	goldGroup->addButton ( new cMenuCheckButton ( position.x+240+86*2, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Much"), settings.gold == SETTING_RESVAL_MUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	goldGroup->addButton ( new cMenuCheckButton ( position.x+240+86*3, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Most"), settings.gold == SETTING_RESVAL_MOST, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( goldGroup );
	iCurrentLine += iLineHeight;

	// Resource frequency field
	resFrequencyLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Resource_Density") +":" );
	menuItems.Add ( resFrequencyLabel );

	resFrequencyGroup = new cMenuRadioGroup();
	resFrequencyGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Thin"), settings.resFrequency == SETTING_RESFREQ_THIN, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	resFrequencyGroup->addButton ( new cMenuCheckButton ( position.x+240+86, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Normal"), settings.resFrequency == SETTING_RESFREQ_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	resFrequencyGroup->addButton ( new cMenuCheckButton ( position.x+240+86*2, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Thick"), settings.resFrequency == SETTING_RESFREQ_THICK, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	resFrequencyGroup->addButton ( new cMenuCheckButton ( position.x+240+86*3, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Most"), settings.resFrequency == SETTING_RESFREQ_MOST, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( resFrequencyGroup );
	iCurrentLine += iLineHeight*3;

	// Bridgehead field
	bridgeheadLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~BridgeHead") +":" );
	menuItems.Add ( bridgeheadLabel );

	bridgeheadGroup = new cMenuRadioGroup();
	bridgeheadGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Mobile"), settings.bridgeHead == SETTING_BRIDGEHEAD_MOBILE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	bridgeheadGroup->addButton ( new cMenuCheckButton ( position.x+240+173, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Definite"), settings.bridgeHead == SETTING_BRIDGEHEAD_DEFINITE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( bridgeheadGroup );
	iCurrentLine += iLineHeight;

	// Game type field
	gameTypeLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Game_Type") +":" );
	menuItems.Add ( gameTypeLabel );

	gameTypeGroup = new cMenuRadioGroup();
	gameTypeGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Type_Turns"), settings.gameType == SETTINGS_GAMETYPE_TURNS, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	gameTypeGroup->addButton ( new cMenuCheckButton ( position.x+240+173, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Type_Simu"), settings.gameType == SETTINGS_GAMETYPE_SIMU, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( gameTypeGroup );
	iCurrentLine += iLineHeight*3;

	// Other options (AlienTechs and Clans):
	/** //alien stuff disabled until we reimplement this proper -- beko Fri Jun 12 20:48:59 CEST 2009
	alienTechLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Alien_Tech") +":" );
	menuItems.Add ( alienTechLabel );
	aliensGroup = new cMenuRadioGroup();
	aliensGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~On"), settings.alienTech == SETTING_ALIENTECH_ON, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	aliensGroup->addButton ( new cMenuCheckButton ( position.x+240+64, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Off"), settings.alienTech == SETTING_ALIENTECH_OFF, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( aliensGroup );
	iCurrentLine += iLineHeight;
	*/

	clansLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n( "Text~Title~Clans") +":" );
	menuItems.Add (clansLabel );
	clansGroup = new cMenuRadioGroup();
	clansGroup->addButton ( new cMenuCheckButton ( position.x+240, position.y+iCurrentLine, lngPack.i18n( "Text~Option~On"), settings.clans == SETTING_CLANS_ON, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	clansGroup->addButton ( new cMenuCheckButton ( position.x+240+64, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Off"), settings.clans == SETTING_CLANS_OFF, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( clansGroup );
	iCurrentLine += iLineHeight*3;

	// Credits field - this is where the money goes
	creditsLabel = new cMenuLabel ( position.x+64, position.y+iCurrentLine, lngPack.i18n ("Text~Title~Credits") +":" );
	menuItems.Add ( creditsLabel );
	iCurrentLine += iLineHeight;
	creditsGroup = new cMenuRadioGroup();
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Lowest") + " ("+iToStr(SETTING_CREDITS_LOWEST)+")", settings.credits == SETTING_CREDITS_LOWEST, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	iCurrentLine += iLineHeight;
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Lower") + " ("+iToStr(SETTING_CREDITS_LOWER)+")", settings.credits == SETTING_CREDITS_LOWER, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	iCurrentLine += iLineHeight;
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Low") + " ("+iToStr(SETTING_CREDITS_LOW)+")", settings.credits == SETTING_CREDITS_LOW, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	iCurrentLine += iLineHeight;
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Normal") + " ("+iToStr(SETTING_CREDITS_NORMAL)+")", settings.credits == SETTING_CREDITS_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	iCurrentLine += iLineHeight;
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Much") + " ("+iToStr(SETTING_CREDITS_MUCH)+")", settings.credits == SETTING_CREDITS_MUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	iCurrentLine += iLineHeight;
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~More") + " ("+iToStr(SETTING_CREDITS_MORE)+")", settings.credits == SETTING_CREDITS_MORE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	iCurrentLine += iLineHeight;
	creditsGroup->addButton ( new cMenuCheckButton ( position.x+140, position.y+iCurrentLine, lngPack.i18n( "Text~Option~Most") + " ("+iToStr(SETTING_CREDITS_MOST)+")", settings.credits == SETTING_CREDITS_MOST, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY ) );
	menuItems.Add ( creditsGroup );
	iCurrentLine += iLineHeight;

}

cSettingsMenu::~cSettingsMenu()
{
	delete titleLabel;
	delete okButton;
	delete backButton;

	delete metalLabel;
	delete oilLabel;
	delete goldLabel;
	delete creditsLabel;
	delete bridgeheadLabel;
	//delete alienTechLabel;
	delete clansLabel;
	delete resFrequencyLabel;
	delete gameTypeLabel;

	delete metalGroup;
	delete oilGroup;
	delete goldGroup;
	delete creditsGroup;
	delete bridgeheadGroup;
	//delete aliensGroup;
	delete clansGroup;
	delete resFrequencyGroup;
	delete gameTypeGroup;
}

void cSettingsMenu::backReleased( void* parent )
{
	cSettingsMenu *menu = static_cast<cSettingsMenu *>((cMenu*)parent);
	menu->gameDataContainer->settings = NULL;
	menu->terminate = true;
}

void cSettingsMenu::okReleased( void* parent )
{
	cSettingsMenu *menu = static_cast<cSettingsMenu *>((cMenu*)parent);
	menu->updateSettings();
	if ( menu->gameDataContainer->settings ) delete menu->gameDataContainer->settings;
	menu->gameDataContainer->settings = new sSettings(menu->settings);

	switch ( menu->gameDataContainer->type )
	{
	case GAME_TYPE_SINGLE:
		{
			cPlanetsSelectionMenu planetSelectionMenu ( menu->gameDataContainer );
			if ( planetSelectionMenu.show() == 1 )
			{
				menu->draw();
				return;
			}
		}
		break;
	case GAME_TYPE_HOTSEAT:
		break;
	case GAME_TYPE_TCPIP:
		break;
	}
	menu->end = true;
}

void cSettingsMenu::updateSettings()
{
	if ( metalGroup->buttonIsChecked ( 0 ) ) settings.metal = SETTING_RESVAL_LOW;
	else if ( metalGroup->buttonIsChecked ( 1 ) ) settings.metal = SETTING_RESVAL_NORMAL;
	else if ( metalGroup->buttonIsChecked ( 2 ) ) settings.metal = SETTING_RESVAL_MUCH;
	else settings.metal = SETTING_RESVAL_MOST;

	if ( oilGroup->buttonIsChecked ( 0 ) ) settings.oil = SETTING_RESVAL_LOW;
	else if ( oilGroup->buttonIsChecked ( 1 ) ) settings.oil = SETTING_RESVAL_NORMAL;
	else if ( oilGroup->buttonIsChecked ( 2 ) ) settings.oil = SETTING_RESVAL_MUCH;
	else settings.oil = SETTING_RESVAL_MOST;

	if ( goldGroup->buttonIsChecked ( 0 ) ) settings.gold = SETTING_RESVAL_LOW;
	else if ( goldGroup->buttonIsChecked ( 1 ) ) settings.gold = SETTING_RESVAL_NORMAL;
	else if ( goldGroup->buttonIsChecked ( 2 ) ) settings.gold = SETTING_RESVAL_MUCH;
	else settings.gold = SETTING_RESVAL_MOST;

	if ( resFrequencyGroup->buttonIsChecked ( 0 ) ) settings.resFrequency = SETTING_RESFREQ_THIN;
	else if ( resFrequencyGroup->buttonIsChecked ( 1 ) ) settings.resFrequency = SETTING_RESFREQ_NORMAL;
	else if ( resFrequencyGroup->buttonIsChecked ( 2 ) ) settings.resFrequency = SETTING_RESFREQ_THICK;
	else settings.resFrequency = SETTING_RESFREQ_MOST;

	if ( creditsGroup->buttonIsChecked ( 0 ) ) settings.credits = SETTING_CREDITS_LOWEST;
	else if ( creditsGroup->buttonIsChecked ( 1 ) ) settings.credits = SETTING_CREDITS_LOWER;
	else if ( creditsGroup->buttonIsChecked ( 2 ) ) settings.credits = SETTING_CREDITS_LOW;
	else if ( creditsGroup->buttonIsChecked ( 3 ) ) settings.credits = SETTING_CREDITS_NORMAL;
	else if ( creditsGroup->buttonIsChecked ( 4 ) ) settings.credits = SETTING_CREDITS_MUCH;
	else if ( creditsGroup->buttonIsChecked ( 5 ) ) settings.credits = SETTING_CREDITS_MORE;
	else settings.credits = SETTING_CREDITS_MOST;

	if ( bridgeheadGroup->buttonIsChecked ( 0 ) ) settings.bridgeHead = SETTING_BRIDGEHEAD_MOBILE;
	else settings.bridgeHead = SETTING_BRIDGEHEAD_DEFINITE;

	/** //alien stuff disabled until we reimplement this proper -- beko Fri Jun 12 20:48:59 CEST 2009
	if ( aliensGroup->buttonIsChecked ( 0 ) ) settings.alienTech = SETTING_ALIENTECH_ON;
	else settings.alienTech = SETTING_ALIENTECH_OFF;
	*/
	settings.alienTech = SETTING_ALIENTECH_OFF;

	if ( clansGroup->buttonIsChecked ( 0 ) ) settings.clans = SETTING_CLANS_ON;
	else settings.clans = SETTING_CLANS_OFF;

	if ( gameTypeGroup->buttonIsChecked ( 0 ) ) settings.gameType = SETTINGS_GAMETYPE_TURNS;
	else settings.gameType = SETTINGS_GAMETYPE_SIMU;
	
}

cPlanetsSelectionMenu::cPlanetsSelectionMenu( cGameDataContainer *gameDataContainer_ ) : cMenu ( LoadPCX ( GFXOD_PLANET_SELECT ) ), gameDataContainer(gameDataContainer_)
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+11, lngPack.i18n ("Text~Title~Choose_Planet") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	okButton = new cMenuButton ( position.x+390, position.y+440, lngPack.i18n ("Text~Button~OK") );
	okButton->setReleasedFunction ( &okReleased );
	okButton->setLocked ( true );
	menuItems.Add ( okButton );

	backButton = new cMenuButton ( position.x+50, position.y+440, lngPack.i18n ("Text~Button~Back") );
	backButton->setReleasedFunction ( &backReleased );
	menuItems.Add ( backButton );

	arrowUpButton = new cMenuButton ( position.x+292, position.y+435, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG );
	arrowUpButton->setReleasedFunction ( &arrowUpReleased );
	menuItems.Add ( arrowUpButton );

	arrowDownButton = new cMenuButton ( position.x+321, position.y+435, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG );
	arrowDownButton->setReleasedFunction ( &arrowDownReleased );
	menuItems.Add ( arrowDownButton );

	int index = 0;
	for ( int y = 0; y < 2; y++ )
	{
		for ( int x = 0; x < 4; x++ )
		{
			planetImages[index] = new cMenuImage ( position.x+21+158*x, position.y+86+198*y );
			planetImages[index]->setReleasedFunction ( &mapReleased );
			planetImages[index]->setClickSound ( SoundData.SNDHudButton );
			menuItems.Add ( planetImages[index] );

			planetTitles[index] = new cMenuLabel ( position.x+77+158*x, position.y+48+198*y );
			planetTitles[index]->setCentered ( true );
			menuItems.Add ( planetTitles[index] );

			index++;
		}
	}

	selectedMapIndex = -1;
	offset = 0;

	loadMaps();
	showMaps();
}

cPlanetsSelectionMenu::~cPlanetsSelectionMenu()
{
	delete titleLabel;

	delete okButton;
	delete backButton;

	delete arrowUpButton;
	delete arrowDownButton;

	for ( int i = 0; i < 8; i++ )
	{
		delete planetImages[i];
		delete planetTitles[i];
	}
}

void cPlanetsSelectionMenu::loadMaps()
{
	maps = getFilesOfDirectory ( SettingsData.sMapsPath );
	for ( unsigned int i = 0; i < maps->Size(); i++ )
	{
		string const& map = (*maps)[i];
		if (map.substr(map.length() - 3, 3).compare("WRL") != 0 && map.substr(map.length() - 3, 3).compare("wrl") != 0)
		{
			maps->Delete ( i );
			i--;
		}
	}
}

void cPlanetsSelectionMenu::showMaps()
{
	for ( int i = 0; i < 8; i++ ) //only 8 maps on one screen
	{
		if ( i+offset < (int)maps->Size() )
		{
			string mapName = (*maps)[i + offset];
			string mapPath = SettingsData.sMapsPath + PATH_DELIMITER + mapName;

			if ( FileExists ( mapPath.c_str() ) )
			{
				SDL_RWops *mapFile = SDL_RWFromFile ( mapPath.c_str(), "rb" );
				if ( mapFile != NULL )
				{
					SDL_RWseek ( mapFile, 5, SEEK_SET );
					int size = SDL_ReadLE16( mapFile );

					sColor Palette[256];
					short sGraphCount;
					SDL_RWseek ( mapFile, 2 + size*size*3, SEEK_CUR );
					sGraphCount = SDL_ReadLE16( mapFile );
					SDL_RWseek ( mapFile, 64*64*sGraphCount, SEEK_CUR );
					SDL_RWread ( mapFile, &Palette, 1, 768 );

					SDL_Surface *mapSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, size, size, 8, 0, 0, 0, 0 );
					mapSurface->pitch = mapSurface->w;

					mapSurface->format->palette->ncolors = 256;
					for (int j = 0; j < 256; j++ )
					{
						mapSurface->format->palette->colors[j].r = Palette[j].cBlue;
						mapSurface->format->palette->colors[j].g = Palette[j].cGreen;
						mapSurface->format->palette->colors[j].b = Palette[j].cRed;
					}
					SDL_RWseek ( mapFile, 9, SEEK_SET );
					for( int iY = 0; iY < size; iY++ )
					{
						for( int iX = 0; iX < size; iX++ )
						{
							unsigned char cColorOffset;
							SDL_RWread ( mapFile, &cColorOffset, 1, 1 );
							Uint8 *pixel = (Uint8*) mapSurface->pixels  + (iY * size + iX);
							*pixel = cColorOffset;
						}
					}
					SDL_RWclose ( mapFile );

					#define MAPWINSIZE 112
					if( mapSurface->w != MAPWINSIZE || mapSurface->h != MAPWINSIZE ) // resize map
					{ 
						SDL_Surface *scaledMap = scaleSurface ( mapSurface, NULL, MAPWINSIZE, MAPWINSIZE );
						SDL_FreeSurface ( mapSurface );
						mapSurface = scaledMap;
					}

	#define SELECTED 0x00C000
	#define UNSELECTED 0x000000
					SDL_Surface *imageSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, MAPWINSIZE+8, MAPWINSIZE+8, SettingsData.iColourDepth, 0, 0, 0, 0 );

					if ( selectedMapIndex == i+offset )
					{
						SDL_FillRect ( imageSurface, NULL, SELECTED );

						if ( font->getTextWide ( ">" + mapName.substr( 0, mapName.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
						{
							while ( font->getTextWide ( ">" + mapName + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
							{
								mapName.erase ( mapName.length()-1, mapName.length() );
							}
							mapName = ">" + mapName + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<";
						}
						else mapName = ">" + mapName.substr( 0, mapName.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<";
					}
					else
					{
						SDL_FillRect ( imageSurface, NULL, UNSELECTED );

						if ( font->getTextWide ( ">" + mapName.substr( 0, mapName.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
						{
							while ( font->getTextWide ( ">" + mapName + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
							{
								mapName.erase ( mapName.length()-1, mapName.length() );
							}
							mapName = mapName + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")";
						}
						else mapName = mapName.substr( 0, mapName.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")";
					}
					SDL_Rect dest = { 4, 4, MAPWINSIZE, MAPWINSIZE };
					SDL_BlitSurface ( mapSurface, NULL, imageSurface, &dest );
					SDL_FreeSurface ( mapSurface );

					planetImages[i]->setImage ( imageSurface );
					SDL_FreeSurface ( imageSurface );
					planetTitles[i]->setText ( mapName );
				}
			}
		}
		else
		{
			planetImages[i]->setImage ( NULL );
			planetTitles[i]->setText ( "" );
		}
	}
	draw();
}

void cPlanetsSelectionMenu::backReleased( void* parent )
{
	cPlanetsSelectionMenu *menu = static_cast<cPlanetsSelectionMenu *>((cMenu*)parent);
	menu->gameDataContainer->map = NULL;
	menu->terminate = true;
}

void cPlanetsSelectionMenu::okReleased( void* parent )
{
	cPlanetsSelectionMenu *menu = static_cast<cPlanetsSelectionMenu *>((cMenu*)parent);
	if ( menu->selectedMapIndex >= 0 && menu->selectedMapIndex < (int)menu->maps->Size() )
	{
		menu->gameDataContainer->map = new cMap();
		menu->gameDataContainer->map->LoadMap ( (*menu->maps)[menu->selectedMapIndex] );
		if ( !menu->gameDataContainer->map ) return;

		switch ( menu->gameDataContainer->type )
		{
		case GAME_TYPE_SINGLE:
			{
				cPlayer *player = new cPlayer ( SettingsData.sPlayerName.c_str(), OtherData.colors[cl_red], 0, MAX_CLIENTS ); // Socketnumber MAX_CLIENTS for lokal client
				menu->gameDataContainer->players.Add ( player );
				
				bool started = false;
				while ( !started )
				{
					if (menu->gameDataContainer->settings->clans == SETTING_CLANS_ON)
					{
						cClanSelectionMenu clanMenu (menu->gameDataContainer, player, false);
						if ( clanMenu.show() != 0 )
						{
							menu->draw();
							menu->gameDataContainer->players.Delete ( 0 );
							return;
						}
					}
					
					cStartupHangarMenu startupHangarMenu( menu->gameDataContainer, player, false );
					if ( startupHangarMenu.show() == 0 ) started = true;
					else if ( menu->gameDataContainer->settings->clans == SETTING_CLANS_OFF )
					{
						menu->draw();
						menu->gameDataContainer->players.Delete ( 0 );
						return;
					}
				}
			}
			break;
		case GAME_TYPE_HOTSEAT:
			break;
		case GAME_TYPE_TCPIP:
			break;
		}
		menu->end = true;
	}
}

void cPlanetsSelectionMenu::arrowDownReleased( void* parent )
{
	cPlanetsSelectionMenu *menu = static_cast<cPlanetsSelectionMenu *>((cMenu*)parent);
	if ( menu->offset+8 < (int)menu->maps->Size() )
	{
		menu->offset += 8;
		menu->showMaps();
	}
}

void cPlanetsSelectionMenu::arrowUpReleased( void* parent )
{
	cPlanetsSelectionMenu *menu = static_cast<cPlanetsSelectionMenu *>((cMenu*)parent);
	if ( menu->offset-8 >= 0 )
	{
		menu->offset -= 8;
		menu->showMaps();
	}
}

void cPlanetsSelectionMenu::mapReleased( void* parent )
{
	cPlanetsSelectionMenu *menu = static_cast<cPlanetsSelectionMenu *>((cMenu*)parent);
	int index = 0;
	if ( mouse->x > menu->position.x+160 ) index++;
	if ( mouse->x > menu->position.x+320 ) index++;
	if ( mouse->x > menu->position.x+470 ) index++;

	if ( mouse->y > menu->position.y+240 ) index+=4;

	index += menu->offset;

	menu->selectedMapIndex = index;
	menu->showMaps();
	menu->okButton->setLocked ( false );
	menu->draw();
}


//-----------------------------------------------------------------------------------
// cClanSelectionMenu
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
cClanSelectionMenu::cClanSelectionMenu( cGameDataContainer *gameDataContainer_, cPlayer *player, bool noReturn )
: cMenu (LoadPCX (GFXOD_CLAN_SELECT))
, gameDataContainer (gameDataContainer_)
, player (player)
, clan (player->getClan () >= 0 ? player->getClan () : 0)
{
	okButton = new cMenuButton ( position.x+390, position.y+440, lngPack.i18n ("Text~Button~OK") );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );

	backButton = new cMenuButton ( position.x+50, position.y+440, lngPack.i18n ("Text~Button~Back") );
	backButton->setReleasedFunction ( &backReleased );
	if ( noReturn ) backButton->setLocked ( true );
	menuItems.Add ( backButton );
	
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+11, "Choose Clan" );
	titleLabel->setCentered (true);
	menuItems.Add (titleLabel);
	
	vector<string> clanLogoPaths;
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo1.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo2.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo3.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo4.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo5.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo6.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo7.pcx");
	clanLogoPaths.push_back (SettingsData.sGfxPath + PATH_DELIMITER + "clanlogo8.pcx");
	
	int xCount = 0;
	int yCount = 0;
	for (int i = 0; i < 8; i++, xCount++)
	{
		if (i == 4)
		{
			xCount = 0;
			yCount = 1;
		}
		SDL_Surface* img = LoadPCX (clanLogoPaths[i].c_str());
		SDL_SetColorKey ( img, SDL_SRCCOLORKEY, 0xFF00FF );
		clanImages[i] = new cMenuImage (position.x + 88 + xCount * 154 - (img ? (img->w / 2) : 0), position.y + 48 + yCount * 150, img);
		clanImages[i]->setReleasedFunction (&clanSelected);
		menuItems.Add (clanImages[i]);
		
		clanNames[i] = new cMenuLabel (position.x + 87 + xCount * 154, position.y + 144 + yCount * 150, cClanData::instance ().getClan (i)->getName ());
		clanNames[i]->setCentered (true);
		menuItems.Add (clanNames[i]);
	}
	clanNames[clan]->setText (">" + cClanData::instance ().getClan (clan)->getName () + "<");

	clanDescription1 = new cMenuLabel (position.x + 47, position.y + 362, "");	
	menuItems.Add (clanDescription1);
	clanDescription2 = new cMenuLabel (position.x + 380, position.y + 362, "");
	menuItems.Add (clanDescription2);
	clanShortDescription = new cMenuLabel (position.x + 47, position.y + 349, "");
	menuItems.Add (clanShortDescription);
	updateClanDescription ();
}

//-----------------------------------------------------------------------------------
cClanSelectionMenu::~cClanSelectionMenu ()
{
	delete okButton;
	delete backButton;
	
	delete titleLabel;
	delete clanDescription1;
	delete clanDescription2;
	
	for (int i = 0; i < 8; i++)
	{
		delete clanImages[i];
		delete clanNames[i];
	}
}

//-----------------------------------------------------------------------------------
void cClanSelectionMenu::okReleased (void* parent)
{
	cClanSelectionMenu* menu = dynamic_cast<cClanSelectionMenu*>((cMenu*)parent);
	if (menu == 0)
		return;
	menu->player->setClan (menu->clan);
	menu->end = true;
}

//-----------------------------------------------------------------------------------
void cClanSelectionMenu::backReleased (void* parent)
{
	cClanSelectionMenu* menu = static_cast<cClanSelectionMenu*>((cMenu*)parent);
	menu->terminate = true;
}

//-----------------------------------------------------------------------------------
void cClanSelectionMenu::clanSelected (void* parent)
{
	cClanSelectionMenu* menu = dynamic_cast<cClanSelectionMenu*>((cMenu*)parent);
	if (menu == 0)
		return;

	int newClan = (mouse->x - menu->position.x - 47) / 154;
	if (mouse->y > menu->position.y + 48 + 140)
		newClan += 4;
	
	if (0 <= newClan && newClan < 8 && newClan != menu->clan)
	{
		menu->clanNames[menu->clan]->setText (cClanData::instance ().getClan (menu->clan)->getName ());
		menu->clan = newClan;
		menu->clanNames[menu->clan]->setText (">" + cClanData::instance ().getClan (menu->clan)->getName () + "<");	
		menu->updateClanDescription ();
		menu->draw();
	}
}

//-----------------------------------------------------------------------------------
void cClanSelectionMenu::updateClanDescription ()
{
	cClan* clanInfo = cClanData::instance ().getClan (clan);
	if (clanInfo)
	{
		vector<string> strings = clanInfo->getClanStatsDescription ();
		
		string desc1;
		for ( unsigned int i = 0; i < 4 && i < strings.size (); i++)
		{
			desc1.append (strings[i]);
			desc1.append ("\n");
		}
		clanDescription1->setText (desc1);

		string desc2;
		for (unsigned int i = 4; i < strings.size (); i++)
		{
			desc2.append (strings[i]);
			desc2.append ("\n");
		}
		clanDescription2->setText (desc2);
		
		clanShortDescription->setText (clanInfo->getDescription ());
	}
	else
	{
		clanDescription1->setText ("Unknown");
		clanDescription1->setText ("");
	}
}

//-----------------------------------------------------------------------------------
void cClanSelectionMenu::handleNetMessage( cNetMessage *message )
{
	switch ( message->iType )
	{
		case MU_MSG_CLAN:
			gameDataContainer->receiveClan ( message );
			break;
		case MU_MSG_LANDING_VEHICLES:
			gameDataContainer->receiveLandingUnits ( message );
			break;
		case MU_MSG_UPGRADES:
			gameDataContainer->receiveUnitUpgrades ( message );
			break;
		case MU_MSG_LANDING_COORDS:
			gameDataContainer->receiveLandingPosition ( message );
			break;
	}
}



//-----------------------------------------------------------------------------------
// cHangarMenu
//-----------------------------------------------------------------------------------

cHangarMenu::cHangarMenu( SDL_Surface *background_, cPlayer *player_, eMenuBackgrounds backgroundType_ ) : cMenu (background_, backgroundType_), player(player_)
{
	selectionChangedFunc = NULL;

	doneButton = new cMenuButton ( position.x+447, position.y+452, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	menuItems.Add ( doneButton );

	backButton = new cMenuButton ( position.x+349, position.y+452, lngPack.i18n ("Text~Button~Back"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	menuItems.Add ( backButton );

	infoImage = new cMenuImage ( position.x+11, position.y+13 );
	menuItems.Add ( infoImage );

	infoText = new cMenuLabel ( position.x+21, position.y+23 );
	infoText->setBox ( 280, 220 );
	menuItems.Add ( infoText );

	infoTextCheckBox = new cMenuCheckButton ( position.x+291, position.y+264, lngPack.i18n ("Text~Comp~Description"), true, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD, cMenuCheckButton::TEXT_ORIENT_LEFT );
	infoTextCheckBox->setClickedFunction ( &infoCheckBoxClicked );
	menuItems.Add ( infoTextCheckBox );

	unitDetails = new cMenuUnitDetails ( position.x+16, position.y+297 );
	menuItems.Add ( unitDetails );

	selectionList = new cMenuUnitsList ( position.x+477,  position.y+50, 154, 326, this, MUL_DIS_TYPE_COSTS );
	menuItems.Add ( selectionList );

	selListUpButton = new cMenuButton ( position.x+471, position.y+387, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	selListUpButton->setReleasedFunction ( &selListUpReleased );
	menuItems.Add ( selListUpButton );

	selListDownButton = new cMenuButton ( position.x+491, position.y+387, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	selListDownButton->setReleasedFunction ( &selListDownReleased );
	menuItems.Add ( selListDownButton );

	selectedUnit = NULL;
}

cHangarMenu::~cHangarMenu()
{
	delete infoImage;
	delete infoText;
	delete infoTextCheckBox;

	delete unitDetails;

	delete selectionList;

	delete selListUpButton;
	delete selListDownButton;

	delete doneButton;
	delete backButton;
}

void cHangarMenu::drawUnitInformation()
{
	if ( !selectedUnit ) return;
	if ( selectedUnit->getUnitID().getVehicle(player) )
	{
		infoImage->setImage ( selectedUnit->getUnitID().getVehicle(player)->info );
		if ( infoTextCheckBox->isChecked() ) infoText->setText ( selectedUnit->getUnitID().getVehicle(player)->data.description );
		else infoText->setText ( "" );
	}
	else if ( selectedUnit->getUnitID().getBuilding(player) )
	{
		infoImage->setImage ( selectedUnit->getUnitID().getBuilding(player)->info );
		if ( infoTextCheckBox->isChecked() ) infoText->setText ( selectedUnit->getUnitID().getBuilding(player)->data.description );
		else infoText->setText ( "" );
	}
}

void cHangarMenu::infoCheckBoxClicked( void* parent )
{
	cHangarMenu *menu = static_cast<cHangarMenu*>((cMenu*)parent);
	menu->drawUnitInformation();
	menu->draw();
}

void cHangarMenu::selListUpReleased( void* parent )
{
	cHangarMenu *menu = static_cast<cHangarMenu*>((cMenu*)parent);
	menu->selectionList->scrollUp();
}

void cHangarMenu::selListDownReleased( void* parent )
{
	cHangarMenu *menu = static_cast<cHangarMenu*>((cMenu*)parent);
	menu->selectionList->scrollDown();
}

void cHangarMenu::setSelectedUnit( cMenuUnitListItem *selectedUnit_ )
{
	selectedUnit = selectedUnit_;
	unitDetails->setSelection ( selectedUnit );
	if ( selectionChangedFunc ) selectionChangedFunc ( this );
	drawUnitInformation();
}

cMenuUnitListItem *cHangarMenu::getSelectedUnit()
{
	return selectedUnit;
}

cAdvListHangarMenu::cAdvListHangarMenu( SDL_Surface *background_, cPlayer *player_ ) : cHangarMenu (background_, player_)
{
	secondList = new cMenuUnitsList ( position.x+330,  position.y+12, 130, 225, this, MUL_DIS_TYPE_CARGO );
	secondList->setDoubleClickedFunction ( &secondListDoubleClicked );
	menuItems.Add ( secondList );

	selectionList->setDoubleClickedFunction ( &selListDoubleClicked );

	secondListUpButton = new cMenuButton ( position.x+327, position.y+240, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	secondListUpButton->setReleasedFunction ( &secondListUpReleased );
	menuItems.Add ( secondListUpButton );

	secondListDownButton = new cMenuButton ( position.x+348, position.y+240, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	secondListDownButton->setReleasedFunction ( &secondListDownReleased );
	menuItems.Add ( secondListDownButton );
}

cAdvListHangarMenu::~cAdvListHangarMenu()
{
	delete secondList;

	delete secondListUpButton;
	delete secondListDownButton;
}

void cAdvListHangarMenu::secondListUpReleased( void* parent )
{
	cAdvListHangarMenu *menu = dynamic_cast<cAdvListHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	menu->secondList->scrollUp();
}

void cAdvListHangarMenu::secondListDownReleased( void* parent )
{
	cAdvListHangarMenu *menu = dynamic_cast<cAdvListHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	menu->secondList->scrollDown();
}

bool cAdvListHangarMenu::selListDoubleClicked( cMenuUnitsList* list, void *parent )
{
	cAdvListHangarMenu *menu = dynamic_cast<cAdvListHangarMenu*>((cHangarMenu*)parent);
	if ( !menu ) return false;
	if ( menu->selectedUnit && menu->selectedUnit == menu->selectionList->getSelectedUnit() )
	{
		sVehicle *vehicle = menu->selectedUnit->getUnitID().getVehicle(menu->player);
		if ( vehicle && menu->checkAddOk ( menu->selectedUnit ) )
		{
			menu->secondList->addUnit ( vehicle->data.ID, menu->player, menu->selectedUnit->getUpgrades(), true, menu->selectedUnit->getFixedResValue() );
			menu->secondList->getItem ( menu->secondList->getSize()-1 )->setResValue ( menu->selectedUnit->getResValue(), false );
			menu->addedCallback ( menu->selectedUnit );
			menu->draw();
		}
		return true;
	}
	return false;
}

bool cAdvListHangarMenu::secondListDoubleClicked( cMenuUnitsList* list, void *parent )
{
	cAdvListHangarMenu *menu = dynamic_cast<cAdvListHangarMenu*>((cHangarMenu*)parent);
	if ( !menu ) return false;
	if ( menu->selectedUnit->getFixedStatus() ) return false;
	if ( menu->selectedUnit && menu->selectedUnit == menu->secondList->getSelectedUnit() )
	{
		menu->removedCallback ( menu->selectedUnit );
		menu->secondList->removeUnit ( menu->selectedUnit );
		if ( menu->selectedUnit == NULL && menu->selectionList->getSelectedUnit() ) menu->setSelectedUnit ( menu->selectionList->getSelectedUnit() );
		menu->draw();
		return true;
	}
	return false;
}

cStartupHangarMenu::cStartupHangarMenu( cGameDataContainer *gameDataContainer_, cPlayer *player_, bool noReturn ) : cHangarMenu ( LoadPCX ( GFXOD_HANGAR ), player_ ), cUpgradeHangarMenu ( player_ ), cAdvListHangarMenu ( NULL, player_ ), gameDataContainer(gameDataContainer_)
{
	/*cMenuLabel *titleLabel = new cMenuLabel ( position.x+552, position.y+11, lngPack.i18n ("Text~Title~Hangar") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );*/

	if ( gameDataContainer->settings ) credits = gameDataContainer->settings->credits;
	else credits = 0;

	selectionChangedFunc = &selectionChanged;

	doneButton->setReleasedFunction ( &doneReleased );
	backButton->setReleasedFunction ( &backReleased );
	if ( noReturn ) backButton->setLocked ( true );

	upgradeBuyGroup = new cMenuRadioGroup();
	upgradeBuyGroup->addButton ( new cMenuCheckButton ( position.x+542, position.y+445, lngPack.i18n ("Text~Button~Buy"), true, false, cMenuCheckButton::RADIOBTN_TYPE_BTN_ROUND ) );
	upgradeBuyGroup->addButton ( new cMenuCheckButton ( position.x+542, position.y+445+17, lngPack.i18n ("Text~Button~Upgrade"), false, false, cMenuCheckButton::RADIOBTN_TYPE_BTN_ROUND ) );
	upgradeBuyGroup->setClickedFunction ( &subButtonsChanged );
	menuItems.Add ( upgradeBuyGroup );

	upgradeFilter = new cMenuUpgradeFilter ( position.x+467, position.y+411, this );
	menuItems.Add ( upgradeFilter );

	materialBar = new cMenuMaterialBar ( position.x+421, position.y+301, position.x+430, position.y+275, 0, cMenuMaterialBar::MAT_BAR_TYPE_METAL );
	materialBar->setClickedFunction ( materialBarClicked );
	menuItems.Add ( materialBar );
	materialBarLabel = new cMenuLabel ( position.x+430, position.y+285, lngPack.i18n ("Text~Title~Cargo") );
	materialBarLabel->setCentered( true );
	menuItems.Add ( materialBarLabel );

	goldBar->setMaximalValue ( credits );
	goldBar->setCurrentValue ( credits );

	materialBarUpButton = new cMenuButton ( position.x+413, position.y+424, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	materialBarUpButton->setReleasedFunction ( &materialBarUpReleased );
	menuItems.Add ( materialBarUpButton );

	materialBarDownButton = new cMenuButton ( position.x+433, position.y+424, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	materialBarDownButton->setReleasedFunction ( &materialBarDownReleased );
	menuItems.Add ( materialBarDownButton );

	generateSelectionList();

	if ( gameDataContainer->settings->bridgeHead == SETTING_BRIDGEHEAD_DEFINITE )
	{
		for ( int i = 0; i < selectionList->getSize(); i++ )
		{
			sVehicle *vehicle = selectionList->getItem ( i )->getUnitID().getVehicle(player);
			if ( !vehicle ) continue;
			if ( vehicle->data.canBuild.compare( "BigBuilding" )==0 || vehicle->data.canBuild.compare( "SmallBuilding" )==0 || vehicle->data.canSurvey )
			{
				cMenuUnitListItem *unit = secondList->addUnit ( vehicle->data.ID, player, selectionList->getItem ( i )->getUpgrades() );
				if ( vehicle->data.canBuild.compare( "BigBuilding" )==0 ) 
					unit->setMinResValue ( 40 );
				else if ( vehicle->data.canBuild.compare( "SmallBuilding" )==0 ) 
					unit->setMinResValue ( 20 );
				unit->setFixed ( true );
			}
		}			
		if ( gameDataContainer->settings->clans == SETTING_CLANS_ON && player->getClan () == 7) // Additional Units for Axis Inc. Clan
		{
			int startCredits = gameDataContainer->settings->credits;
			int numAddConstructors = 0;
			int numAddEngineers = 0;
			if (startCredits < 100)
				numAddEngineers = 1;
			else if (startCredits < 150)
			{
				numAddEngineers = 1;
				numAddConstructors = 1;
			}
			else if (startCredits < 200)
			{
				numAddEngineers = 2;
				numAddConstructors = 1;
			}
			else if (startCredits < 300)
			{
				numAddEngineers = 2;
				numAddConstructors = 2;
			}
			else
			{
				numAddEngineers = 3;
				numAddConstructors = 2;				
			}
			for (int i = 0; i < selectionList->getSize (); i++)
			{
				sVehicle *vehicle = selectionList->getItem (i)->getUnitID ().getVehicle (player);
				if ( !vehicle ) continue;
				
				if (vehicle->data.canBuild.compare( "BigBuilding" )==0)
				{
					for (int j = 0; j < numAddConstructors; j++)
					{
						cMenuUnitListItem *unit = secondList->addUnit (vehicle->data.ID, player, selectionList->getItem (i)->getUpgrades ());
						unit->setFixed (true);
					}
				}
				if (vehicle->data.canBuild.compare( "SmallBuilding" )==0)
				{
					for (int j = 0; j < numAddEngineers; j++)
					{
						cMenuUnitListItem *unit = secondList->addUnit (vehicle->data.ID, player, selectionList->getItem (i)->getUpgrades ());
						unit->setFixed (true);
					}
				}
			}
		}
	}
	if ( selectionList->getSize() > 0 ) setSelectedUnit ( selectionList->getItem ( 0 ) );
}

cStartupHangarMenu::~cStartupHangarMenu()
{
	delete upgradeBuyGroup;

	delete materialBar;
	delete materialBarLabel;
	delete materialBarUpButton;
	delete materialBarDownButton;
}

void cStartupHangarMenu::updateUnitData()
{
	for ( unsigned int i = 0; i < UnitsData.getNrVehicles () + UnitsData.getNrBuildings (); i++ )
	{
		sUnitData *data;
		if ( i < UnitsData.getNrVehicles () ) data = &player->VehicleData[i];
		else data = &player->BuildingData[i - UnitsData.getNrVehicles ()];

		for ( int j = 0; j < 8; j++ )
		{
			sUnitUpgrade *upgrades = unitUpgrades[i];
			switch ( upgrades[j].type )
			{
				case sUnitUpgrade::UPGRADE_TYPE_DAMAGE:
					data->damage = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_SHOTS:
					data->shotsMax = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_RANGE:
					data->range = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_AMMO:
					data->ammoMax = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_ARMOR:
					data->armor = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_HITS:
					data->hitpointsMax = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_SCAN:
					data->scan = upgrades[j].curValue;
					break;
				case sUnitUpgrade::UPGRADE_TYPE_SPEED:
					data->speedMax = upgrades[j].curValue;
					break;
			}
		}
	}
}

void cStartupHangarMenu::backReleased( void* parent )
{
	cStartupHangarMenu *menu = dynamic_cast<cStartupHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	menu->terminate = true;
}

void cStartupHangarMenu::doneReleased( void* parent )
{
	cStartupHangarMenu *menu = dynamic_cast<cStartupHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;

	menu->updateUnitData();

	cList<sLandingUnit> *landingUnits  = new cList<sLandingUnit>;
	for ( int i = 0; i < menu->secondList->getSize(); i++ )
	{
		sLandingUnit landingUnit;
		landingUnit.unitID = menu->secondList->getItem ( i )->getUnitID();
		landingUnit.cargo = menu->secondList->getItem ( i )->getResValue();
		landingUnits->Add ( landingUnit );
	}
	if (menu->gameDataContainer->landingUnits.Size () == 0) // the size can be != 0, if a client sent his landingunits before the host is done with the startup hangar
		menu->gameDataContainer->landingUnits.Add ( landingUnits ); // TODO: alzi, for clients it shouldn't be necessary to store the landing units, or? (pagra)
	else
		menu->gameDataContainer->landingUnits[0] = landingUnits;
	if ( menu->gameDataContainer->type == GAME_TYPE_TCPIP && menu->player->Nr != 0 )
	{
		sendClan ( menu->player->getClan (), menu->player->Nr );
		sendLandingUnits ( landingUnits, menu->player->Nr );
	}

	sendUnitUpgrades ( menu->player );

	cLandingMenu landingMenu ( menu->gameDataContainer, menu->player );
	if ( landingMenu.show() == 1 && menu->gameDataContainer->type != GAME_TYPE_TCPIP)
	{
		menu->draw();
		return;
	}
	menu->end = true;
}

void cStartupHangarMenu::subButtonsChanged( void* parent )
{
	cStartupHangarMenu *menu = dynamic_cast<cStartupHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	menu->generateSelectionList();
	menu->draw();
}

void cStartupHangarMenu::materialBarUpReleased( void* parent )
{
	cStartupHangarMenu *menu = dynamic_cast<cStartupHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	if ( menu->secondList->getSelectedUnit() && menu->credits > 0 )
	{
		int oldCargo = menu->secondList->getSelectedUnit()->getResValue();
		menu->secondList->getSelectedUnit()->setResValue ( oldCargo+5 );
		menu->materialBar->setCurrentValue ( menu->secondList->getSelectedUnit()->getResValue() );
		if ( oldCargo != menu->secondList->getSelectedUnit()->getResValue() )
		{
			menu->credits--;
			menu->goldBar->setCurrentValue ( menu->credits );
		}
		menu->upgradeButtons->setSelection ( menu->selectedUnit );
		menu->draw();
	}
}

void cStartupHangarMenu::materialBarDownReleased( void* parent )
{
	cStartupHangarMenu *menu = dynamic_cast<cStartupHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	if ( menu->secondList->getSelectedUnit() )
	{
		int oldCargo = menu->secondList->getSelectedUnit()->getResValue();
		menu->secondList->getSelectedUnit()->setResValue ( oldCargo-5 );
		menu->materialBar->setCurrentValue ( menu->secondList->getSelectedUnit()->getResValue() );
		if ( oldCargo != menu->secondList->getSelectedUnit()->getResValue() )
		{
			menu->credits++;
			menu->goldBar->setCurrentValue ( menu->credits );
		}
		menu->upgradeButtons->setSelection ( menu->selectedUnit );
		menu->draw();
	}
}

void cStartupHangarMenu::materialBarClicked( void* parent )
{
	cStartupHangarMenu *menu = dynamic_cast<cStartupHangarMenu*>((cMenu*)parent);
	if ( !menu ) return;
	if ( menu->secondList->getSelectedUnit() && menu->secondList->getSelectedUnit()->getUnitID().getVehicle(menu->player) )
	{
		int oldCargo = menu->secondList->getSelectedUnit()->getResValue();
		int newCargo = (int)((float)(menu->position.y+301+115-mouse->y)/115 * menu->secondList->getSelectedUnit()->getUnitID().getUnitDataOriginalVersion(menu->player)->storageResMax );
		if ( newCargo%5 < 3 ) newCargo -= newCargo%5;
		else newCargo += 5-newCargo%5;

		menu->secondList->getSelectedUnit()->setResValue ( newCargo );

		newCargo = menu->secondList->getSelectedUnit()->getResValue();
		int costs = (newCargo-oldCargo)/5;
		if ( costs > menu->credits )
		{
			costs = menu->credits;
			newCargo = costs * 5 + oldCargo;
		}

		menu->secondList->getSelectedUnit()->setResValue ( newCargo );
		menu->materialBar->setCurrentValue ( menu->secondList->getSelectedUnit()->getResValue() );

		menu->credits -= costs;
		menu->goldBar->setCurrentValue ( menu->credits );
		menu->upgradeButtons->setSelection ( menu->selectedUnit );
		menu->draw();
	}
}

void cStartupHangarMenu::generateSelectionList()
{
	sID oldSelectdUnit;
	sBuilding *oldSelectdBuilding = NULL;
	if ( selectionList->getSelectedUnit() ) oldSelectdUnit = selectionList->getSelectedUnit()->getUnitID();

	selectionList->clear();
	bool tank = upgradeFilter->TankIsChecked();
	bool plane = upgradeFilter->PlaneIsChecked();
	bool ship = upgradeFilter->ShipIsChecked();
	bool build = upgradeFilter->BuildingIsChecked();
	bool tnt = upgradeFilter->TNTIsChecked();
	bool buy = upgradeBuyGroup->buttonIsChecked ( 0 );

	if ( buy ) plane = ship = build = false;

	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
	{
		if ( !tank && !ship && !plane ) continue;
		sUnitData &data = UnitsData.getVehicle(i, player->getClan()).data;
		if ( data.isHuman && buy ) continue;
		if ( tnt && !data.canAttack ) continue;
		if ( data.factorAir > 0 && !plane ) continue;
		if ( data.factorSea > 0 && data.factorGround == 0 && !ship ) continue;
		if ( data.factorGround > 0 && !tank ) continue;
		selectionList->addUnit ( data.ID, player, unitUpgrades[i] );
	}

	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
	{
		if ( !build ) continue;
		sUnitData &data = UnitsData.getBuilding(i, player->getClan()).data;
		if ( tnt && !data.canAttack ) continue;
		selectionList->addUnit ( data.ID, player, unitUpgrades[UnitsData.getNrVehicles ()+i] );
	}

	for ( int i = 0; i < selectionList->getSize(); i++ )
	{
		if ( oldSelectdUnit == selectionList->getItem( i )->getUnitID() )
		{
			selectionList->setSelection ( selectionList->getItem( i ) );
			break;
		}
	}
	if ( selectionList->getSelectedUnit() == NULL && selectionList->getSize() > 0 ) selectionList->setSelection ( selectionList->getItem( 0 ) );
}

bool cStartupHangarMenu::isInLandingList( cMenuUnitListItem *item )
{
	for ( int i = 0; i < secondList->getSize(); i++ )
	{
		if ( secondList->getItem ( i ) == item ) return true;
	}
	return false;
}

bool cStartupHangarMenu::checkAddOk ( cMenuUnitListItem *item )
{
	sVehicle *vehicle = item->getUnitID().getVehicle(player);
	if ( !vehicle  ) return false;

	if ( vehicle->data.factorGround == 0 ) return false;
	if ( vehicle->data.isHuman ) return false;
	if ( vehicle->data.buildCosts > credits ) return false;
	return true;
}

void cStartupHangarMenu::addedCallback ( cMenuUnitListItem *item )
{
	sVehicle *vehicle = item->getUnitID().getVehicle(player);
	if ( !vehicle  ) return;

	credits -= vehicle->data.buildCosts;
	goldBar->setCurrentValue ( credits );
	selectionChanged( this );
}

void cStartupHangarMenu::removedCallback ( cMenuUnitListItem *item )
{
	sVehicle *vehicle = item->getUnitID().getVehicle(player);
	if ( !vehicle  ) return;

	credits += vehicle->data.buildCosts + item->getResValue()/5;
	goldBar->setCurrentValue ( credits );
}

void cStartupHangarMenu::handleNetMessage( cNetMessage *message )
{
	switch ( message->iType )
	{
	case MU_MSG_CLAN:
		gameDataContainer->receiveClan ( message );
		break;
	case MU_MSG_LANDING_VEHICLES:
		gameDataContainer->receiveLandingUnits ( message );
		break;
	case MU_MSG_UPGRADES:
		gameDataContainer->receiveUnitUpgrades ( message );
		break;
	case MU_MSG_LANDING_COORDS:
		gameDataContainer->receiveLandingPosition ( message );
		break;
	}
}

void cStartupHangarMenu::selectionChanged( void *parent )
{
	cStartupHangarMenu *menu;
	menu = dynamic_cast<cStartupHangarMenu*>((cHangarMenu*)parent);
	if ( !menu ) menu = dynamic_cast<cStartupHangarMenu*>((cStartupHangarMenu*)parent);
	if ( !menu ) return;
	sVehicle *vehicle;
	if ( menu->secondList->getSelectedUnit() && (vehicle = menu->secondList->getSelectedUnit()->getUnitID().getVehicle(menu->player) ) && vehicle->data.storeResType != sUnitData::STORE_RES_NONE )
	{
		menu->materialBar->setMaximalValue ( vehicle->data.storageResMax );
		menu->materialBar->setCurrentValue ( menu->secondList->getSelectedUnit()->getResValue() );
	}
	else
	{
		menu->materialBar->setMaximalValue ( 0 );
		menu->materialBar->setCurrentValue ( 0 );
	}
	menu->upgradeButtons->setSelection ( menu->selectedUnit );
	menu->draw();
}

cLandingMenu::cLandingMenu ( cGameDataContainer *gameDataContainer_, cPlayer *player_ ) : cMenu ( NULL ), hudSurface (NULL), mapSurface(NULL), gameDataContainer(gameDataContainer_), player(player_)
{
	map = gameDataContainer->map;

	createMap();
	mapImage = new cMenuImage ( 180, 18, mapSurface );
	mapImage->setClickedFunction ( &mapClicked );
	menuItems.Add ( mapImage );

	circlesImage = new cMenuImage ( 180, 18, NULL );
	menuItems.Add ( circlesImage );

	createHud();
	hudImage = new cMenuImage ( 0, 0, hudSurface );
	hudImage->setMovedOverFunction ( &mouseMoved );
	menuItems.Add ( hudImage );

	infoLabel = new cMenuLabel ( position.x+180+(position.w-180)/2-(SettingsData.iScreenW-200)/2, position.y+position.h/2-font->getFontHeight ( FONT_LATIN_BIG ), "", FONT_LATIN_BIG );
	infoLabel->setBox ( (SettingsData.iScreenW-200), font->getFontHeight ( FONT_LATIN_BIG )*2 );
	menuItems.Add ( infoLabel );
}

cLandingMenu::~cLandingMenu()
{
	delete hudImage;
	delete mapImage;
	delete circlesImage;
	delete infoLabel;

	if ( hudSurface ) SDL_FreeSurface ( hudSurface );
	if ( mapSurface ) SDL_FreeSurface ( mapSurface );
}

void cLandingMenu::createHud()
{
	if ( hudSurface ) SDL_FreeSurface ( hudSurface );
	hudSurface = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, SettingsData.iScreenW, SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );

	SDL_FillRect ( hudSurface, NULL, 0xFF00FF );
	SDL_SetColorKey ( hudSurface, SDL_SRCCOLORKEY, 0xFF00FF );

	SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, hudSurface, NULL );
	
	SDL_Rect top, bottom;
	top.x=0;
	top.y= ( SettingsData.iScreenH/2 )-479;

	bottom.x=0;
	bottom.y= ( SettingsData.iScreenH/2 );

	SDL_BlitSurface ( GraphicsData.gfx_panel_top, NULL, hudSurface, &top );
	SDL_BlitSurface ( GraphicsData.gfx_panel_bottom, NULL, hudSurface, &bottom );
}

void cLandingMenu::createMap()
{
	if ( mapSurface ) SDL_FreeSurface ( mapSurface );
	mapSurface = SDL_CreateRGBSurface( SDL_HWSURFACE, SettingsData.iScreenW-192, SettingsData.iScreenH-32, SettingsData.iColourDepth, 0, 0, 0, 0 );

	if ( SDL_MUSTLOCK(mapSurface) ) SDL_LockSurface ( mapSurface );
	for ( int x = 0; x < mapSurface->w; x++ )
	{
		int terrainx = (x * map->size) / mapSurface->w;
		if ( terrainx >= map->size ) terrainx = map->size - 1;
		int offsetx  = ((x * map->size ) % mapSurface->w) * 64 / mapSurface->w;

		for ( int y = 0; y < mapSurface->h; y++ )
		{
			int terrainy = (y * map->size) / mapSurface->h;
			if ( terrainy >= map->size ) terrainy = map->size - 1;
			int offsety  = ((y * map->size ) % mapSurface->h) * 64 / mapSurface->h;

			unsigned int terrainNumber = map->Kacheln[terrainx + terrainy * map->size];
			sTerrain *t	= map->terrain + terrainNumber;
			unsigned int ColorNr = *( ( unsigned char* ) ( t->sf_org->pixels ) + (offsetx + offsety*64));

			unsigned char* pixel = (unsigned char*) &( ( Sint32* ) ( mapSurface->pixels ) ) [x+y*mapSurface->w];
			pixel[0] = map->palette[ColorNr].b;
			pixel[1] = map->palette[ColorNr].g;
			pixel[2] = map->palette[ColorNr].r;
		}
	}
	if ( SDL_MUSTLOCK(mapSurface) )SDL_UnlockSurface ( mapSurface );
}

sTerrain *cLandingMenu::getMapTile ( int x, int y )
{
	double fak;
	int nr;
	if ( x < 0 || x >= SettingsData.iScreenW-192 || y < 0 || y >= SettingsData.iScreenH-32 ) return NULL;

	x = (int)( x * ( 448.0 / ( SettingsData.iScreenW-180 ) ) );
	y = (int)( y * ( 448.0 / ( SettingsData.iScreenH-32 ) ) );

	if ( map->size < 448 )
	{
		fak = 448.0 / map->size;
		x= (int)( x / fak );
		y= (int)( y / fak );
		nr=map->Kacheln[x + y*map->size];
	}
	else
	{
		fak = map->size / 448.0;
		x = (int)( x * fak );
		y = (int)( y * fak );
		nr = map->Kacheln[x + y*map->size];
	}
	return &map->terrain[nr];
}

void cLandingMenu::mapClicked( void* parent )
{
	cLandingMenu *menu = static_cast<cLandingMenu*>((cMenu*)parent);

	if ( menu->landData.landingState == LANDING_POSITION_OK ) return;

	if ( mouse->cur != GraphicsData.gfx_Cmove ) return;

	float fakx = (float)( ( SettingsData.iScreenW-192.0 ) / menu->map->size ); //pixel per field in x direction
	float faky = (float)( ( SettingsData.iScreenH-32.0 ) / menu->map->size );  //pixel per field in y direction

	menu->landData.iLandX = (int) ( ( mouse->x-180 ) / ( 448.0/menu->map->size ) * ( 448.0/ ( SettingsData.iScreenW-192 )));
	menu->landData.iLandY = (int) ( ( mouse->y-18  ) / ( 448.0/menu->map->size ) * ( 448.0/ ( SettingsData.iScreenH-32 )));
	menu->landData.landingState = LANDING_POSITION_OK;

	SDL_Surface *circleSurface = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, SettingsData.iScreenW-192, SettingsData.iScreenH-32, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( circleSurface, NULL, 0xFF00FF );
	SDL_SetColorKey ( circleSurface, SDL_SRCCOLORKEY, 0xFF00FF );

	int posX = (int)(menu->landData.iLandX * fakx);
	int posY = (int)(menu->landData.iLandY * faky);
	//for non 4:3 screen resolutions, the size of the circles is
	//only correct in x dimension, because I don't draw an ellipse
	drawCircle( posX, posY, (int)((LANDING_DISTANCE_WARNING/2)*fakx), SCAN_COLOR, circleSurface );
	drawCircle( posX, posY, (int)((LANDING_DISTANCE_TOO_CLOSE/2)*fakx), RANGE_GROUND_COLOR, circleSurface );

	menu->circlesImage->setImage ( circleSurface );
	SDL_FreeSurface ( circleSurface );

	SHOW_SCREEN
	mouse->SetCursor ( CHand );
	mouse->draw ( false, screen );

	menu->hitPosition();
}

void cLandingMenu::mouseMoved( void* parent )
{
	cLandingMenu *menu = static_cast<cLandingMenu*>((cMenu*)parent);
	sTerrain *terrain = menu->getMapTile ( mouse->x-180, mouse->y-18 );
	if ( mouse->x >= 180 && mouse->x < SettingsData.iScreenW-12 && mouse->y >= 18 && mouse->y < SettingsData.iScreenH-14 && terrain && !( terrain->water || terrain->coast || terrain->blocked ) ) mouse->SetCursor ( CMove );
	else mouse->SetCursor ( CNo );
}

void cLandingMenu::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	if ( key.keysym.sym == SDLK_ESCAPE && key.state == SDL_PRESSED )
	{
		ActiveMenu = NULL;
		// TODO: may use another text here
		cDialogYesNow yesNoDialog ( lngPack.i18n( "Text~Comp~End_Game") );
		if ( yesNoDialog.show() == 0  ) terminate = true;
		else draw();
		ActiveMenu = this;
	}
}

void cLandingMenu::handleNetMessage( cNetMessage *message )
{
	// becouse the messages for landing units and landing positions can be send during
	// the host is in the hangar menu or in the landing selection menu, the gameGataContainer class
	// will receive and handle the messages directly
	switch ( message->iType )
	{
	case MU_MSG_CLAN:
		gameDataContainer->receiveClan ( message );
		break;
	case MU_MSG_LANDING_VEHICLES:
		gameDataContainer->receiveLandingUnits ( message );
		break;
	case MU_MSG_UPGRADES:
		gameDataContainer->receiveUnitUpgrades ( message );
		break;
	case MU_MSG_LANDING_COORDS:
		gameDataContainer->receiveLandingPosition ( message );
		break;
	case MU_MSG_RESELECT_LANDING:
		Log.write("Client: received MU_MSG_RESELECT_LANDING", cLog::eLOG_TYPE_NET_DEBUG);
		landData.landingState = (eLandingState) message->popChar();

		if( landData.landingState == LANDING_POSITION_TOO_CLOSE) infoLabel->setText ( lngPack.i18n ( "Text~Comp~Landing_Too_Close" ) );
		else if ( landData.landingState == LANDING_POSITION_WARNING) infoLabel->setText ( lngPack.i18n ( "Text~Comp~Landing_Warning" ) );

		draw();
		break;
	case MU_MSG_ALL_LANDED:
		ActiveMenu = NULL;
		gameDataContainer->runGame( player->Nr );
		end = true;
		break;
	}
}

void cLandingMenu::hitPosition()
{
	switch ( gameDataContainer->type )
	{
	case GAME_TYPE_SINGLE:
		{
			draw();
			gameDataContainer->landData.Add ( new sClientLandData ( landData ) );

			ActiveMenu = NULL;
			gameDataContainer->runGame( 0 );
			end = true;
		}
		break;
	case GAME_TYPE_TCPIP:
		{
			infoLabel->setText ( lngPack.i18n ( "Text~Multiplayer~Waiting" ) );
			sendLandingCoords ( landData, player->Nr );
			draw();
		}
		break;
	}
}

cNetworkMenu::cNetworkMenu() : cMenu ( LoadPCX ( GFXOD_MULT ) )
{
	ip = SettingsData.sIP;
	port = SettingsData.iPort;

	playersBox = new cMenuPlayersBox ( position.x+465, position.y+284, 167, 124, this );
	menuItems.Add ( playersBox );

	actPlayer = new sMenuPlayer ( SettingsData.sPlayerName, SettingsData.iColor, false, 0, MAX_CLIENTS );
	players.Add ( actPlayer );
	playersBox->setPlayers ( &players );

	backButton = new cMenuButton ( position.x+50, position.y+450, lngPack.i18n ( "Text~Button~Back" ) );
	backButton->setReleasedFunction ( &backReleased );
	menuItems.Add ( backButton );

	sendButton = new cMenuButton ( position.x+470, position.y+416, lngPack.i18n ( "Text~Title~Send" ), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	sendButton->setReleasedFunction ( &sendReleased );
	menuItems.Add ( sendButton );

	mapImage = new cMenuImage ( position.x+33, position.y+106 );
	menuItems.Add ( mapImage );

	mapLabel = new cMenuLabel ( position.x+90, position.y+65 );
	mapLabel->setCentered ( true );
	menuItems.Add ( mapLabel );

	settingsText = new cMenuLabel ( position.x+192, position.y+52 );
	settingsText->setBox ( 246, 146 );
	menuItems.Add ( settingsText );
	showSettingsText();

	chatBox = new cMenuListBox ( position.x+14, position.y+284, 439, 124, 50, this );
	menuItems.Add ( chatBox );

	chatLine = new cMenuLineEdit ( position.x+15, position.y+420, 438, 17, this );
	chatLine->setReturnPressedFunc ( &sendReleased );
	menuItems.Add ( chatLine );

	ipLabel = new cMenuLabel ( position.x+20, position.y+245, lngPack.i18n ( "Text~Title~IP" ) );
	menuItems.Add ( ipLabel );
	portLabel = new cMenuLabel ( position.x+228, position.y+245, lngPack.i18n ( "Text~Title~Port" ) );
	menuItems.Add ( portLabel );
	nameLabel = new cMenuLabel ( position.x+352, position.y+245, lngPack.i18n ( "Text~Title~Player_Name" ) );
	menuItems.Add ( nameLabel );
	colorLabel = new cMenuLabel ( position.x+500, position.y+245, lngPack.i18n ( "Text~Title~Color" ) );
	menuItems.Add ( colorLabel );

	ipLine = new cMenuLineEdit ( position.x+15, position.y+256, 188, 17, this );
	ipLine->setWasKeyInputFunction ( &portIpChanged );
	menuItems.Add ( ipLine );

	portLine = new cMenuLineEdit ( position.x+224, position.y+256, 106, 17, this );
	portLine->setWasKeyInputFunction ( &portIpChanged );
	portLine->setText ( iToStr ( port ) );
	portLine->setTaking ( false, true );
	menuItems.Add ( portLine );

	nameLine = new cMenuLineEdit ( position.x+347, position.y+256, 106, 17, this );
	nameLine->setText ( actPlayer->name );
	nameLine->setWasKeyInputFunction ( &wasNameImput );
	menuItems.Add ( nameLine );

	nextColorButton = new cMenuButton ( position.x+596, position.y+256, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	nextColorButton->setReleasedFunction ( &nextColorReleased );
	menuItems.Add ( nextColorButton );
	prevColorButton = new cMenuButton ( position.x+478, position.y+256, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	prevColorButton->setReleasedFunction ( &prevColorReleased );
	menuItems.Add ( prevColorButton );
	colorImage = new cMenuImage ( position.x+505, position.y+260 );
	setColor ( actPlayer->color );
	menuItems.Add ( colorImage );

	gameDataContainer.type = GAME_TYPE_TCPIP;

	network = new cTCP;
}

cNetworkMenu::~cNetworkMenu()
{
	delete backButton;
	delete sendButton;

	delete mapImage;
	delete mapLabel;

	delete settingsText;

	delete chatBox;
	delete chatLine;

	delete ipLabel;
	delete portLabel;
	delete nameLabel;
	delete colorLabel;

	delete nextColorButton;
	delete prevColorButton;
	delete colorImage;

	delete ipLine;
	delete portLine;
	delete nameLine;

	delete playersBox;

	delete network;
	network = NULL;
}

void cNetworkMenu::showSettingsText()
{
	string text;

	text = lngPack.i18n ( "Text~Main~Version", PACKAGE_VERSION ) + "\n";
	text += "Checksum: " + iToStr( SettingsData.Checksum ) + "\n\n";

	if ( !saveGameString.empty() ) text += lngPack.i18n ( "Text~Title~Savegame" ) + ":\n  " + saveGameString + "\n";
	
	if ( gameDataContainer.map )
	{
		text += lngPack.i18n ( "Text~Title~Map" ) + ": " + gameDataContainer.map->MapName;
		text += " (" + iToStr( gameDataContainer.map->size ) + "x" + iToStr( gameDataContainer.map->size ) + ")\n";
	}
	else if ( gameDataContainer.savegame.empty() ) text += lngPack.i18n ( "Text~Multiplayer~Map_NoSet" ) + "\n";

	text += "\n";

	if ( gameDataContainer.savegame.empty() )
	{
		if ( gameDataContainer.settings )
		{
			sSettings *settings = gameDataContainer.settings;
			text += lngPack.i18n ( "Text~Title~Metal" ) + ": " + settings->getResValString ( settings->metal ) + "\n";
			text += lngPack.i18n ( "Text~Title~Oil" ) + ": " + settings->getResValString ( settings->oil ) + "\n";
			text += lngPack.i18n ( "Text~Title~Gold" ) + ": " + settings->getResValString ( settings->gold ) + "\n";
			text += lngPack.i18n ( "Text~Title~Resource_Density" ) + ": " + settings->getResFreqString() + "\n";
			text += lngPack.i18n ( "Text~Title~Credits" )  + ": " + iToStr( settings->credits ) + "\n";
			text += lngPack.i18n ( "Text~Title~BridgeHead" ) + ": " + ( settings->bridgeHead == SETTING_BRIDGEHEAD_DEFINITE ? lngPack.i18n ( "Text~Option~Definite" ) : lngPack.i18n ( "Text~Option~Mobile" ) ) + "\n";
			text += lngPack.i18n ( "Text~Title~Alien_Tech" ) + ": " + ( settings->alienTech == SETTING_ALIENTECH_ON ? lngPack.i18n ( "Text~Option~On" ) : lngPack.i18n ( "Text~Option~Off" ) ) + "\n";
			text += string ("Clans") + ": " + ( settings->clans == SETTING_CLANS_ON ? lngPack.i18n ( "Text~Option~On" ) : lngPack.i18n ( "Text~Option~Off" ) ) + "\n"; // TODO: translate
			text += lngPack.i18n ( "Text~Title~Game_Type" ) + ": " + ( settings->gameType == SETTINGS_GAMETYPE_TURNS ? lngPack.i18n ( "Text~Option~Type_Turns" ) : lngPack.i18n ( "Text~Option~Type_Simu" ) ) + "\n";
		}
		else if ( gameDataContainer.savegame.empty() ) text += lngPack.i18n ( "Text~Multiplayer~Option_NoSet" ) + "\n";
	}
	settingsText->setText ( text );
}

void cNetworkMenu::showMap()
{
	if ( !gameDataContainer.map ) return;
	SDL_Surface *surface = NULL;
	int size;
	SDL_RWops *fp = SDL_RWFromFile ( (SettingsData.sMapsPath + PATH_DELIMITER + gameDataContainer.map->MapName ).c_str(),"rb" );
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

		surface = SDL_CreateRGBSurface( SDL_SWSURFACE, size, size, 8, 0, 0, 0, 0 );

		if ( SDL_MUSTLOCK ( surface ) ) SDL_LockSurface ( surface );
		surface->pitch = surface->w;

		surface->format->palette->ncolors = 256;
		for (int j = 0; j < 256; j++ )
		{
			surface->format->palette->colors[j].r = Palette[j].cBlue;
			surface->format->palette->colors[j].g = Palette[j].cGreen;
			surface->format->palette->colors[j].b = Palette[j].cRed;
		}
		SDL_RWseek ( fp, 9, SEEK_SET );
		for( int iY = 0; iY < size; iY++ )
		{
			for( int iX = 0; iX < size; iX++ )
			{
				unsigned char cColorOffset;
				SDL_RWread ( fp, &cColorOffset, 1, 1 );
				Uint8 *pixel = (Uint8*) surface->pixels  + (iY * size + iX);
				*pixel = cColorOffset;
			}
		}
		if ( SDL_MUSTLOCK ( surface ) ) SDL_UnlockSurface ( surface );

		SDL_RWclose ( fp );
	}
	if ( surface != NULL )
	{
		SDL_Rect dest;
		dest.x = 33;
		dest.y = 106;
		dest.w = dest.h = 112;
		SDL_Rect rSrc = { 0, 0, dest.w, dest.h };
		#define MAPWINSIZE 112
		if( surface->w != MAPWINSIZE || surface->h != MAPWINSIZE) // resize map
		{ 
			SDL_Surface *scaledMap = scaleSurface ( surface, NULL, MAPWINSIZE, MAPWINSIZE );
			SDL_FreeSurface ( surface );
			surface = scaledMap;
		}
		mapImage->setImage ( surface );
		SDL_FreeSurface ( surface );
	}

	string mapName = gameDataContainer.map->MapName;
	size = gameDataContainer.map->size;

	if ( font->getTextWide ( ">" + mapName.substr( 0, mapName.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
	{
		while ( font->getTextWide ( ">" + mapName + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")<" ) > 140 )
		{
			mapName.erase ( mapName.length()-1, mapName.length() );
		}
		mapName = mapName + "... (" + iToStr ( size ) + "x" + iToStr ( size ) + ")";
	}
	else mapName = mapName.substr( 0, mapName.length()-4 ) + " (" + iToStr ( size ) + "x" + iToStr ( size ) + ")";

	mapLabel->setText ( mapName );
}

void cNetworkMenu::setColor( int color )
{
	SDL_Rect src = { 0, 0, 83, 10 };
	SDL_Surface *colorSurface = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_BlitSurface ( OtherData.colors[color], &src, colorSurface, NULL );
	colorImage->setImage ( colorSurface );
	SDL_FreeSurface ( colorSurface );
}


void cNetworkMenu::saveOptions()
{
	SettingsData.sPlayerName = actPlayer->name;
	SettingsData.iPort = port;
	SettingsData.iColor = actPlayer->color;
	SaveOption ( SAVETYPE_NAME );
	SaveOption ( SAVETYPE_PORT );
	SaveOption ( SAVETYPE_COLOR );
	if ( ip.compare ( "-" ) != 0 )
	{
		SettingsData.sIP = ip;
		SaveOption ( SAVETYPE_IP );
	}
}

void cNetworkMenu::playerReadyClicked ( sMenuPlayer *player )
{
	if ( player != actPlayer ) return;
	player->ready = !player->ready;
	playerSettingsChanged();
}

void cNetworkMenu::backReleased( void* parent )
{
	cNetworkMenu *menu = static_cast<cNetworkMenu*>((cMenu*)parent);
	menu->saveOptions();
	menu->terminate = true;
}

void cNetworkMenu::sendReleased( void* parent )
{
	cNetworkMenu *menu = static_cast<cNetworkMenu*>((cMenu*)parent);
	string chatText = menu->chatLine->getText();
	if ( !chatText.empty() )
	{
		chatText = menu->nameLine->getText() + ": " + chatText;
		menu->chatLine->setText ( "" );
		menu->chatBox->addLine ( chatText );
		menu->draw();
		sendMenuChatMessage ( chatText, NULL, menu->actPlayer->nr );
	}
}

void cNetworkMenu::nextColorReleased( void* parent )
{
	cNetworkMenu *menu = static_cast<cNetworkMenu*>((cMenu*)parent);
	menu->actPlayer->color++;
	if ( menu->actPlayer->color >= PLAYERCOLORS ) menu->actPlayer->color = 0;
	menu->setColor ( menu->actPlayer->color );
	menu->playerSettingsChanged();
	menu->draw();
}

void cNetworkMenu::prevColorReleased( void* parent )
{
	cNetworkMenu *menu = static_cast<cNetworkMenu*>((cMenu*)parent);
	menu->actPlayer->color--;
	if ( menu->actPlayer->color < 0 ) menu->actPlayer->color = PLAYERCOLORS-1;
	menu->setColor ( menu->actPlayer->color );
	menu->playerSettingsChanged();
	menu->draw();
}

void cNetworkMenu::wasNameImput( void* parent )
{
	cNetworkMenu *menu = static_cast<cNetworkMenu*>((cMenu*)parent);
	menu->actPlayer->name = menu->nameLine->getText();
	menu->playerSettingsChanged();
}

void cNetworkMenu::portIpChanged( void* parent )
{
	cNetworkMenu *menu = static_cast<cNetworkMenu*>((cMenu*)parent);
	menu->port = atoi ( menu->portLine->getText().c_str() );
	if ( menu->ipLine->getText().compare ( "-" ) != 0 ) menu->ip = menu->ipLine->getText();
}

cNetworkHostMenu::cNetworkHostMenu()
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+11, lngPack.i18n ("Text~Button~TCPIP_Host") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	okButton = new cMenuButton ( position.x+390, position.y+450, lngPack.i18n ( "Text~Button~OK" ) );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );

	mapButton = new cMenuButton ( position.x+470, position.y+42, lngPack.i18n ( "Text~Title~Choose_Planet" ), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	mapButton->setReleasedFunction ( &mapReleased );
	menuItems.Add ( mapButton );

	settingsButton = new cMenuButton ( position.x+470, position.y+77, lngPack.i18n ( "Text~Title~Options" ), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	settingsButton->setReleasedFunction ( &settingsReleased );
	menuItems.Add ( settingsButton );

	loadButton = new cMenuButton ( position.x+470, position.y+120, lngPack.i18n ( "Text~Button~Game_Load" ), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	loadButton->setReleasedFunction ( &loadReleased );
	menuItems.Add ( loadButton );

	startButton = new cMenuButton ( position.x+470, position.y+200, lngPack.i18n ( "Text~Button~Host_Start" ), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	startButton->setReleasedFunction ( &startReleased );
	menuItems.Add ( startButton );

	ipLine->setText ( "-" );
	ipLine->setReadOnly ( true );
}

cNetworkHostMenu::~cNetworkHostMenu()
{
	delete titleLabel;

	delete okButton;

	delete mapButton;
	delete settingsButton;
	delete loadButton;
	delete startButton;
}

int cNetworkHostMenu::checkAllPlayersReady()
{
	for ( unsigned int i = 0; i < players.Size(); i++ )
	{
		if ( !players[i]->ready ) return i;
	}
	return -1;
}

void cNetworkHostMenu::okReleased( void* parent )
{
	cNetworkHostMenu *menu = static_cast<cNetworkHostMenu*>((cMenu*)parent);

	int playerNr;
	if ( ( !menu->gameDataContainer.settings || !menu->gameDataContainer.map ) && menu->gameDataContainer.savegame.empty() )
	{
		menu->chatBox->addLine ( lngPack.i18n("Text~Multiplayer~Missing_Settings") );
		menu->draw();
		return;
	}
	else if ( ( playerNr = menu->checkAllPlayersReady() ) != -1 )
	{
		menu->chatBox->addLine ( menu->players[playerNr]->name + " " + lngPack.i18n("Text~Multiplayer~Not_Ready") );
		menu->draw();
		return;
	}
	menu->saveOptions ();
	if( !menu->gameDataContainer.savegame.empty() )
	{
		ActiveMenu = NULL;
		if ( menu->runSavedGame() == false )
		{
			ActiveMenu = menu;
			return;
		}
	}
	else
	{
		sendGo ();

		for ( unsigned int i = 0; i < menu->players.Size(); i++ )
		{
			cPlayer *player = new cPlayer ( menu->players[i]->name, OtherData.colors[menu->players[i]->color], menu->players[i]->nr, menu->players[i]->socket );
			menu->gameDataContainer.players.Add ( player );
		}

		bool started = false;
		while ( !started )
		{
			if (menu->gameDataContainer.settings->clans == SETTING_CLANS_ON)
			{
				cClanSelectionMenu clanMenu (&menu->gameDataContainer, menu->gameDataContainer.players[0], true);
				clanMenu.show ();
			}
			
			cStartupHangarMenu hangarMenu( &menu->gameDataContainer, menu->gameDataContainer.players[0], menu->gameDataContainer.settings->clans == SETTING_CLANS_OFF ) ;
			if ( hangarMenu.show() == 0 ) started = true;
		}
	}
	menu->end = true;
}

void cNetworkHostMenu::mapReleased( void* parent )
{
	cNetworkHostMenu *menu = static_cast<cNetworkHostMenu*>((cMenu*)parent);
	cPlanetsSelectionMenu planetsSelectionMenu ( &menu->gameDataContainer );
	planetsSelectionMenu.show();
	menu->showSettingsText();
	menu->showMap();
	sendGameData ( &menu->gameDataContainer, menu->saveGameString );
	menu->draw();
}

void cNetworkHostMenu::settingsReleased( void* parent )
{
	cNetworkHostMenu *menu = static_cast<cNetworkHostMenu*>((cMenu*)parent);
	cSettingsMenu settingsMenu ( &menu->gameDataContainer );
	settingsMenu.show();
	menu->showSettingsText();
	sendGameData ( &menu->gameDataContainer, menu->saveGameString );
	menu->draw();
}

void cNetworkHostMenu::loadReleased( void* parent )
{
	cNetworkHostMenu *menu = static_cast<cNetworkHostMenu*>((cMenu*)parent);
	cLoadMenu loadMenu ( &menu->gameDataContainer );
	loadMenu.show();
	if ( !menu->gameDataContainer.savegame.empty() )
	{
		delete menu->gameDataContainer.settings;
		menu->gameDataContainer.settings = NULL;
		cSavegame savegame ( menu->gameDataContainer.savegameNum );
		savegame.loadHeader ( &menu->saveGameString, NULL, NULL );
		menu->saveGameString += "\n\n" + lngPack.i18n ( "Text~Title~Players" ) +"\n" + savegame.getPlayerNames();

		if ( menu->gameDataContainer.map ) delete menu->gameDataContainer.map;
		cMap *map = new cMap;
		map->LoadMap ( savegame.getMapName() );
		menu->gameDataContainer.map = map;

		sendGameData ( &menu->gameDataContainer, menu->saveGameString );
		menu->showSettingsText();
		menu->showMap();
	}
	menu->draw();
}

void cNetworkHostMenu::startReleased( void* parent )
{
	cNetworkHostMenu *menu = static_cast<cNetworkHostMenu*>((cMenu*)parent);
	if ( network->getConnectionStatus() == 0 ) // Connect only if there isn't a connection jet
	{
		network->setPort( menu->port );

		if ( network->create() == -1)
		{
			menu->chatBox->addLine ( lngPack.i18n("Text~Multiplayer~Network_Error_Socket") );
			Log.write("Error opening socket", cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			menu->chatBox->addLine ( lngPack.i18n("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n("Text~Title~Port") + ": "  + iToStr( menu->port ) + ")" );
			Log.write("Game open (Port: " + iToStr( menu->port) + ")", cLog::eLOG_TYPE_INFO);
			menu->portLine->setReadOnly( true );
		}
		menu->draw();
	}
}

void cNetworkHostMenu::playerSettingsChanged ()
{
	sendPlayerList ( &players );
}

void cNetworkHostMenu::handleNetMessage( cNetMessage *message )
{
	Log.write("Menu: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );
	switch ( message->iType )
	{
	case MU_MSG_CHAT:
		{
			string chatText = message->popString();
			chatBox->addLine ( chatText );
			for ( unsigned int i = 1; i < players.Size(); i++ )
			{
				if ( players[i]->nr == message->iPlayerNr ) continue;
				sendMenuChatMessage ( chatText, players[i] );
			}
			draw();
		}
		break;
	case MU_MSG_NEW_PLAYER:
		{
			sMenuPlayer *player = new sMenuPlayer ( "unidentified", 0, false, (int)players.Size(), message->popInt16() );
			players.Add ( player );
			sendRequestIdentification ( player );
			draw();
		}
		break;
	case MU_MSG_DEL_PLAYER:
		{
			int socket = message->popInt16();
			string playerName;

			//delete player
			for ( unsigned int i = 0; i < players.Size(); i++ )
			{
				if ( players[i]->socket == socket )
				{
					playerName = players[i]->name;
					players.Delete ( i );
				}
			}

			//resort socket numbers
			for ( unsigned int playerNr = 0; playerNr < players.Size(); playerNr++ )
			{
				if ( players[playerNr]->socket > socket && players[playerNr]->socket < MAX_CLIENTS ) players[playerNr]->socket--;
			}

			//resort player numbers
			for ( unsigned int i = 0; i < players.Size(); i++ )
			{
				players[i]->nr = i;
				sendRequestIdentification ( players[i] );
			}

			chatBox->addLine ( lngPack.i18n ( "Text~Multiplayer~Player_Left", playerName ) );

			draw();
			sendPlayerList( &players );
		}
		break;
	case MU_MSG_IDENTIFIKATION:
		{
			int playerNr = message->popInt16();

			bool freshJoined = ( players[playerNr]->name.compare ( "unidentified" ) == 0 );
			players[playerNr]->color = message->popInt16();
			players[playerNr]->name = message->popString();
			players[playerNr]->ready = message->popBool();

			if ( freshJoined ) chatBox->addLine ( lngPack.i18n ( "Text~Multiplayer~Player_Joined", players[playerNr]->name ) );
			
			draw();
			sendPlayerList( &players );
			sendGameData( &gameDataContainer, saveGameString, players[playerNr] );
		}
		break;
	}
}

bool cNetworkHostMenu::runSavedGame()
{
	cSavegame savegame ( gameDataContainer.savegameNum );
	if ( savegame.load() != 1 ) return false;
	// romve all players that do not belong to the save
	for ( unsigned int i = 0; i < players.Size(); i++ )
	{
		unsigned int j;
		for ( j = 0; j < Server->PlayerList->Size(); j++ )
		{
			if ( players[i]->name == (*Server->PlayerList)[j]->name ) 
				break;
		}
		// the player isn't in the list when the loop has gone trough all players and no match was found
		if ( j == Server->PlayerList->Size() )
		{
			network->close ( players[i]->socket );
			for ( unsigned int k = 0; k < players.Size(); k++ )
			{
				if ( players[k]->socket > players[i]->socket && players[k]->socket < MAX_CLIENTS ) players[k]->socket--;
			}
			delete players[i];
			players.Delete ( i );
		}
	}
	// set sockets
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		if ( (*Server->PlayerList)[i]->name == actPlayer->name ) (*Server->PlayerList)[i]->iSocketNum = MAX_CLIENTS;
		else
		{
			for ( unsigned int j = 0; j < players.Size(); j++ )
			{
				if ( (*Server->PlayerList)[i]->name == players[j]->name )
				{
					(*Server->PlayerList)[i]->iSocketNum = players[j]->socket;
					break;
				}
				// stop when a player is missing
				if ( j == players.Size()-1 )
				{
					chatBox->addLine ( lngPack.i18n ( "Text~Multiplayer~Player_Wrong" ) );
					draw();
					return false;
				}
			}
		}
	}
	sendPlayerList ( &players );
	// send client that the game has to be started
	sendGo();

	// copy map for client
	cMap clientMap;
	clientMap.LoadMap ( Server->Map->MapName );

	cList<cPlayer*> clientPlayerList;

	// copy players for client
	cPlayer *localPlayer;
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		cPlayer* addedPlayer = new cPlayer( *(*Server->PlayerList)[i] );
		clientPlayerList.Add( addedPlayer );
		if ( (*Server->PlayerList)[i]->iSocketNum == MAX_CLIENTS ) localPlayer = clientPlayerList[i];
		// reinit unit values
		for ( unsigned int j = 0; j < UnitsData.getNrVehicles (); j++) clientPlayerList[i]->VehicleData[j] = UnitsData.getVehicle (j, addedPlayer->getClan ()).data;
		for ( unsigned int j = 0; j < UnitsData.getNrBuildings (); j++) clientPlayerList[i]->BuildingData[j] = UnitsData.getBuilding (j, addedPlayer->getClan ()).data;
	}
	// init client and his player
	Client = new cClient( &clientMap, &clientPlayerList );
	Client->isInMenu = true;
	Client->initPlayer ( localPlayer );
	for ( unsigned int i = 0; i < clientPlayerList.Size(); i++ )
	{
		clientPlayerList[i]->InitMaps( clientMap.size, &clientMap );
	}

	// send data to all players
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		sendRequestResync( (*Server->PlayerList)[i]->Nr );
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

	return true;
}

cNetworkClientMenu::cNetworkClientMenu()
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+11, lngPack.i18n ("Text~Button~TCPIP_Client") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	connectButton = new cMenuButton ( position.x+470, position.y+200, lngPack.i18n ( "Text~Title~Connect" ), cMenuButton::BUTTON_TYPE_STANDARD_SMALL );
	connectButton->setReleasedFunction ( &connectReleased );
	menuItems.Add ( connectButton );

	ipLine->setText ( ip );
}

cNetworkClientMenu::~cNetworkClientMenu()
{
	delete titleLabel;
	delete connectButton;
}

void cNetworkClientMenu::connectReleased( void* parent )
{
	cNetworkClientMenu *menu = static_cast<cNetworkClientMenu*>((cMenu*)parent);

	if ( network->getConnectionStatus() == 0 ) // Connect only if there isn't a connection jet
	{
		network->setPort( menu->port );
		network->setIP( menu->ip );

		menu->chatBox->addLine ( lngPack.i18n("Text~Multiplayer~Network_Connecting") + menu->ip + ":" + iToStr( menu->port ) ); // e.g. Connecting to 127.0.0.1:55800
		Log.write(("Connecting to " + menu->ip + ":" + iToStr( menu->port )), cLog::eLOG_TYPE_INFO);

		if ( network->connect() == -1 )
		{
			menu->chatBox->addLine ( lngPack.i18n("Text~Multiplayer~Network_Error_Connect") + menu->ip + ":" + iToStr( menu->port ) );
			Log.write("Error on connecting " + menu->ip + ":" + iToStr( menu->port ), cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			menu->chatBox->addLine ( lngPack.i18n("Text~Multiplayer~Network_Connected") );
			Log.write("Connected", cLog::eLOG_TYPE_INFO);
			menu->portLine->setReadOnly( true );
			menu->ipLine->setReadOnly( true );
		}
		menu->draw();
	}
}

void cNetworkClientMenu::playerSettingsChanged ()
{
	sendIdentification ( actPlayer );
}

void cNetworkClientMenu::handleNetMessage( cNetMessage *message )
{
	Log.write("Menu: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );

	switch ( message->iType )
	{
	case MU_MSG_CHAT:
		{
			string chatText = message->popString();
			chatBox->addLine ( chatText );
			draw();
		}
		break;
	case MU_MSG_DEL_PLAYER:
		{
			for ( unsigned int i = 0; i < players.Size(); i++ )
			{
				if ( players[i]->nr == actPlayer->nr ) continue;
				players.Delete ( i );
			}
			actPlayer->ready = false;
			chatBox->addLine ( lngPack.i18n ( "Text~Multiplayer~Lost_Connection", "server" ) );
			draw();
		}
		break;
	case MU_MSG_REQ_IDENTIFIKATION:
		actPlayer->nr = message->popInt16();
		sendIdentification ( actPlayer );
		break;
	case MU_MSG_PLAYERLIST:
		{
			int playerCount = message->popInt16();
			int actPlayerNr = actPlayer->nr;
			while ( players.Size() > 0 )
			{
				delete players[0];
				players.Delete ( 0 );
			}
			for ( int i = 0; i < playerCount; i++ )
			{
				string name = message->popString();
				int color = message->popInt16();
				bool ready = message->popBool();
				int nr = message->popInt16();
				sMenuPlayer *player = new sMenuPlayer ( name, color, ready, nr );
				if ( player->nr == actPlayerNr ) actPlayer = player;
				players.Add ( player );
			}
			draw();
		}
		break;
	case MU_MSG_OPTINS:
		{
			if ( message->popBool() )
			{
				if ( !gameDataContainer.settings ) gameDataContainer.settings = new sSettings;
				sSettings *settings = gameDataContainer.settings;

				settings->metal = (eSettingResourceValue)message->popChar();
				settings->oil = (eSettingResourceValue)message->popChar();
				settings->gold = (eSettingResourceValue)message->popChar();
				settings->resFrequency = (eSettingResFrequency)message->popChar();
				settings->credits = (eSettingsCredits)message->popInt16();
				settings->bridgeHead = (eSettingsBridgeHead)message->popChar();
				settings->alienTech = (eSettingsAlienTech)message->popChar();
				settings->clans = (eSettingsClans)message->popChar();
				settings->gameType = (eSettingsGameType)message->popChar();
			}
			else if ( gameDataContainer.settings )
			{
				delete gameDataContainer.settings;
				gameDataContainer.settings = NULL;
			}

			if ( message->popBool() )
			{
				string mapName = message->popString();
				if ( !gameDataContainer.map || gameDataContainer.map->MapName != mapName )
				{
					if ( gameDataContainer.map ) delete gameDataContainer.map;
					cMap *map = new cMap;
					if ( map->LoadMap ( mapName ) ) gameDataContainer.map = map;
					else
					{
						delete map;
						gameDataContainer.map = NULL;
					}
					showMap();
				}
			}
			else if ( gameDataContainer.map )
			{
				delete gameDataContainer.map;
				gameDataContainer.map = NULL;
				showMap();
			}

			if ( message->popBool() ) saveGameString = message->popString();
			else saveGameString = "";

			showSettingsText();
			draw();
		}
		break;
	case MU_MSG_GO:
		{
			saveOptions ();
			for ( unsigned int i = 0; i < players.Size(); i++ )
			{
				cPlayer *player = new cPlayer ( players[i]->name, OtherData.colors[players[i]->color], players[i]->nr, players[i]->socket );
				gameDataContainer.players.Add ( player );
			}
			if ( !saveGameString.empty() )
			{
				ActiveMenu = NULL;
				gameDataContainer.runGame ( actPlayer->nr );
			}
			else
			{
				bool started = false;
				while ( !started )
				{
					if (gameDataContainer.settings->clans == SETTING_CLANS_ON)
					{
						cClanSelectionMenu clanMenu (&gameDataContainer, gameDataContainer.players[actPlayer->nr], true);
						clanMenu.show ();
					}
					
					cStartupHangarMenu hangarMenu( &gameDataContainer, gameDataContainer.players[actPlayer->nr], gameDataContainer.settings->clans == SETTING_CLANS_OFF ) ;
					if ( hangarMenu.show() == 0 ) started = true;
				}
			}
			end = true;
		}
		break;
	case GAME_EV_REQ_IDENT:
		{
			cDialogYesNow yesNoDialog ( lngPack.i18n( "Text~Comp~End_Game") );
			if ( yesNoDialog.show() == 0  ) sendGameIdentification ( actPlayer, message->popInt16() );
			else draw();
		}
		break;
	case GAME_EV_OK_RECONNECT:
		{
			actPlayer->nr = message->popInt16();
			actPlayer->color = message->popInt16();
			cMap *Map = new cMap;
			if ( !Map->LoadMap ( message->popString() ) ) break;
			gameDataContainer.map = Map;

			int playerCount = message->popInt16();

			gameDataContainer.players.Add ( new cPlayer ( actPlayer->name, OtherData.colors[actPlayer->color], actPlayer->nr ) );
			while ( playerCount > 1 )
			{
				string playername = message->popString();
				int playercolor = message->popInt16();
				int playernr = message->popInt16();
				gameDataContainer.players.Add ( new cPlayer ( playername, OtherData.colors[playercolor], playernr ) );
				playerCount--;
			}

			bool changed = false;
			int size = (int)gameDataContainer.players.Size();
			do
			{
				changed = false;
				for ( int i = 0; i < size-1; i++ )
				{
					if ( gameDataContainer.players[i]->Nr > gameDataContainer.players[i+1]->Nr )
					{
						cPlayer *temp = gameDataContainer.players[i+1];
						gameDataContainer.players[i+1] = gameDataContainer.players[i];
						gameDataContainer.players[i] = temp;
						changed = true;
					}
				}
				size--;
			}
			while ( changed && size );

			ActiveMenu = NULL;
			gameDataContainer.runGame ( actPlayer->nr, true );
			end = true;
		}
		break;
	}
}

cLoadMenu::cLoadMenu( cGameDataContainer *gameDataContainer_, eMenuBackgrounds backgroundType_ ) : cMenu ( LoadPCX ( GFXOD_SAVELOAD ), backgroundType_ ), gameDataContainer ( gameDataContainer_ )
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+12, lngPack.i18n ("Text~Title~Load") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	backButton = new cMenuButton ( position.x+353, position.y+438, lngPack.i18n ( "Text~Button~Back" ), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton );
	backButton->setReleasedFunction ( &backReleased );
	menuItems.Add ( backButton );

	loadButton = new cMenuButton ( position.x+514, position.y+438, lngPack.i18n ( "Text~Button~Load" ), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton );
	loadButton->setReleasedFunction ( &loadReleased );
	menuItems.Add ( loadButton );

	upButton = new cMenuButton ( position.x+33, position.y+438, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG );
	upButton->setReleasedFunction ( &upReleased );
	menuItems.Add ( upButton );

	downButton = new cMenuButton ( position.x+63, position.y+438, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG );
	downButton->setReleasedFunction ( &downReleased );
	menuItems.Add ( downButton );

	for ( int x = 0; x < 2; x++ )
	{
		for ( int y = 0; y < 5; y++ )
		{
			saveSlots[5*x+y] = new cMenuSaveSlot ( position.x+17+402*x, position.y+45+76*y, this );
			saveSlots[5*x+y]->setReleasedFunction ( &slotClicked );
			saveSlots[5*x+y]->setClickSound ( SoundData.SNDObjectMenu );
			menuItems.Add ( saveSlots[5*x+y] );
		}
	}

	offset = 0;
	selected = -1;

	files = getFilesOfDirectory ( SettingsData.sSavesPath );

	loadSaves();
	displaySaves();
}

cLoadMenu::~cLoadMenu()
{
	delete titleLabel;

	delete backButton;
	delete loadButton;

	delete upButton;
	delete downButton;

	for ( int i = 0; i < 10; i++ )
	{
		delete saveSlots[i];
	}

	delete files;
	while ( savefiles.Size() )
	{
		delete savefiles[0];
		savefiles.Delete ( 0 );
	}
}

void cLoadMenu::loadSaves()
{
	for ( unsigned int i = 0; i < files->Size(); i++ )
	{
		// only check for xml files and numbers for this offset
		string const& file = (*files)[i];
		if ( file.length() < 4 || file.substr( file.length() - 3, 3 ).compare( "xml" ) != 0 )
		{
			files->Delete ( i );
			i--;
			continue;
		}
		int number;
		if ( file.length() < 8 || ( number = atoi( file.substr( file.length() - 7, 3 ).c_str() ) ) < offset || number > offset+10 ) continue;
		// don't add files twice
		bool found = false;
		for ( unsigned int j = 0; j < savefiles.Size(); j++ )
		{
			if ( savefiles[j]->number == number )
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
		savefiles.Add ( savefile );
	}
}

void cLoadMenu::displaySaves()
{
	for ( int x = 0; x < 2; x++ )
	{
		for ( int y = 0; y < 5; y++ )
		{
			int filenum = x*5+y;
			cMenuSaveSlot *slot = saveSlots[filenum];
			sSaveFile *savefile = NULL;
			for ( unsigned int i = 0; i < savefiles.Size(); i++ )
			{
				if ( savefiles[i]->number == offset+filenum+1 )
				{
					savefile = savefiles[i];
				}
			}
			if ( savefile )
			{
				slot->setSaveData ( *savefile, selected == filenum+offset );
			}
			else slot->reset( offset+filenum+1, selected == filenum+offset );
		}
	}
}

void cLoadMenu::backReleased( void* parent )
{
	cLoadMenu *menu = static_cast<cLoadMenu*>((cMenu*)parent);
	menu->terminate = true;
}

void cLoadMenu::loadReleased( void* parent )
{
	cLoadMenu *menu = static_cast<cLoadMenu*>((cMenu*)parent);
	sSaveFile *savefile = NULL;
	for ( unsigned int i = 0; i < menu->savefiles.Size(); i++ )
	{
		if ( menu->savefiles[i]->number == menu->selected+1 )
		{
			savefile = menu->savefiles[i];
		}
	}
	if ( savefile )
	{
		menu->gameDataContainer->savegame = savefile->filename;
		menu->gameDataContainer->savegameNum = menu->selected+1;
	}
	menu->end = true;
}

void cLoadMenu::upReleased( void* parent )
{
	cLoadMenu *menu = static_cast<cLoadMenu*>((cMenu*)parent);
	if ( menu->offset > 0 )
	{
		menu->offset -= 10;
		menu->loadSaves();
		menu->displaySaves();
		menu->draw();
	}
}

void cLoadMenu::downReleased( void* parent )
{
	cLoadMenu *menu = static_cast<cLoadMenu*>((cMenu*)parent);
	if ( menu->offset < 90 )
	{
		menu->offset += 10;
		menu->loadSaves();
		menu->displaySaves();
		menu->draw();
	}
}


void cLoadMenu::slotClicked( void* parent )
{
	cLoadMenu *menu = static_cast<cLoadMenu*>((cMenu*)parent);
	int x = mouse->x > menu->position.x+menu->position.w/2 ? 1 : 0;
	int y = 0;
	if ( mouse->y > menu->position.y+118 ) y++;
	if ( mouse->y > menu->position.y+118+77 ) y++;
	if ( mouse->y > menu->position.y+118+77*2 ) y++;
	if ( mouse->y > menu->position.y+118+77*3 ) y++;

	int oldSelection = menu->selected;
	menu->selected = 5*x+y+menu->offset;
	menu->displaySaves();
	menu->extendedSlotClicked( oldSelection );
	menu->draw();
	
	if (mouse->isDoubleClick)
	{
		loadReleased( parent );	
	}
}


cLoadSaveMenu::cLoadSaveMenu( cGameDataContainer *gameDataContainer_ ) : cLoadMenu ( gameDataContainer_, MNU_BG_ALPHA )
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+12, lngPack.i18n ("Text~Title~Load") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	exitButton = new cMenuButton ( position.x+246, position.y+438, lngPack.i18n ( "Text~Button~Exit" ), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton );
	exitButton->setReleasedFunction ( &exitReleased );
	menuItems.Add ( exitButton );

	saveButton = new cMenuButton ( position.x+132, position.y+438, lngPack.i18n ( "Text~Button~Save" ), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton );
	saveButton->setReleasedFunction ( &saveReleased );
	menuItems.Add ( saveButton );

	loadButton->setLocked ( true );
}

cLoadSaveMenu::~cLoadSaveMenu()
{
	delete exitButton;
	delete saveButton;
}

void cLoadSaveMenu::exitReleased( void* parent )
{
	cLoadSaveMenu *menu = static_cast<cLoadSaveMenu*>((cMenu*)parent);
	menu->end = true;
}

void cLoadSaveMenu::saveReleased( void* parent )
{
	cLoadSaveMenu *menu = static_cast<cLoadSaveMenu*>((cMenu*)parent);
	if (  menu->selected < 0 ||  menu->selected > 99 ) return;
	if ( !Server )
	{
		cDialogOK okDialog( lngPack.i18n ( "Text~Multiplayer~Save_Only_Host" ) );
		okDialog.show();
		menu->draw();
		return;
	}

	cSavegame savegame( menu->selected+1 );
	savegame.save ( menu->saveSlots[menu->selected-menu->offset]->getNameEdit()->getText() );

	delete menu->files;
	menu->files = getFilesOfDirectory ( SettingsData.sSavesPath );
	for ( unsigned int i = 0; i < menu->savefiles.Size(); i++ )
	{
		if ( menu->savefiles[i]->number == menu->selected+1 )
		{
			delete menu->savefiles[i];
			menu->savefiles.Delete ( i );
			break;
		}
	}
	menu->loadSaves();
	menu->displaySaves();
	menu->draw();
}

void cLoadSaveMenu::extendedSlotClicked( int oldSelection )
{
	if ( oldSelection != -1 && oldSelection-offset >= 0 && oldSelection-offset < 10)
	{
		saveSlots[oldSelection-offset]->setActivity ( false );
		saveSlots[oldSelection-offset]->getNameEdit()->setActivity ( false );
		saveSlots[oldSelection-offset]->getNameEdit()->setReadOnly ( true );
	}
	saveSlots[selected-offset]->setActivity ( true );
	saveSlots[selected-offset]->getNameEdit()->setActivity ( true );
	activeItem = saveSlots[selected-offset]->getNameEdit();
	saveSlots[selected-offset]->getNameEdit()->setReadOnly ( false );
}

cBuildingsBuildMenu::cBuildingsBuildMenu ( cPlayer *player_, cVehicle *vehicle_ ) : cHangarMenu ( LoadPCX ( GFXOD_BUILD_SCREEN ), player_, MNU_BG_ALPHA )
{
	if ( !Client ) terminate = true;

	vehicle = vehicle_;

	titleLabel = new cMenuLabel ( position.x+405, position.y+11, lngPack.i18n ("Text~Title~Build") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	doneButton->move ( position.x+387, position.y+452 );
	doneButton->setReleasedFunction ( &doneReleased );
	backButton->move ( position.x+300, position.y+452 );
	backButton->setReleasedFunction ( &backReleased );

	selectionList->move ( position.x+477,  position.y+48 );
	selectionList->resize ( 154, 390 );
	selListUpButton->move ( position.x+471, position.y+440 );
	selListDownButton->move ( position.x+491, position.y+440 );
	selectionList->setDoubleClickedFunction ( &selListDoubleClicked );

	if ( vehicle->data.canBuildPath )
	{
		pathButton = new cMenuButton ( position.x+338, position.y+428, lngPack.i18n ( "Text~Button~Path" ), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
		pathButton->setReleasedFunction ( &pathReleased );
		menuItems.Add ( pathButton );
	}

	speedCurHandler = new cMenuBuildSpeedHandler ( position.x+292, position.y+345 );
	menuItems.Add ( speedCurHandler );

	selectionChangedFunc = &selectionChanged;

	generateSelectionList();

	selectionChanged ( this );
}

cBuildingsBuildMenu::~cBuildingsBuildMenu()
{
	delete titleLabel;

	if ( vehicle->data.canBuildPath ) delete pathButton;

	delete speedCurHandler;

	if ( Client ) Client->bFlagDrawHud = true;
}

void cBuildingsBuildMenu::generateSelectionList()
{
	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
	{
		if ( UnitsData.building[i].data.explodesOnContact )continue;

		if ( vehicle->data.canBuild.compare ( UnitsData.building[i].data.buildAs ) != 0 ) continue;

		selectionList->addUnit ( UnitsData.building[i].data.ID, player );

		if ( vehicle->data.storageResCur < player->BuildingData[i].buildCosts) selectionList->getItem ( selectionList->getSize()-1 )->setMarked ( true );
	}

	if ( selectionList->getSize() > 0 ) selectionList->setSelection ( selectionList->getItem ( 0 ) );
}

void cBuildingsBuildMenu::doneReleased ( void *parent )
{
	cBuildingsBuildMenu *menu = static_cast<cBuildingsBuildMenu*>((cMenu*)parent);
	if ( !menu->selectedUnit->getUnitData()->isBig )
	{
		sendWantBuild( menu->vehicle->iID, menu->selectedUnit->getUnitID(), menu->speedCurHandler->getBuildSpeed(), menu->vehicle->PosX + menu->vehicle->PosY * Client->Map->size, false, 0 );
	}
	else
	{
		menu->vehicle->PlaceBand = true;
		menu->vehicle->BuildBigSavedPos = menu->vehicle->PosX + menu->vehicle->PosY * Client->Map->size;

		// save building information temporary to have them when placing band is finished
		menu->vehicle->BuildingTyp = menu->selectedUnit->getUnitID();
		menu->vehicle->BuildRounds = menu->speedCurHandler->getBuildSpeed();

		menu->vehicle->FindNextband();
	}
	menu->end = true;
}

void cBuildingsBuildMenu::pathReleased ( void *parent )
{
	cBuildingsBuildMenu *menu = static_cast<cBuildingsBuildMenu*>((cMenu*)parent);

	menu->vehicle->BuildingTyp = menu->selectedUnit->getUnitID();
	menu->vehicle->BuildRounds = menu->speedCurHandler->getBuildSpeed();

	menu->vehicle->PlaceBand = true;
	menu->end = true;
}

void cBuildingsBuildMenu::backReleased ( void *parent )
{
	cBuildingsBuildMenu *menu = static_cast<cBuildingsBuildMenu*>((cMenu*)parent);
	menu->terminate = true;
}

void cBuildingsBuildMenu::selectionChanged ( void *parent )
{
	cBuildingsBuildMenu *menu = dynamic_cast<cBuildingsBuildMenu*>((cHangarMenu*)parent);
	if ( !menu ) menu = dynamic_cast<cBuildingsBuildMenu*>((cBuildingsBuildMenu*)parent);
	if ( !menu->selectedUnit ) return;

	sUnitData *buildingData = menu->selectedUnit->getUnitID().getUnitDataCurrentVersion ( menu->player );
	int turboBuildTurns[3], turboBuildCosts[3];
	menu->vehicle->calcTurboBuild ( turboBuildTurns, turboBuildCosts, buildingData->buildCosts );

	if ( turboBuildTurns[0] == 0 )
	{
		turboBuildCosts[0] = buildingData->buildCosts;
		turboBuildTurns[0] = buildingData->buildCosts / menu->vehicle->data.needsMetal;
	}
	menu->speedCurHandler->setValues ( turboBuildTurns, turboBuildCosts );
}

bool cBuildingsBuildMenu::selListDoubleClicked ( cMenuUnitsList* list, void *parent )
{
	cBuildingsBuildMenu *menu = static_cast<cBuildingsBuildMenu*>((cHangarMenu*)parent);
	menu->doneReleased( menu );
	return true;
}

cVehiclesBuildMenu::cVehiclesBuildMenu ( cPlayer *player_, cBuilding *building_ ) : cHangarMenu ( LoadPCX ( GFXOD_FAC_BUILD_SCREEN ), player_, MNU_BG_ALPHA ), cAdvListHangarMenu ( NULL, player_ )
{
	if ( !Client ) terminate = true;

	building = building_;

	titleLabel = new cMenuLabel ( position.x+405, position.y+11, lngPack.i18n ("Text~Title~Build") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	doneButton->move ( position.x+387, position.y+452 );
	doneButton->setReleasedFunction ( &doneReleased );
	backButton->move ( position.x+300, position.y+452 );
	backButton->setReleasedFunction ( &backReleased );

	selectionList->move ( position.x+477,  position.y+48 );
	selectionList->resize ( 154, 390 );
	selListUpButton->move ( position.x+471, position.y+440 );
	selListDownButton->move ( position.x+491, position.y+440 );

	secondList->setDisplayType ( MUL_DIS_TYPE_NOEXTRA );
	secondList->move ( position.x+330, position.y+50 );
	secondList->resize ( 130, 232 );
	secondListUpButton->move ( position.x+327, position.y+293 );
	secondListDownButton->move ( position.x+348, position.y+293 );

	speedCurHandler = new cMenuBuildSpeedHandler ( position.x+292, position.y+345 );
	speedCurHandler->setBuildSpeed ( building->BuildSpeed );
	menuItems.Add ( speedCurHandler );

	repeatButton = new cMenuCheckButton ( position.x+447, position.y+322, lngPack.i18n ( "Text~Comp~Repeat" ), building->RepeatBuild, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD, cMenuCheckButton::TEXT_ORIENT_LEFT );
	menuItems.Add ( repeatButton );

	selectionChangedFunc = &selectionChanged;

	generateSelectionList();
	createBuildList();

	selectionChanged ( this );
}

cVehiclesBuildMenu::~cVehiclesBuildMenu()
{
	delete titleLabel;
	delete speedCurHandler;

	delete repeatButton;

	if ( Client ) Client->bFlagDrawHud = true;
}

void cVehiclesBuildMenu::generateSelectionList()
{
	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
	{
		const sVehicle& vehicle = UnitsData.getVehicle (i, player->getClan ());

		bool land = false, water = false;

		int x = building->PosX - 2, y = building->PosY - 1;

		for ( int j = 0; j < 12; j++ )
		{
			if ( j == 4 ||  j == 6 || j == 8 )
			{
				x -= 3;
				y += 1;
			}
			else if ( j == 5 || j == 7 ) x += 3;
			else x++;

			if ( x < 0 || x >= Client->Map->size || y < 0 || y >= Client->Map->size ) continue; 

			int off = x + y * Client->Map->size;
			cBuildingIterator bi = Client->Map->fields[off].getBuildings();
			while ( bi && ( bi->data.surfacePosition != sUnitData::SURFACE_POS_BASE || bi->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA || bi->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE ) ) bi++;

			if ( !Client->Map->IsWater ( off ) || ( bi && ( bi->data.surfacePosition == sUnitData::SURFACE_POS_BASE || bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE ) ) ) land = true;
			else if ( Client->Map->IsWater ( off ) && bi && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA )
			{
				land = true;
				water = true;
				break;
			}
			else if ( Client->Map->IsWater ( off ) ) water = true;
		}

		if ( vehicle.data.factorSea > 0 && vehicle.data.factorGround == 0 && !water ) continue;
		else if ( vehicle.data.factorGround > 0 && vehicle.data.factorSea == 0 && !land ) continue;

		if ( building->data.canBuild.compare ( vehicle.data.buildAs ) != 0 ) continue;

		selectionList->addUnit ( vehicle.data.ID, player, NULL, false, false );
		selectionList->getItem ( selectionList->getSize()-1 )->setResValue ( -1, false );
	}

	if ( selectionList->getSize() > 0 ) selectionList->setSelection ( selectionList->getItem ( 0 ) );
}

void cVehiclesBuildMenu::createBuildList()
{
	for ( unsigned int i = 0; i < building->BuildList->Size(); i++ )
	{
		secondList->addUnit ( (*building->BuildList)[i]->typ->data.ID, building->owner, NULL, true, false );
		secondList->getItem( secondList->getSize()-1 )->setResValue ( (*building->BuildList)[i]->metall_remaining, false );
	}
	if ( secondList->getSize() > 0 )
	{
		setSelectedUnit ( secondList->getItem ( 0 ) );
		secondList->setSelection ( secondList->getItem ( 0 ) );
	}
}

void cVehiclesBuildMenu::doneReleased ( void *parent )
{
	cVehiclesBuildMenu *menu = dynamic_cast<cVehiclesBuildMenu*>((cMenu*)parent);
	if ( !menu ) return;
	cList<sBuildList*> buildList;
	for ( int i = 0; i < menu->secondList->getSize(); i++ )
	{
		sBuildList *buildItem = new sBuildList;
		buildItem->typ = menu->secondList->getItem ( i )->getUnitID().getVehicle();
		buildItem->metall_remaining = menu->secondList->getItem ( i )->getResValue();
		buildList.Add ( buildItem );
	}
	menu->building->BuildSpeed = menu->speedCurHandler->getBuildSpeed();
	sendWantBuildList ( menu->building, buildList, menu->repeatButton->isChecked() );
	menu->end = true;
}

void cVehiclesBuildMenu::backReleased ( void *parent )
{
	cVehiclesBuildMenu *menu = dynamic_cast<cVehiclesBuildMenu*>((cMenu*)parent);
	if ( !menu ) return;
	menu->terminate = true;
}

void cVehiclesBuildMenu::selectionChanged ( void *parent )
{
	cVehiclesBuildMenu *menu = dynamic_cast<cVehiclesBuildMenu*>((cHangarMenu*)parent);
	if ( !menu ) menu = dynamic_cast<cVehiclesBuildMenu*>((cVehiclesBuildMenu*)parent);
	if ( !menu ) return;

	if ( !menu->selectedUnit ) return;

	sUnitData *vehicleData = menu->selectedUnit->getUnitID().getUnitDataCurrentVersion ( menu->player );
	int turboBuildTurns[3], turboBuildCosts[3];
	menu->building->CalcTurboBuild ( turboBuildTurns, turboBuildCosts, vehicleData->buildCosts, menu->selectedUnit->getResValue() );

	menu->speedCurHandler->setValues ( turboBuildTurns, turboBuildCosts );
}

cUpgradeHangarMenu::cUpgradeHangarMenu( cPlayer *owner ) : cHangarMenu ( LoadPCX ( GFXOD_UPGRADE ), owner )
{
	upgradeFilter = new cMenuUpgradeFilter ( position.x+467, position.y+411, this );
	upgradeFilter->setTankChecked (true);
	upgradeFilter->setPlaneChecked (true);
	upgradeFilter->setShipChecked (true);
	upgradeFilter->setBuildingChecked (true);
	menuItems.Add ( upgradeFilter );

	upgradeButtons = new cMenuUpgradeHandler ( position.x+283, position.y+293, this );
	menuItems.Add ( upgradeButtons );

	goldBar = new cMenuMaterialBar ( position.x+372, position.y+301, position.x+381, position.y+275, 0, cMenuMaterialBar::MAT_BAR_TYPE_GOLD );
	menuItems.Add ( goldBar );
	goldBarLabel = new cMenuLabel ( position.x+381, position.y+285, lngPack.i18n ("Text~Title~Gold") );
	goldBarLabel->setCentered( true );
	menuItems.Add ( goldBarLabel );

	initUpgrades ( owner );
}

cUpgradeHangarMenu::~cUpgradeHangarMenu()
{
	delete upgradeFilter;
	delete upgradeButtons;

	delete goldBar;
	delete goldBarLabel;

	delete[] unitUpgrades;
}
void cUpgradeHangarMenu::initUpgrades( cPlayer *player )
{
	unitUpgrades = new sUnitUpgrade[UnitsData.getNrVehicles () + UnitsData.getNrBuildings ()][8];
	for ( unsigned int unitIndex = 0; unitIndex < UnitsData.getNrVehicles () + UnitsData.getNrBuildings (); unitIndex++ )
	{
		sUnitData *data;
		sUnitData *oriData;
		if ( unitIndex < UnitsData.getNrVehicles () )
		{
			oriData = &UnitsData.getVehicle (unitIndex, player->getClan ()).data;
			data = &player->VehicleData[unitIndex];
		}
		else
		{
			oriData = &UnitsData.getBuilding (unitIndex - UnitsData.getNrVehicles (), player->getClan ()).data;
			data = &player->BuildingData[unitIndex - UnitsData.getNrVehicles ()];
		}

		cResearch& researchLevel = player->researchLevel;

		sUnitUpgrade *upgrade = unitUpgrades[unitIndex];
		int i = 0;

		if ( data->canAttack )
		{
			// Damage:
			upgrade[i].active = true;
			upgrade[i].startValue = oriData->damage;
			upgrade[i].curValue = data->damage;
			upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->damage, oriData->damage, cUpgradeCalculator::kAttack, researchLevel);
			upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_DAMAGE;
			i++;
			if ( !data->explodesOnContact )
			{
				// Shots:
				upgrade[i].active = true;
				upgrade[i].startValue = oriData->shotsMax;
				upgrade[i].curValue = data->shotsMax;
				upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->shotsMax, oriData->shotsMax, cUpgradeCalculator::kShots, researchLevel);
				upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_SHOTS;
				i++;
				// Range:
				upgrade[i].active = true;
				upgrade[i].startValue = oriData->range;
				upgrade[i].curValue = data->range;
				upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->range, oriData->range, cUpgradeCalculator::kRange, researchLevel);
				upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_RANGE;
				i++;
				// Ammo:
				upgrade[i].active = true;
				upgrade[i].startValue = oriData->ammoMax;
				upgrade[i].curValue = data->ammoMax;
				upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->ammoMax, oriData->ammoMax, cUpgradeCalculator::kAmmo, researchLevel);
				upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_AMMO;
				i++;
			}
		}

		if ( data->storeResType != sUnitData::STORE_RES_NONE )
		{
			i++;
		}

		if ( data->produceEnergy ) i += 2;

		if ( data->produceHumans ) i++;

		// Armor:
		upgrade[i].active = true;
		upgrade[i].startValue = oriData->armor;
		upgrade[i].curValue = data->armor;
		upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->armor, oriData->armor, cUpgradeCalculator::kArmor, researchLevel);
		upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_ARMOR;
		i++;

		// Hitpoints:
		upgrade[i].active = true;
		upgrade[i].startValue = oriData->hitpointsMax;
		upgrade[i].curValue = data->hitpointsMax;
		upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->hitpointsMax, oriData->hitpointsMax, cUpgradeCalculator::kHitpoints, researchLevel);
		upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_HITS;
		i++;

		// Scan:
		if ( data->scan )
		{
			upgrade[i].active = true;
			upgrade[i].startValue = oriData->scan;
			upgrade[i].curValue = data->scan;
			upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->scan, oriData->scan, cUpgradeCalculator::kScan, researchLevel);
			upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_SCAN;
			i++;
		}

		// Speed:
		if ( data->speedMax )
		{
			upgrade[i].active = true;
			upgrade[i].startValue = oriData->speedMax;
			upgrade[i].curValue = data->speedMax;
			upgrade[i].nextPrice = cUpgradeCalculator::instance().calcPrice (data->speedMax / 4, oriData->speedMax / 4, cUpgradeCalculator::kSpeed, researchLevel);
			upgrade[i].type = sUnitUpgrade::UPGRADE_TYPE_SPEED;
			i++;
		}
	}
}

void cUpgradeHangarMenu::setCredits( int credits_ )
{
	credits = credits_;
	goldBar->setCurrentValue ( credits );
}

int cUpgradeHangarMenu::getCredits()
{
	return credits;
}

cUpgradeMenu::cUpgradeMenu ( cPlayer *player ) : cUpgradeHangarMenu ( player ), cHangarMenu ( LoadPCX ( GFXOD_UPGRADE ), player, MNU_BG_ALPHA )
{
	credits = player->Credits;

	doneButton->setReleasedFunction ( &doneReleased );
	backButton->setReleasedFunction ( &backReleased );

	goldBar->setMaximalValue ( credits );
	goldBar->setCurrentValue ( credits );

	generateSelectionList();
	if (selectedUnit != 0)
	{
		unitDetails->setSelection (selectedUnit);
		upgradeButtons->setSelection (selectedUnit);
	}
	
	selectionChangedFunc = &selectionChanged;
}

cUpgradeMenu::~cUpgradeMenu()
{
	if ( Client ) Client->bFlagDrawHud = true;
}

void cUpgradeMenu::doneReleased ( void *parent )
{
	cUpgradeMenu *menu = dynamic_cast<cUpgradeMenu*>((cMenu*)parent);
	if ( !menu ) return;
	sendTakenUpgrades ( menu->unitUpgrades, menu->player );
	menu->end = true;
}

void cUpgradeMenu::backReleased ( void *parent )
{
	cUpgradeMenu *menu = dynamic_cast<cUpgradeMenu*>((cMenu*)parent);
	if ( !menu ) return;
	menu->terminate = true;
}

void cUpgradeMenu::selectionChanged ( void *parent )
{
	cUpgradeMenu *menu = dynamic_cast<cUpgradeMenu*>((cHangarMenu*)parent);
	if ( !menu ) menu = dynamic_cast<cUpgradeMenu*>((cUpgradeMenu*)parent);
	if ( !menu ) return;
	menu->upgradeButtons->setSelection ( menu->selectedUnit );
	menu->draw();
}

void cUpgradeMenu::generateSelectionList()
{
	sID oldSelectedUnit;
	bool selectOldSelectedUnit = false;
	if ( selectionList->getSelectedUnit() )
	{
		oldSelectedUnit = selectionList->getSelectedUnit()->getUnitID();
		selectOldSelectedUnit = true;
	}

	selectionList->clear();
	bool tank = upgradeFilter->TankIsChecked();
	bool plane = upgradeFilter->PlaneIsChecked();
	bool ship = upgradeFilter->ShipIsChecked();
	bool build = upgradeFilter->BuildingIsChecked();
	bool tnt = upgradeFilter->TNTIsChecked();

	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
	{
		if ( !tank && !ship && !plane ) continue;
		sUnitData &data = UnitsData.getVehicle (i, player->getClan ()).data;
		if ( tnt && !data.canAttack ) continue;
		if ( data.factorAir > 0 && !plane ) continue;
		if ( data.factorSea > 0 && data.factorGround == 0 && !ship ) continue;
		if ( data.factorGround > 0 && !tank ) continue;
		selectionList->addUnit ( data.ID, player, unitUpgrades[i] );
	}

	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
	{
		if ( !build ) continue;
		sUnitData &data = UnitsData.getBuilding (i, player->getClan ()).data;
		if ( tnt && !data.canAttack ) continue;
		selectionList->addUnit ( data.ID, player, unitUpgrades[UnitsData.getNrVehicles () + i] );
	}

	if (selectOldSelectedUnit)
	{
		for ( int i = 0; i < selectionList->getSize(); i++ )
		{
			if ( oldSelectedUnit == selectionList->getItem( i )->getUnitID() )
			{
				selectionList->setSelection ( selectionList->getItem( i ) );
				break;
			}
		}
	}
	if ( selectOldSelectedUnit == false && selectionList->getSize() > 0 ) 
		selectionList->setSelection ( selectionList->getItem( 0 ) );
}


cUnitHelpMenu::cUnitHelpMenu( sID unitID, cPlayer *owner ) : cMenu ( LoadPCX ( GFXOD_HELP ), MNU_BG_ALPHA )
{
	unit = new cMenuUnitListItem ( unitID, owner, NULL, MUL_DIS_TYPE_NOEXTRA, NULL, false );
	init (unitID);
}

cUnitHelpMenu::cUnitHelpMenu( sUnitData* unitData, cPlayer *owner ) : cMenu ( LoadPCX ( GFXOD_HELP ), MNU_BG_ALPHA )
{
	unit = new cMenuUnitListItem ( unitData, owner, NULL, MUL_DIS_TYPE_NOEXTRA, NULL, false );
	init (unitData->ID);
}

void cUnitHelpMenu::init(sID unitID)
{
	titleLabel = new cMenuLabel ( position.x+406, position.y+11, lngPack.i18n( "Text~Title~Unitinfo" ) );
	titleLabel->setCentered ( true );
	menuItems.Add ( titleLabel );
	
	doneButton = new cMenuButton ( position.x+474, position.y+452, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	doneButton->setReleasedFunction ( &doneReleased );
	menuItems.Add ( doneButton );

	infoImage = new cMenuImage ( position.x+11, position.y+13 );
	menuItems.Add ( infoImage );

	infoText = new cMenuLabel ( position.x+354, position.y+67 );
	infoText->setBox ( 269, 176 );
	menuItems.Add ( infoText );

	unitDetails = new cMenuUnitDetails ( position.x+16, position.y+297 );
	unitDetails->setSelection ( unit );
	menuItems.Add ( unitDetails );

	if ( unitID.getVehicle() )
	{
		infoImage->setImage ( unitID.getVehicle()->info );
		infoText->setText ( unitID.getVehicle()->data.description );
	}
	else if ( unitID.getBuilding() )
	{
		infoImage->setImage ( unitID.getBuilding()->info );
		infoText->setText ( unitID.getBuilding()->data.description );
	}
}

cUnitHelpMenu::~cUnitHelpMenu()
{
	delete titleLabel;

	delete infoImage;
	delete infoText;

	delete unitDetails;

	delete doneButton;

	delete unit;

	if ( Client ) Client->bFlagDrawHud = true;
}

void cUnitHelpMenu::doneReleased( void *parent )
{
	cUnitHelpMenu *menu = dynamic_cast<cUnitHelpMenu*>((cMenu*)parent);
	menu->end = true;
}

cStorageMenu::cStorageMenu( cList<cVehicle *> &storageList_, cVehicle *vehicle, cBuilding *building ) : cMenu ( LoadPCX ( GFXOD_STORAGE ), MNU_BG_ALPHA ), storageList(storageList_), ownerVehicle( vehicle ), ownerBuilding(building)
{
	if ( ownerVehicle ) unitData = ownerVehicle->data;
	else if ( ownerBuilding )
	{
		unitData = ownerBuilding->data;
		subBase = ownerBuilding->SubBase;
	}
	else return;

	offset = 0;
	canStorePlanes = unitData.storeUnitsImageType == sUnitData::STORE_UNIT_IMG_PLANE;
	canStoreShips = unitData.storeUnitsImageType == sUnitData::STORE_UNIT_IMG_SHIP;
	canRepairReloadUpgrade = ownerBuilding != NULL;

	metalValue = ownerBuilding ? subBase->Metal : 0;

	if ( !canStorePlanes )
	{
		SDL_Surface *surface = LoadPCX ( GFXOD_STORAGE_GROUND );
		SDL_BlitSurface ( surface, NULL, background, NULL );
	}

	doneButton = new cMenuButton ( position.x+518, position.y+371, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	doneButton->setReleasedFunction ( &doneReleased );
	menuItems.Add ( doneButton );

	upButton = new cMenuButton ( position.x+504, position.y+426, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG, FONT_LATIN_NORMAL );
	upButton->setReleasedFunction ( &upReleased );
	menuItems.Add ( upButton );

	downButton = new cMenuButton ( position.x+532, position.y+426, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG, FONT_LATIN_NORMAL );
	downButton->setReleasedFunction ( &downReleased );
	menuItems.Add ( downButton );

	activateAllButton = new cMenuButton ( position.x+518, position.y+246, lngPack.i18n ( "Text~Button~Active" ), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	activateAllButton->setReleasedFunction ( &activateAllReleased );
	menuItems.Add ( activateAllButton );

	reloadAllButton = new cMenuButton ( position.x+518, position.y+246+25, canRepairReloadUpgrade ? lngPack.i18n ( "Text~Button~Reload" ) : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	reloadAllButton->setReleasedFunction ( &reloadAllReleased );
	menuItems.Add ( reloadAllButton );

	repairAllButton = new cMenuButton ( position.x+518, position.y+246+25*2, canRepairReloadUpgrade ? lngPack.i18n ( "Text~Button~Repair" ) : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	repairAllButton->setReleasedFunction ( &repairAllReleased );
	menuItems.Add ( repairAllButton );

	upgradeAllButton = new cMenuButton ( position.x+518, position.y+246+25*3, canRepairReloadUpgrade ? lngPack.i18n ( "Text~Button~Upgrade" ) : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	upgradeAllButton->setReleasedFunction ( &upgradeAllReleased );
	menuItems.Add ( upgradeAllButton );

	metalBar = new cMenuMaterialBar ( position.x+546, position.y+106, position.x+557, position.y+86, metalValue, cMenuMaterialBar::MAT_BAR_TYPE_METAL );
	metalBar->setCurrentValue ( metalValue );
	menuItems.Add ( metalBar );

	generateItems();

	resetInfos();
}

cStorageMenu::~cStorageMenu()
{
	delete doneButton;

	if ( Client ) Client->bFlagDrawHud = true;
}

void cStorageMenu::generateItems()
{
	int maxX = canStorePlanes ? 2 : 3;
	int xStep = canStorePlanes ? 227 : 155;
	int xStepImage = canStorePlanes ? 227 : 155;
	int startX = canStorePlanes ? 42 : 8;

	for ( int x = 0; x < maxX; x++ )
	{
		for ( int y = 0; y < 2; y++ )
		{
			int index = x + y*maxX;
			activateButtons[index] = new cMenuButton ( position.x+startX+x*xStep, position.y+191+y*236, lngPack.i18n ( "Text~Button~Active" ), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
			activateButtons[index]->setReleasedFunction ( &activateReleased );
			relaodButtons[index] = new cMenuButton ( position.x+startX+x*xStep, position.y+191+25+y*236, canRepairReloadUpgrade ? lngPack.i18n ( "Text~Button~Reload" ) : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
			relaodButtons[index]->setReleasedFunction ( &reloadReleased );
			repairButtons[index] = new cMenuButton ( position.x+startX+75+x*xStep, position.y+191+y*236, canRepairReloadUpgrade ? lngPack.i18n ( "Text~Button~Repair" ) : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
			repairButtons[index]->setReleasedFunction ( &repairReleased );
			upgradeButtons[index] = new cMenuButton ( position.x+startX+75+x*xStep, position.y+191+25+y*236, canRepairReloadUpgrade ? lngPack.i18n ( "Text~Button~Upgrade" ) : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
			upgradeButtons[index]->setReleasedFunction ( &upgradeReleased );

			unitImages[index] = new cMenuImage ( position.x+17+x*xStepImage, position.y+9+y*236 );
			unitNames[index] = new cMenuLabel ( position.x+17+x*xStepImage+5, position.y+9+y*236+5 );
			unitNames[index]->setBox ( canStorePlanes ? 190 : 118, 118 );
			unitInfo[index] = new cMenuStoredUnitDetails ( position.x+startX+17+x*xStepImage, position.y+143+y*236 );

			menuItems.Add ( activateButtons[index] );
			menuItems.Add ( relaodButtons[index] );
			menuItems.Add ( repairButtons[index] );
			menuItems.Add ( upgradeButtons[index] );

			menuItems.Add ( unitImages[index] );
			menuItems.Add ( unitNames[index] );
			menuItems.Add ( unitInfo[index] );
		}
	}
}

void cStorageMenu::resetInfos()
{
	int maxX = canStorePlanes ? 2 : 3;

	for ( int x = 0; x < maxX; x++ )
	{
		for ( int y = 0; y < 2; y++ )
		{
			int pos = x+y*maxX;
			int index = offset*maxX*2 + pos;

			SDL_Surface *srcSurface;
			string name;

			if ( index < (int)storageList.Size() )
			{
				cVehicle *vehicle = storageList[index];
				srcSurface = vehicle->typ->storage;
				name = vehicle->name;
				if (  vehicle->data.version != vehicle->owner->VehicleData[ vehicle->typ->nr].version )
				{
					name += "\n(" + lngPack.i18n ( "Text~Comp~Dated" ) + ")";
				}
				unitInfo[pos]->setUnitData ( &vehicle->data );

				activateButtons[pos]->setLocked ( false );
				if ( vehicle->data.ammoCur != vehicle->data.ammoMax && metalValue >= 2 ) relaodButtons[pos]->setLocked ( false );
				else relaodButtons[pos]->setLocked ( true );
				if ( vehicle->data.hitpointsCur != vehicle->data.hitpointsMax && metalValue >= 2 ) repairButtons[pos]->setLocked ( false );
				else repairButtons[pos]->setLocked ( true );
				if ( vehicle->data.version != vehicle->owner->VehicleData[ vehicle->typ->nr].version && metalValue >= 2 ) upgradeButtons[pos]->setLocked ( false );
				else upgradeButtons[pos]->setLocked ( true );
			}
			else
			{
				name = "";
				if ( canStoreShips ) srcSurface = GraphicsData.gfx_edock;
				else if ( canStorePlanes ) srcSurface = GraphicsData.gfx_ehangar;
				else srcSurface = GraphicsData.gfx_edepot;
				unitInfo[pos]->setUnitData ( NULL );

				activateButtons[pos]->setLocked ( true );
				relaodButtons[pos]->setLocked ( true );
				repairButtons[pos]->setLocked ( true );
				upgradeButtons[pos]->setLocked ( true );
			}

			SDL_Surface *surface = SDL_CreateRGBSurface ( SDL_HWSURFACE, srcSurface->w, srcSurface->h, SettingsData.iColourDepth, 0, 0, 0, 0 );
			SDL_BlitSurface ( srcSurface, NULL, surface, NULL );
			unitImages[pos]->setImage ( surface );

			unitNames[pos]->setText ( name );
		}
	}

	activateAllButton->setLocked ( storageList.Size() == 0 );

	reloadAllButton->setLocked ( true );
	repairAllButton->setLocked ( true );
	upgradeAllButton->setLocked ( true );
	if ( canRepairReloadUpgrade && metalValue >= 2 )
	{
		for ( unsigned int i = 0; i < storageList.Size(); i++ )
		{
			cVehicle *vehicle = storageList[i];
			if ( vehicle->data.ammoCur != vehicle->data.ammoMax ) reloadAllButton->setLocked ( false );
			if ( vehicle->data.hitpointsCur != vehicle->data.hitpointsMax ) repairAllButton->setLocked ( false );
			if ( vehicle->data.version != vehicle->owner->VehicleData[vehicle->typ->nr].version ) upgradeAllButton->setLocked ( false );
		}
	}

	if ( ownerBuilding )
	{
		metalValue = ownerBuilding->SubBase->Metal;
		metalBar->setCurrentValue ( metalValue );
	}

	if ( (offset+1)*maxX*2 < unitData.storageResMax && (offset+1)*maxX*2 < (int)storageList.Size() ) downButton->setLocked ( false );
	else downButton->setLocked ( true );

	if ( offset > 0 ) upButton->setLocked ( false );
	else upButton->setLocked ( true );
}

void cStorageMenu::doneReleased( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	menu->end = true;
}

void cStorageMenu::upReleased( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	menu->offset--;;
	menu->resetInfos();
	menu->draw();
}

void cStorageMenu::downReleased( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	menu->offset++;
	menu->resetInfos();
	menu->draw();
}

int cStorageMenu::getClickedButtonVehIndex ( cMenuButton *buttons[6] )
{
	int maxX = canStorePlanes ? 2 : 3;

	for ( int x = 0; x < maxX; x++ )
	{
		for ( int y = 0; y < 2; y++ )
		{
			int index = offset*maxX*2 + x + y*maxX;
			if ( index >= (int)storageList.Size() ) break;

			if ( buttons[x+y*maxX]->overItem ( mouse->x, mouse->y ) ) return index;
		}
	}
	return -1;
}

void cStorageMenu::activateReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	int index = menu->getClickedButtonVehIndex ( menu->activateButtons );
	if ( index == -1 ) return;

	if ( menu->ownerVehicle )
	{
		menu->ownerVehicle->VehicleToActivate = index;
		if ( menu->unitData.factorAir > 0 ) sendWantActivate ( menu->ownerVehicle->iID, true, menu->storageList[index]->iID, menu->ownerVehicle->PosX, menu->ownerVehicle->PosY );
		else menu->ownerVehicle->ActivatingVehicle = true;
	}
	else if ( menu->ownerBuilding )
	{
		menu->ownerBuilding->VehicleToActivate = index;
		menu->ownerBuilding->ActivatingVehicle = true;
	}

	Client->OverUnitField = NULL;
	mouse->MoveCallback = true;
	menu->end = true;
}

void cStorageMenu::reloadReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	int index = menu->getClickedButtonVehIndex ( menu->relaodButtons );
	if ( index == -1 || !menu->ownerBuilding ) return;
	
	sendWantSupply ( menu->storageList[index]->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REARM );
}

void cStorageMenu::repairReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	int index = menu->getClickedButtonVehIndex ( menu->repairButtons );
	if ( index == -1 || !menu->ownerBuilding ) return;
	
	sendWantSupply ( menu->storageList[index]->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REPAIR );
}

void cStorageMenu::upgradeReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	int index = menu->getClickedButtonVehIndex ( menu->upgradeButtons );
	if ( index == -1 || !menu->ownerBuilding ) return;

	sendWantUpgrade ( menu->ownerBuilding->iID, index, false );
}

void cStorageMenu::activateAllReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	bool hasCheckedPlace[16];
	fill <bool*, bool> ( &hasCheckedPlace[0], &hasCheckedPlace[15], false );

	int unitXPos = menu->ownerBuilding ? menu->ownerBuilding->PosX : menu->ownerVehicle->PosX;
	int unitYPos = menu->ownerBuilding ? menu->ownerBuilding->PosY : menu->ownerVehicle->PosY;
	int id = menu->ownerBuilding ? menu->ownerBuilding->iID : menu->ownerVehicle->iID;
	bool isBig = menu->unitData.isBig;

	for ( unsigned int i = 0; i < menu->storageList.Size(); i++ )
	{
		cVehicle *vehicle = menu->storageList[i];
		bool activated = false;
		for ( int ypos = unitYPos-1, poscount = 0; ypos <= unitYPos+(isBig ? 2 : 1); ypos++ )
		{
			if ( ypos < 0 || ypos >= Client->Map->size ) continue;
			for ( int xpos = unitXPos-1; xpos <= unitXPos+(isBig ? 2 : 1); xpos++, poscount++ )
			{
				if ( xpos < 0 || xpos >= Client->Map->size || ( ( ( ypos == unitYPos && menu->unitData.factorAir == 0 ) || ( ypos == unitYPos+1 && isBig ) ) && ( ( xpos == unitXPos && menu->unitData.factorAir == 0 ) || ( xpos == unitXPos+1 && isBig ) ) ) ) continue;
				if ( ( ( menu->ownerBuilding && menu->ownerBuilding->canExitTo ( xpos, ypos, Client->Map, vehicle->typ ) ) ||
					( menu->ownerVehicle && menu->ownerVehicle->canExitTo ( xpos, ypos, Client->Map, vehicle->typ ) ) )
					&& !hasCheckedPlace[poscount] )
				{
					sendWantActivate ( id, menu->ownerVehicle != NULL, vehicle->iID, xpos, ypos );
					hasCheckedPlace[poscount] = true;
					activated = true;
					break;
				}
			}
			if ( activated ) break;
		}
	}
	menu->end = true;
}

void cStorageMenu::reloadAllReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	if ( !menu->ownerBuilding ) return;
	
	int resources = menu->metalValue;
	for ( unsigned int i = 0; i < menu->storageList.Size(); i++ )
	{
		if ( resources < 1 ) break;
		cVehicle *vehicle = menu->storageList[i];
		if ( vehicle->data.ammoCur != vehicle->data.ammoMax )
		{
			sendWantSupply ( vehicle->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REARM );
			resources--;
		}
	}
}

void cStorageMenu::repairAllReleased ( void *parent )
{
	cStorageMenu *menu = dynamic_cast<cStorageMenu*>((cMenu*)parent);
	if ( !menu->ownerBuilding ) return;
	
	int resources = menu->metalValue;
	for ( unsigned int i = 0; i < menu->storageList.Size(); i++ )
	{
		if ( resources < 1 ) break;
		cVehicle *vehicle = menu->storageList[i];
		if ( vehicle->data.hitpointsCur != vehicle->data.hitpointsMax )
		{
			sendWantSupply ( vehicle->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REPAIR );
			int value = vehicle->data.hitpointsCur;
			while ( value < vehicle->data.hitpointsMax )
			{
				value += Round(((float)vehicle->data.hitpointsMax/vehicle->data.buildCosts)*4);
				resources--;
			}
		}
	}
}

void cStorageMenu::upgradeAllReleased ( void *parent )
{
	cStorageMenu *menu = static_cast<cStorageMenu*>((cMenu*)parent);
	if ( !menu->ownerBuilding ) return;
	
	sendWantUpgrade ( menu->ownerBuilding->iID, 0, true );
}

cMineManagerMenu::cMineManagerMenu( cBuilding *building_ ) : cMenu ( LoadPCX(GFXOD_MINEMANAGER), MNU_BG_ALPHA ), subBase( *building_->SubBase ), building(building_)
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+11, lngPack.i18n ("Text~Title~Mine") );
	titleLabel->setCentered( true );
	menuItems.Add ( titleLabel );

	doneButton = new cMenuButton ( position.x+514, position.y+430, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_HUGE );
	doneButton->setReleasedFunction ( doneReleased );
	menuItems.Add ( doneButton );

	// add the bars before the labels so that the bars will be drawn under the labels
	for ( int i = 0; i < 3; i++ )
	{
		metalBars[i] = new cMenuMaterialBar ( position.x+174, position.y+70+37*i, position.x+174+120, position.y+70+15+37*i, 0, cMenuMaterialBar::MAT_BAR_TYPE_METAL_HORI_BIG, false, false );
		menuItems.Add ( metalBars[i] );
		oilBars[i] = new cMenuMaterialBar ( position.x+174, position.y+190+37*i, position.x+174+120, position.y+190+15+37*i, 0, cMenuMaterialBar::MAT_BAR_TYPE_OIL_HORI_BIG, false, false );
		menuItems.Add ( oilBars[i] );
		goldBars[i] = new cMenuMaterialBar ( position.x+174, position.y+310+37*i, position.x+174+120, position.y+310+15+37*i, 0, cMenuMaterialBar::MAT_BAR_TYPE_GOLD_HORI_BIG, false, false );
		menuItems.Add ( goldBars[i] );

		noneBars[i] = new cMenuMaterialBar ( position.x+174, position.y+70+120*i, position.x+174+120, position.y+310+15+37*i, 0, cMenuMaterialBar::MAT_BAR_TYPE_NONE_HORI_BIG, true, false );
		menuItems.Add ( noneBars[i] );
	}

	for ( int i = 0; i < 3; i++ )
	{
		incButtons[i] = new cMenuButton ( position.x+421, position.y+70+120*i, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_BIG );
		incButtons[i]->setReleasedFunction ( &increaseReleased );
		menuItems.Add ( incButtons[i] );
		decButtons[i] = new cMenuButton ( position.x+139, position.y+70+120*i, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_BIG );
		decButtons[i]->setReleasedFunction ( &decreseReleased );
		menuItems.Add ( decButtons[i] );

		if ( i == 0 ) resourceLabels[i] = new cMenuLabel ( position.x+81, position.y+78, lngPack.i18n ( "Text~Title~Metal" ) );
		else if ( i == 1 ) resourceLabels[i] = new cMenuLabel ( position.x+81, position.y+78+121, lngPack.i18n ( "Text~Title~Oil" ) );
		else  resourceLabels[i] = new cMenuLabel ( position.x+81, position.y+78+121*2, lngPack.i18n ( "Text~Title~Gold" ) );
		resourceLabels[i]->setCentered ( true );
		menuItems.Add ( resourceLabels[i] );

		usageLabels[i] = new cMenuLabel ( position.x+81, position.y+78+37+121*i, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		usageLabels[i]->setCentered ( true );
		menuItems.Add ( usageLabels[i] );

		reserveLabels[i] = new cMenuLabel ( position.x+81, position.y+78+37*2+121*i, lngPack.i18n ( "Text~Comp~Reserve" ) );
		reserveLabels[i]->setCentered ( true );
		menuItems.Add ( reserveLabels[i] );

		metalBarLabels[i] = new cMenuLabel ( position.x+174+120, position.y+70+8+37*i, "", FONT_LATIN_BIG );
		metalBarLabels[i]->setCentered ( true );
		menuItems.Add ( metalBarLabels[i] );

		oilBarLabels[i] = new cMenuLabel ( position.x+174+120, position.y+190+8+37*i, "", FONT_LATIN_BIG );
		oilBarLabels[i]->setCentered ( true );
		menuItems.Add ( oilBarLabels[i] );

		goldBarLabels[i] = new cMenuLabel ( position.x+174+120, position.y+310+8+37*i, "", FONT_LATIN_BIG );
		goldBarLabels[i]->setCentered ( true );
		menuItems.Add ( goldBarLabels[i] );
	}
	metalBars[0]->setClickedFunction ( &barReleased );
	oilBars[0]->setClickedFunction ( &barReleased );
	goldBars[0]->setClickedFunction ( &barReleased );

	setBarValues();
	setBarLabels();
}

cMineManagerMenu::~cMineManagerMenu()
{
	delete titleLabel;
	delete doneButton;

	if ( Client ) Client->bFlagDrawHud = true;

	for ( int i = 0; i < 3; i++ )
	{
		delete incButtons[i];
		delete decButtons[i];

		delete resourceLabels[i];
		delete usageLabels[i];
		delete reserveLabels[i];

		delete metalBars[i];
		delete oilBars[i];
		delete goldBars[i];

		delete metalBarLabels[i];
		delete oilBarLabels[i];
		delete goldBarLabels[i];

		delete noneBars[i];
	}
}

void cMineManagerMenu::setBarValues()
{
	metalBars[0]->setMaximalValue ( subBase.getMaxMetalProd() );
	metalBars[0]->setCurrentValue ( subBase.getMetalProd() );
	metalBars[1]->setMaximalValue ( subBase.MaxMetalNeed );
	metalBars[1]->setCurrentValue ( subBase.MetalNeed );
	metalBars[2]->setMaximalValue ( subBase.MaxMetal );
	metalBars[2]->setCurrentValue ( subBase.Metal );
	if ( subBase.getMaxMetalProd() == 0 )
	{
		noneBars[0]->setMaximalValue ( 1 );
		noneBars[0]->setCurrentValue ( 1 );
	}
	else
	{
		noneBars[0]->setMaximalValue ( subBase.getMaxMetalProd() );
		noneBars[0]->setCurrentValue ( subBase.getMaxMetalProd() - subBase.getMaxAllowedMetalProd() );
	}

	oilBars[0]->setMaximalValue ( subBase.getMaxOilProd() );
	oilBars[0]->setCurrentValue ( subBase.getOilProd() );
	oilBars[1]->setMaximalValue ( subBase.MaxOilNeed );
	oilBars[1]->setCurrentValue ( subBase.OilNeed );
	oilBars[2]->setMaximalValue ( subBase.MaxOil );
	oilBars[2]->setCurrentValue ( subBase.Oil );
	if ( subBase.getMaxOilProd() == 0 )
	{
		noneBars[1]->setMaximalValue ( 1 );
		noneBars[1]->setCurrentValue ( 1 );
	}
	else
	{
		noneBars[1]->setMaximalValue ( subBase.getMaxOilProd() );
		noneBars[1]->setCurrentValue ( subBase.getMaxOilProd() - subBase.getMaxAllowedOilProd() );
	}

	goldBars[0]->setMaximalValue ( subBase.getMaxGoldProd() );
	goldBars[0]->setCurrentValue ( subBase.getGoldProd() );
	goldBars[1]->setMaximalValue ( subBase.MaxGoldNeed );
	goldBars[1]->setCurrentValue ( subBase.GoldNeed );
	goldBars[2]->setMaximalValue ( subBase.MaxGold );
	goldBars[2]->setCurrentValue ( subBase.Gold );
	if ( subBase.getMaxGoldProd() == 0 )
	{
		noneBars[2]->setMaximalValue ( 1 );
		noneBars[2]->setCurrentValue ( 1 );
	}
	else
	{
		noneBars[2]->setMaximalValue ( subBase.getMaxGoldProd() );
		noneBars[2]->setCurrentValue ( subBase.getMaxGoldProd() - subBase.getMaxAllowedGoldProd() );
	}
}

void cMineManagerMenu::setBarLabels()
{
	metalBarLabels[0]->setText ( iToStr ( subBase.getMetalProd() ) );
	metalBarLabels[1]->setText ( secondBarText ( subBase.getMetalProd(), subBase.MetalNeed ) );
	metalBarLabels[2]->setText ( iToStr ( subBase.Metal ) );

	oilBarLabels[0]->setText ( iToStr ( subBase.getOilProd() ) );
	oilBarLabels[1]->setText ( secondBarText ( subBase.getOilProd(), subBase.OilNeed ) );
	oilBarLabels[2]->setText ( iToStr ( subBase.Oil ) );

	goldBarLabels[0]->setText ( iToStr ( subBase.getGoldProd() ) );
	goldBarLabels[1]->setText ( secondBarText ( subBase.getGoldProd(), subBase.GoldNeed ) );
	goldBarLabels[2]->setText ( iToStr ( subBase.Gold ) );
}

string cMineManagerMenu::secondBarText( int prod, int need )
{
	int perTurn = prod - need;
	string text = iToStr ( need ) + " (";
	if ( perTurn > 0 ) text += "+";
	text += iToStr ( perTurn ) + " / " + lngPack.i18n ( "Text~Comp~Turn" ) + ")";
	return text;
}

void cMineManagerMenu::doneReleased( void *parent )
{
	cMineManagerMenu *menu = static_cast<cMineManagerMenu*>((cMenu*)parent);
	sendChangeResources ( menu->building, menu->subBase.getMetalProd(),  menu->subBase.getOilProd(),  menu->subBase.getGoldProd() );
	menu->end = true;
}

void cMineManagerMenu::increaseReleased( void *parent )
{
	cMineManagerMenu *menu = static_cast<cMineManagerMenu*>((cMenu*)parent);
	if ( menu->incButtons[0]->overItem ( mouse->x, mouse->y ) && menu->subBase.getMaxAllowedMetalProd() - menu->subBase.getMetalProd() > 0 )
	{
		menu->subBase.changeMetalProd( +1 );
	}
	else if ( menu->incButtons[1]->overItem ( mouse->x, mouse->y ) && menu->subBase.getMaxAllowedOilProd() - menu->subBase.getOilProd() > 0 )
	{
		menu->subBase.changeOilProd( +1 );
	}
	else if ( menu->incButtons[2]->overItem ( mouse->x, mouse->y ) && menu->subBase.getMaxAllowedGoldProd() - menu->subBase.getGoldProd() > 0 )
	{
		menu->subBase.changeGoldProd( +1 );
	}
	menu->setBarValues();
	menu->setBarLabels();
	menu->draw();
}

void cMineManagerMenu::decreseReleased( void *parent )
{
	cMineManagerMenu *menu = static_cast<cMineManagerMenu*>((cMenu*)parent);
	if ( menu->decButtons[0]->overItem ( mouse->x, mouse->y ) && menu->subBase.getMetalProd() > 0 )
	{
		menu->subBase.changeMetalProd( -1 );
	}
	else if ( menu->decButtons[1]->overItem ( mouse->x, mouse->y ) && menu->subBase.getOilProd() > 0 )
	{
		menu->subBase.changeOilProd( -1 );
	}
	else if ( menu->decButtons[2]->overItem ( mouse->x, mouse->y ) && menu->subBase.getGoldProd() > 0 )
	{
		menu->subBase.changeGoldProd( -1 );
	}
	menu->setBarValues();
	menu->setBarLabels();
	menu->draw();
}

void cMineManagerMenu::barReleased( void *parent )
{
	cMineManagerMenu *menu = static_cast<cMineManagerMenu*>((cMenu*)parent);
	if ( menu->metalBars[0]->overItem ( mouse->x, mouse->y ) )
	{
		int metal =  Round ( ( mouse->x - menu->metalBars[0]->getPosition().x ) * ( menu->subBase.getMaxMetalProd() / (float)menu->metalBars[0]->getPosition().w ) );
		menu->subBase.setMetalProd ( metal );
	}
	else if ( menu->oilBars[0]->overItem ( mouse->x, mouse->y ) )
	{
		int oil =  Round ( ( mouse->x - menu->oilBars[0]->getPosition().x ) * ( menu->subBase.getMaxOilProd() / (float)menu->oilBars[0]->getPosition().w ) );
		menu->subBase.setOilProd ( oil );
	}
	else if ( menu->goldBars[0]->overItem ( mouse->x, mouse->y ) )
	{
		int gold =  Round ( ( mouse->x - menu->goldBars[0]->getPosition().x ) * ( menu->subBase.getMaxGoldProd() / (float)menu->goldBars[0]->getPosition().w ) );
		menu->subBase.setGoldProd ( gold );
	}
	menu->setBarValues();
	menu->setBarLabels();
	menu->draw();
}
