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
#ifndef vehiclesH
#define vehiclesH
#include "defines.h"
#include "maxrconfig.h"
#include <SDL.h>
#include <vector>
#include <array>

#include "main.h" // for sUnitData
#include "unit.h"
#include "sound.h"

class cAutoMJob;
class cBuilding;
class cClientMoveJob;
class cMap;
class cStaticMap;
class cMapField;
class cPlayer;
class cServer;
class cServerMoveJob;
class cApplication;
class cSoundManager;

//-----------------------------------------------------------------------------
// Enum for the symbols
//-----------------------------------------------------------------------------
#ifndef D_eSymbols
#define D_eSymbols

//-----------------------------------------------------------------------------
enum eSymbols
{
	SSpeed,
	SHits,
	SAmmo,
	SMetal,
	SEnergy,
	SShots,
	SOil,
	SGold,
	STrans,
	SHuman,
	SAir
};

//-----------------------------------------------------------------------------
enum eSymbolsBig
{
	SBSpeed,
	SBHits,
	SBAmmo,
	SBAttack,
	SBShots,
	SBRange,
	SBArmor,
	SBScan,
	SBMetal,
	SBOil,
	SBGold,
	SBEnergy,
	SBHuman
};

#endif

//-----------------------------------------------------------------------------
// Struct for the pictures and sounds
//-----------------------------------------------------------------------------
struct sVehicleUIData
{
	std::array<AutoSurface, 8> img, img_org; // 8 Surfaces of the vehicle
    std::array<AutoSurface, 8> shw, shw_org; // 8 Surfaces of shadows
    AutoSurface build, build_org;        // Surfaces when building
    AutoSurface build_shw, build_shw_org; // Surfaces of shadows when building
    AutoSurface clear_small, clear_small_org;        // Surfaces when clearing
    AutoSurface clear_small_shw, clear_small_shw_org; // Surfaces when clearing
    AutoSurface overlay, overlay_org;    // Overlays
    AutoSurface storage; // image of the vehicle in storage
	std::string FLCFile;       // FLC-Video
    AutoSurface info;   // info image

	// Sounds:
	cSoundChunk Wait;
    cSoundChunk WaitWater;
    cSoundChunk Start;
    cSoundChunk StartWater;
    cSoundChunk Stop;
    cSoundChunk StopWater;
    cSoundChunk Drive;
    cSoundChunk DriveWater;
    cSoundChunk Attack;

    sVehicleUIData ();
    sVehicleUIData (sVehicleUIData&& other);
    sVehicleUIData& operator=(sVehicleUIData&& other);
    void scaleSurfaces (float faktor);

private:
    sVehicleUIData (const sVehicleUIData& other) MAXR_DELETE_FUNCTION;
    sVehicleUIData& operator=(const sVehicleUIData& other) MAXR_DELETE_FUNCTION;
};

//-----------------------------------------------------------------------------
/** Class for a vehicle-unit of a player */
//-----------------------------------------------------------------------------
class cVehicle : public cUnit
{
	//-----------------------------------------------------------------------------
public:
	cVehicle (const sUnitData& unitData, cPlayer* Owner, unsigned int ID);
	virtual ~cVehicle();

	virtual bool isAVehicle() const { return true; }
	virtual bool isABuilding() const { return false; }

	virtual const cPosition& getMovementOffset() const MAXR_OVERRIDE_FUNCTION { return tileMovementOffset; }
	void setMovementOffset(const cPosition& newOffset) { tileMovementOffset = newOffset; }

	const sVehicleUIData* uiData;
	cServerMoveJob* ServerMoveJob;
	bool hasAutoMoveJob; // this is just a status information for the server, so that he can write the information to the saves
	bool MoveJobActive; // Gibt an, ob der MoveJob gerade ausgeführt wird
	int ditherX, ditherY; // Dithering für Flugzeuge
	cPosition bandPosition; // X,Y Position für das Band
	cPosition buildBigSavedPosition; // last position before building has started
	bool BuildPath;   // Gibt an, ob ein Pfad gebaut werden soll
	unsigned int BigBetonAlpha; // AlphaWert des großen Betons
	int StartUp;      // Zähler für die Startupannimation
	int DamageFXPointX, DamageFXPointY; // Die Punkte, an denen Rauch bei beschädigung aufsteigen wird
	unsigned int WalkFrame; // Frame der Geh-Annimation
	int lastSpeed;	 //A disabled unit gets this amount of speed back, when it it captured
	int lastShots;	 //A disabled unit gets this amount of shots back, when it it captured

