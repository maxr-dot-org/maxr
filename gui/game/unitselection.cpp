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
#include "../../utility/box.h"

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
bool cUnitSelection::selectVehiclesAt (const cBox<cPosition>& box, const cMap& map, const cPlayer& player)
{
	auto oldSelectedUnit = getSelectedUnit ();

	selectedUnits.clear ();

	for (int x = box.getMinCorner ().x (); x <= box.getMaxCorner ().x (); ++x)
	{
		for (int y = box.getMinCorner ().y (); y <= box.getMaxCorner ().y (); ++y)
		{
			const cPosition position (x, y);

			cVehicle* vehicle = map.getField (position).getVehicle ();
			if (!vehicle ||vehicle->owner != &player) vehicle = map.getField (position).getPlane ();

			if (vehicle && vehicle->owner == &player && !vehicle->isUnitBuildingABuilding () && !vehicle->isUnitClearing() && !vehicle->moving)
			{
				if (vehicle == oldSelectedUnit)
				{
					selectedUnits.insert (selectedUnits.begin (), vehicle);
				}
				else selectedUnits.push_back (vehicle);
			}
		}
	}

	if (oldSelectedUnit != getSelectedUnit ()) mainSelectionChanged ();
	groupSelectionChanged (); // FIXME: call only when the group has really changed!
	selectionChanged ();
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

		if (selectedUnits.size () == 1) mainSelectionChanged ();
		else groupSelectionChanged ();
		selectionChanged ();

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cUnitSelection::deselectUnit (const cUnit& unit)
{
	auto iter = std::find (selectedUnits.begin (), selectedUnits.end (), &unit);
	if (iter == selectedUnits.end ()) return;
	selectedUnits.erase (iter);
}

//------------------------------------------------------------------------------
void cUnitSelection::deselectUnits ()
{
	const auto oldSelectedUnits = selectedUnits.size ();

	selectedUnits.clear ();

	if (oldSelectedUnits > 0) mainSelectionChanged ();
	if (oldSelectedUnits > 1) groupSelectionChanged ();
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
