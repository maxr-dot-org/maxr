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

#include "game/connectionmanager.h"
#include "game/data/map/map.h"
#include "game/data/player/playerbasicdata.h"
#include "game/protocol/lobbymessage.h"
#include "game/startup/lobbyclient.h"
#include "mapdownloader/mapdownload.h"
#include "settings.h"
#include "testutility.h"
#include "utility/color.h"
#include "utility/serialization/binaryarchive.h"
#include "utility/string/utf-8.h"

#include <doctest.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

namespace
{
	//--------------------------------------------------------------------------
	std::vector<unsigned char> readFile (const std::filesystem::path& path)
	{
		std::ifstream file (path, std::ios::binary);
		return {std::istreambuf_iterator<char> (file), std::istreambuf_iterator<char>()};
	}

	//--------------------------------------------------------------------------
	std::unique_ptr<cMuMsgMapDownloadData> makeMapDownloadData (const std::vector<unsigned char>& bytes)
	{
		auto message = std::make_unique<cMuMsgMapDownloadData>();
		message->data.assign (bytes.begin(), bytes.end());
		return message;
	}

	//--------------------------------------------------------------------------
	// Receives a map with this name and these bytes, as a client from the host
	bool receiveMap (const std::filesystem::path& mapFilename, const std::vector<unsigned char>& bytes)
	{
		cMapReceiver receiver (mapFilename, static_cast<int> (bytes.size()));
		receiver.receiveData (*makeMapDownloadData (bytes));
		return receiver.finished();
	}
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("IsValidMapFilename")
{
	CHECK (MapDownload::isValidMapFilename ("Delta.wrl"));
	CHECK (MapDownload::isValidMapFilename ("Three Isles.WRL"));
	CHECK (MapDownload::isValidMapFilename ("valentine's planet.wrl"));
	CHECK (MapDownload::isValidMapFilename ("peak-a-boo.wrl"));
	CHECK (MapDownload::isValidMapFilename (u8"\u041a\u0430\u0440\u0442\u0430.wrl"));
	CHECK (MapDownload::isValidMapFilename ("console.wrl"));
	CHECK (MapDownload::isValidMapFilename ("com10.wrl"));

	CHECK_FALSE (MapDownload::isValidMapFilename (""));
	CHECK_FALSE (MapDownload::isValidMapFilename ("."));
	CHECK_FALSE (MapDownload::isValidMapFilename (".."));
	CHECK_FALSE (MapDownload::isValidMapFilename (".wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("../map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("..\\map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("maps/map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("/map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("C:\\map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("C:map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("\\\\server\\share\\map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map.dll:map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map?.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map\n.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map.dll"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map.wrl.exe"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("map.wrl."));
	CHECK_FALSE (MapDownload::isValidMapFilename ("CON.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("nul.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("aux .wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("com1.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename ("LPT9.map.wrl"));
	CHECK_FALSE (MapDownload::isValidMapFilename (u8"com\u00b9.wrl"));
}

//------------------------------------------------------------------------------
TEST_CASE ("MapReceiverSavesTheMapOnlyIntoTheMapsFolder")
{
	const auto bytes = makeGroundMapFile (16);

	SUBCASE ("A map file name")
	{
		const cTemporaryFile file (".wrl", cSettings::getInstance().getUserMapsDir());

		CHECK (receiveMap (file.getPath().filename(), bytes));
		CHECK (readFile (file.getPath()) == bytes);
	}
	SUBCASE ("A path")
	{
		const cTemporaryFile file (".wrl");
		const auto relativePath = std::filesystem::relative (file.getPath(), cSettings::getInstance().getUserMapsDir()); // with ".."

		for (const auto& path : {file.getPath(), relativePath})
		{
			if (path.empty()) continue; // no relative path between two drives
			INFO (utf8::to_string (path));

			CHECK_FALSE (receiveMap (path, bytes));
			CHECK_FALSE (std::filesystem::exists (file.getPath()));
		}
	}
}

//------------------------------------------------------------------------------
TEST_CASE ("LobbyClientUsesOnlyMapsOfTheMapsFolder")
{
	// a map outside of the maps folders
	const cTemporaryFile file (".wrl");
	const auto bytes = makeGroundMapFile (16);
	file.write (bytes);

	cLobbyClient lobbyClient (std::make_shared<cConnectionManager>(), cPlayerBasicData ({"Player", cRgbColor::red()}, 0, false));

	SUBCASE ("The map selected by the host")
	{
		auto options = std::make_unique<cMuMsgOptions>();
		options->mapFilename = file.getPath();
		options->mapCrc = MapDownload::calculateCheckSum (file.getPath());
		lobbyClient.pushMessage (std::move (options));
		lobbyClient.run();

		const auto& staticMap = lobbyClient.getLobbyPreparationData().staticMap;
		CHECK_FALSE ((staticMap && staticMap->isValid()));
	}
	SUBCASE ("The map downloaded from the host")
	{
		bool downloaded = false;
		std::shared_ptr<cStaticMap> downloadedMap;
		lobbyClient.onDownloadMapFinished.connect ([&] (std::shared_ptr<cStaticMap> staticMap) {
			downloaded = true;
			downloadedMap = staticMap;
		});
		const auto otherBytes = makeGroundMapFile (32);

		lobbyClient.pushMessage (std::make_unique<cMuMsgStartMapDownload> (file.getPath(), static_cast<int> (otherBytes.size())));
		lobbyClient.pushMessage (makeMapDownloadData (otherBytes));
		lobbyClient.pushMessage (std::make_unique<cMuMsgFinishedMapDownload>());
		lobbyClient.run();

		CHECK (downloaded);
		CHECK (downloadedMap == nullptr);
		CHECK (readFile (file.getPath()) == bytes);
	}
}

//------------------------------------------------------------------------------
TEST_CASE ("StaticMapOfAReceivedModelIsLoadedOnlyFromTheMapsFolder")
{
	// a map outside of the maps folders
	const cTemporaryFile file (".wrl");
	file.write (makeGroundMapFile (16));
	cStaticMap sentMap;
	REQUIRE (sentMap.loadMap (file.getPath()));

	std::vector<unsigned char> buffer;
	cBinaryArchiveOut out (buffer);
	out << sentMap;

	cBinaryArchiveIn in (buffer.data(), buffer.size());
	cStaticMap receivedMap;
	CHECK_THROWS_AS (in >> receivedMap, std::runtime_error);
	CHECK_FALSE (receivedMap.isValid());
}
