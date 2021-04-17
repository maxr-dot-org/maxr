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
#include "utility/log.h"
#include "utility/language.h"
#include "utility/string/toString.h"
#include "utility/crc.h"

#include <3rd/tinyxml2/tinyxml2.h>

#include <algorithm>

//------------------------------------------------------------------------------
// ----------- sID Implementation ----------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::string sID::getText() const
{
	char tmp[6];
	TIXML_SNPRINTF(tmp, sizeof(tmp), "%.2d %.2d", firstPart, secondPart);
	return tmp;
}

//------------------------------------------------------------------------------
void sID::generate(const std::string& text)
{
	const std::string::size_type spacePos = text.find(" ", 0);
	firstPart = atoi(text.substr(0, spacePos).c_str());
	secondPart = atoi(text.substr(spacePos, text.length()).c_str());
}

//------------------------------------------------------------------------------
bool sID::less_buildingFirst(const sID& ID) const
{
	return firstPart == ID.firstPart ? secondPart < ID.secondPart : firstPart > ID.firstPart;
}

//------------------------------------------------------------------------------
uint32_t sID::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(firstPart, crc);
	crc = calcCheckSum(secondPart, crc);

	return crc;
}

//------------------------------------------------------------------------------
bool sID::less_vehicleFirst(const sID& ID) const
{
	return firstPart == ID.firstPart ? secondPart < ID.secondPart : firstPart < ID.firstPart;
}

//------------------------------------------------------------------------------
bool sID::operator == (const sID& ID) const
{
	if (firstPart == ID.firstPart && secondPart == ID.secondPart) return true;
	return false;
}

//------------------------------------------------------------------------------
cUnitsData::cUnitsData() :
	crcCache(0),
	crcValid(false)
{
	rubbleBig.isBig = true;
	rubbleSmall.isBig = false;
}

//------------------------------------------------------------------------------
int cUnitsData::getUnitIndexBy(sID id) const
{
	for (unsigned int i = 0; i != staticUnitData.size(); ++i)
	{
		if (staticUnitData[i].ID == id) return i;
	}
	Log.write("Unitdata with id (" + iToStr(id.firstPart) + ", " + iToStr(id.secondPart) + ") not found", cLog::eLOG_TYPE_ERROR);
	return -1;
}

//------------------------------------------------------------------------------
void cUnitsData::initializeIDData()
{
	for (const auto& data : staticUnitData)
	{
		if (data.canBuild == "BigBuilding")
			constructorID = data.ID;
		if (data.canBuild == "SmallBuilding")
			engineerID = data.ID;
		if (data.canSurvey)
			surveyorID = data.ID;
	}
	if (constructorID == sID(0, 0)) Log.write("Constructor index not found. Constructor needs to have the property \"Can_Build = BigBuilding\"", cLog::eLOG_TYPE_ERROR);
	if (engineerID    == sID(0, 0)) Log.write("Engineer index not found. Engineer needs to have the property \"Can_Build = SmallBuilding\"", cLog::eLOG_TYPE_ERROR);
	if (surveyorID    == sID(0, 0)) Log.write("Surveyor index not found. Surveyor needs to have the property \"Can_Survey = Yes\"", cLog::eLOG_TYPE_ERROR);

	crcValid = false;
}

