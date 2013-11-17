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
#include "serverevents.h"

#include "buildings.h"
#include "casualtiestracker.h"
#include "clientevents.h"
#include "clist.h"
#include "events.h"
#include "hud.h"
#include "log.h"
#include "menuevents.h"
#include "movejobs.h"
#include "netmessage.h"
#include "network.h"
#include "player.h"
#include "server.h"
#include "upgradecalculator.h"
#include "vehicles.h"


//------------------------------------------------------------------------------
void sendGo (cServer& server)
{
	cNetMessage* message = new cNetMessage (MU_MSG_GO);
	server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendReselectLanding (cServer& server, eLandingState state, int iPlayer)
{
	cNetMessage* message = new cNetMessage (MU_MSG_RESELECT_LANDING);
	message->pushChar (state);

	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendAllLanded (cServer& server)
{
	cNetMessage* message = new cNetMessage (MU_MSG_ALL_LANDED);
	server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendAddUnit (cServer& server, int iPosX, int iPosY, int iID, bool bVehicle, sID UnitID, int iPlayer, bool bInit, bool bAddToMap)
{
	cNetMessage* message;

	if (bVehicle) message = new cNetMessage (GAME_EV_ADD_VEHICLE);
	else message = new cNetMessage (GAME_EV_ADD_BUILDING);

	message->pushBool (bAddToMap);
	message->pushInt16 (iID);
	message->pushInt16 (iPosX);
	message->pushInt16 (iPosY);
	message->pushID (UnitID);
	message->pushInt16 (iPlayer);
	message->pushBool (bInit);

	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendAddRubble (cServer& server, const cBuilding& building, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_ADD_RUBBLE);

	message->pushInt16 (building.PosX);
	message->pushInt16 (building.PosY);
	message->pushInt16 (building.iID);
	message->pushInt16 (building.RubbleValue);
	message->pushInt16 (building.RubbleTyp);
	message->pushBool (building.data.isBig);

	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendDeleteUnitMessage (cServer& server, const cUnit& unit, int playerNr)
{
	cNetMessage* message = new cNetMessage (unit.isABuilding() ? GAME_EV_DEL_BUILDING : GAME_EV_DEL_VEHICLE);
	message->pushInt16 (unit.iID);
	server.sendNetMessage (message, playerNr);
}

//------------------------------------------------------------------------------
void sendDeleteUnit (cServer& server, const cUnit& unit, int iClient)
{
	if (iClient == -1)
	{
		for (unsigned int i = 0; i < unit.seenByPlayerList.size(); i++)
			sendDeleteUnitMessage (server, unit, unit.seenByPlayerList[i]->getNr());

		//send message to owner, since he is not in the seenByPlayerList
		if (unit.owner != 0)
			sendDeleteUnitMessage (server, unit, unit.owner->getNr());
	}
	else
		sendDeleteUnitMessage (server, unit, iClient);
}

//------------------------------------------------------------------------------
void sendAddEnemyUnit (cServer& server, const cUnit& unit, int iClient)
{
	cNetMessage* message = new cNetMessage (unit.isABuilding() ? GAME_EV_ADD_ENEM_BUILDING : GAME_EV_ADD_ENEM_VEHICLE);

	message->pushInt16 (unit.data.version);
	message->pushInt16 (unit.iID);
	if (unit.isAVehicle())
		message->pushInt16 (unit.dir);
	message->pushInt16 (unit.PosX);
	message->pushInt16 (unit.PosY);
	message->pushID (unit.data.ID);
	message->pushInt16 (unit.owner->getNr());

	server.sendNetMessage (message, iClient);
}

//------------------------------------------------------------------------------
void sendMakeTurnEnd (cServer& server, bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_MAKE_TURNEND);

	message->pushBool (bEndTurn);
	message->pushBool (bWaitForNextPlayer);
	message->pushInt16 (iNextPlayerNum);

	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendTurnFinished (cServer& server, int iPlayerNum, int iTimeDelay, const cPlayer* Player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_FINISHED_TURN);

	message->pushInt32 (iTimeDelay);
	message->pushInt16 (iPlayerNum);

	if (Player != NULL) server.sendNetMessage (message, Player->getNr());
	else server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendUnitData (cServer& server, const cUnit& unit, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_UNIT_DATA);

	// The unit data values
	if (unit.isAVehicle())
	{
		message->pushInt16 (static_cast<const cVehicle*> (&unit)->FlightHigh);
		message->pushInt16 (unit.data.speedMax);
		message->pushInt16 (unit.data.speedCur);
	}
	message->pushInt16 (unit.data.version);
	message->pushInt16 (unit.data.hitpointsMax);
	message->pushInt16 (unit.data.hitpointsCur);
	message->pushInt16 (unit.data.armor);
	message->pushInt16 (unit.data.scan);
	message->pushInt16 (unit.data.range);
	message->pushInt16 (unit.data.shotsMax);
	message->pushInt16 (unit.data.shotsCur);
	message->pushInt16 (unit.data.damage);
	message->pushInt16 (unit.data.storageUnitsMax);
	message->pushInt16 (unit.data.storageUnitsCur);
	message->pushInt16 (unit.data.storageResMax);
	message->pushInt16 (unit.data.storageResCur);
	message->pushInt16 (unit.data.ammoMax);
	message->pushInt16 (unit.data.ammoCur);
	message->pushInt16 (unit.data.buildCosts);

	// Current state of the unit
	// TODO: remove information such sentrystatus,
	//       build or clearrounds from normal data
	//       because this data will be received by enemys, too
	if (unit.isABuilding())
		message->pushInt16 (static_cast<const cBuilding*> (&unit)->points);

	message->pushBool (unit.manualFireActive);
	message->pushBool (unit.sentryActive);

	if (unit.isAVehicle())
	{
		const cVehicle& vehicle = *static_cast<const cVehicle*> (&unit);
		message->pushInt16 (vehicle.ClearingRounds);
		message->pushInt16 (vehicle.BuildRounds);
		message->pushBool (vehicle.IsBuilding);
		message->pushBool (vehicle.IsClearing);
		message->pushInt16 ( (int) vehicle.CommandoRank);
	}
	else
	{
		const cBuilding& building = *static_cast<const cBuilding*> (&unit);
		message->pushBool (building.IsWorking);
		message->pushInt16 (building.researchArea);
	}

	message->pushInt16 (unit.turnsDisabled);

	if (unit.isAVehicle())
		message->pushBool (unit.isBeeingAttacked);

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
	message->pushInt16 (unit.PosX);
	message->pushInt16 (unit.PosY);
	message->pushBool (unit.isAVehicle());
	message->pushInt16 (unit.iID);
	message->pushInt16 (unit.owner->getNr());

	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendSpecificUnitData (cServer& server, const cVehicle& vehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SPECIFIC_UNIT_DATA);
	message->pushInt16 (vehicle.BandY);
	message->pushInt16 (vehicle.BandX);
	message->pushBool (vehicle.BuildPath);
	message->pushID (vehicle.BuildingTyp);
	message->pushInt16 (vehicle.dir);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, vehicle.owner->getNr());
}

//------------------------------------------------------------------------------
void sendChatMessageToClient (cServer& server, const std::string& message, int iType, int iPlayer, const std::string& inserttext)
{
	cNetMessage* newMessage;
	newMessage = new cNetMessage (GAME_EV_CHAT_SERVER);
	newMessage->pushString (inserttext);
	newMessage->pushString (message);
	newMessage->pushChar (iType);
	server.sendNetMessage (newMessage, iPlayer);
}

//------------------------------------------------------------------------------
void sendDoStartWork (cServer& server, const cBuilding& building)
{
	//check all players
	const std::vector<cPlayer*>& playerList = server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const cPlayer* player = playerList[i];

		//do not send to players who can't see the building
		if (!player->canSeeAnyAreaUnder (building) && player != building.owner) continue;

		cNetMessage* message = new cNetMessage (GAME_EV_DO_START_WORK);
		message->pushInt32 (building.iID);
		server.sendNetMessage (message, player->getNr());
	}
}

//------------------------------------------------------------------------------
void sendDoStopWork (cServer& server, const cBuilding& building)
{
	//check all players
	const std::vector<cPlayer*>& playerList = server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		const cPlayer* player = playerList[i];

		//do not send to players who can't see the building
		if (!player->canSeeAnyAreaUnder (building) && player != building.owner) continue;

		cNetMessage* message = new cNetMessage (GAME_EV_DO_STOP_WORK);
		message->pushInt32 (building.iID);
		server.sendNetMessage (message, player->getNr());
	}
}

