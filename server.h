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
#ifndef serverH
#define serverH
#include <SDL.h>
#include "defines.h"
#include "gametimer.h"
#include "jobs.h"
#include "main.h" // for sID
#include "map.h"
#include "network.h"
#include "ringbuffer.h"
#include <vector>

class cBuilding;
class cCasualtiesTracker;
class cNetMessage;
class cPlayer;
class cServerAttackJob;
class cServerMoveJob;
class cTCP;
class cUnit;
struct sClientLandData;
struct sSettings;
struct sVehicle;

/**
* The Types which are possible for a game
*/
enum eGameTypes
{
	GAME_TYPE_SINGLE,		// a singleplayer game
	GAME_TYPE_HOTSEAT,		// a hotseat multiplayer game
	GAME_TYPE_TCPIP			// a multiplayergame over TCP/IP
};

/**
 * Structure for the reports
 */
struct sTurnstartReport
{
	/** unit type of the report */
	sID Type;
	/** counter for this report */
	int iAnz;
};

/**
* Callback funktion for the serverthread
*@author alzi alias DoctorDeath
*/
int CallbackRunServerThread (void* arg);


Uint32 ServerTimerCallback (Uint32 interval, void* arg);

struct sLandingUnit;

/**
* Server class which takes all calculations for the game and has the data of all players
*@author alzi alias DoctorDeath
*/
class cServer : public INetMessageReceiver
{
	friend class cSavegame;
	friend class cServerGame;
	friend class cDebugOutput;
public:
	/**
	 * initialises the server class.
	 *@author alzi alias DoctorDeath
	 *@param network_ non null for GAME_TYPE_TCPIP
	 *@param map The Map for the game
	 *@param PlayerList The list with all players
	 */
	cServer (cTCP* network_, cMap& map, std::vector<cPlayer*>* PlayerList);
	~cServer();

	void setGameSettings (const sSettings& gameSettings);
	void setDeadline (int iDeadline);
	void stop ();

	int getTurnLimit() const { return turnLimit; }
	int getScoreLimit() const { return scoreLimit; }
	/** the type of the current game */
	eGameTypes getGameType() const;

	void setLocalClient (cClient& client) { localClient = &client; }

	cTCP* network;
private:
	/** local client if any. */
	cClient* localClient;

	/** controls the timesynchonous actions on server and client */
	cGameTimerServer gameTimer;
	/** little helper jobs, that do some time dependent actions */
	cJobContainer helperJobs;
	/** a list with all events for the server */
	cRingbuffer<cNetMessage*> eventQueue;

	/** the thread the server runs in */
	SDL_Thread* ServerThread;
	/** true if the server should exit and end his thread */
	bool bExit;


	/** the server is on halt, because a client is nor responding */
	int waitForPlayer;
	/** list with buildings without owner, e. g. rubble fields */
	cBuilding* neutralBuildings;
	/** true if this is a hotseat game */
	bool bHotSeat;
	/** number of active player in hotseat */
	int iHotSeatPlayer;
	/** true if the game should be played in turns */
	bool bPlayTurns;
	/** number of active player in turn based multiplayer game */
	int iActiveTurnPlayerNr;
	/** name of the savegame to load or to save */
	std::string sSaveLoadFile;
	/** index number of the savegame to load or to save */
	int iSaveLoadNumber;
	/** a list with the numbers of all players who have ended theire turn */
	std::vector<cPlayer*> PlayerEndList;
	/** number of current turn */
	int iTurn;
	/** deadline in seconds if the first player has finished his turn*/
	int iTurnDeadline;
	/** Ticks when the deadline has been initialised*/
	unsigned int iDeadlineStartTime;
	/** Number of the Player who wants to end his turn; -1 for no player, -2 for undifined player */
	int iWantPlayerEndNum;
	/** The ID for the next unit*/
	unsigned int iNextUnitID;
	/** if this is true the map will be opened for a defeated player */
	bool openMapDefeat;
	/** List with disconnected players */
	std::vector<cPlayer*> DisconnectedPlayerList;
	/** a sequential id for identifying additional save information from clients */
	int savingID;
	/** the index of the saveslot where additional save info should be added */
	int savingIndex;
	/** stores the gametime of the last turn end. */
	unsigned int lastTurnEnd;
	/** sever is executon all remaining movements, before turn end is processed */
	bool executingRemainingMovements;

	/** victory conditions. One or both must be zero. **/
	int turnLimit, scoreLimit;

	cCasualtiesTracker* casualtiesTracker;

	sFreezeModes freezeModes;
public:
	const cCasualtiesTracker* getCasualtiesTracker() const {return casualtiesTracker;}
	cCasualtiesTracker* getCasualtiesTracker() {return casualtiesTracker;}

private:
	/**
	* returns a pointer to the next event of the eventqueue. If the queue is empty it will return NULL.
	* the returned message and its data structures are valid until the next call of pollEvent()
	*@author eiko
	*@return the next net message or NULL if queue is empty
	*/
	cNetMessage* pollEvent();

