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

#include <string>
#include "game/data/units/unitdata.h"
#include "utility/signal/signal.h"
#include "utility/position.h"

class cClient;
class cJob;
class cMap;
class cMapField;
class cPlayer;
class cServer;
class cVehicle;
template<typename> class cBox;
class cSoundManager;

//-----------------------------------------------------------------------------
class cUnit
{
protected:
	cUnit(const cDynamicUnitData* unitData, const cStaticUnitData* staticData, cPlayer* owner, unsigned int ID);
public:
	virtual ~cUnit();

	unsigned int getId() const { return iID; };

	virtual bool isAVehicle() const = 0;
	virtual bool isABuilding() const = 0;

	cPlayer* getOwner() const;
	void setOwner (cPlayer* owner);

	virtual bool canTransferTo (const cPosition& position, const cMapField& overUnitField) const = 0;
	virtual bool canExitTo (const cPosition& position, const cMap& map, const cStaticUnitData& unitData) const = 0;
	virtual std::string getStatusStr (const cPlayer* whoWantsToKnow, const cUnitsData& unitsData) const = 0;

	virtual void makeReport (cSoundManager& soundManager) const = 0;

	virtual const cPosition& getMovementOffset() const { static const cPosition dummy (0, 0); return dummy; }

	virtual void setDetectedByPlayer (cPlayer* player, bool addToDetectedInThisTurnList = true) = 0;

	const cPosition& getPosition() const;
	void setPosition (cPosition position);

	cBox<cPosition> getArea() const;

	std::vector<cPosition> getAdjacentPositions() const;

	int calcHealth (int damage) const;
	bool isInRange (const cPosition& position) const;
	/// checks whether the coordinates are next to the unit
	bool isNextTo (const cPosition& position) const;
	bool isDisabled() const { return turnsDisabled > 0; }
	bool isAbove (const cPosition& position) const;


	std::string getName() const;
	std::string getNamePrefix() const;
	std::string getDisplayName() const;
	void changeName (const std::string& newName);

	void rotateTo (int newDir);

	/** checks if the unit can attack something at the offset.
	 *  when forceAttack is false, the function only returns true,
	 *  if there is an enemy unit
	 */
	bool canAttackObjectAt (const cPosition& position, const cMap& map, bool forceAttack = false, bool checkRange = true) const;

	/** Upgrades the unit data of this unit to the current,
	 * upgraded version of the player.
	 */
	void upgradeToCurrentVersion();

	void setDisabledTurns (int turns);
	void setSentryActive (bool value);
	void setManualFireActive (bool value);
	void setAttacking (bool value);
	void setIsBeeinAttacked (bool value);
	void setHasBeenAttacked (bool value);

	int getDisabledTurns() const;
	bool isSentryActive() const;
	bool isManualFireActive() const;
	bool isAttacking() const;
	bool isBeeingAttacked() const;
	bool hasBeenAttacked() const;

	int getStoredResources() const;
	void setStoredResources(int value);

	//protected:
	virtual bool isUnitMoving() const { return false; } //test if the vehicle is moving right now. Having a waiting movejob doesn't count as moving
	virtual bool isAutoMoveJobActive() const { return false; }
	virtual bool isUnitWorking() const { return false; }
	virtual bool isUnitClearing() const { return false; }
	virtual bool isUnitLayingMines() const { return false; }
	virtual bool isUnitClearingMines() const { return false; }
	virtual bool isUnitBuildingABuilding() const { return false; }
	virtual bool factoryHasJustFinishedBuilding() const { return false; }
	virtual bool buildingCanBeStarted() const { return false; }
	virtual bool buildingCanBeUpgraded() const { return false; }
	virtual bool canBeStoppedViaUnitMenu() const = 0;

	bool getIsBig() const;
	void setIsBig(bool value);

	virtual uint32_t getChecksum(uint32_t crc) const;