//------------------------------------------------------------------------------
void sendNextMove (cServer& server, const cVehicle& vehicle, int iType, int iSavedSpeed)
{
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
	{
		cNetMessage* message = new cNetMessage (GAME_EV_NEXT_MOVE);
		if (iSavedSpeed >= 0) message->pushChar (iSavedSpeed);
		message->pushChar (iType);
		message->pushInt16 (vehicle.iID);
		server.sendNetMessage (message, vehicle.seenByPlayerList[i]->getNr());
	}

	cNetMessage* message = new cNetMessage (GAME_EV_NEXT_MOVE);
	if (iSavedSpeed >= 0) message->pushChar (iSavedSpeed);
	message->pushChar (iType);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, vehicle.owner->getNr());
}

//------------------------------------------------------------------------------
void sendMoveJobServer (cServer& server, const cServerMoveJob& moveJob, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_MOVE_JOB_SERVER);

	const sWaypoint* waypoint = moveJob.Waypoints;
	int iCount = 0;
	while (waypoint)
	{
		message->pushInt16 (waypoint->Costs);
		message->pushInt16 (waypoint->X);
		message->pushInt16 (waypoint->Y);

		if (message->iLength > PACKAGE_LENGTH - 19)
		{
			Log.write (" Server: Error sending movejob: message too long", cLog::eLOG_TYPE_NET_ERROR);
			delete message;
			return; // don't send movejobs that are to long
		}

		waypoint = waypoint->next;
		iCount++;
	}

	message->pushInt16 (iCount);
	message->pushInt16 (moveJob.iSavedSpeed);
	message->pushInt32 (moveJob.Map->getOffset (moveJob.DestX, moveJob.DestY));
	message->pushInt32 (moveJob.Map->getOffset (moveJob.SrcX, moveJob.SrcY));
	message->pushInt32 (moveJob.Vehicle->iID);

	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendVehicleResources (cServer& server, const cVehicle& vehicle)
{
	int iCount = 0;
	cNetMessage* message = new cNetMessage (GAME_EV_RESOURCES);
	const cMap& map = *server.Map;
	// TODO: only send new scaned resources

	const int minx = std::max (vehicle.PosX - 1, 0);
	const int maxx = std::min (vehicle.PosX + 1, map.getSize() - 1);
	const int miny = std::max (vehicle.PosY - 1, 0);
	const int maxy = std::min (vehicle.PosY + 1, map.getSize() - 1);
	for (int y = miny; y <= maxy; ++y)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const int offset = map.getOffset (x, y);
			if (vehicle.owner->hasResourceExplored (offset)) continue;

			const sResources& resource = map.getResource (offset);
			message->pushInt16 (resource.value);
			message->pushInt16 (resource.typ);
			message->pushInt32 (offset);
			iCount++;
		}
	}
	message->pushInt16 (iCount);

	server.sendNetMessage (message, vehicle.owner->getNr());
}

