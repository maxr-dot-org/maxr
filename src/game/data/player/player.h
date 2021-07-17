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

#include <cassert>
#include <string>
#include <vector>

#include "game/data/base/base.h"
#include "game/data/rangemap.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/unit.h" // sUnitLess
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/upgradecalculator.h"
#include "game/serialization/serialization.h"
#include "utility/arraycrc.h"
#include "utility/color.h"
#include "utility/flatset.h"
#include "utility/position.h"
#include "utility/ranges.h"
#include "utility/signal/signal.h"

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
	template <typename Archive>
	void serialize (Archive& archive)
	{
		archive & NVP (type);
		archive & NVP (count);
	}

	/** unit type of the report */
	sID type;
	/** counter for this report */
	int count;
};

struct sNewTurnPlayerReport
{
public:
	void addUnitBuilt (const sID& unitTypeId);
public:
	std::vector<cResearch::ResearchArea> finishedResearchs;
	std::vector<sTurnstartReport> unitsBuilt;
};

// the Player class //////////////////////////////
class cPlayer
{
	cPlayer (const cPlayer&) = delete;
public:
	cPlayer (const cPlayerBasicData&, const cUnitsData&);
	~cPlayer();

	const std::string& getName() const { return name; }

	bool isHuman() const { return true; } // only human players are implemented yet.
	const cRgbColor& getColor() const { return color; }

	int getId() const { return id; }

	int getCredits() const;
	void setCredits (int credits);

	/** Get the most modern version of a unit (including all his upgrades). */
	cDynamicUnitData* getUnitDataCurrentVersion (const sID&);
	const cDynamicUnitData* getUnitDataCurrentVersion (const sID&) const;

	void setLandingPos (const cPosition& position) { landingPos = position; }
	const cPosition& getLandingPos() const { return landingPos; }

	void initMaps (cMap&);
	const cPosition& getMapSize() const;

	/**
	* Update the scan and detection maps of the player. These maps control,
	* where a player can see other units.
	*/
	void addToScan (const cUnit&);
	void updateScan (const cUnit&, const cPosition& newPosition, bool newIsBig = false);
	void updateScan (const cUnit&, int newScanRange);
	void removeFromScan (const cUnit&);

	const cRangeMap& getScanMap() const;

	void revealResource();
	unsigned int getOffset (const cPosition& pos) const { return pos.x() + pos.y() * mapSize.x(); }
	/**
	* Check weather any part of the unit is covered by the scan area. Do not use this to check, weather
	* a unit is actually visible. Use canSeeUnit() for this purpose
	*/
	bool canSeeAnyAreaUnder (const cUnit&) const;
	/**
	* Check weather unit is visible for the player. The check includes all necessary conditions,
	* including owner, scan area, stealth abilities and detection state.
	*/
	bool canSeeUnit (const cUnit&, const cMap&) const;
	bool canSeeUnit (const cUnit&, const cMapField&, const sTerrain&) const;
	/**
	* Check weather the scan area covers position. Do not use this to check, weather
	* a unit on the position is actually visible. Use canSeeUnit() for this purpose
	*/
	bool canSeeAt (const cPosition&) const;

	void addUnit (std::shared_ptr<cVehicle>);
	void addUnit (std::shared_ptr<cBuilding>);

	std::shared_ptr<cBuilding> removeUnit (const cBuilding&);
	std::shared_ptr<cVehicle> removeUnit (const cVehicle&);

	void removeAllUnits();

	cVehicle* getVehicleFromId (unsigned int id) const;
	cBuilding* getBuildingFromId (unsigned int id) const;

	const cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>>& getVehicles() const;
	const cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>>& getBuildings() const;

	bool hasUnits() const;

	void addToSentryMap (const cUnit&);
	void removeFromSentryMap (const cUnit&);
	void updateSentry (const cUnit&, int newRange);


