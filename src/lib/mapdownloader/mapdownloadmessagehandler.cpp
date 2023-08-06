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

#include "mapdownloadmessagehandler.h"

//------------------------------------------------------------------------------
bool IMapDownloadMessageHandler::handleMessage (const cMultiplayerLobbyMessage& message)
{
	switch (message.getType())
	{
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_START_MAP_DOWNLOAD:
		{
			if (state == eState::None)
			{
				// Can download only one map at a time
				cMuMsgCanceledMapDownload cancelMessage{};
				cancelMessage.playerNr = message.playerNr;
				cancellation (cancelMessage);
			}
			init (static_cast<const cMuMsgStartMapDownload&> (message));
			state = eState::Initialized;
			return true;
		}
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_MAP_DOWNLOAD_DATA:
		{
			if (state == eState::Initialized)
			{
				receivedData (static_cast<const cMuMsgMapDownloadData&> (message));
			}
			return true;
		}
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_CANCELED_MAP_DOWNLOAD:
		{
			if (state == eState::Initialized)
			{
				cancellation (static_cast<const cMuMsgCanceledMapDownload&> (message));
			}
			state = eState::None;
			return true;
		}
		case cMultiplayerLobbyMessage::eMessageType::MU_MSG_FINISHED_MAP_DOWNLOAD:
		{
			if (state == eState::Initialized)
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
//------------------------------------------------------------------------------

void cMapDownloadMessageHandler::init (const cMuMsgStartMapDownload& message)
{
	mapReceiver = std::make_unique<cMapReceiver> (message.mapFilename, message.mapSize);
	lastPercent = 0;
	onPercentChanged (lastPercent);
}

//------------------------------------------------------------------------------
void cMapDownloadMessageHandler::receivedData (const cMuMsgMapDownloadData& message)
{
	if (mapReceiver == nullptr) return;

	mapReceiver->receiveData (message);

	const auto percent = mapReceiver->getBytesReceivedPercent();
	if (lastPercent != percent) onPercentChanged (percent);
	lastPercent = percent;
}

//------------------------------------------------------------------------------
void cMapDownloadMessageHandler::cancellation (const cMuMsgCanceledMapDownload&)
{
	if (mapReceiver == nullptr) return;
	mapReceiver = nullptr;

	onCancelled();
}

//------------------------------------------------------------------------------
void cMapDownloadMessageHandler::finished (const cMuMsgFinishedMapDownload&)
{
	if (mapReceiver == nullptr) return;

	mapReceiver->finished();

	auto staticMap = std::make_shared<cStaticMap>();

	if (!staticMap->loadMap (mapReceiver->getMapFilename())) staticMap = nullptr;

	onDownloaded (staticMap);

	mapReceiver = nullptr;
}
