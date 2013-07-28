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

#include "savegame.h"

#include "buildings.h"
#include "casualtiestracker.h"
#include "files.h"
#include "hud.h"
#include "loaddata.h"
#include "log.h"
#include "menus.h"
#include "movejobs.h"
#include "player.h"
#include "server.h"
#include "settings.h"
#include "upgradecalculator.h"
#include "vehicles.h"
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
	XMLElement* rootnode = SaveFile.NewElement ("MAXR_SAVE_FILE");
	rootnode->SetAttribute ("version", (SAVE_FORMAT_VERSION).c_str());
	SaveFile.LinkEndChild (rootnode);

	writeHeader (server, saveName);
	writeGameInfo (server);
	writeMap (*server.Map);
	writeCasualties (server);

	int unitnum = 0;
	const std::vector<cPlayer*>& playerList = *server.PlayerList;
	for (size_t i = 0; i != playerList.size(); ++i)
	{
		const cPlayer& Player = *playerList[i];
		writePlayer (Player, i);

		for (const cVehicle* vehicle = Player.VehicleList;
			 vehicle;
			 vehicle = vehicle->next)
		{
			if (!vehicle->Loaded)
			{
				writeUnit (server, *vehicle, &unitnum);
				unitnum++;
			}
		}

		for (const cBuilding* building = Player.BuildingList;
			 building;
			 building = building->next)
		{
			writeUnit (server, *building, &unitnum);
			unitnum++;
		}
	}
	int rubblenum = 0;

	for (const cBuilding* rubble = server.neutralBuildings; rubble; rubble = rubble->next)
	{
		writeRubble (server, *rubble, rubblenum);
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
	SaveFile.SaveFile ( (cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml").c_str());

	return 1;
}

//--------------------------------------------------------------------------
bool cSavegame::load (cServer& server)
{
	string fileName = cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml";
	if (SaveFile.LoadFile (fileName.c_str()) != 0)
	{
		return false;
	}
	if (!SaveFile.RootElement()) return false;
	loadedXMLFileName = fileName;

	version = SaveFile.RootElement()->Attribute ("version");
	if (version.compare (SAVE_FORMAT_VERSION))
	{
		Log.write ("Savefile-version differs from the one supported by the game!", cLog::eLOG_TYPE_WARNING);
	}

	// load standard unit values
	if (atoi (version.substr (0, version.find_first_of (".")).c_str()) == 0 &&
		atoi (version.substr (version.find_first_of ("."), version.length() - version.find_first_of (".")).c_str()) <= 2)
	{
		Log.write ("Skiping loading standard unit values because savegame has version 0.2 or older.", LOG_TYPE_DEBUG);
	}
	else
	{
		XMLElement* unitValuesNode = SaveFile.RootElement()->FirstChildElement ("UnitValues");
		if (unitValuesNode != NULL)
		{
			int unitnum = 0;
			XMLElement* unitNode = unitValuesNode->FirstChildElement ("UnitVal_0");
			while (unitNode)
			{
				loadStandardUnitValues (unitNode);

				unitnum++;
				unitNode = unitValuesNode->FirstChildElement ( ("UnitVal_" + iToStr (unitnum)).c_str());
			}
		}
	}

	string gametype;
	loadHeader (NULL, &gametype, NULL);
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
	return true;
}

//--------------------------------------------------------------------------
void cSavegame::recalcSubbases (cServer& server)
{
	std::vector<cPlayer*>& playerList = *server.PlayerList;
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
	const string fileName = cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml";
	if (fileName != loadedXMLFileName)
	{
		loadedXMLFileName.clear();
		if (SaveFile.LoadFile (fileName.c_str()) != 0)
		{
			return;
		}
	}
	if (!SaveFile.RootElement()) return;
	loadedXMLFileName = fileName;

	const XMLElement* headerNode = SaveFile.RootElement()->FirstChildElement ("Header");

	if (name) *name = headerNode->FirstChildElement ("Name")->Attribute ("string");
	if (type) *type = headerNode->FirstChildElement ("Type")->Attribute ("string");
	if (time) *time = headerNode->FirstChildElement ("Time")->Attribute ("string");
}

//--------------------------------------------------------------------------
string cSavegame::getMapName() const
{
	const XMLElement* mapNode = SaveFile.RootElement()->FirstChildElement ("Map");
	if (mapNode != NULL) return mapNode->FirstChildElement ("Name")->Attribute ("string");
	else return "";
}

//--------------------------------------------------------------------------
string cSavegame::getPlayerNames() const
{
	const XMLElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");

	if (playersNode == NULL) return "";

	const XMLElement* playerNode = playersNode->FirstChildElement ("Player_0");
	string playernames = "";
	int playernum = 0;
	while (playerNode)
	{
		playernames += ( (string) playerNode->FirstChildElement ("Name")->Attribute ("string")) + "\n";
		playernum++;
		playerNode = playersNode->FirstChildElement ( ("Player_" + iToStr (playernum)).c_str());
	}
	return playernames;
}

//--------------------------------------------------------------------------
void cSavegame::loadGameInfo (cServer& server)
{
	XMLElement* gameInfoNode = SaveFile.RootElement()->FirstChildElement ("Game");
	if (!gameInfoNode) return;

	server.iTurn = gameInfoNode->FirstChildElement ("Turn")->IntAttribute ("num");

	sSettings gameSetting;

	if (XMLElement* const element = gameInfoNode->FirstChildElement ("PlayTurns"))
	{
		gameSetting.gameType = SETTINGS_GAMETYPE_TURNS;
		server.iActiveTurnPlayerNr = element->IntAttribute ("activeplayer");
	}
#if 1 // old format
	int turnLimit = 0;
	int scoreLimit = 0;
	if (XMLElement* const e = gameInfoNode->FirstChildElement ("TurnLimit")) turnLimit = e->IntAttribute ("num");
	if (XMLElement* const e = gameInfoNode->FirstChildElement ("ScoreLimit")) scoreLimit = e->IntAttribute ("num");

	if (turnLimit != 0)
	{
		gameSetting.victoryType = SETTINGS_VICTORY_TURNS;
		gameSetting.duration = turnLimit;
	}
	else if (scoreLimit != 0)
	{
		gameSetting.victoryType = SETTINGS_VICTORY_POINTS;
		gameSetting.duration = scoreLimit;
	}
	else
	{
		gameSetting.victoryType = SETTINGS_VICTORY_ANNIHILATION;
	}
#endif

	if (XMLElement* e = gameInfoNode->FirstChildElement ("Metal")) gameSetting.metal = (eSettingResourceValue) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("Oil")) gameSetting.oil = (eSettingResourceValue) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("Gold")) gameSetting.gold = (eSettingResourceValue) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("ResFrequency")) gameSetting.resFrequency = (eSettingResFrequency) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("Credits")) gameSetting.credits = e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("BridgeHead")) gameSetting.bridgeHead = (eSettingsBridgeHead) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("AlienTech")) gameSetting.alienTech = (eSettingsAlienTech) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("Clan")) gameSetting.clans = (eSettingsClans) e->IntAttribute ("num");
	//if (XMLElement* e = gameInfoNode->FirstChildElement ("GameType")) gameSetting.gameType = (eSettingsGameType) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("VictoryType")) gameSetting.victoryType = (eSettingsVictoryType) e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("Duration")) gameSetting.duration = e->IntAttribute ("num");
	if (XMLElement* e = gameInfoNode->FirstChildElement ("TurnDeadLine")) gameSetting.iTurnDeadline = e->IntAttribute ("num");

	server.gameSetting = new sSettings (gameSetting);
}

