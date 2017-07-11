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
#include "main.h" // for sID
#include "game/data/units/unit.h" // sUnitLess
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/upgradecalculator.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/flatset.h"
#include "game/data/player/playerbasicdata.h"
#include "utility/serialization/serialization.h"
#include "utility/arraycrc.h"

class cHud;
class cMapField;
class cUnit;
class cPosition;

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

typedef std::vector<int> PointsHistory;

// the Player class //////////////////////////////
class cPlayer
{
	cPlayer (const cPlayer&) MAXR_DELETE_FUNCTION;
public:
	explicit cPlayer(const cPlayerBasicData& splayer_, const cUnitsData& unitsData);
	~cPlayer();

	const std::string& getName() const { return splayer.getName(); }
	void setName (const std::string& name) { splayer.setName (name); }

	bool isHuman() const { return true; } // only human players are implemented yet.
	const cPlayerColor& getColor() const { return splayer.getColor(); }
	void setColor (cPlayerColor color) { return splayer.setColor (std::move (color)); }

	int getId() const { return splayer.getNr(); }

	int getCredits() const;
	void setCredits (int credits);

	/** Get the most modern version of a unit (including all his upgrades). */
	cDynamicUnitData* getUnitDataCurrentVersion (const sID& id);
	const cDynamicUnitData* getUnitDataCurrentVersion (const sID& id) const;

	void setLandingPos (const cPosition& position) { landingPos = position; }
	const cPosition& getLandingPos() const { return landingPos; };

	void initMaps (cMap& map);
	const cPosition& getMapSize() const;
	void doScan();
	void revealMap();
	void revealPosition (const cPosition& position);
	void revealResource();
	unsigned int getOffset (const cPosition& pos) const { return pos.x() + pos.y() * mapSize.x(); }
	bool canSeeAnyAreaUnder (const cUnit& unit) const;
	bool canSeeAt (const cPosition& position) const;

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

	void addSentry (cUnit& u);
	void deleteSentry (cUnit& u);
	void upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, const cUnitsData& originalUnitsData);
	void countEcoSpheres();
	int getScore (int turn) const;
	int getScore() const;
	void setScore (int score, int turn);

	void setClan (int newClan, const cUnitsData& unitsData);
	int getClan() const { return clan; }

	bool getHasFinishedTurn() const;
	void setHasFinishedTurn (bool value);

	bool getIsRemovedFromGame() const;
	void setIsRemovedFromGame (bool value);

	void exploreResource (const cPosition& pos) { ResourceMap.set(getOffset (pos), 1); }
	bool hasResourceExplored (const cPosition& pos) const { return ResourceMap[getOffset (pos)] != 0; }
	bool hasSentriesAir (const cPosition& pos) const { return SentriesMapAir[getOffset (pos)] != 0; }
	bool hasSentriesGround (const cPosition& pos) const { return SentriesMapGround[getOffset (pos)] != 0; }
	bool hasLandDetection (const cPosition& pos) const { return DetectLandMap[getOffset (pos)] != 0; }
	bool hasMineDetection (const cPosition& pos) const { return DetectMinesMap[getOffset (pos)] != 0; }
	bool hasSeaDetection (const cPosition& pos) const { return DetectSeaMap[getOffset (pos)] != 0; }

	void doResearch(const cUnitsData& unitsData);  // proceed with the research at turn end
	void accumulateScore (cServer& server); // at turn end

	void refreshSentryAir();
	void refreshSentryGround();

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

	void makeTurnEnd();

	uint32_t getChecksum(uint32_t crc) const;

	mutable cSignal<void ()> nameChanged;
	mutable cSignal<void ()> colorChanged;
	mutable cSignal<void ()> creditsChanged;
	mutable cSignal<void ()> hasFinishedTurnChanged;
	mutable cSignal<void ()> isRemovedFromGameChanged;
	mutable cSignal<void (cResearch::ResearchArea)> researchCentersWorkingOnAreaChanged;
	mutable cSignal<void ()> researchCentersWorkingTotalChanged;
	mutable cSignal<void ()> turnEndMovementsStarted;

	template <typename T>
	void save(T& archive)
	{
		archive & NVP(splayer);
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
		archive & NVP(isRemovedFromGame);
		archive & NVP(researchState);
	}
	template<typename T>
	void load(T& archive)
	{
		archive & NVP(splayer);

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
		archive & NVP(isRemovedFromGame);
		archive & NVP(researchState);

		doScan();
		refreshSentryAir();
		refreshSentryGround();
		refreshResearchCentersWorkingOnArea();
		countEcoSpheres();
	}
	SERIALIZATION_SPLIT_MEMBER();
private:
	/**
	* draws a circle on the map for the fog
	* @author alzi alias DoctorDeath
	* @param iX X coordinate to the center of the circle
	* @param iY Y coordinate to the center of the circle
	* @param iRadius radius of the circle
	* @param map map were to store the data of the circle
	*/
	void drawSpecialCircle (const cPosition& position, int iRadius, cArrayCrc<uint8_t>& map, const cPosition& mapsize);
	/**
	* draws a big circle on the map for the fog
	* @author alzi alias DoctorDeath
	* @param iX X coordinate to the center of the circle
	* @param iY Y coordinate to the center of the circle
	* @param iRadius radius of the circle
	* @param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig (const cPosition& position, int iRadius, cArrayCrc<uint8_t>& map, const cPosition& mapsize);

	std::string resourceMapToString() const;
	void setResourceMapFromString(const std::string& str);
private:
	cPlayerBasicData splayer;
public:
	std::vector<cDynamicUnitData> dynamicUnitsData; // Current version of vehicles.
	cBase base;               // the base (groups of connected buildings) of the player
private:
	cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>> vehicles;
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> buildings;

	cPosition landingPos;
	cPosition mapSize; // Width and Height of the map.

	// using a special array with cached checksum. This speeds up the calculation of the model checksum.
	cArrayCrc<uint8_t> ScanMap;            // seen Map tiles.
	cArrayCrc<uint8_t> ResourceMap;        // Map with explored resources.
	cArrayCrc<uint8_t> SentriesMapAir;     /**< the covered air area */
	cArrayCrc<uint8_t> SentriesMapGround;  /**< the covered ground area */
	cArrayCrc<uint8_t> DetectLandMap;      // Map mit den Gebieten, die an Land gesehen werden kˆnnen.
	cArrayCrc<uint8_t> DetectSeaMap;       // Map mit den Gebieten, die im Wasser gesehen werden kˆnnen.
	cArrayCrc<uint8_t> DetectMinesMap;     /** the area where the player can detect mines */
public:
	PointsHistory pointsHistory; // history of player's total score (from eco-spheres) for graph
	bool isDefeated;        // true if the player has been defeated
	int numEcos;            // number of ecospheres. call countEcoSpheres to update.
private:
	int clan;

	int credits;

	std::vector<sTurnstartReport> currentTurnUnitReports; //TODO: remove. Shouldn't be part of the game model
	std::vector<int> currentTurnResearchAreasFinished;

	bool hasFinishedTurn;
	bool isRemovedFromGame;

	cResearch researchState;   ///< stores the current research level of the player
	int researchCentersWorkingOnArea[cResearch::kNrResearchAreas]; ///< counts the number of research centers that are currently working on each area
	int researchCentersWorkingTotal;  ///< number of working research centers
};

#endif // game_data_player_playerH
