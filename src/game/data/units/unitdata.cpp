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

#include "game/data/units/unitdata.h"

//------------------------------------------------------------------------------
sUnitData::sUnitData()
{
	version = 0;
	muzzleType = MUZZLE_TYPE_NONE;

	ammoMax = 0;
	ammoCur = 0;
	shotsMax = 0;
	shotsCur = 0;
	range = 0;
	damage = 0;
	canAttack = 0;
	canDriveAndFire = false;

	buildCosts = 0;
	maxBuildFactor = 0;

	canBuildPath = false;
	canBuildRepeat = false;

	// Movement
	speedMax = 0;
	speedCur = 0;

	factorGround = 0.0f;
	factorSea = 0.0f;
	factorAir = 0.0f;
	factorCoast = 0.0f;

	// Abilities
	isBig = false;
	connectsToBase = false;
	armor = 0;
	hitpointsMax = 0;
	hitpointsCur = 0;
	scan = 0;
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

	// Storage
	storageResMax = 0;
	storageResCur = 0;
	storeResType = STORE_RES_NONE;
	storageUnitsMax = 0;
	storageUnitsCur = 0;
	storeUnitsImageType = STORE_UNIT_IMG_NONE;

	// Graphic
	hasClanLogos = false;
	hasCorpse = false;
	hasDamageEffect = false;
	hasBetonUnderground = false;
	hasPlayerColor = false;
	hasOverlay = false;

	buildUpGraphic = false;
	animationMovement = false;
	powerOnGraphic = false;
	isAnimated = false;
	makeTracks = false;

	isConnectorGraphic = false;
	hasFrames = 0;
}

//------------------------------------------------------------------------------
sUnitData::sUnitData (const sUnitData& other)
{
	(*this) = other;
}

//------------------------------------------------------------------------------
sUnitData& sUnitData::operator= (const sUnitData& other)
{
	ID = other.ID;
	name = other.name;
	description = other.description;
	version = other.version;
	muzzleType = other.muzzleType;
	ammoMax = other.getAmmoMax();
	shotsMax = other.getShotsMax();
	range = other.range;
	damage = other.damage;
	canAttack = other.canAttack;
	canDriveAndFire = other.canDriveAndFire;
	buildCosts = other.buildCosts;
	canBuild = other.canBuild;
	buildAs = other.buildAs;
	maxBuildFactor = other.maxBuildFactor;
	canBuildPath = other.canBuildPath;
	canBuildRepeat = other.canBuildRepeat;
	speedMax = other.getSpeedMax();
	speedCur = other.getSpeed();
	factorGround = other.factorGround;
	factorSea = other.factorSea;
	factorAir = other.factorAir;
	factorCoast = other.factorCoast;
	isBig = other.isBig;
	connectsToBase = other.connectsToBase;
	armor = other.armor;
	hitpointsMax = other.getHitpointsMax();
	scan = other.scan;
	modifiesSpeed = other.modifiesSpeed;
	canClearArea = other.canClearArea;
	canBeCaptured = other.canBeCaptured;
	canBeDisabled = other.canBeDisabled;
	canCapture = other.canCapture;
	canDisable = other.canDisable;
	canRepair = other.canRepair;
	canRearm = other.canRearm;
	canResearch = other.canResearch;
	canPlaceMines = other.canPlaceMines;
	canSurvey = other.canSurvey;
	doesSelfRepair = other.doesSelfRepair;
	convertsGold = other.convertsGold;
	canSelfDestroy = other.canSelfDestroy;
	canScore = other.canScore;
	canMineMaxRes = other.canMineMaxRes;
	needsMetal = other.needsMetal;
	needsOil = other.needsOil;
	needsEnergy = other.needsEnergy;
	needsHumans = other.needsHumans;
	produceEnergy = other.produceEnergy;
	produceHumans = other.produceHumans;
	isStealthOn = other.isStealthOn;
	canDetectStealthOn = other.canDetectStealthOn;
	surfacePosition = other.surfacePosition;
	canBeOverbuild = other.canBeOverbuild;
	canBeLandedOn = other.canBeLandedOn;
	canWork = other.canWork;
	explodesOnContact = other.explodesOnContact;
	isHuman = other.isHuman;
	storageResMax = other.storageResMax;
	storageResCur = other.storageResCur;
	storeResType = other.storeResType;
	storageUnitsMax = other.storageUnitsMax;
	storageUnitsCur = other.storageUnitsCur;
	storeUnitsImageType = other.storeUnitsImageType;
	storeUnitsTypes = other.storeUnitsTypes;
	isStorageType = other.isStorageType;
	hasClanLogos = other.hasClanLogos;
	hasCorpse = other.hasCorpse;
	hasDamageEffect = other.hasDamageEffect;
	hasBetonUnderground = other.hasBetonUnderground;
	hasPlayerColor = other.hasPlayerColor;
	hasOverlay = other.hasOverlay;
	buildUpGraphic = other.buildUpGraphic;
	animationMovement = other.animationMovement;
	powerOnGraphic = other.powerOnGraphic;
	isAnimated = other.isAnimated;
	makeTracks = other.makeTracks;
	isConnectorGraphic = other.isConnectorGraphic;
	hasFrames = other.hasFrames;
	hitpointsCur = other.hitpointsCur;
	shotsCur = other.shotsCur;
	ammoCur = other.ammoCur;

	return *this;
}

