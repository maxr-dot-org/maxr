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

#include "lobbymessage.h"

#include "game/data/units/unitdata.h"
#include "game/data/player/clans.h"

std::unique_ptr<cMultiplayerLobbyMessage> cMultiplayerLobbyMessage::createFromBuffer (cBinaryArchiveOut& archive)
{
	eMessageType type;
	archive >> type;

	std::unique_ptr<cMultiplayerLobbyMessage> message;
	switch (type)
	{
	case eMessageType::MU_MSG_CHAT:
		message = std::make_unique<cMuMsgChat> (archive); break;
	case eMessageType::MU_MSG_IDENTIFIKATION:
		message = std::make_unique<cMuMsgIdentification> (archive); break;
	case eMessageType::MU_MSG_PLAYER_NUMBER:
		message = std::make_unique<cMuMsgPlayerNr> (archive); break;
	case eMessageType::MU_MSG_PLAYERLIST:
		message = std::make_unique<cMuMsgPlayerList> (archive); break;
	case eMessageType::MU_MSG_OPTIONS:
		message = std::make_unique<cMuMsgOptions> (archive); break;
	case eMessageType::MU_MSG_SAVESLOTS:
		message = std::make_unique<cMuMsgSaveSlots> (archive); break;
	case eMessageType::MU_MSG_ASK_TO_FINISH_LOBBY:
		message = std::make_unique<cMuMsgAskToFinishLobby> (archive); break;
	case eMessageType::MU_MSG_CANNOT_END_LOBBY:
		message = std::make_unique<cMuMsgCannotEndLobby> (archive); break;
	case eMessageType::MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME:
		message = std::make_unique<cMuMsgDisconnectNotInSavedGame> (archive); break;
	case eMessageType::MU_MSG_START_GAME_PREPARATIONS:
		message = std::make_unique<cMuMsgStartGamePreparations> (archive); break;
	case eMessageType::MU_MSG_START_MAP_DOWNLOAD:
		message = std::make_unique<cMuMsgStartMapDownload> (archive); break;
	case eMessageType::MU_MSG_MAP_DOWNLOAD_DATA:
		message = std::make_unique<cMuMsgMapDownloadData> (archive); break;
	case eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD:
		message = std::make_unique<cMuMsgCanceledMapDownload> (archive); break;
	case eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
		message = std::make_unique<cMuMsgFinishedMapDownload> (archive); break;
	case eMessageType::MU_MSG_REQUEST_MAP:
		message = std::make_unique<cMuMsgRequestMap> (archive); break;
	case eMessageType::MU_MSG_LANDING_STATE:
		message = std::make_unique<cMuMsgLandingState> (archive); break;
	case eMessageType::MU_MSG_LANDING_POSITION:
		message = std::make_unique<cMuMsgLandingPosition> (archive); break;
	case eMessageType::MU_MSG_START_GAME:
		message = std::make_unique<cMuMsgStartGame> (archive); break;
	case eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS:
		message = std::make_unique<cMuMsgInLandingPositionSelectionStatus> (archive); break;
	case eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION:
		message = std::make_unique<cMuMsgPlayerHasSelectedLandingPosition> (archive); break;
	case eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION:
		message = std::make_unique<cMuMsgPlayerAbortedGamePreparations> (archive); break;

	default:
		throw std::runtime_error ("Unknown multiplayer lobby message: " + std::to_string (static_cast<int> (type)));
	}

	return message;
}

cMultiplayerLobbyMessage::eMessageType cMultiplayerLobbyMessage::getType() const
{
	return type;
}

void cMultiplayerLobbyMessage::serialize (cBinaryArchiveIn& archive)
{
	cNetMessage::serialize (archive);
	serializeThis (archive);
}

void cMultiplayerLobbyMessage::serialize (cTextArchiveIn& archive)
{
	cNetMessage::serialize (archive);
	serializeThis (archive);
}

bool ILobbyMessageHandler::handleMessage (const cNetMessage& message)
{
	if (message.getType() != eNetMessageType::MULTIPLAYER_LOBBY) return false;
	return handleMessage (static_cast<const cMultiplayerLobbyMessage&> (message));
}


std::string enumToString (cMultiplayerLobbyMessage::eMessageType value)
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
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_SAVESLOTS:
		return "MU_MSG_SAVESLOTS";
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
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_ASK_TO_FINISH_LOBBY:
		return "MU_MSG_ASK_TO_FINISH_LOBBY";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANNOT_END_LOBBY:
		return "MU_MSG_CANNOT_END_LOBBY";
	case cMultiplayerLobbyMessage::eMessageType::MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME:
		return "MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME";
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
		assert (false);
		return std::to_string (static_cast<int> (value));
	}
}

//------------------------------------------------------------------------------
cMuMsgChat::cMuMsgChat (const std::string& message) :
	message (message)
{}

