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

#include "windowreports.h"

#include "SDLutility/tosdl.h"
#include "game/data/gamesettings.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/report/savedreport.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/turncounter.h"
#include "resources/pcx.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/plot.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/radiogroup.h"
#include "ui/graphical/menu/widgets/special/reportdisadvantageslistviewitem.h"
#include "ui/graphical/menu/widgets/special/reportmessagelistviewitem.h"
#include "ui/graphical/menu/widgets/special/reportunitlistviewitem.h"
#include "ui/uidefines.h"
#include "ui/widgets/frame.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

#include <sstream>

namespace
{
	//--------------------------------------------------------------------------
	std::string getVictoryString (const cGameSettings& gameSettings)
	{
		switch (gameSettings.victoryConditionType)
		{
			case eGameSettingsVictoryCondition::Turns:
				return lngPack.i18n ("Comp~GameEndsAt") + " " + lngPack.plural ("Comp~Turn(s)", gameSettings.victoryTurns);
			case eGameSettingsVictoryCondition::Points:
				return lngPack.i18n ("Comp~GameEndsAt") + " " + lngPack.plural ("Comp~Point(s)", gameSettings.victoryPoints);
			case eGameSettingsVictoryCondition::Death:
				return lngPack.i18n ("Comp~NoLimit");
		}
		return "";
	}
} // namespace

