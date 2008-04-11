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
#include "game.h"
#include "netmessage.h"

Uint32 TimerCallback(Uint32 interval, void *arg);

//enum eFXTyps {fxMuzzleBig,fxMuzzleSmall,fxMuzzleMed,fxMuzzleMedLong,fxExploSmall,fxExploBig,fxExploAir,fxExploWater,fxHit,fxSmoke,fxRocket,fxDarkSmoke,fxTorpedo,fxTracks,fxBubbles,fxCorpse,fxAbsorb};

/**
* Client class which handles the in and output for a player
*@author alzi alias DoctorDeath
*/
class cClient
{
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
	/** framecounter for the animations */
	unsigned int iFrame;
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
	cList<sMessage*> *messages;
	/** number of current turn */
	int iTurn;
	/** true if the turn should be end after all movejobs have been finished */
	bool bWantToEnd;
	/** lists with all FX-Animation */
	cList<sFX*> *FXList,*FXListBottom;
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
	/** tshow infos about the base */
	bool bDebugBase;
	/** show infos about the sentrys */
	bool bDebugWache;
	/** show FX-infos */
	bool bDebugFX;
	/** show infos about the unit under the mouse */
	bool bDebugTrace;
	/** offset for the debug informations on the top of the gamewindow */
	int iDebugOff;

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
	* draws a circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircle( int iX, int iY, int iRadius, char *map );
	/**
	* draws a big circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig( int iX, int iY, int iRadius, char *map );
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
	void addFX( eFXTyps typ, int iX, int iY, sFXRocketInfos* param );
	void addFX( sFX* iNum );
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
	* handles the end of a hotseat game
	*@author alzi alias DoctorDeath
	*@param iNextPlayerNum Number of Player who has ended his turn
	*/
	void makeHotSeatEnd( int iNextPlayerNum );
	void WaitForOtherPlayer( int iPlayerNum );
public:
	/** the active Player */
	cPlayer *ActivePlayer;
	/** the map */
	cMap *Map;
	/** the hud */
	cHud *Hud;
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
	* sends the netMessage to the server.
	* do not try to delete a message after calling this function!
	*@author Eiko
	*@param message The netMessage to be send.
	*/
	void sendNetMessage ( cNetMessage *message );
	
	/**
	* draws the map and everything on it
	*@author alzi alias DoctorDeath
	*@param bPure if true the map is drawn without the things on it.
	*/
	void drawMap( bool bPure = false );

	/**
	* initialises the client class
	*@author alzi alias DoctorDeath
	*/
	void init( cMap *Map, cList<cPlayer*> *PlayerList );
	/**
	* kills the client class
	*@author alzi alias DoctorDeath
	*/
	void kill();
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
	* processes everything that is need for this netMessage
	*@author alzi alias DoctorDeath
	*@param message The netMessage to be handled.
	*@return 0 for success
	*/
	int HandleNetMessage( cNetMessage* message );

	/**
	* increments the iTimeTimer.
	*@author alzi alias DoctorDeath
	*/
	void Timer();
} EX *Client;

#endif
