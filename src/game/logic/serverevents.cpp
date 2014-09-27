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
#include "game/logic/serverevents.h"

#include "game/data/units/building.h"
#include "game/logic/casualtiestracker.h"
#include "game/logic/clientevents.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "menuevents.h"
#include "game/logic/movejobs.h"
#include "netmessage.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/logic/server.h"
#include "game/logic/upgradecalculator.h"
#include "game/data/units/vehicle.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/game/gameguistate.h"
#include "game/data/report/savedreport.h"

//------------------------------------------------------------------------------
void sendAddUnit (cServer& server, const cPosition& position, int id, bool isVehicle, sID unitID, const cPlayer& player, bool isInit, bool shouldAddToMap)
{
	AutoPtr<cNetMessage> message;

	if (isVehicle) message = new cNetMessage (GAME_EV_ADD_VEHICLE);
	else message = new cNetMessage (GAME_EV_ADD_BUILDING);

	message->pushBool (shouldAddToMap);
	message->pushInt16 (id);
	message->pushPosition (position);
	message->pushID (unitID);
	message->pushInt16 (player.getNr());
	message->pushBool (isInit);

	server.sendNetMessage (message, &player);
}

//------------------------------------------------------------------------------
void sendAddRubble (cServer& server, const cBuilding& building, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_ADD_RUBBLE));

	message->pushPosition (building.getPosition());
	message->pushInt16 (building.iID);
	message->pushInt16 (building.RubbleValue);
	message->pushInt16 (building.RubbleTyp);
	message->pushBool (building.data.isBig);

	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendDeleteUnitMessage (cServer& server, const cUnit& unit, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (unit.isABuilding() ? GAME_EV_DEL_BUILDING : GAME_EV_DEL_VEHICLE));
	message->pushInt16 (unit.iID);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendDeleteUnit (cServer& server, const cUnit& unit, const cPlayer* receiver)
{
	if (receiver == nullptr)
	{
		for (unsigned int i = 0; i < unit.seenByPlayerList.size(); i++)
			sendDeleteUnitMessage (server, unit, *unit.seenByPlayerList[i]);

		//send message to owner, since he is not in the seenByPlayerList
		if (unit.getOwner () != 0)
			sendDeleteUnitMessage (server, unit, *unit.getOwner ());
	}
	else
		sendDeleteUnitMessage (server, unit, *receiver);
}

