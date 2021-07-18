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

#include "defines.h"
#include "game/serialization/xmlarchive.h"
#include "utility/files.h"
#include "utility/log.h"

#include <3rd/tinyxml2/tinyxml2.h>

cKeysList KeysList;

namespace serialization
{

	//--------------------------------------------------------------------------
	/* static */ std::string sEnumSerializer<eMouseStyle>::toString (eMouseStyle e)
	{
		switch (e)
		{
			case eMouseStyle::Modern: return "Modern";
			case eMouseStyle::OldSchool: return "OldSchool";
		}
		throw std::runtime_error ("Unsupported enum value for eMouseStyle:" + std::to_string (int(e)));
	}

	//--------------------------------------------------------------------------
	/* static */ eMouseStyle sEnumSerializer<eMouseStyle>::fromString (const std::string& s)
	{
		if (s == "Modern") return eMouseStyle::Modern;
		if (s == "OldSchool") return eMouseStyle::OldSchool;

		Log.write ("Unknown mouseStyle " + s + ", defaulting to Modern", cLog::eLOG_TYPE_WARNING);
		return eMouseStyle::Modern;
	}

}

//------------------------------------------------------------------------------
cKeysList::cKeysList() :
	keyExit (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)),
	keyJumpToAction (cKeyCombination (eKeyModifierType::None, SDLK_F11)),
	keyEndTurn (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)),
	keyChat (cKeyCombination (eKeyModifierType::None, SDLK_TAB)),
	keyScroll8a (cKeyCombination (eKeyModifierType::None, SDLK_UP)),
	keyScroll8b (cKeyCombination (eKeyModifierType::None, SDLK_KP_8)),
	keyScroll2a (cKeyCombination (eKeyModifierType::None, SDLK_DOWN)),
	keyScroll2b (cKeyCombination (eKeyModifierType::None, SDLK_KP_2)),
	keyScroll6a (cKeyCombination (eKeyModifierType::None, SDLK_RIGHT)),
	keyScroll6b (cKeyCombination (eKeyModifierType::None, SDLK_KP_6)),
	keyScroll4a (cKeyCombination (eKeyModifierType::None, SDLK_LEFT)),
	keyScroll4b (cKeyCombination (eKeyModifierType::None, SDLK_KP_4)),
	keyScroll7 (cKeyCombination (eKeyModifierType::None, SDLK_KP_7)),
	keyScroll9 (cKeyCombination (eKeyModifierType::None, SDLK_KP_9)),
	keyScroll1 (cKeyCombination (eKeyModifierType::None, SDLK_KP_1)),
	keyScroll3 (cKeyCombination (eKeyModifierType::None, SDLK_KP_3)),
	keyZoomIna (cKeyCombination (eKeyModifierType::None, SDLK_RIGHTBRACKET)),
	keyZoomInb (cKeyCombination (eKeyModifierType::None, SDLK_KP_PLUS)),
	keyZoomOuta (cKeyCombination (eKeyModifierType::None, SDLK_SLASH)),
	keyZoomOutb (cKeyCombination (eKeyModifierType::None, SDLK_KP_MINUS)),
	keySavePosition1 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F5)),
	keySavePosition2 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F6)),
	keySavePosition3 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F7)),
	keySavePosition4 (cKeyCombination (toEnumFlag (eKeyModifierType::Alt), SDLK_F8)),
	keyPosition1 (cKeyCombination (eKeyModifierType::None, SDLK_F5)),
	keyPosition2 (cKeyCombination (eKeyModifierType::None, SDLK_F6)),
	keyPosition3 (cKeyCombination (eKeyModifierType::None, SDLK_F7)),
	keyPosition4 (cKeyCombination (eKeyModifierType::None, SDLK_F8)),
	keyFog (cKeyCombination (eKeyModifierType::None, SDLK_n)),
	keyGrid (cKeyCombination (eKeyModifierType::None, SDLK_g)),
	keyScan (cKeyCombination (eKeyModifierType::None, SDLK_s)),
	keyRange (cKeyCombination (eKeyModifierType::None, SDLK_r)),
	keyAmmo (cKeyCombination (eKeyModifierType::None, SDLK_m)),
	keyHitpoints (cKeyCombination (eKeyModifierType::None, SDLK_t)),
	keyColors (cKeyCombination (eKeyModifierType::None, SDLK_f)),
	keyStatus (cKeyCombination (eKeyModifierType::None, SDLK_p)),
	keySurvey (cKeyCombination (eKeyModifierType::None, SDLK_h)),
	keyCenterUnit (cKeyCombination (eKeyModifierType::None, SDLK_f)),
	keyUnitDone (cKeyCombination (eKeyModifierType::None, SDLK_e)),
	keyUnitDoneAndNext (cKeyCombination (eKeyModifierType::None, SDLK_SPACE)),
	keyAllDoneAndNext (cKeyCombination (toEnumFlag (eKeyModifierType::Ctrl), SDLK_SPACE)),
	keyUnitNext (cKeyCombination (eKeyModifierType::None, SDLK_w)),
	keyUnitPrev (cKeyCombination (eKeyModifierType::None, SDLK_q)),
	keyUnitMenuAttack (cKeyCombination (eKeyModifierType::None, SDLK_a)),
	keyUnitMenuBuild (cKeyCombination (eKeyModifierType::None, SDLK_b)),
	keyUnitMenuTransfer (cKeyCombination (eKeyModifierType::None, SDLK_x)),
	keyUnitMenuEnter (cKeyCombination (eKeyModifierType::None, SDLK_e)),
	keyUnitMenuAutomove (cKeyCombination (eKeyModifierType::None, SDLK_a)),
	keyUnitMenuStart (cKeyCombination (eKeyModifierType::None, SDLK_s)),
	keyUnitMenuStop (cKeyCombination (eKeyModifierType::None, SDLK_s)),
	keyUnitMenuClear (cKeyCombination (eKeyModifierType::None, SDLK_c)),
	keyUnitMenuSentry (cKeyCombination (eKeyModifierType::None, SDLK_s)),
	keyUnitMenuManualFire (cKeyCombination (eKeyModifierType::None, SDLK_m)),
	keyUnitMenuActivate (cKeyCombination (eKeyModifierType::None, SDLK_a)),
	keyUnitMenuLoad (cKeyCombination (eKeyModifierType::None, SDLK_l)),
	keyUnitMenuReload (cKeyCombination (eKeyModifierType::None, SDLK_r)),
	keyUnitMenuRepair (cKeyCombination (eKeyModifierType::None, SDLK_r)),
	keyUnitMenuLayMine (cKeyCombination (eKeyModifierType::None, SDLK_l)),
	keyUnitMenuClearMine (cKeyCombination (eKeyModifierType::None, SDLK_c)),
	keyUnitMenuDisable (cKeyCombination (eKeyModifierType::None, SDLK_d)),
	keyUnitMenuSteal (cKeyCombination (eKeyModifierType::None, SDLK_s)),
	keyUnitMenuInfo (cKeyCombination (eKeyModifierType::None, SDLK_h)),
	keyUnitMenuDistribute (cKeyCombination (eKeyModifierType::None, SDLK_d)),
	keyUnitMenuResearch (cKeyCombination (eKeyModifierType::None, SDLK_r)),
	keyUnitMenuUpgrade (cKeyCombination (eKeyModifierType::None, SDLK_u)),
	keyUnitMenuDestroy (cKeyCombination (eKeyModifierType::None, SDLK_d)),
	mouseStyle (eMouseStyle::Modern)
{}

