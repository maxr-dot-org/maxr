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
struct sClientSettings{
  cList<sLanding*> *LandingList;
  int LandX,LandY;
  int nr;
};


// Prototypen ////////////////////////////////////////////////////////////////
void RunMainMenu(void);
/**
*Shows a vehicle or a building in the mainscreen. There's
*a 33% chance that it shows a building and a 66% chance to
*show a vehicle. A unit won't be shown twice in order.
*@author beko
*/
void showUnitPicture(void);
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
 * @param files 
 * @param offset 
 * @param selected 
 */
void PlaceMenuButton(std::string sText,int x,int y, int darkness, bool pressed);
/**
 * 
 * @param sText 
 * @param x 
 * @param y 
 * @param pressed 
 */
void PlaceSmallMenuButton(std::string sText,int x,int y,bool pressed);
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
 * @param selection 
 * @param images 
 * @param selected 
 * @param offset 
 * @param tank 
 * @param plane 
 * @param ship 
 * @param build 
 * @param tnt 
 * @param kauf 
 */
void CreateSelectionList(cList<sHUp*> *selection,cList<sHUp*> *images,int *selected,int *offset,bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf);
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
