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

#include "game/data/units/commandodata.h"
#include "game/data/units/unit.h"
#include "game/data/units/unitdata.h"

#include <array>
#include <vector>

class cBuilding;
class cMap;
class cMapField;
class cMoveJob;
class cModel;
class cPlayer;
class cStaticMap;

struct sNewTurnPlayerReport;

enum class eSupplyType
{
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
	cVehicle (unsigned int ID); // used by serialization
	cVehicle (const cStaticUnitData&, const cDynamicUnitData&, cPlayer* Owner, unsigned int ID);
	virtual ~cVehicle();

	bool isAVehicle() const override { return true; }
	bool isABuilding() const override { return false; }

	const sStaticVehicleData& getStaticData() const { return getStaticUnitData().vehicleData; }

	const cPosition& getMovementOffset() const override { return tileMovementOffset; }
	void setMovementOffset (const cPosition& newOffset) { tileMovementOffset = newOffset; }

	void refreshData();
	void proceedBuilding (cModel&, sNewTurnPlayerReport&);
	void continuePathBuilding (cModel&);
	void proceedClearing (cModel&);

	int getPossibleShotCountForSpeed (int speed) const;
	void DecSpeed (int value);
	bool doSurvey (const cMap& map);
	bool canTransferTo (const cPosition& position, const cMapView& map) const override;
	bool canTransferTo (const cUnit& position) const override;
	bool inSentryRange (cModel& model);
	bool canExitTo (const cPosition& position, const cMap& map, const cStaticUnitData& unitData) const override;
	bool canExitTo (const cPosition& position, const cMapView& map, const cStaticUnitData& unitData) const override;
	bool canLoad (const cPosition& position, const cMapView& map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const override;
	bool canSupply (const cMapView& map, const cPosition& position, eSupplyType supplyType) const;
	bool canSupply (const cUnit* unit, eSupplyType supplyType) const override;

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

	/**
	* Is this a plane and is there a landing platform beneath it,
	* that can be used to land on?
	* @author: eiko
	*/
	bool canLand (const cMap& map) const;

	bool isUnitLoaded() const { return loaded; }

	bool isUnitMoving() const { return moving; } //test if the vehicle is moving right now. Having a waiting movejob doesn't count a moving
	bool isUnitClearing() const { return isClearing; }
	bool isUnitLayingMines() const { return layMines; }
	bool isUnitClearingMines() const { return clearMines; }
	bool isUnitBuildingABuilding() const { return isBuilding; }
	bool canBeStoppedViaUnitMenu() const override;
	bool isSurveyorAutoMoveActive() const { return surveyorAutoMoveActive; }

	void setMoving (bool value);
	void setLoaded (bool value);
	void setClearing (bool value);
	void setBuildingABuilding (bool value);
	void setLayMines (bool value);
	void setClearMines (bool value);
	void setBuildTurnsStart (int value);
	void setSurveyorAutoMoveActive (bool value);

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

	void triggerLandingTakeOff (cModel& model);

	uint32_t getChecksum (uint32_t crc) const override;

	mutable cSignal<void()> clearingTurnsChanged;
	mutable cSignal<void()> buildingTurnsChanged;
	mutable cSignal<void()> buildingCostsChanged;
	mutable cSignal<void()> buildingTypeChanged;
	mutable cSignal<void()> flightHeightChanged;

	mutable cSignal<void()> moveJobChanged;
	mutable cSignal<void()> autoMoveJobChanged;
	mutable cSignal<void()> moveJobBlocked;

	template <typename Archive>
	static std::unique_ptr<cVehicle> createFrom (Archive& archive)
	{
		int id;
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (id);
		// clang-format on
		auto res = std::make_unique<cVehicle> (id);
		res->serialize (archive);
		return res;
	}

	template <typename Archive>
	void serialize (Archive& archive)
	{
		cUnit::serializeThis (archive); //serialize cUnit members

		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (surveyorAutoMoveActive);
		archive & NVP (bandPosition);
		archive & NVP (buildBigSavedPosition);
		archive & NVP (BuildPath);
		archive & NVP (WalkFrame);
		archive & NVP (tileMovementOffset);
		archive & NVP (loaded);
		archive & NVP (moving);
		archive & NVP (isBuilding);
		archive & NVP (buildingTyp);
		archive & NVP (buildCosts);
		archive & NVP (buildTurns);
		archive & NVP (buildTurnsStart);
		archive & NVP (buildCostsStart);
		archive & NVP (isClearing);
		archive & NVP (clearingTurns);
		archive & NVP (layMines);
		archive & NVP (clearMines);
		archive & NVP (flightHeight);
		// clang-format on
		commandoData.serialize (archive);
	}

private:
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

public:
	mutable cPosition dither;
	mutable int bigBetonAlpha = 254;
	cPosition bandPosition; // X,Y Position für das Band
	cPosition buildBigSavedPosition; // last position before building has started
	bool BuildPath = false; // Gibt an, ob ein Pfad gebaut werden soll
	cPosition DamageFXPoint; // Die Punkte, an denen Rauch bei beschädigung aufsteigen wird
	unsigned int WalkFrame = 0; // Frame der Geh-Annimation
private:
	cPosition tileMovementOffset; // offset within tile during movement

	bool moving = false;
	cMoveJob* moveJob = nullptr;
	bool surveyorAutoMoveActive = false;

	bool loaded = false;

	bool isBuilding = false;
	sID buildingTyp;
	int buildCosts = 0;
	int buildTurns = 0;
	int buildTurnsStart = 0;
	int buildCostsStart = 0;

	bool isClearing = false;
	int clearingTurns = 0;

	bool layMines = false;
	bool clearMines = false;

	int flightHeight = 0;

	cCommandoData commandoData;
};

#endif // game_data_units_vehicleH
