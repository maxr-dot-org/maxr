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

#include "autosurface.h"
#include "autoptr.h"
#include "defines.h"
#include "menuitems.h"
#include "input.h"
#include "server.h"
#include "base.h" // for sSubBase

class cEventHandling;
class cGameGUI;
class cMapReceiver;
class cMapSender;
class cServer;
class cTCP;

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
	LANDING_STATE_UNKNOWN,      //initial state
	LANDING_POSITION_OK,        //there are no other players near the position
	LANDING_POSITION_WARNING,   //there are players within the warning distance
	LANDING_POSITION_TOO_CLOSE, //the position is too close to another player
	LANDING_POSITION_CONFIRMED  //warnings about nearby players will be ignored, because the player has confirmed his position
};

struct sClientLandData
{
	int iLandX, iLandY;
	int iLastLandX, iLastLandY;
	eLandingState landingState;
	bool receivedOK;

	sClientLandData() : iLandX (0), iLandY (0), iLastLandX (0), iLastLandY (0), landingState (LANDING_STATE_UNKNOWN), receivedOK (false) {}
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
	SETTING_CREDITS_LOWEST = 0,
	SETTING_CREDITS_LOWER = 50,
	SETTING_CREDITS_LOW = 100,
	SETTING_CREDITS_NORMAL = 150,
	SETTING_CREDITS_MUCH = 200,
	SETTING_CREDITS_MORE = 250
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

enum eSettingsClans
{
	SETTING_CLANS_ON,
	SETTING_CLANS_OFF
};

enum eSettingsGameType
{
	SETTINGS_GAMETYPE_SIMU,
	SETTINGS_GAMETYPE_TURNS
};

enum eSettingsVictoryType
{
	SETTINGS_VICTORY_TURNS,
	SETTINGS_VICTORY_POINTS,
	SETTINGS_VICTORY_ANNIHILATION
};

enum eSettingsDuration
{
	SETTINGS_DUR_SHORT = 100,
	SETTINGS_DUR_MEDIUM = 200,
	SETTINGS_DUR_LONG = 400
};

/**
 * A class that contains all settings for a new game.
 *@author alzi
 */
struct sSettings
{
	eSettingResourceValue metal, oil, gold;
	eSettingResFrequency resFrequency;
	unsigned int credits;
	eSettingsBridgeHead bridgeHead;
	eSettingsAlienTech alienTech;
	eSettingsClans clans;
	eSettingsGameType gameType;
	eSettingsVictoryType victoryType;
	int duration;
	/** deadline in seconds when the first player has finished his turn */
	int iTurnDeadline;

	sSettings() : metal (SETTING_RESVAL_NORMAL), oil (SETTING_RESVAL_NORMAL), gold (SETTING_RESVAL_NORMAL), resFrequency (SETTING_RESFREQ_NORMAL), credits (SETTING_CREDITS_NORMAL),
		bridgeHead (SETTING_BRIDGEHEAD_DEFINITE), alienTech (SETTING_ALIENTECH_OFF), clans (SETTING_CLANS_ON), gameType (SETTINGS_GAMETYPE_SIMU), victoryType (SETTINGS_VICTORY_POINTS),
		duration (SETTINGS_DUR_MEDIUM), iTurnDeadline (90) {}

	std::string getResValString (eSettingResourceValue type) const;
	std::string getResFreqString() const;
	std::string getVictoryConditionString() const;
};

/**
 * A class that contains all information to start a new or loaded game.
 * This class can run games automaticaly out of this information.
 *@author alzi
 */
class cGameDataContainer
{
public:
	/** Should this instance of maxr act as the server for a TCP/IP game. */
	bool isServer;

	/** Number of the savegame or -1 for no savegame*/
	int savegameNum;
	/** name of the savegame if the savefile is only on the server and this container is set by a client*/
	std::string savegame;

	/** The settings for the game*/
	sSettings* settings;
	/** The map for the game*/
	cStaticMap* map;

	/** list with all players for the game*/
	std::vector<cPlayer*> players;
	/** list with the selected landing units by each player*/
	std::vector<std::vector<sLandingUnit>*> landingUnits;
	/** the client landing data (landing positions) of the players*/
	std::vector<sClientLandData> landData;
	/** indicates, whether all players have landed */
	bool allLanded;

public:
	cGameDataContainer();
	~cGameDataContainer();