//--------------------------------------------------------------------------
bool cSavegame::loadMap (cServer& server)
{
	XMLElement* mapNode = SaveFile.RootElement()->FirstChildElement ("Map");
	if (mapNode == NULL) return false;

	cStaticMap* staticMap = new cStaticMap;
	string name = mapNode->FirstChildElement ("Name")->Attribute ("string");
	string resourcestr = mapNode->FirstChildElement ("Resources")->Attribute ("data");
	if (!staticMap->loadMap (name))
	{
		delete staticMap;
		return false;
	}
	server.Map = new cMap (*staticMap);
	server.Map->setResourcesFromString (resourcestr);
	return true;
}

//--------------------------------------------------------------------------
void cSavegame::loadPlayers (cServer& server)
{
	std::vector<cPlayer*>* PlayerList = new std::vector<cPlayer*>;
	server.PlayerList = PlayerList;

	XMLElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	if (playersNode == NULL) return;

	int playernum = 0;
	cMap* map = server.Map;
	XMLElement* playerNode = playersNode->FirstChildElement ("Player_0");
	while (playerNode)
	{
		PlayerList->push_back (loadPlayer (playerNode, *map));
		playernum++;
		playerNode = playersNode->FirstChildElement ( ("Player_" + iToStr (playernum)).c_str());
	}
}

//--------------------------------------------------------------------------
cPlayer* cSavegame::loadPlayer (XMLElement* playerNode, cMap& map)
{
	const string name = playerNode->FirstChildElement ("Name")->Attribute ("string");
	const int number = playerNode->FirstChildElement ("Number")->IntAttribute ("num");
	const int color  = playerNode->FirstChildElement ("Color")->IntAttribute ("num");
	cPlayer* Player = new cPlayer (sPlayer (name, color, number));
	Player->initMaps (map);

	Player->Credits = playerNode->FirstChildElement ("Credits")->IntAttribute ("num");

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
		// save the loaded hudoptions to the "HotHud" of the player so that the server can send them later to the clients
		Player->savedHud->offX = hudNode->FirstChildElement ("Offset")->IntAttribute ("x");
		Player->savedHud->offY = hudNode->FirstChildElement ("Offset")->IntAttribute ("y");
		Player->savedHud->zoom = hudNode->FirstChildElement ("Zoom")->FloatAttribute ("num");
		if (hudNode->FirstChildElement ("Colors")) Player->savedHud->colorsChecked = true;
		if (hudNode->FirstChildElement ("Grid")) Player->savedHud->gridChecked = true;
		if (hudNode->FirstChildElement ("Ammo")) Player->savedHud->ammoChecked = true;
		if (hudNode->FirstChildElement ("Fog")) Player->savedHud->fogChecked = true;
		if (hudNode->FirstChildElement ("Range")) Player->savedHud->rangeChecked = true;
		if (hudNode->FirstChildElement ("Scan")) Player->savedHud->scanChecked = true;
		if (hudNode->FirstChildElement ("Status")) Player->savedHud->statusChecked = true;
		if (hudNode->FirstChildElement ("Survey")) Player->savedHud->surveyChecked = true;
		if (hudNode->FirstChildElement ("Hitpoints")) Player->savedHud->hitsChecked = true;
		if (hudNode->FirstChildElement ("MinimapZoom")) Player->savedHud->twoXChecked = true;
		if (hudNode->FirstChildElement ("TNT")) Player->savedHud->tntChecked = true;
		if (hudNode->FirstChildElement ("Lock")) Player->savedHud->lockChecked = true;
		if (hudNode->FirstChildElement ("SelectedUnit")) Player->savedHud->selUnitID = hudNode->FirstChildElement ("SelectedUnit")->IntAttribute ("num");
	}

	// read reports
	const XMLElement* reportsNode = playerNode->FirstChildElement ("Reports");
	if (reportsNode)
	{
		const XMLElement* reportElement = reportsNode->FirstChildElement ("Report");
		while (reportElement)
		{
			if (reportElement->Parent() != reportsNode) break;
			sSavedReportMessage savedReport;
			savedReport.message = reportElement->Attribute ("msg");
			const int tmpInt = reportElement->IntAttribute ("type");
			savedReport.type = (sSavedReportMessage::eReportTypes) tmpInt;
			savedReport.xPos = reportElement->IntAttribute ("xPos");
			savedReport.yPos = reportElement->IntAttribute ("yPos");
			savedReport.unitID.generate (reportElement->Attribute ("id"));
			savedReport.colorNr = reportElement->IntAttribute ("colorNr");
			Player->savedReportsList.push_back (savedReport);
			reportElement = reportElement->NextSiblingElement();
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
			upgradeNode = upgradesNode->FirstChildElement ( ("Unit_" + iToStr (upgradenum)).c_str());
		}
	}

	XMLElement* researchNode = playerNode->FirstChildElement ("Research");
	if (researchNode)
	{
		Player->ResearchCount = researchNode->IntAttribute ("researchCount");
		XMLElement* researchLevelNode = researchNode->FirstChildElement ("ResearchLevel");
		if (researchLevelNode)
			loadResearchLevel (researchLevelNode, Player->researchLevel);
		XMLElement* researchCentersWorkingOnAreaNode = researchNode->FirstChildElement ("CentersWorkingOnArea");
		if (researchCentersWorkingOnAreaNode)
			loadResearchCentersWorkingOnArea (researchCentersWorkingOnAreaNode, Player);
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
			subbaseNode = subbasesNode->FirstChildElement ( ("Subbase_" + iToStr (subbasenum)).c_str());
		}
	}
	return Player;
}

