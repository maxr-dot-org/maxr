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

#ifndef game_data_player_playerH
#define game_data_player_playerH

#include <SDL.h>
#include <cassert>
#include <string>
#include <vector>

#include "defines.h"
#include "game/data/base/base.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/unit.h" // sUnitLess
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/upgradecalculator.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/flatset.h"
#include "utility/serialization/serialization.h"
#include "utility/arraycrc.h"
#include "game/data/rangemap.h"
#include "playercolor.h"

class cHud;
class cMapField;
class cUnit;
class cPosition;
struct sTerrain;
class cPlayerBasicData;

/**
* Structure for generating the report about finished units at turn start
*/
struct sTurnstartReport
{
	template <typename T>
	void serialize(T& archive)
	{
		archive & NVP(type);
		archive & NVP(count);
	}

	/** unit type of the report */
	sID type;
	/** counter for this report */
	int count;
};

class cSavedReport;

// the Player class //////////////////////////////
class cPlayer
{
	cPlayer (const cPlayer&) MAXR_DELETE_FUNCTION;
public:
	explicit cPlayer(const cPlayerBasicData& splayer, const cUnitsData& unitsData);
	~cPlayer();

	const std::string& getName() const { return name; }

	bool isHuman() const { return true; } // only human players are implemented yet.
	const cPlayerColor& getColor() const { return color; }

	int getId() const { return id; }

	int getCredits() const;
	void setCredits (int credits);

	/** Get the most modern version of a unit (including all his upgrades). */
	cDynamicUnitData* getUnitDataCurrentVersion (const sID& id);
	const cDynamicUnitData* getUnitDataCurrentVersion (const sID& id) const;

	void setLandingPos (const cPosition& position) { landingPos = position; }
	const cPosition& getLandingPos() const { return landingPos; };

	void initMaps (cMap& map);
	const cPosition& getMapSize() const;

	/**
	* Update the scan and detection maps of the player. These maps control,
	* where a player can see other units.
	*/
	void addToScan(const cUnit& unit);
	void updateScan(const cUnit& unit, const cPosition& newPosition, bool newIsBig = false);
	void updateScan(const cUnit& unit, int newScanRange);
	void removeFromScan(const cUnit& unit);

	const cRangeMap& getScanMap() const;

	void revealResource();
	unsigned int getOffset (const cPosition& pos) const { return pos.x() + pos.y() * mapSize.x(); }
	/**
	* Check weather any part of the unit is covered by the scan area. Do not use this to check, weather
	* a unit is actually visible. Use canSeeUnit() for this purpose
	*/
	bool canSeeAnyAreaUnder (const cUnit& unit) const;
	/**
	* Check weather unit is visible for the player. The check includes all necessary conditions,
	* including owner, scan area, stealth abilities and detection state.
	*/
	bool canSeeUnit(const cUnit& unit, const cMap& map) const;
	bool canSeeUnit(const cUnit& unit, const cMapField& field, const sTerrain& terrain) const;
	/**
	* Check weather the scan area covers position. Do not use this to check, weather
	* a unit on the position is actually visible. Use canSeeUnit() for this purpose
	*/
	bool canSeeAt(const cPosition& position) const;

	cVehicle& addNewVehicle (const cPosition& position, const cStaticUnitData& unitData, unsigned int uid);
	cBuilding& addNewBuilding (const cPosition& position, const cStaticUnitData& unitData, unsigned int uid);

	void addUnit (std::shared_ptr<cVehicle> vehicle);
	void addUnit (std::shared_ptr<cBuilding> building);

	std::shared_ptr<cBuilding> removeUnit (const cBuilding& building);
	std::shared_ptr<cVehicle> removeUnit (const cVehicle& vehicle);

	void removeAllUnits();

	cVehicle* getVehicleFromId (unsigned int id) const;
	cBuilding* getBuildingFromId (unsigned int id) const;

	const cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>>& getVehicles() const;
	const cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>>& getBuildings() const;

	bool hasUnits() const;

	void addToSentryMap (const cUnit& u);
	void removeFromSentryMap (const cUnit& u);
	void updateSentry (const cUnit& u, int newRange);