	/** Runs the game. If isServer is true, which means that he is the host, a server will be started.
	 * Else only a client will be started. When reconnect is true, it will be reconnected to a running game.
	 * When the container contains a savegamenumber, the savegame will be loaded
	 *@author alzi
	 */
	void runGame (cTCP* network, int playerNr, bool reconnect = false);

	/** handles incoming clan information
	 *  @author pagra */
	void receiveClan (cNetMessage* message);

	/** handles incoming landing units
	 *@author alzi
	 */
	void receiveLandingUnits (cNetMessage* message);

	/** handles incoming unit upgrades
	 *@author alzi
	 */
	void receiveUnitUpgrades (cNetMessage* message);

	/** handles an incoming landing position
	 *@author alzi
	 */
	void receiveLandingPosition (cTCP& network, cNetMessage* message, cMenu* activeMenu);

	cEventHandling& getEventHandler() { return *eventHandler; }
private:

	cPlayer* findPlayerByNr (int nr);

	/** checks whether the landing positions are okay
	 *@author alzi
	 */
	eLandingState checkLandingState (unsigned int playerNr);
	/** loads and runs a saved game
	 *@author alzi
	 */
	void runSavedGame (cTCP* network, int player);

	void runNewGame (cTCP* network, int playerNr, bool reconnect);

private:
	cEventHandling* eventHandler;
};

enum eMenuBackgrounds
{
	MNU_BG_BLACK,
	MNU_BG_ALPHA,
	MNU_BG_TRANSPARENT
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
	bool drawnEveryFrame;
	/** When this is true the show-loop will be end and give 0 as return.
	 * Should be used when the menu is closed by ok or done.
	 */
	bool end;
	/** When this is true the show-loop will be ended and give 1 as return.
	 * Should be used when the menu is closed by abort or back.
	 */
	bool terminate;

	/** The background of the menu. Can be smaller than the screen. */
	AutoSurface background;
	/** The type of the background behind the menu background image, when the image is smaller then the screen. */
	eMenuBackgrounds backgroundType;
	/** The position of the menu on the screen when it is smaller than the screen. The position will be
	 * calculated in the constructor of cMenu and set to the center of the screen.
	 */
	SDL_Rect position;

	std::vector<cMenuTimerBase*> menuTimers;
	/** The list with all menuitems (buttons, images, etc.) of this menu. */
	std::vector<cMenuItem*> menuItems;
	/** Pointer to the currently active menuitem. This one will receive keyboard input. */
	cMenuItem* activeItem;

	/**
	 * initializes members and calculates the menu position on the screen.
	 *@author alzi
	 *@param background_ The background of the surface. Automatically gets deleted
	 *                   when the menu is destroyed.
	 */
	cMenu (SDL_Surface* background_, eMenuBackgrounds backgroundType_ = MNU_BG_BLACK);

	virtual void preDrawFunction() {}

	/** Recalculates the position and size of the menu.
	 */
	virtual void recalcPosition (bool resetItemPositions);

public:
	/**
	* virtual destructor
	*/
	virtual ~cMenu();
	/**
	 * redraws the menu background, the cursor and all menuitems.
	 *@author alzi
	 */
	void draw (bool firstDraw = false, bool showScreen = true);
	/**
	 * displays the menu and focuses all input on this menu until end or terminate are set to true.
	 *@author alzi
	 */
	virtual int show (cClient* client);
	/**
	 * sets end to true what will close the menu.
	 *@author alzi
	 */
	void close();

	/**
	 * will the menu be closed after finishing the current action?
	 *@author eiko
	 */
	bool exiting() const;

	/**
	 * handles mouseclicks, delegates them to the matching menuitem and handles the activity of the menuitems.
	 *@author alzi
	 */
	void handleMouseInput (sMouseState mouseState);
	/**
	 * gives the opinion to handle the mouse input to childclasses.
	 * This function is called at the end of handleMouseInput().
	 *@author alzi
	 */
	virtual void handleMouseInputExtended (sMouseState mouseState) {}
	/**
	 * delegates the keyinput to the active menuitem.
	 *@author alzi
	 */
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);

	/**
	 * sends a netmessage to the given player.
	 *@author alzi
	 */
	static void sendMessage (cTCP& network, cNetMessage* message, const sMenuPlayer* player = NULL, int fromPlayerNr = -1);
	/**
	 * this method will receive the menu-net-messages when this menu is active in the moment the message
	 * has been received. If the message should be handled overwrite this virtual method.
	 *@author alzi
	 */
	virtual void handleNetMessage (cNetMessage* message) {}
	virtual void handleNetMessages() {}

