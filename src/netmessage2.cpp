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

#include "game/logic/action/action.h"

#include "main.h"
#include "netmessage2.h"
#include "utility/string/toString.h"
#include "game/data/model.h"
#include "utility/serialization/serialization.h"
#include "ui/graphical/menu/control/menuevents.h"

std::unique_ptr<cNetMessage2> cNetMessage2::createFromBuffer(const unsigned char* data, int length)
{
	cBinaryArchiveOut archive(data, length);

	eNetMessageType type;
	archive >> type;

	int playerNr;
	archive >> playerNr;

	std::unique_ptr<cNetMessage2> message;
	switch (type)
	{
	case eNetMessageType::TCP_HELLO: 
		message = std::make_unique<cNetMessageTcpHello>(archive); break;
	case eNetMessageType::TCP_WANT_CONNECT: 
		message = std::make_unique<cNetMessageTcpWantConnect>(archive); break;
	case eNetMessageType::TCP_CONNECTED: 
		message = std::make_unique<cNetMessageTcpConnected>(archive); break;
	case eNetMessageType::ACTION:
		message = cAction::createFromBuffer(archive); break;
	case eNetMessageType::GAMETIME_SYNC_SERVER:
		message = std::make_unique<cNetMessageSyncServer>(archive); break;
	case eNetMessageType::GAMETIME_SYNC_CLIENT:
		message = std::make_unique<cNetMessageSyncClient>(archive); break;
	case eNetMessageType::RANDOM_SEED:
		message = std::make_unique<cNetMessageRandomSeed>(archive); break;
	//case eNetMessageType::PLAYERSTATE:
	//	message = std::make_unique<cNe>(archive); break;
	case eNetMessageType::CHAT:
		message = std::make_unique<cNetMessageChat>(archive); break;
	case eNetMessageType::GUI_SAVE_INFO:
		message = std::make_unique<cNetMessageGUISaveInfo>(archive); break;
	case eNetMessageType::REQUEST_GUI_SAVE_INFO:
		message = std::make_unique<cNetMessageRequestGUISaveInfo>(archive); break;
	case eNetMessageType::RESYNC_MODEL:
		message = std::make_unique<cNetMessageResyncModel>(archive); break;
	case eNetMessageType::MULTIPLAYER_LOBBY:
		message = cMultiplayerLobbyMessage::createFromBuffer(archive); break;

	default:
		throw std::runtime_error("Unknown net message type " + iToStr(static_cast<int>(type)));
		break;
	}

	message->playerNr = playerNr;

	return message;
}

//------------------------------------------------------------------------------
std::unique_ptr<cNetMessage2> cNetMessage2::clone() const
{
	std::vector<unsigned char> serialMessage;
	cBinaryArchiveIn archiveIn(serialMessage);
	archiveIn << *this;

	return cNetMessage2::createFromBuffer(serialMessage.data(), serialMessage.size());
}

//------------------------------------------------------------------------------
std::string enumToString(eNetMessageType value)
{
	switch (value)
	{
	case eNetMessageType::ACTION: return "ACTION";
	case eNetMessageType::GAMETIME_SYNC_SERVER: return "GAMETIME_SYNC_SERVER";
	case eNetMessageType::GAMETIME_SYNC_CLIENT: return "GAMETIME_SYNC_CLIENT";
	case eNetMessageType::RANDOM_SEED: return "RANDOM_SEED";
	case eNetMessageType::PLAYERSTATE: return "PLAYERSTATE";
	case eNetMessageType::CHAT: return "CHAT";
	case eNetMessageType::GUI_SAVE_INFO: return "GUI_SAVE_INFO";
	case eNetMessageType::REQUEST_GUI_SAVE_INFO: return "REQUEST_GUI_SAVE_INFO";
	case eNetMessageType::RESYNC_MODEL: return "RESYNC_MODEL";
	case eNetMessageType::MULTIPLAYER_LOBBY: return "MULTIPLAYER_LOBBY";
	case eNetMessageType::TCP_HELLO: return "TCP_HELLO";
	case eNetMessageType::TCP_WANT_CONNECT: return "TCP_WANT_CONNECT";
	case eNetMessageType::TCP_CONNECTED: return "TCP_CONNECTED";
	case eNetMessageType::TCP_CONNECT_FAILED: return "TCP_CONNECT_FAILED";
	case eNetMessageType::TCP_CLOSE: return "TCP_CLOSE";
	default:
		assert(false);
		return toString(static_cast<int>(value));
	}
}

//------------------------------------------------------------------------------
cNetMessageResyncModel::cNetMessageResyncModel(const cModel& model) :
	cNetMessage2(eNetMessageType::RESYNC_MODEL)
{
	cBinaryArchiveIn archive(data);
	archive << model;
}

void cNetMessageResyncModel::apply(cModel& model) const
{
	serialization::cPointerLoader p(model);
	cBinaryArchiveOut archive(data.data(), data.size(), &p);
	archive >> model;
}
