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

#include <string>
#include <utility>
#include <vector>

#include "utility/signal/signal.h"
#include "utility/serialization/serialization.h"


struct sID
{
	sID() : firstPart(0), secondPart(0) {}
	sID(int first, int second) : firstPart(first), secondPart(second) {}

	std::string getText() const;
	void generate(const std::string& text);

	bool isAVehicle() const { return firstPart == 0; }
	bool isABuilding() const { return firstPart == 1; }

	/** Get the basic version of a unit.
	* @param Owner If Owner is given, his clan will be taken
	*        into consideration for modifications of the unit's values.
	* @return the sUnitData of the owner without upgrades
	*         (but with the owner's clan modifications) */
	//const sUnitData* getUnitDataOriginalVersion (const cPlayer* Owner = nullptr) const;

	bool operator== (const sID& ID) const;
	bool operator!= (const sID& rhs) const { return !(*this == rhs); }
	bool operator< (const sID& rhs) const { return less_vehicleFirst(rhs); }
	bool less_vehicleFirst(const sID& ID) const;
	bool less_buildingFirst(const sID& ID) const;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(firstPart);
		archive & NVP(secondPart);
	}

public:
	int firstPart;
	int secondPart;
};

// class for vehicle properties, that are constant and equal for all instances of a unit type
class cStaticUnitData
{
public:
	cStaticUnitData();
	std::string getName() const;
	std::string getDescripton() const;
	void setName(std::string name_){ name = name_; }
	void setDescription(std::string text) { description = text; }

	//sUnitData(const sUnitData& other);			//TODO: default implementation should be ok
	//sUnitData& operator= (const sUnitData& other);//TODO: default implementation should be ok

	// Main
	sID ID;
	
	// Attack
	enum eMuzzleType
	{
		MUZZLE_TYPE_NONE,
		MUZZLE_TYPE_BIG,
		MUZZLE_TYPE_ROCKET,
		MUZZLE_TYPE_SMALL,
		MUZZLE_TYPE_MED,
		MUZZLE_TYPE_MED_LONG,
		MUZZLE_TYPE_ROCKET_CLUSTER,
		MUZZLE_TYPE_TORPEDO,
		MUZZLE_TYPE_SNIPER
	};
	eMuzzleType muzzleType;

	char canAttack;

	bool canDriveAndFire;

	std::string canBuild;
	std::string buildAs;

	int maxBuildFactor;

	bool canBuildPath;
	bool canBuildRepeat;

	float factorGround;
	float factorSea;
	float factorAir;
	float factorCoast;

	// Abilities
	bool connectsToBase;
	float modifiesSpeed;
	bool canClearArea;
	bool canBeCaptured;
	bool canBeDisabled;
	bool canCapture;
	bool canDisable;
	bool canRepair;
	bool canRearm;
	bool canResearch;
	bool canPlaceMines;
	bool canSurvey;
	bool doesSelfRepair;
	int convertsGold;
	bool canSelfDestroy;
	bool canScore;

	int canMineMaxRes;

	int needsMetal;
	int needsOil;
	int needsEnergy;
	int needsHumans;
	int produceEnergy;
	int produceHumans;

	char isStealthOn;
	char canDetectStealthOn;

	enum eSurfacePosition
	{
		SURFACE_POS_BENEATH_SEA,
		SURFACE_POS_ABOVE_SEA,
		SURFACE_POS_BASE,
		SURFACE_POS_ABOVE_BASE,
		SURFACE_POS_GROUND,
		SURFACE_POS_ABOVE
	};
	eSurfacePosition surfacePosition;

	enum eOverbuildType
	{
		OVERBUILD_TYPE_NO,
		OVERBUILD_TYPE_YES,
		OVERBUILD_TYPE_YESNREMOVE
	};
	eOverbuildType canBeOverbuild;

	bool canBeLandedOn;
	bool canWork;
	bool explodesOnContact;
	bool isHuman;
	bool isBig;