	void upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, const cUnitsData& originalUnitsData);

	/** return the number of running ecospheres */
	int getNumEcoSpheres() const;
	/** gets the score value from the score history */
	int getScore (unsigned int turn) const;
	/** gets the current score of the player */
	int getScore() const;
	/** change the score of the player for the current turn by the given value */
	void changeScore (int score);
	/** count generated points at turn end and and create a new entry in the points history */
	void accumulateScore ();

	void setClan (int newClan, const cUnitsData& unitsData);
	int getClan() const { return clan; }

	bool getHasFinishedTurn() const;
	void setHasFinishedTurn (bool value);

	void exploreResource (const cPosition& pos) { resourceMap.set(getOffset (pos), 1); }
	bool hasResourceExplored (const cPosition& pos) const { return resourceMap[getOffset (pos)] != 0; }
	bool hasSentriesAir (const cPosition& pos) const { return sentriesMapAir.get(pos); }
	bool hasSentriesGround (const cPosition& pos) const { return sentriesMapGround.get(pos); }
	bool hasLandDetection (const cPosition& pos) const { return detectLandMap.get(pos); }
	bool hasMineDetection (const cPosition& pos) const { return detectMinesMap.get(pos); }
	bool hasSeaDetection (const cPosition& pos) const { return detectSeaMap.get(pos); }

	void doResearch(const cUnitsData& unitsData);  // proceed with the research at turn end

	void refreshSentryMaps();

	bool mayHaveOffensiveUnit() const;

	void addTurnReportUnit (const sID& unitId);
	void resetTurnReportData();
	const std::vector<sTurnstartReport>& getCurrentTurnUnitReports() const;

	const std::vector<int>& getCurrentTurnResearchAreasFinished() const;
	void setCurrentTurnResearchAreasFinished (std::vector<int> areas);

	bool isCurrentTurnResearchAreaFinished (cResearch::ResearchArea area) const;

	const cResearch& getResearchState() const;
	cResearch& getResearchState();

	int getResearchCentersWorkingTotal() const;
	int getResearchCentersWorkingOnArea (cResearch::ResearchArea area) const;

	void startAResearch (cResearch::ResearchArea researchArea);
	void stopAResearch (cResearch::ResearchArea researchArea);

	void refreshResearchCentersWorkingOnArea();
	void refreshBase(const cMap& map);

	void makeTurnStart(cModel& model);

	uint32_t getChecksum(uint32_t crc) const;

	mutable cSignal<void ()> creditsChanged;
	mutable cSignal<void ()> hasFinishedTurnChanged;
	mutable cSignal<void (cResearch::ResearchArea)> researchCentersWorkingOnAreaChanged;
	mutable cSignal<void ()> researchCentersWorkingTotalChanged;
	mutable cSignal<void ()> turnEndMovementsStarted;
	mutable cSignal<void (const cUnit& unit)> unitDestroyed;
	mutable cSignal<void (const cUnit& unit)> unitAttacked;
	mutable cSignal<void ()> buildErrorBuildPositionBlocked;
	mutable cSignal<void ()> buildErrorInsufficientMaterial;
	mutable cSignal<void (const cUnit& unit)> buildPathInterrupted;
	mutable cSignal<void (const cUnit& unit)> detectedStealthUnit;
	mutable cSignal<void (const cUnit& unit)> stealthUnitDissappeared;
	mutable cSignal<void (const sID& unitId, int unitsCount, int costs)> unitsUpgraded;

	template <typename T>
	void save(T& archive)
	{
		archive & NVP(name);
		archive & NVP(id);
		archive & NVP(color);
		archive & NVP(dynamicUnitsData);
		archive & serialization::makeNvp("vehicleNum", (int)vehicles.size());
		for (auto vehicle : vehicles)
		{
			archive & serialization::makeNvp("vehicleID", vehicle->getId());
			archive & serialization::makeNvp("vehicle", *vehicle);
		}
		archive & serialization::makeNvp("buildingNum", (int)buildings.size());
		for (auto building : buildings)
		{
			archive & serialization::makeNvp("buildingID", building->getId());
			archive & serialization::makeNvp("building", *building);
		}
		archive & NVP(landingPos);
		archive & serialization::makeNvp("ResourceMap", resourceMapToString());
		archive & NVP(pointsHistory);
		archive & NVP(isDefeated);
		archive & NVP(clan);
		archive & NVP(credits);
		archive & NVP(currentTurnResearchAreasFinished);
		archive & NVP(hasFinishedTurn);
		archive & NVP(researchState);
	}
	template<typename T>
	void load(T& archive)
	{
		archive & NVP(name);
		archive & NVP(id);
		archive & NVP(color);

		dynamicUnitsData.clear();
		archive & NVP(dynamicUnitsData);

		vehicles.clear();
		int vehicleNum;
		archive & NVP(vehicleNum);
		for (int i = 0; i < vehicleNum; i++)
		{
			unsigned int vehicleID;
			archive & NVP(vehicleID);
			cStaticUnitData dummy1;
			cDynamicUnitData dummy2;
			auto vehicle = std::make_shared<cVehicle>(dummy1, dummy2, this, vehicleID);
			archive & serialization::makeNvp("vehicle", *vehicle);
			vehicles.insert(std::move(vehicle));
		}

		buildings.clear();
		int buildingNum;
		archive & NVP(buildingNum);
		for (int i = 0; i < buildingNum; i++)
		{
			unsigned int buildingID;
			archive & NVP(buildingID);
			auto building = std::make_shared<cBuilding>(nullptr, nullptr, this, buildingID);
			archive & serialization::makeNvp("building", *building);
			buildings.insert(std::move(building));
		}

		archive & NVP(landingPos);

		std::string ResourceMap;
		archive & NVP(ResourceMap);
		setResourceMapFromString(ResourceMap);

		archive & NVP(pointsHistory);
		archive & NVP(isDefeated);
		archive & NVP(clan);
		archive & NVP(credits);
		archive & NVP(currentTurnResearchAreasFinished);
		archive & NVP(hasFinishedTurn);
		archive & NVP(researchState);

		hasFinishedTurnChanged(); //FIXME: deserialization does not trigger signals on changed data members. But this signal is needed for the gui after loading a save game...
		refreshScanMaps();
		refreshSentryMaps();
		refreshResearchCentersWorkingOnArea();
	}
	SERIALIZATION_SPLIT_MEMBER()
