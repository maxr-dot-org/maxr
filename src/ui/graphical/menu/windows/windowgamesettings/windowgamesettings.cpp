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

#include "game/data/gamesettings.h"
#include "resources/pcx.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/editablecheckbox.h"
#include "ui/graphical/menu/widgets/special/radiogroupvalue.h"
#include "ui/graphical/menu/widgets/tools/validatorint.h"
#include "utility/language.h"
#include "utility/string/toString.h"

namespace
{
	constexpr int unlimited = -2;
	constexpr int custom = -1;
}
//------------------------------------------------------------------------------
cWindowGameSettings::cWindowGameSettings (bool forHotSeatGame_) :
	cWindow (LoadPCX (GFXOD_OPTIONS)),
	forHotSeatGame (forHotSeatGame_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Text~Others~Game_Options"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	int currentLine = 57;
	const int lineHeight = 16;

	// Resources
	auto addRessourceRadioGroup = [&] (const std::string& resourceName) {
		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), resourceName + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
		auto* radioGroup = addChild (std::make_unique<cRadioGroupValue<eGameSettingsResourceAmount>>());
		radioGroup->emplaceCheckBox (eGameSettingsResourceAmount::Limited, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
		radioGroup->emplaceCheckBox (eGameSettingsResourceAmount::Normal, getPosition() + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
		radioGroup->emplaceCheckBox (eGameSettingsResourceAmount::High, getPosition() + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
		radioGroup->emplaceCheckBox (eGameSettingsResourceAmount::TooMuch, getPosition() + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
		currentLine += lineHeight;
		return radioGroup;
	};

	metalGroup = addRessourceRadioGroup (lngPack.i18n ("Text~Title~Metal"));
	oilGroup = addRessourceRadioGroup (lngPack.i18n ("Text~Title~Oil"));
	goldGroup = addRessourceRadioGroup (lngPack.i18n ("Text~Title~Gold"));

	// Density
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Resource_Density") + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	densityGroup = addChild (std::make_unique<cRadioGroupValue<eGameSettingsResourceDensity>>());

	densityGroup->emplaceCheckBox (eGameSettingsResourceDensity::Sparse, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Sparse"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	densityGroup->emplaceCheckBox (eGameSettingsResourceDensity::Normal, getPosition() + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	densityGroup->emplaceCheckBox (eGameSettingsResourceDensity::Dense, getPosition() + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Dense"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	densityGroup->emplaceCheckBox (eGameSettingsResourceDensity::TooMuch, getPosition() + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	currentLine += lineHeight * 2;

	// Bridgehead
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~BridgeHead") + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	bridgeheadGroup = addChild (std::make_unique<cRadioGroupValue<eGameSettingsBridgeheadType>>());
	bridgeheadGroup->emplaceCheckBox (eGameSettingsBridgeheadType::Mobile, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Mobile"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	bridgeheadGroup->emplaceCheckBox (eGameSettingsBridgeheadType::Definite, getPosition() + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Definite"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	currentLine += lineHeight;

	//
	// Game type
	//
	if (!forHotSeatGame)
	{
		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Game_Type") + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	}
	gameTypeGroup = addChild (std::make_unique<cRadioGroupValue<eGameSettingsGameType>>());
	auto* gameTypeTurnsCheckBox = gameTypeGroup->emplaceCheckBox (eGameSettingsGameType::Turns, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Type_Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	gameTypeGroup->emplaceCheckBox (eGameSettingsGameType::Simultaneous, getPosition() + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Type_Simu"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	signalConnectionManager.connect (gameTypeTurnsCheckBox->toggled, [this, gameTypeTurnsCheckBox]()
	{
		if (gameTypeTurnsCheckBox->isChecked()) disableTurnEndDeadlineOptions();
		else enableTurnEndDeadlineOptions();
	});
	if (forHotSeatGame)
	{
		gameTypeGroup->disable();
		gameTypeGroup->hide();
	}
	currentLine += lineHeight * 2;

	// Clans
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Clans") + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	clansGroup = addChild (std::make_unique<cRadioGroupValue<bool>>());
	clansGroup->emplaceCheckBox (true, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~On"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	clansGroup->emplaceCheckBox (false, getPosition() + cPosition (240 + 64, currentLine), lngPack.i18n ("Text~Option~Off"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	currentLine += lineHeight * 2;

	auto savedLine = currentLine;

	// Credits
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Credits_start") + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	currentLine += lineHeight;
	creditsGroup = addChild (std::make_unique<cRadioGroupValue<int>>());

	creditsGroup->emplaceCheckBox (cGameSettings::defaultCreditsNone, getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~None") + " (" + iToStr (cGameSettings::defaultCreditsNone) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight;
	creditsGroup->emplaceCheckBox (cGameSettings::defaultCreditsLow, getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Low") + " (" + iToStr (cGameSettings::defaultCreditsLow) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight;
	creditsGroup->emplaceCheckBox (cGameSettings::defaultCreditsLimited, getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Limited") + " (" + iToStr (cGameSettings::defaultCreditsLimited) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight;
	creditsGroup->emplaceCheckBox (cGameSettings::defaultCreditsNormal, getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Normal") + " (" + iToStr (cGameSettings::defaultCreditsNormal) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight;
	creditsGroup->emplaceCheckBox (cGameSettings::defaultCreditsHigh, getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~High") + " (" + iToStr (cGameSettings::defaultCreditsHigh) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight;
	creditsGroup->emplaceCheckBox (cGameSettings::defaultCreditsMore, getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~More") + " (" + iToStr (cGameSettings::defaultCreditsMore) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight * 2;

	auto savedLine2 = currentLine;

	//
	// Victory conditions
	//
	currentLine = savedLine;
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (300, currentLine), getPosition() + cPosition (400, currentLine + 10)), lngPack.i18n ("Text~Comp~GameEndsAt") + lngPack.i18n("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	victoryGroup = addChild (std::make_unique<cRadioGroupValue<std::pair<eGameSettingsVictoryCondition, int>>>());
	currentLine += lineHeight;

	savedLine = currentLine;
	for (auto turn : cGameSettings::defaultVictoryTurnsOptions)
	{
		victoryGroup->emplaceCheckBox ({eGameSettingsVictoryCondition::Turns, turn}, getPosition() + cPosition (380, currentLine), iToStr (turn) + " " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
		currentLine += lineHeight;
	}
	const auto customTurnArea = cBox<cPosition>(getPosition() + cPosition (330, currentLine), getPosition() + cPosition (420, currentLine + 10));
	customVictoryTurnsCheckBox = victoryGroup->addCheckBox ({eGameSettingsVictoryCondition::Turns, custom}, std::make_unique<cEditableCheckBox>(customTurnArea, lngPack.i18n ("Text~Comp~Turns") + lngPack.i18n ("Text~Punctuation~Colon"), "", FONT_LATIN_NORMAL));
	customVictoryTurnsCheckBox->setText ("420");
	customVictoryTurnsCheckBox->setValidator (std::make_unique<cValidatorInt> (0, 9999));

	currentLine = savedLine;
	for (auto point : cGameSettings::defaultVictoryPointsOptions)
	{
		victoryGroup->emplaceCheckBox ({eGameSettingsVictoryCondition::Points, point}, getPosition() + cPosition (500, currentLine), iToStr (point) + " " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
		currentLine += lineHeight;
	}
	const auto customPointArea = cBox<cPosition>(getPosition() + cPosition (440, currentLine), getPosition() + cPosition (540, currentLine + 10));
	customVictoryPointsCheckBox = victoryGroup->addCheckBox ({eGameSettingsVictoryCondition::Points, custom}, std::make_unique<cEditableCheckBox>(customPointArea, lngPack.i18n ("Text~Comp~Points") + lngPack.i18n ("Text~Punctuation~Colon"), "", FONT_LATIN_NORMAL));
	customVictoryPointsCheckBox->setText ("900");
	customVictoryPointsCheckBox->setValidator (std::make_unique<cValidatorInt> (0, 9999));
	currentLine += lineHeight;

	victoryGroup->emplaceCheckBox ({eGameSettingsVictoryCondition::Death, 0}, getPosition() + cPosition (440, currentLine), lngPack.i18n ("Text~Comp~NoLimit"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true);
	currentLine += lineHeight;

	// Turn Limit
	currentLine = savedLine2;
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Turn_limit") + lngPack.i18n ("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	turnLimitGroup = addChild (std::make_unique<cRadioGroupValue<int>>());
	turnLimitGroup->emplaceCheckBox (unlimited, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Settings~Unlimited_11"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	turnLimitGroup->emplaceCheckBox (cGameSettings::defaultTurnLimitOption0.count(), getPosition() + cPosition (240 + 85         , currentLine), toString (cGameSettings::defaultTurnLimitOption0.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	turnLimitGroup->emplaceCheckBox (cGameSettings::defaultTurnLimitOption1.count(), getPosition() + cPosition (240 + 85 + 40    , currentLine), toString (cGameSettings::defaultTurnLimitOption1.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	turnLimitGroup->emplaceCheckBox (cGameSettings::defaultTurnLimitOption2.count(), getPosition() + cPosition (240 + 85 + 40 * 2, currentLine), toString (cGameSettings::defaultTurnLimitOption2.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	turnLimitGroup->emplaceCheckBox (cGameSettings::defaultTurnLimitOption3.count(), getPosition() + cPosition (240 + 85 + 40 * 3, currentLine), toString (cGameSettings::defaultTurnLimitOption3.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	turnLimitGroup->emplaceCheckBox (cGameSettings::defaultTurnLimitOption4.count(), getPosition() + cPosition (240 + 85 + 40 * 4, currentLine), toString (cGameSettings::defaultTurnLimitOption4.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	turnLimitGroup->emplaceCheckBox (cGameSettings::defaultTurnLimitOption5.count(), getPosition() + cPosition (240 + 85 + 40 * 5, currentLine), toString (cGameSettings::defaultTurnLimitOption5.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	currentLine += lineHeight;

	customTurnLimitCheckBox = turnLimitGroup->addCheckBox (custom, std::make_unique<cEditableCheckBox> (cBox<cPosition> (getPosition() + cPosition (240, currentLine), cPosition (240 + 85 + 30, currentLine + 10)), lngPack.i18n ("Text~Title~Custom_11") + lngPack.i18n ("Text~Punctuation~Colon"), "s", FONT_LATIN_NORMAL));
	customTurnLimitCheckBox->setText ("410");
	customTurnLimitCheckBox->setValidator (std::make_unique<cValidatorInt> (0, 9999));

	currentLine += lineHeight;

	// Turn End Deadline
	turnEndDeadlineLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)),  lngPack.i18n ("Text~Title~Turn_end") + lngPack.i18n ("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	endTurnDeadlineGroup = addChild (std::make_unique<cRadioGroupValue<int>>());
	endTurnDeadlineGroup->emplaceCheckBox (unlimited, getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Settings~Unlimited_11"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	endTurnDeadlineGroup->emplaceCheckBox (cGameSettings::defaultEndTurnDeadlineOption0.count(), getPosition() + cPosition (240 + 85         , currentLine), toString (cGameSettings::defaultEndTurnDeadlineOption0.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	endTurnDeadlineGroup->emplaceCheckBox (cGameSettings::defaultEndTurnDeadlineOption1.count(), getPosition() + cPosition (240 + 85 + 40    , currentLine), toString (cGameSettings::defaultEndTurnDeadlineOption1.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	endTurnDeadlineGroup->emplaceCheckBox (cGameSettings::defaultEndTurnDeadlineOption2.count(), getPosition() + cPosition (240 + 85 + 40 * 2, currentLine), toString (cGameSettings::defaultEndTurnDeadlineOption2.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	endTurnDeadlineGroup->emplaceCheckBox (cGameSettings::defaultEndTurnDeadlineOption3.count(), getPosition() + cPosition (240 + 85 + 40 * 3, currentLine), toString (cGameSettings::defaultEndTurnDeadlineOption3.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	endTurnDeadlineGroup->emplaceCheckBox (cGameSettings::defaultEndTurnDeadlineOption4.count(), getPosition() + cPosition (240 + 85 + 40 * 4, currentLine), toString (cGameSettings::defaultEndTurnDeadlineOption4.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	endTurnDeadlineGroup->emplaceCheckBox (cGameSettings::defaultEndTurnDeadlineOption5.count(), getPosition() + cPosition (240 + 85 + 40 * 5, currentLine), toString (cGameSettings::defaultEndTurnDeadlineOption5.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly);
	currentLine += lineHeight;

	customEndTurnDeadlineCheckBox = endTurnDeadlineGroup->addCheckBox (custom, std::make_unique<cEditableCheckBox> (cBox<cPosition> (getPosition() + cPosition (240, currentLine), cPosition (240 + 85 + 30, currentLine + 10)), lngPack.i18n ("Text~Title~Custom_11") + lngPack.i18n ("Text~Punctuation~Colon"), "s", FONT_LATIN_NORMAL));
	customEndTurnDeadlineCheckBox->setText ("105");
	customEndTurnDeadlineCheckBox->setValidator (std::make_unique<cValidatorInt> (0, 9999));

	currentLine += lineHeight;

	//
	// Buttons
	//
	auto okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowGameSettings::okClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowGameSettings::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowGameSettings::~cWindowGameSettings()
{}

//------------------------------------------------------------------------------
void cWindowGameSettings::applySettings (const cGameSettings& gameSettings)
{
	metalGroup->selectValue (gameSettings.getMetalAmount());
	oilGroup->selectValue (gameSettings.getOilAmount());
	goldGroup->selectValue (gameSettings.getGoldAmount());

	densityGroup->selectValue (gameSettings.getResourceDensity());

	bridgeheadGroup->selectValue (gameSettings.getBridgeheadType());

	gameTypeGroup->selectValue (forHotSeatGame ? eGameSettingsGameType::Turns : gameSettings.getGameType());
	if (gameTypeGroup->getSelectedValue().second == eGameSettingsGameType::Simultaneous) enableTurnEndDeadlineOptions();
	else disableTurnEndDeadlineOptions();

	clansGroup->selectValue (gameSettings.getClansEnabled());

	creditsGroup->selectValue (gameSettings.getStartCredits());

	switch (gameSettings.getVictoryCondition())
	{
		case eGameSettingsVictoryCondition::Turns:
			if (!victoryGroup->selectValue({eGameSettingsVictoryCondition::Turns, gameSettings.getVictoryTurns()}))
			{
				customVictoryTurnsCheckBox->setText (std::to_string (gameSettings.getVictoryTurns()));
				victoryGroup->selectValue({eGameSettingsVictoryCondition::Turns, custom});
			}
			break;
		case eGameSettingsVictoryCondition::Points:
			if (!victoryGroup->selectValue({eGameSettingsVictoryCondition::Points, gameSettings.getVictoryPoints()}))
			{
				customVictoryPointsCheckBox->setText (std::to_string (gameSettings.getVictoryPoints()));
				victoryGroup->selectValue({eGameSettingsVictoryCondition::Points, custom});
			}
			break;
		case eGameSettingsVictoryCondition::Death:
			victoryGroup->selectValue({eGameSettingsVictoryCondition::Death, 0});
			break;
	}

	if (gameSettings.isTurnLimitActive())
	{
		if (!turnLimitGroup->selectValue (gameSettings.getTurnLimit().count()))
		{
			turnLimitGroup->selectValue (custom);
			customTurnLimitCheckBox->setText (toString (gameSettings.getTurnLimit().count()));
		}
	}
	else
	{
		turnLimitGroup->selectValue (unlimited);
	}

	if (gameSettings.isTurnEndDeadlineActive())
	{
		if (!endTurnDeadlineGroup->selectValue (gameSettings.getTurnEndDeadline().count()))
		{
			endTurnDeadlineGroup->selectValue (custom);
			customEndTurnDeadlineCheckBox->setText (toString (gameSettings.getTurnEndDeadline().count()));
		}
	}
	else
	{
		endTurnDeadlineGroup->selectValue (unlimited);
	}
}

//------------------------------------------------------------------------------
cGameSettings cWindowGameSettings::getGameSettings() const
{
	cGameSettings gameSettings;

	gameSettings.setMetalAmount (metalGroup->getSelectedValue().second);
	gameSettings.setOilAmount (oilGroup->getSelectedValue().second);
	gameSettings.setGoldAmount (goldGroup->getSelectedValue().second);
	gameSettings.setResourceDensity (densityGroup->getSelectedValue().second);

	gameSettings.setBridgeheadType (bridgeheadGroup->getSelectedValue().second);

	if (forHotSeatGame) gameSettings.setGameType (eGameSettingsGameType::HotSeat);
	else gameSettings.setGameType (gameTypeGroup->getSelectedValue().second);

	gameSettings.setClansEnabled (clansGroup->getSelectedValue().second);

	gameSettings.setStartCredits (creditsGroup->getSelectedValue().second);

	const auto victoryCondition = victoryGroup->getSelectedValue().second;
	const auto victoryType = victoryCondition.first;
	const auto victoryCount = victoryCondition.second;
	gameSettings.setVictoryCondition (victoryType);
	switch (victoryType)
	{
		case eGameSettingsVictoryCondition::Points:
		{
			gameSettings.setVictoryPoints (victoryCount == custom ? stoi (customVictoryPointsCheckBox->getText()): victoryCount);
			break;
		}
		case eGameSettingsVictoryCondition::Turns:
		{
			gameSettings.setVictoryTurns (victoryCount == custom ? stoi (customVictoryTurnsCheckBox->getText()) : victoryCount);
			break;
		}
		case eGameSettingsVictoryCondition::Death: break;
	}

	gameSettings.setTurnLimitActive (true);
	switch (turnLimitGroup->getSelectedValue().second)
	{
		case custom: gameSettings.setTurnLimit (std::chrono::seconds (atoi (customTurnLimitCheckBox->getText().c_str()))); break;
		case unlimited: gameSettings.setTurnLimitActive (false); break;
		default: gameSettings.setTurnLimit (std::chrono::seconds (turnLimitGroup->getSelectedValue().second)); break;
	}

	gameSettings.setTurnEndDeadlineActive (true);
	switch (endTurnDeadlineGroup->getSelectedValue().second)
	{
		case custom: gameSettings.setTurnEndDeadline (std::chrono::seconds (atoi (customEndTurnDeadlineCheckBox->getText().c_str()))); break;
		case unlimited: gameSettings.setTurnEndDeadlineActive (false); break;
		default: gameSettings.setTurnEndDeadline (std::chrono::seconds (endTurnDeadlineGroup->getSelectedValue().second)); break;
	}

	return gameSettings;
}

//------------------------------------------------------------------------------
void cWindowGameSettings::okClicked()
{
	done();
}

//------------------------------------------------------------------------------
void cWindowGameSettings::backClicked()
{
	close();
}

//------------------------------------------------------------------------------
void cWindowGameSettings::disableTurnEndDeadlineOptions()
{
	endTurnDeadlineGroup->disable();
	endTurnDeadlineGroup->hide();

	turnEndDeadlineLabel->hide();
}

//------------------------------------------------------------------------------
void cWindowGameSettings::enableTurnEndDeadlineOptions()
{
	if (forHotSeatGame) { return; }

	endTurnDeadlineGroup->enable();
	endTurnDeadlineGroup->show();

	turnEndDeadlineLabel->show();
}
