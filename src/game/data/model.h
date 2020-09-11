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

#ifndef game_data_modelH
#define game_data_modelH

#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/player/playerbasicdata.h"
#include "game/logic/attackjob.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/fxeffects.h"
#include "game/logic/jobs/jobcontainer.h"
#include "game/logic/movejob.h"
#include "game/logic/pathcalculator.h"
#include "game/logic/turncounter.h"
#include "units/unit.h"
#include "utility/crossplattformrandom.h"
#include "utility/flatset.h"
#include "utility/serialization/serialization.h"

#include <forward_list>
#include <memory>
#include <vector>

class cBuilding;
class cGameSettings;
class cGameTimer;
class cPosition;
class cStaticMap;
class cTurnTimeClock;
class cTurnTimeDeadline;
class cUnit;
class cVehicle;

struct sID;

class cModel
{
	friend class cDebugOutputWidget;
public:
	cModel();
	~cModel();

	cCrossPlattformRandom randomGenerator;

	unsigned int getGameId() const { return gameId; }
	void initGameId();

	void advanceGameTime();
	unsigned int getGameTime() const;

	uint32_t getChecksum() const;

	void setUnitsData(std::shared_ptr<cUnitsData> unitsData);
	std::shared_ptr<const cUnitsData> getUnitsData() const { return unitsData; };

	std::shared_ptr<const cGameSettings> getGameSettings() const { return gameSettings; };
	void setGameSettings(const cGameSettings& gameSettings);

	std::shared_ptr<const cMap> getMap() const { return map; };
	std::shared_ptr<cMap> getMap() { return map; };
	void setMap(std::shared_ptr<cStaticMap> map);

	const std::shared_ptr<cCasualtiesTracker>& getCasualtiesTracker() { return casualtiesTracker; }
	std::shared_ptr<const cCasualtiesTracker> getCasualtiesTracker() const { return casualtiesTracker; }

	cPlayer* getPlayer(int playerNr);
	const cPlayer* getPlayer(int playerNr) const;
	const cPlayer* getPlayer(std::string player) const;
	const std::vector<std::shared_ptr<cPlayer>>& getPlayerList() const { return /*static_cast<std::vector<std::shared_ptr<const cPlayer>>>*/(playerList); }; //TODO: cast to const cPlayer
	std::vector<std::shared_ptr<cPlayer>>& getPlayerList() { return playerList; };
	void setPlayerList(const std::vector<cPlayerBasicData>& splayers);
	const cPlayer* getActiveTurnPlayer() const;

	std::shared_ptr<const cTurnCounter> getTurnCounter() const;
	std::shared_ptr<const cTurnTimeClock> getTurnTimeClock() const;

	cUnit* getUnitFromID(unsigned int id) const;
	cVehicle* getVehicleFromID(unsigned int id) const;
	cBuilding* getBuildingFromID(unsigned int id) const;

	cVehicle& addVehicle(const cPosition& position, const sID& id, cPlayer* player);
	cBuilding& addBuilding(const cPosition& position, const sID& id, cPlayer* player);
	void destroyUnit(cUnit& unit);

	void addRubble(const cPosition& position, int value, bool big);
	void deleteUnit(cUnit* unit);
	void deleteRubble(cBuilding* rubble);

	cMoveJob* addMoveJob(cVehicle& vehicle, const std::forward_list<cPosition>& path);
	cMoveJob* addMoveJob(cVehicle& vehicle, const cPosition& destination);
	std::vector<const cPlayer*> resumeMoveJobs(const cPlayer* player = nullptr);

	void addAttackJob(cUnit& aggressor, const cPosition& targetPosition);

	void handlePlayerStartTurn(cPlayer& player);
	void handlePlayerFinishedTurn(cPlayer& player);

	void addFx(std::shared_ptr<cFx> fx);
	void addJob(cJob* job);

