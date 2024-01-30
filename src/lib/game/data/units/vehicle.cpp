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

#include "game/data/units/vehicle.h"

#include "game/data/map/map.h"
#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/logic/attackjob.h"
#include "game/logic/client.h"
#include "game/logic/fxeffects.h"
#include "game/logic/jobs/planetakeoffjob.h"
#include "game/logic/jobs/startbuildjob.h"
#include "utility/crc.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/mathtools.h"
#include "utility/random.h"

#include <cmath>

//-----------------------------------------------------------------------------
// cVehicle Class Implementation
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
cVehicle::cVehicle (unsigned int ID) :
	cUnit (nullptr, nullptr, nullptr, ID),
	buildBigSavedPosition (0, 0),
	tileMovementOffset (0, 0)
{
	DamageFXPoint = {random (7) + 26 - 3, random (7) + 26 - 3};
	refreshData();

	clearingTurnsChanged.connect ([this]() { statusChanged(); });
	buildingTurnsChanged.connect ([this]() { statusChanged(); });
	buildingTypeChanged.connect ([this]() { statusChanged(); });
	moveJobChanged.connect ([this]() { statusChanged(); });
	autoMoveJobChanged.connect ([this]() { statusChanged(); });
}

//------------------------------------------------------------------------------
cVehicle::cVehicle (const cStaticUnitData& staticData, const cDynamicUnitData& dynamicData, cPlayer* owner, unsigned int ID) :
	cUnit (&dynamicData, &staticData, owner, ID),
	buildBigSavedPosition (0, 0),
	tileMovementOffset (0, 0)
{
	DamageFXPoint = {random (7) + 26 - 3, random (7) + 26 - 3};
	refreshData();

	clearingTurnsChanged.connect ([this]() { statusChanged(); });
	buildingTurnsChanged.connect ([this]() { statusChanged(); });
	buildingTypeChanged.connect ([this]() { statusChanged(); });
	moveJobChanged.connect ([this]() { statusChanged(); });
	autoMoveJobChanged.connect ([this]() { statusChanged(); });
}

//------------------------------------------------------------------------------
cVehicle::~cVehicle()
{
}

//------------------------------------------------------------------------------
void cVehicle::proceedBuilding (cModel& model, sNewTurnPlayerReport& report)
{
	if (isUnitBuildingABuilding() == false || getBuildTurns() == 0) return;

	setStoredResources (getStoredResources() - (getBuildCosts() / getBuildTurns()));
	setBuildCosts (getBuildCosts() - (getBuildCosts() / getBuildTurns()));

	setBuildTurns (getBuildTurns() - 1);
	if (getBuildTurns() != 0) return;

	const cMap& map = *model.getMap();
	report.addUnitBuilt (getBuildingType());

	// handle pathbuilding
	// here the new building is added (if possible) and
	// the move job to the next field is generated
	// the new build event is generated in cMoveJob::endMove()
	if (bandPosition)
	{
		// Find a next position that either
		// a) is something we can't move to
		//  (in which case we cancel the path building)
		// or b) doesn't have a building type that we're trying to build.
		cPosition nextPosition (getPosition());
		bool found_next = false;

		while (!found_next && (nextPosition != *bandPosition))
		{
			// Calculate the next position in the path.
			if (getPosition().x() > bandPosition->x()) nextPosition.x()--;
			if (getPosition().x() < bandPosition->x()) nextPosition.x()++;
			if (getPosition().y() > bandPosition->y()) nextPosition.y()--;
			if (getPosition().y() < bandPosition->y()) nextPosition.y()++;
			// Can we move to this position?
			// If not, we need to kill the path building now.
			model.sideStepStealthUnit (nextPosition, *this);
			if (!map.possiblePlace (*this, nextPosition, false))
			{
				// We can't build along this path any more.
				break;
			}
			// Can we build at this next position?
			if (map.possiblePlaceBuilding (model.getUnitsData()->getStaticUnitData (getBuildingType()), nextPosition, nullptr))
			{
				// We can build here.
				found_next = true;
				break;
			}
		}

		//If we've found somewhere to move to, move there now.
		const cPosition oldPosition = getPosition();
		if (found_next && model.addMoveJob (*this, nextPosition))
		{
			getMoveJob()->resume();
			if (getOwner())
			{
				if (auto* unitData = getOwner()->getLastUnitData (getBuildingType()))
				{
					unitData->markLastVersionUsed();
				}
			}
			model.addBuilding (oldPosition, getBuildingType(), getOwner());
			setBuildingABuilding (false);
		}
		else
		{
			if (model.getUnitsData()->getStaticUnitData (getBuildingType()).surfacePosition != eSurfacePosition::Ground)
			{
				// add building immediately
				// if it doesn't require the engineer to drive away
				setBuildingABuilding (false);
				if (getOwner())
				{
					if (auto* unitData = getOwner()->getLastUnitData (getBuildingType()))
					{
						unitData->markLastVersionUsed();
					}
				}
				model.addBuilding (getPosition(), getBuildingType(), getOwner());
			}
			bandPosition.reset();
			if (getOwner()) getOwner()->buildPathInterrupted (*this);
		}
	}
	else if (model.getUnitsData()->getStaticUnitData (getBuildingType()).surfacePosition != staticData->surfacePosition)
	{
		// add building immediately
		// if it doesn't require the engineer to drive away
		setBuildingABuilding (false);
		if (getOwner())
		{
			if (auto* unitData = getOwner()->getLastUnitData (getBuildingType()))
			{
				unitData->markLastVersionUsed();
			}
		}
		model.addBuilding (getPosition(), getBuildingType(), getOwner());
	}
}

