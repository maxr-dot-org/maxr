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

#include "game/data/base/base.h"
#include "game/data/player/playersettings.h"
#include "game/data/rangemap.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h" // sUnitLess
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "game/logic/upgradecalculator.h"
#include "utility/arraycrc.h"
#include "utility/color.h"
#include "utility/flatset.h"
#include "utility/position.h"
#include "utility/ranges.h"
#include "utility/serialization/serialization.h"
#include "utility/signal/signal.h"

#include <string>
#include <vector>

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
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (type);
		archive & NVP (count);
		// clang-format on
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
	std::vector<cResearch::eResearchArea> finishedResearchs;
	std::vector<sTurnstartReport> unitsBuilt;
};

// the Player class //////////////////////////////
class cPlayer
{
	cPlayer (const cPlayer&) = delete;

public:
	cPlayer(); // used by serialization
	cPlayer (const cPlayerBasicData&, const cUnitsData&);
	~cPlayer();

	const std::string& getName() const { return player.name; }
	const cRgbColor& getColor() const { return player.color; }

	bool isHuman() const { return true; } // only human players are implemented yet.

	int getId() const { return id; }

	int getCredits() const { return credits; }
	void setCredits (int credits);

	/** Get the most modern version of a unit (including all its upgrades). */
	cDynamicUnitData* getLastUnitData (const sID&);
	const cDynamicUnitData* getLastUnitData (const sID&) const;

	void setLandingPos (const cPosition& position) { landingPos = position; }
	const cPosition& getLandingPos() const { return landingPos; }

	void initMaps (const cPosition& mapSize);
	const cPosition& getMapSize() const { return mapSize; }

	/**
	* Update the scan and detection maps of the player. These maps control,
	* where a player can see other units.
	*/
	void addToScan (const cUnit&);
	void updateScan (const cUnit&, const cPosition& newPosition, bool newIsBig = false);
	void updateScan (const cUnit&, int newScanRange);
	void removeFromScan (const cUnit&);

	const cRangeMap& getScanMap() const { return scanMap; }

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

	const cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>>& getVehicles() const { return vehicles; }
	const cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>>& getBuildings() const { return buildings; }

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

	bool getHasFinishedTurn() const { return hasFinishedTurn; }
	void setHasFinishedTurn (bool value);

	void exploreResource (const cPosition& pos) { resourceMap.set (getOffset (pos), 1); }
	bool hasResourceExplored (const cPosition& pos) const { return resourceMap[getOffset (pos)] != 0; }
	bool hasSentriesAir (const cPosition& pos) const { return sentriesMapAir.get (pos); }
	bool hasSentriesGround (const cPosition& pos) const { return sentriesMapGround.get (pos); }
	bool hasLandDetection (const cPosition& pos) const { return detectLandMap.get (pos); }
	bool hasMineDetection (const cPosition& pos) const { return detectMinesMap.get (pos); }
	bool hasSeaDetection (const cPosition& pos) const { return detectSeaMap.get (pos); }

	std::vector<cResearch::eResearchArea> doResearch (const cUnitsData&); // proceed with the research at turn end

	void refreshSentryMaps();

	bool mayHaveOffensiveUnit() const;

	const cResearch& getResearchState() const { return researchState; }
	cResearch& getResearchState() { return researchState; }

	int getResearchCentersWorkingTotal() const { return researchCentersWorkingTotal; }
	int getResearchCentersWorkingOnArea (cResearch::eResearchArea) const;

	void startAResearch (cResearch::eResearchArea);
	void stopAResearch (cResearch::eResearchArea);

	void refreshResearchCentersWorkingOnArea();
	void refreshBase (const cMap&);

	sNewTurnPlayerReport makeTurnStart (cModel&);

	uint32_t getChecksum (uint32_t crc) const;

	mutable cSignal<void()> creditsChanged;
	mutable cSignal<void()> hasFinishedTurnChanged;
	mutable cSignal<void (cResearch::eResearchArea)> researchCentersWorkingOnAreaChanged;
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
	static std::unique_ptr<cPlayer> createFrom (Archive& archive)
	{
		auto res = std::make_unique<cPlayer>();
		res->serialize (archive);
		return res;
	}

