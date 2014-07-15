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
#ifndef unitH
#define unitH

#include <string>
#include "unitdata.h"
#include "utility/signal/signal.h"
#include "utility/position.h"

class cClient;
class cJob;
class cMap;
class cMapField;
class cPlayer;
class cServer;
class cVehicle;
template<typename T>
class cBox;

//-----------------------------------------------------------------------------
class cUnit
{
public:
	cUnit (const sUnitData* unitData, cPlayer* owner, unsigned int ID);
	virtual ~cUnit();

	virtual bool isAVehicle() const = 0;
	virtual bool isABuilding() const = 0;

	virtual bool canTransferTo (const cPosition& position, const cMapField& overUnitField) const = 0;
	virtual bool canExitTo (const cPosition& position, const cMap& map, const sUnitData& unitData) const = 0;
	virtual std::string getStatusStr (const cPlayer* player) const = 0;

	virtual const cPosition& getMovementOffset() const { static const cPosition dummy(0, 0); return dummy; }

	virtual void setDetectedByPlayer (cServer& server, cPlayer* player, bool addToDetectedInThisTurnList = true) = 0;

	const cPosition& getPosition () const;
	void setPosition (cPosition position);

	std::vector<cPosition> getAdjacentPositions () const;

	int calcHealth (int damage) const;
	bool isInRange (const cPosition& position) const;
	/// checks whether the coordinates are next to the unit
	bool isNextTo (const cPosition& position) const;
	bool isDisabled() const { return turnsDisabled > 0; }
	bool isAbove(const cPosition& position) const;


	const std::string& getName() const { return name; }
	bool isNameOriginal() const { return isOriginalName; }
	std::string getNamePrefix() const;
	std::string getDisplayName() const;
	void changeName (const std::string& newName);

	bool isLocked() const { return lockerPlayer != NULL; }

	void rotateTo (int newDir);

	/** checks if the unit can attack something at the offset
	 *  when forceAttack is false, the function only returns true,
	 *  if there is an enemy unit
	 *  ATTENTION: must not be called with forceAttack == false
	 *             from the server thread!
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
	void setMarkedAsDone (bool value);
	void setHasBeenAttacked (bool value);

	int getDisabledTurns () const;
	bool isSentryActive () const;
	bool isManualFireActive () const;
	bool isAttacking () const;
	bool isBeeingAttacked () const;
	bool isMarkedAsDone () const;
	bool hasBeenAttacked () const;

	//protected:
	virtual bool isUnitMoving() const { return false; }
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

	virtual void executeStopCommand (const cClient& client) const = 0;

	// Important NOTE: This signal will be triggered when the destructor of the unit gets called.
	//                 This means when the signal is triggered it can not be guaranteed that all
	//                 of the objects attributes are still valid (especially the ones of derived classes).
	//                 Therefor you should not access the unit from a function that connects to this signal!
	mutable cSignal<void ()> destroyed;

	mutable cSignal<void ()> positionChanged;

	mutable cSignal<void ()> renamed;
	mutable cSignal<void ()> statusChanged;

	mutable cSignal<void ()> disabledChanged;
	mutable cSignal<void ()> sentryChanged;
	mutable cSignal<void ()> manualFireChanged;
	mutable cSignal<void ()> attackingChanged;
	mutable cSignal<void ()> beeingAttackedChanged;
	mutable cSignal<void ()> markedAsDoneChanged;
	mutable cSignal<void ()> beenAttackedChanged;
	mutable cSignal<void ()> movingChanged;

	mutable cSignal<void ()> stored;
	mutable cSignal<void ()> activated;

	mutable cSignal<void ()> layingMinesChanged;
	mutable cSignal<void ()> clearingMinesChanged;
	mutable cSignal<void ()> buildingChanged;
	mutable cSignal<void ()> clearingChanged;
	mutable cSignal<void ()> workingChanged;

public: // TODO: make protected/private and make getters/setters
	sUnitData data; ///< basic data of the unit
	const unsigned int iID; ///< the identification number of this unit
	int dir; // ?Frame of the unit/current direction the unit is facing?

	std::vector<cVehicle*> storedUnits; ///< list with the vehicles, that are stored in this unit

	cPlayer* owner;
	std::vector<cPlayer*> seenByPlayerList; ///< a list of all players who can see this unit
	std::vector<cPlayer*> detectedByPlayerList; ///< a list of all players who have detected this unit

	// little jobs, running on the vehicle.
	// e.g. rotating to a specific direction
	cJob* job;

	cPlayer* lockerPlayer; // back pointer to (client) player which lock this unit
	//-----------------------------------------------------------------------------
protected:
	cBox<cPosition> getArea() const;

private:
    cPosition position;

	bool isOriginalName; // indicates whether the name has been changed by the player or not
	std::string name;    // name of the building

	int turnsDisabled;  ///< the number of turns this unit will be disabled, 0 if the unit is active
	bool sentryActive; ///< is the unit on sentry?
	bool manualFireActive; ///< if active, then the unit only fires by manual control and not as reaction fire
	bool attacking;  ///< is the unit currently attacking?
	bool beeingAttacked; ///< true when an attack on this unit is running
	bool markedAsDone; ///< the player has pressed the done button for this unit
	bool beenAttacked; //the unit was attacked in this turn
};

template<typename T>
struct sUnitLess
{
	//static_assert(std::is_base_of<cUnit, T>::value, "Invalid template parameter. Has to be of a derived class of cUnit!");

	bool operator()(const std::shared_ptr<T>& left, const std::shared_ptr<T>& right) const
	{
		return left->iID < right->iID;
	}
	bool operator()(const std::shared_ptr<T>& left, const T& right) const
	{
		return left->iID < right.iID;
	}
	bool operator()(const T& left, const std::shared_ptr<T>& right) const
	{
		return left.iID < right->iID;
	}
	bool operator()(const T& left, const T& right) const
	{
		return left.iID < right.iID;
	}
	bool operator()(unsigned int left, const T& right) const
	{
		return left < right.iID;
	}
	bool operator()(const T& left, unsigned int right) const
	{
		return left.iID < right;
	}
	bool operator()(unsigned int left, const std::shared_ptr<T>& right) const
	{
		return left < right->iID;
	}
	bool operator()(const std::shared_ptr<T>& left, unsigned int right) const
	{
		return left->iID < right;
	}
};

#endif