//------------------------------------------------------------------------------
void cVehicle::continuePathBuilding (cModel& model)
{
	if (!bandPosition) return;

	if (getStoredResources() >= getBuildCostsStart() && model.getMap()->possiblePlaceBuilding (model.getUnitsData()->getStaticUnitData (getBuildingType()), getPosition(), nullptr, this))
	{
		model.addJob (std::make_unique<cStartBuildJob> (*this, getPosition(), getIsBig()));
		setBuildingABuilding (true);
		setBuildCosts (getBuildCostsStart());
		setBuildTurns (getBuildTurnsStart());
	}
	else
	{
		bandPosition.reset();
		if (getOwner()) getOwner()->buildPathInterrupted (*this);
	}
}

//------------------------------------------------------------------------------
void cVehicle::proceedClearing (cModel& model)
{
	if (isUnitClearing() == false || getClearingTurns() == 0) return;

	setClearingTurns (getClearingTurns() - 1);

	if (getClearingTurns() > 0) return;

	// clearing finished
	setClearing (false);

	cMap& map = *model.getMap();
	cBuilding* rubble = map.getField (getPosition()).getRubble();
	if (isBig)
	{
		if (getOwner()) getOwner()->updateScan (*this, buildBigSavedPosition);
		map.moveVehicle (*this, buildBigSavedPosition);
	}

	setStoredResources (getStoredResources() + rubble->getRubbleValue());
	model.deleteRubble (*rubble);
}

//------------------------------------------------------------------------------
/** Initializes all unit data to its maximum values */
//------------------------------------------------------------------------------
void cVehicle::refreshData()
{
	if (staticData && staticData->doesSelfRepair)
	{
		const auto newHitPoints = data.getHitpoints() + (4 * data.getHitpointsMax() / data.getBuildCost());
		data.setHitpoints (std::min (data.getHitpointsMax(), newHitPoints));
	}

	// NOTE: according to MAX 1.04 units get their shots/movepoints back even if they are disabled
	data.setSpeed (data.getSpeedMax());
	data.setShots (std::min (data.getAmmo(), data.getShotsMax()));
}

//------------------------------------------------------------------------------
int cVehicle::getPossibleShotCountForSpeed (int speed) const
{
	if (staticData->canAttack == false) {
		return 0;
	}
	if (getStaticData().canDriveAndFire) {
		return data.getShotsMax();
	}
	const int s = speed * data.getShotsMax() / data.getSpeedMax();
	return s;
}

//------------------------------------------------------------------------------
/** Reduces the remaining speedCur and shotsCur during movement */
//------------------------------------------------------------------------------
void cVehicle::DecSpeed (int value)
{
	data.setSpeed (data.getSpeed() - value);
	data.setShots (std::min (data.getShots(), getPossibleShotCountForSpeed(data.getSpeed())));
}

