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

#include "ui/graphical/menu/windows/windowresourcedistribution/windowresourcedistribution.h"

#include "utility/language.h"
#include "resources/pcx.h"
#include "utility/string/toString.h"
#include "game/data/base/base.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/resourcebar.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "game/data/units/building.h"

//------------------------------------------------------------------------------
cWindowResourceDistribution::cWindowResourceDistribution (const cBuilding& building_, std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindow (LoadPCX (GFXOD_MINEMANAGER)),
	subBase (std::make_unique<cSubBase>(*building_.subBase)), // TODO: do we really need to copy the whole subBase?
	building(building_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 11), getPosition() + cPosition (getArea().getMaxCorner().x(), 11 + 10)), lngPack.i18n ("Text~Title~Mine"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	auto turnTimeClockWidget = addChild (std::make_unique<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (525, 16), cPosition (525 + 60, 16 + 10))));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	for (size_t i = 0; i < 3; ++i)
	{
		metalBars[i] = addChild (std::make_unique<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 70 + 37 * i), getPosition() + cPosition (174 + 240, 70 + 37 * i + 30)), 0, 100, eResourceBarType::Metal, eOrientationType::Horizontal));
		oilBars[i] = addChild (std::make_unique<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 190 + 37 * i), getPosition() + cPosition (174 + 240, 190 + 37 * i + 30)), 0, 100, eResourceBarType::Oil, eOrientationType::Horizontal));
		goldBars[i] = addChild (std::make_unique<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 310 + 37 * i), getPosition() + cPosition (174 + 240, 310 + 37 * i + 30)), 0, 100, eResourceBarType::Gold, eOrientationType::Horizontal));

		if (i > 0)
		{
			metalBars[i]->disable();
			oilBars[i]->disable();
			goldBars[i]->disable();
		}
		else
		{
			signalConnectionManager.connect (metalBars[i]->valueChanged, std::bind (&cWindowResourceDistribution::handleMetalChanged, this));
			signalConnectionManager.connect (oilBars[i]->valueChanged, std::bind (&cWindowResourceDistribution::handleOilChanged, this));
			signalConnectionManager.connect (goldBars[i]->valueChanged, std::bind (&cWindowResourceDistribution::handleGoldChanged, this));
		}

		noneBars[i] = addChild (std::make_unique<cResourceBar> (cBox<cPosition> (getPosition() + cPosition (174, 70 + 120 * i), getPosition() + cPosition (174 + 240, 70 + 120 * i + 30)), 0, 100, eResourceBarType::Blocked, eOrientationType::Horizontal));
		noneBars[i]->disable();
		noneBars[i]->setInverted (true);
		noneBars[i]->setValue (30);

		std::string resourceName;
		if (i == 0) resourceName = lngPack.i18n ("Text~Title~Metal");
		else if (i == 1) resourceName = lngPack.i18n ("Text~Title~Oil");
		else resourceName = lngPack.i18n ("Text~Title~Gold");

		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 78 + 121 * i), getPosition() + cPosition (40 + 80, 78 + 121 * i + 10)), resourceName, FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 78 + 37 + 121 * i), getPosition() + cPosition (40 + 80, 78 + 37 + 121 * i + 10)), lngPack.i18n ("Text~Others~Usage_7"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 78 + 37 * 2 + 121 * i), getPosition() + cPosition (40 + 80, 78 + 37 * 2 + 121 * i + 10)), lngPack.i18n ("Text~Comp~Reserve"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

		auto decreaseButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (139, 70 + 120 * i), ePushButtonType::ArrowLeftBig));
		signalConnectionManager.connect (decreaseButton->clicked, [&, i]()
		{
			if (i == 0) metalBars[0]->decrease (1);
			else if (i == 1) oilBars[0]->decrease (1);
			else if (i == 2) goldBars[0]->decrease (1);
		});
		auto increaseButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (421, 70 + 120 * i), ePushButtonType::ArrowRightBig));
		signalConnectionManager.connect (increaseButton->clicked, [&, i]()
		{
			if (i == 0) metalBars[0]->increase (1);
			else if (i == 1) oilBars[0]->increase (1);
			else if (i == 2) goldBars[0]->increase (1);
		});
	}

	// add labels after resource bars so that they will be drawn
	// above the resource bars.
	for (size_t i = 0; i < 3; ++i)
	{
		metalLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (174, 78 + 37 * i), getPosition() + cPosition (174 + 240, 78 + 37 * i + 15)), "Metal", FONT_LATIN_BIG, eAlignmentType::CenterHorizontal));
		oilLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (174, 198 + 37 * i), getPosition() + cPosition (174 + 240, 198 + 37 * i + 15)), "Oil", FONT_LATIN_BIG, eAlignmentType::CenterHorizontal));
		goldLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (174, 318 + 37 * i), getPosition() + cPosition (174 + 240, 318 + 37 * i + 15)), "Gold", FONT_LATIN_BIG, eAlignmentType::CenterHorizontal));

		// disable the label so that they will not receive any mouse events
		metalLabels[i]->disable();
		oilLabels[i]->disable();
		goldLabels[i]->disable();
	}

	auto doneButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (514, 430), ePushButtonType::Huge, lngPack.i18n ("Text~Others~Done")));
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [&]() { done(); });

	setBarLabels();
	setBarValues();

	//close window, when the mine, from which this menu was called, gets destroyed.
	signalConnectionManager.connect(building.destroyed, std::bind(&cWindowResourceDistribution::closeOnUnitDestruction, this));
	//update subbase values, when any other building in the subbase gets destroyed
	signalConnectionManager.connect(building.subBase->destroyed, std::bind(&cWindowResourceDistribution::updateOnSubbaseDestruction, this));
}

