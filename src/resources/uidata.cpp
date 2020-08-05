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

//------------------------------------------------------------------------------
// Globals

cGraphicsData GraphicsData;
cEffectsData EffectsData;
cResourceData ResourceData;
cUnitsData UnitsDataGlobal;
cClanData  ClanDataGlobal;
cUnitsUiData UnitsUiData;
cOtherData OtherData;

//------------------------------------------------------------------------------
cUnitsUiData::cUnitsUiData() :
	rubbleBig(new sBuildingUIData()),
	rubbleSmall(new sBuildingUIData()),
	ptr_small_beton(0),
	ptr_small_beton_org(0),
	ptr_connector(0),
	ptr_connector_org(0),
	ptr_connector_shw(0),
	ptr_connector_shw_org(0)
{}

//------------------------------------------------------------------------------
cUnitsUiData::~cUnitsUiData()
{
	delete rubbleBig;
	delete rubbleSmall;
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
const sVehicleUIData* cUnitsUiData::getVehicleUI(sID id) const
{
	for (unsigned int i = 0; i < vehicleUIs.size(); ++i)
	{
		if (vehicleUIs[i].id == id)
			return &vehicleUIs[i];
	}
	return nullptr;
}
