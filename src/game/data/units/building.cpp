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

#include "game/data/units/building.h"

#include "game/data/map/mapfieldview.h"
#include "game/data/map/mapview.h"
#include "game/data/player/player.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "game/data/units/vehicle.h"
#include "game/logic/client.h"
#include "game/logic/fxeffects.h"
#include "game/logic/upgradecalculator.h"
#include "protocol/netmessage.h"
#include "settings.h"
#include "utility/crc.h"
#include "utility/language.h"
#include "utility/listhelpers.h"
#include "utility/mathtools.h"
#include "utility/random.h"
#include "utility/string/toString.h"

#include <cmath>

using namespace std;


//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem()
{}

//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem (sID type_, int remainingMetal_) :
	type (type_),
	remainingMetal (remainingMetal_)
{}

//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem (const cBuildListItem& other) :
	type (other.type),
	remainingMetal (other.remainingMetal)
{}

//--------------------------------------------------------------------------
cBuildListItem::cBuildListItem (cBuildListItem&& other) :
	type (std::move (other.type)),
	remainingMetal (std::move (other.remainingMetal))
{}

//--------------------------------------------------------------------------
cBuildListItem& cBuildListItem::operator= (const cBuildListItem& other)
{
	type = other.type;
	remainingMetal = other.remainingMetal;
	return *this;
}

//--------------------------------------------------------------------------
cBuildListItem& cBuildListItem::operator= (cBuildListItem && other)
{
	type = std::move (other.type);
	remainingMetal = std::move (other.remainingMetal);
	return *this;
}

//--------------------------------------------------------------------------
const sID& cBuildListItem::getType() const
{
	return type;
}

//--------------------------------------------------------------------------
void cBuildListItem::setType (const sID& type_)
{
	auto oldType = type;
	type = type_;
	if (type != oldType) typeChanged();
}

//--------------------------------------------------------------------------
int cBuildListItem::getRemainingMetal() const
{
	return remainingMetal;
}

//--------------------------------------------------------------------------
void cBuildListItem::setRemainingMetal (int value)
{
	std::swap (remainingMetal, value);
	if (value != remainingMetal) remainingMetalChanged();
}

//--------------------------------------------------------------------------
uint32_t cBuildListItem::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(type, crc);
	crc = calcCheckSum(remainingMetal, crc);

	return crc;
}

//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding (const cStaticUnitData* staticData, const cDynamicUnitData* data, cPlayer* owner, unsigned int ID) :
	cUnit(data, staticData, owner, ID),
	effectAlpha(0),
	wasWorking(false),
	isWorking (false),
	metalPerRound(0)
{
	setSentryActive (staticData && staticData->canAttack != TERRAIN_NONE);

	rubbleTyp = 0;
	rubbleValue = 0;
	researchArea = cResearch::kAttackResearch;

	points = 0;

	BaseN = false;
	BaseBN = false;
	BaseE = false;
	BaseBE = false;
	BaseS = false;
	BaseBS = false;
	BaseW = false;
	BaseBW = false;
	repeatBuild = false;

	subBase = nullptr;
	buildSpeed = 0;

	if (isBig)
	{
		DamageFXPointX  = random (64) + 32;
		DamageFXPointY  = random (64) + 32;
		DamageFXPointX2 = random (64) + 32;
		DamageFXPointY2 = random (64) + 32;
	}
	else
	{
		DamageFXPointX = random (64 - 24);
		DamageFXPointY = random (64 - 24);
		DamageFXPointX2 = 0;
		DamageFXPointY2 = 0;
	}

	refreshData();

	buildListChanged.connect ([&]() { statusChanged(); });
	buildListFirstItemDataChanged.connect ([&]() { statusChanged(); });
	researchAreaChanged.connect ([&]() { statusChanged(); });
	metalPerRoundChanged.connect([&]() { statusChanged(); });
	ownerChanged.connect ([&]() { registerOwnerEvents(); });

	registerOwnerEvents();
}

