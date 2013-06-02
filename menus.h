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

// forward declarations
int GetColorNr (const SDL_Surface* sf);

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

	sClientLandData() : iLandX (0), iLandY (0), iLastLandX (0), iLastLandY (0), landingState (LANDING_STATE_UNKNOWN), receivedOK (true) {}
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
	eSettingsClans clans;
	eSettingsGameType gameType;
	eSettingsVictoryType victoryType;
	eSettingsDuration duration;

	sSettings() : metal (SETTING_RESVAL_NORMAL), oil (SETTING_RESVAL_NORMAL), gold (SETTING_RESVAL_NORMAL), resFrequency (SETTING_RESFREQ_NORMAL), credits (SETTING_CREDITS_NORMAL),
		bridgeHead (SETTING_BRIDGEHEAD_DEFINITE), alienTech (SETTING_ALIENTECH_OFF), clans (SETTING_CLANS_ON), gameType (SETTINGS_GAMETYPE_SIMU), victoryType (SETTINGS_VICTORY_POINTS),
		duration (SETTINGS_DUR_MEDIUM) {}

	std::string getResValString (eSettingResourceValue type) const;
	std::string getResFreqString() const;
	std::string getVictoryConditionString() const;
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
	/** Should this instance of maxr act as the server for a TCP/IP game. */
	bool isServer;

	/** Number of the savegame or -1 for no savegame*/
	int savegameNum;
	/** name of the savegame if the savefile is only on the server and this container is set by a client*/
	std::string savegame;

	/** The settings for the game*/
	sSettings* settings;
	/** The map for the game*/
	cMap* map;

	/** list with all players for the game*/
	std::vector<cPlayer*> players;
	/** list with the selected landing units by each player*/
	std::vector<std::vector<sLandingUnit>*> landingUnits;
	/** the client landing data (landing positions) of the players*/
	std::vector<sClientLandData*> landData;
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
	/** checks whether the landing positions are okay
	 *@author alzi
	 */
	eLandingState checkLandingState (int playerNr);
	/** loads and runs a saved game
	 *@author alzi
	 */
	void runSavedGame (cTCP* network, int player);
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
	virtual int show();
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
	virtual void handleNetMessages () {}

	virtual void handleDestroyUnit (cBuilding* building, cVehicle* vehicle) {}

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
	AutoPtr<cMenuImage>::type infoImage;
	AutoPtr<cMenuLabel>::type creditsLabel;

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
	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type singleButton;
	AutoPtr<cMenuButton>::type multiButton;
	AutoPtr<cMenuButton>::type preferenceButton;
	AutoPtr<cMenuButton>::type licenceButton;
	AutoPtr<cMenuButton>::type exitButton;

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
	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type newGameButton;
	AutoPtr<cMenuButton>::type loadGameButton;
	AutoPtr<cMenuButton>::type backButton;
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
	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type tcpHostButton;
	AutoPtr<cMenuButton>::type tcpClientButton;
	AutoPtr<cMenuButton>::type newHotseatButton;
	AutoPtr<cMenuButton>::type loadHotseatButton;
	AutoPtr<cMenuButton>::type backButton;
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

	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type okButton;
	AutoPtr<cMenuButton>::type backButton;

	AutoPtr<cMenuLabel>::type metalLabel;
	AutoPtr<cMenuLabel>::type oilLabel;
	AutoPtr<cMenuLabel>::type goldLabel;
	AutoPtr<cMenuLabel>::type creditsLabel;
	AutoPtr<cMenuLabel>::type bridgeheadLabel;
	//AutoPtr<cMenuLabel>::type alienTechLabel;
	AutoPtr<cMenuLabel>::type clansLabel;
	AutoPtr<cMenuLabel>::type resFrequencyLabel;
	AutoPtr<cMenuLabel>::type gameTypeLabel;
	AutoPtr<cMenuLabel>::type victoryLabel;

	AutoPtr<cMenuRadioGroup>::type metalGroup;
	AutoPtr<cMenuRadioGroup>::type oilGroup;
	AutoPtr<cMenuRadioGroup>::type goldGroup;
	AutoPtr<cMenuRadioGroup>::type creditsGroup;
	AutoPtr<cMenuRadioGroup>::type bridgeheadGroup;
	//AutoPtr<cMenuRadioGroup>::type aliensGroup;
	AutoPtr<cMenuRadioGroup>::type clansGroup;
	AutoPtr<cMenuRadioGroup>::type resFrequencyGroup;
	AutoPtr<cMenuRadioGroup>::type gameTypeGroup;
	AutoPtr<cMenuRadioGroup>::type victoryGroup;

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

	AutoPtr<cMenuLabel>::type titleLabel;

	AutoPtr<cMenuButton>::type okButton;
	AutoPtr<cMenuButton>::type backButton;

	AutoPtr<cMenuButton>::type arrowUpButton;
	AutoPtr<cMenuButton>::type arrowDownButton;

	AutoPtr<cMenuImage>::type planetImages[8];
	AutoPtr<cMenuLabel>::type planetTitles[8];

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

	AutoPtr<cMenuLabel>::type titleLabel;

	AutoPtr<cMenuImage>::type clanImages[8];
	AutoPtr<cMenuLabel>::type clanNames[8];
	AutoPtr<cMenuLabel>::type clanDescription1;
	AutoPtr<cMenuLabel>::type clanDescription2;
	AutoPtr<cMenuLabel>::type clanShortDescription;

	AutoPtr<cMenuButton>::type okButton;
	AutoPtr<cMenuButton>::type backButton;

	cPlayer* player;
	int clan;

	void updateClanDescription();

