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
#include "defines.h"
#include "main.h"
#include "map.h"
#include "player.h"
#include "attackJobs.h"
#include "engine.h" // for sReport

/**
* The Types which are possible for a game
*/
#define GAME_TYPE_SINGLE	0	// a singleplayer game
#define GAME_TYPE_HOTSEAT	1	// a hotseat multiplayer game
#define GAME_TYPE_TCPIP		2	// a multiplayergame over TCP/IP

#if 0
/**
 * Structure for the reports
 */
struct sReport
{
	/** name of the report */
	string name;
	/** counter for this report */
	int anz;
};
#endif

/**
* Callback funktion for the serverthread
*@author alzi alias DoctorDeath
*/
int CallbackRunServerThread( void *arg );


Uint32 ServerTimerCallback(Uint32 interval, void *arg);

/**
* Server class which takes all calculations for the game and has the data of all players
*@author alzi alias DoctorDeath
*/
class cServer
{
public:
	/**
	 * initialises the server class
	 *@author alzi alias DoctorDeath
	 *@param map The Map for the game
	 *@param PlayerList The list with all players
	 *@param iGameType The type of the game. Can be GAME_TYPE_SINGLE, GAME_TYPE_HOTSEAT or GAME_TYPE_TCPIP
	 */
	cServer(cMap* map, cList<cPlayer*>* PlayerList, int iGameType, bool bPlayTurns);

	~cServer();

private:
	/** a list with all events for the server */
	cList<SDL_Event*> *EventQueue;

	/* a list with all netMessages for the server 
	cList<cNetMessage*> *NetMessageQueue; */

	/** the thread the server runs in */
	SDL_Thread *ServerThread;
	/** mutex for the eventqueue */
	SDL_mutex *QueueMutex;
	/** true if the server should exit and end his thread */
	bool bExit;


	/** current movejob */
	cMJobs *mjobs;
	/** List with all active movejobs */
	cList<cMJobs*> *ActiveMJobs;
	/** true if this is a hotseat game */
	bool bHotSeat;
	/** number of active player in hotseat */
	int iHotSeatPlayer;
	/** true if the game should be played in turns */
	bool bPlayTurns;
	/** number of active player in turn based multiplayer game */
	int iActiveTurnPlayerNr;
	/** name of the savegame to load or to save */
	string sSaveLoadFile;
	/** index number of the savegame to load or to save */
	int iSaveLoadNumber;
	/** the type of the current game */
	int iGameType;
	/** a list with the numbers of all players who have ended theire turn */
	cList<int*> *PlayerEndList;
	/** number of current turn */
	int iTurn;
	/** deadline in seconds if the first player has finished his turn*/
	int iTurnDeadline;
	/** Ticks when the deadline has been initialised*/
	unsigned int iDeadlineStartTime;
	/** The ID for the next unit*/
	unsigned int iNextUnitID;
	/** will be incremented by the Timer */
	unsigned int iTimerTime;
	/** diffrent timers */
	int iTimer0, iTimer1, iTimer2;
	/** ID of the timer */
	SDL_TimerID TimerID;

	/**
	* returns a pointer to the next event of the eventqueue. If the queue is empty it will return NULL.
	* the returned event and its data structures are valid until the next call of pollEvent()
	*@author alzi alias DoctorDeath
	*@return the next SDL_Event or NULL if queue is empty
	*/
	SDL_Event* pollEvent();
	
	//cNetMessage* pollNetMessage();
	
	/**
	* Handels all incoming netMessages from the clients
	*@author Eiko
	*@param message The message to be prozessed
	*@return 0 for success
	*/
	int HandleNetMessage( cNetMessage* message );

	/**
	* checks whether the field is free for landing
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to check.
	*@param iY The Y coordinate to check.
	*@return true if it is free, else false.
	*/
	bool freeForLanding ( int iX, int iY );
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
	cVehicle *landVehicle ( int iX, int iY, int iWidth, int iHeight, sVehicle *Vehicle, cPlayer *Player );
	/**
	* checks whether a player has detected some new enemy units
	*@author alzi alias DoctorDeath
	*/
	void checkPlayerUnits ();