	virtual void handleDestroyUnit (cUnit& destroyedUnit) {}

	void addTimer (cMenuTimerBase* timer);

protected:
	static void cancelReleased (void* parent);
	static void doneReleased (void* parent);
private:
	int lastScreenResX, lastScreenResY;
};

/**
 * A main menu with unit info image and a credits label at the bottom.
 *@author alzi
 */
class cMainMenu : public cMenu
{
	AutoPtr<cMenuImage> infoImage;
	AutoPtr<cMenuLabel> creditsLabel;

public:
	cMainMenu();

private:
	SDL_Surface* getRandomInfoImage();
private:
	static void infoImageReleased (void* parent);
};

/**
 * The menu in the very beginning.
 *@author alzi
 */
class cStartMenu : public cMainMenu
{
	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> singleButton;
	AutoPtr<cMenuButton> multiButton;
	AutoPtr<cMenuButton> preferenceButton;
	AutoPtr<cMenuButton> licenceButton;
	AutoPtr<cMenuButton> exitButton;

public:
	cStartMenu();

private:
	static void singlePlayerReleased (void* parent);
	static void multiPlayerReleased (void* parent);
	static void preferencesReleased (void* parent);
	static void licenceReleased (void* parent);
};

/**
 * The singleplayer menu.
 *@author alzi
 */
class cSinglePlayerMenu : public cMainMenu
{
	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> newGameButton;
	AutoPtr<cMenuButton> loadGameButton;
	AutoPtr<cMenuButton> backButton;
public:
	cSinglePlayerMenu();
private:
	static void newGameReleased (void* parent);
	static void loadGameReleased (void* parent);
};

/**
 * The multiplayer menu.
 *@author alzi
 */
class cMultiPlayersMenu : public cMainMenu
{
	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> tcpHostButton;
	AutoPtr<cMenuButton> tcpClientButton;
	AutoPtr<cMenuButton> newHotseatButton;
	AutoPtr<cMenuButton> loadHotseatButton;
	AutoPtr<cMenuButton> backButton;
public:
	cMultiPlayersMenu();
private:
	static void tcpHostReleased (void* parent);
	static void tcpClientReleased (void* parent);
	static void newHotseatReleased (void* parent);
	static void loadHotseatReleased (void* parent);
};

/**
 * The settings menu.
 *@author alzi
 */
class cSettingsMenu : public cMenu
{
protected:
	sSettings settings;

	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> okButton;
	AutoPtr<cMenuButton> backButton;

	AutoPtr<cMenuLabel> metalLabel;
	AutoPtr<cMenuLabel> oilLabel;
	AutoPtr<cMenuLabel> goldLabel;
	AutoPtr<cMenuLabel> creditsLabel;
	AutoPtr<cMenuLabel> bridgeheadLabel;
	//AutoPtr<cMenuLabel> alienTechLabel;
	AutoPtr<cMenuLabel> clansLabel;
	AutoPtr<cMenuLabel> resFrequencyLabel;
	AutoPtr<cMenuLabel> gameTypeLabel;
	AutoPtr<cMenuLabel> victoryLabel;

	AutoPtr<cMenuRadioGroup> metalGroup;
	AutoPtr<cMenuRadioGroup> oilGroup;
	AutoPtr<cMenuRadioGroup> goldGroup;
	AutoPtr<cMenuRadioGroup> creditsGroup;
	AutoPtr<cMenuRadioGroup> bridgeheadGroup;
	//AutoPtr<cMenuRadioGroup> aliensGroup;
	AutoPtr<cMenuRadioGroup> clansGroup;
	AutoPtr<cMenuRadioGroup> resFrequencyGroup;
	AutoPtr<cMenuRadioGroup> gameTypeGroup;
	AutoPtr<cMenuRadioGroup> victoryGroup;

	void updateSettings();
public:
	explicit cSettingsMenu (const sSettings& settings_);
	const sSettings& getSettings() const { return settings; }
private:
	static void okReleased (void* parent);
};

/**
 * The planet selection.
 *@author alzi
 */
class cPlanetsSelectionMenu : public cMenu
{
protected:
	cTCP* network;
	cGameDataContainer* gameDataContainer;

	AutoPtr<cMenuLabel> titleLabel;

