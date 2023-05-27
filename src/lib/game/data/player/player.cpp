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

#include "game/data/player/player.h"

#include "game/data/report/savedreport.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/client.h"
#include "game/logic/turncounter.h"
#include "utility/crc.h"
#include "utility/listhelpers.h"
#include "utility/ranges.h"
#include "utility/string/toString.h"

#include <cassert>
#include <cmath>

//------------------------------------------------------------------------------
void sNewTurnPlayerReport::addUnitBuilt (const sID& unitTypeId)
{
	auto iter = ranges::find_if (unitsBuilt, [unitTypeId] (const sTurnstartReport& entry) { return entry.type == unitTypeId; });
	if (iter != unitsBuilt.end())
	{
		++iter->count;
	}
	else
	{
		sTurnstartReport entry;
		entry.type = unitTypeId;
		entry.count = 1;
		unitsBuilt.push_back (entry);
	}
}

//------------------------------------------------------------------------------
// Implementation cPlayer class
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cPlayer::cPlayer() :
	base (*this)
{
}
//------------------------------------------------------------------------------
cPlayer::cPlayer (const cPlayerBasicData& splayer, const cUnitsData& unitsData) :
	base (*this),
	player ({splayer.getName(), splayer.getColor()}),
	id (splayer.getNr())
{
	// get the default (no clan) unit data
	dynamicUnitsData = unitsData.getDynamicUnitsData (-1);
}

//------------------------------------------------------------------------------
cPlayer::~cPlayer()
{}

//------------------------------------------------------------------------------
void cPlayer::postLoad (cModel& model)
{
	for (auto& building : getBuildings())
	{
		building->postLoad (model);
	}
	for (auto& vehicle : getVehicles())
	{
		vehicle->postLoad (model);
	}

	hasFinishedTurnChanged(); //FIXME: deserialization does not trigger signals on changed data members. But this signal is needed for the gui after loading a save game...
	refreshScanMaps();
	refreshSentryMaps();
	refreshResearchCentersWorkingOnArea();
}

//------------------------------------------------------------------------------
void cPlayer::setClan (int newClan, const cUnitsData& unitsData)
{
	if (newClan < -1)
		return;
	if (newClan > 0 && static_cast<size_t> (newClan) >= unitsData.getNrOfClans())
		return;

	clan = newClan;

	dynamicUnitsData = unitsData.getDynamicUnitsData (clan);
}

//------------------------------------------------------------------------------
void cPlayer::setCredits (int credits_)
{
	std::swap (credits, credits_);
	if (credits != credits_) creditsChanged();
}

//------------------------------------------------------------------------------
cDynamicUnitData* cPlayer::getUnitDataCurrentVersion (const sID& id)
{
	const cPlayer* constMe = this;
	return const_cast<cDynamicUnitData*> (constMe->getUnitDataCurrentVersion (id));
}