//------------------------------------------------------------------------------
void sendResources (cServer& server, const cPlayer& player)
{
	int iCount = 0;
	cNetMessage* message = new cNetMessage (GAME_EV_RESOURCES);
	const size_t size = server.Map->getSize() * server.Map->getSize();
	for (size_t i = 0; i != size; ++i)
	{
		if (!player.hasResourceExplored (i)) continue;

		const sResources& resource = server.Map->getResource (i);
		message->pushInt16 (resource.value);
		message->pushInt16 (resource.typ);
		message->pushInt32 (i);
		iCount++;

		if (message->iLength >= PACKAGE_LENGTH - 10)
		{
			message->pushInt16 (iCount);
			server.sendNetMessage (message, player.getNr());
			message = new cNetMessage (GAME_EV_RESOURCES);
			iCount = 0;
		}
	}
	if (iCount > 0)
	{
		message->pushInt16 (iCount);
		server.sendNetMessage (message, player.getNr());
	}
}

//------------------------------------------------------------------------------
void sendScore (cServer& server, const cPlayer& subject, int turn, const cPlayer* receiver)
{
	if (!receiver)
	{
		const std::vector<cPlayer*>& playerList = server.PlayerList;
		for (unsigned int n = 0; n < playerList.size(); n++)
			sendScore (server, subject, turn, playerList[n]);
	}
	else
	{
		cNetMessage* msg = new cNetMessage (GAME_EV_SCORE);
		msg->pushInt16 (subject.getScore (turn));
		msg->pushInt16 (turn);
		msg->pushInt16 (subject.getNr());

		server.sendNetMessage (msg, receiver->getNr());
	}
}