	AutoPtr<cMenuButton> okButton;
	AutoPtr<cMenuButton> backButton;

	AutoPtr<cMenuButton> arrowUpButton;
	AutoPtr<cMenuButton> arrowDownButton;

	AutoPtr<cMenuImage> planetImages[8];
	AutoPtr<cMenuLabel> planetTitles[8];

	std::vector<std::string>* maps;
	int selectedMapIndex;
	int offset;

public:
	cPlanetsSelectionMenu (cTCP* network_, cGameDataContainer* gameDataContainer_);

private:
	void loadMaps();
	void showMaps();
private:
	static void backReleased (void* parent);
	static void okReleased (void* parent);
	static void arrowDownReleased (void* parent);
	static void arrowUpReleased (void* parent);
	static void mapReleased (void* parent);
};


/**
 * The clan selection.
 * @author pagra
 */
class cClanSelectionMenu : public cMenu
{
protected:
	cTCP* network;
	cGameDataContainer* gameDataContainer;

	AutoPtr<cMenuLabel> titleLabel;

	AutoPtr<cMenuImage> clanImages[8];
	AutoPtr<cMenuLabel> clanNames[8];
	AutoPtr<cMenuLabel> clanDescription1;
	AutoPtr<cMenuLabel> clanDescription2;
	AutoPtr<cMenuLabel> clanShortDescription;

	AutoPtr<cMenuButton> okButton;
	AutoPtr<cMenuButton> backButton;

	cPlayer* player;
	int clan;

	void updateClanDescription();

public:
	cClanSelectionMenu (cTCP* network_, cGameDataContainer* gameDataContainer_, cPlayer* player, bool noReturn);

	int getClan() const { return clan; }
private:
	virtual void handleNetMessage (cNetMessage* message);
	virtual void handleNetMessages();

private:
	static void clanSelected (void* parent);
	static void okReleased (void* parent);
};

/**
 * A standard hangar menu with one unit selection table,
 * unit info image, unit description, unit details window
 * and two buttons (done and back).
 *@author alzi
 */
class cHangarMenu : public cMenu
{
protected:
	cPlayer* player;

	cMenuLabel titleLabel;
	AutoPtr<cMenuImage> infoImage;
	AutoPtr<cMenuLabel> infoText;
	AutoPtr<cMenuCheckButton> infoTextCheckBox;

	AutoPtr<cMenuUnitDetailsBig> unitDetails;

	AutoPtr<cMenuUnitsList> selectionList;
	cMenuUnitListItem* selectedUnit;

	AutoPtr<cMenuButton> selListUpButton;
	AutoPtr<cMenuButton> selListDownButton;

	AutoPtr<cMenuButton> doneButton;
	AutoPtr<cMenuButton> backButton;

	void drawUnitInformation();

	void (*selectionChangedFunc) (void*);
public:
	cHangarMenu (SDL_Surface* background_, cPlayer* player_, eMenuBackgrounds backgroundType_ = MNU_BG_BLACK);

	void setSelectedUnit (cMenuUnitListItem* selectedUnit_);
	cMenuUnitListItem* getSelectedUnit();

	virtual void generateSelectionList() = 0;
private:
	static void infoCheckBoxClicked (void* parent);
	static void selListUpReleased (void* parent);
	static void selListDownReleased (void* parent);
};

/**
 * A hangar menu with a second unit table, where you can add units by double clicking in the first list.
 *@author alzi
 */
class cAdvListHangarMenu : virtual public cHangarMenu
{
protected:
	AutoPtr<cMenuUnitsList> secondList;

	AutoPtr<cMenuButton> secondListUpButton;
	AutoPtr<cMenuButton> secondListDownButton;

	virtual bool checkAddOk (const cMenuUnitListItem* item) const { return true; }
	virtual void addedCallback (cMenuUnitListItem* item) {}
	virtual void removedCallback (cMenuUnitListItem* item) {}
public:
	cAdvListHangarMenu (SDL_Surface* background_, cPlayer* player_);

private:
	static bool selListDoubleClicked (cMenuUnitsList* list, void* parent);
	static bool secondListDoubleClicked (cMenuUnitsList* list, void* parent);

	static void secondListUpReleased (void* parent);
	static void secondListDownReleased (void* parent);
};

/**
 * An upgrade hangar menu with filter checkbuttons for the unit selection list, goldbar and buttons for
 * upgrading units.
 *@author alzi
 */
