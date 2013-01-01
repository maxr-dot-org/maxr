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
#include <SDL.h>
#include "main.h" /// for sUnitData -> move that to cUnit, too?

class cPlayer;
class cMap;
class cVehicle;
class cGameGUI;
class cJob;

//-----------------------------------------------------------------------------
class cUnit
{
public:
	enum UnitType
	{
		kUTBuilding,
		kUTVehicle
	};

	cUnit (UnitType type, sUnitData* unitData, cPlayer* owner, unsigned int ID);
	virtual ~cUnit();

	bool isVehicle() const { return unitType == kUTVehicle; }
	bool isBuilding() const { return unitType == kUTBuilding; }

	int calcHealth (int damage) const;
	bool isInRange (int x, int y) const;
	bool isNextTo (int x, int y) const;  ///< checks whether the coordinates are next to the unit

	const std::string& getName() const { return name; }
	bool isNameOriginal() const { return isOriginalName; }

	std::string getNamePrefix() const;
	std::string getDisplayName() const;
	void changeName (const std::string& newName);

	SDL_Rect getMenuSize() const;
	bool areCoordsOverMenu (int x, int y) const;
	void setMenuSelection();
	void drawMenu (cGameGUI& gameGUI);
	void menuReleased (cGameGUI& gameGUI);
	virtual void executeAutoMoveJobCommand() {}
	virtual void executeLayMinesCommand() {}
	virtual void executeClearMinesCommand() {}

	int getScreenPosX (bool movementOffset = true) const;
	int getScreenPosY (bool movementOffset = true) const;
	void center() const;

	virtual int getMovementOffsetX() const {return 0;}
	virtual int getMovementOffsetY() const {return 0;}

	void drawMunBar( const SDL_Rect& screenPos ) const;
	void drawHealthBar( const SDL_Rect& screenPos ) const;
	void rotateTo (int newDir);

	virtual void setDetectedByPlayer (cPlayer* player, bool addToDetectedInThisTurnList = true) {}

	/** checks if the unit can attack something at the offset
	 *  when forceAttack is false, the function only returns true, if there is an enemy unit
	 *  ATTENTION: must not be called with forceAttack == false from the server thread!
	 */
	bool canAttackObjectAt (int x, int y, cMap* map, bool forceAttack = false, bool checkRange = true) const;

	void upgradeToCurrentVersion(); ///< Upgrades the unit data of this unit to the current, upgraded version of the player.

	void deleteStoredUnits();


	//------------------------------- public members: TODO: make protected and make getters/setters

	sUnitData data; ///< basic data of the unit
	const unsigned int iID; ///< the identification number of this unit
	int PosX, PosY;
	int dir; // ?Frame of the unit/current direction the unit is facing?
	int turnsDisabled;  ///< the number of turns this unit will be disabled, 0 if the unit is active
	bool sentryActive; ///< is the unit on sentry?
	bool manualFireActive; ///< if active, then the unit only fires by manual control and not as reaction fire
	bool attacking;  ///< is the unit currently attacking?
	bool isBeeingAttacked; ///< true when an attack on this unit is running
	bool isMarkedAsDone; ///< the player has pressed the done button for this unit
	bool hasBeenAttacked; //the unit was attacked in this turn

	cUnit* next; ///< "next"-pointer for the double linked list
	cUnit* prev; ///< "prev"-pointer for the double linked list
	cList<cVehicle*> storedUnits; ///< list with the vehicles, that are stored in this unit

	int selectedMenuButtonIndex;

	cPlayer* owner;
	cList<cPlayer*> seenByPlayerList; ///< a list were the numbers of all players who can see this unit are stored in
	cList<cPlayer*> detectedByPlayerList; ///< a list were the numbers of all players who have deteced this unit are stored in

	cJob* job;	//little jobs, running on the vehicle. e. g. rotating to a spezific direction

	//-----------------------------------------------------------------------------
protected:
	UnitType unitType;

	bool isOriginalName;	// indicates whether the name has been changed by the player or not
	std::string name;		// name of the building

	void drawStatus (const SDL_Rect& screenPos) const;
	int getNumberOfMenuEntries() const;

	virtual bool isUnitLoaded() const { return false; }
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
	virtual bool canBeStoppedViaUnitMenu() const { return false; }

	virtual void executeBuildCommand() {}
	virtual void executeMineManagerCommand() {}
	virtual void executeStopCommand() {}
	virtual void executeActivateStoredVehiclesCommand() {}
	virtual void executeUpdateBuildingCommmand (bool updateAllOfSameType) {}
	virtual void executeSelfDestroyCommand() {}

	virtual sUnitData* getUpgradedUnitData() const = 0;
};

#endif
