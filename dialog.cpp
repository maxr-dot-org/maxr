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

void ShowLicence ()
{
	int b, x, y, lx = 0, ly = 0, lb = 0, index=0;
	string sLicence1;
	string sLicence2;
	string sLicence3;

	sLicence1 = "\
               M.A.X. Reloaded\n\
Copyright (C) 2007  by it's authors\n\
\n\
This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version.";
sLicence2 = "This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.";
sLicence3="You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software \
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";
	
	SDL_Rect dest;
	SDL_Rect rDialogOnScreen;
	SDL_Rect rDialog;
	SDL_Rect rDialogBoxBlack;
	SDL_Rect rDialogBoxBlackOffset = {0,0,0,0};
	SDL_Rect rArrowUp;
	SDL_Rect rArrowDown;
	SDL_Surface *SfDialog;
	SDL_Surface *SfButton;
	
	mouse->SetCursor ( CHand );
	SDL_BlitSurface ( screen, NULL, buffer, NULL ); //write screen to buffer for proper background "picture"

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 300, 231, 32, 0, 0, 0, 0 );
	if (FileExists(GFXOD_DIALOG4));
	{	
		LoadPCXtoSF ( GFXOD_DIALOG4, SfDialog ); //load dialog4.pxc
	}
	SfButton = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 17, 17, 32, 0, 0, 0, 0 );
	//PlaceMenuButton
	
	rDialog.x = dest.x = screen->w / 2 - SfDialog->w / 2;
	rDialog.y = dest.y = screen->h / 2 - SfDialog->h / 2;
	rDialog.w = dest.w = SfDialog->w;
	rDialog.h = dest.h = SfDialog->h;
	rDialogOnScreen.x = dest.x + 32; rDialogBoxBlack.x=32;
	rDialogOnScreen.w = dest.w = 232; rDialogBoxBlack.w=232;
	rDialogOnScreen.y = dest.y + 28; rDialogBoxBlack.y=28;
	rDialogOnScreen.h = dest.h = 142; rDialogBoxBlack.h=142;
	rDialogBoxBlackOffset.x = screen->w / 2 - SfDialog->w / 2 + 32;
	rDialogBoxBlackOffset.y = dest.y = screen->h / 2 - SfDialog->h / 2 + 28; //w, h not needed since SDL_BlitSurface ignores these for destination rect
	
	SDL_BlitSurface ( SfDialog, NULL, buffer, &rDialog );
	PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ).c_str(), rDialog.x + 80, rDialog.y + 185, false );
	
	fonts->OutTextBlock ( ( char * ) sLicence1.c_str(), rDialogOnScreen, buffer );
	
	
	//draw left arrow "up"
	dest.x = rDialog.x + 242;
	dest.y = rDialog.y + 187;
	dest.w = SfDialog->w;
	dest.h = SfDialog->h;
	SDL_BlitSurface ( SfButton, NULL, buffer, &dest );
	//draw right arrow "down"
	dest.x = rDialog.x + 262;
	SDL_BlitSurface ( SfButton, NULL, buffer, &dest );
	
	SHOW_SCREEN
	mouse->draw ( false, screen );
	

	rArrowUp.x = screen->w / 2 - SfDialog->w / 2 + 242;
	rArrowUp.y = screen->h / 2 - SfDialog->h / 2 + 187;
	rArrowUp.w = 17; //TODO: read gfx-size here
	rArrowUp.h = 17; //TODO: read gfx-size here
	
	rArrowDown.x = screen->w / 2 - SfDialog->w / 2 + 262;
	rArrowDown.y = screen->h / 2 - SfDialog->h / 2 + 187;
	rArrowDown.w = 17; //TODO: read gfx-size here
	rArrowDown.h = 17; //TODO: read gfx-size here
		
	dest.x = screen->w / 2 - SfDialog->w / 2;
	dest.y = screen->h / 2 - SfDialog->h / 2;
	dest.w = SfDialog->w;
	dest.h = SfDialog->h;

	while ( 1 )
	{
		// Eingaben holen:
		SDL_PumpEvents();
		// Die Maus:
		mouse->GetPos();
		b = mouse->GetMouseButton();
		x = mouse->x; y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		// OK Button:
		if ( x >= dest.x + 80 && x < dest.x + 80 + 150 && y >= dest.y + 185 && y < dest.y + 185 + 29 )
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
		else if ( x >= dest.x + 242 && x < dest.x + 242 + 17 && y >= dest.y + 187 && y < dest.y + 187 + 17 )
		{
			if ( b && !lb )
			{
				mouse->draw ( false, screen );
				PlayFX ( SoundData.SNDHudButton );
				SDL_FillRect ( buffer,&rArrowUp,0x00C000 ); //TODO: menugraphic arrow up here
				SDL_BlitSurface ( SfDialog, &rDialogBoxBlack, buffer, &rDialogBoxBlackOffset );  //redraw empty textbox
								
				switch(index)
				{
					case 1 : 
						index = 0; 
						fonts->OutTextBlock ( ( char * ) sLicence1.c_str(), rDialogOnScreen, buffer );
						break;
					case 2 : 
						index = 1; 
						fonts->OutTextBlock ( ( char * ) sLicence2.c_str(), rDialogOnScreen, buffer );
						break;
					default: fonts->OutTextBlock ( ( char * ) sLicence1.c_str(), rDialogOnScreen, buffer );
				}
								
				SHOW_SCREEN
				mouse->draw ( false, screen );
			}
		}
		//button down (right)
		else if ( x >= dest.x + 262 && x < dest.x + 262 + 17 && y >= dest.y + 187 && y < dest.y + 187 + 17 )
		{
			if ( b && !lb )
			{
				PlayFX ( SoundData.SNDHudButton );
				SDL_FillRect ( buffer,&rArrowDown,0x00C000 );//TODO: menugraphic arrow down here
				SDL_BlitSurface ( SfDialog, &rDialogBoxBlack, buffer, &rDialogBoxBlackOffset );  //redraw empty textbox

				switch(index)
				{
					case 0 : 
						index = 1;
						fonts->OutTextBlock ( ( char * ) sLicence2.c_str(), rDialogOnScreen, buffer );
						break;
					case 1 : 
						index = 2;
						fonts->OutTextBlock ( ( char * ) sLicence3.c_str(), rDialogOnScreen, buffer );
						break;						
					default: fonts->OutTextBlock ( ( char * ) sLicence3.c_str(), rDialogOnScreen, buffer );
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
	SDL_FreeSurface ( SfButton );
}
