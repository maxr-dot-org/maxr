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
#ifndef playerH
#define playerH

#include <SDL.h>
#include <cassert>
#include <string>
#include <vector>

#include "defines.h"
#include "autoptr.h"
#include "base.h"
#include "main.h" // for sID
#include "upgradecalculator.h"

class cBuilding;
class cGameGUI;
class cHud;
class cMapField;
class cUnit;
class cVehicle;

struct sHudStateContainer;
struct sTurnstartReport;
struct sUnitData;

struct sSavedReportMessage
{
	enum eReportTypes
	{
		REPORT_TYPE_COMP,
		REPORT_TYPE_UNIT,
		REPORT_TYPE_CHAT
	};
public:
	std::string getFullMessage() const { if (xPos == -1) return message; else return "[" + iToStr (xPos) + "," + iToStr (yPos) + "] " + message; }

	void pushInto (cNetMessage& message) const;
	void popFrom (cNetMessage& message);
public:
	std::string message;
	eReportTypes type;
	sID unitID;
	int xPos, yPos;
	int colorNr;
};

typedef std::vector<int> PointsHistory;

/**
 * a structure that includes all information needed in pre-game.
 */
class sPlayer
{
public:
	sPlayer (const std::string& name_, unsigned int color_, int Nr_, int socketIndex_ = -1);

	const std::string& getName() const { return name; }
	void setName (const std::string& name_) { name = name_; }
	unsigned int getColorIndex() const { return colorIndex; }
	void setColorIndex (unsigned int index) { assert (index < PLAYERCOLORS); colorIndex = index; }
	void setToNextColorIndex();
	void setToPrevColorIndex();
	SDL_Surface* getColorSurface() const;
	int getNr() const { return Nr; }
	void setNr (int index) { Nr = index; }
	int getSocketIndex() const { return socketIndex; }
	void setSocketIndex (int index) { socketIndex = index; }
	void setLocal();
	bool isLocal() const;
	void onSocketIndexDisconnected (int socketIndex);
	void setReady (bool ready_) { ready = ready_; }
	bool isReady() const { return ready; }

private:
	std::string name;
	unsigned int colorIndex;
	int Nr; //!< Index in playerList

	// Index in socket array of cServer::network
	// if MAX_CLIENTS it's the local connected player
	// -1 for unknown
	int socketIndex;
	bool ready;
};


// the Player class //////////////////////////////
class cPlayer
{
	cPlayer (const cPlayer&) DELETE_CPP11;
public:
	cPlayer (const sPlayer& splayer);
	~cPlayer();

	const std::string& getName() const { return splayer.getName(); }
	void setName (const std::string& name) { splayer.setName (name); }
	unsigned int getColor() const { return splayer.getColorIndex(); }
	void setColor (unsigned int index) { return splayer.setColorIndex (index); }
	SDL_Surface* getColorSurface() const { return splayer.getColorSurface(); }
	int getNr() const { return splayer.getNr(); }
	int getSocketNum() const { return splayer.getSocketIndex(); }
	void setSocketIndex (int index) { splayer.setSocketIndex (index); }
	void setLocal() { splayer.setLocal(); }
	bool isLocal() const { return splayer.isLocal(); }
	void onSocketIndexDisconnected (unsigned int socketIndex) { splayer.onSocketIndexDisconnected (socketIndex);}

	/** Get the most modern version of a unit (including all his upgrades). */
	sUnitData* getUnitDataCurrentVersion (const sID& ID);

	void setLandingPos(int x, int y) { landingPosX = x; landingPosY = y; }
	int getLandingPosX() const { return landingPosX; }
	int getLandingPosY() const { return landingPosY; }

	void initMaps (cMap& map);
	void doScan();
	void revealMap();
	void revealResource();
	unsigned int getMapSize() const { return mapSize; }
	unsigned int getOffset (int x, int y) const { return x + y * mapSize; }
	bool canSeeAnyAreaUnder (const cUnit& unit) const;

	cVehicle* addVehicle (int posx, int posy, const sID& id, unsigned int uid);
	cBuilding* addBuilding (int posx, int posy, const sID& id, unsigned int uid);

