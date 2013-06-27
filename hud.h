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
#ifndef hudH
#define hudH
#include "SDL_flic.h"
#include "defines.h"
#include "menus.h"
#include "drawingcache.h"

class cClient;
class cFx;

// TODO-list for the gameGUI:
//
// - change code that hud-background will not be drawn every frame (makes ~20fps on this machine!)
// - find a good way do display the chat-box
// - reimplement extra-players
// - add documentation
//

#define HUD_LEFT_WIDTH		180
#define HUD_RIGHT_WIDTH		12
#define HUD_TOTAL_WIDTH		(HUD_LEFT_WIDTH+HUD_RIGHT_WIDTH)
#define HUD_TOP_HIGHT		18
#define HUD_BOTTOM_HIGHT	14
#define HUD_TOTAL_HIGHT		(HUD_TOP_HIGHT+HUD_BOTTOM_HIGHT)

#define MAX_SAVE_POSITIONS	4


/** structure for the messages displayed in the game */
struct sMessage
{
public:
	sMessage (std::string const&, unsigned int age);
	~sMessage();

public:
	char*        msg;
	int          chars;
	int          len;
	unsigned int age;
};

struct sHudStateContainer
{
	bool tntChecked;
	bool hitsChecked;
	bool lockChecked;
	bool surveyChecked;
	bool statusChecked;
	bool scanChecked;
	bool rangeChecked;
	bool twoXChecked;
	bool fogChecked;
	bool ammoChecked;
	bool gridChecked;
	bool colorsChecked;
	float zoom;
	int offX, offY;
	int selUnitID;

	sHudStateContainer() : tntChecked (false), hitsChecked (false), lockChecked (false), surveyChecked (false),
		statusChecked (false), scanChecked (false), rangeChecked (false), twoXChecked (false), fogChecked (false),
		ammoChecked (false), gridChecked (false), colorsChecked (false),
		zoom (0.0), offX (0), offY (0), selUnitID (0) {}
};

struct sHudPosition
{
	int offsetX, offsetY;
	sHudPosition() : offsetX (-1), offsetY (-1) {}
};

struct sMouseBox
{
	sMouseBox();
	bool isTooSmall() const;
	bool isValid() const;
	void invalidate();

	float startX, startY;
	float endX, endY;
};

enum eMouseInputMode
{
	normalInput,
	mouseInputAttackMode,
	placeBand,
	transferMode,
	loadMode,
	muniActive,
	repairActive,
	activateVehicle,
	disableMode,
	stealMode
};


/**
 * This class draws all the debug output on the screen. It is an seperate class, so you can add an "friend class cDebugOutput;" to the class,
 * which contains the data to display. So there is no need to make members public only to use them in the debug output.
 *@author eiko
 */
class cDebugOutput
{
	cServer* server;
	cClient* client;
public:
	/** show infos about the running attackjobs */
	bool debugAjobs;
	/** show infos about the bases of the server. Only works on the host */
	bool debugBaseServer;
	/** show infos about the bases of the client */
	bool debugBaseClient;
	/** show infos about the sentries */
	bool debugSentry;
	/** show FX-infos */
	bool debugFX;
	/** show infos from the server about the unit under the mouse */
	bool debugTraceServer;
	/** show infos from the client about the unit under the mouse */
	bool debugTraceClient;
	/** show infos from the client about the unit under the mouse */
	bool debugPlayers;
	/** show FPS information */
	bool showFPS;
	/** show drawing cache debug information */
	bool debugCache;
	bool debugSync;

	cDebugOutput();
	void setServer (cServer* server_);
	void setClient (cClient* client_);
	void draw();
private:
	void trace();
	void traceVehicle (const cVehicle& vehicle, int* iY, int iX);
	void traceBuilding (const cBuilding& Building, int* iY, int iX);
};

class cGameGUI : public cMenu
{
	friend class cDebugOutput;

	cClient* client;
	SDL_Surface* panelTopGraphic, *panelBottomGraphic;

	/** the currently selected unit */
	cUnit* selectedUnit;
	/** the currently selected group of vehicles */
	std::vector<cVehicle*> selectedVehiclesGroup;
public:
	/** the soundstream of the selected unit */
	int iObjectStream;
private:
	/** list with all messages */
	std::vector<sMessage*> messages;
	/** Coordinates to a important message */
	int msgCoordsX, msgCoordsY;

	cPlayer* player;
	cMap* map;

	float minZoom;

	float calcMinZoom();

	int offX, offY;
	int miniMapOffX, miniMapOffY;
	float zoom;

	/** direction from which the wind is comming */
	float windDir;

	/** framecounter for the animations */
	unsigned int frame;
	/** number of drawn frames per second */
	float framesPerSecond;
	/** number of main loop executions per second */
	float cyclesPerSecond;
	unsigned int loadValue;

	/** the acctual blink color */
	unsigned int blinkColor;

	bool startup;

	/** the FLC-Animation */
	FLI_Animation* FLC;
	/** true when the FLC-animation should be played */
	bool playFLC;

	cDebugOutput debugOutput;

	/** displays additional information about the players in the game */
	bool showPlayers;

	bool helpActive;

