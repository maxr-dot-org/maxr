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
#include <SDL.h>

#include "main.h" // for sUnitData
#include "unit.h"
#include <vector>

class cAutoMJob;
class cBuilding;
class cClientMoveJob;
class cGameGUI;
class cMap;
class cMapField;
class cPlayer;
class cServer;
class cServerMoveJob;

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
struct sVehicle
{
	SDL_Surface* img[8], *img_org[8]; // 8 Surfaces des Vehicles
	SDL_Surface* shw[8], *shw_org[8]; // 8 Surfaces des Schattens
	SDL_Surface* build, *build_org;        // Surfaces beim Bauen
	SDL_Surface* build_shw, *build_shw_org; // Surfaces beim Bauen (Schatten)
	SDL_Surface* clear_small, *clear_small_org;        // Surfaces beim Clearen (die große wird in build geladen)
	SDL_Surface* clear_small_shw, *clear_small_shw_org; // Surfaces beim Clearen (Schatten) (die große wird in build geladen)
	SDL_Surface* overlay, *overlay_org;    // Overlays
	SDL_Surface* storage; // Bild des Vehicles im Lager
	char* FLCFile;       // FLC-Video
	sUnitData data;   // Grunddaten des Vehicles
	int nr;              // Nr dieses Elements
	SDL_Surface* info;   // Infobild

	// Die Sounds:
	struct Mix_Chunk* Wait;
	struct Mix_Chunk* WaitWater;
	struct Mix_Chunk* Start;
	struct Mix_Chunk* StartWater;
	struct Mix_Chunk* Stop;
	struct Mix_Chunk* StopWater;
	struct Mix_Chunk* Drive;
	struct Mix_Chunk* DriveWater;
	struct Mix_Chunk* Attack;

	sVehicle();
	void scaleSurfaces (float factor);
};


//-----------------------------------------------------------------------------
/** Class for a vehicle-unit of a player */
//-----------------------------------------------------------------------------
class cVehicle : public cUnit
{
	//-----------------------------------------------------------------------------
public:
	cVehicle (const sVehicle& v, cPlayer* Owner, unsigned int ID);
	virtual ~cVehicle();

	virtual bool isAVehicle() const { return true; }
	virtual bool isABuilding() const { return false; }

	cVehicle* next; ///< "next"-pointer for the double linked list
	cVehicle* prev; ///< "prev"-pointer for the double linked list
	int OffX, OffY;  // Offset während der Bewegung
	virtual int getMovementOffsetX() const {return OffX;}
	virtual int getMovementOffsetY() const {return OffY;}

	const sVehicle* typ;   // Typ des Vehicles
	bool groupSelected;
	cServerMoveJob* ServerMoveJob;
	cClientMoveJob* ClientMoveJob;
	cAutoMJob* autoMJob; //the auto move AI of the vehicle
	bool hasAutoMoveJob; // this is just a status information for the server, so that he can write the information to the saves
	bool moving;     // Gibt an, ob sich das Vehicle grade bewegt
	bool MoveJobActive; // Gibt an, ob der MoveJob gerade ausgeführt wird
	int ditherX, ditherY; // Dithering für Flugzeuge
	bool IsBuilding;  // Gibt an ob was gebaut wird
	sID BuildingTyp;  // Gibt an, was gebaut wird
	int BuildCosts;   // Die verbleibenden Baukosten
	int BuildRounds;  // Die verbleibenden Baurunden
	int BuildRoundsStart; // Startwert der Baurunden (fürs Pfadbauen)
	int BuildCostsStart;  // Startwert der Baukosten (fürs Pfadbauen)
	int BandX, BandY; // X,Y Position für das Band
	int BuildBigSavedPos; // Letzte Position vor dem Baubeginn
	bool BuildPath;   // Gibt an, ob ein Pfad gebaut werden soll
	bool IsClearing;  // Gibt an, ob einn Feld geräumt wird
	int ClearingRounds; // Gibt an, wie lange ein Feld noch geräumt wird
	unsigned int BigBetonAlpha; // AlphaWert des großen Betons
	int StartUp;      // Zähler für die Startupannimation
	int FlightHigh;   // Die Flughöhe des Flugzeugs
	bool LayMines;    // Gibt an, ob Minen gelegt werden sollen
	bool ClearMines;  // Gibt an, ob Minen geräumt werden sollen
	bool Loaded;      // Gibt an, ob das Vehicle geladen wurde
	int DamageFXPointX, DamageFXPointY; // Die Punkte, an denen Rauch bei beschädigung aufsteigen wird
	unsigned int WalkFrame; // Frame der Geh-Annimation
	float CommandoRank; // Rang des Commandos
	int lastSpeed;	 //A disabled unit gets this amount of speed back, when it it captured
	int lastShots;	 //A disabled unit gets this amount of shots back, when it it captured

