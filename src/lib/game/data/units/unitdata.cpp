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

#include "unitdata.h"

#include "game/data/player/clans.h"
#include "utility/crc.h"
#include "utility/log.h"

#include <algorithm>

cUnitsData UnitsDataGlobal;

namespace serialization
{
	const std::vector<std::pair<eMuzzleType, const char*>>
	sEnumStringMapping<eMuzzleType>::m =
	{
		{eMuzzleType::None, "None"},
		{eMuzzleType::Big, "Big"},
		{eMuzzleType::Rocket, "Rocket"},
		{eMuzzleType::Small, "Small"},
		{eMuzzleType::Med, "Med"},
		{eMuzzleType::MedLong, "MedLong"},
		{eMuzzleType::RocketCluster, "RocketCluster"},
		{eMuzzleType::Torpedo, "Torpedo"},
		{eMuzzleType::Sniper, "Sniper"}
	};
	const std::vector<std::pair<eSurfacePosition, const char*>>
	sEnumStringMapping<eSurfacePosition>::m =
	{
		{eSurfacePosition::BeneathSea, "BeneathSea"},
		{eSurfacePosition::AboveSea, "AboveSea"},
		{eSurfacePosition::Base, "Base"},
		{eSurfacePosition::AboveBase, "AboveBase"},
		{eSurfacePosition::Ground, "Ground"},
		{eSurfacePosition::Above, "Above"}
	};
	const std::vector<std::pair<eOverbuildType, const char*>>
	sEnumStringMapping<eOverbuildType>::m =
	{
		{eOverbuildType::No, "No"},
		{eOverbuildType::Yes, "Yes"},
		{eOverbuildType::YesNRemove, "YesNRemove"}
	};
	const std::vector<std::pair<eStorageUnitsImageType, const char*>>
	sEnumStringMapping<eStorageUnitsImageType>::m =
	{
		{eStorageUnitsImageType::None, "None"},
		{eStorageUnitsImageType::Tank, "Tank"},
		{eStorageUnitsImageType::Plane, "Plane"},
		{eStorageUnitsImageType::Ship, "Ship"},
		{eStorageUnitsImageType::Human, "Human"}
	};

} // namespace serialization
//------------------------------------------------------------------------------
uint32_t sStaticCommonUnitData::computeChecksum (uint32_t crc) const
{
	crc = calcCheckSum (buildAs, crc);
	crc = calcCheckSum (canAttack, crc);
	crc = calcCheckSum (canBeCaptured, crc);
	crc = calcCheckSum (canBeDisabled, crc);
	crc = calcCheckSum (canBuild, crc);
	crc = calcCheckSum (canDetectStealthOn, crc);
	crc = calcCheckSum (canRearm, crc);
	crc = calcCheckSum (canRepair, crc);
	crc = calcCheckSum (doesSelfRepair, crc);
	crc = calcCheckSum (factorGround, crc);
	crc = calcCheckSum (factorSea, crc);
	crc = calcCheckSum (factorAir, crc);
	crc = calcCheckSum (factorCoast, crc);
	crc = calcCheckSum (isStealthOn, crc);
	crc = calcCheckSum (muzzleType, crc);
	crc = calcCheckSum (needsMetal, crc);
	crc = calcCheckSum (needsEnergy, crc);
	crc = calcCheckSum (needsHumans, crc);
	crc = calcCheckSum (needsOil, crc);
	crc = calcCheckSum (produceEnergy, crc);
	crc = calcCheckSum (produceHumans, crc);
	crc = calcCheckSum (storageResMax, crc);
	crc = calcCheckSum (storageUnitsMax, crc);
	crc = calcCheckSum (storeResType, crc);
	crc = calcCheckSum (storageUnitsImageType, crc);
	crc = calcCheckSum (storeUnitsTypes, crc);
	crc = calcCheckSum (surfacePosition, crc);

	return crc;
}

//------------------------------------------------------------------------------
uint32_t sStaticBuildingData::computeChecksum (uint32_t crc) const
{
	crc = calcCheckSum (canBeLandedOn, crc);
	crc = calcCheckSum (canMineMaxRes, crc);
	crc = calcCheckSum (canBeOverbuild, crc);
	crc = calcCheckSum (canResearch, crc);
	crc = calcCheckSum (canScore, crc);
	crc = calcCheckSum (canSelfDestroy, crc);
	crc = calcCheckSum (canWork, crc);
	crc = calcCheckSum (connectsToBase, crc);
	crc = calcCheckSum (convertsGold, crc);
	crc = calcCheckSum (explodesOnContact, crc);
	crc = calcCheckSum (isBig, crc);
	crc = calcCheckSum (maxBuildFactor, crc);
	crc = calcCheckSum (modifiesSpeed, crc);

	return crc;
}

