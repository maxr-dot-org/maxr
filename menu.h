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
#ifndef menuH
#define menuH
#include "defines.h"
#include "main.h"
#include "SDL.h"
#include "map.h"
#include "buildings.h"
#include "player.h"
#include "network.h"

// Globales //////////////////////////////////////////////////////////////////
EX string SaveLoadFile;	// Name of the savegame to load or to save
EX int SaveLoadNumber;	// Index number of the savegame to load or to save

// Strukturen ////////////////////////////////////////////////////////////////
struct sPlayer{
  int what[4];
  string clan[4];
};

// Strukturen ////////////////////////////////////////////////////////////////
struct sPlayerHS{
  int what[8];
  int iColor[8];
  string name[8];
  string clan[8];
};

enum ePlayer
{
	PLAYER_N,
	PLAYER_H,
	PLAYER_AI
};

// Struktur für die Optionen:
struct sOptions{
  int metal,oil,gold,dichte;
  int credits;
  bool FixedBridgeHead;
  bool AlienTech;
  bool PlayRounds;
};

// Struktur für die Upgrades:
struct sHUp{
  SDL_Surface *sf;
  bool vehicle;
  int id;
  int costs;
  sUpgrades upgrades[8];
};

// Struktur für die Landung:
struct sLanding{
  SDL_Surface *sf;
  int id;
  int costs;
  int cargo;
};

// Struktur für die ClientSettings:
struct sClientLandData
{
	cList <sLanding*> *LandingList;
	int iLandX, iLandY;
	int iNr;
};

enum MESSAGE_TYPES
{
	MU_MSG_CHAT = FIRST_MENU_MESSAGE,	// simple text message
	MU_MSG_NEW_PLAYER,			// a new player has connected
	MU_MSG_REQ_IDENTIFIKATION,	// host requests a identifacation of this player
	MU_MSG_IDENTIFIKATION,		// player send his idenetification
	MU_MSG_DEL_PLAYER,			// a player should be deleted
	MU_MSG_PLAYERLIST,			// a list with all players and their data
	MU_MSG_OPTINS,				// all options selected by the host
	MU_MSG_GO,					// host wants to start the game
	MU_MSG_WT_LAND,				// a player wants to land at this position
	MU_MSG_ALL_LANDED,			// all players have selcted there landing points and clients can start game
	MU_MSG_RESOURCES,			// the resources on the map
	MU_MSG_UPGRADES				// data of upgraded units
};

class cMultiPlayerMenu
{
public:
	cMultiPlayerMenu(bool bHost);
	~cMultiPlayerMenu();

	void runNetworkMenu();

private:
	SDL_Surface *sfTmp;
	bool bRefresh;
	bool bHost;
	bool bOptions;
	bool bStartSelecting;
	string sIP;
	string sSaveGame;
	string sMap;
	int iFocus;
	int iPort;
	bool bAllLanded;
	cMap *Map;
	sOptions Options;

	cList<cPlayer*> PlayerList;
	bool *ReadyList;
	int iNextPlayerNr;
	cPlayer *ActualPlayer;
	cList<string> ChatLog;
	cList<sClientLandData*> *ClientDataList;

	void addChatLog( string sMsg );
	void showChatLog();
	void displayGameSettings();
	void displayPlayerList();

	void sendIdentification();
	void sendPlayerList();
	void sendOptions();
	void sendLandingInfo( int iLandX, int iLandY, cList<sLanding*> *LandingList );
	void sendUpgrades();

	int testAllReady();

	void HandleMessages();
	void sendMessage( cNetMessage *Message, int iPlayer = -1 );

public:
	cList<cNetMessage*> MessageList;
} EX *MultiPlayerMenu;

// Prototypen ////////////////////////////////////////////////////////////////
void RunMainMenu(void);
/**
 *
 * @param bIAmMain
 */
void prepareMenu(bool bIAmMain=false);
/**
 *
 * @param
 */
void ExitMenu(void);
/**
 *
 * @param
 */
void RunSPMenu(void);
/**
 *
 * @param
 * @return
 */
string RunPlanetSelect(void);
/**
 *
 * @param files
 * @param offset
 * @param selected
 * @param surface Source Surface for proper background drawing
 */
void ShowPlanets(cList<string> *files,int offset,int selected, SDL_Surface *surface);
/**
 *
 * @param init
 * @return
 */
sOptions RunOptionsMenu(sOptions *init);

/**
 * @author beko
 * @param
 * @return
 */
sPlayerHS runPlayerSelectionHotSeat(void);

/**
 *
 * @param
 * @return
 */
sPlayer runPlayerSelection(void);
/**
 *
 * @param str
 * @param x
 * @param y
 * @param checked
 * @param surface Source Surface for proper background drawing
 * @param center
 */
void placeSelectableText(std::string sText,int x,int y,bool checked, SDL_Surface *surface, bool center=true);
/**
 *
 * @param player
 * @param LandingList
 */
void RunHangar(cPlayer *player, cList<sLanding*> *LandingList);
/**
 *
 * @param x
 * @param y
 * @param map
 */
void SelectLanding(int *x,int *y,cMap *map);
/**
 *
 * @param x
 * @param y
 * @return
 */
int GetKachelBig(int x,int y);
/**
 *
 * @param
 */
void RunMPMenu(void);
/**
 *
 * @param
 */
void placeSmallButton(std::string sText,int x,int y,bool pressed);
/**
 *
 * @param sf
 * @return
 */
int GetColorNr(SDL_Surface *sf);
/**
 *
 * @param
 */
void HeatTheSeat(void);

/**
 *
 * @param players
 */
void showPlayerStatesHotSeat(sPlayerHS players);

/**
 *
 * @param players
 */
void ShowPlayerStates(sPlayer players);
/**
 *
 * @param list
 * @param selected
 * @param offset
 * @param surface Source Surface for proper background drawing
 */
void ShowLandingList(cList<sLanding*> *list,int selected,int offset, SDL_Surface *surface);
/**
 *
 * @param bSave Should you can load savegames in this menu?
 * @return
 */
int ShowDateiMenu( bool bSave );
/**
 *
 * @param files
 * @param offset
 * @param selected
 * @param rDialog SDL_Rect with real Dialog rect depending on screen resolution
 */
void ShowFiles(cList<string> *files, int offset, int selected, bool bSave, bool bCursor, bool bFirstSelect, SDL_Rect rDialog);
/**
 *
 * @param sFileName
 * @param sTime
 * @param sSavegameName
 * @param sMode
 */
void loadMenudatasFromSave ( string sFileName, string *sTime, string *sSavegameName, string *sMode );

#endif
