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

#include <memory>

#include "menuevents.h"
#include "utility/string/toString.h"
#include "game/data/units/unitdata.h"
#include "game/data/player/clans.h"

std::unique_ptr<cMultiplayerLobbyMessage> cMultiplayerLobbyMessage::createFromBuffer(cBinaryArchiveOut& archive)
{
	eMessageType type;
	archive >> type;

	std::unique_ptr<cMultiplayerLobbyMessage> message;
	switch (type)
	{
	case eMessageType::MU_MSG_CHAT:
		message = std::make_unique<cMuMsgChat>(archive); break;
	case eMessageType::MU_MSG_IDENTIFIKATION:
		message = std::make_unique<cMuMsgIdentification>(archive); break;
	case eMessageType::MU_MSG_PLAYER_NUMBER:
		message = std::make_unique<cMuMsgPlayerNr>(archive); break;
	case eMessageType::MU_MSG_PLAYERLIST:
		message = std::make_unique<cMuMsgPlayerList>(archive); break;
	case eMessageType::MU_MSG_OPTIONS:
		message = std::make_unique<cMuMsgOptions>(archive); break;
	case eMessageType::MU_MSG_START_GAME_PREPARATIONS:
		message = std::make_unique<cMuMsgStartGamePreparations>(archive); break;
	case eMessageType::MU_MSG_START_MAP_DOWNLOAD:
		message = std::make_unique<cMuMsgStartMapDownload>(archive); break;
	case eMessageType::MU_MSG_MAP_DOWNLOAD_DATA:
		message = std::make_unique<cMuMsgMapDownloadData>(archive); break;
	case eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD:
		message = std::make_unique<cMuMsgCanceledMapDownload>(archive); break;
	case eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
		message = std::make_unique<cMuMsgFinishedMapDownload>(archive); break;
	case eMessageType::MU_MSG_REQUEST_MAP:
		message = std::make_unique<cMuMsgRequestMap>(archive); break;
	case eMessageType::MU_MSG_LANDING_STATE:
		message = std::make_unique<cMuMsgLandingState>(archive); break;
	case eMessageType::MU_MSG_LANDING_POSITION:
		message = std::make_unique<cMuMsgLandingPosition>(archive); break;
	case eMessageType::MU_MSG_START_GAME:
		message = std::make_unique<cMuMsgStartGame>(archive); break;
	case eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
		message = std::make_unique<cMuMsgInLandingPositionSelectionStatus>(archive); break;
	case eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION:
		message = std::make_unique<cMuMsgPlayerHasSelectedLandingPosition>(archive); break;
	case eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
		message = std::make_unique<cMuMsgPlayerAbortedGamePreparations>(archive); break;

	default:
		throw std::runtime_error("Unknown multiplayer lobby message: " + toString(static_cast<int>(type)));
		break;
	}

	return message;
}

cMultiplayerLobbyMessage::eMessageType cMultiplayerLobbyMessage::getType() const
{
	return type;
}

void cMultiplayerLobbyMessage::serialize(cBinaryArchiveIn& archive)
{
	cNetMessage2::serialize(archive); 
	serializeThis(archive);
}

void cMultiplayerLobbyMessage::serialize(cTextArchiveIn& archive)
{
	cNetMessage2::serialize(archive); 
	serializeThis(archive);
}

cMultiplayerLobbyMessage::cMultiplayerLobbyMessage(eMessageType type) : 
	cNetMessage2(eNetMessageType::MULTIPLAYER_LOBBY), type(type)
{}

std::string enumToString(cMultiplayerLobbyMessage::eMessageType value)
{
	switch (value)
	{
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT: 
		return "MU_MSG_CHAT";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IDENTIFIKATION:
		return "MU_MSG_IDENTIFIKATION";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_NUMBER:
		return "MU_MSG_PLAYER_NUMBER";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYERLIST:
		return "MU_MSG_PLAYERLIST";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_OPTIONS:
		return "MU_MSG_OPTIONS";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_MAP_DOWNLOAD:
		return "MU_MSG_START_MAP_DOWNLOAD";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_MAP_DOWNLOAD_DATA:
		return "MU_MSG_MAP_DOWNLOAD_DATA";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD:
		return "MU_MSG_CANCELED_MAP_DOWNLOAD";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
		return "MU_MSG_FINISHED_MAP_DOWNLOAD";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_REQUEST_MAP:
		return "MU_MSG_REQUEST_MAP";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME_PREPARATIONS:
		return "MU_MSG_START_GAME_PREPARATIONS";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_STATE:
		return "MU_MSG_LANDING_STATE";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_POSITION:
		return "MU_MSG_LANDING_POSITION";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME:
		return "MU_MSG_START_GAME";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
		return "MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION:
		return "MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
		return "MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION";
	default:
		assert(false);
		return toString(static_cast<int>(value));
	}
}

//------------------------------------------------------------------------------
cMuMsgChat::cMuMsgChat(const std::string& message, bool translate /*= false*/, const std::string& insertText /*= ""*/) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_CHAT),
	message(message),
	translate(translate),
	insertText(insertText)
{}

cMuMsgChat::cMuMsgChat(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_CHAT)
{
	serializeThis(archive);
}

void cMuMsgChat::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgChat::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgPlayerNr::cMuMsgPlayerNr(int newPlayerNr) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYER_NUMBER),
	newPlayerNr(newPlayerNr)
{}

cMuMsgPlayerNr::cMuMsgPlayerNr(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYER_NUMBER)
{
	serializeThis(archive);
}

void cMuMsgPlayerNr::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgPlayerNr::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgOptions::cMuMsgOptions() :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_OPTIONS),
	saveInfo(-1),
	mapCrc(0),
	settingsValid(false)
{}

