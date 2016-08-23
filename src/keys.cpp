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

#include "tinyxml2.h"

#include "keys.h"
#include "utility/files.h"
#include "utility/log.h"
#include "main.h" // iToStr
#include "extendedtinyxml.h"

using namespace tinyxml2;

const std::string cKeysList::keyExitName = "KeyExit";
const std::string cKeysList::keyJumpToActionName = "KeyJumpToAction";
const std::string cKeysList::keyEndTurnName = "KeyEndTurn";
const std::string cKeysList::keyChatName = "KeyChat";
const std::string cKeysList::keyScroll8aName = "KeyScroll8a";
const std::string cKeysList::keyScroll8bName = "KeyScroll8b";
const std::string cKeysList::keyScroll2aName = "KeyScroll2a";
const std::string cKeysList::keyScroll2bName = "KeyScroll2b";
const std::string cKeysList::keyScroll6aName = "KeyScroll6a";
const std::string cKeysList::keyScroll6bName = "KeyScroll6b";
const std::string cKeysList::keyScroll4aName = "KeyScroll4a";
const std::string cKeysList::keyScroll4bName = "KeyScroll4b";
const std::string cKeysList::keyScroll7Name = "KeyScroll7";
const std::string cKeysList::keyScroll9Name = "KeyScroll9";
const std::string cKeysList::keyScroll1Name = "KeyScroll1";
const std::string cKeysList::keyScroll3Name = "KeyScroll3";
const std::string cKeysList::keyZoomInaName = "KeyZoomIna";
const std::string cKeysList::keyZoomInbName = "KeyZoomInb";
const std::string cKeysList::keyZoomOutaName = "KeyZoomOuta";
const std::string cKeysList::keyZoomOutbName = "KeyZoomOutb";
const std::string cKeysList::keyFogName = "KeyFog";
const std::string cKeysList::keyGridName = "KeyGrid";
const std::string cKeysList::keyScanName = "KeyScan";
const std::string cKeysList::keyRangeName = "KeyRange";
const std::string cKeysList::keyAmmoName = "KeyAmmo";
const std::string cKeysList::keyHitpointsName = "KeyHitpoints";
const std::string cKeysList::keyColorsName = "KeyColors";
const std::string cKeysList::keyStatusName = "KeyStatus";
const std::string cKeysList::keySurveyName = "KeySurvey";
const std::string cKeysList::keyCenterUnitName = "KeyCenterUnit";
const std::string cKeysList::keyUnitDoneName = "KeyUnitDone";
const std::string cKeysList::keyUnitDoneAndNextName = "KeyUnitDoneAndNext";
const std::string cKeysList::keyAllDoneAndNextName = "KeyAllDoneAndNext";
const std::string cKeysList::keyUnitNextName = "KeyUnitNext";
const std::string cKeysList::keyUnitPrevName = "KeyUnitPrev";
const std::string cKeysList::keyUnitMenuAttackName = "KeyUnitMenuAttack";
const std::string cKeysList::keyUnitMenuBuildName = "KeyUnitMenuBuild";
const std::string cKeysList::keyUnitMenuTransferName = "KeyUnitMenuTransfer";
const std::string cKeysList::keyUnitMenuAutomoveName = "KeyUnitMenuAutomove";
const std::string cKeysList::keyUnitMenuStartName = "KeyUnitMenuStart";
const std::string cKeysList::keyUnitMenuStopName = "KeyUnitMenuStop";
const std::string cKeysList::keyUnitMenuClearName = "KeyUnitMenuClear";
const std::string cKeysList::keyUnitMenuSentryName = "KeyUnitMenuSentry";
const std::string cKeysList::keyUnitMenuManualFireName = "KeyUnitMenuManualFire";
const std::string cKeysList::keyUnitMenuActivateName = "KeyUnitMenuActivate";
const std::string cKeysList::keyUnitMenuLoadName = "KeyUnitMenuLoad";
const std::string cKeysList::keyUnitMenuReloadName = "KeyUnitMenuReload";
const std::string cKeysList::keyUnitMenuRepairName = "KeyUnitMenuRepair";
const std::string cKeysList::keyUnitMenuLayMineName = "KeyUnitMenuLayMine";
const std::string cKeysList::keyUnitMenuClearMineName = "KeyUnitMenuClearMine";
const std::string cKeysList::keyUnitMenuDisableName = "KeyUnitMenuDisable";
const std::string cKeysList::keyUnitMenuStealName = "KeyUnitMenuSteal";
const std::string cKeysList::keyUnitMenuInfoName = "KeyUnitMenuInfo";
const std::string cKeysList::keyUnitMenuDistributeName = "KeyUnitMenuDistribute";
const std::string cKeysList::keyUnitMenuResearchName = "KeyUnitMenuResearch";
const std::string cKeysList::keyUnitMenuUpgradeName = "KeyUnitMenuUpgrade";
const std::string cKeysList::keyUnitMenuDestroyName = "KeyUnitMenuDestroy";

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
	keyAllDoneAndNext (cKeyCombination (toEnumFlag (eKeyModifierType::CtrlLeft) | eKeyModifierType::CtrlRight, SDLK_SPACE)),
	keyUnitNext (cKeyCombination (eKeyModifierType::None, SDLK_w)),
	keyUnitPrev (cKeyCombination (eKeyModifierType::None, SDLK_q)),
	keyUnitMenuAttack (cKeyCombination (eKeyModifierType::None, SDLK_a)),
	keyUnitMenuBuild (cKeyCombination (eKeyModifierType::None, SDLK_b)),
	keyUnitMenuTransfer (cKeyCombination (eKeyModifierType::None, SDLK_x)),
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

	XMLDocument keysXml;
	if (keysXml.LoadFile (KEYS_XMLUsers) != XML_NO_ERROR)
	{
		Log.write ("cannot load keys.xml\ngenerating new file", cLog::eLOG_TYPE_WARNING);
		saveToFile();
		return;
	}

	const XMLElement* keysElement = XmlGetFirstElement (keysXml, "Controles", "Keys", nullptr);

	if (!keysElement)
	{
		Log.write ("invalid keys.xml: missing 'Keys' element\ngenerating new file", cLog::eLOG_TYPE_WARNING);
		saveToFile();
		return;
	}

	const XMLElement* mouseElement = XmlGetFirstElement (keysXml, "Controles", "Mouse", nullptr);

	if (!mouseElement)
	{
		Log.write ("invalid keys.xml: missing 'Mouse' element\ngenerating new file", cLog::eLOG_TYPE_WARNING);
		saveToFile();
		return;
	}

	bool loadedAll = true;
	loadedAll = tryLoadSingleKey (*keysElement, keyExitName, keyExit) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyJumpToActionName, keyJumpToAction) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyEndTurnName, keyEndTurn) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyChatName, keyChat) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll8aName, keyScroll8a) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll8bName, keyScroll8b) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll2aName, keyScroll2a) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll2bName, keyScroll2b) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll6aName, keyScroll6a) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll6bName, keyScroll6b) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll4aName, keyScroll4a) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll4bName, keyScroll4b) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll7Name, keyScroll7) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll9Name, keyScroll9) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll1Name, keyScroll1) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScroll3Name, keyScroll3) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyZoomInaName, keyZoomIna) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyZoomInbName, keyZoomInb) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyZoomOutaName, keyZoomOuta) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyZoomOutbName, keyZoomOutb) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyFogName, keyFog) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyGridName, keyGrid) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyScanName, keyScan) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyRangeName, keyRange) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyAmmoName, keyAmmo) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyHitpointsName, keyHitpoints) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyColorsName, keyColors) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyStatusName, keyStatus) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keySurveyName, keySurvey) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyCenterUnitName, keyCenterUnit) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitDoneName, keyUnitDone) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitDoneAndNextName, keyUnitDoneAndNext) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyAllDoneAndNextName, keyAllDoneAndNext) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitNextName, keyUnitNext) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitPrevName, keyUnitPrev) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuAttackName, keyUnitMenuAttack) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuBuildName, keyUnitMenuBuild) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuTransferName, keyUnitMenuTransfer) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuAutomoveName, keyUnitMenuAutomove) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuStartName, keyUnitMenuStart) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuStopName, keyUnitMenuStop) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuClearName, keyUnitMenuClear) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuSentryName, keyUnitMenuSentry) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuManualFireName, keyUnitMenuManualFire) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuActivateName, keyUnitMenuActivate) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuLoadName, keyUnitMenuLoad) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuReloadName, keyUnitMenuReload) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuRepairName, keyUnitMenuRepair) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuLayMineName, keyUnitMenuLayMine) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuClearMineName, keyUnitMenuClearMine) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuDisableName, keyUnitMenuDisable) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuStealName, keyUnitMenuSteal) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuInfoName, keyUnitMenuInfo) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuDistributeName, keyUnitMenuDistribute) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuResearchName, keyUnitMenuResearch) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuUpgradeName, keyUnitMenuUpgrade) && loadedAll;
	loadedAll = tryLoadSingleKey (*keysElement, keyUnitMenuDestroyName, keyUnitMenuDestroy) && loadedAll;


	const auto mouseStyleElement = mouseElement->FirstChildElement ("MOUSE_STYLE");

	if (mouseStyleElement)
	{
		const auto attribute = mouseStyleElement->FindAttribute ("Text");
		if (attribute)
		{
			if (strcmp (attribute->Value(), "OLD_SCHOOL") == 0)
			{
				mouseStyle = eMouseStyle::OldSchool;
			}
			else if (strcmp (attribute->Value(), "MODERN") == 0)
			{
				mouseStyle = eMouseStyle::Modern;
			}
			else
			{
				mouseStyle = eMouseStyle::Modern;
				Log.write (std::string ("Unknown mouse style '") + attribute->Value() + "'. Fall back to modern style.", cLog::eLOG_TYPE_WARNING);
			}
		}
		else
		{
			Log.write ("Could not find XML attribute 'Text' in element 'MOUSE_STYLE' in keys XML. Skipping loading of mouse style.", cLog::eLOG_TYPE_WARNING);
			loadedAll = false;
		}
	}
	else
	{
		Log.write ("Could not find XML element 'MOUSE_STYLE' in keys XML. Skipping loading of this mouse information.", cLog::eLOG_TYPE_WARNING);
		loadedAll = false;
	}

	// write file completely new if some settings have been missing
	if (!loadedAll)
	{
		saveToFile();
	}

	Log.write ("Done", cLog::eLOG_TYPE_DEBUG);
}

