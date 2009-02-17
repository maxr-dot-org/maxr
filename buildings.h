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
#ifndef buildingsH
#define buildingsH
#include "defines.h"
#include "sound.h"
#include "main.h"
#include <SDL.h>
#include "base.h"
#include "dialog.h"
#include "map.h"
#include "input.h"

// Define zum Updaten:
#define UpdateBuilding(from,to) if((from).hit_points==(from).max_hit_points){(from).hit_points=(to).max_hit_points;}(from).version=(to).version;(from).max_hit_points=(to).max_hit_points;(from).armor=(to).armor;(from).scan=(to).scan;(from).range=(to).range;(from).max_shots=(to).max_shots;(from).damage=(to).damage;(from).max_ammo=(to).max_ammo;(from).iBuilt_Costs=(to).iBuilt_Costs;

// Struct for one upgrade (one kind of value, e.g. max_hit_points) /////////////////////////////////////////////////////
struct sUpgrade {
  bool active; // is this upgrade buyable for the player
  int NextPrice; // what will the next upgrade cost
  int Purchased;
  int *value; // what is the current value
  int StartValue; // the initial value for this unit type
  string name; // the name of this upgrade type, e.g. Ammo
};

// Struct for one upgrade (one kind of value, e.g. max_hit_points) /////////////////////////////////////////////////////
// When the hangar is made nice code, too, the sUpgradeNew and the sUpgrade have to be united, again.
struct sUpgradeNew {
	bool active; // is this upgrade buyable for the player
	int nextPrice; // what will the next upgrade cost
	int purchased; // how many upgrades of this type has the player purchased
	int curValue; // what is the current value
	int startValue; // the value that this unit would have without all upgrades
	string name; // the name of this upgrade type, e.g. Ammo
};

// struct for the images and sounds:
struct sBuilding{
  SDL_Surface *img,*img_org; // Surface des Buildings
  SDL_Surface *shw,*shw_org; // Surfaces des Schattens
  SDL_Surface *eff,*eff_org; // Surfaces des Effektes
  SDL_Surface *video;  // Video
  sUnitData data;  // Grunddaten des Buildings
  char id[4];          // ID dieses Elements
  int nr;              // Nr dieses Elements
  SDL_Surface *info;   // Infobild
  char *text;          // Infotext

  // Die Sounds:
  struct Mix_Chunk *Start;
  struct Mix_Chunk *Running;
  struct Mix_Chunk *Stop;
  struct Mix_Chunk *Attack;

  void scaleSurfaces( float faktor );
};

class cPlayer;
class cBase;
struct sUpgradeStruct;

// enum for the upgrade symbols
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed,SHits,SAmmo,SMetal,SEnergy,SShots,SOil,SGold,STrans,SHuman,SAir};
enum eSymbolsBig {SBSpeed,SBHits,SBAmmo,SBAttack,SBShots,SBRange,SBArmor,SBScan,SBMetal,SBOil,SBGold,SBEnergy,SBHuman};
#endif

struct sBuildStruct
{
public:
	sBuildStruct(SDL_Surface* const sf_, sID const ID_, int const iRemainingMetal_ = -1) :
		sf(sf_),
		ID(ID_),
		iRemainingMetal(iRemainingMetal_)
	{}

	SDL_Surface *sf;
	sID ID;
	int iRemainingMetal;
};

// struct for the building order list
struct sBuildList{
  struct sVehicle *typ;
  int metall_remaining;
};

enum ResourceKind
{
	TYPE_METAL = 0,
	TYPE_OIL   = 1,
	TYPE_GOLD  = 2
};

struct sMineValues
{
	int& GetProd(ResourceKind);

	int GetMaxProd(ResourceKind) const;

	int iMetalProd, iOilProd, iGoldProd;
	int iMaxMetalProd, iMaxOilProd, iMaxGoldProd;

	int iBuildingID;
};

// Die Buulding Klasse ///////////////////////////////////////////////////////
class cBuilding{
public:
	cBuilding(sBuilding *b,cPlayer *Owner,cBase *Base);
	~cBuilding(void);