//--------------------------------------------------------------------------
cBuilding::~cBuilding()
{
}

//----------------------------------------------------
/** Returns a string with the current state */
//----------------------------------------------------
string cBuilding::getStatusStr (const cPlayer* whoWantsToKnow, const cUnitsData& unitsData) const
{
	auto font = cUnicodeFont::font.get();
	if (isDisabled())
	{
		string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (getDisabledTurns()) + ")";
		return sText;
	}
	if (isUnitWorking() || (factoryHasJustFinishedBuilding() && isDisabled() == false))
	{
		// Factory:
		if (!staticData->canBuild.empty() && !buildList.empty() && getOwner() == whoWantsToKnow)
		{
			const cBuildListItem& buildListItem = buildList[0];
			const string& unitName = unitsData.getStaticUnitData(buildListItem.getType()).getName();
			string sText;

			if (buildListItem.getRemainingMetal() > 0)
			{
				int iRound;

				iRound = (int) ceilf (buildListItem.getRemainingMetal() / (float)getMetalPerRound());
				sText = lngPack.i18n ("Text~Comp~Producing") + lngPack.i18n ("Text~Punctuation~Colon");
				sText += unitName + " (";
				sText += iToStr (iRound) + ")";

				if (font->getTextWide (sText, FONT_LATIN_SMALL_WHITE) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing") + lngPack.i18n("Text~Punctuation~Colon") + "\n";
					sText += unitName + " (";
					sText += iToStr (iRound) + ")";
				}

				return sText;
			}
			else //new unit is rdy + which kind of unit
			{
				sText = lngPack.i18n ("Text~Comp~Producing_Fin");
				sText += lngPack.i18n ("Text~Punctuation~Colon");
				sText += unitName;

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing_Fin") + lngPack.i18n("Text~Punctuation~Colon");
					sText += "\n";
					sText += unitName;
				}
				return sText;
			}
		}

		// Research Center
		if (getStaticData().canResearch && getOwner() == whoWantsToKnow && getOwner())
		{
			string sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area) > 0)
				{
					switch (area)
					{
						case cResearch::kAttackResearch: sText += lngPack.i18n ("Text~Others~Attack"); break;
						case cResearch::kShotsResearch: sText += lngPack.i18n ("Text~Others~Shots_7"); break;
						case cResearch::kRangeResearch: sText += lngPack.i18n ("Text~Others~Range"); break;
						case cResearch::kArmorResearch: sText += lngPack.i18n ("Text~Others~Armor_7"); break;
						case cResearch::kHitpointsResearch: sText += lngPack.i18n ("Text~Others~Hitpoints_7"); break;
						case cResearch::kSpeedResearch: sText += lngPack.i18n ("Text~Others~Speed"); break;
						case cResearch::kScanResearch: sText += lngPack.i18n ("Text~Others~Scan"); break;
						case cResearch::kCostResearch: sText += lngPack.i18n ("Text~Others~Costs"); break;
					}
					sText += lngPack.i18n ("Text~Punctuation~Colon") + iToStr (getOwner()->getResearchState().getRemainingTurns (area, getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area))) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if (getStaticData().convertsGold && getOwner() == whoWantsToKnow && getOwner())
		{
			string sText;
			sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			sText += lngPack.i18n ("Text~Title~Credits") + lngPack.i18n ("Text~Punctuation~Colon");
			sText += iToStr (getOwner()->getCredits());
			return sText;
		}
		return lngPack.i18n ("Text~Comp~Working");
	}

	if (isAttacking())
		return lngPack.i18n ("Text~Comp~AttackingStatusStr");
	else if (isBeeingAttacked())
		return lngPack.i18n ("Text~Comp~IsBeeingAttacked");
	else if (isSentryActive())
		return lngPack.i18n ("Text~Comp~Sentry");
	else if (isManualFireActive())
		return lngPack.i18n ("Text~Comp~ReactionFireOff");

	//GoldRaf idle + gold-amount
	if (getStaticData().convertsGold && getOwner() == whoWantsToKnow && getOwner() && !isUnitWorking())
	{
		string sText;
		sText = lngPack.i18n("Text~Comp~Waits") + "\n";
		sText += lngPack.i18n("Text~Title~Credits") + lngPack.i18n("Text~Punctuation~Colon");
		sText += iToStr(getOwner()->getCredits());
		return sText;
	}

	//Research centre idle + projects
	// Research Center
	if (getStaticData().canResearch && getOwner() == whoWantsToKnow && getOwner() && !isUnitWorking())
	{
		string sText = lngPack.i18n("Text~Comp~Waits") + "\n";
		for (int area = 0; area < cResearch::kNrResearchAreas; area++)
		{
			if (getOwner()->getResearchCentersWorkingOnArea((cResearch::ResearchArea)area) > 0)
			{
				switch (area)
				{
				case cResearch::kAttackResearch: sText += lngPack.i18n("Text~Others~Attack"); break;
				case cResearch::kShotsResearch: sText += lngPack.i18n("Text~Others~Shots_7"); break;
				case cResearch::kRangeResearch: sText += lngPack.i18n("Text~Others~Range"); break;
				case cResearch::kArmorResearch: sText += lngPack.i18n("Text~Others~Armor_7"); break;
				case cResearch::kHitpointsResearch: sText += lngPack.i18n("Text~Others~Hitpoints_7"); break;
				case cResearch::kSpeedResearch: sText += lngPack.i18n("Text~Others~Speed"); break;
				case cResearch::kScanResearch: sText += lngPack.i18n("Text~Others~Scan"); break;
				case cResearch::kCostResearch: sText += lngPack.i18n("Text~Others~Costs"); break;
				}
				sText += lngPack.i18n("Text~Punctuation~Colon") + iToStr(getOwner()->getResearchState().getRemainingTurns(area, getOwner()->getResearchCentersWorkingOnArea((cResearch::ResearchArea)area))) + "\n";
			}
		}
		return sText;
	}


	return lngPack.i18n ("Text~Comp~Waits");
}

