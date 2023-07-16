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

#include "settings.h"
#include "utility/log.h"
#include "utility/serialization/jsonarchive.h"

cKeysList KeysList;

namespace serialization
{
	//--------------------------------------------------------------------------
	const std::vector<std::pair<eMouseStyle, const char*>>
	sEnumStringMapping<eMouseStyle>::m =
	{
		{eMouseStyle::Modern, "Modern"},
		{eMouseStyle::OldSchool, "OldSchool"}
	};
} // namespace serialization
//------------------------------------------------------------------------------
cKeysList::cKeysList() :
	keyExit (cKeyCombination (SDLK_ESCAPE)),
	keyJumpToAction (cKeyCombination (SDLK_F11)),
	keyEndTurn (cKeyCombination (SDLK_RETURN)),
	keyChat (cKeyCombination (SDLK_TAB)),
	keyScroll8a (cKeyCombination (SDLK_UP)),
	keyScroll8b (cKeyCombination (SDLK_KP_8)),
	keyScroll2a (cKeyCombination (SDLK_DOWN)),
	keyScroll2b (cKeyCombination (SDLK_KP_2)),
	keyScroll6a (cKeyCombination (SDLK_RIGHT)),
	keyScroll6b (cKeyCombination (SDLK_KP_6)),
	keyScroll4a (cKeyCombination (SDLK_LEFT)),
	keyScroll4b (cKeyCombination (SDLK_KP_4)),
	keyScroll7 (cKeyCombination (SDLK_KP_7)),
	keyScroll9 (cKeyCombination (SDLK_KP_9)),
	keyScroll1 (cKeyCombination (SDLK_KP_1)),
	keyScroll3 (cKeyCombination (SDLK_KP_3)),
	keyZoomIna (cKeyCombination (SDLK_RIGHTBRACKET)),
	keyZoomInb (cKeyCombination (SDLK_KP_PLUS)),
	keyZoomOuta (cKeyCombination (SDLK_SLASH)),
	keyZoomOutb (cKeyCombination (SDLK_KP_MINUS)),
	keySavePosition1 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F5)),
	keySavePosition2 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F6)),
	keySavePosition3 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F7)),
	keySavePosition4 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F8)),
	keyPosition1 (cKeyCombination (SDLK_F5)),
	keyPosition2 (cKeyCombination (SDLK_F6)),
	keyPosition3 (cKeyCombination (SDLK_F7)),
	keyPosition4 (cKeyCombination (SDLK_F8)),
	keyFog (cKeyCombination (SDLK_n)),
	keyGrid (cKeyCombination (SDLK_g)),
	keyScan (cKeyCombination (SDLK_s)),
	keyRange (cKeyCombination (SDLK_r)),
	keyAmmo (cKeyCombination (SDLK_m)),
	keyHitpoints (cKeyCombination (SDLK_t)),
	keyColors (cKeyCombination (SDLK_f)),
	keyStatus (cKeyCombination (SDLK_p)),
	keySurvey (cKeyCombination (SDLK_h)),
	keyCenterUnit (cKeyCombination (SDLK_f)),
	keyUnitDone (cKeyCombination (SDLK_e)),
	keyUnitDoneAndNext (cKeyCombination (SDLK_SPACE)),
	keyAllDoneAndNext (cKeyCombination (toEnumFlag (eKeyModifierType::Ctrl), SDLK_SPACE)),
	keyUnitNext (cKeyCombination (SDLK_w)),
	keyUnitPrev (cKeyCombination (SDLK_q)),
	keyUnitMenuAttack (cKeyCombination (SDLK_a)),
	keyUnitMenuBuild (cKeyCombination (SDLK_b)),
	keyUnitMenuTransfer (cKeyCombination (SDLK_x)),
	keyUnitMenuEnter (cKeyCombination (SDLK_e)),
	keyUnitMenuAutomove (cKeyCombination (SDLK_a)),
	keyUnitMenuStart (cKeyCombination (SDLK_s)),
	keyUnitMenuStop (cKeyCombination (SDLK_s)),
	keyUnitMenuClear (cKeyCombination (SDLK_c)),
	keyUnitMenuSentry (cKeyCombination (SDLK_s)),
	keyUnitMenuManualFire (cKeyCombination (SDLK_m)),
	keyUnitMenuActivate (cKeyCombination (SDLK_a)),
	keyUnitMenuLoad (cKeyCombination (SDLK_l)),
	keyUnitMenuReload (cKeyCombination (SDLK_r)),
	keyUnitMenuRepair (cKeyCombination (SDLK_r)),
	keyUnitMenuLayMine (cKeyCombination (SDLK_l)),
	keyUnitMenuClearMine (cKeyCombination (SDLK_c)),
	keyUnitMenuDisable (cKeyCombination (SDLK_d)),
	keyUnitMenuSteal (cKeyCombination (SDLK_s)),
	keyUnitMenuInfo (cKeyCombination (SDLK_h)),
	keyUnitMenuDistribute (cKeyCombination (SDLK_d)),
	keyUnitMenuResearch (cKeyCombination (SDLK_r)),
	keyUnitMenuUpgrade (cKeyCombination (SDLK_u)),
	keyUnitMenuDestroy (cKeyCombination (SDLK_d)),
	mouseStyle (eMouseStyle::Modern)
{}

//------------------------------------------------------------------------------
void cKeysList::loadFromJsonFile (const std::filesystem::path& path)
{
	std::ifstream file (path);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.warn ("cannot load keys.json\ngenerating new file");
		saveToFile();
		return;
	}
	try
	{
		cJsonArchiveIn in (json);
		serialize (in);

		Log.debug ("Done");
	}
	catch (const std::exception& e)
	{
		Log.warn (std::string ("Error while reading keys: ") + e.what());
		Log.warn ("Overwriting with default settings");
		saveToFile();
	}
}

//------------------------------------------------------------------------------
void cKeysList::loadFromFile()
{
	Log.info ("Loading Keys");

	const auto keysJsonGame = cSettings::getInstance().getDataDir() / "keys.json";
	const auto keysJsonUsers = cSettings::getInstance().getMaxrHomeDir() / "keys.json";

	if (std::filesystem::exists (keysJsonUsers))
	{
		Log.info ("User key-file in use");
		loadFromJsonFile (keysJsonUsers);
	}
	else if (std::filesystem::exists (keysJsonGame))
	{
		std::filesystem::copy_file (keysJsonGame, keysJsonUsers);
		Log.info ("Key-file copied from gamedir to userdir");
		loadFromJsonFile (keysJsonUsers);
	}
	else
	{
		Log.warn ("generating new keys-file");
		saveToFile();
	}
}

//------------------------------------------------------------------------------
void cKeysList::saveToFile()
{
	nlohmann::json json;
	cJsonArchiveOut archive (json);

	serialize (archive);

	std::ofstream file (cSettings::getInstance().getMaxrHomeDir() / "keys.json");
	file << json.dump (0);
}

//------------------------------------------------------------------------------
eMouseStyle cKeysList::getMouseStyle() const
{
	return mouseStyle;
}
