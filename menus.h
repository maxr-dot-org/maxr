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

class cGameDataContainer
{
public:

	eGameTypes type;

	int savegameNum;
	string savegame;

	sSettings *settings;
	cMap *map;

	cList<cPlayer*> players;
	cList<cList<sLandingUnit>*> landingUnits;
	cList<sClientLandData*> landData;


	cGameDataContainer() : settings(NULL), map(NULL), type(GAME_TYPE_SINGLE), savegameNum(-1) {}
	~cGameDataContainer();

	void runGame( int player, bool reconnect = false );
	void receiveLandingUnits ( cNetMessage *message );
	void receiveUnitUpgrades ( cNetMessage *message );
	void receiveLandingPosition ( cNetMessage *message );

private:
	eLandingState checkLandingState( int playerNr );
	void runSavedGame( int player );
};

class cMenu
{
protected:
	bool end;
	bool terminate;

	SDL_Surface *background;
	SDL_Rect position;

	cList<cMenuItem*> menuItems;
	cMenuItem *activeItem;

	cMenu( SDL_Surface *background_ );
	~cMenu();

public:
	void draw();
	int show();

	void handleMouseInput( sMouseState mouseState );
	virtual void handleKeyInput( SDL_keysym keysym, string ch );

	static void sendMessage ( cNetMessage *message, sMenuPlayer *player = NULL );
	virtual void handleNetMessage( cNetMessage *message ) {}
};

EX cMenu *ActiveMenu;

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
};

class cAdvListHangarMenu : public cHangarMenu
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

class cStartupHangarMenu : public cAdvListHangarMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	int credits;

	cMenuRadioGroup *upgradeBuyGroup;

	cMenuCheckButton* checkButtonTank;
	cMenuCheckButton* checkButtonPlane;
	cMenuCheckButton* checkButtonShip;
	cMenuCheckButton* checkButtonBuilding;
	cMenuCheckButton* checkButtonTNT;

	cMenuMaterialBar *goldBar;
	cMenuMaterialBar *materialBar;

	cMenuLabel *goldBarLabel;
	cMenuLabel *materialBarLabel;

	cMenuButton *materialBarUpButton;
	cMenuButton *materialBarDownButton;

	cMenuUpgradeHanlder *upgradeButtons;

	sUnitUpgrade (*unitUpgrades)[8];

	void initUpgrades();
	void generateSelectionList();
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

	void setCredits( int credits_ );
	int getCredits();

	void handleNetMessage( cNetMessage *message );
};

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

class cBuildingsBuildMenu : public cHangarMenu
{
protected:
	cVehicle *vehicle;

	cMenuLabel *titleLabel;
	cMenuButton *pathButton;
	cMenuBuildSpeedHandler *speedHandler;

	void createSelectionList();
public:
	cBuildingsBuildMenu( cPlayer *player_, cVehicle *vehicle_ );
	~cBuildingsBuildMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void pathReleased ( void *parent );
	static void selectionChanged ( void *parent );
};

class cVehiclesBuildMenu : public cAdvListHangarMenu
{
protected:
	cBuilding *building;

	cMenuLabel *titleLabel;
	cMenuBuildSpeedHandler *speedHandler;

	cMenuCheckButton *repeatButton;

	void createSelectionList();
	void createBuildList();
public:
	cVehiclesBuildMenu( cPlayer *player_, cBuilding *building_ );
	~cVehiclesBuildMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void selectionChanged ( void *parent );
};

#endif //menusH