//------------------------------------------------------------------------------
uint32_t sStaticVehicleData::computeChecksum (uint32_t crc) const
{
	crc = calcCheckSum (animationMovement, crc);
	crc = calcCheckSum (canBuildPath, crc);
	crc = calcCheckSum (canCapture, crc);
	crc = calcCheckSum (canClearArea, crc);
	crc = calcCheckSum (canDriveAndFire, crc);
	crc = calcCheckSum (canDisable, crc);
	crc = calcCheckSum (canPlaceMines, crc);
	crc = calcCheckSum (canSurvey, crc);
	crc = calcCheckSum (hasCorpse, crc);
	crc = calcCheckSum (isHuman, crc);
	crc = calcCheckSum (isStorageType, crc);
	crc = calcCheckSum (makeTracks, crc);
	return crc;
}

//------------------------------------------------------------------------------
uint32_t sSpecialBuildingsId::computeChecksum (uint32_t crc) const
{
	crc = calcCheckSum (alienFactory, crc);
	crc = calcCheckSum (connector, crc);
	crc = calcCheckSum (landMine, crc);
	crc = calcCheckSum (mine, crc);
	crc = calcCheckSum (seaMine, crc);
	crc = calcCheckSum (smallBeton, crc);
	crc = calcCheckSum (smallGenerator, crc);
	return crc;
}

//------------------------------------------------------------------------------
void sSpecialBuildingsId::logMissing() const
{
	if (alienFactory == 0) Log.error ("special \"alienFactory\" missing");
	if (connector == 0) Log.error ("special \"connector\" missing");
	if (landMine == 0) Log.error ("special \"landmine\" missing");
	if (mine == 0) Log.error ("special \"mine\" missing");
	if (seaMine == 0) Log.error ("special \"seamine\" missing");
	if (smallBeton == 0) Log.error ("special \"smallBeton\" missing");
	if (smallGenerator == 0) Log.error ("special \"energy\" missing");
}

//------------------------------------------------------------------------------
uint32_t sSpecialVehiclesId::computeChecksum (uint32_t crc) const
{
	crc = calcCheckSum (constructor, crc);
	crc = calcCheckSum (engineer, crc);
	crc = calcCheckSum (surveyor, crc);
	return crc;
}

//------------------------------------------------------------------------------
void sSpecialVehiclesId::logMissing() const
{
	if (constructor == 0) Log.error ("Constructor index not found. Constructor needs to have the property \"Can_Build = BigBuilding\"");
	if (engineer == 0) Log.error ("Engineer index not found. Engineer needs to have the property \"Can_Build = SmallBuilding\"");
	if (surveyor == 0) Log.error ("Surveyor index not found. Surveyor needs to have the property \"Can_Survey = Yes\"");
}

//------------------------------------------------------------------------------
cUnitsData::cUnitsData()
{
	rubbleSmall.ID = {2, 1};
	rubbleSmall.buildingData.isBig = false;
	rubbleBig.ID = {2, 2};
	rubbleBig.buildingData.isBig = true;
}

