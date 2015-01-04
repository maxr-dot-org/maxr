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

#ifndef game_logic_serverH
#define game_logic_serverH

#include <vector>
#include <map>
#include <set>

#include <SDL.h>

#include "utility/autoptr.h"
#include "defines.h"
#include "game/logic/gametimer.h"
#include "game/logic/jobs.h"
#include "main.h" // for sID
#include "game/data/map/map.h"
#include "network.h"
#include "game/data/units/unit.h" // sUnitLess
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/concurrentqueue.h"
#include "utility/flatset.h"

class cBuilding;
class cCasualtiesTracker;
class cNetMessage;
class cPlayer;
class cAttackJob;
class cServerMoveJob;
class cTCP;
class cUnit;
class cTurnClock;
class cTurnTimeClock;
class cTurnTimeDeadline;
class cGameGuiState;
struct sLandingUnit;
class cGameSettings;

/**
* The Types which are possible for a game
*/
enum eGameTypes
{
	GAME_TYPE_SINGLE,  // a singleplayer game
	GAME_TYPE_HOTSEAT, // a hotseat multiplayer game
	GAME_TYPE_TCPIP    // a multiplayergame over TCP/IP
};

enum eServerState
{
	SERVER_STATE_INITGAME, // Choose clan, initial units and land position.
	SERVER_STATE_INGAME,   // Game is running
};

/**
 * Structure for the reports
 */
struct sTurnstartReport
{
	/** unit type of the report */
	sID type;
	/** counter for this report */
	int count;
};

/**
* Callback function for the serverthread
*@author alzi alias DoctorDeath
*/
int CallbackRunServerThread (void* arg);


Uint32 ServerTimerCallback (Uint32 interval, void* arg);


/**
* Server class which takes all calculations for the game and has the data of all players
*@author alzi alias DoctorDeath
*/
class cServer : public INetMessageReceiver
{
	friend class cSavegame;
	friend class cServerGame;
	friend class cDebugOutputWidget;
public:
	/**
	 * initialises the server class.
	 *@author alzi alias DoctorDeath
	 *@param network_ non null for GAME_TYPE_TCPIP
	 */
	explicit cServer (std::shared_ptr<cTCP> network);
	~cServer();

	void setGameSettings (const cGameSettings& gameSettings);
	void setMap (std::shared_ptr<cStaticMap> staticMap);
	void addPlayer (std::unique_ptr<cPlayer> player);
	void start();
	void stop();

	/** the type of the current game */
	eGameTypes getGameType() const;

	void addLocalClient (cClient& client) { localClients.push_back (&client); }

	std::shared_ptr<const cCasualtiesTracker> getCasualtiesTracker() const { return casualtiesTracker;}
	const std::shared_ptr<cCasualtiesTracker>& getCasualtiesTracker() { return casualtiesTracker; }

	/**
	* Handles all incoming netMessages from the clients
	*@author Eiko
	*@param message The message to be prozessed
	*@return 0 for success
	*/
	int handleNetMessage (cNetMessage& message);

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
	*@param playerNumber The number of the player.
	*@return The wanted player.
	*/
	cPlayer& getPlayerFromNumber (int playerNumber);
	const cPlayer& getPlayerFromNumber (int playerNumber) const;
	/**
	* returns the player identified by playerID
	*@author eiko
	*@param playerID Can be the player number (as string) or player name
	*/
	cPlayer* getPlayerFromString (const std::string& playerID);

	void setActiveTurnPlayer (const cPlayer& player);
	cPlayer* getActiveTurnPlayer();

	/**
	 * returns if the player is on the disconnected players list
	 *@author pagra
	 */
	bool isPlayerDisconnected (const cPlayer& player) const;

	/**
	 * puts all players on the disconnected list.
	 * This is useful for loading a game on the dedicated server.
	 *@author pagra
	 */
	void markAllPlayersAsDisconnected();

	/**
	* pushes an event to the eventqueue of the server. This is threadsafe.
	*@author alzi alias DoctorDeath
	*@param event The SDL_Event to be pushed.
	*/
	virtual void pushEvent (std::unique_ptr<cNetMessage> event) MAXR_OVERRIDE_FUNCTION;

