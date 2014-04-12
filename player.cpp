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

#include "player.h"

#include "buildings.h"
#include "client.h"
#include "clist.h"
#include "hud.h"
#include "netmessage.h"
#include "server.h"
#include "serverevents.h"
#include "vehicles.h"

using namespace std;

//------------------------------------------------------------------------------
void sSavedReportMessage::pushInto (cNetMessage& message) const
{
	message.pushInt16 (colorNr);
	message.pushID (unitID);
	message.pushInt16 (yPos);
	message.pushInt16 (xPos);
	message.pushInt16 (type);
	message.pushString (this->message);
}

//------------------------------------------------------------------------------
void sSavedReportMessage::popFrom (cNetMessage& message)
{
	this->message = message.popString();
	type = static_cast<sSavedReportMessage::eReportTypes> (message.popInt16());
	xPos = message.popInt16();
	yPos = message.popInt16();
	unitID = message.popID();
	colorNr = message.popInt16();
}

//------------------------------------------------------------------------------
sPlayer::sPlayer (const string& name_, unsigned int colorIndex_, int nr_, int socketIndex_) :
	name (name_),
	colorIndex (colorIndex_),
	Nr (nr_),
	socketIndex (socketIndex_),
	ready (false)
{
	assert (colorIndex < PLAYERCOLORS);
}

//------------------------------------------------------------------------------
void sPlayer::setLocal()
{
	socketIndex = MAX_CLIENTS;
}

//------------------------------------------------------------------------------
bool sPlayer::isLocal() const
{
	return socketIndex == MAX_CLIENTS;
}

//------------------------------------------------------------------------------
void sPlayer::onSocketIndexDisconnected (int socketIndex_)
{
	if (isLocal() || socketIndex == -1) return;
	if (socketIndex == socketIndex_)
	{
		socketIndex = -1;
	}
	else if (socketIndex > socketIndex_)
	{
		--socketIndex;
	}
}

//------------------------------------------------------------------------------
SDL_Surface* sPlayer::getColorSurface() const
{
	return OtherData.colors[getColorIndex()].get();
}

//------------------------------------------------------------------------------
void sPlayer::setToNextColorIndex()
{
	setColorIndex ((colorIndex + 1) % PLAYERCOLORS);
}

//------------------------------------------------------------------------------
void sPlayer::setToPrevColorIndex()
{
	setColorIndex ((colorIndex - 1 + PLAYERCOLORS) % PLAYERCOLORS);
}

//------------------------------------------------------------------------------
// Implementation cPlayer class
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cPlayer::cPlayer (const sPlayer& splayer_) :
	splayer (splayer_),
	landingPosX (-1),
	landingPosY (-1),
	savedHud (new sHudStateContainer),
	numEcos (0),
	lastDeletedUnit (0),
	clan (-1)
{
	// get the default (no clan) unit data
	VehicleData = UnitsData.getUnitData_Vehicles (-1);
	BuildingData = UnitsData.getUnitData_Buildings (-1);

	VehicleList = NULL;
	BuildingList = NULL;

	workingResearchCenterCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;
	Credits = 0;

	isDefeated = false;
	isRemovedFromGame = false;
	bFinishedTurn = false;

	researchFinished = false;
}

//------------------------------------------------------------------------------
cPlayer::~cPlayer()
{
	// Erst alle geladenen Vehicles lˆschen:

	for (cVehicle* ptr = VehicleList; ptr; ptr = ptr->next)
	{
		ptr->deleteStoredUnits();
	}
	// Jetzt alle Vehicles lˆschen:
	while (VehicleList)
	{
		cVehicle* ptr = VehicleList->next;
		VehicleList->sentryActive = false;
		delete VehicleList;
		VehicleList = ptr;
	}
	while (BuildingList)
	{
		cBuilding* ptr = BuildingList->next;
		BuildingList->sentryActive = false;

		// Stored Vehicles are already deleted; just clear the list
		BuildingList->storedUnits.clear();

		delete BuildingList;
		BuildingList = ptr;
	}

	for (size_t i = 0; i != ReportVehicles.size(); ++i)
	{
		delete ReportVehicles[i];
	}
	ReportVehicles.clear();
	for (size_t i = 0; i != ReportBuildings.size(); ++i)
	{
		delete ReportBuildings[i];
	}
	ReportBuildings.clear();
}

