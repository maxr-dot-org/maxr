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

#ifndef game_logic_attackjobH
#define game_logic_attackjobH

#include <vector>

#include "utility/position.h"
#include "game/data/map/mapview.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"

class cMap;
class cPlayer;
class cFx;
class cUnit;
class cModel;

class cAttackJob
{
public:
	/**
	* selects a target unit from a map field, depending on the attack mode.
	*/
	template <typename MAP>
	static cUnit* selectTarget (const cPosition& position, char attackMode, const MAP& map, const cPlayer* owner);

	cAttackJob() = default;
	cAttackJob (cUnit& aggressor, const cPosition& targetPosition, const cModel& model);

	void run (cModel& model);
	bool finished() const;
	void onRemoveUnit (const cUnit& unit);

	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		archive & NVP (aggressor);
		archive & NVP (targetPosition);
		archive & NVP (lockedTargets);
		archive & NVP (fireDir);
		archive & NVP (counter);
		archive & NVP (state);
	}

private:
	int calcFireDir();
	void lockTarget (const cMap& map);
	void releaseTargets (const cModel& model);
	void fire (cModel& model);
	std::unique_ptr<cFx> createMuzzleFx();
	void impact (cModel& model);
	void impactCluster (cModel& model);
	void impactSingle (const cPosition& position, int attackPoints, cModel& model, std::vector<cUnit*>* avoidTargets = nullptr);

private:
	cUnit* aggressor = nullptr;
	cPosition targetPosition;
	std::vector<int> lockedTargets;
	int fireDir = 0;
	int counter = 0;
	enum eAJStates { S_ROTATING, S_PLAYING_MUZZLE, S_FIRING, S_FINISHED };
	eAJStates state = S_ROTATING;
};

//--------------------------------------------------------------------------
template <typename MAP>
cUnit* cAttackJob::selectTarget (const cPosition& position, char attackMode, const MAP& map, const cPlayer* owner)
{
	static_assert (std::is_same<MAP, cMap>::value || std::is_same<MAP, cMapView>::value, "Type must be cMap or cMapView");

	cVehicle* targetVehicle = nullptr;
	cBuilding* targetBuilding = nullptr;
	const auto& mapField = map.getField (position);

	//planes
	//prefer enemy planes. But select own one, if there is no enemy
	auto planes = mapField.getPlanes();
	for (cVehicle* plane : planes)
	{
		if (plane->getFlightHeight() > 0 && !(attackMode & TERRAIN_AIR))    continue;
		if (plane->getFlightHeight() == 0 && !(attackMode & TERRAIN_GROUND)) continue;

		if (targetVehicle == nullptr)
		{
			targetVehicle = plane;
		}
		else if (targetVehicle->getOwner() == owner)
		{
			if (plane->getOwner() != owner)
			{
				targetVehicle = plane;
			}
		}
	}

	// vehicles
	if (!targetVehicle && (attackMode & TERRAIN_GROUND))
	{
		targetVehicle = mapField.getVehicle();
		if (targetVehicle && (targetVehicle->getStaticUnitData().isStealthOn & TERRAIN_SEA) && map.isWater (position) && !(attackMode & AREA_SUB)) targetVehicle = nullptr;
	}

	// buildings
	if (!targetVehicle && (attackMode & TERRAIN_GROUND))
	{
		targetBuilding = mapField.getBuilding();
		if (targetBuilding && targetBuilding->isRubble()) targetBuilding = nullptr;
	}

	if (targetVehicle) return targetVehicle;
	return targetBuilding;
}

#endif // game_logic_attackjobH