//------------------------------------------------------------------------------
void cUnitsData::initializeClanUnitData(const cClanData& clanData)
{
	crcValid = false;

	clanDynamicUnitData.resize(clanData.getNrClans());

	for (int i = 0; i != clanData.getNrClans(); ++i)
	{
		const cClan* clan = clanData.getClan(i);
		if (clan == nullptr)
			continue;

		std::vector<cDynamicUnitData>& clanListVehicles = clanDynamicUnitData[i];

		// make a copy of the vehicle's stats
		clanListVehicles = dynamicUnitData;
		for (size_t j = 0; j != dynamicUnitData.size(); ++j)
		{
			cDynamicUnitData& clanVehicle = clanListVehicles[j];
			const cClanUnitStat* changedStat = clan->getUnitStat(clanVehicle.getId());
			if (changedStat == nullptr) continue;

			if (changedStat->hasModification("Damage"))
				clanVehicle.setDamage(changedStat->getModificationValue("Damage"));
			if (changedStat->hasModification("Range"))
				clanVehicle.setRange(changedStat->getModificationValue("Range"));
			if (changedStat->hasModification("Armor"))
				clanVehicle.setArmor(changedStat->getModificationValue("Armor"));
			if (changedStat->hasModification("Hitpoints"))
				clanVehicle.setHitpointsMax(changedStat->getModificationValue("Hitpoints"));
			if (changedStat->hasModification("Scan"))
				clanVehicle.setScan(changedStat->getModificationValue("Scan"));
			if (changedStat->hasModification("Speed"))
				clanVehicle.setSpeedMax(changedStat->getModificationValue("Speed") * 4);
			if (changedStat->hasModification("Built_Costs"))
				clanVehicle.setBuildCost(changedStat->getModificationValue("Built_Costs"));
		}
	}
}

//------------------------------------------------------------------------------
bool cUnitsData::isValidId(const sID& id) const
{
	return getUnitIndexBy(id) != -1;
}

//------------------------------------------------------------------------------
size_t cUnitsData::getNrOfClans() const
{
	return clanDynamicUnitData.size();
}

//------------------------------------------------------------------------------
const cDynamicUnitData& cUnitsData::getDynamicUnitData(const sID& id, int clan /*= -1*/) const
{
	if (clan < 0 || static_cast<unsigned>(clan) >= clanDynamicUnitData.size())
	{
		for (const auto& data : dynamicUnitData)
		{
			if (data.getId() == id) return data;
		}
		throw std::runtime_error("Unitdata not found" + id.getText());
	}
	else
	{
		for (const auto& data : clanDynamicUnitData[clan])
		{
			if (data.getId() == id) return data;
		}
		throw std::runtime_error("Unitdata not found" + id.getText());
	}
}

//------------------------------------------------------------------------------
const cStaticUnitData& cUnitsData::getStaticUnitData(const sID& id) const
{
	for (const auto& data : staticUnitData)
	{
		if (data.ID == id) return data;
	}
	throw std::runtime_error("Unitdata not found" + id.getText());
}