//------------------------------------------------------------------------------
void cKeysList::saveToFile()
{
	XMLDocument keysXml;
	auto rootElement = keysXml.NewElement ("Controles");
	keysXml.LinkEndChild (rootElement);

	auto keysElement = keysXml.NewElement ("Keys");
	rootElement->LinkEndChild (keysElement);

	saveSingleKey (*keysElement, keyExitName, keyExit);
	saveSingleKey (*keysElement, keyJumpToActionName, keyJumpToAction);
	saveSingleKey (*keysElement, keyEndTurnName, keyEndTurn);
	saveSingleKey (*keysElement, keyChatName, keyChat);
	saveSingleKey (*keysElement, keyScroll8aName, keyScroll8a);
	saveSingleKey (*keysElement, keyScroll8bName, keyScroll8b);
	saveSingleKey (*keysElement, keyScroll2aName, keyScroll2a);
	saveSingleKey (*keysElement, keyScroll2bName, keyScroll2b);
	saveSingleKey (*keysElement, keyScroll6aName, keyScroll6a);
	saveSingleKey (*keysElement, keyScroll6bName, keyScroll6b);
	saveSingleKey (*keysElement, keyScroll4aName, keyScroll4a);
	saveSingleKey (*keysElement, keyScroll4bName, keyScroll4b);
	saveSingleKey (*keysElement, keyScroll7Name, keyScroll7);
	saveSingleKey (*keysElement, keyScroll9Name, keyScroll9);
	saveSingleKey (*keysElement, keyScroll1Name, keyScroll1);
	saveSingleKey (*keysElement, keyScroll3Name, keyScroll3);
	saveSingleKey (*keysElement, keyZoomInaName, keyZoomIna);
	saveSingleKey (*keysElement, keyZoomInbName, keyZoomInb);
	saveSingleKey (*keysElement, keyZoomOutaName, keyZoomOuta);
	saveSingleKey (*keysElement, keyZoomOutbName, keyZoomOutb);
	saveSingleKey (*keysElement, keyFogName, keyFog);
	saveSingleKey (*keysElement, keyGridName, keyGrid);
	saveSingleKey (*keysElement, keyScanName, keyScan);
	saveSingleKey (*keysElement, keyRangeName, keyRange);
	saveSingleKey (*keysElement, keyAmmoName, keyAmmo);
	saveSingleKey (*keysElement, keyHitpointsName, keyHitpoints);
	saveSingleKey (*keysElement, keyColorsName, keyColors);
	saveSingleKey (*keysElement, keyStatusName, keyStatus);
	saveSingleKey (*keysElement, keySurveyName, keySurvey);
	saveSingleKey (*keysElement, keyCenterUnitName, keyCenterUnit);
	saveSingleKey (*keysElement, keyUnitDoneName, keyUnitDone);
	saveSingleKey (*keysElement, keyUnitDoneAndNextName, keyUnitDoneAndNext);
	saveSingleKey (*keysElement, keyAllDoneAndNextName, keyAllDoneAndNext);
	saveSingleKey (*keysElement, keyUnitNextName, keyUnitNext);
	saveSingleKey (*keysElement, keyUnitPrevName, keyUnitPrev);
	saveSingleKey (*keysElement, keyUnitMenuAttackName, keyUnitMenuAttack);
	saveSingleKey (*keysElement, keyUnitMenuBuildName, keyUnitMenuBuild);
	saveSingleKey (*keysElement, keyUnitMenuTransferName, keyUnitMenuTransfer);
	saveSingleKey (*keysElement, keyUnitMenuAutomoveName, keyUnitMenuAutomove);
	saveSingleKey (*keysElement, keyUnitMenuStartName, keyUnitMenuStart);
	saveSingleKey (*keysElement, keyUnitMenuStopName, keyUnitMenuStop);
	saveSingleKey (*keysElement, keyUnitMenuClearName, keyUnitMenuClear);
	saveSingleKey (*keysElement, keyUnitMenuSentryName, keyUnitMenuSentry);
	saveSingleKey (*keysElement, keyUnitMenuManualFireName, keyUnitMenuManualFire);
	saveSingleKey (*keysElement, keyUnitMenuActivateName, keyUnitMenuActivate);
	saveSingleKey (*keysElement, keyUnitMenuLoadName, keyUnitMenuLoad);
	saveSingleKey (*keysElement, keyUnitMenuReloadName, keyUnitMenuReload);
	saveSingleKey (*keysElement, keyUnitMenuRepairName, keyUnitMenuRepair);
	saveSingleKey (*keysElement, keyUnitMenuLayMineName, keyUnitMenuLayMine);
	saveSingleKey (*keysElement, keyUnitMenuClearMineName, keyUnitMenuClearMine);
	saveSingleKey (*keysElement, keyUnitMenuDisableName, keyUnitMenuDisable);
	saveSingleKey (*keysElement, keyUnitMenuStealName, keyUnitMenuSteal);
	saveSingleKey (*keysElement, keyUnitMenuInfoName, keyUnitMenuInfo);
	saveSingleKey (*keysElement, keyUnitMenuDistributeName, keyUnitMenuDistribute);
	saveSingleKey (*keysElement, keyUnitMenuResearchName, keyUnitMenuResearch);
	saveSingleKey (*keysElement, keyUnitMenuUpgradeName, keyUnitMenuUpgrade);
	saveSingleKey (*keysElement, keyUnitMenuDestroyName, keyUnitMenuDestroy);

	auto mouseElement = keysXml.NewElement ("Mouse");
	rootElement->LinkEndChild (mouseElement);

	auto mouseStyleElement = mouseElement->GetDocument()->NewElement ("MOUSE_STYLE");
	mouseStyleElement->SetAttribute ("Text", mouseStyle == eMouseStyle::OldSchool ? "OLD_SCHOOL" : "MODERN");
	mouseElement->LinkEndChild (mouseStyleElement);

	const auto errorCode = keysXml.SaveFile (KEYS_XMLUsers);
	if (errorCode != XML_NO_ERROR)
	{
		throw std::runtime_error (std::string ("Could not save key controls to '") + KEYS_XMLUsers + "'. Error code is " + iToStr ((int)errorCode) + "."); // TODO: transform error code to text.
	}
}

