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

#include "netmessage.h"

#include "game/data/model.h"
#include "game/logic/action/action.h"
#include "game/logic/turntimeclock.h"
#include "game/protocol/lobbymessage.h"
#include "mapdownloader/mapdownload.h"
#include "maxrversion.h"
#include "utility/serialization/serialization.h"

#include <cassert>
#include <memory>

//------------------------------------------------------------------------------
namespace serialization
{
	const std::vector<std::pair<eNetMessageType, const char*>>
		sEnumStringMapping<eNetMessageType>::m =
			{
				{eNetMessageType::TCP_HELLO, "TCP_HELLO"},
				{eNetMessageType::TCP_WANT_CONNECT, "TCP_WANT_CONNECT"},
				{eNetMessageType::TCP_CONNECTED, "TCP_CONNECTED"},
				{eNetMessageType::TCP_CONNECT_FAILED, "TCP_CONNECT_FAILED"},
				{eNetMessageType::TCP_CLOSE, "TCP_CLOSE"},
				{eNetMessageType::ACTION, "ACTION"},
				{eNetMessageType::GAMETIME_SYNC_SERVER, "GAMETIME_SYNC_SERVER"},
				{eNetMessageType::GAMETIME_SYNC_CLIENT, "GAMETIME_SYNC_CLIENT"},
				{eNetMessageType::RANDOM_SEED, "RANDOM_SEED"},
				{eNetMessageType::FREEZE_MODES, "FREEZE_MODES"},
				{eNetMessageType::REPORT, "REPORT"},
				{eNetMessageType::GUI_SAVE_INFO, "GUI_SAVE_INFO"},
				{eNetMessageType::REQUEST_GUI_SAVE_INFO, "REQUEST_GUI_SAVE_INFO"},
				{eNetMessageType::RESYNC_MODEL, "RESYNC_MODEL"},
				{eNetMessageType::REQUEST_RESYNC_MODEL, "REQUEST_RESYNC_MODEL"},
				{eNetMessageType::MULTIPLAYER_LOBBY, "MULTIPLAYER_LOBBY"},
				{eNetMessageType::GAME_ALREADY_RUNNING, "GAME_ALREADY_RUNNING"},
				{eNetMessageType::WANT_REJOIN_GAME, "WANT_REJOIN_GAME"}};

} // namespace serialization
//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage> cNetMessage::createFromBuffer (const unsigned char* data, int length)
{
	cBinaryArchiveIn archive (data, length);

	eNetMessageType type;
	archive >> NVP (type);

	int playerNr;
	archive >> NVP (playerNr);

	std::unique_ptr<cNetMessage> message;
	switch (type)
	{
		case eNetMessageType::TCP_HELLO:
			message = std::make_unique<cNetMessageTcpHello> (archive);
			break;
		case eNetMessageType::TCP_WANT_CONNECT:
			message = std::make_unique<cNetMessageTcpWantConnect> (archive);
			break;
		case eNetMessageType::TCP_CONNECTED:
			message = std::make_unique<cNetMessageTcpConnected> (archive);
			break;
		case eNetMessageType::TCP_CONNECT_FAILED:
			message = std::make_unique<cNetMessageTcpConnectFailed> (archive);
			break;
		case eNetMessageType::ACTION:
			message = cAction::createFromBuffer (archive);
			break;
		case eNetMessageType::MULTIPLAYER_LOBBY:
			message = cMultiplayerLobbyMessage::createFromBuffer (archive);
			break;
		case eNetMessageType::GAMETIME_SYNC_SERVER:
			message = std::make_unique<cNetMessageSyncServer> (archive);
			break;
		case eNetMessageType::GAMETIME_SYNC_CLIENT:
			message = std::make_unique<cNetMessageSyncClient> (archive);
			break;
		case eNetMessageType::RANDOM_SEED:
			message = std::make_unique<cNetMessageRandomSeed> (archive);
			break;
		case eNetMessageType::FREEZE_MODES:
			message = std::make_unique<cNetMessageFreezeModes> (archive);
			break;
		case eNetMessageType::REPORT:
			message = std::make_unique<cNetMessageReport> (archive);
			break;
		case eNetMessageType::GUI_SAVE_INFO:
			message = std::make_unique<cNetMessageGUISaveInfo> (archive);
			break;
		case eNetMessageType::REQUEST_GUI_SAVE_INFO:
			message = std::make_unique<cNetMessageRequestGUISaveInfo> (archive);
			break;
		case eNetMessageType::RESYNC_MODEL:
			message = std::make_unique<cNetMessageResyncModel> (archive);
			break;
		case eNetMessageType::REQUEST_RESYNC_MODEL:
			message = std::make_unique<cNetMessageRequestResync> (archive);
			break;
		case eNetMessageType::GAME_ALREADY_RUNNING:
			message = std::make_unique<cNetMessageGameAlreadyRunning> (archive);
			break;
		case eNetMessageType::WANT_REJOIN_GAME:
			message = std::make_unique<cNetMessageWantRejoinGame> (archive);
			break;
		default:
			throw std::runtime_error ("Unknown net message type " + std::to_string (static_cast<int> (type)));
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
	cBinaryArchiveOut archive (serialMessage);
	archive << *this;

	return cNetMessage::createFromBuffer (serialMessage.data(), serialMessage.size());
}

//------------------------------------------------------------------------------
cNetMessageTcpHello::cNetMessageTcpHello() :
	packageVersion (PACKAGE_VERSION),
	packageRev (PACKAGE_REV)
{}

//------------------------------------------------------------------------------
cNetMessageTcpWantConnect::cNetMessageTcpWantConnect() :
	packageVersion (PACKAGE_VERSION),
	packageRev (PACKAGE_REV)
{}

//------------------------------------------------------------------------------
cNetMessageTcpConnected::cNetMessageTcpConnected (int playerNr) :
	playerNr (playerNr),
	packageVersion (PACKAGE_VERSION),
	packageRev (PACKAGE_REV)
{}

//------------------------------------------------------------------------------
cNetMessageResyncModel::cNetMessageResyncModel (const cModel& model)
{
	cBinaryArchiveOut archive (data);
	archive << model;
}

void cNetMessageResyncModel::apply (cModel& model) const
{
	cBinaryArchiveIn archive (data.data(), data.size());
	archive >> model;
}

//------------------------------------------------------------------------------
cNetMessageGameAlreadyRunning::cNetMessageGameAlreadyRunning (const cModel& model):
	mapFilename (model.getMap()->getFilename()),
	mapCrc (MapDownload::calculateCheckSum (mapFilename)),
	playerList (ranges::Transform (model.getPlayerList(), [] (const auto& p) { return cPlayerBasicData ({p->getName(), p->getColor()}, p->getId(), p->isDefeated); }))
{
}
