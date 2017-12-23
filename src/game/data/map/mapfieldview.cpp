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

#include "map.h"
#include "game/data/player/player.h"

#include "mapfieldview.h"

//------------------------------------------------------------------------------
cMapFieldView::cMapFieldView(const cMapField& mapField, const sTerrain& terrain, const cPlayer* player) :
	mapField(mapField),
	player(player),
	unitsChanged(mapField.unitsChanged),
	terrain(terrain)
{}

//------------------------------------------------------------------------------
cVehicle* cMapFieldView::getVehicle() const
{
	for (const auto& vehicle : mapField.getVehicles())
	{
		if (!player || player->canSeeUnit(*vehicle, mapField, terrain))
		{
			return vehicle;
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cVehicle* cMapFieldView::getPlane() const
{
	for (const auto& vehicle : mapField.getPlanes())
	{
		if (!player || player->canSeeUnit(*vehicle, mapField, terrain))
		{
			return vehicle;
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapFieldView::getBuilding() const
{
	for (const auto& building : mapField.getBuildings())
	{
		if (!player || player->canSeeUnit(*building, mapField, terrain))
		{
			return building;
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapFieldView::getTopBuilding() const
{
	for (const auto& building : mapField.getBuildings())
	{
		if (!player || player->canSeeUnit(*building, mapField, terrain))
		{
			if ((building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_GROUND ||
				 building->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE) &&
				!building->isRubble())
			{
				return building;
			}
			else
			{
				return nullptr;
			}
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapFieldView::getBaseBuilding() const
{
	for (const auto& building : mapField.getBuildings())
	{
		if (!player || player->canSeeUnit(*building, mapField, terrain))
		{
			if (building->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_GROUND &&
				building->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE &&
				!building->isRubble())
			{
				return building;
			}
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapFieldView::getRubble() const
{
	for (const auto& building : mapField.getBuildings())
	{
		if (!player || player->canSeeUnit(*building, mapField, terrain))
		{
			if (building->isRubble())
			{
				return building;
			}
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
cBuilding* cMapFieldView::getMine() const
{
	for (const auto& building : mapField.getBuildings())
	{
		if (!player || player->canSeeUnit(*building, mapField, terrain))
		{
			if (building->getStaticUnitData().explodesOnContact)
			{
				return building;
			}
		}
	}

	return nullptr;
}

//------------------------------------------------------------------------------
const std::vector<cBuilding*> cMapFieldView::getBuildings() const
{
	if (!player)
	{
		return mapField.getBuildings();
	}

	std::vector<cBuilding*> visibleBuildings;
	for (const auto& building : mapField.getBuildings())
	{
		if (player->canSeeUnit(*building, mapField, terrain))
		{
			visibleBuildings.push_back(building);
		}
	}
	return visibleBuildings;
}

//------------------------------------------------------------------------------
const std::vector<cVehicle*> cMapFieldView::getVehicles() const
{
	if (!player)
	{
		return mapField.getVehicles();
	}

	std::vector<cVehicle*> visibleVehicles;
	for (const auto& vehicle : mapField.getVehicles())
	{
		if (player->canSeeUnit(*vehicle, mapField, terrain))
		{
			visibleVehicles.push_back(vehicle);
		}
	}
	return visibleVehicles;
}

//------------------------------------------------------------------------------
const std::vector<cVehicle*> cMapFieldView::getPlanes() const
{
	if (!player)
	{
		return mapField.getVehicles();
	}

	std::vector<cVehicle*> visibleVehicles;
	for (const auto& vehicle : mapField.getPlanes())
	{
		if (player->canSeeUnit(*vehicle, mapField, terrain))
		{
			visibleVehicles.push_back(vehicle);
		}
	}
	return visibleVehicles;
}

//------------------------------------------------------------------------------
const std::vector<cUnit*> cMapFieldView::getUnits() const
{
	std::vector<cUnit*> visibleUnits;
	if (!player)
	{
		mapField.getUnits(visibleUnits);
		return visibleUnits;
	}
 
	const auto& vehicles = mapField.getVehicles();
	visibleUnits.insert(visibleUnits.end(), vehicles.begin(), vehicles.end());
	const auto& buildings = mapField.getBuildings();
	visibleUnits.insert(visibleUnits.end(), buildings.begin(), buildings.end());
	const auto& planes = mapField.getPlanes();
	visibleUnits.insert(visibleUnits.end(), planes.begin(), planes.end());

	return visibleUnits;
}