void sendUnitScore (cServer& server, const cBuilding& building)
{
	cNetMessage* msg = new cNetMessage (GAME_EV_UNIT_SCORE);
	msg->pushInt16 (building.points);
	msg->pushInt16 (building.iID);
	server.sendNetMessage (msg, building.owner->getNr());
}

void sendNumEcos (cServer& server, cPlayer& subject, const cPlayer* receiver)
{
	subject.countEcoSpheres();

	if (!receiver)
	{
		const std::vector<cPlayer*>& playerList = server.PlayerList;
		for (unsigned int n = 0; n < playerList.size(); n++)
			sendNumEcos (server, subject, playerList[n]);
	}
	else
	{
		cNetMessage* msg = new cNetMessage (GAME_EV_NUM_ECOS);
		msg->pushInt16 (subject.numEcos);
		msg->pushInt16 (subject.getNr());

		server.sendNetMessage (msg, receiver->getNr());
	}
}

void sendGameSettings (cServer& server, const cPlayer& receiver)
{
	cNetMessage* message = new cNetMessage (GAME_EV_GAME_SETTINGS);
	const sSettings* gameSettings = server.getGameSettings();

	if (gameSettings)
	{
		server.getGameSettings()->pushInto (*message);
	}
	message->pushBool (gameSettings != NULL);
	server.sendNetMessage (message, receiver.getNr());
}

//------------------------------------------------------------------------------
void sendBuildAnswer (cServer& server, bool bOK, const cVehicle& vehicle)
{
	//message for the owner
	cNetMessage* message = new cNetMessage (GAME_EV_BUILD_ANSWER);
	if (bOK)
	{
		message->pushInt16 (vehicle.BandY);
		message->pushInt16 (vehicle.BandX);
		message->pushBool (vehicle.BuildPath);
		message->pushInt16 (vehicle.BuildRounds);
		message->pushID (vehicle.BuildingTyp);
		message->pushBool (vehicle.BuildingTyp.getUnitDataOriginalVersion()->isBig);
		message->pushInt16 (vehicle.PosY);
		message->pushInt16 (vehicle.PosX);
	}

	message->pushInt16 (vehicle.iID);
	message->pushBool (bOK);
	server.sendNetMessage (message, vehicle.owner->getNr());

	//message for the enemys
	for (unsigned int i = 0; i < vehicle.seenByPlayerList.size(); i++)
	{
		cNetMessage* message = new cNetMessage (GAME_EV_BUILD_ANSWER);
		if (bOK)
		{
			message->pushBool (vehicle.BuildingTyp.getUnitDataOriginalVersion()->isBig);
			message->pushInt16 (vehicle.PosY);
			message->pushInt16 (vehicle.PosX);
		}
		message->pushInt16 (vehicle.iID);
		message->pushBool (bOK);
		server.sendNetMessage (message, vehicle.seenByPlayerList[i]->getNr());
	}
}

