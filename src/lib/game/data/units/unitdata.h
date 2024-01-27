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

#include "game/data/resourcetype.h"
#include "game/data/units/id.h"
#include "utility/serialization/serialization.h"
#include "utility/signal/signal.h"

#include <optional>
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
namespace serialization
{
	template <>
	struct sEnumStringMapping<eMuzzleType>
	{
		static const std::vector<std::pair<eMuzzleType, const char*>> m;
	};
	template <>
	struct sEnumStringMapping<eSurfacePosition>
	{
		static const std::vector<std::pair<eSurfacePosition, const char*>> m;
	};
	template <>
	struct sEnumStringMapping<eOverbuildType>
	{
		static const std::vector<std::pair<eOverbuildType, const char*>> m;
	};
	template <>
	struct sEnumStringMapping<eStorageUnitsImageType>
	{
		static const std::vector<std::pair<eStorageUnitsImageType, const char*>> m;
	};
} // namespace serialization
struct sStaticCommonUnitData
{
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
	int produceEnergy = 0; // Only one of need/produce Energy can be non-zero
	int produceHumans = 0; // Only one of need/produce Human can be non-zero

	char isStealthOn = 0;
	char canDetectStealthOn = 0;

	eSurfacePosition surfacePosition = eSurfacePosition::BeneathSea;

	// Storage
	int storageResMax = 0;
	eResourceType storeResType = eResourceType::None;

	std::size_t storageUnitsMax = 0;

	eStorageUnitsImageType storageUnitsImageType = eStorageUnitsImageType::None;
	std::vector<std::string> storeUnitsTypes;

	[[nodiscard]] uint32_t computeChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
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
		if (Archive::isWriter)
		{
			archive & serialization::makeNvp ("needsEnergy", needsEnergy > 0 ? needsEnergy : -produceEnergy);
			archive & serialization::makeNvp ("needsHumans", needsHumans > 0 ? needsHumans : -produceHumans);
		}
		else
		{
			archive & NVP (needsEnergy);
			archive & NVP (needsHumans);
			produceEnergy = std::max (0, -needsEnergy);
			produceHumans = std::max (0, -needsHumans);
			needsEnergy = std::max (0, needsEnergy);
			needsHumans = std::max (0, needsHumans);
		}
		archive & NVP (isStealthOn);
		archive & NVP (canDetectStealthOn);
		archive & NVP (surfacePosition);
		archive & NVP (storageResMax);
		archive & NVP (storeResType);
		archive & NVP (storageUnitsMax);
		archive & NVP (storageUnitsImageType);
		archive & NVP (storeUnitsTypes);
		// clang-format on
	}
};

struct sStaticBuildingData
{
	bool canBeLandedOn = false;
	int canMineMaxRes = 0;
	eOverbuildType canBeOverbuild = eOverbuildType::No;
	bool canResearch = false;
	bool canSelfDestroy = false;
	bool canScore = false;
	bool canWork = false;
	bool connectsToBase = false;
	int convertsGold = 0;
	bool explodesOnContact = false;
	bool isBig = false;
	int maxBuildFactor = 0;
	float modifiesSpeed = 0.f;

	[[nodiscard]] uint32_t computeChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (canBeLandedOn);
		archive & NVP (canMineMaxRes);
		archive & NVP (canBeOverbuild);
		archive & NVP (canResearch);
		archive & NVP (canScore);
		archive & NVP (canSelfDestroy);
		archive & NVP (canWork);
		archive & NVP (connectsToBase);
		archive & NVP (convertsGold);
		archive & NVP (explodesOnContact);
		archive & NVP (isBig);
		archive & NVP (maxBuildFactor);
		archive & NVP (modifiesSpeed);
		// clang-format on
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
	bool hasCorpse = false;
	bool isHuman = false;
	bool makeTracks = false;
	bool animationMovement = false;

	std::string isStorageType;

	[[nodiscard]] uint32_t computeChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (animationMovement);
		archive & NVP (canBuildPath);
		archive & NVP (canClearArea);
		archive & NVP (canCapture);
		archive & NVP (canDisable);
		archive & NVP (canDriveAndFire);
		archive & NVP (canPlaceMines);
		archive & NVP (canSurvey);
		archive & NVP (hasCorpse);
		archive & NVP (isHuman);
		archive & NVP (isStorageType);
		archive & NVP (makeTracks);
		// clang-format on
	}
};

// class for vehicle properties, that are constant and equal for all instances of a unit type
class cStaticUnitData : public sStaticCommonUnitData
{
public:
	cStaticUnitData() = default;
	const std::string& getDefaultName() const;
	const std::string& getDefaultDescription() const;
	void setDefaultName (std::string name_) { name = name_; }
	void setDefaultDescription (std::string text) { description = text; }

	uint32_t getChecksum (uint32_t crc) const;

public:
	// Main
	sID ID;

	sStaticVehicleData vehicleData;
	sStaticBuildingData buildingData;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (ID);
		archive & NVP (description);
		archive & NVP (name);
		// clang-format on
		sStaticCommonUnitData::serialize (archive);
		if (ID.isABuilding())
			buildingData.serialize (archive);
		else
			vehicleData.serialize (archive);
	}