	// Storage
	int storageResMax;
	enum eStorageResType
	{
		STORE_RES_NONE,
		STORE_RES_METAL,
		STORE_RES_OIL,
		STORE_RES_GOLD
	};
	eStorageResType storeResType;

	int storageUnitsMax;
	enum eStorageUnitsImageType
	{
		STORE_UNIT_IMG_NONE,
		STORE_UNIT_IMG_TANK,
		STORE_UNIT_IMG_PLANE,
		STORE_UNIT_IMG_SHIP,
		STORE_UNIT_IMG_HUMAN
	};
	eStorageUnitsImageType storeUnitsImageType;
	std::vector<std::string> storeUnitsTypes;
	std::string isStorageType;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(ID);
		archive & NVP(muzzleType);
		archive & NVP(canAttack);
		archive & NVP(canDriveAndFire);
		archive & NVP(canBuild);
		archive & NVP(buildAs);
		archive & NVP(maxBuildFactor);
		archive & NVP(canBuildPath);
		archive & NVP(canBuildRepeat);
		archive & NVP(factorGround);
		archive & NVP(factorSea);
		archive & NVP(factorAir);
		archive & NVP(factorCoast);
		archive & NVP(connectsToBase);
		archive & NVP(modifiesSpeed);
		archive & NVP(canClearArea);
		archive & NVP(canBeCaptured);
		archive & NVP(canBeDisabled);
		archive & NVP(canCapture);
		archive & NVP(canDisable);
		archive & NVP(canRepair);
		archive & NVP(canRearm);
		archive & NVP(canResearch);
		archive & NVP(canPlaceMines);
		archive & NVP(canSurvey);
		archive & NVP(doesSelfRepair);
		archive & NVP(convertsGold);
		archive & NVP(canSelfDestroy);
		archive & NVP(canScore);
		archive & NVP(canMineMaxRes);
		archive & NVP(needsMetal);
		archive & NVP(needsOil);
		archive & NVP(needsEnergy);
		archive & NVP(needsHumans);
		archive & NVP(produceEnergy);
		archive & NVP(produceHumans);
		archive & NVP(isStealthOn);
		archive & NVP(canDetectStealthOn);
		archive & NVP(surfacePosition);
		archive & NVP(canBeOverbuild);
		archive & NVP(canBeLandedOn);
		archive & NVP(canWork);
		archive & NVP(explodesOnContact);
		archive & NVP(isHuman);
		archive & NVP(isBig);
		archive & NVP(storageResMax);
		archive & NVP(storeResType);
		archive & NVP(storageUnitsMax);
		archive & NVP(storeUnitsImageType);
		archive & NVP(storeUnitsTypes);
		archive & NVP(isStorageType);
		archive & NVP(description);
		archive & NVP(name);
	}

private:
	std::string description; //untranslated data from unit xml. Will be used, when translation for the unit is not available
	std::string name;        //untranslated data from unit xml. Will be used, when translation for the unit is not available
};

//class for vehicle properties, that are individual for each instance of a unit
class cDynamicUnitData
{
public:
	cDynamicUnitData();
	cDynamicUnitData(const cDynamicUnitData& other);
	cDynamicUnitData& operator= (const cDynamicUnitData& other);

	void setMaximumCurrentValues();

	sID getId() const;
	void setId(const sID& value);

	int getBuildCost() const;
	void setBuildCost(int value);

	int getVersion() const;
	void setVersion(int value);

	int getSpeed() const;
	void setSpeed(int value);

	int getSpeedMax() const;
	void setSpeedMax(int value);

	int getHitpoints() const;
	void setHitpoints(int value);

	int getHitpointsMax() const;
	void setHitpointsMax(int value);

	int getScan() const;
	void setScan(int value);

	int getRange() const;
	void setRange(int value);

	int getShots() const;
	void setShots(int value);

	int getShotsMax() const;
	void setShotsMax(int value);

	int getAmmo() const;
	void setAmmo(int value);

