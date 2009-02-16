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
#include <sstream>
#include "buttons.h"
#include "dialog.h"
#include "mouse.h"
#include "unifonts.h"
#include "sound.h"
#include "menu.h"
#include "pcx.h"
#include "files.h"
#include "log.h"
#include "loaddata.h"
#include "events.h"
#include "client.h"
#include "input.h"

// shows a yes/no dialog
bool ShowYesNo ( string text, bool bPurgeHud )
{
	#define DIALOGBOX_W 300
	#define DIALOGBOX_H 231
	#define BUTTON_W 150
	#define BUTTON_H 29
	int b, x, y, lx = 0, ly = 0, lb = 0;
	bool ret = false;
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOGBOX_W / 2, SettingsData.iScreenH / 2 - DIALOGBOX_H / 2, DIALOGBOX_W, DIALOGBOX_H };
	SDL_Rect rText = {rDialog.x+20, rDialog.y+20,rDialog.w-40, rDialog.h-150};

	mouse->SetCursor ( CHand );
	if( Client )
	{
		Client->drawMap ( false );
	}
	if ( bPurgeHud ) SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );

	if ( SettingsData.bAlphaEffects )
	{
		SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
	}

	LoadPCXtoSF(GraphicsData.Dialog2Path.c_str(), GraphicsData.gfx_dialog);

	SmallButtonHUD btn_yes(rDialog.x + 80, rDialog.y + 150, "Text~Button~Yes");
	SmallButtonHUD btn_no( rDialog.x + 80, rDialog.y + 185, "Text~Button~No");

	SDL_BlitSurface ( GraphicsData.gfx_dialog, NULL, buffer, &rDialog );
	btn_yes.Draw();
	btn_no.Draw();
	font->showTextAsBlock(rText, text);
	SHOW_SCREEN
	mouse->draw ( false, screen );

	while ( 1 )
	{
		if ( Client )
		{
			Client->handleTimer();
			Client->doGameActions();
		}

		// Eingaben holen:
		EventHandler->HandleEvents();

		// Die Maus:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		x = mouse->x;

		y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		bool const down = b > lb;
		bool const up   = b < lb;
		if (btn_yes.CheckClick(x, y, down, up))
		{
			ret = true;
			break;
		}
		if (btn_no.CheckClick(x, y, down, up))
		{
			ret = false;
			break;
		}

		lx = x;

		ly = y;
		lb = b;
		SDL_Delay ( 1 );
	}

	LoadPCXtoSF(GraphicsData.DialogPath.c_str(), GraphicsData.gfx_dialog);
	if( Client )
	{
		Client->bFlagDrawMap = true;
	}
	return ret;
}

// Zeigt einen Dialog für eine Zahleneingabe an:
int ShowNumberInput ( string text, int iMaxValue, int iDefaultValue )
{
	#define DIALOGBOX_W 300
	#define DIALOGBOX_H 231
	int b, x, y, lx = 0, ly = 0, lb = 0;
	int value = iDefaultValue;
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOGBOX_W / 2, SettingsData.iScreenH / 2 - DIALOGBOX_H / 2, DIALOGBOX_W, DIALOGBOX_H };
	SDL_Rect rTextBox = {rDialog.x + 30, rDialog.y + 30, 238, 114 };
	SDL_Rect rTextField = {rDialog.x + 246, rDialog.y + 171, 30, 10 };
	SDL_Rect rArrowUp = { rDialog.x + 241, rDialog.y + 187, 18, 17};
	SDL_Rect rArrowDown = { rDialog.x + 261, rDialog.y + 187, 18, 17};
	SDL_Rect scr = {rTextField.x - rDialog.x, rTextField.y - rDialog.y, rTextField.w, rTextField.h}; //remove offset
	string stmp = iToStr(iDefaultValue);
	SDL_Surface *SfDialog = NULL;

	if(iMaxValue < 0 || iDefaultValue < 0)
	{
		SDL_FreeSurface(SfDialog);
		Log.write("Can't ask for negative numbers in number dialog", cLog::eLOG_TYPE_WARNING); //dev fucked up
		return -1;
	}
	if(iDefaultValue > iMaxValue)
	{
		iDefaultValue = iMaxValue;
		Log.write("Got default value bigger than maximum value", cLog::eLOG_TYPE_WARNING); //dev fucked up
	}

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, DIALOGBOX_W, DIALOGBOX_H, SettingsData.iColourDepth, 0, 0, 0, 0 );
	if (FileExists(GFXOD_DIALOG4))
	{
		LoadPCXtoSF ( GFXOD_DIALOG6, SfDialog ); //load dialog6.pxc
	}
	SDL_BlitSurface ( SfDialog, NULL, buffer, &rDialog );

	SmallButtonHUD btn_ok(rDialog.x + 80, rDialog.y + 185, "Text~Button~OK");

	btn_ok.Draw();
	font->showTextAsBlock(rTextBox, text);
	font->showText(rTextField, stmp);

	SHOW_SCREEN
	mouse->draw ( false, screen );

	while ( 1 )
	{
		// Eingaben holen:
		EventHandler->HandleEvents();
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
		if (btn_ok.CheckClick(x, y, b > lb, b < lb))
		{
			break;
		}
		//arrow up "increase" numbers
		if ( x >= rArrowUp.x && x <= rArrowUp.x + rArrowUp.w && y >= rArrowUp.y && y <= rArrowUp.y + rArrowUp.h )
		{
			if ( b && !lb )
			{
				SDL_BlitSurface ( SfDialog, &scr, buffer, &rTextField ); //redraw textfield
				value = atoi ( stmp.c_str() );
				value++;
				if( value > iMaxValue )
				{
					Log.write("Numberlimit exceeded", cLog::eLOG_TYPE_WARNING);
					value --;
				}
				else
				{
					PlayFX ( SoundData.SNDHudButton );
				}

				stmp = iToStr(value);
				font->showText(rTextField, stmp);
				SHOW_SCREEN
			}
		}
		//arrow diwn "decrease" numbers
		if ( x >= rArrowDown.x && x <= rArrowDown.x + rArrowDown.w && y >= rArrowDown.y && y <= rArrowDown.y + rArrowDown.h )
		{
			if ( b && !lb )
			{
				SDL_BlitSurface ( SfDialog, &scr, buffer, &rTextField ); //redraw textfield
				value = atoi ( stmp.c_str() );
				if (value > 1)
				{
					value --;
					PlayFX ( SoundData.SNDHudButton );
				}
				else
				{
					Log.write("Negative numbers not allowed", cLog::eLOG_TYPE_WARNING);
				}
				stmp = iToStr(value);
				font->showText(rTextField, stmp);

				SHOW_SCREEN
			}
		}

		lx = x;
		ly = y;
		lb = b;
		SDL_Delay ( 1 );
	}


	SDL_FreeSurface(SfDialog);
	return value;
}

