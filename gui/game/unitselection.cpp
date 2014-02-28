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

#include <algorithm>

#include "unitselection.h"

#include "../../map.h"
#include "../../vehicles.h"
#include "../../buildings.h"

//------------------------------------------------------------------------------
bool cUnitSelection::selectUnitAt (const cMapField& field, bool base)
{
	cVehicle* plane = field.getPlane ();
	if (plane && !plane->moving)
	{
		return selectUnit (*plane);
	}
	cVehicle* vehicle = field.getVehicle ();
	if (vehicle && !vehicle->moving && !(plane /*&& (unitMenuActive || vehicle->owner != player)*/))
	{
		return selectUnit (*vehicle);
	}
	cBuilding* topBuilding = field.getTopBuilding ();
	const cVehicle* selectedVehicle = getSelectedVehicle ();
	if (topBuilding && (base || ((topBuilding->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE || !selectedVehicle) && (!field.getTopBuilding ()->data.canBeLandedOn || (!selectedVehicle || selectedVehicle->data.factorAir == 0)))))
	{
		return selectUnit (*topBuilding);
	}
	cBuilding* baseBuilding = field.getBaseBuilding ();
	if ((base || !selectedVehicle) && baseBuilding && baseBuilding->owner != NULL)
	{
		return selectUnit (*baseBuilding);
	}
	return false;
}

//------------------------------------------------------------------------------
bool cUnitSelection::selectUnit (cUnit& unit, bool add)
{
	if (selectedUnits.size () == 1 && selectedUnits[0] == &unit) return false;

	if (!add) selectedUnits.clear ();

	if (!isSelected (unit))
	{
		selectedUnits.push_back (&unit);

		selectionChanged ();

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cUnitSelection::deselectUnits ()
{
	selectedUnits.clear ();

	selectionChanged ();
}

//------------------------------------------------------------------------------
cUnit* cUnitSelection::getSelectedUnit () const
{
	return selectedUnits.empty () ? nullptr : selectedUnits[0];
}

//------------------------------------------------------------------------------
cVehicle* cUnitSelection::getSelectedVehicle () const
{
	auto selectedUnit = getSelectedUnit ();
	return static_cast<cVehicle*>(selectedUnit && selectedUnit->isAVehicle () ? selectedUnit : nullptr);
}

//------------------------------------------------------------------------------
cBuilding* cUnitSelection::getSelectedBuilding () const
{
	auto selectedUnit = getSelectedUnit ();
	return static_cast<cBuilding*>(selectedUnit && selectedUnit->isABuilding () ? selectedUnit : nullptr);
}

//------------------------------------------------------------------------------
const std::vector<cUnit*>& cUnitSelection::getSelectedUnits () const
{
	return selectedUnits;
}

//------------------------------------------------------------------------------
std::vector<cVehicle*> cUnitSelection::getSelectedVehicles () const
{
	std::vector<cVehicle*> result;
	for (auto i = selectedUnits.begin (); i != selectedUnits.end (); ++i)
	{
		if ((*i)->data.ID.isAVehicle ())
		{
			result.push_back (static_cast<cVehicle*>(*i));
		}
	}
	return result;
}

//------------------------------------------------------------------------------
std::vector<cBuilding*> cUnitSelection::getSelectedBuildings () const
{
	std::vector<cBuilding*> result;
	for (auto i = selectedUnits.begin (); i != selectedUnits.end (); ++i)
	{
		if ((*i)->data.ID.isABuilding ())
		{
			result.push_back (static_cast<cBuilding*>(*i));
		}
	}
	return result;
}

//------------------------------------------------------------------------------
size_t cUnitSelection::getSelectedUnitsCount () const
{
	return selectedUnits.size ();
}

//------------------------------------------------------------------------------
size_t cUnitSelection::getSelectedVehiclesCount () const
{
	size_t result = 0;
	for (auto i = selectedUnits.begin (); i != selectedUnits.end (); ++i)
	{
		if ((*i)->data.ID.isAVehicle ()) ++result;
	}
	return result;
}

//------------------------------------------------------------------------------
size_t cUnitSelection::getSelectedBuildingsCount () const
{
	size_t result = 0;
	for (auto i = selectedUnits.begin (); i != selectedUnits.end (); ++i)
	{
		if ((*i)->data.ID.isABuilding ()) ++result;
	}
	return result;
}

//------------------------------------------------------------------------------
bool cUnitSelection::isSelected (const cUnit& unit) const
{
	return std::find (selectedUnits.begin (), selectedUnits.end (), &unit) != selectedUnits.end ();
}
