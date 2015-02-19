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
#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/tools/validatorint.h"

//------------------------------------------------------------------------------
cWindowGameSettings::cWindowGameSettings (bool forHotSeatGame_) :
	cWindow (LoadPCX (GFXOD_OPTIONS)),
	forHotSeatGame (forHotSeatGame_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Text~Others~Game_Options"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	int currentLine = 57;
	const int lineHeight = 16;

	//
	// Resources
	//

	// Metal
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Metal") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto metalRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	metalLimitedCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalNormalCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalHighCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	metalTooMuchCheckBox = metalRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Oil
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Oil") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto oilRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	oilLimitedCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilNormalCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilHighCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	oilTooMuchCheckBox = oilRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Gold
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Gold") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto goldRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	goldLimitedCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Limited"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldNormalCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldHighCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~High"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	goldTooMuchCheckBox = goldRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	// Density
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Resource_Density") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto densityRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	densitySparseCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Sparse"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityNormalCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86, currentLine), lngPack.i18n ("Text~Option~Normal"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityDenseCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 2, currentLine), lngPack.i18n ("Text~Option~Dense"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	densityTooMuchCheckBox = densityRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 86 * 3, currentLine), lngPack.i18n ("Text~Option~TooMuch"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight * 2;

	//
	// Bridgehead
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~BridgeHead") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto bridgeheadRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	bridgeheadMobileCheckBox = bridgeheadRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Mobile"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	bridgeheadDefiniteCheckBox = bridgeheadRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Definite"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;

	//
	// Game type
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Game_Type") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto gameTypeRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	gameTypeTurnsCheckBox = gameTypeRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~Type_Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	gameTypeSimultaneousCheckBox = gameTypeRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 173, currentLine), lngPack.i18n ("Text~Option~Type_Simu"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	signalConnectionManager.connect (gameTypeTurnsCheckBox->toggled, [this]()
	{
		if (gameTypeTurnsCheckBox->isChecked()) disableTurnEndDeadlineOptions();
		else enableTurnEndDeadlineOptions();
	});
	if (forHotSeatGame)
	{
		gameTypeTurnsCheckBox->disable();
		gameTypeTurnsCheckBox->hide();
		gameTypeSimultaneousCheckBox->disable();
		gameTypeSimultaneousCheckBox->hide();
	}
	currentLine += lineHeight * 2;

	//
	// Clans
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Clans") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto clansRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	clansOnCheckBox = clansRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Option~On"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	clansOffCheckBox = clansRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 64, currentLine), lngPack.i18n ("Text~Option~Off"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight * 2;

	auto savedLine = currentLine;

	//
	// Credits
	//
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Credits_start") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	currentLine += lineHeight;
	auto creditsRadioGroup = addChild (std::make_unique<cRadioGroup> ());

	creditsNoneCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~None") + " (" + iToStr (cGameSettings::defaultCreditsNone) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsLowCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Low") + " (" + iToStr (cGameSettings::defaultCreditsLow) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsLimitedCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Limited") + " (" + iToStr (cGameSettings::defaultCreditsLimited) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsNormalCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~Normal") + " (" + iToStr (cGameSettings::defaultCreditsNormal) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsHighCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~High") + " (" + iToStr (cGameSettings::defaultCreditsHigh) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	creditsMoreCheckBox = creditsRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (140, currentLine), lngPack.i18n ("Text~Option~More") + " (" + iToStr (cGameSettings::defaultCreditsMore) + ")", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight * 2;

	auto savedLine2 = currentLine;

	//
	// Victory conditions
	//
	currentLine = savedLine;
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (300, currentLine), getPosition() + cPosition (400, currentLine + 10)), lngPack.i18n ("Text~Comp~GameEndsAt") + ":", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto victoryRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	currentLine += lineHeight;

	savedLine = currentLine;

	victoryTurns0CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (380, currentLine), iToStr (cGameSettings::defaultVictoryTurnsOption0) + " " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryTurns1CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (380, currentLine), iToStr (cGameSettings::defaultVictoryTurnsOption1) + " " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryTurns2CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (380, currentLine), iToStr (cGameSettings::defaultVictoryTurnsOption2) + " " + lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	currentLine = savedLine;

	victoryPoints0CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (500, currentLine), iToStr (cGameSettings::defaultVictoryPointsOption0) + " " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryPoints1CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (500, currentLine), iToStr (cGameSettings::defaultVictoryPointsOption1) + " " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;
	victoryPoints2CheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (500, currentLine), iToStr (cGameSettings::defaultVictoryPointsOption2) + " " + lngPack.i18n ("Text~Comp~Points"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	victoryNoLimitCheckBox = victoryRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (440, currentLine), lngPack.i18n ("Text~Comp~NoLimit"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly, true));
	currentLine += lineHeight;

	// Turn Limit
	currentLine = savedLine2;
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)), lngPack.i18n ("Text~Title~Turn_limit"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto turnLimitRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	turnLimitNoLimitCheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Settings~Unlimited_11"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnLimit0CheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85, currentLine), iToStr (cGameSettings::defaultTurnLimitOption0.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnLimit1CheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40, currentLine), iToStr (cGameSettings::defaultTurnLimitOption1.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnLimit2CheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 2, currentLine), iToStr (cGameSettings::defaultTurnLimitOption2.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnLimit3CheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 3, currentLine), iToStr (cGameSettings::defaultTurnLimitOption3.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnLimit4CheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 4, currentLine), iToStr (cGameSettings::defaultTurnLimitOption4.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnLimit5CheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 5, currentLine), iToStr (cGameSettings::defaultTurnLimitOption5.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;
	turnLimitCustomCheckBox = turnLimitRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Title~Custom_11") + ": ", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	signalConnectionManager.connect (turnLimitCustomCheckBox->toggled, [this]() { if (turnLimitCustomCheckBox->isChecked()) turnLimitCustomLineEdit->enable(); else turnLimitCustomLineEdit->disable(); });
	turnLimitCustomLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (240 + 85, currentLine), getPosition() + cPosition (240 + 85 + 30, currentLine + lineHeight))));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (240 + 85 + 30, currentLine), getPosition() + cPosition (240 + 85 + 30 + 10, currentLine + 10)), "s", FONT_LATIN_NORMAL, eAlignmentType::Left));
	turnLimitCustomLineEdit->setText ("410");
	turnLimitCustomLineEdit->setValidator (std::make_unique<cValidatorInt> (0, std::numeric_limits<int>::max()));
	turnLimitCustomLineEdit->disable();
	currentLine += lineHeight;

	// Turn End Deadline
	turnEndDeadlineLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (64, currentLine), getPosition() + cPosition (230, currentLine + 10)),  lngPack.i18n ("Text~Title~Turn_end") + ": ", FONT_LATIN_NORMAL, eAlignmentType::Left));
	auto turnEndTurnDeadlineRadioGroup = addChild (std::make_unique<cRadioGroup> ());
	turnEndTurnDeadlineNoLimitCheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Settings~Unlimited_11"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnEndTurnDeadline0CheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85, currentLine), iToStr (cGameSettings::defaultEndTurnDeadlineOption0.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnEndTurnDeadline1CheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40, currentLine), iToStr (cGameSettings::defaultEndTurnDeadlineOption1.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnEndTurnDeadline2CheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 2, currentLine), iToStr (cGameSettings::defaultEndTurnDeadlineOption2.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnEndTurnDeadline3CheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 3, currentLine), iToStr (cGameSettings::defaultEndTurnDeadlineOption3.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnEndTurnDeadline4CheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 4, currentLine), iToStr (cGameSettings::defaultEndTurnDeadlineOption4.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	turnEndTurnDeadline5CheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240 + 85 + 40 * 5, currentLine), iToStr (cGameSettings::defaultEndTurnDeadlineOption5.count()) + "s", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	currentLine += lineHeight;
	turnEndTurnDeadlineCustomCheckBox = turnEndTurnDeadlineRadioGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (240, currentLine), lngPack.i18n ("Text~Title~Custom_11") + ": ", FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::TextOnly));
	signalConnectionManager.connect (turnEndTurnDeadlineCustomCheckBox->toggled, [this]() { if (turnEndTurnDeadlineCustomCheckBox->isChecked()) turnEndTurnDeadlineLineEdit->enable(); else turnEndTurnDeadlineLineEdit->disable(); });
	turnEndTurnDeadlineLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (240 + 85, currentLine), getPosition() + cPosition (240 + 85 + 30, currentLine + lineHeight))));
	turnEndDeadlineSecondsLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (240 + 85 + 30, currentLine), getPosition() + cPosition (240 + 85 + 30 + 10, currentLine + 10)), "s", FONT_LATIN_NORMAL, eAlignmentType::Left));
	turnEndTurnDeadlineLineEdit->setText ("105");
	turnEndTurnDeadlineLineEdit->setValidator (std::make_unique<cValidatorInt> (0, std::numeric_limits<int>::max()));
	turnEndTurnDeadlineLineEdit->disable();
	currentLine += lineHeight;

	if (forHotSeatGame)
	{
		disableTurnEndDeadlineOptions();
	}

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
	switch (gameSettings.getMetalAmount())
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

	switch (gameSettings.getOilAmount())
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

	switch (gameSettings.getGoldAmount())
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

	switch (gameSettings.getResourceDensity())
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

	switch (gameSettings.getBridgeheadType())
	{
		case eGameSettingsBridgeheadType::Mobile:
			bridgeheadMobileCheckBox->setChecked (true);
			break;
		default:
		case eGameSettingsBridgeheadType::Definite:
			bridgeheadDefiniteCheckBox->setChecked (true);
			break;
	}

	switch (gameSettings.getGameType())
	{
		case eGameSettingsGameType::Turns:
			gameTypeTurnsCheckBox->setChecked (true);
			break;
		default:
		case eGameSettingsGameType::Simultaneous:
			gameTypeSimultaneousCheckBox->setChecked (true);
			break;
	}

	if (gameSettings.getClansEnabled()) clansOnCheckBox->setChecked (true);
	else clansOffCheckBox->setChecked (true);

	if (gameSettings.getStartCredits() < cGameSettings::defaultCreditsLow) creditsNoneCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits() < cGameSettings::defaultCreditsLimited) creditsLowCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits() < cGameSettings::defaultCreditsNormal) creditsLimitedCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits() < cGameSettings::defaultCreditsHigh) creditsNormalCheckBox->setChecked (true);
	else if (gameSettings.getStartCredits() < cGameSettings::defaultCreditsMore) creditsHighCheckBox->setChecked (true);
	else creditsMoreCheckBox->setChecked (true);

	switch (gameSettings.getVictoryCondition())
	{
		case eGameSettingsVictoryCondition::Turns:
			if (gameSettings.getVictoryTurns() < cGameSettings::defaultVictoryTurnsOption1) victoryTurns0CheckBox->setChecked (true);
			else if (gameSettings.getVictoryTurns() < cGameSettings::defaultVictoryTurnsOption2) victoryTurns1CheckBox->setChecked (true);
			else victoryTurns2CheckBox->setChecked (true);
			break;
		case eGameSettingsVictoryCondition::Points:
			if (gameSettings.getVictoryPoints() < cGameSettings::defaultVictoryPointsOption1) victoryPoints0CheckBox->setChecked (true);
			else if (gameSettings.getVictoryPoints() < cGameSettings::defaultVictoryPointsOption2) victoryPoints1CheckBox->setChecked (true);
			else victoryPoints2CheckBox->setChecked (true);
			break;
		default:
		case eGameSettingsVictoryCondition::Death:
			victoryNoLimitCheckBox->setChecked (true);
			break;
	}

	if (gameSettings.isTurnLimitActive())
	{
		if (gameSettings.getTurnLimit() == cGameSettings::defaultTurnLimitOption0) turnLimit0CheckBox->setChecked (true);
		else if (gameSettings.getTurnLimit() == cGameSettings::defaultTurnLimitOption1) turnLimit1CheckBox->setChecked (true);
		else if (gameSettings.getTurnLimit() == cGameSettings::defaultTurnLimitOption2) turnLimit2CheckBox->setChecked (true);
		else if (gameSettings.getTurnLimit() == cGameSettings::defaultTurnLimitOption3) turnLimit3CheckBox->setChecked (true);
		else if (gameSettings.getTurnLimit() == cGameSettings::defaultTurnLimitOption4) turnLimit4CheckBox->setChecked (true);
		else if (gameSettings.getTurnLimit() == cGameSettings::defaultTurnLimitOption5) turnLimit5CheckBox->setChecked (true);
		else
		{
			turnLimitCustomCheckBox->setChecked (true);
			turnLimitCustomLineEdit->setText (iToStr (gameSettings.getTurnLimit().count()));
		}
	}
	else
	{
		turnLimitNoLimitCheckBox->setChecked (true);
	}

	if (gameSettings.isTurnEndDeadlineActive())
	{
		if (gameSettings.getTurnEndDeadline() == cGameSettings::defaultEndTurnDeadlineOption0) turnEndTurnDeadline0CheckBox->setChecked (true);
		else if (gameSettings.getTurnEndDeadline() == cGameSettings::defaultEndTurnDeadlineOption1) turnEndTurnDeadline1CheckBox->setChecked (true);
		else if (gameSettings.getTurnEndDeadline() == cGameSettings::defaultEndTurnDeadlineOption2) turnEndTurnDeadline2CheckBox->setChecked (true);
		else if (gameSettings.getTurnEndDeadline() == cGameSettings::defaultEndTurnDeadlineOption3) turnEndTurnDeadline3CheckBox->setChecked (true);
		else if (gameSettings.getTurnEndDeadline() == cGameSettings::defaultEndTurnDeadlineOption4) turnEndTurnDeadline4CheckBox->setChecked (true);
		else if (gameSettings.getTurnEndDeadline() == cGameSettings::defaultEndTurnDeadlineOption5) turnEndTurnDeadline5CheckBox->setChecked (true);
		else
		{
			turnEndTurnDeadlineCustomCheckBox->setChecked (true);
			turnEndTurnDeadlineLineEdit->setText (iToStr (gameSettings.getTurnEndDeadline().count()));
		}
	}
	else
	{
		turnEndTurnDeadlineNoLimitCheckBox->setChecked (true);
	}
}

