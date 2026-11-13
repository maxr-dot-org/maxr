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

#include "game/protocol/lobbymessage.h"
#include "mapdownloader/mapdownload.h"
#include "settings.h"
#include "testutility.h"

#include <cstdint>
#include <doctest.h>
#include <filesystem>
#include <limits>
#include <memory>

//------------------------------------------------------------------------------
TEST_CASE ("IsValidMapFileSize")
{
	CHECK (MapDownload::isValidMapFileSize (1));
	CHECK (MapDownload::isValidMapFileSize (431723)); // Delta.wrl
	CHECK (MapDownload::isValidMapFileSize (64 * 1024 * 1024));

	CHECK_FALSE (MapDownload::isValidMapFileSize (-1));
	CHECK_FALSE (MapDownload::isValidMapFileSize (0));
	CHECK_FALSE (MapDownload::isValidMapFileSize (64 * 1024 * 1024 + 1));
	CHECK_FALSE (MapDownload::isValidMapFileSize (std::numeric_limits<std::int32_t>::max()));
}

//------------------------------------------------------------------------------
TEST_CASE ("MapReceiverRefusesAWrongMapSize")
{
	// where the received map is saved; removed in any case
	const cTemporaryFile file (".wrl", cSettings::getInstance().getUserMapsDir());
	const auto bytes = makeGroundMapFile (16);
	cMuMsgMapDownloadData message;
	message.data.assign (bytes.begin(), bytes.end());

	for (const int size : {-1, 0, 64 * 1024 * 1024 + 1})
	{
		CAPTURE (size);
		std::unique_ptr<cMapReceiver> receiver;
		CHECK_NOTHROW (receiver = std::make_unique<cMapReceiver> (file.getPath().filename(), size));
		if (receiver == nullptr) continue;

		CHECK_FALSE (receiver->receiveData (message));
		CHECK (receiver->getBytesReceivedPercent() == 0);
		CHECK_FALSE (receiver->finished());
		CHECK_FALSE (std::filesystem::exists (file.getPath()));
	}

	// the size of the map
	cMapReceiver receiver (file.getPath().filename(), static_cast<int> (bytes.size()));
	CHECK (receiver.receiveData (message));
	CHECK (receiver.getBytesReceivedPercent() == 100);
	CHECK (receiver.finished());
	CHECK (std::filesystem::exists (file.getPath()));
}
