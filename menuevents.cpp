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
#include "mapdownload.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"

using namespace std;

void sendMenuChatMessage (cTCP& network, const string& chatMsg, const sMenuPlayer* player, int fromPlayerNr, bool translationText)
{
	cNetMessage* message = new cNetMessage (MU_MSG_CHAT);
	message->pushString (chatMsg);
	message->pushBool (translationText);
	cMenu::sendMessage (network, message, player, fromPlayerNr);
}

void sendRequestIdentification (cTCP& network, const sMenuPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_REQ_IDENTIFIKATION);
	message->pushInt16 (player.nr);
	message->pushString (string (PACKAGE_VERSION) + " " + PACKAGE_REV);
	cMenu::sendMessage (network, message, &player);
}

void sendPlayerList (cTCP& network, const std::vector<sMenuPlayer*>& players)
{
	cNetMessage* message = new cNetMessage (MU_MSG_PLAYERLIST);

	for (int i = (int) players.size() - 1; i >= 0; i--)
	{
		const sMenuPlayer* player = players[i];
		message->pushInt16 (player->nr);
		message->pushBool (player->ready);
		message->pushInt16 (player->color);
		message->pushString (player->name);
	}
	message->pushInt16 ((int) players.size());
	cMenu::sendMessage (network, message);
}

void sendGameData (cTCP& network, const cGameDataContainer& gameData, const string& saveGameString, const sMenuPlayer* player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_OPTINS);

	if (!gameData.savegame.empty()) message->pushString (saveGameString);
	message->pushBool (!gameData.savegame.empty());

	if (gameData.map)
	{
		message->pushInt32 (MapDownload::calculateCheckSum (gameData.map->MapName));
		message->pushString (gameData.map->MapName);
	}
	message->pushBool (gameData.map != NULL);

	if (gameData.settings)
	{
		message->pushChar (gameData.settings->gameType);
		message->pushChar (gameData.settings->clans);
		message->pushChar (gameData.settings->alienTech);
		message->pushChar (gameData.settings->bridgeHead);
		message->pushInt16 (gameData.settings->credits);
		message->pushChar (gameData.settings->resFrequency);
		message->pushChar (gameData.settings->gold);
		message->pushChar (gameData.settings->oil);
		message->pushChar (gameData.settings->metal);
		message->pushChar (gameData.settings->victoryType);
		message->pushInt16 (gameData.settings->duration);
	}
	message->pushBool (gameData.settings != NULL);

	cMenu::sendMessage (network, message, player);
}

void sendGo (cTCP& network)
{
	cNetMessage* message = new cNetMessage (MU_MSG_GO);
	cMenu::sendMessage (network, message);
}

void sendIdentification (cTCP& network, const sMenuPlayer& player)
{
	cNetMessage* message = new cNetMessage (MU_MSG_IDENTIFIKATION);
	message->pushString (string (PACKAGE_VERSION) + " " + PACKAGE_REV);
	message->pushBool (player.ready);
	message->pushString (player.name);
	message->pushInt16 (player.color);
	message->pushInt16 (player.nr);
	cMenu::sendMessage (network, message);
}

void sendClan (cTCP& network, int clanNr, int ownerNr)
{
	cNetMessage* message = new cNetMessage (MU_MSG_CLAN);

	message->pushInt16 (clanNr);
	message->pushInt16 (ownerNr);

	// the host has not to send the message over tcpip and we can handle the message directly
	cMenu::sendMessage (network, message);
}

void sendLandingUnits (cTCP& network, const std::vector<sLandingUnit>& landingList, int ownerNr)
{
	cNetMessage* message = new cNetMessage (MU_MSG_LANDING_VEHICLES);

	for (unsigned int i = 0; i < landingList.size(); i++)
	{
		message->pushInt16 (landingList[i].unitID.iSecondPart);
		message->pushInt16 (landingList[i].unitID.iFirstPart);
		message->pushInt16 (landingList[i].cargo);
	}
	message->pushInt16 ( (int) landingList.size());
	message->pushInt16 (ownerNr);

	// the host has not to send the message over tcpip and we can handle the message directly
	cMenu::sendMessage (network, message);
}