//------------------------------------------------------------------------------
void sendAddEnemyUnit (cServer& server, const cUnit& unit, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (unit.isABuilding() ? GAME_EV_ADD_ENEM_BUILDING : GAME_EV_ADD_ENEM_VEHICLE));

	message->pushInt16 (unit.data.getVersion ());
	message->pushInt16 (unit.iID);
	if (unit.isAVehicle())
		message->pushInt16 (unit.dir);
	message->pushPosition (unit.getPosition());
	message->pushID (unit.data.ID);
	message->pushInt16 (unit.getOwner ()->getNr ());

	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendMakeTurnEnd (cServer& server, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_MAKE_TURNEND));

	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendTurnFinished (cServer& server, const cPlayer& playerWhoEndedTurn, const cPlayer* nextPlayer, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_FINISHED_TURN));

	if (nextPlayer)
	{
		message->pushInt16 (nextPlayer->getNr ());
		message->pushBool (true);
	}
	else
	{
		message->pushBool (false);
	}
	message->pushInt16 (playerWhoEndedTurn.getNr());

	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendTurnStartTime (cServer& server, unsigned int gameTime)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_TURN_START_TIME));

	message->pushInt32 (gameTime);

	server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendTurnEndDeadlineStartTime (cServer& server, unsigned int gameTime)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_TURN_END_DEADLINE_START_TIME));

	message->pushInt32 (gameTime);

	server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendUnitData (cServer& server, const cUnit& unit, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_UNIT_DATA));

	// The unit data values
	if (unit.isAVehicle())
	{
		message->pushInt16 (static_cast<const cVehicle*> (&unit)->getFlightHeight ());
		message->pushInt16 (unit.data.speedMax);
		message->pushInt16 (unit.data.speedCur);
	}
	message->pushInt16 (unit.data.getVersion ());
	message->pushInt16 (unit.data.hitpointsMax);
	message->pushInt16 (unit.data.getHitpoints ());
	message->pushInt16 (unit.data.getArmor ());
	message->pushInt16 (unit.data.getScan ());
	message->pushInt16 (unit.data.getRange ());
	message->pushInt16 (unit.data.shotsMax);
	message->pushInt16 (unit.data.getShots ());
	message->pushInt16 (unit.data.getDamage ());
	message->pushInt16 (unit.data.storageUnitsMax);
	message->pushInt16 (unit.data.getStoredUnits ());
	message->pushInt16 (unit.data.storageResMax);
	message->pushInt16 (unit.data.getStoredResources ());
	message->pushInt16 (unit.data.ammoMax);
	message->pushInt16 (unit.data.getAmmo ());
	message->pushInt16 (unit.data.buildCosts);

	// Current state of the unit
	// TODO: remove information such sentrystatus,
	//       build or clearrounds from normal data
	//       because this data will be received by enemys, too
	if (unit.isABuilding())
		message->pushInt16 (static_cast<const cBuilding*> (&unit)->points);

	message->pushBool (unit.isManualFireActive());
	message->pushBool (unit.isSentryActive());

	if (unit.isAVehicle())
	{
		const cVehicle& vehicle = *static_cast<const cVehicle*> (&unit);
		message->pushInt16 (vehicle.getClearingTurns ());
		message->pushInt16 (vehicle.getBuildTurns ());
		message->pushBool (vehicle.isUnitBuildingABuilding ());
		message->pushBool (vehicle.isUnitClearing ());
		message->pushInt16 ((int) vehicle.getCommandoRank());
	}
	else
	{
		const cBuilding& building = *static_cast<const cBuilding*> (&unit);
		message->pushBool (building.isUnitWorking ());
		message->pushInt16 (building.getResearchArea());
	}

	message->pushInt16 (unit.getDisabledTurns());

	message->pushBool (unit.isBeeingAttacked ());
	message->pushBool (unit.isAttacking ());

	if (unit.isNameOriginal() == false)
	{
		message->pushString (unit.getName());
		message->pushBool (true);
	}
	else
		message->pushBool (false);

	if (unit.isAVehicle())
		message->pushBool (unit.data.isBig);

	// Data for identifying the unit by the client
	message->pushPosition (unit.getPosition());
	message->pushBool (unit.isAVehicle());
	message->pushInt16 (unit.iID);
	message->pushInt16 (unit.getOwner ()->getNr ());

	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendUnitData (cServer& server, const cUnit& unit)
{
    if (unit.getOwner ())
    {
        sendUnitData (server, unit, *unit.getOwner ());
    }
    for (size_t i = 0; i < unit.seenByPlayerList.size (); ++i)
    {
        sendUnitData (server, unit, *unit.seenByPlayerList[i]);
    }
}

//------------------------------------------------------------------------------
void sendSpecificUnitData (cServer& server, const cVehicle& vehicle)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SPECIFIC_UNIT_DATA));
	message->pushPosition (vehicle.bandPosition);
	message->pushBool (vehicle.BuildPath);
	message->pushID (vehicle.getBuildingType ());
	message->pushInt16 (vehicle.dir);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, vehicle.getOwner ());
}

//------------------------------------------------------------------------------
void sendDoStartWork (cServer& server, const cBuilding& building)
{
	//check all players
	const auto& playerList = server.playerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const auto& player = *playerList[i];

		//do not send to players who can't see the building
		if (!player.canSeeAnyAreaUnder (building) && &player != building.getOwner ()) continue;

		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_DO_START_WORK));
		message->pushInt32 (building.iID);
		server.sendNetMessage (message, &player);
	}
}

//------------------------------------------------------------------------------
void sendDoStopWork (cServer& server, const cBuilding& building)
{
	//check all players
	const auto& playerList = server.playerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const auto& player = *playerList[i];

		//do not send to players who can't see the building
		if (!player.canSeeAnyAreaUnder (building) && &player != building.getOwner ()) continue;

		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_DO_STOP_WORK));
		message->pushInt32 (building.iID);
		server.sendNetMessage (message, &player);
	}
}