	/**
	* refreshes speedCur and shotsCur and continues building or clearing
	*@author alzi alias DoctorDeath
	*@return true if there has been refreshed something else false.
	*/
	bool refreshData();
	bool refreshData_Build (cServer& server);
	bool refreshData_Clear (cServer& server);

	virtual std::string getStatusStr (const cPlayer* player) const MAXR_OVERRIDE_FUNCTION;
	void DecSpeed (int value);
	void doSurvey (const cServer& server);
	void makeReport (cSoundManager& soundManager);
	virtual bool canTransferTo (const cPosition& position, const cMapField& overUnitField) const MAXR_OVERRIDE_FUNCTION;
	bool InSentryRange (cServer& server);
	virtual bool canExitTo (const cPosition& position, const cMap& map, const sUnitData& unitData) const MAXR_OVERRIDE_FUNCTION;
	bool canLoad (const cPosition& position, const cMap& map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const;
	void storeVehicle (cVehicle& vehicle, cMap& map);
	void exitVehicleTo (cVehicle& vehicle, const cPosition& position, cMap& map);
#define SUPPLY_TYPE_REARM 0
#define SUPPLY_TYPE_REPAIR 1
	/// supplyType: one of SUPPLY_TYPE_REARM and SUPPLY_TYPE_REPAIR
	bool canSupply (const cMap& map, const cPosition& position, int supplyType) const;
	/// supplyType: one of SUPPLY_TYPE_REARM and SUPPLY_TYPE_REPAIR
	bool canSupply (const cUnit* unit, int supplyType) const;
	void calcTurboBuild (std::array<int, 3>& turboBuildTurns, std::array<int, 3>& turboBuildCosts, int buildCosts) const;
	/**
	* lays a mine. Should only be called by the server!
	*@author alzi alias DoctorDeath
	*@return true if a mine has been laid
	*/
	bool layMine (cServer& server);
	/**
	* clears a field from a mine. Should only be called by the server!
	*@author alzi alias DoctorDeath
	*@return true if there was a mine to be cleared
	*/
	bool clearMine (cServer& server);
	/**
	* checks whether the commando action can be performed or not
	*@author alzi alias DoctorDeath
	*/
	bool canDoCommandoAction (const cPosition& position, const cMap& map, bool steal) const;
	bool canDoCommandoAction (const cUnit* unit, bool steal) const;
	/**
	* calculates the chance for disabling or stealing the target unit
	*@author alzi alias DoctorDeath
	*/
	int calcCommandoChance (const cUnit* destUnit, bool steal) const;
	int calcCommandoTurns (const cUnit* destUnit) const;
	/**
	* returns whether this player has detected this unit or not
	*@author alzi alias DoctorDeath
	*@param iPlayerNum number of player for which the status should be checked
	*@return true if the player has detected the unit
	*/
	bool isDetectedByPlayer (const cPlayer* player) const;
	/**
	* removes a player from the detectedByPlayerList
	*/
	void resetDetectedByPlayer (cServer& server, cPlayer* player);
	/**
	* adds a player to the DetecedByPlayerList
	*/
	virtual void setDetectedByPlayer (cServer& server, cPlayer* player, bool addToDetectedInThisTurnList = true);
	/**
	* - detects stealth units in the scan range of the vehicle
	* - checks whether the vehicle has been detected by an other unit
	* the detection maps have to be up to date,
	* when calling this function,
	* this function has to be called on the server
	* every time a unit was moved, builded, unloaded...
	*/
	void makeDetection (cServer& server);

	/** After a movement the detection state of a unit might be reset,
	 * if it was not detected in _this_ turn. */
	void tryResetOfDetectionStateAfterMove (cServer& server);
	/** Resets the list of players, that detected this unit in this turn
	 * (is called at turn end). */
	void clearDetectedInThisTurnPlayerList();
	bool wasDetectedInThisTurnByPlayer (const cPlayer* player) const;

	void blitWithPreScale (SDL_Surface* org_src, SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect, float factor, int frames = 1) const;

	void executeAutoMoveJobCommand (cClient& client);
	void executeLayMinesCommand (const cClient& client);
	void executeClearMinesCommand (const cClient& client);

	/**
	* Is this a plane and is there a landing platform beneath it,
	* that can be used to land on?
	* @author: eiko
	*/
	bool canLand (const cMap& map) const;


	/**
	* draws the main image of the vehicle onto the passed surface
	*/
	void render (const cMap* map, unsigned long long animationTime, const cPlayer* activePlayer, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int alpha = 254) const;
	/**
	* draws the overlay animation of the vehicle on the given surface
	*@author: eiko
	*/
	void drawOverlayAnimation (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const;
	void drawOverlayAnimation (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha = 254) const;

	bool isUnitLoaded () const { return loaded; }

	virtual bool isUnitMoving () const { return moving; }
	virtual bool isAutoMoveJobActive () const { return autoMoveJob != nullptr; }
	virtual bool isUnitClearing () const { return isClearing; }
	virtual bool isUnitLayingMines () const { return layMines; }
	virtual bool isUnitClearingMines () const { return clearMines; }
	virtual bool isUnitBuildingABuilding () const { return isBuilding; }
	virtual bool canBeStoppedViaUnitMenu () const;

	void setMoving (bool value);
	void setLoaded (bool value);
	void setClearing (bool value);
	void setBuildingABuilding (bool value);
	void setLayMines (bool value);
	void setClearMines (bool value);

	int getClearingTurns () const;
	void setClearingTurns (int value);

	float getCommandoRank () const;
	void setCommandoRank (float value);

	const sID& getBuildingType () const;
	void setBuildingType (const sID& id);
	int getBuildCosts () const;
	void setBuildCosts (int value);
	int getBuildTurns () const;
	void setBuildTurns (int value);
	int getBuildCostsStart () const;
	void setBuildCostsStart (int value);
	int getBuildTurnsStart () const;
	void setBuildTurnsStart (int value);

	int getFlightHeight () const;
	void setFlightHeight (int value);

	cClientMoveJob* getClientMoveJob ();
	const cClientMoveJob* getClientMoveJob () const;
	void setClientMoveJob (cClientMoveJob* clientMoveJob);

	cAutoMJob* getAutoMoveJob ();
	const cAutoMJob* getAutoMoveJob () const;
	void startAutoMoveJob (cClient& client);
	void stopAutoMoveJob ();

	/**
	* return the unit which contains this vehicle
	*/
	cBuilding* getContainerBuilding();
	cVehicle* getContainerVehicle();

	mutable cSignal<void ()> clearingTurnsChanged;
	mutable cSignal<void ()> buildingTurnsChanged;
	mutable cSignal<void ()> buildingCostsChanged;
	mutable cSignal<void ()> buildingTypeChanged;
	mutable cSignal<void ()> commandoRankChanged;
	mutable cSignal<void ()> flightHeightChanged;

	mutable cSignal<void ()> clientMoveJobChanged;
	mutable cSignal<void ()> autoMoveJobChanged;
private:

	void render_BuildingOrBigClearing (const cMap& map, unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_smallClearing (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_shadow (const cStaticMap& map, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const;

	//---- sentry and reaction fire helpers ------------------------------------
	/**
	 * Is called after a unit moved one field;
	 * it allows opponent units to react to that movement and
	 * fire on the moving vehicle, if they can.
	 * An opponent unit only fires as reaction to the movement,
	 * if the moving unit is an "offense" for that opponent
	 * (i.e. it could attack a unit/building of the opponent).
	 * @author: pagra
	 */
	bool provokeReactionFire (cServer& server);
	bool doesPlayerWantToFireOnThisVehicleAsReactionFire (cServer& server, const cPlayer* player) const;
	bool makeAttackOnThis (cServer& server, cUnit* opponentUnit, const std::string& reasonForLog) const;
	bool makeSentryAttack (cServer& server, cUnit* unit) const;
	bool isOtherUnitOffendedByThis (cServer& server, const cUnit& otherUnit) const;
	bool doReactionFire (cServer& server, cPlayer* player) const;
	bool doReactionFireForUnit (cServer& server, cUnit* opponentUnit) const;

	/// helper method that returns a list of all players,
	/// that can detect this unit
	std::vector<cPlayer*> calcDetectedByPlayer (cServer& server) const;
	/// list of players, that detected this vehicle in this turn
	std::vector<cPlayer*> detectedInThisTurnByPlayerList;

	cPosition tileMovementOffset;  // offset within tile during movement

	cClientMoveJob* clientMoveJob;
	std::shared_ptr<cAutoMJob> autoMoveJob; //the auto move AI of the vehicle

	bool loaded;
	bool moving;

	bool isBuilding;
	sID buildingTyp;
	int buildCosts;
	int buildTurns;
	int buildTurnsStart;
	int buildCostsStart;

	bool isClearing;
	int clearingTurns;

	bool layMines;
	bool clearMines;

	int flightHeight;

	float commandoRank;

	//--------------------------------------------------------------------------
protected:
	//-- methods, that have been extracted during cUnit refactoring ------------


	// methods needed for execution of unit menu commands
	virtual void executeStopCommand (const cClient& client) const MAXR_OVERRIDE_FUNCTION;
};

#endif
