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

#ifndef game_data_units_unitH
#define game_data_units_unitH

#include "game/data/units/unitdata.h"
#include "utility/position.h"
#include "utility/signal/signal.h"

#include <string>

class cClient;
class cJob;
class cMap;
class cMapField;
class cMapView;
class cModel;
class cPlayer;
class cVehicle;

template <typename>
class cBox;

struct sTerrain;

enum class eSupplyType;

enum eTerrainFlag
{
	None = 0,
	Air = 1,
	Sea = 2,
	Ground = 4,
	Coast = 8,
	AreaSub = 16,
	AreaExpMine = 32
};

//-----------------------------------------------------------------------------
class cUnit
{
protected:
	cUnit (const cDynamicUnitData* unitData, const cStaticUnitData* staticData, cPlayer* owner, unsigned int ID);

public:
	virtual ~cUnit();

	unsigned int getId() const { return iID; };

	virtual bool isAVehicle() const = 0;
	virtual bool isABuilding() const = 0;

	cPlayer* getOwner() const { return owner; }
	void setOwner (cPlayer* owner);

	virtual bool canTransferTo (const cPosition&, const cMapView&) const = 0;
	virtual bool canTransferTo (const cUnit&) const = 0;
	virtual bool canLoad (const cVehicle*, bool checkPosition = true) const = 0;
	virtual bool canExitTo (const cPosition&, const cMapView&, const cStaticUnitData&) const = 0;
	virtual bool canExitTo (const cPosition&, const cMap&, const cStaticUnitData&) const = 0;
	virtual bool canSupply (const cUnit*, eSupplyType) const = 0;

	void storeVehicle (cVehicle&, cMap&);
	void exitVehicleTo (cVehicle&, const cPosition&, cMap&);

	virtual const cPosition& getMovementOffset() const
	{
		static const cPosition dummy (0, 0);
		return dummy;
	}

	const cPosition& getPosition() const { return position; }
	void setPosition (cPosition);

	std::vector<cPosition> getPositions() const;

	cBox<cPosition> getArea() const;

	std::vector<cPosition> getAdjacentPositions() const;

	int calcHealth (int damage) const;
	bool isInRange (const cPosition& position) const;
	/// checks whether the coordinates are next to the unit
	bool isNextTo (const cPosition& position) const;
	bool isDisabled() const { return turnsDisabled > 0; }
	bool isAbove (const cPosition& position) const;

	std::optional<std::string> getCustomName() const;
	void changeName (const std::string& newName);

	void rotateTo (int newDir);

	/** checks if the unit can attack something at the offset.
	 *  when forceAttack is false, the function only returns true,
	 *  if there is an enemy unit
	 */
	bool canAttackObjectAt (const cPosition& position, const cMapView& map, bool forceAttack = false, bool checkRange = true) const;

	/** Upgrades the unit data of this unit to the current,
	 * upgraded version of the player.
	 */
	void upgradeToCurrentVersion();

	/** checks if the unit has stealth abilities on its current position */
	bool isStealthOnCurrentTerrain (const cMapField&, const sTerrain&) const;
	/** check whether this unit has been detected by other players */
	void detectThisUnit (const cMap&, const std::vector<std::shared_ptr<cPlayer>>&);
	/** detects other stealth units in scan range of this unit */
	void detectOtherUnits (const cMap&) const;

	void setDetectedByPlayer (const cPlayer*);
	void resetDetectedByPlayer (const cPlayer*);
	bool isDetectedByPlayer (const cPlayer*) const;
	bool isDetectedByAnyPlayer() const;

	/** Resets the list of players, that detected this unit in this turn
	 * (is called at turn end). */
	void clearDetectedInThisTurnPlayerList();

	void setDisabledTurns (int turns);
	void setSentryActive (bool value);
	void setManualFireActive (bool value);
	void setAttacking (bool value);
	void setIsBeeinAttacked (bool value);
	void setHasBeenAttacked (bool value);

	int getDisabledTurns() const { return turnsDisabled; }
	bool isSentryActive() const { return sentryActive; }
	bool isManualFireActive() const { return manualFireActive; }
	bool isAttacking() const { return attacking; }
	bool isBeeingAttacked() const { return beeingAttacked; }
	bool hasBeenAttacked() const { return beenAttacked; }

	int getStoredResources() const { return storageResCur; }
	void setStoredResources (int value);

	//protected:
	virtual bool canBeStoppedViaUnitMenu() const = 0;

	bool getIsBig() const { return isBig; }
	void setIsBig (bool value);

	void forEachStoredUnits (std::function<void (const cVehicle&)> func) const;
	void forEachStoredUnits (std::function<void (cVehicle&)> func);


	virtual uint32_t getChecksum (uint32_t crc) const;

	// Important NOTE: This signal will be triggered when the destructor of the unit gets called.
	//                 This means when the signal is triggered it can not be guaranteed that all
	//                 of the objects attributes are still valid (especially the ones of derived classes).
	//                 Therefor you should not access the unit from a function that connects to this signal!
	mutable cSignal<void()> destroyed;

	mutable cSignal<void()> ownerChanged;

	mutable cSignal<void()> positionChanged;

	mutable cSignal<void()> renamed;
	mutable cSignal<void()> statusChanged;

