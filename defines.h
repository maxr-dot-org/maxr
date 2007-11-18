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
#define ONE =1
#else
#define EX extern
#define ZERO
#define ONE 
#endif

#define SHOW_SCREEN SDL_BlitSurface(buffer,NULL,screen,NULL);if(SettingsData.bWindowMode)SDL_UpdateRect(screen,0,0,0,0);else{SDL_Flip(screen);}
#define MAXPLAYER_HOTSEAT 8

#endif


#ifdef WIN32
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "\\"
	#endif
	#ifndef TEXT_FILE_LF
		#define TEXT_FILE_LF "\r\n"
	#endif
		// GFX On Demand /////////////////////////////////////////////////////////////
	#define GFXOD_MAIN          "gfx\\main.pcx"
	#define GFXOD_HELP          "gfx\\help_screen.pcx"
	#define GFXOD_OPTIONS       "gfx\\options.pcx"
	#define GFXOD_PLANET_SELECT "gfx\\planet_select.pcx"
	#define GFXOD_HANGAR        "gfx_od\\hangar.pcx"
	#define GFXOD_MULT          "gfx\\multi.pcx"
	#define GFXOD_DIALOG4		"gfx\\dialog4.pcx"
	#define GFXOD_DIALOG5		"gfx\\dialog5.pcx"
	#define GFXOD_DIALOG6		"gfx\\dialog6.pcx"
	#define GFXOD_PLAYER_SELECT "customgame_menu.pcx"
#else
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "//"
	#endif
	#ifndef TEXT_FILE_LF
		#define TEXT_FILE_LF "\n"
	#endif
		// GFX On Demand /////////////////////////////////////////////////////////////
	#define GFXOD_MAIN          "gfx//main.pcx"
	#define GFXOD_HELP          "gfx//help_screen.pcx"
	#define GFXOD_OPTIONS       "gfx//options.pcx"
	#define GFXOD_PLANET_SELECT "gfx//planet_select.pcx"
	#define GFXOD_HANGAR        "gfx_od//hangar.pcx"
	#define GFXOD_MULT          "gfx//multi.pcx"
	#define GFXOD_DIALOG4		"gfx//dialog4.pcx"
	#define GFXOD_DIALOG5		"gfx//dialog5.pcx"
	#define GFXOD_DIALOG6		"gfx//dialog6.pcx"
	#define GFXOD_PLAYER_SELECT "customgame_menu.pcx"
#endif

// We have to take care of these manually !
#ifdef RELEASE
	#define MAXVERSION  "M.A.X. Reloaded 0.1.1 BUILD 200711142155"
#else
	#define MAXVERSION  "M.A.X. Reloaded 0.1.1 SVN"
#endif
#define MAX_VERSION     "0.1.1"
#define MAX_BUILD_DATE  "2007-11-14 21:55:00"

//#define MAXVERSION "M.A.X. Reloaded 0.1.1"
