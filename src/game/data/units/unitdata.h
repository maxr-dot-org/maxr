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

#include "main.h" // sID
#include "utility/signal/signal.h"

// struct for vehicle properties
struct sUnitData
{
	sUnitData();
	sUnitData (const sUnitData& other);

	sUnitData& operator= (const sUnitData& other);

	// Main
	sID ID;
	std::string name;
	std::string description;

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

	// Production
	int buildCosts;
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
	bool isBig;
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

	// Graphic
	bool hasClanLogos;
	bool hasCorpse;
	bool hasDamageEffect;
	bool hasBetonUnderground;
	bool hasPlayerColor;
	bool hasOverlay;

	bool buildUpGraphic;
	bool animationMovement;
	bool powerOnGraphic;
	bool isAnimated;
	bool makeTracks;

	bool isConnectorGraphic;
	int hasFrames;

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

	int getStoredResources() const;
	void setStoredResources (int value);

	mutable cSignal<void ()> versionChanged;
	mutable cSignal<void ()> speedChanged;
	mutable cSignal<void ()> speedMaxChanged;
	mutable cSignal<void ()> hitpointsChanged;
	mutable cSignal<void ()> hitpointsMaxChanged;
	mutable cSignal<void ()> shotsChanged;
	mutable cSignal<void ()> shotsMaxChanged;
	mutable cSignal<void ()> ammoChanged;
	mutable cSignal<void ()> ammoMaxChanged;
	mutable cSignal<void ()> scanChanged;
	mutable cSignal<void ()> rangeChanged;
	mutable cSignal<void ()> damageChanged;
	mutable cSignal<void ()> armorChanged;
	mutable cSignal<void ()> storedResourcesChanged;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(ID);
		archive & NVP(name);
		archive & NVP(description);
		archive & NVP(muzzleType);
		archive & NVP(canAttack);
		archive & NVP(canDriveAndFire);
		archive & NVP(buildCosts);
		archive & NVP(canBuild);
		archive & NVP(buildAs);
		archive & NVP(maxBuildFactor);
		archive & NVP(canBuildPath);
		archive & NVP(canBuildRepeat);
		archive & NVP(factorGround);
		archive & NVP(factorSea);
		archive & NVP(factorAir);
		archive & NVP(factorCoast);
		archive & NVP(isBig);
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
		archive & NVP(storageResMax);
		archive & NVP(storeResType);
		archive & NVP(storageUnitsMax);
		archive & NVP(storeUnitsImageType);
		archive & NVP(storeUnitsTypes);
		archive & NVP(isStorageType);
		archive & NVP(hasClanLogos);
		archive & NVP(hasCorpse);
		archive & NVP(hasDamageEffect);
		archive & NVP(hasBetonUnderground);
		archive & NVP(hasPlayerColor);
		archive & NVP(hasOverlay);
		archive & NVP(buildUpGraphic);
		archive & NVP(animationMovement);
		archive & NVP(powerOnGraphic);
		archive & NVP(isAnimated);
		archive & NVP(makeTracks);
		archive & NVP(isConnectorGraphic);
		archive & NVP(hasFrames);
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
		archive & NVP(storageResCur);
	}
private:
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

	int storageResCur;
};

#endif // game_data_units_unitdataH
