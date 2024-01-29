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
#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/serialization.h"

#include <cassert>
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

struct sNewTurnReport
{
	std::map<int, sNewTurnPlayerReport> reports;
};

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

	void setUnitsData (std::shared_ptr<cUnitsData>);
	std::shared_ptr<const cUnitsData> getUnitsData() const { return unitsData; }

	std::shared_ptr<const cGameSettings> getGameSettings() const { return gameSettings; }
	void setGameSettings (const cGameSettings&);

	std::shared_ptr<const cMap> getMap() const { return map; }
	const std::shared_ptr<cMap>& getMap() { return map; }
	void setMap (std::shared_ptr<cStaticMap>);

	const std::shared_ptr<cCasualtiesTracker>& getCasualtiesTracker() { return casualtiesTracker; }
	std::shared_ptr<const cCasualtiesTracker> getCasualtiesTracker() const { return casualtiesTracker; }

	cPlayer* getPlayer (int playerNr);
	const cPlayer* getPlayer (int playerNr) const;
	const cPlayer* getPlayer (std::string player) const;
	const std::vector<std::shared_ptr<cPlayer>>& getPlayerList() const { return /*static_cast<std::vector<std::shared_ptr<const cPlayer>>>*/ (playerList); } //TODO: cast to const cPlayer
	std::vector<std::shared_ptr<cPlayer>>& getPlayerList() { return playerList; }
	void setPlayerList (const std::vector<cPlayerBasicData>&);
	const cPlayer* getActiveTurnPlayer() const;

	std::shared_ptr<const cTurnCounter> getTurnCounter() const;
	std::shared_ptr<const cTurnTimeClock> getTurnTimeClock() const;

	cUnit* getUnitFromID (unsigned int id) const;
	cVehicle* getVehicleFromID (unsigned int id) const;
	cBuilding* getBuildingFromID (unsigned int id) const;

	cBuilding& addBuilding (const cPosition&, const sID&, cPlayer*);
	cVehicle& addVehicle (const cPosition&, const sID&, cPlayer*);
	void destroyUnit (cUnit&);

	void addRubble (const cPosition&, int value, bool big);
	void deleteUnit (cUnit*);
	void deleteRubble (cBuilding& rubble);

	std::shared_ptr<cBuilding> extractNeutralUnit (const cBuilding& building) { return neutralBuildings.extract (building); }
	std::shared_ptr<cVehicle> extractNeutralUnit (const cVehicle& vehicle) { return neutralVehicles.extract (vehicle); }

	cMoveJob* addMoveJob (cVehicle&, const std::forward_list<cPosition>& path);
	cMoveJob* addMoveJob (cVehicle&, const cPosition& destination);
	std::vector<const cPlayer*> resumeMoveJobs (const cPlayer* = nullptr);

	void addAttackJob (cUnit& aggressor, const cPosition& targetPosition);

	void handlePlayerStartTurn (cPlayer&);
	void handlePlayerFinishedTurn (cPlayer&);

	void addFx (std::shared_ptr<cFx>);
	void addJob (std::unique_ptr<cJob>);

	/**
	* Try to move an undetected enemy stealth unit out of the way, if necessary to free position.
	*/
	void sideStepStealthUnit (const cPosition&, const cVehicle&, const cPosition& bigOffset = cPosition (-1, -1));
	void sideStepStealthUnit (const cPosition&, const cStaticUnitData& vehicleData, cPlayer* vehicleOwner, const cPosition& bigOffset = cPosition (-1, -1));

	mutable cSignal<void()> gameTimeChanged;
	mutable cSignal<void (const cVehicle&)> triggeredAddTracks;
	mutable cSignal<void (const cPlayer&)> playerFinishedTurn; // triggered when a player wants to end the turn
	mutable cSignal<void()> turnEnded; // triggered when all players ended the turn or the turn time clock reached a deadline
	mutable cSignal<void (const sNewTurnReport&)> newTurnStarted; // triggered when the model has done all calculations for the new turn.
	mutable cSignal<void (const std::shared_ptr<cFx>&)> addedEffect;

	mutable cSignal<void (const cUnit& storingUnit, const cUnit& storedUnit)> unitStored;
	mutable cSignal<void (const cUnit& storingUnit, const cUnit& storedUnit)> unitActivated;

	mutable cSignal<void (const cUnit&)> unitSuppliedWithAmmo;
	mutable cSignal<void (const cUnit&)> unitRepaired;

	mutable cSignal<void (const cUnit& source, const cUnit& target)> unitStealDisableFailed;
	mutable cSignal<void (const cUnit& source, const cUnit& target)> unitDisabled;
	mutable cSignal<void (const cUnit& source, const cUnit& target, const cPlayer* previousOwner)> unitStolen;

	mutable cSignal<void (const cVehicle& plane)> planeLanding;
	mutable cSignal<void (const cVehicle& plane)> planeTakeoff;

	mutable cSignal<void (const cPlayer&)> playerHasLost;
	mutable cSignal<void (const cPlayer&)> playerHasWon;
	mutable cSignal<void()> suddenDeathMode;

	template <typename Archive>
	void save (Archive& archive) const
	{
		archive << NVP (gameId);
		archive << NVP (gameTime);
		archive << NVP (randomGenerator);
		archive << serialization::makeNvp ("gameSettings", *gameSettings);
		archive << serialization::makeNvp ("map", *map);
		archive << serialization::makeNvp ("unitsData", *unitsData);
		archive << serialization::makeNvp ("players", playerList);
		archive << NVP (moveJobs);
		archive << NVP (attackJobs);
		archive << NVP (neutralBuildings);
		archive << NVP (neutralVehicles);

		archive << NVP (nextUnitId);
		archive << serialization::makeNvp ("turnCounter", *turnCounter);
		archive << serialization::makeNvp ("turnTimeClock", *turnTimeClock);
		archive << NVP (turnEndDeadline);
		archive << NVP (turnLimitDeadline);
		archive << NVP (turnEndState);
		const auto activeTurnPlayerId = activeTurnPlayer->getId();
		archive << NVP (activeTurnPlayerId);
		archive << NVP (helperJobs);
		archive << serialization::makeNvp ("casualtiesTracker", *casualtiesTracker);
		//TODO: serialize effectList
	}
	template <typename Archive>
	void load (Archive& archive)
	{
		archive >> NVP (gameId);
		archive >> NVP (gameTime);

		archive >> NVP (randomGenerator);
		assert (gameSettings != nullptr);
		archive >> serialization::makeNvp ("gameSettings", *gameSettings);

		if (map == nullptr)
		{
			auto staticMap = std::make_shared<cStaticMap>();
			map = std::make_shared<cMap> (staticMap);
		}
		archive >> serialization::makeNvp ("map", *map);
		map->reset();

		if (unitsData == nullptr)
		{
			unitsData = std::make_shared<cUnitsData>();
		}
		archive >> serialization::makeNvp ("unitsData", *unitsData);
		//TODO: check UIData available

		// Don't load directly playerList as pointer might be stored (signal, gui, ...)
		std::vector<std::shared_ptr<cPlayer>> players;
		archive >> serialization::makeNvp ("players", players);
		playerList.resize (players.size());
		for (std::size_t i = 0; i != playerList.size(); ++i)
		{
			if (playerList[i] == nullptr)
			{
				playerList[i] = std::move (players[i]);
			}
			else
			{
				std::vector<unsigned char> buffer;
				cBinaryArchiveOut out(buffer);
				out << players[i];
				cBinaryArchiveIn in (buffer.data(), buffer.size());
				in >> *playerList[i];
			}
		}
		for (auto& player : playerList)
		{
			player->postLoad (*this);
		}

		archive >> NVP (moveJobs);
		for (auto& moveJob : moveJobs)
		{
			if (moveJob->getVehicleId())
			{
				auto* vehicle = getVehicleFromID (*moveJob->getVehicleId());
				assert (vehicle);
				vehicle->setMoveJob (moveJob.get());
			}
		}
		archive >> NVP (attackJobs);

		archive >> NVP (neutralBuildings);
		for (auto& building : neutralBuildings)
		{
			building->postLoad (*this);
		}
		archive >> NVP (neutralVehicles);
		for (auto& vehicle : neutralVehicles)
		{
			vehicle->postLoad (*this);
		}

		archive >> NVP (nextUnitId);
		archive >> serialization::makeNvp ("turnCounter", *turnCounter);
		archive >> serialization::makeNvp ("turnTimeClock", *turnTimeClock);
		archive >> NVP (turnEndDeadline);
		archive >> NVP (turnLimitDeadline);
		archive >> NVP (turnEndState);

		int activeTurnPlayerId;
		archive >> NVP (activeTurnPlayerId);
		activeTurnPlayer = getPlayer(activeTurnPlayerId);

		archive >> NVP (helperJobs);
		helperJobs.postLoad (*this);

		archive >> serialization::makeNvp ("casualtiesTracker", *casualtiesTracker);
		//TODO: clear effect list, deserialize effects, call addedEffect()

		refreshMapPointer();
		for (auto& player : playerList)
		{
			player->refreshBase (*map);
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
	unsigned int gameId = 0; //this id can be used to check, which log files, and save file belong to the same game.

	unsigned int gameTime = 0;

	std::shared_ptr<cGameSettings> gameSettings;
	std::shared_ptr<cMap> map;
	std::vector<std::shared_ptr<cPlayer>> playerList;
	cPlayer* activeTurnPlayer = nullptr;

	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> neutralBuildings;
	cFlatSet<std::shared_ptr<cVehicle>, sUnitLess<cVehicle>> neutralVehicles;

	int nextUnitId = 0;

	std::shared_ptr<cUnitsData> unitsData;

	std::vector<std::unique_ptr<cMoveJob>> moveJobs;
	std::vector<std::unique_ptr<cAttackJob>> attackJobs;

	std::shared_ptr<cTurnCounter> turnCounter;
	std::shared_ptr<cTurnTimeClock> turnTimeClock;
	unsigned int turnEndDeadline = 0;
	unsigned int turnLimitDeadline = 0;

	std::shared_ptr<cCasualtiesTracker> casualtiesTracker;

	enum class eTurnEndState
	{
		TurnActive,
		ExecuteRemainingMovements,
		ExecuteTurnStart
	};
	eTurnEndState turnEndState = eTurnEndState::TurnActive;

	/** lists with all FX-Animation */
	cFxContainer effectsList;

	/** little helper jobs, that do some time dependent actions */
	cJobContainer helperJobs;
};

#endif