//------------------------------------------------------------------------------
void sendNextMove (cServer& server, const cVehicle& vehicle, int iType, int iSavedSpeed)
{
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
	{
		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_NEXT_MOVE));
		if (iSavedSpeed >= 0) message->pushChar (iSavedSpeed);
		message->pushChar (iType);
		message->pushInt16 (vehicle.iID);
		server.sendNetMessage (message, vehicle.seenByPlayerList[i]);
	}

	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_NEXT_MOVE));
	if (iSavedSpeed >= 0) message->pushChar (iSavedSpeed);
	message->pushChar (iType);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, vehicle.getOwner ());
}

//------------------------------------------------------------------------------
void sendMoveJobServer (cServer& server, const cServerMoveJob& moveJob, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_MOVE_JOB_SERVER));

	const sWaypoint* waypoint = moveJob.Waypoints;
	int iCount = 0;
	while (waypoint)
	{
		message->pushInt16 (waypoint->Costs);
		message->pushPosition (waypoint->position);

		if (message->iLength > PACKAGE_LENGTH - 19)
		{
			Log.write (" Server: Error sending movejob: message too long", cLog::eLOG_TYPE_NET_ERROR);
			return; // don't send movejobs that are to long
		}

		waypoint = waypoint->next;
		iCount++;
	}

	message->pushInt16 (iCount);
	message->pushInt16 (moveJob.iSavedSpeed);
	message->pushPosition (moveJob.destination);
	message->pushPosition (moveJob.source);
	message->pushInt32 (moveJob.Vehicle->iID);

	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendVehicleResources (cServer& server, const cVehicle& vehicle)
{
	int iCount = 0;
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_RESOURCES));
	const cMap& map = *server.Map;
	// TODO: only send new scaned resources

	const int minx = std::max (vehicle.getPosition().x() - 1, 0);
	const int maxx = std::min (vehicle.getPosition().x() + 1, map.getSize().x() - 1);
	const int miny = std::max (vehicle.getPosition().y() - 1, 0);
	const int maxy = std::min (vehicle.getPosition().y() + 1, map.getSize().y() - 1);
	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const cPosition position (x, y);
			if (vehicle.getOwner ()->hasResourceExplored (position)) continue;

			const sResources& resource = map.getResource (position);
			message->pushInt16 (resource.value);
			message->pushInt16 (resource.typ);
			message->pushPosition (position);
			iCount++;
		}
	}
	message->pushInt16 (iCount);

	server.sendNetMessage (message, vehicle.getOwner ());
}

//------------------------------------------------------------------------------
void sendResources (cServer& server, const cPlayer& player)
{
	int iCount = 0;
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_RESOURCES));
	for (int x = 0; x != server.Map->getSize ().x (); ++x)
	{
		for (int y = 0; y != server.Map->getSize ().y (); ++y)
		{
			const cPosition position (x, y);

			if (!player.hasResourceExplored (position)) continue;

			const sResources& resource = server.Map->getResource (position);
			message->pushInt16 (resource.value);
			message->pushInt16 (resource.typ);
			message->pushPosition (position);
			iCount++;

			if (message->iLength >= PACKAGE_LENGTH - 10)
			{
				message->pushInt16 (iCount);
				server.sendNetMessage (message, &player);
				message = new cNetMessage (GAME_EV_RESOURCES);
				iCount = 0;
			}
		}
	}
	if (iCount > 0)
	{
		message->pushInt16 (iCount);
		server.sendNetMessage (message, &player);
	}
}

//------------------------------------------------------------------------------
void sendScore (cServer& server, const cPlayer& subject, int turn, const cPlayer* receiver)
{
	if (!receiver)
	{
		const auto& playerList = server.playerList;
		for (unsigned int n = 0; n < playerList.size(); n++)
			sendScore (server, subject, turn, playerList[n].get());
	}
	else
	{
		AutoPtr<cNetMessage> msg (new cNetMessage (GAME_EV_SCORE));
		msg->pushInt16 (subject.getScore (turn));
		msg->pushInt16 (turn);
		msg->pushInt16 (subject.getNr());

		server.sendNetMessage (msg, receiver);
	}
}

