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
#include "loaddata.h"
#include "upgradecalculator.h"
#include "menus.h"

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
	rootnode->SetAttribute ( "version", (SAVE_FORMAT_VERSION).c_str() );
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

	for ( unsigned int i = 0; i < UnitsData.getNrVehicles () + UnitsData.getNrBuildings (); i++ )
	{
		sUnitData *Data;
		if ( i < UnitsData.getNrVehicles () ) Data = &UnitsData.vehicle[i].data;
		else Data = &UnitsData.building[i - UnitsData.getNrVehicles ()].data;
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

	version = SaveFile->RootElement()->Attribute ( "version" );
	if ( version.compare ( SAVE_FORMAT_VERSION ) )
	{
		Log.write ( "Savefile-version differs from the one supported by the game!", cLog::eLOG_TYPE_WARNING );
	}

	// load standard unit values
	if ( atoi ( version.substr ( 0, version.find_first_of ( "." ) ).c_str() ) == 0 &&
		atoi ( version.substr ( version.find_first_of ( "." ), version.length()-version.find_first_of ( "." ) ).c_str() ) <= 2 )
	{
		Log.write ( "Skiping loading standard unit values becouse savegame has version 0.2 or older.", LOG_TYPE_DEBUG );
	}
	else
	{
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

	TiXmlElement *element;
	int clan = -1;
	if ( element = playerNode->FirstChildElement ("Clan" ) ) element->Attribute ( "num", &clan );
	Player->setClan (clan);

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
				for ( num = 0; num < UnitsData.getNrVehicles (); num++ ) if ( UnitsData.vehicle[num].data.ID == ID ) break;
				loadUpgrade ( upgradeNode, &Player->VehicleData[num] );
			}
			else
			{
				unsigned int num;
				for ( num = 0; num < UnitsData.getNrBuildings (); num++ ) if ( UnitsData.building[num].data.ID == ID ) break;
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
	if ( element = upgradeNode->FirstChildElement( "Ammo" ) ) element->Attribute ( "num", &data->ammoMax );
	if ( element = upgradeNode->FirstChildElement( "HitPoints" ) ) element->Attribute ( "num", &data->hitpointsMax );
	if ( element = upgradeNode->FirstChildElement( "Shots" ) ) element->Attribute ( "num", &data->shotsMax );
	if ( element = upgradeNode->FirstChildElement( "Speed" ) ) element->Attribute ( "num", &data->speedMax );
	if ( element = upgradeNode->FirstChildElement( "Armor" ) ) element->Attribute ( "num", &data->armor );
	if ( element = upgradeNode->FirstChildElement( "Costs" ) ) element->Attribute ( "num", &data->buildCosts );
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
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "shotsCur", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kShotsResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "range", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kRangeResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "armor", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kArmorResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "hitpoints", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kHitpointsResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "speedCur", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kSpeedResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "scan", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kScanResearch );
	researchLevelNode->FirstChildElement( "Level" )->Attribute ( "cost", &value );
	researchLevel.setCurResearchLevel( value, cResearch::kCostResearch );
	
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "attack", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kAttackResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "shotsCur", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kShotsResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "range", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kRangeResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "armor", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kArmorResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "hitpoints", &value );
	researchLevel.setCurResearchPoints( value, cResearch::kHitpointsResearch );
	researchLevelNode->FirstChildElement( "CurPoints" )->Attribute ( "speedCur", &value );
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
	researchCentersWorkingOnAreaNode->Attribute ( "shotsCur", &value );
	player->researchCentersWorkingOnArea[cResearch::kShotsResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "range", &value );
	player->researchCentersWorkingOnArea[cResearch::kRangeResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "armor", &value );
	player->researchCentersWorkingOnArea[cResearch::kArmorResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "hitpoints", &value );
	player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch] = value;
	researchCentersWorkingOnAreaNode->Attribute ( "speedCur", &value );
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
	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
	{
		if ( UnitsData.vehicle[i].data.ID == ID )
		{
			number = i;
			break;
		}
		if ( i == UnitsData.getNrVehicles () - 1 ) return;
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

			StoringVehicle->data.storageUnitsCur--;
			StoringVehicle->storeVehicle ( vehicle, Server->Map );
		}
		else
		{
			cBuilding *StoringBuilding = Server->getBuildingFromID ( storedInID );
			if ( !StoringBuilding ) return;

			StoringBuilding->data.storageUnitsCur--;
			StoringBuilding->storeVehicle ( vehicle, Server->Map );
		}
	}
}

