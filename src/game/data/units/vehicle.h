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

#ifndef game_data_units_vehicleH
#define game_data_units_vehicleH

#include "defines.h"
#include "maxrconfig.h"

#include "game/data/units/commandodata.h"
#include "game/data/units/unitdata.h" // for sUnitData
#include "game/data/units/unit.h"
#include "resources/uidata.h"
#include "resources/sound.h"

#include <SDL.h>

#include <array>
#include <vector>

class cApplication;
class cBuilding;
class cMap;
class cMapField;
class cMoveJob;
class cPlayer;
class cStaticMap;

//-----------------------------------------------------------------------------
// Enum for the symbols
//-----------------------------------------------------------------------------
#ifndef D_eSymbols
#define D_eSymbols

//-----------------------------------------------------------------------------
enum eSymbols
{
	SSpeed,
	SHits,
	SAmmo,
	SMetal,
	SEnergy,
	SShots,
	SOil,
	SGold,
	STrans,
	SHuman,
	SAir
};

//-----------------------------------------------------------------------------
enum eSymbolsBig
{
	SBSpeed,
	SBHits,
	SBAmmo,
	SBAttack,
	SBShots,
	SBRange,
	SBArmor,
	SBScan,
	SBMetal,
	SBOil,
	SBGold,
	SBEnergy,
	SBHuman
};

#endif

//-----------------------------------------------------------------------------
// Struct for the pictures and sounds
//-----------------------------------------------------------------------------
struct sVehicleUIData
{
	sID id;

	bool hasCorpse;
	bool hasDamageEffect;
	bool hasPlayerColor;
	bool hasOverlay;

	bool buildUpGraphic;
	bool animationMovement;
	bool powerOnGraphic;
	bool isAnimated;
	bool makeTracks;

	int hasFrames;


	std::array<AutoSurface, 8> img, img_org; // 8 Surfaces of the vehicle
	std::array<AutoSurface, 8> shw, shw_org; // 8 Surfaces of shadows
	AutoSurface build, build_org;        // Surfaces when building
	AutoSurface build_shw, build_shw_org; // Surfaces of shadows when building
	AutoSurface clear_small, clear_small_org;        // Surfaces when clearing
	AutoSurface clear_small_shw, clear_small_shw_org; // Surfaces when clearing
	AutoSurface overlay, overlay_org;    // Overlays
	AutoSurface storage; // image of the vehicle in storage
	std::string FLCFile;       // FLC-Video
	AutoSurface info;   // info image

	// Sounds:
	cSoundChunk Wait;
	cSoundChunk WaitWater;
	cSoundChunk Start;
	cSoundChunk StartWater;
	cSoundChunk Stop;
	cSoundChunk StopWater;
	cSoundChunk Drive;
	cSoundChunk DriveWater;
	cSoundChunk Attack;

	sVehicleUIData();
	sVehicleUIData (sVehicleUIData&& other);
	sVehicleUIData& operator= (sVehicleUIData && other);
	void scaleSurfaces (float faktor);

private:
	sVehicleUIData (const sVehicleUIData& other) MAXR_DELETE_FUNCTION;
	sVehicleUIData& operator= (const sVehicleUIData& other) MAXR_DELETE_FUNCTION;
};

enum class eSupplyType {
	REARM,
	REPAIR
};

#define MAX_FLIGHT_HEIGHT 64

//-----------------------------------------------------------------------------
/** Class for a vehicle-unit of a player */
//-----------------------------------------------------------------------------
class cVehicle : public cUnit
{
	friend class cDebugOutputWidget;
	//-----------------------------------------------------------------------------
public:
	cVehicle (const cStaticUnitData& staticData,  const cDynamicUnitData& data, cPlayer* Owner, unsigned int ID);
	virtual ~cVehicle();

	bool isAVehicle() const override { return true; }
	bool isABuilding() const override { return false; }

	const cPosition& getMovementOffset() const MAXR_OVERRIDE_FUNCTION { return tileMovementOffset; }
	void setMovementOffset (const cPosition& newOffset) { tileMovementOffset = newOffset; }

	const sVehicleUIData* uiData;
	mutable int ditherX, ditherY;
	mutable int bigBetonAlpha;
	cPosition bandPosition; // X,Y Position für das Band
	cPosition buildBigSavedPosition; // last position before building has started
	bool BuildPath;   // Gibt an, ob ein Pfad gebaut werden soll
	int DamageFXPointX, DamageFXPointY; // Die Punkte, an denen Rauch bei beschädigung aufsteigen wird
	unsigned int WalkFrame; // Frame der Geh-Annimation