void sendUnitScore (cServer& server, const cBuilding& building)
{
	AutoPtr<cNetMessage> msg (new cNetMessage (GAME_EV_UNIT_SCORE));
	msg->pushInt16 (building.points);
	msg->pushInt16 (building.iID);
	server.sendNetMessage (msg, building.getOwner ());
}

void sendNumEcos (cServer& server, cPlayer& subject, const cPlayer* receiver)
{
	subject.countEcoSpheres();

	if (!receiver)
	{
		const auto& playerList = server.playerList;
		for (unsigned int n = 0; n < playerList.size(); n++)
			sendNumEcos (server, subject, playerList[n].get());
	}
	else
	{
		AutoPtr<cNetMessage> msg (new cNetMessage (GAME_EV_NUM_ECOS));
		msg->pushInt16 (subject.numEcos);
		msg->pushInt16 (subject.getNr());

		server.sendNetMessage (msg, receiver);
	}
}

void sendGameSettings (cServer& server, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_GAME_SETTINGS));
	
	const auto& gameSettings = server.getGameSettings ();

	if (!gameSettings) return;

	gameSettings->pushInto (*message);

	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendBuildAnswer (cServer& server, bool bOK, const cVehicle& vehicle)
{
	//message for the owner
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_BUILD_ANSWER));
	if (bOK)
	{
		message->pushPosition (vehicle.bandPosition);
		message->pushBool (vehicle.BuildPath);
		message->pushInt16 (vehicle.getBuildTurns ());
		message->pushID (vehicle.getBuildingType ());
		message->pushBool (vehicle.getBuildingType ().getUnitDataOriginalVersion ()->isBig);
		message->pushPosition (vehicle.getPosition());
	}

	message->pushInt16 (vehicle.iID);
	message->pushBool (bOK);
	server.sendNetMessage (message, vehicle.getOwner ());

	//message for the enemys
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
	{
		AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_BUILD_ANSWER));
		if (bOK)
		{
			message->pushBool (vehicle.getBuildingType ().getUnitDataOriginalVersion ()->isBig);
			message->pushPosition (vehicle.getPosition ());
		}
		message->pushInt16 (vehicle.iID);
		message->pushBool (bOK);
		server.sendNetMessage (message, vehicle.seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void sendStopBuild (cServer& server, int iVehicleID, const cPosition& newPosition, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_STOP_BUILD));
	message->pushPosition (newPosition);
	message->pushInt16 (iVehicleID);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendSubbaseValues (cServer& server, const sSubBase& subBase, const cPlayer& receiver)
{
	// temporary debug check
	if (subBase.isDitributionMaximized() == false)
	{
		Log.write (" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}

	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SUBBASE_VALUES));

	subBase.pushInto (*message);
	message->pushInt16 (subBase.getID());
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendBuildList (cServer& server, const cBuilding& building)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_BUILDLIST));
	message->pushBool (building.RepeatBuild);
	message->pushInt16 (building.BuildSpeed);
	message->pushInt16 (building.MetalPerRound);
	for (int i = (int) building.getBuildListSize() - 1; i >= 0; i--)
	{
		message->pushInt16 (building.getBuildListItem (i).getRemainingMetal ());
		message->pushID (building.getBuildListItem (i).getType ());
	}
	message->pushInt16 ((int)building.getBuildListSize ());
	message->pushInt16 (building.iID);
	server.sendNetMessage (message, building.getOwner ());
}

//------------------------------------------------------------------------------
void sendProduceValues (cServer& server, const cBuilding& building)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_MINE_PRODUCE_VALUES));
	message->pushInt16 (building.MaxGoldProd);
	message->pushInt16 (building.MaxOilProd);
	message->pushInt16 (building.MaxMetalProd);
	message->pushInt16 (building.iID);
	server.sendNetMessage (message, building.getOwner ());
}

//------------------------------------------------------------------------------
void sendSupply (cServer& server, int iDestID, bool bDestVehicle, int iValue, int iType, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SUPPLY));
	message->pushInt16 (iValue);
	message->pushInt16 (iDestID);
	message->pushBool (bDestVehicle);
	message->pushChar (iType);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendDetectionState (cServer& server, const cVehicle& vehicle)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_DETECTION_STATE));
	message->pushBool (!vehicle.detectedByPlayerList.empty());
	message->pushInt32 (vehicle.iID);
	server.sendNetMessage (message, vehicle.getOwner ());
}

