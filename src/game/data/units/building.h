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

#ifndef game_data_units_buildingH
#define game_data_units_buildingH

#include <vector>
#include <array>

#include <SDL.h>

#include "defines.h"
#include "maxrconfig.h"
#include "main.h" // for sUnitData, sID
#include "game/data/units/unit.h"
#include "sound.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "game/logic/upgradecalculator.h" // cResearch::ResearchArea

class cBase;
class cPlayer;
class cVehicle;
class cMap;
class cMapField;
class cServer;

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
struct sBuildingUIData
{
	AutoSurface img, img_org; // Surface of the building
    AutoSurface shw, shw_org; // Surfaces of the shadow
    AutoSurface eff, eff_org; // Surfaces of the effects
    AutoSurface video;  // video
    AutoSurface info;   // info image

	// Die Sounds:
	cSoundChunk Start;
    cSoundChunk Running;
    cSoundChunk Stop;
    cSoundChunk Attack;
    cSoundChunk Wait;

    sBuildingUIData ();
    sBuildingUIData (sBuildingUIData&& other);
    sBuildingUIData& operator=(sBuildingUIData&& other);
	void scaleSurfaces (float faktor);

private:
    sBuildingUIData (const sBuildingUIData& other) MAXR_DELETE_FUNCTION;
    sBuildingUIData& operator=(const sBuildingUIData& other) MAXR_DELETE_FUNCTION;
};

// enum for the upgrade symbols
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed, SHits, SAmmo, SMetal, SEnergy, SShots, SOil, SGold, STrans, SHuman, SAir};
enum eSymbolsBig {SBSpeed, SBHits, SBAmmo, SBAttack, SBShots, SBRange, SBArmor, SBScan, SBMetal, SBOil, SBGold, SBEnergy, SBHuman};
#endif

//--------------------------------------------------------------------------
/** struct for the building order list */
//--------------------------------------------------------------------------
class cBuildListItem
{
public:
	cBuildListItem ();
	cBuildListItem (sID type, int remainingMetal);
	cBuildListItem (const cBuildListItem& other);
	cBuildListItem (cBuildListItem&& other);

	cBuildListItem& operator=(const cBuildListItem& other);
	cBuildListItem& operator=(cBuildListItem&& other);

	const sID& getType () const;
	void setType (const sID& type);

	int getRemainingMetal () const;
	void setRemainingMetal (int value);

	cSignal<void ()> typeChanged;
	cSignal<void ()> remainingMetalChanged;
private:
	sID type;
	int remainingMetal;
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
	cBuilding (const sUnitData* b, cPlayer* Owner, unsigned int ID);
	virtual ~cBuilding();

	virtual bool isAVehicle() const { return false; }
	virtual bool isABuilding() const { return true; }

	const sBuildingUIData* uiData;
	mutable int effectAlpha; // alpha value for the effect

	int RubbleTyp;     // Typ des Drecks
	int RubbleValue;   // Wert des Drecks
	bool BaseN, BaseE, BaseS, BaseW; // is the building connected in this direction?
	bool BaseBN, BaseBE, BaseBS, BaseBW; // is the building connected in this direction (only for big buildings)
	struct sSubBase* SubBase;     // the subbase to which this building belongs
	int MaxMetalProd, MaxOilProd, MaxGoldProd; // the maximum possible production of the building
	int BuildSpeed;  // Die baugeschwindigkeit der Fabrik
	int MetalPerRound; //Die Menge an Metal, die die Fabrik bei momentaner Baugeschwindigkeit pro Runde maximal verbaut
	bool RepeatBuild; // Gibt an, ob der Bau wiederholt werden soll
	int DamageFXPointX, DamageFXPointY, DamageFXPointX2, DamageFXPointY2; // the points, where smoke will be generated when the building is damaged
	/** true if the building was has been working before it was disabled */
	bool wasWorking;
	int points;     // accumulated eco-sphere points

	int playStream();
	virtual std::string getStatusStr (const cPlayer* player) const MAXR_OVERRIDE_FUNCTION;