public:
	cClanSelectionMenu (cTCP* network_, cGameDataContainer* gameDataContainer_, cPlayer* player, bool noReturn);

	int getClan() const { return clan; }
private:
	virtual void handleNetMessage (cNetMessage* message);
	virtual void handleNetMessages ();

private:
	static void clanSelected (void* parent);
	static void okReleased (void* parent);
};

/**
 * A standard hangar menu with one unit selection table, unit info image, unit description, unit details window
 * and two buttons (done and back).
 *@author alzi
 */
class cHangarMenu : public cMenu
{
protected:
	cPlayer* player;

	cMenuLabel titleLabel;
	AutoPtr<cMenuImage>::type infoImage;
	AutoPtr<cMenuLabel>::type infoText;
	AutoPtr<cMenuCheckButton>::type infoTextCheckBox;

	AutoPtr<cMenuUnitDetailsBig>::type unitDetails;

	AutoPtr<cMenuUnitsList>::type selectionList;
	cMenuUnitListItem* selectedUnit;

	AutoPtr<cMenuButton>::type selListUpButton;
	AutoPtr<cMenuButton>::type selListDownButton;

	AutoPtr<cMenuButton>::type doneButton;
	AutoPtr<cMenuButton>::type backButton;

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
	const cClient* client;
	AutoPtr<cMenuUnitsList>::type secondList;

	AutoPtr<cMenuButton>::type secondListUpButton;
	AutoPtr<cMenuButton>::type secondListDownButton;

	virtual bool checkAddOk (const cMenuUnitListItem* item) const { return true; }
	virtual void addedCallback (cMenuUnitListItem* item) {}
	virtual void removedCallback (cMenuUnitListItem* item) {}
public:
	cAdvListHangarMenu (const cClient* client_, SDL_Surface* background_, cPlayer* player_);

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

	AutoPtr<cMenuUpgradeFilter>::type upgradeFilter;
	AutoPtr<cMenuUpgradeHandler>::type upgradeButtons;
	AutoPtr<cMenuMaterialBar>::type goldBar;
	AutoPtr<cMenuLabel>::type goldBarLabel;

	sUnitUpgrade (*unitUpgrades) [8];
	void initUpgrades (cPlayer* player);
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

	AutoPtr<cMenuRadioGroup>::type upgradeBuyGroup;

	AutoPtr<cMenuMaterialBar>::type materialBar;

	AutoPtr<cMenuLabel>::type materialBarLabel;

	AutoPtr<cMenuButton>::type materialBarUpButton;
	AutoPtr<cMenuButton>::type materialBarDownButton;

	bool isInLandingList (const cMenuUnitListItem* item) const;

	virtual bool checkAddOk (const cMenuUnitListItem* item) const;
	virtual void addedCallback (cMenuUnitListItem* item);
	virtual void removedCallback (cMenuUnitListItem* item);

	void updateUnitData();
	void generateInitialLandingUnits();
	void addPlayerLandingUnits (cPlayer& player);

public:
	cStartupHangarMenu (const cClient* client, cTCP* network, cGameDataContainer* gameDataContainer_, cPlayer* player_, bool noReturn);

private:
	virtual void generateSelectionList();
	virtual void handleNetMessage (cNetMessage* message);
	virtual void handleNetMessages ();

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
	virtual void handleNetMessages ();

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