//------------------------------------------------------------------------------
void sendClearAnswer (cServer& server, int answertype, const cVehicle& vehicle, int turns, const cPosition& bigPosition, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_CLEAR_ANSWER));
	message->pushPosition (bigPosition);
	message->pushInt16 (turns);
	message->pushInt16 (vehicle.iID);
	message->pushInt16 (answertype);
	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendStopClear (cServer& server, const cVehicle& vehicle, const cPosition& bigPosition, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_STOP_CLEARING));
	message->pushPosition (bigPosition);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendNoFog (cServer& server, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_NOFOG));
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendDefeated (cServer& server, const cPlayer& player, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_DEFEATED));
	message->pushInt16 (player.getNr());
	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendFreeze (cServer& server, eFreezeMode mode, int waitForPlayer)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_FREEZE));
	message->pushInt16 (waitForPlayer);
	message->pushInt16 (mode);
	server.sendNetMessage (message, nullptr);
}

//------------------------------------------------------------------------------
void sendUnfreeze (cServer& server, eFreezeMode mode)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_UNFREEZE));
	message->pushInt16 (mode);
	server.sendNetMessage (message, nullptr);
}

//------------------------------------------------------------------------------
void sendWaitFor (cServer& server, const cPlayer& player, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_WAIT_FOR));
	message->pushInt32 (player.getNr());
	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendDeletePlayer (cServer& server, const cPlayer& player, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_DEL_PLAYER));
	message->pushInt16 (player.getNr());
	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendRequestIdentification (cTCP& network, int iSocket)
{
	cNetMessage message (GAME_EV_REQ_RECON_IDENT);
	message.pushInt16 (iSocket);
	Log.write ("Server: --> " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	network.sendTo (iSocket, message.iLength, message.serialize());
}

//------------------------------------------------------------------------------
void sendReconnectAnswer (cServer& server, int socketNumber)
{
	cNetMessage message (GAME_EV_RECONNECT_ANSWER);
	message.pushBool (false);

	Log.write ("Server: --> " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	server.network->sendTo (socketNumber, message.iLength, message.serialize());
}

//------------------------------------------------------------------------------
void sendReconnectAnswer (cServer& server, int socketNumber, const cPlayer& player)
{
	cNetMessage message (GAME_EV_RECONNECT_ANSWER);

	const auto& playerList = server.playerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const auto& secondPlayer = *playerList[i];
		if (&player == &secondPlayer) continue;
		message.pushInt16 (secondPlayer.getNr ());
		message.pushColor (secondPlayer.getColor ().getColor ());
		message.pushString (secondPlayer.getName ());
	}
	message.pushInt16 ((int) playerList.size());
	message.pushString (server.Map->getName ());
	message.pushColor (player.getColor ().getColor ());
	message.pushInt16 (player.getNr());

	message.pushBool (true);

	Log.write ("Server: --> " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	server.network->sendTo (socketNumber, message.iLength, message.serialize());
}

//------------------------------------------------------------------------------
void sendTurn (cServer& server, int turn, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_TURN));
	message->pushInt16 (turn);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendGameGuiState (cServer& server, const cGameGuiState& gameGuiState, const cPlayer& player)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_HUD_SETTINGS));

	gameGuiState.pushInto (*message);

	server.sendNetMessage (message, &player);
}

