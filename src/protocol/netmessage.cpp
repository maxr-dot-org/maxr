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

#include "netmessage.h"

#include "game/logic/action/action.h"
#include "utility/string/toString.h"
#include "game/data/model.h"
#include "utility/serialization/serialization.h"
#include "protocol/lobbymessage.h"
#include "game/logic/turntimeclock.h"
#include "mapdownloader/mapdownload.h"
#include "maxrversion.h"

//------------------------------------------------------------------------------
std::string enumToString (eNetMessageType value)
{
	switch (value)
	{
	case eNetMessageType::TCP_HELLO: return "TCP_HELLO";
	case eNetMessageType::TCP_WANT_CONNECT: return "TCP_WANT_CONNECT";
	case eNetMessageType::TCP_CONNECTED: return "TCP_CONNECTED";
	case eNetMessageType::TCP_CONNECT_FAILED: return "TCP_CONNECT_FAILED";
	case eNetMessageType::TCP_CLOSE: return "TCP_CLOSE";
	case eNetMessageType::ACTION: return "ACTION";
	case eNetMessageType::GAMETIME_SYNC_SERVER: return "GAMETIME_SYNC_SERVER";
	case eNetMessageType::GAMETIME_SYNC_CLIENT: return "GAMETIME_SYNC_CLIENT";
	case eNetMessageType::RANDOM_SEED: return "RANDOM_SEED";
	case eNetMessageType::FREEZE_MODES: return "FREEZE_MODES";
	case eNetMessageType::REPORT: return "REPORT";
	case eNetMessageType::GUI_SAVE_INFO: return "GUI_SAVE_INFO";
	case eNetMessageType::REQUEST_GUI_SAVE_INFO: return "REQUEST_GUI_SAVE_INFO";
	case eNetMessageType::RESYNC_MODEL: return "RESYNC_MODEL";
	case eNetMessageType::REQUEST_RESYNC_MODEL: return "REQUEST_RESYNC_MODEL";
	case eNetMessageType::MULTIPLAYER_LOBBY: return "MULTIPLAYER_LOBBY";
	case eNetMessageType::GAME_ALREADY_RUNNING: return "GAME_ALREADY_RUNNING";
	case eNetMessageType::WANT_REJOIN_GAME: return "WANT_REJOIN_GAME";
	default:
		assert (false);
		return toString (static_cast<int> (value));
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cNetMessage::createFromBuffer (const unsigned char* data, int length)
{
	cBinaryArchiveOut archive (data, length);

	eNetMessageType type;
	archive >> type;

	int playerNr;
	archive >> playerNr;

	std::unique_ptr<cNetMessage> message;
	switch (type)
	{
	case eNetMessageType::TCP_HELLO:
		message = std::make_unique<cNetMessageTcpHello> (archive); break;
	case eNetMessageType::TCP_WANT_CONNECT:
		message = std::make_unique<cNetMessageTcpWantConnect> (archive); break;
	case eNetMessageType::TCP_CONNECTED:
		message = std::make_unique<cNetMessageTcpConnected> (archive); break;
	case eNetMessageType::TCP_CONNECT_FAILED:
		message = std::make_unique<cNetMessageTcpConnectFailed> (archive); break;
	case eNetMessageType::ACTION:
		message = cAction::createFromBuffer (archive); break;
	case eNetMessageType::MULTIPLAYER_LOBBY:
		message = cMultiplayerLobbyMessage::createFromBuffer (archive); break;
	case eNetMessageType::GAMETIME_SYNC_SERVER:
		message = std::make_unique<cNetMessageSyncServer> (archive); break;
	case eNetMessageType::GAMETIME_SYNC_CLIENT:
		message = std::make_unique<cNetMessageSyncClient> (archive); break;
	case eNetMessageType::RANDOM_SEED:
		message = std::make_unique<cNetMessageRandomSeed> (archive); break;
	case eNetMessageType::FREEZE_MODES:
		message = std::make_unique<cNetMessageFreezeModes> (archive); break;
	case eNetMessageType::REPORT:
		message = std::make_unique<cNetMessageReport> (archive); break;
	case eNetMessageType::GUI_SAVE_INFO:
		message = std::make_unique<cNetMessageGUISaveInfo> (archive); break;
	case eNetMessageType::REQUEST_GUI_SAVE_INFO:
		message = std::make_unique<cNetMessageRequestGUISaveInfo> (archive); break;
	case eNetMessageType::RESYNC_MODEL:
		message = std::make_unique<cNetMessageResyncModel> (archive); break;
	case eNetMessageType::REQUEST_RESYNC_MODEL:
		message = std::make_unique<cNetMessageRequestResync> (archive); break;
	case eNetMessageType::GAME_ALREADY_RUNNING:
		message = std::make_unique<cNetMessageGameAlreadyRunning> (archive); break;
	case eNetMessageType::WANT_REJOIN_GAME:
		message = std::make_unique<cNetMessageWantRejoinGame> (archive); break;
	default:
		throw std::runtime_error ("Unknown net message type " + iToStr (static_cast<int> (type)));
		break;
	}

	if (archive.dataLeft() > 0)
	{
		//when there is unread data left in the buffer, something is wrong with the message.
		throw std::runtime_error ("cNetMessage: Error while de-serializing. Too much data in buffer");
	}

	message->playerNr = playerNr;

	return message;
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cNetMessage::clone() const
{
	std::vector<unsigned char> serialMessage;
	cBinaryArchiveIn archiveIn (serialMessage);
	archiveIn << *this;

	return cNetMessage::createFromBuffer (serialMessage.data(), serialMessage.size());
}

//------------------------------------------------------------------------------
cNetMessageTcpHello::cNetMessageTcpHello():
	cNetMessage (eNetMessageType::TCP_HELLO),
	packageVersion (PACKAGE_VERSION),
	packageRev (PACKAGE_REV)
{}

//------------------------------------------------------------------------------
cNetMessageTcpWantConnect::cNetMessageTcpWantConnect():
	cNetMessage (eNetMessageType::TCP_WANT_CONNECT),
	ready (false),
	packageVersion (PACKAGE_VERSION),
	packageRev (PACKAGE_REV),
	socket (nullptr)
{}

//------------------------------------------------------------------------------
cNetMessageTcpConnected::cNetMessageTcpConnected (int playerNr) :
	cNetMessage (eNetMessageType::TCP_CONNECTED),
	playerNr (playerNr),
	packageVersion (PACKAGE_VERSION),
	packageRev (PACKAGE_REV)
{}

//------------------------------------------------------------------------------
cNetMessageResyncModel::cNetMessageResyncModel (const cModel& model) :
	cNetMessage (eNetMessageType::RESYNC_MODEL)
{
	cBinaryArchiveIn archive (data);
	archive << model;
}

void cNetMessageResyncModel::apply (cModel& model) const
{
	serialization::cPointerLoader p (model);
	cBinaryArchiveOut archive (data.data(), data.size(), &p);
	archive >> model;
}

//------------------------------------------------------------------------------
cNetMessageGameAlreadyRunning::cNetMessageGameAlreadyRunning (const cModel& model) :
	cNetMessage (eNetMessageType::GAME_ALREADY_RUNNING)
{
	for (const auto& p : model.getPlayerList())
		playerList.push_back (cPlayerBasicData (p->getName(), p->getColor(), p->getId(), p->isDefeated));
	mapName = model.getMap()->getName();
	mapCrc = MapDownload::calculateCheckSum (mapName);
}
