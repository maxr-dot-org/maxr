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
#include "hud.h"
#include "netmessage.h"
#include "attackJobs.h"

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
		int StartFrame;
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
	friend class cHud;
	friend class cPlayer;
	friend class cBuilding;
	friend class cVehicle;

	/** List with all players */
	cList<cPlayer*> *PlayerList;
	/** if a player has entered a chat */
	string sPlayerCheat;
	/** ID of the timer */
	SDL_TimerID TimerID;
	/** will be incremented by the Timer */
	unsigned int iTimerTime;
	/**  the soundstream of the selected unit */
	int iObjectStream;
	/** Object that is under the mouse cursor */
	sGameObjects *OverObject;
	/** the acctual blink color */
	unsigned int iBlinkColor;
	/** the FLC-Animation */
	FLI_Animation *FLC;
	/** name of the FLC-Animation */
	string sFLCname;
	/** videosurface for buildings */
	SDL_Surface *video;
	/** true if the help cursor should be shown */
	bool bHelpActive;
	/** true if the name of a unit is being changed */
	bool bChangeObjectName;
	/** true if a chat message is being entered */
	bool bChatInput;
	/** list with all messages */
	cList<sMessage*> messages;
	/** number of current turn */
	int iTurn;
	/** true if the turn should be end after all movejobs have been finished */
	bool bWantToEnd;
	/** lists with all FX-Animation */
	cList<sFX*> FXList;
	cList<sFX*> FXListBottom;
	/** list with the dirt */
	cBuilding *DirtList;
	/** direction from which the wind is comming */
	float fWindDir;
	/** flags what should be displaxed in the raffinery */
	bool bUpShowTank, bUpShowPlane, bUpShowShip, bUpShowBuild, bUpShowTNT;
	/** Coordinates to a important message */
	int iMsgCoordsX, iMsgCoordsY;
	/** true if the player has been defeated */
	bool bDefeated;
	/** show infos about the running attackjobs */
	bool bDebugAjobs;
	/** show infos about the bases of the server. Only works on the host */
	bool bDebugBaseServer;
	/** show infos about the bases of the client */
	bool bDebugBaseClient;
	/** show infos about the sentrys */
	bool bDebugWache;
	/** show FX-infos */
	bool bDebugFX;
	/** show infos from the server about the unit under the mouse */
	bool bDebugTraceServer;
	/** show infos from the client about the unit under the mouse */
	bool bDebugTraceClient;
	/** offset for the debug informations on the top of the gamewindow */
	int iDebugOff;
	/** how many seconds will be left for this turn */
	int iTurnTime;
	/** Ticks when the TurnTime has been started */
	unsigned int iStartTurnTime;
	/** List with all active movejobs */
	cList<cMJobs*> ActiveMJobs;

	/**
	* checks the input of the player
	*@author alzi alias DoctorDeath
	*@return -1 if game should be closed, else 0.
	*/
	int checkUser();
	/**
	* draws the minimap in the hud
	*@author alzi alias DoctorDeath
	*/
	void drawMiniMap();
	/**
	* draws the video in the hud
	*@author alzi alias DoctorDeath
	*/
	void drawFLC();
	/**
	* displays the effects
	*@author alzi alias DoctorDeath
	*/
	void displayFX();
	/**
	* displays the bottom-effects
	*@author alzi alias DoctorDeath
	*/
	void displayFXBottom();
	/**
	* draws an effect
	*@author alzi alias DoctorDeath
	*@param iNum Number of effect to draw
	*/
	void drawFX( int iNum );
	/**
	* draws an bottom-effect
	*@author alzi alias DoctorDeath
	*@param iNum Number of effect to draw
	*/
	void drawFXBottom( int iNum );
	/**
	* draws a circle
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param iColor color of the circle
	*@param surface surface to drae the circle in
	*/
	void drawCircle( int iX, int iY, int iRadius, int iColor, SDL_Surface *surface );
	/**
	* draws an exitpoint on the ground
	*@author alzi alias DoctorDeath
	*@param iX X coordinate
	*@param iY Y coordinate
	*/
	void drawExitPoint( int iX, int iY );
	/**
	* sets a new wind direction
	*@author alzi alias DoctorDeath
	*@param iDir new direction
	*/
	void setWind( int iDir );
	/**
	* opens or closes the panal over the hud
	*@author alzi alias DoctorDeath
	*@param bOpen if true the panal will be opened, else closed
	*/
	void makePanel( bool bOpen );
	/**
	* sets the next blink color for effects
	*@author alzi alias DoctorDeath
	*/
	void rotateBlinkColor();
	/**
	* Adds an message to be displayed in the game
	*/
	void addMessage ( string sMsg );
	/**
	* handles the game messages
	*@author alzi alias DoctorDeath
	*/
	void handleMessages();
	/**
	* checks whether the input is a comman
	*@author alzi alias DoctorDeath
	*@param sCmd the input string
	*/
	bool doCommand ( string sCmd );

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
	void addUnit( int iPosX, int iPosY, cVehicle *AddedVehicle, bool bInit = false );
	void addUnit( int iPosX, int iPosY, cBuilding *AddedBuilding, bool bInit = false );
	/**
	* returns the player with the given number
	*@author alzi alias DoctorDeath
	*@param iNum The number of the player.
	*@return The wanted player.
	*/
	cPlayer *getPlayerFromNumber ( int iNum );
	/**
	* deletes the unit
	*@author alzi alias DoctorDeath
	*@param Building Building which should be deleted.
	*@param Vehicle Vehicle which should be deleted.
	*/
	void deleteUnit( cBuilding *Building );
	void deleteUnit( cVehicle *Vehicle );
	/**
	* handles the end of a turn
	*@author alzi alias DoctorDeath
	*/
	void handleEnd();
	/**
	* checks whether there are some vehicles that have to move before this turn is ending.
	*@author alzi alias DoctorDeath
	*@return true if there were some vehicles which had to move.
	*/
	bool checkEndActions();
	/**
	* handles the end of a hotseat game
	*@author alzi alias DoctorDeath
	*@param iNextPlayerNum Number of Player who has ended his turn
	*/
	void makeHotSeatEnd( int iNextPlayerNum );
	/**
	* waits until this player gets a message, that he can stop waiting. This function is not finished!
	*@author alzi alias DoctorDeath
	*@param iPlayerNum Number of player for who this player has to wait
	*/
	void waitForOtherPlayer( int iPlayerNum );
	/**
	* handles the rest-time of the current turn
	*@author alzi alias DoctorDeath
	*/
	void handleTurnTime();
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
	void handleMoveJobs ();
	/**
	* moves a vehicle one step closer to the next field
	*@author alzi alias DoctorDeath
	*@param Vehicle The vehicle to be moved
	*/
	void moveVehicle( cVehicle *Vehicle );
	/**
	* sets a vehicle to the next waypoint and makes it ready for the next move
	*@author alzi alias DoctorDeath
	*/
	void doEndMoveVehicle ( cVehicle *Vehicle );
	/**
	* shows the information for the field under the mouse
	*@author alzi alias DoctorDeath
	*/
	void trace ();
	/**
	* displays information about the vehicle on the screen
	*@author alzi alias DoctorDeath
	*@param Vehicle The vehicle for that the information should be displayed
	*@param iY pointer to the Y coords where the text should be drawn. this value will be increased
	*@param iX The X coords where the text should be drawn
	*/
	void traceVehicle ( cVehicle *Vehicle, int *iY, int iX );
	/**
	* displays information about the building on the screen
	*@author alzi alias DoctorDeath
	*@param Building The building for that the information should be displayed
	*@param iY pointer to the Y coords where the text should be drawn. this value will be increased
	*@param iX The X coords where the text should be drawn
	*/
	void traceBuilding ( cBuilding *Building, int *iY, int iX );
	/**
	* continues the path of a building vehicle
	*@author alzi alias DoctorDeath
	*@param Vehicle The vehicle that will continue its path
	*/
	void continuePathBuilding ( cVehicle *Vehicle );
	sSubBase *getSubBaseFromID ( int iID );