//--------------------------------------------------------------------------
/** Refreshs all data to the maximum values */
//--------------------------------------------------------------------------
bool cBuilding::refreshData()
{
	// NOTE: according to MAX 1.04 units get their shots/movepoints back even if they are disabled

	if (data.getShots() < data.getShotsMax())
	{
		data.setShots (std::min (this->data.getShotsMax(), this->data.getAmmo()));
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cBuilding::connectFirstBuildListItem()
{
	buildListFirstItemSignalConnectionManager.disconnectAll();
	if (!buildList.empty())
	{
		buildListFirstItemSignalConnectionManager.connect(buildList[0].remainingMetalChanged, [this]() { buildListFirstItemDataChanged(); });
		buildListFirstItemSignalConnectionManager.connect(buildList[0].typeChanged, [this]() { buildListFirstItemDataChanged(); });
	}
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (const cMap& map)
{
	if (!getOwner()) return;
	if (!isBig)
	{
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0, -1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 1,  0), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0,  1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition (-1,  0), *this, map);
	}
	else
	{
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0, -1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 1, -1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 2,  0), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 2,  1), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 0,  2), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition ( 1,  2), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition (-1,  0), *this, map);
		getOwner()->base.checkNeighbour (getPosition() + cPosition (-1,  1), *this, map);
	}
	CheckNeighbours (map);
}

