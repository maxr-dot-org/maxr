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
#include <cmath>

#include "game/data/player/player.h"

#include "game/data/units/building.h"
#include "game/logic/client.h"
#include "utility/listhelpers.h"
#include "netmessage.h"
#include "game/logic/server.h"
#include "game/logic/serverevents.h"
#include "game/data/units/vehicle.h"
#include "game/data/report/savedreport.h"
#include "game/logic/turnclock.h"

using namespace std;

//------------------------------------------------------------------------------
// Implementation cPlayer class
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cPlayer::cPlayer (const cPlayerBasicData& splayer_, const cUnitsData& unitsData) :
	splayer (splayer_),
	numEcos (0),
	clan (-1),
	hasFinishedTurn (false),
	isRemovedFromGame (false)
{
	// get the default (no clan) unit data
	dynamicUnitsData = unitsData.getDynamicUnitsData(-1);

	researchCentersWorkingTotal = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;
	credits = 0;

	isDefeated = false;

	splayer.nameChanged.connect ([this]() { nameChanged(); });
	splayer.colorChanged.connect ([this]() { colorChanged(); });
}

//------------------------------------------------------------------------------
cPlayer::~cPlayer()
{}

//------------------------------------------------------------------------------
void cPlayer::setClan (int newClan, const cUnitsData& unitsData)
{
	if (newClan < -1)
		return;
	if (newClan > 0 && static_cast<size_t>(newClan) >= unitsData.getNrOfClans())
		return;

	clan = newClan;

	dynamicUnitsData = unitsData.getDynamicUnitsData(clan);
}