	/**
	* sends a netMessage to the client
	* on which the player with 'iPlayerNum' is playing
	* PlayerNum -1 means all players
	*@author alzi alias DoctorDeath
	*@param message The message to be send.
	*@param player The player who should receive this event. Null will send the message to all players.
	*/
	void sendNetMessage (std::unique_ptr<cNetMessage> message, const cPlayer* player = nullptr);

	/**
	* runs the server. Should only be called by the ServerThread!
	*@author alzi alias DoctorDeath
	*/
	void run();

	/**
	* runs all periodically actions on the game model
	*@author eiko
	*/
	void doGameActions();

	/**
	* deletes a Unit
	*@author alzi alias DoctorDeath
	*@param unit the unit which should be deleted.
	*@param notifyClient when false, the Unit is only removed locally.
	*       The caller must make sure to inform the clients
	*/
	void deleteUnit (cUnit* unit, bool notifyClient = true);

	/**
	* deletes a unit (and additional units on the same field if necessary)
	* from the game, creates rubble
	*/
	void destroyUnit (cUnit& unit);

	/**
	* adds the unit to the map and player.
	*@author alzi alias DoctorDeath
	*@param position The position were the unit should be added.
	*@param id id of the unit which should be added.
	*@param Player Player whose vehicle should be added.
	*@param bInit true if this is a initialisation call.
	*/
	cVehicle& addVehicle (const cPosition& position, const sID& id, cPlayer* Player, bool bInit = false, bool bAddToMap = true, unsigned int uid = 0);
	cBuilding& addBuilding (const cPosition& position, const sID& id, cPlayer* Player, bool bInit = false, unsigned int uid = 0);

	/**
	* adds a new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob (cServerMoveJob& MoveJob);
	/**
	* generates a new movejob
	*/
	bool addMoveJob (const cPosition& source, const cPosition& destination, cVehicle* vehicle);
	/**
	* adds a new rubble object to the game
	* @param position The position where the rubble is added
	* @param value the amount of material in the rubble field
	* @param big size of the rubble field
	*/
	void addRubble (const cPosition& position, int value, bool big);
	/**
	* deletes a rubble object from the game
	* @param rubble pointer to the rubble object which will be deleted
	*/
	void deleteRubble (cBuilding* rubble);

	void resyncPlayer (cPlayer& player, bool firstDelete, bool withGuiState);
	void resyncVehicle (const cVehicle& Vehicle, const cPlayer& Player);
	/**
	* deletes a player and all his units
	*@author alzi alias DoctorDeath
	*/
	void deletePlayer (cPlayer& Player);

	void kickPlayer (cPlayer& player);

	void sideStepStealthUnit (const cPosition& position, const cVehicle& vehicle, const cPosition& bigOffset = cPosition (-1, -1));
	void sideStepStealthUnit (const cPosition& position, const sUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset = cPosition (-1, -1));

	void makeAdditionalSaveRequest (int saveNum);

	std::shared_ptr<const cTurnClock> getTurnClock() const { return turnClock; }

	std::shared_ptr<const cTurnTimeClock> getTurnTimeClock() const { return turnTimeClock; }

	std::shared_ptr<const cGameSettings> getGameSettings() const { return gameSettings; }
	bool isTurnBasedGame() const;

	void enableFreezeMode (eFreezeMode mode, int playerNumber = -1);
	void disableFreezeMode (eFreezeMode mode);

	void startTurnTimers();

	void setTurnEndDeadline (const std::chrono::seconds& deadline);
	void setTurnEndDeadlineActive (bool value);

	void setTurnLimit (const std::chrono::seconds& deadline);
	void setTurnLimitActive (bool value);

	const cGameGuiState& getPlayerGameGuiState (const cPlayer& player);
private:
	void startNewGame();

	void placeInitialResources();

	/**
	* lands all units at the given position
	*@author alzi alias DoctorDeath
	*@param landingPosition The coordinates where to land.
	*@param Player The Player who wants to land.
	*@param landingUnits List with all units to land.
	*@param bFixed true if the bridgehead is fixed.
	*/
	void makeLanding (const cPosition& landingPosition, cPlayer& player, const std::vector<sLandingUnit>& landingUnits, bool isFixed);
	//void makeLanding (const std::vector<cPosition>& landPos, const std::vector<std::vector<sLandingUnit>*>& landingUnits);
	void makeLanding();
	/**
	 *
	 */
	void correctLandingPos (cPosition& landingPosition);