	/**
	* handles the pressed end of a player
	*@author alzi alias DoctorDeath
	*/
	void handleEnd ( int iPlayerNum );
	/**
	* executes everthing for a turnend
	*@author alzi alias DoctorDeath
	*@param iPlayerNum Number of player who has pressed the end turn button
	*@param bChangeTurn true if all players have ended their turn and the turnnumber has changed
	*/
	void makeTurnEnd ( int iPlayerNum , bool bChangeTurn );
	/**
	* gets the reportmessage for a player
	*@author alzi alias DoctorDeath
	*@param iPlayerNum Number of player for who the report should be generated
	*@param sReportMsg string buffer for the report string
	*@param iVoiceNum integer buffer for the number which voice sound should be played
	*/
	void getTurnstartReport ( int iPlayerNum, string *sReportMsg, int *iVoiceNum );
	/**
	* adds a report to the reportlist
	*@author alzi alias DoctorDeath
	*@param sName the report name
	*@param bVehicle true if the report is about vehicles
	*@param iPlayerNum Number of player to whos list the report should be added
	*/
	void addReport ( string sName, bool bVehicle, int iPlayerNum );
	/**
	* checks wether the deadline has run down
	*@author alzi alias DoctorDeath
	*/
	void checkDeadline ();
	/**
	* handles the timers iTimer0, iTimer1 and iTimer2
	*@author alzi alias DoctorDeath
	*/
	void handleTimer();
	/**
	* adds an new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob ( cMJobs *MJob );
	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs();
	/**
	* checks whether the next waypoint is not blocked
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be checked
	*/
	void checkMove ( cMJobs *MJob );
	/**
	* moves a vehicle one step closer to the next field
	*@author alzi alias DoctorDeath
	*@param Vehicle The vehicle to be moved
	*/
	void moveVehicle ( cVehicle *Vehicle );
	/**
	* gets the vehicle with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the vehicle
	*/
	cVehicle *getVehicleFromID ( int iID );
public:	
	/** the map */
	cMap *Map;
	/** List with all attackjobs */
	cList<cServerAttackJob*> *AJobs;
	/** List with all players */
	cList<cPlayer*> *PlayerList;
	/** true if the game has been started */
	bool bStarted;

	/**
	* returns the player with the given number
	*@author alzi alias DoctorDeath
	*@param iNum The number of the player.
	*@return The wanted player.
	*/
	cPlayer *getPlayerFromNumber ( int iNum );
	/**
	* pushes an event to the eventqueue of the server. This is threadsafe.
	*@author alzi alias DoctorDeath
	*@param event The SDL_Event to be pushed.
	*@return 0 for success
	*/
	int pushEvent( SDL_Event *event );
	
	//int pushNetMessage( cNetMessage* message );

	/**
	* sends a netMessage to the client on which the player with 'iPlayerNum' is playing
	* PlayerNum -1 means all players
	* message must not be deleted after caling this function
	*@author alzi alias DoctorDeath
	*@param message The message to be send.
	*@param iPlayerNum Number of player who should receive this event.
	*/
	void sendNetMessage( cNetMessage* message, int iPlayerNum = -1 );

	/**
	* runs the server. Should only be called by the ServerThread!
	*@author alzi alias DoctorDeath
	*/
	void run();

	/**
	* deletes the building
	*@author alzi alias DoctorDeath
	*@param Building Building which should be deleted.
	*/
	void deleteBuilding ( cBuilding *Building );
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
	void addUnit( int iPosX, int iPosY, sVehicle *Vehicle, cPlayer *Player, bool bInit = false );
	void addUnit( int iPosX, int iPosY, sBuilding *Building, cPlayer *Player, bool bInit = false );
	/**
	* lands all units at the given position
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to land.
	*@param iY The Y coordinate to land.
	*@param Player The Player who wants to land.
	*@param List List with all units to land.
	*@param bFixed true if the bridgehead is fixed.
	*/
	void makeLanding( int iX, int iY, cPlayer *Player, cList<sLanding*> *List, bool bFixed );
	/**
	* increments the iTimeTimer.
	*@author alzi alias DoctorDeath
	*/
	void Timer();

	void releaseMoveJob ( cMJobs *MJob );
} EX *Server;

#endif
