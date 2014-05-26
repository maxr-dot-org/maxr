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

#include "windowbuildvehicles.h"
#include "../../widgets/label.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/listview.h"
#include "../../widgets/checkbox.h"
#include "../../widgets/special/unitlistviewitembuy.h"
#include "../../widgets/special/unitlistviewitembuild.h"
#include "../../widgets/special/buildspeedhandlerwidget.h"
#include "../../../../pcx.h"
#include "../../../../buildings.h"
#include "../../../../player.h"
#include "../../../../map.h"

//------------------------------------------------------------------------------
cWindowBuildVehicles::cWindowBuildVehicles (const cBuilding& building_, const cMap& map) :
	cWindowAdvancedHangar (LoadPCX (GFXOD_FAC_BUILD_SCREEN), *building_.owner),
	building (building_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (328, 12), getPosition () + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Text~Title~Build_Factory"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	speedHandler = addChild (std::make_unique<cBuildSpeedHandlerWidget> (getPosition () + cPosition (292, 345)));

	selectionUnitList->resize (cPosition(154,380));
	selectionUnitList->setItemDistance (cPosition (0, 2));

	selectionListUpButton->moveTo (getPosition () + cPosition (471, 440));
	selectionListDownButton->moveTo (getPosition () + cPosition (491, 440));

	selectedUnitList->moveTo (getPosition () + cPosition (330, 50));
	selectedUnitList->resize (cPosition (130, 232));

	selectedListUpButton->moveTo (getPosition () + cPosition (327, 293));
	selectedListDownButton->moveTo (getPosition () + cPosition (348, 293));

	backButton->moveTo (getPosition () + cPosition (300, 452));
	okButton->moveTo (getPosition () + cPosition (387, 452));

	repeatCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (447, 322), lngPack.i18n ("Text~Comp~Repeat"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left, eCheckBoxType::Standard));

	generateSelectionList (building, map);
	generateBuildList (building);
}

//------------------------------------------------------------------------------
std::vector<sBuildList> cWindowBuildVehicles::getBuildList () const
{
	std::vector<sBuildList> result;
	for (size_t i = 0; i < getSelectedUnitsCount (); ++i)
	{
		const auto& selectedUnitItem = getSelectedUnit (i);

		sBuildList buildUnit;
		buildUnit.type = selectedUnitItem.getUnitId ();
		buildUnit.metall_remaining = selectedUnitItem.getRemainingMetal ();
		result.push_back (std::move (buildUnit));
	}
	return result;
}

//------------------------------------------------------------------------------
int cWindowBuildVehicles::getSelectedBuildSpeed () const
{
	return static_cast<int>(speedHandler->getBuildSpeedIndex ());
}


//------------------------------------------------------------------------------
bool cWindowBuildVehicles::isRepeatActive () const
{
	return repeatCheckBox->isChecked ();
}

//------------------------------------------------------------------------------
void cWindowBuildVehicles::setActiveUnit (const sID& unitId)
{
	cWindowAdvancedHangar::setActiveUnit (unitId);

	const auto& vehicleData = *building.owner->getUnitDataCurrentVersion (unitId);
	auto selectedUnit = selectedUnitList->getSelectedItem ();
	const auto remainingMetal = selectedUnit ? selectedUnit->getRemainingMetal() : -1;
	std::array<int, 3> turns;
	std::array<int, 3> costs;
	building.calcTurboBuild (turns, costs, vehicleData.buildCosts, remainingMetal);

	speedHandler->setValues (turns, costs);

//	setActiveUpgrades (building.owner->getUnitDataCurrentVersion (unitId));
}

//------------------------------------------------------------------------------
void cWindowBuildVehicles::generateSelectionList (const cBuilding& building, const cMap& map)
{
	bool select = true;
	for (unsigned int i = 0; i < UnitsData.getNrVehicles (); i++)
	{
		sUnitData& unitData = building.owner->VehicleData[i];
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

			const auto& buildings = map.getField (position).getBuildings ();
			auto b_it = buildings.begin ();
			auto b_end = buildings.end ();

			while (b_it != b_end && ((*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE || (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) ++b_it;

			if (!map.isWaterOrCoast (cPosition(x, y)) || (b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_BASE)) land = true;
			else if (map.isWaterOrCoast (cPosition(x, y)) && b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
			{
				land = true;
				water = true;
				break;
			}
			else if (map.isWaterOrCoast (cPosition(x, y))) water = true;
		}

		if (unitData.factorSea > 0 && unitData.factorGround == 0 && !water) continue;
		else if (unitData.factorGround > 0 && unitData.factorSea == 0 && !land) continue;

		if (building.data.canBuild != unitData.buildAs) continue;

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
	for (size_t i = 0; i != building.BuildList.size (); ++i)
	{
		auto& item = addSelectedUnit (building.BuildList[i].type);

		item.setRemainingMetal(building.BuildList[i].metall_remaining);
	}
}