//------------------------------------------------------------------------------
const std::vector<cDynamicUnitData>& cUnitsData::getDynamicUnitsData(int clan /*= -1*/) const
{
	if (clan < 0 || static_cast<unsigned>(clan) >= clanDynamicUnitData.size())
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

uint32_t cUnitsData::getChecksum(uint32_t crc) const
{
	if (!crcValid)
	{
		crcCache = 0;
		crcCache = calcCheckSum(constructorID, crcCache);
		crcCache = calcCheckSum(engineerID, crcCache);
		crcCache = calcCheckSum(surveyorID, crcCache);
		crcCache = calcCheckSum(specialIDLandMine, crcCache);
		crcCache = calcCheckSum(specialIDSeaMine, crcCache);
		crcCache = calcCheckSum(specialIDMine, crcCache);
		crcCache = calcCheckSum(specialIDSmallGen, crcCache);
		crcCache = calcCheckSum(specialIDConnector, crcCache);
		crcCache = calcCheckSum(specialIDSmallBeton, crcCache);
		crcCache = calcCheckSum(staticUnitData, crcCache);
		crcCache = calcCheckSum(dynamicUnitData, crcCache);
		crcCache = calcCheckSum(clanDynamicUnitData, crcCache);

		crcValid = true;
	}

	return calcCheckSum(crcCache, crc);
}

//------------------------------------------------------------------------------
std::string cStaticUnitData::getName() const
{
	std::string translatedName = lngPack.getUnitName(ID);
	if (!translatedName.empty())
		return translatedName;

	return name;
}

//------------------------------------------------------------------------------
std::string cStaticUnitData::getDescripton() const
{
	std::string translatedDescription = lngPack.getUnitDescription(ID);
	if (!translatedDescription.empty())
		return translatedDescription;

	return description;
}

//------------------------------------------------------------------------------
uint32_t cStaticUnitData::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(ID, crc);
	crc = calcCheckSum(muzzleType, crc);
	crc = calcCheckSum(canAttack, crc);
	crc = calcCheckSum(canDriveAndFire, crc);
	crc = calcCheckSum(canBuild, crc);
	crc = calcCheckSum(buildAs, crc);
	crc = calcCheckSum(maxBuildFactor, crc);
	crc = calcCheckSum(canBuildPath, crc);
	crc = calcCheckSum(canBuildRepeat, crc);
	crc = calcCheckSum(factorGround, crc);
	crc = calcCheckSum(factorSea, crc);
	crc = calcCheckSum(factorAir, crc);
	crc = calcCheckSum(factorCoast, crc);
	crc = calcCheckSum(connectsToBase, crc);
	crc = calcCheckSum(modifiesSpeed, crc);
	crc = calcCheckSum(canClearArea, crc);
	crc = calcCheckSum(canBeCaptured, crc);
	crc = calcCheckSum(canBeDisabled, crc);
	crc = calcCheckSum(canCapture, crc);
	crc = calcCheckSum(canDisable, crc);
	crc = calcCheckSum(canRepair, crc);
	crc = calcCheckSum(canRearm, crc);
	crc = calcCheckSum(canResearch, crc);
	crc = calcCheckSum(canPlaceMines, crc);
	crc = calcCheckSum(canSurvey, crc);
	crc = calcCheckSum(doesSelfRepair, crc);
	crc = calcCheckSum(convertsGold, crc);
	crc = calcCheckSum(canSelfDestroy, crc);
	crc = calcCheckSum(canScore, crc);
	crc = calcCheckSum(canMineMaxRes, crc);
	crc = calcCheckSum(needsMetal, crc);
	crc = calcCheckSum(needsOil, crc);
	crc = calcCheckSum(needsEnergy, crc);
	crc = calcCheckSum(needsHumans, crc);
	crc = calcCheckSum(produceEnergy, crc);
	crc = calcCheckSum(produceHumans, crc);
	crc = calcCheckSum(isStealthOn, crc);
	crc = calcCheckSum(canDetectStealthOn, crc);
	crc = calcCheckSum(surfacePosition, crc);
	crc = calcCheckSum(canBeOverbuild, crc);
	crc = calcCheckSum(canBeLandedOn, crc);
	crc = calcCheckSum(canWork, crc);
	crc = calcCheckSum(explodesOnContact, crc);
	crc = calcCheckSum(isHuman, crc);
	crc = calcCheckSum(isBig, crc);
	crc = calcCheckSum(storageResMax, crc);
	crc = calcCheckSum(storeResType, crc);
	crc = calcCheckSum(storageUnitsMax, crc);
	crc = calcCheckSum(storeUnitsImageType, crc);
	crc = calcCheckSum(storeUnitsTypes, crc);
	crc = calcCheckSum(isStorageType, crc);
	crc = calcCheckSum(description, crc);
	crc = calcCheckSum(name, crc);

	return crc;
}

//------------------------------------------------------------------------------
cDynamicUnitData::cDynamicUnitData() :
	id(),
	buildCosts(0),
	version(0),
	speedCur(0),
	speedMax(0),
	hitpointsCur(0),
	hitpointsMax(0),
	shotsCur(0),
	shotsMax(0),
	ammoCur(0),
	ammoMax(0),
	range(0),
	scan(0),
	damage(0),
	armor(0),
	crcCache(0),
	crcValid(false)
{}

//------------------------------------------------------------------------------
cDynamicUnitData::cDynamicUnitData(const cDynamicUnitData& other) :
	id(other.id),
	buildCosts(other.buildCosts),
	version(other.version),
	speedCur(other.speedCur),
	speedMax(other.speedMax),
	hitpointsCur(other.hitpointsCur),
	hitpointsMax(other.hitpointsMax),
	shotsCur(other.shotsCur),
	shotsMax(other.shotsMax),
	ammoCur(other.ammoCur),
	ammoMax(other.ammoMax),
	range(other.range),
	scan(other.scan),
	damage(other.damage),
	armor(other.armor),
	crcCache(0),
	crcValid(false)
{}

