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
#include "client.h"
#include "menu.h"
#include "loaddata.h"
#include "upgradecalculator.h"

//--------------------------------------------------------------------------
cSavegame::cSavegame ( int number )
{
	this->number = number;
	if ( number > 100 ) return;
	sprintf ( numberstr, "%0.3d", number );
}

//--------------------------------------------------------------------------
int cSavegame::save( string saveName )
{
	SaveFile = new TiXmlDocument;

	TiXmlElement *rootnode = new TiXmlElement ( "MAXR_SAVE_FILE" );
	rootnode->SetAttribute ( "version", SAVE_FORMAT_VERSION );
	SaveFile->LinkEndChild ( rootnode );

	writeHeader( saveName );
	writeGameInfo ();
	writeMap( Server->Map );

	int unitnum = 0;
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		cPlayer *Player = (*Server->PlayerList)[i];
		writePlayer( Player, i );
		cVehicle *Vehicle = Player->VehicleList;
		while ( Vehicle )
		{
			if ( !Vehicle->Loaded )
			{
				writeUnit ( Vehicle, &unitnum );
				unitnum++;
			}
			Vehicle = Vehicle->next;
		}
		cBuilding *Building = Player->BuildingList;
		while ( Building )
		{
			writeUnit ( Building, &unitnum );
			unitnum++;
			Building = Building->next;
		}
	}
	int rubblenum = 0;
	cBuilding *Rubble = Server->neutralBuildings;
	while ( Rubble )
	{
		writeRubble ( Rubble, rubblenum );
		rubblenum++;
		Rubble = Rubble->next;
	}

	for ( unsigned int i = 0; i < UnitsData.vehicle.Size()+UnitsData.building.Size(); i++ )
	{
		sUnitData *Data;
		if ( i < UnitsData.vehicle.Size() ) Data = &UnitsData.vehicle[i].data;
		else Data = &UnitsData.building[i-UnitsData.vehicle.Size()].data;
		writeStandardUnitValues ( Data, i );
	}

	SaveFile->SaveFile( ( SettingsData.sSavesPath + PATH_DELIMITER + "Save" + numberstr + ".xml" ).c_str() );

	delete SaveFile;
	return 1;
}

//--------------------------------------------------------------------------
int cSavegame::load()
{
	SaveFile = new TiXmlDocument ();
	if ( !SaveFile->LoadFile ( ( SettingsData.sSavesPath + PATH_DELIMITER + "Save" + numberstr + ".xml" ).c_str() ) )
	{
		return 0;
	}
	if ( !SaveFile->RootElement() ) return 0;

	if ( ((string)SaveFile->RootElement()->Attribute ( "version" )).compare ( SAVE_FORMAT_VERSION ) )
	{
		Log.write ( "Savefile-version differs from the one supported by the game!", cLog::eLOG_TYPE_WARNING );
	}

	// load standard unit values
	TiXmlElement *unitValuesNode = SaveFile->RootElement()->FirstChildElement( "UnitValues" );
	if ( unitValuesNode != NULL )
	{
		int unitnum = 0;
		TiXmlElement *unitNode = unitValuesNode->FirstChildElement( "UnitVal_0" );
		while ( unitNode )
		{
			loadStandardUnitValues ( unitNode );

			unitnum++;
			unitNode = unitValuesNode->FirstChildElement( ("UnitVal_" + iToStr ( unitnum )).c_str() );
		}
	}

	cMap *map = loadMap();
	if ( !map ) return 0;
	cList<cPlayer *> *PlayerList = loadPlayers ( map );

	string gametype;
	loadHeader ( NULL, &gametype, NULL );
	if ( !gametype.compare ( "IND" )  ) Server = new cServer ( map, PlayerList, GAME_TYPE_SINGLE, false );
	else if ( !gametype.compare ( "HOT" )  ) Server = new cServer ( map, PlayerList, GAME_TYPE_HOTSEAT, false );
	else if ( !gametype.compare ( "NET" )  ) Server = new cServer ( map, PlayerList, GAME_TYPE_TCPIP, false );
	else
	{
		Log.write ( "Unknown gametype \"" + gametype + "\". Starting as singleplayergame.", cLog::eLOG_TYPE_INFO );
		Server = new cServer ( map, PlayerList, GAME_TYPE_SINGLE, false );
	}

	loadGameInfo();
	loadUnits ();

	delete SaveFile;
	return 1;
}

//--------------------------------------------------------------------------
void cSavegame::loadHeader( string *name, string *type, string *time )
{
	SaveFile = new TiXmlDocument ();
	if ( !SaveFile->LoadFile ( ( SettingsData.sSavesPath + PATH_DELIMITER + "Save" + numberstr + ".xml" ).c_str() ) ) return;
	if ( !SaveFile->RootElement() ) return;

	TiXmlElement *headerNode = SaveFile->RootElement()->FirstChildElement( "Header" );

	if ( name ) *name = headerNode->FirstChildElement( "Name" )->Attribute ( "string" );
	if ( type ) *type = headerNode->FirstChildElement( "Type" )->Attribute ( "string" );
	if ( time ) *time = headerNode->FirstChildElement( "Time" )->Attribute ( "string" );
}

//--------------------------------------------------------------------------
string cSavegame::getMapName()
{
	TiXmlElement *mapNode = SaveFile->RootElement()->FirstChildElement( "Map" );
	if ( mapNode != NULL ) return mapNode->FirstChildElement( "Name" )->Attribute ( "string" );
	else return "";
}

//--------------------------------------------------------------------------
string cSavegame::getPlayerNames()
{
	string playernames = "";
	TiXmlElement *playersNode = SaveFile->RootElement()->FirstChildElement( "Players" );
	if ( playersNode != NULL )
	{
		int playernum = 0;
		TiXmlElement *playerNode = playersNode->FirstChildElement( "Player_0" );
		while ( playerNode )
		{
			playernames += ((string)playerNode->FirstChildElement ( "Name" )->Attribute ( "string" )) + "\n";
			playernum++;
			playerNode = playersNode->FirstChildElement( ("Player_" + iToStr ( playernum )).c_str() );
		}
	}
	return playernames;
}

//--------------------------------------------------------------------------
void cSavegame::loadGameInfo()
{
	TiXmlElement *gameInfoNode = SaveFile->RootElement()->FirstChildElement( "Game" );
	if ( !gameInfoNode ) return;

	TiXmlElement *element;
	gameInfoNode->FirstChildElement( "Turn" )->Attribute ( "num", &Server->iTurn );
	if ( element = gameInfoNode->FirstChildElement( "Hotseat" ) )
	{
		Server->bHotSeat = true;
		element->Attribute ( "activeplayer", &Server->iHotSeatPlayer );
	}
	if ( element = gameInfoNode->FirstChildElement( "PlayTurns" ) )
	{
		Server->bPlayTurns = true;
		element->Attribute ( "activeplayer", &Server->iActiveTurnPlayerNr );
	}
}

//--------------------------------------------------------------------------
cMap *cSavegame::loadMap()
{
	TiXmlElement *mapNode = SaveFile->RootElement()->FirstChildElement( "Map" );
	if ( mapNode != NULL )
	{
		cMap *map = new cMap;
		string name = mapNode->FirstChildElement( "Name" )->Attribute ( "string" );
		string resourcestr = mapNode->FirstChildElement( "Resources" )->Attribute ( "data" );
		if ( !map->LoadMap ( name ) )
		{
			delete map;
			return NULL;
		}
		convertStringToData ( resourcestr, map->size*map->size, map->Resources );
		return map;
	}
	else return NULL;
}

//--------------------------------------------------------------------------
cList<cPlayer*> * cSavegame::loadPlayers( cMap *map )
{
	cList<cPlayer *> *PlayerList = new cList<cPlayer *>;

	TiXmlElement *playersNode = SaveFile->RootElement()->FirstChildElement( "Players" );
	if ( playersNode != NULL )
	{
		int playernum = 0;
		TiXmlElement *playerNode = playersNode->FirstChildElement( "Player_0" );
		while ( playerNode )
		{
			PlayerList->Add ( loadPlayer ( playerNode, map ) );
			playernum++;
			playerNode = playersNode->FirstChildElement( ("Player_" + iToStr ( playernum )).c_str() );
		}
	}
	else
	{
		// warning
	}
	return PlayerList;
}

