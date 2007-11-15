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
#include <SDL.h>
//#include <dirent.h>
#include "dialog.h"
#include "game.h"
#include "mouse.h"
#include "keyinp.h"
#include "fonts.h"
#include "sound.h"
#include "menu.h"
#include "pcx.h"
#include "files.h"
#include "log.h"
//TODO: dialogs don't interpret \n e.g. from translation files and just print \n in the text on the dialog -- beko
// Zeigt einen Ja/Nein Dialog an:
bool ShowYesNo ( string text )
{
	int b, x, y, lx = 0, ly = 0, lb = 0;
	bool ret = false;
	SDL_Rect dest;

	mouse->SetCursor ( CHand );
	game->DrawMap ( false );
	SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );

	if ( SettingsData.bAlphaEffects )
	{
		SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
	}

	LoadPCXtoSF ( ( char * ) GraphicsData.Dialog2Path.c_str(), GraphicsData.gfx_dialog );

	dest.x = 640 / 2 - 300 / 2;
	dest.y = 480 / 2 - 231 / 2;
	dest.w = 300;
	dest.h = 231;
	SDL_BlitSurface ( GraphicsData.gfx_dialog, NULL, buffer, &dest );
	PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 150, false );
	PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_No" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, false );
	dest.x += 20;
	dest.w -= 40;
	dest.y += 20;
	dest.h -= 150;
	fonts->OutTextBlock ( ( char * ) text.c_str(), dest, buffer );
	SHOW_SCREEN
	mouse->draw ( false, screen );

	while ( 1 )
	{
		if ( game )
		{
			game->engine->Run();
			game->HandleTimer();
		}

		// Eingaben holen:
		SDL_PumpEvents();

		// Die Maus:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		x = mouse->x;

		y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		// Ja Button:
		if ( x >= 640 / 2 - 300 / 2 + 80 && x < 640 / 2 - 300 / 2 + 80 + 150 && y >= 480 / 2 - 231 / 2 + 150 && y < 480 / 2 - 231 / 2 + 150 + 29 )
		{
			if ( b && !lb )
			{
				PlayFX ( SoundData.SNDHudButton );
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 150, true );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				ret = true;
				break;
			}
		}

		// Nein Button:
		if ( x >= 640 / 2 - 300 / 2 + 80 && x < 640 / 2 - 300 / 2 + 80 + 150 && y >= 480 / 2 - 231 / 2 + 185 && y < 480 / 2 - 231 / 2 + 185 + 29 )
		{
			if ( b && !lb )
			{
				PlayFX ( SoundData.SNDHudButton );
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_No" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, true );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				ret = false;
				break;
			}
		}

		lx = x;

		ly = y;
		lb = b;
		SDL_Delay ( 1 );
	}

	LoadPCXtoSF ( ( char * ) GraphicsData.DialogPath.c_str(), GraphicsData.gfx_dialog );

	game->fDrawMap = true;
	return ret;
}