//------------------------------------------------------------------------------
void sendStopBuild (cServer& server, int iVehicleID, int iNewPos, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_STOP_BUILD);
	message->pushInt32 (iNewPos);
	message->pushInt16 (iVehicleID);
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendSubbaseValues (cServer& server, const sSubBase& subBase, int iPlayer)
{
	// temporary debug check
	if (subBase.isDitributionMaximized() == false)
	{
		Log.write (" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}

	cNetMessage* message = new cNetMessage (GAME_EV_SUBBASE_VALUES);

	subBase.pushInto (*message);
	message->pushInt16 (subBase.getID());
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendBuildList (cServer& server, const cBuilding& building)
{
	cNetMessage* message = new cNetMessage (GAME_EV_BUILDLIST);
	message->pushBool (building.RepeatBuild);
	message->pushInt16 (building.BuildSpeed);
	message->pushInt16 (building.MetalPerRound);
	for (int i = (int) building.BuildList.size() - 1; i >= 0; i--)
	{
		message->pushInt16 (building.BuildList[i].metall_remaining);
		message->pushID (building.BuildList[i].type);
	}
	message->pushInt16 ( (int) building.BuildList.size());
	message->pushInt16 (building.iID);
	server.sendNetMessage (message, building.owner->getNr());
}

//------------------------------------------------------------------------------
void sendProduceValues (cServer& server, const cBuilding& building)
{
	cNetMessage* message = new cNetMessage (GAME_EV_MINE_PRODUCE_VALUES);
	message->pushInt16 (building.MaxGoldProd);
	message->pushInt16 (building.MaxOilProd);
	message->pushInt16 (building.MaxMetalProd);
	message->pushInt16 (building.iID);
	server.sendNetMessage (message, building.owner->getNr());
}

//------------------------------------------------------------------------------
void sendTurnReport (cServer& server, cPlayer& player)
{
	// TODO: make sure, that the message size is not exceeded!

	cNetMessage* message = new cNetMessage (GAME_EV_TURN_REPORT);
	int iCount = 0;

	int nrResearchAreasFinished = player.reportResearchAreasFinished.size();
	for (int i = nrResearchAreasFinished - 1; i >= 0; i--)   // count down to get the correct order at the client conveniently
		message->pushChar (player.reportResearchAreasFinished[i]);
	message->pushChar (nrResearchAreasFinished);

	for (size_t i = 0; i != player.ReportBuildings.size(); ++i)
	{
		const sTurnstartReport* report = player.ReportBuildings[i];
		message->pushInt16 (report->iAnz);
		message->pushID (report->Type);
		delete report;
		iCount++;
	}
	player.ReportBuildings.clear();
	for (size_t i = 0; i != player.ReportVehicles.size(); ++i)
	{
		sTurnstartReport* report = player.ReportVehicles[i];
		message->pushInt16 (report->iAnz);
		message->pushID (report->Type);
		delete report;
		iCount++;
	}
	player.ReportVehicles.clear();
	message->pushInt16 (iCount);
	server.sendNetMessage (message, player.getNr());
}

//------------------------------------------------------------------------------
void sendSupply (cServer& server, int iDestID, bool bDestVehicle, int iValue, int iType, int iPlayerNum)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SUPPLY);
	message->pushInt16 (iValue);
	message->pushInt16 (iDestID);
	message->pushBool (bDestVehicle);
	message->pushChar (iType);
	server.sendNetMessage (message, iPlayerNum);
}

//------------------------------------------------------------------------------
void sendDetectionState (cServer& server, const cVehicle& vehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_DETECTION_STATE);
	message->pushBool (!vehicle.detectedByPlayerList.empty());
	message->pushInt32 (vehicle.iID);
	server.sendNetMessage (message, vehicle.owner->getNr());
}

