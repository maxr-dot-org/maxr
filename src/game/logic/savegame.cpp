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

#include "game/logic/savegame.h"
#include "maxrversion.h"
#include "game/data/units/building.h"
#include "game/logic/casualtiestracker.h"
#include "utility/files.h"
#include "loaddata.h"
#include "utility/log.h"
#include "game/logic/movejobs.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "settings.h"
#include "game/logic/upgradecalculator.h"
#include "game/data/units/vehicle.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/game/gameguistate.h"
#include "utility/tounderlyingtype.h"
#include "game/data/report/savedreport.h"
#include "game/logic/turnclock.h"
#include <ctime>

using namespace std;
using namespace tinyxml2;

//--------------------------------------------------------------------------
cSavegame::cSavegame (int number) :
	SaveFile()
{
	this->number = number;
	if (number > 100) return;
	TIXML_SNPRINTF (numberstr, sizeof (numberstr), "%.3d", number);
}

//--------------------------------------------------------------------------
int cSavegame::save (const cServer& server, const string& saveName)
{
	SaveFile.LinkEndChild (SaveFile.NewDeclaration());
	XMLElement* rootnode = SaveFile.NewElement ("MAXR_SAVE_FILE");
	rootnode->SetAttribute ("version", (SAVE_FORMAT_VERSION).c_str());
	SaveFile.LinkEndChild (rootnode);

	writeHeader (server, saveName);
	writeGameInfo (server);
	writeMap (*server.Map);
	writeCasualties (server);

	int unitnum = 0;
	const auto& playerList = server.playerList;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		const cPlayer& Player = *playerList[i];
		writePlayer (Player, i);

		const auto& vehicles = Player.getVehicles();
		for (auto i = vehicles.begin(); i != vehicles.end(); ++i)
		{
			const auto& vehicle = *i;
			if (!vehicle->isUnitLoaded())
			{
				writeUnit (server, *vehicle, &unitnum);
				unitnum++;
			}
		}

		const auto& buildings = Player.getBuildings();
		for (auto i = buildings.begin(); i != buildings.end(); ++i)
		{
			const auto& building = *i;
			writeUnit (server, *building, &unitnum);
			unitnum++;
		}
	}
	int rubblenum = 0;

	for (auto i = server.neutralBuildings.begin(); i != server.neutralBuildings.end(); ++i)
	{
		const auto& building = *i;
		writeRubble (server, *building, rubblenum);
		rubblenum++;
	}

	for (unsigned int i = 0; i != UnitsData.getNrVehicles() + UnitsData.getNrBuildings(); ++i)
	{
		const sUnitData* Data;
		if (i < UnitsData.getNrVehicles()) Data = &UnitsData.svehicles[i];
		else Data = &UnitsData.sbuildings[i - UnitsData.getNrVehicles()];
		writeStandardUnitValues (*Data, i);
	}

	if (!DirExists (cSettings::getInstance().getSavesPath()))
	{
		if (makeDir (cSettings::getInstance().getSavesPath())) Log.write ("Created new save directory: " + cSettings::getInstance().getSavesPath(), cLog::eLOG_TYPE_INFO);
		else Log.write ("Can't create save directory: " + cSettings::getInstance().getSavesPath(), cLog::eLOG_TYPE_ERROR);
	}
	SaveFile.SaveFile ((cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml").c_str());

	return 1;
}

//--------------------------------------------------------------------------
bool cSavegame::loadFile()
{
	if (!SaveFile.RootElement())
	{
		string fileName = cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml";
		if (SaveFile.LoadFile (fileName.c_str()) != 0)
		{
			return false;
		}
		if (!SaveFile.RootElement()) return false;

		loadedXMLFileName = fileName;
	}
	return true;
}

//--------------------------------------------------------------------------
bool cSavegame::load (cServer& server)
{
	if (!loadFile()) return false;

	if (!loadVersion()) return false;

	if (version != cVersion (SAVE_FORMAT_VERSION))
	{
		Log.write ("Save file version differs from the one supported by the game!", cLog::eLOG_TYPE_WARNING);
	}

	// load standard unit values
	if (version <= cVersion (0, 2))
	{
		Log.write ("Skipping loading standard unit values because save game has version 0.2 or older.", cLog::eLOG_TYPE_DEBUG);
	}
	else
	{
		XMLElement* unitValuesNode = SaveFile.RootElement()->FirstChildElement ("UnitValues");
		if (unitValuesNode != nullptr)
		{
			int unitnum = 0;
			XMLElement* unitNode = unitValuesNode->FirstChildElement ("UnitVal_0");
			while (unitNode)
			{
				loadStandardUnitValues (unitNode);

				unitnum++;
				unitNode = unitValuesNode->FirstChildElement (("UnitVal_" + iToStr (unitnum)).c_str());
			}
		}
	}

	string gametype;
	loadHeader (nullptr, &gametype, nullptr);
	if (gametype.compare ("IND") && gametype.compare ("HOT") && gametype.compare ("NET"))
	{
		Log.write ("Unknown gametype \"" + gametype + "\". Starting as singleplayergame.", cLog::eLOG_TYPE_INFO);
	}
	if (loadMap (server) == false)
	{
		return false;
	}
	loadPlayers (server);
	loadGameInfo (server);
	loadUnits (server);
	loadCasualties (server);

	recalcSubbases (server);
	for (size_t i = 0; i < server.playerList.size(); ++i)
	{
		auto& player = server.playerList[i];
		player->refreshResearchCentersWorkingOnArea();
	}
	return true;
}

//--------------------------------------------------------------------------
void cSavegame::recalcSubbases (cServer& server)
{
	const auto& playerList = server.playerList;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		playerList[i]->base.refreshSubbases();
	}

	//set the loaded ressource production values
	for (size_t i = 0; i != SubBasesLoad.size(); ++i)
	{
		cBuilding* building = server.getBuildingFromID (SubBasesLoad[i]->buildingID);
		if (!building) continue;

		sSubBase& sb = *building->SubBase;

		sb.setMetalProd (0);
		sb.setOilProd (0);
		sb.setGoldProd (0);
		sb.setMetalProd (SubBasesLoad[i]->metalProd);
		sb.setOilProd (SubBasesLoad[i]->oilProd);
		sb.setGoldProd (SubBasesLoad[i]->goldProd);
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadHeader (string* name, string* type, string* time)
{
	if (!loadFile()) return;
	if (!loadVersion()) return;

	if (version >= cVersion("1.0"))
	{
		if (name) *name = "Incompatible Savefile";
		return;
	}
	const XMLElement* headerNode = SaveFile.RootElement()->FirstChildElement ("Header");

	if (name) *name = headerNode->FirstChildElement ("Name")->Attribute ("string");
	if (type) *type = headerNode->FirstChildElement ("Type")->Attribute ("string");
	if (time) *time = headerNode->FirstChildElement ("Time")->Attribute ("string");
}

//--------------------------------------------------------------------------
string cSavegame::loadMapName()
{
	if (!loadFile()) return "";

	const XMLElement* mapNode = SaveFile.RootElement()->FirstChildElement ("Map");
	if (mapNode != nullptr) return mapNode->FirstChildElement ("Name")->Attribute ("string");
	else return "";
}

bool cSavegame::loadVersion()
{
	auto versionString = SaveFile.RootElement()->Attribute ("version");

	// TODO: reorganize the whole save game loading/saving to support exception and do not catch this one internally.
	try
	{
		version.parseFromString (versionString);
	}
	catch (std::runtime_error& e)
	{
		Log.write (e.what(), cLog::eLOG_TYPE_ERROR);
		return false;
	}
	return true;
}


//--------------------------------------------------------------------------
std::vector<cPlayerBasicData> cSavegame::loadPlayers()
{
	std::vector<cPlayerBasicData> playerNames;

	if (!loadFile()) return playerNames;

	if (!loadVersion()) return playerNames;

	const XMLElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");

	if (playersNode != nullptr)
	{
		const XMLElement* playerNode = playersNode->FirstChildElement ("Player_0");
		int playernum = 0;
		while (playerNode)
		{
			const auto name = playerNode->FirstChildElement ("Name")->Attribute ("string");
			const int number = playerNode->FirstChildElement ("Number")->IntAttribute ("num");

			cRgbColor playerColor;
			if (version < cVersion (0, 7))
			{
				const int colorIndex = playerNode->FirstChildElement ("Color")->IntAttribute ("num");
				playerColor = cPlayerColor::predefinedColors[colorIndex % cPlayerColor::predefinedColorsCount];
			}
			else
			{
				const auto colorElement = playerNode->FirstChildElement ("Color");
				playerColor = cRgbColor (colorElement->IntAttribute ("red"), colorElement->IntAttribute ("green"), colorElement->IntAttribute ("blue"));
			}

			playerNames.push_back (cPlayerBasicData (name, cPlayerColor (playerColor), number));
			playernum++;
			playerNode = playersNode->FirstChildElement (("Player_" + iToStr (playernum)).c_str());
		}
	}
	return playerNames;
}

//--------------------------------------------------------------------------
cGameSettings cSavegame::loadGameSettings()
{
	if (!loadFile()) return cGameSettings();

	XMLElement* gameInfoNode = SaveFile.RootElement()->FirstChildElement ("Game");
	if (!gameInfoNode) return cGameSettings();

	cGameSettings gameSetting;

	if (XMLElement* const element = gameInfoNode->FirstChildElement ("PlayTurns"))
	{
		gameSetting.setGameType (eGameSettingsGameType::Turns);
	}
	if (XMLElement* const element = gameInfoNode->FirstChildElement ("HotSeat"))
	{
		gameSetting.setGameType (eGameSettingsGameType::HotSeat);
	}

	if (version < cVersion (0, 4))
	{
#if 1 // old format
		int turnLimit = 0;
		int scoreLimit = 0;
		if (XMLElement* const e = gameInfoNode->FirstChildElement ("TurnLimit")) turnLimit = e->IntAttribute ("num");
		if (XMLElement* const e = gameInfoNode->FirstChildElement ("ScoreLimit")) scoreLimit = e->IntAttribute ("num");

		if (turnLimit != 0)
		{
			gameSetting.setVictoryCondition (eGameSettingsVictoryCondition::Turns);
			gameSetting.setVictoryTurns (turnLimit);
		}
		else if (scoreLimit != 0)
		{
			gameSetting.setVictoryCondition (eGameSettingsVictoryCondition::Points);
			gameSetting.setVictoryPoints (scoreLimit);
		}
		else
		{
			gameSetting.setVictoryCondition (eGameSettingsVictoryCondition::Death);
		}
#endif

		auto intToResourceAmount = [ ] (int value) -> eGameSettingsResourceAmount
		{
			switch (value)
			{
				case 0:
					return eGameSettingsResourceAmount::Limited;
					break;
				default:
				case 1:
					return eGameSettingsResourceAmount::Normal;
					break;
				case 2:
					return eGameSettingsResourceAmount::High;
					break;
				case 3:
					return eGameSettingsResourceAmount::TooMuch;
					break;
			}
		};

		if (XMLElement* e = gameInfoNode->FirstChildElement ("Metal")) gameSetting.setMetalAmount (intToResourceAmount (e->IntAttribute ("num")));
		if (XMLElement* e = gameInfoNode->FirstChildElement ("Oil")) gameSetting.setOilAmount (intToResourceAmount (e->IntAttribute ("num")));
		if (XMLElement* e = gameInfoNode->FirstChildElement ("Gold")) gameSetting.setGoldAmount (intToResourceAmount (e->IntAttribute ("num")));


		if (XMLElement* e = gameInfoNode->FirstChildElement ("ResFrequency"))
		{
			int value = e->IntAttribute ("num");
			switch (value)
			{
				case 0:
					gameSetting.setResourceDensity (eGameSettingsResourceDensity::Sparse);
					break;
				case 1:
					gameSetting.setResourceDensity (eGameSettingsResourceDensity::Normal);
					break;
				case 2:
					gameSetting.setResourceDensity (eGameSettingsResourceDensity::Dense);
					break;
				case 3:
					gameSetting.setResourceDensity (eGameSettingsResourceDensity::TooMuch);
					break;
			}
		}
		if (XMLElement* e = gameInfoNode->FirstChildElement ("Credits")) gameSetting.setStartCredits (e->IntAttribute ("num"));
		if (XMLElement* e = gameInfoNode->FirstChildElement ("BridgeHead")) gameSetting.setBridgeheadType (e->IntAttribute ("num") == 0 ? eGameSettingsBridgeheadType::Mobile : eGameSettingsBridgeheadType::Definite);
		if (XMLElement* e = gameInfoNode->FirstChildElement ("Clan")) gameSetting.setClansEnabled (e->IntAttribute ("num") == 0);

		if (XMLElement* e = gameInfoNode->FirstChildElement ("VictoryType"))
		{
			int value = e->IntAttribute ("num");
			switch (value)
			{
				case 0:
					gameSetting.setVictoryCondition (eGameSettingsVictoryCondition::Turns);
					if (XMLElement* e = gameInfoNode->FirstChildElement ("Duration")) gameSetting.setVictoryTurns (e->IntAttribute ("num"));
					break;
				case 1:
					gameSetting.setVictoryCondition (eGameSettingsVictoryCondition::Points);
					if (XMLElement* e = gameInfoNode->FirstChildElement ("Duration")) gameSetting.setVictoryPoints (e->IntAttribute ("num"));
					break;
				case 2:
					gameSetting.setVictoryCondition (eGameSettingsVictoryCondition::Death);
					break;
			}
		}

		if (XMLElement* e = gameInfoNode->FirstChildElement ("TurnDeadline"))
		{
			const auto value = e->IntAttribute ("num");
			if (value >= 0)
			{
				gameSetting.setTurnEndDeadline (std::chrono::seconds (value));
				gameSetting.setTurnEndDeadlineActive (true);
			}
			else
			{
				gameSetting.setTurnEndDeadlineActive (false);
			}
		}
	}
	else
	{
		try
		{
			if (XMLElement* e = gameInfoNode->FirstChildElement ("Metal")) gameSetting.setMetalAmount (gameSettingsResourceAmountFromString (e->Attribute ("string")));
			if (XMLElement* e = gameInfoNode->FirstChildElement ("Oil")) gameSetting.setMetalAmount (gameSettingsResourceAmountFromString (e->Attribute ("string")));
			if (XMLElement* e = gameInfoNode->FirstChildElement ("Gold")) gameSetting.setMetalAmount (gameSettingsResourceAmountFromString (e->Attribute ("string")));

			if (XMLElement* e = gameInfoNode->FirstChildElement ("ResourceDensity")) gameSetting.setResourceDensity (gameSettingsResourceDensityFromString (e->Attribute ("string")));

			if (XMLElement* e = gameInfoNode->FirstChildElement ("BridgeHead")) gameSetting.setBridgeheadType (gameSettingsBridgeheadTypeFromString (e->Attribute ("string")));

			if (XMLElement* e = gameInfoNode->FirstChildElement ("Credits")) gameSetting.setStartCredits (e->IntAttribute ("num"));

			if (XMLElement* e = gameInfoNode->FirstChildElement ("ClansEnabled")) gameSetting.setClansEnabled (e->BoolAttribute ("bool"));

			if (XMLElement* e = gameInfoNode->FirstChildElement ("VictoryCondition")) gameSetting.setVictoryCondition (gameSettingsVictoryConditionFromString (e->Attribute ("string")));
			if (gameSetting.getVictoryCondition() == eGameSettingsVictoryCondition::Turns)
			{
				if (XMLElement* e = gameInfoNode->FirstChildElement ("VictoryTurns")) gameSetting.setVictoryTurns (e->IntAttribute ("num"));
			}
			else if (gameSetting.getVictoryCondition() == eGameSettingsVictoryCondition::Points)
			{
				if (XMLElement* e = gameInfoNode->FirstChildElement ("VictoryPoints")) gameSetting.setVictoryPoints (e->IntAttribute ("num"));
			}

			if (XMLElement* e = gameInfoNode->FirstChildElement ("TurnEndDeadline")) gameSetting.setTurnEndDeadline (std::chrono::seconds (e->IntAttribute ("num")));
			if (XMLElement* e = gameInfoNode->FirstChildElement ("TurnEndDeadlineActive")) gameSetting.setTurnEndDeadlineActive (e->BoolAttribute ("bool"));

			if (XMLElement* e = gameInfoNode->FirstChildElement ("TurnLimit")) gameSetting.setTurnLimit (std::chrono::seconds (e->IntAttribute ("num")));
			if (XMLElement* e = gameInfoNode->FirstChildElement ("TurnLimitActive")) gameSetting.setTurnLimitActive (e->BoolAttribute ("bool"));
		}
		catch (std::runtime_error&)
		{
			// FIXME: handle exception!
		}
	}

	return gameSetting;
}

//--------------------------------------------------------------------------
void cSavegame::loadGameInfo (cServer& server)
{
	XMLElement* gameInfoNode = SaveFile.RootElement()->FirstChildElement ("Game");
	if (!gameInfoNode) return;

	server.turnClock->setTurn (gameInfoNode->FirstChildElement ("Turn")->IntAttribute ("num"));

	*server.gameSettings = loadGameSettings();

	if (server.gameSettings->getGameType() == eGameSettingsGameType::Turns)
	{
		XMLElement* const element = gameInfoNode->FirstChildElement ("PlayTurns");
		server.activeTurnPlayer = &server.getPlayerFromNumber (element->IntAttribute ("activeplayer"));
	}
	if (server.gameSettings->getGameType() == eGameSettingsGameType::HotSeat)
	{
		XMLElement* const element = gameInfoNode->FirstChildElement ("HotSeat");

		if (element)
		{
			server.activeTurnPlayer = &server.getPlayerFromNumber (element->IntAttribute ("activeplayer"));
		}
		else
		{
			server.activeTurnPlayer = server.playerList[0].get();
		}
	}
}

//--------------------------------------------------------------------------
bool cSavegame::loadMap (cServer& server)
{
	XMLElement* mapNode = SaveFile.RootElement()->FirstChildElement ("Map");
	if (mapNode == nullptr) return false;

	auto staticMap = std::make_shared<cStaticMap>();
	string name = mapNode->FirstChildElement ("Name")->Attribute ("string");
	string resourcestr = mapNode->FirstChildElement ("Resources")->Attribute ("data");
	if (!staticMap->loadMap (name))
	{
		return false;
	}
	server.Map = std::make_unique<cMap> (staticMap);
	server.Map->setResourcesFromString (resourcestr);
	return true;
}

//--------------------------------------------------------------------------
void cSavegame::loadPlayers (cServer& server)
{
	auto& players = server.playerList;

	XMLElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	if (playersNode == nullptr) return;

	int playernum = 0;
	cMap& map = *server.Map;
	XMLElement* playerNode = playersNode->FirstChildElement ("Player_0");
	while (playerNode)
	{
		cGameGuiState gameGuiState;
		auto player = loadPlayer (playerNode, map, gameGuiState);
		server.playerGameGuiStates[player->getNr()] = gameGuiState;
		players.push_back (std::move (player));
		playernum++;
		playerNode = playersNode->FirstChildElement (("Player_" + iToStr (playernum)).c_str());
	}
}

//--------------------------------------------------------------------------
std::unique_ptr<cPlayer> cSavegame::loadPlayer (XMLElement* playerNode, cMap& map, cGameGuiState& gameGuiState)
{
	const string name = playerNode->FirstChildElement ("Name")->Attribute ("string");
	const int number = playerNode->FirstChildElement ("Number")->IntAttribute ("num");

	cRgbColor playerColor;
	if (version < cVersion (0, 7))
	{
		const int colorIndex = playerNode->FirstChildElement ("Color")->IntAttribute ("num");
		playerColor = cPlayerColor::predefinedColors[colorIndex % cPlayerColor::predefinedColorsCount];
	}
	else
	{
		const auto colorElement = playerNode->FirstChildElement ("Color");
		playerColor = cRgbColor (colorElement->IntAttribute ("red"), colorElement->IntAttribute ("green"), colorElement->IntAttribute ("blue"));
	}

	auto Player = std::make_unique<cPlayer> (cPlayerBasicData (name, cPlayerColor (playerColor), number));
	Player->initMaps (map);

	const XMLElement* landingPosNode = playerNode->FirstChildElement ("LandingPos");
	if (landingPosNode)
		Player->setLandingPos (landingPosNode->IntAttribute ("x"), landingPosNode->IntAttribute ("y"));

	Player->setCredits (playerNode->FirstChildElement ("Credits")->IntAttribute ("num"));

	if (XMLElement* const e = playerNode->FirstChildElement ("ScoreHistory"))
	{
		int num = 0;
		int i = 0;
		for (XMLElement* s = e->FirstChildElement ("Score"); s; s = s->NextSiblingElement ("Score"))
		{
			num = s->IntAttribute ("num");
			Player->pointsHistory.resize (i + 1);
			Player->pointsHistory[i] = num;
			i++;
		}
		// add current turn
		Player->pointsHistory.push_back (num);
	}

	int clan = -1;
	if (XMLElement* const element = playerNode->FirstChildElement ("Clan")) clan = element->IntAttribute ("num");
	Player->setClan (clan);

	string resourceMap = playerNode->FirstChildElement ("ResourceMap")->Attribute ("data");
	convertStringToScanMap (resourceMap, *Player);

	const XMLElement* hudNode = playerNode->FirstChildElement ("Hud");
	if (hudNode)
	{
		gameGuiState.popFrom (*hudNode, version);
	}

	// read reports
	if (version < cVersion (0, 5))
	{
		Log.write ("Skipping reports from save game because save game version is incompatible", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		const XMLElement* reportsNode = playerNode->FirstChildElement ("Reports");
		if (reportsNode)
		{
			const XMLElement* reportElement = reportsNode->FirstChildElement ("Report");
			while (reportElement)
			{
				if (reportElement->Parent() != reportsNode) break;
				auto report = cSavedReport::createFrom (*reportElement);
				if (report) Player->savedReportsList.push_back (std::move (report));
				reportElement = reportElement->NextSiblingElement();
			}
		}
	}

	XMLElement* upgradesNode = playerNode->FirstChildElement ("Upgrades");
	if (upgradesNode)
	{
		int upgradenum = 0;
		XMLElement* upgradeNode = upgradesNode->FirstChildElement ("Unit_0");
		while (upgradeNode)
		{
			sID ID;
			ID.generate (upgradeNode->FirstChildElement ("Type")->Attribute ("string"));
			if (ID.isAVehicle())
			{
				unsigned int num = UnitsData.getVehicleIndexBy (ID);
				loadUpgrade (upgradeNode, &Player->VehicleData[num]);
			}
			else
			{
				unsigned int num = UnitsData.getBuildingIndexBy (ID);
				loadUpgrade (upgradeNode, &Player->BuildingData[num]);
			}
			upgradenum++;
			upgradeNode = upgradesNode->FirstChildElement (("Unit_" + iToStr (upgradenum)).c_str());
		}
	}

	XMLElement* researchNode = playerNode->FirstChildElement ("Research");
	if (researchNode)
	{
		XMLElement* researchLevelNode = researchNode->FirstChildElement ("ResearchLevel");
		if (researchLevelNode)
			loadResearchLevel (researchLevelNode, Player->getResearchState());

		XMLElement* finishedResearchAreasNode = researchNode->FirstChildElement ("FinishedResearchAreas");
		if (finishedResearchAreasNode)
		{
			std::vector<int> areas;
			const XMLElement* areaNode = finishedResearchAreasNode->FirstChildElement ("Area");
			while (areaNode)
			{
				areas.push_back (areaNode->IntAttribute ("num"));
				areaNode = areaNode->NextSiblingElement ("Area");
			}
			Player->setCurrentTurnResearchAreasFinished (std::move (areas));
		}
	}

	if (XMLElement* const subbasesNode = playerNode->FirstChildElement ("Subbases"))
	{
		int subbasenum = 0;
		const XMLElement* subbaseNode = subbasesNode->FirstChildElement ("Subbase_0");
		while (subbaseNode)
		{
			const XMLElement* buildingIDNode = subbaseNode->FirstChildElement ("buildingID");
			if (buildingIDNode)
			{
				sSubBaseLoad* subBaseLoad = new sSubBaseLoad;

				subBaseLoad->buildingID = buildingIDNode->IntAttribute ("num");
				subBaseLoad->metalProd = subbaseNode->FirstChildElement ("Production")->IntAttribute ("metal");
				subBaseLoad->oilProd = subbaseNode->FirstChildElement ("Production")->IntAttribute ("oil");
				subBaseLoad->goldProd = subbaseNode->FirstChildElement ("Production")->IntAttribute ("gold");
				SubBasesLoad.push_back (subBaseLoad);
			}
			subbasenum++;
			subbaseNode = subbasesNode->FirstChildElement (("Subbase_" + iToStr (subbasenum)).c_str());
		}
	}
	return Player;
}

//--------------------------------------------------------------------------
void cSavegame::loadUpgrade (XMLElement* upgradeNode, sUnitData* data)
{
	data->setVersion (upgradeNode->FirstChildElement ("Version")->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("HitPoints")) data->setHitpointsMax (element->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Shots"))     data->setShotsMax (element->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Speed"))     data->setSpeedMax (element->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Armor"))     data->setArmor (element->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Costs"))     data->buildCosts = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Damage"))    data->setDamage (element->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Range"))     data->setRange (element->IntAttribute ("num"));
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Scan"))      data->setScan (element->IntAttribute ("num"));
}

