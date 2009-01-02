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
#ifndef definesH
#define definesH

#ifdef __main__
#define EX
#define ZERO =0
#else
#define EX extern
#define ZERO
#endif

#define SHOW_SCREEN SDL_BlitSurface(buffer,NULL,screen,NULL);if(SettingsData.bWindowMode)SDL_UpdateRect(screen,0,0,0,0);else{SDL_Flip(screen);}
#define MAXPLAYER_HOTSEAT 8

#ifdef _MSC_VER
	#define CHECK_MEMORY //_CrtCheckMemory();
#else
	#define CHECK_MEMORY 
#endif

#endif

//some defines for typical menus
#define DIALOG_W 640
#define DIALOG_H 480

#define MENU_OFFSET_X	( SettingsData.iScreenW / 2 - DIALOG_W / 2 )
#define MENU_OFFSET_Y	( SettingsData.iScreenH / 2 - DIALOG_H / 2 )

#ifndef PATH_DELIMITER
#	define PATH_DELIMITER "/"
#endif

#ifndef TEXT_FILE_LF
#	ifdef WIN32
#		define TEXT_FILE_LF "\r\n"
#	else
#		define TEXT_FILE_LF "\n"
#	endif
#endif

// GFX On Demand /////////////////////////////////////////////////////////////
#define GFXOD_MAIN            (SettingsData.sGfxPath + PATH_DELIMITER + "main.pcx").c_str()
#define GFXOD_HELP            (SettingsData.sGfxPath + PATH_DELIMITER + "help_screen.pcx").c_str()
#define GFXOD_OPTIONS         (SettingsData.sGfxPath + PATH_DELIMITER + "options.pcx").c_str()
#define GFXOD_PLANET_SELECT   (SettingsData.sGfxPath + PATH_DELIMITER + "planet_select.pcx").c_str()
#define GFXOD_PLAYER_SELECT   (SettingsData.sGfxPath + PATH_DELIMITER + "customgame_menu.pcx").c_str()
#define GFXOD_PLAYERHS_SELECT (SettingsData.sGfxPath + PATH_DELIMITER + "hotseatplayers.pcx").c_str()
#define GFXOD_HANGAR          (SettingsData.sGfxPath + PATH_DELIMITER + "hangar.pcx").c_str()
#define GFXOD_MULT            (SettingsData.sGfxPath + PATH_DELIMITER + "multi.pcx").c_str()
#define GFXOD_DIALOG2         (SettingsData.sGfxPath + PATH_DELIMITER + "dialog2.pcx").c_str()
#define GFXOD_DIALOG4         (SettingsData.sGfxPath + PATH_DELIMITER + "dialog4.pcx").c_str()
#define GFXOD_DIALOG5         (SettingsData.sGfxPath + PATH_DELIMITER + "dialog5.pcx").c_str()
#define GFXOD_DIALOG6         (SettingsData.sGfxPath + PATH_DELIMITER + "dialog6.pcx").c_str()

// Other Resources /////////////////////////////////////////////////////////////
#define MAX_XML               "max.xml"
#define MAX_LOG			"maxr.log"
#define MAX_NET_LOG		"net.log"
#define KEYS_XML              (SettingsData.sExePath + "keys.xml").c_str()
#define SPLASH_BACKGROUND     (SettingsData.sExePath + "init.pcx").c_str()
#if MAC
	#define MAXR_ICON             (SettingsData.sExePath + "maxr_mac.bmp").c_str()
#else
	#define MAXR_ICON             (SettingsData.sExePath + "maxr.bmp").c_str()
#endif

// We have to take care of these manually !
#ifdef RELEASE
#define MAXVERSION  "M.A.X. Reloaded 0.2.2 BUILD 200811231635" // Builddate: JJJJMMDDHHMM
#else
	#define MAXVERSION  "M.A.X. Reloaded 0.2.2 SVN Rev 1762"
#endif
#define MAX_VERSION     "0.2.2"
#define MAX_BUILD_DATE  "2008-12-23 16:35:00"

//#define MAXVERSION "M.A.X. Reloaded 0.2.2"