class cUpgradeHangarMenu : virtual public cHangarMenu
{
protected:
	int credits;

	AutoPtr<cMenuUpgradeFilter> upgradeFilter;
	AutoPtr<cMenuUpgradeHandler> upgradeButtons;
	AutoPtr<cMenuMaterialBar> goldBar;
	AutoPtr<cMenuLabel> goldBarLabel;
	AutoPtr<cMenuLabel> titleLabel;

	sUnitUpgrade (*unitUpgrades) [8];
	void initUpgrades (const cPlayer& player);
public:
	cUpgradeHangarMenu (cPlayer* owner);
	~cUpgradeHangarMenu();
	void setCredits (int credits_);
	int getCredits() const;
};

/**
 * The hangar menu where you select your landing units in the beginning of a new game.
 *@author alzi
 */
class cStartupHangarMenu : public cUpgradeHangarMenu, public cAdvListHangarMenu
{
protected:
	cTCP* network;
	cGameDataContainer* gameDataContainer;

	AutoPtr<cMenuRadioGroup> upgradeBuyGroup;

	AutoPtr<cMenuMaterialBar> materialBar;

	AutoPtr<cMenuLabel> materialBarLabel;

	AutoPtr<cMenuButton> materialBarUpButton;
	AutoPtr<cMenuButton> materialBarDownButton;

	bool isInLandingList (const cMenuUnitListItem* item) const;

	virtual bool checkAddOk (const cMenuUnitListItem* item) const;
	virtual void addedCallback (cMenuUnitListItem* item);
	virtual void removedCallback (cMenuUnitListItem* item);

	void updateUnitData();
	void generateInitialLandingUnits();
	void addPlayerLandingUnits (cPlayer& player);

public:
	cStartupHangarMenu (cTCP* network, cGameDataContainer* gameDataContainer_, cPlayer* player_, bool noReturn);

private:
	virtual void generateSelectionList();
	virtual void handleNetMessage (cNetMessage* message);
	virtual void handleNetMessages();

private:
	static void selectionChanged (void* parent);
	static void doneReleased (void* parent);
	static void subButtonsChanged (void* parent);
	static void materialBarUpReleased (void* parent);
	static void materialBarDownReleased (void* parent);
	static void materialBarClicked (void* parent);
};

/**
 * The landingposition selection menu.
 *@author alzi
 */
class cLandingMenu : public cMenu
{
public:
	cLandingMenu (cTCP* network_, cGameDataContainer* gameDataContainer_, cPlayer* player_);

private:
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);
	virtual void handleNetMessage (cNetMessage* message);
	virtual void handleNetMessages();

protected:
	void createHud();
	void createMap();
	const sTerrain* getMapTile (int x, int y) const;
	void hitPosition();

private:
	static void mapClicked (void* parent);
	static void mouseMoved (void* parent);

protected:
	cTCP* network;
	cGameDataContainer* gameDataContainer;
	cPlayer* player;

	cStaticMap* map;

	AutoSurface hudSurface;
	AutoSurface mapSurface;

	AutoPtr<cMenuImage> hudImage;
	AutoPtr<cMenuImage> mapImage;
	AutoPtr<cMenuImage> circlesImage;
	AutoPtr<cMenuLabel> infoLabel;
	AutoPtr<cMenuLabel> infoLabelConst;

	AutoPtr<cMenuButton> backButton;

	sClientLandData landData;
};

/**
 * A standard menu for network TCP/IP games with ip, port, and playername lineedits,
 * chat lineedit and chat window, color selection and playerlist and map image.
 *@author alzi
 */
class cNetworkMenu : public cMenu
{
protected:
	cTCP* network;
	std::string ip;
	int port;
	std::vector<sMenuPlayer*> players;
	sMenuPlayer* actPlayer;

	AutoPtr<cMenuButton> backButton;
	AutoPtr<cMenuButton> sendButton;

	AutoPtr<cMenuImage> mapImage;
	AutoPtr<cMenuLabel> mapLabel;

	AutoPtr<cMenuLabel> settingsText;

	AutoPtr<cMenuListBox> chatBox;
	AutoPtr<cMenuLineEdit> chatLine;

	AutoPtr<cMenuLabel> ipLabel;
	AutoPtr<cMenuLabel> portLabel;
	AutoPtr<cMenuLabel> nameLabel;
	AutoPtr<cMenuLabel> colorLabel;