//------------------------------------------------------------------------------
cGameSettings cWindowGameSettings::getGameSettings() const
{
	cGameSettings gameSettings;

	if (metalLimitedCheckBox->isChecked()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::Limited);
	else if (metalHighCheckBox->isChecked()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::High);
	else if (metalTooMuchCheckBox->isChecked()) gameSettings.setMetalAmount (eGameSettingsResourceAmount::TooMuch);
	else gameSettings.setMetalAmount (eGameSettingsResourceAmount::Normal);

	if (oilLimitedCheckBox->isChecked()) gameSettings.setOilAmount (eGameSettingsResourceAmount::Limited);
	else if (oilHighCheckBox->isChecked()) gameSettings.setOilAmount (eGameSettingsResourceAmount::High);
	else if (oilTooMuchCheckBox->isChecked()) gameSettings.setOilAmount (eGameSettingsResourceAmount::TooMuch);
	else gameSettings.setOilAmount (eGameSettingsResourceAmount::Normal);

	if (goldLimitedCheckBox->isChecked()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::Limited);
	else if (goldHighCheckBox->isChecked()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::High);
	else if (goldTooMuchCheckBox->isChecked()) gameSettings.setGoldAmount (eGameSettingsResourceAmount::TooMuch);
	else gameSettings.setGoldAmount (eGameSettingsResourceAmount::Normal);

	if (densitySparseCheckBox->isChecked()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Sparse);
	else if (densityDenseCheckBox->isChecked()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::Dense);
	else if (densityTooMuchCheckBox->isChecked()) gameSettings.setResourceDensity (eGameSettingsResourceDensity::TooMuch);
	else gameSettings.setResourceDensity (eGameSettingsResourceDensity::Normal);

	if (bridgeheadMobileCheckBox->isChecked()) gameSettings.setBridgeheadType (eGameSettingsBridgeheadType::Mobile);
	else gameSettings.setBridgeheadType (eGameSettingsBridgeheadType::Definite);

	if (forHotSeatGame) gameSettings.setGameType (eGameSettingsGameType::HotSeat);
	else if (gameTypeTurnsCheckBox->isChecked()) gameSettings.setGameType (eGameSettingsGameType::Turns);
	else gameSettings.setGameType (eGameSettingsGameType::Simultaneous);

	gameSettings.setClansEnabled (clansOnCheckBox->isChecked());

	if (creditsNoneCheckBox->isChecked()) gameSettings.setStartCredits (cGameSettings::defaultCreditsNone);
	else if (creditsLowCheckBox->isChecked()) gameSettings.setStartCredits (cGameSettings::defaultCreditsLow);
	else if (creditsLimitedCheckBox->isChecked()) gameSettings.setStartCredits (cGameSettings::defaultCreditsLimited);
	else if (creditsHighCheckBox->isChecked()) gameSettings.setStartCredits (cGameSettings::defaultCreditsHigh);
	else if (creditsMoreCheckBox->isChecked()) gameSettings.setStartCredits (cGameSettings::defaultCreditsMore);
	else gameSettings.setStartCredits (cGameSettings::defaultCreditsNormal);

	if (victoryTurns0CheckBox->isChecked() || victoryTurns1CheckBox->isChecked() || victoryTurns2CheckBox->isChecked())
	{
		gameSettings.setVictoryCondition (eGameSettingsVictoryCondition::Turns);
		if (victoryTurns0CheckBox->isChecked()) gameSettings.setVictoryTurns (cGameSettings::defaultVictoryTurnsOption0);
		else if (victoryTurns1CheckBox->isChecked()) gameSettings.setVictoryTurns (cGameSettings::defaultVictoryTurnsOption1);
		else gameSettings.setVictoryTurns (cGameSettings::defaultVictoryTurnsOption2);
	}
	else if (victoryPoints0CheckBox->isChecked() || victoryPoints1CheckBox->isChecked() || victoryPoints2CheckBox->isChecked())
	{
		gameSettings.setVictoryCondition (eGameSettingsVictoryCondition::Points);
		if (victoryPoints0CheckBox->isChecked()) gameSettings.setVictoryPoints (cGameSettings::defaultVictoryPointsOption0);
		else if (victoryPoints1CheckBox->isChecked()) gameSettings.setVictoryPoints (cGameSettings::defaultVictoryPointsOption1);
		else gameSettings.setVictoryPoints (cGameSettings::defaultVictoryPointsOption2);
	}
	else gameSettings.setVictoryCondition (eGameSettingsVictoryCondition::Death);

	gameSettings.setTurnLimitActive (!turnLimitNoLimitCheckBox->isChecked());

	if (turnLimit0CheckBox->isChecked()) gameSettings.setTurnLimit (cGameSettings::defaultTurnLimitOption0);
	else if (turnLimit1CheckBox->isChecked()) gameSettings.setTurnLimit (cGameSettings::defaultTurnLimitOption1);
	else if (turnLimit2CheckBox->isChecked()) gameSettings.setTurnLimit (cGameSettings::defaultTurnLimitOption2);
	else if (turnLimit3CheckBox->isChecked()) gameSettings.setTurnLimit (cGameSettings::defaultTurnLimitOption3);
	else if (turnLimit4CheckBox->isChecked()) gameSettings.setTurnLimit (cGameSettings::defaultTurnLimitOption4);
	else if (turnLimit5CheckBox->isChecked()) gameSettings.setTurnLimit (cGameSettings::defaultTurnLimitOption5);
	else if (turnLimitCustomCheckBox->isChecked()) gameSettings.setTurnLimit (std::chrono::seconds (atoi (turnLimitCustomLineEdit->getText().c_str())));

	gameSettings.setTurnEndDeadlineActive (!turnEndTurnDeadlineNoLimitCheckBox->isChecked());

	if (turnEndTurnDeadline0CheckBox->isChecked()) gameSettings.setTurnEndDeadline (cGameSettings::defaultEndTurnDeadlineOption0);
	else if (turnEndTurnDeadline1CheckBox->isChecked()) gameSettings.setTurnEndDeadline (cGameSettings::defaultEndTurnDeadlineOption1);
	else if (turnEndTurnDeadline2CheckBox->isChecked()) gameSettings.setTurnEndDeadline (cGameSettings::defaultEndTurnDeadlineOption2);
	else if (turnEndTurnDeadline3CheckBox->isChecked()) gameSettings.setTurnEndDeadline (cGameSettings::defaultEndTurnDeadlineOption3);
	else if (turnEndTurnDeadline4CheckBox->isChecked()) gameSettings.setTurnEndDeadline (cGameSettings::defaultEndTurnDeadlineOption4);
	else if (turnEndTurnDeadline5CheckBox->isChecked()) gameSettings.setTurnEndDeadline (cGameSettings::defaultEndTurnDeadlineOption5);
	else if (turnEndTurnDeadlineCustomCheckBox->isChecked()) gameSettings.setTurnEndDeadline (std::chrono::seconds (std::atoi (turnEndTurnDeadlineLineEdit->getText().c_str())));

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
	turnEndTurnDeadlineNoLimitCheckBox->disable();
	turnEndTurnDeadlineNoLimitCheckBox->hide();
	turnEndTurnDeadline0CheckBox->disable();
	turnEndTurnDeadline0CheckBox->hide();
	turnEndTurnDeadline1CheckBox->disable();
	turnEndTurnDeadline1CheckBox->hide();
	turnEndTurnDeadline2CheckBox->disable();
	turnEndTurnDeadline2CheckBox->hide();
	turnEndTurnDeadline3CheckBox->disable();
	turnEndTurnDeadline3CheckBox->hide();
	turnEndTurnDeadline4CheckBox->disable();
	turnEndTurnDeadline4CheckBox->hide();
	turnEndTurnDeadline5CheckBox->disable();
	turnEndTurnDeadline5CheckBox->hide();
	turnEndTurnDeadlineCustomCheckBox->disable();
	turnEndTurnDeadlineCustomCheckBox->hide();
	turnEndTurnDeadlineLineEdit->disable();
	turnEndTurnDeadlineLineEdit->hide();

	turnEndDeadlineLabel->hide();
	turnEndDeadlineSecondsLabel->hide();
}

//------------------------------------------------------------------------------
void cWindowGameSettings::enableTurnEndDeadlineOptions()
{
	turnEndTurnDeadlineNoLimitCheckBox->enable();
	turnEndTurnDeadlineNoLimitCheckBox->show();
	turnEndTurnDeadline0CheckBox->enable();
	turnEndTurnDeadline0CheckBox->show();
	turnEndTurnDeadline1CheckBox->enable();
	turnEndTurnDeadline1CheckBox->show();
	turnEndTurnDeadline2CheckBox->enable();
	turnEndTurnDeadline2CheckBox->show();
	turnEndTurnDeadline3CheckBox->enable();
	turnEndTurnDeadline3CheckBox->show();
	turnEndTurnDeadline4CheckBox->enable();
	turnEndTurnDeadline4CheckBox->show();
	turnEndTurnDeadline5CheckBox->enable();
	turnEndTurnDeadline5CheckBox->show();
	turnEndTurnDeadlineCustomCheckBox->enable();
	turnEndTurnDeadlineCustomCheckBox->show();
	turnEndTurnDeadlineLineEdit->enable();
	turnEndTurnDeadlineLineEdit->show();

	turnEndDeadlineLabel->show();
	turnEndDeadlineSecondsLabel->show();
}