//------------------------------------------------------------------------------
void cPlayer::setClan (int newClan)
{
	if (newClan == clan || newClan < -1 || 7 < newClan)
		return;

	clan = newClan;

	VehicleData = UnitsData.getUnitData_Vehicles (clan);
	BuildingData = UnitsData.getUnitData_Buildings (clan);
}

//------------------------------------------------------------------------------
sUnitData* cPlayer::getUnitDataCurrentVersion (const sID& ID)
{
	if (ID.isAVehicle())
	{
		for (size_t i = 0; i != VehicleData.size(); ++i)
		{
			if (VehicleData[i].ID == ID)
				return &VehicleData[i];
		}
	}
	else if (ID.isABuilding())
	{
		for (unsigned int i = 0; i < BuildingData.size(); ++i)
		{
			if (BuildingData[i].ID == ID)
				return &BuildingData[i];
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
/** Adds the vehicle to the list of the player */
//------------------------------------------------------------------------------
cVehicle* cPlayer::addVehicle (int posx, int posy, const sID& id, unsigned int uid)
{
	const sUnitData& unitData = *id.getUnitDataOriginalVersion (this);
	cVehicle* n = new cVehicle (unitData, this, uid);
	n->PosX = posx;
	n->PosY = posy;

	addUnitToList (*n);

	drawSpecialCircle (n->PosX, n->PosY, n->data.scan, ScanMap, mapSize);
	if (n->data.canDetectStealthOn & TERRAIN_GROUND) drawSpecialCircle (n->PosX, n->PosY, n->data.scan, DetectLandMap, mapSize);
	if (n->data.canDetectStealthOn & TERRAIN_SEA) drawSpecialCircle (n->PosX, n->PosY, n->data.scan, DetectSeaMap, mapSize);
	if (n->data.canDetectStealthOn & AREA_EXP_MINE)
	{
		const int minx = std::max (n->PosX - 1, 0);
		const int maxx = std::min (n->PosX + 1, mapSize - 1);
		const int miny = std::max (n->PosY - 1, 0);
		const int maxy = std::min (n->PosY + 1, mapSize - 1);
		for (int x = minx; x <= maxx; ++x)
			for (int y = miny; y <= maxy; ++y)
				DetectMinesMap[x + mapSize * y] = 1;
	}
	return n;
}

//------------------------------------------------------------------------------
/** initialize the maps */
//------------------------------------------------------------------------------
void cPlayer::initMaps (cMap& map)
{
	mapSize = map.getSize();
	const int size = mapSize * mapSize;
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

template <typename T>
T* getPreviousUnitById (T* root, unsigned int id)
{
	if (root == 0 || id < root->iID) return 0;
	T* it = root;
	for (; it->next; it = it->next)
	{
		if (it->next->iID > id)
			return it;
	}
	return it;
}

void cPlayer::addUnitToList (cUnit& addedUnit)
{
	//units in the linked list are sorted in increasing order of IDs

	//find unit before the added unit
	if (addedUnit.isABuilding())
	{
		cBuilding* addedBuilding = static_cast<cBuilding*> (&addedUnit);
		cBuilding* prev = getPreviousUnitById (BuildingList, addedUnit.iID);
		insert_after_in_intrusivelist (BuildingList, prev, *addedBuilding);
	}
	else
	{
		cVehicle* addedVehicle = static_cast<cVehicle*> (&addedUnit);
		cVehicle* prev = getPreviousUnitById (VehicleList, addedUnit.iID);
		insert_after_in_intrusivelist (VehicleList, prev, *addedVehicle);
	}
}

//------------------------------------------------------------------------------
/** Adds the building to the list of the player */
//------------------------------------------------------------------------------
cBuilding* cPlayer::addBuilding (int posx, int posy, const sID& id, unsigned int uid)
{
	const sUnitData* unitData = id.getUnitDataOriginalVersion (this);
	cBuilding* Building = new cBuilding (unitData, this, uid);

	Building->PosX = posx;
	Building->PosY = posy;

	addUnitToList (*Building);

	if (Building->data.scan)
	{
		if (Building->data.isBig) drawSpecialCircleBig (Building->PosX, Building->PosY, Building->data.scan, ScanMap, mapSize);
		else drawSpecialCircle (Building->PosX, Building->PosY, Building->data.scan, ScanMap, mapSize);
	}
	return Building;
}

//------------------------------------------------------------------------------
void cPlayer::addSentry (cUnit& u)
{
	u.sentryActive = true;
	if (u.data.canAttack & TERRAIN_AIR)
	{
		drawSpecialCircle (u.PosX, u.PosY, u.data.range, SentriesMapAir, mapSize);
	}
	if ((u.data.canAttack & TERRAIN_GROUND) || (u.data.canAttack & TERRAIN_SEA))
	{
		drawSpecialCircle (u.PosX, u.PosY, u.data.range, SentriesMapGround, mapSize);
	}
}

//------------------------------------------------------------------------------
void cPlayer::deleteSentry (cUnit& u)
{
	u.sentryActive = false;
	if (u.data.canAttack & TERRAIN_AIR)
	{
		refreshSentryAir();
	}
	else if ((u.data.canAttack & TERRAIN_GROUND) || (u.data.canAttack & TERRAIN_SEA))
	{
		refreshSentryGround();
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshSentryAir()
{
	std::fill (SentriesMapAir.begin(), SentriesMapAir.end(), 0);

	for (const cVehicle* unit = VehicleList; unit; unit = unit->next)
	{
		if (unit->sentryActive && unit->data.canAttack & TERRAIN_AIR)
		{
			drawSpecialCircle (unit->PosX, unit->PosY, unit->data.range, SentriesMapAir, mapSize);
		}
	}

	for (const cBuilding* unit = BuildingList; unit; unit = unit->next)
	{
		if (unit->sentryActive && unit->data.canAttack & TERRAIN_AIR)
		{
			drawSpecialCircle (unit->PosX, unit->PosY, unit->data.range, SentriesMapAir, mapSize);
		}
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshSentryGround()
{
	std::fill (SentriesMapGround.begin(), SentriesMapGround.end(), 0);

	for (const cVehicle* unit = VehicleList; unit; unit = unit->next)
	{
		if (unit->sentryActive && ((unit->data.canAttack & TERRAIN_GROUND) || (unit->data.canAttack & TERRAIN_SEA)))
		{
			drawSpecialCircle (unit->PosX, unit->PosY, unit->data.range, SentriesMapGround, mapSize);
		}
	}
	for (const cBuilding* unit = BuildingList; unit; unit = unit->next)
	{
		if (unit->sentryActive && ((unit->data.canAttack & TERRAIN_GROUND) || (unit->data.canAttack & TERRAIN_SEA)))
		{
			drawSpecialCircle (unit->PosX, unit->PosY, unit->data.range, SentriesMapGround, mapSize);
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
	for (const cVehicle* vp = VehicleList; vp; vp = vp->next)
	{
		if (vp->Loaded) continue;

		if (vp->isDisabled())
			ScanMap[getOffset (vp->PosX, vp->PosY)] = 1;
		else
		{
			if (vp->data.isBig)
				drawSpecialCircleBig (vp->PosX, vp->PosY, vp->data.scan, ScanMap, mapSize);
			else
				drawSpecialCircle (vp->PosX, vp->PosY, vp->data.scan, ScanMap, mapSize);

			//detection maps
			if (vp->data.canDetectStealthOn & TERRAIN_GROUND) drawSpecialCircle (vp->PosX, vp->PosY, vp->data.scan, DetectLandMap, mapSize);
			else if (vp->data.canDetectStealthOn & TERRAIN_SEA) drawSpecialCircle (vp->PosX, vp->PosY, vp->data.scan, DetectSeaMap, mapSize);
			if (vp->data.canDetectStealthOn & AREA_EXP_MINE)
			{
				const int minx = std::max (vp->PosX - 1, 0);
				const int maxx = std::min (vp->PosX + 1, mapSize - 1);
				const int miny = std::max (vp->PosY - 1, 0);
				const int maxy = std::min (vp->PosY + 1, mapSize - 1);
				for (int x = minx; x <= maxx; ++x)
				{
					for (int y = miny; y <= maxy; ++y)
					{
						DetectMinesMap[x + mapSize * y] = 1;
					}
				}
			}
		}
	}

	// iterate the building list
	for (const cBuilding* bp = BuildingList; bp; bp = bp->next)
	{
		if (bp->isDisabled())
			ScanMap[getOffset (bp->PosX, bp->PosY)] = 1;
		else if (bp->data.scan)
		{
			if (bp->data.isBig)
				drawSpecialCircleBig (bp->PosX, bp->PosY, bp->data.scan, ScanMap, mapSize);
			else
				drawSpecialCircle (bp->PosX, bp->PosY, bp->data.scan, ScanMap, mapSize);
		}
	}
}

void cPlayer::revealMap()
{
	std::fill (ScanMap.begin(), ScanMap.end(), 1);
}

void cPlayer::revealResource()
{
	std::fill (ResourceMap.begin(), ResourceMap.end(), 1);
}

bool cPlayer::canSeeAnyAreaUnder (const cUnit& unit) const
{
	const int offset = getOffset (unit.PosX, unit.PosY);

	if (ScanMap[offset] == 1) return true;
	if (!unit.data.isBig) return false;

	return ScanMap[offset + 1] == 1 ||
		   ScanMap[offset + getMapSize()] == 1 ||
		   ScanMap[offset + getMapSize() + 1] == 1;
}

cVehicle* cPlayer::getNextVehicle (cVehicle* start)
{
	start = (start == NULL) ? VehicleList : start->next;
	for (cVehicle* it = start; it; it = it->next)
	{
		if (!it->isMarkedAsDone && (!it->IsBuilding || it->BuildRounds == 0)
			&& !it->IsClearing && !it->sentryActive && !it->Loaded
			&& (it->data.speedCur || it->data.shotsCur))
			return it;
	}
	return NULL;
}

cBuilding* cPlayer::getNextBuilding (cBuilding* start)
{
	start = (start == NULL) ? BuildingList : start->next;
	for (cBuilding* it = start; it; it = it->next)
	{
		if (!it->isMarkedAsDone && !it->IsWorking && !it->sentryActive
			&& (!it->data.canBuild.empty() || it->data.shotsCur
				|| it->data.canMineMaxRes > 0 || it->data.convertsGold > 0
				|| it->data.canResearch))
			return it;
	}
	return NULL;
}

cBuilding* cPlayer::getNextMiningStation (cBuilding* start)
{
	start = (start == NULL) ? BuildingList : start->next;
	for (cBuilding* it = start; it; it = it->next)
	{
		if (it->data.canMineMaxRes > 0)
			return it;
	}
	return NULL;
}

//------------------------------------------------------------------------------
/** Returns the next unit that can still fire/shoot */
//------------------------------------------------------------------------------
cUnit* cPlayer::getNextUnit (cUnit* start)
{
	if (start == NULL || start->owner != this)
	{
		cVehicle* nextVehicle = getNextVehicle (NULL);
		if (nextVehicle) return nextVehicle;
		cBuilding* nextBuilding = getNextBuilding (NULL);
		if (nextBuilding) return nextBuilding;
	}
	else if (start->isAVehicle())
	{
		cVehicle* nextVehicle = getNextVehicle (static_cast<cVehicle*> (start));
		if (nextVehicle) return nextVehicle;
		cBuilding* nextBuilding = getNextBuilding (NULL);
		if (nextBuilding) return nextBuilding;
		nextVehicle = getNextVehicle (NULL);
		if (nextVehicle) return nextVehicle;
	}
	else
	{
		assert (start->isABuilding());
		cBuilding* building = static_cast<cBuilding*> (start);
		cBuilding* nextBuilding = getNextBuilding (building);
		if (nextBuilding) return nextBuilding;
		cVehicle* nextVehicle = getNextVehicle (NULL);
		if (nextVehicle) return nextVehicle;
		nextBuilding = getNextBuilding (NULL);
		if (nextBuilding) return nextBuilding;
	}
	// finally, return the more recent built Mining station.
	// since list order is by increasing age, take the first in list.
	return getNextMiningStation (NULL);
}

cVehicle* cPlayer::getPrevVehicle (cVehicle* start)
{
	start = (start == NULL) ? get_last_of_intrusivelist (VehicleList) : start->prev;
	for (cVehicle* it = start; it; it = it->prev)
	{
		if (!it->isMarkedAsDone && (!it->IsBuilding || it->BuildRounds == 0)
			&& !it->IsClearing && !it->sentryActive && !it->Loaded
			&& (it->data.speedCur || it->data.shotsCur))
			return it;
	}
	return NULL;
}

cBuilding* cPlayer::getPrevBuilding (cBuilding* start)
{
	start = (start == NULL) ? get_last_of_intrusivelist (BuildingList) : start->prev;
	for (cBuilding* it = start; it; it = it->prev)
	{
		if (!it->isMarkedAsDone && !it->IsWorking && !it->sentryActive
			&& (!it->data.canBuild.empty() || it->data.shotsCur
				|| it->data.canMineMaxRes > 0 || it->data.convertsGold > 0
				|| it->data.canResearch))
			return it;
	}
	return NULL;
}

cBuilding* cPlayer::getPrevMiningStation (cBuilding* start)
{
	start = (start == NULL) ? get_last_of_intrusivelist (BuildingList) : start->prev;
	for (cBuilding* it = start; it; it = it->prev)
	{
		if (it->data.canMineMaxRes > 0)
			return it;
	}
	return NULL;
}

//------------------------------------------------------------------------------
/** Returns the previous vehicle, that can still move / shoot */
//------------------------------------------------------------------------------
cUnit* cPlayer::getPrevUnit (cUnit* start)
{
	if (start == NULL || start->owner != this)
	{
		cVehicle* prevVehicle = getPrevVehicle (NULL);
		if (prevVehicle) return prevVehicle;
		cBuilding* prevBuilding = getPrevBuilding (NULL);
		if (prevBuilding) return prevBuilding;
	}
	else if (start->isAVehicle())
	{
		cVehicle* prevVehicle = getPrevVehicle (static_cast<cVehicle*> (start));
		if (prevVehicle) return prevVehicle;
		cBuilding* prevBuilding = getPrevBuilding (NULL);
		if (prevBuilding) return prevBuilding;
		prevVehicle = getPrevVehicle (NULL);
		if (prevVehicle) return prevVehicle;
	}
	else
	{
		assert (start->isABuilding());
		cBuilding* building = static_cast<cBuilding*> (start);
		cBuilding* prevBuilding = getPrevBuilding (building);
		if (prevBuilding) return prevBuilding;
		cVehicle* prevVehicle = getPrevVehicle (NULL);
		if (prevVehicle) return prevVehicle;
		prevBuilding = getPrevBuilding (NULL);
		if (prevBuilding) return prevBuilding;
	}
	// finally, return the more recent built Mining station.
	// since list order is by increasing age, take the first in list.
	return getNextMiningStation (NULL);
}

//------------------------------------------------------------------------------
/** Starts a research center. */
//------------------------------------------------------------------------------
void cPlayer::startAResearch (int researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		workingResearchCenterCount++;
		researchCentersWorkingOnArea[researchArea] += 1;
	}
}

//------------------------------------------------------------------------------
/** Stops a research center. */
//------------------------------------------------------------------------------
void cPlayer::stopAResearch (int researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		workingResearchCenterCount--;
		if (researchCentersWorkingOnArea[researchArea] > 0)
			researchCentersWorkingOnArea[researchArea] -= 1;
	}
}

//------------------------------------------------------------------------------
/** At turnend update the research level */
//------------------------------------------------------------------------------
void cPlayer::doResearch (cServer& server)
{
	bool researchFinished = false;
	std::vector<sUnitData*> upgradedUnitDatas;
	std::vector<int> areasReachingNextLevel;
	reportResearchAreasFinished.clear();
	for (int area = 0; area < cResearch::kNrResearchAreas; ++area)
	{
		if (researchCentersWorkingOnArea[area] > 0 &&
			researchLevel.doResearch (researchCentersWorkingOnArea[area], area))
		{
			// next level reached
			areasReachingNextLevel.push_back (area);
			reportResearchAreasFinished.push_back (area);
			researchFinished = true;
		}
	}
	if (researchFinished)
	{
		upgradeUnitTypes (areasReachingNextLevel, upgradedUnitDatas);

		for (size_t i = 0; i != upgradedUnitDatas.size(); ++i)
			sendUnitUpgrades (server, *upgradedUnitDatas[i], getNr());
	}
	sendResearchLevel (server, researchLevel, getNr());
}

void cPlayer::accumulateScore (cServer& server)
{
	const int now = server.getTurn();
	int deltaScore = 0;

	for (cBuilding* bp = BuildingList; bp; bp = bp->next)
	{
		if (bp->data.canScore && bp->IsWorking)
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

	for (const cBuilding* bp = BuildingList; bp; bp = bp->next)
	{
		if (bp->data.canScore && bp->IsWorking)
			++numEcos;
	}
}

void cPlayer::setScore (int s, int turn)
{
	// turn begins at 1.
	unsigned int t = turn;

	if (pointsHistory.size() < t)
		pointsHistory.resize (t);
	pointsHistory[t - 1] = s;
}

void cPlayer::clearDone()
{
	for (cVehicle* unit = VehicleList; unit; unit = unit->next)
	{
		unit->isMarkedAsDone = false;
	}

	for (cBuilding* unit = BuildingList; unit; unit = unit->next)
	{
		unit->isMarkedAsDone = false;
	}
}

int cPlayer::getScore (int turn) const
{
	// turn begins at 1.
	unsigned int t = turn;

	if (pointsHistory.size() < t)
	{
		const int score = pointsHistory.empty() ? 0 : pointsHistory.back();
		pointsHistory.resize (t, score);
	}
	return pointsHistory[t - 1];
}

//------------------------------------------------------------------------------
void cPlayer::upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, std::vector<sUnitData*>& resultUpgradedUnitDatas)
{
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
	{
		const sUnitData& originalData = UnitsData.getVehicle (i, getClan());
		bool incrementVersion = false;
		for (size_t areaCounter = 0; areaCounter != areasReachingNextLevel.size(); areaCounter++)
		{
			const int researchArea = areasReachingNextLevel[areaCounter];
			const int newResearchLevel = researchLevel.getCurResearchLevel (researchArea);
			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = originalData.damage; break;
				case cResearch::kShotsResearch: startValue = originalData.shotsMax; break;
				case cResearch::kRangeResearch: startValue = originalData.range; break;
				case cResearch::kArmorResearch: startValue = originalData.armor; break;
				case cResearch::kHitpointsResearch: startValue = originalData.hitpointsMax; break;
				case cResearch::kScanResearch: startValue = originalData.scan; break;
				case cResearch::kSpeedResearch: startValue = originalData.speedMax; break;
				case cResearch::kCostResearch: startValue = originalData.buildCosts; break;
			}
			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel - 10,
								   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
								   originalData.isHuman ? cUpgradeCalculator::kInfantry : cUpgradeCalculator::kStandardUnit);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel,
								   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
								   originalData.isHuman ? cUpgradeCalculator::kInfantry : cUpgradeCalculator::kStandardUnit);
			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: VehicleData[i].damage += newResearchBonus - oldResearchBonus; break;
					case cResearch::kShotsResearch: VehicleData[i].shotsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kRangeResearch: VehicleData[i].range += newResearchBonus - oldResearchBonus; break;
					case cResearch::kArmorResearch: VehicleData[i].armor += newResearchBonus - oldResearchBonus; break;
					case cResearch::kHitpointsResearch: VehicleData[i].hitpointsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kScanResearch: VehicleData[i].scan += newResearchBonus - oldResearchBonus; break;
					case cResearch::kSpeedResearch: VehicleData[i].speedMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kCostResearch: VehicleData[i].buildCosts += newResearchBonus - oldResearchBonus; break;
				}
				if (researchArea != cResearch::kCostResearch)   // don't increment the version, if the only change are the costs
					incrementVersion = true;
				if (!Contains (resultUpgradedUnitDatas, & (VehicleData[i])))
					resultUpgradedUnitDatas.push_back (& (VehicleData[i]));
			}
		}
		if (incrementVersion)
			VehicleData[i].version += 1;
	}

	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
	{
		const sUnitData& originalData = UnitsData.getBuilding (i, getClan());
		bool incrementVersion = false;
		for (size_t areaCounter = 0; areaCounter != areasReachingNextLevel.size(); areaCounter++)
		{
			const int researchArea = areasReachingNextLevel[areaCounter];
			const int newResearchLevel = researchLevel.getCurResearchLevel (researchArea);

			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = originalData.damage; break;
				case cResearch::kShotsResearch: startValue = originalData.shotsMax; break;
				case cResearch::kRangeResearch: startValue = originalData.range; break;
				case cResearch::kArmorResearch: startValue = originalData.armor; break;
				case cResearch::kHitpointsResearch: startValue = originalData.hitpointsMax; break;
				case cResearch::kScanResearch: startValue = originalData.scan; break;
				case cResearch::kCostResearch: startValue = originalData.buildCosts; break;
			}
			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel - 10,
								   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
								   cUpgradeCalculator::kBuilding);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch (startValue, newResearchLevel,
								   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
								   cUpgradeCalculator::kBuilding);
			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: BuildingData[i].damage += newResearchBonus - oldResearchBonus; break;
					case cResearch::kShotsResearch: BuildingData[i].shotsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kRangeResearch: BuildingData[i].range += newResearchBonus - oldResearchBonus; break;
					case cResearch::kArmorResearch: BuildingData[i].armor += newResearchBonus - oldResearchBonus; break;
					case cResearch::kHitpointsResearch: BuildingData[i].hitpointsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kScanResearch: BuildingData[i].scan += newResearchBonus - oldResearchBonus; break;
					case cResearch::kCostResearch: BuildingData[i].buildCosts += newResearchBonus - oldResearchBonus; break;
				}
				if (researchArea != cResearch::kCostResearch)   // don't increment the version, if the only change are the costs
					incrementVersion = true;
				if (!Contains (resultUpgradedUnitDatas, & (BuildingData[i])))
					resultUpgradedUnitDatas.push_back (& (BuildingData[i]));
			}
		}
		if (incrementVersion)
			BuildingData[i].version += 1;
	}
}