//------------------------------------------------------------------------------
cWindowReports::cWindowReports (const cModel& model,
                                std::shared_ptr<const cPlayer> localPlayer_,
                                const std::vector<std::unique_ptr<cSavedReport>>& reports_) :
	cWindow (LoadPCX (GFXOD_REPORTS)),
	model (model),
	localPlayer (localPlayer_),
	reports (reports_)
{
	auto* font = cUnicodeFont::font.get();

	auto turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (527, 17), cPosition (527 + 57, 17 + 10)));
	turnTimeClockWidget->setTurnTimeClock (model.getTurnTimeClock());

	auto typeButtonGroup = emplaceChild<cRadioGroup>();
	unitsRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (524, 71), lngPack.i18n ("Others~Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));
	disadvantagesRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (524, 71 + 29), lngPack.i18n ("Others~Disadvantages_8cut"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));
	scoreRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (524, 71 + 29 * 2), lngPack.i18n ("Others~Score"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));
	reportsRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition() + cPosition (524, 71 + 29 * 3), lngPack.i18n ("Others~Reports"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));

	signalConnectionManager.connect (unitsRadioButton->toggled, [this]() { updateActiveFrame(); });
	signalConnectionManager.connect (disadvantagesRadioButton->toggled, [this]() { updateActiveFrame(); });
	signalConnectionManager.connect (scoreRadioButton->toggled, [this]() { updateActiveFrame(); });
	signalConnectionManager.connect (reportsRadioButton->toggled, [this]() { updateActiveFrame(); });

	includedLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (497, 207), getPosition() + cPosition (497 + 100, 207 + font->getFontHeight())), lngPack.i18n ("Others~Included"));

	planesCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 218), lngPack.i18n ("Others~Air_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);
	groundCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 218 + 18), lngPack.i18n ("Others~Ground_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);
	seaCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 218 + 18 * 2), lngPack.i18n ("Others~Sea_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);
	stationaryCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 218 + 18 * 3), lngPack.i18n ("Others~Stationary_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);

	planesCheckBox->setChecked (true);
	groundCheckBox->setChecked (true);
	seaCheckBox->setChecked (true);
	stationaryCheckBox->setChecked (true);

	signalConnectionManager.connect (planesCheckBox->toggled, [this]() { handleFilterChanged(); });
	signalConnectionManager.connect (groundCheckBox->toggled, [this]() { handleFilterChanged(); });
	signalConnectionManager.connect (seaCheckBox->toggled, [this]() { handleFilterChanged(); });
	signalConnectionManager.connect (stationaryCheckBox->toggled, [this]() { handleFilterChanged(); });

	limitedToLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (497, 299), getPosition() + cPosition (497 + 100, 299 + font->getFontHeight())), lngPack.i18n ("Others~Limited_To"));

	produceCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 312), lngPack.i18n ("Others~Produce_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);
	fightCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 312 + 18), lngPack.i18n ("Others~Fight_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);
	damagedCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 312 + 18 * 2), lngPack.i18n ("Others~Damaged_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);
	stealthCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (496, 312 + 18 * 3), lngPack.i18n ("Others~Stealth_Units"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard);

	signalConnectionManager.connect (produceCheckBox->toggled, [this]() { handleFilterChanged(); });
	signalConnectionManager.connect (fightCheckBox->toggled, [this]() { handleFilterChanged(); });
	signalConnectionManager.connect (damagedCheckBox->toggled, [this]() { handleFilterChanged(); });
	signalConnectionManager.connect (stealthCheckBox->toggled, [this]() { handleFilterChanged(); });

	doneButton = emplaceChild<cPushButton> (getPosition() + cPosition (524, 395), ePushButtonType::Angular, lngPack.i18n ("Others~Done"), eUnicodeFontType::LatinNormal);
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [this]() { close(); });

	upButton = emplaceChild<cPushButton> (getPosition() + cPosition (492, 426), ePushButtonType::ArrowUpBig);
	signalConnectionManager.connect (upButton->clicked, [this]() { upPressed(); });
	downButton = emplaceChild<cPushButton> (getPosition() + cPosition (525, 426), ePushButtonType::ArrowDownBig);
	signalConnectionManager.connect (downButton->clicked, [this]() { downPressed(); });

	const cBox<cPosition> frameArea (getPosition() + cPosition (18, 15), getPosition() + cPosition (18 + 458, 15 + 447));

	unitsFrame = emplaceChild<cFrame> (frameArea);
	unitsList = unitsFrame->emplaceChild<cListView<cReportUnitListViewItem>> (frameArea);
	unitsList->setBeginMargin (cPosition (5, 4));
	unitsList->setItemDistance (6);
	signalConnectionManager.connect (unitsList->itemClicked, [this] (cReportUnitListViewItem& item) { handleUnitClicked (item); });

	const auto players = model.getPlayerList();
	disadvantagesFrame = emplaceChild<cFrame> (frameArea);
	disadvantagesList = disadvantagesFrame->emplaceChild<cListView<cReportDisadvantagesListViewItem>> (cBox<cPosition> (frameArea.getMinCorner() + cPosition (0, (players.size() / cReportDisadvantagesListViewItem::maxItemsInRow) + 1) * font->getFontHeight(), frameArea.getMaxCorner()));
	const auto playerNameStartXPos = cReportDisadvantagesListViewItem::unitImageWidth + cReportDisadvantagesListViewItem::unitNameWidth;
	for (size_t i = 0; i < players.size(); ++i)
	{
		const auto& player = players[i];

		const int row = static_cast<int> (i / cReportDisadvantagesListViewItem::maxItemsInRow);
		const int col = static_cast<int> (i % cReportDisadvantagesListViewItem::maxItemsInRow);

		disadvantagesFrame->emplaceChild<cLabel> (cBox<cPosition> (disadvantagesFrame->getPosition() + cPosition (playerNameStartXPos + cReportDisadvantagesListViewItem::casualityLabelWidth * col + (row % 2 == 0 ? 15 : 0), font->getFontHeight() * row), disadvantagesFrame->getPosition() + cPosition (playerNameStartXPos + cReportDisadvantagesListViewItem::casualityLabelWidth * (col + 1) + (row % 2 == 0 ? 15 : 0), font->getFontHeight() * (row + 1))), player->getName(), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	}

	scoreFrame = emplaceChild<cFrame> (frameArea);

	const auto gameSettings = model.getGameSettings();
	if (gameSettings)
	{
		const std::string gameEndString = getVictoryString (*gameSettings);
		victoryLabel = scoreFrame->emplaceChild<cLabel> (cBox<cPosition> (scoreFrame->getPosition() + cPosition (5, 5), scoreFrame->getPosition() + cPosition (5 + 450, 5 + font->getFontHeight())), gameEndString);
	}
	const auto turnClock = model.getTurnCounter();
	for (size_t i = 0; i < players.size(); ++i)
	{
		const auto& player = players[i];

		std::string playerText = player->getName() + lngPack.i18n ("Punctuation~Colon") + lngPack.plural ("Comp~Point(s)", player->getScore (turnClock->getTurn())) + ", " + lngPack.plural ("Comp~EcoSphere(s)", player->getNumEcoSpheres());

		AutoSurface colorSurface (SDL_CreateRGBSurface (0, 8, 8, Video.getColDepth(), 0, 0, 0, 0));
		SDL_FillRect (colorSurface.get(), nullptr, toMappedSdlRGBAColor (player->getColor(), colorSurface->format));
		scoreFrame->emplaceChild<cImage> (scoreFrame->getPosition() + cPosition (5, 20 + font->getFontHeight() * i), colorSurface.get());

		ecosphereLabels.emplace_back (scoreFrame->emplaceChild<cLabel> (cBox<cPosition> (scoreFrame->getPosition() + cPosition (16, 20 + font->getFontHeight() * i), scoreFrame->getPosition() + cPosition (16 + 435, 20 + font->getFontHeight() * (i + 1))), playerText));
	}
	scorePlot = scoreFrame->emplaceChild<cPlot<int, int>> (cBox<cPosition> (scoreFrame->getPosition() + cPosition (0, 20 + font->getFontHeight() * players.size()), scoreFrame->getEndPosition()));

	reportsFrame = emplaceChild<cFrame> (frameArea);
	reportsList = reportsFrame->emplaceChild<cListView<cReportMessageListViewItem>> (frameArea);
	reportsList->setItemDistance (6);
	signalConnectionManager.connect (reportsList->itemClicked, [this] (cReportMessageListViewItem& item) { handleReportClicked (item); });

	updateActiveFrame();
	initializeScorePlot();
}

