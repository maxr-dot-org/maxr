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
#ifndef menusH
#define menusH
#include "defines.h"
#include "menuitems.h"
#include "input.h"
#include "server.h"

int GetColorNr(SDL_Surface *sf);

struct sColor
{
	unsigned char cBlue, cGreen, cRed;
};

struct sLandingUnit
{
	sID unitID;
	int cargo;
};

enum eLandingState
{
	LANDING_STATE_UNKNOWN,		//initial state
	LANDING_POSITION_OK,		//there are no other players near the position
	LANDING_POSITION_WARNING,	//there are players within the warning distance
	LANDING_POSITION_TOO_CLOSE,	//the position is too close to another player
	LANDING_POSITION_CONFIRMED	//warnings about nearby players will be ignored, because the player has confirmed his position
};

struct sClientLandData
{
	int iLandX, iLandY;
	eLandingState landingState;
	int iLastLandX, iLastLandY;

	sClientLandData() : iLandX(0), iLandY(0), iLastLandX(0), iLastLandY(0), landingState ( LANDING_STATE_UNKNOWN ) {}
};

enum eSettingResourceValue
{
	SETTING_RESVAL_LOW,
	SETTING_RESVAL_NORMAL,
	SETTING_RESVAL_MUCH,
	SETTING_RESVAL_MOST
};

enum eSettingResFrequency
{
	SETTING_RESFREQ_THIN,
	SETTING_RESFREQ_NORMAL,
	SETTING_RESFREQ_THICK,
	SETTING_RESFREQ_MOST
};

enum eSettingsCredits
{
	SETTING_CREDITS_LOWEST = 25,
	SETTING_CREDITS_LOWER = 50,
	SETTING_CREDITS_LOW = 100,
	SETTING_CREDITS_NORMAL = 150,
	SETTING_CREDITS_MUCH = 200,
	SETTING_CREDITS_MORE = 250,
	SETTING_CREDITS_MOST = 300
};

enum eSettingsBridgeHead
{
	SETTING_BRIDGEHEAD_MOBILE,
	SETTING_BRIDGEHEAD_DEFINITE
};

enum eSettingsAlienTech
{
	SETTING_ALIENTECH_ON,
	SETTING_ALIENTECH_OFF
};

enum eSettingsGameType
{
	SETTINGS_GAMETYPE_SIMU,
	SETTINGS_GAMETYPE_TURNS
};

/**
 * A class that containes all settings for a new game.
 *@author alzi
 */
struct sSettings
{
	eSettingResourceValue metal, oil, gold;
	eSettingResFrequency resFrequency;
	eSettingsCredits credits;
	eSettingsBridgeHead bridgeHead;
	eSettingsAlienTech alienTech;
	eSettingsGameType gameType;

	sSettings() : metal(SETTING_RESVAL_LOW), oil(SETTING_RESVAL_LOW), gold(SETTING_RESVAL_LOW), resFrequency(SETTING_RESFREQ_THIN), credits(SETTING_CREDITS_LOW),
		bridgeHead (SETTING_BRIDGEHEAD_DEFINITE), alienTech(SETTING_ALIENTECH_OFF), gameType(SETTINGS_GAMETYPE_SIMU) {}

	string getResValString ( eSettingResourceValue type );
	string getResFreqString();

};

/**
 * A class that containes all information to start a new or loaded game.
 * This class can run games automaticaly out of this information.
 *@author alzi
 */
class cGameDataContainer
{
public:

	/** The type of the game. See eGameTypes*/
	eGameTypes type;

	/** Number of the savegame or -1 for no savegame*/
	int savegameNum;
	/** name of the savegame if the savefile is only on the server and this container is set by an client*/
	string savegame;

	/** The settings for the game*/
	sSettings *settings;
	/** The map for the game*/
	cMap *map;

	/** list with all players for the game*/
	cList<cPlayer*> players;
	/** list with the selected landing units by each player*/
	cList<cList<sLandingUnit>*> landingUnits;
	/** the client landing data (landing positions) of the players*/
	cList<sClientLandData*> landData;


	cGameDataContainer() : settings(NULL), map(NULL), type(GAME_TYPE_SINGLE), savegameNum(-1) {}
	~cGameDataContainer();