//------------------------------------------------------------------------------
void sendStoreVehicle (cServer& server, int unitid, bool vehicle, int storedunitid, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_STORE_UNIT));
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (storedunitid);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendActivateVehicle (cServer& server, int unitid, bool vehicle, int activatunitid, const cPosition& position, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_EXIT_UNIT));
	message->pushPosition (position);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (activatunitid);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendDeleteEverything (cServer& server, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_DELETE_EVERYTHING));
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendResearchLevel (cServer& server, const cResearch& researchLevel, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_RESEARCH_LEVEL));
	for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	{
		message->pushInt16 (researchLevel.getCurResearchLevel (area));
		message->pushInt16 (researchLevel.getCurResearchPoints (area));
	}
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendFinishedResearchAreas (cServer& server, const std::vector<int>& areas, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_FINISHED_RESEARCH_AREAS));
	for (size_t i = 0; i < areas.size (); ++i)
	{
		message->pushInt32 (areas[i]);
	}
	message->pushInt32 (areas.size ());
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendRefreshResearchCount (cServer& server, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_REFRESH_RESEARCH_COUNT));
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendUnitUpgrades (cServer& server, const sUnitData& unitData, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_UNIT_UPGRADE_VALUES));
	message->pushInt16 (unitData.hitpointsMax);
	message->pushInt16 (unitData.ammoMax);
	message->pushInt16 (unitData.shotsMax);
	message->pushInt16 (unitData.speedMax);
	message->pushInt16 (unitData.getArmor ());
	message->pushInt16 (unitData.buildCosts);
	message->pushInt16 (unitData.getDamage ());
	message->pushInt16 (unitData.getRange ());
	message->pushInt16 (unitData.getScan ());
	message->pushInt16 (unitData.getVersion ());
	message->pushID (unitData.ID);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendCredits (cServer& server, int newCredits, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_CREDITS_CHANGED));
	message->pushInt16 (newCredits);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendUpgradeBuildings (cServer& server, const std::vector<cBuilding*>& upgradedBuildings, int totalCosts, const cPlayer& receiver)
{
	// send to owner
	AutoPtr<cNetMessage> message;
	int buildingsInMsg = 0;
	for (unsigned int i = 0; i < upgradedBuildings.size(); i++)
	{
		if (message == NULL)
		{
			message = new cNetMessage (GAME_EV_UPGRADED_BUILDINGS);
			buildingsInMsg = 0;
		}

		message->pushInt32 (upgradedBuildings[i]->iID);
		buildingsInMsg++;
		if (message->iLength + 8 > PACKAGE_LENGTH)
		{
			message->pushInt16 ((totalCosts * buildingsInMsg) / (int) upgradedBuildings.size());
			message->pushInt16 (buildingsInMsg);
			server.sendNetMessage (message, &receiver);
			message = NULL;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 ((int) (totalCosts * buildingsInMsg) / (int) upgradedBuildings.size());
		message->pushInt16 (buildingsInMsg);
		server.sendNetMessage (message, &receiver);
		message = NULL;
	}

	// send to other players
	const auto& playerList = server.playerList;
	for (unsigned int n = 0; n < playerList.size(); n++)
	{
		const auto curPlayer = playerList[n].get();
		// don't send to the owner of the buildings
		if (curPlayer == 0 || curPlayer->getNr() == receiver.getNr()) continue;

		for (unsigned int buildingIdx = 0; buildingIdx < upgradedBuildings.size(); buildingIdx++)
		{
			if (Contains (upgradedBuildings[buildingIdx]->seenByPlayerList, curPlayer)) // that player can see the building
				sendUnitData (server, *upgradedBuildings[buildingIdx], *curPlayer);
		}
	}
}

//------------------------------------------------------------------------------
void sendUpgradeVehicles (cServer& server, const std::vector<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, const cPlayer& receiver)
{
	if (upgradedVehicles.size() * 4 > PACKAGE_LENGTH - 50)
	{
		Log.write ("Server: sendUpgradeVehicles: Message would exceed messagesize!!!", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	// send to owner
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_UPGRADED_VEHICLES));
	for (unsigned int i = 0; i < upgradedVehicles.size(); i++)
		message->pushInt32 (upgradedVehicles[i]->iID);

	message->pushInt32 (storingBuildingID);
	message->pushInt16 (totalCosts);
	message->pushInt16 ((int) upgradedVehicles.size());
	server.sendNetMessage (message, &receiver);

	//TODO: send to other players as well?
}

//------------------------------------------------------------------------------
void sendResearchSettings (cServer& server, const std::vector<cBuilding*>& researchCentersToChangeArea, const std::vector<cResearch::ResearchArea>& newAreasForResearchCenters, const cPlayer& receiver)
{
	if (researchCentersToChangeArea.size() != newAreasForResearchCenters.size())
		return;

	AutoPtr<cNetMessage> message;
	int buildingsInMsg = 0;
	for (unsigned int i = 0; i < researchCentersToChangeArea.size(); i++)
	{
		if (message == NULL)
		{
			message = new cNetMessage (GAME_EV_RESEARCH_SETTINGS);
			buildingsInMsg = 0;
		}

		message->pushChar (newAreasForResearchCenters[i]);
		message->pushInt32 (researchCentersToChangeArea[i]->iID);
		buildingsInMsg++;
		if (message->iLength + 7 > PACKAGE_LENGTH)
		{
			message->pushInt16 (buildingsInMsg);
			server.sendNetMessage (message, &receiver);
			message = NULL;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 (buildingsInMsg);
		server.sendNetMessage (message, &receiver);
		message = NULL;
	}
}

//------------------------------------------------------------------------------
void sendClans (cServer& server, const std::vector<std::unique_ptr<cPlayer>>& playerList, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_PLAYER_CLANS));
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		message->pushChar (playerList[i]->getClan());
		message->pushChar (playerList[i]->getNr());
	}
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendGameTime (cServer& server, const cPlayer& receiver, int gameTime)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SET_GAME_TIME));
	message->pushInt32 (gameTime);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendClansToClients (cServer& server, const std::vector<std::unique_ptr<cPlayer>>& playerList)
{
	for (unsigned int n = 0; n < playerList.size(); n++)
		sendClans (server, playerList, *playerList[n]);
}