public:
	std::vector<cDynamicUnitData> dynamicUnitsData; // Current version of vehicles.
	cBase base;               // the base (groups of connected buildings) of the player
	bool isDefeated;        // true if the player has been defeated
	int numEcos;            // number of ecospheres. call countEcoSpheres to update.

private:
	std::string resourceMapToString() const;
	void setResourceMapFromString(const std::string& str);

	void refreshScanMaps();

	std::string name;
	cPlayerColor color;
	int id;

	cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>> vehicles;
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> buildings;

	cPosition landingPos;
	cPosition mapSize; // Width and Height of the map.

	// using a special array with cached checksum. This speeds up the calculation of the model checksum.
	cArrayCrc<uint8_t> resourceMap;        /** Map with explored resources. */
	cRangeMap sentriesMapAir;     /** the covered air area */
	cRangeMap sentriesMapGround;  /** the covered ground area */
	cRangeMap scanMap;            /** seen Map tiles. */
	cRangeMap detectLandMap;      /** the area where the player can detect land stealth units */
	cRangeMap detectSeaMap;       /** the area where the player can detect sea stealth units */
	cRangeMap detectMinesMap;     /** the area where the player can detect mines */
	std::vector<int> pointsHistory; // history of player's total score (from eco-spheres) for graph

	int clan;
	int credits;

	std::vector<sTurnstartReport> currentTurnUnitReports; //TODO: move somewhere else. Shouldn't be part of the game model
	std::vector<int> currentTurnResearchAreasFinished;

	bool hasFinishedTurn;

	cResearch researchState;   ///< stores the current research level of the player
	int researchCentersWorkingOnArea[cResearch::kNrResearchAreas]; ///< counts the number of research centers that are currently working on each area
	int researchCentersWorkingTotal;  ///< number of working research centers
};

#endif // game_data_player_playerH
