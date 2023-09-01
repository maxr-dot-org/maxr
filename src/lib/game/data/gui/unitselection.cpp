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

#include "unitselection.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "utility/box.h"
#include "utility/flatset.h"
#include "utility/listhelpers.h"
#include "utility/ranges.h"

#include <algorithm>
#include <cassert>

//------------------------------------------------------------------------------
bool cUnitSelection::selectUnitAt (const cMapFieldView& field, bool base)
{
	cVehicle* plane = field.getPlane();
	if (plane)
	{
		return selectUnit (*plane);
	}
	cVehicle* vehicle = field.getVehicle();
	if (vehicle && !(plane /*&& (unitMenuActive || vehicle->owner != player)*/))
	{
		return selectUnit (*vehicle);
	}
	cBuilding* topBuilding = field.getTopBuilding();
	const cVehicle* selectedVehicle = getSelectedVehicle();
	if (topBuilding && (base || ((topBuilding->getStaticUnitData().surfacePosition != eSurfacePosition::Above || !selectedVehicle) && (!field.getTopBuilding()->getStaticData().canBeLandedOn || (!selectedVehicle || selectedVehicle->getStaticUnitData().factorAir == 0)))))
	{
		return selectUnit (*topBuilding);
	}
	cBuilding* baseBuilding = field.getBaseBuilding();
	if ((base || !selectedVehicle) && baseBuilding && !baseBuilding->isRubble())
	{
		return selectUnit (*baseBuilding);
	}
	return false;
}

//------------------------------------------------------------------------------
void cUnitSelection::addSelectedUnitBack (cUnit& unit)
{
	selectedUnits.emplace_back();
	selectedUnits.back().first = &unit;
	auto& unitSignalConnectionManager = selectedUnits.back().second;
	unitSignalConnectionManager.connect (unit.destroyed, [this, &unit]() { deselectUnit (unit); });
}

//------------------------------------------------------------------------------
void cUnitSelection::addSelectedUnitFront (cUnit& unit)
{
	auto it = selectedUnits.emplace (selectedUnits.begin());
	it->first = &unit;
	auto& unitSignalConnectionManager = it->second;
	unitSignalConnectionManager.connect (unit.destroyed, [this, &unit]() { deselectUnit (unit); });
}

//------------------------------------------------------------------------------
void cUnitSelection::removeSelectedUnit (const cUnit& unit)
{
	auto iter = ranges::find_if (selectedUnits, [&unit] (const auto& entry) { return entry.first == &unit; });
	if (iter == selectedUnits.end()) return;

	selectedUnits.erase (iter);
}

//------------------------------------------------------------------------------
void cUnitSelection::removeAllSelectedUnits()
{
	selectedUnits.clear();
}

//------------------------------------------------------------------------------
bool cUnitSelection::selectVehiclesAt (const cBox<cPosition>& box, const cMapView& map, const cPlayer& player)
{
	auto oldSelectedUnit = getSelectedUnit();

	removeAllSelectedUnits();

	for (int x = box.getMinCorner().x(); x <= box.getMaxCorner().x(); ++x)
	{
		for (int y = box.getMinCorner().y(); y <= box.getMaxCorner().y(); ++y)
		{
			const cPosition position (x, y);

			cVehicle* vehicle = map.getField (position).getVehicle();
			if (!vehicle || vehicle->getOwner() != &player) vehicle = map.getField (position).getPlane();

			if (vehicle && vehicle->getOwner() == &player && !vehicle->isUnitBuildingABuilding() && !vehicle->isUnitClearing() && !vehicle->isUnitMoving())
			{
				if (vehicle == oldSelectedUnit)
				{
					addSelectedUnitFront (*vehicle);
				}
				else
				{
					addSelectedUnitBack (*vehicle);
				}
			}
		}
	}

	if (oldSelectedUnit != getSelectedUnit()) mainSelectionChanged();
	groupSelectionChanged(); // FIXME: call only when the group has really changed!
	selectionChanged();
	return false;
}