	void defeatLoserPlayers();
	bool isVictoryConditionMet() const;

	void handleNetMessage_TCP_ACCEPT (cNetMessage& message);
	void handleNetMessage_MU_MSG_CLAN (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_VEHICLES (cNetMessage& message);
	void handleNetMessage_MU_MSG_UPGRADES (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_COORDS (cNetMessage& message);
	void handleNetMessage_MU_MSG_READY_TO_START (cNetMessage& message);
	void handleNetMessage_TCP_CLOSE_OR_GAME_EV_WANT_DISCONNECT (cNetMessage& message);
	void handleNetMessage_GAME_EV_CHAT_CLIENT (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_TO_END_TURN (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_START_WORK (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_STOP_WORK (cNetMessage& message);
	void handleNetMessage_GAME_EV_MOVE_JOB_CLIENT (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_STOP_MOVE (cNetMessage& message);
	void handleNetMessage_GAME_EV_MOVEJOB_RESUME (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_ATTACK (cNetMessage& message);
	void handleNetMessage_GAME_EV_MINELAYERSTATUS (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_BUILD (cNetMessage& message);
	void handleNetMessage_GAME_EV_END_BUILDING (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_STOP_BUILDING (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_TRANSFER (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_BUILDLIST (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_EXIT_FIN_VEH (cNetMessage& message);
	void handleNetMessage_GAME_EV_CHANGE_RESOURCES (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_CHANGE_MANUAL_FIRE (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_CHANGE_SENTRY (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_SUPPLY (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_START_CLEAR (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_STOP_CLEAR (cNetMessage& message);
	void handleNetMessage_GAME_EV_ABORT_WAITING (cNetMessage& message);
	void handleNetMessage_GAME_EV_IDENTIFICATION (cNetMessage& message);
	void handleNetMessage_GAME_EV_RECON_SUCCESS (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_LOAD (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_EXIT (cNetMessage& message);
	void handleNetMessage_GAME_EV_REQUEST_RESYNC (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (cNetMessage& message);
	void handleNetMessage_GAME_EV_AUTOMOVE_STATUS (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_COM_ACTION (cNetMessage& message);
	void handleNetMessage_GAME_EV_SAVE_HUD_INFO (cNetMessage& message);
	void handleNetMessage_GAME_EV_SAVE_REPORT_INFO (cNetMessage& message);
	void handleNetMessage_GAME_EV_FIN_SEND_SAVE_INFO (cNetMessage& message);
	void handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_SELFDESTROY (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (cNetMessage& message);
	void handleNetMessage_GAME_EV_END_MOVE_ACTION (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_KICK_PLAYER (cNetMessage& message);

	/**
	* lands the vehicle at a free position in the radius
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to land.
	*@param iY The Y coordinate to land.
	*@param iWidth Width of the field.
	*@param iHeight iHeight of the field.
	*@param unitData Vehicle to land.
	*@param player Player whose vehicle should be land.
	*@return nullptr if the vehicle could not be landed,
	*        else a pointer to the vehicle.
	*/
	cVehicle* landVehicle (const cPosition& landingPosition, int iWidth, int iHeight, const sUnitData& unitData, cPlayer& player);

	/**
	* handles the pressed end of a player
	*@author alzi alias DoctorDeath
	*/
	void handleEnd (cPlayer& player);
	/**
	 * executes everything for a turn start
	 */
	void makeTurnStart (cPlayer& player);
	/**
	* checks whether a player is defeated
	*@author alzi alias DoctorDeath
	*/
	void checkDefeats();

	/**
	* rechecks the end actions when a player wanted to finish his turn
	*@author alzi alias DoctorDeath
	*/
	void handleWantEnd();

	/**
	 * checks whether some units are moving and restarts remaining movements
	 *
	 * @author alzi alias DoctorDeath
	 * @param player The player who will receive the messages
	 *       when the turn can't be finished now; nullptr for all players
	 * @return true if there were found some moving units
	 */
	bool checkRemainingMoveJobs (const cPlayer* player);

	bool executeRemainingMoveJobs (const cPlayer& player);

	/**
	* checks whether the deadline has run down
	*@author alzi alias DoctorDeath
	*/
	void checkDeadline();
	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs();

	/**
	* Calculates the cost, that this upgrade would have for the given player.
	*@author Paul Grathwohl
	*/
	int getUpgradeCosts (const sID& ID, cPlayer& player,
						 int newDamage, int newMaxShots, int newRange, int newMaxAmmo,
						 int newArmor, int newMaxHitPoints, int newScan, int newMaxSpeed);

	/**
	* changes the owner of a vehicle
	*@author alzi alias DoctorDeath
	*/
	void changeUnitOwner (cVehicle& vehicle, cPlayer& newOwner);
	/**
	* stops the buildingprocess of a working vehicle.
	*@author alzi alias DoctorDeath
	*/
	void stopVehicleBuilding (cVehicle& vehicle);

	/**
	 * Helper for destroyUnit() that deletes all buildings
	 * and returns the generated rubble value.
	 */
	int deleteBuildings (cMapField& field, bool deleteConnector);

	void runJobs();

	void checkPlayerUnits (cVehicle& vehicle, cPlayer& MapPlayer);
	void checkPlayerUnits (cBuilding& building, cPlayer& MapPlayer);
	void checkPlayerRubbles (cBuilding& building, cPlayer& MapPlayer);

	void addJob (cJob* job);
public:
	std::shared_ptr<cTCP> network;
private:
	cSignalConnectionManager signalConnectionManager;

	/** local clients if any. */
	std::vector<cClient*> localClients;

	std::map<const cPlayer*, cPosition> playerLandingPositions;
	std::set<const cPlayer*> readyToStartPlayers;
	std::map<const cPlayer*, std::vector<sLandingUnit>> playerLandingUnits;

	/** controls the timesynchoneous actions on server and client */
	std::shared_ptr<cGameTimerServer> gameTimer;
	/** little helper jobs, that do some time dependent actions */
	cJobContainer helperJobs;
	/** a list with all events for the server */
	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue;

	/** the thread the server runs in */
	SDL_Thread* serverThread;
	/** true if the server should exit and end his thread */
	bool bExit;

	/** the server is on halt, because a client is not responding */
	int waitForPlayer;
	/** list with buildings without owner, e.g. rubble fields */
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> neutralBuildings;
	/** number of active player in turn based multiplayer game */
	cPlayer* activeTurnPlayer;
	/** a list with the numbers of all players who have ended their turn. Valid only for simultaneous games */
	std::vector<const cPlayer*> playerEndList;

	std::shared_ptr<cTurnClock> turnClock;

	std::shared_ptr<cTurnTimeClock> turnTimeClock;
	std::shared_ptr<cTurnTimeDeadline> turnEndDeadline;
	std::shared_ptr<cTurnTimeDeadline> turnLimitDeadline;

	/** stores the gametime of the last turn end. */
	unsigned int lastTurnEnd;

	/**
	 * Number of the Player who wants to end his turn:
	 * -1 for no player
	 * -2 for undefined player
	 */
	int pendingEndTurnPlayerNumber;

	/** The next unique ID for the unit creation */
	unsigned int iNextUnitID;
	/** if this is true the map will be opened for a defeated player */
	bool openMapDefeat;
	/** List with disconnected players */
	std::vector<cPlayer*> DisconnectedPlayerList;
	/** a sequential id for identifying additional
	 * save information from clients */
	int savingID;
	/** the index of the saveslot where additional save info should be added */
	int savingIndex;
	/** server is executing all remaining movements,
	 * before turn end is processed */
	bool executingRemainingMovements;

	std::shared_ptr<cGameSettings> gameSettings;
	std::shared_ptr<cCasualtiesTracker> casualtiesTracker;
	sFreezeModes freezeModes;

	std::map<int, cGameGuiState> playerGameGuiStates;
	/** List with all attackjobs */
	std::vector<cAttackJob*> AJobs;
public:
	void addAttackJob (cUnit* aggressor, const cPosition& targetPosition); //TODO: so oder anders?
	/** the map */
	AutoPtr<cMap> Map;
	/** List with all active movejobs */
	std::vector<cServerMoveJob*> ActiveMJobs;
	/** state of the server */
	eServerState serverState;

	/** List with all players */
	std::vector<std::unique_ptr<cPlayer>> playerList;
};

#endif // game_logic_serverH
