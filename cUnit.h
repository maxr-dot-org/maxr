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
/** Author: Anton Reis hackar@maxr.org **/

#ifndef cUnitH
#define cUnitH
#define cUnit_Comp
#include "defines.h"
#include "sound.h"
#include "main.h"
#include <SDL.h>
#include "base.h"
#include "dialog.h"
#include "map.h"
#include "input.h"
#include "upgradecalculator.h"
#include <list>
#include "server.h"
#include "client.h"
#include "math.h"

class cPlayer;

class cUnit {
  public:
    struct sAttacking {
      unsigned int  maxAmmo;
      unsigned int  ammo;
      unsigned int  maxShots;
      unsigned int  shots;
      unsigned int  range;
      bool          shotDecreaseMovement;
    }; // struct sAttacking
    struct sProducing{
      cList<unsigned int> *lProducingUnits;   // List of units this unit can produce
      unsigned int        iProducingMPR;      // How many metall per round does it consume
      unsigned int        iProducingMaxMult;  // Maximal production speed
    }; // struct sProducing
    struct sTransport{
      cList<unsigned int> *lTransport;        // List of units this unit can transport
      unsigned int        iCapacity;          // Amount of the units it can transport
    }; // struct sTransport
    struct sDamageEffects{
      int DamageFXPointX;                     // the points, where smoke will be generated
      int DamageFXPointY;                     //    when the building is damaged
      int DamageFXPointX2;
      int DamageFXPointY2;
    }; // struct sDamageEffects
    struct sAbilities {
      sProducing    *producing;     // Production of the unit
      sTransport    *transport;     // Transportation ability of the unit
      unsigned int  *iCargo[3];     // How many bits of each ressource type
                                    //   this unit can transport
                                    //   0 = Metal
                                    //   1 = Fuel
                                    //   2 = Gold
      int           *iGeneration[5];// How many bits of each ressource type
                                    //   this unit can generate
                                    //   3 = Energy
                                    //   4 = Credits
                                    //   negative = consume
                                    //   Credits shall NOT be negative
      bool          bResearching;   // Wether the unit can do research
      bool          bActivable;     // Wether the unit can be turned off
    }; // struct sAbilities
    struct sUpgrade {   // is used for base values as well
      int values[8];
    }; // struct sUpgrade

    class cGraphics{
      public:
        cGraphics(sBuilding building, sDamageEffects *damageEffects);
        cGraphics(sVehicle vehicle, sDamageEffects *damageEffects);
        ~cGraphics(void);
//      private:
        // -- Building -- //
        SDL_Surface *img,*img_org; // Surface of the building
        SDL_Surface *shw,*shw_org; // Surface of the shadow
        SDL_Surface *eff,*eff_org; // Surface of effect
        // -- Vehicle -- //
        SDL_Surface *vimg[8],*vimg_org[8];    // 8 Surfaces des Vehicles
        SDL_Surface *vshw[8],*vshw_org[8];    // 8 Surfaces des Schattens
        SDL_Surface *build,*build_org;        // Surfaces beim Bauen
        SDL_Surface *build_shw,*build_shw_org;// Surfaces beim Bauen (Schatten)
        SDL_Surface *clear_small,*clear_small_org;        // Surfaces beim Clearen (die groesse wird in build geladen)
        SDL_Surface *clear_small_shw,*clear_small_shw_org;// Surfaces beim Clearen (Schatten) (die groesse wird in build geladen)
        SDL_Surface     *overlay,*overlay_org;// Overlays
        SDL_Surface     *storage;             // Bild des Vehicles im Lager
        char            *FLCFile;             // FLC-Video
        sDamageEffects  *showDamage;          // Data for damage effects

        // -- Description -- //
        SDL_Surface *video;  // Video
        SDL_Surface *info;   // Image
        char *text;          // Description

        void scaleSurfaces( float factor );
    }; // class cGraphics

    class cSounds{
      public:
        cSounds(sBuilding building);
        cSounds(sVehicle vehicle);
        ~cSounds(void);
      private:
        struct Mix_Chunk *Running;
        struct Mix_Chunk *Wait;
        struct Mix_Chunk *WaitWater;
        struct Mix_Chunk *Start;
        struct Mix_Chunk *StartWater;
        struct Mix_Chunk *Stop;
        struct Mix_Chunk *StopWater;
        struct Mix_Chunk *Drive;
        struct Mix_Chunk *DriveWater;
        struct Mix_Chunk *Attack;

    }; // class cSounds

