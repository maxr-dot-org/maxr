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
#ifndef menueventsH
#define menueventsH


#include "netmessage2.h"
#include "game\data\gamesettings.h"
#include "game\data\player\playerbasicdata.h"
#include "game\logic\landingpositionstate.h"

class cMultiplayerLobbyMessage : public cNetMessage2
{
public:
	// When changing this enum, also update function enumToString(eActiontype value)!
	enum class eMessageType {
		MU_MSG_CHAT,                 // simple text message
		MU_MSG_IDENTIFIKATION,	     // player send his properties (name, color, ready)
		MU_MSG_PLAYER_NUMBER,        // host assigns a new player number to a player (used to reassign playernummers when loading a save game)

		MU_MSG_PLAYERLIST,			// a list with all players and their data
		MU_MSG_OPTIONS,				// all options selected by the host
		// Map down/up-load
		MU_MSG_START_MAP_DOWNLOAD,    // the host start a map upload to the client
		MU_MSG_MAP_DOWNLOAD_DATA,     // the host sends map data to the client
		MU_MSG_CANCELED_MAP_DOWNLOAD, // the host canceled the map upload to the client
		MU_MSG_FINISHED_MAP_DOWNLOAD, // the host finished uploading the map
		MU_MSG_REQUEST_MAP,           // a player wants to download a map from the server
		// Game Preparation
		MU_MSG_GO,                  // host wants to start the game/preparation
		MU_MSG_LANDING_STATE,       // informs a client about the state of the landing position selection he is currently in
		MU_MSG_LANDING_POSITION,	// landing position during landing position selection
		MU_MSG_ALL_LANDED,          // all players have selected there landing points and clients can start game
		MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS,
		MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION,
		MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION
	};
	static std::unique_ptr<cMultiplayerLobbyMessage> createFromBuffer(cBinaryArchiveOut& archive);

	eMessageType getType() const;

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

//------------------------------------------------------------------------------
class cMuMsgChat : public cMultiplayerLobbyMessage
{
public:
	cMuMsgChat(const std::string& message, bool translate = false);
	cMuMsgChat(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

	std::string message;
	bool translate;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & message;
		archive & translate;
	}
};

//------------------------------------------------------------------------------
class cMuMsgPlayerNr : public cMultiplayerLobbyMessage
{
public:
	cMuMsgPlayerNr(int newPlayerNr);
	cMuMsgPlayerNr(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

	std::string saveGameName;
	std::vector<cPlayerBasicData> savePlayers;
	std::string mapName;
	Uint32 mapCrc;
	cGameSettings settings;
	bool settingsValid;

private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & saveGameName;
		archive & savePlayers;
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
	cMuMsgPlayerList(const std::vector<std::shared_ptr<cPlayerBasicData>>& playerList_);
	cMuMsgPlayerList(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

	std::vector<cPlayerBasicData> playerList;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & playerList;
	}
};

//------------------------------------------------------------------------------
class cMuMsgGo : public cMultiplayerLobbyMessage
{
public:
	cMuMsgGo();
	cMuMsgGo(cBinaryArchiveOut& archive);
};


//------------------------------------------------------------------------------
class cMuMsgPlayerHasSelectedLandingPosition : public cMultiplayerLobbyMessage
{
public:
	cMuMsgPlayerHasSelectedLandingPosition(int landedPlayer);
	cMuMsgPlayerHasSelectedLandingPosition(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

	eLandingPositionState state;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & state;
	}
};

//------------------------------------------------------------------------------
class cMuMsgAllLanded : public cMultiplayerLobbyMessage
{
public:
	cMuMsgAllLanded();
	cMuMsgAllLanded(cBinaryArchiveOut& archive);
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
	cMuMsgFinishedMapDownload(const std::string& playerName);
	cMuMsgFinishedMapDownload(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

	std::string playerName;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & playerName;
	}
};

//------------------------------------------------------------------------------
class cMuMsgLandingPosition : public cMultiplayerLobbyMessage
{
public:
	cMuMsgLandingPosition(const cPosition& position);
	cMuMsgLandingPosition(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

	virtual void serialize(cBinaryArchiveIn& archive);
	virtual void serialize(cTextArchiveIn& archive);

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

#endif // menueventsH
