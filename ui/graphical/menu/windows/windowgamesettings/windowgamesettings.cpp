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

#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "main.h"
#include "pcx.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/radiogroup.h"

//------------------------------------------------------------------------------
cWindowGameSettings::cWindowGameSettings () :
	cWindow (LoadPCX (GFXOD_OPTIONS))
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 13), getPosition () + cPosition (getArea ().getMaxCorner ().x (), 23)), lngPack.i18n ("Text~Others~Game_Options"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	int currentLine = 57;
	const int lineHeight = 16;
	
	//
	// Resources
	//

	// Metal
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Metal") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto metalRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	metalLimitedCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalNormalCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalHighCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalTooMuchCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Oil
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Oil") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto oilRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	oilLimitedCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilNormalCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilHighCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilTooMuchCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Gold
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Gold") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto goldRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	goldLimitedCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldNormalCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldHighCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldTooMuchCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Density
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Resource_Density") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto densityRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	densitySparseCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Sparse"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityNormalCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityDenseCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Dense"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityTooMuchCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight * 3;

	//
	// Bridgehead
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~BridgeHead") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto bridgeheadRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	bridgeheadMobileCheckBox = bridgeheadRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Mobile"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	bridgeheadDefiniteCheckBox = bridgeheadRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Definite"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	//
	// Game type
	//
	const bool hotSeat = false;
	if (!hotSeat)
	{
		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Game_Type") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
		auto gameTypeRadioGroup = addChild (std::make_unique<cRadioGroup> ());
		gameTypeTurnsCheckBox = gameTypeRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Type_Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
		gameTypeSimultaneousCheckBox = gameTypeRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Type_Simu"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	}
	else
	{
		gameTypeTurnsCheckBox = nullptr;
		gameTypeSimultaneousCheckBox = nullptr;
	}
	currentLine += lineHeight * 3;

	//
	// Clans
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Clans") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto clansRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	clansOnCheckBox = clansRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240, currentLine), lngPack.i18n ("Text~Option~On"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	clansOffCheckBox = clansRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (240 + 64, currentLine), lngPack.i18n ("Text~Option~Off"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight * 3;

	auto savedLine = currentLine;

	//
	// Credits
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (64, currentLine), getPosition () + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Credits_start") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	currentLine += lineHeight;
	auto creditsRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	
	creditsNoneCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (140, currentLine), lngPack.i18n ("Text~Option~None") + " (" + iToStr (cGameSettings::defaultCreditsNone) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsLowCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Low") + " (" + iToStr (cGameSettings::defaultCreditsLow) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsLimitedCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Limited") + " (" + iToStr (cGameSettings::defaultCreditsLimited) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsNormalCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Normal") + " (" + iToStr (cGameSettings::defaultCreditsNormal) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsHighCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (140, currentLine), lngPack.i18n ("Text~Option~High") + " (" + iToStr (cGameSettings::defaultCreditsHigh) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsMoreCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (140, currentLine), lngPack.i18n ("Text~Option~More") + " (" + iToStr (cGameSettings::defaultCreditsMore) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	//
	// Victory conditions
	//
	currentLine = savedLine;
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (300, currentLine), getPosition () + cPosition (400, currentLine + 10)), lngPack.i18n ("Text~Comp~GameEndsAt"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto victoryRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	currentLine += lineHeight;

	savedLine = currentLine;

	victoryTurns100CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (380, currentLine), "100 " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight; 
	victoryTurns200CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (380, currentLine), "200 " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight; 
	victoryTurns400CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (380, currentLine), "400 " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	currentLine = savedLine;

	victoryPoints100CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (500, currentLine), "100 " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryPoints200CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (500, currentLine), "200 " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryPoints400CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (500, currentLine), "400 " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	victoryNoLimitCheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (440, currentLine), lngPack.i18n ("Text~Comp~NoLimit"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	//
	// Buttons
	//
	auto okButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowGameSettings::okClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
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
	case eGameSettingsResourceAmount::Limited:
		metalLimitedCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceAmount::Normal:
		metalNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::High:
		metalHighCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::TooMuch:
		metalTooMuchCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getOilAmount ())
	{
	case eGameSettingsResourceAmount::Limited:
		oilLimitedCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceAmount::Normal:
		oilNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::High:
		oilHighCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::TooMuch:
		oilTooMuchCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getGoldAmount ())
	{
	case eGameSettingsResourceAmount::Limited:
		goldLimitedCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceAmount::Normal:
		goldNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::High:
		goldHighCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceAmount::TooMuch:
		goldTooMuchCheckBox->setChecked (true);
		break;
	}

	switch (gameSettings.getResourceDensity ())
	{
	case eGameSettingsResourceDensity::Sparse:
		densitySparseCheckBox->setChecked (true);
		break;
	default:
	case eGameSettingsResourceDensity::Normal:
		densityNormalCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceDensity::Dense:
		densityDenseCheckBox->setChecked (true);
		break;
	case eGameSettingsResourceDensity::TooMuch:
		densityTooMuchCheckBox->setChecked (true);
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

	if (gameSettings.getStartCredits () < cGameSettings::defaultCreditsLow) creditsNoneCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < cGameSettings::defaultCreditsLimited) creditsLowCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < cGameSettings::defaultCreditsNormal) creditsLimitedCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < cGameSettings::defaultCreditsHigh) creditsNormalCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits () < cGameSettings::defaultCreditsMore) creditsHighCheckBox->setChecked (true);
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

	if (metalLimitedCheckBox->isChecked ()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::Limited);
	else if (metalHighCheckBox->isChecked ()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::High);
	else if (metalTooMuchCheckBox->isChecked ()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::TooMuch);
	else gameSettings.setMetalAmount (eGameSettingsResourceAmount::Normal);

	if (oilLimitedCheckBox->isChecked ()) gameSettings.setOilAmount (eGameSettingsResourceAmount::Limited);
	else if (oilHighCheckBox->isChecked ()) gameSettings.setOilAmount (eGameSettingsResourceAmount::High);
	else if (oilTooMuchCheckBox->isChecked ()) gameSettings.setOilAmount (eGameSettingsResourceAmount::TooMuch);
	else gameSettings.setOilAmount (eGameSettingsResourceAmount::Normal);

	if (goldLimitedCheckBox->isChecked ()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::Limited);
	else if (goldHighCheckBox->isChecked ()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::High);
	else if (goldTooMuchCheckBox->isChecked ()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::TooMuch);
	else gameSettings.setGoldAmount (eGameSettingsResourceAmount::Normal);

	if (densitySparseCheckBox->isChecked ()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Sparse);
	else if (densityDenseCheckBox->isChecked ()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Dense);
	else if (densityTooMuchCheckBox->isChecked ()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::TooMuch);
	else gameSettings.setResourceDensity (eGameSettingsResourceDensity::Normal);

	if (bridgeheadMobileCheckBox->isChecked ()) gameSettings.setBridgeheadType (eGameSettingsBridgeheadType::Mobile);
	else gameSettings.setBridgeheadType (eGameSettingsBridgeheadType::Definite);

	if (gameTypeTurnsCheckBox == nullptr || gameTypeSimultaneousCheckBox == nullptr) gameSettings.setGameType (eGameSettingsGameType::HotSeat);
	if (gameTypeTurnsCheckBox->isChecked ()) gameSettings.setGameType (eGameSettingsGameType::Turns);
	else gameSettings.setGameType (eGameSettingsGameType::Simultaneous);

	gameSettings.setClansEnabled (clansOnCheckBox->isChecked ());

	if (creditsNoneCheckBox->isChecked ()) gameSettings.setStartCredits (cGameSettings::defaultCreditsNone);
	else if (creditsLowCheckBox->isChecked ()) gameSettings.setStartCredits (cGameSettings::defaultCreditsLow);
	else if (creditsLimitedCheckBox->isChecked ()) gameSettings.setStartCredits (cGameSettings::defaultCreditsLimited);
	else if (creditsHighCheckBox->isChecked ()) gameSettings.setStartCredits (cGameSettings::defaultCreditsHigh);
	else if (creditsMoreCheckBox->isChecked ()) gameSettings.setStartCredits (cGameSettings::defaultCreditsMore);
	else gameSettings.setStartCredits (cGameSettings::defaultCreditsNormal);

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
