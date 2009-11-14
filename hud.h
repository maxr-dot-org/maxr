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

// TODO-list for the gameGUI:
//
// - change code that hud-background will not be drawn every frame (makes ~20fps on this machine!)
// - find a good way do display the chat-box
// - reimplement extra-players
// - reimplement rightmousesbuttonscroll
// - add documentation
//

#define HUD_LEFT_WIDTH		180
#define HUD_RIGHT_WIDTH		12
#define HUD_TOTAL_WIDTH		(HUD_LEFT_WIDTH+HUD_RIGHT_WIDTH)
#define HUD_TOP_HIGHT		18
#define HUD_BOTTOM_HIGHT	14
#define HUD_TOTAL_HIGHT		(HUD_TOP_HIGHT+HUD_BOTTOM_HIGHT)

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

	sHudStateContainer() : tntChecked(false), hitsChecked(false), lockChecked(false), surveyChecked(false),
		statusChecked(false), scanChecked(false), rangeChecked(false), twoXChecked(false), fogChecked(false),
		ammoChecked(false), gridChecked(false), colorsChecked(false),
		zoom(0.0), offX(0), offY(0), selUnitID(0) {}
};

struct sHudPosition
{
	int offsetX, offsetY;
	sHudPosition() : offsetX(-1), offsetY(-1) {}
};

struct sMouseBox
{
	float startX, startY;
	float endX, endY;
	bool isTooSmall();
};

enum eMouseInputMode
{
	normalInput,
	attackMode,
	placeBand,
	transferMode,
	loadMode,
	muniActive,
	repairActive,
	activateVehicle,
	disableMode,
	stealMode
};

class cGameGUI : public cMenu
{
	SDL_Surface *panelTopGraphic, *panelBottomGraphic;

	/** the currently selected vehicle */
	cVehicle *selectedVehicle;
	/** the currently selected group of vehicles */
	cList<cVehicle*> selectedVehiclesGroup;
	/** the currently selected building */
	cBuilding *selectedBuilding;

	cPlayer *player;
	cMap *map;

	float minZoom;

	bool closed;

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
	FLI_Animation *FLC;
	/** true when the FLC-animation should be played */
	bool playFLC;

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

	bool helpActive;

	bool needMapDraw, needMiniMapDraw;

	/** drawing cache to speed up the graphic engine */
	cDrawingCache dCache;

	cMapField *overUnitField;
	sMouseBox mouseBox;
	sMouseBox rightMouseBox;
	sMouseState savedMouseState;
	/** Saved positions for hotkeys F5-F8 */
	sHudPosition savedPositions[4];

	SDL_Surface *generateSurface();
	SDL_Surface *generateMiniMapSurface();
	bool loadPanelGraphics();

	void preDrawFunction();

