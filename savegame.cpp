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
#include "server.h"
#include "loaddata.h"
#include "upgradecalculator.h"
#include "menus.h"
#include "settings.h"
#include "hud.h"
#include "files.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "movejobs.h"
#include "casualtiestracker.h"

using namespace std;

//--------------------------------------------------------------------------
cSavegame::cSavegame (int number) :
	SaveFile()
{
	this->number = number;
	if (number > 100) return;
	sprintf (numberstr, "%.3d", number);
}

//--------------------------------------------------------------------------
int cSavegame::save (const cServer& server, const string& saveName)
{
	TiXmlElement* rootnode = new TiXmlElement ("MAXR_SAVE_FILE");
	rootnode->SetAttribute ("version", (SAVE_FORMAT_VERSION).c_str());
	SaveFile.LinkEndChild (rootnode);

	writeHeader (server, saveName);
	writeGameInfo (server);
	writeMap (server.Map);
	writeCasualties (server);

	int unitnum = 0;
	const std::vector<cPlayer*>& playerList = *server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const cPlayer* Player = playerList[i];
		writePlayer (Player, i);

		for (const cVehicle* vehicle = Player->VehicleList;
			 vehicle;
			 vehicle = vehicle->next)
		{
			if (!vehicle->Loaded)
			{
				writeUnit (server, *vehicle, &unitnum);
				unitnum++;
			}
		}

		for (const cBuilding* building = Player->BuildingList;
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

	for (unsigned int i = 0; i < UnitsData.getNrVehicles() + UnitsData.getNrBuildings(); i++)
	{
		const sUnitData* Data;
		if (i < UnitsData.getNrVehicles()) Data = &UnitsData.vehicle[i].data;
		else Data = &UnitsData.building[i - UnitsData.getNrVehicles()].data;
		writeStandardUnitValues (Data, i);
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
int cSavegame::load (cTCP* network)
{
	if (!SaveFile.LoadFile ( (cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml").c_str()))
	{
		return 0;
	}
	if (!SaveFile.RootElement()) return 0;

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
		TiXmlElement* unitValuesNode = SaveFile.RootElement()->FirstChildElement ("UnitValues");
		if (unitValuesNode != NULL)
		{
			int unitnum = 0;
			TiXmlElement* unitNode = unitValuesNode->FirstChildElement ("UnitVal_0");
			while (unitNode)
			{
				loadStandardUnitValues (unitNode);

				unitnum++;
				unitNode = unitValuesNode->FirstChildElement ( ("UnitVal_" + iToStr (unitnum)).c_str());
			}
		}
	}

	cMap* map = loadMap();
	if (!map) return 0;
	std::vector<cPlayer*>* PlayerList = loadPlayers (map);

	string gametype;
	loadHeader (NULL, &gametype, NULL);
	if (!gametype.compare ("IND")) Server = new cServer (network, *map, PlayerList, GAME_TYPE_SINGLE, false);
	else if (!gametype.compare ("HOT")) Server = new cServer (network, *map, PlayerList, GAME_TYPE_HOTSEAT, false);
	else if (!gametype.compare ("NET")) Server = new cServer (network, *map, PlayerList, GAME_TYPE_TCPIP, false);
	else
	{
		Log.write ("Unknown gametype \"" + gametype + "\". Starting as singleplayergame.", cLog::eLOG_TYPE_INFO);
		Server = new cServer (network, *map, PlayerList, GAME_TYPE_SINGLE, false);
	}
	cServer& server = *Server;
	loadGameInfo (server);
	loadUnits (server);
	loadCasualties (server);

	recalcSubbases (server);
	return 1;
}

//--------------------------------------------------------------------------
void cSavegame::recalcSubbases(cServer& server)
{
	std::vector<cPlayer*>& playerList = *server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		playerList[i]->base.refreshSubbases();
	}

	//set the loaded ressource production values
	for (unsigned int i = 0; i < SubBasesLoad.size(); i++)
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
	string fileName = cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml";
	if (fileName != SaveFile.Value())
	{
		if (!SaveFile.LoadFile (fileName.c_str()))
		{
			return;
		}
	}
	if (!SaveFile.RootElement()) return;

	TiXmlElement* headerNode = SaveFile.RootElement()->FirstChildElement ("Header");

	if (name) *name = headerNode->FirstChildElement ("Name")->Attribute ("string");
	if (type) *type = headerNode->FirstChildElement ("Type")->Attribute ("string");
	if (time) *time = headerNode->FirstChildElement ("Time")->Attribute ("string");
}

//--------------------------------------------------------------------------
string cSavegame::getMapName() const
{
	const TiXmlElement* mapNode = SaveFile.RootElement()->FirstChildElement ("Map");
	if (mapNode != NULL) return mapNode->FirstChildElement ("Name")->Attribute ("string");
	else return "";
}

//--------------------------------------------------------------------------
string cSavegame::getPlayerNames() const
{
	string playernames = "";
	const TiXmlElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	if (playersNode != NULL)
	{
		int playernum = 0;
		const TiXmlElement* playerNode = playersNode->FirstChildElement ("Player_0");
		while (playerNode)
		{
			playernames += ( (string) playerNode->FirstChildElement ("Name")->Attribute ("string")) + "\n";
			playernum++;
			playerNode = playersNode->FirstChildElement ( ("Player_" + iToStr (playernum)).c_str());
		}
	}
	return playernames;
}

//--------------------------------------------------------------------------
void cSavegame::loadGameInfo (cServer& server)
{
	TiXmlElement* gameInfoNode = SaveFile.RootElement()->FirstChildElement ("Game");
	if (!gameInfoNode) return;

	gameInfoNode->FirstChildElement ("Turn")->Attribute ("num", &server.iTurn);
	if (TiXmlElement* const element = gameInfoNode->FirstChildElement ("Hotseat"))
	{
		server.bHotSeat = true;
		element->Attribute ("activeplayer", &server.iHotSeatPlayer);
	}
	if (TiXmlElement* const element = gameInfoNode->FirstChildElement ("PlayTurns"))
	{
		server.bPlayTurns = true;
		element->Attribute ("activeplayer", &server.iActiveTurnPlayerNr);
	}

	if (TiXmlElement* const e = gameInfoNode->FirstChildElement ("TurnLimit"))  e->Attribute ("num", &server.turnLimit);
	if (TiXmlElement* const e = gameInfoNode->FirstChildElement ("ScoreLimit")) e->Attribute ("num", &server.scoreLimit);
}

//--------------------------------------------------------------------------
cMap* cSavegame::loadMap()
{
	TiXmlElement* mapNode = SaveFile.RootElement()->FirstChildElement ("Map");
	if (mapNode != NULL)
	{
		cMap* map = new cMap;
		string name = mapNode->FirstChildElement ("Name")->Attribute ("string");
		string resourcestr = mapNode->FirstChildElement ("Resources")->Attribute ("data");
		if (!map->LoadMap (name))
		{
			delete map;
			return NULL;
		}
		convertStringToData (resourcestr, map->size * map->size, map->Resources);
		return map;
	}
	else return NULL;
}

//--------------------------------------------------------------------------
std::vector<cPlayer*>* cSavegame::loadPlayers (cMap* map)
{
	std::vector<cPlayer*>* PlayerList = new std::vector<cPlayer*>;

	TiXmlElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	if (playersNode != NULL)
	{
		int playernum = 0;
		TiXmlElement* playerNode = playersNode->FirstChildElement ("Player_0");
		while (playerNode)
		{
			PlayerList->push_back (loadPlayer (playerNode, map));
			playernum++;
			playerNode = playersNode->FirstChildElement ( ("Player_" + iToStr (playernum)).c_str());
		}
	}
	else
	{
		// warning
	}
	return PlayerList;
}

//--------------------------------------------------------------------------
cPlayer* cSavegame::loadPlayer (TiXmlElement* playerNode, cMap* map)
{
	int number, color;

	string name = playerNode->FirstChildElement ("Name")->Attribute ("string");
	playerNode->FirstChildElement ("Number")->Attribute ("num", &number);
	playerNode->FirstChildElement ("Color")->Attribute ("num", &color);

	cPlayer* Player = new cPlayer (name, OtherData.colors[color], number);
	Player->InitMaps (map->size, map);

	playerNode->FirstChildElement ("Credits")->Attribute ("num", &Player->Credits);

	if (TiXmlElement* const e = playerNode->FirstChildElement ("ScoreHistory"))
	{
		TiXmlElement* s = e->FirstChildElement ("Score");
		int num = 0, i = 0;
		while (s)
		{
			s->Attribute ("num", &num);
			Player->pointsHistory.resize (i + 1);
			Player->pointsHistory[i] = num;
			i++;
			s = s->NextSiblingElement ("Score");
		}
		// add current turn
		Player->pointsHistory.push_back (num);
	}
	else
		number = 0;

	int clan = -1;
	if (TiXmlElement* const element = playerNode->FirstChildElement ("Clan")) element->Attribute ("num", &clan);
	Player->setClan (clan);

	string resourceMap = playerNode->FirstChildElement ("ResourceMap")->Attribute ("data");
	convertStringToScanMap (resourceMap, Player->ResourceMap);

	TiXmlElement* hudNode = playerNode->FirstChildElement ("Hud");
	if (hudNode)
	{
		double tmpDouble;
		// save the loaded hudoptions to the "HotHud" of the player so that the server can send them later to the clients
		hudNode->FirstChildElement ("Offset")->Attribute ("x", &Player->savedHud->offX);
		hudNode->FirstChildElement ("Offset")->Attribute ("y", &Player->savedHud->offY);
		hudNode->FirstChildElement ("Zoom")->Attribute ("num", &tmpDouble);
		Player->savedHud->zoom = (float) tmpDouble;
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
		if (hudNode->FirstChildElement ("SelectedUnit")) hudNode->FirstChildElement ("SelectedUnit")->Attribute ("num", &Player->savedHud->selUnitID);
	}

	// read reports
	TiXmlElement* reportsNode = playerNode->FirstChildElement ("Reports");
	if (reportsNode)
	{
		TiXmlElement* reportElement = reportsNode->FirstChildElement ("Report");
		while (reportElement)
		{
			if (reportElement->Parent() != reportsNode) break;
			sSavedReportMessage savedReport;
			savedReport.message = reportElement->Attribute ("msg");
			int tmpInt;
			reportElement->Attribute ("type", &tmpInt);
			savedReport.type = (sSavedReportMessage::eReportTypes) tmpInt;
			reportElement->Attribute ("xPos", &savedReport.xPos);
			reportElement->Attribute ("yPos", &savedReport.yPos);
			savedReport.unitID.generate (reportElement->Attribute ("id"));
			reportElement->Attribute ("colorNr", &savedReport.colorNr);
			Player->savedReportsList.push_back (savedReport);
			reportElement = reportElement->NextSiblingElement();
		}
	}

	TiXmlElement* upgradesNode = playerNode->FirstChildElement ("Upgrades");
	if (upgradesNode)
	{
		int upgradenum = 0;
		TiXmlElement* upgradeNode = upgradesNode->FirstChildElement ("Unit_0");
		while (upgradeNode)
		{
			sID ID;
			ID.generate (upgradeNode->FirstChildElement ("Type")->Attribute ("string"));
			if (ID.iFirstPart == 0)
			{
				unsigned int num;
				for (num = 0; num < UnitsData.getNrVehicles(); num++) if (UnitsData.vehicle[num].data.ID == ID) break;
				loadUpgrade (upgradeNode, &Player->VehicleData[num]);
			}
			else
			{
				unsigned int num;
				for (num = 0; num < UnitsData.getNrBuildings(); num++) if (UnitsData.building[num].data.ID == ID) break;
				loadUpgrade (upgradeNode, &Player->BuildingData[num]);
			}
			upgradenum++;
			upgradeNode = upgradesNode->FirstChildElement ( ("Unit_" + iToStr (upgradenum)).c_str());
		}
	}

	TiXmlElement* researchNode = playerNode->FirstChildElement ("Research");
	if (researchNode)
	{
		researchNode->Attribute ("researchCount", & (Player->ResearchCount));
		TiXmlElement* researchLevelNode = researchNode->FirstChildElement ("ResearchLevel");
		if (researchLevelNode)
			loadResearchLevel (researchLevelNode, Player->researchLevel);
		TiXmlElement* researchCentersWorkingOnAreaNode = researchNode->FirstChildElement ("CentersWorkingOnArea");
		if (researchCentersWorkingOnAreaNode)
			loadResearchCentersWorkingOnArea (researchCentersWorkingOnAreaNode, Player);
	}

	if (TiXmlElement* const subbasesNode = playerNode->FirstChildElement ("Subbases"))
	{
		int subbasenum = 0;
		TiXmlElement* subbaseNode = subbasesNode->FirstChildElement ("Subbase_0");
		while (subbaseNode)
		{

			TiXmlElement* buildingIDNode = subbaseNode->FirstChildElement ("buildingID");
			if (buildingIDNode)
			{
				sSubBaseLoad* subBaseLoad = new sSubBaseLoad;

				buildingIDNode->Attribute ("num", &subBaseLoad->buildingID);
				subbaseNode->FirstChildElement ("Production")->Attribute ("metal", &subBaseLoad->metalProd);
				subbaseNode->FirstChildElement ("Production")->Attribute ("oil", &subBaseLoad->oilProd);
				subbaseNode->FirstChildElement ("Production")->Attribute ("gold", &subBaseLoad->goldProd);
				SubBasesLoad.push_back (subBaseLoad);
			}
			subbasenum++;
			subbaseNode = subbasesNode->FirstChildElement ( ("Subbase_" + iToStr (subbasenum)).c_str());
		}
	}
	return Player;
}

//--------------------------------------------------------------------------
void cSavegame::loadUpgrade (TiXmlElement* upgradeNode, sUnitData* data)
{
	upgradeNode->FirstChildElement ("Version")->Attribute ("num", &data->version);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Ammo"))      element->Attribute ("num", &data->ammoMax);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("HitPoints")) element->Attribute ("num", &data->hitpointsMax);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Shots"))     element->Attribute ("num", &data->shotsMax);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Speed"))     element->Attribute ("num", &data->speedMax);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Armor"))     element->Attribute ("num", &data->armor);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Costs"))     element->Attribute ("num", &data->buildCosts);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Damage"))    element->Attribute ("num", &data->damage);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Range"))     element->Attribute ("num", &data->range);
	if (TiXmlElement* const element = upgradeNode->FirstChildElement ("Scan"))      element->Attribute ("num", &data->scan);
}

