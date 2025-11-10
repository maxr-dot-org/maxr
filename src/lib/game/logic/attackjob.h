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

#include "game/data/map/map.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "utility/position.h"

#include <vector>

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

	template <ArchiveIn Archive>
	static std::unique_ptr<cAttackJob> createFrom (Archive& archive)
	{
		auto res = std::make_unique<cAttackJob>();
		res->serialize (archive);
		return res;
	}

	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (aggressorId);
		archive & NVP (targetPosition);
		archive & NVP (lockedTargets);
		archive & NVP (fireDir);
		archive & NVP (counter);
		archive & NVP (state);
		// clang-format on
	}

private:
	void lockTarget (const cMap& map, const cUnit& aggressor);
	void releaseTargets (const cModel& model);
	void fire (cModel& model);
	std::unique_ptr<cFx> createMuzzleFx (const cUnit& aggressor);
	void impact (cModel& model);
	void impactCluster (cModel& model);
	void impactSingle (const cPosition& position, int attackPoints, cModel& model, std::vector<cUnit*>* avoidTargets = nullptr);

private:
	int aggressorId = -1;
	cPosition targetPosition;
	std::vector<int> lockedTargets;
	EDirection fireDir = EDirection::North;
	int counter = 0;
	enum class eAJState
	{
		Rotating,
		PlayingMuzzle,
		Firing,
		Finished
	};
	eAJState state = eAJState::Rotating;
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
		if (plane->getFlightHeight() > 0 && !(attackMode & eTerrainFlag::Air)) continue;
		if (plane->getFlightHeight() == 0 && !(attackMode & eTerrainFlag::Ground)) continue;

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
	if (!targetVehicle && (attackMode & eTerrainFlag::Ground))
	{
		targetVehicle = mapField.getVehicle();
		if (targetVehicle && (targetVehicle->getStaticUnitData().isStealthOn & eTerrainFlag::Sea) && map.isWater (position) && !(attackMode & eTerrainFlag::AreaSub)) targetVehicle = nullptr;
	}

	// buildings
	if (!targetVehicle && (attackMode & eTerrainFlag::Ground))
	{
		targetBuilding = mapField.getBuilding();
		if (targetBuilding && targetBuilding->isRubble()) targetBuilding = nullptr;
	}

	if (targetVehicle) return targetVehicle;
	return targetBuilding;
}

#endif // game_logic_attackjobH
