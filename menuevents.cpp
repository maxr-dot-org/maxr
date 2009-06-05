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
#include "client.h"

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
		message->pushChar ( gameData->settings->clans );
		message->pushChar ( gameData->settings->alienTech );
		message->pushChar ( gameData->settings->bridgeHead );
		message->pushInt16 ( gameData->settings->credits );
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

void sendClan ( int clanNr, int ownerNr )
{
	cNetMessage *message = new cNetMessage ( MU_MSG_CLAN );
	
	message->pushInt16( clanNr );
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
	for (size_t i = 0; i < UnitsData.getNrVehicles (); ++i)
	{
		if ( message == NULL )
		{
			message = new cNetMessage ( MU_MSG_UPGRADES );
		}
		if ( player->VehicleData[i].damage != UnitsData.getVehicle (i, player->getClan ()).data.damage ||
			player->VehicleData[i].max_shots != UnitsData.getVehicle (i, player->getClan ()).data.max_shots ||
			player->VehicleData[i].range != UnitsData.getVehicle (i, player->getClan ()).data.range ||
			player->VehicleData[i].max_ammo != UnitsData.getVehicle (i, player->getClan ()).data.max_ammo ||
			player->VehicleData[i].armor != UnitsData.getVehicle (i, player->getClan ()).data.armor ||
			player->VehicleData[i].max_hit_points != UnitsData.getVehicle (i, player->getClan ()).data.max_hit_points ||
			player->VehicleData[i].scan != UnitsData.getVehicle (i, player->getClan ()).data.scan ||
			player->VehicleData[i].max_speed != UnitsData.getVehicle (i, player->getClan ()).data.max_speed )
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
	for (size_t i = 0; i < UnitsData.getNrBuildings (); ++i)
	{
		if ( message == NULL )
		{
			message = new cNetMessage ( MU_MSG_UPGRADES );
		}
		if ( player->BuildingData[i].damage != UnitsData.getBuilding (i, player->getClan ()).data.damage ||
			player->BuildingData[i].max_shots != UnitsData.getBuilding (i, player->getClan ()).data.max_shots ||
			player->BuildingData[i].range != UnitsData.getBuilding (i, player->getClan ()).data.range ||
			player->BuildingData[i].max_ammo != UnitsData.getBuilding (i, player->getClan ()).data.max_ammo ||
			player->BuildingData[i].armor != UnitsData.getBuilding (i, player->getClan ()).data.armor ||
			player->BuildingData[i].max_hit_points != UnitsData.getBuilding (i, player->getClan ()).data.max_hit_points ||
			player->BuildingData[i].scan != UnitsData.getBuilding (i, player->getClan ()).data.scan )
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

void sendReselectLanding ( eLandingState state, sMenuPlayer *player )
{
	cNetMessage* message = new cNetMessage(MU_MSG_RESELECT_LANDING);
	message->pushChar( state );
	
	if ( player->nr == 0 && ActiveMenu )
	{
		ActiveMenu->handleNetMessage ( message );
		delete message;
	}
	else cMenu::sendMessage( message, player );
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

void sendTakenUpgrades ( sUnitUpgrade (*unitUpgrades)[8], cPlayer *player )
{
	cNetMessage* msg = NULL;
	int iCount = 0;

	for ( unsigned int unitIndex = 0; unitIndex < UnitsData.getNrVehicles () + UnitsData.getNrBuildings (); unitIndex++ )
	{
		sUnitUpgrade *curUpgrade = unitUpgrades[unitIndex];
		bool purchased = false;
		for ( int i = 0; i < 8; i++ )
		{
			if ( curUpgrade[i].purchased )
			{
				purchased = true;
				break;
			}
		}

		if (purchased)
		{
			if (msg == NULL)
			{
				msg = new cNetMessage (GAME_EV_WANT_BUY_UPGRADES);
				iCount = 0;
			}
			
			sUnitData *currentVersion;
			if ( unitIndex < UnitsData.getNrVehicles () ) currentVersion = &player->VehicleData[unitIndex];
			else currentVersion = &player->BuildingData[unitIndex - UnitsData.getNrVehicles ()];

			msg->pushInt16 (findUpgradeValue (curUpgrade, 0, currentVersion->max_speed));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 1, currentVersion->scan));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 2, currentVersion->max_hit_points));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 3, currentVersion->armor));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 4, currentVersion->max_ammo));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 5, currentVersion->range));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 6, currentVersion->max_shots));
			msg->pushInt16 (findUpgradeValue (curUpgrade, 7, currentVersion->damage));
			msg->pushInt16 (currentVersion->ID.iSecondPart);
			msg->pushInt16 (currentVersion->ID.iFirstPart);

			iCount++; // msg contains one more upgrade struct
			
			// the msg would be too long, if another upgrade would be written into it. So send it and put the next upgrades in a new message.
			if (msg->iLength + 38 > PACKAGE_LENGTH) 
			{
				msg->pushInt16 (iCount);
				msg->pushInt16 (player->Nr);
				Client->sendNetMessage (msg);
				msg = NULL;
			}
			
		}
	}
	if (msg != NULL)
	{
		msg->pushInt16 (iCount);
		msg->pushInt16 (player->Nr);
		Client->sendNetMessage (msg);
	}
}

int findUpgradeValue ( sUnitUpgrade upgrades[8], int upgradeType, int defaultValue )
{
	switch (upgradeType)
	{
		case 0:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_SPEED )
					return upgrades[i].curValue;
			}
			break;
		case 1:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_SCAN )
					return upgrades[i].curValue;
			}
			break;
		case 2:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_HITS )
					return upgrades[i].curValue;
			}
			break;
		case 3:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_ARMOR )
					return upgrades[i].curValue;
			}
			break;
		case 4:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_AMMO )
					return upgrades[i].curValue;
			}
			break;
		case 5:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_RANGE )
					return upgrades[i].curValue;
			}
			break;
		case 6:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_SHOTS )
					return upgrades[i].curValue;
			}
			break;
		case 7:
			for (int i = 0; i < 8; i++)
			{
				if (upgrades[i].active && upgrades[i].type == sUnitUpgrade::UPGRADE_TYPE_DAMAGE )
					return upgrades[i].curValue;
			}
			break;
	}
	return defaultValue; // the specified upgrade was not found...
}
