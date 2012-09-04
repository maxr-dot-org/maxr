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
#ifndef clientH
#define clientH
#include "SDL_flic.h"
#include "defines.h"
#include "main.h"
#include "hud.h"

class cNetMessage;
class cMap;
class cPlayer;
class cClientAttackJob;
class cClientMoveJob;
class cCasualtiesTracker;
class cClientSpeedCtrl;

Uint32 TimerCallback (Uint32 interval, void* arg);


/** FX types */
enum eFXTyps {fxMuzzleBig, fxMuzzleSmall, fxMuzzleMed, fxMuzzleMedLong, fxExploSmall, fxExploBig, fxExploAir, fxExploWater, fxHit, fxSmoke, fxRocket, fxDarkSmoke, fxTorpedo, fxTracks, fxBubbles, fxCorpse, fxAbsorb};

/** struct for the rocked data */
struct sFXRocketInfos
{
	int ScrX, ScrY;
	int DestX, DestY;
	int dir;
	float fpx, fpy, mx, my;
	cClientAttackJob* aj;
};

/** struct for the dark smoke data */
struct sFXDarkSmoke
{
	int alpha;
	float fx, fy;
	float dx, dy;
};

/** struct for the tracks effect */
struct sFXTracks
{
	int alpha;
	int dir;
};

/** struct for an FX effect */
struct sFX
{
public:
	sFX (eFXTyps typ, int x, int y);
	~sFX();

	eFXTyps typ;
	int PosX, PosY;
	int StartTime;
	int param;
	sFXRocketInfos* rocketInfo;
	sFXDarkSmoke* smokeInfo;
	sFXTracks* trackInfo;
};

/**
* Client class which handles the in and output for a player
*@author alzi alias DoctorDeath
*/
class cClient
{
public:
	cClient (cMap* Map, cList<cPlayer*>* PlayerList);
	~cClient();

private:
	//friend class cGameGUI;
	friend class cDebugOutput;
	friend class cPlayer;
	friend class cBuilding;
	friend class cVehicle;
	friend class cUnit;

	/** the map */
	cMap* Map;
	/** List with all players */
	cList<cPlayer*>* PlayerList;
	/** the active Player */
	cPlayer* ActivePlayer;

	/** list with buildings without owner, e. g. rubble fields */
	cBuilding* neutralBuildings;
	/** number of current turn */
	int iTurn;
	/** flags what should be displaxed in the raffinery */
	bool bUpShowTank, bUpShowPlane, bUpShowShip, bUpShowBuild, bUpShowTNT; //TODO: GameGUI!
	/** true if the player has been defeated */
	bool bDefeated;
	/** how many seconds will be left for this turn */
	int iTurnTime;
	/** Ticks when the TurnTime has been started */
	unsigned int iStartTurnTime;
	/** this client's copy of the victory conditions **/
	int turnLimit, scoreLimit;

	cCasualtiesTracker* casualtiesTracker;

	/**
	* adds the unit to the map and player.
	*@author alzi alias DoctorDeath
	*@param iPosX The X were the unit should be added.
	*@param iPosY The Y were the unit should be added.
	*@param Vehicle Vehicle which should be added.
	*@param Building Building which should be added.
	*@param Player Player whose vehicle should be added.
	*@param bInit true if this is a initialisation call.
	*/
	void addUnit (int iPosX, int iPosY, cVehicle* AddedVehicle, bool bInit = false, bool bAddToMap = true);
	void addUnit (int iPosX, int iPosY, cBuilding* AddedBuilding, bool bInit = false);
	/**
	* handles the end of a hotseat game
	*@author alzi alias DoctorDeath
	*@param iNextPlayerNum Number of Player who has ended his turn
	*/
	void makeHotSeatEnd (int iNextPlayerNum);
	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs();
	/**
	* gets the subbase with the id
	*@author alzi alias DoctorDeath
	*@param iID Id of the subbase
	*/
	sSubBase* getSubBaseFromID (int iID);
	/**
	* freezes the client so that no input of him is possible anymore.
	*@author alzi alias DoctorDeath
	*/
	void freeze();
	/**
	* unfreezes the client.
	*@author alzi alias DoctorDeath
	*/
	void unfreeze();


