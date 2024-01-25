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
#ifndef protocol_lobbymessageH
#define protocol_lobbymessageH

#include "game/data/gamesettings.h"
#include "game/data/player/clans.h"
#include "game/data/player/playerbasicdata.h"
#include "game/data/savegameinfo.h"
#include "game/logic/landingpositionstate.h"
#include "game/protocol/netmessage.h"

class cUnitsData;

class cMultiplayerLobbyMessage : public cNetMessageT<eNetMessageType::MULTIPLAYER_LOBBY>
{
public:
	// When changing this enum, also update sEnumStringMapping<eMessageType>::m
	enum class eMessageType
	{
		MU_MSG_CHAT, // sent by host/client: simple text message
		MU_MSG_IDENTIFIKATION, // sent by client: player send his properties (name, color, ready)
		MU_MSG_PLAYER_NUMBER, // sent by host: assign a new id to a player (used to restore player id when loading a save game)

		MU_MSG_PLAYERLIST, // sent by host: a list with all players and their data
		MU_MSG_OPTIONS, // sent by client/host: all options selected by the host
		MU_MSG_SAVESLOTS, // sent by (dedicated) server: list of slots for load game
		// Map down/up-load
		MU_MSG_START_MAP_DOWNLOAD, // sent by host: start a map upload to the client
		MU_MSG_MAP_DOWNLOAD_DATA, // sent by host: map data for the running map upload
		MU_MSG_CANCELED_MAP_DOWNLOAD, // sent by host: canceled the map upload to the client
		MU_MSG_FINISHED_MAP_DOWNLOAD, // sent by host: finished uploading the map
		MU_MSG_REQUEST_MAP, // sent by client: a client is missing the selected map and requests a download
		// Game Preparation
		MU_MSG_ASK_TO_FINISH_LOBBY, // sent by client: ask to start game preparation
		MU_MSG_CANNOT_END_LOBBY, // sent by server: inform client why game preparation cannot start
		MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME, // sent by host: client is not part of saved game
		MU_MSG_START_GAME_PREPARATIONS, // sent by host: all clients should start game preparation menus
		MU_MSG_LANDING_STATE, // sent by host: informs a client about the state of the landing position selection he is currently in
		MU_MSG_LANDING_POSITION, // sent by client: selected landing position
		MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS, // sent by host/client: is player in landing selection menu?
		MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION, // sent by host: inform clients, which player have already selected a position
		MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION, // sent by host/client: a player has left. Abort game preparations and return to lobby
		// Transition to ingame logic
		MU_MSG_START_GAME, // sent by host: clients should start the ingame client and switch to game gui
	};
	static std::unique_ptr<cMultiplayerLobbyMessage> createFromBuffer (cBinaryArchiveIn& archive);

	eMessageType getType() const;

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cNetMessage::serialize (archive);
		serializeThis (archive);
	}

protected:
	cMultiplayerLobbyMessage (eMessageType type) :
		type (type) {}

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & serialization::makeNvp ("lobbyMessage", type);
		// clang-format on
	}

	eMessageType type;
};

//------------------------------------------------------------------------------
template <cMultiplayerLobbyMessage::eMessageType MsgType>
class cMultiplayerLobbyMessageT : public cMultiplayerLobbyMessage
{
public:
	cMultiplayerLobbyMessageT() :
		cMultiplayerLobbyMessage (MsgType) {}
};

namespace serialization
{
	template <>
	struct sEnumStringMapping<cMultiplayerLobbyMessage::eMessageType>
	{
		static const std::vector<std::pair<cMultiplayerLobbyMessage::eMessageType, const char*>> m;
	};
} // namespace serialization
/**
* Interface called each time a message should be handled.
*/
class ILobbyMessageHandler : public INetMessageHandler
{
protected:
	/**
	 * potentially handle the message
	 * @return if message is handled
	 */
	bool handleMessage (const cNetMessage&) final;

public:
	virtual bool handleMessage (const cMultiplayerLobbyMessage&) = 0;
};