cMuMsgChat::cMuMsgChat (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgChat::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgChat::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgPlayerNr::cMuMsgPlayerNr (int newPlayerNr) :
	newPlayerNr (newPlayerNr)
{}

cMuMsgPlayerNr::cMuMsgPlayerNr (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgPlayerNr::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgPlayerNr::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgOptions::cMuMsgOptions() :
	saveInfo (-1),
	mapCrc (0)
{}

cMuMsgOptions::cMuMsgOptions (cBinaryArchiveOut& archive) :
	saveInfo (-1)
{
	serializeThis (archive);
}

void cMuMsgOptions::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgOptions::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgSaveSlots::cMuMsgSaveSlots(){}

cMuMsgSaveSlots::cMuMsgSaveSlots (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgSaveSlots::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgSaveSlots::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgPlayerList::cMuMsgPlayerList (const std::vector<std::shared_ptr<cPlayerBasicData>>& playerList_)
{
	for (const auto& p : playerList_)
	{
		playerList.push_back (*p);
	}
}

//------------------------------------------------------------------------------
cMuMsgPlayerList::cMuMsgPlayerList (std::vector<cPlayerBasicData> playerList_) :
	playerList (std::move (playerList_))
{
}

cMuMsgPlayerList::cMuMsgPlayerList (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgPlayerList::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgPlayerList::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgAskToFinishLobby::cMuMsgAskToFinishLobby() {}

cMuMsgAskToFinishLobby::cMuMsgAskToFinishLobby (cBinaryArchiveOut& archive) {}

//------------------------------------------------------------------------------
cMuMsgCannotEndLobby::cMuMsgCannotEndLobby()
{}

cMuMsgCannotEndLobby::cMuMsgCannotEndLobby (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgCannotEndLobby::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgCannotEndLobby::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgDisconnectNotInSavedGame::cMuMsgDisconnectNotInSavedGame() {}
cMuMsgDisconnectNotInSavedGame::cMuMsgDisconnectNotInSavedGame (cBinaryArchiveOut& archive) {}

//------------------------------------------------------------------------------
cMuMsgStartGamePreparations::cMuMsgStartGamePreparations (std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cClanData> clanData) :
	unitsData (unitsData),
	clanData (clanData)
{}

cMuMsgStartGamePreparations::cMuMsgStartGamePreparations (cBinaryArchiveOut& archive)
{
	loadThis (archive);
}

void cMuMsgStartGamePreparations::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	saveThis (archive);
}

void cMuMsgStartGamePreparations::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	saveThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgPlayerHasSelectedLandingPosition::cMuMsgPlayerHasSelectedLandingPosition (int landedPlayer) :
	landedPlayer (landedPlayer)
{}

cMuMsgPlayerHasSelectedLandingPosition::cMuMsgPlayerHasSelectedLandingPosition (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgPlayerHasSelectedLandingPosition::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgPlayerHasSelectedLandingPosition::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgInLandingPositionSelectionStatus::cMuMsgInLandingPositionSelectionStatus (int playerNr, bool isIn) :
	landingPlayer (playerNr),
	isIn (isIn)
{}

cMuMsgInLandingPositionSelectionStatus::cMuMsgInLandingPositionSelectionStatus (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgInLandingPositionSelectionStatus::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgInLandingPositionSelectionStatus::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgLandingState::cMuMsgLandingState (eLandingPositionState state) :
	state (state)
{}

cMuMsgLandingState::cMuMsgLandingState (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgLandingState::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgLandingState::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgStartGame::cMuMsgStartGame() {}
cMuMsgStartGame::cMuMsgStartGame (cBinaryArchiveOut& archive) {}

//------------------------------------------------------------------------------
cMuMsgPlayerAbortedGamePreparations::cMuMsgPlayerAbortedGamePreparations() {}
cMuMsgPlayerAbortedGamePreparations::cMuMsgPlayerAbortedGamePreparations (cBinaryArchiveOut& archive) {}

//------------------------------------------------------------------------------
cMuMsgFinishedMapDownload::cMuMsgFinishedMapDownload() {}
cMuMsgFinishedMapDownload::cMuMsgFinishedMapDownload (cBinaryArchiveOut& archive) {}

//------------------------------------------------------------------------------
cMuMsgLandingPosition::cMuMsgLandingPosition (const cPosition& position) :
	position (position)
{}

cMuMsgLandingPosition::cMuMsgLandingPosition (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgLandingPosition::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgLandingPosition::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgRequestMap::cMuMsgRequestMap (const std::string& mapName) :
	mapName (mapName)
{}

cMuMsgRequestMap::cMuMsgRequestMap (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgRequestMap::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgRequestMap::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgStartMapDownload::cMuMsgStartMapDownload (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

cMuMsgStartMapDownload::cMuMsgStartMapDownload (const std::string mapName, int mapSize) :
	mapName (mapName),
	mapSize (mapSize)
{}

void cMuMsgStartMapDownload::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgStartMapDownload::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgMapDownloadData::cMuMsgMapDownloadData()
{}

cMuMsgMapDownloadData::cMuMsgMapDownloadData (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgMapDownloadData::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgMapDownloadData::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

//------------------------------------------------------------------------------
cMuMsgCanceledMapDownload::cMuMsgCanceledMapDownload() {}
cMuMsgCanceledMapDownload::cMuMsgCanceledMapDownload (cBinaryArchiveOut& archive) {}

//------------------------------------------------------------------------------
cMuMsgIdentification::cMuMsgIdentification (const cPlayerBasicData& player) :
	playerName (player.getName()),
	playerColor (player.getColor()),
	ready (player.isReady())
{}

cMuMsgIdentification::cMuMsgIdentification (cBinaryArchiveOut& archive)
{
	serializeThis (archive);
}

void cMuMsgIdentification::serialize (cTextArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}

void cMuMsgIdentification::serialize (cBinaryArchiveIn& archive)
{
	cMultiplayerLobbyMessage::serialize (archive);
	serializeThis (archive);
}