//------------------------------------------------------------------------------
void sendSetAutomoving (cServer& server, const cVehicle& vehicle)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SET_AUTOMOVE));
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, vehicle.getOwner ());
}

//------------------------------------------------------------------------------
void sendCommandoAnswer (cServer& server, bool success, bool steal, const cVehicle& srcUnit, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_COMMANDO_ANSWER));
	message->pushInt16 (srcUnit.iID);
	message->pushBool (steal);
	message->pushBool (success);
	server.sendNetMessage (message, &receiver);
}

//------------------------------------------------------------------------------
void sendRequestSaveInfo (cServer& server, int saveingID)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_REQ_SAVE_INFO));
	message->pushInt16 (saveingID);
	server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendSavedReport (cServer& server, const cSavedReport& savedReport, const cPlayer* receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SAVED_REPORT));

	savedReport.pushInto (*message);
	server.sendNetMessage (message, receiver);
}

//------------------------------------------------------------------------------
void sendCasualtiesReport (cServer& server, const cPlayer* receiver)
{
	const auto casualtiesTracker = server.getCasualtiesTracker().get();
	if (casualtiesTracker)
	{
		std::vector<cNetMessage*> messages;
		casualtiesTracker->prepareNetMessagesForClient (messages, GAME_EV_CASUALTIES_REPORT);
		for (size_t i = 0; i < messages.size(); i++)
		{
			AutoPtr<cNetMessage> message (messages[i]);
			server.sendNetMessage (message, receiver);
		}
	}
}

//------------------------------------------------------------------------------
void sendSelfDestroy (cServer& server, const cBuilding& building)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_SELFDESTROY));
	message->pushInt16 (building.iID);
	server.sendNetMessage (message, building.getOwner ());

	for (unsigned int i = 0; i < building.seenByPlayerList.size(); i++)
	{
		message = new cNetMessage (GAME_EV_SELFDESTROY);
		message->pushInt16 (building.iID);
		server.sendNetMessage (message, building.seenByPlayerList[i]);
	}
}

//------------------------------------------------------------------------------
void sendEndMoveActionToClient (cServer& server, const cVehicle& vehicle, int destID, eEndMoveActionType type)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_END_MOVE_ACTION_SERVER));
	message->pushChar (type);
	message->pushInt32 (destID);
	message->pushInt32 (vehicle.iID);

	server.sendNetMessage (message, vehicle.getOwner ());
}

//------------------------------------------------------------------------------
void sendRevealMap (cServer& server, const cPlayer& receiver)
{
	AutoPtr<cNetMessage> message (new cNetMessage (GAME_EV_REVEAL_MAP));

	server.sendNetMessage (message, &receiver);
}