	/** Runs the game. If player is 0, which means that he is the host, a server will be started.
	 * Else only a client will be started. When reconnect is true, it will be reconnected to a running game.
	 * When the conatainer contains a savegamenumber, the savegame will be loaded
	 *@author alzi
	 */
	void runGame( int player, bool reconnect = false );
	/** handles incomming landing units
	 *@author alzi
	 */
	void receiveLandingUnits ( cNetMessage *message );
	/** handles incomming unit upgrades
	 *@author alzi
	 */
	void receiveUnitUpgrades ( cNetMessage *message );
	/** handles an incomming landing position
	 *@author alzi
	 */
	void receiveLandingPosition ( cNetMessage *message );

private:
	/** checks whether the landing positions are okay
	 *@author alzi
	 */
	eLandingState checkLandingState( int playerNr );
	/** loads and runs a saved game
	 *@author alzi
	 */
	void runSavedGame( int player );
};

/**
 * The main menu class. This class handles the background, the position, the input from mouse and keyboard
 * and all the menu items as buttons, images, labels, etc. All menuclasses in maxr should be a child of
 * this class.
 *@author alzi
 */
class cMenu
{
protected:
	/** When this is true the show-loop will be end and give 0 as return.
	 * Should be used when the menu is closed by ok or done.
	 */
	bool end;
	/** When this is true the show-loop will be end and give 1 as return.
	 * Should be used when the menu is closed by abort or back.
	 */
	bool terminate;

	/** The background of the menu. Can be smaller then the screen*/
	SDL_Surface *background;
	/** The position of the menu on the screen when it is smaller then the screen. The position will be
	 * calculated in the constructor of cMenu und set the the center of the screen.
	 */
	SDL_Rect position;

	/** The list with all menuitems (buttons, images, etc.) of this menu.*/
	cList<cMenuItem*> menuItems;
	/** Pointer to the currently active menuitem. This one will receive keyboard input*/
	cMenuItem *activeItem;

	/**
	 * initialises members and calculates the menu position on the screen.
	 *@author alzi
	 *@param background_ The background of the surface
	 */
	cMenu( SDL_Surface *background_ );
	/**
	 * frees the background surface. This destructor does not delete the menuitems!
	 *@author alzi
	 */
	~cMenu();

public:
	/**
	 * redraws the menu background, the cursor and all menuitems.
	 *@author alzi
	 */
	void draw();
	/**
	 * displays the menu and focuses all input on this menu until end or terminate are set to true.
	 *@author alzi
	 */
	int show();

	/**
	 * handles mouseclicks, gives them to the matching menuitem and handles the activity of the menuitems.
	 *@author alzi
	 */
	void handleMouseInput( sMouseState mouseState );
	/**
	 * gives the keyinput to the active menuitem.
	 *@author alzi
	 */
	virtual void handleKeyInput( SDL_keysym keysym, string ch );

	/**
	 * sends a netmessage to the given player.
	 *@author alzi
	 */
	static void sendMessage ( cNetMessage *message, sMenuPlayer *player = NULL );
	/**
	 * this procedure will receive the menu-net-messages when this menu is active in the moment the message
	 * has been received. If the message should be handles overwrite this virtual function.
	 *@author alzi
	 */
	virtual void handleNetMessage( cNetMessage *message ) {}
};

/** pointer to the currently active menu or NULL if no menu is active */
EX cMenu *ActiveMenu;

/**
 * A main menu with unit info image and a credits label on the bottom.
 *@author alzi
 */
class cMainMenu : public cMenu
{
	cMenuImage *infoImage;
	cMenuLabel *creditsLabel;

public:
	cMainMenu();
	~cMainMenu();

	SDL_Surface *getRandomInfoImage();
	static void infoImageReleased( void* parent );
};

/**
 * The menu in the very beginning.
 *@author alzi
 */
class cStartMenu : public cMainMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *singleButton;
	cMenuButton *multiButton;
	cMenuButton *preferenceButton;
	cMenuButton *licenceButton;
	cMenuButton *exitButton;

public:
	cStartMenu();
	~cStartMenu();

	static void singlePlayerReleased( void* parent );
	static void multiPlayerReleased( void* parent );
	static void preferencesReleased( void* parent );
	static void licenceReleased( void* parent );
	static void exitReleased( void* parent );
};

/**
 * The singleplayer menu.
 *@author alzi
 */
class cSinglePlayerMenu : public cMainMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *newGameButton;
	cMenuButton *loadGameButton;
	cMenuButton *backButton;