//------------------------------------------------------------------------------
int cPlayer::getCredits() const
{
	return credits;
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
const cDynamicUnitData* cPlayer::getUnitDataCurrentVersion(const sID& id) const
{
	for (const auto &data : dynamicUnitsData)
	{
		if (data.getId() == id) return &data;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/** initialize the maps */
//------------------------------------------------------------------------------
void cPlayer::initMaps (cMap& map)
{
	mapSize = map.getSize();
	const int size = mapSize.x() * mapSize.y();
	// Scanner-Map:
	ScanMap.clear();
	ScanMap.resize (size, 0);
	// Ressource-Map
	ResourceMap.clear();
	ResourceMap.resize (size, 0);

	base.map = &map;
	// Sentry-Map:
	SentriesMapAir.clear();
	SentriesMapAir.resize (size, 0);
	SentriesMapGround.clear();
	SentriesMapGround.resize (size, 0);

	// Detect-Maps:
	DetectLandMap.clear();
	DetectLandMap.resize (size, 0);
	DetectSeaMap.clear();
	DetectSeaMap.resize (size, 0);
	DetectMinesMap.clear();
	DetectMinesMap.resize (size, 0);
}

const cPosition& cPlayer::getMapSize() const
{
	return mapSize;
}

//------------------------------------------------------------------------------
cVehicle& cPlayer::addNewVehicle (const cPosition& position, const cStaticUnitData& unitData, unsigned int uid)
{
	auto vehicle = std::make_shared<cVehicle> (unitData, *getUnitDataCurrentVersion(unitData.ID), this, uid);
	vehicle->setPosition (position);

	drawSpecialCircle (vehicle->getPosition(), vehicle->data.getScan(), ScanMap, mapSize);
	if (vehicle->getStaticUnitData().canDetectStealthOn & TERRAIN_GROUND) drawSpecialCircle (vehicle->getPosition(), vehicle->data.getScan(), DetectLandMap, mapSize);
	if (vehicle->getStaticUnitData().canDetectStealthOn & TERRAIN_SEA) drawSpecialCircle(vehicle->getPosition(), vehicle->data.getScan(), DetectSeaMap, mapSize);
	if (vehicle->getStaticUnitData().canDetectStealthOn & AREA_EXP_MINE)
	{
		const int minx = std::max (vehicle->getPosition().x() - 1, 0);
		const int maxx = std::min (vehicle->getPosition().x() + 1, mapSize.x() - 1);
		const int miny = std::max (vehicle->getPosition().y() - 1, 0);
		const int maxy = std::min (vehicle->getPosition().y() + 1, mapSize.y() - 1);
		for (int x = minx; x <= maxx; ++x)
			for (int y = miny; y <= maxy; ++y)
				DetectMinesMap[x + mapSize.x() * y] = 1;
	}

	auto result = vehicles.insert (std::move (vehicle));
	assert (result.second);

	return * (*result.first);
}

//------------------------------------------------------------------------------
cBuilding& cPlayer::addNewBuilding (const cPosition& position, const cStaticUnitData& unitData, unsigned int uid)
{
	auto building = std::make_shared<cBuilding>(&unitData, getUnitDataCurrentVersion(unitData.ID), this, uid);

	building->setPosition (position);

	if (building->data.getScan())
	{
		if (building->getIsBig()) drawSpecialCircleBig (building->getPosition(), building->data.getScan(), ScanMap, mapSize);
		else drawSpecialCircle (building->getPosition(), building->data.getScan(), ScanMap, mapSize);
	}

	auto result = buildings.insert (std::move (building));
	assert (result.second);
	return * (*result.first);
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
	auto iter = buildings.find (building);
	if (iter == buildings.end()) return nullptr;

	auto removed = *iter;
	buildings.erase (iter);
	return removed;
}

//------------------------------------------------------------------------------
std::shared_ptr<cVehicle> cPlayer::removeUnit (const cVehicle& vehicle)
{
	auto iter = vehicles.find (vehicle);
	if (iter == vehicles.end()) return nullptr;

	auto removed = *iter;
	vehicles.erase (iter);
	return removed;
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
	auto iter =  vehicles.find (id);
	return iter == vehicles.end() ? nullptr : (*iter).get();
}

//------------------------------------------------------------------------------
cBuilding* cPlayer::getBuildingFromId (unsigned int id) const
{
	auto iter = buildings.find (id);
	return iter == buildings.end() ? nullptr : (*iter).get();
}

//------------------------------------------------------------------------------
const cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>>& cPlayer::getVehicles() const
{
	return vehicles;
}

//------------------------------------------------------------------------------
const cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>>& cPlayer::getBuildings() const
{
	return buildings;
}

//------------------------------------------------------------------------------
void cPlayer::addSentry (cUnit& u)
{
	u.setSentryActive (true);
	if (u.getStaticUnitData().canAttack & TERRAIN_AIR)
	{
		drawSpecialCircle (u.getPosition(), u.data.getRange(), SentriesMapAir, mapSize);
	}
	if ((u.getStaticUnitData().canAttack & TERRAIN_GROUND) || (u.getStaticUnitData().canAttack & TERRAIN_SEA))
	{
		drawSpecialCircle (u.getPosition(), u.data.getRange(), SentriesMapGround, mapSize);
	}
}

//------------------------------------------------------------------------------
void cPlayer::deleteSentry (cUnit& u)
{
	u.setSentryActive (false);
	if (u.getStaticUnitData().canAttack & TERRAIN_AIR)
	{
		refreshSentryAir();
	}
	else if ((u.getStaticUnitData().canAttack & TERRAIN_GROUND) || (u.getStaticUnitData().canAttack & TERRAIN_SEA))
	{
		refreshSentryGround();
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshSentryAir()
{
	std::fill (SentriesMapAir.begin(), SentriesMapAir.end(), 0);

	for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
	{
		const auto& unit = *i;
		if (unit->isSentryActive() && unit->getStaticUnitData().canAttack & TERRAIN_AIR)
		{
			drawSpecialCircle (unit->getPosition(), unit->data.getRange(), SentriesMapAir, mapSize);
		}
	}

	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& unit = *i;
		if (unit->isSentryActive() && unit->getStaticUnitData().canAttack & TERRAIN_AIR)
		{
			drawSpecialCircle (unit->getPosition(), unit->data.getRange(), SentriesMapAir, mapSize);
		}
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshSentryGround()
{
	std::fill (SentriesMapGround.begin(), SentriesMapGround.end(), 0);

	for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
	{
		const auto& unit = *i;
		if (unit->isSentryActive() && ((unit->getStaticUnitData().canAttack & TERRAIN_GROUND) || (unit->getStaticUnitData().canAttack & TERRAIN_SEA)))
		{
			drawSpecialCircle (unit->getPosition(), unit->data.getRange(), SentriesMapGround, mapSize);
		}
	}
	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& unit = *i;
		if (unit->isSentryActive() && ((unit->getStaticUnitData().canAttack & TERRAIN_GROUND) || (unit->getStaticUnitData().canAttack & TERRAIN_SEA)))
		{
			drawSpecialCircle (unit->getPosition(), unit->data.getRange(), SentriesMapGround, mapSize);
		}
	}
}

//------------------------------------------------------------------------------
/** Does a scan for all units of the player */
//------------------------------------------------------------------------------
void cPlayer::doScan()
{
	if (isDefeated) return;
	std::fill (ScanMap.begin(), ScanMap.end(), 0);
	std::fill (DetectLandMap.begin(), DetectLandMap.end(), 0);
	std::fill (DetectSeaMap.begin(), DetectSeaMap.end(), 0);
	std::fill (DetectMinesMap.begin(), DetectMinesMap.end(), 0);

	// iterate the vehicle list
	for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
	{
		const auto& vp = *i;
		if (vp->isUnitLoaded()) continue;

		if (vp->isDisabled())
			ScanMap[getOffset (vp->getPosition())] = 1;
		else
		{
			if (vp->getIsBig())
				drawSpecialCircleBig (vp->getPosition(), vp->data.getScan(), ScanMap, mapSize);
			else
				drawSpecialCircle (vp->getPosition(), vp->data.getScan(), ScanMap, mapSize);

			//detection maps
			if (vp->getStaticUnitData().canDetectStealthOn & TERRAIN_GROUND) drawSpecialCircle(vp->getPosition(), vp->data.getScan(), DetectLandMap, mapSize);
			else if (vp->getStaticUnitData().canDetectStealthOn & TERRAIN_SEA) drawSpecialCircle(vp->getPosition(), vp->data.getScan(), DetectSeaMap, mapSize);
			if (vp->getStaticUnitData().canDetectStealthOn & AREA_EXP_MINE)
			{
				const int minx = std::max (vp->getPosition().x() - 1, 0);
				const int maxx = std::min (vp->getPosition().x() + 1, mapSize.x() - 1);
				const int miny = std::max (vp->getPosition().y() - 1, 0);
				const int maxy = std::min (vp->getPosition().y() + 1, mapSize.y() - 1);
				for (int x = minx; x <= maxx; ++x)
				{
					for (int y = miny; y <= maxy; ++y)
					{
						DetectMinesMap[x + mapSize.x() * y] = 1;
					}
				}
			}
		}
	}

	// iterate the building list
	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& bp = *i;
		if (bp->isDisabled())
			ScanMap[getOffset (bp->getPosition())] = 1;
		else if (bp->data.getScan())
		{
			if (bp->getIsBig())
				drawSpecialCircleBig (bp->getPosition(), bp->data.getScan(), ScanMap, mapSize);
			else
				drawSpecialCircle (bp->getPosition(), bp->data.getScan(), ScanMap, mapSize);
		}
	}
}

void cPlayer::revealMap()
{
	std::fill (ScanMap.begin(), ScanMap.end(), 1);
}

void cPlayer::revealPosition (const cPosition& position)
{
	if (position.x() < 0 || position.x() >= mapSize.x() || position.y() < 0 || position.y() >= mapSize.y()) return;

	ScanMap[getOffset (position)] = 1;
}

void cPlayer::revealResource()
{
	std::fill (ResourceMap.begin(), ResourceMap.end(), 1);
}

bool cPlayer::canSeeAnyAreaUnder (const cUnit& unit) const
{
	if (canSeeAt (unit.getPosition())) return true;
	if (!unit.getIsBig()) return false;

	return canSeeAt (unit.getPosition() + cPosition (0, 1)) ||
		   canSeeAt (unit.getPosition() + cPosition (1, 1)) ||
		   canSeeAt (unit.getPosition() + cPosition (1, 0));
}

cVehicle* cPlayer::getNextVehicle (cVehicle* start) const
{
	if (vehicles.empty()) return nullptr;

	auto it = (start == nullptr) ? vehicles.begin() : vehicles.find (*start);
	if (start != nullptr && it != vehicles.end()) ++it;
	for (; it != vehicles.end(); ++it)
	{
		if (! (*it)->isMarkedAsDone() && (! (*it)->isUnitBuildingABuilding() || (*it)->getBuildTurns() == 0)
			&& ! (*it)->isUnitClearing() && ! (*it)->isSentryActive() && ! (*it)->isUnitLoaded()
			&& ((*it)->data.getSpeed() || (*it)->data.getShots()))
		{
			return it->get();
		}
	}
	return nullptr;
}

cBuilding* cPlayer::getNextBuilding (cBuilding* start) const
{
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.begin() : buildings.find (*start);
	if (start != nullptr && it != buildings.end()) ++it;
	for (; it != buildings.end(); ++it)
	{
		if (! (*it)->isMarkedAsDone() && ! (*it)->isUnitWorking() && ! (*it)->isSentryActive()
			&& (!(*it)->getStaticUnitData().canBuild.empty() || (*it)->data.getShots()
			|| (*it)->getStaticUnitData().canMineMaxRes > 0 || (*it)->getStaticUnitData().convertsGold > 0
			|| (*it)->getStaticUnitData().canResearch))
		{
			return it->get();
		}
	}
	return nullptr;
}

cBuilding* cPlayer::getNextMiningStation (cBuilding* start) const
{
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.begin() : buildings.find (*start);
	if (start != nullptr && it != buildings.end()) ++it;
	for (; it != buildings.end(); ++it)
	{
		if ((*it)->getStaticUnitData().canMineMaxRes > 0)
		{
			return it->get();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/** Returns the next unit that can still fire/shoot */
//------------------------------------------------------------------------------
cUnit* cPlayer::getNextUnit (cUnit* start) const
{
	if (start == nullptr || start->getOwner() != this)
	{
		cVehicle* nextVehicle = getNextVehicle (nullptr);
		if (nextVehicle) return nextVehicle;
		cBuilding* nextBuilding = getNextBuilding (nullptr);
		if (nextBuilding) return nextBuilding;
	}
	else if (start->isAVehicle())
	{
		cVehicle* nextVehicle = getNextVehicle (static_cast<cVehicle*> (start));
		if (nextVehicle) return nextVehicle;
		cBuilding* nextBuilding = getNextBuilding (nullptr);
		if (nextBuilding) return nextBuilding;
		nextVehicle = getNextVehicle (nullptr);
		if (nextVehicle) return nextVehicle;
	}
	else
	{
		assert (start->isABuilding());
		cBuilding* building = static_cast<cBuilding*> (start);
		cBuilding* nextBuilding = getNextBuilding (building);
		if (nextBuilding) return nextBuilding;
		cVehicle* nextVehicle = getNextVehicle (nullptr);
		if (nextVehicle) return nextVehicle;
		nextBuilding = getNextBuilding (nullptr);
		if (nextBuilding) return nextBuilding;
	}
	// finally, return the more recent built Mining station.
	// since list order is by increasing age, take the first in list.
	return getNextMiningStation (nullptr);
}

cVehicle* cPlayer::getPrevVehicle (cVehicle* start) const
{
	if (vehicles.empty()) return nullptr;

	auto it = (start == nullptr) ? vehicles.end() - 1 : vehicles.find (*start);
	if (start != nullptr && it != vehicles.begin() && it != vehicles.end()) --it;
	for (; it != vehicles.end(); --it)
	{
		if (! (*it)->isMarkedAsDone() && (! (*it)->isUnitBuildingABuilding() || (*it)->getBuildTurns() == 0)
			&& ! (*it)->isUnitClearing() && ! (*it)->isSentryActive() && ! (*it)->isUnitLoaded()
			&& ((*it)->data.getSpeed() || (*it)->data.getShots()))
		{
			return it->get();
		}
		if (it == vehicles.begin()) break;
	}
	return nullptr;
}

cBuilding* cPlayer::getPrevBuilding (cBuilding* start) const
{
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.end() - 1 : buildings.find (*start);
	if (start != nullptr && it != buildings.begin() && it != buildings.end()) --it;
	for (; it != buildings.end(); --it)
	{
		if (! (*it)->isMarkedAsDone() && ! (*it)->isUnitWorking() && ! (*it)->isSentryActive()
			&& (! (*it)->getStaticUnitData().canBuild.empty() || (*it)->data.getShots()
				|| (*it)->getStaticUnitData().canMineMaxRes > 0 || (*it)->getStaticUnitData().convertsGold > 0
				|| (*it)->getStaticUnitData().canResearch))
		{
			return it->get();
		}
		if (it == buildings.begin()) break;
	}
	return nullptr;
}

cBuilding* cPlayer::getPrevMiningStation (cBuilding* start) const
{
	if (buildings.empty()) return nullptr;

	auto it = (start == nullptr) ? buildings.end() - 1 : buildings.find (*start);
	for (; it != buildings.end(); --it)
	{
		if ((*it)->getStaticUnitData().canMineMaxRes > 0)
		{
			return it->get();
		}
		if (it == buildings.begin()) break;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
/** Returns the previous vehicle, that can still move / shoot */
//------------------------------------------------------------------------------
cUnit* cPlayer::getPrevUnit (cUnit* start) const
{
	if (start == nullptr || start->getOwner() != this)
	{
		cVehicle* prevVehicle = getPrevVehicle (nullptr);
		if (prevVehicle) return prevVehicle;
		cBuilding* prevBuilding = getPrevBuilding (nullptr);
		if (prevBuilding) return prevBuilding;
	}
	else if (start->isAVehicle())
	{
		cVehicle* prevVehicle = getPrevVehicle (static_cast<cVehicle*> (start));
		if (prevVehicle) return prevVehicle;
		cBuilding* prevBuilding = getPrevBuilding (nullptr);
		if (prevBuilding) return prevBuilding;
		prevVehicle = getPrevVehicle (nullptr);
		if (prevVehicle) return prevVehicle;
	}
	else
	{
		assert (start->isABuilding());
		cBuilding* building = static_cast<cBuilding*> (start);
		cBuilding* prevBuilding = getPrevBuilding (building);
		if (prevBuilding) return prevBuilding;
		cVehicle* prevVehicle = getPrevVehicle (nullptr);
		if (prevVehicle) return prevVehicle;
		prevBuilding = getPrevBuilding (nullptr);
		if (prevBuilding) return prevBuilding;
	}
	// finally, return the more recent built Mining station.
	// since list order is by increasing age, take the first in list.
	return getNextMiningStation (nullptr);
}

//--------------------------------------------------------------------------
std::string cPlayer::resourceMapToString() const
{
	std::string str;
	str.reserve(ResourceMap.size() + 1);
	for (size_t i = 0; i != ResourceMap.size(); ++i)
	{
		str += getHexValue(ResourceMap[i]);
	}
	return str;
}

//--------------------------------------------------------------------------
void cPlayer::setResourceMapFromString(const std::string& str)
{
	for (size_t i = 0; i != ResourceMap.size(); ++i)
	{
		ResourceMap[i] = getByteValue(str, i);
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
void cPlayer::startAResearch (cResearch::ResearchArea researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		++researchCentersWorkingTotal;
		++researchCentersWorkingOnArea[researchArea];

		researchCentersWorkingOnAreaChanged (researchArea);
		researchCentersWorkingTotalChanged();
	}
}

//------------------------------------------------------------------------------
/** Stops a research center. */
//------------------------------------------------------------------------------
void cPlayer::stopAResearch (cResearch::ResearchArea researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		--researchCentersWorkingTotal;
		if (researchCentersWorkingOnArea[researchArea] > 0)
		{
			--researchCentersWorkingOnArea[researchArea];
			researchCentersWorkingOnAreaChanged (researchArea);
		}
		researchCentersWorkingTotalChanged();
	}
}

//------------------------------------------------------------------------------
/** At turn end update the research level */
//------------------------------------------------------------------------------
void cPlayer::doResearch (const cUnitsData& unitsData)
{
	bool researchFinished = false;
	std::vector<int> areasReachingNextLevel;
	currentTurnResearchAreasFinished.clear();
	for (int area = 0; area < cResearch::kNrResearchAreas; ++area)
	{
		if (researchCentersWorkingOnArea[area] > 0 &&
			researchState.doResearch (researchCentersWorkingOnArea[area], area))
		{
			// next level reached
			areasReachingNextLevel.push_back (area);
			currentTurnResearchAreasFinished.push_back (area);
			researchFinished = true;
		}
	}
	if (researchFinished)
	{
		upgradeUnitTypes (areasReachingNextLevel, unitsData);
	}
}

void cPlayer::accumulateScore (cServer& server)
{
	const int now = server.getTurnClock()->getTurn();
	int deltaScore = 0;

	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& bp = *i;
		if (bp->getStaticUnitData().canScore && bp->isUnitWorking())
		{
			bp->points++;
			deltaScore++;

			sendUnitScore (server, *bp);
		}
	}
	setScore (getScore (now) + deltaScore, now);
	sendScore (server, *this, now);
}

void cPlayer::countEcoSpheres()
{
	numEcos = 0;

	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& bp = *i;
		if (bp->getStaticUnitData().canScore && bp->isUnitWorking())
			++numEcos;
	}
}

void cPlayer::setScore (int s, int turn)
{
	// turn begins at 1.
	unsigned int t = turn;

	if (pointsHistory.size() < t)
	{
		pointsHistory.resize(t, pointsHistory.back());
	}
	pointsHistory[t - 1] = s;
}

void cPlayer::clearDone()
{
	for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
	{
		const auto& unit = *i;
		unit->setMarkedAsDone (false);
	}

	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& unit = *i;
		unit->setMarkedAsDone (false);
	}
}

int cPlayer::getScore (int turn) const
{
	// turn begins at 1.
	unsigned int t = turn;

	if (pointsHistory.size() < t)
	{
		return pointsHistory.empty() ? 0 : pointsHistory.back();
	}
	return pointsHistory[t - 1];
}

int cPlayer::getScore() const
{
	return pointsHistory.back();
}

//------------------------------------------------------------------------------
void cPlayer::upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, const cUnitsData& originalUnitsData)
{
	for (auto& unitData : dynamicUnitsData)
	{
		const cDynamicUnitData& originalData = originalUnitsData.getDynamicUnitData(unitData.getId(), getClan());
		bool incrementVersion = false;
		for (auto researchArea : areasReachingNextLevel)
		{
			if (unitData.getId().isABuilding() && researchArea == cResearch::kSpeedResearch) continue;

			const int newResearchLevel = researchState.getCurResearchLevel (researchArea);
			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = originalData.getDamage(); break;
				case cResearch::kShotsResearch: startValue = originalData.getShotsMax(); break;
				case cResearch::kRangeResearch: startValue = originalData.getRange(); break;
				case cResearch::kArmorResearch: startValue = originalData.getArmor(); break;
				case cResearch::kHitpointsResearch: startValue = originalData.getHitpointsMax(); break;
				case cResearch::kScanResearch: startValue = originalData.getScan(); break;
				case cResearch::kSpeedResearch: startValue = originalData.getSpeedMax(); break;
				case cResearch::kCostResearch: startValue = originalData.getBuildCost(); break;
			}

			cUpgradeCalculator::UnitTypes unitType = cUpgradeCalculator::kStandardUnit;
			if (unitData.getId().isABuilding()) unitType = cUpgradeCalculator::kBuilding;
			if (originalUnitsData.getStaticUnitData(unitData.getId()).isHuman) unitType = cUpgradeCalculator::kInfantry;

			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel - 10,
								   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1, unitType);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel,
								   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1, unitType);

			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: unitData.setDamage (unitData.getDamage() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kShotsResearch: unitData.setShotsMax (unitData.getShotsMax() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kRangeResearch: unitData.setRange (unitData.getRange() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kArmorResearch: unitData.setArmor (unitData.getArmor() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kHitpointsResearch: unitData.setHitpointsMax (unitData.getHitpointsMax() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kScanResearch: unitData.setScan (unitData.getScan() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kSpeedResearch: unitData.setSpeedMax (unitData.getSpeedMax() + newResearchBonus - oldResearchBonus); break;
					case cResearch::kCostResearch: unitData.setBuildCost( unitData.getBuildCost() + newResearchBonus - oldResearchBonus); break;
				}
				if (researchArea != cResearch::kCostResearch)   // don't increment the version, if the only change are the costs
					incrementVersion = true;
			}
		}
		if (incrementVersion)
			unitData.setVersion (unitData.getVersion() + 1);
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshResearchCentersWorkingOnArea()
{
	int oldResearchCentersWorkingOnArea[cResearch::kNrResearchAreas];

	int newResearchCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		oldResearchCentersWorkingOnArea[i] = researchCentersWorkingOnArea[i];
		researchCentersWorkingOnArea[i] = 0;
	}

	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& building = *i;
		if (building->getStaticUnitData().canResearch && building->isUnitWorking())
		{
			researchCentersWorkingOnArea[building->getResearchArea()] += 1;
			newResearchCount++;
		}
	}
	std::swap (researchCentersWorkingTotal, newResearchCount);

	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		if (oldResearchCentersWorkingOnArea[i] != researchCentersWorkingOnArea[i])
		{
			researchCentersWorkingOnAreaChanged ((cResearch::ResearchArea)i);
		}
	}
	if (researchCentersWorkingTotal != newResearchCount) researchCentersWorkingTotalChanged();
}

//------------------------------------------------------------------------------
bool cPlayer::mayHaveOffensiveUnit() const
{
	for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
	{
		const auto& vehicle = *i;
		if (vehicle->getStaticUnitData().canAttack || !vehicle->getStaticUnitData().canBuild.empty()) return true;
	}
	for (auto i = buildings.begin(); i != buildings.end(); ++i)
	{
		const auto& building = *i;
		if (building->getStaticUnitData().canAttack || !building->getStaticUnitData().canBuild.empty()) return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cPlayer::addTurnReportUnit (const sID& unitTypeId)
{
	auto iter = std::find_if (currentTurnUnitReports.begin(), currentTurnUnitReports.end(), [unitTypeId] (const sTurnstartReport & entry) { return entry.type == unitTypeId; });
	if (iter != currentTurnUnitReports.end())
	{
		++iter->count;
	}
	else
	{
		sTurnstartReport entry;
		entry.type = unitTypeId;
		entry.count = 1;
		currentTurnUnitReports.push_back (entry);
	}
}

//------------------------------------------------------------------------------
void cPlayer::resetTurnReportData()
{
	currentTurnUnitReports.clear();
}

//------------------------------------------------------------------------------
const std::vector<sTurnstartReport>& cPlayer::getCurrentTurnUnitReports() const
{
	return currentTurnUnitReports;
}

//------------------------------------------------------------------------------
const std::vector<int>& cPlayer::getCurrentTurnResearchAreasFinished() const
{
	return currentTurnResearchAreasFinished;
}

//------------------------------------------------------------------------------
void cPlayer::setCurrentTurnResearchAreasFinished (std::vector<int> areas)
{
	currentTurnResearchAreasFinished = std::move (areas);
}

//------------------------------------------------------------------------------
bool cPlayer::isCurrentTurnResearchAreaFinished (cResearch::ResearchArea area) const
{
	return std::find (currentTurnResearchAreasFinished.begin(), currentTurnResearchAreasFinished.end(), area) != currentTurnResearchAreasFinished.end();
}

//------------------------------------------------------------------------------
const cResearch& cPlayer::getResearchState() const
{
	return researchState;
}

//------------------------------------------------------------------------------
cResearch& cPlayer::getResearchState()
{
	return researchState;
}

//------------------------------------------------------------------------------
int cPlayer::getResearchCentersWorkingTotal() const
{
	return researchCentersWorkingTotal;
}

//------------------------------------------------------------------------------
int cPlayer::getResearchCentersWorkingOnArea (cResearch::ResearchArea area) const
{
	return researchCentersWorkingOnArea[area];
}

//------------------------------------------------------------------------------
bool cPlayer::canSeeAt (const cPosition& position) const
{
	if (position.x() < 0 || position.x() >= mapSize.x() || position.y() < 0 || position.y() >= mapSize.y()) return false;

	return ScanMap[getOffset (position)] != 0;
}

//------------------------------------------------------------------------------
void cPlayer::drawSpecialCircle (const cPosition& position, int iRadius, std::vector<char>& map, const cPosition& mapsize)
{
	const float PI_ON_180 = 0.017453f;
	const float PI_ON_4 = PI_ON_180 * 45;
	if (iRadius <= 0) return;

	iRadius *= 10;
	const float step = (PI_ON_180 * 90 - acosf (1.0f / iRadius)) / 2;

	for (float angle = 0; angle <= PI_ON_4; angle += step)
	{
		int rx = (int) (cosf (angle) * iRadius);
		int ry = (int) (sinf (angle) * iRadius);
		rx /= 10;
		ry /= 10;

		int x1 = rx + position.x();
		int x2 = -rx + position.x();
		for (int k = x2; k <= x1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize.x()) break;
			if (position.y() + ry >= 0 && position.y() + ry < mapsize.y())
				map[k + (position.y() + ry) * mapsize.x()] |= 1;
			if (position.y() - ry >= 0 && position.y() - ry < mapsize.y())
				map[k + (position.y() - ry) * mapsize.x()] |= 1;
		}

		x1 = ry + position.x();
		x2 = -ry + position.x();
		for (int k = x2; k <= x1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize.x()) break;
			if (position.y() + rx >= 0 && position.y() + rx < mapsize.y())
				map[k + (position.y() + rx) *mapsize.x()] |= 1;
			if (position.y() - rx >= 0 && position.y() - rx < mapsize.y())
				map[k + (position.y() - rx) *mapsize.x()] |= 1;
		}
	}
}

//------------------------------------------------------------------------------
void cPlayer::drawSpecialCircleBig (const cPosition& position, int iRadius, std::vector<char>& map, const cPosition& mapsize)
{
	const float PI_ON_180 = 0.017453f;
	const float PI_ON_4 = PI_ON_180 * 45;
	if (iRadius <= 0) return;

	--iRadius;
	iRadius *= 10;
	const float step = (PI_ON_180 * 90 - acosf (1.0f / iRadius)) / 2;
	for (float angle = 0; angle <= PI_ON_4; angle += step)
	{
		int rx = (int) (cosf (angle) * iRadius);
		int ry = (int) (sinf (angle) * iRadius);
		rx /= 10;
		ry /= 10;

		int x1 = rx + position.x();
		int x2 = -rx + position.x();
		for (int k = x2; k <= x1 + 1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize.x()) break;
			if (position.y() + ry >= 0 && position.y() + ry < mapsize.y())
				map[k + (position.y() + ry) *mapsize.x()] |= 1;
			if (position.y() - ry >= 0 && position.y() - ry < mapsize.y())
				map[k + (position.y() - ry) *mapsize.x()] |= 1;

			if (position.y() + ry + 1 >= 0 && position.y() + ry + 1 < mapsize.y())
				map[k + (position.y() + ry + 1) *mapsize.x()] |= 1;
			if (position.y() - ry + 1 >= 0 && position.y() - ry + 1 < mapsize.y())
				map[k + (position.y() - ry + 1) *mapsize.x()] |= 1;
		}

		x1 = ry + position.x();
		x2 = -ry + position.x();
		for (int k = x2; k <= x1 + 1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize.x()) break;
			if (position.y() + rx >= 0 && position.y() + rx < mapsize.y())
				map[k + (position.y() + rx) *mapsize.x()] |= 1;
			if (position.y() - rx >= 0 && position.y() - rx < mapsize.y())
				map[k + (position.y() - rx) *mapsize.x()] |= 1;

			if (position.y() + rx + 1 >= 0 && position.y() + rx + 1 < mapsize.y())
				map[k + (position.y() + rx + 1) *mapsize.x()] |= 1;
			if (position.y() - rx + 1 >= 0 && position.y() - rx + 1 < mapsize.y())
				map[k + (position.y() - rx + 1) *mapsize.x()] |= 1;
		}
	}
}

//------------------------------------------------------------------------------
bool cPlayer::getHasFinishedTurn() const
{
	return hasFinishedTurn;
}

//------------------------------------------------------------------------------
void cPlayer::setHasFinishedTurn (bool value)
{
	std::swap (hasFinishedTurn, value);
	if (hasFinishedTurn != value) hasFinishedTurnChanged();
}

//------------------------------------------------------------------------------
bool cPlayer::getIsRemovedFromGame() const
{
	return isRemovedFromGame;
}

//------------------------------------------------------------------------------
void cPlayer::setIsRemovedFromGame (bool value)
{
	std::swap (isRemovedFromGame, value);
	if (isRemovedFromGame != value) isRemovedFromGameChanged();
}
