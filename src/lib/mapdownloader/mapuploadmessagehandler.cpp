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

#include "mapuploadmessagehandler.h"

#include "game/data/map/map.h"
#include "mapdownloader/mapdownload.h"

#include <cassert>
#include <memory>

bool IMapUploadMessageHandler::handleMessage (const cMultiplayerLobbyMessage& message)
{
	if (message.playerNr == -1) { return false; } // each player should do individual request

	auto& state = states[message.playerNr];

	switch (message.getType())
	{
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_REQUEST_MAP:
		{
			if (state == eState::None)
			{
				// Can upload only one map at a time (by player)
				//cancellation (cMuMsgCanceledMapDownload{}.From (message.playerNr));
			}
			requested (static_cast<const cMuMsgRequestMap&> (message));
			state = eState::Requested;
			return true;
		}
#if 0 // Client cannot cancel currently
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD:
		{
			if (state == eState::Requested)
			{
				cancellation (static_cast<const cMuMsgCanceledMapDownload&> (message));
			}
			state = eState::None;
			return true;
		}
#endif
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
		{
			if (state == eState::Requested)
			{
				finished (static_cast<const cMuMsgFinishedMapDownload&> (message));
			}
			state = eState::None;
			return true;
		}
		default: return false;
	}
}

//------------------------------------------------------------------------------
cMapUploadMessageHandler::cMapUploadMessageHandler (std::shared_ptr<cConnectionManager> connectionManager, std::function<const cStaticMap*()> mapProvider) :
	connectionManager (std::move (connectionManager)),
	mapProvider (std::move (mapProvider))
{
	assert (this->connectionManager);
	assert (this->mapProvider);
}

//------------------------------------------------------------------------------
void cMapUploadMessageHandler::requested (const cMuMsgRequestMap& message)
{
	const auto* map = mapProvider();

	if (map == nullptr || MapDownload::isMapOriginal (map->getFilename())) return;

	auto& mapSender = mapSenders[message.playerNr];

	mapSender = std::make_unique<cMapSender> (*connectionManager, message.playerNr, map->getFilename());
	mapSender->runInThread();

	onRequested (message.playerNr);
}

//------------------------------------------------------------------------------
void cMapUploadMessageHandler::finished (const cMuMsgFinishedMapDownload& message)
{
	onFinished (message.playerNr);
}