void cWindowResourceDistribution::updateOnSubbaseDestruction()
{
	signalConnectionManager.connect(building.subBase->destroyed, std::bind(&cWindowResourceDistribution::updateOnSubbaseDestruction, this));
	subBase = std::make_unique<cSubBase>(*building.subBase);
	setBarLabels();
	setBarValues();
}

//------------------------------------------------------------------------------
int cWindowResourceDistribution::getMetalProduction()
{
	return subBase->getMetalProd();
}

//------------------------------------------------------------------------------
int cWindowResourceDistribution::getOilProduction()
{
	return subBase->getOilProd();
}

//------------------------------------------------------------------------------
int cWindowResourceDistribution::getGoldProduction()
{
	return subBase->getGoldProd();
}

//------------------------------------------------------------------------------
std::string cWindowResourceDistribution::secondBarText (int prod, int need)
{
	int perTurn = prod - need;
	std::string text = iToStr (need) + " (";
	if (perTurn > 0) text += "+";
	text += iToStr (perTurn) + " / " + lngPack.i18n ("Text~Comp~Turn_5") + ")";
	return text;
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::setBarLabels()
{
	metalLabels[0]->setText (iToStr (subBase->getMetalProd()));
	metalLabels[1]->setText (secondBarText (subBase->getMetalProd(), subBase->getMetalNeed()));
	metalLabels[2]->setText (iToStr (subBase->getMetalStored()));

	oilLabels[0]->setText (iToStr (subBase->getOilProd()));
	oilLabels[1]->setText (secondBarText (subBase->getOilProd(), subBase->getOilNeed()));
	oilLabels[2]->setText (iToStr (subBase->getOilStored()));

	goldLabels[0]->setText (iToStr (subBase->getGoldProd()));
	goldLabels[1]->setText (secondBarText (subBase->getGoldProd(), subBase->getGoldNeed()));
	goldLabels[2]->setText (iToStr (subBase->getGoldStored()));
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::setBarValues()
{
	metalBars[0]->setMaxValue (subBase->getMaxMetalProd());
	metalBars[0]->setValue (subBase->getMetalProd());
	metalBars[1]->setMaxValue (subBase->getMaxMetalNeed());
	metalBars[1]->setValue (subBase->getMetalNeed());
	metalBars[2]->setMaxValue (subBase->getMaxMetalStored());
	metalBars[2]->setValue (subBase->getMetalStored());

	noneBars[0]->setMaxValue (subBase->getMaxMetalProd());
	noneBars[0]->setValue (subBase->getMaxMetalProd() - subBase->getMaxAllowedMetalProd());

	oilBars[0]->setMaxValue (subBase->getMaxOilProd());
	oilBars[0]->setValue (subBase->getOilProd());
	oilBars[1]->setMaxValue (subBase->getMaxOilNeed());
	oilBars[1]->setValue (subBase->getOilNeed());
	oilBars[2]->setMaxValue (subBase->getMaxOilStored());
	oilBars[2]->setValue (subBase->getOilStored());

	noneBars[1]->setMaxValue (subBase->getMaxOilProd());
	noneBars[1]->setValue (subBase->getMaxOilProd() - subBase->getMaxAllowedOilProd());

	goldBars[0]->setMaxValue (subBase->getMaxGoldProd());
	goldBars[0]->setValue (subBase->getGoldProd());
	goldBars[1]->setMaxValue (subBase->getMaxGoldNeed());
	goldBars[1]->setValue (subBase->getGoldNeed());
	goldBars[2]->setMaxValue (subBase->getMaxGoldStored());
	goldBars[2]->setValue (subBase->getGoldStored());

	noneBars[2]->setMaxValue (subBase->getMaxGoldProd());
	noneBars[2]->setValue (subBase->getMaxGoldProd() - subBase->getMaxAllowedGoldProd());
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::handleMetalChanged()
{
	subBase->setMetalProd (metalBars[0]->getValue());

	setBarValues();
	setBarLabels();
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::handleOilChanged()
{
	subBase->setOilProd (oilBars[0]->getValue());

	setBarValues();
	setBarLabels();
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::handleGoldChanged()
{
	subBase->setGoldProd (goldBars[0]->getValue());

	setBarValues();
	setBarLabels();
}

//------------------------------------------------------------------------------
void cWindowResourceDistribution::closeOnUnitDestruction()
{
	close();
	auto application = getActiveApplication();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Others~Unit_destroyed")));
	}
}