	/** return the number of running ecospheres */
	int getNumEcoSpheres() const;
	/** gets the score value from the score history */
	int getScore (unsigned int turn) const;
	/** gets the current score of the player */
	int getScore() const;
	/** change the score of the player for the current turn by the given value */
	void changeScore (int score);
	/** count generated points at turn end and and create a new entry in the points history */
	void accumulateScore();

	void setClan (int newClan, const cUnitsData&);
	int getClan() const { return clan; }

	bool getHasFinishedTurn() const;
	void setHasFinishedTurn (bool value);

	void exploreResource (const cPosition& pos) { resourceMap.set (getOffset (pos), 1); }
	bool hasResourceExplored (const cPosition& pos) const { return resourceMap[getOffset (pos)] != 0; }
	bool hasSentriesAir (const cPosition& pos) const { return sentriesMapAir.get (pos); }
	bool hasSentriesGround (const cPosition& pos) const { return sentriesMapGround.get (pos); }
	bool hasLandDetection (const cPosition& pos) const { return detectLandMap.get (pos); }
	bool hasMineDetection (const cPosition& pos) const { return detectMinesMap.get (pos); }
	bool hasSeaDetection (const cPosition& pos) const { return detectSeaMap.get (pos); }

	std::vector<cResearch::ResearchArea> doResearch (const cUnitsData&);  // proceed with the research at turn end

	void refreshSentryMaps();

	bool mayHaveOffensiveUnit() const;

	const cResearch& getResearchState() const;
	cResearch& getResearchState();

	int getResearchCentersWorkingTotal() const;
	int getResearchCentersWorkingOnArea (cResearch::ResearchArea) const;

	void startAResearch (cResearch::ResearchArea);
	void stopAResearch (cResearch::ResearchArea);

	void refreshResearchCentersWorkingOnArea();
	void refreshBase (const cMap&);

	sNewTurnPlayerReport makeTurnStart (cModel&);

	uint32_t getChecksum (uint32_t crc) const;

	mutable cSignal<void()> creditsChanged;
	mutable cSignal<void()> hasFinishedTurnChanged;
	mutable cSignal<void (cResearch::ResearchArea)> researchCentersWorkingOnAreaChanged;
	mutable cSignal<void()> researchCentersWorkingTotalChanged;
	mutable cSignal<void()> turnEndMovementsStarted;
	mutable cSignal<void (const cUnit&)> unitDestroyed;
	mutable cSignal<void (const cUnit&)> unitAttacked;
	mutable cSignal<void()> buildErrorBuildPositionBlocked;
	mutable cSignal<void()> buildErrorInsufficientMaterial;
	mutable cSignal<void (const cUnit&)> buildPathInterrupted;
	mutable cSignal<void (const cUnit&)> detectedStealthUnit;
	mutable cSignal<void (const cUnit&)> stealthUnitDissappeared;
	mutable cSignal<void (const sID&, int unitsCount, int costs)> unitsUpgraded;