//------------------------------------------------------------------------------
void sendClearAnswer (cServer& server, int answertype, const cVehicle& vehicle, int turns, int bigoffset, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_CLEAR_ANSWER);
	message->pushInt16 (bigoffset);
	message->pushInt16 (turns);
	message->pushInt16 (vehicle.iID);
	message->pushInt16 (answertype);
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendStopClear (cServer& server, const cVehicle& vehicle, int bigoffset, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_STOP_CLEARING);
	message->pushInt16 (bigoffset);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendNoFog (cServer& server, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_NOFOG);
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendDefeated (cServer& server, const cPlayer& player, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_DEFEATED);
	message->pushInt16 (player.getNr());
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendFreeze (cServer& server, eFreezeMode mode, int waitForPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_FREEZE);
	message->pushInt16 (waitForPlayer);
	message->pushInt16 (mode);
	server.sendNetMessage (message, -1);
}

//------------------------------------------------------------------------------
void sendUnfreeze (cServer& server, eFreezeMode mode)
{
	cNetMessage* message = new cNetMessage (GAME_EV_UNFREEZE);
	message->pushInt16 (mode);
	server.sendNetMessage (message, -1);
}

//------------------------------------------------------------------------------
void sendWaitFor (cServer& server, int waitForPlayerNr, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_WAIT_FOR);
	message->pushInt16 (waitForPlayerNr);
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendDeletePlayer (cServer& server, const cPlayer& player, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_DEL_PLAYER);
	message->pushInt16 (player.getNr());
	server.sendNetMessage (message, iPlayer);
}

