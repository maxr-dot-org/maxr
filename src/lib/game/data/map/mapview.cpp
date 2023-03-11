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

#include "mapview.h"

#include "game/data/map/map.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/player/player.h"

#include <cassert>
#include <set>

//------------------------------------------------------------------------------
cMapView::cMapView (std::shared_ptr<const cMap> map_, std::shared_ptr<const cPlayer> player_) :
	map (map_),
	player (player_)
{
	assert (map != nullptr);

	if (player)
	{
		connectionManager.connect (player->detectedStealthUnit, [this] (const cUnit& unit) {
			// check, if the unit was in stealth mode, to prevent double 'enemy detected' report
			if (unit.isStealthOnCurrentTerrain (map->getField (unit.getPosition()), map->staticMap->getTerrain (unit.getPosition())))
			{
				// stealth unit uncovered
				unitAppeared (unit);
			}
		});

		connectionManager.connect (player->stealthUnitDissappeared, [this] (const cUnit& unit) {
			if (player->canSeeAt (unit.getPosition()))
			{
				// unit entered stealth mode
				unitDissappeared (unit);
			}
		});

		connectionManager.connect (player->getScanMap().positionsInRange, [this] (const std::vector<cPosition>& positions) {
			// scan area of player has changed
			std::set<const cUnit*> units;
			for (const auto& position : positions)
			{
				const auto& vehicles = map->getField (position).getVehicles();
				for (const auto& unit : vehicles)
				{
					if (player->canSeeUnit (*unit, *map))
					{
						units.insert (unit);
					}
				}
				const auto& buildings = map->getField (position).getBuildings();
				for (const auto& unit : buildings)
				{
					if (player->canSeeUnit (*unit, *map))
					{
						units.insert (unit);
					}
				}
				const auto& planes = map->getField (position).getPlanes();
				for (const auto& unit : planes)
				{
					if (player->canSeeUnit (*unit, *map))
					{
						units.insert (unit);
					}
				}
			}

			for (const auto& unit : units)
			{
				unitAppeared (*unit);
			}
		});

		connectionManager.connect (player->getScanMap().positionsOutOfRange, [this] (const std::vector<cPosition>& positions) {
			// scan area of player has changed
			std::set<const cUnit*> units;
			for (const auto& position : positions)
			{
				const auto& vehicles = map->getField (position).getVehicles();
				for (const auto& unit : vehicles)
				{
					if (!player->canSeeAnyAreaUnder (*unit))
					{
						units.insert (unit);
					}
				}
				const auto& buildings = map->getField (position).getBuildings();
				for (const auto& unit : buildings)
				{
					if (!player->canSeeAnyAreaUnder (*unit))
					{
						units.insert (unit);
					}
				}
				const auto& planes = map->getField (position).getPlanes();
				for (const auto& unit : planes)
				{
					if (!player->canSeeAnyAreaUnder (*unit))
					{
						units.insert (unit);
					}
				}
			}

			for (const auto& unit : units)
			{
				unitDissappeared (*unit);
			}
		});
	}

	connectionManager.connect (map->addedUnit, [this] (const cUnit& unit) {
		if (!player || player->canSeeUnit (unit, *map))
		{
			// unit unloaded or new unit exited factory
			unitAppeared (unit);
		}
	});

	connectionManager.connect (map->movedVehicle, [this] (const cUnit& unit, const cPosition& oldPosition) {
		if (!player)
		{
			unitMoved (unit, oldPosition);
			return;
		}

		bool unitIsVisible = player->canSeeUnit (unit, *map);
		bool unitWasVisible = player->canSeeAt (oldPosition);

		if (unitIsVisible && !unitWasVisible)
		{
			// unit moved into scan area
			unitAppeared (unit);
		}
		else if (!unitIsVisible && unitWasVisible)
		{
			// unit moved out of scan area
			unitDissappeared (unit);
		}
		else if (unitIsVisible)
		{
			// unit moved within scan area
			unitMoved (unit, oldPosition);
		}
	});

	connectionManager.connect (map->removedUnit, [this] (const cUnit& unit) {
		if (!player || player->canSeeUnit (unit, *map))
		{
			// unit loaded or unit destroyed
			unitDissappeared (unit);
		}
	});

	if (player)
	{
		connectionManager.connect (player->getScanMap().changed, [this]() { scanAreaChanged(); });
	}
}

//------------------------------------------------------------------------------
bool cMapView::isValidPosition (const cPosition& position) const
{
	return map->isValidPosition (position);
}

//------------------------------------------------------------------------------
bool cMapView::isPositionVisible (const cPosition& position) const
{
	if (player)
	{
		return player->canSeeAt (position);
	}
	else
	{
		return true;
	}
}

//------------------------------------------------------------------------------
bool cMapView::isWaterOrCoast (const cPosition& position) const
{
	return map->isWaterOrCoast (position);
}

//------------------------------------------------------------------------------
bool cMapView::isWater (const cPosition& position) const
{
	return map->isWater (position);
}

//------------------------------------------------------------------------------
bool cMapView::isCoast (const cPosition& position) const
{
	return map->isCoast (position);
}

//------------------------------------------------------------------------------
bool cMapView::isBlocked (const cPosition& position) const
{
	return map->isBlocked (position);
}

//------------------------------------------------------------------------------
bool cMapView::canSeeUnit (const cUnit& unit) const
{
	return !player || player->canSeeUnit (unit, *map);
}

//------------------------------------------------------------------------------
cPosition cMapView::getSize() const
{
	return map->getSize();
}

//------------------------------------------------------------------------------
int cMapView::getOffset (const cPosition& position) const
{
	return map->getOffset (position);
}

//------------------------------------------------------------------------------
const cMapFieldView cMapView::getField (const cPosition& position) const
{
	return cMapFieldView (map->getField (position), map->staticMap->getTerrain (position), player.get());
}

//------------------------------------------------------------------------------
const sResources& cMapView::getResource (const cPosition& position) const
{
	if (!player || player->hasResourceExplored (position))
	{
		return map->getResource (position);
	}
	else
	{
		static sResources res;
		res.typ = eResourceType::None;
		return res;
	}
}

//------------------------------------------------------------------------------
bool cMapView::possiblePlace (const cVehicle& vehicle, const cPosition& position, bool ignoreMovingVehicles /*= false*/) const
{
	return map->possiblePlace (vehicle, position, player != nullptr, ignoreMovingVehicles);
}

//------------------------------------------------------------------------------
bool cMapView::possiblePlaceVehicle (const cStaticUnitData& vehicleData, const cPosition& position) const
{
	return map->possiblePlaceVehicle (vehicleData, position, player.get());
}

//------------------------------------------------------------------------------
bool cMapView::possiblePlaceBuilding (const cStaticUnitData& buildingData, const cPosition& position, const cVehicle* vehicle /*= nullptr*/) const
{
	return map->possiblePlaceBuilding (buildingData, position, player.get(), vehicle);
}