	// Important NOTE: This signal will be triggered when the destructor of the unit gets called.
	//                 This means when the signal is triggered it can not be guaranteed that all
	//                 of the objects attributes are still valid (especially the ones of derived classes).
	//                 Therefor you should not access the unit from a function that connects to this signal!
	mutable cSignal<void ()> destroyed;

	mutable cSignal<void ()> ownerChanged;

	mutable cSignal<void ()> positionChanged;

	mutable cSignal<void ()> renamed;
	mutable cSignal<void ()> statusChanged;

	mutable cSignal<void ()> disabledChanged;
	mutable cSignal<void ()> sentryChanged;
	mutable cSignal<void ()> manualFireChanged;
	mutable cSignal<void ()> attackingChanged;
	mutable cSignal<void ()> beeingAttackedChanged;
	mutable cSignal<void ()> beenAttackedChanged;
	mutable cSignal<void ()> movingChanged;

	mutable cSignal<void ()> storedUnitsChanged; //the unit has loaded or unloaded another unit
	mutable cSignal<void ()> stored;            //this unit has been loaded by another unit
	mutable cSignal<void ()> activated;         //this unit has been unloaded by another unit

	mutable cSignal<void ()> layingMinesChanged;
	mutable cSignal<void ()> clearingMinesChanged;
	mutable cSignal<void ()> buildingChanged;
	mutable cSignal<void ()> clearingChanged;
	mutable cSignal<void ()> workingChanged;
	mutable cSignal<void ()> storedResourcesChanged;
	mutable cSignal<void ()> isBigChanged;

	template<typename T>
	void serializeThis(T& archive)
	{
		archive & NVP(data);
		//archive & NVP(iID);  //const member. needs to be deserialized before calling constructor
		archive & NVP(dir);
		archive & NVP(storedUnits);
		archive & NVP(detectedByPlayerList);
		archive & NVP(owner);
		archive & NVP(position);
		archive & NVP(customName);
		archive & NVP(turnsDisabled);
		archive & NVP(sentryActive);
		archive & NVP(manualFireActive);
		archive & NVP(attacking);
		archive & NVP(beeingAttacked);
		archive & NVP(beenAttacked);
		archive & NVP(isBig);
		archive & NVP(storageResCur);

		if (!archive.isWriter && data.getId() != sID(0, 0))
		{
			//restore pointer to static unit data
			archive.getPointerLoader()->get(data.getId(), staticData);
		}

		//TODO: detection?
	}

	
public: // TODO: make protected/private and make getters/setters
	const cStaticUnitData& getStaticUnitData() const;
	cDynamicUnitData data;		// basic data of the unit
	const unsigned int iID;		// the identification number of this unit
	int dir;					// ?Frame of the unit/current direction the unit is facing?

	std::vector<cVehicle*> storedUnits;		// list with the vehicles, that are stored in this unit

	std::vector<cPlayer*> seenByPlayerList; // a list of all players who can see this unit //TODO: remove
	std::vector<cPlayer*> detectedByPlayerList;		// a list of all players who have detected this unit

	// little jobs, running on the vehicle.
	// e.g. rotating to a specific direction
	cJob* job;

	mutable int alphaEffectValue;

	//-----------------------------------------------------------------------------
protected:
	const cStaticUnitData* staticData;
	bool isBig;

	//-----------------------------------------------------------------------------
private:
	cPlayer* owner;
	cPosition position;

	std::string customName; //stores the name of the unit, when the player enters an own name for the unit. Otherwise the string is empty.

	int turnsDisabled;  ///< the number of turns this unit will be disabled, 0 if the unit is active
	bool sentryActive; ///< is the unit on sentry?
	bool manualFireActive; ///< if active, then the unit only fires by manual control and not as reaction fire
	bool attacking;  ///< is the unit currently attacking?
	bool beeingAttacked; ///< true when an attack on this unit is running
	bool beenAttacked; //the unit was attacked in this turn
	int storageResCur; //amount of stored ressources

};

template<typename T>
struct sUnitLess
{
	//static_assert(std::is_base_of<cUnit, T>::value, "Invalid template parameter. Has to be of a derived class of cUnit!");

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