//--------------------------------------------------------------------------
void cSavegame::loadResearchLevel (TiXmlElement* researchLevelNode, cResearch& researchLevel)
{
	int value;
	researchLevelNode->FirstChildElement ("Level")->Attribute ("attack", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kAttackResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("shots", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kShotsResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("range", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kRangeResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("armor", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kArmorResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("hitpoints", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kHitpointsResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("speed", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kSpeedResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("scan", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kScanResearch);
	researchLevelNode->FirstChildElement ("Level")->Attribute ("cost", &value);
	researchLevel.setCurResearchLevel (value, cResearch::kCostResearch);

	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("attack", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kAttackResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("shots", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kShotsResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("range", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kRangeResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("armor", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kArmorResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("hitpoints", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kHitpointsResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("speed", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kSpeedResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("scan", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kScanResearch);
	researchLevelNode->FirstChildElement ("CurPoints")->Attribute ("cost", &value);
	researchLevel.setCurResearchPoints (value, cResearch::kCostResearch);
}

//--------------------------------------------------------------------------
void cSavegame::loadResearchCentersWorkingOnArea (TiXmlElement* researchCentersWorkingOnAreaNode, cPlayer* player)
{
	int value;
	researchCentersWorkingOnAreaNode->Attribute ("attack", &value);
	player->researchCentersWorkingOnArea[cResearch::kAttackResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("shots", &value);
	player->researchCentersWorkingOnArea[cResearch::kShotsResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("range", &value);
	player->researchCentersWorkingOnArea[cResearch::kRangeResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("armor", &value);
	player->researchCentersWorkingOnArea[cResearch::kArmorResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("hitpoints", &value);
	player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("speed", &value);
	player->researchCentersWorkingOnArea[cResearch::kSpeedResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("scan", &value);
	player->researchCentersWorkingOnArea[cResearch::kScanResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ("cost", &value);
	player->researchCentersWorkingOnArea[cResearch::kCostResearch] = value;
}

//--------------------------------------------------------------------------
void cSavegame::loadCasualties (cServer& server)
{
	if (server.getCasualtiesTracker() == 0)
		return;

	TiXmlElement* casualtiesNode = SaveFile.RootElement()->FirstChildElement ("Casualties");
	if (casualtiesNode == 0)
		return;

	server.getCasualtiesTracker()->initFromXML (casualtiesNode);
}

//--------------------------------------------------------------------------
void cSavegame::loadUnits (cServer& server)
{
	TiXmlElement* unitsNode = SaveFile.RootElement()->FirstChildElement ("Units");
	if (unitsNode != NULL)
	{
		int unitnum = 0;
		TiXmlElement* unitNode = unitsNode->FirstChildElement ("Unit_0");
		while (unitNode)
		{
			sID ID;
			ID.generate (unitNode->FirstChildElement ("Type")->Attribute ("string"));
			if (ID.iFirstPart == 0) loadVehicle (server, unitNode, ID);
			else if (ID.iFirstPart == 1) loadBuilding (server, unitNode, ID);

			unitnum++;
			unitNode = unitsNode->FirstChildElement ( ("Unit_" + iToStr (unitnum)).c_str());
		}
		// read nextid-value before loading rubble, so that the rubble will get new ids.
		int nextID;
		unitsNode->FirstChildElement ("NextUnitID")->Attribute ("num", &nextID);
		server.iNextUnitID = nextID;

		int rubblenum = 0;
		TiXmlElement* rubbleNode = unitsNode->FirstChildElement ("Rubble_0");
		while (rubbleNode)
		{
			loadRubble (server, rubbleNode);
			rubblenum++;
			rubbleNode = unitsNode->FirstChildElement ( ("Rubble_" + iToStr (rubblenum)).c_str());
		}
		generateMoveJobs (server);
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadVehicle (cServer& server, TiXmlElement* unitNode, sID& ID)
{
	int tmpinteger, number = -1, x, y;
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
	{
		if (UnitsData.vehicle[i].data.ID == ID)
		{
			number = i;
			break;
		}
		if (i == UnitsData.getNrVehicles() - 1) return;
	}
	unitNode->FirstChildElement ("Owner")->Attribute ("num", &tmpinteger);
	cPlayer* owner = getPlayerFromNumber (*server.PlayerList, tmpinteger);

	unitNode->FirstChildElement ("Position")->Attribute ("x", &x);
	unitNode->FirstChildElement ("Position")->Attribute ("y", &y);
	unitNode->FirstChildElement ("ID")->Attribute ("num", &tmpinteger);
	cVehicle* vehicle = server.addUnit (x, y, &UnitsData.vehicle[number], owner, true, unitNode->FirstChildElement ("Stored_In") == NULL, tmpinteger);

	if (unitNode->FirstChildElement ("Name")->Attribute ("notDefault") && strcmp (unitNode->FirstChildElement ("Name")->Attribute ("notDefault"), "1") == 0)
		vehicle->changeName (unitNode->FirstChildElement ("Name")->Attribute ("string"));

	loadUnitValues (unitNode, &vehicle->data);

	unitNode->FirstChildElement ("Direction")->Attribute ("num", &vehicle->dir);
	double tmpdouble;
	if (TiXmlElement* const element = unitNode->FirstChildElement ("CommandoRank"))
	{
		element->Attribute ("num", &tmpdouble);
		vehicle->CommandoRank = (float) tmpdouble;
	}
	if (unitNode->FirstChildElement ("IsBig")) server.Map->moveVehicleBig (vehicle, x, y);
	if (unitNode->FirstChildElement ("Disabled")) unitNode->FirstChildElement ("Disabled")->Attribute ("turns", &vehicle->turnsDisabled);
	if (unitNode->FirstChildElement ("LayMines")) vehicle->LayMines = true;
	if (unitNode->FirstChildElement ("AutoMoving")) vehicle->hasAutoMoveJob = true;
	if (unitNode->FirstChildElement ("OnSentry"))
	{
		owner->addSentry (vehicle);
	}
	if (unitNode->FirstChildElement ("ManualFire")) vehicle->manualFireActive = true;

	if (TiXmlElement* const element = unitNode->FirstChildElement ("Building"))
	{
		vehicle->IsBuilding = true;
		if (element->Attribute ("type_id") != NULL)
		{
			vehicle->BuildingTyp.generate (element->Attribute ("type_id"));
		}
		// be downward compatible and looke for 'type' too
		else if (element->Attribute ("type") != NULL)
		{
			//element->Attribute ( "type", &vehicle->BuildingTyp );
		}
		element->Attribute ("turns", &vehicle->BuildRounds);
		element->Attribute ("costs", &vehicle->BuildCosts);
		element->Attribute ("savedpos", &vehicle->BuildBigSavedPos);

		if (element->Attribute ("path"))
		{
			vehicle->BuildPath = true;
			element->Attribute ("turnsstart", &vehicle->BuildRoundsStart);
			element->Attribute ("costsstart", &vehicle->BuildCostsStart);
			element->Attribute ("endx", &vehicle->BandX);
			element->Attribute ("endy", &vehicle->BandY);
		}
	}
	if (TiXmlElement* const element = unitNode->FirstChildElement ("Clearing"))
	{
		vehicle->IsClearing = true;
		element->Attribute ("turns", &vehicle->ClearingRounds);
		element->Attribute ("savedpos", &vehicle->BuildBigSavedPos);
	}

	if (TiXmlElement* const element = unitNode->FirstChildElement ("Movejob"))
	{
		sMoveJobLoad* MoveJob = new sMoveJobLoad;
		MoveJob->vehicle = vehicle;
		element->Attribute ("destx", &MoveJob->destX);
		element->Attribute ("desty", &MoveJob->destY);

		MoveJobsLoad.push_back (MoveJob);
	}

	// read the players which have detected this unit
	if (TiXmlElement* const detectedNode = unitNode->FirstChildElement ("IsDetectedByPlayers"))
	{
		int playerNodeNum = 0;
		while (TiXmlElement* const element = detectedNode->FirstChildElement ( ("Player_" + iToStr (playerNodeNum)).c_str()))
		{
			int playerNum;
			element->Attribute ("nr", &playerNum);
			int wasDetectedThisTurnAttrib = 1;
			if (element->Attribute ("ThisTurn", &wasDetectedThisTurnAttrib) == 0)
				wasDetectedThisTurnAttrib = 1; // for old savegames, that don't have this attribute, set it to "detected this turn"
			bool wasDetectedThisTurn = (wasDetectedThisTurnAttrib != 0);
			cPlayer* Player = server.getPlayerFromNumber (playerNum);
			if (Player)
			{
				vehicle->setDetectedByPlayer (server, Player, wasDetectedThisTurn);
			}
			playerNodeNum++;
		}
	}

	// since we write all stored vehicles imediatly after the storing unit we can be sure that this one has been loaded yet
	if (TiXmlElement* const element = unitNode->FirstChildElement ("Stored_In"))
	{
		int storedInID;
		int isVehicle;
		element->Attribute ("id", &storedInID);
		element->Attribute ("is_vehicle", &isVehicle);
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
void cSavegame::loadBuilding (cServer& server, TiXmlElement* unitNode, sID& ID)
{
	int tmpinteger, number = -1, x, y;
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
	{
		if (UnitsData.building[i].data.ID == ID)
		{
			number = i;
			break;
		}
		if (i == UnitsData.getNrBuildings() - 1) return;
	}
	unitNode->FirstChildElement ("Owner")->Attribute ("num", &tmpinteger);
	cPlayer* owner = getPlayerFromNumber (*server.PlayerList, tmpinteger);

	unitNode->FirstChildElement ("Position")->Attribute ("x", &x);
	unitNode->FirstChildElement ("Position")->Attribute ("y", &y);
	unitNode->FirstChildElement ("ID")->Attribute ("num", &tmpinteger);
	cBuilding* building = server.addUnit (x, y, &UnitsData.building[number], owner, true, tmpinteger);

	if (unitNode->FirstChildElement ("Name")->Attribute ("notDefault") && strcmp (unitNode->FirstChildElement ("Name")->Attribute ("notDefault"), "1") == 0)
		building->changeName (unitNode->FirstChildElement ("Name")->Attribute ("string"));

	loadUnitValues (unitNode, &building->data);

	if (unitNode->FirstChildElement ("IsWorking")) building->IsWorking = true;
	if (unitNode->FirstChildElement ("wasWorking")) building->wasWorking = true;
	if (unitNode->FirstChildElement ("Disabled")) unitNode->FirstChildElement ("Disabled")->Attribute ("turns", &building->turnsDisabled);
	if (unitNode->FirstChildElement ("ResearchArea")) unitNode->FirstChildElement ("ResearchArea")->Attribute ("area", & (building->researchArea));
	if (unitNode->FirstChildElement ("Score")) unitNode->FirstChildElement ("Score")->Attribute ("num", & (building->points));
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

	if (TiXmlElement* const element = unitNode->FirstChildElement ("Building"))
	{
		TiXmlElement* buildNode = element;
		if (TiXmlElement* const element = buildNode->FirstChildElement ("BuildSpeed"))    element->Attribute ("num", &building->BuildSpeed);
		if (TiXmlElement* const element = buildNode->FirstChildElement ("MetalPerRound")) element->Attribute ("num", &building->MetalPerRound);
		if (buildNode->FirstChildElement ("RepeatBuild")) building->RepeatBuild = true;

		int itemnum = 0;
		TiXmlElement* itemElement = buildNode->FirstChildElement ("BuildList")->FirstChildElement ("Item_0");
		while (itemElement)
		{
			sBuildList* listitem = new sBuildList;
			if (itemElement->Attribute ("type_id") != NULL)
			{
				listitem->type.generate (itemElement->Attribute ("type_id"));
			}
			// be downward compatible and looke for 'type' too
			else if (itemElement->Attribute ("type") != NULL)
			{
				int typenr;
				itemElement->Attribute ("type", &typenr);
				listitem->type = UnitsData.vehicle[typenr].data.ID;
			}
			itemElement->Attribute ("metall_remaining", &listitem->metall_remaining);
			building->BuildList->push_back (listitem);

			itemnum++;
			itemElement = buildNode->FirstChildElement ("BuildList")->FirstChildElement ( ("Item_" + iToStr (itemnum)).c_str());
		}
	}

	// read the players which have detected this unit
	if (TiXmlElement* const detectedNode = unitNode->FirstChildElement ("IsDetectedByPlayers"))
	{
		int playerNodeNum = 0;
		while (TiXmlElement* const element = detectedNode->FirstChildElement ( ("Player_" + iToStr (playerNodeNum)).c_str()))
		{
			int playerNum;
			element->Attribute ("nr", &playerNum);
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
void cSavegame::loadRubble (cServer& server, TiXmlElement* rubbleNode)
{
	int x, y, rubblevalue;
	bool big = false;

	rubbleNode->FirstChildElement ("Position")->Attribute ("x", &x);
	rubbleNode->FirstChildElement ("Position")->Attribute ("y", &y);
	rubbleNode->FirstChildElement ("RubbleValue")->Attribute ("num", &rubblevalue);

	if (rubbleNode->FirstChildElement ("Big")) big = true;

	server.addRubble (x, y, rubblevalue, big);
}

//--------------------------------------------------------------------------
void cSavegame::loadUnitValues (TiXmlElement* unitNode, sUnitData* Data)
{
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Version")) Element->Attribute ("num", &Data->version);

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Max_Hitpoints")) Element->Attribute ("num", &Data->hitpointsMax);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Max_Ammo"))      Element->Attribute ("num", &Data->ammoMax);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Max_Speed"))     Element->Attribute ("num", &Data->speedMax);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Max_Shots"))     Element->Attribute ("num", &Data->shotsMax);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Armor"))         Element->Attribute ("num", &Data->armor);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Damage"))        Element->Attribute ("num", &Data->damage);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Range"))         Element->Attribute ("num", &Data->range);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Scan"))          Element->Attribute ("num", &Data->scan);

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Hitpoints")) Element->Attribute ("num", &Data->hitpointsCur);
	else Data->hitpointsCur = Data->hitpointsMax;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Ammo")) Element->Attribute ("num", &Data->ammoCur);
	else Data->ammoCur = Data->ammoMax;

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("ResCargo"))  Element->Attribute ("num", &Data->storageResCur);
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("UnitCargo")) Element->Attribute ("num", &Data->storageUnitsCur);
	// look for "Cargo" to be savegamecompatible
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Cargo"))
	{
		Element->Attribute ("num", &Data->storageResCur);
		Data->storageUnitsCur = Data->storageResCur;
	}

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Speed")) Element->Attribute ("num", &Data->speedCur);
	else Data->speedCur = Data->speedMax;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Shots")) Element->Attribute ("num", &Data->shotsCur);
	else Data->shotsCur = Data->shotsMax;
}

//--------------------------------------------------------------------------
void cSavegame::loadStandardUnitValues (TiXmlElement* unitNode)
{
	if (unitNode == NULL) return;
	sUnitData* Data = NULL;

	// get the unit data
	sID ID;
	ID.generate (unitNode->FirstChildElement ("ID")->Attribute ("string"));
	if (ID.iFirstPart == 0)
	{
		for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
		{
			if (UnitsData.vehicle[i].data.ID == ID)
			{
				Data = &UnitsData.vehicle[i].data;
				break;
			}
		}
	}
	else if (ID.iFirstPart == 1)
	{
		for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
		{
			if (UnitsData.building[i].data.ID == ID)
			{
				Data = &UnitsData.building[i].data;
				break;
			}
		}
	}
	else return;
	if (Data == NULL) return;

	Data->ID = ID;

	Data->name = unitNode->FirstChildElement ("Name")->Attribute ("string");

	unitNode->FirstChildElement ("Hitpoints")->Attribute ("num", &Data->hitpointsMax);
	unitNode->FirstChildElement ("Armor")->Attribute ("num", &Data->armor);
	unitNode->FirstChildElement ("Built_Costs")->Attribute ("num", &Data->buildCosts);

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Scan"))     Element->Attribute ("num", &Data->scan);     else Data->scan     = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Movement")) Element->Attribute ("num", &Data->speedMax); else Data->speedMax = 0;
	Data->speedMax *= 4;

	int tmpInt;

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("MuzzleType"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->muzzleType = (sUnitData::eMuzzleType) tmpInt;
	}
	else
	{
		Data->muzzleType = sUnitData::MUZZLE_TYPE_NONE;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Shots"))  Element->Attribute ("num", &Data->shotsMax); else Data->shotsMax = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Ammo"))   Element->Attribute ("num", &Data->ammoMax);  else Data->ammoMax  = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Range"))  Element->Attribute ("num", &Data->range);    else Data->range    = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Damage")) Element->Attribute ("num", &Data->damage);   else Data->damage   = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Can_Attack"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->canAttack = tmpInt;
	}
	else
	{
		Data->canAttack = TERRAIN_NONE;
	}

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Can_Build"))        Data->canBuild = Element->Attribute ("string");    else Data->canBuild = "";
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Build_As"))         Data->buildAs  = Element->Attribute ("string");    else Data->buildAs = "";
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Max_Build_Factor")) Element->Attribute ("num", &Data->maxBuildFactor); else Data->maxBuildFactor = 0;

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Storage_Res_Max"))   Element->Attribute ("num", &Data->storageResMax);   else Data->storageResMax   = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Storage_Units_Max")) Element->Attribute ("num", &Data->storageUnitsMax); else Data->storageUnitsMax = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Store_Res_Type"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->storeResType = (sUnitData::eStorageResType) tmpInt;
	}
	else
	{
		Data->storeResType = sUnitData::STORE_RES_NONE;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("StoreUnits_Image_Type"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->storeUnitsImageType = (sUnitData::eStorageUnitsImageType) tmpInt;
	}
	else
	{
		Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_NONE;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Is_Storage_Type")) Data->isStorageType = Element->Attribute ("string"); else Data->isStorageType = "";
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("StoreUnitsTypes"))
	{
		string storeUnitsString = Element->Attribute ("string");
		if (storeUnitsString.length() > 0)
		{
			int pos = -1;
			do
			{
				int lastpos = pos;
				pos = storeUnitsString.find_first_of ("+", pos + 1);
				if (pos == string::npos) pos = storeUnitsString.length();
				Data->storeUnitsTypes.push_back (storeUnitsString.substr (lastpos + 1, pos - (lastpos + 1)));
			}
			while (pos < (int) storeUnitsString.length());
		}
	}

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Needs_Energy"))     Element->Attribute ("num", &Data->needsEnergy);   else Data->needsEnergy   = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Needs_Humans"))     Element->Attribute ("num", &Data->needsHumans);   else Data->needsHumans   = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Needs_Oil"))        Element->Attribute ("num", &Data->needsOil);      else Data->needsOil      = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Needs_Metal"))      Element->Attribute ("num", &Data->needsMetal);    else Data->needsMetal    = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Converts_Gold"))    Element->Attribute ("num", &Data->convertsGold);  else Data->convertsGold  = 0;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Can_Mine_Max_Res")) Element->Attribute ("num", &Data->canMineMaxRes); else Data->canMineMaxRes = 0;
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

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Is_Stealth_On"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->isStealthOn = tmpInt;
	}
	else
	{
		Data->isStealthOn = TERRAIN_NONE;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Can_Detect_Stealth_On"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->canDetectStealthOn = tmpInt;
	}
	else
	{
		Data->canDetectStealthOn = TERRAIN_NONE;
	}

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Surface_Position"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->surfacePosition = (sUnitData::eSurfacePosition) tmpInt;
	}
	else
	{
		Data->surfacePosition = sUnitData::SURFACE_POS_GROUND;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Can_Be_Overbuild"))
	{
		Element->Attribute ("num", &tmpInt);
		Data->canBeOverbuild = (sUnitData::eOverbuildType) tmpInt;
	}
	else
	{
		Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_NO;
	}

	double tmpdouble;
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Factor_Air"))
	{
		Element->Attribute ("num", &tmpdouble);
		Data->factorAir = (float) tmpdouble;
	}
	else
	{
		Data->factorAir = 0;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Factor_Coast"))
	{
		Element->Attribute ("num", &tmpdouble);
		Data->factorCoast = (float) tmpdouble;
	}
	else
	{
		Data->factorCoast = 0;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Factor_Ground"))
	{
		Element->Attribute ("num", &tmpdouble);
		Data->factorGround = (float) tmpdouble;
	}
	else
	{
		Data->factorGround = 0;
	}
	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Factor_Sea"))
	{
		Element->Attribute ("num", &tmpdouble);
		Data->factorSea = (float) tmpdouble;
	}
	else
	{
		Data->factorSea = 0;
	}

	if (TiXmlElement* const Element = unitNode->FirstChildElement ("Factor_Sea"))
	{
		Element->Attribute ("num", &tmpdouble);
		Data->modifiesSpeed = (float) tmpdouble;
	}
	else
	{
		Data->modifiesSpeed = 0;
	}

	Data->canBuildPath = unitNode->FirstChildElement ("Can_Build_Path") != NULL;
	Data->canBuildRepeat = unitNode->FirstChildElement ("Can_Build_Repeat") != NULL;
	Data->buildIntern = unitNode->FirstChildElement ("Build_Intern") != NULL;
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
	for (unsigned int i = 0; i < MoveJobsLoad.size(); i++)
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
	for (unsigned int i = 0; i < PlayerList.size(); i++)
	{
		if (PlayerList[i]->Nr == number) return PlayerList[i];
	}
	return NULL;
}

//--------------------------------------------------------------------------
string cSavegame::convertDataToString (const sResources* resources, int size) const
{
	string str = "";
	for (int i = 0; i < size; i++)
	{
		str += getHexValue (resources[i].typ);
		str += getHexValue (resources[i].value);
	}
	return str;
}

//--------------------------------------------------------------------------
string cSavegame::getHexValue (unsigned char byte) const
{
	string str = "";
	const char hexChars[] = "0123456789ABCDEF";
	const unsigned char high = (byte >> 4) & 0x0F;
	const unsigned char low = byte & 0x0F;

	str += hexChars[high];
	str += hexChars[low];
	return str;
}

//--------------------------------------------------------------------------
void cSavegame::convertStringToData (const string& str, int size, sResources* resources)
{
	for (int i = 0; i < size; i++)
	{
		resources[i].typ = getByteValue (str.substr (i * 4, 2));
		resources[i].value = getByteValue (str.substr (i * 4 + 2, 2));
	}
}

//--------------------------------------------------------------------------
unsigned char cSavegame::getByteValue (const string& str) const
{
	unsigned char first = str[0] - '0';
	unsigned char second = str[1] - '0';

	if (first >= 'A' - '0')
		first -= 'A' - '0' - 10;
	if (second >= 'A' - '0')
		second -= 'A' - '0' - 10;
	return (first * 16 + second);
}

//--------------------------------------------------------------------------
string cSavegame::convertScanMapToString (const char* data, int size) const
{
	string str = "";
	for (int i = 0; i < size; i++)
	{
		if (data[i] > 0) str += "1";
		else str += "0";
	}
	return str;
}

//--------------------------------------------------------------------------
void cSavegame::convertStringToScanMap (const string& str, char* data)
{
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (!str.substr (i, 1).compare ("1")) data[i] = 1;
		else data[i] = 0;
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeHeader (const cServer& server, const string& saveName)
{
	TiXmlElement* headerNode = addMainElement (SaveFile.RootElement(), "Header");

	addAttributeElement (headerNode, "Game_Version", "string", PACKAGE_VERSION);
	addAttributeElement (headerNode, "Name", "string", saveName);
	switch (server.gameType)
	{
		case GAME_TYPE_SINGLE:
			addAttributeElement (headerNode, "Type", "string", "IND");
			break;
		case GAME_TYPE_HOTSEAT:
			addAttributeElement (headerNode, "Type", "string", "HOT");
			break;
		case GAME_TYPE_TCPIP:
			addAttributeElement (headerNode, "Type", "string", "NET");
			break;
	}
	time_t tTime;
	tm* tmTime;
	char timestr[21];
	tTime = time (NULL);
	tmTime = localtime (&tTime);
	strftime (timestr, 21, "%d.%m.%y %H:%M", tmTime);

	addAttributeElement (headerNode, "Time", "string", timestr);
}

//--------------------------------------------------------------------------
void cSavegame::writeGameInfo (const cServer& server)
{
	TiXmlElement* gemeinfoNode = addMainElement (SaveFile.RootElement(), "Game");

	addAttributeElement (gemeinfoNode, "Turn", "num", iToStr (server.iTurn));
	if (server.bHotSeat) addAttributeElement (gemeinfoNode, "Hotseat", "activeplayer", iToStr (server.iHotSeatPlayer));
	if (server.bPlayTurns) addAttributeElement (gemeinfoNode, "PlayTurns", "activeplayer", iToStr (server.iActiveTurnPlayerNr));

	addAttributeElement (gemeinfoNode, "TurnLimit", "num", iToStr (server.turnLimit));
	addAttributeElement (gemeinfoNode, "ScoreLimit", "num", iToStr (server.scoreLimit));
}

//--------------------------------------------------------------------------
void cSavegame::writeMap (const cMap* Map)
{
	TiXmlElement* mapNode = addMainElement (SaveFile.RootElement(), "Map");
	addAttributeElement (mapNode, "Name", "string", Map->MapName);
	addAttributeElement (mapNode, "Resources", "data", convertDataToString (Map->Resources, Map->size * Map->size));
}

//--------------------------------------------------------------------------
void cSavegame::writePlayer (const cPlayer* Player, int number)
{
	// generate players node if it doesn't exists
	TiXmlElement* playersNode;
	if (! (playersNode = SaveFile.RootElement()->FirstChildElement ("Players")))
	{
		playersNode = addMainElement (SaveFile.RootElement(), "Players");
	}

	// add node for the player
	TiXmlElement* playerNode = addMainElement (playersNode, "Player_" + iToStr (number));

	// write the main information
	addAttributeElement (playerNode, "Name", "string", Player->name);
	addAttributeElement (playerNode, "Credits", "num", iToStr (Player->Credits));
	addAttributeElement (playerNode, "Clan", "num", iToStr (Player->getClan()));
	addAttributeElement (playerNode, "Color", "num", iToStr (GetColorNr (Player->color)));
	addAttributeElement (playerNode, "Number", "num", iToStr (Player->Nr));
	addAttributeElement (playerNode, "ResourceMap", "data", convertScanMapToString (Player->ResourceMap, Server->Map->size * Server->Map->size));

	// player score
	TiXmlElement* scoreNode = addMainElement (playerNode, "ScoreHistory");
	for (unsigned int i = 0; i < Player->pointsHistory.size(); i++)
	{
		TiXmlElement* e = addMainElement (scoreNode, "Score");
		e->SetAttribute ("num", iToStr (Player->pointsHistory[i]).c_str());
	}

	// write data of upgraded units
	TiXmlElement* upgradesNode = addMainElement (playerNode, "Upgrades");
	int upgrades = 0;
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
	{
		if (Player->VehicleData[i].version > 0
			|| Player->VehicleData[i].buildCosts != UnitsData.getVehicle (i, Player->getClan()).data.buildCosts)    // if only costs were researched, the version is not incremented
		{
			writeUpgrade (upgradesNode, upgrades, &Player->VehicleData[i], &UnitsData.getVehicle (i, Player->getClan()).data);
			upgrades++;
		}
	}
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
	{
		if (Player->BuildingData[i].version > 0
			|| Player->BuildingData[i].buildCosts != UnitsData.getBuilding (i, Player->getClan()).data.buildCosts)    // if only costs were researched, the version is not incremented
		{
			writeUpgrade (upgradesNode, upgrades, &Player->BuildingData[i], &UnitsData.getBuilding (i, Player->getClan()).data);
			upgrades++;
		}
	}

	TiXmlElement* researchNode = addMainElement (playerNode, "Research");
	researchNode->SetAttribute ("researchCount", iToStr (Player->ResearchCount).c_str());
	TiXmlElement* researchLevelNode = addMainElement (researchNode, "ResearchLevel");
	writeResearchLevel (researchLevelNode, Player->researchLevel);
	TiXmlElement* researchCentersWorkingOnAreaNode = addMainElement (researchNode, "CentersWorkingOnArea");
	writeResearchCentersWorkingOnArea (researchCentersWorkingOnAreaNode, Player);


	// write subbases
	TiXmlElement* subbasesNode = addMainElement (playerNode, "Subbases");
	for (unsigned int i = 0; i < Player->base.SubBases.size(); i++)
	{
		const sSubBase* SubBase = Player->base.SubBases[i];
		TiXmlElement* subbaseNode = addMainElement (subbasesNode, "Subbase_" + iToStr (i));

		//write the ID of the first building, to identify the subbase at load time
		addAttributeElement (subbaseNode, "buildingID", "num", iToStr (SubBase->buildings[0]->iID));
		TiXmlElement* element = addMainElement (subbaseNode, "Production");
		element->SetAttribute ("metal", iToStr (SubBase->getMetalProd()).c_str());
		element->SetAttribute ("oil", iToStr (SubBase->getOilProd()).c_str());
		element->SetAttribute ("gold", iToStr (SubBase->getGoldProd()).c_str());
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeUpgrade (TiXmlElement* upgradesNode, int upgradenumber, const sUnitData* data, const sUnitData* originaldata)
{
	TiXmlElement* upgradeNode = addMainElement (upgradesNode, "Unit_" + iToStr (upgradenumber));
	addAttributeElement (upgradeNode, "Type", "string", data->ID.getText());
	addAttributeElement (upgradeNode, "Version", "num", iToStr (data->version));
	if (data->ammoMax != originaldata->ammoMax) addAttributeElement (upgradeNode, "Ammo", "num", iToStr (data->ammoMax));
	if (data->hitpointsMax != originaldata->hitpointsMax) addAttributeElement (upgradeNode, "HitPoints", "num", iToStr (data->hitpointsMax));
	if (data->shotsMax != originaldata->shotsMax) addAttributeElement (upgradeNode, "Shots", "num", iToStr (data->shotsMax));
	if (data->speedMax != originaldata->speedMax) addAttributeElement (upgradeNode, "Speed", "num", iToStr (data->speedMax));
	if (data->armor != originaldata->armor) addAttributeElement (upgradeNode, "Armor", "num", iToStr (data->armor));
	if (data->buildCosts != originaldata->buildCosts) addAttributeElement (upgradeNode, "Costs", "num", iToStr (data->buildCosts));
	if (data->damage != originaldata->damage) addAttributeElement (upgradeNode, "Damage", "num", iToStr (data->damage));
	if (data->range != originaldata->range) addAttributeElement (upgradeNode, "Range", "num", iToStr (data->range));
	if (data->scan != originaldata->scan) addAttributeElement (upgradeNode, "Scan", "num", iToStr (data->scan));
}

//--------------------------------------------------------------------------
void cSavegame::writeResearchLevel (TiXmlElement* researchLevelNode, const cResearch& researchLevel)
{
	TiXmlElement* levelNode = addMainElement (researchLevelNode, "Level");
	levelNode->SetAttribute ("attack", iToStr (researchLevel.getCurResearchLevel (cResearch::kAttackResearch)).c_str());
	levelNode->SetAttribute ("shots", iToStr (researchLevel.getCurResearchLevel (cResearch::kShotsResearch)).c_str());
	levelNode->SetAttribute ("range", iToStr (researchLevel.getCurResearchLevel (cResearch::kRangeResearch)).c_str());
	levelNode->SetAttribute ("armor", iToStr (researchLevel.getCurResearchLevel (cResearch::kArmorResearch)).c_str());
	levelNode->SetAttribute ("hitpoints", iToStr (researchLevel.getCurResearchLevel (cResearch::kHitpointsResearch)).c_str());
	levelNode->SetAttribute ("speed", iToStr (researchLevel.getCurResearchLevel (cResearch::kSpeedResearch)).c_str());
	levelNode->SetAttribute ("scan", iToStr (researchLevel.getCurResearchLevel (cResearch::kScanResearch)).c_str());
	levelNode->SetAttribute ("cost", iToStr (researchLevel.getCurResearchLevel (cResearch::kCostResearch)).c_str());

	TiXmlElement* curPointsNode = addMainElement (researchLevelNode, "CurPoints");
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
void cSavegame::writeResearchCentersWorkingOnArea (TiXmlElement* researchCentersWorkingOnAreaNode, const cPlayer* player)
{
	researchCentersWorkingOnAreaNode->SetAttribute ("attack", iToStr (player->researchCentersWorkingOnArea[cResearch::kAttackResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("shots", iToStr (player->researchCentersWorkingOnArea[cResearch::kShotsResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("range", iToStr (player->researchCentersWorkingOnArea[cResearch::kRangeResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("armor", iToStr (player->researchCentersWorkingOnArea[cResearch::kArmorResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("hitpoints", iToStr (player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("speed", iToStr (player->researchCentersWorkingOnArea[cResearch::kSpeedResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("scan", iToStr (player->researchCentersWorkingOnArea[cResearch::kScanResearch]).c_str());
	researchCentersWorkingOnAreaNode->SetAttribute ("cost", iToStr (player->researchCentersWorkingOnArea[cResearch::kCostResearch]).c_str());
}

//--------------------------------------------------------------------------
void cSavegame::writeCasualties (const cServer& server)
{
	if (server.getCasualtiesTracker() == 0)
		return;

	TiXmlElement* casualtiesNode = addMainElement (SaveFile.RootElement(), "Casualties");
	server.getCasualtiesTracker()->storeToXML (casualtiesNode);
}

//--------------------------------------------------------------------------
TiXmlElement* cSavegame::writeUnit (const cServer& server, const cVehicle& vehicle, int* unitnum)
{
	// add units node if it doesn't exists
	TiXmlElement* unitsNode;
	if (! (unitsNode = SaveFile.RootElement()->FirstChildElement ("Units")))
	{
		unitsNode = addMainElement (SaveFile.RootElement(), "Units");
		addAttributeElement (unitsNode, "NextUnitID", "num", iToStr (server.iNextUnitID));
	}

	// add the unit node
	TiXmlElement* unitNode = addMainElement (unitsNode, "Unit_" + iToStr (*unitnum));

	// write main information
	addAttributeElement (unitNode, "Type", "string", vehicle.data.ID.getText());
	addAttributeElement (unitNode, "ID", "num", iToStr (vehicle.iID));
	addAttributeElement (unitNode, "Owner", "num", iToStr (vehicle.owner->Nr));
	addAttributeElement (unitNode, "Position", "x", iToStr (vehicle.PosX), "y", iToStr (vehicle.PosY));
	// add information whether the unitname isn't serverdefault, so that it would be readed when loading but is in the save to make him more readable
	addAttributeElement (unitNode, "Name", "string", vehicle.isNameOriginal() ? vehicle.data.name : vehicle.getName(), "notDefault", vehicle.isNameOriginal() ? "0" : "1");

	// write the standard unit values which are the same for vehicles and buildings
	writeUnitValues (unitNode, &vehicle.data, &vehicle.owner->VehicleData[vehicle.typ->nr]);

	// add additional status information
	addAttributeElement (unitNode, "Direction", "num", iToStr (vehicle.dir));
	if (vehicle.data.canCapture || vehicle.data.canDisable) addAttributeElement (unitNode, "CommandoRank", "num", dToStr (vehicle.CommandoRank));
	if (vehicle.data.isBig) addMainElement (unitNode, "IsBig");
	if (vehicle.turnsDisabled > 0) addAttributeElement (unitNode, "Disabled", "turns", iToStr (vehicle.turnsDisabled));
	if (vehicle.LayMines) addMainElement (unitNode, "LayMines");
	if (vehicle.sentryActive) addMainElement (unitNode, "OnSentry");
	if (vehicle.manualFireActive) addMainElement (unitNode, "ManualFire");
	if (vehicle.hasAutoMoveJob) addMainElement (unitNode, "AutoMoving");

	if (vehicle.IsBuilding)
	{
		TiXmlElement* element = addMainElement (unitNode, "Building");
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
	if (vehicle.detectedByPlayerList.size() > 0)
	{
		TiXmlElement* detecedByNode = addMainElement (unitNode, "IsDetectedByPlayers");
		for (unsigned int i = 0; i < vehicle.detectedByPlayerList.size(); i++)
		{
			addAttributeElement (detecedByNode, "Player_" + iToStr (i),
								 "nr", iToStr (vehicle.detectedByPlayerList[i]->Nr),
								 "ThisTurn", vehicle.wasDetectedInThisTurnByPlayer (vehicle.detectedByPlayerList[i]) ? "1" : "0");
		}
	}

	// write all stored vehicles
	for (unsigned int i = 0; i < vehicle.storedUnits.size(); i++)
	{
		(*unitnum) ++;
		TiXmlElement* storedNode = writeUnit (server, *vehicle.storedUnits[i], unitnum);
		addAttributeElement (storedNode, "Stored_In", "id", iToStr (vehicle.iID), "is_vehicle", "1");
	}
	return unitNode;
}

//--------------------------------------------------------------------------
void cSavegame::writeUnit (const cServer& server, const cBuilding& building, int* unitnum)
{
	// add units node if it doesn't exists
	TiXmlElement* unitsNode;
	if (! (unitsNode = SaveFile.RootElement()->FirstChildElement ("Units")))
	{
		unitsNode = addMainElement (SaveFile.RootElement(), "Units");
		addAttributeElement (unitsNode, "NextUnitID", "num", iToStr (server.iNextUnitID));
	}

	// add the unit node
	TiXmlElement* unitNode = addMainElement (unitsNode, "Unit_" + iToStr (*unitnum));

	// write main information
	addAttributeElement (unitNode, "Type", "string", building.data.ID.getText());
	addAttributeElement (unitNode, "ID", "num", iToStr (building.iID));
	addAttributeElement (unitNode, "Owner", "num", iToStr (building.owner->Nr));
	addAttributeElement (unitNode, "Position", "x", iToStr (building.PosX), "y", iToStr (building.PosY));

	// add information whether the unitname isn't serverdefault, so that it would be readed when loading but is in the save to make him more readable
	addAttributeElement (unitNode, "Name", "string", building.isNameOriginal() ? building.data.name : building.getName(), "notDefault", building.isNameOriginal() ? "0" : "1");

	// write the standard values
	writeUnitValues (unitNode, &building.data, &building.owner->BuildingData[building.typ->nr]);

	// write additional stauts information
	if (building.IsWorking) addMainElement (unitNode, "IsWorking");
	if (building.wasWorking) addMainElement (unitNode, "wasWorking");
	if (building.turnsDisabled > 0) addAttributeElement (unitNode, "Disabled", "turns", iToStr (building.turnsDisabled));

	if (building.data.canResearch)
	{
		TiXmlElement* researchNode = addMainElement (unitNode, "ResearchArea");
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
	if (building.BuildList && building.BuildList->size() > 0)
	{
		TiXmlElement* buildNode = addMainElement (unitNode, "Building");
		addAttributeElement (buildNode, "BuildSpeed", "num", iToStr (building.BuildSpeed));
		addAttributeElement (buildNode, "MetalPerRound", "num", iToStr (building.MetalPerRound));
		if (building.RepeatBuild) addMainElement (buildNode, "RepeatBuild");

		TiXmlElement* buildlistNode = addMainElement (buildNode, "BuildList");
		for (unsigned int i = 0; i < building.BuildList->size(); i++)
		{
			addAttributeElement (buildlistNode, "Item_" + iToStr (i), "type_id", (*building.BuildList) [i]->type.getText(), "metall_remaining", iToStr ( (*building.BuildList) [i]->metall_remaining));
		}
	}

	// write from which players this unit has been detected
	if (building.detectedByPlayerList.size() > 0)
	{
		TiXmlElement* detecedByNode = addMainElement (unitNode, "IsDetectedByPlayers");
		for (unsigned int i = 0; i < building.detectedByPlayerList.size(); i++)
		{
			addAttributeElement (detecedByNode, "Player_" + iToStr (i), "nr", iToStr (building.detectedByPlayerList[i]->Nr));
		}
	}

	// write all stored vehicles
	for (unsigned int i = 0; i < building.storedUnits.size(); i++)
	{
		(*unitnum) ++;
		TiXmlElement* storedNode = writeUnit (server, *building.storedUnits[i], unitnum);
		addAttributeElement (storedNode, "Stored_In", "id", iToStr (building.iID), "is_vehicle", "0");
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeRubble (const cServer& server, const cBuilding& building, int rubblenum)
{
	// add units node if it doesn't exists
	TiXmlElement* unitsNode;
	if (! (unitsNode = SaveFile.RootElement()->FirstChildElement ("Units")))
	{
		unitsNode = addMainElement (SaveFile.RootElement(), "Units");
		addAttributeElement (unitsNode, "NextUnitID", "num", iToStr (server.iNextUnitID));
	}

	// add the rubble node
	TiXmlElement* rubbleNode = addMainElement (unitsNode, "Rubble_" + iToStr (rubblenum));

	addAttributeElement (rubbleNode, "Position", "x", iToStr (building.PosX), "y", iToStr (building.PosY));
	addAttributeElement (rubbleNode, "RubbleValue", "num", iToStr (building.RubbleValue));
	if (building.data.isBig) addMainElement (rubbleNode, "Big");
}

//--------------------------------------------------------------------------
void cSavegame::writeUnitValues (TiXmlElement* unitNode, const sUnitData* Data, const sUnitData* OwnerData)
{
	// write the standard status values
	if (Data->hitpointsCur != Data->hitpointsMax) addAttributeElement (unitNode, "Hitpoints", "num", iToStr (Data->hitpointsCur));
	if (Data->ammoCur != Data->ammoMax) addAttributeElement (unitNode, "Ammo", "num", iToStr (Data->ammoCur));

	if (Data->storageResCur > 0) addAttributeElement (unitNode, "ResCargo", "num", iToStr (Data->storageResCur));
	if (Data->storageUnitsCur > 0) addAttributeElement (unitNode, "UnitCargo", "num", iToStr (Data->storageUnitsCur));

	if (Data->speedCur != Data->speedMax) addAttributeElement (unitNode, "Speed", "num", iToStr (Data->speedCur));
	if (Data->shotsCur != Data->shotsMax) addAttributeElement (unitNode, "Shots", "num", iToStr (Data->shotsCur));

	// write upgrade values that differ from the acctual unit values of the owner
	if (OwnerData->version > 0)
	{
		addAttributeElement (unitNode, "Version", "num", iToStr (Data->version));
		if (Data->hitpointsMax != OwnerData->hitpointsMax) addAttributeElement (unitNode, "Max_Hitpoints", "num", iToStr (Data->hitpointsMax));
		if (Data->ammoMax != OwnerData->ammoMax) addAttributeElement (unitNode, "Max_Ammo", "num", iToStr (Data->ammoMax));
		if (Data->speedMax != OwnerData->speedMax) addAttributeElement (unitNode, "Max_Speed", "num", iToStr (Data->speedMax));
		if (Data->shotsMax != OwnerData->shotsMax) addAttributeElement (unitNode, "Max_Shots", "num", iToStr (Data->shotsMax));

		if (Data->armor != OwnerData->armor) addAttributeElement (unitNode, "Armor", "num", iToStr (Data->armor));
		if (Data->damage != OwnerData->damage) addAttributeElement (unitNode, "Damage", "num", iToStr (Data->damage));
		if (Data->range != OwnerData->range) addAttributeElement (unitNode, "Range", "num", iToStr (Data->range));
		if (Data->scan != OwnerData->scan) addAttributeElement (unitNode, "Scan", "num", iToStr (Data->scan));
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeStandardUnitValues (const sUnitData* Data, int unitnum)
{
	// add the main node if it doesn't exists
	TiXmlElement* unitValuesNode;
	if (! (unitValuesNode = SaveFile.RootElement()->FirstChildElement ("UnitValues")))
	{
		unitValuesNode = addMainElement (SaveFile.RootElement(), "UnitValues");
	}
	// add the unit node
	TiXmlElement* unitNode = addMainElement (unitValuesNode, "UnitVal_" + iToStr (unitnum));
	addAttributeElement (unitNode, "ID", "string", Data->ID.getText());
	addAttributeElement (unitNode, "Name", "string", Data->name);

	addAttributeElement (unitNode, "Hitpoints", "num", iToStr (Data->hitpointsMax));
	addAttributeElement (unitNode, "Armor", "num", iToStr (Data->armor));
	addAttributeElement (unitNode, "Built_Costs", "num", iToStr (Data->buildCosts));

	if (Data->scan > 0) addAttributeElement (unitNode, "Scan", "num", iToStr (Data->scan));

	if (Data->speedMax > 0) addAttributeElement (unitNode, "Movement", "num", iToStr (Data->speedMax / 4));

	if (Data->muzzleType > 0) addAttributeElement (unitNode, "MuzzleType", "num", iToStr (Data->muzzleType));
	if (Data->shotsMax > 0) addAttributeElement (unitNode, "Shots", "num", iToStr (Data->shotsMax));
	if (Data->ammoMax > 0) addAttributeElement (unitNode, "Ammo", "num", iToStr (Data->ammoMax));
	if (Data->range > 0) addAttributeElement (unitNode, "Range", "num", iToStr (Data->range));
	if (Data->damage > 0) addAttributeElement (unitNode, "Damage", "num", iToStr (Data->damage));
	if (Data->canAttack != TERRAIN_NONE) addAttributeElement (unitNode, "Can_Attack", "num", iToStr (Data->canAttack));

	if (!Data->canBuild.empty()) addAttributeElement (unitNode, "Can_Build", "string", Data->canBuild);
	if (!Data->buildAs.empty()) addAttributeElement (unitNode, "Build_As", "string", Data->buildAs);
	if (Data->maxBuildFactor > 0) addAttributeElement (unitNode, "Max_Build_Factor", "num", iToStr (Data->maxBuildFactor));

	if (Data->storageResMax > 0) addAttributeElement (unitNode, "Storage_Res_Max", "num", iToStr (Data->storageResMax));
	if (Data->storageUnitsMax > 0) addAttributeElement (unitNode, "Storage_Units_Max", "num", iToStr (Data->storageUnitsMax));
	if (Data->storeResType != sUnitData::STORE_RES_NONE) addAttributeElement (unitNode, "Store_Res_Type", "num", iToStr (Data->storeResType));
	if (Data->storeUnitsImageType != sUnitData::STORE_UNIT_IMG_NONE) addAttributeElement (unitNode, "StoreUnits_Image_Type", "num", iToStr (Data->storeUnitsImageType));
	if (!Data->isStorageType.empty()) addAttributeElement (unitNode, "Is_Storage_Type", "string", Data->isStorageType);
	if (!Data->storeUnitsTypes.empty())
	{
		string storeUnitsTypes = Data->storeUnitsTypes[0];
		for (unsigned int i = 1; i < Data->storeUnitsTypes.size(); i++)
		{
			storeUnitsTypes += "+" + Data->storeUnitsTypes[i];
		}
		addAttributeElement (unitNode, "StoreUnitsTypes", "string", storeUnitsTypes);
	}

	if (Data->needsEnergy != 0) addAttributeElement (unitNode, "Needs_Energy", "num", iToStr (Data->needsEnergy));
	if (Data->produceEnergy != 0) addAttributeElement (unitNode, "Needs_Energy", "num", iToStr (-Data->produceEnergy));
	if (Data->needsHumans != 0) addAttributeElement (unitNode, "Needs_Humans", "num", iToStr (Data->needsHumans));
	if (Data->produceHumans != 0) addAttributeElement (unitNode, "Needs_Humans", "num", iToStr (-Data->produceHumans));
	if (Data->needsOil != 0) addAttributeElement (unitNode, "Needs_Oil", "num", iToStr (Data->needsOil));
	if (Data->needsMetal != 0) addAttributeElement (unitNode, "Needs_Metal", "num", iToStr (Data->needsMetal));
	if (Data->convertsGold != 0) addAttributeElement (unitNode, "Converts_Gold", "num", iToStr (Data->convertsGold));
	if (Data->canMineMaxRes != 0) addAttributeElement (unitNode, "Can_Mine_Max_Res", "num", iToStr (Data->canMineMaxRes));

	if (Data->isStealthOn != TERRAIN_NONE) addAttributeElement (unitNode, "Is_Stealth_On", "num", iToStr (Data->isStealthOn));
	if (Data->canDetectStealthOn != TERRAIN_NONE) addAttributeElement (unitNode, "Can_Detect_Stealth_On", "num", iToStr (Data->canDetectStealthOn));

	if (Data->surfacePosition != sUnitData::SURFACE_POS_GROUND) addAttributeElement (unitNode, "Surface_Position", "num", iToStr (Data->surfacePosition));
	if (Data->canBeOverbuild != sUnitData::OVERBUILD_TYPE_NO) addAttributeElement (unitNode, "Can_Be_Overbuild", "num", iToStr (Data->canBeOverbuild));

	if (Data->factorAir != 0.0) addAttributeElement (unitNode, "Factor_Air", "num", dToStr (Data->factorAir));
	if (Data->factorCoast != 0.0) addAttributeElement (unitNode, "Factor_Coast", "num", dToStr (Data->factorCoast));
	if (Data->factorGround != 0.0) addAttributeElement (unitNode, "Factor_Ground", "num", dToStr (Data->factorGround));
	if (Data->factorSea != 0.0) addAttributeElement (unitNode, "Factor_Sea", "num", dToStr (Data->factorSea));

	if (Data->modifiesSpeed != 0.0) addAttributeElement (unitNode, "Factor_Sea", "num", dToStr (Data->modifiesSpeed));

	if (Data->canBuildPath) addMainElement (unitNode, "Can_Build_Path");
	if (Data->canBuildRepeat) addMainElement (unitNode, "Can_Build_Repeat");
	if (Data->buildIntern) addMainElement (unitNode, "Build_Intern");
	if (Data->connectsToBase) addMainElement (unitNode, "Connects_To_Base");

	if (Data->canBeCaptured) addMainElement (unitNode, "Can_Be_Captured");
	if (Data->canBeDisabled) addMainElement (unitNode, "Can_Be_Disabled");
	if (Data->canCapture) addMainElement (unitNode, "Can_Capture");
	if (Data->canDisable) addMainElement (unitNode, "Can_Disable");
	if (Data->canSurvey) addMainElement (unitNode, "Can_Survey");
	if (Data->doesSelfRepair) addMainElement (unitNode, "Does_Self_Repair");
	if (Data->canSelfDestroy) addMainElement (unitNode, "Can_Self_Destroy");
	if (Data->explodesOnContact) addMainElement (unitNode, "Explodes_On_Contact");
	if (Data->isHuman) addMainElement (unitNode, "Is_Human");
	if (Data->canBeLandedOn) addMainElement (unitNode, "Can_Be_Landed_On");
	if (Data->canWork) addMainElement (unitNode, "Can_Work");

	if (Data->isBig) addMainElement (unitNode, "Is_Big");
	if (Data->canRepair) addMainElement (unitNode, "Can_Repair");
	if (Data->canRearm) addMainElement (unitNode, "Can_Rearm");
	if (Data->canResearch) addMainElement (unitNode, "Can_Research");
	if (Data->canClearArea) addMainElement (unitNode, "Can_Clear");
	if (Data->canPlaceMines) addMainElement (unitNode, "Can_Place_Mines");
	if (Data->canDriveAndFire) addMainElement (unitNode, "Can_Drive_And_Fire");
}

//--------------------------------------------------------------------------
void cSavegame::writeAdditionalInfo (sHudStateContainer hudState, std::vector<sSavedReportMessage>& list, const cPlayer* player)
{
	if (!SaveFile.LoadFile ( (cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml").c_str())) return;
	if (!SaveFile.RootElement()) return;

	// first get the players node
	TiXmlElement* playersNode = SaveFile.RootElement()->FirstChildElement ("Players");
	int playernum = 0;
	TiXmlElement* playerNode = NULL;
	do
	{
		playerNode = playersNode->FirstChildElement ( ("Player_" + iToStr (playernum)).c_str());
		if (!playerNode) return;
		int number;
		playerNode->FirstChildElement ("Number")->Attribute ("num", &number);
		if (number == player->Nr) break;
		playernum++;
	}
	while (playerNode != NULL);

	// write the hud settings
	TiXmlElement* hudNode = addMainElement (playerNode, "Hud");
	addAttributeElement (hudNode, "SelectedUnit", "num", iToStr (hudState.selUnitID));
	addAttributeElement (hudNode, "Offset", "x", iToStr (hudState.offX), "y", iToStr (hudState.offY));
	addAttributeElement (hudNode, "Zoom", "num", dToStr (hudState.zoom));
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
	TiXmlElement* reportsNode = addMainElement (playerNode, "Reports");

	for (size_t i = 0; i != list.size(); ++i)
	{
		TiXmlElement* reportElement = addMainElement (reportsNode, "Report");
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
void cSavegame::addAttributeElement (TiXmlElement* node, const string& nodename, const string& attributename, const string& value, const string& attributename2, const string& value2)
{
	TiXmlElement* element = addMainElement (node, nodename);
	element->SetAttribute (attributename.c_str(), value.c_str());
	if (attributename2.compare ("")) element->SetAttribute (attributename2.c_str(), value2.c_str());
}

//--------------------------------------------------------------------------
TiXmlElement* cSavegame::addMainElement (TiXmlElement* node, const string& nodename)
{
	TiXmlElement* element = new TiXmlElement (nodename.c_str());
	node->LinkEndChild (element);
	return element;
}
