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
#include "menuevents.h"
#include "netmessage.h"
#include "serverevents.h"

void sendMenuChatMessage ( string chatMsg, sMenuPlayer *player )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_CHAT );
	message->pushString ( chatMsg );
	cMenu::sendMessage ( message, player );
}

void sendRequestIdentification ( sMenuPlayer *player )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_REQ_IDENTIFIKATION );
	message->pushInt16 ( player->nr );
	cMenu::sendMessage ( message, player );
}

void sendPlayerList ( cList<sMenuPlayer*> *players )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_PLAYERLIST );

	for ( int i = (int)players->Size()-1; i >= 0; i-- )
	{
		const sMenuPlayer *player = (*players)[i];
		message->pushInt16( player->nr );
		message->pushBool ( player->ready );
		message->pushInt16( player->color );
		message->pushString( player->name );
	}
	message->pushInt16 ( (int)players->Size() );
	cMenu::sendMessage ( message );
}

void sendGameData ( cGameDataContainer *gameData, string saveGameString, sMenuPlayer *player )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_OPTINS );

	if ( !gameData->savegame.empty() ) message->pushString ( saveGameString );
	message->pushBool ( !gameData->savegame.empty() );

	if ( gameData->map ) message->pushString ( gameData->map->MapName );
	message->pushBool ( gameData->map != NULL );

	if ( gameData->settings )
	{
		message->pushChar ( gameData->settings->gameType );
		message->pushChar ( gameData->settings->alienTech );
		message->pushChar ( gameData->settings->bridgeHead );
		message->pushChar ( gameData->settings->credits );
		message->pushChar ( gameData->settings->resFrequency );
		message->pushChar ( gameData->settings->gold );
		message->pushChar ( gameData->settings->oil );
		message->pushChar ( gameData->settings->metal );
	}
	message->pushBool ( gameData->settings != NULL );

	cMenu::sendMessage ( message, player );
}

void sendGo ()
{
	cNetMessage *message = new cNetMessage ( MU_MSG_GO );
	cMenu::sendMessage ( message );
}

void sendIdentification ( sMenuPlayer *player )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_IDENTIFIKATION );
	message->pushBool ( player->ready );
	message->pushString ( player->name );
	message->pushInt16( player->color );
	message->pushInt16( player->nr );
	cMenu::sendMessage ( message );
}

void sendLandingUnits ( cList<sLandingUnit> *landingList, int ownerNr )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_LANDING_VEHICLES );

	for ( unsigned int i = 0; i < landingList->Size(); i++ )
	{
		message->pushInt16( (*landingList)[i].unitID.iSecondPart );
		message->pushInt16( (*landingList)[i].unitID.iFirstPart );
		message->pushInt16( (*landingList)[i].cargo );
	}
	message->pushInt16( (int)landingList->Size() );
	message->pushInt16( ownerNr );

	// the host has not to send the message over tcpip and since he has
	// always the playernumber 0 we can handle the message directly
	if ( ownerNr == 0 && ActiveMenu )
	{
		ActiveMenu->handleNetMessage ( message );
		delete message;
	}
	else cMenu::sendMessage ( message );
}