	AutoPtr<cMenuButton> nextColorButton;
	AutoPtr<cMenuButton> prevColorButton;
	AutoPtr<cMenuImage> colorImage;

	AutoPtr<cMenuLineEdit> ipLine;
	AutoPtr<cMenuLineEdit> portLine;
	AutoPtr<cMenuImage> setDefaultPortImage;
	AutoPtr<cMenuLineEdit> nameLine;

	AutoPtr<cMenuPlayersBox> playersBox;

	cGameDataContainer gameDataContainer;
	std::string saveGameString;
	std::string triedLoadMap;

	void showSettingsText();
	void showMap();
	void setColor (int color);
	void saveOptions();
	void changePlayerReadyState (sMenuPlayer* player);
	bool enteredCommand (const std::string& text);
	void runGamePreparation (cPlayer& player);

public:
	cNetworkMenu();
	~cNetworkMenu();

	void playerReadyClicked (sMenuPlayer* player);

private:
	virtual void playerSettingsChanged() = 0;
	virtual void handleNetMessages();

private:
	static void backReleased (void* parent);
	static void sendReleased (void* parent);

	static void nextColorReleased (void* parent);
	static void prevColorReleased (void* parent);

	static void wasNameImput (void* parent);
	static void portIpChanged (void* parent);
	static void setDefaultPort (void* parent);
};

/**
 * The host network menu.
 *@author alzi
 */
class cNetworkHostMenu : public cNetworkMenu
{
protected:
	AutoPtr<cMenuLabel> titleLabel;

	AutoPtr<cMenuButton> okButton;

	AutoPtr<cMenuButton> mapButton;
	AutoPtr<cMenuButton> settingsButton;
	AutoPtr<cMenuButton> loadButton;
	AutoPtr<cMenuButton> startButton;

	int checkAllPlayersReady() const;
	void checkTakenPlayerAttr (sMenuPlayer* player);
	bool runSavedGame();

	std::vector<cMapSender*> mapSenders;

public:
	cNetworkHostMenu();
	~cNetworkHostMenu();

private:
	void handleNetMessage_MU_MSG_CHAT (cNetMessage* message);
	void handleNetMessage_TCP_ACCEPT (cNetMessage* message);
	void handleNetMessage_TCP_CLOSE (cNetMessage* message);
	void handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage* message);
	void handleNetMessage_MU_MSG_REQUEST_MAP (cNetMessage* message);
	void handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (cNetMessage* message);

	virtual void handleNetMessage (cNetMessage* message);
	virtual void playerSettingsChanged();

private:
	static void okReleased (void* parent);
	static void mapReleased (void* parent);
	static void settingsReleased (void* parent);
	static void loadReleased (void* parent);
	static void startReleased (void* parent);
};

/**
 * The client network menu
 *@author alzi
 */
class cNetworkClientMenu : public cNetworkMenu
{
	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> connectButton;

	cMapReceiver* mapReceiver;
	std::string lastRequestedMap;

public:
	cNetworkClientMenu();
	~cNetworkClientMenu();

private:
	void handleNetMessage_MU_MSG_CHAT (cNetMessage* message);
	void handleNetMessage_TCP_CLOSE (cNetMessage* message);
	void handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (cNetMessage* message);
	void handleNetMessage_MU_MSG_PLAYERLIST (cNetMessage* message);
	void handleNetMessage_MU_MSG_OPTINS (cNetMessage* message);
	void initMapDownload (cNetMessage* message);
	void receiveMapData (cNetMessage* message);
	void canceledMapDownload (cNetMessage* message);
	void finishedMapDownload (cNetMessage* message);
	void handleNetMessage_MU_MSG_GO (cNetMessage* message);
	void handleNetMessage_GAME_EV_REQ_RECON_IDENT (cNetMessage* message);
	void handleNetMessage_GAME_EV_RECONNECT_ANSWER (cNetMessage* message);

	virtual void handleNetMessage (cNetMessage* message);
	virtual void playerSettingsChanged();
private:
	static void connectReleased (void* parent);
};

/**
 * The load menu.
 *@author alzi
 */
class cLoadMenu : public cMenu
{
protected:
	cGameDataContainer* gameDataContainer;

	AutoPtr<cMenuLabel> titleLabel;

	AutoPtr<cMenuButton> backButton;
	AutoPtr<cMenuButton> loadButton;

	AutoPtr<cMenuButton> upButton;
	AutoPtr<cMenuButton> downButton;