	/**
	* Try to move an undetected enemy stealth unit out of the way, if necessary to free position.
	*/
	void sideStepStealthUnit(const cPosition& position, const cVehicle& vehicle, const cPosition& bigOffset = cPosition(-1, -1));
	void sideStepStealthUnit(const cPosition& position, const cStaticUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset = cPosition(-1, -1));


	mutable cSignal<void()> gameTimeChanged;
	mutable cSignal<void(const cVehicle& vehicle)> triggeredAddTracks;
	mutable cSignal<void(const cPlayer& player)> playerFinishedTurn; // triggered when a player wants to end the turn
	mutable cSignal<void()> turnEnded; // triggered when all players ended the turn or the turn time clock reached a deadline
	mutable cSignal<void()> newTurnStarted; // triggered when the model has done all calculations for the new turn.
	mutable cSignal<void (const std::shared_ptr<cFx>& fx)> addedEffect;

	mutable cSignal<void (const cUnit& storingUnit, const cUnit& storedUnit)> unitStored;
	mutable cSignal<void (const cUnit& storingUnit, const cUnit& storedUnit)> unitActivated;

	mutable cSignal<void(const cUnit& unit)> unitSuppliedWithAmmo;
	mutable cSignal<void(const cUnit& unit)> unitRepaired;

	mutable cSignal<void(const cUnit& source, const cUnit& target)> unitStealDisableFailed;
	mutable cSignal<void(const cUnit& source, const cUnit& target)> unitDisabled;
	mutable cSignal<void(const cUnit& source, const cUnit& target, const cPlayer& previousOwner)> unitStolen;

	mutable cSignal<void(const cVehicle& plane)> planeLanding;
	mutable cSignal<void(const cVehicle& plane)> planeTakeoff;

	mutable cSignal<void(const cPlayer& player)> playerHasLost;
	mutable cSignal<void(const cPlayer& player)> playerHasWon;
	mutable cSignal<void()> suddenDeathMode;


