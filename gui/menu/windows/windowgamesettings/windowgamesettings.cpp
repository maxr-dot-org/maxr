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

#include "windowgamesettings.h"
#include "gamesettings.h"
#include "../../../../main.h"
#include "../../../../pcx.h"
#include "../../widgets/label.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/checkbox.h"
#include "../../widgets/radiogroup.h"

enum eSettingsStartCredits
{
	SETTING_CREDITS_LOWEST = 0,
	SETTING_CREDITS_LOWER = 50,
	SETTING_CREDITS_LOW = 100,
	SETTING_CREDITS_NORMAL = 150,
	SETTING_CREDITS_MUCH = 200,
	SETTING_CREDITS_MORE = 250
};

//------------------------------------------------------------------------------
cWindowGameSettings::cWindowGameSettings () :
	cWindow (LoadPCX (GFXOD_OPTIONS))
{
	const auto& menuPosition = getArea ().getMinCorner ();

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (0, 13), menuPosition + cPosition (getArea ().getMaxCorner ().x (), 23)), lngPack.i18n ("Text~Button~Game_Options"), FONT_LATIN_NORMAL, eAlignmentType::Center));

	int currentLine = 57;
	const int lineHeight = 16;
	
	//
	// Resources
	//

	// Metal
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Metal") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	metalRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	metalLowCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Low"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalNormalCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalMuchCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Much"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalMostCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~Most"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Oil
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Oil") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	oilRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	oilLowCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Low"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilNormalCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilMuchCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Much"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilMostCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~Most"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Gold
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Gold") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	goldRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	goldLowCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Low"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldNormalCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldMuchCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Much"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldMostCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~Most"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Density
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Resource_Density") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	densityRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	densityThinCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Thin"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityNormalCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityThickCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Thick"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityMostCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~Most"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight * 3;

	//
	// Bridgehead
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~BridgeHead") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	bridgeheadRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	bridgeheadMobileCheckBox = bridgeheadRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Mobile"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	bridgeheadDefiniteCheckBox = bridgeheadRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Definite"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	//
	// Game type
	//
	const bool hotSeat = false;
	if (!hotSeat)
	{
		addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Game_Type") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
		gameTypeRadioGroup = addChild (std::make_unique<cRadioGroup> ());
		gameTypeTurnsCheckBox = gameTypeRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Type_Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
		gameTypeSimultaneousCheckBox = gameTypeRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Type_Simu"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	}
	else
	{
		gameTypeRadioGroup = nullptr;
		gameTypeTurnsCheckBox = nullptr;
		gameTypeSimultaneousCheckBox = nullptr;
	}
	currentLine += lineHeight * 3;

	//
	// Clans
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Clans") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	clansRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	clansOnCheckBox = clansRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240, currentLine), lngPack.i18n ("Text~Option~On"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	clansOffCheckBox = clansRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (240 + 64, currentLine), lngPack.i18n ("Text~Option~Off"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight * 3;

	auto savedLine = currentLine;

	//
	// Credits
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (64, currentLine), menuPosition + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Credits_start") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	currentLine += lineHeight;
	creditsRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	
	creditsLowestCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (140, currentLine), lngPack.i18n ("Text~Option~None") + " (" + iToStr (SETTING_CREDITS_LOWEST) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsLowerCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Lower") + " (" + iToStr (SETTING_CREDITS_LOWER) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsLowCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Low") + " (" + iToStr (SETTING_CREDITS_LOW) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsNormalCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Normal") + " (" + iToStr (SETTING_CREDITS_NORMAL) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsMuchCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Much") + " (" + iToStr (SETTING_CREDITS_MUCH) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsMoreCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (140, currentLine), lngPack.i18n ("Text~Option~More") + " (" + iToStr (SETTING_CREDITS_MORE) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	//
	// Victory conditions
	//
	currentLine = savedLine;
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (300, currentLine), menuPosition + cPosition (400, currentLine + 10)), lngPack.i18n ("Text~Comp~GameEndsAt"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto victoryRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	currentLine += lineHeight;

	savedLine = currentLine;

	victoryTurns100CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (380, currentLine), "100 " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight; 
	victoryTurns200CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (380, currentLine), "200 " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight; 
	victoryTurns400CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (380, currentLine), "400 " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	currentLine = savedLine;

	victoryPoints100CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (500, currentLine), "100 " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryPoints200CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (500, currentLine), "200 " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryPoints400CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (500, currentLine), "400 " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	victoryNoLimitCheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (menuPosition + cPosition (440, currentLine), lngPack.i18n ("Text~Comp~NoLimit"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	//
	// Buttons
	//
	auto okButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Button~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowGameSettings::okClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Button~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowGameSettings::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowGameSettings::~cWindowGameSettings ()
{}

//------------------------------------------------------------------------------
void cWindowGameSettings::applySettings (const cGameSettings& gameSettings)
{
	switch (gameSettings.getMetalAmount ())
	{
	case eGameSettingsResourceAmount::Low:
		metalLowCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceAmount::Normal:
		metalNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::Much:
		metalMuchCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::Most:
		metalMostCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getOilAmount ())
	{
	case eGameSettingsResourceAmount::Low:
		oilLowCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceAmount::Normal:
		oilNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::Much:
		oilMuchCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::Most:
		oilMostCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getGoldAmount ())
	{
	case eGameSettingsResourceAmount::Low:
		goldLowCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceAmount::Normal:
		goldNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::Much:
		goldMuchCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::Most:
		goldMostCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getResourceDensity ())
	{
	case eGameSettingsResourceDensity::Thin:
		densityThinCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceDensity::Normal:
		densityNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceDensity::Thick:
		densityThickCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceDensity::Most:
		densityMostCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getBridgeheadType ())
	{
	case eGameSettingsBridgeheadType::Mobile:
		bridgeheadMobileCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsBridgeheadType::Definite:
		bridgeheadDefiniteCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getGameType ())
	{
	case eGameSettingsGameType::Turns:
		if (gameTypeTurnsCheckBox != nullptr) gameTypeTurnsCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsGameType::Simultaneous:
		if (gameTypeSimultaneousCheckBox != nullptr) gameTypeSimultaneousCheckBox->setChecked (true);
		break;
	}

	if (gameSettings.getClansEnabled ()) clansOnCheckBox->setChecked (true);
	else clansOffCheckBox->setChecked (true);

	if (gameSettings.getStartCredits () < SETTING_CREDITS_LOWER) creditsLowestCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < SETTING_CREDITS_LOW) creditsLowerCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < SETTING_CREDITS_NORMAL) creditsLowCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < SETTING_CREDITS_MUCH) creditsNormalCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < SETTING_CREDITS_MORE) creditsMuchCheckBox->setChecked (true);
	else creditsMoreCheckBox->setChecked (true);

	switch (gameSettings.getVictoryCondition ())
	{
	case eGameSettingsVictoryCondition::Turns:
		if (gameSettings.getVictoryTurns () < 200) victoryTurns100CheckBox->setChecked (true);
		else if (gameSettings.getVictoryTurns () < 400) victoryTurns200CheckBox->setChecked (true);
		else victoryTurns400CheckBox->setChecked (true);
		break;
	case eGameSettingsVictoryCondition::Points:
		if (gameSettings.getVictoryPoints () < 200) victoryPoints100CheckBox->setChecked (true);
		else if (gameSettings.getVictoryPoints () < 400) victoryPoints200CheckBox->setChecked (true);
		else victoryPoints400CheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsVictoryCondition::Death:
		victoryNoLimitCheckBox->setChecked (true);
		break;
	}
}

