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

#include "autosurface.h"
#include "hud.h"
#include "main.h"
#include "mouse.h"
#include "sound.h"
#include "dialog.h"
#include "unifonts.h"
#include "client.h"
#include "serverevents.h"
#include "keys.h"
#include "input.h"
#include "pcx.h"
#include "player.h"
#include "settings.h"
#include "events.h"

bool sMouseBox::isTooSmall()
{
	if ( startX == -1 || startY == -1 || endX == -1 || endY == -1 ) return true;
	return !(endX > startX+(10/64.0) || endX < startX-(10/64.0) || endY > startY+(10/64.0) || endY < startY-(10/64.0));
}

cGameGUI::cGameGUI( cPlayer *player_, cMap *map_ ) :
	cMenu ( generateSurface() ),
	player ( player_ ),
	map ( map_ )
{
	frame = 0;
	zoom = 1.0;
	offX = offY = 0;
	framesPerSecond = cyclesPerSecond = 0;
	loadValue = 0;
	panelTopGraphic = NULL, panelBottomGraphic = NULL;
	activeItem = NULL;
	overUnitField = NULL;
	helpActive = false;
	blinkColor = 0xFFFFFF;
	FLC = NULL;
	playFLC = true;

	debugAjobs = false;
	debugBaseServer = false;
	debugBaseClient = false;
	debugSentry = false;
	debugFX = false;
	debugTraceServer = false;
	debugTraceClient = false;
	debugPlayers = false;
	showFPS = false;
	debugCache = false;

	selectedVehicle = NULL;
	selectedBuilding = NULL;

	minZoom = (float)(( max(SettingsData.iScreenH - HUD_TOTAL_HIGHT, SettingsData.iScreenW - HUD_TOTAL_WIDTH) / (float)map->size ) / 64.0);
	minZoom = max ( minZoom, ((int)( 64.0*minZoom )+( minZoom >= 1.0 ? 0 : 1 )) / (float)64.0 );

	generateSurface();

	setWind(random(360));

	closed = false;

	zoomSlider = new cMenuSlider ( 20, 274, minZoom, 1.0, this, 130, cMenuSlider::SLIDER_TYPE_HUD_ZOOM, cMenuSlider::SLIDER_DIR_RIGHTMIN  );
	zoomSlider->setMoveCallback ( &zoomSliderMoved );
	menuItems.Add ( zoomSlider );
	menuItems.Add ( zoomSlider->scroller );

	// generate checkbuttons
	surveyButton = new cMenuCheckButton ( 2, 296, lngPack.i18n( "Text~Hud~Survey"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_00, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( surveyButton );
	hitsButton = new cMenuCheckButton ( 57, 296, lngPack.i18n( "Text~Hud~Hitpoints"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_01, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( hitsButton );
	scanButton = new cMenuCheckButton ( 112, 296, lngPack.i18n( "Text~Hud~Scan"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_02, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( scanButton );
	statusButton = new cMenuCheckButton ( 2, 296+18, lngPack.i18n( "Text~Hud~Status"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_10, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( statusButton );
	ammoButton = new cMenuCheckButton ( 57, 296+18, lngPack.i18n( "Text~Hud~Ammo"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_11, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( ammoButton );
	gridButton = new cMenuCheckButton ( 112, 296+18, lngPack.i18n( "Text~Hud~Grid"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_12, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( gridButton );
	colorButton = new cMenuCheckButton ( 2, 296+18+16, lngPack.i18n( "Text~Hud~Color"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_20, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( colorButton );
	rangeButton = new cMenuCheckButton ( 57, 296+18+16, lngPack.i18n( "Text~Hud~Range"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_21, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( rangeButton );
	fogButton = new cMenuCheckButton ( 112, 296+18+16, lngPack.i18n( "Text~Hud~Fog"), false, false, cMenuCheckButton::CHECKBOX_HUD_INDEX_22, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( fogButton );

	lockButton = new cMenuCheckButton ( 32, 227, "", false, false, cMenuCheckButton::CHECKBOX_HUD_LOCK, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	menuItems.Add ( lockButton );

	TNTButton = new cMenuCheckButton ( 136, 413, "", false, false, cMenuCheckButton::CHECKBOX_HUD_TNT, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	TNTButton->setClickedFunction ( &changedMiniMap );
	menuItems.Add ( TNTButton );
	twoXButton = new cMenuCheckButton ( 136,387, "", false, false, cMenuCheckButton::CHECKBOX_HUD_2X, cMenuCheckButton::TEXT_ORIENT_RIGHT, FONT_LATIN_NORMAL, SoundData.SNDHudSwitch );
	twoXButton->setClickedFunction ( &changedMiniMap );
	menuItems.Add ( twoXButton );

	helpButton = new cMenuButton ( 20, 250, "", cMenuButton::BUTTON_TYPE_HUD_HELP );
	helpButton->setReleasedFunction ( &helpReleased );
	menuItems.Add ( helpButton );
	centerButton = new cMenuButton ( 4,227, "", cMenuButton::BUTTON_TYPE_HUD_CENTER );
	centerButton->setReleasedFunction ( &centerReleased );
	menuItems.Add ( centerButton );

	reportsButton = new cMenuButton ( 101, 252, lngPack.i18n( "Text~Hud~Log"), cMenuButton::BUTTON_TYPE_HUD_REPORT, FONT_LATIN_SMALL_WHITE );
	reportsButton->setReleasedFunction ( &reportsReleased );
	menuItems.Add ( reportsButton );
	chatButton = new cMenuButton ( 51, 252, lngPack.i18n( "Text~Hud~Chat"), cMenuButton::BUTTON_TYPE_HUD_CHAT, FONT_LATIN_SMALL_WHITE );
	chatButton->setReleasedFunction ( &chatReleased );
	menuItems.Add ( chatButton );

	nextButton = new cMenuButton ( 124, 227, ">>", cMenuButton::BUTTON_TYPE_HUD_NEXT, FONT_LATIN_SMALL_WHITE );
	nextButton->setReleasedFunction ( &nextReleased );
	menuItems.Add ( nextButton );
	prevButton = new cMenuButton ( 60, 227, "<<", cMenuButton::BUTTON_TYPE_HUD_PREV, FONT_LATIN_SMALL_WHITE );
	prevButton->setReleasedFunction ( &prevReleased );
	menuItems.Add ( prevButton );
	doneButton = new cMenuButton ( 99, 227, lngPack.i18n( "Text~Hud~Proceed"), cMenuButton::BUTTON_TYPE_HUD_DONE, FONT_LATIN_SMALL_WHITE );
	doneButton->setReleasedFunction ( &doneReleased );
	menuItems.Add ( doneButton );

	miniMapImage = new cMenuImage ( MINIMAP_POS_X, MINIMAP_POS_Y, generateMiniMapSurface() );
	miniMapImage->setClickedFunction ( &miniMapClicked );
	miniMapImage->setMovedOverFunction ( &miniMapMovedOver );
	menuItems.Add ( miniMapImage );

	coordsLabel = new cMenuLabel ( 265+32, (SettingsData.iScreenH-21)+3 );
	coordsLabel->setCentered ( true );
	menuItems.Add ( coordsLabel );

	unitNameLabel = new cMenuLabel ( 343+106, (SettingsData.iScreenH-21)+3 );
	unitNameLabel->setCentered ( true );
	menuItems.Add ( unitNameLabel );

	turnLabel = new cMenuLabel ( 498, 7 );
	turnLabel->setCentered ( true );
	menuItems.Add ( turnLabel );

	timeLabel = new cMenuLabel ( 564, 7 );
	timeLabel->setCentered ( true );
	menuItems.Add ( timeLabel );

	endButton = new cMenuButton ( 391, 4, lngPack.i18n( "Text~Hud~End"), cMenuButton::BUTTON_TYPE_HUD_END, FONT_LATIN_NORMAL );
	endButton->setReleasedFunction ( &endReleased );
	menuItems.Add ( endButton );

	preferencesButton = new cMenuButton ( 86, 4, lngPack.i18n( "Text~Hud~Settings"), cMenuButton::BUTTON_TYPE_HUD_PREFERENCES, FONT_LATIN_SMALL_WHITE );
	preferencesButton->setReleasedFunction ( &preferencesReleased );
	menuItems.Add ( preferencesButton );
	filesButton = new cMenuButton ( 17, 3, lngPack.i18n( "Text~Hud~Files"), cMenuButton::BUTTON_TYPE_HUD_FILES, FONT_LATIN_SMALL_WHITE );
	filesButton->setReleasedFunction ( &filesReleased );
	menuItems.Add ( filesButton );

	playButton = new cMenuButton ( 146, 123, "", cMenuButton::BUTTON_TYPE_HUD_PLAY );
	playButton->setReleasedFunction ( &playReleased );
	menuItems.Add ( playButton );
	stopButton = new cMenuButton ( 146, 143, "", cMenuButton::BUTTON_TYPE_HUD_STOP );
	stopButton->setReleasedFunction ( &stopReleased );
	menuItems.Add ( stopButton );

	FLCImage = new cMenuImage(10, 29, NULL);
	menuItems.Add ( FLCImage );

	unitDetails = new cMenuUnitDetails ( 8, 171, false, player );
	menuItems.Add ( unitDetails );

	chatBox = new cMenuChatBox ( HUD_LEFT_WIDTH+5, SettingsData.iScreenH-48, this );
	chatBox->setDisabled ( true );
	chatBox->setReturnPressedFunc ( &chatBoxReturnPressed );
	menuItems.Add ( chatBox );

	infoTextLabel = new cMenuLabel ( HUD_LEFT_WIDTH+(SettingsData.iScreenW-HUD_TOTAL_WIDTH)/2, 235, "", FONT_LATIN_BIG );
	infoTextLabel->setCentered ( true );
	infoTextLabel->setDisabled ( true );
	menuItems.Add ( infoTextLabel );

	infoTextAdditionalLabel = new cMenuLabel ( HUD_LEFT_WIDTH+(SettingsData.iScreenW-HUD_TOTAL_WIDTH)/2, 235+font->getFontHeight( FONT_LATIN_BIG ), "" );
	infoTextAdditionalLabel->setCentered ( true );
	infoTextAdditionalLabel->setDisabled ( true );
	menuItems.Add ( infoTextAdditionalLabel );

	selUnitStatusStr = new cMenuLabel ( 10, 40, "", FONT_LATIN_SMALL_WHITE );
	menuItems.Add ( selUnitStatusStr );

	updateTurn( 1 );
}

cGameGUI::~cGameGUI()
{
	zoom = 1.0;
	scaleSurfaces();

	if ( FLC ) FLI_Close ( FLC );

	delete surveyButton;
	delete hitsButton;
	delete scanButton;
	delete statusButton;
	delete ammoButton;
	delete gridButton;
	delete colorButton;
	delete rangeButton;
	delete fogButton;

	delete lockButton;

	delete TNTButton;
	delete twoXButton;

	delete helpButton;
	delete centerButton;
	delete reportsButton;
	delete chatButton;
	delete nextButton;
	delete prevButton;
	delete doneButton;
}

int cGameGUI::show()
{
	drawnEveryFrame = true;

	cMenu *lastActiveMenu = ActiveMenu;
	ActiveMenu = this;

	// do startup actions
	makePanel( true );
	startup = true;
	if ( Client->bWaitForOthers ) setInfoTexts ( lngPack.i18n ( "Text~Multiplayer~Wait_Until", Client->getPlayerFromNumber( 0 )->name ), "" );

	int lastMouseX = 0, lastMouseY = 0;

	while ( !end )
	{
		EventHandler->HandleEvents();

		mouse->GetPos();
		if ( mouse->moved() )
		{
			handleMouseMove();

			for ( unsigned int i = 0; i < menuItems.Size(); i++ )
			{
				cMenuItem *menuItem = menuItems[i];
				if ( menuItem->overItem( lastMouseX, lastMouseY ) && !menuItem->overItem( mouse->x, mouse->y ) ) menuItem->hoveredAway( this );
				else if ( !menuItem->overItem( lastMouseX, lastMouseY ) && menuItem->overItem( mouse->x, mouse->y ) ) menuItem->hoveredOn( this );
				else if ( menuItem->overItem( lastMouseX, lastMouseY ) && menuItem->overItem( mouse->x, mouse->y ) ) menuItem->movedMouseOver( lastMouseX, lastMouseY, this );
				else if ( menuItem == activeItem ) menuItem->somewhereMoved();
			}
		}

		lastMouseX = mouse->x;
		lastMouseY = mouse->y;

		if ( !SettingsData.bFastMode ) SDL_Delay ( 10 );

		Client->doGameActions();

		if ( startup )
		{
			if ( player->BuildingList ) player->BuildingList->Center();
			else if ( player->VehicleList ) player->VehicleList->Center();
			startup = false;
		}

		checkScroll();
		changeWindDir();
		updateStatusText();

		if ( needMiniMapDraw )
		{
			AutoSurface mini(generateMiniMapSurface());
			miniMapImage->setImage(mini);
			needMiniMapDraw = false;
		}
		if ( Client->timer100ms )
		{
			if ( FLC != NULL && playFLC )
			{
				FLI_NextFrame ( FLC );
				FLCImage->setImage ( FLC->surface );
			}
			rotateBlinkColor();
		}

		draw();
		frame++;

		handleFramesPerSecond();

		if ( terminate )
		{
			if ( lastActiveMenu ) lastActiveMenu->returnToCallback();
			return 1;
		}
	}

	makePanel ( false );

	if ( lastActiveMenu ) lastActiveMenu->returnToCallback();
	return 0;
}

void cGameGUI::returnToCallback()
{
	ActiveMenu = this;
}

SDL_Surface *cGameGUI::generateSurface()
{
	SDL_Rect scr, dest;
	SDL_Surface *surface = SDL_CreateRGBSurface ( SDL_HWSURFACE, SettingsData.iScreenW, SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );

	SDL_FillRect ( surface, NULL, 0xFF00FF );
	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );

	{ AutoSurface tmpSurface(LoadPCX(SettingsData.sGfxPath + PATH_DELIMITER + "hud_left.pcx"));
		if (tmpSurface)
		{
			SDL_BlitSurface ( tmpSurface, NULL, surface, NULL );
		}
	}

	{ AutoSurface tmpSurface(LoadPCX(SettingsData.sGfxPath + PATH_DELIMITER + "hud_top.pcx"));
		if (tmpSurface)
		{
			scr.x = 0;
			scr.y = 0;
			scr.w = tmpSurface->w;
			scr.h = tmpSurface->h;
			dest.x = HUD_LEFT_WIDTH;
			dest.y = 0;
			SDL_BlitSurface(tmpSurface, &scr, surface, &dest);
			scr.x = 1275;
			scr.w = 18;
			scr.h = HUD_TOP_HIGHT;
			dest.x = surface->w-HUD_TOP_HIGHT;
			SDL_BlitSurface(tmpSurface, &scr, surface, &dest);
		}
	}

	{ AutoSurface tmpSurface(LoadPCX(SettingsData.sGfxPath + PATH_DELIMITER + "hud_right.pcx"));
		if (tmpSurface)
		{
			scr.x = 0;
			scr.y = 0;
			scr.w = tmpSurface->w;
			scr.h = tmpSurface->h;
			dest.x = surface->w-HUD_RIGHT_WIDTH;
			dest.y = HUD_TOP_HIGHT;
			SDL_BlitSurface(tmpSurface, &scr, surface, &dest);
		}
	}

	{ AutoSurface tmpSurface(LoadPCX(SettingsData.sGfxPath + PATH_DELIMITER + "hud_bottom.pcx"));
		if (tmpSurface)
		{
			scr.x = 0;
			scr.y = 0;
			scr.w = tmpSurface->w;
			scr.h = tmpSurface->h;
			dest.x = HUD_LEFT_WIDTH;
			dest.y = surface->h-24;
			SDL_BlitSurface(tmpSurface, &scr, surface, &dest);
			scr.x = 1275;
			scr.w = 23;
			scr.h = 24;
			dest.x = surface->w-23;
			SDL_BlitSurface(tmpSurface, &scr, surface, &dest);
			scr.x = 1299;
			scr.w = 16;
			scr.h = 22;
			dest.x = HUD_LEFT_WIDTH-16;
			dest.y = surface->h-22;
			SDL_BlitSurface(tmpSurface, &scr, surface, &dest);
		}
	}

	if ( SettingsData.iScreenH > 480 )
	{
		AutoSurface tmpSurface(LoadPCX(SettingsData.sGfxPath + PATH_DELIMITER + "logo.pcx"));
		if (tmpSurface)
		{
			dest.x = 9;
			dest.y = SettingsData.iScreenH-HUD_TOTAL_HIGHT-15;
			SDL_BlitSurface ( tmpSurface, NULL, surface, &dest );
		}
	}
	return surface;
}

SDL_Surface *cGameGUI::generateMiniMapSurface()
{
	SDL_Surface* minimapSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, MINIMAP_SIZE, MINIMAP_SIZE, 32, 0, 0, 0, 0);
	Uint32* minimap = ( (Uint32*) minimapSurface->pixels );

	//set getZoom() factor
	int zoomFactor = 1;
	if ( twoXChecked() ) zoomFactor = MINIMAP_ZOOM_FACTOR;

	//set drawing offset, to center the minimap on the screen position
	int minimapOffsetX = 0, minimapOffsetY = 0;	//position of the upper left edge on the map
	if ( zoomFactor > 1 )
	{
		int centerPosX = (int) (offX / 64.0 + (SettingsData.iScreenW - HUD_TOTAL_WIDTH) / ((int)(getZoom()*64.0) * 2));
		int centerPosY = (int) (offY / 64.0 + (SettingsData.iScreenH -  HUD_TOTAL_HIGHT) / ((int)(getZoom()*64.0) * 2));
		minimapOffsetX = centerPosX - (map->size / (zoomFactor * 2));
		minimapOffsetY = centerPosY - (map->size / (zoomFactor * 2));

		if ( minimapOffsetX < 0 ) minimapOffsetX = 0;
		if ( minimapOffsetY < 0 ) minimapOffsetY = 0;
		if ( minimapOffsetX > map->size - (map->size / zoomFactor) ) minimapOffsetX = map->size - (map->size / zoomFactor);
		if ( minimapOffsetY > map->size - (map->size / zoomFactor) ) minimapOffsetY = map->size - (map->size / zoomFactor);
	}
	miniMapOffX = minimapOffsetX;
	miniMapOffY = minimapOffsetY;

	//draw the landscape
	for ( int miniMapX = 0; miniMapX < MINIMAP_SIZE; miniMapX++ )
	{
		//calculate the field on the map
		int terrainx = (miniMapX * map->size) / (MINIMAP_SIZE * zoomFactor) + minimapOffsetX;
		if ( terrainx >= map->size ) terrainx = map->size - 1;

		//calculate the position within the terrain graphic (for better rendering of maps < 112)
		int offsetx  = ((miniMapX * map->size ) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

		for ( int miniMapY = 0; miniMapY < MINIMAP_SIZE; miniMapY++ )
		{
			int terrainy =  (miniMapY * map->size) / (MINIMAP_SIZE * zoomFactor) + minimapOffsetY;
			if ( terrainy >= map->size ) terrainy = map->size - 1;
			int offsety  = ((miniMapY * map->size ) % (MINIMAP_SIZE * zoomFactor)) * 64 / (MINIMAP_SIZE * zoomFactor);

			SDL_Color sdlcolor;
			Uint8* terrainPixels = (Uint8*) map->terrain[map->Kacheln[terrainx+terrainy*map->size]].sf_org->pixels;
			Uint8 index = terrainPixels[offsetx + offsety*64];
			sdlcolor = map->terrain[map->Kacheln[terrainx+terrainy*map->size]].sf_org->format->palette->colors[index];
			Uint32 color = (sdlcolor.r << 16) + (sdlcolor.g << 8) + sdlcolor.b;

			minimap[miniMapX+miniMapY*MINIMAP_SIZE] = color;
		}
	}

	if ( player )
	{
		//draw the fog
		for ( int miniMapX = 0; miniMapX < MINIMAP_SIZE; miniMapX++ )
		{
			int terrainx = (miniMapX * map->size) / (MINIMAP_SIZE * zoomFactor) + minimapOffsetX;
			for ( int miniMapY = 0; miniMapY < MINIMAP_SIZE; miniMapY++ )
			{
				int terrainy = (miniMapY * map->size) / (MINIMAP_SIZE * zoomFactor) + minimapOffsetY;

				if ( !player->ScanMap[terrainx + terrainy * map->size] )
				{
					Uint8* color = (Uint8*) &minimap[miniMapX+miniMapY*MINIMAP_SIZE];
					color[0] = (Uint8) ( color[0] * 0.6 );
					color[1] = (Uint8) ( color[1] * 0.6 );
					color[2] = (Uint8) ( color[2] * 0.6 );
				}
			}
		}

		//draw the units
		//here we go through each map field instead of through each minimap pixel,
		//to make sure, that every unit is diplayed and has the same size on the minimap.

		//the size of the rect, that is drawn for each unit
		int size = (int) ceil((float) MINIMAP_SIZE * zoomFactor / map->size);
		if ( size < 2 ) size = 2;
		SDL_Rect rect;
		rect.h = size;
		rect.w = size;

		for ( int mapx = 0; mapx < map->size; mapx++ )
		{
			rect.x = ( (mapx - minimapOffsetX) * MINIMAP_SIZE * zoomFactor ) / map->size;
			if ( rect.x < 0 || rect.x >= MINIMAP_SIZE ) continue;
			for ( int mapy = 0; mapy < map->size; mapy++ )
			{
				rect.y = ( (mapy - minimapOffsetY) * MINIMAP_SIZE * zoomFactor ) / map->size;
				if ( rect.y < 0 || rect.y >= MINIMAP_SIZE ) continue;

				if ( !player->ScanMap[mapx + mapy * map->size] ) continue;

				cMapField& field = (*map)[mapx + mapy * map->size];

				//draw building
				cBuilding* building = field.getBuildings();
				if ( building && building->owner )
				{
					if ( !tntChecked() || building->data.canAttack )
					{
						unsigned int color = *( (unsigned int*) building->owner->color->pixels );
						SDL_FillRect( minimapSurface, &rect, color);
					}
				}

				//draw vehicle
				cVehicle* vehicle = field.getVehicles();
				if ( vehicle )
				{
					if ( !tntChecked() || vehicle->data.canAttack )
					{
						unsigned int color = *( (unsigned int*) vehicle->owner->color->pixels );
						SDL_FillRect( minimapSurface, &rect, color);
					}
				}

				//draw plane
				vehicle = field.getPlanes();
				if ( vehicle )
				{
					if ( !tntChecked() || vehicle->data.canAttack )
					{
						unsigned int color = *( (unsigned int*) vehicle->owner->color->pixels );
						SDL_FillRect( minimapSurface, &rect, color);
					}
				}
			}
		}
	}


	//draw the screen borders
	int startx, starty, endx, endy;
	startx = (int) ((((offX / 64.0) - minimapOffsetX) * MINIMAP_SIZE * zoomFactor) / map->size);
	starty = (int) ((((offY / 64.0) - minimapOffsetY) * MINIMAP_SIZE * zoomFactor) / map->size);
	endx = (int) ( startx + ((SettingsData.iScreenW - HUD_TOTAL_WIDTH) * MINIMAP_SIZE * zoomFactor) / (map->size * (getZoom()*64.0)));
	endy = (int) ( starty + ((SettingsData.iScreenH -  HUD_TOTAL_HIGHT) * MINIMAP_SIZE * zoomFactor) / (map->size * (getZoom()*64.0)));

	if ( endx == MINIMAP_SIZE ) endx = MINIMAP_SIZE - 1; //workaround
	if ( endy == MINIMAP_SIZE ) endy = MINIMAP_SIZE - 1;

	for ( int y = starty; y <= endy; y++ )
	{
		if ( y < 0 || y >= MINIMAP_SIZE ) continue;
		if ( startx >= 0 && startx < MINIMAP_SIZE )
		{
			minimap[y*MINIMAP_SIZE + startx] = MINIMAP_COLOR;
		}
		if ( endx >= 0 && endx < MINIMAP_SIZE )
		{
			minimap[y*MINIMAP_SIZE + endx] = MINIMAP_COLOR;
		}
	}
	for ( int x = startx; x <= endx; x++ )
	{
		if ( x < 0 || x >= MINIMAP_SIZE ) continue;
		if ( starty >= 0 && starty < MINIMAP_SIZE )
		{
			minimap[starty * MINIMAP_SIZE + x] = MINIMAP_COLOR;
		}
		if ( endy >= 0 && endy < MINIMAP_SIZE )
		{
			minimap[endy * MINIMAP_SIZE + x] = MINIMAP_COLOR;
		}
	}
	return minimapSurface;
}

void cGameGUI::setOffsetPosition ( int x, int y )
{
	offX = x;
	offY = y;
}

void cGameGUI::setZoom( float newZoom, bool setScroller )
{
	zoom = newZoom;
	if ( zoom < minZoom ) zoom = minZoom;
	if ( zoom > 1.0 ) zoom = 1.0;

	if ( setScroller ) this->zoomSlider->setValue ( zoom );

	static float lastZoom = 1.0;
	if ( lastZoom != getZoom() )
	{
		int off;
		int lastScreenPixel = (int)( (float)( SettingsData.iScreenW-HUD_TOTAL_WIDTH ) / lastZoom );
		int newScreenPixel = (int)( (float)( SettingsData.iScreenW-HUD_TOTAL_WIDTH ) / getZoom() );
		off = (lastScreenPixel-newScreenPixel)/2 ;
		lastZoom = getZoom();

		offX += off;
		offY += off;
	}
	if ( SettingsData.bPreScale ) scaleSurfaces();
	scaleColors();

	doScroll ( 0 );
}

float cGameGUI::getZoom()
{
	return ( getTileSize()/(float)64.0 );
}

int cGameGUI::getTileSize()
{
	return Round( 64.0*zoom );
}

void cGameGUI::setVideoSurface ( SDL_Surface *videoSurface )
{
	FLCImage->setImage ( videoSurface );
}

void cGameGUI::setFLC ( FLI_Animation *FLC_ )
{
	FLC = FLC_;
	if ( FLC )
	{
		FLI_Rewind ( FLC );
		FLI_NextFrame ( FLC );
		FLCImage->setImage ( FLC->surface );
	}
	else FLCImage->setImage ( NULL );
}

void cGameGUI::setPlayer ( cPlayer *player_ )
{
	player = player_;
	unitDetails->setOwner ( player );
}

void cGameGUI::setUnitDetailsData ( cVehicle *vehicle, cBuilding *building )
{
	unitDetails->setSelection ( vehicle, building );
}

void cGameGUI::updateTurn ( int turn )
{
	turnLabel->setText ( iToStr ( turn ) );
}

void cGameGUI::updateTurnTime ( int time )
{
	if ( time < 0 ) timeLabel->setText ( "" );
	else timeLabel->setText ( iToStr ( time ) );
}

void cGameGUI::callMiniMapDraw()
{
	needMiniMapDraw = true;
}

void cGameGUI::setEndButtonLock( bool locked )
{
	endButton->setLocked( locked );
}

void cGameGUI::handleFramesPerSecond()
{
	static Uint32 lastTicks = 0;
	static Uint32 lastFrame = 0;
	static Uint32 cycles = 0;
	static Uint32 inverseLoad = 0; //this is 10*(100% - load)
	static Uint32 lastTickLoad = 0;

	cycles++;
	Uint32 ticks = SDL_GetTicks();

	if ( ticks != lastTickLoad ) inverseLoad++;
	lastTickLoad = ticks;

	if ( ticks > lastTicks + 1000 )
	{
		float a = 0.5f;	//low pass filter coefficient

		framesPerSecond = (1-a)*(frame - lastFrame)*1000/(float)(ticks - lastTicks) + a*framesPerSecond;
		lastFrame = frame;

		cyclesPerSecond = (1-a)*cycles*1000 / (float) (ticks - lastTicks) + a*cyclesPerSecond;
		cycles = 0;

		loadValue = Round((1-a)*(1000 - inverseLoad) + a*loadValue);
		inverseLoad = 0;

		lastTicks = ticks;
	}
}

void cGameGUI::rotateBlinkColor()
{
	static bool dec = true;
	if ( dec )
	{
		blinkColor -= 0x0A0A0A;
		if ( blinkColor <= 0xA0A0A0 ) dec = false;
	}
	else
	{
		blinkColor += 0x0A0A0A;
		if ( blinkColor >= 0xFFFFFF ) dec = true;
	}
}

bool cGameGUI::checkScroll()
{
	if ( mouse->x <= 0 && mouse->y > 30 && mouse->y < SettingsData.iScreenH-30-18 )
	{
		mouse->SetCursor ( CPfeil4 );
		doScroll ( 4 );
		return true;
	}
	else if ( mouse->x >= SettingsData.iScreenW-18 && mouse->y > 30 && mouse->y < SettingsData.iScreenH-30-18 )
	{
		mouse->SetCursor ( CPfeil6 );
		doScroll ( 6 );
		return true;
	}
	else if ( ( mouse->x <= 0 && mouse->y <= 30 ) || ( mouse->y <= 0 && mouse->x <= 30 ) )
	{
		mouse->SetCursor ( CPfeil7 );
		doScroll ( 7 );
		return true;
	}
	else if ( ( mouse->x >= SettingsData.iScreenW-18 && mouse->y <= 30 ) || ( mouse->y<=0&&mouse->x>=SettingsData.iScreenW-30-18 ) )
	{
		mouse->SetCursor ( CPfeil9 );
		doScroll ( 9 );
		return true;
	}
	else if ( mouse->y <= 0 && mouse->x > 30 && mouse->x < SettingsData.iScreenW-30-18 )
	{
		mouse->SetCursor ( CPfeil8 );
		doScroll ( 8 );
		return true;
	}
	else if ( mouse->y >= SettingsData.iScreenH-18 && mouse->x > 30 && mouse->x < SettingsData.iScreenW-30-18 )
	{
		mouse->SetCursor ( CPfeil2 );
		doScroll ( 2 );
		return true;
	}
	else if ( ( mouse->x <= 0 && mouse->y >= SettingsData.iScreenH-30-18 ) || ( mouse->y >= SettingsData.iScreenH-18 && mouse->x <= 30 ) )
	{
		mouse->SetCursor ( CPfeil1 );
		doScroll ( 1 );
		return true;
	}
	else if ( ( mouse->x >= SettingsData.iScreenW-18 && mouse->y >= SettingsData.iScreenH-30-18 ) || ( mouse->y >= SettingsData.iScreenH-18 && mouse->x >= SettingsData.iScreenW-30-18 ) )
	{
		mouse->SetCursor ( CPfeil3 );
		doScroll ( 3 );
		return true;
	}
	return false;
}

void cGameGUI::updateUnderMouseObject()
{
	static int lastX = -1, lastY = -1;
	int x, y;

	mouse->GetKachel ( &x, &y );

	lastX = x;
	lastY = y;

	if ( x == -1 ) return;

	// draw the coordinates:
	/*array to get map coords in sceme XXX-YYY\0 = 8 characters
	a case where I accept an array since I don't know a better
	method to format x and y easily with leading 0 -- beko */
	char str[8];
	sprintf(str, "%.3d-%.3d", x, y);
	coordsLabel->setText ( str );

	if ( !player->ScanMap[x+y*map->size] )
	{
		overUnitField = NULL;
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			SDL_Rect r;
			r.x = 1; r.y = 29;
			r.h = 3; r.w = 35;
			SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
		}
		return;
	}
	// check wether there is a unit under the mouse:
	overUnitField = map->fields + (x + y * map->size);
	if ( mouse->cur == GraphicsData.gfx_Csteal && selectedVehicle )
	{
		selectedVehicle->drawCommandoCursor ( map->size*y+x, true );
	}
	else if ( mouse->cur == GraphicsData.gfx_Cdisable && selectedVehicle )
	{
		selectedVehicle->drawCommandoCursor ( map->size*y+x, false );
	}
	if ( overUnitField->getVehicles() != NULL )
	{
		unitNameLabel->setText ( overUnitField->getVehicles()->name );
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			if ( selectedVehicle )
			{
				selectedVehicle->DrawAttackCursor ( map->size*y+x );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->DrawAttackCursor ( map->size*y+x );
			}
		}
	}
	else if ( overUnitField->getPlanes() != NULL )
	{
		unitNameLabel->setText ( overUnitField->getPlanes()->name );
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			if ( selectedVehicle )
			{
				selectedVehicle->DrawAttackCursor ( map->size*y+x );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->DrawAttackCursor ( map->size*y+x );
			}
		}
	}
	else if ( overUnitField->getTopBuilding() != NULL )
	{
		unitNameLabel->setText ( overUnitField->getTopBuilding()->name );
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			if ( selectedVehicle )
			{
				selectedVehicle->DrawAttackCursor ( map->size*y+x );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->DrawAttackCursor ( map->size*y+x );
			}
		}
	}
	else if ( overUnitField->getBaseBuilding() != NULL )
	{
		unitNameLabel->setText ( overUnitField->getBaseBuilding()->name );
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			if ( selectedVehicle )
			{
				selectedVehicle->DrawAttackCursor ( map->size*y+x );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->DrawAttackCursor ( map->size*y+x );
			}
		}
	}
	else
	{
		unitNameLabel->setText ( "" );
		if ( mouse->cur == GraphicsData.gfx_Cattack )
		{
			SDL_Rect r;
			r.x = 1; r.y = 29;
			r.h = 3; r.w = 35;
			SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
		}
		overUnitField = NULL;
	}
	// place band:
	if ( selectedVehicle && selectedVehicle->PlaceBand )
	{
		selectedVehicle->FindNextband();
	}
}

void cGameGUI::incFrame()
{
	frame++;
}

void cGameGUI::setStartup ( bool startup_ )
{
	startup = startup_;
}

void cGameGUI::setSelVehicle( cVehicle *vehicle )
{
	selectedVehicle = vehicle;
}

void cGameGUI::setSelBuilding( cBuilding *building )
{
	selectedBuilding = building;
}

void cGameGUI::setInfoTexts ( string infoText, string additionalInfoText )
{
	infoTextLabel->setText ( infoText );
	infoTextLabel->setDisabled ( infoText.empty() );
	infoTextAdditionalLabel->setText ( additionalInfoText );
	infoTextAdditionalLabel->setDisabled ( additionalInfoText.empty() );
}

void cGameGUI::updateMouseCursor()
{
	updateUnderMouseObject();

	int x = mouse->x;
	int y = mouse->y;

	for ( unsigned int i = 0; i < menuItems.Size(); i++ )
	{
		if ( menuItems[i]->overItem( x, y ) && !menuItems[i]->isDisabled() )
		{
			mouse->SetCursor ( CHand );
			return;
		}
	}

	if ( selectedVehicle && selectedVehicle->PlaceBand && selectedVehicle->owner == Client->ActivePlayer )
	{
		if ( x >= HUD_LEFT_WIDTH )
		{
			mouse->SetCursor ( CBand );
		}
		else
		{
			mouse->SetCursor ( CNo );
		}
	}
	else if ( ( selectedVehicle && selectedVehicle->Transfer && selectedVehicle->owner == Client->ActivePlayer ) || ( selectedBuilding && selectedBuilding->Transfer && selectedBuilding->owner == Client->ActivePlayer ) )
	{
		if ( selectedVehicle )
		{
			if ( overUnitField && selectedVehicle->CanTransferTo ( overUnitField ) )
			{
				mouse->SetCursor ( CTransf );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else
		{
			if ( overUnitField && selectedBuilding->CanTransferTo ( overUnitField ) )
			{
				mouse->SetCursor ( CTransf );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
	}
	else if ( !helpActive )
	{
		if ( x < HUD_LEFT_WIDTH )
		{
			if ( mouse->SetCursor ( CHand ) )
			{
				overUnitField = NULL;
			}
			return;
		}

		if ( ( selectedVehicle && selectedVehicle->MenuActive&&selectedVehicle->MouseOverMenu ( x,y ) ) ||
		        ( selectedBuilding && selectedBuilding->MenuActive&&selectedBuilding->MouseOverMenu ( x,y ) ) )
		{
			mouse->SetCursor ( CHand );
		}
		else if ( selectedVehicle&&selectedVehicle->AttackMode&&selectedVehicle->owner==Client->ActivePlayer&&x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<SettingsData.iScreenW-HUD_RIGHT_WIDTH&&y<SettingsData.iScreenH-HUD_BOTTOM_HIGHT )
		{
			if ( !( selectedVehicle->data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && !Client->Map->IsWater( mouse->GetKachelOff() ) ))
			{
				if ( mouse->SetCursor ( CAttack ))
				{
					selectedVehicle->DrawAttackCursor( mouse->GetKachelOff());
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->DisableActive&&selectedVehicle->owner==Client->ActivePlayer&&x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<SettingsData.iScreenW-HUD_RIGHT_WIDTH&&y<SettingsData.iScreenH-HUD_BOTTOM_HIGHT )
		{
			if ( selectedVehicle->canDoCommandoAction ( mouse->GetKachelOff()%Client->Map->size, mouse->GetKachelOff()/Client->Map->size, Client->Map, false ) )
			{
				if ( mouse->SetCursor ( CDisable ) )
				{
					selectedVehicle->drawCommandoCursor( mouse->GetKachelOff(), false );
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->StealActive&&selectedVehicle->owner==Client->ActivePlayer&&x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<SettingsData.iScreenW-HUD_RIGHT_WIDTH&&y<SettingsData.iScreenH-HUD_BOTTOM_HIGHT )
		{
			if ( selectedVehicle->canDoCommandoAction ( mouse->GetKachelOff()%Client->Map->size, mouse->GetKachelOff()/Client->Map->size, Client->Map, true ) )
			{
				if ( mouse->SetCursor ( CSteal ) )
				{
					selectedVehicle->drawCommandoCursor( mouse->GetKachelOff(), true );
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<SettingsData.iScreenW-HUD_RIGHT_WIDTH&&y<SettingsData.iScreenH-HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction ( mouse->GetKachelOff()%Client->Map->size, mouse->GetKachelOff()/Client->Map->size, Client->Map, false )&& ( !overUnitField->getVehicles() || !overUnitField->getVehicles()->Disabled ) )
		{
			if ( mouse->SetCursor ( CDisable ) )
			{
				selectedVehicle->drawCommandoCursor( mouse->GetKachelOff(), false );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<SettingsData.iScreenW-HUD_RIGHT_WIDTH&&y<SettingsData.iScreenH-HUD_BOTTOM_HIGHT && selectedVehicle->canDoCommandoAction ( mouse->GetKachelOff()%Client->Map->size, mouse->GetKachelOff()/Client->Map->size, Client->Map, true ) )
		{
			if ( mouse->SetCursor ( CSteal ) )
			{
				selectedVehicle->drawCommandoCursor( mouse->GetKachelOff(), true );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->AttackMode&&selectedBuilding->owner==Client->ActivePlayer&&x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<SettingsData.iScreenW-HUD_RIGHT_WIDTH&&y<SettingsData.iScreenH-HUD_BOTTOM_HIGHT )
		{
			if ( selectedBuilding->IsInRange ( mouse->GetKachelOff(), Client->Map ) )
			{
				if ( mouse->SetCursor ( CAttack ))
				{
					selectedBuilding->DrawAttackCursor( mouse->GetKachelOff());
				}
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->CanAttackObject ( mouse->GetKachelOff(), Client->Map, false, false ) )
		{
			if ( mouse->SetCursor ( CAttack ))
			{
				selectedVehicle->DrawAttackCursor( mouse->GetKachelOff() );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->owner==Client->ActivePlayer&&selectedBuilding->CanAttackObject ( mouse->GetKachelOff(), Client->Map ) )
		{
			if ( mouse->SetCursor ( CAttack ))
			{
				selectedBuilding->DrawAttackCursor( mouse->GetKachelOff() );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->MuniActive )
		{
			if ( selectedVehicle->canSupply ( mouse->GetKachelOff(), SUPPLY_TYPE_REARM ) )
			{
				mouse->SetCursor ( CMuni );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->RepairActive )
		{
			if ( selectedVehicle->canSupply ( mouse->GetKachelOff(), SUPPLY_TYPE_REPAIR ) )
			{
				mouse->SetCursor ( CRepair );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if (overUnitField &&
				(
					overUnitField->getVehicles() ||
					overUnitField->getPlanes() ||
					(
						overUnitField->getBuildings() &&
						overUnitField->getBuildings()->owner
					)
				) &&
				(
					!selectedVehicle                               ||
					selectedVehicle->owner != Client->ActivePlayer ||
					(
						(
							selectedVehicle->data.factorAir > 0 ||
							overUnitField->getVehicles() ||
							(
								overUnitField->getTopBuilding() &&
								overUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE
							) ||
							(
								MouseStyle == OldSchool &&
								overUnitField->getPlanes()
							)
						) &&
						(
							selectedVehicle->data.factorAir == 0 ||
							overUnitField->getPlanes() ||
							(
								MouseStyle == OldSchool &&
								(
									overUnitField->getVehicles() ||
									(
										overUnitField->getTopBuilding() &&
										overUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE &&
										!overUnitField->getTopBuilding()->data.canBeLandedOn
									)
								)
							)
						) &&
						!selectedVehicle->LoadActive &&
						!selectedVehicle->ActivatingVehicle
					)
				) &&
				(
					!selectedBuilding                               ||
					selectedBuilding->owner != Client->ActivePlayer ||
					(
						(
							!selectedBuilding->BuildList                    ||
							!selectedBuilding->BuildList->Size()            ||
							selectedBuilding->IsWorking                     ||
							(*selectedBuilding->BuildList)[0]->metall_remaining > 0
						) &&
						!selectedBuilding->LoadActive &&
						!selectedBuilding->ActivatingVehicle
					)
				)
				)
		{
			mouse->SetCursor ( CSelect );
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->LoadActive )
		{
			if ( selectedVehicle->canLoad ( mouse->GetKachelOff(), Client->Map, false ) )
			{
				mouse->SetCursor ( CLoad );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer&&selectedVehicle->ActivatingVehicle )
		{
			int x, y;
			mouse->GetKachel( &x, &y );
			if (selectedVehicle->canExitTo(x, y, Client->Map,selectedVehicle->StoredVehicles[selectedVehicle->VehicleToActivate]->typ) )
			{
				mouse->SetCursor ( CActivate );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedVehicle&&selectedVehicle->owner==Client->ActivePlayer && x>=HUD_LEFT_WIDTH&&y>=HUD_TOP_HIGHT&&x<HUD_LEFT_WIDTH+ ( SettingsData.iScreenW-HUD_TOTAL_WIDTH ) && y<HUD_TOP_HIGHT+ ( SettingsData.iScreenH-HUD_TOTAL_HIGHT ) )
		{
			if ( !selectedVehicle->IsBuilding&&!selectedVehicle->IsClearing&&!selectedVehicle->LoadActive&&!selectedVehicle->ActivatingVehicle )
			{
				if ( selectedVehicle->MoveJobActive )
				{
					mouse->SetCursor ( CNo );
				}
				else if ( Client->Map->possiblePlace( selectedVehicle, mouse->GetKachelOff() ))
				{
					mouse->SetCursor ( CMove );
				}
				else
				{
					mouse->SetCursor ( CNo );
				}
			}
			else if ( selectedVehicle->IsBuilding || selectedVehicle->IsClearing )
			{
				int x, y;
				mouse->GetKachel( &x, &y );
				if ( ( ( selectedVehicle->IsBuilding&&selectedVehicle->BuildRounds == 0 ) ||
					 ( selectedVehicle->IsClearing&&selectedVehicle->ClearingRounds == 0 ) ) &&
					 Client->Map->possiblePlace( selectedVehicle, mouse->GetKachelOff()) && selectedVehicle->isNextTo(x, y))
				{
					mouse->SetCursor( CMove );
				}
				else
				{
					mouse->SetCursor( CNo );
				}
			}
		}
		else if (
				selectedBuilding                                &&
				selectedBuilding->owner == Client->ActivePlayer &&
				selectedBuilding->BuildList                     &&
				selectedBuilding->BuildList->Size()             &&
				!selectedBuilding->IsWorking                    &&
				(*selectedBuilding->BuildList)[0]->metall_remaining <= 0)
		{
			int x, y;
			mouse->GetKachel( &x, &y);
			if ( selectedBuilding->canExitTo(x, y, Client->Map, (*selectedBuilding->BuildList)[0]->typ))
			{
				mouse->SetCursor ( CActivate );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->owner==Client->ActivePlayer&&selectedBuilding->ActivatingVehicle )
		{
			int x, y;
			mouse->GetKachel( &x, &y);
			if ( selectedBuilding->canExitTo(x, y, Client->Map, selectedBuilding->StoredVehicles[selectedBuilding->VehicleToActivate]->typ))
			{
				mouse->SetCursor ( CActivate );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else if ( selectedBuilding&&selectedBuilding->owner==Client->ActivePlayer&&selectedBuilding->LoadActive )
		{
			if ( selectedBuilding->canLoad ( mouse->GetKachelOff(), Client->Map, false ) )
			{
				mouse->SetCursor ( CLoad );
			}
			else
			{
				mouse->SetCursor ( CNo );
			}
		}
		else
		{
			mouse->SetCursor ( CHand );
		}
	}
	else
	{
		mouse->SetCursor ( CHelp );
	}
}

void cGameGUI::handleMouseMove()
{
	if ( checkScroll() ) return;

	updateMouseCursor();

	// update mouseboxes
	if ( savedMouseState.leftButtonPressed && !savedMouseState.rightButtonPressed && mouseBox.startX != -1 && mouse->x > HUD_LEFT_WIDTH )
	{
		mouseBox.endX = (float)( ( ( mouse->x-HUD_LEFT_WIDTH ) + (offX*getZoom()) ) / getTileSize() );
		mouseBox.endY = (float)( ( ( mouse->y-HUD_TOP_HIGHT ) + (offY*getZoom()) ) / getTileSize() );
	}
	if ( savedMouseState.rightButtonPressed && !savedMouseState.leftButtonPressed && rightMouseBox.startX != -1 && mouse->x > HUD_LEFT_WIDTH )
	{
		rightMouseBox.endX = (float)( ( ( mouse->x-HUD_LEFT_WIDTH ) + (offX*getZoom()) ) / getTileSize() );
		rightMouseBox.endY = (float)( ( ( mouse->y-HUD_TOP_HIGHT ) + (offY*getZoom()) ) / getTileSize() );
	}
}

void cGameGUI::handleMouseInputExtended( sMouseState mouseState )
{
	for ( unsigned int i = 0; i < menuItems.Size(); i++ )
	{
		if ( !menuItems[i]->isDisabled() && menuItems[i]->overItem( mouse->x, mouse->y ) ) return;
	}

	bool changeAllowed = !Client->bWaitForOthers;

	savedMouseState = mouseState;

	cVehicle* overVehicle = NULL;
	cVehicle* overPlane = NULL;
	cBuilding* overBuilding = NULL;
	cBuilding* overBaseBuilding = NULL;
	if ( overUnitField )
	{
		overVehicle  = overUnitField->getVehicles();
		overPlane    = overUnitField->getPlanes();
		overBuilding = overUnitField->getTopBuilding();
		overBaseBuilding = overUnitField->getBaseBuilding();
	}

	if ( selectedVehicle && selectedVehicle->MenuActive && selectedVehicle->MouseOverMenu ( mouse->x, mouse->y ) )
	{
		if ( mouseState.leftButtonReleased && !mouseState.rightButtonPressed ) selectedVehicle->menuReleased();
		return;
	}
	else if ( selectedBuilding && selectedBuilding->MenuActive && selectedBuilding->MouseOverMenu ( mouse->x, mouse->y ) )
	{
		if ( mouseState.leftButtonReleased && !mouseState.rightButtonPressed ) selectedBuilding->menuReleased();
		return;
	}

	// handle input on the map
	if ( MouseStyle == OldSchool && mouseState.rightButtonReleased && !mouseState.leftButtonPressed && overUnitField )
	{
		if (( overVehicle && overVehicle == selectedVehicle ) || ( overPlane && overPlane == selectedVehicle ))
		{
			cUnitHelpMenu helpMenu ( &selectedVehicle->data, selectedVehicle->owner );
			helpMenu.show();
		}
		else if (( overBuilding && overBuilding == selectedBuilding ) || ( overBaseBuilding && overBaseBuilding == selectedBuilding ) )
		{
			cUnitHelpMenu helpMenu ( &selectedBuilding->data, selectedBuilding->owner );
			helpMenu.show();
		}
		else if ( overUnitField ) selectUnit ( overUnitField, true );
	}
	else if ( ( mouseState.rightButtonReleased && !mouseState.leftButtonPressed && rightMouseBox.isTooSmall()) || ( MouseStyle == OldSchool && mouseState.leftButtonPressed && mouseState.rightButtonReleased ) )
	{
		if ( helpActive )
		{
			helpActive = false;
		}
		else
		{
			deselectGroup();
			if ( overUnitField && (
			            ( selectedVehicle && ( overVehicle == selectedVehicle || overPlane == selectedVehicle ) ) ||
			            ( selectedBuilding && ( overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding ) ) ) )
			{
				int next = -1;

				if ( selectedVehicle )
				{
					if ( overPlane == selectedVehicle )
					{
						if ( overVehicle ) next = 'v';
						else if ( overBuilding ) next = 't';
						else if ( overBaseBuilding ) next = 'b';
					}
					else
					{
						if ( overBuilding ) next = 't';
						else if ( overBaseBuilding ) next = 'b';
						else if ( overPlane ) next = 'p';
					}

					selectedVehicle->Deselct();
					selectedVehicle = NULL;
					StopFXLoop ( Client->iObjectStream );
				}
				else if ( selectedBuilding )
				{
					if ( overBuilding == selectedBuilding )
					{
						if ( overBaseBuilding ) next = 'b';
						else if ( overPlane ) next = 'p';
						else if ( overUnitField->getVehicles() ) next = 'v';
					}
					else
					{
						if ( overPlane ) next = 'p';
						else if ( overUnitField->getVehicles() ) next = 'v';
						else if ( overBuilding ) next = 't';
					}

					selectedBuilding->Deselct();
					selectedBuilding = NULL;
					StopFXLoop ( Client->iObjectStream );
				}
				switch ( next )
				{
					case 't':
						selectedBuilding = overBuilding ;
						selectedBuilding->Select();
						Client->iObjectStream=selectedBuilding->playStream();
						break;
					case 'b':
						selectedBuilding = overBaseBuilding;
						selectedBuilding->Select();
						Client->iObjectStream=selectedBuilding->playStream();
						break;
					case 'v':
						selectedVehicle = overVehicle;
						selectedVehicle->Select();
						Client->iObjectStream = selectedVehicle->playStream();
						break;
					case 'p':
						selectedVehicle = overPlane;
						selectedVehicle->Select();
						Client->iObjectStream = selectedVehicle->playStream();
						break;
				}
			}
			else if ( selectedVehicle != NULL )
			{
				selectedVehicle->Deselct();
				selectedVehicle = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			else if ( selectedBuilding!=NULL )
			{
				selectedBuilding->Deselct();
				selectedBuilding = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
		}
	}

	if ( mouseState.rightButtonReleased && !mouseState.leftButtonPressed )
	{
		rightMouseBox.startX = rightMouseBox.startY = -1;
		rightMouseBox.endX = rightMouseBox.endY = -1;
	}
	else if ( mouseState.rightButtonPressed && !mouseState.leftButtonPressed && rightMouseBox.startX == -1 && mouse->x > HUD_LEFT_WIDTH && mouse->y > 20 )
	{
		rightMouseBox.startX = (float)( ( ( mouse->x-HUD_LEFT_WIDTH ) + offX ) / 64.0 );
		rightMouseBox.startY = (float)( ( ( mouse->y-HUD_TOP_HIGHT ) + offY ) / 64.0 );
	}
	if ( mouseState.leftButtonReleased && !mouseState.rightButtonPressed )
	{
		// Store the currently selected unit to determine if the lock state of the clicked unit maybe has to be changed.
		// If the selected unit changes during the click handling, then the newly selected unit has to be added / removed from the "locked units" list.
		cVehicle* oldSelectedVehicleForLock = selectedVehicle;
		cBuilding* oldSelectedBuildingForLock = selectedBuilding;

		if ( !mouseBox.isTooSmall() )
		{
			selectBoxVehicles( mouseBox );
		}
		else if ( changeAllowed && selectedVehicle && mouse->cur == GraphicsData.gfx_Ctransf )
		{
			if ( overVehicle )
			{
				cDialogTransfer transferDialog ( NULL, selectedVehicle, NULL, overVehicle );
				transferDialog.show();
			}
			else if ( overBuilding )
			{
				cDialogTransfer transferDialog ( NULL, selectedVehicle, overBuilding, NULL );
				transferDialog.show();
			}
		}
		else if ( changeAllowed && selectedBuilding && mouse->cur == GraphicsData.gfx_Ctransf )
		{
			if ( overVehicle )
			{
				cDialogTransfer transferDialog ( selectedBuilding, NULL, NULL, overVehicle );
				transferDialog.show();
			}
			else if ( overBuilding )
			{
				cDialogTransfer transferDialog ( selectedBuilding, NULL, overBuilding, NULL );
				transferDialog.show();
			}
		}
		else if ( changeAllowed && selectedVehicle && selectedVehicle->PlaceBand && mouse->cur == GraphicsData.gfx_Cband )
		{
			selectedVehicle->PlaceBand = false;
			if ( selectedVehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig )
			{
				sendWantBuild ( selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, selectedVehicle->BandX+selectedVehicle->BandY*map->size, false, 0 );
			}
			else
			{
				sendWantBuild ( selectedVehicle->iID, selectedVehicle->BuildingTyp, selectedVehicle->BuildRounds, selectedVehicle->PosX+selectedVehicle->PosY*map->size, true, selectedVehicle->BandX+selectedVehicle->BandY*map->size );
			}
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedBuilding && selectedBuilding->ActivatingVehicle )
		{
			sendWantActivate ( selectedBuilding->iID, false, selectedBuilding->StoredVehicles[selectedBuilding->VehicleToActivate]->iID, mouse->GetKachelOff()%map->size, mouse->GetKachelOff()/map->size );
			updateMouseCursor();
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedVehicle && selectedVehicle->ActivatingVehicle )
		{
			sendWantActivate ( selectedVehicle->iID, true, selectedVehicle->StoredVehicles[selectedVehicle->VehicleToActivate]->iID, mouse->GetKachelOff()%map->size, mouse->GetKachelOff()/map->size );
			updateMouseCursor();
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cactivate && selectedBuilding && selectedBuilding->BuildList && selectedBuilding->BuildList->Size())
		{
			int iX, iY;
			mouse->GetKachel ( &iX, &iY );
			sendWantExitFinishedVehicle ( selectedBuilding, iX, iY );
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cload && selectedBuilding && selectedBuilding->LoadActive )
		{
			if ( overVehicle )
			{
				if ( selectedBuilding->isNextTo ( overVehicle->PosX, overVehicle->PosY ) ) sendWantLoad ( selectedBuilding->iID, false, overVehicle->iID );
				else
				{
					// the constructor does everything for us
					cEndMoveAction *endMoveAction = new cEndMoveAction ( EMAT_GET_IN, selectedBuilding, NULL, NULL, overVehicle );
					if ( !endMoveAction->getSuccess() ) delete endMoveAction;
				}
			}
			else if ( overPlane )
			{
				if ( selectedBuilding->isNextTo ( overPlane->PosX, overPlane->PosY ) ) sendWantLoad ( selectedBuilding->iID, false, overPlane->iID );
				else
				{
					// the constructor does everything for us
					cEndMoveAction *endMoveAction = new cEndMoveAction ( EMAT_GET_IN, selectedBuilding, NULL, NULL, overPlane );
					if ( !endMoveAction->getSuccess() ) delete endMoveAction;
				}
			}
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cload && selectedVehicle && selectedVehicle->LoadActive )
		{
			if ( selectedVehicle->data.factorAir > 0 && overVehicle )
			{
				if ( overVehicle->PosX == selectedVehicle->PosX && overVehicle->PosY == selectedVehicle->PosY ) sendWantLoad ( selectedVehicle->iID, true, overVehicle->iID );
				else
				{
					// the constructor does everything for us
					cEndMoveAction *endMoveAction = new cEndMoveAction ( EMAT_LOAD, NULL, selectedVehicle, NULL, overVehicle );
					if ( !endMoveAction->getSuccess() ) delete endMoveAction;
				}
			}
			else if ( overVehicle )
			{
				if ( selectedVehicle->isNextTo ( overVehicle->PosX, overVehicle->PosY ) ) sendWantLoad ( selectedVehicle->iID, true, overVehicle->iID );
				else
				{
					// the constructor does everything for us
					cEndMoveAction *endMoveAction = new cEndMoveAction ( EMAT_GET_IN, NULL, selectedVehicle, NULL, overVehicle );
					if ( !endMoveAction->getSuccess() ) delete endMoveAction;
				}
			}
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cmuni && selectedVehicle && selectedVehicle->MuniActive )
		{
			if ( overVehicle ) sendWantSupply ( overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if ( overPlane && overPlane->FlightHigh == 0 ) sendWantSupply ( overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
			else if ( overBuilding ) sendWantSupply ( overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REARM);
		}
		else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Crepair && selectedVehicle && selectedVehicle->RepairActive )
		{
			if ( overVehicle ) sendWantSupply ( overVehicle->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if ( overPlane && overPlane->FlightHigh == 0 ) sendWantSupply ( overPlane->iID, true, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
			else if ( overBuilding ) sendWantSupply ( overBuilding->iID, false, selectedVehicle->iID, true, SUPPLY_TYPE_REPAIR);
		}
		else if ( !helpActive )
		{
			//Hud.CheckButtons();
			// check whether the mouse is over an unit menu:
			if ( ( selectedVehicle&&selectedVehicle->MenuActive&&selectedVehicle->MouseOverMenu ( mouse->x,mouse->y ) ) ||
			        ( selectedBuilding&&selectedBuilding->MenuActive&&selectedBuilding->MouseOverMenu ( mouse->x,mouse->y ) ) )
			{
			}
			else
				// check, if the player wants to attack:
				if ( changeAllowed && mouse->cur==GraphicsData.gfx_Cattack&&selectedVehicle&&!selectedVehicle->Attacking&&!selectedVehicle->MoveJobActive )
				{
					cVehicle* vehicle;
					cBuilding* building;
					selectTarget( vehicle, building, mouse->GetKachelOff(), selectedVehicle->data.canAttack, Client->Map);

					if ( selectedVehicle->IsInRange ( mouse->GetKachelOff(), map ) )
					{
						//find target ID
						int targetId = 0;
						if ( vehicle ) targetId = vehicle->iID;

						Log.write(" Client: want to attack offset " + iToStr(mouse->GetKachelOff()) + ", Vehicle ID: " + iToStr(targetId), cLog::eLOG_TYPE_NET_DEBUG );
						sendWantAttack( targetId, mouse->GetKachelOff(), selectedVehicle->iID, true );
					}
					else
					{
						// the constructor does everything for us
						cEndMoveAction *endMoveAction = new cEndMoveAction ( EMAT_ATTACK, NULL, selectedVehicle, building, vehicle, mouse->GetKachelOff()%map->size, mouse->GetKachelOff()/map->size );
						if ( !endMoveAction->getSuccess() ) delete endMoveAction;
					}
				}
				else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cattack && selectedBuilding && !selectedBuilding->Attacking )
				{
					//find target ID
					int targetId = 0;
					cVehicle* vehicle;
					cBuilding* building;
					selectTarget( vehicle, building, mouse->GetKachelOff(), selectedBuilding->data.canAttack, Client->Map);
					if ( vehicle ) targetId = vehicle->iID;

					int offset = selectedBuilding->PosX + selectedBuilding->PosY * map->size;
					sendWantAttack( targetId, mouse->GetKachelOff(), offset, false );
				}
				else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Csteal && selectedVehicle )
				{
					if ( overVehicle ) sendWantComAction ( selectedVehicle->iID, overVehicle->iID, true, true );
					else if ( overPlane && overPlane->FlightHigh == 0 ) sendWantComAction ( selectedVehicle->iID, overVehicle->iID, true, true );
				}
				else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cdisable && selectedVehicle )
				{
					if ( overVehicle ) sendWantComAction ( selectedVehicle->iID, overVehicle->iID, true, false );
					else if ( overPlane && overPlane->FlightHigh == 0 ) sendWantComAction ( selectedVehicle->iID, overPlane->iID, true, false );
					else if ( overBuilding ) sendWantComAction ( selectedVehicle->iID, overBuilding->iID, false, false );
				}
				else if ( MouseStyle == OldSchool && overUnitField && selectUnit ( overUnitField, false ) )
				{}
				else if ( changeAllowed && mouse->cur == GraphicsData.gfx_Cmove && selectedVehicle && !selectedVehicle->moving && !selectedVehicle->Attacking )
				{
					if ( selectedVehicle->IsBuilding )
					{
						int iX, iY;
						mouse->GetKachel ( &iX, &iY );
						sendWantEndBuilding ( selectedVehicle, iX, iY );
					}
					else
					{
						if ( selectedVehiclesGroup.Size() > 1 ) Client->startGroupMove();
						else Client->addMoveJob( selectedVehicle, mouse->GetKachelOff() );
					}
				}
				else if ( overUnitField )
				{
					// open unit menu
					if ( changeAllowed && selectedVehicle && ( overPlane == selectedVehicle || overVehicle == selectedVehicle ) )
					{
						if ( !selectedVehicle->moving && selectedVehicle->owner == player )
						{
							selectedVehicle->MenuActive = !selectedVehicle->MenuActive;
							if ( selectedVehicle->MenuActive ) selectedVehicle->selMenuNr = -1;
							PlayFX ( SoundData.SNDHudButton );
						}
					}
					else if ( changeAllowed && selectedBuilding&& ( overBaseBuilding == selectedBuilding || overBuilding == selectedBuilding ) )
					{
						if ( selectedBuilding->owner == player )
						{
							selectedBuilding->MenuActive = !selectedBuilding->MenuActive;
							if ( selectedBuilding->MenuActive ) selectedBuilding->selMenuNr = -1;
							PlayFX ( SoundData.SNDHudButton );
						}
					}
					// select unit when using modern style
					else if ( MouseStyle == Modern ) selectUnit ( overUnitField, true );
				}
		}
		else if ( overUnitField )
		{
			if ( overPlane )
			{
				cUnitHelpMenu helpMenu ( &overPlane->data, overPlane->owner );
				helpMenu.show();
			}
			else if ( overVehicle )
			{
				cUnitHelpMenu helpMenu ( &overVehicle->data, overVehicle->owner );
				helpMenu.show();
			}
			else if ( overBuilding )
			{
				cUnitHelpMenu helpMenu ( &overBuilding->data, overBuilding->owner );
				helpMenu.show();
			}
			else if ( overBaseBuilding )
			{
				cUnitHelpMenu helpMenu ( &overBaseBuilding->data, overBaseBuilding->owner );
				helpMenu.show();
			}
			helpActive = false;
		}

		// toggle the lock state of an enemy unit, if it is newly selected / deselected
		if ( overUnitField && lockChecked() )
		{
			if (selectedVehicle && selectedVehicle != oldSelectedVehicleForLock && selectedVehicle->owner != player)
				player->ToggelLock ( overUnitField );
			else if (selectedBuilding && selectedBuilding != oldSelectedBuildingForLock && selectedBuilding->owner != player)
				player->ToggelLock ( overUnitField );
		}

		mouseBox.startX = mouseBox.startY = -1;
		mouseBox.endX = mouseBox.endY = -1;
	}
	else if ( mouseState.leftButtonPressed && !mouseState.rightButtonPressed && mouseBox.startX == -1 && mouse->x > HUD_LEFT_WIDTH && mouse->y > 20 )
	{
		mouseBox.startX = (float)( ( ( mouse->x-HUD_LEFT_WIDTH ) + (offX*getZoom()) ) / getTileSize() );
		mouseBox.startY = (float)( ( ( mouse->y-HUD_TOP_HIGHT ) + (offY*getZoom()) ) / getTileSize() );
	}

	// check getZoom() via mousewheel
	if ( mouseState.wheelUp ) setZoom ( getZoom()+(float)0.05, true );
	else if ( mouseState.wheelDown ) setZoom ( getZoom()-(float)0.05, true );
}

void cGameGUI::doScroll( int dir )
{
	int step;
	step = SettingsData.iScrollSpeed;
	switch ( dir )
	{
		case 1:
			offX -= step;
			offY += step;
			break;
		case 2:
			offY += step;
			break;
		case 3:
			offX += step;
			offY += step;
			break;
		case 4:
			offX -= step;
			break;
		case 6:
			offX += step;
			break;
		case 7:
			offX -= step;
			offY -= step;
			break;
		case 8:
			offY -= step;
			break;
		case 9:
			offX += step;
			offY -= step;
			break;
	}

	if ( offX < 0 ) offX = 0;
	if ( offY < 0 ) offY = 0;
	int maxX = map->size*64-(int)((SettingsData.iScreenW-HUD_TOTAL_WIDTH)/getZoom());
	int maxY = map->size*64-(int)((SettingsData.iScreenH-HUD_TOTAL_HIGHT)/getZoom());
	if ( offX > maxX ) offX = maxX;
	if ( offY > maxY ) offY = maxY;

	callMiniMapDraw();
}

void cGameGUI::doCommand( string cmd )
{
	if ( cmd.compare( "/fps on" ) == 0 ) { showFPS = true; return;}
	if ( cmd.compare( "/fps off" ) == 0 ) { showFPS = false; return;}
	if ( cmd.compare( "/base client" ) == 0 ) { debugBaseClient = true; debugBaseServer = false; return; }
	if ( cmd.compare( "/base server" ) == 0 ) { if (Server) debugBaseServer = true; debugBaseClient = false; return; }
	if ( cmd.compare( "/base off" ) == 0 ) { debugBaseServer = false; debugBaseClient = false; return; }
	if ( cmd.compare( "/sentry server" ) == 0 ) { if (Server) debugSentry = true; return; }
	if ( cmd.compare( "/sentry off" ) == 0 ) { debugSentry = false; return; }
	if ( cmd.compare( "/fx on" ) == 0 ) { debugFX = true; return; }
	if ( cmd.compare( "/fx off" ) == 0 ) { debugFX = false; return; }
	if ( cmd.compare( "/trace server" ) == 0 ) { if ( Server ) debugTraceServer = true; debugTraceClient = false; return; }
	if ( cmd.compare( "/trace client" ) == 0 ) { debugTraceClient = true; debugTraceServer = false; return; }
	if ( cmd.compare( "/trace off" ) == 0 ) { debugTraceServer = false; debugTraceClient = false; return; }
	if ( cmd.compare( "/ajobs on" ) == 0 ) { debugAjobs = true; return; }
	if ( cmd.compare( "/ajobs off" ) == 0 ) { debugAjobs = false; return; }
	if ( cmd.compare( "/checkpos on" ) == 0 && Server ) { Server->bDebugCheckPos = true; return; }
	if ( cmd.compare( "/checkpos off") == 0 && Server ) { Server->bDebugCheckPos = false; return; }
	if ( cmd.compare( "/checkpos" ) == 0 && Server ) { sendCheckVehiclePositions(); return; }
	if ( cmd.compare( "/players on" ) == 0 ) { debugPlayers = true; return; }
	if ( cmd.compare( "/players off" ) == 0 ) { debugPlayers = false; return; }
	if ( cmd.substr( 0, 12 ).compare( "/cache size " ) == 0 )
	{
		int size = atoi ( cmd.substr ( 12, cmd.length() ).c_str() );
		//since atoi is too stupid to report an error, do an extra check, when the number is 0
		if ( size == 0 && cmd[12] != '0' ) return;

		getDCache()->setMaxCacheSize( size );
	}
	if ( cmd.compare("/cache flush") == 0 )
	{
		getDCache()->flush();
	}
	if ( cmd.compare("/cache debug on") == 0 )
	{
		debugCache = true;
	}
	if ( cmd.compare("/cache debug off") == 0 )
	{
		debugCache = false;
	}

	if ( cmd.substr( 0, 6 ).compare( "/kick " ) == 0 )
	{
		if ( cmd.length() > 6 && Server )
		{
			int playerNum = -1;
			//first try to find player by name
			for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
			{
				if ( (*Server->PlayerList)[i]->name.compare( cmd.substr ( 6, cmd.length() )) == 0 )
				{
					playerNum = (*Server->PlayerList)[i]->Nr;
				}
			}
			//then by number
			if ( playerNum == -1 )
			{
				playerNum = atoi ( cmd.substr ( 6, cmd.length() ).c_str() );
			}

			//server cannot be kicked
			if ( playerNum == 0 ) return;

			cPlayer *Player = Server->getPlayerFromNumber ( playerNum );
			if ( !Player ) return;

			// close the socket
			if ( network ) network->close ( Player->iSocketNum );
			for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
			{
				if ( (*Server->PlayerList)[i]->iSocketNum > Player->iSocketNum && (*Server->PlayerList)[i]->iSocketNum < MAX_CLIENTS ) (*Server->PlayerList)[i]->iSocketNum--;
			}
			// delete the player
			Server->deletePlayer ( Player );
		}
	}
	if ( cmd.substr( 0, 9 ).compare( "/credits " ) == 0 )
	{
		if ( cmd.length() > 9 && Server )
		{
			int playerNum = -1;
			string playerStr = cmd.substr ( 9, cmd.find_first_of ( " ", 9 )-9 );
			string creditsStr = cmd.substr ( cmd.find_first_of ( " ", 9 )+1, cmd.length() );
			//first try to find player by name
			for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
			{
				if ( (*Server->PlayerList)[i]->name.compare( playerStr ) == 0 )
				{
					playerNum = (*Server->PlayerList)[i]->Nr;
				}
			}
			//then by number
			if ( playerNum == -1 )
			{
				playerNum = atoi ( playerStr.c_str() );
			}

			//since atoi is too stupid to report an error, do an extra check, when the number is 0
			if ( playerNum == 0 ) return;

			cPlayer *Player = Server->getPlayerFromNumber ( playerNum );
			if ( !Player ) return;

			int credits = atoi ( creditsStr.c_str() );

			Player->Credits = credits;

			sendCredits ( credits, Player->Nr );
		}
	}
	if ( cmd.substr( 0, 12 ).compare( "/disconnect " ) == 0 )
	{
		if ( cmd.length() > 12 && Server )
		{
			int playerNum = -1;
			//first try to find player by name
			for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
			{
				if ( (*Server->PlayerList)[i]->name.compare( cmd.substr ( 12, cmd.length() )) == 0 )
				{
					playerNum = (*Server->PlayerList)[i]->Nr;
				}
			}
			//then by number
			if ( playerNum == -1 )
			{
				playerNum = atoi ( cmd.substr ( 12, cmd.length() ).c_str() );
			}

			//server cannot be disconnected
			if ( playerNum == 0 ) return;

			cPlayer *Player = Server->getPlayerFromNumber ( playerNum );
			if ( !Player ) return;

			//can not disconnect local players
			if ( Player->iSocketNum == MAX_CLIENTS ) return;

			SDL_Event* event = new SDL_Event;
			event->type = NETWORK_EVENT;
			event->user.code = TCP_CLOSEEVENT;
			event->user.data1 = new Sint16[1];
			((Sint16*)event->user.data1)[0] = Player->iSocketNum;
			Server->pushEvent ( event );
		}
	}
	if ( cmd.substr( 0, 9 ).compare( "/deadline"  ) == 0 )
	{
		if(cmd.length() > 9  && Server)
		{
			int i = 90;
			i = atoi ( cmd.substr ( 9, cmd.length() ).c_str());
			Server->setDeadline(i);
			Log.write("Deadline changed to "  + iToStr( i ) , cLog::eLOG_TYPE_INFO);
		}
		return;
	}
	if ( cmd.substr( 0, 7 ).compare( "/resync" ) == 0 )
	{
		if ( cmd.length() > 7 && Server )
		{
			unsigned int playernum = atoi ( cmd.substr ( 7, 8 ).c_str() );
			sendRequestResync( playernum );
		}
		else
		{
			if ( Server )
			{
				for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
				{
					sendRequestResync( 	(*Server->PlayerList)[i]->Nr );
				}
			}
			else
			{
				sendRequestResync( Client->ActivePlayer->Nr );
			}
		}
		return;
	}
	if ( cmd.substr( 0, 5 ).compare( "/mark"  ) == 0 )
	{
		cmd.erase(0, 5 );
		cNetMessage* message = new cNetMessage( GAME_EV_WANT_MARK_LOG );
		message->pushString( cmd );
		Client->sendNetMessage( message );
		return;
	}
	if ( cmd.substr( 0, 7 ).compare( "/color " ) == 0 ) {
		int cl=0;sscanf ( cmd.c_str(),"color %d",&cl );cl%=8;player->color=OtherData.colors[cl];return;}
	if ( cmd.compare( "/fog off" ) == 0 && Server )
	{
		memset ( Server->getPlayerFromNumber(player->Nr)->ScanMap,1,map->size*map->size );
		memset ( player->ScanMap,1,map->size*map->size );
		return;
	}

	if ( cmd.compare( "/survey" ) == 0 )
	{
		if ( network && !network->isHost() ) return;
		memcpy ( map->Resources , Server->Map->Resources, map->size*map->size*sizeof ( sResources ) );
		memset ( player->ResourceMap,1,map->size*map->size );
		return;
	}

	if ( cmd.compare( "/credits" ) == 0 )
	{
		return;
	}
	if ( cmd.substr( 0, 5 ).compare( "/kill " ) == 0 )
	{
		int x,y;
		sscanf ( cmd.c_str(),"kill %d,%d",&x,&y );
		/*engine->DestroyObject ( x+y*map->size,false );
		engine->DestroyObject ( x+y*map->size,true );*/
		return;
	}
	if ( cmd.compare( "/load" ) == 0 )
	{
		/*if ( SelectedVehicle ) {SelectedVehicle->data.cargo=SelectedVehicle->data.max_cargo;SelectedVehicle->data.ammoCur=SelectedVehicle->data.ammoMax;SelectedVehicle->ShowDetails();}
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
			SelectedBuilding->data.ammoCur=SelectedBuilding->data.ammoMax;SelectedBuilding->ShowDetails();
		}*/
		return;
	}
}

void cGameGUI::setWind( int dir )
{
	windDir = (float)(dir/57.29577);
}

bool cGameGUI::selectUnit( cMapField *OverUnitField, bool base )
{
	deselectGroup();
	if ( OverUnitField->getPlanes() && !OverUnitField->getPlanes()->moving )
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if ( selectedVehicle == OverUnitField->getPlanes() )
		{
			if ( selectedVehicle->owner == player )
			{
				selectedVehicle->MenuActive = !selectedVehicle->MenuActive;
				PlayFX ( SoundData.SNDHudButton );
			}
		}
		else
		{
			if ( selectedVehicle )
			{
				selectedVehicle->Deselct();
				selectedVehicle = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->Deselct();
				selectedBuilding = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			selectedVehicle = OverUnitField->getPlanes();
			selectedVehicle->Select();
			Client->iObjectStream = selectedVehicle->playStream();
		}
		return true;
	}
	else if ( OverUnitField->getVehicles() && !OverUnitField->getVehicles()->moving && !( OverUnitField->getPlanes() && ( OverUnitField->getVehicles()->MenuActive || OverUnitField->getVehicles()->owner != player ) ) )
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if ( selectedVehicle == OverUnitField->getVehicles() )
		{
			if ( selectedVehicle->owner == player )
			{
				selectedVehicle->MenuActive = !selectedVehicle->MenuActive;
				PlayFX ( SoundData.SNDHudButton );
			}
		}
		else
		{
			if ( selectedVehicle )
			{
				selectedVehicle->Deselct();
				selectedVehicle = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->Deselct();
				selectedBuilding = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			selectedVehicle = OverUnitField->getVehicles();
			selectedVehicle->Select();
			Client->iObjectStream = selectedVehicle->playStream();
		}
		return true;
	}
	else if ( OverUnitField->getTopBuilding() && ( base || ( ( OverUnitField->getTopBuilding()->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE || !selectedVehicle ) && ( !OverUnitField->getTopBuilding()->data.canBeLandedOn || ( !selectedVehicle || selectedVehicle->data.factorAir == 0 ) ) ) ) )
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if ( selectedBuilding == OverUnitField->getTopBuilding() )
		{
			if ( selectedBuilding->owner == player )
			{
				selectedBuilding->MenuActive = !selectedBuilding->MenuActive;
				PlayFX ( SoundData.SNDHudButton );
			}
		}
		else
		{
			if ( selectedVehicle )
			{
				selectedVehicle->Deselct();
				selectedVehicle = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->Deselct();
				selectedBuilding = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			selectedBuilding = OverUnitField->getTopBuilding();
			selectedBuilding->Select();
			Client->iObjectStream = selectedBuilding->playStream();
		}
		return true;
	}
	else if ( ( base || !selectedVehicle )&& OverUnitField->getBaseBuilding() && OverUnitField->getBaseBuilding()->owner != NULL )
	{
		// TOFIX: add that the unit renaming will be aborted here when active
		if ( selectedBuilding == OverUnitField->getBaseBuilding() )
		{
			if ( selectedBuilding->owner == player )
			{
				selectedBuilding->MenuActive = !selectedBuilding->MenuActive;
				PlayFX ( SoundData.SNDHudButton );
			}
		}
		else
		{
			if ( selectedVehicle )
			{
				selectedVehicle->Deselct();
				selectedVehicle = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			else if ( selectedBuilding )
			{
				selectedBuilding->Deselct();
				selectedBuilding = NULL;
				StopFXLoop ( Client->iObjectStream );
			}
			selectedBuilding = OverUnitField->getBaseBuilding();
			selectedBuilding->Select();
			Client->iObjectStream = selectedBuilding->playStream();
		}
		return true;
	}
	return false;
}

void cGameGUI::selectBoxVehicles ( sMouseBox &box )
{
	if ( box.startX < 0 || box.startY < 0 || box.endX < 0 || box.endY < 0 ) return;
	int startFieldX, startFieldY;
	int endFieldX, endFieldY;
	startFieldX = (int)min ( box.startX, box.endX );
	startFieldY = (int)min ( box.startY, box.endY );
	endFieldX = (int)max ( box.startX, box.endX );
	endFieldY = (int)max ( box.startY, box.endY );

	deselectGroup();

	bool newSelected = true;
	for ( int x = startFieldX; x <= endFieldX; x++ )
	{
		for ( int y = startFieldY; y <= endFieldY; y++ )
		{
			int offset = x+y*map->size;

			cVehicle *vehicle;
			vehicle = (*map)[offset].getVehicles();
			if ( !vehicle || vehicle->owner != player ) vehicle = (*map)[offset].getPlanes();

			if ( vehicle && vehicle->owner == player && !vehicle->IsBuilding && !vehicle->IsClearing && !vehicle->moving )
			{
				if ( vehicle == selectedVehicle )
				{
					newSelected = false;
					selectedVehiclesGroup.Insert( 0, vehicle );
				}
				else selectedVehiclesGroup.Add ( vehicle );
				vehicle->groupSelected = true;
			}
		}
	}
	if ( newSelected && selectedVehiclesGroup.Size() > 0 )
	{
		if ( selectedVehicle ) selectedVehicle->Deselct();
		selectedVehicle = selectedVehiclesGroup[0];
		selectedVehicle->Select();
	}
	if ( selectedVehiclesGroup.Size() == 1 )
	{
		selectedVehiclesGroup[0]->groupSelected = false;
		selectedVehiclesGroup.Delete( 0 );
	}

	if ( selectedBuilding )
	{
		selectedBuilding->Deselct();
		selectedBuilding = NULL;
	}
}

void cGameGUI::updateStatusText()
{
	if ( selectedVehicle ) selUnitStatusStr->setText ( selectedVehicle->getStatusStr() );
	else if ( selectedBuilding ) selUnitStatusStr->setText ( selectedBuilding->getStatusStr() );
	else selUnitStatusStr->setText ( "" );
}

void cGameGUI::deselectGroup ()
{
	while ( selectedVehiclesGroup.Size() )
	{
		selectedVehiclesGroup[0]->groupSelected = false;
		selectedVehiclesGroup.Delete ( 0 );
	}
}

void cGameGUI::changeWindDir()
{
	if ( Client->timer400ms && SettingsData.bDamageEffects )
	{
		static int nextChange = 25, nextDirChange = 25, dir = 90, change = 3;
		if ( nextChange == 0 )
		{
			nextChange = 10 + random(20);
			dir += change;
			setWind ( dir );
			if ( dir >= 360 ) dir -= 360;
			else if ( dir < 0 ) dir += 360;

			if ( nextDirChange == 0 )
			{
				nextDirChange = random(25) + 10;
				change        = random(11) -  5;
			}
			else nextDirChange--;

		}
		else nextChange--;
	}
}

bool cGameGUI::loadPanelGraphics()
{
	if ( !panelTopGraphic ) panelTopGraphic = LoadPCX ( SettingsData.sGfxPath + PATH_DELIMITER + "panel_top.pcx" );
	if ( !panelBottomGraphic ) panelBottomGraphic = LoadPCX ( SettingsData.sGfxPath + PATH_DELIMITER + "panel_top.pcx" );

	if ( !panelTopGraphic || !panelBottomGraphic ) return false;
	return true;
}

void cGameGUI::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	// first check whether the end key was pressed
	if ( ( activeItem != chatBox || chatBox->isDisabled() ) && key.keysym.sym == KeysList.KeyEndTurn && !Client->bWaitForOthers )
	{
		if ( key.state == SDL_PRESSED && !endButton->getIsClicked() ) endButton->clicked ( this );
		else if ( key.state == SDL_RELEASED && endButton->getIsClicked() && !Client->bWantToEnd ) endButton->released ( this );
		return;
	}

	// we will handle only pressed keys for all other hotkeys
	if ( key.state != SDL_PRESSED ) return;

	// check whether the player wants to abort waiting
	if ( Client->bWaitForOthers && Client->waitReconnect && key.keysym.sym == SDLK_F2 )
	{
		sendAbortWaiting ();
	}

	if ( key.keysym.sym == KeysList.KeyExit )
	{
		cDialogYesNo yesNoDialog(lngPack.i18n("Text~Comp~End_Game"));
		if ( yesNoDialog.show() == 0  ) end = true;
	}
	else if ( activeItem && !activeItem->isDisabled() && activeItem->handleKeyInput ( key.keysym, ch, this ) )
	{}
	else if ( key.keysym.sym == KeysList.KeyJumpToAction )
	{
		if ( Client->iMsgCoordsX != -1 )
		{
			offX = Client->iMsgCoordsX * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenW - HUD_TOTAL_WIDTH) / (2 * getTileSize() ) ) * 64 ) ) + 32;
			offY = Client->iMsgCoordsY * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenH - HUD_TOTAL_HIGHT ) / (2 * getTileSize() ) ) * 64 ) ) + 32;
			Client->iMsgCoordsX = -1;
		}
	}
	else if ( key.keysym.sym == KeysList.KeyChat )
	{
		if ( !(key.keysym.mod & KMOD_ALT) )
		{
			if ( chatBox->isDisabled() ) chatBox->setDisabled( false );
			activeItem = chatBox;
			chatBox->setActivity ( true );
		}
	}
	// scroll and getZoom() hotkeys
	else if ( key.keysym.sym == KeysList.KeyScroll1 ) doScroll ( 1 );
	else if ( key.keysym.sym == KeysList.KeyScroll3 ) doScroll ( 3 );
	else if ( key.keysym.sym == KeysList.KeyScroll7 ) doScroll ( 7 );
	else if ( key.keysym.sym == KeysList.KeyScroll9 ) doScroll ( 9 );
	else if ( key.keysym.sym == KeysList.KeyScroll2a || key.keysym.sym == KeysList.KeyScroll2b ) doScroll ( 2 );
	else if ( key.keysym.sym == KeysList.KeyScroll4a || key.keysym.sym == KeysList.KeyScroll4b ) doScroll ( 4 );
	else if ( key.keysym.sym == KeysList.KeyScroll6a || key.keysym.sym == KeysList.KeyScroll6b ) doScroll ( 6 );
	else if ( key.keysym.sym == KeysList.KeyScroll8a || key.keysym.sym == KeysList.KeyScroll8b ) doScroll ( 8 );
	else if ( key.keysym.sym == KeysList.KeyZoomIna || key.keysym.sym == KeysList.KeyZoomInb ) setZoom ( (float)(getZoom()+0.05), true );
	else if ( key.keysym.sym == KeysList.KeyZoomOuta || key.keysym.sym == KeysList.KeyZoomOutb ) setZoom ( (float)(getZoom()-0.05), true );
	// position handling hotkeys
	else if ( key.keysym.sym == KeysList.KeyCenterUnit && selectedVehicle ) selectedVehicle->Center();
	else if ( key.keysym.sym == KeysList.KeyCenterUnit && selectedBuilding ) selectedBuilding->Center();
	else if ( key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_ALT )
	{
		savedPositions[0].offsetX = offX;
		savedPositions[0].offsetY = offY;
	}
	else if ( key.keysym.sym == SDLK_F6 && key.keysym.mod & KMOD_ALT )
	{
		savedPositions[1].offsetX = offX;
		savedPositions[1].offsetY = offY;
	}
	else if ( key.keysym.sym == SDLK_F7 && key.keysym.mod & KMOD_ALT )
	{
		savedPositions[2].offsetX = offX;
		savedPositions[2].offsetY = offY;
	}
	else if ( key.keysym.sym == SDLK_F8 && key.keysym.mod & KMOD_ALT )
	{
		savedPositions[3].offsetX = offX;
		savedPositions[3].offsetY = offY;
	}
	else if ( key.keysym.sym == SDLK_F5 && savedPositions[0].offsetX >= 0 && savedPositions[0].offsetY >= 0 )
	{
		offX = savedPositions[0].offsetX;
		offY = savedPositions[0].offsetY;
	}
	else if ( key.keysym.sym == SDLK_F6 && savedPositions[1].offsetX >= 0 && savedPositions[1].offsetY >= 0 )
	{
		offX = savedPositions[1].offsetX;
		offY = savedPositions[1].offsetY;
	}
	else if ( key.keysym.sym == SDLK_F7 && savedPositions[2].offsetX >= 0 && savedPositions[2].offsetY >= 0 )
	{
		offX = savedPositions[2].offsetX;
		offY = savedPositions[2].offsetY;
	}
	else if ( key.keysym.sym == SDLK_F8 && savedPositions[3].offsetX >= 0 && savedPositions[3].offsetY >= 0 )
	{
		offX = savedPositions[3].offsetX;
		offY = savedPositions[3].offsetY;
	}
	// Hotkeys for the unit menues
	else if ( key.keysym.sym == KeysList.KeyUnitMenuAttack && selectedVehicle && selectedVehicle->data.canAttack && selectedVehicle->data.shotsCur && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->AttackMode = true;
		updateMouseCursor();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuAttack && selectedBuilding && selectedBuilding->data.canAttack && selectedBuilding->data.shotsCur && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		selectedBuilding->AttackMode = true;
		updateMouseCursor();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuBuild && selectedVehicle && !selectedVehicle->data.canBuild.empty() && !selectedVehicle->IsBuilding && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		sendWantStopMove ( selectedVehicle->iID );
		cBuildingsBuildMenu buildMenu ( player, selectedVehicle );
		buildMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuBuild && selectedBuilding && !selectedBuilding->data.canBuild.empty() && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		cVehiclesBuildMenu buildMenu ( player, selectedBuilding );
		buildMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuTransfer && selectedVehicle && selectedVehicle->data.storeResType != sUnitData::STORE_RES_NONE && !selectedVehicle->IsBuilding && !selectedVehicle->IsClearing && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->Transfer = true;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuTransfer && selectedBuilding && selectedBuilding->data.storeResType != sUnitData::STORE_RES_NONE && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		selectedBuilding->Transfer = true;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuAutomove && selectedVehicle && selectedVehicle->data.canSurvey && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		if ( selectedVehicle->autoMJob == NULL ) selectedVehicle->autoMJob = new cAutoMJob ( selectedVehicle );
		else
		{
			delete selectedVehicle->autoMJob;
			selectedVehicle->autoMJob = NULL;
		}
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuStart && selectedBuilding && selectedBuilding->data.canWork && !selectedBuilding->IsWorking && ( (selectedBuilding->BuildList && selectedBuilding->BuildList->Size()) || selectedBuilding->data.canBuild.empty() ) && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		sendWantStartWork( selectedBuilding );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuStop && selectedVehicle && ( selectedVehicle->ClientMoveJob || ( selectedVehicle->IsBuilding && selectedVehicle->BuildRounds ) || ( selectedVehicle->IsClearing && selectedVehicle->ClearingRounds ) ) && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		if ( selectedVehicle->ClientMoveJob ) sendWantStopMove ( selectedVehicle->iID );
		else if ( selectedVehicle->IsBuilding ) sendWantStopBuilding ( selectedVehicle->iID );
		else if ( selectedVehicle->IsClearing ) sendWantStopClear ( selectedVehicle );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuStop && selectedBuilding && selectedBuilding->IsWorking && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		sendWantStopWork( selectedBuilding );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuClear && selectedVehicle && selectedVehicle->data.canClearArea && map->fields[selectedVehicle->PosX+selectedVehicle->PosY*map->size].getRubble() && !selectedVehicle->IsClearing && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		sendWantStartClear ( selectedVehicle );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuSentry && selectedVehicle && ( selectedVehicle->bSentryStatus || selectedVehicle->data.canAttack ) && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		sendChangeSentry ( selectedVehicle->iID, true );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuSentry && selectedBuilding && ( selectedBuilding->bSentryStatus || selectedBuilding->data.canAttack ) && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		sendChangeSentry ( selectedBuilding->iID, false );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuActivate && selectedVehicle && selectedVehicle->data.storageUnitsMax > 0 && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		cStorageMenu storageMenu ( selectedVehicle->StoredVehicles, selectedVehicle, NULL );
		storageMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuActivate && selectedBuilding && selectedBuilding->data.storageUnitsMax > 0 && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		cStorageMenu storageMenu ( selectedBuilding->StoredVehicles, NULL, selectedBuilding );
		storageMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuLoad && selectedVehicle && selectedVehicle->data.storageUnitsMax > 0 && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->LoadActive = !selectedVehicle->LoadActive;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuLoad && selectedBuilding && selectedBuilding->data.storageUnitsMax > 0 && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		selectedBuilding->LoadActive = !selectedBuilding->LoadActive;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuReload && selectedVehicle && selectedVehicle->data.canRearm && selectedVehicle->data.storageResCur >= 2 && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->MuniActive = true;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuRepair && selectedVehicle && selectedVehicle->data.canRepair && selectedVehicle->data.storageResCur >= 2 && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->RepairActive = true;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuLayMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur > 0 && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->LayMines = !selectedVehicle->LayMines;
		selectedVehicle->ClearMines = false;
		sendMineLayerStatus( selectedVehicle );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuClearMine && selectedVehicle && selectedVehicle->data.canPlaceMines && selectedVehicle->data.storageResCur < selectedVehicle->data.storageResMax && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->ClearMines = !selectedVehicle->ClearMines;
		selectedVehicle->LayMines = false;
		sendMineLayerStatus ( selectedVehicle );
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuDisable && selectedVehicle && selectedVehicle->data.canDisable && selectedVehicle->data.shotsCur && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->DisableActive = true;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuSteal && selectedVehicle && selectedVehicle->data.canCapture && selectedVehicle->data.shotsCur && !Client->bWaitForOthers && selectedVehicle->owner == player )
	{
		selectedVehicle->StealActive = true;
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuInfo && selectedVehicle )
	{
		cUnitHelpMenu helpMenu ( &selectedVehicle->data, selectedVehicle->owner );
		helpMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuInfo && selectedBuilding )
	{
		cUnitHelpMenu helpMenu ( &selectedBuilding->data, selectedBuilding->owner );
		helpMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuDistribute && selectedBuilding && selectedBuilding->data.canMineMaxRes > 0 && selectedBuilding->IsWorking && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		cMineManagerMenu mineManager ( selectedBuilding );
		mineManager.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuResearch && selectedBuilding && selectedBuilding->data.canResearch && selectedBuilding->IsWorking && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		cDialogResearch researchDialog ( selectedBuilding->owner );
		researchDialog.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuUpgrade && selectedBuilding && selectedBuilding->data.convertsGold && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		cUpgradeMenu upgradeMenu ( selectedBuilding->owner );
		upgradeMenu.show();
	}
	else if ( key.keysym.sym == KeysList.KeyUnitMenuDestroy && selectedBuilding && selectedBuilding->data.canSelfDestroy && !Client->bWaitForOthers && selectedBuilding->owner == player )
	{
		Client->addMessage ( lngPack.i18n ( "Text~Error_Messages~INFO_Not_Implemented" ) );
	}
	// Hotkeys for the hud
	else if ( key.keysym.sym == KeysList.KeyFog ) setFog ( !fogChecked() );
	else if ( key.keysym.sym == KeysList.KeyGrid ) setGrid ( !gridChecked() );
	else if ( key.keysym.sym == KeysList.KeyScan ) setScan ( !scanChecked() );
	else if ( key.keysym.sym == KeysList.KeyRange ) setRange ( !rangeChecked() );
	else if ( key.keysym.sym == KeysList.KeyAmmo ) setAmmo ( !ammoChecked() );
	else if ( key.keysym.sym == KeysList.KeyHitpoints ) setHits ( !hitsChecked() );
	else if ( key.keysym.sym == KeysList.KeyColors ) setColor ( !colorChecked() );
	else if ( key.keysym.sym == KeysList.KeyStatus ) setStatus ( !statusChecked() );
	else if ( key.keysym.sym == KeysList.KeySurvey ) setSurvey ( !surveyChecked() );
}

void cGameGUI::helpReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->helpActive = !gui->helpActive;
	gui->updateMouseCursor();
}

void cGameGUI::centerReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	if ( gui->selectedVehicle ) gui->selectedVehicle->Center();
	else if ( gui->selectedBuilding ) gui->selectedBuilding->Center();
}

void cGameGUI::reportsReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	cReportsMenu reportMenu ( gui->player );
	reportMenu.show();
}

void cGameGUI::chatReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->chatBox->setDisabled ( !gui->chatBox->isDisabled() );
	if ( gui->activeItem ) gui->activeItem->setActivity ( false );
	gui->activeItem = gui->chatBox;
	gui->chatBox->setActivity ( true );
}

void cGameGUI::nextReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	cVehicle *v = gui->player->GetNextVehicle();
	if ( v )
	{
		if ( gui->selectedVehicle )
		{
			gui->selectedVehicle->Deselct();
			StopFXLoop ( Client->iObjectStream );
		}
		v->Select();
		v->Center();
		Client->iObjectStream = v->playStream();
		gui->selectedVehicle = v;
	}
}

void cGameGUI::prevReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	cVehicle *v = gui->player->GetPrevVehicle();
	if ( v )
	{
		if ( gui->selectedVehicle )
		{
			gui->selectedVehicle->Deselct();
			StopFXLoop ( Client->iObjectStream );
		}
		v->Select();
		v->Center();
		Client->iObjectStream = v->playStream();
		gui->selectedVehicle = v;
	}
}

void cGameGUI::doneReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	if ( gui->selectedVehicle && gui->selectedVehicle->ClientMoveJob && gui->selectedVehicle->ClientMoveJob->bSuspended && gui->selectedVehicle->data.speedCur )
	{
		gui->selectedVehicle->ClientMoveJob->calcNextDir();
	}
}

void cGameGUI::changedMiniMap( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->callMiniMapDraw();
}

void cGameGUI::miniMapClicked( void *parent )
{
	static int lastX = 0, lastY = 0;
	int x = mouse->x;
	int y = mouse->y;
	if ( lastX == x && lastY == y ) return;

	cGameGUI *gui = static_cast<cGameGUI*>(parent);

	gui->offX = gui->miniMapOffX * 64 + ((x - MINIMAP_POS_X) * gui->map->size * 64) / (MINIMAP_SIZE * (gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1) );
	gui->offY = gui->miniMapOffY * 64 + ((y - MINIMAP_POS_Y) * gui->map->size * 64) / (MINIMAP_SIZE * (gui->twoXChecked() ? MINIMAP_ZOOM_FACTOR : 1) );
	gui->offX -= (int)( (SettingsData.iScreenW - HUD_TOTAL_WIDTH) / (gui->getZoom() * 2.0) );
	gui->offY -= (int)( (SettingsData.iScreenH -  HUD_TOTAL_HIGHT) / (gui->getZoom() * 2.0) );

	//check map borders
	gui->offX = max ( gui->offX, 0 );
	gui->offY = max ( gui->offY, 0 );
	gui->offX = min ( gui->offX, gui->map->size * 64 - (int)((SettingsData.iScreenW - HUD_TOTAL_WIDTH) / gui->getZoom()) );
	gui->offY = min ( gui->offY, gui->map->size * 64 - (int)((SettingsData.iScreenH -  HUD_TOTAL_HIGHT) / gui->getZoom()) );

	//workaround for click and hold on the minimap while it is zoomed:
	//we warp the mouse so that it stays over the position of the screen
	//does not work as intended in some cases --Eiko
	/*int lastMinimapOffsetX = gui->miniMapOffX;
	int lastMinimapOffsetY = gui->miniMapOffY;
	if ( lastMinimapOffsetX != minimapOffsetX )
	{
		x = MINIMAP_POS_X + MINIMAP_SIZE/2;
	}
	if ( lastMinimapOffsetY != minimapOffsetY )
	{
		y = MINIMAP_POS_Y - 1 + MINIMAP_SIZE/2;
	}
	SDL_WarpMouse( x, y );*/
	lastX = x;
	lastY = y;

	gui->doScroll ( 0 );
}

void cGameGUI::miniMapMovedOver( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	if ( gui->miniMapImage->getIsClicked() )
	{
		gui->miniMapClicked ( parent );
	}
}

void cGameGUI::zoomSliderMoved( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->setZoom ( gui->zoomSlider->getValue(), false );
}

void cGameGUI::endReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->endButton->setLocked( true );
	Client->handleEnd();
}

void cGameGUI::preferencesReleased( void *parent )
{
	cDialogPreferences preferencesDialog;
	preferencesDialog.show();
}

void cGameGUI::filesReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	cLoadSaveMenu loadSaveMenu ( new cGameDataContainer ); // TODO: memory leak?
	if ( loadSaveMenu.show() != 1 ) gui->end = true;
}

void cGameGUI::playReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->playFLC = true;
}

void cGameGUI::stopReleased( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	gui->playFLC = false;
}

void cGameGUI::chatBoxReturnPressed( void *parent )
{
	cGameGUI *gui = static_cast<cGameGUI*>(parent);
	string chatString = gui->chatBox->getText();
	if ( !chatString.empty() )
	{
		if ( chatString[0] == '/' ) gui->doCommand( chatString );
		else sendChatMessageToServer( gui->player->name + ": " + chatString );
		gui->chatBox->setText ( "" );
	}
	gui->chatBox->setActivity ( false );
	gui->activeItem = NULL;
	gui->chatBox->setDisabled ( true );
}

void cGameGUI::preDrawFunction()
{
	// draw the map screen with everything on it
	int zoomOffX = (int)(offX*getZoom());
	int zoomOffY = (int)(offY*getZoom());

	int startX = ((offX-1)/64)-1 < 0 ? 0 : ((offX-1)/64)-1;
	int startY = ((offY-1)/64)-1 < 0 ? 0 : ((offY-1)/64)-1;

	int endX = Round( offX/64.0 + (float)(SettingsData.iScreenW-HUD_TOTAL_WIDTH) / getTileSize() );
	if ( endX >= map->size ) endX = map->size-1;
	int endY = Round( offY/64.0 + (float)(SettingsData.iScreenH-HUD_TOTAL_HIGHT) / getTileSize() );
	if ( endY >= map->size ) endY = map->size-1;

	if ( Client->timer400ms ) map->generateNextAnimationFrame();

	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, SettingsData.iScreenW - HUD_TOTAL_WIDTH, SettingsData.iScreenH - HUD_TOTAL_HIGHT };
	SDL_SetClipRect( buffer, &clipRect );

	drawTerrain( zoomOffX, zoomOffY );
	if ( gridChecked() ) drawGrid( zoomOffX, zoomOffY );

	displayBottomFX();

	dCache.resetStatistics();

	drawBaseUnits ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	drawTopBuildings ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	drawShips ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	drawAboveSeaBaseUnits ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	drawVehicles ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	drawConnectors ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	drawPlanes ( startX, startY, endX, endY, zoomOffX, zoomOffY );

	if ( surveyChecked() || ( selectedVehicle && selectedVehicle->owner == player && selectedVehicle->data.canSurvey ) )
	{
		drawResources ( startX, startY, endX, endY, zoomOffX, zoomOffY );
	}

	if ( selectedVehicle && ( ( selectedVehicle->ClientMoveJob && selectedVehicle->ClientMoveJob->bSuspended ) || selectedVehicle->BuildPath ) )
	{
		selectedVehicle->DrawPath();
	}

	drawDebugOutput();

	drawSelectionBox( zoomOffX, zoomOffY );

	SDL_SetClipRect( buffer, NULL );

	drawUnitCircles();

	if ( selectedVehicle && selectedVehicle->MenuActive ) selectedVehicle->DrawMenu( &savedMouseState );
	else if ( selectedBuilding && selectedBuilding->MenuActive ) selectedBuilding->DrawMenu( &savedMouseState );

	displayFX();

	displayMessages();
}

void cGameGUI::drawTerrain( int zoomOffX, int zoomOffY )
{
	int tileSize = Client->gameGUI.getTileSize();
	SDL_Rect dest, tmp;
	dest.y = HUD_TOP_HIGHT-zoomOffY;
	// draw the terrain
	struct sTerrain *terr;
	for ( int y = 0; y < map->size; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX;
		if ( dest.y >= HUD_TOP_HIGHT-tileSize )
		{
			int pos = y*map->size;
			for ( int x = 0 ; x < map->size; x++ )
			{
				if ( dest.x >= HUD_LEFT_WIDTH-tileSize )
				{
					tmp = dest;
					terr = map->terrain+map->Kacheln[pos];

					// draw the fog:
					if ( fogChecked() && !player->ScanMap[pos] )
					{
						if ( !SettingsData.bPreScale && ( terr->shw->w != tileSize || terr->shw->h != tileSize ) ) scaleSurface ( terr->shw_org, terr->shw, tileSize, tileSize );
						SDL_BlitSurface ( terr->shw,NULL,buffer,&tmp );
					}
					else
					{
						if ( !SettingsData.bPreScale && ( terr->sf->w != tileSize || terr->sf->h != tileSize ) ) scaleSurface ( terr->sf_org, terr->sf, tileSize, tileSize );
						SDL_BlitSurface ( terr->sf,NULL,buffer,&tmp );
					}
				}
				pos++;
				dest.x += tileSize;
				if ( dest.x > SettingsData.iScreenW-13 ) break;
			}
		}
		dest.y += tileSize;
		if ( dest.y > SettingsData.iScreenH-15 ) break;
	}
}

void cGameGUI::drawGrid( int zoomOffX, int zoomOffY )
{
	int tileSize = Client->gameGUI.getTileSize();
	SDL_Rect dest;
	dest.x = HUD_LEFT_WIDTH;
	dest.y = HUD_TOP_HIGHT+tileSize-(zoomOffY%tileSize);
	dest.w = SettingsData.iScreenW-HUD_TOTAL_WIDTH;
	dest.h = 1;
	for ( int y = 0; y < ( SettingsData.iScreenH-HUD_TOTAL_HIGHT ) / tileSize+1; y++ )
	{
		SDL_FillRect ( buffer, &dest, GRID_COLOR );
		dest.y += tileSize;
	}
	dest.x = HUD_LEFT_WIDTH+tileSize-(zoomOffX%tileSize);
	dest.y = HUD_TOP_HIGHT;
	dest.w = 1;
	dest.h = SettingsData.iScreenH-HUD_TOTAL_HIGHT;
	for ( int x = 0; x < ( SettingsData.iScreenW-HUD_TOTAL_WIDTH ) /tileSize+1; x++ )
	{
		SDL_FillRect ( buffer, &dest, GRID_COLOR );
		dest.x += tileSize;
	}
}

void cGameGUI::displayFX()
{
	if ( !Client->FXList.Size() ) return;

	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, SettingsData.iScreenW - HUD_TOTAL_WIDTH, SettingsData.iScreenH - HUD_TOTAL_HIGHT };
	SDL_SetClipRect( buffer, &clipRect );

	for ( int i = (int)Client->FXList.Size() - 1; i >= 0; i-- )
	{
		drawFX ( i );
	}
	SDL_SetClipRect( buffer, NULL );
}

void cGameGUI::drawFX( int num )
{
	SDL_Rect scr,dest;
	sFX *fx;

	fx = Client->FXList[num];
	if ( !player->ScanMap[fx->PosX/64+fx->PosY/64*map->size] && fx->typ != fxRocket ) return;

	switch ( fx->typ )
	{
		case fxMuzzleBig:
			if ( !EffectsData.fx_muzzle_big ) break;
			CHECK_SCALING( EffectsData.fx_muzzle_big[1], EffectsData.fx_muzzle_big[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 2 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x=(int)(getZoom()*64.0)*fx->param;
			scr.y=0;
			scr.w=(int)(getZoom()*64.0);
			scr.h=(int)(getZoom()*64.0);
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_big[1],&scr,buffer,&dest );
			break;
		case fxMuzzleSmall:
			if ( !EffectsData.fx_muzzle_small ) break;
			CHECK_SCALING( EffectsData.fx_muzzle_small[1], EffectsData.fx_muzzle_small[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 2 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x=(int)(getZoom()*64.0)*fx->param;
			scr.y=0;
			scr.w=(int)(getZoom()*64.0);
			scr.h=(int)(getZoom()*64.0);
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_small[1],&scr,buffer,&dest );
			break;
		case fxMuzzleMed:
			if ( !EffectsData.fx_muzzle_med ) break;
			CHECK_SCALING( EffectsData.fx_muzzle_med[1], EffectsData.fx_muzzle_med[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 2 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x=(int)(getZoom()*64.0)*fx->param;
			scr.y=0;
			scr.w=(int)(getZoom()*64.0);
			scr.h=(int)(getZoom()*64.0);
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_med[1],&scr,buffer,&dest );
			break;
		case fxMuzzleMedLong:
			if ( !EffectsData.fx_muzzle_med ) break;
			CHECK_SCALING( EffectsData.fx_muzzle_med[1], EffectsData.fx_muzzle_med[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 5 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x=(int)(getZoom()*64.0)*fx->param;
			scr.y=0;
			scr.w=(int)(getZoom()*64.0);
			scr.h=(int)(getZoom()*64.0);
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_muzzle_med[1],&scr,buffer,&dest );
			break;
		case fxHit:
			if ( !EffectsData.fx_hit ) break;
			CHECK_SCALING( EffectsData.fx_hit[1], EffectsData.fx_hit[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 5 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x=(int)(getZoom()*64.0)* ((Client->iTimerTime-fx->StartTime)/2);
			scr.y=0;
			scr.w=(int)(getZoom()*64.0);
			scr.h=(int)(getZoom()*64.0);
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_hit[1],&scr,buffer,&dest );
			break;
		case fxExploSmall:
			if ( !EffectsData.fx_explo_small ) break;
			CHECK_SCALING( EffectsData.fx_explo_small[1], EffectsData.fx_explo_small[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 14 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x = (int) ((int)(getZoom()*64.0) * 114 * ( (Client->iTimerTime - fx->StartTime)/2 ) / 64.0);
			scr.y = 0;
			scr.w = (int) ((int)(getZoom()*64.0) * 114 / 64.0);
			scr.h = (int) ((int)(getZoom()*64.0) * 108 / 64.0);
			dest.x = HUD_LEFT_WIDTH - ( (int) ( ( offX- ( fx->PosX - 57 ) ) * getZoom() ) );
			dest.y = HUD_TOP_HIGHT -  ( (int) ( ( offY- ( fx->PosY - 54 ) ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_explo_small[1], &scr, buffer, &dest );
			break;
		case fxExploBig:
			if ( !EffectsData.fx_explo_big ) break;
			CHECK_SCALING( EffectsData.fx_explo_big[1], EffectsData.fx_explo_big[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 28 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x = (int) ((int)(getZoom()*64.0) * 307 * ( (Client->iTimerTime - fx->StartTime)/2 ) / 64.0);
			scr.y = 0;
			scr.w = (int) ((int)(getZoom()*64.0) * 307 / 64.0);
			scr.h = (int) ((int)(getZoom()*64.0) * 194 / 64.0);
			dest.x = HUD_LEFT_WIDTH- ( (int) ( ( offX- ( fx->PosX - 134 ) ) * getZoom() ) );
			dest.y = HUD_TOP_HIGHT-  ( (int) ( ( offY- ( fx->PosY - 85 ) ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_explo_big[1], &scr, buffer, &dest );
			break;
		case fxExploWater:
			if ( !EffectsData.fx_explo_water ) break;
			CHECK_SCALING( EffectsData.fx_explo_water[1], EffectsData.fx_explo_water[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 14 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x = (int) ((int)(getZoom()*64.0) * 114 * ( (Client->iTimerTime - fx->StartTime)/2 ) / 64.0);
			scr.y = 0;
			scr.w = (int) ((int)(getZoom()*64.0) * 114 / 64.0);
			scr.h = (int) ((int)(getZoom()*64.0) * 108 / 64.0);
			dest.x = HUD_LEFT_WIDTH- ( (int) ( ( offX- ( fx->PosX - 57 ) ) * getZoom() ) );
			dest.y = HUD_TOP_HIGHT-  ( (int) ( ( offY- ( fx->PosY - 54 ) ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_explo_water[1],&scr,buffer,&dest );
			break;
		case fxExploAir:
			if ( !EffectsData.fx_explo_air ) break;
			CHECK_SCALING( EffectsData.fx_explo_air[1], EffectsData.fx_explo_air[0], getZoom() );
			if ( (Client->iTimerTime - fx->StartTime)/2 > 14 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x = (int) ((int)(getZoom()*64.0) * 137 * ( (Client->iTimerTime - fx->StartTime)/2 ) / 64.0);
			scr.y = 0;
			scr.w = (int) ((int)(getZoom()*64.0) * 137 / 64.0);
			scr.h = (int) ((int)(getZoom()*64.0) * 121 / 64.0);
			dest.x = HUD_LEFT_WIDTH- ( ( int ) ( ( offX- ( fx->PosX - 61 ) ) * getZoom() ) );
			dest.y = HUD_TOP_HIGHT-  ( ( int ) ( ( offY- ( fx->PosY - 68 ) ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_explo_air[1],&scr,buffer,&dest );
			break;
		case fxSmoke:
			if ( !EffectsData.fx_smoke ) break;
			CHECK_SCALING( EffectsData.fx_smoke[1], EffectsData.fx_smoke[0], getZoom() );
			if ( (Client->iTimerTime-fx->StartTime)/2 > 100/4 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			SDL_SetAlpha ( EffectsData.fx_smoke[1],SDL_SRCALPHA,100- ( (Client->iTimerTime-fx->StartTime)/2 ) *4 );
			scr.y=scr.x=0;
			scr.w=EffectsData.fx_smoke[1]->h;
			scr.h=EffectsData.fx_smoke[1]->h;
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX- ( fx->PosX-EffectsData.fx_smoke[0]->h/2+32 ) ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY- ( fx->PosY-EffectsData.fx_smoke[0]->h/2+32 ) ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_smoke[1],&scr,buffer,&dest );
			break;
		case fxRocket:
		{
			if ( !EffectsData.fx_rocket ) break;
			CHECK_SCALING( EffectsData.fx_rocket[1], EffectsData.fx_rocket[0], getZoom() );
			sFXRocketInfos *ri;
			ri= fx->rocketInfo;

			scr.x=ri->dir*EffectsData.fx_rocket[1]->h;
			scr.y=0;
			scr.h=scr.w=EffectsData.fx_rocket[1]->h;
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX- ( fx->PosX-EffectsData.fx_rocket[0]->h/2+32 ) ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY- ( fx->PosY-EffectsData.fx_rocket[0]->h/2+32 ) ) * getZoom() ) );

			if ( player->ScanMap[fx->PosX/64+fx->PosY/64*map->size] )
				SDL_BlitSurface ( EffectsData.fx_rocket[1],&scr,buffer,&dest );

			break;
		}
		case fxDarkSmoke:
		{
			if ( !EffectsData.fx_dark_smoke ) break;
			CHECK_SCALING( EffectsData.fx_dark_smoke[1], EffectsData.fx_dark_smoke[0], getZoom() );
			sFXDarkSmoke *dsi;
			dsi = fx->smokeInfo;
			if ( (Client->iTimerTime-fx->StartTime)/2 > 50 || dsi->alpha <= 1 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x= ( int ) ( 0.375*(int)(getZoom()*64.0) ) * ( (Client->iTimerTime-fx->StartTime)/2 );
			scr.y=0;
			scr.w=EffectsData.fx_dark_smoke[1]->h;
			scr.h=EffectsData.fx_dark_smoke[1]->h;
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX- ( ( int ) dsi->fx ) ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY- ( ( int ) dsi->fy ) ) * getZoom() ) );

			SDL_SetAlpha ( EffectsData.fx_dark_smoke[1],SDL_SRCALPHA,dsi->alpha );
			SDL_BlitSurface ( EffectsData.fx_dark_smoke[1],&scr,buffer,&dest );

			if ( Client->timer50ms )
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
			if ( !EffectsData.fx_absorb ) break;
			CHECK_SCALING( EffectsData.fx_absorb[1], EffectsData.fx_absorb[0], getZoom() );
			if ( (Client->iTimerTime-fx->StartTime)/2 > 10 )
			{
				delete fx;
				Client->FXList.Delete ( num );
				return;
			}
			scr.x=(int)(getZoom()*64.0)* ( (Client->iTimerTime-fx->StartTime)/2 );
			scr.y=0;
			scr.w=(int)(getZoom()*64.0);
			scr.h=(int)(getZoom()*64.0);
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_absorb[1],&scr,buffer,&dest );
			break;
		}
	}
}

void cGameGUI::displayBottomFX()
{
	if ( !Client->FXListBottom.Size() ) return;

	SDL_Rect oldClipRect = buffer->clip_rect;
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, SettingsData.iScreenW - HUD_TOTAL_WIDTH, SettingsData.iScreenH - HUD_TOTAL_HIGHT };
	SDL_SetClipRect( buffer, &clipRect );

	for ( int i = (int)Client->FXListBottom.Size() - 1; i >= 0; i-- )
	{
		drawBottomFX ( i );
	}
	SDL_SetClipRect( buffer, &oldClipRect );
}

void cGameGUI::drawBottomFX( int num )
{
	SDL_Rect scr, dest;

	sFX *fx = Client->FXListBottom[num];
	if ( ( !player->ScanMap[fx->PosX/64+fx->PosY/64*map->size] ) && fx->typ != fxTorpedo && fx->typ != fxTracks ) return;
	switch ( fx->typ )
	{
		case fxTorpedo:
		{
			CHECK_SCALING( EffectsData.fx_rocket[1], EffectsData.fx_rocket[0], getZoom() );
			sFXRocketInfos *ri;
			ri = fx->rocketInfo;

			scr.x=ri->dir*EffectsData.fx_rocket[1]->h;
			scr.y=0;
			scr.h=scr.w=EffectsData.fx_rocket[1]->h;
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX- ( fx->PosX-EffectsData.fx_rocket[0]->h/2+32 ) ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY- ( fx->PosY-EffectsData.fx_rocket[0]->h/2+32 ) ) * getZoom() ) );

			if ( player->ScanMap[fx->PosX/64+fx->PosY/64*map->size] )
			{
				SDL_BlitSurface ( EffectsData.fx_rocket[1],&scr,buffer,&dest );
			}
			break;
		}
		case fxTracks:
		{
			CHECK_SCALING( EffectsData.fx_tracks[1], EffectsData.fx_tracks[0], getZoom() );
			sFXTracks *tri;
			tri = fx->trackInfo;
			if ( tri->alpha<=1 )
			{
				delete fx;
				Client->FXListBottom.Delete ( num );
				return;
			}

			SDL_SetAlpha ( EffectsData.fx_tracks[1],SDL_SRCALPHA,tri->alpha );
			if ( Client->timer50ms )
			{
				tri->alpha--;
			}

			if ( !player->ScanMap[fx->PosX/64+fx->PosY/64*map->size] ) return;
			scr.y=0;
			scr.w=scr.h=EffectsData.fx_tracks[1]->h;
			scr.x=tri->dir*scr.w;
			dest.x = HUD_LEFT_WIDTH - (int)( (offX-fx->PosX) * getZoom() );
			dest.y = HUD_TOP_HIGHT - (int)( (offY-fx->PosY) * getZoom() );
			SDL_BlitSurface ( EffectsData.fx_tracks[1],&scr,buffer,&dest );
			break;
		}
		case fxBubbles:
			CHECK_SCALING( EffectsData.fx_smoke[1], EffectsData.fx_smoke[0], getZoom() );
			if ( (Client->iTimerTime-fx->StartTime)/2 > 100/4 )
			{
				delete fx;
				Client->FXListBottom.Delete ( num );
				return;
			}
			SDL_SetAlpha ( EffectsData.fx_smoke[1],SDL_SRCALPHA,100- ( (Client->iTimerTime-fx->StartTime)/2 ) *4 );
			scr.y=scr.x=0;
			scr.w=EffectsData.fx_smoke[1]->h;
			scr.h=EffectsData.fx_smoke[1]->h;
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX- ( fx->PosX-EffectsData.fx_smoke[0]->h/2+32 ) ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY- ( fx->PosY-EffectsData.fx_smoke[0]->h/2+32 ) ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_smoke[1],&scr,buffer,&dest );
			break;
		case fxCorpse:
			CHECK_SCALING( EffectsData.fx_corpse[1], EffectsData.fx_corpse[0], getZoom() );
			SDL_SetAlpha ( EffectsData.fx_corpse[1],SDL_SRCALPHA,fx->param-- );
			scr.y=scr.x=0;
			scr.w=EffectsData.fx_corpse[1]->h;
			scr.h=EffectsData.fx_corpse[1]->h;
			dest.x=HUD_LEFT_WIDTH- ( ( int ) ( ( offX-fx->PosX ) * getZoom() ) );
			dest.y=HUD_TOP_HIGHT- ( ( int ) ( ( offY-fx->PosY ) * getZoom() ) );
			SDL_BlitSurface ( EffectsData.fx_corpse[1],&scr,buffer,&dest );

			if ( fx->param<=0 )
			{
				delete fx;
				Client->FXListBottom.Delete ( num );
				return;
			}
			break;
	}
}

void cGameGUI::drawBaseUnits( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	int tileSize = Client->gameGUI.getTileSize();
	SDL_Rect dest;
	//draw rubble and all base buildings (without bridges)
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startX;

	for ( int y = startX; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			cBuildingIterator bi = map->fields[pos].getBuildings();
			while ( !bi.end ) bi++;
			bi--;

			while ( !bi.rend && ( bi->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || bi->data.surfacePosition == sUnitData::SURFACE_POS_BASE || !bi->owner ) )
			{
				if ( player->ScanMap[pos]||
					( bi->data.isBig && ( ( x < endX && player->ScanMap[pos+1] ) || ( y < endY && player->ScanMap[pos+map->size] ) || ( x < endX && y < endY && player->ScanMap[pos+map->size+1] ) ) ) )
				{
					if ( bi->PosX == x && bi->PosY == y )
					{
						bi->draw ( &dest );
					}
				}
				bi--;
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawTopBuildings( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	SDL_Rect dest;
	int tileSize = Client->gameGUI.getTileSize();
	//draw top buildings (except connectors)
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for (  int x = startX; x <= endX; x++ )
		{
			cBuilding* building = map->fields[pos].getBuildings();
			if ( building && building->data.surfacePosition == sUnitData::SURFACE_POS_GROUND  )
			{

				if ( player->ScanMap[pos]||
						( building->data.isBig && ( ( x < endX && player->ScanMap[pos+1] ) || ( y < endY && player->ScanMap[pos+map->size] ) || ( x < endX && y < endY && player->ScanMap[pos+map->size+1] ) ) ) )
				{
					if ( building->PosX == x && building->PosY == y )	//make sure a big building is drawn only once
					{
						building->draw ( &dest );

						if ( debugBaseClient && building->SubBase )
						{
							sSubBase *sb;
							SDL_Rect tmp = { dest.x, dest.y, getTileSize(), 8 };
							if ( building->data.isBig ) tmp.w *= 2;
							sb = building->SubBase;
							// the VS compiler gives a warning on casting a pointer to long.
							// therfore we will first cast to long long and then cut this to Unit32 again.
							SDL_FillRect ( buffer,&tmp, (Uint32)(long long)(sb));
							font->showText(dest.x+1,dest.y+1, iToStr( sb->iID ), FONT_LATIN_SMALL_WHITE);
							string sTmp = "m "+iToStr(sb->Metal)+"/"+iToStr(sb->MaxMetal)+" +"+iToStr(sb->MetalProd-sb->MetalNeed);
							font->showText(dest.x+1,dest.y+1+8, sTmp, FONT_LATIN_SMALL_WHITE);

							sTmp = "o "+iToStr(sb->Oil)+"/"+iToStr(sb->MaxOil)+" +"+iToStr(sb->OilProd-sb->OilNeed);
							font->showText(dest.x+1,dest.y+1+16, sTmp, FONT_LATIN_SMALL_WHITE);

							sTmp = "g "+iToStr(sb->Gold)+"/"+iToStr(sb->MaxGold)+" +"+iToStr(sb->GoldProd-sb->GoldNeed);
							font->showText(dest.x+1,dest.y+1+24, sTmp, FONT_LATIN_SMALL_WHITE);
						}
						if ( debugBaseServer && building->SubBase )
						{
							sSubBase *sb;
							SDL_Rect tmp = { dest.x, dest.y, getTileSize(), 8 };
							if ( building->data.isBig ) tmp.w*=2;
							sb = Server->Map->fields[pos].getBuildings()->SubBase;
							// the VS compiler gives a warning on casting a pointer to long.
							// therfore we will first cast to long long and then cut this to Unit32 again.
							SDL_FillRect ( buffer,&tmp, (Uint32)(long long)(sb) );
							font->showText(dest.x+1,dest.y+1, iToStr( sb->iID ), FONT_LATIN_SMALL_WHITE);
							string sTmp = "m "+iToStr(sb->Metal)+"/"+iToStr(sb->MaxMetal)+" +"+iToStr(sb->MetalProd-sb->MetalNeed);
							font->showText(dest.x+1,dest.y+1+8, sTmp, FONT_LATIN_SMALL_WHITE);

							sTmp = "o "+iToStr(sb->Oil)+"/"+iToStr(sb->MaxOil)+" +"+iToStr(sb->OilProd-sb->OilNeed);
							font->showText(dest.x+1,dest.y+1+16, sTmp, FONT_LATIN_SMALL_WHITE);

							sTmp = "g "+iToStr(sb->Gold)+"/"+iToStr(sb->MaxGold)+" +"+iToStr(sb->GoldProd-sb->GoldNeed);
							font->showText(dest.x+1,dest.y+1+24, sTmp, FONT_LATIN_SMALL_WHITE);
						}
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawShips( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	SDL_Rect dest;
	int tileSize = Client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			if ( player->ScanMap[pos] )
			{
				cVehicle* vehicle = map->fields[pos].getVehicles();
				if ( vehicle && vehicle->data.factorSea > 0 && vehicle->data.factorGround == 0 )
				{
					vehicle->draw ( dest );
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawAboveSeaBaseUnits( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	SDL_Rect dest;
	int tileSize = Client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			if ( player->ScanMap[pos] )
			{
				cBuildingIterator building = map->fields[pos].getBuildings();
				do
				{
					if ( building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA )
					{
						building->draw ( &dest );
					}
					building++;
				} while ( !building.end );

				building = map->fields[pos].getBuildings();
				do
				{
					if ( building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE )
					{
						building->draw ( &dest );
					}
					building++;
				} while ( !building.end );

				cVehicle* vehicle = map->fields[pos].getVehicles();
				if ( vehicle && (vehicle->IsClearing || vehicle->IsBuilding) && ( player->ScanMap[pos] || ( x < endX && player->ScanMap[pos+1] ) || ( y < endY && player->ScanMap[pos+map->size] ) || ( x < endX && y < endY && player->ScanMap[pos+map->size+1] ) ) )
				{
					if ( vehicle->PosX == x && vehicle->PosY == y )	//make sure a big vehicle is drawn only once
					{
						vehicle->draw ( dest );
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawVehicles( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	SDL_Rect dest;
	int tileSize = Client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			if ( player->ScanMap[pos] )
			{
				cVehicle* vehicle = map->fields[pos].getVehicles();
				if ( vehicle && vehicle->data.factorGround != 0 && !vehicle->IsBuilding && !vehicle->IsClearing )
				{
					vehicle->draw ( dest );
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawConnectors( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	SDL_Rect dest;
	int tileSize = Client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			if ( player->ScanMap[pos] )
			{
				cBuilding* building = map->fields[pos].getTopBuilding();
				if ( building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE )
				{
					building->draw ( &dest );
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawPlanes( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	SDL_Rect dest;
	int tileSize = Client->gameGUI.getTileSize();
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			if ( player->ScanMap[pos] )
			{
				cVehicle* plane = map->fields[pos].getPlanes();
				if ( plane )
				{
					plane->draw ( dest );
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawResources( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY )
{
	int tileSize = Client->gameGUI.getTileSize();
	SDL_Rect dest, tmp, src = { 0, 0, tileSize, tileSize };
	dest.y = HUD_TOP_HIGHT-zoomOffY+tileSize*startY;
	for ( int y = startY; y <= endY; y++ )
	{
		dest.x = HUD_LEFT_WIDTH-zoomOffX+tileSize*startX;
		int pos = y*map->size+startX;
		for ( int x = startX; x <= endX; x++ )
		{
			if ( player->ResourceMap[pos] )
			{
				if ( map->Resources[pos].typ == RES_NONE )
				{
					src.x = 0;
					tmp = dest;
					if ( !SettingsData.bPreScale && ( ResourceData.res_metal->w != ResourceData.res_metal_org->w/64*tileSize || ResourceData.res_metal->h != tileSize ) ) scaleSurface ( ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w/64*tileSize, tileSize );
					SDL_BlitSurface ( ResourceData.res_metal,&src,buffer,&tmp );
				}
				else
				{
					src.x = map->Resources[pos].value*tileSize;
					tmp = dest;
					if ( map->Resources[pos].typ == RES_METAL )
					{
						if ( !SettingsData.bPreScale && ( ResourceData.res_metal->w != ResourceData.res_metal_org->w/64*tileSize || ResourceData.res_metal->h != tileSize ) ) scaleSurface ( ResourceData.res_metal_org, ResourceData.res_metal, ResourceData.res_metal_org->w/64*tileSize, tileSize );
						SDL_BlitSurface ( ResourceData.res_metal, &src, buffer, &tmp );
					}
					else if ( map->Resources[pos].typ == RES_OIL )
					{
						if ( !SettingsData.bPreScale && ( ResourceData.res_oil->w != ResourceData.res_oil_org->w/64*tileSize || ResourceData.res_oil->h != tileSize ) ) scaleSurface ( ResourceData.res_oil_org, ResourceData.res_oil, ResourceData.res_oil_org->w/64*tileSize, tileSize );
						SDL_BlitSurface ( ResourceData.res_oil, &src, buffer, &tmp );
					}
					else
					{
						if ( !SettingsData.bPreScale && ( ResourceData.res_gold->w != ResourceData.res_gold_org->w/64*tileSize || ResourceData.res_gold->h != tileSize ) ) scaleSurface ( ResourceData.res_gold_org, ResourceData.res_gold, ResourceData.res_gold_org->w/64*tileSize, tileSize );
						SDL_BlitSurface ( ResourceData.res_gold, &src, buffer, &tmp );
					}
				}
			}
			pos++;
			dest.x += tileSize;
		}
		dest.y += tileSize;
	}
}

void cGameGUI::drawDebugSentry()
{
}

void cGameGUI::drawSelectionBox( int zoomOffX, int zoomOffY )
{
	if ( mouseBox.startX == -1 || mouseBox.startY == -1 || mouseBox.endX == -1 || mouseBox.endY == -1 ) return;

	Uint32 color = 0xFFFF00;
	SDL_Rect d;

	int mouseStartX = (int)(min(mouseBox.startX, mouseBox.endX)*getTileSize());
	int mouseStartY = (int)(min(mouseBox.startY, mouseBox.endY)*getTileSize());
	int mouseEndX = (int)(max(mouseBox.startX, mouseBox.endX)*getTileSize());
	int mouseEndY = (int)(max(mouseBox.startY, mouseBox.endY)*getTileSize());

	d.h = 1;
	d.w = mouseEndX-mouseStartX;
	d.x = mouseStartX-zoomOffX+HUD_LEFT_WIDTH;
	d.y = mouseEndY-zoomOffY+20;
	SDL_FillRect ( buffer, &d, color );

	d.h = 1;
	d.w = mouseEndX-mouseStartX;
	d.x = mouseStartX-zoomOffX+HUD_LEFT_WIDTH;
	d.y = mouseStartY-zoomOffY+20;
	SDL_FillRect ( buffer, &d, color );

	d.h = mouseEndY-mouseStartY;
	d.w = 1;
	d.x = mouseStartX-zoomOffX+HUD_LEFT_WIDTH;
	d.y = mouseStartY-zoomOffY+20;
	SDL_FillRect ( buffer, &d, color );

	d.h = mouseEndY-mouseStartY;
	d.w = 1;
	d.x = mouseEndX-zoomOffX+HUD_LEFT_WIDTH;
	d.y = mouseStartY-zoomOffY+20;
	SDL_FillRect ( buffer, &d, color );
}

void cGameGUI::drawDebugOutput()
{
	#define DEBUGOUT_X_POS		(SettingsData.iScreenW-140)

	int debugOff = 30;

	if ( debugPlayers )
	{
		font->showText(DEBUGOUT_X_POS, debugOff, "Players: " + iToStr( (int)Client->PlayerList->Size() ), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);

		SDL_Rect rDest = { DEBUGOUT_X_POS, debugOff, 20, 10 };
		SDL_Rect rSrc = { 0, 0, 20, 10 };
		SDL_Rect rDotDest = {DEBUGOUT_X_POS-10, debugOff, 10, 10 };
		SDL_Rect rBlackOut = {DEBUGOUT_X_POS+20, debugOff, 0, 10 };
		for ( unsigned int i = 0; i < Client->PlayerList->Size(); i++ )
		{
			//HACK SHOWFINISHEDPLAYERS
			SDL_Rect rDot = { 10 , 0, 10, 10 }; //for green dot

			if( (*Client->PlayerList)[i]->bFinishedTurn && (*Client->PlayerList)[i] != player)
			{
				SDL_BlitSurface( GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest );
			}
			else if(  (*Client->PlayerList)[i] == player && Client->bWantToEnd )
			{
				SDL_BlitSurface( GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest );
			}
			else
			{
				rDot.x = 0; //for red dot
				SDL_BlitSurface( GraphicsData.gfx_player_ready, &rDot, buffer, &rDotDest );
			}

			SDL_BlitSurface ( (*Client->PlayerList)[i]->color, &rSrc, buffer, &rDest );
			if ( (*Client->PlayerList)[i] == player )
			{
				string sTmpLine = " " + (*Client->PlayerList)[i]->name + ", nr: " + iToStr ( (*Client->PlayerList)[i]->Nr ) + " << you! ";
				rBlackOut.w = font->getTextWide(sTmpLine, FONT_LATIN_SMALL_WHITE); //black out background for better recognizing
				SDL_FillRect(buffer, &rBlackOut, 0x000000);
				font->showText(rBlackOut.x, debugOff+1, sTmpLine , FONT_LATIN_SMALL_WHITE);
			}
			else
			{
				string sTmpLine = " " + (*Client->PlayerList)[i]->name + ", nr: " + iToStr ( (*Client->PlayerList)[i]->Nr ) + " ";
				rBlackOut.w = font->getTextWide(sTmpLine, FONT_LATIN_SMALL_WHITE); //black out background for better recognizing
				SDL_FillRect(buffer, &rBlackOut, 0x000000);
				font->showText(rBlackOut.x, debugOff+1, sTmpLine , FONT_LATIN_SMALL_WHITE);
			}
			debugOff += 10; //use 10 for pixel high of dots instead of text high
			rDest.y = rDotDest.y = rBlackOut.y = debugOff;

		}
	}

	if ( debugAjobs )
	{
		font->showText(DEBUGOUT_X_POS, debugOff, "ClientAttackJobs: " + iToStr((int)Client->attackJobs.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		if ( Server )
		{
			font->showText(DEBUGOUT_X_POS, debugOff, "ServerAttackJobs: " + iToStr((int)Server->AJobs.Size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		}
	}

	if ( debugBaseClient )
	{
		font->showText(DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr((int)player->base.SubBases.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight ( FONT_LATIN_SMALL_WHITE );
	}

	if ( debugBaseServer )
	{
		cPlayer* serverPlayer = Server->getPlayerFromNumber(player->Nr);
		font->showText(DEBUGOUT_X_POS, debugOff, "subbases: " + iToStr((int)serverPlayer->base.SubBases.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight ( FONT_LATIN_SMALL_WHITE );
	}

	if ( debugSentry )
	{
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			cPlayer *Player = (*Server->PlayerList)[i];
			font->showText(DEBUGOUT_X_POS, debugOff, Player->name + " (" + iToStr ( Player->Nr ) + ") s-air: " + iToStr((int)Player->SentriesAir.Size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
			font->showText(DEBUGOUT_X_POS, debugOff, Player->name + " (" + iToStr ( Player->Nr ) + ") s-ground: " + iToStr((int)Player->SentriesGround.Size()), FONT_LATIN_SMALL_WHITE);
			debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		}
	}

	if ( debugFX )
	{
		/*font->showText(DEBUGOUT_X_POS, debugOff, "fx-count: " + iToStr((int)FXList.Size() + (int)FXListBottom.Size()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "wind-dir: " + iToStr(( int ) ( fWindDir*57.29577 )), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);*/
	}
	if ( debugTraceServer || debugTraceClient )
	{
		trace();
	}
	if ( debugCache )
	{
		font->showText(DEBUGOUT_X_POS, debugOff, "Max cache size: " + iToStr(dCache.getMaxCacheSize()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "cache size: " + iToStr(dCache.getCacheSize()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "cache hits: " + iToStr(dCache.getCacheHits()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "cache misses: " + iToStr(dCache.getCacheMisses()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "not cached: " + iToStr(dCache.getNotCached()), FONT_LATIN_SMALL_WHITE);
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
	}

	if ( showFPS )
	{
		font->showText(DEBUGOUT_X_POS, debugOff, "FPS: " + iToStr( Round( framesPerSecond ) ), FONT_LATIN_SMALL_WHITE );
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "Cycles/s: " + iToStr( Round( cyclesPerSecond ) ), FONT_LATIN_SMALL_WHITE );
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
		font->showText(DEBUGOUT_X_POS, debugOff, "Load: " + iToStr( loadValue/10 ) + "." + iToStr( loadValue%10 ) + "%", FONT_LATIN_SMALL_WHITE );
		debugOff += font->getFontHeight(FONT_LATIN_SMALL_WHITE);
	}
}

void cGameGUI::displayMessages()
{
	if ( Client->messages.Size() == 0 ) return;

	sMessage *message;
	int height = 0;
	for ( int i = (int)Client->messages.Size() - 1; i >= 0; i-- )
	{
		message = Client->messages[i];
		height += 17 + font->getFontHeight() * ( message->len  / (SettingsData.iScreenW - 300) );
	}
	SDL_Rect scr = { 0, 0, SettingsData.iScreenW - 200, height+6 };
	SDL_Rect dest = { 180, 30, 0, 0 };

	if ( SettingsData.bAlphaEffects ) SDL_BlitSurface ( GraphicsData.gfx_shadow, &scr, buffer, &dest );
	dest.x = 180+2; dest.y = 34;
	dest.w = SettingsData.iScreenW - 204;
	dest.h = height;

	for ( unsigned int i = 0; i < Client->messages.Size(); i++ )
	{
		message = Client->messages[i];
		string msgString = message->msg;
		//HACK TO SHOW PLAYERCOLOR IN CHAT
		int color = -1;
		for(unsigned int i = 0; i < msgString.length(); i++)
		{
			if(msgString[i] == ':') //scan for chatmessages from _players_
			{
				string tmpString = msgString.substr( 0, i );
				for ( unsigned int i = 0; i < Client->PlayerList->Size(); i++ )
				{
					cPlayer* const Player = (*Client->PlayerList)[i];
					if (Player)
					{
						if(tmpString.compare( Player->name ) == 0)
						{
							color = GetColorNr(Player->color);
							break;
						}
					}
				}
				break;
			}
		}
		if(color != -1)
		{
			#define CELLSPACE 3
			SDL_Rect rColorSrc = { 0, 0, 10, font->getFontHeight() };
			SDL_Rect rDest = dest;
			rDest.w = rColorSrc.w;
			rDest.h = rColorSrc.h;
			SDL_BlitSurface(OtherData.colors[color], &rColorSrc, buffer, &rDest ); //blit color
			dest.x += rColorSrc.w + CELLSPACE; //add border for color
			dest.w -= rColorSrc.w + CELLSPACE;
			dest.y = font->showTextAsBlock( dest, msgString );
			dest.x -= rColorSrc.w + CELLSPACE; //reset border from color
			dest.w += rColorSrc.w + CELLSPACE;
		}
		else
		{
			dest.y = font->showTextAsBlock( dest, msgString );
		}

		dest.y += 5;
	}
}

void cGameGUI::scaleColors()
{
	for ( int i = 0; i < PLAYERCOLORS; i++ )
	{
		scaleSurface( OtherData.colors_org[i], OtherData.colors[i], (int) (OtherData.colors_org[i]->w * getZoom()), (int) (OtherData.colors_org[i]->h * getZoom()) );
	}
}

void cGameGUI::scaleSurfaces ()
{
	// Terrain:
	sTerrain*& tlist = map->terrain;
	int numberOfTerrains = map->iNumberOfTerrains;
	for (int i = 0; i < numberOfTerrains; ++i)
	{
		sTerrain& t = tlist[i];
		scaleSurface ( t.sf_org, t.sf, getTileSize(), getTileSize() );
		scaleSurface ( t.shw_org, t.shw, getTileSize(), getTileSize() );
	}
	// Vehicles:
	for (unsigned int i = 0; i < UnitsData.getNrVehicles (); ++i)
	{
		UnitsData.vehicle[i].scaleSurfaces( getZoom() );
	}
	// Buildings:
	for (unsigned int i = 0; i < UnitsData.getNrBuildings (); ++i)
	{
		UnitsData.building[i].scaleSurfaces ( getZoom() );
	}

	if ( UnitsData.dirt_small_org && UnitsData.dirt_small ) scaleSurface ( UnitsData.dirt_small_org,UnitsData.dirt_small, (int) ( UnitsData.dirt_small_org->w * getZoom() ), (int) ( UnitsData.dirt_small_org->h * getZoom() ) );
	if ( UnitsData.dirt_small_shw_org && UnitsData.dirt_small_shw ) scaleSurface ( UnitsData.dirt_small_shw_org,UnitsData.dirt_small_shw, (int) ( UnitsData.dirt_small_shw_org->w * getZoom() ), (int) ( UnitsData.dirt_small_shw_org->h * getZoom() ) );
	if ( UnitsData.dirt_big_org && UnitsData.dirt_big ) scaleSurface ( UnitsData.dirt_big_org,UnitsData.dirt_big, (int) ( UnitsData.dirt_big_org->w * getZoom() ), (int) ( UnitsData.dirt_big_org->h * getZoom() ) );
	if ( UnitsData.dirt_big_shw_org && UnitsData.dirt_big_shw ) scaleSurface ( UnitsData.dirt_big_shw_org,UnitsData.dirt_big_shw, (int) ( UnitsData.dirt_big_shw_org->w * getZoom() ), (int) ( UnitsData.dirt_big_shw_org->h * getZoom() ) );

	// Bänder:
	if ( GraphicsData.gfx_band_small_org && GraphicsData.gfx_band_small ) scaleSurface ( GraphicsData.gfx_band_small_org,GraphicsData.gfx_band_small,getTileSize(),getTileSize() );
	if ( GraphicsData.gfx_band_big_org && GraphicsData.gfx_band_big ) scaleSurface ( GraphicsData.gfx_band_big_org,GraphicsData.gfx_band_big,getTileSize()*2,getTileSize()*2 );

	// Resources:
	if ( ResourceData.res_metal_org && ResourceData.res_metal ) scaleSurface ( ResourceData.res_metal_org,ResourceData.res_metal,ResourceData.res_metal_org->w/64*getTileSize(),getTileSize() );
	if ( ResourceData.res_oil_org && ResourceData.res_oil ) scaleSurface ( ResourceData.res_oil_org,ResourceData.res_oil,ResourceData.res_oil_org->w/64*getTileSize(),getTileSize() );
	if ( ResourceData.res_gold_org && ResourceData.res_gold ) scaleSurface ( ResourceData.res_gold_org,ResourceData.res_gold,ResourceData.res_gold_org->w/64*getTileSize(),getTileSize() );

	// Big Beton:
	if ( GraphicsData.gfx_big_beton_org && GraphicsData.gfx_big_beton ) scaleSurface ( GraphicsData.gfx_big_beton_org,GraphicsData.gfx_big_beton,getTileSize()*2,getTileSize()*2 );

	// Andere:
	if ( GraphicsData.gfx_exitpoints_org && GraphicsData.gfx_exitpoints ) scaleSurface ( GraphicsData.gfx_exitpoints_org,GraphicsData.gfx_exitpoints,GraphicsData.gfx_exitpoints_org->w/64*getTileSize(),getTileSize() );

	// FX:
#define SCALE_FX(a) if (a) scaleSurface(a[0],a[1], (a[0]->w * getTileSize())/64 , (a[0]->h * getTileSize())/64);
	SCALE_FX ( EffectsData.fx_explo_small );
	SCALE_FX ( EffectsData.fx_explo_big );
	SCALE_FX ( EffectsData.fx_explo_water );
	SCALE_FX ( EffectsData.fx_explo_air );
	SCALE_FX ( EffectsData.fx_muzzle_big );
	SCALE_FX ( EffectsData.fx_muzzle_small );
	SCALE_FX ( EffectsData.fx_muzzle_med );
	SCALE_FX ( EffectsData.fx_hit );
	SCALE_FX ( EffectsData.fx_smoke );
	SCALE_FX ( EffectsData.fx_rocket );
	SCALE_FX ( EffectsData.fx_dark_smoke );
	SCALE_FX ( EffectsData.fx_tracks );
	SCALE_FX ( EffectsData.fx_corpse );
	SCALE_FX ( EffectsData.fx_absorb );
}

void cGameGUI::makePanel( bool open )
{
	SDL_Rect tmp;
	if ( open )
	{
		PlayFX ( SoundData.SNDPanelOpen );
		SDL_Rect top = { 0, ( SettingsData.iScreenH/2 )-479, 171, 479 };
		SDL_Rect bottom = { 0, ( SettingsData.iScreenH/2 ) , 171, 481 };
		SDL_BlitSurface ( GraphicsData.gfx_panel_top, NULL, buffer, &tmp );
		tmp = bottom;
		SDL_BlitSurface ( GraphicsData.gfx_panel_bottom ,NULL, buffer, &tmp );
		while ( top.y > -479 )
		{
			SHOW_SCREEN
			mouse->draw ( false, screen );
			SDL_Delay ( 10 );
			top.y -= 10;
			bottom.y += 10;
			draw( false, false );
			tmp = top;
			SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&tmp );
			SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&bottom );
		}
	}
	else
	{
		PlayFX ( SoundData.SNDPanelClose );
		SDL_Rect top = { 0, -480, 171, 479 };
		SDL_Rect bottom = { 0, SettingsData.iScreenH , 171, 481 };
		while ( bottom.y>SettingsData.iScreenH/2 )
		{
			SHOW_SCREEN
			mouse->draw ( false, screen );
			SDL_Delay ( 10 );
			top.y += 10;
			if ( top.y> ( SettingsData.iScreenH/2 )-479-9 ) top.y = ( SettingsData.iScreenH/2 )-479;
			bottom.y -= 10;
			if ( bottom.y < SettingsData.iScreenH/2+9 ) bottom.y = SettingsData.iScreenH/2;
			draw( false, false );
			tmp = top;
			SDL_BlitSurface ( GraphicsData.gfx_panel_top,NULL,buffer,&tmp );
			tmp = bottom;
			SDL_BlitSurface ( GraphicsData.gfx_panel_bottom,NULL,buffer,&tmp );
		}
		SHOW_SCREEN
		mouse->draw ( false, screen );
		SDL_Delay ( 100 );
	}
}

void cGameGUI::trace ()
{
	int y, x;
	cMapField* field;

	mouse->GetKachel ( &x, &y );
	if ( x < 0 || y < 0 ) return;

	if ( debugTraceServer ) field = Server->Map->fields + ( Server->Map->size*y+x );
	else field = map->fields + ( map->size*y+x );

	y = 18+5+8;
	x = 180+5;

	if ( field->getVehicles() ) { traceVehicle ( field->getVehicles(), &y, x ); y += 20; }
	if ( field->getPlanes() ) { traceVehicle ( field->getPlanes(), &y, x ); y += 20; }
	cBuildingIterator bi = field->getBuildings();
	while ( !bi.end ) { traceBuilding ( bi, &y, x ); y += 20; bi++;}
}

void cGameGUI::traceVehicle ( cVehicle *vehicle, int *y, int x )
{
	string tmpString;

	tmpString = "name: \"" + vehicle->name + "\" id: \"" + iToStr ( vehicle->iID ) + "\" owner: \"" + vehicle->owner->name + "\" posX: +" + iToStr ( vehicle->PosX ) + " posY: " + iToStr ( vehicle->PosY ) + " offX: " + iToStr ( vehicle->OffX ) + " offY: " + iToStr ( vehicle->OffY );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "dir: " + iToStr ( vehicle->dir ) + " selected: " + iToStr ( vehicle->selected ) + " moving: +" + iToStr ( vehicle->moving ) + " mjob: "  + pToStr ( vehicle->ClientMoveJob ) + " speed: " + iToStr ( vehicle->data.speedCur ) + " mj_active: " + iToStr ( vehicle->MoveJobActive ) + " menu_active: " + iToStr ( vehicle->MenuActive );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "attack_mode: " + iToStr ( vehicle->AttackMode ) + " attacking: " + iToStr ( vehicle->Attacking ) + " on sentry: +" + iToStr ( vehicle->bSentryStatus ) + " transfer: " + iToStr ( vehicle->Transfer ) + " ditherx: " + iToStr (vehicle->ditherX ) + " dithery: " + iToStr ( vehicle->ditherY );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "is_building: " + iToStr ( vehicle->IsBuilding ) + " building_typ: " + vehicle->BuildingTyp.getText() + " build_costs: +" + iToStr ( vehicle->BuildCosts ) + " build_rounds: " + iToStr ( vehicle->BuildRounds ) + " build_round_start: " + iToStr (vehicle->BuildRoundsStart );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "place_band: " + iToStr ( vehicle->PlaceBand ) + " bandx: " + iToStr ( vehicle->BandX ) + " bandy: +" + iToStr ( vehicle->BandY ) + " build_big_saved_pos: " + iToStr ( vehicle->BuildBigSavedPos ) + " build_path: " + iToStr (vehicle->BuildPath );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = " is_clearing: " + iToStr ( vehicle->IsClearing ) + " clearing_rounds: +" + iToStr ( vehicle->ClearingRounds ) + " clear_big: " + iToStr ( vehicle->data.isBig ) + " loaded: " + iToStr (vehicle->Loaded );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "commando_rank: " + dToStr ( Round ( vehicle->CommandoRank, 2 ) ) + " steal_active: " + iToStr ( vehicle->StealActive ) + " disable_active: +" + iToStr ( vehicle->DisableActive ) + " disabled: " + iToStr ( vehicle->Disabled ) /*+ " detection_override: " + iToStr (vehicle->detection_override )*/;
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "is_locked: " + iToStr ( vehicle->IsLocked ) + /*" detected: " + iToStr ( vehicle->detected ) +*/ " clear_mines: +" + iToStr ( vehicle->ClearMines ) + " lay_mines: " + iToStr ( vehicle->LayMines ) + " repair_active: " + iToStr (vehicle->RepairActive ) + " muni_active: " + iToStr (vehicle->MuniActive );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString =
		"load_active: "            + iToStr(vehicle->LoadActive) +
		" activating_vehicle: "    + iToStr(vehicle->ActivatingVehicle) +
		" vehicle_to_activate: +"  + iToStr(vehicle->VehicleToActivate) +
		" stored_vehicles_count: " + iToStr((int)vehicle->StoredVehicles.Size());
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	if ( vehicle->StoredVehicles.Size() )
	{
		cVehicle *StoredVehicle;
		for (unsigned int i = 0; i < vehicle->StoredVehicles.Size(); i++)
		{
			StoredVehicle = vehicle->StoredVehicles[i];
			font->showText(x, *y, " store " + iToStr(i)+": \""+StoredVehicle->name+"\"", FONT_LATIN_SMALL_WHITE);
			*y += 8;
		}
	}

	if ( debugTraceServer )
	{
		tmpString = "seen by players: owner";
		for (unsigned int i = 0; i < vehicle->SeenByPlayerList.Size(); i++)
		{
			tmpString += ", \"" + vehicle->SeenByPlayerList[i]->name + "\"";
		}
		font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
		*y+=8;
	}
}

void cGameGUI::traceBuilding ( cBuilding *building, int *y, int x )
{
	string tmpString;

	tmpString = "name: \"" + building->name + "\" id: \"" + iToStr ( building->iID ) + "\" owner: \"" + ( building->owner?building->owner->name:"<null>" ) + "\" posX: +" + iToStr ( building->PosX ) + " posY: " + iToStr ( building->PosY ) + " selected: " + iToStr ( building->selected );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "dir: " + iToStr ( building->dir ) + " menu_active: " + iToStr ( building->MenuActive ) + " on sentry: +" + iToStr ( building->bSentryStatus ) + " attacking_mode: +" + iToStr ( building->AttackMode ) + " base: " + pToStr ( building->base ) + " sub_base: " + pToStr (building->SubBase );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "attacking: " + iToStr ( building->Attacking ) + " UnitsData.dirt_typ: " + iToStr ( building->RubbleTyp ) + " UnitsData.dirt_value: +" + iToStr ( building->RubbleValue ) + " big_dirt: " + iToStr ( building->data.isBig ) + " is_working: " + iToStr (building->IsWorking ) + " transfer: " + iToStr (building->Transfer );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = " max_metal_p: " + iToStr ( building->MaxMetalProd ) + " max_oil_p: " + iToStr (building->MaxOilProd ) + " max_gold_p: " + iToStr (building->MaxGoldProd );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString = "is_locked: " + iToStr ( building->IsLocked ) + " disabled: " + iToStr ( building->Disabled ) /*+ " detected: +" + iToStr ( building->detected )*/ + " activating_vehicle: " + iToStr ( building->ActivatingVehicle ) + " vehicle_to_activate: " + iToStr (building->VehicleToActivate );
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	tmpString =
		"load_active: "            + iToStr(building->LoadActive) +
		" stored_vehicles_count: " + iToStr((int)building->StoredVehicles.Size());
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	if (building->StoredVehicles.Size())
	{
		cVehicle *StoredVehicle;
		for (unsigned int i = 0; i < building->StoredVehicles.Size(); i++)
		{
			StoredVehicle = building->StoredVehicles[i];
			font->showText(x, *y, " store " + iToStr(i)+": \""+StoredVehicle->name+"\"", FONT_LATIN_SMALL_WHITE);
			*y+=8;
		}
	}

	tmpString =
		"build_speed: "        + iToStr(building->BuildSpeed)  +
		" repeat_build: "      + iToStr(building->RepeatBuild) +
		" build_list_count: +" + iToStr(building->BuildList ? (int)building->BuildList->Size() : 0);
	font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
	*y+=8;

	if (building->BuildList && building->BuildList->Size())
	{
		sBuildList *BuildingList;
		for (unsigned int i = 0; i < building->BuildList->Size(); i++)
		{
			BuildingList = (*building->BuildList)[i];
			font->showText(x, *y, "  build "+iToStr(i)+": "+iToStr(BuildingList->typ->nr)+" \""+UnitsData.vehicle[BuildingList->typ->nr].data.name+"\"", FONT_LATIN_SMALL_WHITE);
			*y+=8;
		}
	}

	if ( debugTraceServer )
	{
		tmpString = "seen by players: owner";
		for (unsigned int i = 0; i < building->SeenByPlayerList.Size(); i++)
		{
			tmpString += ", \"" + building->SeenByPlayerList[i]->name + "\"";
		}
		font->showText(x,*y, tmpString, FONT_LATIN_SMALL_WHITE);
		*y+=8;
	}
}

void cGameGUI::drawUnitCircles()
{
	SDL_Rect clipRect = { HUD_LEFT_WIDTH, HUD_TOP_HIGHT, SettingsData.iScreenW - HUD_TOTAL_WIDTH, SettingsData.iScreenH - HUD_TOTAL_HIGHT };
	SDL_SetClipRect( buffer, &clipRect );

	if ( selectedVehicle )
	{
		cVehicle& v   = *selectedVehicle; // XXX not const is suspicious
		int const spx = v.GetScreenPosX();
		int const spy = v.GetScreenPosY();
		if ( scanChecked() )
		{
			if ( v.data.isBig )
			{
				drawCircle ( spx+ getTileSize(), spy + getTileSize(), v.data.scan * getTileSize(), SCAN_COLOR, buffer );
			}
			else
			{
				drawCircle ( spx + getTileSize()/2, spy + getTileSize()/2, v.data.scan * getTileSize(), SCAN_COLOR, buffer );
			}
		}
		if ( rangeChecked() )
		{
			if (v.data.canAttack & TERRAIN_AIR) drawCircle(spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.range * getTileSize() + 2, RANGE_AIR_COLOR, buffer);
			else drawCircle(spx + getTileSize() / 2, spy + getTileSize() / 2, v.data.range * getTileSize() + 1, RANGE_GROUND_COLOR, buffer);
		}
		if (v.owner == player &&
				(
					v.IsBuilding && v.BuildRounds    == 0 ||
					v.IsClearing && v.ClearingRounds == 0
					) && !v.BuildPath )
		{
			if ( v.data.isBig )
			{
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY - 1)) drawExitPoint(spx - getTileSize(),     spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX    , v.PosY - 1)) drawExitPoint(spx,                spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX + 1, v.PosY - 1)) drawExitPoint(spx + getTileSize(),     spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX + 2, v.PosY - 1)) drawExitPoint(spx + getTileSize() * 2, spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY    )) drawExitPoint(spx - getTileSize(),     spy);
				if ( map->possiblePlace(&v, v.PosX + 2, v.PosY    )) drawExitPoint(spx + getTileSize() * 2, spy);
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY + 1)) drawExitPoint(spx - getTileSize(),     spy + getTileSize());
				if ( map->possiblePlace(&v, v.PosX + 2, v.PosY + 1)) drawExitPoint(spx + getTileSize() * 2, spy + getTileSize());
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY + 2)) drawExitPoint(spx - getTileSize(),     spy + getTileSize() * 2);
				if ( map->possiblePlace(&v, v.PosX    , v.PosY + 2)) drawExitPoint(spx,                spy + getTileSize() * 2);
				if ( map->possiblePlace(&v, v.PosX + 1, v.PosY + 2)) drawExitPoint(spx + getTileSize(),     spy + getTileSize() * 2);
				if ( map->possiblePlace(&v, v.PosX + 2, v.PosY + 2)) drawExitPoint(spx + getTileSize() * 2, spy + getTileSize() * 2);
			}
			else
			{
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY - 1)) drawExitPoint(spx - getTileSize(), spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX    , v.PosY - 1)) drawExitPoint(spx,            spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX + 1, v.PosY - 1)) drawExitPoint(spx + getTileSize(), spy - getTileSize());
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY    )) drawExitPoint(spx - getTileSize(), spy			);
				if ( map->possiblePlace(&v, v.PosX + 1, v.PosY    )) drawExitPoint(spx + getTileSize(), spy			);
				if ( map->possiblePlace(&v, v.PosX - 1, v.PosY + 1)) drawExitPoint(spx - getTileSize(), spy + getTileSize());
				if ( map->possiblePlace(&v, v.PosX    , v.PosY + 1)) drawExitPoint(spx,            spy + getTileSize());
				if ( map->possiblePlace(&v, v.PosX + 1, v.PosY + 1)) drawExitPoint(spx + getTileSize(), spy + getTileSize());
			}
		}
		if (v.PlaceBand)
		{
			if ( v.BuildingTyp.getUnitDataOriginalVersion()->isBig )
			{
				SDL_Rect dest;
				dest.x = HUD_LEFT_WIDTH - (int)( offX * getZoom() ) + getTileSize() * v.BandX;
				dest.y =  HUD_TOP_HIGHT - (int)( offY * getZoom() ) + getTileSize() * v.BandY;
				CHECK_SCALING( GraphicsData.gfx_band_big, GraphicsData.gfx_band_big_org, (float) getTileSize()/64.0 );
				SDL_BlitSurface(GraphicsData.gfx_band_big, NULL, buffer, &dest);
			}
			else
			{
				int x;
				int y;
				mouse->GetKachel(&x, &y);
				if (x == v.PosX || y == v.PosY)
				{
					SDL_Rect dest;
					dest.x = HUD_LEFT_WIDTH - (int)( offX * getZoom() ) + getTileSize() * x;
					dest.y =  HUD_TOP_HIGHT - (int)( offY * getZoom() ) + getTileSize() * y;
					CHECK_SCALING( GraphicsData.gfx_band_small, GraphicsData.gfx_band_small_org, (float) getTileSize()/64.0 );
					SDL_BlitSurface(GraphicsData.gfx_band_small, NULL, buffer, &dest);
					v.BandX     = x;
					v.BandY     = y;
				}
				else
				{
					v.BandX = v.PosX;
					v.BandY = v.PosY;
				}
			}
		}
		if (v.ActivatingVehicle && v.owner == player)
		{
			v.DrawExitPoints(v.StoredVehicles[v.VehicleToActivate]->typ);
		}
	}
	else if ( selectedBuilding )
	{
		int spx,spy;
		spx=selectedBuilding->GetScreenPosX();
		spy=selectedBuilding->GetScreenPosY();
		if ( scanChecked() )
		{
			if ( selectedBuilding->data.isBig )
			{
				drawCircle ( spx+getTileSize(),
				             spy+getTileSize(),
				             selectedBuilding->data.scan*getTileSize(),SCAN_COLOR,buffer );
			}
			else
			{
				drawCircle ( spx+getTileSize()/2,
				             spy+getTileSize()/2,
				             selectedBuilding->data.scan*getTileSize(),SCAN_COLOR,buffer );
			}
		}
		if ( rangeChecked() && (selectedBuilding->data.canAttack & TERRAIN_GROUND) && !selectedBuilding->data.explodesOnContact )
		{
			drawCircle ( spx+getTileSize()/2,
			             spy+getTileSize()/2,
			             selectedBuilding->data.range*getTileSize()+2,RANGE_GROUND_COLOR,buffer );
		}
		if ( rangeChecked() && (selectedBuilding->data.canAttack & TERRAIN_AIR) )
		{
			drawCircle ( spx+getTileSize()/2,
			             spy+getTileSize()/2,
			             selectedBuilding->data.range*getTileSize()+2,RANGE_AIR_COLOR,buffer );
		}

		if (selectedBuilding->BuildList                              &&
				selectedBuilding->BuildList->Size()                      &&
				!selectedBuilding->IsWorking                             &&
				(*selectedBuilding->BuildList)[0]->metall_remaining <= 0 &&
				selectedBuilding->owner == player)
		{
			selectedBuilding->DrawExitPoints((*selectedBuilding->BuildList)[0]->typ);
		}
		if ( selectedBuilding->ActivatingVehicle&&selectedBuilding->owner==player )
		{
			selectedBuilding->DrawExitPoints(selectedBuilding->StoredVehicles[selectedBuilding->VehicleToActivate]->typ);
		}
	}
	player->DrawLockList();

	SDL_SetClipRect( buffer, NULL );
}

void cGameGUI::drawExitPoint ( int x, int y )
{
	SDL_Rect dest, scr;
	int nr = ANIMATION_SPEED%5;
	scr.y = 0;
	scr.h = scr.w = getTileSize();
	scr.x = getTileSize()*nr;
	dest.x = x;
	dest.y = y;
	float factor = (float)(getTileSize()/64.0);

	CHECK_SCALING( GraphicsData.gfx_exitpoints, GraphicsData.gfx_exitpoints_org, factor );
	SDL_BlitSurface ( GraphicsData.gfx_exitpoints, &scr, buffer, &dest );
}


/*void cHud::ExtraPlayers ( string sPlayer, int iColor, int iPos, bool bFinished, bool bActive)
{
	if(bShowPlayers)
	{
		//BEGIN PREP WORK
		//draw players beside minimap
		SDL_Rect rDest;
		SDL_Rect rSrc = { 0, 0, GraphicsData.gfx_hud_extra_players->w, GraphicsData.gfx_hud_extra_players->h};

		if(SettingsData.iScreenH >= 768) //draw players under minimap if screenres is big enough
		{
			rSrc.x = 18; //skip eyecandy spit before playerbar

			rDest.x = 3;
			rDest.y = 482 + GraphicsData.gfx_hud_extra_players->h * iPos; //draw players downwards
		}
		else //draw players beside minimap if screenres is to small
		{
			rDest.x = 161;
			rDest.y = 480 - 82 - GraphicsData.gfx_hud_extra_players->h * iPos; //draw players upwards
		}


		SDL_Rect rDot = { 10 , 0, 10, 10 }; //for green dot
		SDL_Rect rDotDest = { rDest.x + 23 - rSrc.x, rDest.y + 6, rDot.w, rDot.h };

		SDL_Rect rColorSrc = { 0, 0, 10, 12 };
		SDL_Rect rColorDest = { rDest.x + 40 - rSrc.x, rDest.y + 6, rColorSrc.w, rColorSrc.h };
		//END PREP WORK
		//BEGIN DRAW PLAYERS
		SDL_BlitSurface( GraphicsData.gfx_hud_extra_players, &rSrc, GraphicsData.gfx_hud, &rDest ); //blit box
		if(!bFinished)
		{
			rDot.x = 0; //red dot
		}
		if(bActive)
		{
			SDL_BlitSurface( GraphicsData.gfx_player_ready, &rDot, GraphicsData.gfx_hud, &rDotDest ); //blit dot
			SDL_BlitSurface(OtherData.colors[iColor], &rColorSrc, GraphicsData.gfx_hud, &rColorDest ); //blit color
		}
		else
		{
			font->showText(rColorDest.x+3, rColorDest.y+2, "X" , FONT_LATIN_SMALL_RED, GraphicsData.gfx_hud); //blit X for defeated/dropped players
		}
		font->showText(rDest.x+= (59 - rSrc.x), rDest.y+6, sPlayer , FONT_LATIN_NORMAL, GraphicsData.gfx_hud); //blit name
		//END DRAW PLAYERS
	}
}*/