//------------------------------------------------------------------------------
class cMuMsgChat : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_CHAT>
{
public:
	cMuMsgChat (const std::string& message);
	cMuMsgChat (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::string message;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (message);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerNr : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_NUMBER>
{
public:
	cMuMsgPlayerNr (int newPlayerNr);
	cMuMsgPlayerNr (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	int newPlayerNr;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (newPlayerNr);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgOptions : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_OPTIONS>
{
public:
	cMuMsgOptions();
	explicit cMuMsgOptions (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	cSaveGameInfo saveInfo;
	std::filesystem::path mapFilename;
	Uint32 mapCrc = 0;
	std::optional<cGameSettings> settings;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (saveInfo);
		archive & NVP (mapFilename);
		archive & NVP (mapCrc);
		archive & NVP (settings);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgSaveSlots : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_SAVESLOTS>
{
public:
	cMuMsgSaveSlots();
	cMuMsgSaveSlots (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::vector<cSaveGameInfo> saveGames;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (saveGames);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerList : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYERLIST>
{
public:
	explicit cMuMsgPlayerList (const std::vector<std::shared_ptr<cPlayerBasicData>>&);
	explicit cMuMsgPlayerList (std::vector<cPlayerBasicData>);
	cMuMsgPlayerList (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::vector<cPlayerBasicData> playerList;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (playerList);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgAskToFinishLobby : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_ASK_TO_FINISH_LOBBY>
{
public:
	cMuMsgAskToFinishLobby();
	cMuMsgAskToFinishLobby (cBinaryArchiveIn& archive);
};

//------------------------------------------------------------------------------
class cMuMsgCannotEndLobby : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANNOT_END_LOBBY>
{
public:
	cMuMsgCannotEndLobby();
	cMuMsgCannotEndLobby (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	bool missingSettings = false;
	std::vector<cPlayerBasicData> notReadyPlayers;
	bool hostNotInSavegame = false;
	std::vector<cPlayerBasicData> missingPlayers;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (missingSettings);
		archive & NVP (notReadyPlayers);
		archive & NVP (hostNotInSavegame);
		archive & NVP (missingPlayers);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgDisconnectNotInSavedGame : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_DISCONNECT_NOT_IN_SAVED_GAME>
{
public:
	cMuMsgDisconnectNotInSavedGame();
	cMuMsgDisconnectNotInSavedGame (cBinaryArchiveIn& archive);
};

//------------------------------------------------------------------------------
class cMuMsgStartGamePreparations : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME_PREPARATIONS>
{
public:
	cMuMsgStartGamePreparations (std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cClanData> clanData);
	cMuMsgStartGamePreparations (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		saveThis (archive);
	}

	std::shared_ptr<const cUnitsData> unitsData;
	std::shared_ptr<const cClanData> clanData;

private:
	template <typename Archive>
	void loadThis (Archive& archive)
	{
		auto unitDataNonConst = std::make_shared<cUnitsData>();
		archive >> serialization::makeNvp ("unitsData", *unitDataNonConst);
		unitsData = unitDataNonConst;

		auto clanDataNonConst = std::make_shared<cClanData>();
		archive >> serialization::makeNvp ("clanData", *clanDataNonConst);
		clanData = clanDataNonConst;
	}
	template <typename Archive>
	void saveThis (Archive& archive)
	{
		archive << serialization::makeNvp ("unitsData", *unitsData);
		archive << serialization::makeNvp ("clanData", *clanData);
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerHasSelectedLandingPosition : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION>
{
public:
	cMuMsgPlayerHasSelectedLandingPosition (int landedPlayer);
	cMuMsgPlayerHasSelectedLandingPosition (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	int landedPlayer;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (landedPlayer);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgInLandingPositionSelectionStatus : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS>
{
public:
	cMuMsgInLandingPositionSelectionStatus (int playerNr, bool isIn);
	cMuMsgInLandingPositionSelectionStatus (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	int landingPlayer;
	bool isIn;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (landingPlayer);
		archive & NVP (isIn);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgLandingState : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_STATE>
{
public:
	cMuMsgLandingState (eLandingPositionState state);
	cMuMsgLandingState (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	eLandingPositionState state;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (state);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgStartGame : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_GAME>
{
public:
	cMuMsgStartGame();
	cMuMsgStartGame (cBinaryArchiveIn& archive);
};

//------------------------------------------------------------------------------
class cMuMsgPlayerAbortedGamePreparations : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION>
{
public:
	cMuMsgPlayerAbortedGamePreparations();
	cMuMsgPlayerAbortedGamePreparations (cBinaryArchiveIn& archive);
};

//------------------------------------------------------------------------------
class cMuMsgFinishedMapDownload : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD>
{
public:
	cMuMsgFinishedMapDownload();
	cMuMsgFinishedMapDownload (cBinaryArchiveIn& archive);
};

//------------------------------------------------------------------------------
class cMuMsgLandingPosition : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_LANDING_POSITION>
{
public:
	cMuMsgLandingPosition (const cPosition& position);
	cMuMsgLandingPosition (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	cPosition position;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (position);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgRequestMap : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_REQUEST_MAP>
{
public:
	cMuMsgRequestMap (const std::filesystem::path& mapFilename);
	cMuMsgRequestMap (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::filesystem::path mapFilename;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (mapFilename);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgStartMapDownload : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_MAP_DOWNLOAD>
{
public:
	cMuMsgStartMapDownload (const std::filesystem::path& mapFilename, int mapSize);
	cMuMsgStartMapDownload (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::filesystem::path mapFilename;
	int mapSize;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (mapFilename);
		archive & NVP (mapSize);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgMapDownloadData : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_MAP_DOWNLOAD_DATA>
{
public:
	cMuMsgMapDownloadData();
	cMuMsgMapDownloadData (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::vector<char> data;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (data);
		// clang-format on
	}
};

//------------------------------------------------------------------------------
class cMuMsgCanceledMapDownload : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD>
{
public:
	cMuMsgCanceledMapDownload();
	cMuMsgCanceledMapDownload (cBinaryArchiveIn& archive);
};

//------------------------------------------------------------------------------
class cMuMsgIdentification : public cMultiplayerLobbyMessageT<cMultiplayerLobbyMessage::eMessageType::MU_MSG_IDENTIFIKATION>
{
public:
	cMuMsgIdentification (const cPlayerBasicData& player);
	cMuMsgIdentification (cBinaryArchiveIn& archive);

	void serialize (cBinaryArchiveOut& archive) override;
	void serialize (cJsonArchiveOut& archive) override
	{
		cMultiplayerLobbyMessage::serialize (archive);
		serializeThis (archive);
	}

	std::string playerName;
	cRgbColor playerColor;
	bool ready;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (playerColor);
		archive & NVP (playerName);
		archive & NVP (ready);
		// clang-format on
	}
};

#endif