//--------------------------------------------------------------------------
void cSavegame::loadResearchLevel (XMLElement* researchLevelNode, cResearch& researchLevel)
{
	const struct
	{
		const char* name;
		cResearch::ResearchArea area;
	} data[] =
	{
		{"attack", cResearch::kAttackResearch},
		{"shots", cResearch::kShotsResearch},
		{"range", cResearch::kRangeResearch},
		{"armor", cResearch::kArmorResearch},
		{"hitpoints", cResearch::kHitpointsResearch},
		{"speed", cResearch::kSpeedResearch},
		{"scan", cResearch::kScanResearch},
		{"cost", cResearch::kCostResearch}
	};

	const XMLElement* level = researchLevelNode->FirstChildElement ("Level");
	for (int i = 0; i != sizeof (data) / sizeof (*data); ++i)
	{
		const int value = level->IntAttribute (data[i].name);
		researchLevel.setCurResearchLevel (value, data[i].area);
	}
	const XMLElement* curPoint = researchLevelNode->FirstChildElement ("CurPoints");
	for (int i = 0; i != sizeof (data) / sizeof (*data); ++i)
	{
		const int value = curPoint->IntAttribute (data[i].name);
		researchLevel.setCurResearchPoints (value, data[i].area);
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadCasualties (cServer& server)
{
	if (server.getCasualtiesTracker() == 0)
		return;

	XMLElement* casualtiesNode = SaveFile.RootElement()->FirstChildElement ("Casualties");
	if (casualtiesNode == 0)
		return;

	server.getCasualtiesTracker()->initFromXML (casualtiesNode);
}

//--------------------------------------------------------------------------
void cSavegame::loadUnits (cServer& server)
{
	XMLElement* unitsNode = SaveFile.RootElement()->FirstChildElement ("Units");
	if (unitsNode == nullptr) return;

	int unitnum = 0;
	XMLElement* unitNode = unitsNode->FirstChildElement ("Unit_0");
	while (unitNode)
	{
		sID ID;
		ID.generate (unitNode->FirstChildElement ("Type")->Attribute ("string"));
		if (ID.isAVehicle()) loadVehicle (server, unitNode, ID);
		else if (ID.isABuilding()) loadBuilding (server, unitNode, ID);

		unitnum++;
		unitNode = unitsNode->FirstChildElement (("Unit_" + iToStr (unitnum)).c_str());
	}
	// read nextid-value before loading rubble,
	// so that the rubble will get new ids.
	const int nextID = unitsNode->FirstChildElement ("NextUnitID")->IntAttribute ("num");
	server.iNextUnitID = nextID;

	int rubblenum = 0;
	XMLElement* rubbleNode = unitsNode->FirstChildElement ("Rubble_0");
	while (rubbleNode)
	{
		loadRubble (server, rubbleNode);
		rubblenum++;
		rubbleNode = unitsNode->FirstChildElement (("Rubble_" + iToStr (rubblenum)).c_str());
	}
	generateMoveJobs (server);
}

//--------------------------------------------------------------------------
void cSavegame::loadVehicle (cServer& server, XMLElement* unitNode, const sID& ID)
{
	int tmpinteger;
	unitNode->FirstChildElement ("Owner")->QueryIntAttribute ("num", &tmpinteger);
	auto& owner = server.getPlayerFromNumber (tmpinteger);

	int x, y;
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("x", &x);
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("y", &y);
	unitNode->FirstChildElement ("ID")->QueryIntAttribute ("num", &tmpinteger);
	auto& vehicle = server.addVehicle (cPosition (x, y), ID, &owner, true, unitNode->FirstChildElement ("Stored_In") == nullptr, tmpinteger);

	if (unitNode->FirstChildElement ("Name")->Attribute ("notDefault") && strcmp (unitNode->FirstChildElement ("Name")->Attribute ("notDefault"), "1") == 0)
		vehicle.changeName (unitNode->FirstChildElement ("Name")->Attribute ("string"));

	loadUnitValues (unitNode, &vehicle.data);

	unitNode->FirstChildElement ("Direction")->QueryIntAttribute ("num", &vehicle.dir);
	if (XMLElement* const element = unitNode->FirstChildElement ("CommandoRank"))
	{
		vehicle.setCommandoRank (element->FloatAttribute ("num"));
	}
	if (unitNode->FirstChildElement ("IsBig")) server.Map->moveVehicleBig (vehicle, cPosition (x, y));
	if (unitNode->FirstChildElement ("Disabled")) vehicle.setDisabledTurns (unitNode->FirstChildElement ("Disabled")->IntAttribute ("turns"));
	if (unitNode->FirstChildElement ("LayMines")) vehicle.setLayMines (true);
	if (unitNode->FirstChildElement ("AutoMoving")) vehicle.hasAutoMoveJob = true;
	if (unitNode->FirstChildElement ("OnSentry"))
	{
		owner.addSentry (vehicle);
	}
	if (unitNode->FirstChildElement ("ManualFire")) vehicle.setManualFireActive (true);

	if (XMLElement* const element = unitNode->FirstChildElement ("Building"))
	{
		if (version < cVersion (0, 9))
		{
			vehicle.setBuildingABuilding (true);
		}
		else
		{
			vehicle.setBuildingABuilding (element->BoolAttribute ("building"));
		}
		if (element->Attribute ("type_id") != nullptr)
		{
			sID temp;
			temp.generate (element->Attribute ("type_id"));
			vehicle.setBuildingType (temp);
		}
		// be downward compatible and looke for 'type' too
		else if (element->Attribute ("type") != nullptr)
		{
			// element->Attribute ("type", &vehicle.BuildingTyp);
		}
		vehicle.setBuildTurns (element->IntAttribute ("turns"));
		vehicle.setBuildCosts (element->IntAttribute ("costs"));

		// use offset because of backward compatibility
		int buildBigSavedPosOffset;
		element->QueryIntAttribute ("savedpos", &buildBigSavedPosOffset);
		vehicle.buildBigSavedPosition.x() = buildBigSavedPosOffset % server.Map->getSize().x();
		vehicle.buildBigSavedPosition.y() = buildBigSavedPosOffset / server.Map->getSize().x();

		if (element->Attribute ("path"))
		{
			vehicle.BuildPath = true;
			vehicle.setBuildTurnsStart (element->IntAttribute ("turnsstart"));
			vehicle.setBuildCostsStart (element->IntAttribute ("costsstart"));
			element->QueryIntAttribute ("endx", &vehicle.bandPosition.x());
			element->QueryIntAttribute ("endy", &vehicle.bandPosition.y());

			if (!vehicle.isUnitBuildingABuilding() &&
				vehicle.data.getStoredResources() >= vehicle.getBuildCostsStart() && server.Map->possiblePlaceBuilding(*vehicle.getBuildingType().getUnitDataOriginalVersion(), vehicle.getPosition(), &vehicle))
			{
				server.addJob (new cStartBuildJob (vehicle, vehicle.getPosition(), vehicle.data.isBig));
				vehicle.setBuildingABuilding (true);
				vehicle.setBuildCosts (vehicle.getBuildCostsStart());
				vehicle.setBuildTurns (vehicle.getBuildTurnsStart());
			}
			else
			{
				vehicle.BuildPath = false;
			}
		}
	}
	if (XMLElement* const element = unitNode->FirstChildElement ("Clearing"))
	{
		vehicle.setClearing (true);
		vehicle.setClearingTurns (element->IntAttribute ("turns"));
		// use offset because of backward compatibility
		int buildBigSavedPosOffset;
		element->QueryIntAttribute ("savedpos", &buildBigSavedPosOffset);
		vehicle.buildBigSavedPosition.x() = buildBigSavedPosOffset % server.Map->getSize().x();
		vehicle.buildBigSavedPosition.y() = buildBigSavedPosOffset / server.Map->getSize().x();
	}

	if (XMLElement* const element = unitNode->FirstChildElement ("Movejob"))
	{
		sMoveJobLoad* MoveJob = new sMoveJobLoad;
		MoveJob->vehicle = &vehicle;
		element->QueryIntAttribute ("destx", &MoveJob->destX);
		element->QueryIntAttribute ("desty", &MoveJob->destY);

		MoveJobsLoad.push_back (MoveJob);
	}

	// read the players which have detected this unit
	if (XMLElement* const detectedNode = unitNode->FirstChildElement ("IsDetectedByPlayers"))
	{
		int playerNodeNum = 0;
		while (XMLElement* const element = detectedNode->FirstChildElement (("Player_" + iToStr (playerNodeNum)).c_str()))
		{
			int playerNum;
			element->QueryIntAttribute ("nr", &playerNum);
			int wasDetectedThisTurnAttrib = 1;
			// for old savegames, that don't have this attribute, set it to "detected this turn"
			if (element->QueryIntAttribute ("ThisTurn", &wasDetectedThisTurnAttrib) != XML_NO_ERROR)
				wasDetectedThisTurnAttrib = 1;
			bool wasDetectedThisTurn = (wasDetectedThisTurnAttrib != 0);
			try
			{
				cPlayer& Player = server.getPlayerFromNumber(playerNum);
				vehicle.setDetectedByPlayer(server, &Player, wasDetectedThisTurn);
			}
			catch (std::runtime_error &e)
			{
				Log.write(std::string("Could not set detectedByPlayer info. Reason: ") + e.what(), cLog::eLOG_TYPE_ERROR);
			}
			playerNodeNum++;
		}
	}

	// since we write all stored vehicles immediately after the storing unit
	// we can be sure that this one has been loaded yet
	if (XMLElement* const element = unitNode->FirstChildElement ("Stored_In"))
	{
		int storedInID;
		int isVehicle;
		element->QueryIntAttribute ("id", &storedInID);
		element->QueryIntAttribute ("is_vehicle", &isVehicle);
		if (isVehicle)
		{
			cVehicle* storingVehicle = server.getVehicleFromID (storedInID);
			if (!storingVehicle) return;

			storingVehicle->data.setStoredUnits (storingVehicle->data.getStoredUnits() - 1);
			storingVehicle->storeVehicle (vehicle, *server.Map);
		}
		else
		{
			cBuilding* storingBuilding = server.getBuildingFromID (storedInID);
			if (!storingBuilding) return;

			storingBuilding->data.setStoredUnits (storingBuilding->data.getStoredUnits() - 1);
			storingBuilding->storeVehicle (vehicle, *server.Map);
		}
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadBuilding (cServer& server, XMLElement* unitNode, const sID& ID)
{
	int tmpinteger;
	unitNode->FirstChildElement ("Owner")->QueryIntAttribute ("num", &tmpinteger);
	auto& owner = server.getPlayerFromNumber (tmpinteger);

	int x, y;
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("x", &x);
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("y", &y);
	unitNode->FirstChildElement ("ID")->QueryIntAttribute ("num", &tmpinteger);
	auto& building = server.addBuilding (cPosition (x, y), ID, &owner, true, tmpinteger);

	if (unitNode->FirstChildElement ("Name")->Attribute ("notDefault") && strcmp (unitNode->FirstChildElement ("Name")->Attribute ("notDefault"), "1") == 0)
		building.changeName (unitNode->FirstChildElement ("Name")->Attribute ("string"));

	loadUnitValues (unitNode, &building.data);

	if (unitNode->FirstChildElement ("IsWorking")) building.setWorking (true);
	if (unitNode->FirstChildElement ("wasWorking")) building.wasWorking = true;
	if (unitNode->FirstChildElement ("Disabled")) building.setDisabledTurns (unitNode->FirstChildElement ("Disabled")->IntAttribute ("turns"));
	if (unitNode->FirstChildElement ("ResearchArea")) building.setResearchArea ((cResearch::ResearchArea)unitNode->FirstChildElement ("ResearchArea")->IntAttribute ("area"));
	if (unitNode->FirstChildElement ("Score")) unitNode->FirstChildElement ("Score")->QueryIntAttribute ("num", & (building.points));
	if (unitNode->FirstChildElement ("OnSentry"))
	{
		if (!building.isSentryActive())
		{
			owner.addSentry (building);
		}
	}
	else if (building.isSentryActive())
	{
		owner.deleteSentry (building);
	}
	if (unitNode->FirstChildElement ("ManualFire")) building.setManualFireActive (true);
	if (unitNode->FirstChildElement ("HasBeenAttacked")) building.setHasBeenAttacked (true);

	if (XMLElement* const element = unitNode->FirstChildElement ("Building"))
	{
		XMLElement* buildNode = element;
		if (XMLElement* const element = buildNode->FirstChildElement ("BuildSpeed"))    building.setBuildSpeed (element->IntAttribute ("num"));
		if (XMLElement* const element = buildNode->FirstChildElement ("MetalPerRound")) building.setMetalPerRound (element->IntAttribute ("num"));
		if (buildNode->FirstChildElement ("RepeatBuild")) building.setRepeatBuild(true);

		int itemnum = 0;
		const XMLElement* itemElement = buildNode->FirstChildElement ("BuildList")->FirstChildElement ("Item_0");
		while (itemElement)
		{
			cBuildListItem listitem;
			if (itemElement->Attribute ("type_id") != nullptr)
			{
				sID id;
				id.generate (itemElement->Attribute ("type_id"));
				listitem.setType (id);
			}
			// be downward compatible and look for 'type' too
			else if (itemElement->Attribute ("type") != nullptr)
			{
				int typenr;
				itemElement->QueryIntAttribute ("type", &typenr);
				listitem.setType (UnitsData.svehicles[typenr].ID);
			}
			listitem.setRemainingMetal (itemElement->IntAttribute ("metall_remaining"));
			building.addBuildListItem (std::move (listitem));

			itemnum++;
			itemElement = buildNode->FirstChildElement ("BuildList")->FirstChildElement (("Item_" + iToStr (itemnum)).c_str());
		}
	}

	// read the players which have detected this unit
	if (XMLElement* const detectedNode = unitNode->FirstChildElement ("IsDetectedByPlayers"))
	{
		int playerNodeNum = 0;
		while (XMLElement* const element = detectedNode->FirstChildElement (("Player_" + iToStr (playerNodeNum)).c_str()))
		{
			int playerNum;
			element->QueryIntAttribute ("nr", &playerNum);
			auto& player = server.getPlayerFromNumber (playerNum);
			building.setDetectedByPlayer (server, &player);
			playerNodeNum++;
		}
	}

	if(building.isUnitWorking() && building.data.needsMetal && building.getBuildListSize() == 0)
	{
		Log.write("Found a building that can build vehicles and that is working but has no items in the build list. Disabling working state. Unit ID is " + iToStr(building.iID), cLog::eLOG_TYPE_WARNING);
		building.setWorking(false);
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadRubble (cServer& server, XMLElement* rubbleNode)
{
	int x, y, rubblevalue;
	bool big = false;

	rubbleNode->FirstChildElement ("Position")->QueryIntAttribute ("x", &x);
	rubbleNode->FirstChildElement ("Position")->QueryIntAttribute ("y", &y);
	rubbleNode->FirstChildElement ("RubbleValue")->QueryIntAttribute ("num", &rubblevalue);

	if (rubbleNode->FirstChildElement ("Big")) big = true;

	server.addRubble (cPosition (x, y), rubblevalue, big);
}

//--------------------------------------------------------------------------
void cSavegame::loadUnitValues (XMLElement* unitNode, sUnitData* Data)
{
	if (XMLElement* const Element = unitNode->FirstChildElement ("Version")) Data->setVersion (Element->IntAttribute ("num"));

	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Hitpoints")) Data->setHitpointsMax (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Ammo"))      Data->setAmmoMax (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Speed"))     Data->setSpeedMax (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Shots"))     Data->setShotsMax (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Armor"))         Data->setArmor (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Damage"))        Data->setDamage (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Range"))         Data->setRange (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("Scan"))          Data->setScan (Element->IntAttribute ("num"));

	if (XMLElement* const Element = unitNode->FirstChildElement ("Hitpoints")) Data->setHitpoints (Element->IntAttribute ("num"));
	else Data->setHitpoints (Data->getHitpointsMax());
	if (XMLElement* const Element = unitNode->FirstChildElement ("Ammo")) Data->setAmmo (Element->IntAttribute ("num"));
	else Data->setAmmo (Data->getAmmoMax());

	if (XMLElement* const Element = unitNode->FirstChildElement ("ResCargo"))  Data->setStoredResources (Element->IntAttribute ("num"));
	if (XMLElement* const Element = unitNode->FirstChildElement ("UnitCargo")) Data->setStoredUnits (Element->IntAttribute ("num"));
	// look for "Cargo" to be savegamecompatible
	if (XMLElement* const Element = unitNode->FirstChildElement ("Cargo"))
	{
		Data->setStoredResources (Element->IntAttribute ("num"));
		Data->setStoredUnits (Data->getStoredResources());
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Speed")) Data->setSpeed (Element->IntAttribute ("num"));
	else Data->setSpeed (Data->getSpeedMax());
	if (XMLElement* const Element = unitNode->FirstChildElement ("Shots")) Data->setShots (Element->IntAttribute ("num"));
	else Data->setShots (Data->getShotsMax());
}

void Split (const std::string& s, const char* seps, std::vector<std::string>& words)
{
	if (s.empty()) return;
	size_t beg = 0;
	size_t end = s.find_first_of (seps, beg);

	while (end != string::npos)
	{
		words.push_back (s.substr (beg, end - beg));
		beg = end + 1;
		end = s.find_first_of (seps, beg);
	}
	words.push_back (s.substr (beg));
}

//--------------------------------------------------------------------------
void cSavegame::loadStandardUnitValues (XMLElement* unitNode)
{
	if (unitNode == nullptr) return;
	sUnitData* Data = nullptr;

	// get the unit data
	sID ID;
	ID.generate (unitNode->FirstChildElement ("ID")->Attribute ("string"));
	if (ID.isAVehicle())
	{
		Data = &UnitsData.svehicles[UnitsData.getVehicleIndexBy (ID)];
	}
	else if (ID.isABuilding())
	{
		Data = &UnitsData.sbuildings[UnitsData.getBuildingIndexBy (ID)];
	}
	if (Data == nullptr) return;

	Data->ID = ID;

	Data->name = unitNode->FirstChildElement ("Name")->Attribute ("string");

	Data->setHitpointsMax (unitNode->FirstChildElement ("Hitpoints")->IntAttribute ("num"));
	Data->setArmor (unitNode->FirstChildElement ("Armor")->IntAttribute ("num"));
	unitNode->FirstChildElement ("Built_Costs")->QueryIntAttribute ("num", &Data->buildCosts);

	if (XMLElement* const Element = unitNode->FirstChildElement ("Scan"))     Data->setScan (Element->IntAttribute ("num"));      else Data->setScan (0);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Movement")) Data->setSpeedMax (Element->IntAttribute ("num") * 4); else Data->setSpeedMax (0);


	int tmpInt;

	if (XMLElement* const Element = unitNode->FirstChildElement ("MuzzleType"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->muzzleType = (sUnitData::eMuzzleType) tmpInt;
	}
	else
	{
		Data->muzzleType = sUnitData::MUZZLE_TYPE_NONE;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Shots"))  Data->setShotsMax (Element->IntAttribute ("num"));     else Data->setShotsMax (0);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Ammo"))   Data->setAmmoMax (Element->IntAttribute ("num"));     else Data->setAmmoMax (0);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Range"))  Data->setRange (Element->IntAttribute ("num"));     else Data->setRange (0);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Damage")) Data->setDamage (Element->IntAttribute ("num"));     else Data->setDamage (0);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Can_Attack"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->canAttack = tmpInt;
	}
	else
	{
		Data->canAttack = TERRAIN_NONE;
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Can_Build"))        Data->canBuild = Element->Attribute ("string");    else Data->canBuild = "";
	if (XMLElement* const Element = unitNode->FirstChildElement ("Build_As"))         Data->buildAs  = Element->Attribute ("string");    else Data->buildAs = "";
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Build_Factor")) Element->QueryIntAttribute ("num", &Data->maxBuildFactor); else Data->maxBuildFactor = 0;

	if (XMLElement* const Element = unitNode->FirstChildElement ("Storage_Res_Max"))   Element->QueryIntAttribute ("num", &Data->storageResMax);   else Data->storageResMax   = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Storage_Units_Max")) Element->QueryIntAttribute ("num", &Data->storageUnitsMax); else Data->storageUnitsMax = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Store_Res_Type"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->storeResType = (sUnitData::eStorageResType) tmpInt;
	}
	else
	{
		Data->storeResType = sUnitData::STORE_RES_NONE;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("StoreUnits_Image_Type"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->storeUnitsImageType = (sUnitData::eStorageUnitsImageType) tmpInt;
	}
	else
	{
		Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_NONE;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Is_Storage_Type")) Data->isStorageType = Element->Attribute ("string"); else Data->isStorageType = "";
	if (XMLElement* const Element = unitNode->FirstChildElement ("StoreUnitsTypes"))
	{
		string storeUnitsString = Element->Attribute ("string");
		Data->storeUnitsTypes.clear();
		Split (storeUnitsString, "+", Data->storeUnitsTypes);
		Data->storeUnitsTypes.erase (std::unique (Data->storeUnitsTypes.begin(), Data->storeUnitsTypes.end()), Data->storeUnitsTypes.end());
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Needs_Energy"))     Element->QueryIntAttribute ("num", &Data->needsEnergy);   else Data->needsEnergy   = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Needs_Humans"))     Element->QueryIntAttribute ("num", &Data->needsHumans);   else Data->needsHumans   = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Needs_Oil"))        Element->QueryIntAttribute ("num", &Data->needsOil);      else Data->needsOil      = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Needs_Metal"))      Element->QueryIntAttribute ("num", &Data->needsMetal);    else Data->needsMetal    = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Converts_Gold"))    Element->QueryIntAttribute ("num", &Data->convertsGold);  else Data->convertsGold  = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Can_Mine_Max_Res")) Element->QueryIntAttribute ("num", &Data->canMineMaxRes); else Data->canMineMaxRes = 0;
	if (Data->needsEnergy < 0)
	{
		Data->produceEnergy = abs (Data->needsEnergy);
		Data->needsEnergy = 0;
	}
	else Data->produceEnergy = 0;
	if (Data->needsHumans < 0)
	{
		Data->produceHumans = abs (Data->needsHumans);
		Data->needsHumans = 0;
	}
	else Data->produceHumans = 0;

	if (XMLElement* const Element = unitNode->FirstChildElement ("Is_Stealth_On"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->isStealthOn = tmpInt;
	}
	else
	{
		Data->isStealthOn = TERRAIN_NONE;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Can_Detect_Stealth_On"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->canDetectStealthOn = tmpInt;
	}
	else
	{
		Data->canDetectStealthOn = TERRAIN_NONE;
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Surface_Position"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->surfacePosition = (sUnitData::eSurfacePosition) tmpInt;
	}
	else
	{
		Data->surfacePosition = sUnitData::SURFACE_POS_GROUND;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Can_Be_Overbuild"))
	{
		Element->QueryIntAttribute ("num", &tmpInt);
		Data->canBeOverbuild = (sUnitData::eOverbuildType) tmpInt;
	}
	else
	{
		Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_NO;
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Factor_Air"))
	{
		Element->QueryFloatAttribute ("num", &Data->factorAir);
	}
	else
	{
		Data->factorAir = 0;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Factor_Coast"))
	{
		Element->QueryFloatAttribute ("num", &Data->factorCoast);
	}
	else
	{
		Data->factorCoast = 0;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Factor_Ground"))
	{
		Element->QueryFloatAttribute ("num", &Data->factorGround);
	}
	else
	{
		Data->factorGround = 0;
	}
	if (XMLElement* const Element = unitNode->FirstChildElement ("Factor_Sea"))
	{
		Element->QueryFloatAttribute ("num", &Data->factorSea);
	}
	else
	{
		Data->factorSea = 0;
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Factor_Sea"))
	{
		Element->QueryFloatAttribute ("num", &Data->modifiesSpeed);
	}
	else
	{
		Data->modifiesSpeed = 0;
	}

	Data->canBuildPath = unitNode->FirstChildElement ("Can_Build_Path") != nullptr;
	Data->canBuildRepeat = unitNode->FirstChildElement ("Can_Build_Repeat") != nullptr;
	Data->connectsToBase = unitNode->FirstChildElement ("Connects_To_Base") != nullptr;

	Data->canBeCaptured = unitNode->FirstChildElement ("Can_Be_Captured") != nullptr;
	Data->canBeDisabled = unitNode->FirstChildElement ("Can_Be_Disabled") != nullptr;
	Data->canCapture = unitNode->FirstChildElement ("Can_Capture") != nullptr;
	Data->canDisable = unitNode->FirstChildElement ("Can_Disable") != nullptr;
	Data->canSurvey = unitNode->FirstChildElement ("Can_Survey") != nullptr;
	Data->doesSelfRepair = unitNode->FirstChildElement ("Does_Self_Repair") != nullptr;
	Data->canSelfDestroy = unitNode->FirstChildElement ("Can_Self_Destroy") != nullptr;
	Data->explodesOnContact = unitNode->FirstChildElement ("Explodes_On_Contact") != nullptr;
	Data->isHuman = unitNode->FirstChildElement ("Is_Human") != nullptr;
	Data->canBeLandedOn = unitNode->FirstChildElement ("Can_Be_Landed_On") != nullptr;
	Data->canWork = unitNode->FirstChildElement ("Can_Work") != nullptr;

	Data->isBig = unitNode->FirstChildElement ("Is_Big") != nullptr;
	Data->canRepair = unitNode->FirstChildElement ("Can_Repair") != nullptr;
	Data->canRearm = unitNode->FirstChildElement ("Can_Rearm") != nullptr;
	Data->canResearch = unitNode->FirstChildElement ("Can_Research") != nullptr;
	Data->canClearArea = unitNode->FirstChildElement ("Can_Clear") != nullptr;
	Data->canPlaceMines = unitNode->FirstChildElement ("Can_Place_Mines") != nullptr;
	Data->canDriveAndFire = unitNode->FirstChildElement ("Can_Drive_And_Fire") != nullptr;
}

//--------------------------------------------------------------------------
void cSavegame::generateMoveJobs (cServer& server)
{
	for (unsigned int i = 0; i < MoveJobsLoad.size(); ++i)
	{
		cServerMoveJob* MoveJob = new cServerMoveJob (server, MoveJobsLoad[i]->vehicle->getPosition(), cPosition (MoveJobsLoad[i]->destX, MoveJobsLoad[i]->destY), MoveJobsLoad[i]->vehicle);
		if (!MoveJob->calcPath())
		{
			delete MoveJob;
			MoveJobsLoad[i]->vehicle->ServerMoveJob = nullptr;
		}
		else server.addActiveMoveJob (*MoveJob);
		delete MoveJobsLoad[i];
	}
}

//--------------------------------------------------------------------------
cPlayer* cSavegame::getPlayerFromNumber (const std::vector<cPlayer*>& PlayerList, int number)
{
	for (unsigned int i = 0; i < PlayerList.size(); ++i)
	{
		if (PlayerList[i]->getNr() == number) return PlayerList[i];
	}
	return nullptr;
}

//--------------------------------------------------------------------------
string cSavegame::convertScanMapToString (const cPlayer& player) const
{
	string str = "";
	const size_t size = player.getMapSize().x() * player.getMapSize().y();
	str.reserve (size);
	for (int x = 0; x != player.getMapSize().x(); ++x)
	{
		for (int y = 0; y != player.getMapSize().y(); ++y)
		{
			if (player.hasResourceExplored (cPosition (x, y))) str += "1";
			else str += "0";
		}
	}
	return str;
}

//--------------------------------------------------------------------------
void cSavegame::convertStringToScanMap (const string& str, cPlayer& player)
{
	for (int x = 0; x != player.getMapSize().x(); ++x)
	{
		for (int y = 0; y != player.getMapSize().y(); ++y)
		{
			if (!str.compare (x * player.getMapSize().y() + y, 1, "1")) player.exploreResource (cPosition (x, y));
		}
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeHeader (const cServer& server, const string& saveName)
{
	XMLElement* headerNode = addMainElement (SaveFile.RootElement(), "Header");

	addAttributeElement (headerNode, "Game_Version", "string", PACKAGE_VERSION);
	addAttributeElement (headerNode, "Game_Revision", "string", PACKAGE_REV);
	addAttributeElement (headerNode, "Name", "string", saveName);
	switch (server.getGameType())
	{
		case GAME_TYPE_SINGLE: addAttributeElement (headerNode, "Type", "string", "IND"); break;
		case GAME_TYPE_HOTSEAT: addAttributeElement (headerNode, "Type", "string", "HOT"); break;
		case GAME_TYPE_TCPIP: addAttributeElement (headerNode, "Type", "string", "NET"); break;
	}
	char timestr[21];
	time_t tTime = time (nullptr);
	tm* tmTime = localtime (&tTime);
	strftime (timestr, 21, "%d.%m.%y %H:%M", tmTime);

	addAttributeElement (headerNode, "Time", "string", timestr);
}

//--------------------------------------------------------------------------
void cSavegame::writeGameInfo (const cServer& server)
{
	XMLElement* gameinfoNode = addMainElement (SaveFile.RootElement(), "Game");

	addAttributeElement (gameinfoNode, "Turn", "num", iToStr (server.getTurnClock()->getTurn()));
	if (server.isTurnBasedGame()) addAttributeElement (gameinfoNode, "PlayTurns", "activeplayer", iToStr (server.activeTurnPlayer->getNr()));
	if (server.getGameSettings()->getGameType() == eGameSettingsGameType::HotSeat) addAttributeElement (gameinfoNode, "HotSeat", "activeplayer", iToStr (server.activeTurnPlayer->getNr()));

	const auto& gameSetting = *server.getGameSettings();

	addAttributeElement (gameinfoNode, "Metal", "string", gameSettingsResourceAmountToString (gameSetting.getMetalAmount()));
	addAttributeElement (gameinfoNode, "Oil", "string", gameSettingsResourceAmountToString (gameSetting.getOilAmount()));
	addAttributeElement (gameinfoNode, "Gold", "string", gameSettingsResourceAmountToString (gameSetting.getGoldAmount()));

	addAttributeElement (gameinfoNode, "ResourceDensity", "string", gameSettingsResourceDensityToString (gameSetting.getResourceDensity()));

	addAttributeElement (gameinfoNode, "BridgeHead", "string", gameSettingsBridgeheadTypeToString (gameSetting.getBridgeheadType()));

	addAttributeElement (gameinfoNode, "ClansEnabled", "bool", bToStr (gameSetting.getClansEnabled()));

	addAttributeElement (gameinfoNode, "Credits", "num", iToStr (gameSetting.getStartCredits()));

	addAttributeElement (gameinfoNode, "VictoryCondition", "string", gameSettingsVictoryConditionToString (gameSetting.getVictoryCondition()));
	if (gameSetting.getVictoryCondition() == eGameSettingsVictoryCondition::Turns)	addAttributeElement (gameinfoNode, "VictoryTurns", "num", iToStr (gameSetting.getVictoryTurns()));
	else if (gameSetting.getVictoryCondition() == eGameSettingsVictoryCondition::Points) addAttributeElement (gameinfoNode, "VictoryPoints", "num", iToStr (gameSetting.getVictoryPoints()));

	addAttributeElement (gameinfoNode, "TurnEndDeadline", "num", iToStr (gameSetting.getTurnEndDeadline().count()));
	addAttributeElement (gameinfoNode, "TurnEndDeadlineActive", "bool", bToStr (gameSetting.isTurnEndDeadlineActive()));

	addAttributeElement (gameinfoNode, "TurnLimit", "num", iToStr (gameSetting.getTurnLimit().count()));
	addAttributeElement (gameinfoNode, "TurnLimitActive", "bool", bToStr (gameSetting.isTurnLimitActive()));
}

//--------------------------------------------------------------------------
void cSavegame::writeMap (const cMap& Map)
{
	XMLElement* mapNode = addMainElement (SaveFile.RootElement(), "Map");
	addAttributeElement (mapNode, "Name", "string", Map.getName());
	addAttributeElement (mapNode, "Resources", "data", Map.resourcesToString());
}

//--------------------------------------------------------------------------
void cSavegame::writePlayer (const cPlayer& Player, int number)
{
	// generate players node if it doesn't exists
	XMLElement* playersNode;
	if (! (playersNode = SaveFile.RootElement()->FirstChildElement ("Players")))
	{
		playersNode = addMainElement (SaveFile.RootElement(), "Players");
	}

	// add node for the player
	XMLElement* playerNode = addMainElement (playersNode, "Player_" + iToStr (number));

	// write the main information
	addAttributeElement (playerNode, "Name", "string", Player.getName());
	addAttributeElement (playerNode, "Credits", "num", iToStr (Player.getCredits()));
	addAttributeElement (playerNode, "Clan", "num", iToStr (Player.getClan()));

	XMLElement* colorElement = addMainElement (playerNode, "Color");
	colorElement->SetAttribute ("red", iToStr (Player.getColor().getColor().r).c_str());
	colorElement->SetAttribute ("green", iToStr (Player.getColor().getColor().g).c_str());
	colorElement->SetAttribute ("blue", iToStr (Player.getColor().getColor().b).c_str());

	addAttributeElement (playerNode, "Number", "num", iToStr (Player.getNr()));
	addAttributeElement (playerNode, "ResourceMap", "data", convertScanMapToString (Player));

	addAttributeElement (playerNode, "LandingPos", "x", iToStr (Player.getLandingPosX()));
	addAttributeElement (playerNode, "LandingPos", "y", iToStr (Player.getLandingPosY()));

	// player score
	XMLElement* scoreNode = addMainElement (playerNode, "ScoreHistory");
	for (size_t i = 0; i != Player.pointsHistory.size(); ++i)
	{
		XMLElement* e = addMainElement (scoreNode, "Score");
		e->SetAttribute ("num", iToStr (Player.pointsHistory[i]).c_str());
	}

	// write data of upgraded units
	XMLElement* upgradesNode = addMainElement (playerNode, "Upgrades");
	int upgrades = 0;
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (Player.VehicleData[i].getVersion() > 0
			|| Player.VehicleData[i].buildCosts != UnitsData.getVehicle (i, Player.getClan()).buildCosts)
		{
			writeUpgrade (upgradesNode, upgrades, Player.VehicleData[i], UnitsData.getVehicle (i, Player.getClan()));
			upgrades++;
		}
	}
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (Player.BuildingData[i].getVersion() > 0
			|| Player.BuildingData[i].buildCosts != UnitsData.getBuilding (i, Player.getClan()).buildCosts)
		{
			writeUpgrade (upgradesNode, upgrades, Player.BuildingData[i], UnitsData.getBuilding (i, Player.getClan()));
			upgrades++;
		}
	}

	XMLElement* researchNode = addMainElement (playerNode, "Research");
	XMLElement* researchLevelNode = addMainElement (researchNode, "ResearchLevel");
	writeResearchLevel (researchLevelNode, Player.getResearchState());

	const auto& finishedResearchAreas = Player.getCurrentTurnResearchAreasFinished();

	if (!finishedResearchAreas.empty())
	{
		XMLElement* finishedResearchAreasNode = addMainElement (researchNode, "FinishedResearchAreas");
		for (size_t i = 0; i < finishedResearchAreas.size(); ++i)
		{
			addAttributeElement (finishedResearchAreasNode, "Area", "num", iToStr (finishedResearchAreas[i]));
		}
	}

	// write subbases
	XMLElement* subbasesNode = addMainElement (playerNode, "Subbases");
	for (size_t i = 0; i != Player.base.SubBases.size(); ++i)
	{
		const sSubBase& SubBase = *Player.base.SubBases[i];
		XMLElement* subbaseNode = addMainElement (subbasesNode, "Subbase_" + iToStr (i));

		//write the ID of the first building, to identify the subbase at load time
		addAttributeElement (subbaseNode, "buildingID", "num", iToStr (SubBase.buildings[0]->iID));
		XMLElement* element = addMainElement (subbaseNode, "Production");
		element->SetAttribute ("metal", iToStr (SubBase.getMetalProd()).c_str());
		element->SetAttribute ("oil", iToStr (SubBase.getOilProd()).c_str());
		element->SetAttribute ("gold", iToStr (SubBase.getGoldProd()).c_str());
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeUpgrade (XMLElement* upgradesNode, int upgradenumber, const sUnitData& data, const sUnitData& originaldata)
{
	XMLElement* upgradeNode = addMainElement (upgradesNode, "Unit_" + iToStr (upgradenumber));
	addAttributeElement (upgradeNode, "Type", "string", data.ID.getText());
	addAttributeElement (upgradeNode, "Version", "num", iToStr (data.getVersion()));
	if (data.getAmmoMax() != originaldata.getAmmoMax()) addAttributeElement (upgradeNode, "Ammo", "num", iToStr (data.getAmmoMax()));
	if (data.getHitpointsMax() != originaldata.getHitpointsMax()) addAttributeElement (upgradeNode, "HitPoints", "num", iToStr (data.getHitpointsMax()));
	if (data.getShotsMax() != originaldata.getShotsMax()) addAttributeElement (upgradeNode, "Shots", "num", iToStr (data.getShotsMax()));
	if (data.getSpeedMax() != originaldata.getSpeedMax()) addAttributeElement (upgradeNode, "Speed", "num", iToStr (data.getSpeedMax()));
	if (data.getArmor() != originaldata.getArmor()) addAttributeElement (upgradeNode, "Armor", "num", iToStr (data.getArmor()));
	if (data.buildCosts != originaldata.buildCosts) addAttributeElement (upgradeNode, "Costs", "num", iToStr (data.buildCosts));
	if (data.getDamage() != originaldata.getDamage()) addAttributeElement (upgradeNode, "Damage", "num", iToStr (data.getDamage()));
	if (data.getRange() != originaldata.getRange()) addAttributeElement (upgradeNode, "Range", "num", iToStr (data.getRange()));
	if (data.getScan() != originaldata.getScan()) addAttributeElement (upgradeNode, "Scan", "num", iToStr (data.getScan()));
}

//--------------------------------------------------------------------------
void cSavegame::writeResearchLevel (XMLElement* researchLevelNode, const cResearch& researchLevel)
{
	XMLElement* levelNode = addMainElement (researchLevelNode, "Level");
	levelNode->SetAttribute ("attack", iToStr (researchLevel.getCurResearchLevel (cResearch::kAttackResearch)).c_str());
	levelNode->SetAttribute ("shots", iToStr (researchLevel.getCurResearchLevel (cResearch::kShotsResearch)).c_str());
	levelNode->SetAttribute ("range", iToStr (researchLevel.getCurResearchLevel (cResearch::kRangeResearch)).c_str());
	levelNode->SetAttribute ("armor", iToStr (researchLevel.getCurResearchLevel (cResearch::kArmorResearch)).c_str());
	levelNode->SetAttribute ("hitpoints", iToStr (researchLevel.getCurResearchLevel (cResearch::kHitpointsResearch)).c_str());
	levelNode->SetAttribute ("speed", iToStr (researchLevel.getCurResearchLevel (cResearch::kSpeedResearch)).c_str());
	levelNode->SetAttribute ("scan", iToStr (researchLevel.getCurResearchLevel (cResearch::kScanResearch)).c_str());
	levelNode->SetAttribute ("cost", iToStr (researchLevel.getCurResearchLevel (cResearch::kCostResearch)).c_str());

	XMLElement* curPointsNode = addMainElement (researchLevelNode, "CurPoints");
	curPointsNode->SetAttribute ("attack", iToStr (researchLevel.getCurResearchPoints (cResearch::kAttackResearch)).c_str());
	curPointsNode->SetAttribute ("shots", iToStr (researchLevel.getCurResearchPoints (cResearch::kShotsResearch)).c_str());
	curPointsNode->SetAttribute ("range", iToStr (researchLevel.getCurResearchPoints (cResearch::kRangeResearch)).c_str());
	curPointsNode->SetAttribute ("armor", iToStr (researchLevel.getCurResearchPoints (cResearch::kArmorResearch)).c_str());
	curPointsNode->SetAttribute ("hitpoints", iToStr (researchLevel.getCurResearchPoints (cResearch::kHitpointsResearch)).c_str());
	curPointsNode->SetAttribute ("speed", iToStr (researchLevel.getCurResearchPoints (cResearch::kSpeedResearch)).c_str());
	curPointsNode->SetAttribute ("scan", iToStr (researchLevel.getCurResearchPoints (cResearch::kScanResearch)).c_str());
	curPointsNode->SetAttribute ("cost", iToStr (researchLevel.getCurResearchPoints (cResearch::kCostResearch)).c_str());
}

//--------------------------------------------------------------------------
void cSavegame::writeCasualties (const cServer& server)
{
	if (server.getCasualtiesTracker() == 0)
		return;

	XMLElement* casualtiesNode = addMainElement (SaveFile.RootElement(), "Casualties");
	server.getCasualtiesTracker()->storeToXML (casualtiesNode);
}

//--------------------------------------------------------------------------
XMLElement* cSavegame::writeUnit (const cServer& server, const cVehicle& vehicle, int* unitnum)
{
	// add units node if it doesn't exists
	XMLElement* unitsNode;
	if (! (unitsNode = SaveFile.RootElement()->FirstChildElement ("Units")))
	{
		unitsNode = addMainElement (SaveFile.RootElement(), "Units");
		addAttributeElement (unitsNode, "NextUnitID", "num", iToStr (server.iNextUnitID));
	}

	// add the unit node
	XMLElement* unitNode = addMainElement (unitsNode, "Unit_" + iToStr (*unitnum));

	// write main information
	addAttributeElement (unitNode, "Type", "string", vehicle.data.ID.getText());
	addAttributeElement (unitNode, "ID", "num", iToStr (vehicle.iID));
	addAttributeElement (unitNode, "Owner", "num", iToStr (vehicle.getOwner()->getNr()));
	addAttributeElement (unitNode, "Position", "x", iToStr (vehicle.getPosition().x()), "y", iToStr (vehicle.getPosition().y()));
	// add information whether the unitname isn't serverdefault,
	// so that it would be readed when loading
	// but is in the save to make him more readable
	addAttributeElement (unitNode, "Name", "string", vehicle.isNameOriginal() ? vehicle.data.name : vehicle.getName(), "notDefault", vehicle.isNameOriginal() ? "0" : "1");

	// write the standard unit values
	// which are the same for vehicles and buildings
	writeUnitValues (unitNode, vehicle.data, *vehicle.getOwner()->getUnitDataCurrentVersion (vehicle.data.ID));

	// add additional status information
	addAttributeElement (unitNode, "Direction", "num", iToStr (vehicle.dir));
	if (vehicle.data.canCapture || vehicle.data.canDisable) addAttributeElement (unitNode, "CommandoRank", "num", fToStr (vehicle.getCommandoRank()));
	if (vehicle.data.isBig) addMainElement (unitNode, "IsBig");
	if (vehicle.isDisabled()) addAttributeElement (unitNode, "Disabled", "turns", iToStr (vehicle.getDisabledTurns()));
	if (vehicle.isUnitLayingMines()) addMainElement (unitNode, "LayMines");
	if (vehicle.isSentryActive()) addMainElement (unitNode, "OnSentry");
	if (vehicle.isManualFireActive()) addMainElement (unitNode, "ManualFire");
	if (vehicle.hasAutoMoveJob) addMainElement (unitNode, "AutoMoving");

	if (vehicle.isUnitBuildingABuilding() || vehicle.BuildPath)
	{
		XMLElement* element = addMainElement (unitNode, "Building");
		element->SetAttribute ("building", bToStr (vehicle.isUnitBuildingABuilding()).c_str());
		element->SetAttribute ("type_id", vehicle.getBuildingType().getText().c_str());
		element->SetAttribute ("turns", iToStr (vehicle.getBuildTurns()).c_str());
		element->SetAttribute ("costs", iToStr (vehicle.getBuildCosts()).c_str());
		if (vehicle.data.isBig) element->SetAttribute ("savedpos", iToStr (vehicle.buildBigSavedPosition.x() + vehicle.buildBigSavedPosition.y() * server.Map->getSize().x()).c_str());

		if (vehicle.BuildPath)
		{
			element->SetAttribute ("path", "1");
			element->SetAttribute ("turnsstart", iToStr (vehicle.getBuildTurnsStart()).c_str());
			element->SetAttribute ("costsstart", iToStr (vehicle.getBuildCostsStart()).c_str());
			element->SetAttribute ("endx", iToStr (vehicle.bandPosition.x()).c_str());
			element->SetAttribute ("endy", iToStr (vehicle.bandPosition.y()).c_str());
		}
	}
	if (vehicle.isUnitClearing()) addAttributeElement (unitNode, "Clearing", "turns", iToStr (vehicle.getClearingTurns()), "savedpos", iToStr (vehicle.buildBigSavedPosition.x() + vehicle.buildBigSavedPosition.y() * server.Map->getSize().x()));
	if (vehicle.ServerMoveJob) addAttributeElement (unitNode, "Movejob", "destx", iToStr (vehicle.ServerMoveJob->destination.x()), "desty", iToStr (vehicle.ServerMoveJob->destination.y()));

	// write from which players this unit has been detected
	if (vehicle.detectedByPlayerList.empty() == false)
	{
		XMLElement* detecedByNode = addMainElement (unitNode, "IsDetectedByPlayers");
		for (size_t i = 0; i != vehicle.detectedByPlayerList.size(); ++i)
		{
			addAttributeElement (detecedByNode, "Player_" + iToStr (i),
								 "nr", iToStr (vehicle.detectedByPlayerList[i]->getNr()),
								 "ThisTurn", vehicle.wasDetectedInThisTurnByPlayer (vehicle.detectedByPlayerList[i]) ? "1" : "0");
		}
	}

	// write all stored vehicles
	for (size_t i = 0; i != vehicle.storedUnits.size(); ++i)
	{
		(*unitnum) ++;
		XMLElement* storedNode = writeUnit (server, *vehicle.storedUnits[i], unitnum);
		addAttributeElement (storedNode, "Stored_In", "id", iToStr (vehicle.iID), "is_vehicle", "1");
	}
	return unitNode;
}

//--------------------------------------------------------------------------
void cSavegame::writeUnit (const cServer& server, const cBuilding& building, int* unitnum)
{
	// add units node if it doesn't exists
	XMLElement* unitsNode = SaveFile.RootElement()->FirstChildElement ("Units");
	if (!unitsNode)
	{
		unitsNode = addMainElement (SaveFile.RootElement(), "Units");
		addAttributeElement (unitsNode, "NextUnitID", "num", iToStr (server.iNextUnitID));
	}

	// add the unit node
	XMLElement* unitNode = addMainElement (unitsNode, "Unit_" + iToStr (*unitnum));

	// write main information
	addAttributeElement (unitNode, "Type", "string", building.data.ID.getText());
	addAttributeElement (unitNode, "ID", "num", iToStr (building.iID));
	addAttributeElement (unitNode, "Owner", "num", iToStr (building.getOwner()->getNr()));
	addAttributeElement (unitNode, "Position", "x", iToStr (building.getPosition().x()), "y", iToStr (building.getPosition().y()));

	// add information whether the unitname isn't serverdefault, so that it would be readed when loading but is in the save to make him more readable
	addAttributeElement (unitNode, "Name", "string", building.isNameOriginal() ? building.data.name : building.getName(), "notDefault", building.isNameOriginal() ? "0" : "1");

	// write the standard values
	writeUnitValues (unitNode, building.data, *building.getOwner()->getUnitDataCurrentVersion (building.data.ID));

	// write additional stauts information
	if (building.isUnitWorking()) addMainElement (unitNode, "IsWorking");
	if (building.wasWorking) addMainElement (unitNode, "wasWorking");
	if (building.isDisabled()) addAttributeElement (unitNode, "Disabled", "turns", iToStr (building.getDisabledTurns()));

	if (building.data.canResearch)
	{
		XMLElement* researchNode = addMainElement (unitNode, "ResearchArea");
		researchNode->SetAttribute ("area", iToStr (building.getResearchArea()).c_str());
	}
	if (building.data.canScore)
	{
		addAttributeElement (unitNode, "Score", "num", iToStr (building.points));
	}
	if (building.isSentryActive()) addMainElement (unitNode, "OnSentry");
	if (building.isManualFireActive()) addMainElement (unitNode, "ManualFire");
	if (building.hasBeenAttacked()) addMainElement (unitNode, "HasBeenAttacked");

	// write the buildlist
	if (!building.isBuildListEmpty())
	{
		XMLElement* buildNode = addMainElement (unitNode, "Building");
		addAttributeElement (buildNode, "BuildSpeed", "num", iToStr (building.getBuildSpeed()));
		addAttributeElement (buildNode, "MetalPerRound", "num", iToStr (building.getMetalPerRound()));
		if (building.getRepeatBuild()) addMainElement (buildNode, "RepeatBuild");

		XMLElement* buildlistNode = addMainElement (buildNode, "BuildList");
		for (size_t i = 0; i != building.getBuildListSize(); ++i)
		{
			addAttributeElement (buildlistNode, "Item_" + iToStr (i), "type_id", building.getBuildListItem (i).getType().getText(), "metall_remaining", iToStr (building.getBuildListItem (i).getRemainingMetal()));
		}
	}

	// write from which players this unit has been detected
	if (building.detectedByPlayerList.empty() == false)
	{
		XMLElement* detecedByNode = addMainElement (unitNode, "IsDetectedByPlayers");
		for (size_t i = 0; i != building.detectedByPlayerList.size(); ++i)
		{
			addAttributeElement (detecedByNode, "Player_" + iToStr (i), "nr", iToStr (building.detectedByPlayerList[i]->getNr()));
		}
	}

	// write all stored vehicles
	for (size_t i = 0; i != building.storedUnits.size(); ++i)
	{
		(*unitnum) ++;
		XMLElement* storedNode = writeUnit (server, *building.storedUnits[i], unitnum);
		addAttributeElement (storedNode, "Stored_In", "id", iToStr (building.iID), "is_vehicle", "0");
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeRubble (const cServer& server, const cBuilding& building, int rubblenum)
{
	// add units node if it doesn't exists
	XMLElement* unitsNode = SaveFile.RootElement()->FirstChildElement ("Units");
	if (!unitsNode)
	{
		unitsNode = addMainElement (SaveFile.RootElement(), "Units");
		addAttributeElement (unitsNode, "NextUnitID", "num", iToStr (server.iNextUnitID));
	}

	// add the rubble node
	XMLElement* rubbleNode = addMainElement (unitsNode, "Rubble_" + iToStr (rubblenum));

	addAttributeElement (rubbleNode, "Position", "x", iToStr (building.getPosition().x()), "y", iToStr (building.getPosition().y()));
	addAttributeElement (rubbleNode, "RubbleValue", "num", iToStr (building.RubbleValue));
	if (building.data.isBig) addMainElement (rubbleNode, "Big");
}

//--------------------------------------------------------------------------
void cSavegame::writeUnitValues (XMLElement* unitNode, const sUnitData& Data, const sUnitData& OwnerData)
{
	// write the standard status values
	if (Data.getHitpoints() != Data.getHitpointsMax()) addAttributeElement (unitNode, "Hitpoints", "num", iToStr (Data.getHitpoints()));
	if (Data.getAmmo() != Data.getAmmoMax()) addAttributeElement (unitNode, "Ammo", "num", iToStr (Data.getAmmo()));

	if (Data.getStoredResources() > 0) addAttributeElement (unitNode, "ResCargo", "num", iToStr (Data.getStoredResources()));
	if (Data.getStoredUnits() > 0) addAttributeElement (unitNode, "UnitCargo", "num", iToStr (Data.getStoredUnits()));

	if (Data.getSpeed() != Data.getSpeedMax()) addAttributeElement (unitNode, "Speed", "num", iToStr (Data.getSpeed()));
	if (Data.getShots() != Data.getShotsMax()) addAttributeElement (unitNode, "Shots", "num", iToStr (Data.getShots()));

	// write upgrade values that differ from the acctual unit values of the owner
	if (OwnerData.getVersion() <= 0) return;

	addAttributeElement (unitNode, "Version", "num", iToStr (Data.getVersion()));
	if (Data.getHitpointsMax() != OwnerData.getHitpointsMax()) addAttributeElement (unitNode, "Max_Hitpoints", "num", iToStr (Data.getHitpointsMax()));
	if (Data.getAmmoMax() != OwnerData.getAmmoMax()) addAttributeElement (unitNode, "Max_Ammo", "num", iToStr (Data.getAmmoMax()));
	if (Data.getSpeedMax() != OwnerData.getSpeedMax()) addAttributeElement (unitNode, "Max_Speed", "num", iToStr (Data.getSpeedMax()));
	if (Data.getShotsMax() != OwnerData.getShotsMax()) addAttributeElement (unitNode, "Max_Shots", "num", iToStr (Data.getShotsMax()));

	if (Data.getArmor() != OwnerData.getArmor()) addAttributeElement (unitNode, "Armor", "num", iToStr (Data.getArmor()));
	if (Data.getDamage() != OwnerData.getDamage()) addAttributeElement (unitNode, "Damage", "num", iToStr (Data.getDamage()));
	if (Data.getRange() != OwnerData.getRange()) addAttributeElement (unitNode, "Range", "num", iToStr (Data.getRange()));
	if (Data.getScan() != OwnerData.getScan()) addAttributeElement (unitNode, "Scan", "num", iToStr (Data.getScan()));
}

//--------------------------------------------------------------------------
void cSavegame::writeStandardUnitValues (const sUnitData& Data, int unitnum)
{
	// add the main node if it doesn't exists
	XMLElement* unitValuesNode = SaveFile.RootElement()->FirstChildElement ("UnitValues");
	if (!unitValuesNode)
	{
		unitValuesNode = addMainElement (SaveFile.RootElement(), "UnitValues");
	}
	// add the unit node
	XMLElement* unitNode = addMainElement (unitValuesNode, "UnitVal_" + iToStr (unitnum));
	addAttributeElement (unitNode, "ID", "string", Data.ID.getText());
	addAttributeElement (unitNode, "Name", "string", Data.name);

	addAttributeElement (unitNode, "Hitpoints", "num", iToStr (Data.getHitpointsMax()));
	addAttributeElement (unitNode, "Armor", "num", iToStr (Data.getArmor()));
	addAttributeElement (unitNode, "Built_Costs", "num", iToStr (Data.buildCosts));

	if (Data.getScan() > 0) addAttributeElement (unitNode, "Scan", "num", iToStr (Data.getScan()));

	if (Data.getSpeedMax() > 0) addAttributeElement (unitNode, "Movement", "num", iToStr (Data.getSpeedMax() / 4));

	if (Data.muzzleType > 0) addAttributeElement (unitNode, "MuzzleType", "num", iToStr (Data.muzzleType));
	if (Data.getShotsMax() > 0) addAttributeElement (unitNode, "Shots", "num", iToStr (Data.getShotsMax()));
	if (Data.getAmmoMax() > 0) addAttributeElement (unitNode, "Ammo", "num", iToStr (Data.getAmmoMax()));
	if (Data.getRange() > 0) addAttributeElement (unitNode, "Range", "num", iToStr (Data.getRange()));
	if (Data.getDamage() > 0) addAttributeElement (unitNode, "Damage", "num", iToStr (Data.getDamage()));
	if (Data.canAttack != TERRAIN_NONE) addAttributeElement (unitNode, "Can_Attack", "num", iToStr (Data.canAttack));

	if (!Data.canBuild.empty()) addAttributeElement (unitNode, "Can_Build", "string", Data.canBuild);
	if (!Data.buildAs.empty()) addAttributeElement (unitNode, "Build_As", "string", Data.buildAs);
	if (Data.maxBuildFactor > 0) addAttributeElement (unitNode, "Max_Build_Factor", "num", iToStr (Data.maxBuildFactor));

	if (Data.storageResMax > 0) addAttributeElement (unitNode, "Storage_Res_Max", "num", iToStr (Data.storageResMax));
	if (Data.storageUnitsMax > 0) addAttributeElement (unitNode, "Storage_Units_Max", "num", iToStr (Data.storageUnitsMax));
	if (Data.storeResType != sUnitData::STORE_RES_NONE) addAttributeElement (unitNode, "Store_Res_Type", "num", iToStr (Data.storeResType));
	if (Data.storeUnitsImageType != sUnitData::STORE_UNIT_IMG_NONE) addAttributeElement (unitNode, "StoreUnits_Image_Type", "num", iToStr (Data.storeUnitsImageType));
	if (!Data.isStorageType.empty()) addAttributeElement (unitNode, "Is_Storage_Type", "string", Data.isStorageType);
	if (!Data.storeUnitsTypes.empty())
	{
		string storeUnitsTypes = Data.storeUnitsTypes[0];
		for (size_t i = 1; i != Data.storeUnitsTypes.size(); ++i)
		{
			storeUnitsTypes += "+" + Data.storeUnitsTypes[i];
		}
		addAttributeElement (unitNode, "StoreUnitsTypes", "string", storeUnitsTypes);
	}

	if (Data.needsEnergy != 0) addAttributeElement (unitNode, "Needs_Energy", "num", iToStr (Data.needsEnergy));
	if (Data.produceEnergy != 0) addAttributeElement (unitNode, "Needs_Energy", "num", iToStr (-Data.produceEnergy));
	if (Data.needsHumans != 0) addAttributeElement (unitNode, "Needs_Humans", "num", iToStr (Data.needsHumans));
	if (Data.produceHumans != 0) addAttributeElement (unitNode, "Needs_Humans", "num", iToStr (-Data.produceHumans));
	if (Data.needsOil != 0) addAttributeElement (unitNode, "Needs_Oil", "num", iToStr (Data.needsOil));
	if (Data.needsMetal != 0) addAttributeElement (unitNode, "Needs_Metal", "num", iToStr (Data.needsMetal));
	if (Data.convertsGold != 0) addAttributeElement (unitNode, "Converts_Gold", "num", iToStr (Data.convertsGold));
	if (Data.canMineMaxRes != 0) addAttributeElement (unitNode, "Can_Mine_Max_Res", "num", iToStr (Data.canMineMaxRes));

	if (Data.isStealthOn != TERRAIN_NONE) addAttributeElement (unitNode, "Is_Stealth_On", "num", iToStr (Data.isStealthOn));
	if (Data.canDetectStealthOn != TERRAIN_NONE) addAttributeElement (unitNode, "Can_Detect_Stealth_On", "num", iToStr (Data.canDetectStealthOn));

	if (Data.surfacePosition != sUnitData::SURFACE_POS_GROUND) addAttributeElement (unitNode, "Surface_Position", "num", iToStr (Data.surfacePosition));
	if (Data.canBeOverbuild != sUnitData::OVERBUILD_TYPE_NO) addAttributeElement (unitNode, "Can_Be_Overbuild", "num", iToStr (Data.canBeOverbuild));

	if (Data.factorAir != 0.0f) addAttributeElement (unitNode, "Factor_Air", "num", fToStr (Data.factorAir));
	if (Data.factorCoast != 0.0f) addAttributeElement (unitNode, "Factor_Coast", "num", fToStr (Data.factorCoast));
	if (Data.factorGround != 0.0f) addAttributeElement (unitNode, "Factor_Ground", "num", fToStr (Data.factorGround));
	if (Data.factorSea != 0.0f) addAttributeElement (unitNode, "Factor_Sea", "num", fToStr (Data.factorSea));

	if (Data.modifiesSpeed != 0.0f) addAttributeElement (unitNode, "Factor_Sea", "num", fToStr (Data.modifiesSpeed));

	if (Data.canBuildPath) addMainElement (unitNode, "Can_Build_Path");
	if (Data.canBuildRepeat) addMainElement (unitNode, "Can_Build_Repeat");
	if (Data.connectsToBase) addMainElement (unitNode, "Connects_To_Base");

	if (Data.canBeCaptured) addMainElement (unitNode, "Can_Be_Captured");
	if (Data.canBeDisabled) addMainElement (unitNode, "Can_Be_Disabled");
	if (Data.canCapture) addMainElement (unitNode, "Can_Capture");
	if (Data.canDisable) addMainElement (unitNode, "Can_Disable");
	if (Data.canSurvey) addMainElement (unitNode, "Can_Survey");
	if (Data.doesSelfRepair) addMainElement (unitNode, "Does_Self_Repair");
	if (Data.canSelfDestroy) addMainElement (unitNode, "Can_Self_Destroy");
	if (Data.explodesOnContact) addMainElement (unitNode, "Explodes_On_Contact");
	if (Data.isHuman) addMainElement (unitNode, "Is_Human");
	if (Data.canBeLandedOn) addMainElement (unitNode, "Can_Be_Landed_On");
	if (Data.canWork) addMainElement (unitNode, "Can_Work");

	if (Data.isBig) addMainElement (unitNode, "Is_Big");
	if (Data.canRepair) addMainElement (unitNode, "Can_Repair");
	if (Data.canRearm) addMainElement (unitNode, "Can_Rearm");
	if (Data.canResearch) addMainElement (unitNode, "Can_Research");
	if (Data.canClearArea) addMainElement (unitNode, "Can_Clear");
	if (Data.canPlaceMines) addMainElement (unitNode, "Can_Place_Mines");
	if (Data.canDriveAndFire) addMainElement (unitNode, "Can_Drive_And_Fire");
}

//--------------------------------------------------------------------------
void cSavegame::writeAdditionalInfo (const cGameGuiState& gameGuiState, std::vector<std::unique_ptr<cSavedReport>>& list, const cPlayer* player)
{
	string fileName = cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml"; //TODO: private method load()
	if (SaveFile.LoadFile (fileName.c_str()) != 0) return;
	if (!SaveFile.RootElement()) return;
	loadedXMLFileName = fileName;

	// first get the players node
	XMLElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	int playernum = 0;
	XMLElement* playerNode = nullptr;
	do
	{
		playerNode = playersNode->FirstChildElement (("Player_" + iToStr (playernum)).c_str());
		if (!playerNode) return;
		int number;
		playerNode->FirstChildElement ("Number")->QueryIntAttribute ("num", &number);
		if (number == player->getNr()) break;
		playernum++;
	}
	while (playerNode != nullptr);

	XMLElement* hudNode = addMainElement (playerNode, "Hud");
	gameGuiState.pushInto (*hudNode);

	// add reports
	XMLElement* reportsNode = addMainElement (playerNode, "Reports");

	for (size_t i = 0; i != list.size(); ++i)
	{
		XMLElement* reportElement = addMainElement (reportsNode, "Report");
		list[i]->pushInto (*reportElement);
	}
	list.clear();

	if (!DirExists (cSettings::getInstance().getSavesPath()))
	{
		if (makeDir (cSettings::getInstance().getSavesPath())) Log.write ("Created new save directory: " + cSettings::getInstance().getSavesPath(), cLog::eLOG_TYPE_INFO);
		else Log.write ("Can't create save directory: " + cSettings::getInstance().getSavesPath(), cLog::eLOG_TYPE_ERROR);
	}

	SaveFile.SaveFile ((cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml").c_str());
}

//--------------------------------------------------------------------------
void cSavegame::addAttributeElement (XMLElement* node, const string& nodename, const string& attributename, const string& value, const string& attributename2, const string& value2)
{
	XMLElement* element = addMainElement (node, nodename);
	element->SetAttribute (attributename.c_str(), value.c_str());
	if (!attributename2.empty()) element->SetAttribute (attributename2.c_str(), value2.c_str());
}

//--------------------------------------------------------------------------
XMLElement* cSavegame::addMainElement (XMLElement* node, const string& nodename)
{
	XMLElement* element = node->GetDocument()->NewElement (nodename.c_str());
	node->LinkEndChild (element);
	return element;
}
