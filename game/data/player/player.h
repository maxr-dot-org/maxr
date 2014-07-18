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
#include "autoptr.h"
#include "base.h"
#include "main.h" // for sID
#include "unit.h" // sUnitLess
#include "upgradecalculator.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/flatset.h"
#include "game/data/player/playerbasicdata.h"

class cBuilding;
class cHud;
class cMapField;
class cUnit;
class cVehicle;
class cPosition;

struct sHudStateContainer;
struct sTurnstartReport;
struct sUnitData;

class cSavedReport;

typedef std::vector<int> PointsHistory;

// the Player class //////////////////////////////
class cPlayer
{
	cPlayer (const cPlayer&) MAXR_DELETE_FUNCTION;
public:
	cPlayer (const cPlayerBasicData& splayer);
	~cPlayer();

	const std::string& getName() const { return splayer.getName(); }
	void setName (const std::string& name) { splayer.setName (name); }

	const cPlayerColor& getColor () const { return splayer.getColor (); }
	void setColor (cPlayerColor color) { return splayer.setColor (std::move(color)); }

	int getNr() const { return splayer.getNr(); }

	int getSocketNum() const { return splayer.getSocketIndex(); }
	void setSocketIndex (int index) { splayer.setSocketIndex (index); }

	void onSocketIndexDisconnected (unsigned int socketIndex) { splayer.onSocketIndexDisconnected (socketIndex); }

	void setLocal() { splayer.setLocal(); }
	bool isLocal() const { return splayer.isLocal(); }

	/** Get the most modern version of a unit (including all his upgrades). */
	sUnitData* getUnitDataCurrentVersion (const sID& id);
	const sUnitData* getUnitDataCurrentVersion (const sID& id) const;

	void setLandingPos(int x, int y) { landingPosX = x; landingPosY = y; }
	int getLandingPosX() const { return landingPosX; }
	int getLandingPosY() const { return landingPosY; }

	void initMaps (cMap& map);
	const cPosition& getMapSize () const;
	void doScan();
	void revealMap();
	void revealPosition (const cPosition& position);
	void revealResource();
	unsigned int getOffset (const cPosition& pos) const { return pos.x () + pos.y () * mapSize.x (); }
	bool canSeeAnyAreaUnder (const cUnit& unit) const;
	bool canSeeAt (const cPosition& position) const;

	cVehicle& addNewVehicle (const cPosition& position, const sID& id, unsigned int uid);
	cBuilding& addNewBuilding (const cPosition& position, const sID& id, unsigned int uid);

	void addUnit (std::shared_ptr<cVehicle> vehicle);
	void addUnit (std::shared_ptr<cBuilding> building);

	std::shared_ptr<cBuilding> removeUnit (const cBuilding& building);
	std::shared_ptr<cVehicle> removeUnit (const cVehicle& vehicle);

	void removeAllUnits ();

	cVehicle* getVehicleFromId (unsigned int id) const;
	cBuilding* getBuildingFromId (unsigned int id) const;

	const cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>>& getVehicles () const;
	const cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>>& getBuildings () const;

	cUnit* getNextUnit (cUnit* start) const;
	cUnit* getPrevUnit (cUnit* start) const;

	bool hasUnits () const;

	void addSentry (cUnit& u);
	void deleteSentry (cUnit& u);
	void startAResearch (int researchArea);
	void stopAResearch (int researchArea);
	void upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, std::vector<sUnitData*>& resultUpgradedUnitDatas);
	void refreshResearchCentersWorkingOnArea();
	void countEcoSpheres();
	int getScore (int turn) const;
	int getScore () const;
	void setScore (int score, int turn);
	void clearDone();

	void addSavedReport (std::unique_ptr<cSavedReport> savedReport);
	const std::vector<std::unique_ptr<cSavedReport>>& getSavedReports () const;

	void setClan (int newClan);
	int getClan() const { return clan; }

	bool getHasFinishedTurn () const;
	void setHasFinishedTurn (bool value);

	void exploreResource (const cPosition& pos) { ResourceMap[getOffset (pos)] = 1; }
	bool hasResourceExplored (const cPosition& pos) const { return ResourceMap[getOffset (pos)] != 0; }
	bool hasSentriesAir (const cPosition& pos) const { return SentriesMapAir[getOffset (pos)] != 0; }
	bool hasSentriesGround (const cPosition& pos) const { return SentriesMapGround[getOffset (pos)] != 0; }
	bool hasLandDetection (const cPosition& pos) const { return DetectLandMap[getOffset (pos)] != 0; }
	bool hasMineDetection (const cPosition& pos) const { return DetectMinesMap[getOffset (pos)] != 0; }
	bool hasSeaDetection (const cPosition& pos) const { return DetectSeaMap[getOffset (pos)] != 0; }

	void doResearch (cServer& server);  ///< proceed with the research at turn end
	void accumulateScore (cServer& server); // at turn end

	void refreshSentryAir();
	void refreshSentryGround();

	bool mayHaveOffensiveUnit() const;

	void addTurnReportUnit (const sID& unitId);
	void resetTurnReportData ();
	const std::vector<sTurnstartReport>& getCurrentTurnUnitReports () const;

	const std::vector<int>& getCurrentTurnResearchAreasFinished () const;

	mutable cSignal<void ()> nameChanged;
	mutable cSignal<void ()> colorChanged;
	mutable cSignal<void (const cSavedReport&)> reportAdded;
	mutable cSignal<void ()> hasFinishedTurnChanged;