void sendUnitUpgrades (cTCP* network, const cPlayer& player, cMenu* activeMenu)
{
	cNetMessage* message = NULL;
	int count = 0;

	// send vehicles
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); ++i)
	{
		if (message == NULL)
		{
			message = new cNetMessage (MU_MSG_UPGRADES);
		}
		const sUnitData& playerData = player.VehicleData[i];
		const sUnitData& originalData = UnitsData.getVehicle (i, player.getClan()).data;
		if (playerData.damage != originalData.damage ||
			playerData.shotsMax != originalData.shotsMax ||
			playerData.range != originalData.range ||
			playerData.ammoMax != originalData.ammoMax ||
			playerData.armor != originalData.armor ||
			playerData.hitpointsMax != originalData.hitpointsMax ||
			playerData.scan != originalData.scan ||
			playerData.speedMax != originalData.speedMax)
		{
			message->pushInt16 (playerData.speedMax);
			message->pushInt16 (playerData.scan);
			message->pushInt16 (playerData.hitpointsMax);
			message->pushInt16 (playerData.armor);
			message->pushInt16 (playerData.ammoMax);
			message->pushInt16 (playerData.range);
			message->pushInt16 (playerData.shotsMax);
			message->pushInt16 (playerData.damage);
			message->pushInt16 (playerData.ID.iSecondPart);
			message->pushInt16 (playerData.ID.iFirstPart);
			message->pushBool (true);  // true for vehciles

			count++;
		}

		if (message->iLength + 38 > PACKAGE_LENGTH)
		{
			message->pushInt16 (count);
			message->pushInt16 (player.Nr);
			if (activeMenu)
			{
				activeMenu->handleNetMessage (message);
				delete message;
			}
			else if (network) cMenu::sendMessage (*network, message);
			message = NULL;
			count = 0;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 (count);
		message->pushInt16 (player.Nr);
		if (activeMenu)
		{
			activeMenu->handleNetMessage (message);
			delete message;
		}
		else if (network) cMenu::sendMessage (*network, message);
		message = NULL;
		count = 0;
	}

	// send buildings
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		if (message == NULL)
		{
			message = new cNetMessage (MU_MSG_UPGRADES);
		}
		const sUnitData& playerData = player.BuildingData[i];
		const sUnitData& originalData = UnitsData.getBuilding (i, player.getClan()).data;
		if (playerData.damage != originalData.damage ||
			playerData.shotsMax != originalData.shotsMax ||
			playerData.range != originalData.range ||
			playerData.ammoMax != originalData.ammoMax ||
			playerData.armor != originalData.armor ||
			playerData.hitpointsMax != originalData.hitpointsMax ||
			playerData.scan != originalData.scan)
		{
			message->pushInt16 (playerData.scan);
			message->pushInt16 (playerData.hitpointsMax);
			message->pushInt16 (playerData.armor);
			message->pushInt16 (playerData.ammoMax);
			message->pushInt16 (playerData.range);
			message->pushInt16 (playerData.shotsMax);
			message->pushInt16 (playerData.damage);
			message->pushInt16 (playerData.ID.iSecondPart);
			message->pushInt16 (playerData.ID.iFirstPart);
			message->pushBool (false);  // false for buildings

			count++;
		}

		if (message->iLength + 34 > PACKAGE_LENGTH)
		{
			message->pushInt16 (count);
			message->pushInt16 (player.Nr);
			if (activeMenu)
			{
				activeMenu->handleNetMessage (message);
				delete message;
			}
			else if (network) cMenu::sendMessage (*network, message);
			message = NULL;
			count = 0;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 (count);
		message->pushInt16 (player.Nr);
		if (activeMenu)
		{
			activeMenu->handleNetMessage (message);
			delete message;
		}
		else if (network) cMenu::sendMessage (*network, message);
	}
}

void sendLandingCoords (cTCP& network, const sClientLandData& c, int ownerNr, cMenu* activeMenu)
{
	Log.write ("Client: sending landing coords", cLog::eLOG_TYPE_NET_DEBUG);
	cNetMessage* message = new cNetMessage (MU_MSG_LANDING_COORDS);
	message->pushInt16 (c.iLandY);
	message->pushInt16 (c.iLandX);
	message->pushChar (ownerNr);

	if (activeMenu)
	{
		activeMenu->handleNetMessage (message);
		delete message;
	}
	else cMenu::sendMessage (network, message);
}