private:
	std::string description; //untranslated data from unit json. Will be used, when translation for the unit is not available
	std::string name; //untranslated data from unit json. Will be used, when translation for the unit is not available
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

	bool canBeUpgradedTo (const cDynamicUnitData&) const;
	void makeVersionDirty();
	void markLastVersionUsed();

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

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (id);
		archive & NVP (buildCosts);
		archive & NVP (version);
		archive & NVP (dirtyVersion);
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
		// clang-format on

		if (!Archive::isWriter)
			crcCache = std::nullopt;
	}

private:
	// Main
	sID id;

	// Production
	int buildCosts = 0;

	int version = 0;
	bool dirtyVersion = false; // version should not be increased when stats change
	                           // but when unit uses (via upgrape/build) the last version .

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

struct sSpecialBuildingsId
{
	void logMissing() const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (alienFactory);
		archive & NVP (connector);
		archive & NVP (landMine);
		archive & NVP (mine);
		archive & NVP (seaMine);
		archive & NVP (smallBeton);
		archive & NVP (smallGenerator);
		// clang-format on
	}

	[[nodiscard]] uint32_t computeChecksum (uint32_t crc) const;

	int alienFactory = 0;
	int connector = 0;
	int landMine = 0;
	int mine = 0;
	int seaMine = 0;
	int smallBeton = 0;
	int smallGenerator = 0;
};

struct sSpecialVehiclesId
{
	void logMissing() const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (constructor);
		archive & NVP (engineer);
		archive & NVP (surveyor);
		// clang-format on
	}

	[[nodiscard]] uint32_t computeChecksum (uint32_t crc) const;

	int constructor = 0;
	int engineer = 0;
	int surveyor = 0;
};

class cUnitsData
{
public:
	cUnitsData();

	void initializeIDData();
	void initializeClanUnitData (const cClanData& clanData);

	void addData (const cDynamicUnitData& data)
	{
		crcCache = std::nullopt;
		dynamicUnitData.push_back (data);
	}
	void addData (const cStaticUnitData& data)
	{
		crcCache = std::nullopt;
		staticUnitData.push_back (data);
	}

	bool isValidId (const sID& id) const;
	size_t getNrOfClans() const;

	// clan = -1: without clans
	const cDynamicUnitData& getDynamicUnitData (const sID& id, int clan = -1) const;
	const cStaticUnitData& getStaticUnitData (const sID& id) const;

	// clan = -1: without clans
	const std::vector<cDynamicUnitData>& getDynamicUnitsData (int clan = -1) const;
	const std::vector<cStaticUnitData>& getStaticUnitsData() const;

	uint32_t getChecksum (uint32_t crc) const;

	const cStaticUnitData& getRubbleSmallData() const { return rubbleSmall; }
	const cStaticUnitData& getRubbleBigData() const { return rubbleBig; }

	sID getConstructorID() const { return sID (0, specialVehicles.constructor); }
	sID getEngineerID() const { return sID (0, specialVehicles.engineer); }
	sID getSurveyorID() const { return sID (0, specialVehicles.surveyor); }

	sID getAlienFactoryID() const { return sID (1, specialBuildings.alienFactory); }
	sID getConnectorID() const { return sID (1, specialBuildings.connector); }
	sID getLandMineID() const { return sID (1, specialBuildings.landMine); }
	sID getMineID() const { return sID (1, specialBuildings.mine); }
	sID getSeaMineID() const { return sID (1, specialBuildings.seaMine); }
	sID getSmallBetonID() const { return sID (1, specialBuildings.smallBeton); }
	sID getSmallGeneratorID() const { return sID (1, specialBuildings.smallGenerator); }

	void setSpecialBuildingIDs (sSpecialBuildingsId ids)
	{
		specialBuildings = ids;
		crcCache = std::nullopt;
	}

	template <typename Archive>
	void serialize (Archive& archive)
	{
		if (!Archive::isWriter)
		{
			staticUnitData.clear();
			dynamicUnitData.clear();
			clanDynamicUnitData.clear();
			crcCache = std::nullopt;
		}

		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (specialBuildings);
		archive & NVP (specialVehicles);
		archive & NVP (staticUnitData);
		archive & NVP (dynamicUnitData);
		archive & NVP (clanDynamicUnitData);
		// clang-format on
	}

private:
	sSpecialBuildingsId specialBuildings;
	sSpecialVehiclesId specialVehicles;

	// the static unit data
	std::vector<cStaticUnitData> staticUnitData;

	// the dynamic unit data. Standard version without clan modifications
	std::vector<cDynamicUnitData> dynamicUnitData;

	// the dynamic unit data. Contains the modified versions for the clans
	std::vector<std::vector<cDynamicUnitData>> clanDynamicUnitData;

	cStaticUnitData rubbleSmall;
	cStaticUnitData rubbleBig;

	// unitdata does not change during the game.
	// So caching the checksum saves a lot cpu resources.
	mutable std::optional<uint32_t> crcCache;
};

extern cUnitsData UnitsDataGlobal;

#endif // game_data_units_unitdataH