	void HandleNetMessage_TCP_CLOSE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CHAT_SERVER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_PLAYER_CLANS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_ENEM_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_ENEM_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WAIT_FOR (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MAKE_TURNEND (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FINISHED_TURN (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNIT_DATA (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DO_START_WORK (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DO_STOP_WORK (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NEXT_MOVE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ATTACKJOB_LOCK_TARGET (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ATTACKJOB_FIRE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ATTACKJOB_IMPACT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RESOURCES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_BUILD_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STOP_BUILD (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SUBBASE_VALUES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_BUILDLIST (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MINE_PRODUCE_VALUES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_TURN_REPORT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MARK_LOG (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SUPPLY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_RUBBLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DETECTION_STATE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CLEAR_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STOP_CLEARING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NOFOG (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEFEATED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FREEZE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNFREEZE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WAIT_RECON (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ABORT_WAIT_RECON (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_PLAYER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_TURN (cNetMessage& message);
	void HandleNetMessage_GAME_EV_HUD_SETTINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STORE_UNIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_EXIT_UNIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DELETE_EVERYTHING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CREDITS_CHANGED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RESEARCH_LEVEL (cNetMessage& message);
	void HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SET_AUTOMOVE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_COMMANDO_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_REQ_SAVE_INFO (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SAVED_REPORT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CASUALTIES_REPORT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SCORE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NUM_ECOS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNIT_SCORE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_VICTORY_CONDITIONS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SELFDESTROY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (cNetMessage& message);

public:
	cGameTimer gameTimer; //TODO: private
	/**  the soundstream of the selected unit */
	int iObjectStream;
	/** lists with all FX-Animation */
	cList<sFX*> FXList;
	cList<sFX*> FXListBottom;
	/** list with the running clientAttackJobs */
	cList<cClientAttackJob*> attackJobs;
	/** List with all active movejobs */
	cList<cClientMoveJob*> ActiveMJobs;
	/** the hud */
	cGameGUI gameGUI;
	/** true if the turn should be end after all movejobs have been finished */
	bool bWantToEnd;
	/** true if allian technologies are activated */
	bool bAlienTech;
	
	/** shows if the player has to wait for other players */
	bool bWaitForOthers;
	bool waitReconnect;
	/**
	* handles the end of a turn
	*@author alzi alias DoctorDeath
	*/
	void handleEnd();

	/**
	* handles the game relevant actions (for example moving the current position of a rocket)
	* of the fx-effects, so that they are handled also, when the effects are not drawn.
	*/
	void runFX();

	/**
	* handles the rest-time of the current turn
	*@author alzi alias DoctorDeath
	*/
	void handleTurnTime();

	void calcNextGameTimeTick();

	/**
	* creates a new moveJob an transmits it to the server
	* @param vehicle the vehicle to be moved
	* @param iDestOffset the Destination
	*/
	int addMoveJob (cVehicle* vehicle, int DestX, int DestY, cList<cVehicle*>* group = NULL);
	void startGroupMove();
	/**
	* adds an new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob (cClientMoveJob* MJob);
	/**
	* returns the player with the given number
	*@author alzi alias DoctorDeath
	*@param iNum The number of the player.
	*@return The wanted player.
	*/
	cPlayer* getPlayerFromNumber (int iNum);
	/**
	* deletes the unit
	*@author alzi alias DoctorDeath
	*@param Building Building which should be deleted.
	*@param Vehicle Vehicle which should be deleted.
	*/
	void deleteUnit (cBuilding* Building);
	void deleteUnit (cVehicle* Vehicle);
	/**
	* sends the netMessage to the server.
	* do not try to delete a message after calling this function!
	*@author Eiko
	*@param message The netMessage to be send.
	*/
	void sendNetMessage (cNetMessage* message);
	/**
	* gets the vehicle with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the vehicle
	*/
	cVehicle* getVehicleFromID (unsigned int iID);
	cBuilding* getBuildingFromID (unsigned int iID);

	/**
	* initialises this client for the player.
	*@author alzi alias DoctorDeath
	*@param Player The player.
	*/
	void initPlayer (cPlayer* Player);
	
	/**
	* handles move and attack jobs
	* this function should be called in all menu loops
	*/
	void doGameActions();

	/**
	* processes everything that is need for this netMessage
	*@author alzi alias DoctorDeath
	*@param message The netMessage to be handled.
	*@return 0 for success
	*/
	int HandleNetMessage (cNetMessage* message);
	/**
	* adds an effect
	*@author alzi alias DoctorDeath
	*@param typ typ of the effect
	*@param iX X coordinate were the effect should be added
	*@param iY Y coordinate were the effect should be added
	*@param iParam
	*@param param
	*@param iNum
	*/
	void addFX (eFXTyps typ, int iX, int iY, int iParam);
	void addFX (eFXTyps typ, int x, int y, cClientAttackJob* aj, int iDestOff, int iFireDir);
	void addFX (sFX* iNum);

	/**
	*destroys a unit
	*play FX, add rubble and delete Unit
	*/
	void destroyUnit (cVehicle* vehicle);
	void destroyUnit (cBuilding* building);

	void getVictoryConditions (int* turnLimit, int* scoreLimit) const;
	int getTurn() const;

	void deletePlayer (cPlayer* player);

	cCasualtiesTracker* getCasualties() {return casualtiesTracker;}
	cMap* getMap() { return Map; }
	cList<cPlayer*>* getPlayerList() { return PlayerList; }
	cPlayer* getActivePlayer() { return ActivePlayer; };

};

extern cClient* Client;

#endif
