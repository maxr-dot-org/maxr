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

#include "windowbuildbuildings.h"

#include "game/data/player/player.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "resources/pcx.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/buildspeedhandlerwidget.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowBuildBuildings::cWindowBuildBuildings (const cVehicle& vehicle_, std::shared_ptr<const cTurnTimeClock> turnTimeClock, std::shared_ptr<const cUnitsData> unitsData) :
	cWindowHangar (LoadPCX (GFXOD_BUILD_SCREEN), unitsData, *vehicle_.getOwner()),
	vehicle (vehicle_)
{
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (328, 12), getPosition() + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Text~Title~Build_Vehicle"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	auto turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (523, 16), cPosition (523 + 65, 16 + 10)));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	speedHandler = emplaceChild<cBuildSpeedHandlerWidget> (getPosition() + cPosition (292, 345));

	selectionUnitList->resize (cPosition (154, 380));
	selectionUnitList->setItemDistance (2);

	selectionListUpButton->moveTo (getPosition() + cPosition (471, 440));
	selectionListDownButton->moveTo (getPosition() + cPosition (491, 440));

	backButton->moveTo (getPosition() + cPosition (300, 452));
	okButton->moveTo (getPosition() + cPosition (387, 452));

	if (vehicle.getStaticData().canBuildPath)
	{
		pathButton = emplaceChild<cPushButton> (getPosition() + cPosition (338, 428), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Path"), eUnicodeFontType::LatinNormal);
		signalConnectionManager.connect (pathButton->clicked, [this]() { donePath(); });
	}

	generateSelectionList (vehicle, *unitsData);

	signalConnectionManager.connect (selectionUnitClickedSecondTime, [this] (const cUnitListViewItemBuy&) { done(); });

	signalConnectionManager.connect (vehicle.destroyed, [this]() { closeOnUnitDestruction(); });
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::retranslate()
{
	cWindowHangar::retranslate();

	titleLabel->setText (lngPack.i18n ("Text~Title~Build_Vehicle"));
	if (pathButton)
	{
		pathButton->setText (lngPack.i18n ("Text~Others~Path"));
	}
}

//------------------------------------------------------------------------------
const sID* cWindowBuildBuildings::getSelectedUnitId() const
{
	return getActiveUnit();
}

//------------------------------------------------------------------------------
int cWindowBuildBuildings::getSelectedBuildSpeed() const
{
	return static_cast<int> (speedHandler->getBuildSpeedIndex());
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::setActiveUnit (const sID& unitId)
{
	cWindowHangar::setActiveUnit (unitId);

	if (!vehicle.getOwner()) return;
	const auto& buildingData = *vehicle.getOwner()->getLastUnitData (unitId);
	std::array<int, 3> turns;
	std::array<int, 3> costs;
	vehicle.calcTurboBuild (turns, costs, buildingData.getBuildCost());

	speedHandler->setValues (turns, costs);
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::generateSelectionList (const cVehicle& vehicle, const cUnitsData& unitsData)
{
	bool select = true;
	for (const auto& data : unitsData.getStaticUnitsData())
	{
		if (data.ID.isAVehicle()) continue;
		if (data.buildingData.explodesOnContact) continue;

		if (vehicle.getStaticUnitData().canBuild != data.buildAs) continue;

		auto& item = addSelectionUnit (data.ID);

		if (select)
		{
			setSelectedSelectionItem (item);
			select = false;
		}

		if (vehicle.getOwner() && vehicle.getStoredResources() < vehicle.getOwner()->getLastUnitData (data.ID)->getBuildCost()) item.markAsInsufficient();
	}
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::closeOnUnitDestruction()
{
	close();
	auto application = getActiveApplication();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Others~Unit_destroyed")));
	}
}
