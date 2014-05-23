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

#include "keys.h"

//------------------------------------------------------------------------------
cKeysList::cKeysList () :
	keyExit("Esc"),
	keyJumpToAction("F11"),
	keyEndTurn("Return"),
	keyChat("Tab"),
	keyScroll8a("Up"),
	keyScroll8b("KP8"),
	keyScroll2a("Down"),
	keyScroll2b("KP2"),
	keyScroll6a("Right"),
	keyScroll6b("KP6"),
	keyScroll4a("Left"),
	keyScroll4b("KP4"),
	keyScroll7("KP7"),
	keyScroll9("KP9"),
	keyScroll1("KP1"),
	keyScroll3("KP3"),
	keyZoomIna("]"),
	keyZoomInb("KPPlus"),
	keyZoomOuta("/"),
	keyZoomOutb("KPMinus"),
	keyFog("N"),
	keyGrid("G"),
	keyScan("S"),
	keyRange("R"),
	keyAmmo("A"),
	keyHitpoints("T"),
	keyColors("F"),
	keyStatus("P"),
	keySurvey("H"),
	keyCalcPath("Shift"),
	keyCenterUnit("F"),
	keyUnitDone("E"),
	keyUnitDoneAndNext("Space"),
	keyUnitNext("W"),
	keyUnitPrev("Q"),
	keyUnitMenuAttack("A"),
	keyUnitMenuBuild("B"),
	keyUnitMenuTransfer("X"),
	keyUnitMenuAutomove("A"),
	keyUnitMenuStart("S"),
	keyUnitMenuStop("S"),
	keyUnitMenuClear("C"),
	keyUnitMenuSentry("S"),
	keyUnitMenuManualFire("M"),
	keyUnitMenuActivate("A"),
	keyUnitMenuLoad("L"),
	keyUnitMenuReload("R"),
	keyUnitMenuRepair("R"),
	keyUnitMenuLayMine("L"),
	keyUnitMenuClearMine("C"),
	keyUnitMenuDisable("D"),
	keyUnitMenuSteal("S"),
	keyUnitMenuInfo("H"),
	keyUnitMenuDistribute("D"),
	keyUnitMenuResearch("R"),
	keyUnitMenuUpgrade("U"),
	keyUnitMenuDestroy("D")
{}

//------------------------------------------------------------------------------
void cKeysList::loadFromFile ()
{

}

//------------------------------------------------------------------------------
void cKeysList::saveToFile ()
{

}