	int getAmmoMax() const;
	void setAmmoMax(int value);

	int getDamage() const;
	void setDamage(int value);

	int getArmor() const;
	void setArmor(int value);

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
	void serialize(T& archive)
	{
		archive & NVP(id);
		archive & NVP(buildCosts);
		archive & NVP(version);
		archive & NVP(speedCur);
		archive & NVP(speedMax);
		archive & NVP(hitpointsCur);
		archive & NVP(hitpointsMax);
		archive & NVP(shotsCur);
		archive & NVP(shotsMax);
		archive & NVP(ammoCur);
		archive & NVP(ammoMax);
		archive & NVP(range);
		archive & NVP(scan);
		archive & NVP(damage);
		archive & NVP(armor);
	}
private:
	// Main
	sID id;

	// Production
	int buildCosts;

	int version;

	int speedCur;
	int speedMax;

	int hitpointsCur;
	int hitpointsMax;
	int shotsCur;
	int shotsMax;
	int ammoCur;
	int ammoMax;

	int range;
	int scan;

	int damage;
	int armor;
};

class cUnitsData
{
public:
	void initializeIDData();
	void initializeClanUnitData();

	void addData(const cDynamicUnitData& data) { dynamicUnitData.push_back(data); }
	void addData(const cStaticUnitData& data)  { staticUnitData.push_back(data); }

	bool isValidId(const sID& id) const;


	// clan = -1: without clans
	const cDynamicUnitData& getDynamicUnitData(const sID& id, int clan = -1) const;
	const cStaticUnitData& getStaticUnitData(const sID& id) const;

	// clan = -1: without clans
	const std::vector<cDynamicUnitData>& getDynamicUnitsData(int clan = -1) const;
	const std::vector<cStaticUnitData>& getStaticUnitsData() const;


	const cStaticUnitData& getConstructorData() const { return getStaticUnitData(constructorID); }
	const cStaticUnitData& getEngineerData() const { return getStaticUnitData(engineerID); }
	const cStaticUnitData& getSurveyorData() const { return getStaticUnitData(surveyorID); }
	const cStaticUnitData& getMineData() const { return getStaticUnitData(specialIDMine); }
	const cStaticUnitData& getSmallGeneratorData() const { return getStaticUnitData(specialIDSmallGen); }
	const cStaticUnitData& getLandMineData() const { return getStaticUnitData(specialIDLandMine); }
	const cStaticUnitData& getSeaMineData() const { return getStaticUnitData(specialIDSeaMine); }

	sID constructorID;
	sID engineerID;
	sID surveyorID;
	sID specialIDLandMine;
	sID specialIDSeaMine;
	sID specialIDMine;
	sID specialIDSmallGen;
	sID specialIDConnector;
	sID specialIDSmallBeton;

	template <typename T>
	void serialize(T& archive)
	{
		if (!archive.isWriter)
		{
			staticUnitData.clear();
			dynamicUnitData.clear();
			clanDynamicUnitData.clear();
		}

		archive & NVP(constructorID);
		archive & NVP(engineerID);
		archive & NVP(surveyorID);
		archive & NVP(specialIDLandMine);
		archive & NVP(specialIDSeaMine);
		archive & NVP(specialIDMine);
		archive & NVP(specialIDSmallGen);
		archive & NVP(specialIDConnector);
		archive & NVP(specialIDSmallBeton);
		archive & NVP(staticUnitData);
		archive & NVP(dynamicUnitData);
		archive & NVP(clanDynamicUnitData);
	}

private:
	int getUnitIndexBy(sID id) const;

	// the static unit data
	std::vector<cStaticUnitData> staticUnitData;

	// the dynamic unit data. Standard version without clan modifications
	std::vector<cDynamicUnitData> dynamicUnitData;

	// the dynamic unit data. Contains the modified versions for the clans
	std::vector<std::vector<cDynamicUnitData> > clanDynamicUnitData;

};

#endif // game_data_units_unitdataH
