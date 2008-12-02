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

cSavegame::cSavegame ( int number )
{
	this->number = number;
	if ( number > 100 ) return;
	sprintf ( numberstr, "%0.3d", number );
}

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
			writeUnit ( Vehicle, unitnum );
			unitnum++;
			Vehicle = Vehicle->next;
		}
		cBuilding *Building = Player->BuildingList;
		while ( Building )
		{
			writeUnit ( Building, unitnum );
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

	SaveFile->SaveFile( ( SettingsData.sSavesPath + PATH_DELIMITER + "Save" + numberstr + ".xml" ).c_str() );

	delete SaveFile;
	return 1;
}

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
		// warning
	}

	cMap *map = loadMap();
	cList<cPlayer *> *PlayerList = loadPlayers ( map );

	string gametype;
	loadHeader ( NULL, &gametype, NULL );
	if ( !gametype.compare ( "IND" )  ) Server = new cServer ( map, PlayerList, GAME_TYPE_SINGLE, false );
	else if ( !gametype.compare ( "HOT" )  ) Server = new cServer ( map, PlayerList, GAME_TYPE_HOTSEAT, false );
	else if ( !gametype.compare ( "NET" )  ) Server = new cServer ( map, PlayerList, GAME_TYPE_TCPIP, false );
	else
	{
		cLog::write ( "Unknown gametype \"" + gametype + "\". Starting as singleplayergame.", cLog::eLOG_TYPE_INFO );
		Server = new cServer ( map, PlayerList, GAME_TYPE_SINGLE, false );
	}

	loadGameInfo();
	loadUnits ();

	delete SaveFile;
	return 1;
}

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

string cSavegame::getMapName()
{
	TiXmlElement *mapNode = SaveFile->RootElement()->FirstChildElement( "Map" );
	if ( mapNode != NULL ) return mapNode->FirstChildElement( "Name" )->Attribute ( "string" );
	else return "";
}

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

cMap *cSavegame::loadMap()
{
	TiXmlElement *mapNode = SaveFile->RootElement()->FirstChildElement( "Map" );
	if ( mapNode != NULL )
	{
		cMap *map = new cMap;
		string name = mapNode->FirstChildElement( "Name" )->Attribute ( "string" );
		string resourcestr = mapNode->FirstChildElement( "Resources" )->Attribute ( "data" );
		map->LoadMap ( name );

		convertStringToData ( resourcestr, map->size*map->size, map->Resources );
		return map;
	}
	else return NULL;
}

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
		if ( hudNode->FirstChildElement( "Radar" ) ) Player->HotHud.Radar = true;
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

	// TODO: load researches

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

void cSavegame::loadUpgrade ( TiXmlElement *upgradeNode, sUnitData *data )
{
	upgradeNode->FirstChildElement( "Version" )->Attribute ( "num", &data->version );
	TiXmlElement *element;
	if ( element = upgradeNode->FirstChildElement( "Ammo" ) ) element->Attribute ( "num", &data->max_ammo );
	if ( element = upgradeNode->FirstChildElement( "HitPoints" ) ) element->Attribute ( "num", &data->max_hit_points );
	if ( element = upgradeNode->FirstChildElement( "Shots" ) ) element->Attribute ( "num", &data->max_shots );
	if ( element = upgradeNode->FirstChildElement( "Speed" ) ) element->Attribute ( "num", &data->max_speed );
	if ( element = upgradeNode->FirstChildElement( "Armor" ) ) element->Attribute ( "num", &data->armor );
	if ( element = upgradeNode->FirstChildElement( "Costs" ) ) element->Attribute ( "num", &data->costs );
	if ( element = upgradeNode->FirstChildElement( "Demage" ) ) element->Attribute ( "num", &data->damage );
	if ( element = upgradeNode->FirstChildElement( "Range" ) ) element->Attribute ( "num", &data->range );
	if ( element = upgradeNode->FirstChildElement( "Scan" ) ) element->Attribute ( "num", &data->scan );
}

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
		int rubblenum = 0;
		TiXmlElement *rubbleNode = unitsNode->FirstChildElement( "Rubble_0" );
		while ( rubbleNode )
		{
			loadRubble ( rubbleNode );
			rubblenum++;
			rubbleNode = unitsNode->FirstChildElement( ("Rubble_" + iToStr ( rubblenum )).c_str() );
		}
		generateMoveJobs();

		int nextID;
		unitsNode->FirstChildElement( "NextUnitID" )->Attribute ( "num", &nextID );
		Server->iNextUnitID = nextID;
	}
}