	/**
	* Draws the vehicle to the screen buffer.
	* Takes the main image from the cache or calls cVehicle::render()
	*/
	void draw (SDL_Rect screenPosition, cGameGUI& gameGUI);
	void Select (cGameGUI& gameGUI);

	/**
	* refreshes speedCur and shotsCur and continues building or clearing
	*@author alzi alias DoctorDeath
	*@return true if there has been refreshed something else false.
	*/
	bool refreshData();
	bool refreshData_Build (cServer& server);
	bool refreshData_Clear (cServer& server);

	void DrawPath (cGameGUI& gameGUI);
	virtual std::string getStatusStr (const cGameGUI& gameGUI) const;
	void DecSpeed (int value);
	void FindNextband (cGameGUI& gameGUI);
	void doSurvey (const cServer& server);
	void MakeReport (cGameGUI& gameGUI);
	virtual bool CanTransferTo (int x, int y, cMapField* OverUnitField) const;
	bool InSentryRange (cServer& server);
	void DrawExitPoints (const sVehicle* typ, cGameGUI& gameGUI) const;
	bool canExitTo (const int x, const int y, const cMap* map, const sVehicle* typ) const;
	bool canLoad (int x, int y, const cMap* Map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const;
	void storeVehicle (cVehicle* Vehicle, cMap* Map);
	void exitVehicleTo (cVehicle* Vehicle, int offset, cMap* Map);
#define SUPPLY_TYPE_REARM	0
#define SUPPLY_TYPE_REPAIR	1
	bool canSupply (const cClient& client, int x, int y, int supplyType) const;  ///< supplyType: one of SUPPLY_TYPE_REARM and SUPPLY_TYPE_REPAIR
	bool canSupply (const cUnit* unit, int supplyType) const;  ///< supplyType: one of SUPPLY_TYPE_REARM and SUPPLY_TYPE_REPAIR
	void calcTurboBuild (int (&iTurboBuildRounds)[3], int (&iTurboBuildCosts)[3], int iBuild_Costs);
	/**
	* lays a mine. Should only be called by the server!
	*@author alzi alias DoctorDeath
	*@return true if a mine has been layed
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
	bool canDoCommandoAction (int x, int y, const cMap* map, bool steal) const;
	/**
	* draws the commando curser for stealing or disabling with the calculated chance
	*@author alzi alias DoctorDeath
	*/
	void drawCommandoCursor (cGameGUI& gameGUI, int x, int y, bool steal) const;
	/**
	* calculates the chance for disabling or stealing the target unit
	*@author alzi alias DoctorDeath
	*/
	int calcCommandoChance (const cUnit* destUnit, bool steal) const;
	int calcCommandoTurns (const cUnit* destUnit) const;
	/**
	* returns whether this player has detected this unit or not
	*@author alzi alias DoctorDeath
	*@param iPlayerNum number of player for which the stauts sould be checked
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
	* the detection maps have to be up to date, when calling this funktion
	* this function has to be called on the server everytime a unit was moved, builded, unloaded...
	*/
	void makeDetection (cServer& server);

	/** After a movement the detection state of a unit might be reset, if it was not detected in _this_ turn. */
	void tryResetOfDetectionStateAfterMove (cServer& server);
	/** Resets the list of players, that detected this unit in this turn (is called at turn end). */
	void clearDetectedInThisTurnPlayerList();
	bool wasDetectedInThisTurnByPlayer (const cPlayer* player) const;

	void blitWithPreScale (SDL_Surface* org_src, SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect, float factor, int frames = 1);

	// methods needed for execution of unit menu commands - refactored during cUnit-refactoring
	virtual void executeAutoMoveJobCommand (cClient& client);
	virtual void executeLayMinesCommand (const cClient& client);
	virtual void executeClearMinesCommand (const cClient& client);

	/**
	* Is this a plane and is there a landing platform beneath it, that can be used to land on?
	* @author: eiko
	*/
	bool canLand (const cMap& map) const;


	/**
	* draws the main image of the vehicle onto the passed surface
	*/
	void render (const cClient* client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow);
	void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int alpha = 255);
	/**
	* draws the overlay animation of the vehicle on the given surface
	*@author: eiko
	*/
	void drawOverlayAnimation (const cClient* client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor);
	void drawOverlayAnimation (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha = 255);