//------------------------------------------------------------------------------
void cVehicle::calcTurboBuild (std::array<int, 3>& turboBuildTurns, std::array<int, 3>& turboBuildCosts, int buildCosts) const
{
	turboBuildTurns[0] = 0;
	turboBuildTurns[1] = 0;
	turboBuildTurns[2] = 0;

	// step 1x
	if (getStoredResources() >= buildCosts)
	{
		turboBuildCosts[0] = buildCosts;
		// prevent division by zero
		const auto needsMetal = staticData->needsMetal == 0 ? 1 : staticData->needsMetal;
		turboBuildTurns[0] = (int) ceilf (turboBuildCosts[0] / (float) (needsMetal));
	}

	// step 2x
	// calculate building time and costs
	int a = turboBuildCosts[0];
	int rounds = turboBuildTurns[0];
	int costs = turboBuildCosts[0];

	while (a >= 4 && getStoredResources() >= costs + 4)
	{
		rounds--;
		costs += 4;
		a -= 4;
	}

	if (rounds < turboBuildTurns[0] && rounds > 0 && turboBuildTurns[0])
	{
		turboBuildCosts[1] = costs;
		turboBuildTurns[1] = rounds;
	}

	// step 4x
	a = turboBuildCosts[1];
	rounds = turboBuildTurns[1];
	costs = turboBuildCosts[1];

	while (a >= 10 && costs < staticData->storageResMax - 2)
	{
		int inc = 24 - std::min (16, a);
		if (costs + inc > getStoredResources()) break;

		rounds--;
		costs += inc;
		a -= 16;
	}

	if (rounds < turboBuildTurns[1] && rounds > 0 && turboBuildTurns[1])
	{
		turboBuildCosts[2] = costs;
		turboBuildTurns[2] = rounds;
	}
}

//------------------------------------------------------------------------------
/** Scans for resources */
//------------------------------------------------------------------------------
bool cVehicle::doSurvey (const cMap& map)
{
	if (!getOwner()) return false;
	auto& owner = *getOwner();
	bool resourceFound = false;

	const int minx = std::max (getPosition().x() - 1, 0);
	const int maxx = std::min (getPosition().x() + 1, owner.getMapSize().x() - 1);
	const int miny = std::max (getPosition().y() - 1, 0);
	const int maxy = std::min (getPosition().y() + 1, owner.getMapSize().y() - 1);

	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const cPosition position (x, y);
			if (!owner.hasResourceExplored (position) && map.getResource (position).typ != eResourceType::None)
			{
				resourceFound = true;
			}

			owner.exploreResource (position);
		}
	}

	return resourceFound;
}

