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
#include <vector>
#include "defines.h"
#include "base.h"
#include "main.h" // for sID
#include "upgradecalculator.h"

struct sVehicle;
struct sUnitData;
struct sBuilding;
class cVehicle;
class cBuilding;
class cGameGUI;
class cHud;
class cMapField;
class cUnit;
struct sHudStateContainer;
struct sTurnstartReport;

struct sSavedReportMessage
{
	enum eReportTypes
	{
		REPORT_TYPE_COMP,
		REPORT_TYPE_UNIT,
		REPORT_TYPE_CHAT
	};

	std::string message;
	eReportTypes type;
	sID unitID;
	int xPos, yPos;
	int colorNr;
};

typedef std::vector<int> PointsHistory;


// Die Player-Klasse /////////////////////////////////////////////////////////
class cPlayer
{
	friend class cServer;
public:
	cPlayer (const std::string& Name, SDL_Surface* Color, int nr, int iSocketNum = -1);
	cPlayer (const cPlayer& Player);
	~cPlayer();

	/** Get the most modern version of a unit (including all his upgrades). */
	sUnitData* getUnitDataCurrentVersion (const sID& ID);

	void initMaps (cMap& map);
	void doScan();
	void revealMap();
	void revealResource();

	cVehicle* addVehicle (int posx, int posy, const sVehicle& v, unsigned int ID);
	cBuilding* addBuilding (int posx, int posy, const sBuilding& b, unsigned int ID);

	cUnit* getNextUnit (cUnit* start);
	cUnit* getPrevUnit (cUnit* start);
	void addSentry (cUnit* u);
	void deleteSentry (cUnit* u);
	void startAResearch (int researchArea);
	void stopAResearch (int researchArea);
	void upgradeUnitTypes (const std::vector<int>& areasReachingNextLevel, std::vector<sUnitData*>& resultUpgradedUnitDatas);
	void refreshResearchCentersWorkingOnArea();
	void deleteLock (cUnit& unit);
	void toggelLock (cMapField* OverUnitField);
	void countEcoSpheres();
	int getScore (int turn) const;
	void setScore (int score, int turn);
	void clearDone();

	void addSavedReport (const std::string& message, sSavedReportMessage::eReportTypes type, sID unitID = sID(), int xPos = -1, int yPos = -1, int colorNr = -1);

	void setClan (int newClan);
	int getClan() const { return clan; }

	void addUnitToList (cUnit* addedUnit);

	void doResearch (cServer& server);  ///< proceed with the research at turn end
	void accumulateScore (cServer& server); // at turn end
private:
	/**
	* draws a circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircle (int iX, int iY, int iRadius, std::vector<char>& map, int mapsize);
	/**
	* draws a big circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig (int iX, int iY, int iRadius, std::vector<char>& map, int mapsize);

	cBuilding* getNextBuilding (cBuilding* start);
	cBuilding* getNextMiningStation (cBuilding* start);
	cVehicle* getNextVehicle (cVehicle* start);

	cBuilding* getPrevBuilding (cBuilding* start);
	cBuilding* getPrevMiningStation (cBuilding* start);
	cVehicle* getPrevVehicle (cVehicle* start);

	void refreshSentryAir();
	void refreshSentryGround();

public:
	std::string name;
	SDL_Surface* color;
	int Nr;

	std::vector<sUnitData> VehicleData; // Daten aller Vehicles f¸r diesen Player.
	cVehicle* VehicleList;     // Liste aller Vehicles des Spielers.
	std::vector<sUnitData> BuildingData; // Daten aller Buildings f¸r diesen Player.
	cBuilding* BuildingList;     // Liste aller Buildings des Spielers.
	cBase base;               // Die Basis dieses Spielers.

	int mapSize; // Width (and Height) of the map.
	std::vector<char> ScanMap;             // Map mit dem Scannerflags.
	std::vector<char> ResourceMap;         // Map mit aufgedeckten Resourcen. / Map with explored resources.
	//std::vector<sSentry*> SentriesAir;   /** list with all units on sentry that can attack planes */
	std::vector<char> SentriesMapAir;      /** the covered air area */
	//std::vector<sSentry*> SentriesGround; /** list with all units on sentry that can attack ground units */
	std::vector<char> SentriesMapGround;   /** the covered ground area */
	std::vector<char> DetectLandMap;       // Map mit den Gebieten, die an Land gesehen werden kˆnnen.
	std::vector<char> DetectSeaMap;        // Map mit den Gebieten, die im Wasser gesehen werden kˆnnen.
	std::vector<char> DetectMinesMap;      /** the area where the player can detect mines */
	cResearch researchLevel;   ///< stores the current research level of the player
	int researchCentersWorkingOnArea[cResearch::kNrResearchAreas]; ///< counts the number of research centers that are currently working on each area
	int ResearchCount;         ///< number of working research centers
	int Credits;               // Anzahl der erworbenen Credits.
	mutable PointsHistory pointsHistory; // history of player's total score (from eco-spheres) for graph
	sHudStateContainer* savedHud;
	std::vector<sTurnstartReport*> ReportVehicles; // Reportlisten.
	std::vector<sTurnstartReport*> ReportBuildings; // Reportlisten.
	std::vector<sSavedReportMessage> savedReportsList;
	std::vector<int> reportResearchAreasFinished; ///< stores, which research areas were just finished (for reporting at turn end)
	std::vector<cUnit*> LockList;  // Liste mit gelockten Objekten.
	int iSocketNum; // Number of socket over which this player is connected in network game
					// if MAX_CLIENTS its the local connected player; -1 for unknown
	bool bFinishedTurn;     //true when player send his turn end
	bool isDefeated;        // true if the player has been defeated
	bool isRemovedFromGame; // true if the player has been removed from the game.
	int numEcos;            // number of ecospheres. call countEcoSpheres on server to update.
	bool researchFinished;
	unsigned int lastDeletedUnit;  /**used for detecting ownerchanges of a unit, e.g. a unit is readded with different player*/
private:
	int clan;
};

#endif