//--------------------------------------------------------------------------
void cSavegame::loadUpgrade (XMLElement* upgradeNode, sUnitData* data)
{
	data->version = upgradeNode->FirstChildElement ("Version")->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("HitPoints")) data->hitpointsMax = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Shots"))     data->shotsMax = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Speed"))     data->speedMax = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Armor"))     data->armor = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Costs"))     data->buildCosts = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Damage"))    data->damage = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Range"))     data->range = element->IntAttribute ("num");
	if (XMLElement* const element = upgradeNode->FirstChildElement ("Scan"))      data->scan = element->IntAttribute ("num");
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
void cSavegame::loadResearchCentersWorkingOnArea (XMLElement* researchCentersWorkingOnAreaNode, cPlayer* player)
{
	int value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("attack");
	player->researchCentersWorkingOnArea[cResearch::kAttackResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("shots");
	player->researchCentersWorkingOnArea[cResearch::kShotsResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("range");
	player->researchCentersWorkingOnArea[cResearch::kRangeResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("armor");
	player->researchCentersWorkingOnArea[cResearch::kArmorResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("hitpoints");
	player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("speed");
	player->researchCentersWorkingOnArea[cResearch::kSpeedResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("scan");
	player->researchCentersWorkingOnArea[cResearch::kScanResearch] = value;
	value = researchCentersWorkingOnAreaNode->IntAttribute ("cost");
	player->researchCentersWorkingOnArea[cResearch::kCostResearch] = value;
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
	if (unitsNode == NULL) return;

	int unitnum = 0;
	XMLElement* unitNode = unitsNode->FirstChildElement ("Unit_0");
	while (unitNode)
	{
		sID ID;
		ID.generate (unitNode->FirstChildElement ("Type")->Attribute ("string"));
		if (ID.isAVehicle()) loadVehicle (server, unitNode, ID);
		else if (ID.isABuilding()) loadBuilding (server, unitNode, ID);

		unitnum++;
		unitNode = unitsNode->FirstChildElement ( ("Unit_" + iToStr (unitnum)).c_str());
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
		rubbleNode = unitsNode->FirstChildElement ( ("Rubble_" + iToStr (rubblenum)).c_str());
	}
	generateMoveJobs (server);
}

//--------------------------------------------------------------------------
void cSavegame::loadVehicle (cServer& server, XMLElement* unitNode, const sID& ID)
{
	int tmpinteger;
	unitNode->FirstChildElement ("Owner")->QueryIntAttribute ("num", &tmpinteger);
	cPlayer* owner = getPlayerFromNumber (*server.PlayerList, tmpinteger);

	int x, y;
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("x", &x);
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("y", &y);
	unitNode->FirstChildElement ("ID")->QueryIntAttribute ("num", &tmpinteger);
	cVehicle* vehicle = server.addVehicle (x, y, ID, owner, true, unitNode->FirstChildElement ("Stored_In") == NULL, tmpinteger);

	if (unitNode->FirstChildElement ("Name")->Attribute ("notDefault") && strcmp (unitNode->FirstChildElement ("Name")->Attribute ("notDefault"), "1") == 0)
		vehicle->changeName (unitNode->FirstChildElement ("Name")->Attribute ("string"));

	loadUnitValues (unitNode, &vehicle->data);

	unitNode->FirstChildElement ("Direction")->QueryIntAttribute ("num", &vehicle->dir);
	if (XMLElement* const element = unitNode->FirstChildElement ("CommandoRank"))
	{
		element->QueryFloatAttribute ("num", &vehicle->CommandoRank);
	}
	if (unitNode->FirstChildElement ("IsBig")) server.Map->moveVehicleBig (*vehicle, x, y);
	if (unitNode->FirstChildElement ("Disabled")) unitNode->FirstChildElement ("Disabled")->QueryIntAttribute ("turns", &vehicle->turnsDisabled);
	if (unitNode->FirstChildElement ("LayMines")) vehicle->LayMines = true;
	if (unitNode->FirstChildElement ("AutoMoving")) vehicle->hasAutoMoveJob = true;
	if (unitNode->FirstChildElement ("OnSentry"))
	{
		owner->addSentry (vehicle);
	}
	if (unitNode->FirstChildElement ("ManualFire")) vehicle->manualFireActive = true;

	if (XMLElement* const element = unitNode->FirstChildElement ("Building"))
	{
		vehicle->IsBuilding = true;
		if (element->Attribute ("type_id") != NULL)
		{
			vehicle->BuildingTyp.generate (element->Attribute ("type_id"));
		}
		// be downward compatible and looke for 'type' too
		else if (element->Attribute ("type") != NULL)
		{
			// element->Attribute ("type", &vehicle->BuildingTyp);
		}
		element->QueryIntAttribute ("turns", &vehicle->BuildRounds);
		element->QueryIntAttribute ("costs", &vehicle->BuildCosts);
		element->QueryIntAttribute ("savedpos", &vehicle->BuildBigSavedPos);

		if (element->Attribute ("path"))
		{
			vehicle->BuildPath = true;
			element->QueryIntAttribute ("turnsstart", &vehicle->BuildRoundsStart);
			element->QueryIntAttribute ("costsstart", &vehicle->BuildCostsStart);
			element->QueryIntAttribute ("endx", &vehicle->BandX);
			element->QueryIntAttribute ("endy", &vehicle->BandY);
		}
	}
	if (XMLElement* const element = unitNode->FirstChildElement ("Clearing"))
	{
		vehicle->IsClearing = true;
		element->QueryIntAttribute ("turns", &vehicle->ClearingRounds);
		element->QueryIntAttribute ("savedpos", &vehicle->BuildBigSavedPos);
	}

	if (XMLElement* const element = unitNode->FirstChildElement ("Movejob"))
	{
		sMoveJobLoad* MoveJob = new sMoveJobLoad;
		MoveJob->vehicle = vehicle;
		element->QueryIntAttribute ("destx", &MoveJob->destX);
		element->QueryIntAttribute ("desty", &MoveJob->destY);

		MoveJobsLoad.push_back (MoveJob);
	}

	// read the players which have detected this unit
	if (XMLElement* const detectedNode = unitNode->FirstChildElement ("IsDetectedByPlayers"))
	{
		int playerNodeNum = 0;
		while (XMLElement* const element = detectedNode->FirstChildElement ( ("Player_" + iToStr (playerNodeNum)).c_str()))
		{
			int playerNum;
			element->QueryIntAttribute ("nr", &playerNum);
			int wasDetectedThisTurnAttrib = 1;
			// for old savegames, that don't have this attribute, set it to "detected this turn"
			if (element->QueryIntAttribute ("ThisTurn", &wasDetectedThisTurnAttrib) != XML_NO_ERROR)
				wasDetectedThisTurnAttrib = 1;
			bool wasDetectedThisTurn = (wasDetectedThisTurnAttrib != 0);
			cPlayer* Player = server.getPlayerFromNumber (playerNum);
			if (Player)
			{
				vehicle->setDetectedByPlayer (server, Player, wasDetectedThisTurn);
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
			cVehicle* StoringVehicle = server.getVehicleFromID (storedInID);
			if (!StoringVehicle) return;

			StoringVehicle->data.storageUnitsCur--;
			StoringVehicle->storeVehicle (vehicle, server.Map);
		}
		else
		{
			cBuilding* StoringBuilding = server.getBuildingFromID (storedInID);
			if (!StoringBuilding) return;

			StoringBuilding->data.storageUnitsCur--;
			StoringBuilding->storeVehicle (vehicle, server.Map);
		}
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadBuilding (cServer& server, XMLElement* unitNode, const sID& ID)
{
	int tmpinteger;
	unitNode->FirstChildElement ("Owner")->QueryIntAttribute ("num", &tmpinteger);
	cPlayer* owner = getPlayerFromNumber (*server.PlayerList, tmpinteger);

	int x, y;
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("x", &x);
	unitNode->FirstChildElement ("Position")->QueryIntAttribute ("y", &y);
	unitNode->FirstChildElement ("ID")->QueryIntAttribute ("num", &tmpinteger);
	cBuilding* building = server.addBuilding (x, y, ID, owner, true, tmpinteger);

	if (unitNode->FirstChildElement ("Name")->Attribute ("notDefault") && strcmp (unitNode->FirstChildElement ("Name")->Attribute ("notDefault"), "1") == 0)
		building->changeName (unitNode->FirstChildElement ("Name")->Attribute ("string"));

	loadUnitValues (unitNode, &building->data);

	if (unitNode->FirstChildElement ("IsWorking")) building->IsWorking = true;
	if (unitNode->FirstChildElement ("wasWorking")) building->wasWorking = true;
	if (unitNode->FirstChildElement ("Disabled")) unitNode->FirstChildElement ("Disabled")->QueryIntAttribute ("turns", &building->turnsDisabled);
	if (unitNode->FirstChildElement ("ResearchArea")) unitNode->FirstChildElement ("ResearchArea")->QueryIntAttribute ("area", & (building->researchArea));
	if (unitNode->FirstChildElement ("Score")) unitNode->FirstChildElement ("Score")->QueryIntAttribute ("num", & (building->points));
	if (unitNode->FirstChildElement ("OnSentry"))
	{
		if (!building->sentryActive)
		{
			owner->addSentry (building);
		}
	}
	else if (building->sentryActive)
	{
		owner->deleteSentry (building);
	}
	if (unitNode->FirstChildElement ("ManualFire")) building->manualFireActive = true;
	if (unitNode->FirstChildElement ("HasBeenAttacked")) building->hasBeenAttacked = true;

	if (XMLElement* const element = unitNode->FirstChildElement ("Building"))
	{
		XMLElement* buildNode = element;
		if (XMLElement* const element = buildNode->FirstChildElement ("BuildSpeed"))    element->QueryIntAttribute ("num", &building->BuildSpeed);
		if (XMLElement* const element = buildNode->FirstChildElement ("MetalPerRound")) element->QueryIntAttribute ("num", &building->MetalPerRound);
		if (buildNode->FirstChildElement ("RepeatBuild")) building->RepeatBuild = true;

		int itemnum = 0;
		const XMLElement* itemElement = buildNode->FirstChildElement ("BuildList")->FirstChildElement ("Item_0");
		while (itemElement)
		{
			sBuildList listitem;
			if (itemElement->Attribute ("type_id") != NULL)
			{
				listitem.type.generate (itemElement->Attribute ("type_id"));
			}
			// be downward compatible and look for 'type' too
			else if (itemElement->Attribute ("type") != NULL)
			{
				int typenr;
				itemElement->QueryIntAttribute ("type", &typenr);
				listitem.type = UnitsData.svehicles[typenr].ID;
			}
			itemElement->QueryIntAttribute ("metall_remaining", &listitem.metall_remaining);
			building->BuildList.push_back (listitem);

			itemnum++;
			itemElement = buildNode->FirstChildElement ("BuildList")->FirstChildElement ( ("Item_" + iToStr (itemnum)).c_str());
		}
	}

	// read the players which have detected this unit
	if (XMLElement* const detectedNode = unitNode->FirstChildElement ("IsDetectedByPlayers"))
	{
		int playerNodeNum = 0;
		while (XMLElement* const element = detectedNode->FirstChildElement ( ("Player_" + iToStr (playerNodeNum)).c_str()))
		{
			int playerNum;
			element->QueryIntAttribute ("nr", &playerNum);
			cPlayer* Player = server.getPlayerFromNumber (playerNum);
			if (Player)
			{
				building->setDetectedByPlayer (server, Player);
			}
			playerNodeNum++;
		}
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

	server.addRubble (x, y, rubblevalue, big);
}

//--------------------------------------------------------------------------
void cSavegame::loadUnitValues (XMLElement* unitNode, sUnitData* Data)
{
	if (XMLElement* const Element = unitNode->FirstChildElement ("Version")) Element->QueryIntAttribute ("num", &Data->version);

	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Hitpoints")) Element->QueryIntAttribute ("num", &Data->hitpointsMax);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Ammo"))      Element->QueryIntAttribute ("num", &Data->ammoMax);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Speed"))     Element->QueryIntAttribute ("num", &Data->speedMax);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Max_Shots"))     Element->QueryIntAttribute ("num", &Data->shotsMax);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Armor"))         Element->QueryIntAttribute ("num", &Data->armor);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Damage"))        Element->QueryIntAttribute ("num", &Data->damage);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Range"))         Element->QueryIntAttribute ("num", &Data->range);
	if (XMLElement* const Element = unitNode->FirstChildElement ("Scan"))          Element->QueryIntAttribute ("num", &Data->scan);

	if (XMLElement* const Element = unitNode->FirstChildElement ("Hitpoints")) Element->QueryIntAttribute ("num", &Data->hitpointsCur);
	else Data->hitpointsCur = Data->hitpointsMax;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Ammo")) Element->QueryIntAttribute ("num", &Data->ammoCur);
	else Data->ammoCur = Data->ammoMax;

	if (XMLElement* const Element = unitNode->FirstChildElement ("ResCargo"))  Element->QueryIntAttribute ("num", &Data->storageResCur);
	if (XMLElement* const Element = unitNode->FirstChildElement ("UnitCargo")) Element->QueryIntAttribute ("num", &Data->storageUnitsCur);
	// look for "Cargo" to be savegamecompatible
	if (XMLElement* const Element = unitNode->FirstChildElement ("Cargo"))
	{
		Element->QueryIntAttribute ("num", &Data->storageResCur);
		Data->storageUnitsCur = Data->storageResCur;
	}

	if (XMLElement* const Element = unitNode->FirstChildElement ("Speed")) Element->QueryIntAttribute ("num", &Data->speedCur);
	else Data->speedCur = Data->speedMax;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Shots")) Element->QueryIntAttribute ("num", &Data->shotsCur);
	else Data->shotsCur = Data->shotsMax;
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
	if (unitNode == NULL) return;
	sUnitData* Data = NULL;

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
	if (Data == NULL) return;

	Data->ID = ID;

	Data->name = unitNode->FirstChildElement ("Name")->Attribute ("string");

	unitNode->FirstChildElement ("Hitpoints")->QueryIntAttribute ("num", &Data->hitpointsMax);
	unitNode->FirstChildElement ("Armor")->QueryIntAttribute ("num", &Data->armor);
	unitNode->FirstChildElement ("Built_Costs")->QueryIntAttribute ("num", &Data->buildCosts);

	if (XMLElement* const Element = unitNode->FirstChildElement ("Scan"))     Element->QueryIntAttribute ("num", &Data->scan);     else Data->scan     = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Movement")) Element->QueryIntAttribute ("num", &Data->speedMax); else Data->speedMax = 0;
	Data->speedMax *= 4;

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
	if (XMLElement* const Element = unitNode->FirstChildElement ("Shots"))  Element->QueryIntAttribute ("num", &Data->shotsMax); else Data->shotsMax = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Ammo"))   Element->QueryIntAttribute ("num", &Data->ammoMax);  else Data->ammoMax  = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Range"))  Element->QueryIntAttribute ("num", &Data->range);    else Data->range    = 0;
	if (XMLElement* const Element = unitNode->FirstChildElement ("Damage")) Element->QueryIntAttribute ("num", &Data->damage);   else Data->damage   = 0;
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
		Split (storeUnitsString, "+", Data->storeUnitsTypes);
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

	Data->canBuildPath = unitNode->FirstChildElement ("Can_Build_Path") != NULL;
	Data->canBuildRepeat = unitNode->FirstChildElement ("Can_Build_Repeat") != NULL;
	Data->connectsToBase = unitNode->FirstChildElement ("Connects_To_Base") != NULL;

	Data->canBeCaptured = unitNode->FirstChildElement ("Can_Be_Captured") != NULL;
	Data->canBeDisabled = unitNode->FirstChildElement ("Can_Be_Disabled") != NULL;
	Data->canCapture = unitNode->FirstChildElement ("Can_Capture") != NULL;
	Data->canDisable = unitNode->FirstChildElement ("Can_Disable") != NULL;
	Data->canSurvey = unitNode->FirstChildElement ("Can_Survey") != NULL;
	Data->doesSelfRepair = unitNode->FirstChildElement ("Does_Self_Repair") != NULL;
	Data->canSelfDestroy = unitNode->FirstChildElement ("Can_Self_Destroy") != NULL;
	Data->explodesOnContact = unitNode->FirstChildElement ("Explodes_On_Contact") != NULL;
	Data->isHuman = unitNode->FirstChildElement ("Is_Human") != NULL;
	Data->canBeLandedOn = unitNode->FirstChildElement ("Can_Be_Landed_On") != NULL;
	Data->canWork = unitNode->FirstChildElement ("Can_Work") != NULL;

	Data->isBig = unitNode->FirstChildElement ("Is_Big") != NULL;
	Data->canRepair = unitNode->FirstChildElement ("Can_Repair") != NULL;
	Data->canRearm = unitNode->FirstChildElement ("Can_Rearm") != NULL;
	Data->canResearch = unitNode->FirstChildElement ("Can_Research") != NULL;
	Data->canClearArea = unitNode->FirstChildElement ("Can_Clear") != NULL;
	Data->canPlaceMines = unitNode->FirstChildElement ("Can_Place_Mines") != NULL;
	Data->canDriveAndFire = unitNode->FirstChildElement ("Can_Drive_And_Fire") != NULL;
}

//--------------------------------------------------------------------------
void cSavegame::generateMoveJobs (cServer& server)
{
	for (unsigned int i = 0; i < MoveJobsLoad.size(); ++i)
	{
		cServerMoveJob* MoveJob = new cServerMoveJob (server, MoveJobsLoad[i]->vehicle->PosX, MoveJobsLoad[i]->vehicle->PosY, MoveJobsLoad[i]->destX, MoveJobsLoad[i]->destY, MoveJobsLoad[i]->vehicle);
		if (!MoveJob->calcPath())
		{
			delete MoveJob;
			MoveJobsLoad[i]->vehicle->ServerMoveJob = NULL;
		}
		else server.addActiveMoveJob (MoveJob);
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
	return NULL;
}

//--------------------------------------------------------------------------
string cSavegame::convertScanMapToString (const cPlayer& player) const
{
	string str = "";
	const size_t size = Square (player.getMapSize());
	str.reserve (size);
	for (size_t i = 0; i != size; ++i)
	{
		if (player.hasResourceExplored (i)) str += "1";
		else str += "0";
	}
	return str;
}

//--------------------------------------------------------------------------
void cSavegame::convertStringToScanMap (const string& str, cPlayer& player)
{
	for (size_t i = 0; i != str.length(); ++i)
	{
		if (!str.substr (i, 1).compare ("1")) player.exploreResource (i);
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeHeader (const cServer& server, const string& saveName)
{
	XMLElement* headerNode = addMainElement (SaveFile.RootElement(), "Header");

	addAttributeElement (headerNode, "Game_Version", "string", PACKAGE_VERSION);
	addAttributeElement (headerNode, "Name", "string", saveName);
	switch (server.getGameType())
	{
		case GAME_TYPE_SINGLE: addAttributeElement (headerNode, "Type", "string", "IND"); break;
		case GAME_TYPE_HOTSEAT: addAttributeElement (headerNode, "Type", "string", "HOT"); break;
		case GAME_TYPE_TCPIP: addAttributeElement (headerNode, "Type", "string", "NET"); break;
	}
	char timestr[21];
	time_t tTime = time (NULL);
	tm* tmTime = localtime (&tTime);
	strftime (timestr, 21, "%d.%m.%y %H:%M", tmTime);

	addAttributeElement (headerNode, "Time", "string", timestr);
}

//--------------------------------------------------------------------------
void cSavegame::writeGameInfo (const cServer& server)
{
	XMLElement* gameinfoNode = addMainElement (SaveFile.RootElement(), "Game");

	addAttributeElement (gameinfoNode, "Turn", "num", iToStr (server.iTurn));
	if (server.isTurnBasedGame()) addAttributeElement (gameinfoNode, "PlayTurns", "activeplayer", iToStr (server.iActiveTurnPlayerNr));

	const sSettings& gameSetting = *server.getGameSettings();
	addAttributeElement (gameinfoNode, "Metal", "num", iToStr (gameSetting.metal));
	addAttributeElement (gameinfoNode, "Oil", "num", iToStr (gameSetting.oil));
	addAttributeElement (gameinfoNode, "Gold", "num", iToStr (gameSetting.gold));
	addAttributeElement (gameinfoNode, "ResFrequency", "num", iToStr (gameSetting.resFrequency));
	addAttributeElement (gameinfoNode, "Credits", "num", iToStr (gameSetting.credits));
	addAttributeElement (gameinfoNode, "BridgeHead", "num", iToStr (gameSetting.bridgeHead));
	addAttributeElement (gameinfoNode, "AlienTech", "num", iToStr (gameSetting.alienTech));
	addAttributeElement (gameinfoNode, "Clan", "num", iToStr (gameSetting.clans));
	//addAttributeElement (gameinfoNode, "GameType", "num", iToStr (gameSetting.gameType));
	addAttributeElement (gameinfoNode, "VictoryType", "num", iToStr (gameSetting.victoryType));
	addAttributeElement (gameinfoNode, "Duration", "num", iToStr (gameSetting.duration));
	addAttributeElement (gameinfoNode, "TurnDeadLine", "num", iToStr (gameSetting.iTurnDeadline));
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
	addAttributeElement (playerNode, "Credits", "num", iToStr (Player.Credits));
	addAttributeElement (playerNode, "Clan", "num", iToStr (Player.getClan()));
	addAttributeElement (playerNode, "Color", "num", iToStr (Player.getColor()));
	addAttributeElement (playerNode, "Number", "num", iToStr (Player.getNr()));
	addAttributeElement (playerNode, "ResourceMap", "data", convertScanMapToString (Player));

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
		if (Player.VehicleData[i].version > 0
			|| Player.VehicleData[i].buildCosts != UnitsData.getVehicle (i, Player.getClan()).buildCosts)
		{
			writeUpgrade (upgradesNode, upgrades, Player.VehicleData[i], UnitsData.getVehicle (i, Player.getClan()));
			upgrades++;
		}
	}
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		// if only costs were researched, the version is not incremented
		if (Player.BuildingData[i].version > 0
			|| Player.BuildingData[i].buildCosts != UnitsData.getBuilding (i, Player.getClan()).buildCosts)
		{
			writeUpgrade (upgradesNode, upgrades, Player.BuildingData[i], UnitsData.getBuilding (i, Player.getClan()));
			upgrades++;
		}
	}

	XMLElement* researchNode = addMainElement (playerNode, "Research");
	researchNode->SetAttribute ("researchCount", iToStr (Player.ResearchCount).c_str());
	XMLElement* researchLevelNode = addMainElement (researchNode, "ResearchLevel");
	writeResearchLevel (researchLevelNode, Player.researchLevel);
	XMLElement* researchCentersWorkingOnAreaNode = addMainElement (researchNode, "CentersWorkingOnArea");
	writeResearchCentersWorkingOnArea (researchCentersWorkingOnAreaNode, Player);

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
	addAttributeElement (upgradeNode, "Version", "num", iToStr (data.version));
	if (data.ammoMax != originaldata.ammoMax) addAttributeElement (upgradeNode, "Ammo", "num", iToStr (data.ammoMax));
	if (data.hitpointsMax != originaldata.hitpointsMax) addAttributeElement (upgradeNode, "HitPoints", "num", iToStr (data.hitpointsMax));
	if (data.shotsMax != originaldata.shotsMax) addAttributeElement (upgradeNode, "Shots", "num", iToStr (data.shotsMax));
	if (data.speedMax != originaldata.speedMax) addAttributeElement (upgradeNode, "Speed", "num", iToStr (data.speedMax));
	if (data.armor != originaldata.armor) addAttributeElement (upgradeNode, "Armor", "num", iToStr (data.armor));
	if (data.buildCosts != originaldata.buildCosts) addAttributeElement (upgradeNode, "Costs", "num", iToStr (data.buildCosts));
	if (data.damage != originaldata.damage) addAttributeElement (upgradeNode, "Damage", "num", iToStr (data.damage));
	if (data.range != originaldata.range) addAttributeElement (upgradeNode, "Range", "num", iToStr (data.range));
	if (data.scan != originaldata.scan) addAttributeElement (upgradeNode, "Scan", "num", iToStr (data.scan));
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
void cSavegame::writeResearchCentersWorkingOnArea (XMLElement* researchCentersWorkingOnAreaNode, const cPlayer& player)
{
	researchCentersWorkingOnAreaNode->SetAttribute ("attack", iToStr (player.researchCentersWorkingOnArea[cResearch::kAttackResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("shots", iToStr (player.researchCentersWorkingOnArea[cResearch::kShotsResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("range", iToStr (player.researchCentersWorkingOnArea[cResearch::kRangeResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("armor", iToStr (player.researchCentersWorkingOnArea[cResearch::kArmorResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("hitpoints", iToStr (player.researchCentersWorkingOnArea[cResearch::kHitpointsResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("speed", iToStr (player.researchCentersWorkingOnArea[cResearch::kSpeedResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("scan", iToStr (player.researchCentersWorkingOnArea[cResearch::kScanResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("cost", iToStr (player.researchCentersWorkingOnArea[cResearch::kCostResearch]).c_str());
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
	addAttributeElement (unitNode, "Owner", "num", iToStr (vehicle.owner->getNr()));
	addAttributeElement (unitNode, "Position", "x", iToStr (vehicle.PosX), "y", iToStr (vehicle.PosY));
	// add information whether the unitname isn't serverdefault,
	// so that it would be readed when loading
	// but is in the save to make him more readable
	addAttributeElement (unitNode, "Name", "string", vehicle.isNameOriginal() ? vehicle.data.name : vehicle.getName(), "notDefault", vehicle.isNameOriginal() ? "0" : "1");

	// write the standard unit values
	// which are the same for vehicles and buildings
	writeUnitValues (unitNode, vehicle.data, *vehicle.owner->getUnitDataCurrentVersion (vehicle.data.ID));

	// add additional status information
	addAttributeElement (unitNode, "Direction", "num", iToStr (vehicle.dir));
	if (vehicle.data.canCapture || vehicle.data.canDisable) addAttributeElement (unitNode, "CommandoRank", "num", fToStr (vehicle.CommandoRank));
	if (vehicle.data.isBig) addMainElement (unitNode, "IsBig");
	if (vehicle.isDisabled()) addAttributeElement (unitNode, "Disabled", "turns", iToStr (vehicle.turnsDisabled));
	if (vehicle.LayMines) addMainElement (unitNode, "LayMines");
	if (vehicle.sentryActive) addMainElement (unitNode, "OnSentry");
	if (vehicle.manualFireActive) addMainElement (unitNode, "ManualFire");
	if (vehicle.hasAutoMoveJob) addMainElement (unitNode, "AutoMoving");

	if (vehicle.IsBuilding)
	{
		XMLElement* element = addMainElement (unitNode, "Building");
		element->SetAttribute ("type_id", vehicle.BuildingTyp.getText().c_str());
		element->SetAttribute ("turns", iToStr (vehicle.BuildRounds).c_str());
		element->SetAttribute ("costs", iToStr (vehicle.BuildCosts).c_str());
		if (vehicle.data.isBig) element->SetAttribute ("savedpos", iToStr (vehicle.BuildBigSavedPos).c_str());

		if (vehicle.BuildPath)
		{
			element->SetAttribute ("path", "1");
			element->SetAttribute ("turnsstart", iToStr (vehicle.BuildRoundsStart).c_str());
			element->SetAttribute ("costsstart", iToStr (vehicle.BuildCostsStart).c_str());
			element->SetAttribute ("endx", iToStr (vehicle.BandX).c_str());
			element->SetAttribute ("endy", iToStr (vehicle.BandY).c_str());
		}
	}
	if (vehicle.IsClearing) addAttributeElement (unitNode, "Clearing", "turns", iToStr (vehicle.ClearingRounds), "savedpos", iToStr (vehicle.BuildBigSavedPos));
	if (vehicle.ServerMoveJob) addAttributeElement (unitNode, "Movejob", "destx", iToStr (vehicle.ServerMoveJob->DestX), "desty", iToStr (vehicle.ServerMoveJob->DestY));

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
	addAttributeElement (unitNode, "Owner", "num", iToStr (building.owner->getNr()));
	addAttributeElement (unitNode, "Position", "x", iToStr (building.PosX), "y", iToStr (building.PosY));

	// add information whether the unitname isn't serverdefault, so that it would be readed when loading but is in the save to make him more readable
	addAttributeElement (unitNode, "Name", "string", building.isNameOriginal() ? building.data.name : building.getName(), "notDefault", building.isNameOriginal() ? "0" : "1");

	// write the standard values
	writeUnitValues (unitNode, building.data, *building.owner->getUnitDataCurrentVersion (building.data.ID));

	// write additional stauts information
	if (building.IsWorking) addMainElement (unitNode, "IsWorking");
	if (building.wasWorking) addMainElement (unitNode, "wasWorking");
	if (building.isDisabled()) addAttributeElement (unitNode, "Disabled", "turns", iToStr (building.turnsDisabled));

	if (building.data.canResearch)
	{
		XMLElement* researchNode = addMainElement (unitNode, "ResearchArea");
		researchNode->SetAttribute ("area", iToStr (building.researchArea).c_str());
	}
	if (building.data.canScore)
	{
		addAttributeElement (unitNode, "Score", "num", iToStr (building.points));
	}
	if (building.sentryActive) addMainElement (unitNode, "OnSentry");
	if (building.manualFireActive) addMainElement (unitNode, "ManualFire");
	if (building.hasBeenAttacked) addMainElement (unitNode, "HasBeenAttacked");

	// write the buildlist
	if (building.BuildList.empty() == false)
	{
		XMLElement* buildNode = addMainElement (unitNode, "Building");
		addAttributeElement (buildNode, "BuildSpeed", "num", iToStr (building.BuildSpeed));
		addAttributeElement (buildNode, "MetalPerRound", "num", iToStr (building.MetalPerRound));
		if (building.RepeatBuild) addMainElement (buildNode, "RepeatBuild");

		XMLElement* buildlistNode = addMainElement (buildNode, "BuildList");
		for (size_t i = 0; i != building.BuildList.size(); ++i)
		{
			addAttributeElement (buildlistNode, "Item_" + iToStr (i), "type_id", building.BuildList[i].type.getText(), "metall_remaining", iToStr (building.BuildList[i].metall_remaining));
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

	addAttributeElement (rubbleNode, "Position", "x", iToStr (building.PosX), "y", iToStr (building.PosY));
	addAttributeElement (rubbleNode, "RubbleValue", "num", iToStr (building.RubbleValue));
	if (building.data.isBig) addMainElement (rubbleNode, "Big");
}

//--------------------------------------------------------------------------
void cSavegame::writeUnitValues (XMLElement* unitNode, const sUnitData& Data, const sUnitData& OwnerData)
{
	// write the standard status values
	if (Data.hitpointsCur != Data.hitpointsMax) addAttributeElement (unitNode, "Hitpoints", "num", iToStr (Data.hitpointsCur));
	if (Data.ammoCur != Data.ammoMax) addAttributeElement (unitNode, "Ammo", "num", iToStr (Data.ammoCur));

	if (Data.storageResCur > 0) addAttributeElement (unitNode, "ResCargo", "num", iToStr (Data.storageResCur));
	if (Data.storageUnitsCur > 0) addAttributeElement (unitNode, "UnitCargo", "num", iToStr (Data.storageUnitsCur));

	if (Data.speedCur != Data.speedMax) addAttributeElement (unitNode, "Speed", "num", iToStr (Data.speedCur));
	if (Data.shotsCur != Data.shotsMax) addAttributeElement (unitNode, "Shots", "num", iToStr (Data.shotsCur));

	// write upgrade values that differ from the acctual unit values of the owner
	if (OwnerData.version <= 0) return;

	addAttributeElement (unitNode, "Version", "num", iToStr (Data.version));
	if (Data.hitpointsMax != OwnerData.hitpointsMax) addAttributeElement (unitNode, "Max_Hitpoints", "num", iToStr (Data.hitpointsMax));
	if (Data.ammoMax != OwnerData.ammoMax) addAttributeElement (unitNode, "Max_Ammo", "num", iToStr (Data.ammoMax));
	if (Data.speedMax != OwnerData.speedMax) addAttributeElement (unitNode, "Max_Speed", "num", iToStr (Data.speedMax));
	if (Data.shotsMax != OwnerData.shotsMax) addAttributeElement (unitNode, "Max_Shots", "num", iToStr (Data.shotsMax));

	if (Data.armor != OwnerData.armor) addAttributeElement (unitNode, "Armor", "num", iToStr (Data.armor));
	if (Data.damage != OwnerData.damage) addAttributeElement (unitNode, "Damage", "num", iToStr (Data.damage));
	if (Data.range != OwnerData.range) addAttributeElement (unitNode, "Range", "num", iToStr (Data.range));
	if (Data.scan != OwnerData.scan) addAttributeElement (unitNode, "Scan", "num", iToStr (Data.scan));
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

	addAttributeElement (unitNode, "Hitpoints", "num", iToStr (Data.hitpointsMax));
	addAttributeElement (unitNode, "Armor", "num", iToStr (Data.armor));
	addAttributeElement (unitNode, "Built_Costs", "num", iToStr (Data.buildCosts));

	if (Data.scan > 0) addAttributeElement (unitNode, "Scan", "num", iToStr (Data.scan));

	if (Data.speedMax > 0) addAttributeElement (unitNode, "Movement", "num", iToStr (Data.speedMax / 4));

	if (Data.muzzleType > 0) addAttributeElement (unitNode, "MuzzleType", "num", iToStr (Data.muzzleType));
	if (Data.shotsMax > 0) addAttributeElement (unitNode, "Shots", "num", iToStr (Data.shotsMax));
	if (Data.ammoMax > 0) addAttributeElement (unitNode, "Ammo", "num", iToStr (Data.ammoMax));
	if (Data.range > 0) addAttributeElement (unitNode, "Range", "num", iToStr (Data.range));
	if (Data.damage > 0) addAttributeElement (unitNode, "Damage", "num", iToStr (Data.damage));
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
void cSavegame::writeAdditionalInfo (sHudStateContainer hudState, std::vector<sSavedReportMessage>& list, const cPlayer* player)
{
	string fileName = cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml"; //TODO: private method load()
	if (SaveFile.LoadFile (fileName.c_str()) != 0) return;
	if (!SaveFile.RootElement()) return;
	loadedXMLFileName = fileName;

	// first get the players node
	XMLElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	int playernum = 0;
	XMLElement* playerNode = NULL;
	do
	{
		playerNode = playersNode->FirstChildElement ( ("Player_" + iToStr (playernum)).c_str());
		if (!playerNode) return;
		int number;
		playerNode->FirstChildElement ("Number")->QueryIntAttribute ("num", &number);
		if (number == player->getNr()) break;
		playernum++;
	}
	while (playerNode != NULL);

	// write the hud settings
	XMLElement* hudNode = addMainElement (playerNode, "Hud");
	addAttributeElement (hudNode, "SelectedUnit", "num", iToStr (hudState.selUnitID));
	addAttributeElement (hudNode, "Offset", "x", iToStr (hudState.offX), "y", iToStr (hudState.offY));
	addAttributeElement (hudNode, "Zoom", "num", fToStr (hudState.zoom));
	if (hudState.colorsChecked) addMainElement (hudNode, "Colors");
	if (hudState.gridChecked) addMainElement (hudNode, "Grid");
	if (hudState.ammoChecked) addMainElement (hudNode, "Ammo");
	if (hudState.fogChecked) addMainElement (hudNode, "Fog");
	if (hudState.twoXChecked) addMainElement (hudNode, "MinimapZoom");
	if (hudState.rangeChecked) addMainElement (hudNode, "Range");
	if (hudState.scanChecked) addMainElement (hudNode, "Scan");
	if (hudState.statusChecked) addMainElement (hudNode, "Status");
	if (hudState.surveyChecked) addMainElement (hudNode, "Survey");
	if (hudState.lockChecked) addMainElement (hudNode, "Lock");
	if (hudState.hitsChecked) addMainElement (hudNode, "Hitpoints");
	if (hudState.tntChecked) addMainElement (hudNode, "TNT");

	// add reports
	XMLElement* reportsNode = addMainElement (playerNode, "Reports");

	for (size_t i = 0; i != list.size(); ++i)
	{
		XMLElement* reportElement = addMainElement (reportsNode, "Report");
		reportElement->SetAttribute ("msg", list[i].message.c_str());
		reportElement->SetAttribute ("type", iToStr (list[i].type).c_str());
		reportElement->SetAttribute ("xPos", iToStr (list[i].xPos).c_str());
		reportElement->SetAttribute ("yPos", iToStr (list[i].yPos).c_str());
		reportElement->SetAttribute ("id", list[i].unitID.getText().c_str());
		reportElement->SetAttribute ("colorNr", iToStr (list[i].colorNr).c_str());
	}
	list.clear();

	if (!DirExists (cSettings::getInstance().getSavesPath()))
	{
		if (makeDir (cSettings::getInstance().getSavesPath())) Log.write ("Created new save directory: " + cSettings::getInstance().getSavesPath(), cLog::eLOG_TYPE_INFO);
		else Log.write ("Can't create save directory: " + cSettings::getInstance().getSavesPath(), cLog::eLOG_TYPE_ERROR);
	}

	SaveFile.SaveFile ( (cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml").c_str());
}

//--------------------------------------------------------------------------
void cSavegame::addAttributeElement (XMLElement* node, const string& nodename, const string& attributename, const string& value, const string& attributename2, const string& value2)
{
	XMLElement* element = addMainElement (node, nodename);
	element->SetAttribute (attributename.c_str(), value.c_str());
	if (attributename2.compare ("")) element->SetAttribute (attributename2.c_str(), value2.c_str());
}

//--------------------------------------------------------------------------
XMLElement* cSavegame::addMainElement (XMLElement* node, const string& nodename)
{
	XMLElement* element = node->GetDocument()->NewElement (nodename.c_str());
	node->LinkEndChild (element);
	return element;
}