//------------------------------------------------------------------------------
void cPlayer::refreshResearchCentersWorkingOnArea()
{
	int newResearchCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;

	for (const cBuilding* curBuilding = BuildingList; curBuilding; curBuilding = curBuilding->next)
	{
		if (curBuilding->data.canResearch && curBuilding->IsWorking)
		{
			researchCentersWorkingOnArea[curBuilding->researchArea] += 1;
			newResearchCount++;
		}
	}
	workingResearchCenterCount = newResearchCount;
}

//------------------------------------------------------------------------------
bool cPlayer::mayHaveOffensiveUnit() const
{
	for (const cVehicle* vehicle = VehicleList; vehicle; vehicle = vehicle->next)
	{
		if (vehicle->data.canAttack || !vehicle->data.canBuild.empty()) return true;
	}
	for (const cBuilding* building = BuildingList; building; building = building->next)
	{
		if (building->data.canAttack || !building->data.canBuild.empty()) return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cPlayer::deleteLock (cUnit& unit)
{
	std::vector<cUnit*>::iterator it = std::find (LockList.begin(), LockList.end(), &unit);
	if (it != LockList.end()) LockList.erase (it);
	unit.lockerPlayer = NULL;
}

//------------------------------------------------------------------------------
/** Toggles the lock state of a unit under the mouse
 * (when locked it's range and scan is displayed, although the unit is not selected).
*/
//------------------------------------------------------------------------------
void cPlayer::toggleLock (cMapField& OverUnitField)
{
	cUnit* unit = NULL;
	if (OverUnitField.getBaseBuilding() && OverUnitField.getBaseBuilding()->owner != this)
	{
		unit = OverUnitField.getBaseBuilding();
	}
	else if (OverUnitField.getTopBuilding() && OverUnitField.getTopBuilding()->owner != this)
	{
		unit = OverUnitField.getTopBuilding();
	}
	if (OverUnitField.getVehicle() && OverUnitField.getVehicle()->owner != this)
	{
		unit = OverUnitField.getVehicle();
	}
	if (OverUnitField.getPlane() && OverUnitField.getPlane()->owner != this)
	{
		unit = OverUnitField.getPlane();
	}
	if (unit == NULL) return;

	std::vector<cUnit*>::iterator it = std::find (LockList.begin(), LockList.end(), unit);
	if (it == LockList.end())
	{
		unit->lockerPlayer = this;
		LockList.push_back (unit);
	}
	else
	{
		unit->lockerPlayer = NULL;
		LockList.erase (it);
	}
}

//------------------------------------------------------------------------------
void cPlayer::drawSpecialCircle (int iX, int iY, int iRadius, std::vector<char>& map, int mapsize)
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

		int x1 = rx + iX;
		int x2 = -rx + iX;
		for (int k = x2; k <= x1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize) break;
			if (iY + ry >= 0 && iY + ry < mapsize)
				map[k + (iY + ry) * mapsize] |= 1;
			if (iY - ry >= 0 && iY - ry < mapsize)
				map[k + (iY - ry) * mapsize] |= 1;
		}

		x1 = ry + iX;
		x2 = -ry + iX;
		for (int k = x2; k <= x1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize) break;
			if (iY + rx >= 0 && iY + rx < mapsize)
				map[k + (iY + rx) *mapsize] |= 1;
			if (iY - rx >= 0 && iY - rx < mapsize)
				map[k + (iY - rx) *mapsize] |= 1;
		}
	}
}