//------------------------------------------------------------------------------
bool cUnitSelection::selectUnit (cUnit& unit, bool add)
{
	if (selectedUnits.size() == 1 && selectedUnits[0].first == &unit) return false;

	if (!canSelect (&unit)) return false;

	if (!add) removeAllSelectedUnits();

	if (!isSelected (unit))
	{
		addSelectedUnitFront (unit);

		if (selectedUnits.size() == 1)
			mainSelectionChanged();
		else
			groupSelectionChanged();
		selectionChanged();

		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cUnitSelection::deselectUnit (const cUnit& unit)
{
	const auto oldSelectedUnitsCount = selectedUnits.size();
	const auto isMainUnit = !selectedUnits.empty() && selectedUnits[0].first == &unit;

	removeSelectedUnit (unit);

	if (selectedUnits.size() != oldSelectedUnitsCount)
	{
		if (isMainUnit) mainSelectionChanged();
		if (selectedUnits.size() > 0) groupSelectionChanged();
		selectionChanged();
	}
}

//------------------------------------------------------------------------------
void cUnitSelection::deselectUnits()
{
	if (selectedUnits.empty()) return;

	const auto oldSelectedUnitsCount = selectedUnits.size();

	removeAllSelectedUnits();

	if (oldSelectedUnitsCount > 0) mainSelectionChanged();
	if (oldSelectedUnitsCount > 1) groupSelectionChanged();
	selectionChanged();
}

//------------------------------------------------------------------------------
cUnit* cUnitSelection::getSelectedUnit() const
{
	return selectedUnits.empty() ? nullptr : selectedUnits[0].first;
}

//------------------------------------------------------------------------------
cVehicle* cUnitSelection::getSelectedVehicle() const
{
	return dynamic_cast<cVehicle*> (getSelectedUnit());
}

//------------------------------------------------------------------------------
cBuilding* cUnitSelection::getSelectedBuilding() const
{
	return dynamic_cast<cBuilding*> (getSelectedUnit());
}

//------------------------------------------------------------------------------
std::vector<cUnit*> cUnitSelection::getSelectedUnits() const
{
	return ranges::Transform (selectedUnits, [] (const auto& p) { return p.first; });
}

//------------------------------------------------------------------------------
std::vector<cVehicle*> cUnitSelection::getSelectedVehicles() const
{
	std::vector<cVehicle*> result;
	for (const auto& p : selectedUnits)
	{
		auto* unit = p.first;
		if (unit->isAVehicle())
		{
			result.push_back (static_cast<cVehicle*> (unit));
		}
	}
	return result;
}

//------------------------------------------------------------------------------
std::vector<cBuilding*> cUnitSelection::getSelectedBuildings() const
{
	std::vector<cBuilding*> result;
	for (const auto& p : selectedUnits)
	{
		auto* unit = p.first;
		if (unit->isABuilding())
		{
			result.push_back (static_cast<cBuilding*> (unit));
		}
	}
	return result;
}

//------------------------------------------------------------------------------
size_t cUnitSelection::getSelectedUnitsCount() const
{
	return selectedUnits.size();
}

//------------------------------------------------------------------------------
size_t cUnitSelection::getSelectedVehiclesCount() const
{
	return ranges::count_if (selectedUnits, [] (const auto& p) { return p.first->isAVehicle(); });
}

//------------------------------------------------------------------------------
size_t cUnitSelection::getSelectedBuildingsCount() const
{
	return ranges::count_if (selectedUnits, [] (const auto& p) { return p.first->isABuilding(); });
}

//------------------------------------------------------------------------------
bool cUnitSelection::isSelected (const cUnit& unit) const
{
	auto iter = ranges::find_if (selectedUnits, [&unit] (const auto& entry) { return entry.first == &unit; });
	return iter != selectedUnits.end();
}

//------------------------------------------------------------------------------
bool cUnitSelection::canSelect (const cUnit* unit) const
{
	return unit && !(unit->isABuilding() && static_cast<const cBuilding*> (unit)->isRubble());
}

//------------------------------------------------------------------------------
bool cUnitSelection::selectNextUnit (const cPlayer& player, const std::vector<unsigned int>& doneList)
{
	const auto unit = getNextUnit (player, doneList, getSelectedUnit());
	if (unit == nullptr) return false;

	return selectUnit (*unit);
}

//------------------------------------------------------------------------------
bool cUnitSelection::selectPrevUnit (const cPlayer& player, const std::vector<unsigned int>& doneList)
{
	const auto unit = getPrevUnit (player, doneList, getSelectedUnit());
	if (unit == nullptr) return false;

	return selectUnit (*unit);
}

//------------------------------------------------------------------------------
cVehicle* cUnitSelection::getNextVehicle (const cPlayer& player, const std::vector<unsigned int>& doneList, const cVehicle* start) const
{
	const auto& vehicles = player.getVehicles();
	if (vehicles.empty()) return nullptr;

	auto it = (start == nullptr) ? vehicles.begin() : vehicles.find (*start);
	if (start != nullptr && it != vehicles.end()) ++it;
	for (; it != vehicles.end(); ++it)
	{
		const cVehicle& v = **it;
		if (!ranges::contains (doneList, v.getId()) && (!v.isUnitBuildingABuilding() || v.getBuildTurns() == 0) && !v.isUnitClearing() && !v.isSentryActive() && !v.isUnitLoaded() && (v.data.getSpeed() || v.data.getShots()))
		{
			return it->get();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cUnitSelection::getNextBuilding (const cPlayer& player, const std::vector<unsigned int>& doneList, const cBuilding* start) const
{
	const auto& buildings = player.getBuildings();
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.begin() : buildings.find (*start);
	if (start != nullptr && it != buildings.end()) ++it;
	for (; it != buildings.end(); ++it)
	{
		const cBuilding& b = **it;
		const auto& unitData = b.getStaticUnitData();
		const auto& buildingData = unitData.buildingData;
		if (!ranges::contains (doneList, b.getId()) && !b.isUnitWorking() && !b.isSentryActive() && (!unitData.canBuild.empty() || b.data.getShots() || buildingData.canMineMaxRes > 0 || buildingData.convertsGold > 0 || buildingData.canResearch))
		{
			return it->get();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cUnitSelection::getNextMiningStation (const cPlayer& player, const cBuilding* start) const
{
	const auto& buildings = player.getBuildings();
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.begin() : buildings.find (*start);
	if (start != nullptr && it != buildings.end()) ++it;
	for (; it != buildings.end(); ++it)
	{
		if ((*it)->getStaticData().canMineMaxRes > 0)
		{
			return it->get();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cUnit* cUnitSelection::getNextUnit (const cPlayer& player, const std::vector<unsigned int>& doneList, cUnit* start) const
{
	if (start == nullptr || !start->getOwner() || start->getOwner()->getId() != player.getId())
	{
		cVehicle* nextVehicle = getNextVehicle (player, doneList, nullptr);
		if (nextVehicle) return nextVehicle;
		cBuilding* nextBuilding = getNextBuilding (player, doneList, nullptr);
		if (nextBuilding) return nextBuilding;
	}
	else if (start->isAVehicle())
	{
		cVehicle* nextVehicle = getNextVehicle (player, doneList, static_cast<cVehicle*> (start));
		if (nextVehicle) return nextVehicle;
		cBuilding* nextBuilding = getNextBuilding (player, doneList, nullptr);
		if (nextBuilding) return nextBuilding;
		nextVehicle = getNextVehicle (player, doneList, nullptr);
		if (nextVehicle) return nextVehicle;
	}
	else
	{
		assert (start->isABuilding());
		cBuilding* building = static_cast<cBuilding*> (start);
		cBuilding* nextBuilding = getNextBuilding (player, doneList, building);
		if (nextBuilding) return nextBuilding;
		cVehicle* nextVehicle = getNextVehicle (player, doneList, nullptr);
		if (nextVehicle) return nextVehicle;
		nextBuilding = getNextBuilding (player, doneList, nullptr);
		if (nextBuilding) return nextBuilding;
	}
	// finally, return the more recent built Mining station.
	// since list order is by increasing age, take the first in list.
	return getNextMiningStation (player, nullptr);
}

//------------------------------------------------------------------------------
cVehicle* cUnitSelection::getPrevVehicle (const cPlayer& player, const std::vector<unsigned int>& doneList, const cVehicle* start) const
{
	const auto& vehicles = player.getVehicles();
	if (vehicles.empty()) return nullptr;

	auto it = (start == nullptr) ? vehicles.end() - 1 : vehicles.find (*start);
	if (start != nullptr && it == vehicles.begin()) return nullptr;
	if (start != nullptr && it != vehicles.begin() && it != vehicles.end()) --it;
	for (; it != vehicles.end(); --it)
	{
		const cVehicle& v = **it;
		if (!ranges::contains (doneList, v.getId()) && (!v.isUnitBuildingABuilding() || v.getBuildTurns() == 0) && !v.isUnitClearing() && !v.isSentryActive() && !v.isUnitLoaded() && (v.data.getSpeed() || v.data.getShots()))
		{
			return it->get();
		}
		if (it == vehicles.begin()) break;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cUnitSelection::getPrevBuilding (const cPlayer& player, const std::vector<unsigned int>& doneList, const cBuilding* start) const
{
	const auto& buildings = player.getBuildings();
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.end() - 1 : buildings.find (*start);
	if (start != nullptr && it == buildings.begin()) return nullptr;
	if (start != nullptr && it != buildings.begin() && it != buildings.end()) --it;
	for (; it != buildings.end(); --it)
	{
		const cBuilding& b = **it;
		const auto& unitData = b.getStaticUnitData();
		const auto& buildingData = unitData.buildingData;
		if (!ranges::contains (doneList, b.getId()) && !b.isUnitWorking() && !b.isSentryActive() && (!unitData.canBuild.empty() || b.data.getShots() || buildingData.canMineMaxRes > 0 || buildingData.convertsGold > 0 || buildingData.canResearch))
		{
			return it->get();
		}
		if (it == buildings.begin()) break;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cUnitSelection::getPrevMiningStation (const cPlayer& player, const cBuilding* start) const
{
	const auto& buildings = player.getBuildings();
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.end() - 1 : buildings.find (*start);
	for (; it != buildings.end(); --it)
	{
		if ((*it)->getStaticData().canMineMaxRes > 0)
		{
			return it->get();
		}
		if (it == buildings.begin()) break;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cUnit* cUnitSelection::getPrevUnit (const cPlayer& player, const std::vector<unsigned int>& doneList, cUnit* start) const
{
	if (start == nullptr || !start->getOwner() || start->getOwner()->getId() != player.getId())
	{
		cVehicle* prevVehicle = getPrevVehicle (player, doneList, nullptr);
		if (prevVehicle) return prevVehicle;
		cBuilding* prevBuilding = getPrevBuilding (player, doneList, nullptr);
		if (prevBuilding) return prevBuilding;
	}
	else if (start->isAVehicle())
	{
		cVehicle* prevVehicle = getPrevVehicle (player, doneList, static_cast<cVehicle*> (start));
		if (prevVehicle) return prevVehicle;
		cBuilding* prevBuilding = getPrevBuilding (player, doneList, nullptr);
		if (prevBuilding) return prevBuilding;
		prevVehicle = getPrevVehicle (player, doneList, nullptr);
		if (prevVehicle) return prevVehicle;
	}
	else
	{
		assert (start->isABuilding());
		cBuilding* building = static_cast<cBuilding*> (start);
		cBuilding* prevBuilding = getPrevBuilding (player, doneList, building);
		if (prevBuilding) return prevBuilding;
		cVehicle* prevVehicle = getPrevVehicle (player, doneList, nullptr);
		if (prevVehicle) return prevVehicle;
		prevBuilding = getPrevBuilding (player, doneList, nullptr);
		if (prevBuilding) return prevBuilding;
	}
	// finally, return the more recent built Mining station.
	// since list order is by increasing age, take the first in list.
	return getNextMiningStation (player, nullptr);
}