void cSavegame::loadVehicle( TiXmlElement *unitNode, sID &ID )
{
	if ( !Server ) return;
	int tmpinteger, number, x, y;
	for ( int i = 0; i < UnitsData.vehicle.Size(); i++ )
	{
		if ( UnitsData.vehicle[i].data.ID == ID )
		{
			number = i;
			break;
		}
	}
	unitNode->FirstChildElement( "Owner" )->Attribute ( "num", &tmpinteger );
	cPlayer *owner = getPlayerFromNumber ( Server->PlayerList, tmpinteger );

	unitNode->FirstChildElement( "Position" )->Attribute ( "x", &x );
	unitNode->FirstChildElement( "Position" )->Attribute ( "y", &y );
	cVehicle *vehicle = Server->addUnit ( x, y, &UnitsData.vehicle[number], owner );

	unitNode->FirstChildElement( "ID" )->Attribute ( "num", &tmpinteger );
	vehicle->iID = tmpinteger;
	vehicle->name = unitNode->FirstChildElement( "Name" )->Attribute ( "string" );

	loadUnitValues ( unitNode, &vehicle->data );

	TiXmlElement *element;
	unitNode->FirstChildElement( "Direction" )->Attribute ( "num", &vehicle->dir );
	if ( unitNode->FirstChildElement( "IsBig" ) ) vehicle->data.is_big = true;
	if ( unitNode->FirstChildElement( "Disabled" ) ) vehicle->Disabled = true;
	if ( unitNode->FirstChildElement( "LayMines" ) ) vehicle->LayMines = true;
	if ( unitNode->FirstChildElement( "OnSentry" ) )
	{
		vehicle->bSentryStatus = true;
		owner->addSentryVehicle ( vehicle );
	}

	if ( element = unitNode->FirstChildElement( "Building" ) )
	{
		vehicle->IsBuilding = true;
		element->Attribute ( "type", &vehicle->BuildingTyp );
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
}

void cSavegame::loadBuilding( TiXmlElement *unitNode, sID &ID )
{
	if ( !Server ) return;
	int tmpinteger, number, x, y;
	for ( int i = 0; i < UnitsData.building.Size(); i++ )
	{
		if ( UnitsData.building[i].data.ID == ID )
		{
			number = i;
			break;
		}
	}
	unitNode->FirstChildElement( "Owner" )->Attribute ( "num", &tmpinteger );
	cPlayer *owner = getPlayerFromNumber ( Server->PlayerList, tmpinteger );

	unitNode->FirstChildElement( "Position" )->Attribute ( "x", &x );
	unitNode->FirstChildElement( "Position" )->Attribute ( "y", &y );
	cBuilding *building = Server->addUnit ( x, y, &UnitsData.building[number], owner );

	unitNode->FirstChildElement( "ID" )->Attribute ( "num", &tmpinteger );
	building->iID = tmpinteger;
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
			int typenr;
			element->Attribute ( "type", &typenr );
			listitem->typ = &UnitsData.vehicle[typenr];
			element->Attribute ( "metall_remaining", &listitem->metall_remaining );
			building->BuildList->Add ( listitem );

			itemnum++;
			element = buildNode->FirstChildElement( "BuildList" )->FirstChildElement( ("Item_" + iToStr ( itemnum )).c_str() );
		}
	}
}

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

void cSavegame::loadUnitValues ( TiXmlElement *unitNode, sUnitData *Data )
{
	TiXmlElement *Element;
	if ( Element = unitNode->FirstChildElement( "Hitpoints" ) ) Element->Attribute ( "num", &Data->hit_points );
	if ( Element = unitNode->FirstChildElement( "Ammo" ) ) Element->Attribute ( "num", &Data->ammo );
	if ( Element = unitNode->FirstChildElement( "Cargo" ) ) Element->Attribute ( "num", &Data->cargo );
	if ( Element = unitNode->FirstChildElement( "Speed" ) ) Element->Attribute ( "num", &Data->speed );
	if ( Element = unitNode->FirstChildElement( "Shots" ) ) Element->Attribute ( "num", &Data->shots );

	if ( Element = unitNode->FirstChildElement( "Version" ) ) Element->Attribute ( "num", &Data->version );
	if ( Data->version > 1 )
	{
		if ( Element = unitNode->FirstChildElement( "Max_Hitpoints" ) ) Element->Attribute ( "num", &Data->max_hit_points );
		if ( Element = unitNode->FirstChildElement( "Max_Ammo" ) ) Element->Attribute ( "num", &Data->max_ammo );
		if ( Element = unitNode->FirstChildElement( "Max_Speed" ) ) Element->Attribute ( "num", &Data->max_speed );
		if ( Element = unitNode->FirstChildElement( "Max_Shots" ) ) Element->Attribute ( "num", &Data->max_shots );
		if ( Element = unitNode->FirstChildElement( "Armor" ) ) Element->Attribute ( "num", &Data->armor );
		if ( Element = unitNode->FirstChildElement( "Damage" ) ) Element->Attribute ( "num", &Data->damage );
		if ( Element = unitNode->FirstChildElement( "Range" ) ) Element->Attribute ( "num", &Data->range );
		if ( Element = unitNode->FirstChildElement( "Scan" ) ) Element->Attribute ( "num", &Data->scan );
	}
}

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