//------------------------------------------------------------------------------
/** checks, if resources can be transferred to the unit */
//------------------------------------------------------------------------------
bool cVehicle::canTransferTo (const cPosition& position, const cMapView& map) const
{
	const auto& field = map.getField (position);

	const cUnit* unit = field.getVehicle();
	if (unit)
	{
		return canTransferTo (*unit);
	}

	unit = field.getTopBuilding();
	if (unit)
	{
		return canTransferTo (*unit);
	}

	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::canTransferTo (const cUnit& unit) const
{
	if (!unit.isNextTo (getPosition()))
		return false;

	if (&unit == this)
		return false;

	if (unit.getOwner() != getOwner())
		return false;

	if (unit.isAVehicle())
	{
		const cVehicle* v = static_cast<const cVehicle*> (&unit);

		if (v->staticData->storeResType != staticData->storeResType)
			return false;

		if (v->isUnitBuildingABuilding() || v->isUnitClearing())
			return false;

		return true;
	}
	else if (unit.isABuilding())
	{
		const cBuilding* b = static_cast<const cBuilding*> (&unit);

		if (!b->subBase)
			return false;

		const sMiningResource& maxStored = b->subBase->getMaxResourcesStored();
		if (staticData->storeResType == eResourceType::Metal && maxStored.metal == 0)
			return false;

		if (staticData->storeResType == eResourceType::Oil && maxStored.oil == 0)
			return false;

		if (staticData->storeResType == eResourceType::Gold && maxStored.oil == 0)
			return false;

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::makeAttackOnThis (cModel& model, cUnit* opponentUnit, const std::string& reasonForLog) const
{
	cMapView mapView (model.getMap(), nullptr);
	const cUnit* target = cAttackJob::selectTarget (getPosition(), opponentUnit->getStaticUnitData().canAttack, mapView, getOwner());
	if (target != this) return false;

	NetLog.debug (" cVehicle: " + reasonForLog + ": attacking " + toString (getPosition()) + ", Aggressor ID: " + std::to_string (opponentUnit->iID) + ", Target ID: " + std::to_string (target->getId()));

	model.addAttackJob (*opponentUnit, getPosition());

	return true;
}

//------------------------------------------------------------------------------
bool cVehicle::makeSentryAttack (cModel& model, cUnit* sentryUnit) const
{
	cMapView mapView (model.getMap(), nullptr);
	if (sentryUnit != 0 && sentryUnit->isSentryActive() && sentryUnit->canAttackObjectAt (getPosition(), mapView, true))
	{
		if (makeAttackOnThis (model, sentryUnit, "sentry reaction"))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::inSentryRange (cModel& model)
{
	for (const auto& player : model.getPlayerList())
	{
		if (player.get() == getOwner()) continue;

		// Don't attack invisible units
		if (!player->canSeeUnit (*this, *model.getMap())) continue;

		// Check sentry type
		if (staticData->factorAir > 0 && player->hasSentriesAir (getPosition()) == 0) continue;
		// Check sentry type
		if (staticData->factorAir == 0 && player->hasSentriesGround (getPosition()) == 0) continue;

		for (const auto& vehicle : player->getVehicles())
		{
			if (makeSentryAttack (model, vehicle.get()))
				return true;
		}
		for (const auto& building : player->getBuildings())
		{
			if (makeSentryAttack (model, building.get()))
				return true;
		}
	}

	return provokeReactionFire (model);
}

//------------------------------------------------------------------------------
bool cVehicle::isOtherUnitOffendedByThis (const cModel& model, const cUnit& otherUnit) const
{
	// don't treat the cheap buildings
	// (connectors, roads, beton blocks) as offendable
	if (otherUnit.isABuilding() && model.getUnitsData()->getDynamicUnitData (otherUnit.data.getId()).getBuildCost() <= 2)
		return false;

	cMapView mapView (model.getMap(), nullptr);
	if (isInRange (otherUnit.getPosition()) && canAttackObjectAt (otherUnit.getPosition(), mapView, true, false))
	{
		// test, if this vehicle can really attack the opponentVehicle
		cUnit* target = cAttackJob::selectTarget (otherUnit.getPosition(), staticData->canAttack, mapView, getOwner());
		if (target == &otherUnit)
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::doesPlayerWantToFireOnThisVehicleAsReactionFire (const cModel& model, const cPlayer* player) const
{
	if (model.getGameSettings()->gameType == eGameSettingsGameType::Turns)
	{
		// In the turn based game style,
		// the opponent always fires on the unit if he can,
		// regardless if the unit is offending or not.
		return true;
	}
	else
	{
		// check if there is a vehicle or building of player, that is offended
		for (const auto& opponentVehicle : player->getVehicles())
		{
			if (isOtherUnitOffendedByThis (model, *opponentVehicle))
				return true;
		}
		for (const auto& opponentBuilding : player->getBuildings())
		{
			if (isOtherUnitOffendedByThis (model, *opponentBuilding))
				return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::doReactionFireForUnit (cModel& model, cUnit* opponentUnit) const
{
	cMapView mapView (model.getMap(), nullptr);
	if (opponentUnit->isSentryActive() == false && opponentUnit->isManualFireActive() == false
	    && opponentUnit->canAttackObjectAt (getPosition(), mapView, true)
	    // Possible TODO: better handling of stealth units.
	    // e.g. do reaction fire, if already detected ?
	    && (opponentUnit->isAVehicle() == false || opponentUnit->getStaticUnitData().isStealthOn == eTerrainFlag::None))
	{
		if (makeAttackOnThis (model, opponentUnit, "reaction fire"))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::doReactionFire (cModel& model, cPlayer* player) const
{
	// search a unit of the opponent, that could fire on this vehicle
	// first look for a building
	for (const auto& opponentBuilding : player->getBuildings())
	{
		if (doReactionFireForUnit (model, opponentBuilding.get()))
			return true;
	}
	for (const auto& opponentVehicle : player->getVehicles())
	{
		if (doReactionFireForUnit (model, opponentVehicle.get()))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::provokeReactionFire (cModel& model)
{
	// unit can't fire, so it can't provoke a reaction fire
	if (staticData->canAttack == false || data.getShots() <= 0 || data.getAmmo() <= 0)
		return false;

	const auto& playerList = model.getPlayerList();
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		cPlayer& player = *playerList[i];
		if (&player == getOwner())
			continue;

		// The vehicle can't be seen by the opposing player.
		// No possibility for reaction fire.
		if (!player.canSeeUnit (*this, *model.getMap()))
			continue;

		if (!doesPlayerWantToFireOnThisVehicleAsReactionFire (model, &player))
			continue;

		if (doReactionFire (model, &player))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::canExitTo (const cPosition& position, const cMapView& map, const cStaticUnitData& unitData) const
{
	if (!map.possiblePlaceVehicle (unitData, position)) return false;
	if (staticData->factorAir > 0 && (position != getPosition())) return false;
	if (!isNextTo (position)) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cVehicle::canExitTo (const cPosition& position, const cMap& map, const cStaticUnitData& unitData) const
{
	if (!map.possiblePlaceVehicle (unitData, position, getOwner())) return false;
	if (staticData->factorAir > 0 && (position != getPosition())) return false;
	if (!isNextTo (position)) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cVehicle::canLoad (const cPosition& position, const cMapView& map, bool checkPosition) const
{
	if (map.isValidPosition (position) == false) return false;

	return canLoad (map.getField (position).getVehicle(), checkPosition);
}

//------------------------------------------------------------------------------
bool cVehicle::canLoad (const cVehicle* vehicle, bool checkPosition) const
{
	if (loaded) return false;

	if (!vehicle) return false;

	if (vehicle->isUnitLoaded()) return false;

	if (storedUnits.size() >= static_cast<unsigned> (staticData->storageUnitsMax)) return false;

	if (checkPosition && !isNextTo (vehicle->getPosition())) return false;

	if (checkPosition && staticData->factorAir > 0 && (vehicle->getPosition() != getPosition())) return false;

	if (!ranges::contains (staticData->storeUnitsTypes, vehicle->getStaticData().isStorageType)) return false;

	if (vehicle->moving || vehicle->isAttacking()) return false;

	if (vehicle->getOwner() != getOwner() || vehicle->isUnitBuildingABuilding() || vehicle->isUnitClearing()) return false;

	if (vehicle->isBeeingAttacked()) return false;

	return true;
}

//------------------------------------------------------------------------------
/** Checks, if an object can get ammunition. */
//------------------------------------------------------------------------------
bool cVehicle::canSupply (const cMapView& map, const cPosition& position, eSupplyType supplyType) const
{
	if (map.isValidPosition (position) == false) return false;

	const auto& field = map.getField (position);
	if (field.getVehicle())
		return canSupply (field.getVehicle(), supplyType);
	else if (field.getPlane())
		return canSupply (field.getPlane(), supplyType);
	else if (field.getTopBuilding())
		return canSupply (field.getTopBuilding(), supplyType);

	return false;
}

//------------------------------------------------------------------------------
bool cVehicle::canSupply (const cUnit* unit, eSupplyType supplyType) const
{
	if (unit == nullptr || unit == this)
		return false;

	if (getStoredResources() <= 0)
		return false;

	if (unit->isNextTo (getPosition()) == false)
		return false;

	if (unit->isAVehicle() && unit->getStaticUnitData().factorAir > 0 && static_cast<const cVehicle*> (unit)->getFlightHeight() > 0)
		return false;

	if (unit->isAVehicle() && static_cast<const cVehicle*> (unit)->isUnitMoving())
		return false;

	if (unit->isAttacking())
		return false;

	switch (supplyType)
	{
		case eSupplyType::REARM:
			if (unit->getStaticUnitData().canAttack == false || unit->data.getAmmo() >= unit->data.getAmmoMax())
				return false;
			if (!staticData->canRearm)
				return false;
			break;
		case eSupplyType::REPAIR:
			if (unit->data.getHitpoints() >= unit->data.getHitpointsMax())
				return false;
			if (!staticData->canRepair)
				return false;
			break;
		default:
			return false;
	}

	return true;
}

//------------------------------------------------------------------------------
void cVehicle::layMine (cModel& model)
{
	if (getStoredResources() <= 0) return;

	const cMap& map = *model.getMap();
	const sID explosiveMineId = (staticData->factorSea > 0 && staticData->factorGround == 0) ? model.getUnitsData()->getSeaMineID() : model.getUnitsData()->getLandMineID();
	const auto& staticExplosiveMineData = model.getUnitsData()->getStaticUnitData (explosiveMineId);

	if (!map.possiblePlaceBuilding (staticExplosiveMineData, getPosition(), nullptr, this)) return;
	model.addBuilding (getPosition(), explosiveMineId, getOwner());
	setStoredResources (getStoredResources() - 1);

	if (getStoredResources() <= 0) setLayMines (false);
}

//------------------------------------------------------------------------------
void cVehicle::clearMine (cModel& model)
{
	const cMap& map = *model.getMap();
	cBuilding* mine = map.getField (getPosition()).getMine();

	if (!mine || mine->getOwner() != getOwner() || getStoredResources() >= staticData->storageResMax) return;

	// sea minelayer can't collect land mines and vice versa
	if (mine->getStaticUnitData().factorGround > 0 && staticData->factorGround == 0) return;
	if (mine->getStaticUnitData().factorSea > 0 && staticData->factorSea == 0) return;

	model.deleteUnit (mine);
	setStoredResources (getStoredResources() + 1);

	if (getStoredResources() >= staticData->storageResMax) setClearMines (false);

	return;
}

//------------------------------------------------------------------------------
void cVehicle::tryResetOfDetectionStateBeforeMove (const cMap& map, const std::vector<std::shared_ptr<cPlayer>>& playerList)
{
	for (const auto& player : playerList)
	{
		if (!ranges::contains (detectedInThisTurnByPlayerList, player->getId()) && !checkDetectedByPlayer (*player, map))
		{
			resetDetectedByPlayer (player.get());
		}
	}
}

//------------------------------------------------------------------------------
uint32_t cVehicle::getChecksum (uint32_t crc) const
{
	crc = cUnit::getChecksum (crc);
	crc = calcCheckSum (surveyorAutoMoveActive, crc);
	crc = calcCheckSum (bandPosition, crc);
	crc = calcCheckSum (buildBigSavedPosition, crc);
	crc = calcCheckSum (WalkFrame, crc);
	crc = calcCheckSum (tileMovementOffset, crc);
	crc = calcCheckSum (loaded, crc);
	crc = calcCheckSum (moving, crc);
	crc = calcCheckSum (isBuilding, crc);
	crc = calcCheckSum (buildingTyp, crc);
	crc = calcCheckSum (buildCosts, crc);
	crc = calcCheckSum (buildTurns, crc);
	crc = calcCheckSum (buildTurnsStart, crc);
	crc = calcCheckSum (buildCostsStart, crc);
	crc = calcCheckSum (isClearing, crc);
	crc = calcCheckSum (clearingTurns, crc);
	crc = calcCheckSum (layMines, crc);
	crc = calcCheckSum (clearMines, crc);
	crc = calcCheckSum (flightHeight, crc);
	crc = commandoData.calcCheckSum (crc);

	return crc;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
bool cVehicle::canBeStoppedViaUnitMenu() const
{
	return (moveJob != nullptr || (isUnitBuildingABuilding() && getBuildTurns() > 0) || (isUnitClearing() && getClearingTurns() > 0));
}

//------------------------------------------------------------------------------
bool cVehicle::canLand (const cMap& map) const
{
	// normal vehicles are always "landed"
	if (staticData->factorAir == 0) return true;

	//vehicle busy?
	if ((moveJob && !moveJob->isFinished()) || isAttacking()) return false;
	if (isUnitMoving()) return false;

	// landing pad there?
	const std::vector<cBuilding*>& buildings = map.getField (getPosition()).getBuildings();
	const auto b_it = ranges::find_if (buildings, [] (const auto* b) { return b->getStaticData().canBeLandedOn; });
	if (b_it == buildings.end()) return false;

	// is the landing pad already occupied?
	const std::vector<cVehicle*>& v = map.getField (getPosition()).getPlanes();
	if (ranges::any_of (v, [&] (const cVehicle* vehicle) { return vehicle->getFlightHeight() < 64 && vehicle->iID != iID; })) return false;

	// returning true before checking owner, because a stolen vehicle
	// can stay on an enemy landing pad until it is moved
	if (getFlightHeight() == 0) return true;

	if ((*b_it)->getOwner() != getOwner()) return false;

	return true;
}

//------------------------------------------------------------------------------
void cVehicle::setMoving (bool value)
{
	std::swap (moving, value);
	if (value != moving) movingChanged();
}

//------------------------------------------------------------------------------
void cVehicle::setLoaded (bool value)
{
	std::swap (loaded, value);
	if (value != loaded)
	{
		if (loaded)
			stored();
		else
			activated();
	}
}

//------------------------------------------------------------------------------
void cVehicle::setClearing (bool value)
{
	std::swap (isClearing, value);
	if (value != isClearing) clearingChanged();
}

//------------------------------------------------------------------------------
void cVehicle::setBuildingABuilding (bool value)
{
	std::swap (isBuilding, value);
	if (value != isBuilding) buildingChanged();
}

//------------------------------------------------------------------------------
void cVehicle::setLayMines (bool value)
{
	std::swap (layMines, value);
	if (value != layMines) layingMinesChanged();
}

//------------------------------------------------------------------------------
void cVehicle::setClearMines (bool value)
{
	std::swap (clearMines, value);
	if (value != clearMines) clearingMinesChanged();
}

//------------------------------------------------------------------------------
int cVehicle::getClearingTurns() const
{
	return clearingTurns;
}

//------------------------------------------------------------------------------
void cVehicle::setClearingTurns (int value)
{
	std::swap (clearingTurns, value);
	if (value != clearingTurns) clearingTurnsChanged();
}

//------------------------------------------------------------------------------
const sID& cVehicle::getBuildingType() const
{
	return buildingTyp;
}

//------------------------------------------------------------------------------
void cVehicle::setBuildingType (const sID& id)
{
	auto oldId = id;
	buildingTyp = id;
	if (buildingTyp != oldId) buildingTypeChanged();
}

//------------------------------------------------------------------------------
int cVehicle::getBuildCosts() const
{
	return buildCosts;
}

//------------------------------------------------------------------------------
void cVehicle::setBuildCosts (int value)
{
	std::swap (buildCosts, value);
	if (value != buildCosts) buildingCostsChanged();
}

//------------------------------------------------------------------------------
int cVehicle::getBuildTurns() const
{
	return buildTurns;
}

//------------------------------------------------------------------------------
void cVehicle::setBuildTurns (int value)
{
	std::swap (buildTurns, value);
	if (value != buildTurns) buildingTurnsChanged();
}

//------------------------------------------------------------------------------
int cVehicle::getBuildCostsStart() const
{
	return buildCostsStart;
}

//------------------------------------------------------------------------------
void cVehicle::setBuildCostsStart (int value)
{
	std::swap (buildCostsStart, value);
	//if (value != buildCostsStart) event();
}

//------------------------------------------------------------------------------
int cVehicle::getBuildTurnsStart() const
{
	return buildTurnsStart;
}

//------------------------------------------------------------------------------
void cVehicle::setBuildTurnsStart (int value)
{
	std::swap (buildTurnsStart, value);
	//if (value != buildTurnsStart) event();
}

//------------------------------------------------------------------------------
void cVehicle::setSurveyorAutoMoveActive (bool value)
{
	std::swap (surveyorAutoMoveActive, value);

	if (value != surveyorAutoMoveActive) autoMoveJobChanged();
}

//------------------------------------------------------------------------------
int cVehicle::getFlightHeight() const
{
	return flightHeight;
}

//------------------------------------------------------------------------------
void cVehicle::setFlightHeight (int value)
{
	value = std::min (std::max (value, 0), MAX_FLIGHT_HEIGHT);
	std::swap (flightHeight, value);
	if (flightHeight != value) flightHeightChanged();
}

//------------------------------------------------------------------------------
cMoveJob* cVehicle::getMoveJob()
{
	return moveJob;
}

//------------------------------------------------------------------------------
const cMoveJob* cVehicle::getMoveJob() const
{
	return moveJob;
}

//------------------------------------------------------------------------------
void cVehicle::setMoveJob (cMoveJob* moveJob_)
{
	std::swap (moveJob, moveJob_);
	if (moveJob != moveJob_) moveJobChanged();
}

//------------------------------------------------------------------------------
void cVehicle::triggerLandingTakeOff (cModel& model)
{
	if (canLand (*model.getMap()))
	{
		// height should be 0
		if (flightHeight > 0)
		{
			model.addJob (std::make_unique<cPlaneTakeoffJob> (*this));
		}
	}
	else
	{
		// height should be MAX
		if (flightHeight < MAX_FLIGHT_HEIGHT)
		{
			model.addJob (std::make_unique<cPlaneTakeoffJob> (*this));
		}
	}
}