//--------------------------------------------------------------------------
void cSavegame::loadBuilding( TiXmlElement *unitNode, sID &ID )
{
	if ( !Server ) return;
	int tmpinteger, number, x, y;
	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
	{
		if ( UnitsData.building[i].data.ID == ID )
		{
			number = i;
			break;
		}
		if ( i == UnitsData.getNrBuildings () - 1 ) return;
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
				for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
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

	if ( Element = unitNode->FirstChildElement( "Max_Hitpoints" ) ) Element->Attribute ( "num", &Data->hitpointsMax );
	if ( Element = unitNode->FirstChildElement( "Max_Ammo" ) ) Element->Attribute ( "num", &Data->ammoMax );
	if ( Element = unitNode->FirstChildElement( "Max_Speed" ) ) Element->Attribute ( "num", &Data->speedMax );
	if ( Element = unitNode->FirstChildElement( "Max_Shots" ) ) Element->Attribute ( "num", &Data->shotsMax );
	if ( Element = unitNode->FirstChildElement( "Armor" ) ) Element->Attribute ( "num", &Data->armor );
	if ( Element = unitNode->FirstChildElement( "Damage" ) ) Element->Attribute ( "num", &Data->damage );
	if ( Element = unitNode->FirstChildElement( "Range" ) ) Element->Attribute ( "num", &Data->range );
	if ( Element = unitNode->FirstChildElement( "Scan" ) ) Element->Attribute ( "num", &Data->scan );

	if ( Element = unitNode->FirstChildElement( "Hitpoints" ) ) Element->Attribute ( "num", &Data->hitpointsCur );
	else Data->hitpointsCur = Data->hitpointsMax;
	if ( Element = unitNode->FirstChildElement( "Ammo" ) ) Element->Attribute ( "num", &Data->ammoCur );
	else Data->ammoCur = Data->ammoMax;

	if ( Element = unitNode->FirstChildElement( "ResCargo" ) ) Element->Attribute ( "num", &Data->storageResCur );
	if ( Element = unitNode->FirstChildElement( "UnitCargo" ) ) Element->Attribute ( "num", &Data->storageUnitsCur );
	// look for "Cargo" to be savegamecompatible
	if ( Element = unitNode->FirstChildElement( "Cargo" ) )
	{
		Element->Attribute ( "num", &Data->storageResCur );
		Data->storageUnitsCur = Data->storageResCur;
	}

	if ( Element = unitNode->FirstChildElement( "Speed" ) ) Element->Attribute ( "num", &Data->speedCur );
	else Data->speedCur = Data->speedMax;
	if ( Element = unitNode->FirstChildElement( "Shots" ) ) Element->Attribute ( "num", &Data->shotsCur );
	else Data->shotsCur = Data->shotsMax;
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
		for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
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
		for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
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

	Data->ID = ID;

	Data->name = unitNode->FirstChildElement( "Name" )->Attribute ( "string" );
	unitNode->FirstChildElement( "Version" )->Attribute ( "num", &Data->version );

	unitNode->FirstChildElement( "Hitpoints" )->Attribute ( "num", &Data->hitpointsMax );
	unitNode->FirstChildElement( "Armor" )->Attribute ( "num", &Data->armor );
	unitNode->FirstChildElement( "Built_Costs" )->Attribute ( "num", &Data->buildCosts );

	if ( Element = unitNode->FirstChildElement( "Scan" ) ) Element->Attribute ( "num", &Data->scan ); else Data->scan = 0;
	if ( Element = unitNode->FirstChildElement( "Movement" ) ) Element->Attribute ( "num", &Data->speedMax ); else Data->speedMax = 0;
	Data->speedMax *= 4;
	
	int tmpInt;

	if ( Element = unitNode->FirstChildElement( "MuzzleType" ) ) { Element->Attribute ( "num", &tmpInt ); Data->muzzleType = (sUnitData::eMuzzleType)tmpInt; } else Data->muzzleType = sUnitData::MUZZLE_TYPE_NONE;
	if ( Element = unitNode->FirstChildElement( "Shots" ) ) Element->Attribute ( "num", &Data->shotsMax ); else Data->shotsMax = 0;
	if ( Element = unitNode->FirstChildElement( "Ammo" ) ) Element->Attribute ( "num", &Data->ammoMax ); else Data->ammoMax = 0;
	if ( Element = unitNode->FirstChildElement( "Range" ) ) Element->Attribute ( "num", &Data->range ); else Data->range = 0;
	if ( Element = unitNode->FirstChildElement( "Damage" ) ) Element->Attribute ( "num", &Data->damage ); else Data->damage = 0;
	if ( Element = unitNode->FirstChildElement( "Can_Attack" ) ) { Element->Attribute ( "num", &tmpInt ); Data->canAttack = tmpInt; } else Data->canAttack = TERRAIN_NONE;

	if ( Element = unitNode->FirstChildElement( "Can_Build" ) ) Data->canBuild = Element->Attribute ( "string" ); else Data->canBuild = "";
	if ( Element = unitNode->FirstChildElement( "Build_As" ) ) Data->buildAs = Element->Attribute ( "string" ); else Data->buildAs = "";
	if ( Element = unitNode->FirstChildElement( "Max_Build_Factor" ) ) Element->Attribute ( "num", &Data->maxBuildFactor ); else Data->maxBuildFactor = 0;

	if ( Element = unitNode->FirstChildElement( "Storage_Res_Max" ) ) Element->Attribute ( "num", &Data->storageResMax ); else Data->storageResMax = 0;
	if ( Element = unitNode->FirstChildElement( "Storage_Units_Max" ) ) Element->Attribute ( "num", &Data->storageUnitsMax ); else Data->storageUnitsMax = 0;
	if ( Element = unitNode->FirstChildElement( "Store_Res_Type" ) ) { Element->Attribute ( "num", &tmpInt ); Data->storeResType = (sUnitData::eStorageResType)tmpInt; } else Data->storeResType = sUnitData::STORE_RES_NONE;
	if ( Element = unitNode->FirstChildElement( "StoreUnits_Image_Type" ) ) { Element->Attribute ( "num", &tmpInt ); Data->storeUnitsImageType = (sUnitData::eStorageUnitsImageType)tmpInt; } else Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_NONE;
	if ( Element = unitNode->FirstChildElement( "Is_Storage_Type" ) ) Data->isStorageType = Element->Attribute ( "string" ); else Data->isStorageType = "";
	if ( Element = unitNode->FirstChildElement( "StoreUnitsTypes" ) )
	{
		string storeUnitsString = Element->Attribute ( "string" );
		if ( storeUnitsString.length() > 0 )
		{
			int pos = -1;
			do
			{
				int lastpos = pos;
				pos = storeUnitsString.find_first_of ( "+", pos+1 );
				if ( pos == string::npos ) pos = storeUnitsString.length();
				Data->storeUnitsTypes.push_back ( storeUnitsString.substr ( lastpos+1, pos-(lastpos+1) ) );
			}
			while ( pos < (int)storeUnitsString.length() );
		}
	}
	
	if ( Element = unitNode->FirstChildElement( "Needs_Energy" ) ) Element->Attribute ( "num", &Data->needsEnergy ); else Data->needsEnergy = 0;
	if ( Element = unitNode->FirstChildElement( "Needs_Humans" ) ) Element->Attribute ( "num", &Data->needsHumans ); else Data->needsHumans = 0;
	if ( Element = unitNode->FirstChildElement( "Needs_Oil" ) ) Element->Attribute ( "num", &Data->needsOil ); else Data->needsOil = 0;
	if ( Element = unitNode->FirstChildElement( "Needs_Metal" ) ) Element->Attribute ( "num", &Data->needsMetal ); else Data->needsMetal = 0;
	if ( Element = unitNode->FirstChildElement( "Converts_Gold" ) ) Element->Attribute ( "num", &Data->convertsGold ); else Data->convertsGold = 0;
	if ( Element = unitNode->FirstChildElement( "Can_Mine_Max_Res" ) ) Element->Attribute ( "num", &Data->canMineMaxRes ); else Data->canMineMaxRes = 0;
	if ( Data->needsEnergy < 0 )
	{
		Data->produceEnergy = abs( Data->needsEnergy );
		Data->needsEnergy = 0;
	} else Data->produceEnergy = 0;
	if ( Data->needsHumans < 0 )
	{
		Data->produceHumans = abs( Data->needsHumans );
		Data->needsHumans = 0;
	} else Data->produceHumans = 0;

	if ( Element = unitNode->FirstChildElement( "Is_Stealth_On" ) ) { Element->Attribute ( "num", &tmpInt ); Data->isStealthOn = tmpInt; } else Data->isStealthOn = TERRAIN_NONE;
	if ( Element = unitNode->FirstChildElement( "Can_Detect_Stealth_On" ) ) { Element->Attribute ( "num", &tmpInt ); Data->canDetectStealthOn = tmpInt; } else Data->canDetectStealthOn = TERRAIN_NONE;

	if ( Element = unitNode->FirstChildElement( "Surface_Position" ) ) { Element->Attribute ( "num", &tmpInt ); Data->surfacePosition = (sUnitData::eSurfacePosition)tmpInt; } else Data->surfacePosition = sUnitData::SURFACE_POS_NORMAL;
	if ( Element = unitNode->FirstChildElement( "Can_Be_Overbuild" ) ) { Element->Attribute ( "num", &tmpInt ); Data->canBeOverbuild = (sUnitData::eOverbuildType)tmpInt; } else Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_NO;

	double tmpdouble;
	if ( Element = unitNode->FirstChildElement( "Factor_Air" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->factorAir = (float)tmpdouble; } else Data->factorAir = 0;
	if ( Element = unitNode->FirstChildElement( "Factor_Coast" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->factorCoast = (float)tmpdouble; } else Data->factorCoast = 0;
	if ( Element = unitNode->FirstChildElement( "Factor_Ground" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->factorGround = (float)tmpdouble; } else Data->factorGround = 0;
	if ( Element = unitNode->FirstChildElement( "Factor_Sea" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->factorSea = (float)tmpdouble; } else Data->factorSea = 0;
	
	if ( Element = unitNode->FirstChildElement( "Factor_Sea" ) ) { Element->Attribute ( "num", &tmpdouble ); Data->modifiesSpeed = (float)tmpdouble; } else Data->modifiesSpeed = 0;

	Data->canBuildPath = unitNode->FirstChildElement( "Can_Build_Path" ) != NULL;
	Data->canBuildRepeat = unitNode->FirstChildElement( "Can_Build_Repeat" ) != NULL;
	Data->buildIntern = unitNode->FirstChildElement( "Build_Intern" ) != NULL;
	Data->connectsToBase = unitNode->FirstChildElement( "Connects_To_Base" ) != NULL;
	
	Data->canBeCaptured = unitNode->FirstChildElement( "Can_Be_Captured" ) != NULL;
	Data->canBeDisabled = unitNode->FirstChildElement( "Can_Be_Disabled" ) != NULL;
	Data->canCapture = unitNode->FirstChildElement( "Can_Capture" ) != NULL;
	Data->canDisable = unitNode->FirstChildElement( "Can_Disable" ) != NULL;
	Data->canSurvey = unitNode->FirstChildElement( "Can_Survey" ) != NULL;
	Data->doesSelfRepair = unitNode->FirstChildElement( "Does_Self_Repair" ) != NULL;
	Data->canSelfDestroy = unitNode->FirstChildElement( "Can_Self_Destroy" ) != NULL;
	Data->explodesOnContact = unitNode->FirstChildElement( "Explodes_On_Contact" ) != NULL;
	Data->isHuman = unitNode->FirstChildElement( "Is_Human" ) != NULL;
	Data->canBeLandedOn = unitNode->FirstChildElement( "Can_Be_Landed_On" ) != NULL;
	Data->canWork = unitNode->FirstChildElement( "Can_Work" ) != NULL;

	Data->isBig = unitNode->FirstChildElement( "Is_Big" ) != NULL;
	Data->canRepair = unitNode->FirstChildElement( "Can_Repair" ) != NULL;
	Data->canRearm = unitNode->FirstChildElement( "Can_Rearm" ) != NULL;
	Data->canResearch = unitNode->FirstChildElement( "Can_Research" ) != NULL;
	Data->canClearArea = unitNode->FirstChildElement( "Can_Clear" ) != NULL;
	Data->canPlaceMines = unitNode->FirstChildElement( "Can_Place_Mines" ) != NULL;
	Data->canDriveAndFire = unitNode->FirstChildElement( "Can_Drive_And_Fire" ) != NULL;
}

//--------------------------------------------------------------------------
void cSavegame::generateMoveJobs ()
{
	for ( unsigned int i = 0; i < MoveJobsLoad.Size(); i++ )
	{
		cServerMoveJob *MoveJob = new cServerMoveJob( MoveJobsLoad[i]->vehicle->PosX+MoveJobsLoad[i]->vehicle->PosY*Server->Map->size, MoveJobsLoad[i]->destX+MoveJobsLoad[i]->destY*Server->Map->size, MoveJobsLoad[i]->vehicle->data.factorAir > 0, MoveJobsLoad[i]->vehicle );
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
	switch ( Server->gameType )
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
	addAttributeElement ( playerNode, "Clan", "num", iToStr ( Player->getClan () ) );
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
	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
	{
		if ( Player->VehicleData[i].version > 1 
			|| Player->VehicleData[i].buildCosts != UnitsData.getVehicle (i, Player->getClan ()).data.buildCosts )  // if only costs were researched, the version is not incremented
		{
			writeUpgrade ( upgradesNode, upgrades, &Player->VehicleData[i], &UnitsData.getVehicle (i, Player->getClan ()).data );
			upgrades++;
		}
	}
	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
	{
		if ( Player->BuildingData[i].version > 1 
			|| Player->BuildingData[i].buildCosts != UnitsData.getBuilding (i, Player->getClan ()).data.buildCosts )  // if only costs were researched, the version is not incremented
		{
			writeUpgrade ( upgradesNode, upgrades, &Player->BuildingData[i], &UnitsData.getBuilding (i, Player->getClan ()).data );
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
	if ( data->ammoMax != originaldata->ammoMax ) addAttributeElement ( upgradeNode, "Ammo", "num", iToStr ( data->ammoMax ) );
	if ( data->hitpointsMax != originaldata->hitpointsMax ) addAttributeElement ( upgradeNode, "HitPoints", "num", iToStr ( data->hitpointsMax ) );
	if ( data->shotsMax != originaldata->shotsMax ) addAttributeElement ( upgradeNode, "Shots", "num", iToStr ( data->shotsMax ) );
	if ( data->speedMax != originaldata->speedMax ) addAttributeElement ( upgradeNode, "Speed", "num", iToStr ( data->speedMax ) );
	if ( data->armor != originaldata->armor ) addAttributeElement ( upgradeNode, "Armor", "num", iToStr ( data->armor ) );
	if ( data->buildCosts != originaldata->buildCosts ) addAttributeElement ( upgradeNode, "Costs", "num", iToStr ( data->buildCosts ) );
	if ( data->damage != originaldata->damage ) addAttributeElement ( upgradeNode, "Damage", "num", iToStr ( data->damage ) );
	if ( data->range != originaldata->range ) addAttributeElement ( upgradeNode, "Range", "num", iToStr ( data->range ) );
	if ( data->scan != originaldata->scan ) addAttributeElement ( upgradeNode, "Scan", "num", iToStr ( data->scan ) );
}

//--------------------------------------------------------------------------
void cSavegame::writeResearchLevel( TiXmlElement *researchLevelNode, cResearch& researchLevel )
{
	TiXmlElement *levelNode = addMainElement ( researchLevelNode, "Level" );
	levelNode->SetAttribute( "attack", iToStr (researchLevel.getCurResearchLevel (cResearch::kAttackResearch)).c_str() );
	levelNode->SetAttribute( "shotsCur", iToStr (researchLevel.getCurResearchLevel (cResearch::kShotsResearch)).c_str() );
	levelNode->SetAttribute( "range", iToStr (researchLevel.getCurResearchLevel (cResearch::kRangeResearch)).c_str() );
	levelNode->SetAttribute( "armor", iToStr (researchLevel.getCurResearchLevel (cResearch::kArmorResearch)).c_str() );
	levelNode->SetAttribute( "hitpoints", iToStr (researchLevel.getCurResearchLevel (cResearch::kHitpointsResearch)).c_str() );
	levelNode->SetAttribute( "speedCur", iToStr (researchLevel.getCurResearchLevel (cResearch::kSpeedResearch)).c_str() );
	levelNode->SetAttribute( "scan", iToStr (researchLevel.getCurResearchLevel (cResearch::kScanResearch)).c_str() );
	levelNode->SetAttribute( "cost", iToStr (researchLevel.getCurResearchLevel (cResearch::kCostResearch)).c_str() );

	TiXmlElement *curPointsNode = addMainElement ( researchLevelNode, "CurPoints" );
	curPointsNode->SetAttribute( "attack", iToStr (researchLevel.getCurResearchPoints (cResearch::kAttackResearch)).c_str() );
	curPointsNode->SetAttribute( "shotsCur", iToStr (researchLevel.getCurResearchPoints (cResearch::kShotsResearch)).c_str() );
	curPointsNode->SetAttribute( "range", iToStr (researchLevel.getCurResearchPoints (cResearch::kRangeResearch)).c_str() );
	curPointsNode->SetAttribute( "armor", iToStr (researchLevel.getCurResearchPoints (cResearch::kArmorResearch)).c_str() );
	curPointsNode->SetAttribute( "hitpoints", iToStr (researchLevel.getCurResearchPoints (cResearch::kHitpointsResearch)).c_str() );
	curPointsNode->SetAttribute( "speedCur", iToStr (researchLevel.getCurResearchPoints (cResearch::kSpeedResearch)).c_str() );
	curPointsNode->SetAttribute( "scan", iToStr (researchLevel.getCurResearchPoints (cResearch::kScanResearch)).c_str() );
	curPointsNode->SetAttribute( "cost", iToStr (researchLevel.getCurResearchPoints (cResearch::kCostResearch)).c_str() );	
}

//--------------------------------------------------------------------------
void cSavegame::writeResearchCentersWorkingOnArea (TiXmlElement *researchCentersWorkingOnAreaNode, cPlayer *player)
{
	researchCentersWorkingOnAreaNode->SetAttribute( "attack", iToStr (player->researchCentersWorkingOnArea[cResearch::kAttackResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "shotsCur", iToStr (player->researchCentersWorkingOnArea[cResearch::kShotsResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "range", iToStr (player->researchCentersWorkingOnArea[cResearch::kRangeResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "armor", iToStr (player->researchCentersWorkingOnArea[cResearch::kArmorResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "hitpoints", iToStr (player->researchCentersWorkingOnArea[cResearch::kHitpointsResearch]).c_str() );
	researchCentersWorkingOnAreaNode->SetAttribute( "speedCur", iToStr (player->researchCentersWorkingOnArea[cResearch::kSpeedResearch]).c_str() );
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
	if ( Vehicle->data.canCapture || Vehicle->data.canDisable ) addAttributeElement ( unitNode, "CommandoRank", "num", dToStr ( Vehicle->CommandoRank ).c_str() );
	if ( Vehicle->data.isBig ) addMainElement ( unitNode, "IsBig" );
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
		if ( Vehicle->data.isBig ) element->SetAttribute ( "savedpos", iToStr ( Vehicle->BuildBigSavedPos ).c_str() );

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
	if ( Building->data.canResearch )
	{
		TiXmlElement *researchNode = addMainElement ( unitNode, "ResearchArea" );
		researchNode->SetAttribute ( "area", iToStr(Building->researchArea).c_str() );
	}
	if ( Building->bSentryStatus ) addMainElement ( unitNode, "OnSentry" );
	if ( Building->hasBeenAttacked ) addMainElement ( unitNode, "HasBeenAttacked" );

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
	if ( Building->data.isBig ) addMainElement ( rubbleNode, "Big" );
}

//--------------------------------------------------------------------------
void cSavegame::writeUnitValues ( TiXmlElement *unitNode, sUnitData *Data, sUnitData *OwnerData )
{
	// write the standard status values
	if ( Data->hitpointsCur != Data->hitpointsMax ) addAttributeElement ( unitNode, "Hitpoints", "num", iToStr ( Data->hitpointsCur ) );
	if ( Data->ammoCur != Data->ammoMax ) addAttributeElement ( unitNode, "Ammo", "num", iToStr ( Data->ammoCur ) );
	
	if ( Data->storageResCur > 0 ) addAttributeElement ( unitNode, "ResCargo", "num", iToStr ( Data->storageResCur ) );
	if ( Data->storageUnitsCur > 0 ) addAttributeElement ( unitNode, "UnitCargo", "num", iToStr ( Data->storageUnitsCur ) );

	if ( Data->speedCur != Data->speedMax ) addAttributeElement ( unitNode, "Speed", "num", iToStr ( Data->speedCur ) );
	if ( Data->shotsCur != Data->shotsMax ) addAttributeElement ( unitNode, "Shots", "num", iToStr ( Data->shotsCur ) );

	// write upgrade values that differ from the acctual unit values of the owner
	if ( OwnerData->version > 1 )
	{
		addAttributeElement ( unitNode, "Version", "num", iToStr ( Data->version ) );
		if ( Data->hitpointsMax != OwnerData->hitpointsMax ) addAttributeElement ( unitNode, "Max_Hitpoints", "num", iToStr ( Data->hitpointsMax ) );
		if ( Data->ammoMax != OwnerData->ammoMax ) addAttributeElement ( unitNode, "Max_Ammo", "num", iToStr ( Data->ammoMax ) );
		if ( Data->speedMax != OwnerData->speedMax ) addAttributeElement ( unitNode, "Max_Speed", "num", iToStr ( Data->speedMax ) );
		if ( Data->shotsMax != OwnerData->shotsMax ) addAttributeElement ( unitNode, "Max_Shots", "num", iToStr ( Data->shotsMax ) );

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
	addAttributeElement ( unitNode, "Name", "string", Data->name );
	addAttributeElement ( unitNode, "Version", "num", iToStr ( Data->version ) );

	addAttributeElement ( unitNode, "Hitpoints", "num", iToStr ( Data->hitpointsMax ) );
	addAttributeElement ( unitNode, "Armor", "num", iToStr ( Data->armor ) );
	addAttributeElement ( unitNode, "Built_Costs", "num", iToStr ( Data->buildCosts ) );

	if ( Data->scan > 0 ) addAttributeElement ( unitNode, "Scan", "num", iToStr ( Data->scan ) );

	if ( Data->speedMax > 0 ) addAttributeElement ( unitNode, "Movement", "num", iToStr ( Data->speedMax/4 ) );

	if ( Data->muzzleType > 0 ) addAttributeElement ( unitNode, "MuzzleType", "num", iToStr ( Data->muzzleType ) );
	if ( Data->shotsMax > 0 ) addAttributeElement ( unitNode, "Shots", "num", iToStr ( Data->shotsMax ) );
	if ( Data->ammoMax > 0 ) addAttributeElement ( unitNode, "Ammo", "num", iToStr ( Data->ammoMax ) );
	if ( Data->range > 0 ) addAttributeElement ( unitNode, "Range", "num", iToStr ( Data->range ) );
	if ( Data->damage > 0 ) addAttributeElement ( unitNode, "Damage", "num", iToStr ( Data->damage ) );
	if ( Data->canAttack != TERRAIN_NONE ) addAttributeElement ( unitNode, "Can_Attack", "num", iToStr ( Data->canAttack ) );

	if ( !Data->canBuild.empty() ) addAttributeElement ( unitNode, "Can_Build", "string", Data->canBuild );
	if ( !Data->buildAs.empty() ) addAttributeElement ( unitNode, "Build_As", "string", Data->buildAs );
	if ( Data->maxBuildFactor > 0 ) addAttributeElement ( unitNode, "Max_Build_Factor", "num", iToStr ( Data->maxBuildFactor) );

	if ( Data->storageResMax > 0 ) addAttributeElement ( unitNode, "Storage_Res_Max", "num", iToStr ( Data->storageResMax ) );
	if ( Data->storageUnitsMax > 0 ) addAttributeElement ( unitNode, "Storage_Units_Max", "num", iToStr ( Data->storageUnitsMax ) );
	if ( Data->storeResType != sUnitData::STORE_RES_NONE ) addAttributeElement ( unitNode, "Store_Res_Type", "num", iToStr ( Data->storeResType ) );
	if ( Data->storeUnitsImageType != sUnitData::STORE_UNIT_IMG_NONE ) addAttributeElement ( unitNode, "StoreUnits_Image_Type", "num", iToStr ( Data->storeUnitsImageType ) );
	if ( !Data->isStorageType.empty() ) addAttributeElement ( unitNode, "Is_Storage_Type", "string", Data->isStorageType );
	if ( !Data->storeUnitsTypes.empty() )
	{
		string storeUnitsTypes = Data->storeUnitsTypes[0];
		for ( unsigned int i = 1; i < Data->storeUnitsTypes.size(); i++ )
		{
			storeUnitsTypes += "+" + Data->storeUnitsTypes[i];
		}
		addAttributeElement ( unitNode, "StoreUnitsTypes", "string", storeUnitsTypes );
	}

	if ( Data->needsEnergy != 0 ) addAttributeElement ( unitNode, "Needs_Energy", "num", iToStr ( Data->needsEnergy ) );
	if ( Data->produceEnergy != 0 ) addAttributeElement ( unitNode, "Needs_Energy", "num", iToStr ( -Data->produceEnergy ) );
	if ( Data->needsHumans != 0 ) addAttributeElement ( unitNode, "Needs_Humans", "num", iToStr ( Data->needsHumans ) );
	if ( Data->produceHumans != 0 ) addAttributeElement ( unitNode, "Needs_Humans", "num", iToStr ( -Data->produceHumans ) );
	if ( Data->needsOil != 0 ) addAttributeElement ( unitNode, "Needs_Oil", "num", iToStr ( Data->needsOil ) );
	if ( Data->needsMetal != 0 ) addAttributeElement ( unitNode, "Needs_Metal", "num", iToStr ( Data->needsMetal ) );
	if ( Data->convertsGold != 0 ) addAttributeElement ( unitNode, "Converts_Gold", "num", iToStr ( Data->convertsGold ) );
	if ( Data->canMineMaxRes != 0 ) addAttributeElement ( unitNode, "Can_Mine_Max_Res", "num", iToStr ( Data->canMineMaxRes ) );
	
	if ( Data->isStealthOn != TERRAIN_NONE ) addAttributeElement ( unitNode, "Is_Stealth_On", "num", iToStr ( Data->isStealthOn ) );
	if ( Data->canDetectStealthOn != TERRAIN_NONE ) addAttributeElement ( unitNode, "Can_Detect_Stealth_On", "num", iToStr ( Data->canDetectStealthOn ) );
	
	if ( Data->surfacePosition != sUnitData::SURFACE_POS_NORMAL ) addAttributeElement ( unitNode, "Surface_Position", "num", iToStr ( Data->surfacePosition ) );
	if ( Data->canBeOverbuild != sUnitData::OVERBUILD_TYPE_NO ) addAttributeElement ( unitNode, "Can_Be_Overbuild", "num", iToStr ( Data->canBeOverbuild ) );

	if( Data->factorAir != 0.0 ) addAttributeElement ( unitNode, "Factor_Air", "num", dToStr ( Data->factorAir ) );
	if( Data->factorCoast != 0.0 ) addAttributeElement ( unitNode, "Factor_Coast", "num", dToStr ( Data->factorCoast ) );
	if( Data->factorGround != 0.0 ) addAttributeElement ( unitNode, "Factor_Ground", "num", dToStr ( Data->factorGround ) );
	if( Data->factorSea != 0.0 ) addAttributeElement ( unitNode, "Factor_Sea", "num", dToStr ( Data->factorSea ) );
	
	if( Data->modifiesSpeed != 0.0 ) addAttributeElement ( unitNode, "Factor_Sea", "num", dToStr ( Data->modifiesSpeed ) );

	if( Data->canBuildPath ) addMainElement ( unitNode, "Can_Build_Path" );
	if( Data->canBuildRepeat ) addMainElement ( unitNode, "Can_Build_Repeat" );
	if( Data->buildIntern ) addMainElement ( unitNode, "Build_Intern" );
	if( Data->connectsToBase ) addMainElement ( unitNode, "Connects_To_Base" );
	
	if( Data->canBeCaptured ) addMainElement ( unitNode, "Can_Be_Captured" );
	if( Data->canBeDisabled ) addMainElement ( unitNode, "Can_Be_Disabled" );
	if( Data->canCapture ) addMainElement ( unitNode, "Can_Capture" );
	if( Data->canDisable ) addMainElement ( unitNode, "Can_Disable" );
	if( Data->canSurvey ) addMainElement ( unitNode, "Can_Survey" );
	if( Data->doesSelfRepair ) addMainElement ( unitNode, "Does_Self_Repair" );
	if( Data->canSelfDestroy ) addMainElement ( unitNode, "Can_Self_Destroy" );
	if( Data->explodesOnContact ) addMainElement ( unitNode, "Explodes_On_Contact" );
	if( Data->isHuman ) addMainElement ( unitNode, "Is_Human" );
	if( Data->canBeLandedOn ) addMainElement ( unitNode, "Can_Be_Landed_On" );
	if( Data->canWork ) addMainElement ( unitNode, "Can_Work" );

	if( Data->isBig ) addMainElement ( unitNode, "Is_Big" );
	if( Data->canRepair ) addMainElement ( unitNode, "Can_Repair" );
	if( Data->canRearm ) addMainElement ( unitNode, "Can_Rearm" );
	if( Data->canResearch ) addMainElement ( unitNode, "Can_Research" );
	if( Data->canClearArea ) addMainElement ( unitNode, "Can_Clear" );
	if( Data->canPlaceMines ) addMainElement ( unitNode, "Can_Place_Mines" );
	if( Data->canDriveAndFire ) addMainElement ( unitNode, "Can_Drive_And_Fire" );
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