	/** the identification number of this unit */
	unsigned int iID;
	/** a list were the numbers of all players who can see this building are stored in */
	cList<cPlayer*> SeenByPlayerList;
	/** a list were the numbers of all players who have deteced this vehicle are stored in */
	cList<cPlayer*> DetectedByPlayerList;
	int PosX,PosY;   // Position auf der Karte
	sBuilding *typ;  // Typ des Buildings
	bool selected;   // is the building selected?
	string name; // Name des Buildings
	cPlayer *owner;  // owner of the building
	sUnitData data;    // Daten des Buildings
	cBuilding *next,*prev; // pointers for the linked list
	bool MenuActive; // is the menu currently active?
	bool AttackMode; // Gibt an, ob der AttackMode grad aktiv ist
	bool bIsBeeingAttacked; /** true when an attack on this building is running */
	int RubbleTyp;     // Typ des Drecks
	int RubbleValue;   // Wert des Drecks
	int StartUp;     // counter for the startup animation
	cBase *base;     // the base to which this building belongs
	bool BaseN,BaseE,BaseS,BaseW; // is the building connected in this direction?
	bool BaseBN,BaseBE,BaseBS,BaseBW; // is the building connected in this direction (only for big buildings)
	struct sSubBase *SubBase;     // the subbase to which this building belongs
	int EffectAlpha; // alpha value for the effect
	bool EffectInc;  // is the effect counted upwards or dounwards?
	bool IsWorking;  // is the building currently working?
	bool bSentryStatus;		/** true if the building is on sentry */
	bool Transfer;   // Gibt an, ob ein Transfer statfinden soll
	int MetalProd,OilProd,GoldProd; // the production of the building
	int MaxMetalProd,MaxOilProd,MaxGoldProd; // the maximum possible production of the building
	int dir;         // ?Frame of the building?
	bool Attacking;  // Gibt an, ob das Building gerade angreift
	cList<sBuildList*> *BuildList; // Die Bauliste der Fabrik
	int BuildSpeed;  // Die baugeschwindigkeit der Fabrik
	int MetalPerRound; //Die Menge an Metal, die die Fabrik bei momentaner Baugeschwindigkeit pro Runde maximal verbaut
	bool RepeatBuild; // Gibt an, ob der Bau wiederholt werden soll
	bool LoadActive; // Gibt an, ob ein Vehicle geladen werden soll
	cList<cVehicle*> StoredVehicles; // Liste mit geladenen Vehicles
	int VehicleToActivate; // Nummer des Vehicles, dass aktiviert werden soll
	bool ActivatingVehicle; // Gibt an, ob ein Vehicle aktiviert werden soll
	int DamageFXPointX,DamageFXPointY,DamageFXPointX2,DamageFXPointY2; // the points, where smoke will be generated when the building is damaged
	int Disabled;    // the time this unit will be disabled
	bool IsLocked;   // Gibt an, ob dieses Building in irgend einer Log-Liste ist
	int wantRedrawedStoredOffset;
	bool hasBeenAttacked;

