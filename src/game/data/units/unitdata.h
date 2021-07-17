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

#ifndef game_data_units_unitdataH
#define game_data_units_unitdataH

#include "game/data/units/id.h"
#include "game/data/resourcetype.h"
#include "game/serialization/serialization.h"

#include "utility/signal/signal.h"

#include "config/workaround/cpp17/optional.h"
#include <string>
#include <utility>
#include <vector>

class cClanData;

enum class eMuzzleType
{
	None,
	Big,
	Rocket,
	Small,
	Med,
	MedLong,
	RocketCluster,
	Torpedo,
	Sniper
};

enum class eSurfacePosition
{
	BeneathSea,
	AboveSea,
	Base,
	AboveBase,
	Ground,
	Above
};

enum class eOverbuildType
{
	No,
	Yes,
	YesNRemove
};

enum class eStorageUnitsImageType
{
	None,
	Tank,
	Plane,
	Ship,
	Human
};


struct sStaticBuildingData
{
	bool canBeLandedOn = false;
	int canMineMaxRes = 0;
	bool canResearch = false;
	bool canSelfDestroy = false;
	bool canScore = false;
	bool canWork = false;
	bool connectsToBase = false;
	int convertsGold = 0;
	bool isBig = false;
	int maxBuildFactor = 0;
	float modifiesSpeed = 0.f;

	uint32_t computeChecksum (uint32_t crc) const;

	template <typename T>
	void serialize (T& archive)
	{
		archive & NVP (canBeLandedOn);
		archive & NVP (canMineMaxRes);
		archive & NVP (canResearch);
		archive & NVP (canScore);
		archive & NVP (canSelfDestroy);
		archive & NVP (canWork);
		archive & NVP (connectsToBase);
		archive & NVP (convertsGold);
		archive & NVP (isBig);
		archive & NVP (maxBuildFactor);
		archive & NVP (modifiesSpeed);
	}

};

struct sStaticVehicleData
{
	bool canBuildPath = false;
	bool canClearArea = false;
	bool canCapture = false;
	bool canDisable = false;
	bool canDriveAndFire = false;
	bool canPlaceMines = false;
	bool canSurvey = false;
	bool isHuman = false;
	bool hasCorpse = false;
	bool makeTracks = false;
	bool animationMovement = false;

	std::string isStorageType;

	uint32_t computeChecksum (uint32_t crc) const;

	template <typename T>
	void serialize (T& archive)
	{
		archive & NVP (canBuildPath);
		archive & NVP (canClearArea);
		archive & NVP (canCapture);
		archive & NVP (canDisable);
		archive & NVP (canDriveAndFire);
		archive & NVP (canPlaceMines);
		archive & NVP (canSurvey);
		archive & NVP (isHuman);
		archive & NVP (isStorageType);
	}
};

// class for vehicle properties, that are constant and equal for all instances of a unit type
class cStaticUnitData
{
public:
	cStaticUnitData() = default;
	const std::string& getDefaultName() const;
	const std::string& getDefaultDescription() const;
	void setDefaultName (std::string name_){ name = name_; }
	void setDefaultDescription (std::string text) { description = text; }

	uint32_t getChecksum (uint32_t crc) const;
public:
	// Main
	sID ID;

	// Attack
	eMuzzleType muzzleType = eMuzzleType::None;

	char canAttack = 0;

	std::string canBuild;
	std::string buildAs;

	float factorGround = 0.f;
	float factorSea = 0.f;
	float factorAir = 0.f;
	float factorCoast = 0.f;

	// Abilities
	bool canBeCaptured = false;
	bool canBeDisabled = false;
	bool canRearm = false;
	bool canRepair = false;
	bool doesSelfRepair = false;
	bool isAlien = false;

	int needsMetal = 0;
	int needsOil = 0;
	int needsEnergy = 0;
	int needsHumans = 0;
	int produceEnergy = 0;
	int produceHumans = 0;

	char isStealthOn = 0;
	char canDetectStealthOn = 0;

	eSurfacePosition surfacePosition = eSurfacePosition::BeneathSea;

	eOverbuildType canBeOverbuild = eOverbuildType::No;

	bool explodesOnContact = false;

	// Storage
	int storageResMax = 0;
	eResourceType storeResType = eResourceType::None;

	std::size_t storageUnitsMax = 0;

	eStorageUnitsImageType storeUnitsImageType = eStorageUnitsImageType::None;
	std::vector<std::string> storeUnitsTypes;

	sStaticVehicleData vehicleData;
	sStaticBuildingData buildingData;

