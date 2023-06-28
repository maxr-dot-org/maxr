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

#include "windowresourcedistribution.h"

#include "game/data/base/base.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/logic/subbaseresourcedistribution.h"
#include "resources/pcx.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/resourcebar.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "ui/widgets/label.h"
#include "utility/language.h"
#include "utility/listhelpers.h"

namespace
{
	//------------------------------------------------------------------------------
	std::string secondBarText (int prod, int need)
	{
		int perTurn = prod - need;
		std::string text = std::to_string (need) + " (";
		if (perTurn > 0) text += "+";
		text += std::to_string (perTurn) + " / " + lngPack.i18n ("Comp~Turn_5") + ")";
		return text;
	}
} // namespace

//------------------------------------------------------------------------------
cWindowResourceDistribution::cWindowResourceDistribution (const cBuilding& building_, std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindow (LoadPCX (GFXOD_MINEMANAGER)),
	building (building_)
{
	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 11), getPosition() + cPosition (getArea().getMaxCorner().x(), 11 + 10)), lngPack.i18n ("Title~Mine"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	auto turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (525, 16), cPosition (525 + 60, 16 + 10)));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	for (size_t i = 0; i < 3; ++i)
	{
		metalBars[i] = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 70 + 37 * i), getPosition() + cPosition (174 + 240, 70 + 37 * i + 30)), 0, 100, eResourceBarType::Metal, eOrientationType::Horizontal);
		oilBars[i] = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 190 + 37 * i), getPosition() + cPosition (174 + 240, 190 + 37 * i + 30)), 0, 100, eResourceBarType::Oil, eOrientationType::Horizontal);
		goldBars[i] = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 310 + 37 * i), getPosition() + cPosition (174 + 240, 310 + 37 * i + 30)), 0, 100, eResourceBarType::Gold, eOrientationType::Horizontal);

		if (i > 0)
		{
			metalBars[i]->disable();
			oilBars[i]->disable();
			goldBars[i]->disable();
		}

		noneBars[i] = emplaceChild<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 70 + 120 * i), getPosition() + cPosition (174 + 240, 70 + 120 * i + 30)), 0, 100, eResourceBarType::Blocked, eOrientationType::Horizontal);
		noneBars[i]->disable();
		noneBars[i]->setInverted (true);
		noneBars[i]->setValue (0);

		std::string resourceName;
		if (i == 0)
			resourceName = lngPack.i18n ("Title~Metal");
		else if (i == 1)
			resourceName = lngPack.i18n ("Title~Oil");
		else
			resourceName = lngPack.i18n ("Title~Gold");

		resourceLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 78 + 121 * i), getPosition() + cPosition (40 + 80, 78 + 121 * i + 10)), resourceName, eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
		usageLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 78 + 37 + 121 * i), getPosition() + cPosition (40 + 80, 78 + 37 + 121 * i + 10)), lngPack.i18n ("Others~Usage_7"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
		reserveLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 78 + 37 * 2 + 121 * i), getPosition() + cPosition (40 + 80, 78 + 37 * 2 + 121 * i + 10)), lngPack.i18n ("Comp~Reserve"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

		auto decreaseButton = emplaceChild<cPushButton> (getPosition() + cPosition (139, 70 + 120 * i), ePushButtonType::ArrowLeftBig);
		signalConnectionManager.connect (decreaseButton->clicked, [this, i]() {
			if (i == 0)
				metalBars[0]->decrease (1);
			else if (i == 1)
				oilBars[0]->decrease (1);
			else if (i == 2)
				goldBars[0]->decrease (1);
		});
		auto increaseButton = emplaceChild<cPushButton> (getPosition() + cPosition (421, 70 + 120 * i), ePushButtonType::ArrowRightBig);
		signalConnectionManager.connect (increaseButton->clicked, [this, i]() {
			if (i == 0 && metalBars[0]->getValue() + noneBars[i]->getValue() < metalBars[0]->getMaxValue())
				metalBars[0]->increase (1);
			else if (i == 1 && oilBars[0]->getValue() + noneBars[i]->getValue() < oilBars[0]->getMaxValue())
				oilBars[0]->increase (1);
			else if (i == 2 && goldBars[0]->getValue() + noneBars[i]->getValue() < goldBars[0]->getMaxValue())
				goldBars[0]->increase (1);
		});
	}

	// add labels after resource bars so that they will be drawn
	// above the resource bars.
	for (size_t i = 0; i < 3; ++i)
	{
		metalLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (174, 78 + 37 * i), getPosition() + cPosition (174 + 240, 78 + 37 * i + 15)), "Metal", eUnicodeFontType::LatinBig, eAlignmentType::CenterHorizontal);
		oilLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (174, 198 + 37 * i), getPosition() + cPosition (174 + 240, 198 + 37 * i + 15)), "Oil", eUnicodeFontType::LatinBig, eAlignmentType::CenterHorizontal);
		goldLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (174, 318 + 37 * i), getPosition() + cPosition (174 + 240, 318 + 37 * i + 15)), "Gold", eUnicodeFontType::LatinBig, eAlignmentType::CenterHorizontal);

		// disable the label so that they will not receive any mouse events
		metalLabels[i]->disable();
		oilLabels[i]->disable();
		goldLabels[i]->disable();
	}

	// reset to zero first as 'FixedMax' depends on other bars
	metalBars[0]->setValue (0);
	oilBars[0]->setValue (0);
	goldBars[0]->setValue (0);

	setBarValues();
	setBarLabels();

	const sMiningResource& prod = building.subBase->getProd();
	metalBars[0]->setValue (prod.metal);
	oilBars[0]->setValue (prod.oil);
	goldBars[0]->setValue (prod.oil);

	setBarValues();
	setBarLabels();

	signalConnectionManager.connect (metalBars[0]->valueChanged, [this]() { handleMetalChanged(); });
	signalConnectionManager.connect (oilBars[0]->valueChanged, [this]() { handleOilChanged(); });
	signalConnectionManager.connect (goldBars[0]->valueChanged, [this]() { handleGoldChanged(); });

	doneButton = emplaceChild<cPushButton> (getPosition() + cPosition (514, 430), ePushButtonType::Huge, lngPack.i18n ("Others~Done"));
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [this]() { done(); });

	//close window, when the mine, from which this menu was called, gets destroyed.
	signalConnectionManager.connect (building.destroyed, [this]() { closeOnUnitDestruction(); });
	//update subbase values, when any other building in the subbase gets destroyed
	if (building.getOwner()) signalConnectionManager.connect (building.getOwner()->base.onSubbaseConfigurationChanged, [this] (const std::vector<cBuilding*>& buildings) { updateOnSubbaseChanged (buildings); });
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::retranslate()
{
	cWindow::retranslate();

	titleLabel->setText (lngPack.i18n ("Title~Mine"));

	resourceLabels[0]->setText (lngPack.i18n ("Title~Metal"));
	resourceLabels[1]->setText (lngPack.i18n ("Title~Oil"));
	resourceLabels[2]->setText (lngPack.i18n ("Title~Gold"));
	for (size_t i = 0; i < 3; ++i)
	{
		usageLabels[i]->setText (lngPack.i18n ("Others~Usage_7"));
		reserveLabels[i]->setText (lngPack.i18n ("Comp~Reserve"));
	}
	setBarLabels();

	doneButton->setText (lngPack.i18n ("Others~Done"));
}

