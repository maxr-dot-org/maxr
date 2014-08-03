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
	cKeySequence keyCenterUnit;
	cKeySequence keyUnitDone;
	cKeySequence keyUnitDoneAndNext;
	cKeySequence keyAllDoneAndNext;
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

	const static std::string keyExitName;
	const static std::string keyJumpToActionName;
	const static std::string keyEndTurnName;
	const static std::string keyChatName;
	const static std::string keyScroll8aName;
	const static std::string keyScroll8bName;
	const static std::string keyScroll2aName;
	const static std::string keyScroll2bName;
	const static std::string keyScroll6aName;
	const static std::string keyScroll6bName;
	const static std::string keyScroll4aName;
	const static std::string keyScroll4bName;
	const static std::string keyScroll7Name;
	const static std::string keyScroll9Name;
	const static std::string keyScroll1Name;
	const static std::string keyScroll3Name;
	const static std::string keyZoomInaName;
	const static std::string keyZoomInbName;
	const static std::string keyZoomOutaName;
	const static std::string keyZoomOutbName;
	const static std::string keyFogName;
	const static std::string keyGridName;
	const static std::string keyScanName;
	const static std::string keyRangeName;
	const static std::string keyAmmoName;
	const static std::string keyHitpointsName;
	const static std::string keyColorsName;
	const static std::string keyStatusName;
	const static std::string keySurveyName;
	const static std::string keyCenterUnitName;
	const static std::string keyUnitDoneName;
	const static std::string keyUnitDoneAndNextName;
	const static std::string keyAllDoneAndNextName;
	const static std::string keyUnitNextName;
	const static std::string keyUnitPrevName;
	const static std::string keyUnitMenuAttackName;
	const static std::string keyUnitMenuBuildName;
	const static std::string keyUnitMenuTransferName;
	const static std::string keyUnitMenuAutomoveName;
	const static std::string keyUnitMenuStartName;
	const static std::string keyUnitMenuStopName;
	const static std::string keyUnitMenuClearName;
	const static std::string keyUnitMenuSentryName;
	const static std::string keyUnitMenuManualFireName;
	const static std::string keyUnitMenuActivateName;
	const static std::string keyUnitMenuLoadName;
	const static std::string keyUnitMenuReloadName;
	const static std::string keyUnitMenuRepairName;
	const static std::string keyUnitMenuLayMineName;
	const static std::string keyUnitMenuClearMineName;
	const static std::string keyUnitMenuDisableName;
	const static std::string keyUnitMenuStealName;
	const static std::string keyUnitMenuInfoName;
	const static std::string keyUnitMenuDistributeName;
	const static std::string keyUnitMenuResearchName;
	const static std::string keyUnitMenuUpgradeName;
	const static std::string keyUnitMenuDestroyName;

	bool tryLoadSingleKey (const tinyxml2::XMLElement& parentElement, const std::string& elementName, cKeySequence& destination);
	void saveSingleKey (tinyxml2::XMLElement& parentElement, const std::string& elementName, const cKeySequence& source);
} EX KeysList;

typedef enum {OldSchool, Modern} eMouseStyle;
EX eMouseStyle MouseStyle;

#endif // keys_H