public:
	cSinglePlayerMenu();
	~cSinglePlayerMenu();

	static void newGameReleased( void* parent );
	static void loadGameReleased( void* parent );
	static void backReleased( void* parent );
};

/**
 * A the multiplayer menu.
 *@author alzi
 */
class cMultiPlayersMenu : public cMainMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *tcpHostButton;
	cMenuButton *tcpClientButton;
	cMenuButton *newHotseatButton;
	cMenuButton *loadHotseatButton;
	cMenuButton *backButton;
public:
	cMultiPlayersMenu();
	~cMultiPlayersMenu();

	static void tcpHostReleased( void* parent );
	static void tcpClientReleased( void* parent );
	static void newHotseatReleased( void* parent );
	static void loadHotseatReleased( void* parent );
	static void backReleased( void* parent );
};

/**
 * The settings menu.
 *@author alzi
 */
class cSettingsMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;
	sSettings settings;

	cMenuLabel *titleLabel;
	cMenuButton *okButton;
	cMenuButton *backButton;

	cMenuLabel *resourcesLabel;
	cMenuLabel *metalLabel;
	cMenuLabel *oilLabel;
	cMenuLabel *goldLabel;
	cMenuLabel *creditsLabel;
	cMenuLabel *bridgeheadLabel;
	cMenuLabel *aliensLabel;
	cMenuLabel *resFrequencyLabel;
	cMenuLabel *gameTypeLabel;

	cMenuRadioGroup *metalGroup;
	cMenuRadioGroup *oilGroup;
	cMenuRadioGroup *goldGroup;
	cMenuRadioGroup *creditsGroup;
	cMenuRadioGroup *bridgeheadGroup;
	cMenuRadioGroup *aliensGroup;
	cMenuRadioGroup *resFrequencyGroup;
	cMenuRadioGroup *gameTypeGroup;

	void updateSettings();
public:
	cSettingsMenu( cGameDataContainer *gameDataContainer_ );
	~cSettingsMenu();

	static void backReleased( void* parent );
	static void okReleased( void* parent );
};

/**
 * The planet selection.
 *@author alzi
 */
class cPlanetsSelectionMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuLabel *titleLabel;

	cMenuButton *okButton;
	cMenuButton *backButton;

	cMenuButton *arrowUpButton;
	cMenuButton *arrowDownButton;

	cMenuImage *planetImages[8];
	cMenuLabel *planetTitles[8];

	cList<string> *maps;
	int selectedMapIndex;
	int offset;

public:
	cPlanetsSelectionMenu( cGameDataContainer *gameDataContainer_ );
	~cPlanetsSelectionMenu();

	void loadMaps();
	void showMaps();

	static void backReleased( void* parent );
	static void okReleased( void* parent );
	static void arrowDownReleased( void* parent );
	static void arrowUpReleased( void* parent );
	static void mapReleased( void* parent );
};

/**
 * A standard hangar menu with one unit selection table, unit info image, unit description, unit details window
 * and two buttons (done and back).
 *@author alzi
 */
class cHangarMenu : public cMenu
{
protected:
	cPlayer *player;

	cMenuImage *infoImage;
	cMenuLabel *infoText;
	cMenuCheckButton *infoTextCheckBox;

	cMenuUnitDetails *unitDetails;

	cMenuUnitsList *selectionList;
	cMenuUnitListItem *selectedUnit;

	cMenuButton *selListUpButton;
	cMenuButton *selListDownButton;

	cMenuButton *doneButton;
	cMenuButton *backButton;

	void drawUnitInformation();

	void (*selectionChangedFunc)(void *);
public:
	cHangarMenu( SDL_Surface *background_, cPlayer *player_ );
	~cHangarMenu();

	static void infoCheckBoxClicked( void* parent );
	static void selListUpReleased( void* parent );
	static void selListDownReleased( void* parent );

	void setSelectedUnit( cMenuUnitListItem *selectedUnit_ );
	cMenuUnitListItem *getSelectedUnit();

	virtual void generateSelectionList() {}
};

/**
 * A hangar menu with a second unit table, where you can add units by double clicking in the first list.
 *@author alzi
 */
class cAdvListHangarMenu : virtual public cHangarMenu
{
protected:
	cMenuUnitsList *secondList;

	cMenuButton *secondListUpButton;
	cMenuButton *secondListDownButton;