//------------------------------------------------------------------------------
const cDynamicUnitData* cPlayer::getUnitDataCurrentVersion (const sID& id) const
{
	for (const auto& data : dynamicUnitsData)
	{
		if (data.getId() == id) return &data;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/** initialize the maps */
//------------------------------------------------------------------------------
void cPlayer::initMaps (const cPosition& mapSize)
{
	this->mapSize = mapSize;
	const int size = mapSize.x() * mapSize.y();

	resourceMap.resize (size, 0);
	sentriesMapAir.resize (mapSize);
	sentriesMapGround.resize (mapSize);

	scanMap.resize (mapSize);
	detectLandMap.resize (mapSize);
	detectSeaMap.resize (mapSize);
	detectMinesMap.resize (mapSize);
}

//------------------------------------------------------------------------------
void cPlayer::addToScan (const cUnit& unit)
{
	assert (!unit.isDisabled());

	const int size = unit.getIsBig() ? 2 : 1;

	scanMap.add (unit.data.getScan(), unit.getPosition(), size);
	const auto canDetectStealthOn = unit.getStaticUnitData().canDetectStealthOn;
	if (canDetectStealthOn & eTerrainFlag::Ground)
	{
		detectLandMap.add (unit.data.getScan(), unit.getPosition(), size);
	}
	if (canDetectStealthOn & eTerrainFlag::Sea)
	{
		detectSeaMap.add (unit.data.getScan(), unit.getPosition(), size);
	}
	if (canDetectStealthOn & eTerrainFlag::AreaExpMine)
	{
		// mines can only be detected on directly adjacent fields
		detectMinesMap.add (1, unit.getPosition(), size, true);
	}
}

//------------------------------------------------------------------------------
void cPlayer::updateScan (const cUnit& unit, const cPosition& newPosition, bool newIsBig)
{
	assert (!unit.isDisabled());

	const int oldSize = unit.getIsBig() ? 2 : 1;
	const int newSize = newIsBig ? 2 : 1;
	scanMap.update (unit.data.getScan(), unit.getPosition(), newPosition, oldSize, newSize);
	const auto canDetectStealthOn = unit.getStaticUnitData().canDetectStealthOn;
	if (canDetectStealthOn & eTerrainFlag::Ground)
	{
		detectLandMap.update (unit.data.getScan(), unit.getPosition(), newPosition, oldSize, newSize);
	}
	if (canDetectStealthOn & eTerrainFlag::Sea)
	{
		detectSeaMap.update (unit.data.getScan(), unit.getPosition(), newPosition, oldSize, newSize);
	}
	if (canDetectStealthOn & eTerrainFlag::AreaExpMine)
	{
		// mines can only be detected on directly adjacent fields
		detectMinesMap.update (1, unit.getPosition(), newPosition, oldSize, newSize, true);
	}
}

//------------------------------------------------------------------------------
void cPlayer::updateScan (const cUnit& unit, int newScanRange)
{
	assert (!unit.isDisabled());

	const int size = unit.getIsBig() ? 2 : 1;
	scanMap.update (unit.data.getScan(), newScanRange, unit.getPosition(), size);
	const auto canDetectStealthOn = unit.getStaticUnitData().canDetectStealthOn;
	if (canDetectStealthOn & eTerrainFlag::Ground)
	{
		detectLandMap.update (unit.data.getScan(), newScanRange, unit.getPosition(), size);
	}
	if (canDetectStealthOn & eTerrainFlag::Sea)
	{
		detectSeaMap.update (unit.data.getScan(), newScanRange, unit.getPosition(), size);
	}
	// mine detection does not change
}

//------------------------------------------------------------------------------
void cPlayer::removeFromScan (const cUnit& unit)
{
	const int size = unit.getIsBig() ? 2 : 1;

	scanMap.remove (unit.data.getScan(), unit.getPosition(), size);
	const auto canDetectStealthOn = unit.getStaticUnitData().canDetectStealthOn;
	if (canDetectStealthOn & eTerrainFlag::Ground)
	{
		detectLandMap.remove (unit.data.getScan(), unit.getPosition(), size);
	}
	if (canDetectStealthOn & eTerrainFlag::Sea)
	{
		detectSeaMap.remove (unit.data.getScan(), unit.getPosition(), size);
	}
	if (canDetectStealthOn & eTerrainFlag::AreaExpMine)
	{
		// mines can only be detected on directly adjacent fields
		detectMinesMap.remove (1, unit.getPosition(), size, true);
	}
}

//------------------------------------------------------------------------------
void cPlayer::addUnit (std::shared_ptr<cVehicle> vehicle)
{
	vehicles.insert (std::move (vehicle));
}

//------------------------------------------------------------------------------
void cPlayer::addUnit (std::shared_ptr<cBuilding> building)
{
	buildings.insert (std::move (building));
}

//------------------------------------------------------------------------------
std::shared_ptr<cBuilding> cPlayer::removeUnit (const cBuilding& building)
{
	return buildings.extract (building);
}

//------------------------------------------------------------------------------
std::shared_ptr<cVehicle> cPlayer::removeUnit (const cVehicle& vehicle)
{
	return vehicles.extract (vehicle);
}

//------------------------------------------------------------------------------
void cPlayer::removeAllUnits()
{
	vehicles.clear();
	buildings.clear();
}

//------------------------------------------------------------------------------
cVehicle* cPlayer::getVehicleFromId (unsigned int id) const
{
	auto iter = vehicles.find (id);
	return iter == vehicles.end() ? nullptr : (*iter).get();
}

//------------------------------------------------------------------------------
cBuilding* cPlayer::getBuildingFromId (unsigned int id) const
{
	auto iter = buildings.find (id);
	return iter == buildings.end() ? nullptr : (*iter).get();
}

//------------------------------------------------------------------------------
void cPlayer::addToSentryMap (const cUnit& u)
{
	assert (u.isSentryActive());

	const int size = u.getIsBig() ? 2 : 1;
	const auto canAttack = u.getStaticUnitData().canAttack;
	if (canAttack & eTerrainFlag::Air)
	{
		sentriesMapAir.add (u.data.getRange(), u.getPosition(), size);
	}
	else if (canAttack & (eTerrainFlag::Ground | eTerrainFlag::Sea))
	{
		sentriesMapGround.add (u.data.getRange(), u.getPosition(), size);
	}
}

//------------------------------------------------------------------------------
void cPlayer::removeFromSentryMap (const cUnit& u)
{
	const int size = u.getIsBig() ? 2 : 1;
	const auto canAttack = u.getStaticUnitData().canAttack;
	if (canAttack & eTerrainFlag::Air)
	{
		sentriesMapAir.remove (u.data.getRange(), u.getPosition(), size);
	}
	else if (canAttack & (eTerrainFlag::Ground | eTerrainFlag::Sea))
	{
		sentriesMapGround.remove (u.data.getRange(), u.getPosition(), size);
	}
}

//------------------------------------------------------------------------------
void cPlayer::updateSentry (const cUnit& u, int newRange)
{
	if (!u.isSentryActive()) return;

	const int size = u.getIsBig() ? 2 : 1;
	const auto canAttack = u.getStaticUnitData().canAttack;
	if (canAttack & eTerrainFlag::Air)
	{
		sentriesMapAir.update (u.data.getRange(), newRange, u.getPosition(), size);
	}
	else if (canAttack & (eTerrainFlag::Ground | eTerrainFlag::Sea))
	{
		sentriesMapGround.update (u.data.getRange(), newRange, u.getPosition(), size);
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshSentryMaps()
{
	sentriesMapAir.reset();
	sentriesMapGround.reset();

	for (const auto& unit : vehicles)
	{
		if (unit->isSentryActive())
		{
			addToSentryMap (*unit);
		}
	}

	for (const auto& unit : buildings)
	{
		if (unit->isSentryActive())
		{
			addToSentryMap (*unit);
		}
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshScanMaps()
{
	// simply clear the maps and re-add all units would lead to a lot of
	// false triggered events (like unit detected, etc.).
	// so we use a special method, that only triggers signals on fields that
	// really changed during the refresh.

	//1 save original maps
	const auto scanMapCopy = scanMap.getMap();
	const auto detectLandMapCopy = detectLandMap.getMap();
	const auto detectSeaMapCopy = detectSeaMap.getMap();
	const auto detectMinesMapCopy = detectMinesMap.getMap();

	//2 add all units (yes they are on the scan maps twice now)
	for (const auto& vehicle : vehicles)
	{
		if (vehicle->isUnitLoaded()) continue;
		addToScan (*vehicle);
	}

	for (const auto& building : buildings)
	{
		addToScan (*building);
	}

	//3 subtract values from the saved maps
	scanMap.subtract (scanMapCopy);
	detectLandMap.subtract (detectLandMapCopy);
	detectSeaMap.subtract (detectSeaMapCopy);
	detectMinesMap.subtract (detectMinesMapCopy);
}

//------------------------------------------------------------------------------
void cPlayer::revealResource()
{
	resourceMap.fill (1);
}

//------------------------------------------------------------------------------
bool cPlayer::canSeeAnyAreaUnder (const cUnit& unit) const
{
	return ranges::any_of (unit.getPositions(), [this] (const auto& position) { return canSeeAt (position); });
}

//------------------------------------------------------------------------------
bool cPlayer::canSeeUnit (const cUnit& unit, const cMap& map) const
{
	const auto& position = unit.getPosition();
	return canSeeUnit (unit, map.getField (position), map.staticMap->getTerrain (position));
}

//------------------------------------------------------------------------------
bool cPlayer::canSeeUnit (const cUnit& unit, const cMapField& field, const sTerrain& terrain) const
{
	if (isDefeated) return true;

	if (unit.isAVehicle() && static_cast<const cVehicle*> (&unit)->isUnitLoaded()) return false;

	if (unit.getOwner() == this) return true;

	if (!canSeeAnyAreaUnder (unit)) return false;

	if (!unit.isStealthOnCurrentTerrain (field, terrain)) return true;

	return unit.isDetectedByPlayer (this);
}

//--------------------------------------------------------------------------
std::string cPlayer::resourceMapToString() const
{
	std::string str;
	str.reserve (2 * resourceMap.size());
	for (size_t i = 0; i != resourceMap.size(); ++i)
	{
		str += getHexValue (resourceMap[i]);
	}
	return str;
}

//--------------------------------------------------------------------------
void cPlayer::setResourceMapFromString (const std::string& str)
{
	for (size_t i = 0; i != resourceMap.size(); ++i)
	{
		resourceMap.set (i, getByteValue (str, 2 * i));
	}
}

//------------------------------------------------------------------------------
bool cPlayer::hasUnits() const
{
	return !vehicles.empty() || !buildings.empty();
}

//------------------------------------------------------------------------------
/** Starts a research center. */
//------------------------------------------------------------------------------
void cPlayer::startAResearch (cResearch::eResearchArea researchArea)
{
	++researchCentersWorkingTotal;
	++researchCentersWorkingOnArea[static_cast<int> (researchArea)];

	researchCentersWorkingOnAreaChanged (researchArea);
	researchCentersWorkingTotalChanged();
}

//------------------------------------------------------------------------------
/** Stops a research center. */
//------------------------------------------------------------------------------
void cPlayer::stopAResearch (cResearch::eResearchArea researchArea)
{
	--researchCentersWorkingTotal;
	if (researchCentersWorkingOnArea[static_cast<int> (researchArea)] > 0)
	{
		--researchCentersWorkingOnArea[static_cast<int> (researchArea)];
		researchCentersWorkingOnAreaChanged (researchArea);
	}
	researchCentersWorkingTotalChanged();
}

//------------------------------------------------------------------------------
/** At turn end update the research level */
//------------------------------------------------------------------------------
std::vector<cResearch::eResearchArea> cPlayer::doResearch (const cUnitsData& unitsData)
{
	std::vector<cResearch::eResearchArea> areasReachingNextLevel;

	for (int area = 0; area < cResearch::kNrResearchAreas; ++area)
	{
		if (researchCentersWorkingOnArea[area] > 0 && researchState.doResearch (researchCentersWorkingOnArea[area], static_cast<cResearch::eResearchArea> (area)))
		{
			// next level reached
			areasReachingNextLevel.push_back (cResearch::eResearchArea (area));
		}
	}
	if (!areasReachingNextLevel.empty())
	{
		upgradeUnitTypes (areasReachingNextLevel, unitsData);
	}
	return areasReachingNextLevel;
}

//------------------------------------------------------------------------------
void cPlayer::accumulateScore()
{
	int deltaScore = 0;

	for (const auto& b : buildings)
	{
		if (b->getStaticData().canScore && b->isUnitWorking())
		{
			b->points++;
			deltaScore++;
		}
	}
	pointsHistory.push_back (getScore() + deltaScore);
}

//------------------------------------------------------------------------------
int cPlayer::getNumEcoSpheres() const
{
	return ranges::count_if (buildings, [] (const auto& building) { return building->getStaticData().canScore && building->isUnitWorking(); });
}

//------------------------------------------------------------------------------
void cPlayer::changeScore (int s)
{
	pointsHistory.back() += s;
}

//------------------------------------------------------------------------------
int cPlayer::getScore (unsigned int turn) const
{
	if (pointsHistory.size() < turn)
	{
		return pointsHistory.empty() ? 0 : pointsHistory.back();
	}
	return pointsHistory[turn - 1];
}

//------------------------------------------------------------------------------
int cPlayer::getScore() const
{
	return pointsHistory.empty() ? 0 : pointsHistory.back();
}

//------------------------------------------------------------------------------
void cPlayer::upgradeUnitTypes (const std::vector<cResearch::eResearchArea>& areasReachingNextLevel, const cUnitsData& originalUnitsData)
{
	for (auto& unitData : dynamicUnitsData)
	{
		const cDynamicUnitData& originalData = originalUnitsData.getDynamicUnitData (unitData.getId(), getClan());
		for (auto researchArea : areasReachingNextLevel)
		{
			if (unitData.getId().isABuilding() && researchArea == cResearch::eResearchArea::SpeedResearch) continue;

			const int newResearchLevel = researchState.getCurResearchLevel (researchArea);
			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::eResearchArea::AttackResearch: startValue = originalData.getDamage(); break;
				case cResearch::eResearchArea::ShotsResearch: startValue = originalData.getShotsMax(); break;
				case cResearch::eResearchArea::RangeResearch: startValue = originalData.getRange(); break;
				case cResearch::eResearchArea::ArmorResearch: startValue = originalData.getArmor(); break;
				case cResearch::eResearchArea::HitpointsResearch: startValue = originalData.getHitpointsMax(); break;
				case cResearch::eResearchArea::ScanResearch: startValue = originalData.getScan(); break;
				case cResearch::eResearchArea::SpeedResearch: startValue = originalData.getSpeedMax(); break;
				case cResearch::eResearchArea::CostResearch: startValue = originalData.getBuildCost(); break;
			}

			cUpgradeCalculator::eUnitType unitType = cUpgradeCalculator::eUnitType::StandardUnit;
			if (unitData.getId().isABuilding()) unitType = cUpgradeCalculator::eUnitType::Building;
			if (originalUnitsData.getStaticUnitData (unitData.getId()).vehicleData.isHuman) unitType = cUpgradeCalculator::eUnitType::Infantry;

			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel - 10, researchArea == cResearch::eResearchArea::CostResearch ? std::make_optional (cUpgradeCalculator::eUpgradeType::Cost) : std::nullopt, unitType);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel, researchArea == cResearch::eResearchArea::CostResearch ? std::make_optional (cUpgradeCalculator::eUpgradeType::Cost) : std::nullopt, unitType);

			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::eResearchArea::AttackResearch: unitData.setDamage (unitData.getDamage() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::ShotsResearch: unitData.setShotsMax (unitData.getShotsMax() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::RangeResearch: unitData.setRange (unitData.getRange() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::ArmorResearch: unitData.setArmor (unitData.getArmor() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::HitpointsResearch: unitData.setHitpointsMax (unitData.getHitpointsMax() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::ScanResearch: unitData.setScan (unitData.getScan() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::SpeedResearch: unitData.setSpeedMax (unitData.getSpeedMax() + newResearchBonus - oldResearchBonus); break;
					case cResearch::eResearchArea::CostResearch: unitData.setBuildCost (unitData.getBuildCost() + newResearchBonus - oldResearchBonus); break;
				}
				if (researchArea != cResearch::eResearchArea::CostResearch) // don't increment the version, if the only change are the costs
				{
					unitData.makeVersionDirty();
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshResearchCentersWorkingOnArea()
{
	const auto oldResearchCentersWorkingOnArea = researchCentersWorkingOnArea;
	researchCentersWorkingOnArea = {};

	int newResearchCount = 0;
	for (const auto& building : buildings)
	{
		if (building->getStaticData().canResearch && building->isUnitWorking())
		{
			researchCentersWorkingOnArea[static_cast<int> (building->getResearchArea())] += 1;
			newResearchCount++;
		}
	}
	std::swap (researchCentersWorkingTotal, newResearchCount);

	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		if (oldResearchCentersWorkingOnArea[i] != researchCentersWorkingOnArea[i])
		{
			researchCentersWorkingOnAreaChanged ((cResearch::eResearchArea) i);
		}
	}
	if (researchCentersWorkingTotal != newResearchCount) researchCentersWorkingTotalChanged();
}

//------------------------------------------------------------------------------
void cPlayer::refreshBase (const cMap& map)
{
	base.clear();
	for (auto& building : buildings)
	{
		base.addBuilding (*building, map);
	}
}

//------------------------------------------------------------------------------
sNewTurnPlayerReport cPlayer::makeTurnStart (cModel& model)
{
	setHasFinishedTurn (false);

	base.checkTurnEnd();

	sNewTurnPlayerReport report;
	base.makeTurnStart (report);

	// reload all buildings
	for (auto& building : buildings)
	{
		if (building->isDisabled())
		{
			building->setDisabledTurns (building->getDisabledTurns() - 1);
			if (!building->isDisabled())
			{
				addToScan (*building);
				if (building->wasWorking)
				{
					building->startWork();
					building->wasWorking = false;
				}
			}
		}
		building->refreshData();
	}

	// reload all vehicles
	for (auto& vehicle : vehicles)
	{
		if (vehicle->isDisabled())
		{
			vehicle->setDisabledTurns (vehicle->getDisabledTurns() - 1);
			if (!vehicle->isDisabled())
			{
				addToScan (*vehicle);
			}
		}
		vehicle->refreshData();
		vehicle->proceedBuilding (model, report);
		vehicle->proceedClearing (model);
	}

	//just to prevent, that an error in scanmap updates have a permanent impact
	refreshScanMaps();
	refreshSentryMaps();

	// allow stealth vehicles to enter stealth mode, when they move in the new turn
	for (auto& vehicle : vehicles)
	{
		vehicle->clearDetectedInThisTurnPlayerList();
	}

	// do research:
	report.finishedResearchs = doResearch (*model.getUnitsData());

	// eco-spheres:
	accumulateScore();

	// Gun'em down:
	for (auto& vehicle : vehicles)
	{
		vehicle->inSentryRange (model);
	}
	return report;
}

//------------------------------------------------------------------------------
uint32_t cPlayer::getChecksum (uint32_t crc) const
{
	crc = player.getCheckSum (crc);
	crc = calcCheckSum (id, crc);
	crc = calcCheckSum (dynamicUnitsData, crc);
	crc = calcCheckSum (base, crc);
	crc = calcCheckSum (vehicles, crc);
	crc = calcCheckSum (buildings, crc);
	crc = calcCheckSum (landingPos, crc);
	crc = calcCheckSum (mapSize, crc);
	crc = calcCheckSum (scanMap, crc);
	crc = calcCheckSum (resourceMap, crc);
	crc = calcCheckSum (sentriesMapAir, crc);
	crc = calcCheckSum (sentriesMapGround, crc);
	crc = calcCheckSum (detectLandMap, crc);
	crc = calcCheckSum (detectSeaMap, crc);
	crc = calcCheckSum (detectMinesMap, crc);
	crc = calcCheckSum (pointsHistory, crc);
	crc = calcCheckSum (isDefeated, crc);
	crc = calcCheckSum (clan, crc);
	crc = calcCheckSum (credits, crc);
	crc = calcCheckSum (hasFinishedTurn, crc);
	crc = calcCheckSum (researchState, crc);
	crc = calcCheckSum (researchCentersWorkingOnArea, crc);
	crc = calcCheckSum (researchCentersWorkingTotal, crc);

	return crc;
}

//------------------------------------------------------------------------------
bool cPlayer::mayHaveOffensiveUnit() const
{
	const auto canAttackOrBuild = [] (const auto& unit) {
		const auto& staticUnitData = unit->getStaticUnitData();
		return staticUnitData.canAttack || !staticUnitData.canBuild.empty();
	};
	return ranges::any_of (vehicles, canAttackOrBuild) || ranges::any_of (buildings, canAttackOrBuild);
}

//------------------------------------------------------------------------------
int cPlayer::getResearchCentersWorkingOnArea (cResearch::eResearchArea area) const
{
	return researchCentersWorkingOnArea[static_cast<int> (area)];
}

//------------------------------------------------------------------------------
bool cPlayer::canSeeAt (const cPosition& position) const
{
	if (isDefeated) return true;

	return scanMap.get (position);
}

//------------------------------------------------------------------------------
void cPlayer::setHasFinishedTurn (bool value)
{
	std::swap (hasFinishedTurn, value);
	if (hasFinishedTurn != value) hasFinishedTurnChanged();
}
