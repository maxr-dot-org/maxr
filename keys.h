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
#ifndef keysH
#define keysH
#include "defines.h"
#include "SDL.h"
#include "main.h"

// Globale Daten /////////////////////////////////////////////////////////////
class cKeysList
{
public:
	SDLKey KeyExit;
	SDLKey KeyJumpToAction;
	SDLKey KeyEndTurn;
	SDLKey KeyChat;
	SDLKey KeyScroll8a;
	SDLKey KeyScroll8b;
	SDLKey KeyScroll2a;
	SDLKey KeyScroll2b;
	SDLKey KeyScroll6a;
	SDLKey KeyScroll6b;
	SDLKey KeyScroll4a;
	SDLKey KeyScroll4b;
	SDLKey KeyScroll7;
	SDLKey KeyScroll9;
	SDLKey KeyScroll1;
	SDLKey KeyScroll3;
	SDLKey KeyZoomIna;
	SDLKey KeyZoomInb;
	SDLKey KeyZoomOuta;
	SDLKey KeyZoomOutb;
	SDLKey KeyFog;
	SDLKey KeyGrid;
	SDLKey KeyScan;
	SDLKey KeyRange;
	SDLKey KeyAmmo;
	SDLKey KeyHitpoints;
	SDLKey KeyColors;
	SDLKey KeyStatus;
	SDLKey KeySurvey;
	SDLKey KeyCalcPath;
} EX KeysList;

typedef enum {OldSchool,Modern}eMouseStyle;
EX eMouseStyle MouseStyle;

// Prototypen ////////////////////////////////////////////////////////////////
int LoadKeys();
const char *GetKeyString(SDLKey key);
SDLKey GetKeyFromString(string key);
/**
	* Generats a new keys.xml file
	*/
void GenerateKeysXml();

#endif
