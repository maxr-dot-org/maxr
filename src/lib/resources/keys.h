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

#ifndef resources_keysH
#define resources_keysH

#include "input/keyboard/keysequence.h"
#include "utility/serialization/serialization.h"

#include <filesystem>

enum class eMouseStyle
{
	OldSchool,
	Modern
};

namespace serialization
{
	template <>
	struct sEnumStringMapping<eMouseStyle>
	{
		static const std::vector<std::pair<eMouseStyle, const char*>> m;
	};
} // namespace serialization
class cKeysList
{
public:
	cKeysList();

	void loadFromFile();
	void saveToFile();

	eMouseStyle getMouseStyle() const;

public:
	void loadFromJsonFile (const std::filesystem::path&);

public:
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
	cKeySequence keySavePosition1;
	cKeySequence keySavePosition2;
	cKeySequence keySavePosition3;
	cKeySequence keySavePosition4;
	cKeySequence keyPosition1;
	cKeySequence keyPosition2;
	cKeySequence keyPosition3;
	cKeySequence keyPosition4;
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
	cKeySequence keyUnitMenuEnter;
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

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (keyExit);
		archive & NVP (keyJumpToAction);
		archive & NVP (keyEndTurn);
		archive & NVP (keyChat);
		archive & NVP (keyScroll8a);
		archive & NVP (keyScroll8b);
		archive & NVP (keyScroll2a);
		archive & NVP (keyScroll2b);
		archive & NVP (keyScroll6a);
		archive & NVP (keyScroll6b);
		archive & NVP (keyScroll4a);
		archive & NVP (keyScroll4b);
		archive & NVP (keyScroll7);
		archive & NVP (keyScroll9);
		archive & NVP (keyScroll1);
		archive & NVP (keyScroll3);
		archive & NVP (keyZoomIna);
		archive & NVP (keyZoomInb);
		archive & NVP (keyZoomOuta);
		archive & NVP (keyZoomOutb);
		archive & NVP (keySavePosition1);
		archive & NVP (keySavePosition2);
		archive & NVP (keySavePosition3);
		archive & NVP (keySavePosition4);
		archive & NVP (keyPosition1);
		archive & NVP (keyPosition2);
		archive & NVP (keyPosition3);
		archive & NVP (keyPosition4);
		archive & NVP (keyFog);
		archive & NVP (keyGrid);
		archive & NVP (keyScan);
		archive & NVP (keyRange);
		archive & NVP (keyAmmo);
		archive & NVP (keyHitpoints);
		archive & NVP (keyColors);
		archive & NVP (keyStatus);
		archive & NVP (keySurvey);
		archive & NVP (keyCenterUnit);
		archive & NVP (keyUnitDone);
		archive & NVP (keyUnitDoneAndNext);
		archive & NVP (keyAllDoneAndNext);
		archive & NVP (keyUnitNext);
		archive & NVP (keyUnitPrev);
		archive & NVP (keyUnitMenuAttack);
		archive & NVP (keyUnitMenuBuild);
		archive & NVP (keyUnitMenuTransfer);
		archive & NVP (keyUnitMenuEnter);
		archive & NVP (keyUnitMenuAutomove);
		archive & NVP (keyUnitMenuStart);
		archive & NVP (keyUnitMenuStop);
		archive & NVP (keyUnitMenuClear);
		archive & NVP (keyUnitMenuSentry);
		archive & NVP (keyUnitMenuManualFire);
		archive & NVP (keyUnitMenuActivate);
		archive & NVP (keyUnitMenuLoad);
		archive & NVP (keyUnitMenuReload);
		archive & NVP (keyUnitMenuRepair);
		archive & NVP (keyUnitMenuLayMine);
		archive & NVP (keyUnitMenuClearMine);
		archive & NVP (keyUnitMenuDisable);
		archive & NVP (keyUnitMenuSteal);
		archive & NVP (keyUnitMenuInfo);
		archive & NVP (keyUnitMenuDistribute);
		archive & NVP (keyUnitMenuResearch);
		archive & NVP (keyUnitMenuUpgrade);
		archive & NVP (keyUnitMenuDestroy);

		archive & NVP (mouseStyle);
		// clang-format on
	}

private:
	eMouseStyle mouseStyle;
};

extern cKeysList KeysList;

#endif