public:
	/** framecounter for the animations */
	unsigned int iFrame;
	/** the active Player */
	cPlayer *ActivePlayer;
	/** list with the running clientAttackJobs */
	cList<cClientAttackJob*> attackJobs;
	/** the map */
	cMap *Map;
	/** the hud */
	cHud Hud;
	/** the currently selected vehicle */
	cVehicle *SelectedVehicle;
	/** the currently selected building */
	cBuilding *SelectedBuilding;
	/** true if allian technologies are activated */
	bool bAlienTech;
	/** true if the client should end */
	bool bExit;
	/** diffrent timers */
	int iTimer0, iTimer1, iTimer2;
	/** shows if the player has to wait for other players */
	bool bWaitForOthers;

	/** flag if the hud has to be drawn */
	bool bFlagDrawHud;
	/** flag if the map has to be drawn */
	bool bFlagDrawMap;
	/** flag if anything has to be drawn  */
	bool bFlagDraw;
	/** flag if the minimap has to be drawn  */
	bool bFlagDrawMMap;

	/**
	* handles the timers iTimer0, iTimer1 and iTimer2
	*@author alzi alias DoctorDeath
	*/
	void handleTimer();
	/**
	* creates a new moveJob an transmits it to the server
	* @param vehicle the vehicle to be moved
	* @param iDestOffset the Destination
	*/
	void addMoveJob(cVehicle* vehicle, int iDestOffset);
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
	* draws the map and everything on it
	*@author alzi alias DoctorDeath
	*@param bPure if true the map is drawn without the things on it.
	*/
	void drawMap( bool bPure = false );

	/**
	* initialises this client for the player.
	*@author alzi alias DoctorDeath
	*@param Player The player.
	*/
	void initPlayer( cPlayer *Player );
	/**
	* will be called when the mouse is moved.
	*@author alzi alias DoctorDeath
	*@param bForce force to redraw the mouse even when the mouse has not been moved.
	*/
	void mouseMoveCallback ( bool bForce );
	/**
	* runs the client.
	*@author alzi alias DoctorDeath
	*/
	void run();
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
	/** displays a message with 'goto' coordinates */
	void addCoords (const string msg,int x,int y );
	/**
	* releases a movejob to be deleted
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be released
	*/
	void releaseMoveJob ( cMJobs *MJob );
} EX *Client;

#endif