	template <typename Archive>
	void save (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312

		archive & NVP (player);
		archive & NVP (id);
		archive & NVP (dynamicUnitsData);
		// clang-format on

		// should be saved in "correct order"
		// references first to allow to restore pointers.
		//
		// More complex case to handle:
		// commando in car, which is in air transport which is in hangar.
		// topological sort might be applied.
		// but 3 passes on vehicles should be enough.
		const auto hasStoredUnits = [] (const auto& vehicle) { return !vehicle->storedUnits.empty(); };
		const std::function<bool (const std::shared_ptr<cVehicle>&)> filters[] =
			{
				[&] (const auto& vehicle) { return !hasStoredUnits (vehicle); },
				[&] (const auto& vehicle) { return hasStoredUnits (vehicle) && ranges::none_of (vehicle->storedUnits, hasStoredUnits); },
				[&] (const auto& vehicle) { return hasStoredUnits (vehicle) && ranges::any_of (vehicle->storedUnits, hasStoredUnits); }};
		std::vector<std::shared_ptr<cVehicle>> orderedVehicles;
		for (const auto& filter : filters)
		{
			for (const auto& vehicle : vehicles)
			{
				if (filter (vehicle))
				{
					orderedVehicles.push_back (vehicle);
				}
			}
		}
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312

		archive & serialization::makeNvp ("vehicles", orderedVehicles);
		archive & NVP (buildings);

		archive & NVP (mapSize);
		archive & NVP (landingPos);
		archive & serialization::makeNvp ("ResourceMap", resourceMapToString());
		archive & NVP (pointsHistory);
		archive & NVP (isDefeated);
		archive & NVP (clan);
		archive & NVP (credits);
		archive & NVP (hasFinishedTurn);
		archive & NVP (researchState);
		// clang-format on
	}
	template <typename Archive>
	void load (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (player);
		archive & NVP (id);

		dynamicUnitsData.clear();
		archive & NVP (dynamicUnitsData);

		archive & NVP (vehicles);
		archive & NVP (buildings);

		for (auto& vehicle : vehicles)
		{
			vehicle->setOwner (this);
		}
		for (auto& building : buildings)
		{
			building->setOwner (this);
		}

		archive & NVP (mapSize);
		initMaps (mapSize);
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
		// clang-format on
	}

	SERIALIZATION_SPLIT_MEMBER()

	void postLoad (cModel&);

private:
	void upgradeUnitTypes (const std::vector<cResearch::eResearchArea>&, const cUnitsData& originalUnitsData);

	std::string resourceMapToString() const;
	void setResourceMapFromString (const std::string&);

	void refreshScanMaps();

private:
	std::vector<cDynamicUnitData> dynamicUnitsData; // Current version of vehicles.

public:
	bool isDefeated = false; // true if the player has been defeated

private:
	sPlayerSettings player;
	int id = -1;

	cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>> vehicles;
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> buildings;

public:
	cBase base; // the base (groups of connected buildings) of the player

private:
	cPosition landingPos{-1, -1};
	cPosition mapSize; // Width and Height of the map.

	// using a special array with cached checksum. This speeds up the calculation of the model checksum.
	cArrayCrc<uint8_t> resourceMap; /** Map with explored resources. */
	cRangeMap sentriesMapAir; /** the covered air area */
	cRangeMap sentriesMapGround; /** the covered ground area */
	cRangeMap scanMap; /** seen Map tiles. */
	cRangeMap detectLandMap; /** the area where the player can detect land stealth units */
	cRangeMap detectSeaMap; /** the area where the player can detect sea stealth units */
	cRangeMap detectMinesMap; /** the area where the player can detect mines */
	std::vector<int> pointsHistory; // history of player's total score (from eco-spheres) for graph

	int clan = -1;
	int credits = 0;

	bool hasFinishedTurn = false;

	cResearch researchState; ///< stores the current research level of the player
	std::array<int, cResearch::kNrResearchAreas> researchCentersWorkingOnArea{}; ///< counts the number of research centers that are currently working on each area
	int researchCentersWorkingTotal = 0; ///< number of working research centers
};

#endif // game_data_player_playerH
