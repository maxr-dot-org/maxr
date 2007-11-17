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
void showLicence ()
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
				PlaceSmallButton ( lngPack.Translate ( "Text~Menu_Main~Button_OK" ).c_str(), 640 / 2 - 300 / 2 + 80, 480 / 2 - 231 / 2 + 185, true );
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

// showPreferences ////////////////////////////////////////////////////////////
// Zeigt das Präferenzenfenster an:
void showPreferences ( void )
{	
	bool OldMusicMute, OldSoundMute, OldVoiceMute, OldbAutoSave, OldbAnimations, OldbShadows, OldbAlphaEffects, OldbDamageEffects, OldbDamageEffectsVehicles, OldbMakeTracks;
	bool FertigPressed = false, AbbruchPressed = false, Input = false;
	int OldiScrollSpeed, OldMusicVol, OldSoundVol, OldVoiceVol;
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b;
	string OldName;
	string stmp;
	string sTmp;
	SDL_Rect scr, dest, rFont;
	Uint8 *keystate;
	bool cursor = true;
	SDL_Surface *SfDialog;
	
	//position x of all sliderbars
	#define BAR_X 140
	#define BAR_Y 110
	#define CELLSPACE 20
	#define CELLSPACE_FONT 20

	OldMusicMute = SettingsData.MusicMute;
	OldSoundMute = SettingsData.SoundMute;
	OldVoiceMute = SettingsData.VoiceMute;
	OldbAutoSave = SettingsData.bAutoSave;
	OldbAnimations = SettingsData.bAnimations;
	OldbShadows = SettingsData.bShadows;
	OldbAlphaEffects = SettingsData.bAlphaEffects;
	OldiScrollSpeed = SettingsData.iScrollSpeed;
	OldMusicVol = SettingsData.MusicVol;
	OldSoundVol = SettingsData.SoundVol;
	OldVoiceVol = SettingsData.VoiceVol;
	OldName = game->ActivePlayer->name;
	OldbDamageEffects = SettingsData.bDamageEffects;
	OldbDamageEffectsVehicles = SettingsData.bDamageEffectsVehicles;
	OldbMakeTracks = SettingsData.bMakeTracks;

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 400, 422, SettingsData.iColourDepth, 0, 0, 0, 0 );

	if ( SettingsData.bAlphaEffects )
	{
		SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
	}

	if(FileExists(GFXOD_DIALOG5));
	{
		LoadPCXtoSF (GFXOD_DIALOG5, SfDialog );	
		//blit black titlebar behind textfield for playername
		scr.x=108;
		scr.y=12;
		dest.w=scr.w=186;
		dest.h=scr.h=18;
		dest.x=108;
		dest.y=154;
		SDL_BlitSurface ( SfDialog,&scr,SfDialog,&dest ); 

	}
	
	 //blit dialog to buffer
	dest.x = 120;
	dest.y = 29;
	dest.w = SfDialog->w;
	dest.h = SfDialog->h;
	
	SDL_BlitSurface ( SfDialog, NULL, buffer, &dest );
	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Cancel" ), false, 118+120, 383+29, buffer); 
	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Done" ), false, 208+120, 383+29, buffer); 
	
	rFont.x = 120 + 160; //TOFIX: Text not centered
	rFont.y = 29 + 15;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Preferences" ).c_str(),rFont.x,rFont.y,buffer);


	//BEGIN BLOCK SOUND
	//Headline
	sTmp = lngPack.Translate ( "Text~Game_Settings~Title_Volume" ) + ":";
	rFont.x = 145;
	rFont.y = 85;
	rFont.w = fonts->GetTextLen(sTmp.c_str());
	fonts->OutText(sTmp.c_str(),rFont.x,rFont.y,buffer);

	//Music
	rFont.x = 145; 	rFont.w = 100;
	rFont.y = 105;
	rFont.h = CELLSPACE_FONT;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Music" ).c_str(),rFont.x,rFont.y,buffer);
	drawSlider ( BAR_X+120,BAR_Y,SettingsData.MusicVol*2,SfDialog );
	drawCheckbox ( 210+120,73+29,SettingsData.MusicMute, buffer);
	rFont.x = 355; 	rFont.w = 140;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Disable" ).c_str(),rFont.x,rFont.y,buffer);
		
	//Effectsound
	rFont.x = 145; 	rFont.w = 100;
	rFont.y += CELLSPACE_FONT;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Effects" ).c_str(),rFont.x,rFont.y,buffer);
	drawSlider ( BAR_X+120,BAR_Y+CELLSPACE,SettingsData.SoundVol*2, SfDialog );
	drawCheckbox ( 210+120,93+29,SettingsData.SoundMute,buffer );
	rFont.x = 355; 	rFont.w = 140;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Disable" ).c_str(),rFont.x,rFont.y,buffer);

	//Voices
	rFont.x = 145; 	rFont.w = 100;
	rFont.y += CELLSPACE_FONT;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Voices" ).c_str(),rFont.x,rFont.y,buffer);
	drawSlider ( BAR_X+120,BAR_Y+CELLSPACE*2,SettingsData.VoiceVol*2,SfDialog );
	drawCheckbox ( 210+120,113+29,SettingsData.VoiceMute,buffer );
	rFont.x = 355; 	rFont.w = 140;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Disable" ).c_str(),rFont.x,rFont.y,buffer);
	//END BLOCK SOUND
	//BEGIN BLOCK PLAYERNAME	
	
	
	rFont.x = 145; 	rFont.w = 100;
	rFont.y = 158+29;
	fonts->OutText(lngPack.Translate( "Text~Game_Start~Title_Player_Name" ).c_str(),rFont.x,rFont.y,buffer);
	fonts->OutText ( ( char * ) game->ActivePlayer->name.c_str(),122+120,158+29,buffer );

	//END BLOCK PLAYERNAME

	rFont.x = 145+25; rFont.w = 100;
	rFont.y = 193+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Animation" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 25+120,193+29,SettingsData.bAnimations,buffer );
	
	rFont.x = 145+25; rFont.w = 100;
	rFont.y = 213+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Shadows" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 25+120,213+29,SettingsData.bShadows,buffer );
	
	rFont.x = 145+25; rFont.w = 100;
	rFont.y = 233+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Alphaeffects" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 25+120,233+29,SettingsData.bAlphaEffects,buffer );
	
	rFont.x = 145+210; rFont.w = 100;
	rFont.y = 193+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_ShowDamage" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 210+120,193+29,SettingsData.bDamageEffects,buffer );
	
	rFont.x = 145+210; rFont.w = 100;
	rFont.y = 213+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_ShowDamageVehicle" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 210+120,213+29,SettingsData.bDamageEffectsVehicles,buffer );
	
	rFont.x = 145+210; rFont.w = 100;
	rFont.y = 233+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Tracks" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 210+120,233+29,SettingsData.bMakeTracks,buffer );
	
	rFont.x = 145; 	rFont.w = 100;
	rFont.y = 261+25;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Scrollspeed" ).c_str(),rFont.x,rFont.y,buffer);
	drawSlider ( BAR_X+120,261+29,SettingsData.iScrollSpeed*5, SfDialog );
	
	rFont.x = 145+25; rFont.w = 100;
	rFont.y = 290+33;
	fonts->OutText(lngPack.Translate( "Text~Game_Settings~Title_Autosave" ).c_str(),rFont.x,rFont.y,buffer);
	drawCheckbox ( 25+120,290+29,SettingsData.bAutoSave,buffer );
	
	SHOW_SCREEN

	mouse->GetBack ( buffer );
	while ( 1 )
	{
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Tasten prüfen:
		keystate=SDL_GetKeyState ( NULL );
		if ( Input )
		{
			if ( DoKeyInp ( keystate ) ||timer2 )
			{
				scr.x=116;
				scr.y=154;
				dest.w=scr.w=184;
				dest.h=scr.h=17;
				dest.x=116+120;
				dest.y=154+29;
				SDL_BlitSurface ( SfDialog,&scr,buffer,&dest );
				if ( InputEnter )
				{
					fonts->OutText ( ( char * ) InputStr.c_str(),122+120,158+29,buffer );
					Input=false;
					game->ActivePlayer->name=InputStr;
				}
				else
				{
					stmp = InputStr; stmp += "_";
					if ( fonts->GetTextLen ( ( char * ) stmp.c_str() ) >178 )
					{
						InputStr.erase ( InputStr.length()-1 );
					}
					if ( cursor )
					{
						stmp = InputStr; stmp += "_";
						fonts->OutText ( ( char * ) stmp.c_str(),122+120,158+29,buffer );
					}
					else
					{
						fonts->OutText ( ( char * ) InputStr.c_str(),122+120,158+29,buffer );
					}
					if ( timer2 ) cursor=!cursor;
				}
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}
		if ( b )
		{		//firstbar
			if ( x>=BAR_X+120&&x<BAR_X+120+57&&y>=81+29-7&&y<=81+29+10&& ( x!=LastMouseX||y!=LastMouseY||!LastB ) )
			{
				SettingsData.MusicVol= ( x- ( BAR_X+120 ) ) * (int)( 128.0/57 );
				if ( SettingsData.MusicVol>=125 ) SettingsData.MusicVol=128;
				drawSlider ( BAR_X+120,81+29,SettingsData.MusicVol*2, SfDialog );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SetMusicVol ( SettingsData.MusicVol );
			}
			else if ( x>=BAR_X+120&&x<BAR_X+120+57&&y>=BAR_Y+CELLSPACE-7&&y<=BAR_Y+CELLSPACE+10&& ( x!=LastMouseX||y!=LastMouseY||!LastB ) )
			{
				SettingsData.SoundVol= ( x- ( BAR_X+120 ) ) * (int)( 128.0/57 );
				if ( SettingsData.SoundVol>=125 ) SettingsData.SoundVol=128;
				drawSlider ( BAR_X+120,BAR_Y+CELLSPACE,SettingsData.SoundVol*2, SfDialog );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( x>=BAR_X+120&&x<BAR_X+120+57&&y>=BAR_Y+CELLSPACE*2-7&&y<=BAR_Y+CELLSPACE*2+10&& ( x!=LastMouseX||y!=LastMouseY||!LastB ) )
			{
				SettingsData.VoiceVol= ( x- ( BAR_X+120 ) ) * (int)( 128.0/57 );
				if ( SettingsData.VoiceVol>=125 ) SettingsData.VoiceVol=128;
				drawSlider ( BAR_X+120,BAR_Y+CELLSPACE*2,SettingsData.VoiceVol*2, SfDialog );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			else if ( x>=BAR_X+120&&x<BAR_X+120+57&&y>=261+29-7&&y<=261+29+10&& ( x!=LastMouseX||y!=LastMouseY||!LastB ) )
			{
				SettingsData.iScrollSpeed= ( int ) ( ( x- ( BAR_X+120 ) ) * ( 255.0/57 ) ) /5;
				drawSlider ( BAR_X+120,261+29,SettingsData.iScrollSpeed*5, SfDialog );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
			if ( !LastB )
			{
				if ( x>=210+120&&x<210+120+18&&y>=73+29&&y<73+29+17 )
				{
					SettingsData.MusicMute=!SettingsData.MusicMute;
					drawCheckbox ( 210+120,73+29,SettingsData.MusicMute,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					if ( SettingsData.MusicMute )
					{
						StopMusic();
					}
					else
					{
						StartMusic();
					}
				}
				else if ( x>=210+120&&x<210+120+18&&y>=93+29&&y<93+29+17 )
				{
					SettingsData.SoundMute=!SettingsData.SoundMute;
					drawCheckbox ( 210+120,93+29,SettingsData.SoundMute,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=210+120&&x<210+120+18&&y>=113+29&&y<113+29+17 )
				{
					SettingsData.VoiceMute=!SettingsData.VoiceMute;
					drawCheckbox ( 210+120,113+29,SettingsData.VoiceMute,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=25+120&&x<25+120+18&&y>=290+29&&y<290+29+17 )
				{
					SettingsData.bAutoSave=!SettingsData.bAutoSave;
					drawCheckbox ( 25+120,290+29,SettingsData.bAutoSave,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=25+120&&x<25+120+18&&y>=193+29&&y<193+29+17 )
				{
					SettingsData.bAnimations=!SettingsData.bAnimations;
					drawCheckbox ( 25+120,193+29,SettingsData.bAnimations,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=25+120&&x<25+120+18&&y>=213+29&&y<213+29+17 )
				{
					SettingsData.bShadows=!SettingsData.bShadows;
					drawCheckbox ( 25+120,213+29,SettingsData.bShadows,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=25+120&&x<25+120+18&&y>=233+29&&y<233+29+17 )
				{
					SettingsData.bAlphaEffects=!SettingsData.bAlphaEffects;
					drawCheckbox ( 25+120,233+29,SettingsData.bAlphaEffects,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=116+120&&x<116+120+184&&y>=154+29&&y<154+29+17&&!Input )
				{
					Input=true;
					InputStr=game->ActivePlayer->name;
					stmp = InputStr; stmp += "_";
					fonts->OutText ( ( char * ) stmp.c_str(),122+120,158+29,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=210+120&&x<210+120+18&&y>=193+29&&y<193+29+17 )
				{
					SettingsData.bDamageEffects=!SettingsData.bDamageEffects;
					drawCheckbox ( 210+120,193+29,SettingsData.bDamageEffects,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=210+120&&x<210+120+18&&y>=213+29&&y<213+29+17 )
				{
					SettingsData.bDamageEffectsVehicles=!SettingsData.bDamageEffectsVehicles;
					drawCheckbox ( 210+120,213+29,SettingsData.bDamageEffectsVehicles,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=210+120&&x<210+120+18&&y>=233+29&&y<233+29+17 )
				{
					SettingsData.bMakeTracks=!SettingsData.bMakeTracks;
					drawCheckbox ( 210+120,233+29,SettingsData.bMakeTracks,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
			}
		}
		// Fertig-Button:
		if ( x>=208+120&&x<208+120+77&&y>=383+29&&y<382+29+24 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton(lngPack.Translate( "Text~Menu_Main~Button_Done" ), true, 208+120, 383+29, buffer); 
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				if ( Input )
				{
					game->ActivePlayer->name=InputStr;
				}
				if ( strcmp ( game->ActivePlayer->name.c_str(),OldName.c_str() ) !=0 )
				{
					game->engine->ChangePlayerName ( game->ActivePlayer->name );
				}
				return;
			}
		}
		else if ( FertigPressed )
		{
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Done" ), false, 208+120, 383+29, buffer); 
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
			
		}
		// Abbruch-Button:
		if ( x>=118+120&&x<118+120+77&&y>=383+29&&y<382+29+24 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton(lngPack.Translate( "Text~Menu_Main~Button_Cancel" ), true, 118+120, 383+29, buffer); 
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				SettingsData.MusicMute = OldMusicMute;
				SettingsData.SoundMute = OldSoundMute;
				SettingsData.VoiceMute = OldVoiceMute;
				SettingsData.bAutoSave = OldbAutoSave;
				SettingsData.bAnimations = OldbAnimations;
				SettingsData.bShadows = OldbShadows;
				SettingsData.bAlphaEffects = OldbAlphaEffects;
				SettingsData.iScrollSpeed = OldiScrollSpeed;
				SettingsData.MusicVol = OldMusicVol;
				SettingsData.SoundVol = OldSoundVol;
				SettingsData.VoiceVol = OldVoiceVol;
				SettingsData.bDamageEffects = OldbDamageEffects;
				SettingsData.bDamageEffectsVehicles = OldbDamageEffectsVehicles;
				SettingsData.bMakeTracks = OldbMakeTracks;
				game->ActivePlayer->name = OldName;
				SetMusicVol ( SettingsData.MusicVol );
				return;
			}
		}
		else if ( AbbruchPressed )
		{
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Cancel" ), false, 118+120, 383+29, buffer); 
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}


		LastMouseX=x;LastMouseY=y;
		LastB=b;
		SDL_Delay ( 1 );
	}
	SDL_FreeSurface (SfDialog);
}

bool showSelfdestruction()
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b;
	SDL_Rect scr,dest;
	bool AbbruchPressed=false;
	bool ScharfPressed=false;
	bool DestroyPressed=true;
	bool Scharf=false;
	int GlasHeight=56;

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	game->DrawMap();
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	if ( SettingsData.bAlphaEffects ) SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,buffer,NULL );
	dest.x=233;scr.x=0;
	dest.y=199;scr.y=0;
	scr.w=dest.w=GraphicsData.gfx_destruction->w;
	scr.h=dest.h=GraphicsData.gfx_destruction->h/2;
	SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );



	dest.w=59;
	dest.h=56;
	dest.x=233+15;
	dest.y=199+13;
	SDL_BlitSurface ( GraphicsData.gfx_destruction_glas,NULL,buffer,&dest );
	
	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Hot" ), false, 233+89,199+14,buffer);
	drawButton(lngPack.Translate( "Text~Menu_Main~Button_Cancel" ), false, 233+89,199+46,buffer);


	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );
	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		//FIXME: check whether game is really running
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Abbruch-Button:
		if ( x>=233+89&&x<233+89+71&&y>=199+46&&y<199+46+21 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton ); //pressed
				drawButton(lngPack.Translate( "Text~Menu_Main~Button_Cancel" ), true, 233+89,199+46,buffer);
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				return false;
			}
		}
		else if ( AbbruchPressed )
		{
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Cancel" ), false, 233+89,199+46,buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}
		// Scharf-Button:
		if ( !Scharf&&x>=233+89&&x<233+89+71&&y>=199+14&&y<199+14+21 )
		{
			if ( b&&!ScharfPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				drawButton(lngPack.Translate( "Text~Menu_Main~Button_Hot" ), true, 233+89,199+14,buffer);
				SHOW_SCREEN
				mouse->draw ( false,screen );
				ScharfPressed=true;
			}
			else if ( !b&&LastB )
			{
				Scharf=true;
				PlayFX ( SoundData.SNDArm );
			}
		}
		else if ( !Scharf&&ScharfPressed )
		{
			drawButton(lngPack.Translate( "Text~Menu_Main~Button_Hot" ), false, 233+89,199+14,buffer);
			SHOW_SCREEN
			mouse->draw ( false,screen );
			ScharfPressed=false;
		}
		// Das Schutzglas hochfahren:
		if ( Scharf&&GlasHeight>0&&timer0 )
		{
			scr.x=15;
			scr.y=13;
			scr.w=dest.w=59;
			scr.h=dest.h=56;
			dest.x=233+15;
			dest.y=199+13;
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
			GlasHeight-=10;
			if ( GlasHeight>0 )
			{
				scr.x=0;scr.y=0;
				scr.h=dest.h=GlasHeight;
				SDL_BlitSurface ( GraphicsData.gfx_destruction_glas,&scr,buffer,&dest );
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Zerstören-Button:
		if ( GlasHeight<=0&&x>=233+15&&x<233+15+59&&y>=199+13&&y<199+13+56 )
		{
			if ( b&&!DestroyPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=15;
				scr.y=95;
				dest.w=scr.w=59;
				dest.h=scr.h=56;
				dest.x=233+15;
				dest.y=199+13;
				SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				DestroyPressed=true;
			}
			else if ( !b&&LastB )
			{
				return true; //user told us to blow something up
			}
		}
		else if ( GlasHeight<=0&&DestroyPressed )
		{
			scr.x=15;
			scr.y=13;
			dest.w=scr.w=59;
			dest.h=scr.h=56;
			dest.x=233+15;
			dest.y=199+13;
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DestroyPressed=false;
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
}

 //FIXME: offset method only works on fixed resolution 640x460. 
void drawSlider ( int offx,int offy,int value, SDL_Surface *surface )
{
	SDL_Rect scr, dest;
	#define SLIDER_W 14
	#define SLIDER_H 17
	
	//BEGIN REDRAW DIALOG UNDER SLIDER
	/*Offset to read clean background from +/- 7 to 
	*overdraw slider because slider is 14 fat and can
	*show half over the ends of the sliderbar*/
	scr.x = offx - 120 - SLIDER_W / 2; //scr.x & scr.y = topleft
	scr.y = offy - 29 ;
	scr.w = 57 + SLIDER_W;
	scr.h = SLIDER_H;
	dest.x = offx - 6;
	dest.y = offy - 7;	
	dest.w = scr.w + SLIDER_W;
	dest.h = SLIDER_H;
	SDL_BlitSurface ( surface,&scr,buffer,&dest ); //dist
	//END REDRAW DIALOG UNDER SLIDER
	
	//BEGIN DRAW SLIDERBAR
	scr.x=334; //get sliderbar from hud_stuff.pxc
	scr.y=82;
	scr.w=58;
	scr.h=3;	
	dest.y += 7;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	//END DRAW SLIDERBAR

	//BEGIN DRAW SLIDER
	scr.x=412; //get slider from hud_stuff.pcx
	scr.y=46;
	scr.w=SLIDER_W;
	scr.h=SLIDER_H;
	
	dest.w=scr.w;
	dest.h=scr.h;
	dest.x=offx-6+ ( int ) ( ( 57/255.0 ) *value );
	dest.y=offy-SLIDER_W / 2;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	//END DRAW SLIDER
}

void drawCheckbox ( int offx,int offy,bool set, SDL_Surface *surface )
{
	SDL_Rect scr,dest;
	scr.x=393;
	if ( !set )
	{
		scr.y=46; //button pressed
	}
	else
	{
		scr.y=64; //button unpressed
	}
	dest.w=scr.w=18; //get button from hud_gfx
	dest.h=scr.h=17;
	dest.x=offx;
	dest.y=offy;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,&dest );
}

void drawButton (string sText, bool bPressed, int x, int y, SDL_Surface *surface)
{
	SDL_Rect scr, dest;
	int iPx; //for moving fonts 1 pixel down on click
	if(bPressed)
	{
		scr.x=230; //clicked button
		iPx = 6;
	}
	else
	{
		scr.x=308; //unclicked button
		iPx = 5;
	}
	scr.y=455; //get button from gfx_hud.pcx
	dest.w=scr.w=77;
	dest.h=scr.h=23;
	dest.x = x;
	dest.y = y;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,&dest ); //show button on string
	fonts->OutTextCenter(sText.c_str(),dest.x+dest.w/2,dest.y+iPx,buffer); //show text centered on button
}