//------------------------------------------------------------------------------
cGameSettings cWindowGameSettings::getGameSettings () const
{
	cGameSettings gameSettings;

	if (metalLowCheckBox->isChecked ()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::Low);
	else if (metalMuchCheckBox->isChecked ()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::Much);
	else if (metalMostCheckBox->isChecked ()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::Most);
	else gameSettings.setMetalAmount (eGameSettingsResourceAmount::Normal);

	if (oilLowCheckBox->isChecked ()) gameSettings.setOilAmount (eGameSettingsResourceAmount::Low);
	else if (oilMuchCheckBox->isChecked ()) gameSettings.setOilAmount (eGameSettingsResourceAmount::Much);
	else if (oilMostCheckBox->isChecked ()) gameSettings.setOilAmount (eGameSettingsResourceAmount::Most);
	else gameSettings.setOilAmount (eGameSettingsResourceAmount::Normal);

	if (goldLowCheckBox->isChecked ()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::Low);
	else if (goldMuchCheckBox->isChecked ()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::Much);
	else if (goldMostCheckBox->isChecked ()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::Most);
	else gameSettings.setGoldAmount (eGameSettingsResourceAmount::Normal);

	if (densityThinCheckBox->isChecked ()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Thin);
	else if (densityThickCheckBox->isChecked ()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Thick);
	else if (densityMostCheckBox->isChecked ()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Most);
	else gameSettings.setResourceDensity (eGameSettingsResourceDensity::Normal);

	if (bridgeheadMobileCheckBox->isChecked ()) gameSettings.setBridgeheadType (eGameSettingsBridgeheadType::Mobile);
	else gameSettings.setBridgeheadType (eGameSettingsBridgeheadType::Definite);

	if (gameTypeTurnsCheckBox == nullptr || gameTypeSimultaneousCheckBox == nullptr) gameSettings.setGameType (eGameSettingsGameType::HotSeat);
	if (gameTypeTurnsCheckBox->isChecked ()) gameSettings.setGameType (eGameSettingsGameType::Turns);
	else gameSettings.setGameType (eGameSettingsGameType::Simultaneous);

	gameSettings.setClansEnabled (clansOnCheckBox->isChecked ());

	if (creditsLowestCheckBox->isChecked ()) gameSettings.setStartCredits (SETTING_CREDITS_LOWEST);
	else if (creditsLowerCheckBox->isChecked ()) gameSettings.setStartCredits (SETTING_CREDITS_LOWER);
	else if (creditsLowCheckBox->isChecked ()) gameSettings.setStartCredits (SETTING_CREDITS_LOW);
	else if (creditsMuchCheckBox->isChecked ()) gameSettings.setStartCredits (SETTING_CREDITS_MUCH);
	else if (creditsMoreCheckBox->isChecked ()) gameSettings.setStartCredits (SETTING_CREDITS_MORE);
	else gameSettings.setStartCredits (SETTING_CREDITS_NORMAL);

	if (victoryTurns100CheckBox->isChecked () || victoryTurns100CheckBox->isChecked () || victoryTurns100CheckBox->isChecked ())
	{
		gameSettings.setVictoryCondition (eGameSettingsVictoryCondition::Turns);
		if (victoryTurns100CheckBox->isChecked ()) gameSettings.setVictoryTurns (100);
		else if (victoryTurns200CheckBox->isChecked ()) gameSettings.setVictoryTurns (200);
		else gameSettings.setVictoryTurns (400);
	}
	else if (victoryPoints100CheckBox->isChecked () || victoryPoints200CheckBox->isChecked () || victoryPoints400CheckBox->isChecked ())
	{
		gameSettings.setVictoryCondition (eGameSettingsVictoryCondition::Points);
		if (victoryPoints100CheckBox->isChecked ()) gameSettings.setVictoryPoints (100);
		else if (victoryPoints200CheckBox->isChecked ()) gameSettings.setVictoryPoints (200);
		else gameSettings.setVictoryPoints (400);
	}
	else gameSettings.setVictoryCondition (eGameSettingsVictoryCondition::Death);

	return gameSettings;
}

//------------------------------------------------------------------------------
void cWindowGameSettings::okClicked ()
{
	done ();
}

//------------------------------------------------------------------------------
void cWindowGameSettings::backClicked ()
{
	close ();
}