//--------------------------------------------------------------------------
/** Checks, if there are neighbours */
//--------------------------------------------------------------------------
void cBuilding::CheckNeighbours (const cMap& map)
{
	if (!getOwner()) return;
#define CHECK_NEIGHBOUR(x, y, m) \
	if (map.isValidPosition (cPosition(x, y))) \
	{ \
		const cBuilding* b = map.getField(cPosition(x, y)).getTopBuilding(); \
		if (b && b->getOwner() && b->getOwner() == getOwner() && b->getStaticData().connectsToBase) \
		{m = true;}else{m = false;} \
	}

	if (!isBig)
	{
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() - 1, BaseN)
		CHECK_NEIGHBOUR (getPosition().x() + 1, getPosition().y()    , BaseE)
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() + 1, BaseS)
		CHECK_NEIGHBOUR (getPosition().x() - 1, getPosition().y()    , BaseW)
	}
	else
	{
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() - 1, BaseN)
		CHECK_NEIGHBOUR (getPosition().x() + 1, getPosition().y() - 1, BaseBN)
		CHECK_NEIGHBOUR (getPosition().x() + 2, getPosition().y()    , BaseE)
		CHECK_NEIGHBOUR (getPosition().x() + 2, getPosition().y() + 1, BaseBE)
		CHECK_NEIGHBOUR (getPosition().x()    , getPosition().y() + 2, BaseS)
		CHECK_NEIGHBOUR (getPosition().x() + 1, getPosition().y() + 2, BaseBS)
		CHECK_NEIGHBOUR (getPosition().x() - 1, getPosition().y()    , BaseW)
		CHECK_NEIGHBOUR (getPosition().x() - 1, getPosition().y() + 1, BaseBW)
	}
}

//--------------------------------------------------------------------------
/** starts the building for the server thread */
//--------------------------------------------------------------------------
void cBuilding::startWork ()
{
	if (isUnitWorking())
	{
		return;
	}

	if (isDisabled())
	{
		//TODO: is report needed?
		//sendSavedReport (server, cSavedReportSimple (eSavedReportType::BuildingDisabled), getOwner());
		return;
	}

	if (!subBase->startBuilding (*this))
		return;

	// research building
	if (getStaticData().canResearch && getOwner())
	{
		getOwner()->startAResearch (researchArea);
	}

	if (getStaticData().canScore)
	{
		//sendNumEcos (server, *getOwner());
	}
}

//--------------------------------------------------------------------------
/** Stops the building's working */
//--------------------------------------------------------------------------
void cBuilding::stopWork (bool forced)
{
	if (!isUnitWorking()) return;

	if (!subBase->stopBuilding (*this, forced))
		return;

	if (getStaticData().canResearch && getOwner())
	{
		getOwner()->stopAResearch (researchArea);
	}

	if (getStaticData().canScore)
	{
		//sendNumEcos (server, *getOwner());
	}
}

bool cBuilding::canTransferTo(const cPosition& position, const cMapView& map) const
{
	const auto& field = map.getField(position);

	const cUnit* unit = field.getVehicle();
	if (unit)
	{
		return canTransferTo(*unit);
	}

	unit = field.getTopBuilding();
	if (unit)
	{
		return canTransferTo(*unit);
	}

	return false;
}