	template<typename T>
	void save(T& archive)
	{
		archive << NVP(gameId);
		archive << NVP(gameTime);
		archive << serialization::makeNvp("gameSettings", *gameSettings);
		archive << serialization::makeNvp("map", *map);
		archive << serialization::makeNvp("unitsData", *unitsData);
		archive << serialization::makeNvp("numPlayers", (int)playerList.size());
		for (auto player : playerList)
		{
			archive << serialization::makeNvp("player", *player);
		}
		archive << serialization::makeNvp("numMoveJobs", (int)moveJobs.size());
		for (auto moveJob : moveJobs)
		{
			archive << serialization::makeNvp("moveJob", *moveJob);
		}
		archive << serialization::makeNvp("numAttackJobs", (int)attackJobs.size());
		for (auto attackJob : attackJobs)
		{
			archive << serialization::makeNvp("attackJob", *attackJob);
		}
		archive << serialization::makeNvp("neutralBuildingNum", (int)neutralBuildings.size());
		for (auto building : neutralBuildings)
		{
			archive << serialization::makeNvp("buildingID", building->getId());
			archive << serialization::makeNvp("building", *building);
		}
		archive << NVP(nextUnitId);
		archive << serialization::makeNvp("turnCounter", *turnCounter);
		archive << serialization::makeNvp("turnTimeClock", *turnTimeClock);
		archive << NVP(turnEndDeadline);
		archive << NVP(turnLimitDeadline);
		archive << NVP(turnEndState);
		archive << NVP(activeTurnPlayer);
		archive << NVP(helperJobs);
		archive << serialization::makeNvp("causaliesTracker", *casualtiesTracker);
		//TODO: serialize effectList
	}
	template<typename T>
	void load(T& archive)
	{
		archive >> NVP(gameId);
		archive >> NVP(gameTime);

		assert(gameSettings != nullptr);
		archive >> serialization::makeNvp("gameSettings", *gameSettings);

		if (map == nullptr)
		{
			auto staticMap = std::make_shared<cStaticMap>();
			map = std::make_shared<cMap>(staticMap);
		}
		archive >> serialization::makeNvp("map", *map);
		map->reset();

		if (unitsData == nullptr)
		{
			unitsData = std::make_shared<cUnitsData>();
		}
		archive >> serialization::makeNvp("unitsData", *unitsData);
		//TODO: check UIData available

		int numPlayers;
		archive >> NVP(numPlayers);
		playerList.resize(numPlayers);
		for (auto& player : playerList)
		{
			if (player == nullptr)
			{
				cPlayerBasicData basicPlayerData;
				player = std::make_shared<cPlayer>(basicPlayerData, *unitsData);
			}
			player->initMaps(*map);
			archive >> serialization::makeNvp("player", *player);
		}
		int numMoveJobs;
		archive >> NVP(numMoveJobs);
		for (auto moveJob : moveJobs)
		{
			delete moveJob;
		}
		moveJobs.clear();
		moveJobs.resize(numMoveJobs);
		for (auto& moveJob : moveJobs)
		{
			moveJob = new cMoveJob();
			archive >> serialization::makeNvp("moveJob", *moveJob);
		}
		int numAttackJobs;
		archive >> NVP(numAttackJobs);
		for (auto attackJob : attackJobs)
		{
			delete attackJob;
		}
		attackJobs.clear();
		attackJobs.resize(numAttackJobs);
		for (auto& attackJob : attackJobs)
		{
			attackJob = new cAttackJob();
			archive >> serialization::makeNvp("attackJob", *attackJob);
		}

		neutralBuildings.clear();
		int neutralBuildingNum;
		archive >> NVP(neutralBuildingNum);
		for (int i = 0; i < neutralBuildingNum; i++)
		{
			unsigned int buildingID;
			archive >> NVP(buildingID);
			auto building = std::make_shared<cBuilding>(nullptr, nullptr, nullptr, buildingID);
			archive >> serialization::makeNvp("building", *building);
			neutralBuildings.insert(std::move(building));
		}
		archive >> NVP(nextUnitId);
		archive >> serialization::makeNvp("turnCounter", *turnCounter);
		archive >> serialization::makeNvp("turnTimeClock", *turnTimeClock);
		archive >> NVP(turnEndDeadline);
		archive >> NVP(turnLimitDeadline);
		archive >> NVP(turnEndState);
		archive >> NVP(activeTurnPlayer);
		archive >> NVP(helperJobs);
		archive >> serialization::makeNvp("causaliesTracker", *casualtiesTracker);
		//TODO: clear effect list, deserialize effects, call addedEffect()

		refreshMapPointer();
		for (auto& player : playerList)
		{
			player->refreshBase(*map);
		}
	}
	SERIALIZATION_SPLIT_MEMBER()

private:
	void refreshMapPointer();
	void runMoveJobs();
	void runAttackJobs();
	void handleTurnEnd();

	bool isVictoryConditionMet() const;
	void defeatLoserPlayers();
	void checkDefeats();

	//------------------------------------------------------------------------------
	unsigned int gameId; //this id can be used to check, which log files, and save file belong to the same game.

	unsigned int gameTime;

	std::shared_ptr<cGameSettings> gameSettings;
	std::shared_ptr<cMap> map;
	std::vector<std::shared_ptr<cPlayer>> playerList;
	cPlayer* activeTurnPlayer;

	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> neutralBuildings;

	int nextUnitId;

	std::shared_ptr<cUnitsData> unitsData;

	std::vector<cMoveJob*> moveJobs;
	std::vector<cAttackJob*> attackJobs;

	std::shared_ptr<cTurnCounter> turnCounter;
	std::shared_ptr<cTurnTimeClock> turnTimeClock;
	unsigned int turnEndDeadline;
	unsigned int turnLimitDeadline;

	std::shared_ptr<cCasualtiesTracker> casualtiesTracker;

	enum {TURN_ACTIVE, EXECUTE_REMAINING_MOVEMENTS, EXECUTE_TURN_START} turnEndState;

	/** lists with all FX-Animation */
	cFxContainer effectsList;

	/** little helper jobs, that do some time dependent actions */
	cJobContainer helperJobs;
};

#endif
