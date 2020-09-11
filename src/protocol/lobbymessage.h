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

#include "protocol/netmessage.h"
#include "game/data/gamesettings.h"
#include "game/data/player/playerbasicdata.h"
#include "game/logic/landingpositionstate.h"
#include "game/data/savegameinfo.h"

class cUnitsData;
class cClanData;

class cMultiplayerLobbyMessage : public cNetMessage
{
public:
	// When changing this enum, also update function enumToString(eActiontype value)!
	enum class eMessageType {
		MU_MSG_CHAT,                 // sent by host/client: simple text message
		MU_MSG_IDENTIFIKATION,	     // sent by client: player send his properties (name, color, ready)
		MU_MSG_PLAYER_NUMBER,        // sent by host: assign a new id to a player (used to restore player id when loading a save game)

		MU_MSG_PLAYERLIST,			// sent by host: a list with all players and their data
		MU_MSG_OPTIONS,				// sent by host: all options selected by the host
		// Map down/up-load
		MU_MSG_START_MAP_DOWNLOAD,    // sent by host: start a map upload to the client
		MU_MSG_MAP_DOWNLOAD_DATA,     // sent by host: map data for the running map upload
		MU_MSG_CANCELED_MAP_DOWNLOAD, // sent by host: canceled the map upload to the client
		MU_MSG_FINISHED_MAP_DOWNLOAD, // sent by host: finished uploading the map
		MU_MSG_REQUEST_MAP,           // sent by client: a client is missing the selected map and requests a download
		// Game Preparation
		MU_MSG_START_GAME_PREPARATIONS,  // sent by host: all clients should start game preparation menus
		MU_MSG_LANDING_STATE,         // sent by host: informs a client about the state of the landing position selection he is currently in
		MU_MSG_LANDING_POSITION,	  // sent by client: selected landing position
		MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS, // sent by host/client: is player in landing selection menu?
		MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION, // sent by host: inform clients, which player have already selected a position
		MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION,  // sent by host/client: a player has left. Abort game preparations and return to lobby
		// Transition to ingame logic
		MU_MSG_START_GAME,            // sent by host: clients should start the ingame client and swith to game gui
	};
	static std::unique_ptr<cMultiplayerLobbyMessage> createFromBuffer(cBinaryArchiveOut& archive);

	eMessageType getType() const;

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

protected:
	cMultiplayerLobbyMessage(eMessageType type);
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & type;
	}

	eMessageType type;
};

