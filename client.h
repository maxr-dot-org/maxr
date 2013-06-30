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
#include "hud.h"
#include "jobs.h"
#include "main.h"
#include "network.h"

class cCasualtiesTracker;
class cClientAttackJob;
class cClientMoveJob;
class cEventHandling;
class cFx;
class cJob;
class cMap;
class cNetMessage;
class cPlayer;
class cServer;
class cStaticMap;
class cTCP;

Uint32 TimerCallback (Uint32 interval, void* arg);

/**
* Client class which handles the in and output for a player
*@author alzi alias DoctorDeath
*/
class cClient : public INetMessageReceiver
{
	friend class cDebugOutput;
	friend class cPlayer;
public:
	cClient (cServer* server_, cTCP* network_, cEventHandling& eventHandling_, cStaticMap& staticMap, std::vector<cPlayer*>* PlayerList);
	~cClient();

	// Return local server if any.
	// TODO: should be const cServer*
	cServer* getServer() const { return server; }
	cEventHandling& getEventHandling() { return *eventHandling; }
	virtual void pushEvent (cNetMessage* message);

	void enableFreezeMode (eFreezeMode mode, int playerNumber = -1);
	void disableFreezeMode (eFreezeMode mode);
	bool isFreezed() const;
	int getFreezeInfoPlayerNumber() const;
	bool getFreezeMode (eFreezeMode mode) const;

	/**
	* handles the end of a turn
	*@author alzi alias DoctorDeath
	*/
	void handleEnd();

	void addJob (cJob* job);

	/**
	* handles the game relevant actions
	* (for example moving the current position of a rocket)
	* of the fx-effects,
	* so that they are handled also, when the effects are not drawn.
	*/
	void runFx();

	/**
	* handles the rest-time of the current turn
	*@author alzi alias DoctorDeath
	*/
	void handleTurnTime();

	/**
	* creates a new moveJob and transmits it to the server
	* @param vehicle the vehicle to be moved
	* @param iDestOffset the Destination
	*/
	int addMoveJob (cVehicle* vehicle, int DestX, int DestY, const std::vector<cVehicle*>* group = NULL);
	void startGroupMove (const std::vector<cVehicle*>& group_, int mainDestX, int mainDestY);
	/**
	* adds a new movejob
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
	* returns the player identified by playerID
	*@author eiko
	*@param playerID Can be a representation of the player number or player name
	*/
	cPlayer* getPlayerFromString (const std::string& playerID);
	/**
	* deletes the unit
	*@author alzi alias DoctorDeath
	*@param Building Building which should be deleted.
	*@param Vehicle Vehicle which should be deleted.
	*/
	void deleteUnit (cBuilding* Building, cMenu* activeMenu);
	void deleteUnit (cVehicle* Vehicle, cMenu* activeMenu);
	/**
	* sends the netMessage to the server.
	* do not try to delete a message after calling this function!
	*@author Eiko
	*@param message The netMessage to be send.
	*/
	void sendNetMessage (cNetMessage* message) const;
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
	void doGameActions (cMenu* activeMenu);

	/**
	* processes everything that is need for this netMessage
	*@author alzi alias DoctorDeath
	*@param message The netMessage to be handled.
	*@return 0 for success
	*/
	int HandleNetMessage (cNetMessage* message, cMenu* activeMenu);

	void addFx (cFx* fx);

	/**
	* destroys a unit
	* play FX, add rubble and delete Unit
	*/
	void destroyUnit (cVehicle* vehicle);
	void destroyUnit (cBuilding* building);

	int getTurnLimit() const { return turnLimit; }
	int getScoreLimit() const { return scoreLimit; }
	int getTurn() const;

	void deletePlayer (cPlayer* player);

	cCasualtiesTracker& getCasualties() { return *casualtiesTracker; }
	const cMap* getMap() const { return Map; }
	cMap* getMap() { return Map; }
	const std::vector<cPlayer*>& getPlayerList() const { return *PlayerList; }
	std::vector<cPlayer*>& getPlayerList() { return *PlayerList; }
	const cPlayer* getActivePlayer() const { return ActivePlayer; }
	cPlayer* getActivePlayer() { return ActivePlayer; }
private:
	/**
	* adds the unit to the map and player.
	*@author alzi alias DoctorDeath
	*@param iPosX The X were the unit should be added.
	*@param iPosY The Y were the unit should be added.
	*@param Vehicle Vehicle which should be added.
	*@param Building Building which should be added.
	*@param Player Player whose vehicle should be added.
	*@param bInit true if this is an initialisation call.
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

	void runJobs();

	void HandleNetMessage_TCP_CLOSE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CHAT_SERVER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_PLAYER_CLANS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message, cMenu* activeMenu);
	void HandleNetMessage_GAME_EV_DEL_VEHICLE (cNetMessage& message, cMenu* activeMenu);
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
	void HandleNetMessage_GAME_EV_SUPPLY (cNetMessage& message, cMenu* activeMenu);
	void HandleNetMessage_GAME_EV_ADD_RUBBLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DETECTION_STATE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CLEAR_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STOP_CLEARING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NOFOG (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEFEATED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FREEZE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNFREEZE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_PLAYER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_TURN (cNetMessage& message);
	void HandleNetMessage_GAME_EV_HUD_SETTINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STORE_UNIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_EXIT_UNIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DELETE_EVERYTHING (cNetMessage& message, cMenu* activeMenu);
	void HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CREDITS_CHANGED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message, cMenu* activeMenu);
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
	void HandleNetMessage_GAME_EV_SET_GAME_TIME (cNetMessage& message);

private:
	cServer* server;
	cTCP* network;
	cEventHandling* eventHandling;
	/** the map */
	cMap* Map;
	/** List with all players */
	std::vector<cPlayer*>* PlayerList;
	/** the active Player */
	cPlayer* ActivePlayer;

	cJobContainer helperJobs;

	/** list with buildings without owner, e. g. rubble fields */
	cBuilding* neutralBuildings;
	/** number of current turn */
	int iTurn;
	/** true if the player has been defeated */
	bool bDefeated;
	/** how many seconds will be left for this turn */
	int iTurnTime;
	/** Ticks when the TurnTime has been started */
	unsigned int iStartTurnTime;
	/** this client's copy of the victory conditions **/
	int turnLimit;
	int scoreLimit;

	cCasualtiesTracker* casualtiesTracker;

	sFreezeModes freezeModes;
public:
	cGameTimerClient gameTimer;
	/** lists with all FX-Animation */
	std::vector<cFx*> FxList;
	/** list with the running clientAttackJobs */
	std::vector<cClientAttackJob*> attackJobs;
	/** List with all active movejobs */
	std::vector<cClientMoveJob*> ActiveMJobs;
	/** the hud */
	// TODO: this should be a pointer to the gameGui instance,
	// so it is possible to have a GUI-less client for ai implementation
	cGameGUI gameGUI;

	/** true if the turn should be end after all movejobs have been finished */
	bool bWantToEnd;
};

#endif