	void Draw(SDL_Rect *dest);
	void Select(void);
	void Deselct(void);
	void ShowDetails(void);
	void GenerateName(void);
	int PlayStram(void);
	std::string GetStatusStr(void);
	void DrawSymbol(eSymbols sym,int x,int y,int maxx,int value,int maxvalue,SDL_Surface *sf);
	void DrawNumber(int x,int y,int value,int maxvalue,SDL_Surface *sf);
	/**
	* refreshes the shots of this building
	*@author alzi alias DoctorDeath
	*@return 1 if there has been refreshed something else 0.
	*/
	int refreshData();
	void ShowHelp(void);
	void DrawSymbolBig(eSymbolsBig sym,int x,int y,int maxx,int value,int orgvalue,SDL_Surface *sf);
	void Center(void);
	void DrawMunBar(void) const;
	void DrawHelthBar(void) const;
	void drawStatus() const;
	int GetScreenPosX(void) const;
	int GetScreenPosY(void) const;
	int CalcHelth(int damage);
	void DrawMenu( sMouseState *mouseState = NULL );
	int GetMenuPointAnz(void);
	SDL_Rect GetMenuSize(void);
	bool MouseOverMenu(int mx,int my);
	void SelfDestructionMenu(void);
	void ShowBigDetails(void);
	void updateNeighbours( cMap *map );
	void CheckNeighbours( cMap *Map );
	void DrawConnectors(SDL_Rect dest);
	void ServerStartWork();
	void ClientStartWork();
	void ServerStopWork(bool override);
	void ClientStopWork();
	bool CanTransferTo( class cMapField *OverUnitField ); /** check whether a transfer to an unit on the field is possible */
	void CheckRessourceProd(void);
	void showMineManager();
	void doMineInc(ResourceKind, cList<sMineValues*>& Mines);
	void doMineDec(ResourceKind, cList<sMineValues*>& Mines);
	void calcMineFree ( cList<sMineValues*> *Mines, int *iFreeM, int *iFreeO, int *iFreeG );
	void MakeMineBars(int iTempSBMetalProd, int iTempSBOilProd, int iTempSBGoldProd, int MaxM,int MaxO,int MaxG,int *FreeM,int *FreeO,int *FreeG);
	void DrawMineBar(int typ,int value,int max_value,int offy,bool number,int fixed);
	bool IsInRange(int off, cMap *Map);
	/*
	* checks if the unit can attack the offset
	* when override is false, the funktion only returns true, when there is an enemy unit
	* ATTENTION: must not be called with override == false from the server thread!
	*/
	bool CanAttackObject(int off, cMap *Map, bool override=false);
	void DrawAttackCursor( int offset );
	void RotateTo(int Dir);
	void ShowBuildMenu(void);
	void ShowBuildList(cList<sBuildStruct*>& list, int selected, int offset, bool showInfo);
	void DrawBuildButtons(int speed);
	void ShowToBuildList(cList<sBuildStruct*>& list, int selected, int offset, bool showInfo);
	void CalcTurboBuild(int *iTurboBuildRounds, int *iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal = -1);
	void DrawExitPoints(sVehicle *typ);
	bool canExitTo ( const int x, const int y, const cMap* map, const sVehicle *typ ) const;
	bool canLoad( int offset, cMap *Map );
	bool canLoad( cVehicle *Vehicle );
	void storeVehicle( cVehicle *Vehicle, cMap *Map );
	void ShowStorage(void);
	void DrawStored(int off);
	void ShowStorageMetalBar(void);
	void exitVehicleTo( cVehicle *Vehicle, int offset, cMap *Map );
	void MakeStorageButtonsAlle(bool *AlleAufladenEnabled, bool *AlleReparierenEnabled, bool *AlleUpgradenEnabled);
	void ShowResearch(void);
	void ShowResearchSchieber(void);
	void MakeResearchSchieber(int x,int y);
	void SendUpdateStored(int index);

	/** 
	* Shows the upgrade screen and sends upgrade-requests to the server, if the user presses the "done" button. 
	*/
	void ShowUpgrade ();
	
	/**
	* returns whether this player has detected this unit or not
	*@author alzi alias DoctorDeath
	*@param player player for which the stauts sould be checked
	*@return true if the player has detected the unit
	*/
	bool isDetectedByPlayer( cPlayer* player );
	/**
	* adds a player to the DetecedByPlayerList
	*/
	void resetDetectedByPlayer( cPlayer* player );
	/**
	* removes a player from the detectedByPlayerList
	*/
	void setDetectedByPlayer( cPlayer* player );
	/**
	* - checks whether the building has been detected by an other unit
	* the detection maps have to be up to date, when calling this funktion
	* this function has to be called on the server everytime a building is added
	*/
	void makeDetection();
	/**
	* checks whether the coordinates are next to the building
	*/
	bool isNextTo( int x, int y) const;
	

	//-----------------------------------------------------------
protected:
	void ShowUpgradeList (cList<sUpgradeStruct*>& list, int selected, int offset, bool description, int curCredits);
	void ShowGoldBar (int startCredits, int curCredits);
	void initUpgradesVehicle (sUpgradeNew u[], int vehicleTypeIdx);
	void initUpgradesBuilding (sUpgradeNew u[], int buildingTypeIdx);
	void CreateUpgradeList (cList<sUpgradeStruct*>& selection, cList<sUpgradeStruct*>& images, int& selected, int& offset);
	void ShowUpgradeSubButtons ();
	int findUpgradeValue (sUpgradeStruct* upgradeStruct, int upgradeType, int defaultValue = 0);
	void sendUpgrades (cList<sUpgradeStruct*>& allUpgradeStructs, cPlayer* player);
};

#endif
