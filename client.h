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
#include "player.h"
#include "map.h"
#include "netmessage.h"
#include "attackJobs.h"
#include "input.h"
#include "drawingcache.h"
#include "hud.h"

Uint32 TimerCallback(Uint32 interval, void *arg);


/** structure for the messages displayed in the game */
struct sMessage
{
	public:
		sMessage(std::string const&, unsigned int age);
		~sMessage();

	public:
		char*        msg;
		int          chars;
		int          len;
		unsigned int age;
};

/** FX types */
enum eFXTyps {fxMuzzleBig,fxMuzzleSmall,fxMuzzleMed,fxMuzzleMedLong,fxExploSmall,fxExploBig,fxExploAir,fxExploWater,fxHit,fxSmoke,fxRocket,fxDarkSmoke,fxTorpedo,fxTracks,fxBubbles,fxCorpse,fxAbsorb};

/** struct for the rocked data */
struct sFXRocketInfos{
  int ScrX,ScrY;
  int DestX,DestY;
  int dir;
  float fpx,fpy,mx,my;
  cClientAttackJob *aj;
};

/** struct for the dark smoke data */
struct sFXDarkSmoke{
  int alpha;
  float fx,fy;
  float dx,dy;
};

/** struct for the tracks effect */
struct sFXTracks{
  int alpha;
  int dir;
};

/** struct for an FX effect */
struct sFX
{
	public:
		sFX( eFXTyps typ, int x, int y);
		~sFX();

		eFXTyps typ;
		int PosX,PosY;
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
	cClient(cMap* Map, cList<cPlayer*>* PlayerList);
	~cClient();

private:
	friend class cGameGUI;
	friend class cPlayer;
	friend class cBuilding;
	friend class cVehicle;
	friend class cUnit;

	/** List with all players */
	cList<cPlayer*> *PlayerList;
	/** list with buildings without owner, e. g. rubble fields */
	cBuilding* neutralBuildings;
	/** ID of the timer */
	SDL_TimerID TimerID;
	/** list with all messages */
	cList<sMessage*> messages;
	/** number of current turn */
	int iTurn;
	/** lists with all FX-Animation */
	cList<sFX*> FXList;
	cList<sFX*> FXListBottom;
	/** flags what should be displaxed in the raffinery */
	bool bUpShowTank, bUpShowPlane, bUpShowShip, bUpShowBuild, bUpShowTNT;
	/** Coordinates to a important message */
	int iMsgCoordsX, iMsgCoordsY;
	/** true if the player has been defeated */
	bool bDefeated;
	/** how many seconds will be left for this turn */
	int iTurnTime;
	/** Ticks when the TurnTime has been started */
	unsigned int iStartTurnTime;

	/**
	* handles the game relevant actions (for example moving the current position of a rocket)
	* of the fx-effects, so that they are handled also, when the effects are not drawn.
	*/
	void runFX();
	/**
	* handles the game messages
	*@author alzi alias DoctorDeath
	*/
	void handleMessages();

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
	void addUnit( int iPosX, int iPosY, cVehicle *AddedVehicle, bool bInit = false, bool bAddToMap = true );
	void addUnit( int iPosX, int iPosY, cBuilding *AddedBuilding, bool bInit = false );
	/**
	* returns the player with the given number
	*@author alzi alias DoctorDeath
	*@param iNum The number of the player.
	*@return The wanted player.
	*/
	cPlayer *getPlayerFromNumber ( int iNum );
	/**
	* handles the end of a turn
	*@author alzi alias DoctorDeath
	*/
	void handleEnd();
	/**
	* handles the end of a hotseat game
	*@author alzi alias DoctorDeath
	*@param iNextPlayerNum Number of Player who has ended his turn
	*/
	void makeHotSeatEnd( int iNextPlayerNum );
	/**
	* handles the rest-time of the current turn
	*@author alzi alias DoctorDeath
	*/
	void handleTurnTime();
	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs ();
	/**
	* gets the subbase with the id
	*@author alzi alias DoctorDeath
	*@param iID Id of the subbase
	*/
	sSubBase *getSubBaseFromID ( int iID );
public:
	/**  the soundstream of the selected unit */
	int iObjectStream;
	/** the active Player */
	cPlayer *ActivePlayer;
	/** list with the running clientAttackJobs */
	cList<cClientAttackJob*> attackJobs;
	/** List with all active movejobs */
	cList<cClientMoveJob*> ActiveMJobs;
	/** the map */
	cMap *Map;
	/** the hud */
	cGameGUI gameGUI;
	/** true if the turn should be end after all movejobs have been finished */
	bool bWantToEnd;
	/** true if allian technologies are activated */
	bool bAlienTech;
	/** will be incremented by the Timer */
	unsigned int iTimerTime;
	/** diffrent timers */
	bool timer50ms, timer100ms, timer400ms;
	/** shows if the player has to wait for other players */
	bool bWaitForOthers;
	bool waitReconnect;

	/**
	* handles the timers timer50ms, timer100ms and timer400ms
	*@author alzi alias DoctorDeath
	*/
	void handleTimer();
	/**
	* creates a new moveJob an transmits it to the server
	* @param vehicle the vehicle to be moved
	* @param iDestOffset the Destination
	*/
	void addMoveJob(cVehicle* vehicle, int iDestOffset, cList<cVehicle*> *group = NULL);
	void startGroupMove ();
	/**
	* adds an new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob ( cClientMoveJob *MJob );
	/**
	* deletes the unit
	*@author alzi alias DoctorDeath
	*@param Building Building which should be deleted.
	*@param Vehicle Vehicle which should be deleted.
	*/
	void deleteUnit( cBuilding *Building );
	void deleteUnit( cVehicle *Vehicle );
	/**
	* sends the netMessage to the server.
	* do not try to delete a message after calling this function!
	*@author Eiko
	*@param message The netMessage to be send.
	*/
	void sendNetMessage ( cNetMessage *message );
	/**
	* gets the vehicle with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the vehicle
	*/
	cVehicle *getVehicleFromID ( int iID );
	cBuilding *getBuildingFromID ( int iID );

	/**
	* initialises this client for the player.
	*@author alzi alias DoctorDeath
	*@param Player The player.
	*/
	void initPlayer( cPlayer *Player );
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
	int HandleNetMessage( cNetMessage* message );
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
	void addFX( eFXTyps typ, int iX, int iY, int iParam );
	void addFX ( eFXTyps typ,int x,int y, cClientAttackJob* aj, int iDestOff, int iFireDir );
	void addFX( sFX* iNum );

	/**
	* increments the iTimeTimer.
	*@author alzi alias DoctorDeath
	*/
	void Timer();
	/**
	* Adds an message to be displayed in the game
	*/
	void addMessage ( string sMsg );
	/** displays a message with 'goto' coordinates */
	string addCoords (const string msg,int x,int y );
	/**
	*destroys a unit
	*play FX, add rubble and delete Unit
	*/
	void destroyUnit( cVehicle* vehicle );
	void destroyUnit( cBuilding* building );

	void checkVehiclePositions( cNetMessage* message );
} EX *Client;

#endif