	AutoPtr<cMenuSaveSlot> saveSlots[10];

	std::vector<std::string>* files;
	std::vector<sSaveFile*> savefiles;

	int offset;
	int selected;

	void loadSaves();
	void displaySaves();

public:
	cLoadMenu (cGameDataContainer* gameDataContainer_, eMenuBackgrounds backgroundType_ = MNU_BG_BLACK);
	~cLoadMenu();

private:
	virtual void extendedSlotClicked (int oldSelection) {}

private:
	static void backReleased (void* parent);
	static void loadReleased (void* parent);
	static void upReleased (void* parent);
	static void downReleased (void* parent);
	static void slotClicked (void* parent);
};

/**
 * The load and save menu (ingame data menu).
 *@author alzi
 */
class cLoadSaveMenu : public cLoadMenu
{
protected:
	cGameDataContainer gameDataContainer;
	cClient* client;
	cServer* server;
	AutoPtr<cMenuButton> exitButton;
	AutoPtr<cMenuButton> saveButton;

public:
	cLoadSaveMenu (cClient& client_, cServer* server_);

private:
	virtual void extendedSlotClicked (int oldSelection);
private:
	static void exitReleased (void* parent);
	static void saveReleased (void* parent);
};

/**
 * The menu to build buildings.
 *@author alzi
 */
class cBuildingsBuildMenu : public cHangarMenu
{
protected:
	cClient* client;
	cVehicle* vehicle;

	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> pathButton;
	AutoPtr<cMenuBuildSpeedHandler> speedHandler;

public:
	cBuildingsBuildMenu (cClient& client, cPlayer* player_, cVehicle* vehicle_);

private:
	virtual void generateSelectionList();
	virtual void handleDestroyUnit (cUnit& destroyedUnit);

private:
	static void doneReleased (void* parent);
	static void backReleased (void* parent);
	static void pathReleased (void* parent);
	static void selectionChanged (void* parent);
	static bool selListDoubleClicked (cMenuUnitsList* list, void* parent);
};

/**
 * The menu to build vehicles.
 *@author alzi
 */
class cVehiclesBuildMenu : public cAdvListHangarMenu
{
protected:
	const cGameGUI* gameGUI;
	cBuilding* building;

	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuBuildSpeedHandler> speedHandler;

	AutoPtr<cMenuCheckButton> repeatButton;

	void createBuildList();
public:
	cVehiclesBuildMenu (const cGameGUI& gameGUI_, cPlayer* player_, cBuilding* building_);

private:
	virtual void generateSelectionList();
	virtual void handleDestroyUnit (cUnit& destroyedUnit);

private:
	static void doneReleased (void* parent);
	static void selectionChanged (void* parent);
};

/**
 * The upgrade menu.
 *@author alzi
 */
class cUpgradeMenu : public cUpgradeHangarMenu
{
	static bool tank;
	static bool plane;
	static bool ship;
	static bool build;
	static bool tnt;
	cClient* client;
protected:
public:
	explicit cUpgradeMenu (cClient& client_);

private:
	virtual void generateSelectionList();
private:
	static void doneReleased (void* parent);
	static void selectionChanged (void* parent);
};

class cUnitHelpMenu : public cMenu
{
protected:
	AutoPtr<cMenuLabel> titleLabel;

	AutoPtr<cMenuImage> infoImage;
	AutoPtr<cMenuLabel> infoText;

	AutoPtr<cMenuUnitDetailsBig> unitDetails;

	AutoPtr<cMenuButton> doneButton;

	AutoPtr<cMenuUnitListItem> unit;

	void init (sID unitID);
public:
	cUnitHelpMenu (sID unitID, cPlayer* owner);
	cUnitHelpMenu (sUnitData* unitData, cPlayer* owner);

private:
	virtual void handleDestroyUnit (cUnit& destroyedUnit);
};

class cStorageMenu : public cMenu
{
protected:
	cClient* client;
	cVehicle* ownerVehicle;
	cBuilding* ownerBuilding;
	std::vector<cVehicle*>& storageList;
	sUnitData unitData;
	sSubBase* subBase;

	bool canStorePlanes;
	bool canStoreShips;
	bool canRepairReloadUpgrade;

	bool voiceTypeAll;
	bool voicePlayed;

	int metalValue;

	int offset;

	AutoPtr<cMenuButton> doneButton;
	AutoPtr<cMenuButton> downButton;
	AutoPtr<cMenuButton> upButton;