//------------------------------------------------------------------------------
void cPlayer::drawSpecialCircleBig (int iX, int iY, int iRadius, std::vector<char>& map, int mapsize)
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

		int x1 = rx + iX;
		int x2 = -rx + iX;
		for (int k = x2; k <= x1 + 1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize) break;
			if (iY + ry >= 0 && iY + ry < mapsize)
				map[k + (iY + ry) *mapsize] |= 1;
			if (iY - ry >= 0 && iY - ry < mapsize)
				map[k + (iY - ry) *mapsize] |= 1;

			if (iY + ry + 1 >= 0 && iY + ry + 1 < mapsize)
				map[k + (iY + ry + 1) *mapsize] |= 1;
			if (iY - ry + 1 >= 0 && iY - ry + 1 < mapsize)
				map[k + (iY - ry + 1) *mapsize] |= 1;
		}

		x1 = ry + iX;
		x2 = -ry + iX;
		for (int k = x2; k <= x1 + 1; k++)
		{
			if (k < 0) continue;
			if (k >= mapsize) break;
			if (iY + rx >= 0 && iY + rx < mapsize)
				map[k + (iY + rx) *mapsize] |= 1;
			if (iY - rx >= 0 && iY - rx < mapsize)
				map[k + (iY - rx) *mapsize] |= 1;

			if (iY + rx + 1 >= 0 && iY + rx + 1 < mapsize)
				map[k + (iY + rx + 1) *mapsize] |= 1;
			if (iY - rx + 1 >= 0 && iY - rx + 1 < mapsize)
				map[k + (iY - rx + 1) *mapsize] |= 1;
		}
	}
}

//------------------------------------------------------------------------------
const sSavedReportMessage& cPlayer::addSavedReport (const string& message, sSavedReportMessage::eReportTypes type, sID unitID, int xPos, int yPos, int colorNr)
{
	sSavedReportMessage savedReport;
	savedReport.message = message;
	savedReport.type = type;
	savedReport.xPos = xPos;
	savedReport.yPos = yPos;
	savedReport.unitID = unitID;
	savedReport.colorNr = colorNr;

	savedReportsList.push_back (savedReport);
	return savedReportsList.back();
}