void ShowOK ( string sText, bool bPurgeHud )
{
	int b, x, y, lx = 0, ly = 0, lb = 0;
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DIALOGBOX_W / 2, SettingsData.iScreenH / 2 - DIALOGBOX_H / 2, DIALOGBOX_W, DIALOGBOX_H };
	SDL_Rect rText = {rDialog.x+20, rDialog.y+20,rDialog.w-40, rDialog.h-150};
	Uint8 *keystate;
	bool bLastKeystate = false;
	SDL_Surface *SfDialog = NULL;
	SDL_Surface *SfBackground = NULL;

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, DIALOGBOX_W, DIALOGBOX_H, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SfBackground = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, DIALOGBOX_W, DIALOGBOX_H, SettingsData.iColourDepth, 0, 0, 0, 0 );

	if (FileExists(GFXOD_DIALOG4))
	{
		LoadPCXtoSF ( GFXOD_DIALOG2, SfDialog ); //load dialog2.pxc
	}

	SDL_BlitSurface( buffer, &rDialog, SfBackground, NULL); //store background

	mouse->SetCursor ( CHand );

	if ( !bPurgeHud )
	{
		SDL_Rect dest;

		dest.x = 180;
		dest.y = 18;
		dest.w = dest.h = 448;
		SDL_FillRect ( buffer, &dest, 0 );
		SDL_BlitSurface ( GraphicsData.gfx_hud, NULL, buffer, NULL );
		dest.x = 15;
		dest.y = 354;
		dest.w = 112;
		dest.h = 113;
		SDL_FillRect ( buffer, &dest, 0 );

		if ( SettingsData.bAlphaEffects )
		{
			SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
		}
	}

	SmallButtonHUD btn_ok(rDialog.x + 80, rDialog.y + 185, "Text~Button~OK");

	SDL_BlitSurface ( SfDialog, NULL, buffer, &rDialog );
	font->showTextAsBlock(rText, sText);
	btn_ok.Draw();
	SHOW_SCREEN
	mouse->draw ( false, screen );

	SDL_Delay ( 200 );
	while ( 1 )
	{
		if ( !bPurgeHud && Client )
		{
			Client->handleTimer();
			Client->doGameActions();
		}

		// Eingaben holen:
		EventHandler->HandleEvents();
		keystate = SDL_GetKeyState( NULL );

		// Die Maus:
		mouse->GetPos();

		b = mouse->GetMouseButton();

		x = mouse->x;

		y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		if ((bLastKeystate && !keystate[SDLK_RETURN]) ||
				btn_ok.CheckClick(x, y, b > lb, b < lb))
		{
			break;
		}

		lx = x;
		ly = y;
		lb = b;
		SDL_Delay ( 1 );
		if ( keystate[SDLK_RETURN] ) bLastKeystate = true;
		else bLastKeystate = false;
	}

	SHOW_SCREEN

	if ( !bPurgeHud )
	{
		Client->bFlagDrawMap = true;
	}
	else
	{
		SDL_BlitSurface( SfBackground, NULL, buffer, &rDialog); //restore background
		SHOW_SCREEN
	}

	SDL_FreeSurface(SfDialog);
	SDL_FreeSurface(SfBackground);
}