// Zeigt einen Dialog für eine Zahleneingabe an:
int ShowNumberInput ( string text )
{
	int b, x, y, lx = 0, ly = 0, lb = 0;
	int value = 2;
	SDL_Rect dest, scr;
	Uint8 *keystate;
	bool Cursor = true;
	string stmp;

	LoadPCXtoSF ( ( char * ) GraphicsData.Dialog3Path.c_str(), GraphicsData.gfx_dialog );
	dest.x = 640 / 2 - 300 / 2;
	dest.y = 480 / 2 - 231 / 2;
	dest.w = 300;
	dest.h = 231;
	SDL_BlitSurface ( GraphicsData.gfx_dialog, NULL, buffer, &dest );
	PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, false );
	dest.x += 20;
	dest.w -= 40;
	dest.y += 20;
	dest.h -= 150;
	fonts->OutTextBlock ( ( char * ) text.c_str(), dest, buffer );
	fonts->OutText ( "_", 20 + 170 + 7, 167 + 124 + 4, buffer );
	InputStr = "";
	SHOW_SCREEN
	mouse->draw ( false, screen );

	while ( 1 )
	{
		// Eingaben holen:
		SDL_PumpEvents();
		// Die Maus:
		mouse->GetPos();
		b = mouse->GetMouseButton();
		x = mouse->x;
		y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		// Die Tastatur:
		keystate = SDL_GetKeyState ( NULL );

		if ( DoKeyInp ( keystate ) || timer2 )
		{
			scr.x = 20;
			scr.y = 167;
			dest.w = scr.w = 259;
			dest.h = scr.h = 17;
			dest.x = 20 + 170;
			dest.y = 167 + 124;
			SDL_BlitSurface ( GraphicsData.gfx_dialog, &scr, buffer, &dest );
			stmp = InputStr;
			stmp += "_";

			if ( fonts->GetTextLen ( ( char * ) stmp.c_str() ) >= 246 )
			{
				InputStr.erase ( InputStr.length() - 1, 0 );
			}

			if ( Cursor )
			{
				Cursor = false;
				stmp = InputStr;
				stmp += "_";
				fonts->OutText ( ( char * ) stmp.c_str(), dest.x + 7, dest.y + 4, buffer );
			}
			else
			{
				Cursor = true;
				fonts->OutText ( ( char * ) InputStr.c_str(), dest.x + 7, dest.y + 4, buffer );
			}

			SHOW_SCREEN

			mouse->draw ( false, screen );
		}

		// OK Button:
		if ( x >= 640 / 2 - 300 / 2 + 80 && x < 640 / 2 - 300 / 2 + 80 + 150 && y >= 480 / 2 - 231 / 2 + 185 && y < 480 / 2 - 231 / 2 + 185 + 29 )
		{
			if ( b && !lb )
			{
				PlayFX ( SoundData.SNDHudButton );
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, true );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				break;
			}
		}

		lx = x;

		ly = y;
		lb = b;
		SDL_Delay ( 1 );
	}

	LoadPCXtoSF ( ( char * )  GraphicsData.DialogPath.c_str(), GraphicsData.gfx_dialog );

	value = atoi ( InputStr.c_str() );
	return value;
}

void ShowOK ( string text, bool pure )
{
	int b, x, y, lx = 0, ly = 0, lb = 0;
	SDL_Rect dest;

	mouse->SetCursor ( CHand );

	if ( !pure )
	{
		dest.x = 180;
		dest.y = 18;
		dest.w = dest.h = 448;
		SDL_FillRect ( buffer, &dest, 0 );
		SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );
		dest.x = 15;
		dest.y = 356;
		dest.w = dest.h = 112;
		SDL_FillRect ( buffer, &dest, 0 );

		if ( SettingsData.bAlphaEffects )
		{
			SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
		}
	}

	LoadPCXtoSF ( ( char * ) GraphicsData.Dialog2Path.c_str(), GraphicsData.gfx_dialog );

	dest.x = 640 / 2 - 300 / 2;
	dest.y = 480 / 2 - 231 / 2;
	dest.w = 300;
	dest.h = 231;
	SDL_BlitSurface ( GraphicsData.gfx_dialog, NULL, buffer, &dest );
	PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, false );
	dest.x += 20;
	dest.w -= 40;
	dest.y += 20;
	dest.h -= 150;
	fonts->OutTextBlock ( ( char * ) text.c_str(), dest, buffer );
	SHOW_SCREEN
	mouse->draw ( false, screen );

	while ( 1 )
	{
		if ( !pure && game )
		{
			game->engine->Run();
			game->HandleTimer();
		}

		// Eingaben holen:
		SDL_PumpEvents();

		// Die Maus:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		x = mouse->x;

		y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		// OK Button:
		if ( x >= 640 / 2 - 300 / 2 + 80 && x < 640 / 2 - 300 / 2 + 80 + 150 && y >= 480 / 2 - 231 / 2 + 185 && y < 480 / 2 - 231 / 2 + 185 + 29 )
		{
			if ( b && !lb )
			{
				PlayFX ( SoundData.SNDHudButton );
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, true );
				SHOW_SCREEN
				mouse->draw ( false, screen );
				break;
			}
		}

		lx = x;

		ly = y;
		lb = b;
		SDL_Delay ( 1 );
	}

	LoadPCXtoSF ( ( char * ) GraphicsData.DialogPath.c_str(), GraphicsData.gfx_dialog );

	if ( !pure )
		game->fDrawMap = true;
}