//------------------------------------------------------------
bool cBuilding::canTransferTo (const cUnit& unit) const
{
	if (unit.getOwner() != getOwner())
		return false;

	if (&unit == this)
		return false;


	if (unit.isAVehicle())
	{
		const cVehicle* v = static_cast<const cVehicle*>(&unit);


		if (v->getStaticUnitData().storeResType != staticData->storeResType)
			return false;

		if (v->isUnitBuildingABuilding() || v->isUnitClearing())
			return false;

		for (const auto b : subBase->getBuildings())
		{
			if (b->isNextTo (v->getPosition())) return true;
		}

		return false;
	}
	else if (unit.isABuilding())
	{
		const cBuilding* b = static_cast<const cBuilding*>(&unit);
		if (b->subBase != subBase)
			return false;

		if (staticData->storeResType != b->getStaticUnitData().storeResType)
			return false;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo(const cPosition& position, const cMap& map, const cStaticUnitData& vehicleData) const
{
	if (!map.possiblePlaceVehicle(vehicleData, position, getOwner())) return false;
	if (!isNextTo(position)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo(const cPosition& position, const cMapView& map, const cStaticUnitData& vehicleData) const
{
	if (!map.possiblePlaceVehicle(vehicleData, position)) return false;
	if (!isNextTo(position)) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cPosition& position, const cMapView& map, bool checkPosition) const
{
	if (map.isValidPosition (position) == false) return false;

	if (canLoad (map.getField (position).getPlane(), checkPosition)) return true;
	else return canLoad (map.getField (position).getVehicle(), checkPosition);
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad (const cVehicle* vehicle, bool checkPosition) const
{
	if (!vehicle) return false;

	if (vehicle->isUnitLoaded()) return false;

	if (storedUnits.size() == staticData->storageUnitsMax) return false;

	if (checkPosition && !isNextTo (vehicle->getPosition())) return false;

	if (!Contains(staticData->storeUnitsTypes, vehicle->getStaticData().isStorageType)) return false;

	if (vehicle->isUnitMoving() || vehicle->isAttacking()) return false;

	if (vehicle->getOwner() != getOwner() || vehicle->isUnitBuildingABuilding() || vehicle->isUnitClearing()) return false;

	if (vehicle->isBeeingAttacked()) return false;

	return true;
}

//------------------------------------------------------------------------------
bool cBuilding::canSupply(const cUnit* unit, eSupplyType supplyType) const
{
	if (unit == nullptr || unit->isABuilding())
		return false;

	if (subBase->getResourcesStored().metal <= 0)
		return false;

	if (!Contains(storedUnits, static_cast<const cVehicle*>(unit)))
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

//-------------------------------------------------------------------------------
/** checks the resources that are available under the mining station */
//--------------------------------------------------------------------------

void cBuilding::initMineResourceProd (const cMap& map)
{
	if (!getStaticData().canMineMaxRes) return;

	auto position = getPosition();

	maxProd = {0, 0, 0};
	const sResources* res = &map.getResource (position);

	if (res->typ != eResourceType::None) maxProd.get (res->typ) += res->value;

	if (isBig)
	{
		position.x()++;
		res = &map.getResource(position);
		if (res->typ != eResourceType::None) maxProd.get (res->typ) += res->value;

		position.y()++;
		res = &map.getResource(position);
		if (res->typ != eResourceType::None) maxProd.get (res->typ) += res->value;

		position.x()--;
		res = &map.getResource(position);
		if (res->typ != eResourceType::None) maxProd.get (res->typ) += res->value;
	}

	maxProd.metal = min (maxProd.metal, getStaticData().canMineMaxRes);
	maxProd.oil = min (maxProd.oil, getStaticData().canMineMaxRes);
	maxProd.gold = min (maxProd.gold, getStaticData().canMineMaxRes);

	// set default mine allocation
	int freeProductionCapacity = getStaticData().canMineMaxRes;
	prod.metal = maxProd.metal;
	freeProductionCapacity -= prod.metal;
	prod.gold = min(maxProd.gold, freeProductionCapacity);
	freeProductionCapacity -= prod.gold;
	prod.oil = min(maxProd.oil, freeProductionCapacity);
}

//--------------------------------------------------------------------------
/** calculates the costs and the duration of the 3 buildspeeds
 * for the vehicle with the given base costs
 * iRemainingMetal is only needed for recalculating costs of vehicles
 * in the Buildqueue and is set per default to -1 */
//--------------------------------------------------------------------------
void cBuilding::calcTurboBuild (std::array<int, 3>& turboBuildRounds, std::array<int, 3>& turboBuildCosts, int vehicleCosts, int remainingMetal) const
{
	// first calc costs for a new Vehical

	// 1x
	turboBuildCosts[0] = vehicleCosts;

	// 2x
	int a = turboBuildCosts[0];
	turboBuildCosts[1] = turboBuildCosts[0];

	while (a >= 2 * staticData->needsMetal)
	{
		turboBuildCosts[1] += 2 * staticData->needsMetal;
		a -= 2 * staticData->needsMetal;
	}

	// 4x
	turboBuildCosts[2] = turboBuildCosts[1];
	a = turboBuildCosts[1];

	while (a >= 15)
	{
		turboBuildCosts[2] += (12 * staticData->needsMetal - min(a, 8 * staticData->needsMetal));
		a -= 8 * staticData->needsMetal;
	}

	// now this is a little bit tricky ...
	// trying to calculate a plausible value,
	// if we are changing the speed of an already started build-job
	if (remainingMetal >= 0)
	{
		float WorkedRounds;

		switch (buildSpeed)  // BuildSpeed here is the previous build speed
		{
			case 0:
				WorkedRounds = (turboBuildCosts[0] - remainingMetal) / (1.f * staticData->needsMetal);
				turboBuildCosts[0] -= (int) (1     *  1 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (0.5f  *  4 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (0.25f * 12 * staticData->needsMetal * WorkedRounds);
				break;

			case 1:
				WorkedRounds = (turboBuildCosts[1] - remainingMetal) / (float)(4 * staticData->needsMetal);
				turboBuildCosts[0] -= (int) (2    *  1 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (1    *  4 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (0.5f * 12 * staticData->needsMetal * WorkedRounds);
				break;

			case 2:
				WorkedRounds = (turboBuildCosts[2] - remainingMetal) / (float)(12 * staticData->needsMetal);
				turboBuildCosts[0] -= (int) (4 *  1 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[1] -= (int) (2 *  4 * staticData->needsMetal * WorkedRounds);
				turboBuildCosts[2] -= (int) (1 * 12 * staticData->needsMetal * WorkedRounds);
				break;
		}
	}

	// calc needed turns
	turboBuildRounds[0] = (int) ceilf (turboBuildCosts[0] / (1.f * staticData->needsMetal));

	if (getStaticData().maxBuildFactor > 1)
	{
		turboBuildRounds[1] = (int) ceilf (turboBuildCosts[1] / (4.f * staticData->needsMetal));
		turboBuildRounds[2] = (int) ceilf (turboBuildCosts[2] / (12.f * staticData->needsMetal));
	}
	else
	{
		turboBuildRounds[1] = 0;
		turboBuildRounds[2] = 0;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- methods, that already have been extracted as part of cUnit refactoring
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool cBuilding::factoryHasJustFinishedBuilding() const
{
	return (!buildList.empty() && isUnitWorking() == false && buildList[0].getRemainingMetal() <= 0);
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeStarted() const
{
	return (getStaticData().canWork && isUnitWorking() == false
		&& (!buildList.empty() || staticData->canBuild.empty()));
}

//-----------------------------------------------------------------------------
bool cBuilding::buildingCanBeUpgraded() const
{
	if (!getOwner()) return false;
	const cDynamicUnitData& upgraded = *getOwner()->getUnitDataCurrentVersion (data.getId());
	return data.getVersion() != upgraded.getVersion() && subBase && subBase->getResourcesStored().metal >= 2;
}

//-----------------------------------------------------------------------------
bool cBuilding::isBuildListEmpty() const
{
	return buildList.empty();
}

//-----------------------------------------------------------------------------
size_t cBuilding::getBuildListSize() const
{
	return buildList.size();
}

//-----------------------------------------------------------------------------
const cBuildListItem& cBuilding::getBuildListItem (size_t index) const
{
	return buildList[index];
}

//-----------------------------------------------------------------------------
cBuildListItem& cBuilding::getBuildListItem (size_t index)
{
	return buildList[index];
}

//-----------------------------------------------------------------------------
void cBuilding::setBuildList (std::vector<cBuildListItem> buildList_)
{
	buildList = std::move (buildList_);

	connectFirstBuildListItem();

	buildListChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::addBuildListItem (cBuildListItem item)
{
	buildList.push_back (std::move (item));

	connectFirstBuildListItem();

	buildListChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::removeBuildListItem (size_t index)
{
	buildList.erase (buildList.begin() + index);

	connectFirstBuildListItem();

	buildListChanged();
}

//-----------------------------------------------------------------------------
int cBuilding::getBuildSpeed() const
{
	return buildSpeed;
}

//-----------------------------------------------------------------------------
int cBuilding::getMetalPerRound() const
{
	if (buildList.size() > 0)
		return min(metalPerRound, buildList[0].getRemainingMetal());
	else
		return 0;
}

//-----------------------------------------------------------------------------
bool cBuilding::getRepeatBuild() const
{
	return repeatBuild;
}

//-----------------------------------------------------------------------------
void cBuilding::setWorking (bool value)
{
	std::swap (isWorking, value);
	if (value != isWorking) workingChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::setBuildSpeed(int value)
{
	std::swap(buildSpeed, value);
	if(value != buildSpeed) buildSpeedChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::setMetalPerRound(int value)
{
	std::swap(metalPerRound, value);
	if(value != metalPerRound) metalPerRoundChanged();
}

//-----------------------------------------------------------------------------
void cBuilding::setRepeatBuild(bool value)
{
	std::swap(repeatBuild, value);
	if(value != repeatBuild) repeatBuildChanged();
}

//-----------------------------------------------------------------------------
const sMiningResource& cBuilding::getMaxProd() const
{
	return maxProd;
}

//-----------------------------------------------------------------------------
void cBuilding::setResearchArea (cResearch::ResearchArea area)
{
	std::swap (researchArea, area);
	if (researchArea != area) researchAreaChanged();
}

//-----------------------------------------------------------------------------
cResearch::ResearchArea cBuilding::getResearchArea() const
{
	return researchArea;
}

//------------------------------------------------------------------------------
void cBuilding::setRubbleValue(int value, cCrossPlattformRandom& randomGenerator)
{
	rubbleValue = value;

	if (isBig)
	{
		rubbleTyp = randomGenerator.get(2);
	}
	else
	{
		rubbleTyp = randomGenerator.get(5);
	}
}

//------------------------------------------------------------------------------
int cBuilding::getRubbleValue() const
{
	return rubbleValue;
}

//------------------------------------------------------------------------------
uint32_t cBuilding::getChecksum(uint32_t crc) const
{
	crc = cUnit::getChecksum(crc);
	crc = calcCheckSum(rubbleTyp, crc);
	crc = calcCheckSum(rubbleValue, crc);
	crc = calcCheckSum(BaseN, crc);
	crc = calcCheckSum(BaseE, crc);
	crc = calcCheckSum(BaseS, crc);
	crc = calcCheckSum(BaseW, crc);
	crc = calcCheckSum(BaseBN, crc);
	crc = calcCheckSum(BaseBE, crc);
	crc = calcCheckSum(BaseBS, crc);
	crc = calcCheckSum(BaseBW, crc);
	crc = calcCheckSum(prod, crc);
	crc = calcCheckSum(wasWorking, crc);
	crc = calcCheckSum(points, crc);
	crc = calcCheckSum(isWorking, crc);
	crc = calcCheckSum(buildSpeed, crc);
	crc = calcCheckSum(metalPerRound, crc);
	crc = calcCheckSum(repeatBuild, crc);
	crc = calcCheckSum(maxProd, crc);
	crc = calcCheckSum(researchArea, crc);
	crc = calcCheckSum(buildList, crc);

	return crc;
}

//-----------------------------------------------------------------------------
void cBuilding::registerOwnerEvents()
{
	ownerSignalConnectionManager.disconnectAll();

	if (getOwner() == nullptr || staticData == nullptr) return;

	if (getStaticData().convertsGold)
	{
		ownerSignalConnectionManager.connect (getOwner()->creditsChanged, [&]() { statusChanged(); });
	}

	if (getStaticData().canResearch)
	{
		ownerSignalConnectionManager.connect (getOwner()->researchCentersWorkingOnAreaChanged, [&] (cResearch::ResearchArea) { statusChanged(); });
		ownerSignalConnectionManager.connect (getOwner()->getResearchState().neededResearchPointsChanged, [&] (cResearch::ResearchArea) { statusChanged(); });
		ownerSignalConnectionManager.connect (getOwner()->getResearchState().currentResearchPointsChanged, [&] (cResearch::ResearchArea) { statusChanged(); });
	}
}