	virtual bool checkAddOk ( cMenuUnitListItem *item ) { return true; }
	virtual void addedCallback ( cMenuUnitListItem *item ) {}
	virtual void removedCallback ( cMenuUnitListItem *item ) {}
public:
	cAdvListHangarMenu( SDL_Surface *background_, cPlayer *player_ );
	~cAdvListHangarMenu();

	static bool selListDoubleClicked( cMenuUnitsList* list, void* parent );
	static bool secondListDoubleClicked( cMenuUnitsList* list, void* parent );

	static void secondListUpReleased( void* parent );
	static void secondListDownReleased( void* parent );
};

/**
 * A upgrade hangar menu with filter checkbuttons for the unit selection list, goldbar and buttons for
 * upgrading units.
 *@author alzi
 */
class cUpgradeHangarMenu : virtual public cHangarMenu
{
protected:
	int credits;

	cMenuUpgradeFilter *upgradeFilter;
	cMenuUpgradeHandler *upgradeButtons;
	cMenuMaterialBar *goldBar;
	cMenuLabel *goldBarLabel;

	sUnitUpgrade (*unitUpgrades)[8];
	void initUpgrades( cPlayer *player );
public:
	cUpgradeHangarMenu( cPlayer *owner );
	~cUpgradeHangarMenu();

	void setCredits( int credits_ );
	int getCredits();
};

/**
 * The hangar menu where you selected your landind units in the beginning of a new game.
 *@author alzi
 */
class cStartupHangarMenu : public cUpgradeHangarMenu, public cAdvListHangarMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuRadioGroup *upgradeBuyGroup;

	cMenuMaterialBar *materialBar;

	cMenuLabel *materialBarLabel;

	cMenuButton *materialBarUpButton;
	cMenuButton *materialBarDownButton;

	bool isInLandingList( cMenuUnitListItem *item );

	bool checkAddOk ( cMenuUnitListItem *item );
	void addedCallback ( cMenuUnitListItem *item );
	void removedCallback ( cMenuUnitListItem *item );
public:
	cStartupHangarMenu( cGameDataContainer *gameDataContainer_, cPlayer *player_ );
	~cStartupHangarMenu();

	static void selectionChanged( void* parent );

	static void backReleased( void* parent );
	static void doneReleased( void* parent );
	static void subButtonsChanged( void* parent );
	static void materialBarUpReleased( void* parent );
	static void materialBarDownReleased( void* parent );
	static void materialBarClicked( void* parent );

	void handleNetMessage( cNetMessage *message );

	void generateSelectionList();
};

/**
 * The landingposition selection menu.
 *@author alzi
 */
class cLandingMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;
	
	cPlayer *player;

	cMap *map;

	SDL_Surface *hudSurface;
	SDL_Surface *mapSurface;

	cMenuImage *hudImage;
	cMenuImage *mapImage;
	cMenuImage *circlesImage;
	cMenuLabel *infoLabel;

	sClientLandData landData;

	void createHud();
	void createMap();
	sTerrain *getMapTile ( int x, int y );
	void hitPosition();
public:
	cLandingMenu( cGameDataContainer *gameDataContainer_, cPlayer *player_ );
	~cLandingMenu();

	static void mapClicked( void* parent );
	static void mouseMoved( void* parent );

	void handleKeyInput( SDL_keysym keysym, string ch );
	void handleNetMessage( cNetMessage *message );
};

/**
 * A standard menu for network TCP/IP games with ip, port, and playername lineedits,
 * chat lineedit and chat window, color selection and playerlist and map image.
 *@author alzi
 */
class cNetworkMenu : public cMenu
{
protected:
	string ip;
	int port;
	cList<sMenuPlayer*> players;
	sMenuPlayer *actPlayer;

	cMenuButton *backButton;
	cMenuButton *sendButton;

	cMenuImage *mapImage;
	cMenuLabel *mapLabel;

	cMenuLabel *settingsText;

	cMenuListBox *chatBox;
	cMenuLineEdit *chatLine;

	cMenuLabel *ipLabel;
	cMenuLabel *portLabel;
	cMenuLabel *nameLabel;
	cMenuLabel *colorLabel;

	cMenuButton *nextColorButton;
	cMenuButton *prevColorButton;
	cMenuImage *colorImage;

	cMenuLineEdit *ipLine;
	cMenuLineEdit *portLine;
	cMenuLineEdit *nameLine;

	cMenuPlayersBox *playersBox;

