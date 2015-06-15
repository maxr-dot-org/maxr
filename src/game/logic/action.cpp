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

#include "action.h"
#include "game/data/model.h"
#include "game/data/gamesettings.h"
#include "game/data/map/map.h"
#include "utility/log.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"


void cActionInitNewGame::execute(cModel& model)
{
	//TODO: clan
	//TODO: upgrades
	//TODO: place ressources
	//TODO: copy credits
	makeLanding(landingPosition, *model.getPlayer(playerNr), landingUnits, model);
}

//------------------------------------------------------------------------------
void cActionInitNewGame::makeLanding(const cPosition& landingPosition, cPlayer& player, const std::vector<sLandingUnit>& landingUnits, cModel& model)
{
	cMap& map = *model.getMap();

	// Find place for mine if bridgehead is fixed
	if (model.getGameSettings()->getBridgeheadType() == eGameSettingsBridgeheadType::Definite)
	{
		if (map.possiblePlaceBuilding(*UnitsData.specialIDSmallGen.getUnitDataOriginalVersion(), landingPosition + cPosition(-1, 0)) &&
			map.possiblePlaceBuilding(*UnitsData.specialIDMine.getUnitDataOriginalVersion(), landingPosition + cPosition(0, -1)) &&
			map.possiblePlaceBuilding(*UnitsData.specialIDMine.getUnitDataOriginalVersion(), landingPosition + cPosition(1, -1)) &&
			map.possiblePlaceBuilding(*UnitsData.specialIDMine.getUnitDataOriginalVersion(), landingPosition + cPosition(1, 0)) &&
			map.possiblePlaceBuilding(*UnitsData.specialIDMine.getUnitDataOriginalVersion(), landingPosition + cPosition(0, 0)))
		{
			// place buildings:
			model.addBuilding(landingPosition + cPosition(-1, 0), UnitsData.specialIDSmallGen, &player, true);
			model.addBuilding(landingPosition + cPosition(0, -1), UnitsData.specialIDMine, &player, true);
		}
		else
		{
			Log.write("couldn't place player start mine: " + player.getName(), cLog::eLOG_TYPE_ERROR);
		}
	}

	int iWidth = 2;
	int iHeight = 2;
	for (size_t i = 0; i != landingUnits.size(); ++i)
	{
		const sLandingUnit& landing = landingUnits[i];
		cVehicle* vehicle = landVehicle(landingPosition, iWidth, iHeight, *landing.unitID.getUnitDataOriginalVersion(&player), player, model);
		while (!vehicle)
		{
			iWidth += 2;
			iHeight += 2;
			vehicle = landVehicle(landingPosition, iWidth, iHeight, *landing.unitID.getUnitDataOriginalVersion(&player), player, model);
		}
		if (landing.cargo && vehicle)
		{
			vehicle->data.setStoredResources(landing.cargo);
		}
	}
}
//------------------------------------------------------------------------------
cVehicle* cActionInitNewGame::landVehicle(const cPosition& landingPosition, int iWidth, int iHeight, const sUnitData& unitData, cPlayer& player, cModel& model)
{
	for (int offY = -iHeight / 2; offY < iHeight / 2; ++offY)
	{
		for (int offX = -iWidth / 2; offX < iWidth / 2; ++offX)
		{
			if (!model.getMap()->possiblePlaceVehicle(unitData, landingPosition + cPosition(offX, offY), &player)) continue;

			return &model.addVehicle(landingPosition + cPosition(offX, offY), unitData.ID, &player, true);
		}
	}
	return nullptr;
}