/** shows licence infobox on screen (don't call this within game since I this doesn't care about ongoing engine
*@author beko
*/
void showLicence ()
{
	int b, x, y, lx = 0, ly = 0, lb = 0, index=0;

	SDL_Rect rDialogOnScreen; //dialog blitted on the screen
	SDL_Rect rDialog; //our dialog
	SDL_Rect rDialogBoxBlack; //spot to draw text inside
	SDL_Rect rDialogBoxBlackOffset = {0,0,0,0}; //for redrawing only the black part PLUS buttons
	SDL_Rect rArrowUp;
	SDL_Rect rArrowDown;
	SDL_Surface *SfDialog;
	//BEGIN CREATING LICENCE TEXTS
	string sLicenceIntro1;
	string sLicenceIntro2;
	string sLicence4Intro1;
	string sLicence1;
	string sLicence2;
	string sLicence3;
 	string sLicence4;
 	sLicenceIntro1 = "\"M.A.X. Reloaded\"";
	sLicenceIntro2 = "(C) 2007 by its authors";
	sLicence4Intro1 = "AUTHORS:";

	sLicence1 = "\
This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version.";
sLicence2 = "\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.";
sLicence3="\
You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software \
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";
	//BEGIN CREATING AUTHORS TEXT
	char line[72];
	stringstream ssLicence4;

	//open AUTHOR
	string sAuthors = SettingsData.sDataDir + PATH_DELIMITER +
#ifdef WIN32
		"AUTHORS.txt";
#elif __amigaos4
		"AUTHORS.txt";
#else
		"AUTHORS";
#endif
	FILE* const fp = fopen(sAuthors.c_str(), "r");
	if ( fp != NULL  )
	{	//read authors from file
		while(fgets(line, 72, fp)) //snip entrys longer 72
		{
			ssLicence4 << line;
		}
		fclose(fp);
	}
	else
	{
		ssLicence4 << "Couldn't read AUTHORS"; //missing file - naughty
	}

	sLicence4 = ssLicence4.str();
	//END CREATING AUTHORS TEXT
	//END CREATING LICENCE TEXTS

	mouse->SetCursor ( CHand );
	SDL_BlitSurface ( screen, NULL, buffer, NULL ); //write screen to buffer for proper background "picture"

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 300, 231, SettingsData.iColourDepth, 0, 0, 0, 0 );
	if (FileExists(GFXOD_DIALOG4))
	{
		LoadPCXtoSF ( GFXOD_DIALOG4, SfDialog ); //load dialog4.pxc
	}

	//set some rects
	rDialog.x = screen->w / 2 - SfDialog->w / 2;
	rDialog.y = screen->h / 2 - SfDialog->h / 2;

	rDialogOnScreen.x = rDialog.x + 35;
	rDialogBoxBlack.x=32;
	rDialogOnScreen.w = 232;
	rDialogBoxBlack.w=SfDialog->w-32;
	rDialogOnScreen.y = rDialog.y + 30 + 3* font->getFontHeight();
	rDialogBoxBlack.y=28;
	rDialogOnScreen.h = 142;
	rDialogBoxBlack.h=SfDialog->h-28;

	rDialogBoxBlackOffset.x = screen->w / 2 - SfDialog->w / 2 + 32;
	rDialogBoxBlackOffset.y = screen->h / 2 - SfDialog->h / 2 + 28; //w, h not needed since SDL_BlitSurface ignores these for destination rect

	SmallButtonHUD btn_ok(rDialog.x + 80, rDialog.y + 185, "Text~Button~OK");

	//create start dialog
	SDL_BlitSurface ( SfDialog, NULL, buffer, &rDialog );
	btn_ok.Draw();
	font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30, sLicenceIntro1);
	font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30 + font->getFontHeight(), sLicenceIntro2);
	font->showTextAsBlock(rDialogOnScreen, sLicence1);

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
		EventHandler->HandleEvents(); //get hid-input
		mouse->GetPos(); //get mouseposition
		b = mouse->GetMouseButton();
		x = mouse->x; y = mouse->y;

		if ( lx != x || ly != y )
		{
			mouse->draw ( true, screen );
		}

		// OK Button:
		if (btn_ok.CheckClick(x, y, b > lb, b < lb))
		{
			break;
		}
		//button up (left)
		else if ( x >= rDialog.x + 241 && x < rDialog.x + 241 + 18 && y >= rDialog.y + 187 && y < rDialog.y + 187 + 17 )
		{
			if ( b && !lb )
			{
				SDL_BlitSurface ( SfDialog, &rDialogBoxBlack, buffer, &rDialogBoxBlackOffset );  //redraw empty textbox
				font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30, sLicenceIntro1);
				font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30 + font->getFontHeight(), sLicenceIntro2);
				btn_ok.Draw();

				switch(index)
				{
					case 1 :
						index = 0;
						PlayFX ( SoundData.SNDHudButton );
						drawDialogArrow(buffer, &rArrowUp, ARROW_TYPE_UP); //first entry in list needs this to disable arrow
						font->showTextAsBlock(rDialogOnScreen, sLicence1);
						break;
					case 2 :
						index = 1;
						PlayFX ( SoundData.SNDHudButton );
						font->showTextAsBlock(rDialogOnScreen, sLicence2);
						break;
					case 3: index = 2;
						PlayFX ( SoundData.SNDHudButton );
						font->showTextAsBlock(rDialogOnScreen, sLicence3);
						break;
					default: //should not happen
						Log.write("Invalid index - can't show text in dialog",cLog::eLOG_TYPE_WARNING);
						drawDialogArrow(buffer, &rArrowUp, ARROW_TYPE_UP);
						font->showTextAsBlock(rDialogOnScreen, sLicence1);
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
				font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30, sLicenceIntro1);
				btn_ok.Draw();

				switch(index)
				{
					case 0 :
						index = 1;
						PlayFX ( SoundData.SNDHudButton );
						font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30 + font->getFontHeight(), sLicenceIntro2);
						font->showTextAsBlock(rDialogOnScreen, sLicence2);
						break;
					case 1 :
						index = 2;
						PlayFX ( SoundData.SNDHudButton );
						font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30 + font->getFontHeight(), sLicenceIntro2);
						font->showTextAsBlock(rDialogOnScreen, sLicence3);
						break;
					case 2:
						index = 3;
						PlayFX ( SoundData.SNDHudButton );
						drawDialogArrow(buffer, &rArrowDown, ARROW_TYPE_DOWN); //last entry in list needs this to disable arrow
						font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30 + font->getFontHeight(), sLicence4Intro1);
						font->showText(rDialogOnScreen, sLicence4, FONT_LATIN_SMALL_WHITE);
						break;
					default: //should not happen
						Log.write("Invalid index - can't show text in dialog",cLog::eLOG_TYPE_WARNING);
						drawDialogArrow(buffer, &rArrowDown, ARROW_TYPE_DOWN);
						font->showTextCentered(rDialog.x + SfDialog->w / 2, rDialog.y + 30 + font->getFontHeight(), sLicence4Intro1);
						font->showText(rDialogOnScreen, sLicence4, FONT_LATIN_SMALL_WHITE);
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
			Log.write("Invalid arrow type - can't serve here", cLog::eLOG_TYPE_WARNING);
			break;
	}

	if(surface)
	{
		//get gfx arrow from menu graphic and blit it to tmp
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,dest );
	}
	else
	{
		Log.write("Invalid surface - can't apply gfx arrow", cLog::eLOG_TYPE_WARNING);
	}
}