//------------------------------------------------------------------------------
cDynamicUnitData& cDynamicUnitData::operator=(const cDynamicUnitData& other)
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
	crcCache = 0;
	crcValid = false;

	return *this;
}

//------------------------------------------------------------------------------
sID cDynamicUnitData::getId() const
{
	return id;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setId(const sID& value)
{
	id = value;
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getBuildCost() const
{
	return buildCosts;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setBuildCost(int value)
{
	std::swap(buildCosts, value);
	if (buildCosts != value) buildCostsChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getVersion() const
{
	return version;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setVersion(int value)
{
	std::swap(version, value);
	if (version != value) versionChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getSpeed() const
{
	return speedCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setSpeed(int value)
{
	std::swap(speedCur, value);
	if (speedCur != value) speedChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getSpeedMax() const
{
	return speedMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setSpeedMax(int value)
{
	std::swap(speedMax, value);
	if (speedMax != value) speedMaxChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getHitpoints() const
{
	return hitpointsCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setHitpoints(int value)
{
	std::swap(hitpointsCur, value);
	if (hitpointsCur != value) hitpointsChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getHitpointsMax() const
{
	return hitpointsMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setHitpointsMax(int value)
{
	std::swap(hitpointsMax, value);
	if (hitpointsMax != value) hitpointsMaxChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getScan() const
{
	return scan;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setScan(int value)
{
	std::swap(scan, value);
	if (scan != value) scanChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getRange() const
{
	return range;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setRange(int value)
{
	std::swap(range, value);
	if (range != value) rangeChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getShots() const
{
	return shotsCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setShots(int value)
{
	std::swap(shotsCur, value);
	if (shotsCur != value) shotsChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getShotsMax() const
{
	return shotsMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setShotsMax(int value)
{
	std::swap(shotsMax, value);
	if (shotsMax != value) shotsMaxChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getAmmo() const
{
	return ammoCur;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setAmmo(int value)
{
	std::swap(ammoCur, value);
	if (ammoCur != value) ammoChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getAmmoMax() const
{
	return ammoMax;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setAmmoMax(int value)
{
	std::swap(ammoMax, value);
	if (ammoMax != value) ammoMaxChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getDamage() const
{
	return damage;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setDamage(int value)
{
	std::swap(damage, value);
	if (damage != value) damageChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
int cDynamicUnitData::getArmor() const
{
	return armor;
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setArmor(int value)
{
	std::swap(armor, value);
	if (armor != value) armorChanged();
	crcValid = false;
}

//------------------------------------------------------------------------------
uint32_t cDynamicUnitData::getChecksum(uint32_t crc) const
{
	if (!crcValid)
	{
		crcCache = 0;
		crcCache = calcCheckSum(id, crcCache);
		crcCache = calcCheckSum(buildCosts, crcCache);
		crcCache = calcCheckSum(version, crcCache);
		crcCache = calcCheckSum(speedCur, crcCache);
		crcCache = calcCheckSum(speedMax, crcCache);
		crcCache = calcCheckSum(hitpointsCur, crcCache);
		crcCache = calcCheckSum(hitpointsMax, crcCache);
		crcCache = calcCheckSum(shotsCur, crcCache);
		crcCache = calcCheckSum(shotsMax, crcCache);
		crcCache = calcCheckSum(ammoCur, crcCache);
		crcCache = calcCheckSum(ammoMax, crcCache);
		crcCache = calcCheckSum(range, crcCache);
		crcCache = calcCheckSum(scan, crcCache);
		crcCache = calcCheckSum(damage, crcCache);
		crcCache = calcCheckSum(armor, crcCache);

		crcValid = true;
	}

	return calcCheckSum(crcCache, crc);
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setMaximumCurrentValues()
{
	speedCur     = speedMax;
	ammoCur      = ammoMax;
	shotsCur     = shotsMax;
	hitpointsCur = hitpointsMax;

	crcValid = false;
}