	cMap* map;

	AutoSurface hudSurface;
	AutoSurface mapSurface;

	AutoPtr<cMenuImage>::type hudImage;
	AutoPtr<cMenuImage>::type mapImage;
	AutoPtr<cMenuImage>::type circlesImage;
	AutoPtr<cMenuLabel>::type infoLabel;

	AutoPtr<cMenuButton>::type backButton;

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

	AutoPtr<cMenuButton>::type backButton;
	AutoPtr<cMenuButton>::type sendButton;

	AutoPtr<cMenuImage>::type mapImage;
	AutoPtr<cMenuLabel>::type mapLabel;

	AutoPtr<cMenuLabel>::type settingsText;

	AutoPtr<cMenuListBox>::type chatBox;
	AutoPtr<cMenuLineEdit>::type chatLine;

	AutoPtr<cMenuLabel>::type ipLabel;
	AutoPtr<cMenuLabel>::type portLabel;
	AutoPtr<cMenuLabel>::type nameLabel;
	AutoPtr<cMenuLabel>::type colorLabel;

	AutoPtr<cMenuButton>::type nextColorButton;
	AutoPtr<cMenuButton>::type prevColorButton;
	AutoPtr<cMenuImage>::type colorImage;

	AutoPtr<cMenuLineEdit>::type ipLine;
	AutoPtr<cMenuLineEdit>::type portLine;
	AutoPtr<cMenuImage>::type setDefaultPortImage;
	AutoPtr<cMenuLineEdit>::type nameLine;

	AutoPtr<cMenuPlayersBox>::type playersBox;

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
	AutoPtr<cMenuLabel>::type titleLabel;

	AutoPtr<cMenuButton>::type okButton;

	AutoPtr<cMenuButton>::type mapButton;
	AutoPtr<cMenuButton>::type settingsButton;
	AutoPtr<cMenuButton>::type loadButton;
	AutoPtr<cMenuButton>::type startButton;

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
	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type connectButton;

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

	AutoPtr<cMenuLabel>::type titleLabel;

	AutoPtr<cMenuButton>::type backButton;
	AutoPtr<cMenuButton>::type loadButton;

	AutoPtr<cMenuButton>::type upButton;
	AutoPtr<cMenuButton>::type downButton;

	AutoPtr<cMenuSaveSlot>::type saveSlots[10];

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
	cServer* server;
	AutoPtr<cMenuButton>::type exitButton;
	AutoPtr<cMenuButton>::type saveButton;

public:
	explicit cLoadSaveMenu (cServer* server_);

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

	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type pathButton;
	AutoPtr<cMenuBuildSpeedHandler>::type speedHandler;

public:
	cBuildingsBuildMenu (cClient& client, cPlayer* player_, cVehicle* vehicle_);

private:
	virtual void generateSelectionList();
	virtual void handleDestroyUnit (cBuilding* destroyedBuilding, cVehicle* destroyedVehicle);

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

	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuBuildSpeedHandler>::type speedHandler;

	AutoPtr<cMenuCheckButton>::type repeatButton;

	void createBuildList();
public:
	cVehiclesBuildMenu (const cGameGUI& gameGUI_, cPlayer* player_, cBuilding* building_);

private:
	virtual void generateSelectionList();
	virtual void handleDestroyUnit (cBuilding* destroyedBuilding, cVehicle* destroyedVehicle);

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
	cUpgradeMenu (cClient& client_, cPlayer* player);

private:
	virtual void generateSelectionList();
private:
	static void doneReleased (void* parent);
	static void selectionChanged (void* parent);
};

class cUnitHelpMenu : public cMenu
{
protected:
	AutoPtr<cMenuLabel>::type titleLabel;

	AutoPtr<cMenuImage>::type infoImage;
	AutoPtr<cMenuLabel>::type infoText;

	AutoPtr<cMenuUnitDetailsBig>::type unitDetails;

	AutoPtr<cMenuButton>::type doneButton;

	AutoPtr<cMenuUnitListItem>::type unit;

	void init (sID unitID);
public:
	cUnitHelpMenu (const cClient& client, sID unitID, cPlayer* owner);
	cUnitHelpMenu (const cClient& client, sUnitData* unitData, cPlayer* owner);

private:
	virtual void handleDestroyUnit (cBuilding* destroyedBuilding, cVehicle* destroyedVehicle);
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