//------------------------------------------------------------------------------
void cUnitsData::initializeIDData()
{
	for (const auto& data : staticUnitData)
	{
		if (data.canBuild == "BigBuilding")
			specialVehicles.constructor = data.ID.secondPart;
		if (data.canBuild == "SmallBuilding")
			specialVehicles.engineer = data.ID.secondPart;
		if (data.vehicleData.canSurvey)
			specialVehicles.surveyor = data.ID.secondPart;
	}
	specialVehicles.logMissing();

	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
void cUnitsData::initializeClanUnitData (const cClanData& clanData)
{
	crcCache = std::nullopt;

	clanDynamicUnitData.reserve (clanData.getClans().size());

	for (const cClan& clan : clanData.getClans())
	{
		clanDynamicUnitData.emplace_back();
		std::vector<cDynamicUnitData>& clanListVehicles = clanDynamicUnitData.back();

		// make a copy of the vehicle's stats
		clanListVehicles = dynamicUnitData;
		for (cDynamicUnitData& clanVehicle : clanListVehicles)
		{
			const cClanUnitStat* changedStat = clan.getUnitStat (clanVehicle.getId());
			if (changedStat == nullptr) continue;

			if (const auto modif = changedStat->getModificationValue (eClanModification::Damage))
				clanVehicle.setDamage (*modif);
			if (const auto modif = changedStat->getModificationValue (eClanModification::Range))
				clanVehicle.setRange (*modif);
			if (const auto modif = changedStat->getModificationValue (eClanModification::Armor))
				clanVehicle.setArmor (*modif);
			if (const auto modif = changedStat->getModificationValue (eClanModification::Hitpoints))
				clanVehicle.setHitpointsMax (*modif);
			if (const auto modif = changedStat->getModificationValue (eClanModification::Scan))
				clanVehicle.setScan (*modif);
			if (const auto modif = changedStat->getModificationValue (eClanModification::Speed))
				clanVehicle.setSpeedMax (*modif * 4);
			if (const auto modif = changedStat->getModificationValue (eClanModification::Built_Costs))
				clanVehicle.setBuildCost (*modif);
		}
	}
}

//------------------------------------------------------------------------------
bool cUnitsData::isValidId (const sID& id) const
{
	if (ranges::any_of (staticUnitData, [&] (const auto& unitData) { return unitData.ID == id; }))
	{
		return true;
	}
	Log.error ("Unitdata with id (" + std::to_string (id.firstPart) + ", " + std::to_string (id.secondPart) + ") not found");
	return false;
}

//------------------------------------------------------------------------------
size_t cUnitsData::getNrOfClans() const
{
	return clanDynamicUnitData.size();
}

//------------------------------------------------------------------------------
const cDynamicUnitData& cUnitsData::getDynamicUnitData (const sID& id, int clan /*= -1*/) const
{
	if (clan < 0 || static_cast<unsigned> (clan) >= clanDynamicUnitData.size())
	{
		for (const auto& data : dynamicUnitData)
		{
			if (data.getId() == id) return data;
		}
		throw std::runtime_error ("Unitdata not found " + id.getText());
	}
	else
	{
		for (const auto& data : clanDynamicUnitData[clan])
		{
			if (data.getId() == id) return data;
		}
		throw std::runtime_error ("Unitdata not found " + id.getText());
	}
}

//------------------------------------------------------------------------------
const cStaticUnitData& cUnitsData::getStaticUnitData (const sID& id) const
{
	for (const auto& data : staticUnitData)
	{
		if (data.ID == id) return data;
	}
	throw std::runtime_error ("Unitdata not found " + id.getText());
}

//------------------------------------------------------------------------------
const std::vector<cDynamicUnitData>& cUnitsData::getDynamicUnitsData (int clan /*= -1*/) const
{
	if (clan < 0 || static_cast<unsigned> (clan) >= clanDynamicUnitData.size())
	{
		return dynamicUnitData;
	}
	else
	{
		return clanDynamicUnitData[clan];
	}
}

//------------------------------------------------------------------------------
const std::vector<cStaticUnitData>& cUnitsData::getStaticUnitsData() const
{
	return staticUnitData;
}

//------------------------------------------------------------------------------
uint32_t cUnitsData::getChecksum (uint32_t crc) const
{
	if (!crcCache)
	{
		crcCache = 0;
		*crcCache = specialBuildings.computeChecksum (*crcCache);
		*crcCache = specialVehicles.computeChecksum (*crcCache);
		*crcCache = calcCheckSum (staticUnitData, *crcCache);
		*crcCache = calcCheckSum (dynamicUnitData, *crcCache);
		*crcCache = calcCheckSum (clanDynamicUnitData, *crcCache);
	}

	return calcCheckSum (*crcCache, crc);
}

//------------------------------------------------------------------------------
const std::string& cStaticUnitData::getDefaultName() const
{
	return name;
}

//------------------------------------------------------------------------------
const std::string& cStaticUnitData::getDefaultDescription() const
{
	return description;
}

//------------------------------------------------------------------------------
uint32_t cStaticUnitData::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (ID, crc);
	crc = calcCheckSum (name, crc);
	crc = calcCheckSum (description, crc);

	crc = sStaticCommonUnitData::computeChecksum (crc);
	if (ID.isABuilding())
		crc = buildingData.computeChecksum (crc);
	else
		crc = vehicleData.computeChecksum (crc);

	return crc;
}

//------------------------------------------------------------------------------
cDynamicUnitData::cDynamicUnitData (const cDynamicUnitData& other) :
	id (other.id),
	buildCosts (other.buildCosts),
	version (other.version),
	speedCur (other.speedCur),
	speedMax (other.speedMax),
	hitpointsCur (other.hitpointsCur),
	hitpointsMax (other.hitpointsMax),
	shotsCur (other.shotsCur),
	shotsMax (other.shotsMax),
	ammoCur (other.ammoCur),
	ammoMax (other.ammoMax),
	range (other.range),
	scan (other.scan),
	damage (other.damage),
	armor (other.armor),
	crcCache (std::nullopt)
{}

