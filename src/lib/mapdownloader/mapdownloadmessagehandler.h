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

#ifndef mapdownloader_mapdownloadmessagehandlerH
#define mapdownloader_mapdownloadmessagehandlerH

#include "game/data/map/map.h"
#include "game/protocol/lobbymessage.h"
#include "mapdownloader/mapdownload.h"
#include "utility/signal/signal.h"

#include <memory>

class IMapDownloadMessageHandler : public ILobbyMessageHandler
{
protected:
	bool handleMessage (const cMultiplayerLobbyMessage&) final;

protected:
	virtual void init (const cMuMsgStartMapDownload&) = 0;
	virtual void receivedData (const cMuMsgMapDownloadData&) = 0;
	virtual void cancellation (const cMuMsgCanceledMapDownload&) = 0;
	virtual void finished (const cMuMsgFinishedMapDownload&) = 0;

private:
	enum class eState
	{
		None,
		Initialized
	};
	eState state = eState::None;
};

class cMapDownloadMessageHandler final : public IMapDownloadMessageHandler
{
public:
	cSignal<void (std::size_t)> onPercentChanged;
	cSignal<void()> onCancelled;
	cSignal<void (std::shared_ptr<cStaticMap>)> onDownloaded;

private:
	void init (const cMuMsgStartMapDownload&) override;
	void receivedData (const cMuMsgMapDownloadData&) override;
	void cancellation (const cMuMsgCanceledMapDownload&) override;
	void finished (const cMuMsgFinishedMapDownload&) override;

private:
	std::unique_ptr<cMapReceiver> mapReceiver;
	std::size_t lastPercent = 0;
};

#endif
