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

#ifndef unitdataH
#define unitdataH

#include <string>
#include <utility>
#include <vector>

#include "main.h" // sID
#include "utility/signal/signal.h"

// struct for vehicle properties
struct sUnitData
{
	sUnitData ();
	sUnitData (const sUnitData& other);

	sUnitData& operator=(const sUnitData& other);

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

	int ammoMax;
	int shotsMax;
	int range;
	int damage;

	char canAttack;

	bool canDriveAndFire;

	// Production
	int buildCosts;
	std::string canBuild;
	std::string buildAs;

	int maxBuildFactor;

	bool canBuildPath;
	bool canBuildRepeat;

	// Movement
	int speedMax;
	int speedCur;

	float factorGround;
	float factorSea;
	float factorAir;
	float factorCoast;

	// Abilities
	bool isBig;
	bool connectsToBase;
	int armor;
	int hitpointsMax;
	int scan;
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
	int storageResCur;
	enum eStorageResType
	{
		STORE_RES_NONE,
		STORE_RES_METAL,
		STORE_RES_OIL,
		STORE_RES_GOLD
	};
	eStorageResType storeResType;

	int storageUnitsMax;
	int storageUnitsCur;
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

	int getVersion () const;
	void setVersion (int value);

	int getHitpoints () const;
	void setHitpoints (int value);

	int getShots () const;
	void setShots (int value);

	int getAmmo () const;
	void setAmmo (int value);

	cSignal<void ()> versionChanged;
	cSignal<void ()> hitpointsChanged;
	cSignal<void ()> shotsChanged;
	cSignal<void ()> ammoChanged;
private:
	int version;

	int hitpointsCur;
	int shotsCur;
	int ammoCur;

};

#endif // unitdataH
