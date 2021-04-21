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

#include "ui/graphical/menu/windows/windowbuildvehicles/windowbuildvehicles.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuild.h"
#include "ui/graphical/menu/widgets/special/buildspeedhandlerwidget.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/application.h"
#include "utility/language.h"
#include "resources/pcx.h"
#include "game/data/units/building.h"
#include "game/data/player/player.h"
#include "game/data/map/mapview.h"
#include "game/data/map/mapfieldview.h"

//------------------------------------------------------------------------------
cWindowBuildVehicles::cWindowBuildVehicles (const cBuilding& building_, const cMapView& map, std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cTurnTimeClock> turnTimeClock) :
	cWindowAdvancedHangar (LoadPCX (GFXOD_FAC_BUILD_SCREEN), unitsData, *building_.getOwner()),
	building (building_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (328, 12), getPosition() + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Text~Title~Build_Factory"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	auto turnTimeClockWidget = addChild (std::make_unique<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (523, 16), cPosition (523 + 65, 16 + 10))));
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));

	speedHandler = addChild (std::make_unique<cBuildSpeedHandlerWidget> (getPosition() + cPosition (292, 345)));

	selectionUnitList->resize (cPosition (154, 380));
	selectionUnitList->setItemDistance (2);

	selectionListUpButton->moveTo (getPosition() + cPosition (471, 440));
	selectionListDownButton->moveTo (getPosition() + cPosition (491, 440));

	selectedUnitList->moveTo (getPosition() + cPosition (330, 50));
	selectedUnitList->resize (cPosition (130, 232));

	selectedListUpButton->moveTo (getPosition() + cPosition (327, 293));
	selectedListDownButton->moveTo (getPosition() + cPosition (348, 293));

	backButton->moveTo (getPosition() + cPosition (300, 452));
	okButton->moveTo (getPosition() + cPosition (387, 452));

	repeatCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (447, 322), lngPack.i18n ("Text~Comp~Repeat"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::Standard));

	generateSelectionList (building, map, *unitsData);
	generateBuildList (building);

	speedHandler->setBuildSpeedIndex(building.getBuildSpeed());
	repeatCheckBox->setChecked(building.getRepeatBuild());

	signalConnectionManager.connect (building.destroyed, std::bind (&cWindowBuildVehicles::closeOnUnitDestruction, this));
}

//------------------------------------------------------------------------------
std::vector<sID> cWindowBuildVehicles::getBuildList() const
{
	std::vector<sID> result;
	for (size_t i = 0; i < getSelectedUnitsCount(); ++i)
	{
		const auto& selectedUnitItem = getSelectedUnit (i);

		result.push_back (selectedUnitItem.getUnitId());
	}
	return result;
}

//------------------------------------------------------------------------------
int cWindowBuildVehicles::getSelectedBuildSpeed() const
{
	return static_cast<int> (speedHandler->getBuildSpeedIndex());
}


//------------------------------------------------------------------------------
bool cWindowBuildVehicles::isRepeatActive() const
{
	return repeatCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void cWindowBuildVehicles::setActiveUnit (const sID& unitId)
{
	cWindowAdvancedHangar::setActiveUnit (unitId);
	if (!building.getOwner()) return;

	const auto& vehicleData = *building.getOwner()->getUnitDataCurrentVersion (unitId);
	auto selectedUnit = selectedUnitList->getSelectedItem();
	const auto remainingMetal = selectedUnit ? selectedUnit->getRemainingMetal() : -1;
	std::array<int, 3> turns;
	std::array<int, 3> costs;
	building.calcTurboBuild (turns, costs, vehicleData.getBuildCost(), remainingMetal);

	speedHandler->setValues (turns, costs);
}

//------------------------------------------------------------------------------
void cWindowBuildVehicles::generateSelectionList (const cBuilding& building, const cMapView& map, const cUnitsData& unitsData)
{
	bool select = true;
	for (const auto& unitData : unitsData.getStaticUnitsData())
	{
		if (unitData.ID.isABuilding()) continue;

		bool land = false;
		bool water = false;
		int x = building.getPosition().x() - 2;
		int y = building.getPosition().y() - 1;

		for (int j = 0; j < 12; j++)
		{
			if (j == 4 || j == 6 || j == 8)
			{
				x -= 3;
				y += 1;
			}
			else if (j == 5 || j == 7) x += 3;
			else x++;

			const cPosition position (x, y);

			if (map.isValidPosition (position) == false) continue;

			const auto& buildings = map.getField (position).getBuildings();
			auto b_it = buildings.begin();
			auto b_end = buildings.end();

			while (b_it != b_end && ((*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::Above || (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase)) ++b_it;

			if (!map.isWaterOrCoast (cPosition (x, y)) || (b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::Base)) land = true;
			else if (map.isWaterOrCoast (cPosition (x, y)) && b_it != b_end && (*b_it)->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea)
			{
				land = true;
				water = true;
				break;
			}
			else if (map.isWaterOrCoast (cPosition (x, y))) water = true;
		}

		if (unitData.factorSea > 0 && unitData.factorGround == 0 && !water) continue;
		else if (unitData.factorGround > 0 && unitData.factorSea == 0 && !land) continue;

		if (building.getStaticUnitData().canBuild != unitData.buildAs) continue;

		auto& item = addSelectionUnit (unitData.ID);

		if (select)
		{
			setSelectedSelectionItem (item);
			select = false;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowBuildVehicles::generateBuildList (const cBuilding& building)
{
	for (size_t i = 0; i != building.getBuildListSize(); ++i)
	{
		auto& item = addSelectedUnit (building.getBuildListItem (i).getType());

		item.setRemainingMetal (building.getBuildListItem (i).getRemainingMetal());
	}
}

//------------------------------------------------------------------------------
void cWindowBuildVehicles::closeOnUnitDestruction()
{
	close();
	auto application = getActiveApplication();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Others~Unit_destroyed")));
	}
}