//------------------------------------------------------------------------------
void cKeysList::loadFromFile()
{
	Log.write ("Loading Keys", cLog::eLOG_TYPE_INFO);

	if (!FileExists (KEYS_XMLUsers) && !FileExists (KEYS_XMLGame))
	{
		Log.write ("generating new keys-file", cLog::eLOG_TYPE_WARNING);
		saveToFile();
		return;
	}
	else if (!FileExists (KEYS_XMLUsers))
	{
		copyFile (KEYS_XMLGame, KEYS_XMLUsers);
		Log.write ("Key-file copied from gamedir to userdir", cLog::eLOG_TYPE_INFO);
	}
	else // => (FileExists (KEYS_XMLUsers))
	{
		Log.write ("User key-file in use", cLog::eLOG_TYPE_INFO);
	}

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile (KEYS_XMLUsers) != tinyxml2::XML_NO_ERROR)
	{
		Log.write ("cannot load keys.xml\ngenerating new file", cLog::eLOG_TYPE_WARNING);
		saveToFile();
		return;
	}
	cXmlArchiveOut in (*doc.RootElement());

	try
	{
		serialize (in);

		Log.write ("Done", cLog::eLOG_TYPE_DEBUG);
	}
	catch (const std::exception& e)
	{
		Log.write (std::string ("Error while reading keys: ") + e.what(), cLog::eLOG_TYPE_WARNING);
		Log.write ("Overwriting with default settings", cLog::eLOG_TYPE_WARNING);
		saveToFile();
	}
}

//------------------------------------------------------------------------------
void cKeysList::saveToFile()
{
	tinyxml2::XMLDocument doc;
	doc.LinkEndChild (doc.NewElement ("Controles"));

	cXmlArchiveIn out (*doc.RootElement());

	serialize (out);

	const auto errorCode = doc.SaveFile (KEYS_XMLUsers);
	if (errorCode != tinyxml2::XML_NO_ERROR)
	{
		throw std::runtime_error (std::string ("Could not save key controls to '") + KEYS_XMLUsers + "'. Error code is " + std::to_string ((int)errorCode) + "."); // TODO: transform error code to text.
	}
}

//------------------------------------------------------------------------------
eMouseStyle cKeysList::getMouseStyle() const
{
	return mouseStyle;
}