//------------------------------------------------------------------------------
void cWindowReports::retranslate()
{
	cWindow::retranslate();

	unitsRadioButton->setText (lngPack.i18n ("Others~Units"));
	disadvantagesRadioButton->setText (lngPack.i18n ("Others~Disadvantages_8cut"));
	scoreRadioButton->setText (lngPack.i18n ("Others~Score"));
	reportsRadioButton->setText (lngPack.i18n ("Others~Reports"));

	includedLabel->setText (lngPack.i18n ("Others~Included"));

	planesCheckBox->setText (lngPack.i18n ("Others~Air_Units"));
	groundCheckBox->setText (lngPack.i18n ("Others~Ground_Units"));
	seaCheckBox->setText (lngPack.i18n ("Others~Sea_Units"));
	stationaryCheckBox->setText (lngPack.i18n ("Others~Stationary_Units"));

	limitedToLabel->setText (lngPack.i18n ("Others~Limited_To"));

	produceCheckBox->setText (lngPack.i18n ("Others~Produce_Units"));
	fightCheckBox->setText (lngPack.i18n ("Others~Fight_Units"));
	damagedCheckBox->setText (lngPack.i18n ("Others~Damaged_Units"));
	stealthCheckBox->setText (lngPack.i18n ("Others~Stealth_Units"));

	doneButton->setText (lngPack.i18n ("Others~Done"));

	const auto gameSettings = model.getGameSettings();
	if (gameSettings)
	{
		std::string gameEndString = getVictoryString (*gameSettings);
		victoryLabel->setText (gameEndString);
	}
	const auto players = model.getPlayerList();
	const auto turnClock = model.getTurnCounter();
	for (size_t i = 0; i < players.size(); ++i)
	{
		const auto& player = players[i];

		std::string playerText = player->getName() + lngPack.i18n ("Punctuation~Colon") + lngPack.plural ("Comp~Point(s)", player->getScore (turnClock->getTurn())) + ", " + lngPack.plural ("Comp~EcoSphere(s)", player->getNumEcoSpheres());

		ecosphereLabels[i]->setText (playerText);
	}
}

//------------------------------------------------------------------------------
void cWindowReports::updateActiveFrame()
{
	if (unitsRadioButton->isChecked())
	{
		rebuildUnitList();

		unitsFrame->show();
		unitsFrame->enable();
		upButton->unlock();
		downButton->unlock();
	}
	else
	{
		unitsFrame->hide();
		unitsFrame->disable();
	}

	if (disadvantagesRadioButton->isChecked())
	{
		rebuildDisadvantagesList();

		disadvantagesFrame->show();
		disadvantagesFrame->enable();
		upButton->unlock();
		downButton->unlock();
	}
	else
	{
		disadvantagesFrame->hide();
		disadvantagesFrame->disable();
	}

	if (scoreRadioButton->isChecked())
	{
		scoreFrame->show();
		scoreFrame->enable();
		upButton->lock();
		downButton->lock();
	}
	else
	{
		scoreFrame->hide();
		scoreFrame->disable();
	}

	if (reportsRadioButton->isChecked())
	{
		rebuildReportsList();

		reportsFrame->show();
		reportsFrame->enable();
		upButton->unlock();
		downButton->unlock();
	}
	else
	{
		reportsFrame->hide();
		reportsFrame->disable();
	}
}