	cUnit* getNextUnit (cUnit* start);
	cUnit* getPrevUnit (cUnit* start);
	void addSentry (cUnit& u);
	void deleteSentry (cUnit& u);
	void startAResearch (int researchArea);
	void stopAResearch (int researchArea);
	void upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, std::vector<sUnitData*>& resultUpgradedUnitDatas);
	void refreshResearchCentersWorkingOnArea();
	void deleteLock (cUnit& unit);
	void toggleLock (cMapField& OverUnitField);
	void countEcoSpheres();
	int getScore (int turn) const;
	void setScore (int score, int turn);
	void clearDone();

	const sSavedReportMessage& addSavedReport (const std::string& message, sSavedReportMessage::eReportTypes type, sID unitID = sID(), int xPos = -1, int yPos = -1, int colorNr = -1);

	void setClan (int newClan);
	int getClan() const { return clan; }

	void addUnitToList (cUnit& addedUnit);

	void exploreResource (int offset) { ResourceMap[offset] = 1; }
	bool hasResourceExplored (int offset) const { return ResourceMap[offset] != 0; }
	bool hasSentriesAir (int offset) const { return SentriesMapAir[offset] != 0; }
	bool hasSentriesGround (int offset) const { return SentriesMapGround[offset] != 0; }
	bool hasLandDetection (int offset) const { return DetectLandMap[offset] != 0; }
	bool hasMineDetection (int offset) const { return DetectMinesMap[offset] != 0; }
	bool hasSeaDetection (int offset) const { return DetectSeaMap[offset] != 0; }

	void doResearch (cServer& server);  ///< proceed with the research at turn end
	void accumulateScore (cServer& server); // at turn end

	void refreshSentryAir();
	void refreshSentryGround();

	bool mayHaveOffensiveUnit() const;
private:
	/**
	* draws a circle on the map for the fog
	* @author alzi alias DoctorDeath
	* @param iX X coordinate to the center of the circle
	* @param iY Y coordinate to the center of the circle
	* @param iRadius radius of the circle
	* @param map map were to store the data of the circle
	*/
	void drawSpecialCircle (int iX, int iY, int iRadius, std::vector<char>& map, int mapsize);
	/**
	* draws a big circle on the map for the fog
	* @author alzi alias DoctorDeath
	* @param iX X coordinate to the center of the circle
	* @param iY Y coordinate to the center of the circle
	* @param iRadius radius of the circle
	* @param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig (int iX, int iY, int iRadius, std::vector<char>& map, int mapsize);

	cBuilding* getNextBuilding (cBuilding* start);
	cBuilding* getNextMiningStation (cBuilding* start);
	cVehicle* getNextVehicle (cVehicle* start);

	cBuilding* getPrevBuilding (cBuilding* start);
	cBuilding* getPrevMiningStation (cBuilding* start);
	cVehicle* getPrevVehicle (cVehicle* start);

private:
	sPlayer splayer;
public:
	std::vector<sUnitData> VehicleData; // Current version of vehicles.
	cVehicle* VehicleList;     // List of all vehicles of the player.
	std::vector<sUnitData> BuildingData; // Current version of buildings.
	cBuilding* BuildingList;  // List of all building of the player.
	cBase base;               // Die Basis dieses Spielers.
private:
	int landingPosX;
	int landingPosY;
	int mapSize; // Width (and Height) of the map.
public:
	std::vector<char> ScanMap;            // seen Map tile.
private:
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
	std::vector<sTurnstartReport*> ReportVehicles; // Reportlisten.
	std::vector<sTurnstartReport*> ReportBuildings; // Reportlisten.
	std::vector<sSavedReportMessage> savedReportsList;
	std::vector<int> reportResearchAreasFinished; ///< stores, which research areas were just finished (for reporting at turn end)
	std::vector<cUnit*> LockList;  // List of locked units.
	bool bFinishedTurn;     // true when player send his turn end
	bool isDefeated;        // true if the player has been defeated
	bool isRemovedFromGame; // true if the player has been removed from the game.
	int numEcos;            // number of ecospheres. call countEcoSpheres on server to update.
	bool researchFinished;
	unsigned int lastDeletedUnit;  /*!< used for detecting ownerchanges of a unit, e.g. a unit is readded with different player*/
private:
	int clan;
};

#endif