//------------------------------------------------------------------------------
int sUnitData::getVersion() const
{
	return version;
}

//------------------------------------------------------------------------------
void sUnitData::setVersion (int value)
{
	std::swap (version, value);
	if (version != value) versionChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getSpeed() const
{
	return speedCur;
}

//------------------------------------------------------------------------------
void sUnitData::setSpeed (int value)
{
	std::swap (speedCur, value);
	if (speedCur != value) speedChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getSpeedMax() const
{
	return speedMax;
}

//------------------------------------------------------------------------------
void sUnitData::setSpeedMax (int value)
{
	std::swap (speedMax, value);
	if (speedMax != value) speedMaxChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getHitpoints() const
{
	return hitpointsCur;
}

//------------------------------------------------------------------------------
void sUnitData::setHitpoints (int value)
{
	std::swap (hitpointsCur, value);
	if (hitpointsCur != value) hitpointsChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getHitpointsMax() const
{
	return hitpointsMax;
}

//------------------------------------------------------------------------------
void sUnitData::setHitpointsMax (int value)
{
	std::swap (hitpointsMax, value);
	if (hitpointsMax != value) hitpointsMaxChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getScan() const
{
	return scan;
}

//------------------------------------------------------------------------------
void sUnitData::setScan (int value)
{
	std::swap (scan, value);
	if (scan != value) scanChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getRange() const
{
	return range;
}

//------------------------------------------------------------------------------
void sUnitData::setRange (int value)
{
	std::swap (range, value);
	if (range != value) rangeChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getShots() const
{
	return shotsCur;
}

//------------------------------------------------------------------------------
void sUnitData::setShots (int value)
{
	std::swap (shotsCur, value);
	if (shotsCur != value) shotsChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getShotsMax() const
{
	return shotsMax;
}

//------------------------------------------------------------------------------
void sUnitData::setShotsMax (int value)
{
	std::swap (shotsMax, value);
	if (shotsMax != value) shotsMaxChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getAmmo() const
{
	return ammoCur;
}

//------------------------------------------------------------------------------
void sUnitData::setAmmo (int value)
{
	std::swap (ammoCur, value);
	if (ammoCur != value) ammoChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getAmmoMax() const
{
	return ammoMax;
}

//------------------------------------------------------------------------------
void sUnitData::setAmmoMax (int value)
{
	std::swap (ammoMax, value);
	if (ammoMax != value) ammoMaxChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getDamage() const
{
	return damage;
}

//------------------------------------------------------------------------------
void sUnitData::setDamage (int value)
{
	std::swap (damage, value);
	if (damage != value) damageChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getArmor() const
{
	return armor;
}

//------------------------------------------------------------------------------
void sUnitData::setArmor (int value)
{
	std::swap (armor, value);
	if (armor != value) armorChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getStoredResources() const
{
	return storageResCur;
}

//------------------------------------------------------------------------------
void sUnitData::setStoredResources (int value)
{
	value = std::max (std::min (value, storageResMax), 0);
	std::swap (storageResCur, value);
	if (storageResCur != value) storedResourcesChanged();
}

//------------------------------------------------------------------------------
int sUnitData::getStoredUnits() const
{
	return storageUnitsCur;
}

//------------------------------------------------------------------------------
void sUnitData::setStoredUnits (int value)
{
	value = std::max (std::min (value, storageUnitsMax), 0);
	std::swap (storageUnitsCur, value);
	if (storageUnitsCur != value) storedUnitsChanged();
}