	mutable cSignal<void()> disabledChanged;
	mutable cSignal<void()> sentryChanged;
	mutable cSignal<void()> manualFireChanged;
	mutable cSignal<void()> attackingChanged;
	mutable cSignal<void()> beeingAttackedChanged;
	mutable cSignal<void()> beenAttackedChanged;
	mutable cSignal<void()> movingChanged;

	mutable cSignal<void()> storedUnitsChanged; //the unit has loaded or unloaded another unit
	mutable cSignal<void()> stored; //this unit has been loaded by another unit
	mutable cSignal<void()> activated; //this unit has been unloaded by another unit

	mutable cSignal<void()> layingMinesChanged;
	mutable cSignal<void()> clearingMinesChanged;
	mutable cSignal<void()> buildingChanged;
	mutable cSignal<void()> clearingChanged;
	mutable cSignal<void()> workingChanged;
	mutable cSignal<void()> storedResourcesChanged;
	mutable cSignal<void()> isBigChanged;

	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (data);
		if (Archive::isWriter)
		{
			int id = iID;
			archive & NVP (id); //const member. needs to be deserialized before calling constructor
			storedUnitIds = ranges::Transform (storedUnits, [] (auto* unit) { return unit->getId(); });
		}
		else
		{
			storedUnitIds.clear();
		}
		archive & NVP (dir);
		archive & NVP (storedUnitIds);
		archive & NVP (detectedByPlayerList);
		archive & NVP (detectedInThisTurnByPlayerList);
		archive & NVP (position);
		archive & NVP (customName);
		archive & NVP (turnsDisabled);
		archive & NVP (sentryActive);
		archive & NVP (manualFireActive);
		archive & NVP (attacking);
		archive & NVP (beeingAttacked);
		archive & NVP (beenAttacked);
		archive & NVP (isBig);
		archive & NVP (storageResCur);
		archive & NVP (jobActive);
		// clang-format on
	}

	void postLoad (cModel&);

public: // TODO: make protected/private and make getters/setters
	const cStaticUnitData& getStaticUnitData() const;
	cDynamicUnitData data; // basic data of the unit
	const unsigned int iID; // the identification number of this unit
	int dir = 0; // ?Frame of the unit/current direction the unit is facing?

private:
	std::vector<unsigned int> storedUnitIds; // equivalent of storedUnits, for serialization only.
public:
	std::vector<cVehicle*> storedUnits; // list with the vehicles, that are stored in this unit

	// little jobs, running on the vehicle.
	// e.g. rotating to a specific direction
	bool jobActive = false;

	mutable int alphaEffectValue = 0;

	//-----------------------------------------------------------------------------
protected:
	/** checks if this is a stealth unit, and if it can be detected by player
	* at it's current position. Sets the "detectedByPlayer" state in this case.
	* Returns true, if the unit is detected by player (no matter if it was detected before.
	*/
	bool checkDetectedByPlayer (const cPlayer&, const cMap&) const;

	/** Detection state of stealth units. Use cPlayer::canSeeUnit() to check
	*   if the unit is actually visible at the moment.
	*   This list is always empty for units without stealth abilities.
	*/
	std::vector<int> detectedByPlayerList;

	/** list of players, that detected this vehicle in this turn */
	std::vector<int> detectedInThisTurnByPlayerList;

	const cStaticUnitData* staticData = nullptr;
	bool isBig = false;

	//-----------------------------------------------------------------------------
private:
	cPlayer* owner = nullptr;
	cPosition position;

	std::string customName; //stores the name of the unit, when the player enters an own name for the unit. Otherwise the string is empty.

	int turnsDisabled = 0; ///< the number of turns this unit will be disabled, 0 if the unit is active
	bool sentryActive = false; ///< is the unit on sentry?
	bool manualFireActive = false; ///< if active, then the unit only fires by manual control and not as reaction fire
	bool attacking = false; ///< is the unit currently attacking?
	bool beeingAttacked = false; ///< true when an attack on this unit is running
	bool beenAttacked = false; //the unit was attacked in this turn
	int storageResCur = 0; //amount of stored resources
};

template <typename T>
struct sUnitLess
{
	//static_assert (std::is_base_of<cUnit, T>::value, "Invalid template parameter. Has to be of a derived class of cUnit!");

	bool operator() (const std::shared_ptr<T>& left, const std::shared_ptr<T>& right) const
	{
		return left->iID < right->iID;
	}
	bool operator() (const std::shared_ptr<T>& left, const T& right) const
	{
		return left->iID < right.iID;
	}
	bool operator() (const T& left, const std::shared_ptr<T>& right) const
	{
		return left.iID < right->iID;
	}
	bool operator() (const T& left, const T& right) const
	{
		return left.iID < right.iID;
	}
	bool operator() (unsigned int left, const T& right) const
	{
		return left < right.iID;
	}
	bool operator() (const T& left, unsigned int right) const
	{
		return left.iID < right;
	}
	bool operator() (unsigned int left, const std::shared_ptr<T>& right) const
	{
		return left < right->iID;
	}
	bool operator() (const std::shared_ptr<T>& left, unsigned int right) const
	{
		return left->iID < right;
	}
};

#endif // game_data_units_unitH