	void drawTerrain( int zoomOffX, int zoomOffY );
	void drawGrid( int zoomOffX, int zoomOffY );
	void drawBaseUnits( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawTopBuildings( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawShips( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawAboveSeaBaseUnits( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawVehicles( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawConnectors( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawPlanes( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawResources( int startX, int startY, int endX, int endY, int zoomOffX, int zoomOffY );
	void drawDebugSentry();
	void drawSelectionBox( int zoomOffX, int zoomOffY );
	void drawUnitCircles();
	void drawDebugOutput();

	void displayMessages();

	void scaleSurfaces();
	void scaleColors();

	/**
	* opens or closes the panal over the hud
	*@author alzi alias DoctorDeath
	*@param bOpen if true the panal will be opened, else closed
	*/
	void makePanel( bool open );

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
	* displays the effects
	*@author alzi alias DoctorDeath
	*/
	void displayFX();
	/**
	* displays the bottom-effects
	*@author alzi alias DoctorDeath
	*/
	void displayBottomFX();

	bool checkScroll();
	void updateUnderMouseObject();
	void handleMouseMove();
	void handleMouseInputExtended( sMouseState mouseState );
	void handleFramesPerSecond();
	/**
	* sets the next blink color for effects
	*@author alzi alias DoctorDeath
	*/
	void rotateBlinkColor();
	void doScroll( int dir );
	/**
	* checks whether the input is a comman
	*@author alzi alias DoctorDeath
	*@param sCmd the input string
	*/
	void doCommand( string cmd );
	/**
	* sets a new wind direction
	*@author alzi alias DoctorDeath
	*@param iDir new direction
	*/
	void setWind( int dir );
	void changeWindDir();
	bool selectUnit( cMapField *OverUnitField, bool base );
	/**
	* selects all vehicles which are within the mousebox
	*@author alzi alias DoctorDeath
	*/
	void selectBoxVehicles ( sMouseBox &box );
	/**
	* deselects all group selected vehicles
	*@author alzi alias DoctorDeath
	*/
	void deselectGroup ();

	void updateStatusText();

	cMenuSlider *zoomSlider;

	cMenuButton *endButton;

	cMenuButton *preferencesButton;
	cMenuButton *filesButton;

	cMenuButton *playButton;
	cMenuButton *stopButton;

	cMenuImage *FLCImage;
	cMenuUnitDetails *unitDetails;

	cMenuCheckButton *surveyButton;
	cMenuCheckButton *hitsButton;
	cMenuCheckButton *scanButton;
	cMenuCheckButton *statusButton;
	cMenuCheckButton *ammoButton;
	cMenuCheckButton *gridButton;
	cMenuCheckButton *colorButton;
	cMenuCheckButton *rangeButton;
	cMenuCheckButton *fogButton;

	cMenuCheckButton *lockButton;

	cMenuCheckButton *TNTButton;
	cMenuCheckButton *twoXButton;

	cMenuButton *helpButton;
	cMenuButton *centerButton;
	cMenuButton *reportsButton;
	cMenuButton *chatButton;
	cMenuButton *nextButton;
	cMenuButton *prevButton;
	cMenuButton *doneButton;

	cMenuImage *miniMapImage;

	cMenuLabel *coordsLabel;
	cMenuLabel *unitNameLabel;
	cMenuLabel *turnLabel;
	cMenuLabel *timeLabel;

	cMenuChatBox *chatBox;

	cMenuLabel *infoTextLabel, *infoTextAdditionalLabel;

	cMenuLabel *selUnitStatusStr;
	cMenuLineEdit *selUnitNameEdit;

	static void helpReleased( void *parent );
	static void centerReleased( void *parent );
	static void reportsReleased( void *parent );
	static void chatReleased( void *parent );
	static void nextReleased( void *parent );
	static void prevReleased( void *parent );
	static void doneReleased( void *parent );

	static void changedMiniMap( void *parent );

	static void miniMapClicked( void *parent );
	static void miniMapMovedOver( void *parent );

	static void zoomSliderMoved( void *parent );

	static void endReleased( void *parent );

	static void preferencesReleased( void *parent );
	static void filesReleased( void *parent );

	static void playReleased( void *parent );
	static void stopReleased( void *parent );

	static void chatBoxReturnPressed( void *parent );
public:
	cGameGUI( cPlayer *player_, cMap *map_ );
	~cGameGUI();

	int show();
	void returnToCallback();

	void handleKeyInput( SDL_KeyboardEvent &key, string ch );

	bool surveyChecked() { return surveyButton->isChecked(); }
	bool hitsChecked() { return hitsButton->isChecked(); }
	bool scanChecked() { return scanButton->isChecked(); }
	bool statusChecked() { return statusButton->isChecked(); }
	bool ammoChecked() { return ammoButton->isChecked(); }
	bool gridChecked() { return gridButton->isChecked(); }
	bool colorChecked() { return colorButton->isChecked(); }
	bool rangeChecked() { return rangeButton->isChecked(); }
	bool fogChecked() { return fogButton->isChecked(); }
	bool lockChecked() { return lockButton->isChecked(); }
	bool tntChecked() { return TNTButton->isChecked(); }
	bool twoXChecked() { return twoXButton->isChecked(); }

	void setSurvey ( bool checked ) { surveyButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setHits ( bool checked ) { hitsButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setScan ( bool checked ) { scanButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setStatus ( bool checked ) { statusButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setAmmo ( bool checked ) { ammoButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setGrid ( bool checked ) { gridButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setColor ( bool checked ) { colorButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setRange ( bool checked ) { rangeButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setFog ( bool checked ) { fogButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setLock ( bool checked ) { lockButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setTNT ( bool checked ) { TNTButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }
	void setTwoX ( bool checked ) { twoXButton->setChecked ( checked ); PlayFX ( SoundData.SNDHudSwitch ); }

	/**
	* draws an effect
	*@author alzi alias DoctorDeath
	*@param iNum Number of effect to draw
	*/
	// TODO: find a solution that this function will not need to be public.
	// yet it is public becouse the client has to call the drawing for rockets out of cClient::runFX()
	void drawFX( int num );
	/**
	* draws an bottom-effect
	*@author alzi alias DoctorDeath
	*@param iNum Number of effect to draw
	*/
	void drawBottomFX( int num );

	/**
	* draws an exitpoint on the ground
	*@author alzi alias DoctorDeath
	*@param iX X coordinate
	*@param iY Y coordinate
	*/
	void drawExitPoint ( int x, int y );
	void callMiniMapDraw();
	void setEndButtonLock( bool locked );

	void setOffsetPosition ( int x, int y );
	int getOffsetX() { return offX; }
	int getOffsetY() { return offY; }

	void setZoom( float newZoom, bool setScroller );
	float getZoom();
	int getTileSize();

	void setVideoSurface ( SDL_Surface *videoSurface );
	void setFLC ( FLI_Animation *FLC_ );
	FLI_Animation *getFLC() { return FLC; }

	cDrawingCache *getDCache() { return &dCache; }

	unsigned int getFrame() { return frame; }
	unsigned int getBlinkColor() { return blinkColor; }

	bool getAJobDebugStatus() { return debugAjobs; }

	float getWindDir() { return windDir; }

	void setPlayer ( cPlayer *player_ );

	void setUnitDetailsData ( cVehicle *vehicle, cBuilding *building );

	void updateTurn ( int turn );
	void updateTurnTime ( int time );

	void updateMouseCursor();

	void incFrame();

	void setStartup ( bool startup_ );

	cVehicle *getSelVehicle() { return selectedVehicle; }
	cList<cVehicle*> *getSelVehiclesGroup() { return &selectedVehiclesGroup; }
	cBuilding *getSelBuilding() { return selectedBuilding; }

	void selectUnit( cVehicle *vehicle );
	void selectUnit( cBuilding *building );
	void deselectUnit();
	

	void setInfoTexts ( string infoText, string additionalInfoText );

	/**
	* activates 'mode' if not active, activates 'normalInput' otherwise
	*@author eiko
	*/
	void toggleMouseInputMode( eMouseInputMode mode );

	/**
	* checks, whether the selected action in mouseInputMode is possible and resets the mode to normal otherwise.
	*@author eiko
	*/
	void checkMouseInputMode();

	bool unitMenuActive;
	eMouseInputMode mouseInputMode;
};

#endif
