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

#include "mapfieldview.h"

#include "game/data/player/player.h"
#include "map.h"
#include "utility/listhelpers.h"

namespace
{
	//--------------------------------------------------------------------------
	auto makeFilterUnitSeenByPlayer (const cPlayer& player, const cMapField& mapField, const sTerrain& terrain)
	{
		return [&] (const cUnit* unit) { return player.canSeeUnit (*unit, mapField, terrain); };
	}
} // namespace

//------------------------------------------------------------------------------
cMapFieldView::cMapFieldView (const cMapField& mapField, const sTerrain& terrain, const cPlayer* player) :
	unitsChanged (mapField.unitsChanged),
	mapField (mapField),
	terrain (terrain),
	player (player)
{}

//------------------------------------------------------------------------------
cVehicle* cMapFieldView::getVehicle() const
{
	for (const auto& vehicle : mapField.getVehicles())
	{
		if (!player || player->canSeeUnit (*vehicle, mapField, terrain))
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
		if (!player || player->canSeeUnit (*vehicle, mapField, terrain))
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
		if (!player || player->canSeeUnit (*building, mapField, terrain))
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
		if (!player || player->canSeeUnit (*building, mapField, terrain))
		{
			if ((building->getStaticUnitData().surfacePosition == eSurfacePosition::Ground || building->getStaticUnitData().surfacePosition == eSurfacePosition::Above) && !building->isRubble())
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
		if (!player || player->canSeeUnit (*building, mapField, terrain))
		{
			if (building->getStaticUnitData().surfacePosition != eSurfacePosition::Ground && building->getStaticUnitData().surfacePosition != eSurfacePosition::Above && !building->isRubble())
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
		if (!player || player->canSeeUnit (*building, mapField, terrain))
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
		if (!player || player->canSeeUnit (*building, mapField, terrain))
		{
			if (building->getStaticData().explodesOnContact)
			{
				return building;
			}
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
bool cMapFieldView::hasBridgeOrPlattform() const
{
	for (const auto& building : mapField.getBuildings())
	{
		if (!player || player->canSeeUnit (*building, mapField, terrain))
		{
			if ((building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea || building->getStaticUnitData().surfacePosition == eSurfacePosition::Base) && !building->isRubble())
			{
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
std::vector<cBuilding*> cMapFieldView::getBuildings() const
{
	if (!player)
	{
		return mapField.getBuildings();
	}
	return Filter (mapField.getBuildings(), makeFilterUnitSeenByPlayer (*player, mapField, terrain));
}

//------------------------------------------------------------------------------
std::vector<cVehicle*> cMapFieldView::getVehicles() const
{
	if (!player)
	{
		return mapField.getVehicles();
	}
	return Filter (mapField.getVehicles(), makeFilterUnitSeenByPlayer (*player, mapField, terrain));
}

//------------------------------------------------------------------------------
std::vector<cVehicle*> cMapFieldView::getPlanes() const
{
	if (!player)
	{
		return mapField.getPlanes();
	}
	return Filter (mapField.getPlanes(), makeFilterUnitSeenByPlayer (*player, mapField, terrain));
}

//------------------------------------------------------------------------------
std::vector<cUnit*> cMapFieldView::getUnits() const
{
	std::vector<cUnit*> visibleUnits = mapField.getUnits();

	if (!player)
	{
		return visibleUnits;
	}
	return Filter (visibleUnits, makeFilterUnitSeenByPlayer (*player, mapField, terrain));
}