	bool needMiniMapDraw;

	bool shiftPressed;

	/** drawing cache to speed up the graphic engine */
	cDrawingCache dCache;

	cMapField* overUnitField;
	sMouseBox mouseBox;
	sMouseBox rightMouseBox;
	sMouseState savedMouseState;
	/** Saved positions for hotkeys F5-F8 */
	sHudPosition savedPositions[MAX_SAVE_POSITIONS];

	/** lists with all FX-Animations. Gui-only (= not synchonous to game time) */
	std::vector<cFx*> FxList;

	SDL_Surface* generateMiniMapSurface();
	bool loadPanelGraphics();

	void preDrawFunction();

	void drawAttackCursor (int x, int y) const;
	void drawTerrain (int zoomOffX, int zoomOffY);
	void drawGrid (int zoomOffX, int zoomOffY);
	void drawBaseUnits (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawTopBuildings (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawTopBuildings_DebugBaseClient (const cBuilding& building, const SDL_Rect& dest);
	void drawTopBuildings_DebugBaseServer (const cBuilding& building, const SDL_Rect& dest, unsigned int offset);
	void drawShips (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawAboveSeaBaseUnits (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawVehicles (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawConnectors (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawPlanes (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawResources (int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY);
	void drawSelectionBox (int zoomOffX, int zoomOffY);
	void drawUnitCircles();
	void drawLockList (cPlayer& player);

	void displayMessages();

	void scaleSurfaces();
	void scaleColors();

	/** handles the game messages */
	void handleMessages();

	/**
	* opens or closes the panal over the hud
	*@author alzi alias DoctorDeath
	*@param bOpen if true the panal will be opened, else closed
	*/
	void makePanel (bool open);

	bool checkScroll();
	void updateUnderMouseObject();
	void handleMouseMove();
	void handleMouseInputExtended (sMouseState mouseState);
	void handleFramesPerSecond();
	/**
	* sets the next blink color for effects
	*@author alzi alias DoctorDeath
	*/
	void rotateBlinkColor();
	void doScroll (int dir);
	/**
	* checks whether the input is a command
	*@author alzi alias DoctorDeath
	*@param sCmd the input string
	*/
	void doCommand (const std::string& cmd);
	/**
	* sets a new wind direction
	*@author alzi alias DoctorDeath
	*@param iDir new direction
	*/
	void setWind (int dir);
	void changeWindDir();
	bool selectUnit (cMapField* OverUnitField, bool base);
	/**
	* selects all vehicles which are within the mousebox
	*@author alzi alias DoctorDeath
	*/
	void selectBoxVehicles (sMouseBox& box);
	/**
	* deselects all group selected vehicles
	*@author alzi alias DoctorDeath
	*/
	void deselectGroup();

	void updateStatusText();

	void resetMiniMapOffset();

	void savePosition (int slotNumber);
	void jumpToSavedPos (int slotNumber);

	cMenuSlider zoomSlider;

	cMenuButton endButton;

	cMenuButton preferencesButton;
	cMenuButton filesButton;

	cMenuButton playButton;
	cMenuButton stopButton;

	cMenuImage FLCImage;
	cMenuUnitDetails unitDetails;

	cMenuCheckButton surveyButton;
	cMenuCheckButton hitsButton;
	cMenuCheckButton scanButton;
	cMenuCheckButton statusButton;
	cMenuCheckButton ammoButton;
	cMenuCheckButton gridButton;
	cMenuCheckButton colorButton;
	cMenuCheckButton rangeButton;
	cMenuCheckButton fogButton;

	cMenuCheckButton lockButton;

	cMenuCheckButton TNTButton;
	cMenuCheckButton twoXButton;
	cMenuCheckButton playersButton;

	cMenuButton helpButton;
	cMenuButton centerButton;
	cMenuButton reportsButton;
	cMenuButton chatButton;
	cMenuButton nextButton;
	cMenuButton prevButton;
	cMenuButton doneButton;

	cMenuImage miniMapImage;

	cMenuLabel coordsLabel;
	cMenuLabel unitNameLabel;
	cMenuLabel turnLabel;
	cMenuLabel timeLabel;

	cMenuChatBox chatBox;

	cMenuLabel infoTextLabel;
	cMenuLabel infoTextAdditionalLabel;

	cMenuLabel selUnitStatusStr;
	cMenuLabel selUnitNamePrefixStr;
	cMenuLineEdit selUnitNameEdit;

	std::vector<cMenuPlayerInfo*> playersInfo;

	static void helpReleased (void* parent);
	static void centerReleased (void* parent);
	static void reportsReleased (void* parent);
	static void chatReleased (void* parent);
	static void nextReleased (void* parent);
	static void prevReleased (void* parent);
	static void doneReleased (void* parent);

	static void twoXReleased (void* parent);

	static void playersReleased (void* parent);

	static void changedMiniMap (void* parent);

	static void miniMapClicked (void* parent);
	static void miniMapRightClicked (void* parent);
	static void miniMapMovedOver (void* parent);

	static void zoomSliderMoved (void* parent);

	static void endReleased (void* parent);

	static void preferencesReleased (void* parent);
	static void filesReleased (void* parent);

	static void playReleased (void* parent);
	static void stopReleased (void* parent);
	static void swithAnimationReleased (void* parent);

	static void chatBoxReturnPressed (void* parent);

	static void unitNameReturnPressed (void* parent);

	void recalcPosition (bool resetItemPositions);
	void setInfoTexts (const std::string& infoText, const std::string& additionalInfoText);

	void drawFx (bool bottom) const;
	void runFx ();

	void selectUnit_vehicle (cVehicle& vehicle);
	void selectUnit_building (cBuilding& building);

public:
	explicit cGameGUI (cMap* map_);
	~cGameGUI();

	// this means every 100ms because iTimerTime will increase every 50ms.
	unsigned int getAnimationSpeed() const { return iTimerTime / 2; }

	/** SDL_Timer for animations */
	SDL_TimerID TimerID;
	/** will be incremented by the Timer */
	unsigned int iTimerTime;
	/** gui timers for animations only */
	bool timer10ms, timer50ms, timer100ms, timer400ms;
	void handleTimer();
	void Timer();

	void addFx (cFx* fx);

	/** Adds an message to be displayed in the game */
	void addMessage (const std::string& sMsg);
	/** displays a message with 'goto' coordinates */
	std::string addCoords (const std::string& msg, int x, int y);

	void setClient (cClient* client);
	virtual int show (cClient* client);

	void updateInfoTexts();

	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);

	const cClient* getClient() const { return client; }
	cClient* getClient() { return client; }
	bool surveyChecked() const { return surveyButton.isChecked(); }
	bool hitsChecked() const { return hitsButton.isChecked(); }
	bool scanChecked() const { return scanButton.isChecked(); }
	bool statusChecked() const { return statusButton.isChecked(); }
	bool ammoChecked() const { return ammoButton.isChecked(); }
	bool gridChecked() const { return gridButton.isChecked(); }
	bool colorChecked() const { return colorButton.isChecked(); }
	bool rangeChecked() const { return rangeButton.isChecked(); }
	bool fogChecked() const { return fogButton.isChecked(); }
	bool lockChecked() const { return lockButton.isChecked(); }
	bool tntChecked() const { return TNTButton.isChecked(); }
	bool twoXChecked() const { return twoXButton.isChecked(); }

	void setSurvey (bool checked) { surveyButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setHits (bool checked) { hitsButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setScan (bool checked) { scanButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setStatus (bool checked) { statusButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setAmmo (bool checked) { ammoButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setGrid (bool checked) { gridButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setColor (bool checked) { colorButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setRange (bool checked) { rangeButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setFog (bool checked) { fogButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setLock (bool checked) { lockButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setTNT (bool checked) { TNTButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }
	void setTwoX (bool checked) { resetMiniMapOffset(); twoXButton.setChecked (checked); PlayFX (SoundData.SNDHudSwitch); }

	/**
	* draws an exitpoint on the ground
	*@author alzi alias DoctorDeath
	*@param iX X coordinate
	*@param iY Y coordinate
	*/
	void drawExitPoint (int x, int y);
	void callMiniMapDraw();
	void setEndButtonLock (bool locked);

	void setOffsetPosition (int x, int y);
	void checkOffsetPosition();
	int getOffsetX() const { return offX; }
	int getOffsetY() const { return offY; }

	SDL_Rect calcScreenPos (int x, int y) const;

	void setZoom (float newZoom, bool setScroller, bool centerToMouse);
	float getZoom() const;
	int getTileSize() const;

	void setVideoSurface (SDL_Surface* videoSurface);
	void setFLC (FLI_Animation* FLC_);
	FLI_Animation* getFLC() { return FLC; }

	cDrawingCache* getDCache() { return &dCache; }

	unsigned int getFrame() const { return frame; }
	unsigned int getBlinkColor() const { return blinkColor; }

	bool getAJobDebugStatus() const { return debugOutput.debugAjobs; }

	float getWindDir() const { return windDir; }

	void setPlayer (cPlayer* player_);

	void setUnitDetailsData (cVehicle* vehicle, cBuilding* building);

	void updateTurn (int turn);
	void updateTurnTime (int time);

	void updateMouseCursor();

	void incFrame();

	void setStartup (bool startup_);

	std::vector<cVehicle*>* getSelVehiclesGroup() { return &selectedVehiclesGroup; }
	const cUnit* getSelectedUnit() const { return selectedUnit; }
	cUnit* getSelectedUnit() { return selectedUnit; }
	cVehicle* getSelectedVehicle();
	cBuilding* getSelectedBuilding();

	void selectUnit (cUnit& unit);
	void deselectUnit();

	/**
	* activates 'mode' if not active, activates 'normalInput' otherwise
	*@author eiko
	*/
	void toggleMouseInputMode (eMouseInputMode mode);

	/**
	* checks, whether the selected action in mouseInputMode is possible and resets the mode to normal otherwise.
	*@author eiko
	*/
	void checkMouseInputMode();
	static SDL_Surface* generateSurface();

	bool unitMenuActive;
	eMouseInputMode mouseInputMode;
};

#endif