cPlayer *cSavegame::getPlayerFromNumber ( cList<cPlayer*> *PlayerList, int number )
{
	for ( unsigned int i = 0; i < PlayerList->Size(); i++ )
	{
		if ( (*PlayerList)[i]->Nr == number ) return (*PlayerList)[i];
	}
	return NULL;
}

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

void cSavegame::convertStringToData ( string str, int size, sResources *resources )
{
	for ( int i = 0; i < size; i++ )
	{
		if ( i == 79 )
			int haha = 0;
		resources[i].typ = getByteValue ( str.substr( i*4, 2 ).c_str() );
		resources[i].value = getByteValue ( str.substr( i*4+2, 2 ).c_str() );
	}
}

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

void cSavegame::convertStringToScanMap ( string str, char *data )
{
	for ( int i = 0; i < str.length(); i++ )
	{
		if ( !str.substr ( i, 1 ).compare ( "1" ) ) data[i] = 1;
		else data[i] = 0;
	}
}

void cSavegame::writeHeader( string saveName )
{
	TiXmlElement *headerNode = addMainElement ( SaveFile->RootElement(), "Header" );

	addAttributeElement ( headerNode, "Game_Version", "string", MAXVERSION );
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

void cSavegame::writeGameInfo()
{
	TiXmlElement *gemeinfoNode = addMainElement ( SaveFile->RootElement(), "Game" );

	addAttributeElement ( gemeinfoNode, "Turn", "num", iToStr ( Server->iTurn ) );
	if ( Server->bHotSeat ) addAttributeElement ( gemeinfoNode, "Hotseat", "activeplayer", iToStr ( Server->iHotSeatPlayer ) );
	if ( Server->bPlayTurns ) addAttributeElement ( gemeinfoNode, "PlayTurns", "activeplayer", iToStr ( Server->iActiveTurnPlayerNr ) );
}

void cSavegame::writeMap( cMap *Map )
{
	TiXmlElement *mapNode = addMainElement ( SaveFile->RootElement(), "Map" );
	addAttributeElement ( mapNode, "Name", "string", Map->MapName );
	addAttributeElement ( mapNode, "Resources", "data", convertDataToString ( Map->Resources, Map->size*Map->size ) );
}

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
		if ( Client->Hud.Radar ) addMainElement ( hudNode, "Radar" );
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
	// TODO: implement researches
	TiXmlElement *researchesNode = addMainElement ( playerNode, "Researches" );

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
	if ( data->costs != originaldata->costs ) addAttributeElement ( upgradeNode, "Costs", "num", iToStr ( data->costs ) );
	if ( data->damage != originaldata->damage ) addAttributeElement ( upgradeNode, "Demage", "num", iToStr ( data->damage ) );
	if ( data->range != originaldata->range ) addAttributeElement ( upgradeNode, "Range", "num", iToStr ( data->range ) );
	if ( data->scan != originaldata->scan ) addAttributeElement ( upgradeNode, "Scan", "num", iToStr ( data->scan ) );
}