//--------------------------------------------------------------------------
cPlayer *cSavegame::loadPlayer( TiXmlElement *playerNode, cMap *map )
{
	int number, color;

	string name = playerNode->FirstChildElement( "Name" )->Attribute ( "string" );
	playerNode->FirstChildElement( "Number" )->Attribute ( "num", &number );
	playerNode->FirstChildElement( "Color" )->Attribute ( "num", &color );

	cPlayer *Player = new cPlayer ( name, OtherData.colors[color], number );
	Player->InitMaps ( map->size, map );

	playerNode->FirstChildElement( "Credits" )->Attribute ( "num", &Player->Credits );
	string resourceMap = playerNode->FirstChildElement( "ResourceMap" )->Attribute ( "data" );
	convertStringToScanMap ( resourceMap, Player->ResourceMap );

	TiXmlElement *hudNode = playerNode->FirstChildElement( "Hud" );
	if ( hudNode )
	{
		// save the loaded hudoptions to the "HotHud" of the player so that the server can send them later to the clients
		hudNode->FirstChildElement( "Offset" )->Attribute ( "x", &Player->HotHud.OffX );
		hudNode->FirstChildElement( "Offset" )->Attribute ( "y", &Player->HotHud.OffY );
		hudNode->FirstChildElement( "Zoom" )->Attribute ( "num", &Player->HotHud.Zoom );
		if ( hudNode->FirstChildElement( "Colors" ) ) Player->HotHud.Farben = true;
		if ( hudNode->FirstChildElement( "Grid" ) ) Player->HotHud.Gitter = true;
		if ( hudNode->FirstChildElement( "Ammo" ) ) Player->HotHud.Munition = true;
		if ( hudNode->FirstChildElement( "Fog" ) ) Player->HotHud.Nebel = true;
		if ( hudNode->FirstChildElement( "Range" ) ) Player->HotHud.Reichweite = true;
		if ( hudNode->FirstChildElement( "Scan" ) ) Player->HotHud.Scan = true;
		if ( hudNode->FirstChildElement( "Status" ) ) Player->HotHud.Status = true;
		if ( hudNode->FirstChildElement( "Survey" ) ) Player->HotHud.Studie = true;
		if ( hudNode->FirstChildElement( "Hitpoints" ) ) Player->HotHud.Treffer = true;
		if ( hudNode->FirstChildElement( "MinimapZoom" ) ) Player->HotHud.MinimapZoom = true;
		if ( hudNode->FirstChildElement( "TNT" ) ) Player->HotHud.TNT = true;
		if ( hudNode->FirstChildElement( "Lock" ) ) Player->HotHud.Lock = true;
		if ( hudNode->FirstChildElement( "SelectedVehicle" ) ) hudNode->FirstChildElement( "SelectedVehicle" )->Attribute ( "num", &Player->HotHud.tmpSelectedUnitID );
		else if ( hudNode->FirstChildElement( "SelectedBuilding" ) ) hudNode->FirstChildElement( "SelectedBuilding" )->Attribute ( "num", &Player->HotHud.tmpSelectedUnitID );
	}

	TiXmlElement *upgradesNode = playerNode->FirstChildElement( "Upgrades" );
	if ( upgradesNode )
	{
		int upgradenum = 0;
		TiXmlElement *upgradeNode = upgradesNode->FirstChildElement( "Unit_0" );
		while ( upgradeNode )
		{
			sID ID;
			ID.generate ( upgradeNode->FirstChildElement ( "Type" )->Attribute( "string" ) );
			if ( ID.iFirstPart == 0 )
			{
				unsigned int num;
				for ( num = 0; num < UnitsData.vehicle.Size(); num++ ) if ( UnitsData.vehicle[num].data.ID == ID ) break;
				loadUpgrade ( upgradeNode, &Player->VehicleData[num] );
			}
			else
			{
				unsigned int num;
				for ( num = 0; num < UnitsData.building.Size(); num++ ) if ( UnitsData.building[num].data.ID == ID ) break;
				loadUpgrade ( upgradeNode, &Player->BuildingData[num] );
			}
			upgradenum++;
			upgradeNode = upgradesNode->FirstChildElement( ("Unit_" + iToStr ( upgradenum )).c_str() );
		}
	}

	TiXmlElement *researchNode = playerNode->FirstChildElement( "Research" );
	if (researchNode)
	{
		researchNode->Attribute ( "researchCount", &(Player->ResearchCount) );
		TiXmlElement *researchLevelNode = researchNode->FirstChildElement( "ResearchLevel" );
		if ( researchLevelNode )
			loadResearchLevel( researchLevelNode, Player->researchLevel );
		TiXmlElement *researchCentersWorkingOnAreaNode = researchNode->FirstChildElement( "CentersWorkingOnArea" );
		if ( researchCentersWorkingOnAreaNode )
			loadResearchCentersWorkingOnArea( researchCentersWorkingOnAreaNode, Player );
	}

	TiXmlElement *subbasesNode;
	if ( subbasesNode = playerNode->FirstChildElement( "Subbases" ) )
	{
		int subbasenum = 0;
		TiXmlElement *subbaseNode = subbasesNode->FirstChildElement( "Subbase_0" );
		while ( subbaseNode )
		{
			sSubBaseLoad *subBaseLoad = new sSubBaseLoad;
			subBaseLoad->Player = Player;
			subbaseNode->FirstChildElement( "ID" )->Attribute ( "num", &subBaseLoad->SubBase.iID );
			subbaseNode->FirstChildElement( "Resources" )->Attribute ( "metal", &subBaseLoad->SubBase.Metal );
			subbaseNode->FirstChildElement( "Resources" )->Attribute ( "oil", &subBaseLoad->SubBase.Oil );
			subbaseNode->FirstChildElement( "Resources" )->Attribute ( "gold", &subBaseLoad->SubBase.Gold );
			subbaseNode->FirstChildElement( "Production" )->Attribute ( "metal", &subBaseLoad->SubBase.MetalProd );
			subbaseNode->FirstChildElement( "Production" )->Attribute ( "oil", &subBaseLoad->SubBase.OilProd );
			subbaseNode->FirstChildElement( "Production" )->Attribute ( "gold", &subBaseLoad->SubBase.GoldProd );
			subbaseNode->FirstChildElement( "Production" )->Attribute ( "energy", &subBaseLoad->SubBase.EnergyProd );
			subbaseNode->FirstChildElement( "Production" )->Attribute ( "human", &subBaseLoad->SubBase.HumanProd );
			subbaseNode->FirstChildElement( "Need" )->Attribute ( "metal", &subBaseLoad->SubBase.MetalNeed );
			subbaseNode->FirstChildElement( "Need" )->Attribute ( "oil", &subBaseLoad->SubBase.OilNeed );
			subbaseNode->FirstChildElement( "Need" )->Attribute ( "gold", &subBaseLoad->SubBase.GoldNeed );
			subbaseNode->FirstChildElement( "Need" )->Attribute ( "energy", &subBaseLoad->SubBase.EnergyNeed );
			subbaseNode->FirstChildElement( "Need" )->Attribute ( "human", &subBaseLoad->SubBase.HumanNeed );
			SubBasesLoad.Add ( subBaseLoad );

			subbasenum++;
			subbaseNode = subbasesNode->FirstChildElement( ("Subbase_" + iToStr ( subbasenum )).c_str() );
		}
	}
	return Player;
}

//--------------------------------------------------------------------------
void cSavegame::loadUpgrade ( TiXmlElement *upgradeNode, sUnitData *data )
{
	upgradeNode->FirstChildElement( "Version" )->Attribute ( "num", &data->version );
	TiXmlElement *element;
	if ( element = upgradeNode->FirstChildElement( "Ammo" ) ) element->Attribute ( "num", &data->max_ammo );
	if ( element = upgradeNode->FirstChildElement( "HitPoints" ) ) element->Attribute ( "num", &data->max_hit_points );
	if ( element = upgradeNode->FirstChildElement( "Shots" ) ) element->Attribute ( "num", &data->max_shots );
	if ( element = upgradeNode->FirstChildElement( "Speed" ) ) element->Attribute ( "num", &data->max_speed );
	if ( element = upgradeNode->FirstChildElement( "Armor" ) ) element->Attribute ( "num", &data->armor );
	if ( element = upgradeNode->FirstChildElement( "Costs" ) ) element->Attribute ( "num", &data->iBuilt_Costs );
	if ( element = upgradeNode->FirstChildElement( "Damage" ) ) element->Attribute ( "num", &data->damage );
	if ( element = upgradeNode->FirstChildElement( "Range" ) ) element->Attribute ( "num", &data->range );
	if ( element = upgradeNode->FirstChildElement( "Scan" ) ) element->Attribute ( "num", &data->scan );
}

//--------------------------------------------------------------------------
void cSavegame::loadResearchLevel ( TiXmlElement *researchLevelNode, cResearch& researchLevel )
{
	int value;
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "attack", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kAttackResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "shots", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kShotsResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "range", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kRangeResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "armor", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kArmorResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "hitpoints", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kHitpointsResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "speed", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kSpeedResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "scan", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kScanResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "cost", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kCostResearch );
	
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "attack", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kAttackResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "shots", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kShotsResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "range", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kRangeResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "armor", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kArmorResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "hitpoints", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kHitpointsResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "speed", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kSpeedResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "scan", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kScanResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "cost", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kCostResearch );
}

//--------------------------------------------------------------------------
void cSavegame::loadResearchCentersWorkingOnArea( TiXmlElement *researchCentersWorkingOnAreaNode, cPlayer *player )
{
	int value;
	researchCentersWorkingOnAreaNode->Attribute ( "attack", &value );
	player->researchCentersWorkingOnArea[cResearch::kAttackResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "shots", &value );
	player->researchCentersWorkingOnArea[cResearch::kShotsResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "range", &value );
	player->researchCentersWorkingOnArea[cResearch::kRangeResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "armor", &value );
	player->researchCentersWorkingOnArea[cResearch::kArmorResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "hitpoints", &value );
	player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "speed", &value );
	player->researchCentersWorkingOnArea[cResearch::kSpeedResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "scan", &value );
	player->researchCentersWorkingOnArea[cResearch::kScanResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "cost", &value );
	player->researchCentersWorkingOnArea[cResearch::kCostResearch] = value;	
}