	cGameDataContainer gameDataContainer;
	string saveGameString;

	void showSettingsText();
	void showMap();
	void setColor( int color );
public:
	cNetworkMenu();
	~cNetworkMenu();

	void playerReadyClicked ( sMenuPlayer *player );

	static void backReleased( void* parent );
	static void sendReleased( void* parent );

	static void nextColorReleased( void* parent );
	static void prevColorReleased( void* parent );

	static void wasNameImput( void* parent );
	static void portIpChanged( void* parent );

	virtual void playerSettingsChanged () {}
};

/**
 * The host network menu.
 *@author alzi
 */
class cNetworkHostMenu : public cNetworkMenu
{
protected:
	cMenuLabel *titleLabel;

	cMenuButton *okButton;

	cMenuButton *mapButton;
	cMenuButton *settingsButton;
	cMenuButton *loadButton;
	cMenuButton *startButton;

	int checkAllPlayersReady();
	void runSavedGame();

public:
	cNetworkHostMenu();
	~cNetworkHostMenu();

	static void okReleased( void* parent );
	static void mapReleased( void* parent );
	static void settingsReleased( void* parent );
	static void loadReleased( void* parent );
	static void startReleased( void* parent );

	void handleNetMessage( cNetMessage *message );
	void playerSettingsChanged ();
};

/**
 * The client network menu
 *@author alzi
 */
class cNetworkClientMenu : public cNetworkMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *connectButton;

public:
	cNetworkClientMenu();
	~cNetworkClientMenu();

	static void connectReleased( void* parent );

	void handleNetMessage( cNetMessage *message );
	void playerSettingsChanged ();
};

/**
 * The load menu.
 *@author alzi
 */
class cLoadMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuLabel *titleLabel;

	cMenuButton *backButton;
	cMenuButton *loadButton;

	cMenuButton *upButton;
	cMenuButton *downButton;

	cMenuSaveSlot *saveSlots[10];

	cList<string> *files;
	cList<sSaveFile*> savefiles;

	int offset;
	int selected;

	void loadSaves();
	void displaySaves();

public:
	cLoadMenu( cGameDataContainer *gameDataContainer_ );
	~cLoadMenu();

	static void backReleased( void* parent );
	static void loadReleased( void* parent );

	static void upReleased( void* parent );
	static void downReleased( void* parent );

	static void slotClicked( void* parent );

	virtual void extendedSlotClicked( int oldSelection ) {}
};

/**
 * The load and save menu (ingame data menu).
 *@author alzi
 */
class cLoadSaveMenu : public cLoadMenu
{
protected:
	cMenuButton *exitButton;
	cMenuButton *saveButton;

public:
	cLoadSaveMenu( cGameDataContainer *gameDataContainer_ );
	~cLoadSaveMenu();

	static void exitReleased( void* parent );
	static void saveReleased( void* parent );

	void extendedSlotClicked( int oldSelection );
};

/**
 * The menu to build buildings.
 *@author alzi
 */
class cBuildingsBuildMenu : public cHangarMenu
{
protected:
	cVehicle *vehicle;

	cMenuLabel *titleLabel;
	cMenuButton *pathButton;
	cMenuBuildSpeedHandler *speedHandler;

public:
	cBuildingsBuildMenu( cPlayer *player_, cVehicle *vehicle_ );
	~cBuildingsBuildMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void pathReleased ( void *parent );
	static void selectionChanged ( void *parent );
	static bool selListDoubleClicked ( cMenuUnitsList* list, void *parent );

	void generateSelectionList();
};

/**
 * The menu to build vehicles.
 *@author alzi
 */
class cVehiclesBuildMenu : public cAdvListHangarMenu
{
protected:
	cBuilding *building;

	cMenuLabel *titleLabel;
	cMenuBuildSpeedHandler *speedHandler;

	cMenuCheckButton *repeatButton;

	void createBuildList();
public:
	cVehiclesBuildMenu( cPlayer *player_, cBuilding *building_ );
	~cVehiclesBuildMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void selectionChanged ( void *parent );

	void generateSelectionList();
};

/**
 * The upgrade menu.
 *@author alzi
 */
class cUpgradeMenu : public cUpgradeHangarMenu
{
protected:
public:
	cUpgradeMenu( cPlayer *player );
	~cUpgradeMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void selectionChanged ( void *parent );

	void generateSelectionList();
};

#endif //menusH
