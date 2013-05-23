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
#ifndef buildingsH
#define buildingsH
#include <SDL.h>
#include "defines.h"
#include "upgradecalculator.h"
#include "main.h" // for sUnitData, sID
#include "unit.h"
#include <vector>

class cBase;
class cGameGUI;
class cPlayer;
class cVehicle;
class cMap;
class cMapField;
class cServer;
struct sVehicle;

//--------------------------------------------------------------------------
/** Struct for one upgrade (one kind of value, e.g. hitpointsMax) */
//--------------------------------------------------------------------------
struct sUpgrade
{
	bool active; // is this upgrade buyable for the player
	int NextPrice; // what will the next upgrade cost
	int Purchased;
	int* value; // what is the current value
	int StartValue; // the initial value for this unit type
	std::string name; // the name of this upgrade type, e.g. Ammo
};

//--------------------------------------------------------------------------
/** Struct for one upgrade (one kind of value, e.g. hitpointsMax)
	When the hangar is made nice code, too, the sUpgradeNew and the sUpgrade have to be united, again. */
//--------------------------------------------------------------------------
struct sUpgradeNew
{
	bool active; // is this upgrade buyable for the player
	int nextPrice; // what will the next upgrade cost
	int purchased; // how many upgrades of this type has the player purchased
	int curValue; // what is the current value
	int startValue; // the value that this unit would have without all upgrades
	std::string name; // the name of this upgrade type, e.g. Ammo
};

//--------------------------------------------------------------------------
/** struct for the images and sounds */
//--------------------------------------------------------------------------
struct sBuilding
{
	SDL_Surface* img, *img_org; // Surface des Buildings
	SDL_Surface* shw, *shw_org; // Surfaces des Schattens
	SDL_Surface* eff, *eff_org; // Surfaces des Effektes
	SDL_Surface* video;  // Video
	sUnitData data;  // Grunddaten des Buildings
	int nr;              // Nr dieses Elements
	SDL_Surface* info;   // Infobild

	// Die Sounds:
	struct Mix_Chunk* Start;
	struct Mix_Chunk* Running;
	struct Mix_Chunk* Stop;
	struct Mix_Chunk* Attack;
	struct Mix_Chunk* Wait;

	void scaleSurfaces (float faktor);
};

// enum for the upgrade symbols
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed, SHits, SAmmo, SMetal, SEnergy, SShots, SOil, SGold, STrans, SHuman, SAir};
enum eSymbolsBig {SBSpeed, SBHits, SBAmmo, SBAttack, SBShots, SBRange, SBArmor, SBScan, SBMetal, SBOil, SBGold, SBEnergy, SBHuman};
#endif

//--------------------------------------------------------------------------
struct sBuildStruct
{
public:
	sBuildStruct (SDL_Surface* const sf_, const sID& ID_, int const iRemainingMetal_ = -1) :
		sf (sf_),
		ID (ID_),
		iRemainingMetal (iRemainingMetal_)
	{}

	SDL_Surface* sf;
	sID ID;
	int iRemainingMetal;
};

//--------------------------------------------------------------------------
/** struct for the building order list */
//--------------------------------------------------------------------------
struct sBuildList
{
	sID type;
	int metall_remaining;
};

//--------------------------------------------------------------------------
enum ResourceKind
{
	TYPE_METAL = 0,
	TYPE_OIL   = 1,
	TYPE_GOLD  = 2
};

//--------------------------------------------------------------------------
/** Class cBuilding for one building. */
//--------------------------------------------------------------------------
class cBuilding : public cUnit
{
public:
	cBuilding (const sBuilding* b, cPlayer* Owner, unsigned int ID);
	~cBuilding();

