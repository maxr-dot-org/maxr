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

#include <vector>
#include <memory>

#include "utility/flatset.h"
#include "units/unit.h"
#include "utility/crossplattformrandom.h"
#include "utility/serialization/serialization.h"
#include "game/data/map/map.h"

class cPlayerBasicData;
class cGameSettings;
class cPlayer;
class cStaticMap;
class cBuilding;
class cVehicle;
struct sID;
class cPosition;
class cUnit;
class cGameTimer;

class cModel
{
	friend class cDebugOutputWidget;
public:
	cModel();

	cCrossPlattformRandom randomGenerator;

	void runJobs(const cGameTimer& timer); //advanceGameTime
	uint32_t calcChecksum() const;


	std::shared_ptr<const cGameSettings> getGameSettings() const { return gameSettings; };
	std::shared_ptr<cGameSettings> getGameSettings() { return gameSettings; };
	void setGameSettings(const cGameSettings& gameSettings);

	std::shared_ptr<const cMap> getMap() const { return map; };
	std::shared_ptr<cMap> getMap() { return map; };
	void setMap(std::shared_ptr<cStaticMap> map);

	cPlayer* getPlayer(int playerNr);
	const cPlayer* getPlayer(int playerNr) const;
	const cPlayer* getPlayer(std::string player) const;
	const std::vector<std::shared_ptr<cPlayer>>& getPlayerList() const { return /*static_cast<std::vector<std::shared_ptr<const cPlayer>>>*/(playerList); }; //TODO: cast to const cPlayer
	std::vector<std::shared_ptr<cPlayer>>& getPlayerList() { return playerList; };
	void setPlayerList(const std::vector<cPlayerBasicData>& splayers);

	cUnit* getUnitFromID(unsigned int id) const;
	cVehicle* getVehicleFromID(unsigned int id) const;
	cBuilding* getBuildingFromID(unsigned int id) const;

	//TODO: check if init and addToMap are needed
	cVehicle& addVehicle(const cPosition& position, const sID& id, cPlayer* player, bool init = false, bool addToMap = true);
	cBuilding& addBuilding(const cPosition& position, const sID& id, cPlayer* player, bool init = false);
	void deleteUnit(cUnit* unit);
	void deleteRubble(cBuilding* rubble);

	template<typename T>
	void save(T& archive)
	{
		archive & serialization::makeNvp("gameSettings", *gameSettings);
		archive & serialization::makeNvp("map", *map);
		archive & serialization::makeNvp("numPlayers", (int)playerList.size());
		for (auto player : playerList)
		{
			archive & serialization::makeNvp("player", *player);
		}
		
		//archive & NVP(neutralBuildings);
		archive & NVP(nextUnitId);
	};
	template<typename T>
	void load(T& archive)
	{
		assert(gameSettings != nullptr);
		archive & serialization::makeNvp("gameSettings", *gameSettings);

		if (map == nullptr)
		{
			auto staticMap = std::make_shared<cStaticMap>();
			map = std::make_shared<cMap>(staticMap);
		}
		archive & serialization::makeNvp("map", *map);

		playerList.clear();
		int numPlayers;
		archive & NVP(numPlayers);
		for (int i = 0; i < numPlayers; i++)
		{
			cPlayerBasicData basicPlayerData;
			auto player = std::make_shared<cPlayer>(basicPlayerData);
			player->initMaps(*map);
			playerList.push_back(player);
			archive & serialization::makeNvp("player", *player);
		}
		refreshMapPointer();
		
		archive & NVP(nextUnitId);
	}
	SERIALIZATION_SPLIT_MEMBER();

private:
	void refreshMapPointer();



	std::shared_ptr<cGameSettings> gameSettings;
	std::shared_ptr<cMap> map;
	std::vector<std::shared_ptr<cPlayer>> playerList;

	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> neutralBuildings;

	int nextUnitId;
	//jobs
	//casualtiesTracker
	//turnnr
	//turn u. deadline timer
	//gametime?
	
	//effect list

	//signalConnectionManager?

	//random generator
};

#endif