void cSavegame::writeUnit ( cVehicle *Vehicle, int unitnum )
{
	// add units node if it doesn't exists
	TiXmlElement *unitsNode;
	if ( !(unitsNode = SaveFile->RootElement()->FirstChildElement ( "Units" )) )
	{
		unitsNode = addMainElement ( SaveFile->RootElement(), "Units" );
		addAttributeElement ( unitsNode, "NextUnitID", "num", iToStr ( Server->iNextUnitID ) );
	}

	// add the unit node
	TiXmlElement *unitNode = addMainElement ( unitsNode, "Unit_" + iToStr( unitnum ) );

	// write main information
	addAttributeElement ( unitNode, "Type", "string", Vehicle->data.ID.getText() );
	addAttributeElement ( unitNode, "ID", "num", iToStr ( Vehicle->iID ) );
	addAttributeElement ( unitNode, "Owner", "num", iToStr ( Vehicle->owner->Nr ) );
	addAttributeElement ( unitNode, "Position", "x", iToStr ( Vehicle->PosX ), "y", iToStr ( Vehicle->PosY ) );
	addAttributeElement ( unitNode, "Name", "string", Vehicle->name );

	// write the standard unit values which are the same for vehicles and buildings
	writeUnitValues ( unitNode, &Vehicle->data, &Vehicle->owner->VehicleData[Vehicle->typ->nr] );

	// add additional status information
	addAttributeElement ( unitNode, "Direction", "num", iToStr ( Vehicle->dir ).c_str() );
	if ( Vehicle->data.is_big ) addMainElement ( unitNode, "IsBig" );
	if ( Vehicle->Disabled ) addMainElement ( unitNode, "Disabled" );
	if ( Vehicle->LayMines ) addMainElement ( unitNode, "LayMines" );
	if ( Vehicle->bSentryStatus ) addMainElement ( unitNode, "OnSentry" );

	if ( Vehicle->IsBuilding )
	{
		TiXmlElement *element = addMainElement ( unitNode, "Building" );
		element->SetAttribute ( "type", iToStr ( Vehicle->BuildingTyp ).c_str() );
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
}

void cSavegame::writeUnit ( cBuilding *Building, int unitnum )
{
	// add units node if it doesn't exists
	TiXmlElement *unitsNode;
	if ( !(unitsNode = SaveFile->RootElement()->FirstChildElement ( "Units" )) )
	{
		unitsNode = addMainElement ( SaveFile->RootElement(), "Units" );
		addAttributeElement ( unitsNode, "NextUnitID", "num", iToStr ( Server->iNextUnitID ) );
	}

	// add the unit node
	TiXmlElement *unitNode = addMainElement ( unitsNode, "Unit_" + iToStr( unitnum ) );

	// write main information
	addAttributeElement ( unitNode, "Type", "string", Building->data.ID.getText() );
	addAttributeElement ( unitNode, "ID", "num", iToStr ( Building->iID ) );
	addAttributeElement ( unitNode, "Owner", "num", iToStr ( Building->owner->Nr ) );
	addAttributeElement ( unitNode, "Position", "x", iToStr ( Building->PosX ), "y", iToStr ( Building->PosY ) );
	addAttributeElement ( unitNode, "Name", "string", Building->name );
	if ( Building->SubBase ) addAttributeElement ( unitNode, "SubBase", "num", iToStr (Building->SubBase->iID ) );

	// write the standard values
	writeUnitValues ( unitNode, &Building->data, &Building->owner->BuildingData[Building->typ->nr] );

	// write additional stauts information
	if ( Building->IsWorking ) addMainElement ( unitNode, "IsWorking" );
	if ( Building->bSentryStatus ) addMainElement ( unitNode, "OnSentry" );

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
			addAttributeElement ( buildlistNode, "Item_" + iToStr ( i ), "type", iToStr ((*Building->BuildList)[i]->typ->nr ), "metall_remaining", iToStr ((*Building->BuildList)[i]->metall_remaining ) );
		}
	}
}

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

void cSavegame::writeUnitValues ( TiXmlElement *unitNode, sUnitData *Data, sUnitData *OwnerData )
{
	// write the standard status values
	if ( Data->hit_points != Data->max_hit_points ) addAttributeElement ( unitNode, "Hitpoints", "num", iToStr ( Data->hit_points ) );
	if ( Data->ammo != Data->max_ammo ) addAttributeElement ( unitNode, "Ammo", "num", iToStr ( Data->ammo ) );
	if ( Data->cargo > 0 ) addAttributeElement ( unitNode, "Cargo", "num", iToStr ( Data->cargo ) );
	if ( Data->speed != Data->max_speed ) addAttributeElement ( unitNode, "Speed", "num", iToStr ( Data->speed ) );
	if ( Data->shots != Data->max_shots ) addAttributeElement ( unitNode, "Shots", "num", iToStr ( Data->shots ) );

	// write upgrade values that differ from the acctual unit values of the owner
	if ( Data->version > 1 )
	{
		addAttributeElement ( unitNode, "version", "num", iToStr ( Data->version ) );
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

void cSavegame::addAttributeElement( TiXmlElement *node, string nodename, string attributename, string value, string attributename2, string value2 )
{
	TiXmlElement *element = addMainElement ( node, nodename );
	element->SetAttribute ( attributename.c_str(), value.c_str() );
	if ( attributename2.compare("") ) element->SetAttribute ( attributename2.c_str(), value2.c_str() );
}

TiXmlElement *cSavegame::addMainElement( TiXmlElement *node, string nodename )
{
	TiXmlElement *element = new TiXmlElement ( nodename.c_str() );
	node->LinkEndChild( element );
	return element;
}
