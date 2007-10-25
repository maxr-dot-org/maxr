
//////////////////////////////////////////////////////////////////////////////
// M.A.X. - menu.h
//////////////////////////////////////////////////////////////////////////////
#ifndef menuH
#define menuH
#include "defines.h"
#include "main.h"
#include "SDL.h"
#include "map.h"
#include "buildings.h"
#include "player.h"
#include "fstcpip.h"

// Globales //////////////////////////////////////////////////////////////////
class cMultiPlayer;
EX SDL_Surface *TmpSf ZERO;
EX cMultiPlayer *MultiPlayer;
EX string LoadFile;

// Strukturen ////////////////////////////////////////////////////////////////
struct sPlayer{
  int what[4];
  string clan[4];
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
  TList *LandingList;
  int LandX,LandY;
  int nr;
};


// Prototypen ////////////////////////////////////////////////////////////////
void RunMainMenu(void);
void PlaceButton(const char *str,int x,int y,bool pressed);
/**
*Shows a vehicle or a building in the mainscreen. There's
*a 33% chance that it shows a building and a 66% chance to
*show a vehicle. A unit won't be shown twice in order.
*@author beko
*/
void ShowInfo(void);
void EnterMenu(bool limited=false);
void ExitMenu(void);
void RunSPMenu(void);
string RunPlanetSelect(void);
void ShowPlanets(TList *files,int offset,int selected);
sOptions RunOptionsMenu(sOptions *init);
sPlayer RunPlayerSelect(void);
void PlaceSelectText(const char *str,int x,int y,bool checked,bool center=true);
void RunHangar(cPlayer *player, TList *LandingList);
void SelectLanding(int *x,int *y,cMap *map);
int GetKachelBig(int x,int y);
void RunMPMenu(void);
void PlaceSmallButton(char *str,int x,int y,bool pressed);
void PlaceMenuButton(char *str,int x,int y, int darkness, bool pressed);
void PlaceSmallMenuButton(char *str,int x,int y,bool pressed);
int GetColorNr(SDL_Surface *sf);
void HeatTheSeat(void);
void ShowPlayerStates(sPlayer players);
void ShowLandingList(TList *list,int selected,int offset);
void CreateSelectionList(TList *selection,TList *images,int *selected,int *offset,bool tank,bool plane,bool ship,bool build,bool tnt,bool kauf);
int ShowDateiMenu(void);
void ShowFiles(TList *files, int offset, int selected);

// MultiPlayer Klasse ////////////////////////////////////////////////////////
class cMultiPlayer{
public:
  cMultiPlayer(bool host,bool tcp);
  ~cMultiPlayer(void);

  TList *MessageList;// Liste mit allen empfangenen Nachrichten
  TList *ChatList;   // Liste mit den Chatnachrichten
  cPlayer *MyPlayer; // Der aktuelle Spieler
  TList *PlayerList; // Liste mit allen Spielern
  string Titel;      // Titel des Menüs
  sOptions options;  // Optionen des Spiels
  bool host,tcp;     // Eigenschaften des Sockets
  cFSTcpIp *fstcpip; // Die Com-Schnittstelle
  string map;	     // Name der Map-Datei
  string IP;         // IP für die Verbindung
  int Port;          // Port für die Verbindung
  bool Refresh;      // Gibt an, ob neue Daten eingetroffen sind
  int NextPlayerID;  // Nächste eindeutige Player-ID
  bool no_options;   // Gibt an, ob schon Optionen eingestellt wurden
  bool WaitForGo;    // Gibt an, ob der Host auf das Go der Clients wartet
  int ClientsToGo;   // Verbleibende Anzahl an Clients, die nich Go geben müssen
  bool LetsGo;       // Gibt an, ob der Host das LetsGo Signal gab
  cMap *map_obj;     // Die Map zum spielen
  TList *ClientSettingsList; // Liste mit allen empfangenen ClientSettings
  string SaveGame; // Name des zu ladenden Savegames

  void RunMenu(void);
  void DisplayGameSettings();
  void DisplayPlayerList(void);
  void ShowChatLog(void);
  void AddChatLog(string str);
  void HandleMenuMessages();
  void ClientConnectedCallBack(void);
  void ClientDistconnect(void);
  void ServerDisconnect(void);
  void ChangeFarbeName(void);
  void SendPlayerList(void);
  void SendOptions(void);
  void TransmitRessources(void);
  void ServerWait(int LandX,int LandY,TList *LandingList);
  void ClientWait(int LandX,int LandY,TList *LandingList);
  void TransmitPlayerUpgrades(cPlayer *p);
  void TransmitPlayerLanding(int nr,int x,int y,TList *ll);
  bool TestPlayerList(void);
  bool TestPlayerListLoad(void);  
  TList* SplitMessage(string msg);
};

#endif