// showPreferences ////////////////////////////////////////////////////////////
// Zeigt das Präferenzenfenster an:
void showPreferences ( void )
{
	bool OldMusicMute, OldSoundMute, OldVoiceMute, OldbAutoSave, OldbAnimations, OldbShadows, OldbAlphaEffects, OldbDamageEffects, OldbDamageEffectsVehicles, OldbMakeTracks, bOldIntro, bOldWindowMode;
	int OldiScrollSpeed, OldMusicVol, OldSoundVol, OldVoiceVol;
	int LastMouseX = 0, LastMouseY = 0, LastB = 0, x, y, b;
	string OldName;
	string sTmpName;
	string sTmp;
	SDL_Rect scr, dest, rFont;
	bool cursor = true;
	SDL_Surface *SfDialog;

	//position x of all sliderbars
	#define BAR_X 140 + rDialog.x
	#define BAR_Y 81 + rDialog.y
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
	OldName = sTmpName =SettingsData.sPlayerName;
	OldbDamageEffects = SettingsData.bDamageEffects;
	OldbDamageEffectsVehicles = SettingsData.bDamageEffectsVehicles;
	OldbMakeTracks = SettingsData.bMakeTracks;
	bOldIntro = SettingsData.bIntro;
	bOldWindowMode = SettingsData.bWindowMode;

	SfDialog = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 400, 422, SettingsData.iColourDepth, 0, 0, 0, 0 );

	SDL_Rect rDialog = { screen->w / 2 - SfDialog->w / 2, screen->h / 2 - SfDialog->h / 2, SfDialog->w, SfDialog->h };
	SDL_Rect rSldMusic = { BAR_X,BAR_Y, 57, 10 };
	SDL_Rect rSldEffect = { BAR_X,BAR_Y+CELLSPACE, 57, 10 };
	SDL_Rect rSldVoice = { BAR_X,BAR_Y+CELLSPACE*2, 57, 10 };
	SDL_Rect rSldSpeed = { BAR_X,261+rDialog.y, 57, 10 };

	if ( SettingsData.bAlphaEffects )
	{
		SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, buffer, NULL );
	}

	if(FileExists(GFXOD_DIALOG5))
	{
		LoadPCXtoSF (GFXOD_DIALOG5, SfDialog );
		//blit black titlebar behind textfield for playername
		scr.x=108;
		scr.y=12;
		scr.w=186;
		scr.h=18;
		dest.x=108;
		dest.y=154;
		SDL_BlitSurface ( SfDialog,&scr,SfDialog,&dest );

	}



	 //blit dialog to buffer
	dest.x = rDialog.x;
	dest.y = rDialog.y;

	NormalButton btn_cancel(rDialog.x + 118, rDialog.y + 383, "Text~Button~Cancel");
	NormalButton btn_done(  rDialog.x + 208, rDialog.y + 383, "Text~Button~Done");

	SDL_BlitSurface ( SfDialog, NULL, buffer, &dest );
	btn_cancel.Draw();
	btn_done.Draw();

	rFont.x = rDialog.x + rDialog.w/2;
	rFont.y = rDialog.y + 15;
	font->showTextCentered(rFont.x, rFont.y, lngPack.i18n( "Text~Settings~Preferences" ));


	//BEGIN BLOCK SOUND
	//Headline
	sTmp = lngPack.i18n ( "Text~Settings~Volume" ) + ":";
	rFont.x = rDialog.x + 25;
	rFont.y = rDialog.y + 56;
	rFont.w = font->getTextWide(sTmp);
	font->showText(rFont, sTmp);

	//Music
	rFont.x = rDialog.x + 25;
	rFont.w = 100;
	rFont.y = rDialog.y + 76;
	rFont.h = CELLSPACE_FONT;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Music" ));
	drawSlider (SfDialog, rSldMusic.x,rSldMusic.y,SettingsData.MusicVol*2,buffer );
	drawCheckbox ( 210+rDialog.x,73+rDialog.y,SettingsData.MusicMute, buffer);
	rFont.x = rDialog.x + 235; 	rFont.w = 140;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Disable" ));

	//Effectsound
	rFont.x = rDialog.x + 25;
	rFont.w = 100;
	rFont.y += CELLSPACE_FONT;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Effects" ));

	drawSlider (SfDialog, rSldEffect.x,rSldEffect.y,SettingsData.SoundVol*2, buffer );
	drawCheckbox ( 210+rDialog.x,93+rDialog.y,SettingsData.SoundMute,buffer );
	rFont.x = rDialog.x + 235;
	rFont.w = 140;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Disable" ));


	//Voices
	rFont.x = rDialog.x + 25; 	rFont.w = 100;
	rFont.y += CELLSPACE_FONT;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Voices" ));
	drawSlider (SfDialog, rSldVoice.x,rSldVoice.y,SettingsData.VoiceVol*2,buffer );
	drawCheckbox ( 210+rDialog.x,113+rDialog.y,SettingsData.VoiceMute,buffer );
	rFont.x =  rDialog.x + 235; 	rFont.w = 140;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Disable" ));
	//END BLOCK SOUND
	//BEGIN BLOCK PLAYERNAME


	rFont.x = rDialog.x + 25; 	rFont.w = 100;
	rFont.y = 158+rDialog.y;
	font->showText(rFont, lngPack.i18n( "Text~Title~Player_Name" ));
	font->showText(122+rDialog.x,158+rDialog.y, SettingsData.sPlayerName);
	InputHandler->setInputStr ( sTmpName );

	//END BLOCK PLAYERNAME

	rFont.x = rDialog.x + 50; rFont.w = 100;
	rFont.y = rDialog.y + 197;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Animation" ));

	drawCheckbox ( 25+rDialog.x,193+rDialog.y,SettingsData.bAnimations,buffer );

	rFont.x = rDialog.x + 50; rFont.w = 100;
	rFont.y = rDialog.y + 217;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Shadows" ));
	drawCheckbox ( 25+rDialog.x,213+rDialog.y,SettingsData.bShadows,buffer );

	rFont.x = rDialog.x + 50; rFont.w = 100;
	rFont.y = rDialog.y + 237;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Alphaeffects" ));
	drawCheckbox ( 25+rDialog.x,233+rDialog.y,SettingsData.bAlphaEffects,buffer );

	rFont.x = rDialog.x + 25+210; rFont.w = 100;
	rFont.y = rDialog.y + 197;
	font->showText(rFont, lngPack.i18n( "Text~Settings~ShowDamage" ));
	drawCheckbox ( 210+rDialog.x,193+rDialog.y,SettingsData.bDamageEffects,buffer );

	rFont.x = rDialog.x + 25+210; rFont.w = 100;
	rFont.y = rDialog.y + 217;
	font->showText(rFont, lngPack.i18n( "Text~Settings~ShowDamageVehicle" ));
	drawCheckbox ( 210+rDialog.x,213+rDialog.y,SettingsData.bDamageEffectsVehicles,buffer );

	rFont.x = rDialog.x + 25+210; rFont.w = 100;
	rFont.y = rDialog.y + 237;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Tracks" ));
	drawCheckbox ( 210+rDialog.x,233+rDialog.y,SettingsData.bMakeTracks,buffer );

	rFont.x = rDialog.x + 25; 	rFont.w = 100;
	rFont.y = rDialog.y+232+25;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Scrollspeed" ));
	drawSlider (SfDialog, rSldSpeed.x,rSldSpeed.y,SettingsData.iScrollSpeed*5, buffer );

	rFont.x = rDialog.x + 50; rFont.w = 100;
	rFont.y = rDialog.y + 294;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Autosave" ));
	drawCheckbox ( 25+rDialog.x,290+rDialog.y,SettingsData.bAutoSave,buffer );

	rFont.x = rDialog.x + 50; rFont.w = 100;
	rFont.y = rDialog.y + 314;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Intro" ));
	drawCheckbox ( 25+rDialog.x,310+rDialog.y,SettingsData.bIntro,buffer );

	rFont.x = rDialog.x + 50; rFont.w = 100;
	rFont.y = rDialog.y + 334;
	font->showText(rFont, lngPack.i18n( "Text~Settings~Window" ));
	drawCheckbox ( 25+rDialog.x,330+rDialog.y,SettingsData.bWindowMode,buffer );
		
	//BEGIN DRAWING RESOLUTION MODES

	//if(strcmp("640,480",SettingsData.
	int iResMode;
	int iNewResMode;

	if(SettingsData.iScreenW == 640 && SettingsData.iScreenH == 480) iResMode = 0;
	else if(SettingsData.iScreenW == 800 && SettingsData.iScreenH == 600) iResMode = 1;
	else if(SettingsData.iScreenW == 1024 && SettingsData.iScreenH == 768) iResMode = 2;
	else if(SettingsData.iScreenW == 1024 && SettingsData.iScreenH == 960) iResMode = 3;
	else if(SettingsData.iScreenW == 1280 && SettingsData.iScreenH == 960) iResMode = 4;
	else if(SettingsData.iScreenW == 1280 && SettingsData.iScreenH == 1024) iResMode = 5;
	else iResMode = -1; //unsupported screen mode - possible but may cause graphical glitches
	iNewResMode = iResMode;
	bool bMyMode = false;

	const char* sModes[6];

	sModes[0]="640x480";
	sModes[1]="800x600";
	sModes[2]="1024x768";
	sModes[3]="1024x960";
	sModes[4]="1280x960";
	sModes[5]="1280x1024";

	for (int i = 0; i<3; i++)
	{
		if(i==iResMode) bMyMode = true;
		else bMyMode = false;
		rFont.x = rDialog.x + 185; rFont.w = 100;
		rFont.y = rDialog.y + 294+20*i;
		font->showText(rFont, (char*)sModes[i]);
		drawCheckbox ( 160+rDialog.x,290+20*i+rDialog.y,bMyMode,buffer );
	}
	for (int i = 4; i<7; i++)
	{
		if(i-1==iResMode) bMyMode = true;
		else bMyMode = false;
		rFont.x = rDialog.x + 285; rFont.w = 100;
		rFont.y = rDialog.y + 294+20*(i-4);
		font->showText(rFont, (char*)sModes[i-1]);
		drawCheckbox ( 260+rDialog.x,290+20*(i-4)+rDialog.y,bMyMode,buffer );
	}

	if(iResMode == -1)
	{
		rFont.x = rDialog.x + 100; rFont.w = 200;
		rFont.y = rDialog.y + 294+20*3;
		font->showText(rFont, lngPack.i18n( "Text~Comp~ResolutionWarning" ));
	}
	//END DRAWING RESOLUTION MODES

	SHOW_SCREEN

	mouse->GetBack ( buffer );

	int timer2 = 0;
	while ( 1 )
	{
		// Die Engine laufen lassen:
		if(Client)
		{
			Client->handleTimer();
			timer2 = Client->iTimer2;
			Client->doGameActions();
		}
		// Events holen:
		EventHandler->HandleEvents();

		if ( InputHandler->checkHasBeenInput() )
		{
			scr.x = 116;
			scr.y = 154;
			scr.w = 184;
			scr.h = 17;
			dest.x = 116+rDialog.x;
			dest.y = 154+rDialog.y;
			SDL_BlitSurface ( SfDialog, &scr, buffer, &dest );
			sTmpName = InputHandler->getInputStr();
			font->showText( 122+rDialog.x, 158+rDialog.y, InputHandler->getInputStr() );
			SHOW_SCREEN
			mouse->draw ( true,screen );
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
			if ( x >= rSldMusic.x && x < rSldMusic.x + rSldMusic.w && y >= rSldMusic.y - 7 && y <= rSldMusic.y + rSldMusic.h && ( x != LastMouseX || y != LastMouseY || !LastB ) )
			{
				SettingsData.MusicVol= ( x- ( rSldMusic.x ) ) * (int)( 128.0/57 );
				if ( SettingsData.MusicVol>=125 ) SettingsData.MusicVol=128;
				drawSlider (SfDialog, rSldMusic.x,rSldMusic.y,SettingsData.MusicVol*2, buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SetMusicVol ( SettingsData.MusicVol );
				InputHandler->setInputState ( false );
			}
			else if ( x >= rSldEffect.x && x < rSldEffect.x + rSldEffect.w && y >= rSldEffect.y - 7 && y <= rSldEffect.y + rSldEffect.h && ( x != LastMouseX || y != LastMouseY || !LastB ) )
			{
				SettingsData.SoundVol= ( x- ( rSldEffect.x ) ) * (int)( 128.0/57 );
				if ( SettingsData.SoundVol>=125 ) SettingsData.SoundVol=128;
				drawSlider (SfDialog, rSldEffect.x,rSldEffect.y,SettingsData.SoundVol*2, buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				InputHandler->setInputState ( false );
			}
			else if ( x >= rSldVoice.x && x < rSldVoice.x + rSldVoice.w && y >= rSldVoice.y - 7 && y <= rSldVoice.y + rSldVoice.h && ( x != LastMouseX || y != LastMouseY || !LastB ) )
			{
				SettingsData.VoiceVol= ( x- ( rSldVoice.x ) ) * (int)( 128.0/57 );
				if ( SettingsData.VoiceVol>=125 ) SettingsData.VoiceVol=128;
				drawSlider (SfDialog, rSldVoice.x, rSldVoice.y,SettingsData.VoiceVol*2, buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				InputHandler->setInputState ( false );
			}
			else if ( x >= rSldSpeed.x && x < rSldSpeed.x + rSldSpeed.w && y >= rSldSpeed.y - 7 && y <= rSldSpeed.y + rSldSpeed.h && ( x != LastMouseX || y != LastMouseY || !LastB ) )
			{
				SettingsData.iScrollSpeed= ( int ) ( ( x- ( rSldSpeed.x ) ) * ( 255.0/57 ) ) /5;
				drawSlider (SfDialog, rSldSpeed.x,rSldSpeed.y,SettingsData.iScrollSpeed*5, buffer );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				InputHandler->setInputState ( false );
			}
			if ( !LastB )
			{
				if ( x>=210+rDialog.x&&x<210+rDialog.x+18&&y>=73+rDialog.y&&y<73+rDialog.y+17 )
				{
					SettingsData.MusicMute=!SettingsData.MusicMute;
					drawCheckbox ( 210+rDialog.x,73+rDialog.y,SettingsData.MusicMute,buffer );
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
					InputHandler->setInputState ( false );
				}
				else if ( x>=210+rDialog.x&&x<210+rDialog.x+18&&y>=93+rDialog.y&&y<93+rDialog.y+17 )
				{
					SettingsData.SoundMute=!SettingsData.SoundMute;
					drawCheckbox ( 210+rDialog.x,93+rDialog.y,SettingsData.SoundMute,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=210+rDialog.x&&x<210+rDialog.x+18&&y>=113+rDialog.y&&y<113+rDialog.y+17 )
				{
					SettingsData.VoiceMute=!SettingsData.VoiceMute;
					drawCheckbox ( 210+rDialog.x,113+rDialog.y,SettingsData.VoiceMute,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=25+rDialog.x&&x<25+rDialog.x+18&&y>=290+rDialog.y&&y<290+rDialog.y+17 )
				{
					SettingsData.bAutoSave=!SettingsData.bAutoSave;
					drawCheckbox ( 25+rDialog.x,290+rDialog.y,SettingsData.bAutoSave,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=25+rDialog.x&&x<25+rDialog.x+18&&y>=310+rDialog.y&&y<310+rDialog.y+17 )
				{
					SettingsData.bIntro=!SettingsData.bIntro;
					drawCheckbox ( 25+rDialog.x,310+rDialog.y,SettingsData.bIntro,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=25+rDialog.x&&x<25+rDialog.x+18&&y>=330+rDialog.y&&y<330+rDialog.y+17 )
				{
					SettingsData.bWindowMode=!SettingsData.bWindowMode;
					drawCheckbox ( 25+rDialog.x,330+rDialog.y,SettingsData.bWindowMode,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=25+rDialog.x&&x<25+rDialog.x+18&&y>=193+rDialog.y&&y<193+rDialog.y+17 )
				{
					SettingsData.bAnimations=!SettingsData.bAnimations;
					drawCheckbox ( 25+rDialog.x,193+rDialog.y,SettingsData.bAnimations,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=25+rDialog.x&&x<25+rDialog.x+18&&y>=213+rDialog.y&&y<213+rDialog.y+17 )
				{
					SettingsData.bShadows=!SettingsData.bShadows;
					drawCheckbox ( 25+rDialog.x,213+rDialog.y,SettingsData.bShadows,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=25+rDialog.x&&x<25+rDialog.x+18&&y>=233+rDialog.y&&y<233+rDialog.y+17 )
				{
					SettingsData.bAlphaEffects=!SettingsData.bAlphaEffects;
					drawCheckbox ( 25+rDialog.x,233+rDialog.y,SettingsData.bAlphaEffects,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=116+rDialog.x&&x<116+rDialog.x+184&&y>=154+rDialog.y&&y<154+rDialog.y+17&&!InputHandler->getInputState() )
				{
					InputHandler->setInputState ( true );
					SHOW_SCREEN
					mouse->draw ( false,screen );
				}
				else if ( x>=210+rDialog.x&&x<210+rDialog.x+18&&y>=193+rDialog.y&&y<193+rDialog.y+17 )
				{
					SettingsData.bDamageEffects=!SettingsData.bDamageEffects;
					drawCheckbox ( 210+rDialog.x,193+rDialog.y,SettingsData.bDamageEffects,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=210+rDialog.x&&x<210+rDialog.x+18&&y>=213+rDialog.y&&y<213+rDialog.y+17 )
				{
					SettingsData.bDamageEffectsVehicles=!SettingsData.bDamageEffectsVehicles;
					drawCheckbox ( 210+rDialog.x,213+rDialog.y,SettingsData.bDamageEffectsVehicles,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				}
				else if ( x>=210+rDialog.x&&x<210+rDialog.x+18&&y>=233+rDialog.y&&y<233+rDialog.y+17 )
				{
					SettingsData.bMakeTracks=!SettingsData.bMakeTracks;
					drawCheckbox ( 210+rDialog.x,233+rDialog.y,SettingsData.bMakeTracks,buffer );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					InputHandler->setInputState ( false );
				} //Start of combobox system for resolution modes
				else if(x>=160+rDialog.x && x<=160+rDialog.x+18&&y>=290+20*0+rDialog.y&&y<290+20*0+rDialog.y+17)
				{
					iNewResMode=0;
					InputHandler->setInputState ( false );
				}
				else if(x>=160+rDialog.x && x<=160+rDialog.x+18&&y>=290+20*1+rDialog.y&&y<290+20*1+rDialog.y+17)
				{
					iNewResMode=1;
					InputHandler->setInputState ( false );
				}
				else if(x>=160+rDialog.x && x<=160+rDialog.x+18&&y>=290+20*2+rDialog.y&&y<290+20*2+rDialog.y+17)
				{
					iNewResMode=2;
					InputHandler->setInputState ( false );
				}
				else if(x>=260+rDialog.x && x<=260+rDialog.x+18&&y>=290+20*0+rDialog.y&&y<290+20*0+rDialog.y+17)
				{
					iNewResMode=3;
					InputHandler->setInputState ( false );
				}
				else if(x>=260+rDialog.x && x<=260+rDialog.x+18&&y>=290+20*1+rDialog.y&&y<290+20*1+rDialog.y+17)
				{
					iNewResMode=4;
					InputHandler->setInputState ( false );
				}
				else if(x>=260+rDialog.x && x<=260+rDialog.x+18&&y>=290+20*2+rDialog.y&&y<290+20*2+rDialog.y+17)
				{
					iNewResMode=5;
					InputHandler->setInputState ( false );
				}

				if(iResMode != iNewResMode)
				{
					iResMode = -1;
					for (int i = 0; i<3; i++)
					{
						if(i==iNewResMode) bMyMode = true;
						else bMyMode = false;
						drawCheckbox ( 160+rDialog.x,290+20*i+rDialog.y,bMyMode,buffer );
					}
					for (int i = 4; i<7; i++)
					{
						if(i-1==iNewResMode) bMyMode = true;
						else bMyMode = false;
						drawCheckbox ( 260+rDialog.x,290+20*(i-4)+rDialog.y,bMyMode,buffer );
					}
					SHOW_SCREEN
				}
				 //End of combobox system for resolution modes
			}
		}

		bool const down = b > LastB;
		bool const up   = b < LastB;

		if (btn_done.CheckClick(x, y, down, up))
		{
			SettingsData.sPlayerName = sTmpName;
			if ( Client)
			{
				Client->ActivePlayer->name=sTmpName;
			}
			// Save new settings to max.xml
			SaveOption ( SAVETYPE_MUSICMUTE );
			SaveOption ( SAVETYPE_SOUNDMUTE );
			SaveOption ( SAVETYPE_VOICEMUTE );
			SaveOption ( SAVETYPE_AUTOSAVE );
			SaveOption ( SAVETYPE_ANIMATIONS );
			SaveOption ( SAVETYPE_SHADOWS );
			SaveOption ( SAVETYPE_ALPHA );
			SaveOption ( SAVETYPE_SCROLLSPEED );
			SaveOption ( SAVETYPE_MUSICVOL );
			SaveOption ( SAVETYPE_SOUNDVOL );
			SaveOption ( SAVETYPE_VOICEVOL );
			SaveOption ( SAVETYPE_DAMAGEEFFECTS_BUILDINGS );
			SaveOption ( SAVETYPE_DAMAGEEFFECTS_VEHICLES );
			SaveOption ( SAVETYPE_TRACKS );
			// TODO: remove game
			SaveOption ( SAVETYPE_NAME );
			SaveOption ( SAVETYPE_INTRO );	
			SaveOption ( SAVETYPE_WINDOW );	

			if(iNewResMode == -1)
			{
				InputHandler->setInputState ( false );
				SDL_FreeSurface (SfDialog);
				return; //don't attempt to save resolution. this case only happens if a not supported resolution was set in maxr.xml and no new resolution has been choosen in the preferences dialog
			}
			else
			{
				//store old resolution
				int iOldScreenW = SettingsData.iScreenW;
				int iOldScreenH = SettingsData.iScreenH;
				switch(iNewResMode)
				{
					case(0):
						SettingsData.iScreenW = 640;
						SettingsData.iScreenH = 480; 
						break;
					case(1):
						SettingsData.iScreenW = 800;
						SettingsData.iScreenH = 600; 
						break;
					case(2):
						SettingsData.iScreenW = 1024;
						SettingsData.iScreenH = 768; 
						break;
					case(3):
						SettingsData.iScreenW = 1024;
						SettingsData.iScreenH = 960; 
						break;
					case(4):
						SettingsData.iScreenW = 1280;
						SettingsData.iScreenH = 960; 
						break;
					case(5):
						SettingsData.iScreenW = 1280;
						SettingsData.iScreenH = 1024; 
						break;
					default:
						Log.write("Can't save unknown resolution mode " + iToStr(iNewResMode), cLog::eLOG_TYPE_WARNING);
				}
				SaveOption ( SAVETYPE_RESOLUTION );
				//restore old resolution
				SettingsData.iScreenW = iOldScreenW;
				SettingsData.iScreenH = iOldScreenH;
				if(iResMode != iNewResMode) ShowOK(lngPack.i18n( "Text~Comp~ResolutionChange" ), true);
			}
			InputHandler->setInputState ( false );
			SDL_FreeSurface (SfDialog);
			return;
		}

		if (btn_cancel.CheckClick(x, y, down, up))
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
			SettingsData.bIntro = bOldIntro;
			SettingsData.bWindowMode = bOldWindowMode;
			if(Client) Client->ActivePlayer->name = OldName;
			SetMusicVol ( SettingsData.MusicVol );

			InputHandler->setInputState ( false );
			SDL_FreeSurface (SfDialog);
			return;
		}

		LastMouseX=x;LastMouseY=y;
		LastB=b;
		SDL_Delay ( 1 );
	}
}

bool showSelfdestruction()
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b;
	bool DestroyPressed=true;
	bool Scharf=false;

	#define DLG_W GraphicsData.gfx_destruction->w
	#define DLG_H GraphicsData.gfx_destruction->h/2
	#define BTN_W 71
	#define BTN_H 21
	#define DSTR_W 59
	#define DSTR_H 56

	int GlasHeight=DSTR_H;
	SDL_Rect rDialog = { SettingsData.iScreenW / 2 - DLG_W / 2, SettingsData.iScreenH / 2 - DLG_H / 2, DLG_W, DLG_H };
	SDL_Rect rDialogSrc = { 0,0,DLG_W, DLG_H};
	SDL_Rect rGlass = {rDialog.x+15, rDialog.y+13, DSTR_W, DSTR_H};
	SDL_Rect rDestroySrc = {15,13,DSTR_W,DSTR_H}; //red button
	SDL_Rect rDestroyPressedSrc = {15,95,DSTR_W,DSTR_H}; //red button pressed
	SDL_Rect rDestroyDest = { rDialog.x + 15, rDialog.y + 13, DSTR_W, DSTR_H}; //destination pos for red button

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	if( Client )
	{
		Client->drawMap();
	}
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	if ( SettingsData.bAlphaEffects )
	{
		SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,buffer,NULL );
	}
	SDL_BlitSurface ( GraphicsData.gfx_destruction,&rDialogSrc,buffer,&rDialog ); //blit dialog
	SDL_BlitSurface ( GraphicsData.gfx_destruction_glas,NULL,buffer,&rGlass ); //blit security glass

	NormalButton btn_hot(   rDialog.x + 89, rDialog.y + 14, "Text~Button~Hot");
	NormalButton btn_cancel(rDialog.x + 89, rDialog.y + 46, "Text~Button~Cancel");
	btn_hot.Draw();
	btn_cancel.Draw();

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );
	while ( 1 )
	{
		if(Client)
		{
			if ( Client->SelectedBuilding == NULL ) break;
			Client->handleTimer();
			Client->doGameActions();
		}

		// Events holen:
		EventHandler->HandleEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		bool const down = b > LastB;
		bool const up   = b < LastB;

		if (btn_cancel.CheckClick(x, y, down, up))
		{
			return false;
		}

		if (btn_hot.CheckClick(x, y, down, up))
		{
			btn_hot.Lock();
			PlayFX(SoundData.SNDArm);
			Scharf = true;
		}

		// Das Schutzglas hochfahren:
		if ( Scharf&&GlasHeight>0&&Client->iTimer0 )
		{
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&rDestroySrc,buffer,&rDestroyDest );
			GlasHeight-=10;
			if ( GlasHeight>0 )
			{
				rGlass.h=GlasHeight;
				rGlass.x=0;rGlass.y=0;
				SDL_Rect rTmp={rDestroyDest.x, rDestroyDest.y, rDestroyDest.w, rGlass.h};
				SDL_BlitSurface ( GraphicsData.gfx_destruction_glas,&rGlass,buffer,&rTmp );
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Zerstören-Button:
		if ( GlasHeight<=0&&x>=rDestroyDest.x&&x<rDestroyDest.x+rDestroyDest.w&&y>=rDestroyDest.y&&y<rDestroyDest.y+rDestroyDest.h )
		{
			if ( b&&!DestroyPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				SDL_BlitSurface ( GraphicsData.gfx_destruction,&rDestroyPressedSrc,buffer,&rDestroyDest );
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
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&rDestroySrc,buffer,&rDestroyDest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DestroyPressed=false;
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
	return false;
}

void drawSlider (SDL_Surface *sfDialog,int offx,int offy,int value, SDL_Surface *surface )
{
	SDL_Rect scr, dest;
	SDL_Rect rDialog = { screen->w / 2 - sfDialog->w / 2, screen->h / 2 - sfDialog->h / 2, sfDialog->w, sfDialog->h };
	#define SLIDER_W 14
	#define SLIDER_H 17

	//BEGIN REDRAW DIALOG UNDER SLIDER
	/*Offset to read clean background from +/- 7 to
	*overdraw slider because slider is 14 fat and can
	*show half over the ends of the sliderbar*/
	scr.x = offx - rDialog.x - SLIDER_W / 2; //scr.x & scr.y = topleft
	scr.y = offy - rDialog.y ;
	scr.w = 57 + SLIDER_W;
	scr.h = SLIDER_H;
	dest.x = offx - 6;
	dest.y = offy - 7;
	SDL_BlitSurface ( sfDialog,&scr,surface,&dest );
	//END REDRAW DIALOG UNDER SLIDER

	//BEGIN DRAW SLIDERBAR
	scr.x=334; //get sliderbar from hud_stuff.pxc
	scr.y=82;
	scr.w=58;
	scr.h=3;
	dest.y += 7;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,&dest );
	//END DRAW SLIDERBAR

	//BEGIN DRAW SLIDER
	scr.x=412; //get slider from hud_stuff.pcx
	scr.y=46;
	scr.w=SLIDER_W;
	scr.h=SLIDER_H;

	dest.x=offx-6+ ( int ) ( ( 57/255.0 ) *value );
	dest.y=offy-SLIDER_W / 2;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,&dest );
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
	scr.w=18; //get button from hud_gfx
	scr.h=17;
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
		iPx = 5;
	}
	else
	{
		scr.x=308; //unclicked button
		iPx = 4;
	}
	scr.y=455; //get button from gfx_hud.pcx
	scr.w=77;
	scr.h=23;
	dest.x = x;
	dest.y = y;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,surface,&dest ); //show button on string
	font->showTextCentered(dest.x+scr.w/2,dest.y+iPx, sText, FONT_LATIN_NORMAL, surface);
}

void drawContextItem(string sText, bool bPressed, int x, int y, SDL_Surface *surface)
{
	SDL_Rect dest={x,y,42,21};
	SDL_Rect src={0,0,42,21}; //default button deselected
	if(bPressed) src.y+=21;

	SDL_BlitSurface ( GraphicsData.gfx_context_menu, &src, surface, &dest );
	font->showTextCentered ( dest.x + dest.w / 2, dest.y + (dest.h / 2 - font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2) +1, sText, FONT_LATIN_SMALL_WHITE );

	return;
}