    struct sUnit {
      cGraphics     graphics;
      cSounds       sounds;
      sUpgrade      baseValues;
      sAbilities    abilities;
      string        name;
      unsigned int  version;
      unsigned int  HP;
      bool          bBig;                     // Wether the unit is a 2x2 unit
    }; // struct sUnit

    cUnit(int buildingType,unsigned int PosX, unsigned int PosY, cPlayer *owner,cBase *base);
    ~cUnit(void);

    static void init_AddUnit(sUnit *unit);
    static void init_SetUnits(cList<sUnit*> *units);
    string getStatusStr(void);
    int refreshData(void);

    void generateName(void);
    void setSelected(bool selected);
    bool isSelected(void);
    cPlayer* getOwner(void);
    bool isAttacked(void);
  private:
    static cList<sUnit*>  *allUnits;
    sUnit                 *unitData;

    class cIProducing {
      public:
        //--------------------------------------------------------------------------
        /** struct for the building order list */
        //--------------------------------------------------------------------------
        struct sBuildList{
          unsigned int unit;
          unsigned int metall_remaining;
        };

        cIProducing(cUnit *unit);
        ~cIProducing(void);
        const cList<sBuildList*> getBuildList(void);
        unsigned int getMult(void);
        bool productionFinished(void);
      private:
        cUnit               *unit;      // The unit this class belongs to
        cList<sBuildList*>  *lBuildList;
        unsigned int iMult;             // current production speed
    }; // class cIProducing
    class cIAttacking{
      public:
        cIAttacking(void);
        ~cIAttacking(void);
        int refreshData(void);
        unsigned int getAmmo(void);
        bool isAttacking(void);
      private:
        unsigned int  iMaxAmmo;
        unsigned int  iAmmo;
        unsigned int  iMaxShots;
        unsigned int  iShots;
        unsigned int  iRange;
        bool          bShotDecreaseMovement;
        bool          bAttacking;
    }; // class cIAttacking
    class cIGenerating{
      public:
        cIGenerating(int generating[5]);          // value = max(0, value)
        ~cIGenerating(void);
        unsigned int GetProd(ResourceKind const resource);
        unsigned int GetMaxProd(ResourceKind const resource) const;
      private:
        unsigned int generation[5];
        unsigned int maxGeneration[5];
    }; // class cIGenerating
    class cIConsuming{
      public:
        cIConsuming(int consuming[5]);           // value = min(0, value)
        ~cIConsuming(void);
      private:
        unsigned int consume[5];
    }; // class cIConsuming
    class cIActivable{
      public:
        cIActivable(void);
        ~cIActivable(void);
        void setActive(bool working);
        bool active(void);
      private:
        bool bWorking;
    }; // class cIActivable
    class cIMovable {
      private:
        unsigned int iMovePoints;
    }; // class cIMovable

    class cIGraphics {
      public:
        cIGraphics(cUnit *unit);
        ~cIGraphics(void);
        void  setEffectAlpha(int effectAlpha);
        void  setEffectInc(bool effectInc);
        int   getEffectAlpha(void);
        bool  getEffectInc(void);
        void  getStartUp(void);
        void  draw (SDL_Rect *screenPos);
        void  drawHealthBar(void);
        void  drawMunBar(void);
        void  drawStatus(void);
      private:
        int   iEffectAlpha;   // Alpha value for the effect
        bool  bEffectInc;     // Is the effect counted upwards or downwards?
        int   iStartUp;       // Counter for start-up animation
        cUnit *unit;           // The unit this graphics belongs to
        void  render( SDL_Surface* surface, const SDL_Rect& dest);
    }; // class cIGraphics
    // -- Handles producing, training -- //
    unsigned int    iBuildingType;// position in the *allUnits
    unsigned int    iID;          // The ID of this unit
    unsigned int    iPosX,iPosY;  // Position on the map
    //int           iRubbleType;  // Type of debris
    //int           iRubbleValue; // Value of debris
    int             iDir;         // Facing direction (used for attack, move)
    string          name;         // Name of the unit
    unsigned int    iVersion;
    unsigned int    iMaxHP;
    unsigned int    iHP;
    bool            bSelected;
    bool            bIsAttacked;
    
    cIProducing     *producing;
    cIGraphics      *graphics;
    cIMovable       *moving;
    cIAttacking     *attacking;
    cIActivable     *activable;
    cPlayer         *owner;
}; // class cUnit
#endif
