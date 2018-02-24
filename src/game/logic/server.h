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

#include "defines.h"
#include "game/logic/gametimer.h"
#include "game/logic/jobs/job.h"
#include "main.h" // for sID
#include "game/data/map/map.h"
#include "network.h"
#include "game/data/units/unit.h" // sUnitLess
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/concurrentqueue.h"
#include "utility/flatset.h"
#include "game/data/savegameinfo.h"
#include "game/data/model.h"
#include "connectionmanager.h"

class cBuilding;
class cCasualtiesTracker;
class cNetMessage;
class cPlayer;
class cAttackJob;
class cServerMoveJob;
class cTCP;
class cUnit;
class cTurnCounter;
class cTurnTimeClock;
class cTurnTimeDeadline;
class cGameGuiState;
struct sLandingUnit;
class cGameSettings;

/**
* The Types which are possible for a game
*/
/*
enum eGameTypes
{
	GAME_TYPE_SINGLE,  // a singleplayer game
	GAME_TYPE_HOTSEAT, // a hotseat multiplayer game
	GAME_TYPE_TCPIP    // a multiplayergame over TCP/IP
};
*/

enum eServerState
{
	SERVER_STATE_INITGAME, // Choose clan, initial units and land position.
	SERVER_STATE_INGAME,   // Game is running
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

	cModel model;

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
	virtual void pushEvent (std::unique_ptr<cNetMessage> event);
	virtual std::unique_ptr<cNetMessage> popEvent();
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
	* deletes a player and all his units
	*@author alzi alias DoctorDeath
	*/
	void deletePlayer (cPlayer& Player);

	void kickPlayer (cPlayer& player);

	void sideStepStealthUnit (const cPosition& position, const cVehicle& vehicle, const cPosition& bigOffset = cPosition (-1, -1));
	void sideStepStealthUnit (const cPosition& position, const cStaticUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset = cPosition (-1, -1));

	std::shared_ptr<const cTurnCounter> getTurnClock() const { return turnClock; }

	std::shared_ptr<const cGameSettings> getGameSettings() const { return gameSettings; }
	bool isTurnBasedGame() const;

	const cGameGuiState& getPlayerGameGuiState (const cPlayer& player);
private:

	void defeatLoserPlayers();
	bool isVictoryConditionMet() const;

	void handleNetMessage_MU_MSG_CLAN (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_VEHICLES (cNetMessage& message);
	void handleNetMessage_MU_MSG_UPGRADES (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_COORDS (cNetMessage& message);
	void handleNetMessage_MU_MSG_READY_TO_START (cNetMessage& message);
	void handleNetMessage_GAME_EV_CHANGE_RESOURCES (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_MARK_LOG (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_SUPPLY (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_VEHICLE_UPGRADE (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_START_CLEAR (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_STOP_CLEAR (cNetMessage& message);
	void handleNetMessage_GAME_EV_ABORT_WAITING (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_EXIT (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_BUY_UPGRADES (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_BUILDING_UPGRADE (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_RESEARCH_CHANGE (cNetMessage& message);
	void handleNetMessage_GAME_EV_AUTOMOVE_STATUS (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_COM_ACTION (cNetMessage& message);
	void handleNetMessage_GAME_EV_REQUEST_CASUALTIES_REPORT (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_CHANGE_UNIT_NAME (cNetMessage& message);
	void handleNetMessage_GAME_EV_END_MOVE_ACTION (cNetMessage& message);
	void handleNetMessage_GAME_EV_WANT_KICK_PLAYER (cNetMessage& message);

	/**
	* checks whether a player is defeated
	*@author alzi alias DoctorDeath
	*/
	void checkDefeats();

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
	
	void checkPlayerUnits (cVehicle& vehicle, cPlayer& MapPlayer);
	void checkPlayerUnits (cBuilding& building, cPlayer& MapPlayer);
	void checkPlayerRubbles (cBuilding& building, cPlayer& MapPlayer);
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

	std::shared_ptr<cTurnCounter> turnClock;


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

	std::shared_ptr<cGameSettings> gameSettings;
	std::shared_ptr<cCasualtiesTracker> casualtiesTracker;

	std::map<int, cGameGuiState> playerGameGuiStates;
public:
	void addAttackJob (cUnit* aggressor, const cPosition& targetPosition); //TODO: so oder anders?
	/** the map */
	std::unique_ptr<cMap> Map;
	/** List with all active movejobs */
	std::vector<cServerMoveJob*> ActiveMJobs;
	/** state of the server */
	eServerState serverState;

	/** List with all players */
	std::vector<std::unique_ptr<cPlayer>> playerList;
};

#endif // game_logic_serverH