	cBuilding* next; ///< "next"-pointer for the double linked list
	cBuilding* prev; ///< "prev"-pointer for the double linked list
	const sBuilding* typ;  // Typ des Buildings
	int RubbleTyp;     // Typ des Drecks
	int RubbleValue;   // Wert des Drecks
	int StartUp;     // counter for the startup animation
	bool BaseN, BaseE, BaseS, BaseW; // is the building connected in this direction?
	bool BaseBN, BaseBE, BaseBS, BaseBW; // is the building connected in this direction (only for big buildings)
	struct sSubBase* SubBase;     // the subbase to which this building belongs
	int EffectAlpha; // alpha value for the effect
	bool EffectInc;  // is the effect counted upwards or dounwards?
	bool IsWorking;  // is the building currently working?
	int researchArea; ///< if the building can research, this is the area the building last researched or is researching
	int MaxMetalProd, MaxOilProd, MaxGoldProd; // the maximum possible production of the building
	std::vector<sBuildList*>* BuildList; // Die Bauliste der Fabrik
	int BuildSpeed;  // Die baugeschwindigkeit der Fabrik
	int MetalPerRound; //Die Menge an Metal, die die Fabrik bei momentaner Baugeschwindigkeit pro Runde maximal verbaut
	bool RepeatBuild; // Gibt an, ob der Bau wiederholt werden soll
	int VehicleToActivate; // Nummer des Vehicles, dass aktiviert werden soll
	int DamageFXPointX, DamageFXPointY, DamageFXPointX2, DamageFXPointY2; // the points, where smoke will be generated when the building is damaged
	int lastShots;   //A disabled unit gets this amount of shots back, when it it captured
	/** true if the building was has been working before it was disabled */
	bool wasWorking;
	bool IsLocked;   // Gibt an, ob dieses Building in irgend einer Lock-Liste ist
	int points;     // accumulated eco-sphere points

	/**
	* draws the building to the screen. It takes the main image from the drawing cache, or calls the cBuilding::render() function.
	*/
	void draw (SDL_Rect* dest, cGameGUI& gameGUI);
	void Select(cGameGUI& gameGUI);
	void Deselct(cGameGUI& gameGUI);

	int playStream();
	std::string getStatusStr (const cGameGUI& gameGUI) const;
	/**
	* refreshes the shotsCur of this building
	*@author alzi alias DoctorDeath
	*@return 1 if there has been refreshed something, else 0.
	*/
	int refreshData();
	void DrawSymbolBig (eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface* sf);
	void updateNeighbours (const cMap* map);
	void CheckNeighbours (const cMap* Map);
	void ServerStartWork (cServer& server);
	void ClientStartWork (cGameGUI& gameGUI);
	void ServerStopWork (cServer& server, bool override);
	void ClientStopWork (cGameGUI& gameGUI);
	bool CanTransferTo (const cGameGUI& gameGUI, cMapField* OverUnitField);  /** check whether a transfer to a unit on the field is possible */
	void CheckRessourceProd(const cServer& server);
	void CalcTurboBuild (int* iTurboBuildRounds, int* iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal = -1);
	void DrawExitPoints (const sVehicle* typ, cGameGUI& gameGUI);
	bool canExitTo (const int x, const int y, const cMap* map, const sVehicle* typ) const;
	bool canLoad (int x, int y, const cMap* Map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const;
	void storeVehicle (cVehicle* Vehicle, cMap* Map);
	void exitVehicleTo (cVehicle* Vehicle, int offset, cMap* Map);

	/**
	* returns whether this player has detected this unit or not
	*@author alzi alias DoctorDeath
	*@param player player for which the stauts sould be checked
	*@return true if the player has detected the unit
	*/
	bool isDetectedByPlayer (const cPlayer* player) const;
	/**
	* removes a player from the detectedByPlayerList
	*/
	void resetDetectedByPlayer (const cPlayer* player);
	/**
	* adds a player to the DetecedByPlayerList
	*/
	virtual void setDetectedByPlayer (cServer& server, cPlayer* player, bool addToDetectedInThisTurnList = true);
	/**
	* - checks whether the building has been detected by an other unit
	* the detection maps have to be up to date, when calling this funktion
	* this function has to be called on the server everytime a building is added
	*/
	void makeDetection (cServer& server);

	/**
	* draws the main image of the building onto the given surface
	*/
	void render (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete);
private:
	/**
	* draws the connectors onto the given surface
	*/
	void drawConnectors (SDL_Surface* surface, SDL_Rect dest, float zoomFactor, bool drawShadow);

	//-----------------------------------------------------------------------------
protected:
	//-- methods, that have been extracted during cUnit refactoring ---------------
	virtual bool isUnitWorking() const { return IsWorking; }
	virtual bool factoryHasJustFinishedBuilding() const;
	virtual bool buildingCanBeStarted() const;
	virtual bool buildingCanBeUpgraded() const;
	virtual bool canBeStoppedViaUnitMenu() const { return isUnitWorking(); }

	// methods needed for execution of unit menu commands
	virtual void executeBuildCommand (cGameGUI& gameGUI);
	virtual void executeMineManagerCommand (const cClient& client);
	virtual void executeStopCommand (const cClient& client);
	virtual void executeActivateStoredVehiclesCommand (cClient& client);
	virtual void executeUpdateBuildingCommmand (const cClient& client, bool updateAllOfSameType);
	virtual void executeSelfDestroyCommand (const cClient& client);

	virtual sUnitData* getUpgradedUnitData() const;
};

#endif