	/**
	* refreshes speedCur and shotsCur and continues building or clearing
	*@author alzi alias DoctorDeath
	*@return true if there has been refreshed something else false.
	*/
	bool refreshData();
	void proceedBuilding (cModel& model);
	void continuePathBuilding(cModel& model);
	void proceedClearing(cModel& model);

	std::string getStatusStr (const cPlayer* player, const cUnitsData& unitsData) const MAXR_OVERRIDE_FUNCTION;
	void DecSpeed (int value);
	bool doSurvey(const cMap& map);
	bool canTransferTo (const cPosition& position, const cMapView& map) const MAXR_OVERRIDE_FUNCTION;
	bool canTransferTo (const cUnit& position) const MAXR_OVERRIDE_FUNCTION;
	bool inSentryRange (cModel& model);
	bool canExitTo (const cPosition& position, const cMap& map, const cStaticUnitData& unitData) const MAXR_OVERRIDE_FUNCTION;
	bool canExitTo(const cPosition& position, const cMapView& map, const cStaticUnitData& unitData) const MAXR_OVERRIDE_FUNCTION;
	bool canLoad(const cPosition& position, const cMapView& map, bool checkPosition = true) const;
	bool canLoad(const cVehicle* Vehicle, bool checkPosition = true) const MAXR_OVERRIDE_FUNCTION;
	bool canSupply (const cMapView& map, const cPosition& position, eSupplyType supplyType) const;
	bool canSupply (const cUnit* unit, eSupplyType supplyType) const MAXR_OVERRIDE_FUNCTION;


	void calcTurboBuild (std::array<int, 3>& turboBuildTurns, std::array<int, 3>& turboBuildCosts, int buildCosts) const;
	/**
	* lays a mine at the current position of the unit.
	*/
	void layMine (cModel& model);
	/**
	* clear a mine at the current position of the unit.
	*/
	void clearMine (cModel& model);

	/** When starting a movement, or when unloading a stored unit, the detection state of the unit might be reset,
	 * if it was not detected in _this_ turn. */
	void tryResetOfDetectionStateBeforeMove (const cMap& map, const std::vector<std::shared_ptr<cPlayer>>& playerList);

	static void blitWithPreScale (SDL_Surface* org_src, SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect, float factor, int frames = 1);

	/**
	* Is this a plane and is there a landing platform beneath it,
	* that can be used to land on?
	* @author: eiko
	*/
	bool canLand (const cMap& map) const;