	AutoPtr<cMenuImage> unitImages[6];
	AutoPtr<cMenuLabel> unitNames[6];
	AutoPtr<cMenuStoredUnitDetails> unitInfo[6];

	AutoPtr<cMenuButton> activateButtons[6];
	AutoPtr<cMenuButton> reloadButtons[6];
	AutoPtr<cMenuButton> repairButtons[6];
	AutoPtr<cMenuButton> upgradeButtons[6];

	AutoPtr<cMenuButton> activateAllButton;
	AutoPtr<cMenuButton> reloadAllButton;
	AutoPtr<cMenuButton> repairAllButton;
	AutoPtr<cMenuButton> upgradeAllButton;

	AutoPtr<cMenuMaterialBar> metalBar;

	void generateItems();

	int getClickedButtonVehIndex (AutoPtr<cMenuButton> (&buttons) [6]);
public:
	cStorageMenu (cClient& client_, std::vector<cVehicle*>& storageList_, cUnit& unit);

private:
	friend class cClient;
	void resetInfos();
	void playVoice (int Type);
private:
	virtual void handleDestroyUnit (cUnit& destroyedUnit);

private:
	static void upReleased (void* parent);
	static void downReleased (void* parent);

	static void activateReleased (void* parent);
	static void reloadReleased (void* parent);
	static void repairReleased (void* parent);
	static void upgradeReleased (void* parent);

	static void activateAllReleased (void* parent);
	static void reloadAllReleased (void* parent);
	static void repairAllReleased (void* parent);
	static void upgradeAllReleased (void* parent);
};

class cMineManagerMenu : public cMenu
{
	const cClient* client;
	cBuilding* building;
	sSubBase subBase;

	AutoPtr<cMenuLabel> titleLabel;
	AutoPtr<cMenuButton> doneButton;

	AutoPtr<cMenuMaterialBar> metalBars[3];
	AutoPtr<cMenuMaterialBar> oilBars[3];
	AutoPtr<cMenuMaterialBar> goldBars[3];

	AutoPtr<cMenuLabel> metalBarLabels[3];
	AutoPtr<cMenuLabel> oilBarLabels[3];
	AutoPtr<cMenuLabel> goldBarLabels[3];

	AutoPtr<cMenuMaterialBar> noneBars[3];

	AutoPtr<cMenuLabel> resourceLabels[3];
	AutoPtr<cMenuLabel> usageLabels[3];
	AutoPtr<cMenuLabel> reserveLabels[3];

	AutoPtr<cMenuButton> incButtons[3];
	AutoPtr<cMenuButton> decButtons[3];

	void setBarValues();
	void setBarLabels();

	std::string secondBarText (int prod, int need);
public:
	cMineManagerMenu (const cClient& client_, cBuilding* building_);

private:
	virtual void handleDestroyUnit (cUnit& destroyedUnit);

private:
	static void doneReleased (void* parent);

	static void increaseReleased (void* parent);
	static void decreseReleased (void* parent);

	static void barReleased (void* parent);
};

class cReportsMenu : public cMenu
{
	cClient* client;

	AutoPtr<cMenuRadioGroup> typeButtonGroup;

	AutoPtr<cMenuLabel> includedLabel;
	AutoPtr<cMenuCheckButton> planesCheckBtn;
	AutoPtr<cMenuCheckButton> groundCheckBtn;
	AutoPtr<cMenuCheckButton> seaCheckBtn;
	AutoPtr<cMenuCheckButton> stationaryCheckBtn;

	AutoPtr<cMenuLabel> borderedLabel;
	AutoPtr<cMenuCheckButton> buildCheckBtn;
	AutoPtr<cMenuCheckButton> fightCheckBtn;
	AutoPtr<cMenuCheckButton> damagedCheckBtn;
	AutoPtr<cMenuCheckButton> stealthCheckBtn;

	AutoPtr<cMenuButton> doneButton;
	AutoPtr<cMenuButton> upButton;
	AutoPtr<cMenuButton> downButton;

	AutoPtr<cMenuReportsScreen> dataScreen;
public:
	cReportsMenu (cClient& client);

	void scrollCallback (bool upPossible, bool downPossible);
	void doubleClicked (cUnit* unit);
private:
	static void upReleased (void* parent);
	static void downReleased (void* parent);

	static void typeChanged (void* parent);

	static void filterClicked (void* parent);
};

#endif //menusH
