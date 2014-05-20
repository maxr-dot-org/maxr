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

#ifndef keys_H
#define keys_H

#include "defines.h"
#include "input/keyboard/keysequence.h"

// Globale Daten /////////////////////////////////////////////////////////////
class cKeysList
{
public:
	cKeysList ();

	void loadFromFile ();
	void saveToFile ();

	cKeySequence keyExit;
	cKeySequence keyJumpToAction;
	cKeySequence keyEndTurn;
	cKeySequence keyChat;
	cKeySequence keyScroll8a;
	cKeySequence keyScroll8b;
	cKeySequence keyScroll2a;
	cKeySequence keyScroll2b;
	cKeySequence keyScroll6a;
	cKeySequence keyScroll6b;
	cKeySequence keyScroll4a;
	cKeySequence keyScroll4b;
	cKeySequence keyScroll7;
	cKeySequence keyScroll9;
	cKeySequence keyScroll1;
	cKeySequence keyScroll3;
	cKeySequence keyZoomIna;
	cKeySequence keyZoomInb;
	cKeySequence keyZoomOuta;
	cKeySequence keyZoomOutb;
	cKeySequence keyFog;
	cKeySequence keyGrid;
	cKeySequence keyScan;
	cKeySequence keyRange;
	cKeySequence keyAmmo;
	cKeySequence keyHitpoints;
	cKeySequence keyColors;
	cKeySequence keyStatus;
	cKeySequence keySurvey;
	cKeySequence keyCalcPath;
	cKeySequence keyCenterUnit;
	cKeySequence keyUnitDone;
	cKeySequence keyUnitDoneAndNext;
	cKeySequence keyUnitNext;
	cKeySequence keyUnitPrev;
	cKeySequence keyUnitMenuAttack;
	cKeySequence keyUnitMenuBuild;
	cKeySequence keyUnitMenuTransfer;
	cKeySequence keyUnitMenuAutomove;
	cKeySequence keyUnitMenuStart;
	cKeySequence keyUnitMenuStop;
	cKeySequence keyUnitMenuClear;
	cKeySequence keyUnitMenuSentry;
	cKeySequence keyUnitMenuManualFire;
	cKeySequence keyUnitMenuActivate;
	cKeySequence keyUnitMenuLoad;
	cKeySequence keyUnitMenuReload;
	cKeySequence keyUnitMenuRepair;
	cKeySequence keyUnitMenuLayMine;
	cKeySequence keyUnitMenuClearMine;
	cKeySequence keyUnitMenuDisable;
	cKeySequence keyUnitMenuSteal;
	cKeySequence keyUnitMenuInfo;
	cKeySequence keyUnitMenuDistribute;
	cKeySequence keyUnitMenuResearch;
	cKeySequence keyUnitMenuUpgrade;
	cKeySequence keyUnitMenuDestroy;
private:
} EX KeysList;

typedef enum {OldSchool, Modern} eMouseStyle;
EX eMouseStyle MouseStyle;

#endif // keys_H