	/**
	* draws the main image of the vehicle onto the passed surface
	*/
	void render (const cMapView* map, unsigned long long animationTime, const cPlayer* activePlayer, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int alpha = 254) const;
	static void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, const sVehicleUIData& uiData, const cPlayer* owner, int dir = 0, int walkFrame = 0, int alpha = 254);
	/**
	* draws the overlay animation of the vehicle on the given surface
	*@author: eiko
	*/
	void drawOverlayAnimation (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const;
	void drawOverlayAnimation (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha = 254) const;
	static void drawOverlayAnimation (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, const sVehicleUIData& uiData, int frameNr = 0, int alpha = 254);

	bool isUnitLoaded() const { return loaded; }

	bool isUnitMoving() const MAXR_OVERRIDE_FUNCTION { return moving; } //test if the vehicle is moving right now. Having a waiting movejob doesn't count a moving
	bool isUnitClearing() const MAXR_OVERRIDE_FUNCTION { return isClearing; }
	bool isUnitLayingMines() const MAXR_OVERRIDE_FUNCTION { return layMines; }
	bool isUnitClearingMines() const MAXR_OVERRIDE_FUNCTION { return clearMines; }
	bool isUnitBuildingABuilding() const MAXR_OVERRIDE_FUNCTION { return isBuilding; }
	bool canBeStoppedViaUnitMenu() const MAXR_OVERRIDE_FUNCTION;
	bool isSurveyorAutoMoveActive() const MAXR_OVERRIDE_FUNCTION { return surveyorAutoMoveActive; };

	void setMoving (bool value);
	void setLoaded (bool value);
	void setClearing (bool value);
	void setBuildingABuilding (bool value);
	void setLayMines (bool value);
	void setClearMines (bool value);
	void setBuildTurnsStart (int value);
	void setSurveyorAutoMoveActive(bool value);

	int getClearingTurns() const;
	void setClearingTurns (int value);

	const cCommandoData& getCommandoData() const { return commandoData; }
	cCommandoData& getCommandoData() { return commandoData; }

	const sID& getBuildingType() const;
	void setBuildingType (const sID& id);
	int getBuildCosts() const;
	void setBuildCosts (int value);
	int getBuildTurns() const;
	void setBuildTurns (int value);
	int getBuildCostsStart() const;
	void setBuildCostsStart (int value);
	int getBuildTurnsStart() const;


	int getFlightHeight() const;
	void setFlightHeight (int value);

	cMoveJob* getMoveJob();
	const cMoveJob* getMoveJob() const;
	void setMoveJob (cMoveJob* moveJob);

	void triggerLandingTakeOff(cModel& model);

	/**
	* return the unit which contains this vehicle
	*/
	cBuilding* getContainerBuilding();
	cVehicle* getContainerVehicle();

	uint32_t getChecksum(uint32_t crc) const override;

	mutable cSignal<void ()> clearingTurnsChanged;
	mutable cSignal<void ()> buildingTurnsChanged;
	mutable cSignal<void ()> buildingCostsChanged;
	mutable cSignal<void ()> buildingTypeChanged;
	mutable cSignal<void ()> flightHeightChanged;

	mutable cSignal<void ()> moveJobChanged;
	mutable cSignal<void ()> autoMoveJobChanged;
	mutable cSignal<void ()> moveJobBlocked;

	template <typename T>
	void serialize(T& archive)
	{
		cUnit::serializeThis (archive); //serialize cUnit members

		archive & NVP(surveyorAutoMoveActive);
		archive & NVP(bandPosition);
		archive & NVP(buildBigSavedPosition);
		archive & NVP(BuildPath);
		archive & NVP(WalkFrame);
		archive & NVP(tileMovementOffset);
		archive & NVP(loaded);
		archive & NVP(moving);
		archive & NVP(isBuilding);
		archive & NVP(buildingTyp);
		archive & NVP(buildCosts);
		archive & NVP(buildTurns);
		archive & NVP(buildTurnsStart);
		archive & NVP(buildCostsStart);
		archive & NVP(isClearing);
		archive & NVP(clearingTurns);
		archive & NVP(layMines);
		archive & NVP(clearMines);
		archive & NVP(flightHeight);
		commandoData.serialize (archive);

		if (!archive.isWriter)
		{
			uiData = UnitsUiData.getVehicleUI(data.getId());
		}
	}
private:

	void render_BuildingOrBigClearing (const cMapView& map, unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_smallClearing (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_shadow (const cMapView& map, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const;

	//---- sentry and reaction fire helpers ------------------------------------
	/**
	 * Is called after a unit moved one field;
	 * it allows opponent units to react to that movement and
	 * fire on the moving vehicle, if they can.
	 * An opponent unit only fires as reaction to the movement,
	 * if the moving unit is an "offense" for that opponent
	 * (i.e. it could attack a unit/building of the opponent).
	 * @author: pagra
	 */
	bool provokeReactionFire (cModel& model);
	bool doesPlayerWantToFireOnThisVehicleAsReactionFire (const cModel& model, const cPlayer* player) const;
	bool makeAttackOnThis (cModel& model, cUnit* opponentUnit, const std::string& reasonForLog) const;
	bool makeSentryAttack (cModel& model, cUnit* unit) const;
	bool isOtherUnitOffendedByThis (const cModel& model, const cUnit& otherUnit) const;
	bool doReactionFire (cModel& model, cPlayer* player) const;
	bool doReactionFireForUnit (cModel& model, cUnit* opponentUnit) const;

	cPosition tileMovementOffset;  // offset within tile during movement

	bool moving;
	cMoveJob* moveJob;
	bool surveyorAutoMoveActive;

	bool loaded;

	bool isBuilding;
	sID buildingTyp;
	int buildCosts;
	int buildTurns;
	int buildTurnsStart;
	int buildCostsStart;

	bool isClearing;
	int clearingTurns;

	bool layMines;
	bool clearMines;

	int flightHeight;

	cCommandoData commandoData;
};

#endif // game_data_units_vehicleH
