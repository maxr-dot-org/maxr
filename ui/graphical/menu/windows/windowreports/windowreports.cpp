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

#include <sstream>

#include "ui/graphical/menu/windows/windowreports/windowreports.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/radiogroup.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/frame.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/plot.h"
#include "ui/graphical/menu/widgets/special/reportunitlistviewitem.h"
#include "ui/graphical/menu/widgets/special/reportdisadvantageslistviewitem.h"
#include "ui/graphical/menu/widgets/special/reportmessagelistviewitem.h"
#include "pcx.h"
#include "main.h"
#include "game/data/player/player.h"
#include "vehicles.h"
#include "buildings.h"
#include "casualtiestracker.h"
#include "game/data/report/savedreport.h"
#include "game/logic/turnclock.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"

namespace {
std::string plural (int n, const std::string& sing, const std::string& plu)
{
	std::stringstream ss;
	ss << n << " ";
	ss << lngPack.i18n (n == 1 ? sing : plu);
	return ss.str ();
}
}

//------------------------------------------------------------------------------
cWindowReports::cWindowReports (std::vector<std::shared_ptr<const cPlayer>> players_,
								std::shared_ptr<const cPlayer> localPlayer_,
								std::shared_ptr<const cCasualtiesTracker> casualties_,
								std::shared_ptr<const cTurnClock> turnClock_,
								std::shared_ptr<const cTurnTimeClock> turnTimeClock,
								std::shared_ptr<const cGameSettings> gameSettings_) :
	cWindow (LoadPCX (GFXOD_REPORTS)),
	players (std::move(players_)),
	localPlayer (localPlayer_),
	casualties (std::move(casualties_)),
	turnClock (std::move (turnClock_)),
	gameSettings (std::move (gameSettings_)),
	unitListDirty (true),
	disadvantagesListDirty (true),
	reportsListDirty (true)
{
	auto turnTimeClockWidget = addChild (std::make_unique<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (527, 17), cPosition (527 + 57, 17 + 10))));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	auto typeButtonGroup = addChild (std::make_unique<cRadioGroup> ());
	unitsRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (524, 71), lngPack.i18n ("Text~Others~Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));
	disadvantagesRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (524, 71 + 29), lngPack.i18n ("Text~Others~Disadvantages_8cut"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));
	scoreRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (524, 71 + 29 * 2), lngPack.i18n ("Text~Others~Score"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));
	reportsRadioButton = typeButtonGroup->addButton (std::make_unique<cCheckBox> (getPosition () + cPosition (524, 71 + 29 * 3), lngPack.i18n ("Text~Others~Reports"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular));

	signalConnectionManager.connect (unitsRadioButton->toggled, std::bind (&cWindowReports::updateActiveFrame, this));
	signalConnectionManager.connect (disadvantagesRadioButton->toggled, std::bind (&cWindowReports::updateActiveFrame, this));
	signalConnectionManager.connect (scoreRadioButton->toggled, std::bind (&cWindowReports::updateActiveFrame, this));
	signalConnectionManager.connect (reportsRadioButton->toggled, std::bind (&cWindowReports::updateActiveFrame, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (497, 207), getPosition () + cPosition (497 + 100, 207 + font->getFontHeight ())), lngPack.i18n ("Text~Others~Included") + ":"));

	planesCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 218), lngPack.i18n ("Text~Others~Air_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	groundCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 218 + 18), lngPack.i18n ("Text~Others~Ground_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	seaCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 218 + 18 * 2), lngPack.i18n ("Text~Others~Sea_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	stationaryCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 218 + 18 * 3), lngPack.i18n ("Text~Others~Stationary_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	
	planesCheckBox->setChecked (true);
	groundCheckBox->setChecked (true);
	seaCheckBox->setChecked (true);
	stationaryCheckBox->setChecked (true);
	
	signalConnectionManager.connect (planesCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	signalConnectionManager.connect (groundCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	signalConnectionManager.connect (seaCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	signalConnectionManager.connect (stationaryCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (497, 299), getPosition () + cPosition (497 + 100, 299 + font->getFontHeight ())), lngPack.i18n ("Text~Others~Limited_To") + ":"));

	produceCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 312), lngPack.i18n ("Text~Others~Produce_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	fightCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 312 + 18), lngPack.i18n ("Text~Others~Fight_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	damagedCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 312 + 18 * 2), lngPack.i18n ("Text~Others~Damaged_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	stealthCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (496, 312 + 18 * 3), lngPack.i18n ("Text~Others~Stealth_Units"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Right, eCheckBoxType::Standard));
	
	signalConnectionManager.connect (produceCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	signalConnectionManager.connect (fightCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	signalConnectionManager.connect (damagedCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));
	signalConnectionManager.connect (stealthCheckBox->toggled, std::bind (&cWindowReports::handleFilterChanged, this));

	auto doneButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (524, 395), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, std::bind (&cWindowReports::close, this));

	upButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (492, 426), ePushButtonType::ArrowUpBig));
	signalConnectionManager.connect (upButton->clicked, std::bind (&cWindowReports::upPressed, this));
	downButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (525, 426), ePushButtonType::ArrowDownBig));
	signalConnectionManager.connect (downButton->clicked, std::bind (&cWindowReports::downPressed, this));

	const cBox<cPosition> frameArea(getPosition () + cPosition (18, 15), getPosition () + cPosition (18+458, 15+447));

	using namespace std::placeholders;

	unitsFrame = addChild (std::make_unique<cFrame> (frameArea));
	unitsList = unitsFrame->addChild (std::make_unique<cListView<cReportUnitListViewItem>> (frameArea));
	unitsList->setBeginMargin (cPosition (5, 4));
	unitsList->setItemDistance (cPosition (0, 6));
	signalConnectionManager.connect (unitsList->itemClicked, std::bind (&cWindowReports::handleUnitClicked, this, _1));

	disadvantagesFrame = addChild (std::make_unique<cFrame> (frameArea));
	disadvantagesList = disadvantagesFrame->addChild (std::make_unique<cListView<cReportDisadvantagesListViewItem>> (cBox<cPosition> (frameArea.getMinCorner () + cPosition (0, (players.size () / cReportDisadvantagesListViewItem::maxItemsInRow) + 1) * font->getFontHeight (), frameArea.getMaxCorner ())));
	const auto playerNameStartXPos = cReportDisadvantagesListViewItem::unitImageWidth + cReportDisadvantagesListViewItem::unitNameWidth;
	for (size_t i = 0; i < players.size (); ++i)
	{
		const auto& player = players[i];

		const int row = static_cast<int>(i / cReportDisadvantagesListViewItem::maxItemsInRow);
		const int col = static_cast<int>(i % cReportDisadvantagesListViewItem::maxItemsInRow);

		disadvantagesFrame->addChild (std::make_unique<cLabel> (cBox<cPosition> (disadvantagesFrame->getPosition () + cPosition (playerNameStartXPos + cReportDisadvantagesListViewItem::casualityLabelWidth * col + (row % 2 == 0 ? 15 : 0), font->getFontHeight ()*row), disadvantagesFrame->getPosition () + cPosition (playerNameStartXPos + cReportDisadvantagesListViewItem::casualityLabelWidth * (col+1) + (row % 2 == 0 ? 15 : 0), font->getFontHeight ()*(row+1))), player->getName (), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	}

	scoreFrame = addChild (std::make_unique<cFrame> (frameArea));

	if (gameSettings)
	{
		std::string gameEndString;
		switch (gameSettings->getVictoryCondition ())
		{
		case eGameSettingsVictoryCondition::Turns:
			gameEndString = lngPack.i18n ("Text~Comp~GameEndsAt") + " " + plural (gameSettings->getVictoryTurns (), "Text~Comp~Turn_5", "Text~Comp~Turns");
			break;
		case eGameSettingsVictoryCondition::Points:
			gameEndString = lngPack.i18n ("Text~Comp~GameEndsAt") + " " + plural (gameSettings->getVictoryPoints (), "Text~Comp~Point", "Text~Comp~Points");
			break;
		case eGameSettingsVictoryCondition::Death:
			gameEndString = lngPack.i18n ("Text~Comp~NoLimit");
			break;
		}
		scoreFrame->addChild (std::make_unique<cLabel> (cBox<cPosition> (scoreFrame->getPosition () + cPosition (5, 5), scoreFrame->getPosition () + cPosition (5 + 450, 5 + font->getFontHeight ())), gameEndString));
	}
	for (size_t i = 0; i < players.size (); ++i)
	{
		const auto& player = players[i];

		std::string playerText = player->getName () + ": " + plural (player->getScore (turnClock->getTurn ()), "Text~Comp~Point", "Text~Comp~Points") + ", " + plural (player->numEcos, "Text~Comp~EcoSphere", "Text~Comp~EcoSpheres");

		AutoSurface colorSurface(SDL_CreateRGBSurface (0, 8, 8, Video.getColDepth (), 0, 0, 0, 0));
		const auto c = player->getColor ().getColor ().toMappedSdlRGBAColor (colorSurface->format);
		SDL_FillRect (colorSurface.get (), nullptr, player->getColor().getColor().toMappedSdlRGBAColor (colorSurface->format));
		scoreFrame->addChild (std::make_unique<cImage> (scoreFrame->getPosition () + cPosition (5, 20 + font->getFontHeight () * i), colorSurface.get ()));

		scoreFrame->addChild (std::make_unique<cLabel> (cBox<cPosition> (scoreFrame->getPosition () + cPosition (16, 20 + font->getFontHeight () * i), scoreFrame->getPosition () + cPosition (16 + 435, 20 + font->getFontHeight () * (i+1))), playerText));
	}
	scorePlot = scoreFrame->addChild (std::make_unique<cPlot<int, int>> (cBox<cPosition> (scoreFrame->getPosition () + cPosition (0, 20 + font->getFontHeight () * players.size ()), scoreFrame->getEndPosition())));

	reportsFrame = addChild (std::make_unique<cFrame> (frameArea));
	reportsList = reportsFrame->addChild (std::make_unique<cListView<cReportMessageListViewItem>> (frameArea));
	reportsList->setItemDistance (cPosition (0, 6));
	signalConnectionManager.connect (reportsList->itemClicked, std::bind (&cWindowReports::handleReportClicked, this, _1));

	updateActiveFrame ();
	initializeScorePlot ();
}

//------------------------------------------------------------------------------
void cWindowReports::updateActiveFrame ()
{
	if (unitsRadioButton->isChecked ())
	{
		rebuildUnitList ();

		unitsFrame->show ();
		unitsFrame->enable ();
		upButton->unlock ();
		downButton->unlock ();
	}
	else
	{
		unitsFrame->hide ();
		unitsFrame->disable ();
	}

	if (disadvantagesRadioButton->isChecked ())
	{
		rebuildDisadvantagesList ();

		disadvantagesFrame->show ();
		disadvantagesFrame->enable ();
		upButton->unlock ();
		downButton->unlock ();
	}
	else
	{
		disadvantagesFrame->hide ();
		disadvantagesFrame->disable ();
	}

	if (scoreRadioButton->isChecked ())
	{
		scoreFrame->show ();
		scoreFrame->enable ();
		upButton->lock ();
		downButton->lock ();
	}
	else
	{
		scoreFrame->hide ();
		scoreFrame->disable ();
	}

	if (reportsRadioButton->isChecked ())
	{
		rebuildReportsList ();

		reportsFrame->show ();
		reportsFrame->enable ();
		upButton->unlock ();
		downButton->unlock ();
	}
	else
	{
		reportsFrame->hide ();
		reportsFrame->disable ();
	}
}

//------------------------------------------------------------------------------
void cWindowReports::upPressed ()
{
	if (unitsRadioButton->isChecked ())
	{
		unitsList->pageUp ();
	}
	if (disadvantagesRadioButton->isChecked ())
	{
		disadvantagesList->pageUp ();
	}
	if (reportsRadioButton->isChecked ())
	{
		reportsList->pageUp ();
	}
}

//------------------------------------------------------------------------------
void cWindowReports::downPressed ()
{
	if (unitsRadioButton->isChecked ())
	{
		unitsList->pageDown ();
	}
	if (disadvantagesRadioButton->isChecked ())
	{
		disadvantagesList->pageDown ();
	}
	if (reportsRadioButton->isChecked ())
	{
		reportsList->pageDown ();
	}
}

//-----------------------------------------------------------------------------
bool cWindowReports::checkFilter (const sUnitData& data) const
{
	if (data.ID.isAVehicle())
	{
		if (data.factorAir > 0 && !planesCheckBox->isChecked()) return false;
		if (data.factorGround > 0 && (data.factorSea == 0 || !seaCheckBox->isChecked ()) && !groundCheckBox->isChecked ()) return false;
		if (data.factorSea > 0 && (data.factorGround == 0 || !groundCheckBox->isChecked ()) && !seaCheckBox->isChecked ()) return false;
	}
	else if (data.ID.isABuilding ())
	{
		if (!stationaryCheckBox->isChecked ()) return false;
	}

	if (data.canBuild.empty () && produceCheckBox->isChecked ()) return false;
	if (!data.canAttack && fightCheckBox->isChecked ()) return false;
	if (data.getHitpoints () >= data.hitpointsMax && damagedCheckBox->isChecked ()) return false;
	if (!data.isStealthOn && stealthCheckBox->isChecked ()) return false;

	if (data.surfacePosition != sUnitData::SURFACE_POS_GROUND) return false;

	return true;
}

//------------------------------------------------------------------------------
void cWindowReports::handleFilterChanged ()
{
	unitListDirty = true;
	disadvantagesListDirty = true;

	if (unitsRadioButton->isChecked ())
	{
		rebuildUnitList ();
	}
	if (disadvantagesRadioButton->isChecked ())
	{
		rebuildDisadvantagesList ();
	}
}

//------------------------------------------------------------------------------
void cWindowReports::rebuildUnitList ()
{
	// TODO: if this turns out to be a real performance problem we may need to 
	//       implement filter support directly into cListView.

	if (!unitListDirty) return;

	unitsList->clearItems ();

	if (localPlayer != nullptr)
	{
		const auto& vehicles = localPlayer->getVehicles ();
		for (auto i = vehicles.begin (); i != vehicles.end (); ++i)
		{
			const auto& vehicle = *i;
			if (checkFilter (vehicle->data))
			{
				unitsList->addItem (std::make_unique<cReportUnitListViewItem> (*vehicle));
			}
		}

		if (stationaryCheckBox->isChecked ())
		{
			const auto& buildings = localPlayer->getBuildings ();
			for (auto i = buildings.begin (); i != buildings.end (); ++i)
			{
				const auto& building = *i;
				if (checkFilter (building->data))
				{
					unitsList->addItem (std::make_unique<cReportUnitListViewItem> (*building));
				}
			}
		}
	}

	unitListDirty = false;
}

//------------------------------------------------------------------------------
void cWindowReports::rebuildDisadvantagesList ()
{
	if (!disadvantagesListDirty) return;

	disadvantagesList->clearItems ();

	if (!casualties) return;

	const auto unitTypesWithLosses = casualties->getUnitTypesWithLosses ();

	for (size_t i = 0; i < unitTypesWithLosses.size (); ++i)
	{
		const auto& unitId = unitTypesWithLosses[i];

		const auto& unitData = UnitsData.getUnit (unitId);

		if (!checkFilter (unitData)) continue;

		std::vector<int> unitCasualities;
		unitCasualities.reserve (players.size ());
		for (size_t j = 0; j < players.size (); ++j)
		{
			const auto& player = players[j];

			if (player)
			{
				unitCasualities.push_back(casualties->getCasualtiesOfUnitType (unitId, player->getNr ()));
			}
		}

		disadvantagesList->addItem (std::make_unique<cReportDisadvantagesListViewItem> (unitId, std::move (unitCasualities)));
	}

	disadvantagesListDirty = false;
}

//------------------------------------------------------------------------------
void cWindowReports::rebuildReportsList ()
{
	if (!reportsListDirty) return;

	if (!localPlayer) return;

	reportsList->clearItems ();

	const auto& savedReports = localPlayer->getSavedReports ();

	cReportMessageListViewItem* lastItem = nullptr;
	for (size_t i = 0; i < savedReports.size (); ++i)
	{
		const auto& savedReport = savedReports[i];

		if (savedReport)
		{
			if (savedReport->getType () == eSavedReportType::Chat) continue;

			lastItem = reportsList->addItem (std::make_unique<cReportMessageListViewItem> (*savedReport));
		}
	}
	if (lastItem) reportsList->scrollToItem (lastItem);

	reportsListDirty = false;
}

//------------------------------------------------------------------------------
void cWindowReports::initializeScorePlot ()
{
	if (!turnClock) return;

	auto extrapolate = [ ](const cPlayer& p, int c, int t)
	{
		if (t <= c) return p.getScore (t);
		else return p.getScore (c) + p.numEcos * (t-c);
	};

	const int displayTurns = 50;

	auto maxTurns = turnClock->getTurn () + 20;
	auto minTurns = maxTurns - displayTurns;

	if (minTurns < 1)
	{
		const auto over = 1 - minTurns;
		maxTurns += over;
		minTurns += over;
	}

	int maxScore = 0;
	int minScore = std::numeric_limits<int>::max ();
	for (int turn = minTurns; turn <= maxTurns; ++turn)
	{
		for (size_t i = 0; i < players.size (); ++i)
		{
			maxScore = std::max (maxScore, extrapolate (*players[i], turnClock->getTurn (), turn));
			minScore = std::min (minScore, extrapolate (*players[i], turnClock->getTurn (), turn));
		}
	}

	if (maxScore - minScore < 10)
	{
		maxScore = minScore + 10;
	}

	const cColor axisColor (164, 164, 164);
	const cColor limitColor (128, 128, 128);

	scorePlot->getXAxis ().setInterval (minTurns, maxTurns);
	scorePlot->getXAxis ().setColor (axisColor);

	scorePlot->getYAxis ().setInterval (minScore, maxScore);
	scorePlot->getYAxis ().setColor (axisColor);

	for (size_t i = 0; i < players.size (); ++i)
	{
		auto& graph = scorePlot->addGraph ([=](int x){ return extrapolate (*players[i], turnClock->getTurn (), x); });
		graph.setColor (players[i]->getColor ().getColor ());
	}

	{
		auto& marker = scorePlot->addXMarker (turnClock->getTurn ());
		marker.setColor (limitColor);
	}

	if (gameSettings)
	{
		if (gameSettings->getVictoryCondition () == eGameSettingsVictoryCondition::Turns)
		{
			auto& marker = scorePlot->addXMarker (gameSettings->getVictoryTurns());
			marker.setColor (limitColor);
		}
		else if (gameSettings->getVictoryCondition () == eGameSettingsVictoryCondition::Points)
		{
			auto& marker = scorePlot->addYMarker (gameSettings->getVictoryPoints());
			marker.setColor (limitColor);
		}
	}
}

//------------------------------------------------------------------------------
void cWindowReports::handleUnitClicked (cReportUnitListViewItem& item)
{
	if (&item == unitsList->getSelectedItem ())
	{
		unitClickedSecondTime (item.getUnit ());
	}
}

//------------------------------------------------------------------------------
void cWindowReports::handleReportClicked (cReportMessageListViewItem& item)
{
	if (&item == reportsList->getSelectedItem ())
	{
		reportClickedSecondTime (item.getReport ());
	}
}