/** shows licence infobox on screen (don't call this within game since I this doesn't care about ongoing engine 
*@author beko
*/
void ShowLicence ()
{
	int b, x, y, lx = 0, ly = 0, lb = 0, index=0;
	string sLicence1;
	string sLicence2;
	string sLicence3;
	SDL_Rect rDialogOnScreen; //dialog blitted on the screen
	SDL_Rect rDialog; //our dialog
	SDL_Rect rDialogBoxBlack; //spot to draw text inside
	SDL_Rect rDialogBoxBlackOffset = {0,0,0,0}; //for redrawing only the black part PLUS buttons
	SDL_Rect rArrowUp;
	SDL_Rect rArrowDown;
	SDL_Surface *SfDialog;
	SDL_Surface *SfButton;
	
	sLicence1 = "\
               M.A.X. Reloaded\n\
Copyright (C) 2007  by it's authors\n\
\n\
This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version.";
sLicence2 = "\
               M.A.X. Reloaded\n\
Copyright (C) 2007  by it's authors\n\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.";
sLicence3="\
               M.A.X. Reloaded\n\
Copyright (C) 2007  by it's authors\n\n\
You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software \
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";

	
	mouse->SetCursor ( CHand );
	SDL_BlitSurface ( screen, NULL, buffer, NULL ); //write screen to buffer for proper background "picture"

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 300, 231, SettingsData.iColourDepth, 0, 0, 0, 0 );
	if (FileExists(GFXOD_DIALOG4));
	{	
		LoadPCXtoSF ( GFXOD_DIALOG4, SfDialog ); //load dialog4.pxc
	}
	
	//set some rects
	rDialog.x = screen->w / 2 - SfDialog->w / 2;
	rDialog.y = screen->h / 2 - SfDialog->h / 2;
	
	rDialogOnScreen.x = rDialog.x + 32; 
	rDialogBoxBlack.x=32;
	rDialogOnScreen.w = 232; 
	rDialogBoxBlack.w=SfDialog->w-32;
	rDialogOnScreen.y = rDialog.y + 28; 
	rDialogBoxBlack.y=28;
	rDialogOnScreen.h = 142; 
	rDialogBoxBlack.h=SfDialog->h-28;
	
	rDialogBoxBlackOffset.x = screen->w / 2 - SfDialog->w / 2 + 32;
	rDialogBoxBlackOffset.y = screen->h / 2 - SfDialog->h / 2 + 28; //w, h not needed since SDL_BlitSurface ignores these for destination rect
	
	//create start dialog
	SDL_BlitSurface ( SfDialog, NULL, buffer, &rDialog );
	PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ).c_str(), rDialog.x + 80, rDialog.y + 185, false );
	fonts->OutTextBlock ( ( char * ) sLicence1.c_str(), rDialogOnScreen, buffer );
	
	//draw left arrow "up"
	rArrowUp.x = screen->w / 2 - SfDialog->w / 2 + 241;
	rArrowUp.y = screen->h / 2 - SfDialog->h / 2 + 187;
	rArrowUp.w = 18;
	rArrowUp.h = 17;
	drawDialogArrow(buffer, &rArrowUp, ARROW_TYPE_UP); //up is deselected now since not usable at start here (index already 0)

	//draw right arrow "down"
	rArrowDown.x = screen->w / 2 - SfDialog->w / 2 + 261;
	rArrowDown.y = screen->h / 2 - SfDialog->h / 2 + 187;
	rArrowDown.w = 18;
	rArrowDown.h = 17;
	//drawDialogArrow(buffer, &rArrowDown, ARROW_TYPE_DOWN); //uncommented since we want Up-Arrow to be active at start
	
	SHOW_SCREEN
	mouse->draw ( false, screen );


	while ( 1 )
	{
		SDL_PumpEvents(); //get hid-input
		mouse->GetPos(); //get mouseposition
		b = mouse->GetMouseButton();
		x = mouse->x; y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		// OK Button:
		if ( x >= rDialog.x + 80 && x < rDialog.x + 80 + 150 && y >= rDialog.y + 185 && y < rDialog.y + 185 + 29 )
		{
			if ( b && !lb )
			{
				PlayFX ( SoundData.SNDHudButton );
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_Yes" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, true );
				EnterMenu();
				SHOW_SCREEN
				mouse->draw ( false, screen );
				SDL_Delay ( 2 );
				break;
			}
		}
		//button up (left)
		else if ( x >= rDialog.x + 241 && x < rDialog.x + 241 + 18 && y >= rDialog.y + 187 && y < rDialog.y + 187 + 17 )
		{
			if ( b && !lb )
			{
				SDL_BlitSurface ( SfDialog, &rDialogBoxBlack, buffer, &rDialogBoxBlackOffset );  //redraw empty textbox
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ).c_str(), rDialog.x + 80, rDialog.y + 185, false );
		
				switch(index)
				{
					case 1 : 
						index = 0; 
						PlayFX ( SoundData.SNDHudButton );
						drawDialogArrow(buffer, &rArrowUp, ARROW_TYPE_UP);
						fonts->OutTextBlock ( ( char * ) sLicence1.c_str(), rDialogOnScreen, buffer );
						break;
					case 2 : 
						index = 1; 
						PlayFX ( SoundData.SNDHudButton );
						fonts->OutTextBlock ( ( char * ) sLicence2.c_str(), rDialogOnScreen, buffer );
						break;
					default: //should not happen
						cLog::write("Invalid index - can't show text in dialog",cLog::eLOG_TYPE_WARNING);
						drawDialogArrow(buffer, &rArrowUp, ARROW_TYPE_UP); 
						fonts->OutTextBlock ( ( char * ) sLicence1.c_str(), rDialogOnScreen, buffer );
				}
								
				SHOW_SCREEN
				mouse->draw ( false, screen );
			}
		}
		//button down (right)
		else if ( x >= rDialog.x + 261 && x < rDialog.x + 261 + 18 && y >= rDialog.y + 187 && y < rDialog.y + 187 + 17 )
		{
			if ( b && !lb )
			{
				SDL_BlitSurface ( SfDialog, &rDialogBoxBlack, buffer, &rDialogBoxBlackOffset );  //redraw empty textbox
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ).c_str(), rDialog.x + 80, rDialog.y + 185, false );

				switch(index)
				{
					case 0 : 
						index = 1;
						PlayFX ( SoundData.SNDHudButton );
						fonts->OutTextBlock ( ( char * ) sLicence2.c_str(), rDialogOnScreen, buffer );
						break;
					case 1 : 
						index = 2;
						PlayFX ( SoundData.SNDHudButton );
						drawDialogArrow(buffer, &rArrowDown, ARROW_TYPE_DOWN);
						fonts->OutTextBlock ( ( char * ) sLicence3.c_str(), rDialogOnScreen, buffer );
						break;						
					default: //should not happen
						cLog::write("Invalid index - can't show text in dialog",cLog::eLOG_TYPE_WARNING);
						drawDialogArrow(buffer, &rArrowDown, ARROW_TYPE_DOWN); 
						fonts->OutTextBlock ( ( char * ) sLicence3.c_str(), rDialogOnScreen, buffer );
				}
				
				SHOW_SCREEN
				mouse->draw ( false, screen );
			}
		}		

		lx = x;

		ly = y;
		lb = b;
		SDL_Delay ( 1 );
	}
	SDL_FreeSurface ( SfDialog ); //remove old surface from heap
}

/**gets tiny up or down arrow from hud gfx and writes arrow to surface*/
void drawDialogArrow(SDL_Surface *surface, SDL_Rect *dest, int type)
{
	SDL_Rect scr = {230,151,18,17}; //arrow gfx is 18x17 in size

	switch(type)
	{
		case ARROW_TYPE_UP :
			scr.x = 230;
			break;
		case ARROW_TYPE_DOWN : 
			scr.x = 249;
			break;
		default: 
			cLog::write("Invalid arrow type - can't serve here", cLog::eLOG_TYPE_WARNING);
			break;
	}
	
	if(surface)
	{
		//get gfx arrow from menu graphic and blit it to tmp
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,dest ); 
	}
	else
	{
		cLog::write("Invalid surface - can't apply gfx arrow", cLog::eLOG_TYPE_WARNING);
	}
}