private:
	/**
	* draws a circle on the map for the fog
	* @author alzi alias DoctorDeath
	* @param iX X coordinate to the center of the circle
	* @param iY Y coordinate to the center of the circle
	* @param iRadius radius of the circle
	* @param map map were to store the data of the circle
	*/
	void drawSpecialCircle (const cPosition& position, int iRadius, std::vector<char>& map, const cPosition& mapsize);
	/**
	* draws a big circle on the map for the fog
	* @author alzi alias DoctorDeath
	* @param iX X coordinate to the center of the circle
	* @param iY Y coordinate to the center of the circle
	* @param iRadius radius of the circle
	* @param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig (const cPosition& position, int iRadius, std::vector<char>& map, const cPosition& mapsize);

	cBuilding* getNextBuilding (cBuilding* start) const;
	cBuilding* getNextMiningStation (cBuilding* start) const;
	cVehicle* getNextVehicle (cVehicle* start) const;

	cBuilding* getPrevBuilding (cBuilding* start) const;
	cBuilding* getPrevMiningStation (cBuilding* start) const;
	cVehicle* getPrevVehicle (cVehicle* start) const;

private:
	cPlayerBasicData splayer;
public:
	std::vector<sUnitData> VehicleData; // Current version of vehicles.
	std::vector<sUnitData> BuildingData; // Current version of buildings.
	cBase base;               // Die Basis dieses Spielers.
private:
	cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>> vehicles;
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> buildings;

	int landingPosX;
	int landingPosY;
	cPosition mapSize; // Width and Height of the map.

	std::vector<char> ScanMap;            // seen Map tile.
	std::vector<char> ResourceMap;        // Map with explored resources.
	std::vector<char> SentriesMapAir;     /**< the covered air area */
	std::vector<char> SentriesMapGround;  /**< the covered ground area */
	std::vector<char> DetectLandMap;      // Map mit den Gebieten, die an Land gesehen werden kˆnnen.
	std::vector<char> DetectSeaMap;       // Map mit den Gebieten, die im Wasser gesehen werden kˆnnen.
	std::vector<char> DetectMinesMap;     /** the area where the player can detect mines */
public:
	cResearch researchLevel;   ///< stores the current research level of the player
	int researchCentersWorkingOnArea[cResearch::kNrResearchAreas]; ///< counts the number of research centers that are currently working on each area
	int workingResearchCenterCount;  ///< number of working research centers
	int Credits;               // Anzahl der erworbenen Credits.
	mutable PointsHistory pointsHistory; // history of player's total score (from eco-spheres) for graph
	AutoPtr<sHudStateContainer> savedHud;
	std::vector<std::unique_ptr<cSavedReport>> savedReportsList;
	bool isDefeated;        // true if the player has been defeated
	bool isRemovedFromGame; // true if the player has been removed from the game.
	int numEcos;            // number of ecospheres. call countEcoSpheres on server to update.
	bool researchFinished;
	unsigned int lastDeletedUnit;  /*!< used for detecting ownerchanges of a unit, e.g. a unit is readded with different player*/
private:
	int clan;

	std::vector<sTurnstartReport> currentTurnUnitReports;
	std::vector<int> currentTurnResearchAreasFinished;

	bool hasFinishedTurn;
};

#endif // game_data_player_playerH