void sendUnitUpgrades ( cPlayer *player )
{
	cNetMessage *message = NULL;
	int count = 0;

	// send vehicles
	for (size_t i = 0; i < UnitsData.vehicle.Size(); ++i)
	{
		if ( message == NULL )
		{
			message = new cNetMessage ( MU_MSG_UPGRADES );
		}
		if ( player->VehicleData[i].damage != UnitsData.vehicle[i].data.damage ||
			player->VehicleData[i].max_shots != UnitsData.vehicle[i].data.max_shots ||
			player->VehicleData[i].range != UnitsData.vehicle[i].data.range ||
			player->VehicleData[i].max_ammo != UnitsData.vehicle[i].data.max_ammo ||
			player->VehicleData[i].armor != UnitsData.vehicle[i].data.armor ||
			player->VehicleData[i].max_hit_points != UnitsData.vehicle[i].data.max_hit_points ||
			player->VehicleData[i].scan != UnitsData.vehicle[i].data.scan ||
			player->VehicleData[i].max_speed != UnitsData.vehicle[i].data.max_speed )
		{
			message->pushInt16( player->VehicleData[i].max_speed );
			message->pushInt16( player->VehicleData[i].scan );
			message->pushInt16( player->VehicleData[i].max_hit_points );
			message->pushInt16( player->VehicleData[i].armor );
			message->pushInt16( player->VehicleData[i].max_ammo );
			message->pushInt16( player->VehicleData[i].range );
			message->pushInt16( player->VehicleData[i].max_shots );
			message->pushInt16( player->VehicleData[i].damage );
			message->pushInt16( player->VehicleData[i].ID.iSecondPart );
			message->pushInt16( player->VehicleData[i].ID.iFirstPart );
			message->pushBool( true ); // true for vehciles

			count++;
		}

		if ( message->iLength+38 > PACKAGE_LENGTH )
		{
			message->pushInt16 ( count );
			message->pushInt16 ( player->Nr );
			if ( player->Nr == 0 && ActiveMenu )
			{
				ActiveMenu->handleNetMessage ( message );
				delete message;
			}
			else cMenu::sendMessage ( message );
			message = NULL;
			count = 0;
		}
	}
	if ( message != NULL )
	{
		message->pushInt16 ( count );
		message->pushInt16 ( player->Nr );
			if ( player->Nr == 0 && ActiveMenu )
			{
				ActiveMenu->handleNetMessage ( message );
				delete message;
			}
			else cMenu::sendMessage ( message );
		message = NULL;
		count = 0;
	}

	// send buildings
	for (size_t i = 0; i < UnitsData.building.Size(); ++i)
	{
		if ( message == NULL )
		{
			message = new cNetMessage ( MU_MSG_UPGRADES );
		}
		if ( player->BuildingData[i].damage != UnitsData.building[i].data.damage ||
			player->BuildingData[i].max_shots != UnitsData.building[i].data.max_shots ||
			player->BuildingData[i].range != UnitsData.building[i].data.range ||
			player->BuildingData[i].max_ammo != UnitsData.building[i].data.max_ammo ||
			player->BuildingData[i].armor != UnitsData.building[i].data.armor ||
			player->BuildingData[i].max_hit_points != UnitsData.building[i].data.max_hit_points ||
			player->BuildingData[i].scan != UnitsData.building[i].data.scan )
		{
			message->pushInt16( player->BuildingData[i].scan );
			message->pushInt16( player->BuildingData[i].max_hit_points );
			message->pushInt16( player->BuildingData[i].armor );
			message->pushInt16( player->BuildingData[i].max_ammo );
			message->pushInt16( player->BuildingData[i].range );
			message->pushInt16( player->BuildingData[i].max_shots );
			message->pushInt16( player->BuildingData[i].damage );
			message->pushInt16( player->BuildingData[i].ID.iSecondPart );
			message->pushInt16( player->BuildingData[i].ID.iFirstPart );
			message->pushBool( false ); // false for buildings

			count++;
		}

		if ( message->iLength+34 > PACKAGE_LENGTH )
		{
			message->pushInt16 ( count );
			message->pushInt16 ( player->Nr );
			if ( player->Nr == 0 && ActiveMenu )
			{
				ActiveMenu->handleNetMessage ( message );
				delete message;
			}
			else cMenu::sendMessage ( message );
			message = NULL;
			count = 0;
		}
	}
	if ( message != NULL )
	{
		message->pushInt16 ( count );
		message->pushInt16 ( player->Nr );
		if ( player->Nr == 0 && ActiveMenu )
		{
			ActiveMenu->handleNetMessage ( message );
			delete message;
		}
		else cMenu::sendMessage ( message );
	}
}

void sendLandingCoords ( sClientLandData& c, int ownerNr )
{
	Log.write("Client: sending landing coords", cLog::eLOG_TYPE_NET_DEBUG);
	cNetMessage* message = new cNetMessage( MU_MSG_LANDING_COORDS );
	message->pushInt16( c.iLandY );
	message->pushInt16( c.iLandX );
	message->pushChar( ownerNr );

	if ( ownerNr == 0 && ActiveMenu )
	{
		ActiveMenu->handleNetMessage ( message );
		delete message;
	}
	else cMenu::sendMessage( message );
}

void sendReselectLanding ( eLandingState state, int playerNr )
{
	cNetMessage* message = new cNetMessage(MU_MSG_RESELECT_LANDING);
	message->pushChar( state );
	
	if ( playerNr == 0 && ActiveMenu )
	{
		ActiveMenu->handleNetMessage ( message );
		delete message;
	}
	else cMenu::sendMessage( message );
}

void sendAllLanded ()
{
	cNetMessage* message = new cNetMessage( MU_MSG_ALL_LANDED );
	cMenu::sendMessage( message );
	message = new cNetMessage( MU_MSG_ALL_LANDED );
	ActiveMenu->handleNetMessage ( message );
	delete message;
}

void sendGameIdentification ( sMenuPlayer *player, int socket )
{
	cNetMessage *message = new cNetMessage ( GAME_EV_IDENTIFICATION );
	message->pushInt16 ( socket );
	message->pushString ( player->name );
	cMenu::sendMessage( message );
}

void sendReconnectionSuccess( int playerNr )
{
	cNetMessage *message = new cNetMessage ( GAME_EV_RECON_SUCESS );
	message->pushInt16 ( playerNr );
	cMenu::sendMessage ( message );
}