cMuMsgOptions::cMuMsgOptions(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_OPTIONS),
	saveInfo(-1)
{
	serializeThis(archive);
}

void cMuMsgOptions::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgOptions::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgPlayerList::cMuMsgPlayerList(const std::vector<std::shared_ptr<cPlayerBasicData>>& playerList_) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYERLIST)
{
	for (const auto& p : playerList_)
	{
		playerList.push_back(*p);
	}
}

cMuMsgPlayerList::cMuMsgPlayerList(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYERLIST)
{
	serializeThis(archive);
}

void cMuMsgPlayerList::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgPlayerList::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgStartGamePreparations::cMuMsgStartGamePreparations(std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cClanData> clanData) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_START_GAME_PREPARATIONS),
	unitsData(unitsData),
	clanData(clanData)
{}

cMuMsgStartGamePreparations::cMuMsgStartGamePreparations(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_START_GAME_PREPARATIONS)
{
	loadThis(archive);
}

void cMuMsgStartGamePreparations::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	saveThis(archive);
}

void cMuMsgStartGamePreparations::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	saveThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgPlayerHasSelectedLandingPosition::cMuMsgPlayerHasSelectedLandingPosition(int landedPlayer) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION),
	landedPlayer(landedPlayer)
{}

cMuMsgPlayerHasSelectedLandingPosition::cMuMsgPlayerHasSelectedLandingPosition(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION)
{
	serializeThis(archive);
}

void cMuMsgPlayerHasSelectedLandingPosition::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

void cMuMsgPlayerHasSelectedLandingPosition::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgInLandingPositionSelectionStatus::cMuMsgInLandingPositionSelectionStatus(int playerNr, bool isIn) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS),
	landingPlayer(playerNr),
	isIn(isIn)
{}

cMuMsgInLandingPositionSelectionStatus::cMuMsgInLandingPositionSelectionStatus(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS)
{
	serializeThis(archive);
}

void cMuMsgInLandingPositionSelectionStatus::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgInLandingPositionSelectionStatus::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgLandingState::cMuMsgLandingState(eLandingPositionState state) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_LANDING_STATE),
	state(state)
{}

cMuMsgLandingState::cMuMsgLandingState(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_LANDING_STATE)
{
	serializeThis(archive);
}

void cMuMsgLandingState::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

void cMuMsgLandingState::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgStartGame::cMuMsgStartGame(cBinaryArchiveOut& archive) :
cMultiplayerLobbyMessage(eMessageType::MU_MSG_START_GAME)
{}

cMuMsgStartGame::cMuMsgStartGame() :
cMultiplayerLobbyMessage(eMessageType::MU_MSG_START_GAME)
{}

//------------------------------------------------------------------------------
cMuMsgPlayerAbortedGamePreparations::cMuMsgPlayerAbortedGamePreparations(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION)
{}

cMuMsgPlayerAbortedGamePreparations::cMuMsgPlayerAbortedGamePreparations() :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION)
{}


//------------------------------------------------------------------------------
cMuMsgFinishedMapDownload::cMuMsgFinishedMapDownload(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD)
{
	serializeThis(archive);
}

cMuMsgFinishedMapDownload::cMuMsgFinishedMapDownload(const std::string& playerName) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD),
	playerName(playerName)
{}

void cMuMsgFinishedMapDownload::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

void cMuMsgFinishedMapDownload::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgLandingPosition::cMuMsgLandingPosition(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_LANDING_POSITION)
{
	serializeThis(archive);
}

cMuMsgLandingPosition::cMuMsgLandingPosition(const cPosition& position) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_LANDING_POSITION),
	position(position)
{}

void cMuMsgLandingPosition::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgLandingPosition::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgRequestMap::cMuMsgRequestMap(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_REQUEST_MAP)
{
	serializeThis(archive);
}

cMuMsgRequestMap::cMuMsgRequestMap(const std::string& mapName) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_REQUEST_MAP)
{}

void cMuMsgRequestMap::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

void cMuMsgRequestMap::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgStartMapDownload::cMuMsgStartMapDownload(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_START_MAP_DOWNLOAD)
{
	serializeThis(archive);
}

cMuMsgStartMapDownload::cMuMsgStartMapDownload(const std::string mapName, int mapSize) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_START_MAP_DOWNLOAD),
	mapName(mapName),
	mapSize(mapSize)
{}

void cMuMsgStartMapDownload::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgStartMapDownload::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgMapDownloadData::cMuMsgMapDownloadData(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_MAP_DOWNLOAD_DATA)
{
	serializeThis(archive);
}

cMuMsgMapDownloadData::cMuMsgMapDownloadData() :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_MAP_DOWNLOAD_DATA)
{}

void cMuMsgMapDownloadData::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive); 
	serializeThis(archive);
}

void cMuMsgMapDownloadData::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

//------------------------------------------------------------------------------
cMuMsgCanceledMapDownload::cMuMsgCanceledMapDownload(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD)
{}

cMuMsgCanceledMapDownload::cMuMsgCanceledMapDownload() :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD)
{}

//------------------------------------------------------------------------------
cMuMsgIdentification::cMuMsgIdentification(cBinaryArchiveOut& archive) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_IDENTIFIKATION)
{
	serializeThis(archive);
}

cMuMsgIdentification::cMuMsgIdentification(const cPlayerBasicData& player) :
	cMultiplayerLobbyMessage(eMessageType::MU_MSG_IDENTIFIKATION),
	playerName(player.getName()),
	playerColor(player.getColor().getColor()),
	ready(player.isReady())
{}

void cMuMsgIdentification::serialize(cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}

void cMuMsgIdentification::serialize(cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize(archive);
	serializeThis(archive);
}