sMiningResource cWindowResourceDistribution::getProduction() const
{
	return {metalBars[0]->getValue(), oilBars[0]->getValue(), goldBars[0]->getValue()};
}

void cWindowResourceDistribution::updateOnSubbaseChanged (const std::vector<cBuilding*>& buildings)
{
	if (building.subBase == nullptr || !Contains (buildings, &building)) return;

	// reset to zero first as 'FixedMax' depends on other bars
	metalBars[0]->setValue (0);
	oilBars[0]->setValue (0);
	goldBars[0]->setValue (0);

	const sMiningResource& prod = building.subBase->getProd();
	metalBars[0]->setValue (prod.metal);
	oilBars[0]->setValue (prod.oil);
	goldBars[0]->setValue (prod.oil);
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::setBarLabels()
{
	const auto& needed = building.subBase->getResourcesNeeded();
	const auto& stored = building.subBase->getResourcesStored();
	const auto& prod = getProduction();

	metalLabels[0]->setText (std::to_string (prod.metal));
	metalLabels[1]->setText (secondBarText (prod.metal, needed.metal));
	metalLabels[2]->setText (std::to_string (stored.metal));

	oilLabels[0]->setText (std::to_string (prod.oil));
	oilLabels[1]->setText (secondBarText (prod.oil, needed.oil));
	oilLabels[2]->setText (std::to_string (stored.oil));

	goldLabels[0]->setText (std::to_string (prod.gold));
	goldLabels[1]->setText (secondBarText (prod.gold, needed.gold));
	goldLabels[2]->setText (std::to_string (stored.gold));
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::setBarValues()
{
	const sMiningResource& needed = building.subBase->getResourcesNeeded();
	const sMiningResource& maxNeeded = building.subBase->getMaxResourcesNeeded();
	const sMiningResource& stored = building.subBase->getResourcesStored();
	const sMiningResource& maxStored = building.subBase->getMaxResourcesStored();
	const sMiningResource& maxAllowed = computeMaxAllowedProduction (*building.subBase, getProduction());
	const sMiningResource maxProd = building.subBase->getMaxProd();

	metalBars[0]->setMaxValue (maxProd.metal);
	metalBars[0]->setFixedMaxValue (maxAllowed.metal);
	metalBars[1]->setMaxValue (maxNeeded.metal);
	metalBars[1]->setValue (needed.metal);
	metalBars[2]->setMaxValue (maxStored.metal);
	metalBars[2]->setValue (stored.metal);

	noneBars[0]->setMaxValue (maxProd.metal);
	noneBars[0]->setValue (maxProd.metal - maxAllowed.metal);

	oilBars[0]->setMaxValue (maxProd.oil);
	oilBars[0]->setFixedMaxValue (maxAllowed.oil);
	oilBars[1]->setMaxValue (maxNeeded.oil);
	oilBars[1]->setValue (needed.oil);
	oilBars[2]->setMaxValue (maxStored.oil);
	oilBars[2]->setValue (stored.oil);

	noneBars[1]->setMaxValue (maxProd.oil);
	noneBars[1]->setValue (maxProd.oil - maxAllowed.oil);

	goldBars[0]->setMaxValue (maxProd.gold);
	goldBars[0]->setFixedMaxValue (maxAllowed.gold);
	goldBars[1]->setMaxValue (maxNeeded.gold);
	goldBars[1]->setValue (needed.gold);
	goldBars[2]->setMaxValue (maxStored.gold);
	goldBars[2]->setValue (stored.gold);

	noneBars[2]->setMaxValue (maxProd.gold);
	noneBars[2]->setValue (maxProd.gold - maxAllowed.gold);
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::handleMetalChanged()
{
	if (inSignal) return;
	inSignal = true;

	if (metalBars[0]->getValue() > metalBars[0]->getMaxValue() - noneBars[0]->getValue())
	{
		metalBars[0]->setValue (metalBars[0]->getMaxValue() - noneBars[1]->getValue());
	}
	//	else
	{
		setBarValues();
		setBarLabels();
	}
	inSignal = false;
}
//------------------------------------------------------------------------------
void cWindowResourceDistribution::handleOilChanged()
{
	if (inSignal) return;
	inSignal = true;
	if (oilBars[0]->getValue() > oilBars[0]->getMaxValue() - noneBars[1]->getValue())
	{
		oilBars[0]->setValue (oilBars[0]->getMaxValue() - noneBars[1]->getValue());
	}
	//	else
	{
		setBarValues();
		setBarLabels();
	}
	inSignal = false;
}
//------------------------------------------------------------------------------
void cWindowResourceDistribution::handleGoldChanged()
{
	if (inSignal) return;
	inSignal = true;
	if (goldBars[0]->getValue() > goldBars[0]->getMaxValue() - noneBars[2]->getValue())
	{
		goldBars[0]->setValue (goldBars[0]->getMaxValue() - noneBars[2]->getValue());
	}
	setBarValues();
	setBarLabels();
	inSignal = false;
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::closeOnUnitDestruction()
{
	close();
	auto application = getActiveApplication();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Others~Unit_destroyed")));
	}
}
