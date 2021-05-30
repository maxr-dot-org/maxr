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

#include "uidata.h"

#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "resources/buildinguidata.h"
#include "resources/vehicleuidata.h"

//------------------------------------------------------------------------------
// Globals

cGraphicsData GraphicsData;
cEffectsData EffectsData;
cResourceData ResourceData;
cUnitsUiData UnitsUiData;
cOtherData OtherData;

//------------------------------------------------------------------------------
cUnitsUiData::cUnitsUiData() :
	rubbleBig (std::make_unique<sBuildingUIData>()),
	rubbleSmall (std::make_unique<sBuildingUIData>())
{}

//------------------------------------------------------------------------------
cUnitsUiData::~cUnitsUiData()
{
}

//------------------------------------------------------------------------------
const sBuildingUIData* cUnitsUiData::getBuildingUI(sID id) const
{
	for (unsigned int i = 0; i < buildingUIs.size(); ++i)
	{
		if (buildingUIs[i].id == id)
			return &buildingUIs[i];
	}
	return nullptr;
}

//------------------------------------------------------------------------------
const sBuildingUIData& cUnitsUiData::getBuildingUI(const cBuilding& building) const
{
	if (building.isRubble())
	{
		return building.getIsBig() ? *UnitsUiData.rubbleBig : *UnitsUiData.rubbleSmall;
	}
	return *getBuildingUI (building.getStaticUnitData().ID);
}

//------------------------------------------------------------------------------
const sVehicleUIData* cUnitsUiData::getVehicleUI(sID id) const
{
	for (unsigned int i = 0; i < vehicleUIs.size(); ++i)
	{
		if (vehicleUIs[i].id == id)
			return &vehicleUIs[i];
	}
	return nullptr;
}