	template <typename T>
	void serialize (T& archive)
	{
		archive & NVP (ID);
		archive & NVP (muzzleType);
		archive & NVP (canAttack);
		archive & NVP (canBuild);
		archive & NVP (canRearm);
		archive & NVP (canRepair);
		archive & NVP (buildAs);
		archive & NVP (factorGround);
		archive & NVP (factorSea);
		archive & NVP (factorAir);
		archive & NVP (factorCoast);
		archive & NVP (canBeCaptured);
		archive & NVP (canBeDisabled);
		archive & NVP (doesSelfRepair);
		archive & NVP (isAlien);
		archive & NVP (needsMetal);
		archive & NVP (needsOil);
		archive & NVP (needsEnergy);
		archive & NVP (needsHumans);
		archive & NVP (produceEnergy);
		archive & NVP (produceHumans);
		archive & NVP (isStealthOn);
		archive & NVP (canDetectStealthOn);
		archive & NVP (surfacePosition);
		archive & NVP (canBeOverbuild);
		archive & NVP (explodesOnContact);
		archive & NVP (storageResMax);
		archive & NVP (storeResType);
		archive & NVP (storageUnitsMax);
		archive & NVP (storeUnitsImageType);
		archive & NVP (storeUnitsTypes);
		archive & NVP (description);
		archive & NVP (name);

		buildingData.serialize (archive);
		vehicleData.serialize (archive);
	}

private:
	std::string description; //untranslated data from unit xml. Will be used, when translation for the unit is not available
	std::string name;        //untranslated data from unit xml. Will be used, when translation for the unit is not available
};

//class for vehicle properties, that are individual for each instance of a unit
class cDynamicUnitData
{
public:
	cDynamicUnitData() = default;
	cDynamicUnitData (const cDynamicUnitData&);
	cDynamicUnitData& operator= (const cDynamicUnitData&);

	void setMaximumCurrentValues();

	sID getId() const;
	void setId (const sID& value);

	int getBuildCost() const;
	void setBuildCost (int value);

	int getVersion() const;
	void setVersion (int value);

	int getSpeed() const;
	void setSpeed (int value);

	int getSpeedMax() const;
	void setSpeedMax (int value);

	int getHitpoints() const;
	void setHitpoints (int value);

	int getHitpointsMax() const;
	void setHitpointsMax (int value);

	int getScan() const;
	void setScan (int value);

	int getRange() const;
	void setRange (int value);

	int getShots() const;
	void setShots (int value);

	int getShotsMax() const;
	void setShotsMax (int value);

	int getAmmo() const;
	void setAmmo (int value);

	int getAmmoMax() const;
	void setAmmoMax (int value);

	int getDamage() const;
	void setDamage (int value);

	int getArmor() const;
	void setArmor (int value);

	uint32_t getChecksum (uint32_t crc) const;

	mutable cSignal<void()> buildCostsChanged;
	mutable cSignal<void()> versionChanged;
	mutable cSignal<void()> speedChanged;
	mutable cSignal<void()> speedMaxChanged;
	mutable cSignal<void()> hitpointsChanged;
	mutable cSignal<void()> hitpointsMaxChanged;
	mutable cSignal<void()> shotsChanged;
	mutable cSignal<void()> shotsMaxChanged;
	mutable cSignal<void()> ammoChanged;
	mutable cSignal<void()> ammoMaxChanged;
	mutable cSignal<void()> scanChanged;
	mutable cSignal<void()> rangeChanged;
	mutable cSignal<void()> damageChanged;
	mutable cSignal<void()> armorChanged;

	template <typename T>
	void serialize (T& archive)
	{
		archive & NVP (id);
		archive & NVP (buildCosts);
		archive & NVP (version);
		archive & NVP (speedCur);
		archive & NVP (speedMax);
		archive & NVP (hitpointsCur);
		archive & NVP (hitpointsMax);
		archive & NVP (shotsCur);
		archive & NVP (shotsMax);
		archive & NVP (ammoCur);
		archive & NVP (ammoMax);
		archive & NVP (range);
		archive & NVP (scan);
		archive & NVP (damage);
		archive & NVP (armor);

		if (!T::isWritter)
			crcCache = std::nullopt;
	}
private:
	// Main
	sID id;

	// Production
	int buildCosts = 0;

	int version = 0;

	int speedCur = 0;
	int speedMax = 0;

	int hitpointsCur = 0;
	int hitpointsMax = 0;
	int shotsCur = 0;
	int shotsMax = 0;
	int ammoCur = 0;
	int ammoMax = 0;

	int range = 0;
	int scan = 0;

	int damage = 0;
	int armor = 0;

	mutable std::optional<uint32_t> crcCache;
};

class cUnitsData
{
public:
	cUnitsData();

	void initializeIDData();
	void initializeClanUnitData (const cClanData& clanData);

