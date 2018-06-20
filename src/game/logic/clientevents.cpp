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

#include "game/logic/clientevents.h"

#include "game/data/units/building.h"
#include "game/data/units/landingunit.h"
#include "game/logic/client.h"
#include "utility/log.h"
#include "netmessage.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/data/units/vehicle.h"
#include "utility/position.h"
#include "game/data/report/savedreport.h"
#include "ui/graphical/game/gameguistate.h"
#include "../data/report/savedreportchat.h"

using namespace std;




void sendChatMessageToServer (const cClient& client, const string& msg, const cPlayer& player)
{
	cNetMessageReport netMsg(std::make_unique<cSavedReportChat>(player, msg));
	client.sendNetMessage (netMsg);
}

void sendAbortWaiting (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_ABORT_WAITING);
	client.sendNetMessage (std::move (message));
}

void sendSetAutoStatus (const cClient& client, int vehicleID, bool set)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_AUTOMOVE_STATUS);
	message->pushBool (set);
	message->pushInt16 (vehicleID);
	client.sendNetMessage (std::move (message));
}

//------------------------------------------------------------------------
void sendUpgradeBuilding (const cClient& client, const cBuilding& building, bool upgradeAll)
{
	if (building.getOwner() == 0)
		return;

	const cDynamicUnitData& currentVersion = building.data;
	const cDynamicUnitData& upgradedVersion = *building.getOwner()->getUnitDataCurrentVersion (building.data.getId());
	if (currentVersion.getVersion() >= upgradedVersion.getVersion())
		return; // already uptodate

	auto msg = std::make_unique<cNetMessage> (GAME_EV_WANT_BUILDING_UPGRADE);
	msg->pushBool (upgradeAll);
	msg->pushInt32 (building.iID);

	client.sendNetMessage (std::move (msg));
}

void sendWantUpgrade (const cClient& client, int buildingID, int storageSlot, bool upgradeAll)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_VEHICLE_UPGRADE);
	message->pushInt32 (buildingID);
	if (!upgradeAll) message->pushInt16 (storageSlot);
	message->pushBool (upgradeAll);
	client.sendNetMessage (std::move (message));
}

void sendRequestCasualtiesReport (const cClient& client)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_REQUEST_CASUALTIES_REPORT);
	client.sendNetMessage (std::move (message));
}

void sentWantKickPlayer (const cClient& client, const cPlayer& player)
{
	auto message = std::make_unique<cNetMessage> (GAME_EV_WANT_KICK_PLAYER);
	message->pushInt32 (player.getId());
	client.sendNetMessage (std::move (message));
}