//------------------------------------------------------------------------------
void sendRequestIdentification (cTCP& network, int iSocket)
{
	cNetMessage message (GAME_EV_REQ_RECON_IDENT);
	message.pushInt16 (iSocket);
	Log.write ("Server: <-- " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	network.sendTo (iSocket, message.iLength, message.serialize());
}

//------------------------------------------------------------------------------
void sendReconnectAnswer (cServer& server, int socketNumber)
{
	cNetMessage message (GAME_EV_RECONNECT_ANSWER);
	message.pushBool (false);

	Log.write ("Server: <-- " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	server.network->sendTo (socketNumber, message.iLength, message.serialize());
}

//------------------------------------------------------------------------------
void sendReconnectAnswer (cServer& server, int socketNumber, const cPlayer& player)
{
	cNetMessage message (GAME_EV_RECONNECT_ANSWER);

	const std::vector<cPlayer*>& playerList = server.PlayerList;
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		cPlayer const* SecondPlayer = playerList[i];
		if (&player == SecondPlayer) continue;
		message.pushInt16 (SecondPlayer->getNr());
		message.pushInt16 (SecondPlayer->getColor());
		message.pushString (SecondPlayer->getName());
	}
	message.pushInt16 ( (int) playerList.size());
	message.pushString (server.Map->getName());
	message.pushInt16 (player.getColor());
	message.pushInt16 (player.getNr());

	message.pushBool (true);

	Log.write ("Server: <-- " + message.getTypeAsString() + ", Hexdump: " + message.getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	server.network->sendTo (socketNumber, message.iLength, message.serialize());
}

//------------------------------------------------------------------------------
void sendTurn (cServer& server, int turn, unsigned int gameTime, const cPlayer& player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_TURN);
	message->pushInt32 (gameTime);
	message->pushInt16 (turn);
	server.sendNetMessage (message, player.getNr());
}

//------------------------------------------------------------------------------
void sendHudSettings (cServer& server, const cPlayer& player)
{
	const sHudStateContainer& hudStates = *player.savedHud;
	cNetMessage* message = new cNetMessage (GAME_EV_HUD_SETTINGS);

	message->pushBool (hudStates.tntChecked);
	message->pushBool (hudStates.hitsChecked);
	message->pushBool (hudStates.lockChecked);
	message->pushBool (hudStates.surveyChecked);
	message->pushBool (hudStates.statusChecked);
	message->pushBool (hudStates.scanChecked);
	message->pushBool (hudStates.rangeChecked);
	message->pushBool (hudStates.twoXChecked);
	message->pushBool (hudStates.fogChecked);
	message->pushBool (hudStates.ammoChecked);
	message->pushBool (hudStates.gridChecked);
	message->pushBool (hudStates.colorsChecked);
	message->pushFloat (hudStates.zoom);
	message->pushInt16 (hudStates.offY);
	message->pushInt16 (hudStates.offX);
	message->pushInt16 (hudStates.selUnitID);
	server.sendNetMessage (message, player.getNr());
}

//------------------------------------------------------------------------------
void sendStoreVehicle (cServer& server, int unitid, bool vehicle, int storedunitid, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_STORE_UNIT);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (storedunitid);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendActivateVehicle (cServer& server, int unitid, bool vehicle, int activatunitid, int x, int y, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_EXIT_UNIT);
	message->pushInt16 (y);
	message->pushInt16 (x);
	message->pushInt16 (unitid);
	message->pushBool (vehicle);
	message->pushInt16 (activatunitid);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendDeleteEverything (cServer& server, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_DELETE_EVERYTHING);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendResearchLevel (cServer& server, const cResearch& researchLevel, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_RESEARCH_LEVEL);
	for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	{
		message->pushInt16 (researchLevel.getCurResearchLevel (area));
		message->pushInt16 (researchLevel.getCurResearchPoints (area));
	}
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendRefreshResearchCount (cServer& server, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_REFRESH_RESEARCH_COUNT);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendUnitUpgrades (cServer& server, const sUnitData& unitData, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_UNIT_UPGRADE_VALUES);
	message->pushInt16 (unitData.hitpointsMax);
	message->pushInt16 (unitData.ammoMax);
	message->pushInt16 (unitData.shotsMax);
	message->pushInt16 (unitData.speedMax);
	message->pushInt16 (unitData.armor);
	message->pushInt16 (unitData.buildCosts);
	message->pushInt16 (unitData.damage);
	message->pushInt16 (unitData.range);
	message->pushInt16 (unitData.scan);
	message->pushInt16 (unitData.version);
	message->pushID (unitData.ID);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendCredits (cServer& server, int newCredits, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_CREDITS_CHANGED);
	message->pushInt16 (newCredits);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendUpgradeBuildings (cServer& server, const std::vector<cBuilding*>& upgradedBuildings, int totalCosts, int player)
{
	// send to owner
	cNetMessage* message = NULL;
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
			message->pushInt16 ( (totalCosts * buildingsInMsg) / (int) upgradedBuildings.size());
			message->pushInt16 (buildingsInMsg);
			server.sendNetMessage (message, player);
			message = NULL;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 ( (int) (totalCosts * buildingsInMsg) / (int) upgradedBuildings.size());
		message->pushInt16 (buildingsInMsg);
		server.sendNetMessage (message, player);
		message = NULL;
	}

	// send to other players
	const std::vector<cPlayer*>& playerList = server.PlayerList;
	for (unsigned int n = 0; n < playerList.size(); n++)
	{
		const cPlayer* curPlayer = playerList[n];
		// don't send to the owner of the buildings
		if (curPlayer == 0 || curPlayer->getNr() == player) continue;

		for (unsigned int buildingIdx = 0; buildingIdx < upgradedBuildings.size(); buildingIdx++)
		{
			if (Contains (upgradedBuildings[buildingIdx]->seenByPlayerList, curPlayer)) // that player can see the building
				sendUnitData (server, *upgradedBuildings[buildingIdx], curPlayer->getNr());
		}
	}
}

//------------------------------------------------------------------------------
void sendUpgradeVehicles (cServer& server, const std::vector<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, int player)
{
	if (upgradedVehicles.size() * 4 > PACKAGE_LENGTH - 50)
	{
		Log.write ("Server: sendUpgradeVehicles: Message would exceed messagesize!!!", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	// send to owner
	cNetMessage* message = new cNetMessage (GAME_EV_UPGRADED_VEHICLES);
	for (unsigned int i = 0; i < upgradedVehicles.size(); i++)
		message->pushInt32 (upgradedVehicles[i]->iID);

	message->pushInt32 (storingBuildingID);
	message->pushInt16 (totalCosts);
	message->pushInt16 ( (int) upgradedVehicles.size());
	server.sendNetMessage (message, player);

	//TODO: send to other players as well?
}

//------------------------------------------------------------------------------
void sendResearchSettings (cServer& server, const std::vector<cBuilding*>& researchCentersToChangeArea, const std::vector<int>& newAreasForResearchCenters, int player)
{
	if (researchCentersToChangeArea.size() != newAreasForResearchCenters.size())
		return;

	cNetMessage* message = NULL;
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
			server.sendNetMessage (message, player);
			message = NULL;
		}
	}
	if (message != NULL)
	{
		message->pushInt16 (buildingsInMsg);
		server.sendNetMessage (message, player);
		message = NULL;
	}
}