	void addData (const cDynamicUnitData& data) { crcCache = std::nullopt; dynamicUnitData.push_back (data); }
	void addData (const cStaticUnitData& data)  { crcCache = std::nullopt; staticUnitData.push_back (data); }

	bool isValidId (const sID& id) const;
	size_t getNrOfClans() const;

	// clan = -1: without clans
	const cDynamicUnitData& getDynamicUnitData (const sID& id, int clan = -1) const;
	const cStaticUnitData& getStaticUnitData (const sID& id) const;

	// clan = -1: without clans
	const std::vector<cDynamicUnitData>& getDynamicUnitsData (int clan = -1) const;
	const std::vector<cStaticUnitData>& getStaticUnitsData() const;

	uint32_t getChecksum (uint32_t crc) const;

	const cStaticUnitData& getConstructorData() const { return getStaticUnitData (constructorID); }
	const cStaticUnitData& getEngineerData() const { return getStaticUnitData (engineerID); }
	const cStaticUnitData& getSurveyorData() const { return getStaticUnitData (surveyorID); }
	const cStaticUnitData& getMineData() const { return getStaticUnitData (specialIDMine); }
	const cStaticUnitData& getSmallGeneratorData() const { return getStaticUnitData (specialIDSmallGen); }
	const cStaticUnitData& getLandMineData() const { return getStaticUnitData (specialIDLandMine); }
	const cStaticUnitData& getSeaMineData() const { return getStaticUnitData (specialIDSeaMine); }
	const cStaticUnitData& getRubbleSmallData() const { return rubbleSmall; }
	const cStaticUnitData& getRubbleBigData() const { return rubbleBig; }

	sID getConstructorID() const { return constructorID; }
	sID getEngineerID() const { return engineerID; }
	sID getSurveyorID() const { return surveyorID; }
	sID getSpecialIDLandMine() const { return specialIDLandMine; }
	sID getSpecialIDSeaMine() const { return specialIDSeaMine; }
	sID getSpecialIDMine() const { return specialIDMine; }
	sID getSpecialIDSmallGen() const { return specialIDSmallGen; }
	sID getSpecialIDConnector() const { return specialIDConnector; }
	sID getSpecialIDSmallBeton() const { return specialIDSmallBeton; }

	void setSpecialIDLandMine (sID id) { specialIDLandMine = id; crcCache = std::nullopt; }
	void setSpecialIDSeaMine (sID id)  { specialIDSeaMine = id; crcCache = std::nullopt; }
	void setSpecialIDMine (sID id) { specialIDMine = id; crcCache = std::nullopt; }
	void setSpecialIDSmallGen (sID id) { specialIDSmallGen = id; crcCache = std::nullopt; }
	void setSpecialIDConnector (sID id) { specialIDConnector = id; crcCache = std::nullopt; }
	void setSpecialIDSmallBeton (sID id) { specialIDSmallBeton = id; crcCache = std::nullopt; }

	template <typename T>
	void serialize (T& archive)
	{
		if (!T::isWritter)
		{
			staticUnitData.clear();
			dynamicUnitData.clear();
			clanDynamicUnitData.clear();
			crcCache = std::nullopt;
		}

		archive & NVP (constructorID);
		archive & NVP (engineerID);
		archive & NVP (surveyorID);
		archive & NVP (specialIDLandMine);
		archive & NVP (specialIDSeaMine);
		archive & NVP (specialIDMine);
		archive & NVP (specialIDSmallGen);
		archive & NVP (specialIDConnector);
		archive & NVP (specialIDSmallBeton);
		archive & NVP (staticUnitData);
		archive & NVP (dynamicUnitData);
		archive & NVP (clanDynamicUnitData);
	}

private:
	int getUnitIndexBy (sID id) const;

	sID constructorID;
	sID engineerID;
	sID surveyorID;
	sID specialIDLandMine;
	sID specialIDSeaMine;
	sID specialIDMine;
	sID specialIDSmallGen;
	sID specialIDConnector;
	sID specialIDSmallBeton;

	// the static unit data
	std::vector<cStaticUnitData> staticUnitData;

	// the dynamic unit data. Standard version without clan modifications
	std::vector<cDynamicUnitData> dynamicUnitData;

	// the dynamic unit data. Contains the modified versions for the clans
	std::vector<std::vector<cDynamicUnitData> > clanDynamicUnitData;

	cStaticUnitData rubbleSmall;
	cStaticUnitData rubbleBig;

	// unitdata does not change during the game.
	// So caching the checksum saves a lot cpu resources.
	mutable std::optional<uint32_t> crcCache;
};

extern cUnitsData UnitsDataGlobal;

#endif // game_data_units_unitdataH