	virtual void makeReport (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;

	/**
	* refreshes the shotsCur of this building
	*@author alzi alias DoctorDeath
	*@return 1 if there has been refreshed something, else 0.
	*/
	bool refreshData();
	void DrawSymbolBig (eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface* sf);
	void updateNeighbours (const cMap& map);
	void CheckNeighbours (const cMap& Map);
	void ServerStartWork (cServer& server);
	void clientStartWork ();
	void ServerStopWork (cServer& server, bool override);
	void clientStopWork ();
	/** check whether a transfer to a unit on the field is possible */
	virtual bool canTransferTo (const cPosition& position, const cMapField& overUnitField) const MAXR_OVERRIDE_FUNCTION;
	void CheckRessourceProd (const cServer& server);
	void calcTurboBuild (std::array<int, 3>& turboBuildRounds, std::array<int, 3>& turboBuildCosts, int vehicleCosts, int remainingMetal = -1) const;
	virtual bool canExitTo (const cPosition& position, const cMap& map, const sUnitData& unitData) const MAXR_OVERRIDE_FUNCTION;
	bool canLoad (const cPosition& position, const cMap& map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const;
	void storeVehicle (cVehicle& vehicle, cMap& map);
	void exitVehicleTo (cVehicle& vehicle, const cPosition& position, cMap& map);

	/**
	* returns whether this player has detected this unit or not
	*@author alzi alias DoctorDeath
	*@param player player for which the status should be checked
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
	* the detection maps have to be up to date, when calling this function
	* this function has to be called on the server
	* every time a building is added
	*/
	void makeDetection (cServer& server);

	/**
	* draws the main image of the building onto the given surface
	*/
	void render (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete) const;
	void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int animationTime = 0, int alpha = 254) const;
	static void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, const sUnitData& data, const sBuildingUIData& uiData, const cPlayer* owner, int frameNr = 0, int alpha = 254);

	void executeUpdateBuildingCommmand (const cClient& client, bool updateAllOfSameType) const;

	virtual bool isUnitWorking () const { return isWorking; }
	virtual bool factoryHasJustFinishedBuilding () const;
	virtual bool buildingCanBeStarted () const;
	virtual bool buildingCanBeUpgraded () const;
	virtual bool canBeStoppedViaUnitMenu () const { return isUnitWorking (); }

	bool isBuildListEmpty () const;
	size_t getBuildListSize () const;
	const cBuildListItem& getBuildListItem (size_t index) const;
	cBuildListItem& getBuildListItem (size_t index);
	void setBuildList (std::vector<cBuildListItem> buildList);
	void addBuildListItem (cBuildListItem item);
	void removeBuildListItem (size_t index);

	void setWorking (bool value);

	void setResearchArea (cResearch::ResearchArea area);
	cResearch::ResearchArea getResearchArea () const;

	cSignal<void ()> buildListChanged;
	cSignal<void ()> buildListFirstItemDataChanged;
	cSignal<void ()> researchAreaChanged;
private:
	cSignalConnectionManager buildListFirstItemSignalConnectionManager;
	cSignalConnectionManager ownerSignalConnectionManager;

	/**
	* draws the connectors onto the given surface
	*/
	void drawConnectors (SDL_Surface* surface, SDL_Rect dest, float zoomFactor, bool drawShadow) const;

	void render_rubble (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_beton (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const;

	bool isWorking;  // is the building currently working?

	cResearch::ResearchArea researchArea; ///< if the building can research, this is the area the building last researched or is researching

	std::vector<cBuildListItem> buildList; // list with the units to be build by this factory

	void registerOwnerEvents ();

	//-----------------------------------------------------------------------------
protected:
	//-- methods, that have been extracted during cUnit refactoring ---------------

	// methods needed for execution of unit menu commands
	virtual void executeStopCommand (const cClient& client) const MAXR_OVERRIDE_FUNCTION;
};

#endif // game_data_units_buildingH