	void HandleNetMessage_TCP_ACCEPT (cNetMessage& message);
	void HandleNetMessage_TCP_CLOSE_OR_GAME_EV_WANT_DISCONNECT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CHAT_CLIENT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_TO_END_TURN (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_START_WORK (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_STOP_WORK (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MOVE_JOB_CLIENT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_STOP_MOVE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MOVEJOB_RESUME (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_ATTACK (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ATTACKJOB_FINISHED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MINELAYERSTATUS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_BUILD (cNetMessage& message);
	void HandleNetMessage_GAME_EV_END_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_STOP_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_TRANSFER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_BUILDLIST (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CHANGE_RESOURCES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_SUPPLY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_START_CLEAR (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_STOP_CLEAR (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ABORT_WAITING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_IDENTIFICATION (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RECON_SUCESS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_LOAD (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_EXIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_REQUEST_RESYNC (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_BUY_UPGRADES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_AUTOMOVE_STATUS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_COM_ACTION (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SAVE_HUD_INFO (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SAVE_REPORT_INFO (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (cNetMessage& message);
	void HandleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_SELFDESTROY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (cNetMessage& message);
	void HandleNetMessage_GAME_EV_END_MOVE_ACTION (cNetMessage& message);

public:
	/**
	* Handels all incoming netMessages from the clients
	*@author Eiko
	*@param message The message to be prozessed
	*@return 0 for success
	*/
	int HandleNetMessage (cNetMessage* message);
private:

	/**
	* lands the vehicle at a free position in the radius
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to land.
	*@param iY The Y coordinate to land.
	*@param iWidth Width of the field.
	*@param iHeight iHeight of the field.
	*@param Vehicle Vehicle to land.
	*@param Player Player whose vehicle should be land.
	*@return NULL if the vehicle could not be landed, else a pointer to the vehicle.
	*/
	cVehicle* landVehicle (int iX, int iY, int iWidth, int iHeight, sVehicle* Vehicle, cPlayer* Player);

	/**
	* handles the pressed end of a player
	*@author alzi alias DoctorDeath
	*/
	void handleEnd (int iPlayerNum);
	/**
	* executes everthing for a turnend
	*@author alzi alias DoctorDeath
	*@param iPlayerNum Number of player who has pressed the end turn button
	*@param bChangeTurn true if all players have ended their turn and the turnnumber has changed
	*/
	void makeTurnEnd();
	/**
	* checks whether a player is defeated
	*@author alzi alias DoctorDeath
	*/
	void checkDefeats();

public:
	/**
	* rechecks the end actions when a player wanted to finish his turn
	*@author alzi alias DoctorDeath
	*/
	void handleWantEnd();
private:
	/**
	* checks whether some units are moving and restarts remaining movements
	*@author alzi alias DoctorDeath
	*@param iPlayer The player who will receive the messages when the turn can't be finished now; -1 for all players
	*@return true if there were found some moving units
	*/
	bool checkEndActions (int iPlayer);
public:
	/**
	* checks wether the deadline has run down
	*@author alzi alias DoctorDeath
	*/
	void checkDeadline();
	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs();

private:
	/**
	* Calculates the cost, that this upgrade would have for the given player.
	*@author Paul Grathwohl
	*/
	int getUpgradeCosts (const sID& ID, cPlayer* player, bool bVehicle,
						 int newDamage, int newMaxShots, int newRange, int newMaxAmmo,
						 int newArmor, int newMaxHitPoints, int newScan, int newMaxSpeed);

	/**
	* changes the owner of a vehicle
	*@author alzi alias DoctorDeath
	*/
	void changeUnitOwner (cVehicle* vehicle, cPlayer* newOwner);
	/**
	* stops the buildingprocess of a working vehicle.
	*@author alzi alias DoctorDeath
	*/
	void stopVehicleBuilding (cVehicle* vehicle);

	/**
	 * Helper for destroyUnit(cBuilding) that deletes all buildings and returns the generated rubble value.
	 * @author Paul Grathwohl
	 */
	int deleteBuildings (std::vector<cBuilding*>& buildings);

	void runJobs ();

	void checkPlayerUnits (cVehicle& vehicle, cPlayer& MapPlayer);
	void checkPlayerUnits (cBuilding& building, cPlayer& MapPlayer);
	void checkPlayerRubbles (cBuilding& building, cPlayer& MapPlayer);

public:
	/** the map */
	cMap* Map;
	/** List with all attackjobs */
	std::vector<cServerAttackJob*> AJobs;
	/** List with all active movejobs */
	std::vector<cServerMoveJob*> ActiveMJobs;
	/** List with all players */
	std::vector<cPlayer*>* PlayerList;
	/** true if the game has been started */
	bool bStarted;

	void addJob (cJob* job);

	/**
	 * gets the unit with the ID
	 *@param iID The ID of the unit
	 */
	cUnit* getUnitFromID (unsigned int iID) const;
	/**
	* gets the vehicle with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the vehicle
	*/
	cVehicle* getVehicleFromID (unsigned int iID) const;
	/**
	* gets the bulding with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the building
	*/
	cBuilding* getBuildingFromID (unsigned int iID) const;

	/**
	* checks whether a player has detected some new enemy units
	*@author alzi alias DoctorDeath
	*/
	void checkPlayerUnits();

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
	*@param playerID Can be a string representation of the player number or player name
	*/
	cPlayer* getPlayerFromString (const std::string& playerID);

	/**
	 * returns if the player is on the disconnected players list
	 *@author pagra
	 */
	bool isPlayerDisconnected (const cPlayer* player) const;

	/**
	 * puts all players on the disconnected list. This is useful for loading a
	 * game on the dedicated server.
	 *@author pagra
	 */
	void markAllPlayersAsDisconnected();

	/**
	* pushes an event to the eventqueue of the server. This is threadsafe.
	*@author alzi alias DoctorDeath
	*@param event The SDL_Event to be pushed.
	*/
	void pushEvent (cNetMessage* event);


	/**
	* sends a netMessage to the client on which the player with 'iPlayerNum' is playing
	* PlayerNum -1 means all players
	* message must not be deleted after caling this function
	*@author alzi alias DoctorDeath
	*@param message The message to be send.
	*@param iPlayerNum Number of player who should receive this event.
	*/
	void sendNetMessage (cNetMessage* message, int iPlayerNum = -1);

	/**
	* runs the server. Should only be called by the ServerThread!
	*@author alzi alias DoctorDeath
	*/
	void run();

	/**
	* this function is responsible for running all periodically actions on the game modell
	*@author eiko
	*/
	void doGameActions ();

	/**
	* deletes a Unit
	*@author alzi alias DoctorDeath
	*@param unit the unit which should be deleted.
	*@param notifyClient when false, the Unit is only removed locally on the Server. The caller must make sure to inform the clients
	*/
	void deleteUnit (cUnit* unit, bool notifyClient = true);

	/**
	* deletes an unit (and additional units on the same field if nessesarry)
	* from the game, creates rubble
	* does not notify the client! the the caller has to take care of the nessesary actions on the client
	*/
	void destroyUnit (cVehicle* vehicle);
	void destroyUnit (cBuilding* building);


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
	cVehicle* addUnit (int iPosX, int iPosY, const sVehicle* Vehicle, cPlayer* Player, bool bInit = false, bool bAddToMap = true, unsigned int ID = 0);
	cBuilding* addUnit (int iPosX, int iPosY, const sBuilding* Building, cPlayer* Player, bool bInit = false, unsigned int ID = 0);


	void placeInitialResources (const std::vector<sClientLandData*>& landData, const sSettings& settings);

	/**
	* lands all units at the given position
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to land.
	*@param iY The Y coordinate to land.
	*@param Player The Player who wants to land.
	*@param List List with all units to land.
	*@param bFixed true if the bridgehead is fixed.
	*/
	void makeLanding (int iX, int iY, cPlayer* Player, std::vector<sLandingUnit>* List, bool bFixed);
	/**
	 *
	 */
	void correctLandingPos (int& iX, int& iY);
	/**
	* adds a report to the reportlist
	*@author alzi alias DoctorDeath
	*@param sName the report name
	*@param bVehicle true if the report is about vehicles
	*@param iPlayerNum Number of player to whos list the report should be added
	*/
	void addReport (sID Type, bool bVehicle, int iPlayerNum);
	/**
	* adds an new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob (cServerMoveJob* MoveJob);
	/**
	* generates a new movejob
	*/
	bool addMoveJob (int srcX, int srcY, int destX, int destY, cVehicle* vehicle);
	/**
	* adds a new rubble object to the game
	* @param x,y the position where the rubble is added
	* @param value the amount of material in the rubble field
	* @param big size of the rubble field
	*/
	void addRubble (int x, int y, int value, bool big);
	/**
	* deletes a rubble object from the game
	* @param rubble pointer to the rubble object which will be deleted
	*/
	void deleteRubble (cBuilding* rubble);

	void resyncPlayer (cPlayer* Player, bool firstDelete = false);
	void resyncVehicle (cVehicle* Vehicle, cPlayer* Player);
	/**
	* deletes a player and all his units
	*@author alzi alias DoctorDeath
	*/
	void deletePlayer (cPlayer* Player);

	void kickPlayer (cPlayer* player);


	void sideStepStealthUnit (int PosX, int PosY, cVehicle* vehicle, int bigOffset = -1);
	void sideStepStealthUnit (int PosX, int PosY, sUnitData& vehicleData, cPlayer* vehicleOwner, int bigOffset = -1);

	void makeAdditionalSaveRequest (int saveNum);

	int getTurn() const;

	bool isTurnBasedGame() const { return bPlayTurns; }

	void enableFreezeMode (eFreezeMode mode, int playerNumber = -1);
	void disableFreezeMode (eFreezeMode mode);

};

#endif