//------------------------------------------------------------------------------
void sendClans (cServer& server, const std::vector<cPlayer*>& playerList, const cPlayer* toPlayer)
{
	if (toPlayer == NULL)
		return;
	cNetMessage* message = new cNetMessage (GAME_EV_PLAYER_CLANS);
	for (unsigned int i = 0; i < playerList.size(); i++)
	{
		message->pushChar (playerList[i]->getClan());
		message->pushChar (playerList[i]->getNr());
	}
	server.sendNetMessage (message, toPlayer->getNr());
}

//------------------------------------------------------------------------------
void sendGameTime (cServer& server, const cPlayer& player, int gameTime)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SET_GAME_TIME);
	message->pushInt32 (gameTime);
	server.sendNetMessage (message, player.getNr());
}

//------------------------------------------------------------------------------
void sendClansToClients (cServer& server, const std::vector<cPlayer*>& playerList)
{
	for (unsigned int n = 0; n < playerList.size(); n++)
		sendClans (server, playerList, playerList[n]);
}

//------------------------------------------------------------------------------
void sendSetAutomoving (cServer& server, const cVehicle& vehicle)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SET_AUTOMOVE);
	message->pushInt16 (vehicle.iID);
	server.sendNetMessage (message, vehicle.owner->getNr());
}

//------------------------------------------------------------------------------
void sendCommandoAnswer (cServer& server, bool success, bool steal, const cVehicle& srcUnit, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_COMMANDO_ANSWER);
	message->pushInt16 (srcUnit.iID);
	message->pushBool (steal);
	message->pushBool (success);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendRequestSaveInfo (cServer& server, int saveingID)
{
	cNetMessage* message = new cNetMessage (GAME_EV_REQ_SAVE_INFO);
	message->pushInt16 (saveingID);
	server.sendNetMessage (message);
}

//------------------------------------------------------------------------------
void sendSavedReport (cServer& server, const sSavedReportMessage& savedReport, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SAVED_REPORT);

	savedReport.pushInto (*message);
	server.sendNetMessage (message, player);
}

//------------------------------------------------------------------------------
void sendCasualtiesReport (cServer& server, int player)
{
	cCasualtiesTracker* casualtiesTracker = server.getCasualtiesTracker();
	if (casualtiesTracker)
	{
		std::vector<cNetMessage*> messages;
		casualtiesTracker->prepareNetMessagesForClient (messages, GAME_EV_CASUALTIES_REPORT);
		for (size_t i = 0; i < messages.size(); i++)
			server.sendNetMessage (messages[i], player);
	}
}

//------------------------------------------------------------------------------
void sendSelfDestroy (cServer& server, const cBuilding& building)
{
	cNetMessage* message = new cNetMessage (GAME_EV_SELFDESTROY);
	message->pushInt16 (building.iID);
	server.sendNetMessage (message, building.owner->getNr());

	for (unsigned int i = 0; i < building.seenByPlayerList.size(); i++)
	{
		cNetMessage* message = new cNetMessage (GAME_EV_SELFDESTROY);
		message->pushInt16 (building.iID);
		server.sendNetMessage (message, building.seenByPlayerList[i]->getNr());
	}
}

//------------------------------------------------------------------------------
void sendEndMoveActionToClient (cServer& server, const cVehicle& vehicle, int destID, eEndMoveActionType type)
{
	cNetMessage* message = new cNetMessage (GAME_EV_END_MOVE_ACTION_SERVER);
	message->pushChar (type);
	message->pushInt32 (destID);
	message->pushInt32 (vehicle.iID);

	server.sendNetMessage (message, vehicle.owner->getNr());
}