std::string enumToString(cMultiplayerLobbyMessage::eMessageType value);

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
class cMuMsgChat : public cMultiplayerLobbyMessage
{
public:
	cMuMsgChat(const std::string& message, bool translate = false, const std::string& insertText = "");
	cMuMsgChat(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::string message;
	bool translate;
	std::string insertText;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & message;
		archive & translate;
		archive & insertText;
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerNr : public cMultiplayerLobbyMessage
{
public:
	cMuMsgPlayerNr(int newPlayerNr);
	cMuMsgPlayerNr(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	int newPlayerNr;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & newPlayerNr;
	}
};

//------------------------------------------------------------------------------
class cMuMsgOptions : public cMultiplayerLobbyMessage
{
public:
	cMuMsgOptions();
	cMuMsgOptions(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	cSaveGameInfo saveInfo;
	std::string mapName;
	Uint32 mapCrc;
	cGameSettings settings;
	bool settingsValid;

private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & saveInfo;
		archive & mapName;
		archive & mapCrc;
		archive & settings;
		archive & settingsValid;
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerList : public cMultiplayerLobbyMessage
{
public:
	explicit cMuMsgPlayerList(const std::vector<std::shared_ptr<cPlayerBasicData>>&);
	explicit cMuMsgPlayerList(std::vector<cPlayerBasicData>);
	cMuMsgPlayerList(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::vector<cPlayerBasicData> playerList;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & playerList;
	}
};

//------------------------------------------------------------------------------
class cMuMsgStartGamePreparations : public cMultiplayerLobbyMessage
{
public:
	cMuMsgStartGamePreparations(std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cClanData> clanData);
	cMuMsgStartGamePreparations(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::shared_ptr<const cUnitsData> unitsData;
	std::shared_ptr<const cClanData> clanData;
private:
	template<typename T>
	void loadThis(T& archive)
	{
		auto unitDataNonConst = std::make_shared<cUnitsData>();
		archive >> *unitDataNonConst;
		unitsData = unitDataNonConst;

		auto clanDataNonConst = std::make_shared<cClanData>();
		archive >> *clanDataNonConst;
		clanData = clanDataNonConst;
	}
	template<typename T>
	void saveThis(T& archive)
	{
		archive << *unitsData;
		archive << *clanData;
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerHasSelectedLandingPosition : public cMultiplayerLobbyMessage
{
public:
	cMuMsgPlayerHasSelectedLandingPosition(int landedPlayer);
	cMuMsgPlayerHasSelectedLandingPosition(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	int landedPlayer;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & landedPlayer;
	}
};

//------------------------------------------------------------------------------
class cMuMsgInLandingPositionSelectionStatus : public cMultiplayerLobbyMessage
{
public:
	cMuMsgInLandingPositionSelectionStatus(int playerNr, bool isIn);
	cMuMsgInLandingPositionSelectionStatus(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	int landingPlayer;
	bool isIn;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & landingPlayer;
		archive & isIn;
	}
};

//------------------------------------------------------------------------------
class cMuMsgLandingState : public cMultiplayerLobbyMessage
{
public:
	cMuMsgLandingState(eLandingPositionState state);
	cMuMsgLandingState(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	eLandingPositionState state;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & state;
	}
};

//------------------------------------------------------------------------------
class cMuMsgStartGame : public cMultiplayerLobbyMessage
{
public:
	cMuMsgStartGame();
	cMuMsgStartGame(cBinaryArchiveOut& archive);
};

//------------------------------------------------------------------------------
class cMuMsgGameStarted : public cMultiplayerLobbyMessage
{
public:
	cMuMsgGameStarted();
	cMuMsgGameStarted(cBinaryArchiveOut& archive);
};

//------------------------------------------------------------------------------
class cMuMsgPlayerAbortedGamePreparations : public cMultiplayerLobbyMessage
{
public:
	cMuMsgPlayerAbortedGamePreparations();
	cMuMsgPlayerAbortedGamePreparations(cBinaryArchiveOut& archive);
};

//------------------------------------------------------------------------------
class cMuMsgFinishedMapDownload : public cMultiplayerLobbyMessage
{
public:
	cMuMsgFinishedMapDownload();
	cMuMsgFinishedMapDownload(cBinaryArchiveOut& archive);
};

//------------------------------------------------------------------------------
class cMuMsgLandingPosition : public cMultiplayerLobbyMessage
{
public:
	cMuMsgLandingPosition(const cPosition& position);
	cMuMsgLandingPosition(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	cPosition position;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & position;
	}
};

//------------------------------------------------------------------------------
class cMuMsgRequestMap : public cMultiplayerLobbyMessage
{
public:
	cMuMsgRequestMap(const std::string& mapName);
	cMuMsgRequestMap(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::string mapName;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & mapName;
	}
};

//------------------------------------------------------------------------------
class cMuMsgStartMapDownload : public cMultiplayerLobbyMessage
{
public:
	cMuMsgStartMapDownload(const std::string mapName, int mapSize);
	cMuMsgStartMapDownload(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::string mapName;
	int mapSize;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & mapName;
		archive & mapSize;
	}
};

//------------------------------------------------------------------------------
class cMuMsgMapDownloadData : public cMultiplayerLobbyMessage
{
public:
	cMuMsgMapDownloadData();
	cMuMsgMapDownloadData(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::vector<char> data;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & data;
	}
};

//------------------------------------------------------------------------------
class cMuMsgCanceledMapDownload : public cMultiplayerLobbyMessage
{
public:
	cMuMsgCanceledMapDownload();
	cMuMsgCanceledMapDownload(cBinaryArchiveOut& archive);
};

//------------------------------------------------------------------------------
class cMuMsgIdentification : public cMultiplayerLobbyMessage
{
public:
	cMuMsgIdentification(const cPlayerBasicData& player);
	cMuMsgIdentification(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override;
	void serialize(cTextArchiveIn& archive) override;

	std::string playerName;
	cRgbColor playerColor;
	bool ready;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & playerColor;
		archive & playerName;
		archive & ready;
	}
};

#endif
