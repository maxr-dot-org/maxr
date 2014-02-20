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
#include "main.h" /// for sUnitData -> move that to cUnit, too?

class cClient;
class cGameGUI;
class cJob;
class cMap;
class cMapField;
class cPlayer;
class cServer;
class cVehicle;
class cPosition;
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

	virtual bool CanTransferTo (int x, int y, cMapField* OverUnitField) const = 0;
	virtual std::string getStatusStr (const cGameGUI& gameGUI) const = 0;

	virtual int getMovementOffsetX() const {return 0;}
	virtual int getMovementOffsetY() const {return 0;}

	virtual void setDetectedByPlayer (cServer& server, cPlayer* player, bool addToDetectedInThisTurnList = true) = 0;

	int calcHealth (int damage) const;
	bool isInRange (int x, int y) const;
	/// checks whether the coordinates are next to the unit
	bool isNextTo (int x, int y) const;
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
	bool canAttackObjectAt (int x, int y, cMap& map, bool forceAttack = false, bool checkRange = true) const;

	/** Upgrades the unit data of this unit to the current,
	 * upgraded version of the player.
	 */
	void upgradeToCurrentVersion();

	void deleteStoredUnits();

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

	virtual void executeBuildCommand (cGameGUI&) = 0;
	virtual void executeStopCommand (const cClient& client) = 0;
	virtual void executeActivateStoredVehiclesCommand (cGameGUI& gameGUI) = 0;

public: // TODO: make protected/private and make getters/setters
	sUnitData data; ///< basic data of the unit
	const unsigned int iID; ///< the identification number of this unit
	int PosX;
	int PosY;
	int dir; // ?Frame of the unit/current direction the unit is facing?
	int turnsDisabled;  ///< the number of turns this unit will be disabled, 0 if the unit is active
	bool sentryActive; ///< is the unit on sentry?
	bool manualFireActive; ///< if active, then the unit only fires by manual control and not as reaction fire
	bool attacking;  ///< is the unit currently attacking?
	bool isBeeingAttacked; ///< true when an attack on this unit is running
	bool isMarkedAsDone; ///< the player has pressed the done button for this unit
	bool hasBeenAttacked; //the unit was attacked in this turn

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
	bool isOriginalName; // indicates whether the name has been changed by the player or not
	std::string name;    // name of the building

	cBox<cPosition> getArea() const;
};

//
// double linked-list with elements of type T.
// Following member should exist
// T* T::next;
// T* T::prev;
//
template <typename T>
void remove_from_intrusivelist (T*& root, T& elem)
{
	if (&elem == root) root = elem.next;
	if (elem.next) elem.next->prev = elem.prev;
	if (elem.prev) elem.prev->next = elem.next;
	elem.next = NULL;
	elem.prev = NULL;
}

template<typename T>
void push_front_into_intrusivelist (T*& root, T& elem)
{
	elem.prev = NULL;
	elem.next = root;
	if (root != NULL) root->prev = &elem;
	root = &elem;
}

template<typename T> void insert_after_in_intrusivelist (T*& root, T* it, T& elem)
{
	if (it == NULL)
	{
		push_front_into_intrusivelist (root, elem);
		return;
	}
	elem.next = it->next;
	if (elem.next != NULL) elem.next->prev = &elem;
	it->next = &elem;
	elem.prev = it;
}

template <typename T> T* get_last_of_intrusivelist (T* root)
{
	if (root == NULL) return NULL;
	T* it = root;
	for (; it->next; it = it->next)
		; // Nothing
	return it;
}

#endif