//--------------------------------------------------------------------------
void cSavegame::loadUnits ()
{
	if ( !Server ) return;

	TiXmlElement *unitsNode = SaveFile->RootElement()->FirstChildElement( "Units" );
	if ( unitsNode != NULL )
	{
		int unitnum = 0;
		TiXmlElement *unitNode = unitsNode->FirstChildElement( "Unit_0" );
		while ( unitNode )
		{
			sID ID;
			ID.generate ( unitNode->FirstChildElement( "Type" )->Attribute ( "string" ) );
			if ( ID.iFirstPart == 0 ) loadVehicle ( unitNode, ID );
			else if ( ID.iFirstPart == 1 ) loadBuilding ( unitNode, ID );

			unitnum++;
			unitNode = unitsNode->FirstChildElement( ("Unit_" + iToStr ( unitnum )).c_str() );
		}
		// read nextid-value before loading rubble, so that the rubble will get new ids.
		int nextID;
		unitsNode->FirstChildElement( "NextUnitID" )->Attribute ( "num", &nextID );
		Server->iNextUnitID = nextID;

		int rubblenum = 0;
		TiXmlElement *rubbleNode = unitsNode->FirstChildElement( "Rubble_0" );
		while ( rubbleNode )
		{
			loadRubble ( rubbleNode );
			rubblenum++;
			rubbleNode = unitsNode->FirstChildElement( ("Rubble_" + iToStr ( rubblenum )).c_str() );
		}
		generateMoveJobs();
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadVehicle( TiXmlElement *unitNode, sID &ID )
{
	if ( !Server ) return;
	int tmpinteger, number, x, y;
	for ( unsigned int i = 0; i < UnitsData.vehicle.Size(); i++ )
	{
		if ( UnitsData.vehicle[i].data.ID == ID )
		{
			number = i;
			break;
		}
		if ( i == UnitsData.vehicle.Size()-1 ) return;
	}
	unitNode->FirstChildElement( "Owner" )->Attribute ( "num", &tmpinteger );
	cPlayer *owner = getPlayerFromNumber ( Server->PlayerList, tmpinteger );

	unitNode->FirstChildElement( "Position" )->Attribute ( "x", &x );
	unitNode->FirstChildElement( "Position" )->Attribute ( "y", &y );
	cVehicle *vehicle = Server->addUnit ( x, y, &UnitsData.vehicle[number], owner, true, unitNode->FirstChildElement( "Stored_In" )==NULL );

	unitNode->FirstChildElement( "ID" )->Attribute ( "num", &tmpinteger );
	vehicle->iID = tmpinteger;
	if ( unitNode->FirstChildElement( "Name" )->Attribute ( "notDefault" ) && strcmp ( unitNode->FirstChildElement( "Name" )->Attribute ( "notDefault" ), "1" ) ) 
		vehicle->name = unitNode->FirstChildElement( "Name" )->Attribute ( "string" );

	loadUnitValues ( unitNode, &vehicle->data );

	TiXmlElement *element;
	unitNode->FirstChildElement( "Direction" )->Attribute ( "num", &vehicle->dir );
	double tmpdouble;
	if ( element = unitNode->FirstChildElement( "CommandoRank" ) ) { element->Attribute ( "num", &tmpdouble ); vehicle->CommandoRank = (float)tmpdouble; }
	if ( unitNode->FirstChildElement( "IsBig" ) ) Server->Map->moveVehicleBig( vehicle, x, y );
	if ( unitNode->FirstChildElement( "Disabled" ) ) vehicle->Disabled = true;
	if ( unitNode->FirstChildElement( "LayMines" ) ) vehicle->LayMines = true;
	if ( unitNode->FirstChildElement( "AutoMoving" ) ) vehicle->hasAutoMoveJob = true;
	if ( unitNode->FirstChildElement( "OnSentry" ) )
	{
		vehicle->bSentryStatus = true;
		owner->addSentryVehicle ( vehicle );
	}

	if ( element = unitNode->FirstChildElement( "Building" ) )
	{
		vehicle->IsBuilding = true;
		if ( element->Attribute ( "type_id" ) != NULL )
		{
			vehicle->BuildingTyp.generate ( element->Attribute ( "type_id" ) );
		}
		// be downward compatible and looke for 'type' too 
		else if ( element->Attribute ( "type" ) != NULL )
		{
			//element->Attribute ( "type", &vehicle->BuildingTyp );
		}
		element->Attribute ( "turns", &vehicle->BuildRounds );
		element->Attribute ( "costs", &vehicle->BuildCosts );
		element->Attribute ( "savedpos", &vehicle->BuildBigSavedPos );

		if ( element->Attribute ( "path") )
		{
			vehicle->BuildPath = true;
			element->Attribute ( "turnsstart", &vehicle->BuildRoundsStart );
			element->Attribute ( "costsstart", &vehicle->BuildCostsStart );
			element->Attribute ( "endx", &vehicle->BandX );
			element->Attribute ( "endy", &vehicle->BandY );
		}
	}
	if ( element = unitNode->FirstChildElement( "Clearing" ) )
	{
		vehicle->IsClearing = true;
		element->Attribute ( "turns", &vehicle->ClearingRounds );
		element->Attribute ( "savedpos", &vehicle->BuildBigSavedPos );
	}

	if ( element = unitNode->FirstChildElement( "Movejob" ) )
	{
		sMoveJobLoad *MoveJob = new sMoveJobLoad;
		MoveJob->vehicle = vehicle;
		element->Attribute ( "destx", &MoveJob->destX );
		element->Attribute ( "desty", &MoveJob->destY );

		MoveJobsLoad.Add ( MoveJob );
	}

	// read the players which have detected this unit
	TiXmlElement *detectedNode;
	if ( detectedNode = unitNode->FirstChildElement( "IsDetectedByPlayers" ) )
	{
		int playerNodeNum = 0;
		while ( element = detectedNode->FirstChildElement ( ("Player_" + iToStr( playerNodeNum )).c_str() ) )
		{
			int playerNum;
			element->Attribute ( "nr", &playerNum );
			cPlayer *Player = Server->getPlayerFromNumber ( playerNum );
			if ( Player )
			{
				vehicle->setDetectedByPlayer ( Player );
			}
			playerNodeNum++;
		}
	}

	// since we write all stored vehicles imediatly after the storing unit we can be sure that this one has been loaded yet
	if ( element = unitNode->FirstChildElement( "Stored_In" ) )
	{
		int storedInID;
		int isVehicle;
		element->Attribute ( "id", &storedInID );
		element->Attribute ( "is_vehicle", &isVehicle );
		if ( isVehicle )
		{
			cVehicle *StoringVehicle = Server->getVehicleFromID ( storedInID );
			if ( !StoringVehicle ) return;

			StoringVehicle->data.cargo--;
			StoringVehicle->storeVehicle ( vehicle, Server->Map );
		}
		else
		{
			cBuilding *StoringBuilding = Server->getBuildingFromID ( storedInID );
			if ( !StoringBuilding ) return;

			StoringBuilding->data.cargo--;
			StoringBuilding->storeVehicle ( vehicle, Server->Map );
		}
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadBuilding( TiXmlElement *unitNode, sID &ID )
{
	if ( !Server ) return;
	int tmpinteger, number, x, y;
	for ( unsigned int i = 0; i < UnitsData.building.Size(); i++ )
	{
		if ( UnitsData.building[i].data.ID == ID )
		{
			number = i;
			break;
		}
		if ( i == UnitsData.building.Size()-1 ) return;
	}
	unitNode->FirstChildElement( "Owner" )->Attribute ( "num", &tmpinteger );
	cPlayer *owner = getPlayerFromNumber ( Server->PlayerList, tmpinteger );

	unitNode->FirstChildElement( "Position" )->Attribute ( "x", &x );
	unitNode->FirstChildElement( "Position" )->Attribute ( "y", &y );
	cBuilding *building = Server->addUnit ( x, y, &UnitsData.building[number], owner, true );

	unitNode->FirstChildElement( "ID" )->Attribute ( "num", &tmpinteger );
	building->iID = tmpinteger;
	if ( unitNode->FirstChildElement( "Name" )->Attribute ( "notDefault" ) && strcmp ( unitNode->FirstChildElement( "Name" )->Attribute ( "notDefault" ), "1" ) ) 
		building->name = unitNode->FirstChildElement( "Name" )->Attribute ( "string" );

	TiXmlElement *element;
	int subbaseID;
	if ( element = unitNode->FirstChildElement( "SubBase" ) )
	{
		element->Attribute ( "num", &subbaseID );
		for ( unsigned int i = 0; i < SubBasesLoad.Size(); i++ )
		{
			if ( SubBasesLoad[i]->Player != owner ) continue;
			if ( SubBasesLoad[i]->SubBase.iID != subbaseID ) continue;
			building->SubBase->Metal = SubBasesLoad[i]->SubBase.Metal;
			building->SubBase->Oil = SubBasesLoad[i]->SubBase.Oil;
			building->SubBase->Gold = SubBasesLoad[i]->SubBase.Gold;
			building->SubBase->MetalProd = SubBasesLoad[i]->SubBase.MetalProd;
			building->SubBase->OilProd = SubBasesLoad[i]->SubBase.OilProd;
			building->SubBase->GoldProd = SubBasesLoad[i]->SubBase.GoldProd;
			building->SubBase->EnergyProd = SubBasesLoad[i]->SubBase.EnergyProd;
			building->SubBase->HumanProd = SubBasesLoad[i]->SubBase.HumanProd;
			building->SubBase->MetalNeed = SubBasesLoad[i]->SubBase.MetalNeed;
			building->SubBase->OilNeed = SubBasesLoad[i]->SubBase.OilNeed;
			building->SubBase->GoldNeed = SubBasesLoad[i]->SubBase.GoldNeed;
			building->SubBase->EnergyNeed = SubBasesLoad[i]->SubBase.EnergyNeed;
			building->SubBase->HumanNeed = SubBasesLoad[i]->SubBase.HumanNeed;
			break;
		}
	}

	loadUnitValues ( unitNode, &building->data );

	if ( unitNode->FirstChildElement( "IsWorking" ) ) building->IsWorking = true;
	if ( unitNode->FirstChildElement( "ResearchArea" ) ) unitNode->FirstChildElement( "ResearchArea" )->Attribute( "area", &(building->researchArea) );
	if ( unitNode->FirstChildElement( "OnSentry" ) )
	{
		if ( !building->bSentryStatus )
		{
			building->bSentryStatus = true;
			owner->addSentryBuilding ( building );
		}
	}
	else if ( building->bSentryStatus )
	{
		owner->deleteSentryBuilding ( building );
		building->bSentryStatus = false;
	}
	if ( unitNode->FirstChildElement( "HasBeenAttacked" ) ) building->hasBeenAttacked = true;

	if ( element = unitNode->FirstChildElement( "MetalProd" ) ) element->Attribute ( "num", &building->MetalProd );
	if ( element = unitNode->FirstChildElement( "GoldProd" ) ) element->Attribute ( "num", &building->GoldProd );
	if ( element = unitNode->FirstChildElement( "OilProd" ) ) element->Attribute ( "num", &building->OilProd );

	if ( element = unitNode->FirstChildElement( "Building" ) )
	{
		TiXmlElement *buildNode = element;
		if ( element = buildNode->FirstChildElement( "BuildSpeed" ) ) element->Attribute ( "num", &building->BuildSpeed );
		if ( element = buildNode->FirstChildElement( "MetalPerRound" ) ) element->Attribute ( "num", &building->MetalPerRound );
		if ( buildNode->FirstChildElement( "RepeatBuild" ) ) building->RepeatBuild = true;

		int itemnum = 0;
		element = buildNode->FirstChildElement( "BuildList" )->FirstChildElement( "Item_0" );
		while ( element )
		{
			sBuildList *listitem = new sBuildList;
			if ( element->Attribute ( "type_id" ) != NULL )
			{
				sID BuildID;
				BuildID.generate ( element->Attribute ( "type_id" ) );
				for ( unsigned int i = 0; i < UnitsData.vehicle.Size(); i++ )
				{
					if ( BuildID == UnitsData.vehicle[i].data.ID )
					{
						listitem->typ = &UnitsData.vehicle[i];
						break;
					}
				}
			}
			// be downward compatible and looke for 'type' too 
			else if ( element->Attribute ( "type" ) != NULL )
			{
				int typenr;
				element->Attribute ( "type", &typenr );
				listitem->typ = &UnitsData.vehicle[typenr];
			}
			element->Attribute ( "metall_remaining", &listitem->metall_remaining );
			building->BuildList->Add ( listitem );

			itemnum++;
			element = buildNode->FirstChildElement( "BuildList" )->FirstChildElement( ("Item_" + iToStr ( itemnum )).c_str() );
		}
	}

	// read the players which have detected this unit
	TiXmlElement *detectedNode;
	if ( detectedNode = unitNode->FirstChildElement( "IsDetectedByPlayers" ) )
	{
		int playerNodeNum = 0;
		while ( element = detectedNode->FirstChildElement ( ("Player_" + iToStr( playerNodeNum )).c_str() ) )
		{
			int playerNum;
			element->Attribute ( "nr", &playerNum );
			cPlayer *Player = Server->getPlayerFromNumber ( playerNum );
			if ( Player )
			{
				building->setDetectedByPlayer ( Player );
			}
			playerNodeNum++;
		}
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadRubble( TiXmlElement *rubbleNode )
{
	int x, y, rubblevalue;
	bool big = false;

	rubbleNode->FirstChildElement ( "Position" )->Attribute ( "x", &x );
	rubbleNode->FirstChildElement ( "Position" )->Attribute ( "y", &y );
	rubbleNode->FirstChildElement ( "RubbleValue" )->Attribute ( "num", &rubblevalue );

	if ( rubbleNode->FirstChildElement ( "Big" ) ) big = true;

	Server->addRubble ( x+y*Server->Map->size, rubblevalue, big );
}

//--------------------------------------------------------------------------
void cSavegame::loadUnitValues ( TiXmlElement *unitNode, sUnitData *Data )
{
	TiXmlElement *Element;
	if ( Element = unitNode->FirstChildElement( "Version" ) ) Element->Attribute ( "num", &Data->version );

	if ( Element = unitNode->FirstChildElement( "Max_Hitpoints" ) ) Element->Attribute ( "num", &Data->max_hit_points );
	if ( Element = unitNode->FirstChildElement( "Max_Ammo" ) ) Element->Attribute ( "num", &Data->max_ammo );
	if ( Element = unitNode->FirstChildElement( "Max_Speed" ) ) Element->Attribute ( "num", &Data->max_speed );
	if ( Element = unitNode->FirstChildElement( "Max_Shots" ) ) Element->Attribute ( "num", &Data->max_shots );
	if ( Element = unitNode->FirstChildElement( "Armor" ) ) Element->Attribute ( "num", &Data->armor );
	if ( Element = unitNode->FirstChildElement( "Damage" ) ) Element->Attribute ( "num", &Data->damage );
	if ( Element = unitNode->FirstChildElement( "Range" ) ) Element->Attribute ( "num", &Data->range );
	if ( Element = unitNode->FirstChildElement( "Scan" ) ) Element->Attribute ( "num", &Data->scan );

	if ( Element = unitNode->FirstChildElement( "Hitpoints" ) ) Element->Attribute ( "num", &Data->hit_points );
	else Data->hit_points = Data->max_hit_points;
	if ( Element = unitNode->FirstChildElement( "Ammo" ) ) Element->Attribute ( "num", &Data->ammo );
	else Data->ammo = Data->max_ammo;
	if ( Element = unitNode->FirstChildElement( "Cargo" ) ) Element->Attribute ( "num", &Data->cargo );
	if ( Element = unitNode->FirstChildElement( "Speed" ) ) Element->Attribute ( "num", &Data->speed );
	else Data->speed = Data->max_speed;
	if ( Element = unitNode->FirstChildElement( "Shots" ) ) Element->Attribute ( "num", &Data->shots );
	else Data->shots = Data->max_shots;
}

//--------------------------------------------------------------------------
void cSavegame::loadStandardUnitValues ( TiXmlElement *unitNode )
{
	if ( unitNode == NULL ) return;
	TiXmlElement *Element;
	sUnitData *Data = NULL;
	int unitNum;
	bool isVehicle;


	// get the unit data
	sID ID;
	ID.generate ( unitNode->FirstChildElement( "ID" )->Attribute ( "string" ) );
	if ( ID.iFirstPart == 0 )
	{
		for ( unsigned int i = 0; i < UnitsData.vehicle.Size(); i++ )
		{
			if ( UnitsData.vehicle[i].data.ID == ID )
			{
				Data = &UnitsData.vehicle[i].data;
				unitNum = i;
				isVehicle = true;
				break;
			}
		}
	}
	else if ( ID.iFirstPart == 1 )
	{
		for ( unsigned int i = 0; i < UnitsData.building.Size(); i++ )
		{
			if ( UnitsData.building[i].data.ID == ID )
			{
				Data = &UnitsData.building[i].data;
				unitNum = i;
				isVehicle = false;
				break;
			}
		}
	}
	else return;
	if ( Data == NULL ) return;
	SetDefaultUnitData ( Data );

	Data->ID = ID;
	Data->szName = unitNode->FirstChildElement( "Name" )->Attribute ( "string" );

	unitNode->FirstChildElement( "Hitpoints" )->Attribute ( "num", &Data->iHitpoints_Max );
	unitNode->FirstChildElement( "Armor" )->Attribute ( "num", &Data->iArmor );
	unitNode->FirstChildElement( "Built_Costs" )->Attribute ( "num", &Data->iBuilt_Costs );
	unitNode->FirstChildElement( "Built_Costs_Max" )->Attribute ( "num", &Data->iBuilt_Costs_Max );
	if ( Element = unitNode->FirstChildElement( "Shield" ) ) Element->Attribute ( "num", &Data->iEnergy_Shield_Strength_Max );

	if ( Element = unitNode->FirstChildElement( "Scan_Range_Sight" ) ) Element->Attribute ( "num", &Data->iScan_Range_Sight );
	if ( Element = unitNode->FirstChildElement( "Scan_Range_Mine" ) ) Element->Attribute ( "num", &Data->iScan_Range_Mine );
	if ( Element = unitNode->FirstChildElement( "Scan_Range_Resource" ) ) Element->Attribute ( "num", &Data->iScan_Range_Resources );
	if ( Element = unitNode->FirstChildElement( "Scan_Range_Submarine" ) ) Element->Attribute ( "num", &Data->iScan_Range_Submarine );
	if ( Element = unitNode->FirstChildElement( "Scan_Range_Infantry" ) ) Element->Attribute ( "num", &Data->iScan_Range_Infantry );

	if ( Element = unitNode->FirstChildElement( "Movement" ) ) Element->Attribute ( "num", &Data->iMovement_Max );
	Data->iMovement_Max *= 4;
	if ( Element = unitNode->FirstChildElement( "Makes_Tracks" ) ) Element->Attribute ( "num", &Data->iMakes_Tracks );

	if ( Element = unitNode->FirstChildElement( "Movement_Allowed" ) ) Element->Attribute ( "num", &Data->Weapons[0].iMovement_Allowed );
	if ( Element = unitNode->FirstChildElement( "MuzzleType" ) ) Element->Attribute ( "num", &Data->Weapons[0].iMuzzleType );
	if ( Element = unitNode->FirstChildElement( "Shots" ) ) Element->Attribute ( "num", &Data->Weapons[0].iShots_Max );
	if ( Element = unitNode->FirstChildElement( "Ammo" ) ) Element->Attribute ( "num", &Data->Weapons[0].iAmmo_Quantity_Max );

	if ( Element = unitNode->FirstChildElement( "Air_Range" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Air_Range );
	if ( Element = unitNode->FirstChildElement( "Infantry_Range" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Infantry_Range );
	if ( Element = unitNode->FirstChildElement( "Land_Range" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Land_Range );
	if ( Element = unitNode->FirstChildElement( "Mine_Range" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Mine_Range );
	if ( Element = unitNode->FirstChildElement( "Sea_Range" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Sea_Range );
	if ( Element = unitNode->FirstChildElement( "Submarine_Range" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Submarine_Range );

	if ( Element = unitNode->FirstChildElement( "Air_Damage" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Air_Damage );
	if ( Element = unitNode->FirstChildElement( "Infantry_Damage" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Infantry_Damage );
	if ( Element = unitNode->FirstChildElement( "Land_Damage" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Land_Damage );
	if ( Element = unitNode->FirstChildElement( "Mine_Damage" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Mine_Damage );
	if ( Element = unitNode->FirstChildElement( "Sea_Damage" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Sea_Damage );
	if ( Element = unitNode->FirstChildElement( "Submarine_Damage" ) ) Element->Attribute ( "num", &Data->Weapons[0].iTarget_Submarine_Damage );

	if ( Element = unitNode->FirstChildElement( "Capacity_Metal" ) ) Element->Attribute ( "num", &Data->iCapacity_Metal_Max );
	if ( Element = unitNode->FirstChildElement( "Capacity_Oil" ) ) Element->Attribute ( "num", &Data->iCapacity_Oil_Max );
	if ( Element = unitNode->FirstChildElement( "Capacity_Gold" ) ) Element->Attribute ( "num", &Data->iCapacity_Gold_Max );
	if ( Element = unitNode->FirstChildElement( "Capacity_Units_Air" ) ) Element->Attribute ( "num", &Data->iCapacity_Units_Air_Max );
	if ( Element = unitNode->FirstChildElement( "Capacity_Units_Sea" ) ) Element->Attribute ( "num", &Data->iCapacity_Units_Sea_Max );
	if ( Element = unitNode->FirstChildElement( "Capacity_Units_Ground" ) ) Element->Attribute ( "num", &Data->iCapacity_Units_Ground_Max );
	if ( Element = unitNode->FirstChildElement( "Capacity_Units_Infantry" ) ) Element->Attribute ( "num", &Data->iCapacity_Units_Infantry_Max );

	if ( Element = unitNode->FirstChildElement( "Needs_Energy" ) ) Element->Attribute ( "num", &Data->iNeeds_Energy );
	if ( Element = unitNode->FirstChildElement( "Needs_Humans" ) ) Element->Attribute ( "num", &Data->iNeeds_Humans );
	if ( Element = unitNode->FirstChildElement( "Needs_Oil" ) ) Element->Attribute ( "num", &Data->iNeeds_Oil );
	if ( Element = unitNode->FirstChildElement( "Needs_Metal" ) ) Element->Attribute ( "num", &Data->iNeeds_Metal );
	if ( Element = unitNode->FirstChildElement( "Converts_Gold" ) ) Element->Attribute ( "num", &Data->iConverts_Gold );

	double tmpdouble;
	if ( Element = unitNode->FirstChildElement( "Costs_Ground" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fCosts_Ground = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Costs_Sea" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fCosts_Sea = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Costs_Air" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fCosts_Air = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Costs_Submarine" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fCosts_Submarine = (float)tmpdouble; }

	if ( Element = unitNode->FirstChildElement( "Factor_Coast" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Coast = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Wood" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Wood = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Road" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Road = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Bridge" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Bridge = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Platform" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Platform = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Monorail" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Monorail = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Wreck" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Wreck = (float)tmpdouble; }
	if ( Element = unitNode->FirstChildElement( "Factor_Mountains" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->fFactor_Mountains = (float)tmpdouble; }

	if ( Element = unitNode->FirstChildElement( "Size_Length" ) ) Element->Attribute ( "num", &Data->iSize_Length );
	if ( Element = unitNode->FirstChildElement( "Size_Width" ) ) Element->Attribute ( "num", &Data->iSize_Width );

	if ( unitNode->FirstChildElement( "Can_Repair" ) ) Data->bCan_Repair = true;
	if ( unitNode->FirstChildElement( "Can_Rearm" ) ) Data->bCan_Rearm = true;
	if ( unitNode->FirstChildElement( "Can_Research" ) ) Data->bCan_Research = true;
	if ( unitNode->FirstChildElement( "Can_Clear" ) ) Data->bCan_Clear_Area = true;
	if ( unitNode->FirstChildElement( "Can_Place_Mines" ) ) Data->bCan_Place_Mines = true;
	if ( unitNode->FirstChildElement( "Has_Animation_Movement" ) ) Data->bAnimation_Movement = true;
	if ( unitNode->FirstChildElement( "Has_Power_On_Grafic" ) ) Data->bPower_On_Grafic = true;
	if ( unitNode->FirstChildElement( "Has_Overlay" ) ) Data->bHas_Overlay = true;

	translateUnitData ( Data->ID, ID.iFirstPart == 0 );
	ConvertData ( unitNum, isVehicle );
}

//--------------------------------------------------------------------------
void cSavegame::generateMoveJobs ()
{
	for ( unsigned int i = 0; i < MoveJobsLoad.Size(); i++ )
	{
		cServerMoveJob *MoveJob = new cServerMoveJob( MoveJobsLoad[i]->vehicle->PosX+MoveJobsLoad[i]->vehicle->PosY*Server->Map->size, MoveJobsLoad[i]->destX+MoveJobsLoad[i]->destY*Server->Map->size, MoveJobsLoad[i]->vehicle->data.can_drive == DRIVE_AIR, MoveJobsLoad[i]->vehicle );
		if ( !MoveJob->calcPath() )
		{
			delete MoveJob;
			MoveJobsLoad[i]->vehicle->ServerMoveJob = NULL;
		}
		else Server->addActiveMoveJob ( MoveJob );
		delete MoveJobsLoad[i];
	}
}

//--------------------------------------------------------------------------
cPlayer *cSavegame::getPlayerFromNumber ( cList<cPlayer*> *PlayerList, int number )
{
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		if ( (*PlayerList)[i]->Nr == number ) return (*PlayerList)[i];
	}
	return NULL;
}

//--------------------------------------------------------------------------
string cSavegame::convertDataToString ( sResources *resources, int size )
{
	string str = "";
	for ( int i = 0; i < size; i++ )
	{
		str += getHexValue ( resources[i].typ );
		str += getHexValue ( resources[i].value );
	}
	return str;
}

//--------------------------------------------------------------------------
string cSavegame::getHexValue ( unsigned char byte )
{
	string str = "";
	unsigned char temp = byte/16;
	if ( temp < 10 ) str += ('0' + temp);
	else str += ( 'A' + temp - 10 );

	temp = byte%16;
	if ( temp < 10 ) str += ('0' + temp);
	else str += ( 'A' + temp - 10 );

	return str;
}

//--------------------------------------------------------------------------
void cSavegame::convertStringToData ( string str, int size, sResources *resources )
{
	for ( int i = 0; i < size; i++ )
	{
		resources[i].typ = getByteValue ( str.substr( i*4, 2 ).c_str() );
		resources[i].value = getByteValue ( str.substr( i*4+2, 2 ).c_str() );
	}
}

//--------------------------------------------------------------------------
unsigned char cSavegame::getByteValue ( string str )
{
	unsigned char first = str.substr ( 0, 1 ).c_str()[0] - '0';
	unsigned char second = str.substr ( 1, 1 ).c_str()[0] - '0';

	if ( first >= 'A'-'0' )
		first -= 'A'-'0'-10;
	if ( second >= 'A'-'0' )
		second -= 'A'-'0'-10;
	return ( first*16+second );
}

//--------------------------------------------------------------------------
string cSavegame::convertScanMapToString ( char *data, int size )
{
	string str = "";
	for ( int i = 0; i < size; i++ )
	{
		if  ( data[i] > 0 ) str += "1";
		else str += "0";
	}
	return str;
}

//--------------------------------------------------------------------------
void cSavegame::convertStringToScanMap ( string str, char *data )
{
	for ( unsigned int i = 0; i < str.length(); i++ )
	{
		if ( !str.substr ( i, 1 ).compare ( "1" ) ) data[i] = 1;
		else data[i] = 0;
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeHeader( string saveName )
{
	TiXmlElement *headerNode = addMainElement ( SaveFile->RootElement(), "Header" );

	addAttributeElement ( headerNode, "Game_Version", "string", PACKAGE_VERSION );
	addAttributeElement ( headerNode, "Name", "string", saveName );
	switch ( Server->iGameType )
	{
	case GAME_TYPE_SINGLE:
		addAttributeElement ( headerNode, "Type", "string", "IND" );
		break;
	case GAME_TYPE_HOTSEAT:
		addAttributeElement ( headerNode, "Type", "string", "HOT" );
		break;
	case GAME_TYPE_TCPIP:
		addAttributeElement ( headerNode, "Type", "string", "NET" );
		break;
	}
	time_t tTime;
	tm *tmTime;
	char timestr[21];
	tTime = time ( NULL );
	tmTime = localtime ( &tTime );
	strftime( timestr, 21, "%d.%m.%y %H:%M", tmTime );

	addAttributeElement ( headerNode, "Time", "string", timestr );
}

//--------------------------------------------------------------------------
void cSavegame::writeGameInfo()
{
	TiXmlElement *gemeinfoNode = addMainElement ( SaveFile->RootElement(), "Game" );

	addAttributeElement ( gemeinfoNode, "Turn", "num", iToStr ( Server->iTurn ) );
	if ( Server->bHotSeat ) addAttributeElement ( gemeinfoNode, "Hotseat", "activeplayer", iToStr ( Server->iHotSeatPlayer ) );
	if ( Server->bPlayTurns ) addAttributeElement ( gemeinfoNode, "PlayTurns", "activeplayer", iToStr ( Server->iActiveTurnPlayerNr ) );
}

//--------------------------------------------------------------------------
void cSavegame::writeMap( cMap *Map )
{
	TiXmlElement *mapNode = addMainElement ( SaveFile->RootElement(), "Map" );
	addAttributeElement ( mapNode, "Name", "string", Map->MapName );
	addAttributeElement ( mapNode, "Resources", "data", convertDataToString ( Map->Resources, Map->size*Map->size ) );
}

//--------------------------------------------------------------------------
void cSavegame::writePlayer( cPlayer *Player, int number )
{
	// generate players node if it doesn't exists
	TiXmlElement *playersNode;
	if ( !(playersNode = SaveFile->RootElement()->FirstChildElement ( "Players" )) )
	{
		playersNode = addMainElement ( SaveFile->RootElement(), "Players" );
	}

	// add node for the player
	TiXmlElement *playerNode = addMainElement ( playersNode, "Player_" + iToStr ( number ) );

	// write the main information
	addAttributeElement ( playerNode, "Name", "string", Player->name );
	addAttributeElement ( playerNode, "Credits", "num", iToStr ( Player->Credits ) );
	addAttributeElement ( playerNode, "Color", "num", iToStr ( GetColorNr ( Player->color ) ) );
	addAttributeElement ( playerNode, "Number", "num", iToStr ( Player->Nr ) );
	addAttributeElement ( playerNode, "ResourceMap", "data", convertScanMapToString ( Player->ResourceMap, Server->Map->size*Server->Map->size ) );

	// write the hud settings
	if ( Client && Client->ActivePlayer->Nr == Player->Nr )
	{
		// TODO: save hudoptions of non-local players
		TiXmlElement *hudNode = addMainElement ( playerNode, "Hud" );
		addAttributeElement ( hudNode, "Offset", "x", iToStr ( Client->Hud.OffX ), "y", iToStr ( Client->Hud.OffY ) );
		addAttributeElement ( hudNode, "Zoom", "num", iToStr ( Client->Hud.Zoom ) );
		if ( Client->Hud.Farben ) addMainElement ( hudNode, "Colors" );
		if ( Client->Hud.Gitter ) addMainElement ( hudNode, "Grid" );
		if ( Client->Hud.Munition ) addMainElement ( hudNode, "Ammo" );
		if ( Client->Hud.Nebel ) addMainElement ( hudNode, "Fog" );
		if ( Client->Hud.Reichweite ) addMainElement ( hudNode, "Range" );
		if ( Client->Hud.Scan ) addMainElement ( hudNode, "Scan" );
		if ( Client->Hud.Status ) addMainElement ( hudNode, "Status" );
		if ( Client->Hud.Studie ) addMainElement ( hudNode, "Survey" );
		if ( Client->Hud.Treffer ) addMainElement ( hudNode, "Hitpoints" );
		if ( Client->Hud.MinimapZoom ) addMainElement ( hudNode, "MinimapZoom" );
		if ( Client->Hud.TNT ) addMainElement ( hudNode, "TNT" );
		if ( Client->Hud.Lock ) addMainElement ( hudNode, "Colors" );
		if ( Client->SelectedVehicle ) addAttributeElement ( hudNode, "SelectedVehicle", "num", iToStr ( Client->SelectedVehicle->iID ) );
		else if ( Client->SelectedBuilding ) addAttributeElement ( hudNode, "SelectedBuilding", "num", iToStr ( Client->SelectedBuilding->iID ) );
	}

	// write data of upgraded units
	TiXmlElement *upgradesNode = addMainElement ( playerNode, "Upgrades" );
	int upgrades = 0;
	for ( unsigned int i = 0; i < UnitsData.vehicle.Size(); i++ )
	{
		if ( Player->VehicleData[i].version > 1 )
		{
			writeUpgrade ( upgradesNode, upgrades, &Player->VehicleData[i], &UnitsData.vehicle[i].data );
			upgrades++;
		}
	}
	for ( unsigned int i = 0; i < UnitsData.building.Size(); i++ )
	{
		if ( Player->BuildingData[i].version > 1 )
		{
			writeUpgrade ( upgradesNode, upgrades, &Player->BuildingData[i], &UnitsData.building[i].data );
			upgrades++;
		}
	}

	TiXmlElement *researchNode = addMainElement( playerNode, "Research" );
	researchNode->SetAttribute( "researchCount", iToStr( Player->ResearchCount ).c_str() );
	TiXmlElement *researchLevelNode = addMainElement ( researchNode, "ResearchLevel" );
	writeResearchLevel( researchLevelNode, Player->researchLevel );
	TiXmlElement *researchCentersWorkingOnAreaNode = addMainElement ( researchNode, "CentersWorkingOnArea" );
	writeResearchCentersWorkingOnArea( researchCentersWorkingOnAreaNode, Player );
	

	// write subbases
	TiXmlElement *subbasesNode = addMainElement ( playerNode, "Subbases" );
	for ( unsigned int i = 0; i < Player->base.SubBases.Size(); i++ )
	{
		sSubBase *SubBase = Player->base.SubBases[i];
		TiXmlElement *subbaseNode = addMainElement ( subbasesNode, "Subbase_" + iToStr ( i ) );

		addAttributeElement ( subbaseNode, "ID", "num", iToStr ( SubBase->iID ) );
		TiXmlElement *element = addMainElement ( subbaseNode, "Resources" );
		element->SetAttribute ( "metal", iToStr ( SubBase->Metal ).c_str() );
		element->SetAttribute ( "oil", iToStr ( SubBase->Oil ).c_str() );
		element->SetAttribute ( "gold", iToStr ( SubBase->Gold ).c_str() );
		element = addMainElement ( subbaseNode, "Production" );
		element->SetAttribute ( "metal", iToStr ( SubBase->MetalProd ).c_str() );
		element->SetAttribute ( "oil", iToStr ( SubBase->OilProd ).c_str() );
		element->SetAttribute ( "gold", iToStr ( SubBase->GoldProd ).c_str() );
		element->SetAttribute ( "energy", iToStr ( SubBase->EnergyProd ).c_str() );
		element->SetAttribute ( "human", iToStr ( SubBase->HumanProd ).c_str() );
		element = addMainElement ( subbaseNode, "Need" );
		element->SetAttribute ( "metal", iToStr ( SubBase->MetalNeed ).c_str() );
		element->SetAttribute ( "oil", iToStr ( SubBase->OilNeed ).c_str() );
		element->SetAttribute ( "gold", iToStr ( SubBase->GoldNeed ).c_str() );
		element->SetAttribute ( "energy", iToStr ( SubBase->EnergyNeed ).c_str() );
		element->SetAttribute ( "human", iToStr ( SubBase->HumanNeed ).c_str() );
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeUpgrade ( TiXmlElement *upgradesNode, int upgradenumber, sUnitData *data, sUnitData *originaldata )
{
	TiXmlElement *upgradeNode = addMainElement ( upgradesNode, "Unit_" + iToStr ( upgradenumber ) );
	addAttributeElement ( upgradeNode, "Type", "string", data->ID.getText() );
	addAttributeElement ( upgradeNode, "Version", "num", iToStr ( data->version ) );
	if ( data->max_ammo != originaldata->max_ammo ) addAttributeElement ( upgradeNode, "Ammo", "num", iToStr ( data->max_ammo ) );
	if ( data->max_hit_points != originaldata->max_hit_points ) addAttributeElement ( upgradeNode, "HitPoints", "num", iToStr ( data->max_hit_points ) );
	if ( data->max_shots != originaldata->max_shots ) addAttributeElement ( upgradeNode, "Shots", "num", iToStr ( data->max_shots ) );
	if ( data->max_speed != originaldata->max_speed ) addAttributeElement ( upgradeNode, "Speed", "num", iToStr ( data->max_speed ) );
	if ( data->armor != originaldata->armor ) addAttributeElement ( upgradeNode, "Armor", "num", iToStr ( data->armor ) );
	if ( data->iBuilt_Costs != originaldata->iBuilt_Costs ) addAttributeElement ( upgradeNode, "Costs", "num", iToStr ( data->iBuilt_Costs ) );
	if ( data->damage != originaldata->damage ) addAttributeElement ( upgradeNode, "Damage", "num", iToStr ( data->damage ) );
	if ( data->range != originaldata->range ) addAttributeElement ( upgradeNode, "Range", "num", iToStr ( data->range ) );
	if ( data->scan != originaldata->scan ) addAttributeElement ( upgradeNode, "Scan", "num", iToStr ( data->scan ) );
}

//--------------------------------------------------------------------------
void cSavegame::writeResearchLevel( TiXmlElement *researchLevelNode, cResearch& researchLevel )
{
	TiXmlElement *levelNode = addMainElement ( researchLevelNode, "Level" );
	levelNode->SetAttribute( "attack", iToStr (researchLevel.getCurResearchLevel (cResearch::kAttackResearch)).c_str() );
	levelNode->SetAttribute( "shots", iToStr (researchLevel.getCurResearchLevel (cResearch::kShotsResearch)).c_str() );
	levelNode->SetAttribute( "range", iToStr (researchLevel.getCurResearchLevel (cResearch::kRangeResearch)).c_str() );
	levelNode->SetAttribute( "armor", iToStr (researchLevel.getCurResearchLevel (cResearch::kArmorResearch)).c_str() );
	levelNode->SetAttribute( "hitpoints", iToStr (researchLevel.getCurResearchLevel (cResearch::kHitpointsResearch)).c_str() );
	levelNode->SetAttribute( "speed", iToStr (researchLevel.getCurResearchLevel (cResearch::kSpeedResearch)).c_str() );
	levelNode->SetAttribute( "scan", iToStr (researchLevel.getCurResearchLevel (cResearch::kScanResearch)).c_str() );
	levelNode->SetAttribute( "cost", iToStr (researchLevel.getCurResearchLevel (cResearch::kCostResearch)).c_str() );

	TiXmlElement *curPointsNode = addMainElement ( researchLevelNode, "CurPoints" );
	curPointsNode->SetAttribute( "attack", iToStr (researchLevel.getCurResearchPoints (cResearch::kAttackResearch)).c_str() );
	curPointsNode->SetAttribute( "shots", iToStr (researchLevel.getCurResearchPoints (cResearch::kShotsResearch)).c_str() );
	curPointsNode->SetAttribute( "range", iToStr (researchLevel.getCurResearchPoints (cResearch::kRangeResearch)).c_str() );
	curPointsNode->SetAttribute( "armor", iToStr (researchLevel.getCurResearchPoints (cResearch::kArmorResearch)).c_str() );
	curPointsNode->SetAttribute( "hitpoints", iToStr (researchLevel.getCurResearchPoints (cResearch::kHitpointsResearch)).c_str() );
	curPointsNode->SetAttribute( "speed", iToStr (researchLevel.getCurResearchPoints (cResearch::kSpeedResearch)).c_str() );
	curPointsNode->SetAttribute( "scan", iToStr (researchLevel.getCurResearchPoints (cResearch::kScanResearch)).c_str() );
	curPointsNode->SetAttribute( "cost", iToStr (researchLevel.getCurResearchPoints (cResearch::kCostResearch)).c_str() );	
}

//--------------------------------------------------------------------------
void cSavegame::writeResearchCentersWorkingOnArea (TiXmlElement *researchCentersWorkingOnAreaNode, cPlayer *player)
{
	researchCentersWorkingOnAreaNode->SetAttribute( "attack", iToStr (player->researchCentersWorkingOnArea[cResearch::kAttackResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "shots", iToStr (player->researchCentersWorkingOnArea[cResearch::kShotsResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "range", iToStr (player->researchCentersWorkingOnArea[cResearch::kRangeResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "armor", iToStr (player->researchCentersWorkingOnArea[cResearch::kArmorResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "hitpoints", iToStr (player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "speed", iToStr (player->researchCentersWorkingOnArea[cResearch::kSpeedResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "scan", iToStr (player->researchCentersWorkingOnArea[cResearch::kScanResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "cost", iToStr (player->researchCentersWorkingOnArea[cResearch::kCostResearch]).c_str() );		
}

//--------------------------------------------------------------------------
TiXmlElement *cSavegame::writeUnit ( cVehicle *Vehicle, int *unitnum )
{
	// add units node if it doesn't exists
	TiXmlElement *unitsNode;
	if ( !(unitsNode = SaveFile->RootElement()->FirstChildElement ( "Units" )) )
	{
		unitsNode = addMainElement ( SaveFile->RootElement(), "Units" );
		addAttributeElement ( unitsNode, "NextUnitID", "num", iToStr ( Server->iNextUnitID ) );
	}

	// add the unit node
	TiXmlElement *unitNode = addMainElement ( unitsNode, "Unit_" + iToStr( *unitnum ) );

	// write main information
	addAttributeElement ( unitNode, "Type", "string", Vehicle->data.ID.getText() );
	addAttributeElement ( unitNode, "ID", "num", iToStr ( Vehicle->iID ) );
	addAttributeElement ( unitNode, "Owner", "num", iToStr ( Vehicle->owner->Nr ) );
	addAttributeElement ( unitNode, "Position", "x", iToStr ( Vehicle->PosX ), "y", iToStr ( Vehicle->PosY ) );
	// add information whether the unitname isn't serverdefault, so that it would be readed when loading but is in the save to make him more readable
	string wasName = Vehicle->name;
	Vehicle->GenerateName();
	addAttributeElement ( unitNode, "Name", "string", Vehicle->name, "notDefault", ( !Vehicle->name.compare ( wasName ) ) ? "0" : "1" );
	Vehicle->name = wasName;

	// write the standard unit values which are the same for vehicles and buildings
	writeUnitValues ( unitNode, &Vehicle->data, &Vehicle->owner->VehicleData[Vehicle->typ->nr] );

	// add additional status information
	addAttributeElement ( unitNode, "Direction", "num", iToStr ( Vehicle->dir ).c_str() );
	if ( Vehicle->data.is_commando ) addAttributeElement ( unitNode, "CommandoRank", "num", dToStr ( Vehicle->CommandoRank ).c_str() );
	if ( Vehicle->data.is_big ) addMainElement ( unitNode, "IsBig" );
	if ( Vehicle->Disabled ) addMainElement ( unitNode, "Disabled" );
	if ( Vehicle->LayMines ) addMainElement ( unitNode, "LayMines" );
	if ( Vehicle->bSentryStatus ) addMainElement ( unitNode, "OnSentry" );
	if ( Vehicle->hasAutoMoveJob ) addMainElement ( unitNode, "AutoMoving" );

	if ( Vehicle->IsBuilding )
	{
		TiXmlElement *element = addMainElement ( unitNode, "Building" );
		element->SetAttribute ( "type_id", Vehicle->BuildingTyp.getText().c_str() );
		element->SetAttribute ( "turns", iToStr ( Vehicle->BuildRounds ).c_str() );
		element->SetAttribute ( "costs", iToStr ( Vehicle->BuildCosts ).c_str() );
		if ( Vehicle->data.is_big ) element->SetAttribute ( "savedpos", iToStr ( Vehicle->BuildBigSavedPos ).c_str() );

		if ( Vehicle->BuildPath )
		{
			element->SetAttribute ( "path", "1" );
			element->SetAttribute ( "turnsstart", iToStr ( Vehicle->BuildRoundsStart ).c_str() );
			element->SetAttribute ( "costsstart", iToStr ( Vehicle->BuildCostsStart ).c_str() );
			element->SetAttribute ( "endx", iToStr ( Vehicle->BandX ).c_str() );
			element->SetAttribute ( "endy", iToStr ( Vehicle->BandY ).c_str() );
		}
	}
	if ( Vehicle->IsClearing ) addAttributeElement ( unitNode, "Clearing", "turns", iToStr ( Vehicle->ClearingRounds ).c_str(), "savedpos", iToStr ( Vehicle->BuildBigSavedPos ).c_str() );
	if ( Vehicle->ServerMoveJob ) addAttributeElement ( unitNode, "Movejob", "destx", iToStr ( Vehicle->ServerMoveJob->DestX ).c_str(), "desty", iToStr ( Vehicle->ServerMoveJob->DestY ).c_str()  );

	// write from which players this unit has been detected
	if ( Vehicle->DetectedByPlayerList.Size() > 0 )
	{
		TiXmlElement *detecedByNode = addMainElement ( unitNode, "IsDetectedByPlayers" );
		for ( unsigned int i = 0; i < Vehicle->DetectedByPlayerList.Size(); i++ )
		{
			addAttributeElement ( detecedByNode, "Player_" + iToStr ( i ), "nr", iToStr ( Vehicle->DetectedByPlayerList[i]->Nr ) );
		}
	}

	// write all stored vehicles
	for ( unsigned int i = 0; i < Vehicle->StoredVehicles.Size(); i++ )
	{
		(*unitnum)++;
		TiXmlElement *storedNode = writeUnit ( Vehicle->StoredVehicles[i], unitnum );
		addAttributeElement ( storedNode, "Stored_In", "id", iToStr ( Vehicle->iID ), "is_vehicle", "1" );
	}
	return unitNode;
}

//--------------------------------------------------------------------------
void cSavegame::writeUnit ( cBuilding *Building, int *unitnum )
{
	// add units node if it doesn't exists
	TiXmlElement *unitsNode;
	if ( !(unitsNode = SaveFile->RootElement()->FirstChildElement ( "Units" )) )
	{
		unitsNode = addMainElement ( SaveFile->RootElement(), "Units" );
		addAttributeElement ( unitsNode, "NextUnitID", "num", iToStr ( Server->iNextUnitID ) );
	}

	// add the unit node
	TiXmlElement *unitNode = addMainElement ( unitsNode, "Unit_" + iToStr( *unitnum ) );

	// write main information
	addAttributeElement ( unitNode, "Type", "string", Building->data.ID.getText() );
	addAttributeElement ( unitNode, "ID", "num", iToStr ( Building->iID ) );
	addAttributeElement ( unitNode, "Owner", "num", iToStr ( Building->owner->Nr ) );
	addAttributeElement ( unitNode, "Position", "x", iToStr ( Building->PosX ), "y", iToStr ( Building->PosY ) );

	// add information whether the unitname isn't serverdefault, so that it would be readed when loading but is in the save to make him more readable
	string wasName = Building->name;
	Building->GenerateName();
	addAttributeElement ( unitNode, "Name", "string", Building->name, "notDefault", ( !Building->name.compare ( wasName ) ) ? "0" : "1" );
	Building->name = wasName;

	if ( Building->SubBase ) addAttributeElement ( unitNode, "SubBase", "num", iToStr (Building->SubBase->iID ) );

	// write the standard values
	writeUnitValues ( unitNode, &Building->data, &Building->owner->BuildingData[Building->typ->nr] );

	// write additional stauts information
	if ( Building->IsWorking ) addMainElement ( unitNode, "IsWorking" );
	if ( Building->data.can_research )
	{
		TiXmlElement *researchNode = addMainElement ( unitNode, "ResearchArea" );
		researchNode->SetAttribute ( "area", iToStr(Building->researchArea).c_str() );
	}
	if ( Building->bSentryStatus ) addMainElement ( unitNode, "OnSentry" );
	if ( Building->hasBeenAttacked ) addMainElement ( unitNode, "HasBeenAttacked" );

	if ( Building->MetalProd > 0 ) addAttributeElement ( unitNode, "MetalProd", "num", iToStr ( Building->MetalProd ) );
	if ( Building->GoldProd > 0 ) addAttributeElement ( unitNode, "GoldProd", "num", iToStr ( Building->GoldProd ) );
	if ( Building->OilProd > 0 ) addAttributeElement ( unitNode, "OilProd", "num", iToStr ( Building->OilProd ) );

	// write the buildlist
	if ( Building->BuildList && Building->BuildList->Size() > 0 )
	{
		TiXmlElement *buildNode = addMainElement ( unitNode, "Building" );
		addAttributeElement ( buildNode, "BuildSpeed", "num", iToStr ( Building->BuildSpeed ) );
		addAttributeElement ( buildNode, "MetalPerRound", "num", iToStr ( Building->MetalPerRound ) );
		if ( Building->RepeatBuild ) addMainElement ( buildNode, "RepeatBuild" );

		TiXmlElement *buildlistNode = addMainElement ( buildNode, "BuildList" );
		for ( unsigned int i = 0; i < Building->BuildList->Size(); i++ )
		{
			addAttributeElement ( buildlistNode, "Item_" + iToStr ( i ), "type_id", (*Building->BuildList)[i]->typ->data.ID.getText(), "metall_remaining", iToStr ((*Building->BuildList)[i]->metall_remaining ) );
		}
	}

	// write from which players this unit has been detected
	if ( Building->DetectedByPlayerList.Size() > 0 )
	{
		TiXmlElement *detecedByNode = addMainElement ( unitNode, "IsDetectedByPlayers" );
		for ( unsigned int i = 0; i < Building->DetectedByPlayerList.Size(); i++ )
		{
			addAttributeElement ( detecedByNode, "Player_" + iToStr ( i ), "nr", iToStr ( Building->DetectedByPlayerList[i]->Nr ) );
		}
	}

	// write all stored vehicles
	for ( unsigned int i = 0; i < Building->StoredVehicles.Size(); i++ )
	{
		(*unitnum)++;
		TiXmlElement *storedNode = writeUnit ( Building->StoredVehicles[i], unitnum );
		addAttributeElement ( storedNode, "Stored_In", "id", iToStr ( Building->iID ), "is_vehicle", "0" );
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeRubble ( cBuilding *Building, int rubblenum )
{
	// add units node if it doesn't exists
	TiXmlElement *unitsNode;
	if ( !(unitsNode = SaveFile->RootElement()->FirstChildElement ( "Units" )) )
	{
		unitsNode = addMainElement ( SaveFile->RootElement(), "Units" );
		addAttributeElement ( unitsNode, "NextUnitID", "num", iToStr ( Server->iNextUnitID ) );
	}

	// add the rubble node
	TiXmlElement *rubbleNode = addMainElement ( unitsNode, "Rubble_" + iToStr( rubblenum ) );

	addAttributeElement ( rubbleNode, "Position", "x", iToStr ( Building->PosX ), "y", iToStr ( Building->PosY ) );
	addAttributeElement ( rubbleNode, "RubbleValue", "num", iToStr ( Building->RubbleValue ) );
	if ( Building->data.is_big ) addMainElement ( rubbleNode, "Big" );
}

//--------------------------------------------------------------------------
void cSavegame::writeUnitValues ( TiXmlElement *unitNode, sUnitData *Data, sUnitData *OwnerData )
{
	// write the standard status values
	if ( Data->hit_points != Data->max_hit_points ) addAttributeElement ( unitNode, "Hitpoints", "num", iToStr ( Data->hit_points ) );
	if ( Data->ammo != Data->max_ammo ) addAttributeElement ( unitNode, "Ammo", "num", iToStr ( Data->ammo ) );
	if ( Data->cargo > 0 ) addAttributeElement ( unitNode, "Cargo", "num", iToStr ( Data->cargo ) );
	if ( Data->speed != Data->max_speed ) addAttributeElement ( unitNode, "Speed", "num", iToStr ( Data->speed ) );
	if ( Data->shots != Data->max_shots ) addAttributeElement ( unitNode, "Shots", "num", iToStr ( Data->shots ) );

	// write upgrade values that differ from the acctual unit values of the owner
	if ( OwnerData->version > 1 )
	{
		addAttributeElement ( unitNode, "Version", "num", iToStr ( Data->version ) );
		if ( Data->max_hit_points != OwnerData->max_hit_points ) addAttributeElement ( unitNode, "Max_Hitpoints", "num", iToStr ( Data->max_hit_points ) );
		if ( Data->max_ammo != OwnerData->max_ammo ) addAttributeElement ( unitNode, "Max_Ammo", "num", iToStr ( Data->max_ammo ) );
		if ( Data->max_speed != OwnerData->max_speed ) addAttributeElement ( unitNode, "Max_Speed", "num", iToStr ( Data->max_speed ) );
		if ( Data->max_shots != OwnerData->max_shots ) addAttributeElement ( unitNode, "Max_Shots", "num", iToStr ( Data->max_shots ) );

		if ( Data->armor != OwnerData->armor ) addAttributeElement ( unitNode, "Armor", "num", iToStr ( Data->armor ) );
		if ( Data->damage != OwnerData->damage ) addAttributeElement ( unitNode, "Damage", "num", iToStr ( Data->damage ) );
		if ( Data->range != OwnerData->range ) addAttributeElement ( unitNode, "Range", "num", iToStr ( Data->range ) );
		if ( Data->scan != OwnerData->scan ) addAttributeElement ( unitNode, "Scan", "num", iToStr ( Data->scan ) );
	}
}

//--------------------------------------------------------------------------
void cSavegame::writeStandardUnitValues ( sUnitData *Data, int unitnum )
{
	// add the main node if it doesn't exists
	TiXmlElement *unitValuesNode;
	if ( !(unitValuesNode = SaveFile->RootElement()->FirstChildElement ( "UnitValues" )) )
	{
		unitValuesNode = addMainElement ( SaveFile->RootElement(), "UnitValues" );
	}
	// add the unit node
	TiXmlElement *unitNode = addMainElement ( unitValuesNode, "UnitVal_" + iToStr( unitnum ) );
	addAttributeElement ( unitNode, "ID", "string", Data->ID.getText() );
	addAttributeElement ( unitNode, "Name", "string", Data->szName );

	addAttributeElement ( unitNode, "Hitpoints", "num", iToStr ( Data->iHitpoints_Max ) );
	addAttributeElement ( unitNode, "Armor", "num", iToStr ( Data->iArmor ) );
	addAttributeElement ( unitNode, "Built_Costs", "num", iToStr ( Data->iBuilt_Costs ) );
	addAttributeElement ( unitNode, "Built_Costs_Max", "num", iToStr ( Data->iBuilt_Costs_Max ) );
	if ( Data->iEnergy_Shield_Strength_Max > 0 ) addAttributeElement ( unitNode, "Shield", "num", iToStr ( Data->iEnergy_Shield_Strength_Max ) );

	if ( Data->iScan_Range_Sight > 1 ) addAttributeElement ( unitNode, "Scan_Range_Sight", "num", iToStr ( Data->iScan_Range_Sight ) );
	if ( Data->iScan_Range_Mine > 0 ) addAttributeElement ( unitNode, "Scan_Range_Mine", "num", iToStr ( Data->iScan_Range_Mine ) );
	if ( Data->iScan_Range_Resources > 0 ) addAttributeElement ( unitNode, "Scan_Range_Resource", "num", iToStr ( Data->iScan_Range_Resources ) );
	if ( Data->iScan_Range_Submarine > 0 ) addAttributeElement ( unitNode, "Scan_Range_Submarine", "num", iToStr ( Data->iScan_Range_Submarine ) );
	if ( Data->iScan_Range_Infantry > 0 ) addAttributeElement ( unitNode, "Scan_Range_Infantry", "num", iToStr ( Data->iScan_Range_Infantry ) );

	if ( Data->iMovement_Max > 0 ) addAttributeElement ( unitNode, "Movement", "num", iToStr ( Data->iMovement_Max/4 ) );
	if ( Data->iMakes_Tracks != 3 ) addAttributeElement ( unitNode, "Makes_Tracks", "num", iToStr ( Data->iMakes_Tracks ) );

	if ( Data->Weapons[0].iMovement_Allowed > 0 ) addAttributeElement ( unitNode, "Movement_Allowed", "num", iToStr ( Data->Weapons[0].iMovement_Allowed ) );
	if ( Data->Weapons[0].iMuzzleType > 0 ) addAttributeElement ( unitNode, "MuzzleType", "num", iToStr ( Data->Weapons[0].iMuzzleType ) );
	if ( Data->Weapons[0].iShots_Max > 0 ) addAttributeElement ( unitNode, "Shots", "num", iToStr ( Data->Weapons[0].iShots_Max ) );
	if ( Data->Weapons[0].iAmmo_Quantity_Max > 0 ) addAttributeElement ( unitNode, "Ammo", "num", iToStr ( Data->Weapons[0].iAmmo_Quantity_Max ) );

	if ( Data->Weapons[0].iTarget_Air_Range > 0 ) addAttributeElement ( unitNode, "Air_Range", "num", iToStr ( Data->Weapons[0].iTarget_Air_Range ) );
	if ( Data->Weapons[0].iTarget_Infantry_Range > 0 ) addAttributeElement ( unitNode, "Infantry_Range", "num", iToStr ( Data->Weapons[0].iTarget_Infantry_Range ) );
	if ( Data->Weapons[0].iTarget_Land_Range > 0 ) addAttributeElement ( unitNode, "Land_Range", "num", iToStr ( Data->Weapons[0].iTarget_Land_Range ) );
	if ( Data->Weapons[0].iTarget_Mine_Range > 0 ) addAttributeElement ( unitNode, "Mine_Range", "num", iToStr ( Data->Weapons[0].iTarget_Mine_Range ) );
	if ( Data->Weapons[0].iTarget_Sea_Range > 0 ) addAttributeElement ( unitNode, "Sea_Range", "num", iToStr ( Data->Weapons[0].iTarget_Sea_Range ) );
	if ( Data->Weapons[0].iTarget_Submarine_Range > 0 ) addAttributeElement ( unitNode, "Submarine_Range", "num", iToStr ( Data->Weapons[0].iTarget_Submarine_Range ) );

	if ( Data->Weapons[0].iTarget_Air_Damage > 0 ) addAttributeElement ( unitNode, "Air_Damage", "num", iToStr ( Data->Weapons[0].iTarget_Air_Damage ) );
	if ( Data->Weapons[0].iTarget_Infantry_Damage > 0 ) addAttributeElement ( unitNode, "Infantry_Damage", "num", iToStr ( Data->Weapons[0].iTarget_Infantry_Damage ) );
	if ( Data->Weapons[0].iTarget_Land_Damage > 0 ) addAttributeElement ( unitNode, "Land_Damage", "num", iToStr ( Data->Weapons[0].iTarget_Land_Damage ) );
	if ( Data->Weapons[0].iTarget_Mine_Damage > 0 ) addAttributeElement ( unitNode, "Mine_Damage", "num", iToStr ( Data->Weapons[0].iTarget_Mine_Damage ) );
	if ( Data->Weapons[0].iTarget_Sea_Damage > 0 ) addAttributeElement ( unitNode, "Sea_Damage", "num", iToStr ( Data->Weapons[0].iTarget_Sea_Damage ) );
	if ( Data->Weapons[0].iTarget_Submarine_Damage > 0 ) addAttributeElement ( unitNode, "Submarine_Damage", "num", iToStr ( Data->Weapons[0].iTarget_Submarine_Damage ) );

	if ( Data->iCapacity_Metal_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Metal", "num", iToStr ( Data->iCapacity_Metal_Max ) );
	if ( Data->iCapacity_Oil_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Oil", "num", iToStr ( Data->iCapacity_Oil_Max ) );
	if ( Data->iCapacity_Gold_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Gold", "num", iToStr ( Data->iCapacity_Gold_Max ) );
	if ( Data->iCapacity_Units_Air_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Units_Air", "num", iToStr ( Data->iCapacity_Units_Air_Max ) );
	if ( Data->iCapacity_Units_Sea_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Units_Sea", "num", iToStr ( Data->iCapacity_Units_Sea_Max ) );
	if ( Data->iCapacity_Units_Ground_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Units_Ground", "num", iToStr ( Data->iCapacity_Units_Ground_Max ) );
	if ( Data->iCapacity_Units_Infantry_Max > 0 ) addAttributeElement ( unitNode, "Capacity_Units_Infantry", "num", iToStr ( Data->iCapacity_Units_Infantry_Max ) );

	if ( Data->iNeeds_Energy != 0 ) addAttributeElement ( unitNode, "Needs_Energy", "num", iToStr ( Data->iNeeds_Energy ) );
	if ( Data->iNeeds_Humans != 0 ) addAttributeElement ( unitNode, "Needs_Humans", "num", iToStr ( Data->iNeeds_Humans ) );
	if ( Data->iNeeds_Oil != 0 ) addAttributeElement ( unitNode, "Needs_Oil", "num", iToStr ( Data->iNeeds_Oil ) );
	if ( Data->iNeeds_Metal != 0 ) addAttributeElement ( unitNode, "Needs_Metal", "num", iToStr ( Data->iNeeds_Metal ) );
	if ( Data->iConverts_Gold != 0 ) addAttributeElement ( unitNode, "Converts_Gold", "num", iToStr ( Data->iConverts_Gold ) );

	if( Data->fCosts_Ground != 1.0 ) addAttributeElement ( unitNode, "Costs_Ground", "num", dToStr ( Data->fCosts_Ground ) );
	if( Data->fCosts_Sea != 0.0 ) addAttributeElement ( unitNode, "Costs_Sea", "num", dToStr ( Data->fCosts_Sea ) );
	if( Data->fCosts_Air != 0.0 ) addAttributeElement ( unitNode, "Costs_Air", "num", dToStr ( Data->fCosts_Air ) );
	if( Data->fCosts_Submarine != 0.0 ) addAttributeElement ( unitNode, "Costs_Submarine", "num", dToStr ( Data->fCosts_Submarine ) );

	if( Data->fFactor_Coast != 1.5 ) addAttributeElement ( unitNode, "Factor_Coast", "num", dToStr ( Data->fFactor_Coast ) );
	if( Data->fFactor_Wood != 1.5 ) addAttributeElement ( unitNode, "Factor_Wood", "num", dToStr ( Data->fFactor_Wood ) );
	if( Data->fFactor_Road != 0.5 ) addAttributeElement ( unitNode, "Factor_Road", "num", dToStr ( Data->fFactor_Road ) );
	if( Data->fFactor_Bridge != 1.0 ) addAttributeElement ( unitNode, "Factor_Bridge", "num", dToStr ( Data->fFactor_Bridge ) );
	if( Data->fFactor_Platform != 1.0 ) addAttributeElement ( unitNode, "Factor_Platform", "num", dToStr ( Data->fFactor_Platform ) );
	if( Data->fFactor_Monorail != 0.0 ) addAttributeElement ( unitNode, "Factor_Monorail", "num", dToStr ( Data->fFactor_Monorail ) );
	if( Data->fFactor_Wreck != 1.0 ) addAttributeElement ( unitNode, "Factor_Wreck", "num", dToStr ( Data->fFactor_Wreck ) );
	if( Data->fFactor_Mountains != 0.0 ) addAttributeElement ( unitNode, "Factor_Mountains", "num", dToStr ( Data->fFactor_Mountains ) );

	if( Data->iSize_Length != 1 ) addAttributeElement ( unitNode, "Size_Length", "num", iToStr ( Data->iSize_Length ) );
	if( Data->iSize_Width != 1 ) addAttributeElement ( unitNode, "Size_Width", "num", iToStr ( Data->iSize_Width ) );

	if( Data->bCan_Repair ) addMainElement ( unitNode, "Can_Repair" );
	if( Data->bCan_Rearm ) addMainElement ( unitNode, "Can_Rearm" );
	if( Data->bCan_Research ) addMainElement ( unitNode, "Can_Research" );
	if( Data->bCan_Clear_Area ) addMainElement ( unitNode, "Can_Clear" );
	if( Data->bCan_Place_Mines ) addMainElement ( unitNode, "Can_Place_Mines" );
	if( Data->bAnimation_Movement ) addMainElement ( unitNode, "Has_Animation_Movement" );
	if( Data->bPower_On_Grafic ) addMainElement ( unitNode, "Has_Power_On_Grafic" );
	if( Data->bHas_Overlay ) addMainElement ( unitNode, "Has_Overlay" );
}

//--------------------------------------------------------------------------
void cSavegame::addAttributeElement( TiXmlElement *node, string nodename, string attributename, string value, string attributename2, string value2 )
{
	TiXmlElement *element = addMainElement ( node, nodename );
	element->SetAttribute ( attributename.c_str(), value.c_str() );
	if ( attributename2.compare("") ) element->SetAttribute ( attributename2.c_str(), value2.c_str() );
}

//--------------------------------------------------------------------------
TiXmlElement *cSavegame::addMainElement( TiXmlElement *node, string nodename )
{
	TiXmlElement *element = new TiXmlElement ( nodename.c_str() );
	node->LinkEndChild( element );
	return element;
}