//------------------------------------------------------------------------------
cDynamicUnitData& cDynamicUnitData::operator= (const cDynamicUnitData& other)
{
	id = other.id;
	buildCosts = other.buildCosts;
	version = other.version;
	speedCur = other.speedCur;
	speedMax = other.speedMax;
	hitpointsCur = other.hitpointsCur;
	hitpointsMax = other.hitpointsMax;
	shotsCur = other.shotsCur;
	shotsMax = other.shotsMax;
	ammoCur = other.ammoCur;
	ammoMax = other.ammoMax;
	range = other.range;
	scan = other.scan;
	damage = other.damage;
	armor = other.armor;
	crcCache = std::nullopt;

	return *this;
}

//------------------------------------------------------------------------------
sID cDynamicUnitData::getId() const
{
	return id;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setId (const sID& value)
{
	id = value;
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getBuildCost() const
{
	return buildCosts;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setBuildCost (int value)
{
	std::swap (buildCosts, value);
	if (buildCosts != value) buildCostsChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getVersion() const
{
	return version;
}

//------------------------------------------------------------------------------
bool cDynamicUnitData::canBeUpgradedTo (const cDynamicUnitData& other) const
{
	return other.dirtyVersion || version < other.version;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::makeVersionDirty()
{
	dirtyVersion = true;
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::markLastVersionUsed()
{
	if (!dirtyVersion)
	{
		return;
	}
	dirtyVersion = false;
	setVersion (version + 1);
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setVersion (int value)
{
	std::swap (version, value);
	if (version != value) versionChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getSpeed() const
{
	return speedCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setSpeed (int value)
{
	std::swap (speedCur, value);
	if (speedCur != value) speedChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getSpeedMax() const
{
	return speedMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setSpeedMax (int value)
{
	std::swap (speedMax, value);
	if (speedMax != value) speedMaxChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getHitpoints() const
{
	return hitpointsCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setHitpoints (int value)
{
	std::swap (hitpointsCur, value);
	if (hitpointsCur != value) hitpointsChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getHitpointsMax() const
{
	return hitpointsMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setHitpointsMax (int value)
{
	std::swap (hitpointsMax, value);
	if (hitpointsMax != value) hitpointsMaxChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getScan() const
{
	return scan;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setScan (int value)
{
	std::swap (scan, value);
	if (scan != value) scanChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getRange() const
{
	return range;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setRange (int value)
{
	std::swap (range, value);
	if (range != value) rangeChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getShots() const
{
	return shotsCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setShots (int value)
{
	std::swap (shotsCur, value);
	if (shotsCur != value) shotsChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getShotsMax() const
{
	return shotsMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setShotsMax (int value)
{
	std::swap (shotsMax, value);
	if (shotsMax != value) shotsMaxChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getAmmo() const
{
	return ammoCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setAmmo (int value)
{
	std::swap (ammoCur, value);
	if (ammoCur != value) ammoChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getAmmoMax() const
{
	return ammoMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setAmmoMax (int value)
{
	std::swap (ammoMax, value);
	if (ammoMax != value) ammoMaxChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getDamage() const
{
	return damage;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setDamage (int value)
{
	std::swap (damage, value);
	if (damage != value) damageChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getArmor() const
{
	return armor;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setArmor (int value)
{
	std::swap (armor, value);
	if (armor != value) armorChanged();
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
uint32_t cDynamicUnitData::getChecksum (uint32_t crc) const
{
	if (!crcCache)
	{
		crcCache = 0;
		*crcCache = calcCheckSum (id, *crcCache);
		*crcCache = calcCheckSum (buildCosts, *crcCache);
		*crcCache = calcCheckSum (version, *crcCache);
		*crcCache = calcCheckSum (dirtyVersion, *crcCache);
		*crcCache = calcCheckSum (speedCur, *crcCache);
		*crcCache = calcCheckSum (speedMax, *crcCache);
		*crcCache = calcCheckSum (hitpointsCur, *crcCache);
		*crcCache = calcCheckSum (hitpointsMax, *crcCache);
		*crcCache = calcCheckSum (shotsCur, *crcCache);
		*crcCache = calcCheckSum (shotsMax, *crcCache);
		*crcCache = calcCheckSum (ammoCur, *crcCache);
		*crcCache = calcCheckSum (ammoMax, *crcCache);
		*crcCache = calcCheckSum (range, *crcCache);
		*crcCache = calcCheckSum (scan, *crcCache);
		*crcCache = calcCheckSum (damage, *crcCache);
		*crcCache = calcCheckSum (armor, *crcCache);
	}

	return calcCheckSum (*crcCache, crc);
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setMaximumCurrentValues()
{
	speedCur = speedMax;
	ammoCur = ammoMax;
	shotsCur = shotsMax;
	hitpointsCur = hitpointsMax;

	crcCache = std::nullopt;
}