	bool isUnitLoaded() const { return Loaded; }
	/**
	* return the unit which contains this vehicle
	*/
	cBuilding* getContainerBuilding();
	cVehicle* getContainerVehicle();

private:
	void drawPath_BuildPath (cGameGUI& gameGUI);

	void render_BuildingOrBigClearing (const cClient& client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow);
	void render_smallClearing (const cClient& client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow);
	void render_shadow (const cClient& client, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor);

	//---- sentry and reaction fire helpers ---------------------------------------
	/**
	 * Is called after a unit moved one field; it allows opponent units to react to that movement and fire on the moving vehicle, if they can.
	 * An opponent unit only fires as reaction to the movement, if the moving unit is an "offense" for that opponent (i.e. it could attack a unit/building of the opponent).
	 * @author: pagra
	 */
	bool provokeReactionFire (cServer& server);
	bool doesPlayerWantToFireOnThisVehicleAsReactionFire (cServer& server, const cPlayer* player) const;
	bool makeAttackOnThis (cServer& server, cUnit* opponentUnit, const std::string& reasonForLog) const;
	bool makeSentryAttack (cServer& server, cUnit* unit) const;
	bool isOtherUnitOffendedByThis (cServer& server, const cUnit& otherUnit) const;
	bool doReactionFire (cServer& server, cPlayer* player) const;
	bool doReactionFireForUnit (cServer& server, cUnit* opponentUnit) const;

	std::vector<cPlayer*> calcDetectedByPlayer (cServer& server) const;  ///< helper method that returns a list of all players, that can detect this unit
	std::vector<cPlayer*> detectedInThisTurnByPlayerList; ///< list of players, that detected this vehicle in this turn

	//-----------------------------------------------------------------------------
protected:
	//-- methods, that have been extracted during cUnit refactoring ---------------

	virtual bool isUnitMoving() const { return moving; }
	virtual bool isAutoMoveJobActive() const { return autoMJob != 0; }
	virtual bool isUnitClearing() const { return IsClearing; }
	virtual bool isUnitLayingMines() const { return LayMines; }
	virtual bool isUnitClearingMines() const { return ClearMines; }
	virtual bool isUnitBuildingABuilding() const { return IsBuilding; }
	virtual bool canBeStoppedViaUnitMenu() const;

	// methods needed for execution of unit menu commands
	virtual void executeBuildCommand (cGameGUI& gameGUI);
	virtual void executeStopCommand (const cClient& client);
	virtual void executeActivateStoredVehiclesCommand (cClient& client);

	virtual const sUnitData* getUpgradedUnitData() const;
};

#endif