//------------------------------------------------------------------------------
void cWindowReports::upPressed()
{
	if (unitsRadioButton->isChecked())
	{
		unitsList->pageUp();
	}
	if (disadvantagesRadioButton->isChecked())
	{
		disadvantagesList->pageUp();
	}
	if (reportsRadioButton->isChecked())
	{
		reportsList->pageUp();
	}
}

//------------------------------------------------------------------------------
void cWindowReports::downPressed()
{
	if (unitsRadioButton->isChecked())
	{
		unitsList->pageDown();
	}
	if (disadvantagesRadioButton->isChecked())
	{
		disadvantagesList->pageDown();
	}
	if (reportsRadioButton->isChecked())
	{
		reportsList->pageDown();
	}
}

//------------------------------------------------------------------------------
bool cWindowReports::checkFilter (const cUnit& unit) const
{
	if (unit.data.getHitpoints() >= unit.data.getHitpointsMax() && damagedCheckBox->isChecked()) return false;

	return checkFilter (unit.getStaticUnitData());
}

//-----------------------------------------------------------------------------
bool cWindowReports::checkFilter (const cStaticUnitData& data) const
{
	if (data.ID.isAVehicle())
	{
		if (data.factorAir > 0 && !planesCheckBox->isChecked()) return false;
		if (data.factorGround > 0 && (data.factorSea == 0 || !seaCheckBox->isChecked()) && !groundCheckBox->isChecked()) return false;
		if (data.factorSea > 0 && (data.factorGround == 0 || !groundCheckBox->isChecked()) && !seaCheckBox->isChecked()) return false;
	}
	else if (data.ID.isABuilding())
	{
		if (!stationaryCheckBox->isChecked()) return false;
	}

	if (data.canBuild.empty() && produceCheckBox->isChecked()) return false;
	if (!data.canAttack && fightCheckBox->isChecked()) return false;

	if (!data.isStealthOn && stealthCheckBox->isChecked()) return false;

	if (data.surfacePosition != eSurfacePosition::Ground) return false;

	return true;
}

//------------------------------------------------------------------------------
void cWindowReports::handleFilterChanged()
{
	unitListDirty = true;
	disadvantagesListDirty = true;

	if (unitsRadioButton->isChecked())
	{
		rebuildUnitList();
	}
	if (disadvantagesRadioButton->isChecked())
	{
		rebuildDisadvantagesList();
	}
}

//------------------------------------------------------------------------------
void cWindowReports::rebuildUnitList()
{
	// TODO: if this turns out to be a real performance problem we may need to
	//       implement filter support directly into cListView.

	if (!unitListDirty) return;

	unitsList->clearItems();

	if (localPlayer != nullptr)
	{
		const auto unitsData = model.getUnitsData();
		const auto& vehicles = localPlayer->getVehicles();
		for (const auto& vehicle : vehicles)
		{
			if (checkFilter (*vehicle))
			{
				unitsList->addItem (std::make_unique<cReportUnitListViewItem> (*vehicle, *unitsData));
			}
		}

		if (stationaryCheckBox->isChecked())
		{
			for (const auto& building : localPlayer->getBuildings())
			{
				if (checkFilter (*building))
				{
					unitsList->addItem (std::make_unique<cReportUnitListViewItem> (*building, *unitsData));
				}
			}
		}
	}

	unitListDirty = false;
}

