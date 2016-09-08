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

#include <algorithm>

#include "unitdata.h"
#include "tinyxml2.h"
#include "game/data/player/clans.h"
#include "utility/log.h"
#include "utility/language.h"


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

}

//------------------------------------------------------------------------------
void cUnitsData::initializeClanUnitData()
{
	cClanData& clanData = cClanData::instance();
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
	if (clan < 0 || clan >= clanDynamicUnitData.size())
	{
		for (const auto& data : dynamicUnitData)
		{
			if (data.getId() == id) return data;
		}
		assert(false);
	}
	else
	{
		for (const auto& data : clanDynamicUnitData[clan])
		{
			if (data.getId() == id) return data;
		}
		assert(false);
	}
}

//------------------------------------------------------------------------------
const cStaticUnitData& cUnitsData::getStaticUnitData(const sID& id) const
{
	for (const auto& data : staticUnitData)
	{
		if (data.ID == id) return data;
	}
	assert(false);
}

//------------------------------------------------------------------------------
const std::vector<cDynamicUnitData>& cUnitsData::getDynamicUnitsData(int clan /*= -1*/) const
{
	if (clan < 0 || clan >= clanDynamicUnitData.size())
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
cStaticUnitData::cStaticUnitData()
{
	muzzleType = MUZZLE_TYPE_NONE;

	canAttack = 0;
	canDriveAndFire = false;

	maxBuildFactor = 0;

	canBuildPath = false;
	canBuildRepeat = false;

	// Movement
	factorGround = 0.0f;
	factorSea = 0.0f;
	factorAir = 0.0f;
	factorCoast = 0.0f;

	// Abilities
	connectsToBase = false;
	modifiesSpeed = 0.0f;
	canClearArea = false;
	canBeCaptured = false;
	canBeDisabled = false;
	canCapture = false;
	canDisable = false;
	canRepair = false;
	canRearm = false;
	canResearch = false;
	canPlaceMines = false;
	canSurvey = false;
	doesSelfRepair = false;
	convertsGold = 0;
	canSelfDestroy = false;
	canScore = false;

	canMineMaxRes = 0;
	needsMetal = 0;
	needsOil = 0;
	needsEnergy = 0;
	needsHumans = 0;
	produceEnergy = 0;
	produceHumans = 0;

	isStealthOn = 0;
	canDetectStealthOn = 0;

	surfacePosition = SURFACE_POS_BENEATH_SEA;
	canBeOverbuild = OVERBUILD_TYPE_NO;

	canBeLandedOn = false;
	canWork = false;
	explodesOnContact = false;
	isHuman = false;
	isBig = false;

	// Storage
	storageResMax = 0;
	storeResType = STORE_RES_NONE;
	storageUnitsMax = 0;
	storeUnitsImageType = STORE_UNIT_IMG_NONE;
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
	armor(0)
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
	armor(other.armor)
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
}

//------------------------------------------------------------------------------
void cDynamicUnitData::setMaximumCurrentValues()
{
	speedCur     = speedMax;
	ammoCur      = ammoMax;
	shotsCur     = shotsMax;
	hitpointsCur = hitpointsMax;
}