	AutoPtr<cMenuButton>::type doneButton;
	AutoPtr<cMenuButton>::type downButton;
	AutoPtr<cMenuButton>::type upButton;

	AutoPtr<cMenuImage>::type unitImages[6];
	AutoPtr<cMenuLabel>::type unitNames[6];
	AutoPtr<cMenuStoredUnitDetails>::type unitInfo[6];

	AutoPtr<cMenuButton>::type activateButtons[6];
	AutoPtr<cMenuButton>::type reloadButtons[6];
	AutoPtr<cMenuButton>::type repairButtons[6];
	AutoPtr<cMenuButton>::type upgradeButtons[6];

	AutoPtr<cMenuButton>::type activateAllButton;
	AutoPtr<cMenuButton>::type reloadAllButton;
	AutoPtr<cMenuButton>::type repairAllButton;
	AutoPtr<cMenuButton>::type upgradeAllButton;

	AutoPtr<cMenuMaterialBar>::type metalBar;

	void generateItems();

	int getClickedButtonVehIndex (AutoPtr<cMenuButton>::type (&buttons) [6]);
public:
	cStorageMenu (cClient& client_, std::vector<cVehicle*>& storageList_, cVehicle* vehicle, cBuilding* building);

private:
	friend class cClient;
	void resetInfos();
	void playVoice (int Type);
private:
	virtual void handleDestroyUnit (cBuilding* destroyedBuilding, cVehicle* destroyedVehicle);

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

	AutoPtr<cMenuLabel>::type titleLabel;
	AutoPtr<cMenuButton>::type doneButton;

	AutoPtr<cMenuMaterialBar>::type metalBars[3];
	AutoPtr<cMenuMaterialBar>::type oilBars[3];
	AutoPtr<cMenuMaterialBar>::type goldBars[3];

	AutoPtr<cMenuLabel>::type metalBarLabels[3];
	AutoPtr<cMenuLabel>::type oilBarLabels[3];
	AutoPtr<cMenuLabel>::type goldBarLabels[3];

	AutoPtr<cMenuMaterialBar>::type noneBars[3];

	AutoPtr<cMenuLabel>::type resourceLabels[3];
	AutoPtr<cMenuLabel>::type usageLabels[3];
	AutoPtr<cMenuLabel>::type reserveLabels[3];

	AutoPtr<cMenuButton>::type incButtons[3];
	AutoPtr<cMenuButton>::type decButtons[3];

	void setBarValues();
	void setBarLabels();

	std::string secondBarText (int prod, int need);
public:
	cMineManagerMenu (const cClient& client_, cBuilding* building_);

private:
	virtual void handleDestroyUnit (cBuilding* destroyedBuilding, cVehicle* destroyedVehicle);

private:
	static void doneReleased (void* parent);

	static void increaseReleased (void* parent);
	static void decreseReleased (void* parent);

	static void barReleased (void* parent);
};

class cReportsMenu : public cMenu
{
	cClient* client;
	cPlayer* owner;

	AutoPtr<cMenuRadioGroup>::type typeButtonGroup;

	AutoPtr<cMenuLabel>::type includedLabel;
	AutoPtr<cMenuCheckButton>::type planesCheckBtn;
	AutoPtr<cMenuCheckButton>::type groundCheckBtn;
	AutoPtr<cMenuCheckButton>::type seaCheckBtn;
	AutoPtr<cMenuCheckButton>::type stationaryCheckBtn;

	AutoPtr<cMenuLabel>::type borderedLabel;
	AutoPtr<cMenuCheckButton>::type buildCheckBtn;
	AutoPtr<cMenuCheckButton>::type fightCheckBtn;
	AutoPtr<cMenuCheckButton>::type damagedCheckBtn;
	AutoPtr<cMenuCheckButton>::type stealthCheckBtn;

	AutoPtr<cMenuButton>::type doneButton;
	AutoPtr<cMenuButton>::type upButton;
	AutoPtr<cMenuButton>::type downButton;

	AutoPtr<cMenuReportsScreen>::type dataScreen;
public:
	cReportsMenu (cClient& client, cPlayer* owner_);

	void scrollCallback (bool upPossible, bool downPossible);
	void doubleClicked (cVehicle* vehicle, cBuilding* building);
private:
	static void upReleased (void* parent);
	static void downReleased (void* parent);

	static void typeChanged (void* parent);

	static void filterClicked (void* parent);
};

#endif //menusH