	template <typename Archive>
	void save (Archive& archive)
	{
		archive & NVP (name);
		archive & NVP (id);
		archive & NVP (color);
		archive & NVP (dynamicUnitsData);
		archive & serialization::makeNvp ("vehicleNum", (int)vehicles.size());
		// should be saved in "correct order"
		// references first to allow to restore pointers.
		//
		// More complex case to handle:
		// commando in car, which is in air transport which is in hangar.
		// topological sort might be applied.
		// but 3 passes on vehicles should be enough.
		const auto hasStoredUnits = [](const auto& vehicle){ return !vehicle->storedUnits.empty(); };
		const std::function<bool (const std::shared_ptr<cVehicle>&)> filters[] =
		{
			[&](const auto& vehicle){ return !hasStoredUnits (vehicle); },
			[&](const auto& vehicle){ return hasStoredUnits (vehicle) && ranges::none_of (vehicle->storedUnits, hasStoredUnits);},
			[&](const auto& vehicle){ return hasStoredUnits (vehicle) && ranges::any_of (vehicle->storedUnits, hasStoredUnits);}
		};
		for (auto filter : filters)
		{
			for (auto vehicle : vehicles)
			{
				if (filter (vehicle))
				{
					archive & serialization::makeNvp ("vehicleID", vehicle->getId());
					archive & serialization::makeNvp ("vehicle", *vehicle);
				}
			}
		}
		archive & serialization::makeNvp ("buildingNum", (int)buildings.size());
		for (auto building : buildings)
		{
			archive & serialization::makeNvp ("buildingID", building->getId());
			archive & serialization::makeNvp ("building", *building);
		}
		archive & NVP (landingPos);
		archive & serialization::makeNvp ("ResourceMap", resourceMapToString());
		archive & NVP (pointsHistory);
		archive & NVP (isDefeated);
		archive & NVP (clan);
		archive & NVP (credits);
		archive & NVP (hasFinishedTurn);
		archive & NVP (researchState);
	}
	template <typename Archive>
	void load (Archive& archive)
	{
		archive & NVP (name);
		archive & NVP (id);
		archive & NVP (color);

		dynamicUnitsData.clear();
		archive & NVP (dynamicUnitsData);

		vehicles.clear();
		int vehicleNum;
		archive & NVP (vehicleNum);
		for (int i = 0; i < vehicleNum; i++)
		{
			unsigned int vehicleID;
			archive & NVP (vehicleID);
			cStaticUnitData dummy1;
			cDynamicUnitData dummy2;
			auto vehicle = std::make_shared<cVehicle> (dummy1, dummy2, this, vehicleID);
			archive & serialization::makeNvp ("vehicle", *vehicle);
			vehicles.insert (std::move (vehicle));
		}

		buildings.clear();
		int buildingNum;
		archive & NVP (buildingNum);
		for (int i = 0; i < buildingNum; i++)
		{
			unsigned int buildingID;
			archive & NVP (buildingID);
			auto building = std::make_shared<cBuilding> (nullptr, nullptr, this, buildingID);
			archive & serialization::makeNvp ("building", *building);
			buildings.insert (std::move (building));
		}

		archive & NVP (landingPos);

		std::string ResourceMap;
		archive & NVP (ResourceMap);
		setResourceMapFromString (ResourceMap);

		archive & NVP (pointsHistory);
		archive & NVP (isDefeated);
		archive & NVP (clan);
		archive & NVP (credits);
		archive & NVP (hasFinishedTurn);
		archive & NVP (researchState);

		hasFinishedTurnChanged(); //FIXME: deserialization does not trigger signals on changed data members. But this signal is needed for the gui after loading a save game...
		refreshScanMaps();
		refreshSentryMaps();
		refreshResearchCentersWorkingOnArea();
	}
	SERIALIZATION_SPLIT_MEMBER()
private:
	void upgradeUnitTypes (const std::vector<cResearch::ResearchArea>&, const cUnitsData& originalUnitsData);

	std::string resourceMapToString() const;
	void setResourceMapFromString (const std::string&);

	void refreshScanMaps();

public:
	std::vector<cDynamicUnitData> dynamicUnitsData; // Current version of vehicles.
	cBase base;               // the base (groups of connected buildings) of the player
	bool isDefeated;        // true if the player has been defeated
	int numEcos;            // number of ecospheres. call countEcoSpheres to update.

private:
	std::string name;
	cRgbColor color;
	int id;

	cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>> vehicles;
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> buildings;

	cPosition landingPos {-1, -1};
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


	bool hasFinishedTurn;

	cResearch researchState;   ///< stores the current research level of the player
	int researchCentersWorkingOnArea[cResearch::kNrResearchAreas]; ///< counts the number of research centers that are currently working on each area
	int researchCentersWorkingTotal;  ///< number of working research centers
};

#endif // game_data_player_playerH