void sendReselectLanding (cTCP& network, eLandingState state, const sMenuPlayer* player, cMenu* activeMenu)
{
	cNetMessage* message = new cNetMessage (MU_MSG_RESELECT_LANDING);
	message->pushChar (state);

	if (!DEDICATED_SERVER && player->nr == 0 && activeMenu)
	{
		activeMenu->handleNetMessage (message);
		delete message;
	}
	else cMenu::sendMessage (network, message, player);
}

void sendAllLanded (cTCP& network, cMenu* activeMenu)
{
	cNetMessage* message = new cNetMessage (MU_MSG_ALL_LANDED);
	cMenu::sendMessage (network, message);
	if (activeMenu)
	{
		message = new cNetMessage (MU_MSG_ALL_LANDED);
		activeMenu->handleNetMessage (message);
		delete message;
	}
}

void sendGameIdentification (cTCP& network, const sMenuPlayer& player, int socket)
{
	cNetMessage* message = new cNetMessage (GAME_EV_IDENTIFICATION);
	message->pushInt16 (socket);
	message->pushString (player.name);
	cMenu::sendMessage (network, message);
}

void sendReconnectionSuccess (cTCP& network, int playerNr)
{
	cNetMessage* message = new cNetMessage (GAME_EV_RECON_SUCESS);
	message->pushInt16 (playerNr);
	cMenu::sendMessage (network, message);
}

void sendRequestMap (cTCP& network, const string& mapName, int playerNr)
{
	cNetMessage* msg = new cNetMessage (MU_MSG_REQUEST_MAP);
	msg->pushString (mapName);
	msg->pushInt16 (playerNr);
	cMenu::sendMessage (network, msg);
}

static int findUpgradeValue (sUnitUpgrade upgrades[8], sUnitUpgrade::eUpgradeTypes upgradeType, int defaultValue)
{
	for (int i = 0; i < 8; ++i)
	{
		if (upgrades[i].active && upgrades[i].type == upgradeType)
			return upgrades[i].curValue;
	}
	return defaultValue; // the specified upgrade was not found...
}

void sendTakenUpgrades (const cClient& client, sUnitUpgrade (*unitUpgrades) [8], const cPlayer* player)
{
	cNetMessage* msg = NULL;
	int iCount = 0;

	for (unsigned int unitIndex = 0; unitIndex < UnitsData.getNrVehicles() + UnitsData.getNrBuildings(); unitIndex++)
	{
		sUnitUpgrade* curUpgrade = unitUpgrades[unitIndex];
		bool purchased = false;
		for (int i = 0; i < 8; i++)
		{
			if (curUpgrade[i].purchased)
			{
				purchased = true;
				break;
			}
		}

		if (!purchased) continue;

		if (msg == NULL)
		{
			msg = new cNetMessage (GAME_EV_WANT_BUY_UPGRADES);
			iCount = 0;
		}

		const sUnitData* currentVersion;
		if (unitIndex < UnitsData.getNrVehicles()) currentVersion = &player->VehicleData[unitIndex];
		else currentVersion = &player->BuildingData[unitIndex - UnitsData.getNrVehicles()];

		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_SCAN, currentVersion->scan));
		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_HITS, currentVersion->hitpointsMax));
		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_ARMOR, currentVersion->armor));
		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_AMMO, currentVersion->ammoMax));
		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_RANGE, currentVersion->range));
		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_SHOTS, currentVersion->shotsMax));
		msg->pushInt16 (findUpgradeValue (curUpgrade, sUnitUpgrade::UPGRADE_TYPE_DAMAGE, currentVersion->damage));
		msg->pushInt16 (currentVersion->ID.iSecondPart);
		msg->pushInt16 (currentVersion->ID.iFirstPart);

		iCount++; // msg contains one more upgrade struct

		// the msg would be too long, if another upgrade would be written into it. So send it and put the next upgrades in a new message.
		if (msg->iLength + 38 > PACKAGE_LENGTH)
		{
			msg->pushInt16 (iCount);
			msg->pushInt16 (player->Nr);
			client.sendNetMessage (msg);
			msg = NULL;
		}
	}

	if (msg != NULL)
	{
		msg->pushInt16 (iCount);
		msg->pushInt16 (player->Nr);
		client.sendNetMessage (msg);
	}
}