//------------------------------------------------------------------------------
bool cKeysList::tryLoadSingleKey (const tinyxml2::XMLElement& parentElement, const std::string& elementName, cKeySequence& destination)
{
	const auto keyElement = parentElement.FirstChildElement (elementName.c_str());

	if (!keyElement)
	{
		Log.write ("Could not find XML element '" + elementName + "' in keys XML. Skipping loading of this key.", cLog::eLOG_TYPE_WARNING);
		return false;
	}

	const auto attribute = keyElement->FindAttribute ("Text");

	if (!attribute)
	{
		Log.write ("Could not find XML attribute 'Text' in element '" + elementName + "' in keys XML. Skipping loading of this key.", cLog::eLOG_TYPE_WARNING);
		return false;
	}

	try
	{
		destination = cKeySequence (attribute->Value());
	}
	catch (std::runtime_error& e)
	{
		Log.write ("Error while reading key sequence of element '" + elementName + "' in keys XML: " + e.what(), cLog::eLOG_TYPE_WARNING);
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
void cKeysList::saveSingleKey (tinyxml2::XMLElement& parentElement, const std::string& elementName, const cKeySequence& source)
{
	auto keyElement = parentElement.GetDocument()->NewElement (elementName.c_str());

	keyElement->SetAttribute ("Text", source.toString().c_str());

	parentElement.LinkEndChild (keyElement);
}

//------------------------------------------------------------------------------
eMouseStyle cKeysList::getMouseStyle() const
{
	return mouseStyle;
}