//------------------------------------------------------------------------------
void cWindowReports::rebuildDisadvantagesList()
{
	if (!disadvantagesListDirty) return;

	disadvantagesList->clearItems();

	const auto casualties = model.getCasualtiesTracker();
	if (!casualties) return;

	const auto unitTypesWithLosses = casualties->getUnitTypesWithLosses();
	const auto unitsData = model.getUnitsData();
	const auto players = model.getPlayerList();
	for (const auto& unitId : unitTypesWithLosses)
	{
		const cStaticUnitData& unitData = unitsData->getStaticUnitData (unitId);

		if (!checkFilter (unitData)) continue;

		std::vector<int> unitCasualities;
		unitCasualities.reserve (players.size());
		for (const auto& player : players)
		{
			if (player)
			{
				unitCasualities.push_back (casualties->getCasualtiesOfUnitType (unitId, player->getId()));
			}
		}

		disadvantagesList->addItem (std::make_unique<cReportDisadvantagesListViewItem> (unitData, std::move (unitCasualities)));
	}

	disadvantagesListDirty = false;
}

//------------------------------------------------------------------------------
void cWindowReports::rebuildReportsList()
{
	if (!reportsListDirty) return;

	if (!localPlayer) return;

	reportsList->clearItems();

	cReportMessageListViewItem* lastItem = nullptr;
	for (const auto& savedReport : reports)
	{
		if (savedReport)
		{
			if (savedReport->getType() == eSavedReportType::Chat) continue;

			lastItem = reportsList->addItem (std::make_unique<cReportMessageListViewItem> (*savedReport, model));
		}
	}
	if (lastItem) reportsList->scrollToItem (lastItem);

	reportsListDirty = false;
}

//------------------------------------------------------------------------------
void cWindowReports::initializeScorePlot()
{
	const auto turnClock = model.getTurnCounter();

	if (!turnClock) return;

	auto extrapolate = [] (const cPlayer& p, int c, int t) {
		if (t <= c)
			return p.getScore (t);
		else
			return p.getScore (c) + p.getNumEcoSpheres() * (t - c);
	};

	const int displayTurns = 50;

	auto maxTurns = turnClock->getTurn() + 20;
	auto minTurns = maxTurns - displayTurns;

	if (minTurns < 1)
	{
		const auto over = 1 - minTurns;
		maxTurns += over;
		minTurns += over;
	}

	const auto players = model.getPlayerList();
	int maxScore = 0;
	int minScore = std::numeric_limits<int>::max();
	for (int turn = minTurns; turn <= maxTurns; ++turn)
	{
		for (const auto& player : players)
		{
			maxScore = std::max (maxScore, extrapolate (*player, turnClock->getTurn(), turn));
			minScore = std::min (minScore, extrapolate (*player, turnClock->getTurn(), turn));
		}
	}
	maxScore = std::max(maxScore, minScore + 10);

	const cRgbColor axisColor (164, 164, 164);
	const cRgbColor limitColor (128, 128, 128);

	scorePlot->getXAxis().setInterval (minTurns, maxTurns);
	scorePlot->getXAxis().setColor (axisColor);

	scorePlot->getYAxis().setInterval (minScore, maxScore);
	scorePlot->getYAxis().setColor (axisColor);

	for (const auto& player : players)
	{
		auto& graph = scorePlot->addGraph ([=] (int x) { return extrapolate (*player, turnClock->getTurn(), x); });
		graph.setColor (player->getColor());
	}

	{
		auto& marker = scorePlot->addXMarker (turnClock->getTurn());
		marker.setColor (limitColor);
	}

	const auto gameSettings = model.getGameSettings();
	if (gameSettings)
	{
		switch (gameSettings->victoryConditionType)
		{
			case eGameSettingsVictoryCondition::Turns:
			{
				auto& marker = scorePlot->addXMarker (gameSettings->victoryTurns);
				marker.setColor (limitColor);
				break;
			}
			case eGameSettingsVictoryCondition::Points:
			{
				auto& marker = scorePlot->addYMarker (gameSettings->victoryPoints);
				marker.setColor (limitColor);
				break;
			}
			default: break;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowReports::handleUnitClicked (cReportUnitListViewItem& item)
{
	if (&item == unitsList->getSelectedItem())
	{
		unitClickedSecondTime (item.getUnit());
	}
}

//------------------------------------------------------------------------------
void cWindowReports::handleReportClicked (cReportMessageListViewItem& item)
{
	if (&item == reportsList->getSelectedItem())
	{
		reportClickedSecondTime (item.getReport());
	}
}
