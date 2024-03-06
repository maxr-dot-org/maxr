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

/* Author: Paul Grathwohl */

#include "mapdownload.h"

#include "game/connectionmanager.h"
#include "game/protocol/lobbymessage.h"
#include "game/protocol/netmessage.h"
#include "settings.h"
#include "utility/crc.h"
#include "utility/log.h"
#include "utility/narrow_cast.h"
#include "utility/string/tolower.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------------
bool MapDownload::isMapOriginal (const std::filesystem::path& mapFilename, int32_t checksum)
{
	std::string lowerMapName (to_lower_copy (mapFilename.string()));

	const struct
	{
		const char* filename = nullptr;
		int32_t checksum = 0;
	} maps[] =
		{
			{"bottleneck.wrl", 344087468},
			{"flash point.wrl", 1702427970},
			{"freckles.wrl", 1401869069},
			{"frigia.wrl", 1612651246},
			{"great circle.wrl", 1041139234},
			{"great divide.wrl", 117739146},
			{"hammerhead.wrl", 1969035068},
			{"high impact.wrl", 268073155},
			{"ice berg.wrl", 1382754034},
			{"iron cross.wrl", 1704409466},
			{"islandia.wrl", 1893077128},
			{"long floes.wrl", 289119678},
			{"long passage.wrl", 231873358},
			{"middle sea.wrl", 959897984},
			{"new luzon.wrl", 1422663356},
			{"peak-a-boo.wrl", 2072925938},
			{"sanctuary.wrl", 1286420600},
			{"sandspit.wrl", 2040193020},
			{"snowcrab.wrl", 10554807},
			{"splatterscape.wrl", 486474018},
			{"the cooler.wrl", 451439582},
			{"three rings.wrl", 1682525072},
			{"ultima thule.wrl", 1397392934},
			{"valentine's planet.wrl", 280492815}};

	if (ranges::any_of (maps, [&] (const auto& map) { return lowerMapName == map.filename; }))
	{
		return true;
	}
	if (checksum == 0)
		checksum = calculateCheckSum (lowerMapName);
	if (ranges::any_of (maps, [&] (const auto& map) { return map.checksum == checksum; }))
	{
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
std::filesystem::path MapDownload::getExistingMapFilePath (const std::filesystem::path& mapFilename)
{
	auto filenameFactory = cSettings::getInstance().getMapsPath() / mapFilename;
	if (std::filesystem::exists (filenameFactory))
		return filenameFactory;
	if (!cSettings::getInstance().getUserMapsDir().empty())
	{
		auto filenameUser = cSettings::getInstance().getUserMapsDir() / mapFilename;
		if (std::filesystem::exists (filenameUser))
			return filenameUser;
	}
	return "";
}

//------------------------------------------------------------------------------
uint32_t MapDownload::calculateCheckSum (const std::filesystem::path& mapFilename)
{
	uint32_t result = 0;
	auto filename = cSettings::getInstance().getMapsPath() / mapFilename;
	std::ifstream file (filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open() && !cSettings::getInstance().getUserMapsDir().empty())
	{
		// try to open the map from the user's maps dir
		filename = cSettings::getInstance().getUserMapsDir() / mapFilename;
		file.open (filename, std::ios::in | std::ios::binary | std::ios::ate);
	}
	if (file.is_open())
	{
		const int mapSize = (int) file.tellg();
		std::vector<char> data (mapSize);
		file.seekg (0, std::ios::beg);

		file.read (data.data(), 9); // read only header
		const int width = data[5] + data[6] * 256;
		const int height = data[7] + data[8] * 256;
		// the information after this is only for graphic stuff
		// and not necessary for comparing two maps
		const int relevantMapDataSize = width * height * 3;

		if (relevantMapDataSize + 9 <= mapSize)
		{
			file.read (data.data() + 9, relevantMapDataSize);
			if (!file.bad() && !file.eof())
				result = calcCheckSum (data.data(), relevantMapDataSize + 9, 0);
		}
	}
	return result;
}

//------------------------------------------------------------------------------
// cMapReceiver implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cMapReceiver::cMapReceiver (const std::filesystem::path& mapFilename, int mapSize) :
	mapFilename (mapFilename),
	readBuffer (mapSize)
{
}

//------------------------------------------------------------------------------
bool cMapReceiver::receiveData (const cMuMsgMapDownloadData& message)
{
	const int bytesInMsg = message.data.size();
	if (bytesInMsg <= 0 || bytesReceived + bytesInMsg > readBuffer.size())
		return false;

	for (int i = 0; i < bytesInMsg; i++)
		readBuffer[bytesReceived + i] = message.data[i];

	bytesReceived += bytesInMsg;
	std::ostringstream os;
	os << "MapReceiver: Received Data for map " << mapFilename << ": "
	   << bytesReceived << "/" << readBuffer.size();
	Log.debug (os.str());
	return true;
}

//------------------------------------------------------------------------------
bool cMapReceiver::finished()
{
	Log.debug ("MapReceiver: Received complete map");

	if (bytesReceived != readBuffer.size())
		return false;
	std::filesystem::path mapsFolder = cSettings::getInstance().getUserMapsDir();
	if (mapsFolder.empty())
		mapsFolder = cSettings::getInstance().getMapsPath();
	const auto filename = mapsFolder / mapFilename;
	std::ofstream newMapFile (filename, std::ios::out | std::ios::binary);
	if (newMapFile.bad())
		return false;
	newMapFile.write (readBuffer.data(), readBuffer.size());
	if (newMapFile.bad())
		return false;
	newMapFile.close();
	if (newMapFile.bad())
		return false;

	return true;
}

//------------------------------------------------------------------------------
// cMapSender implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cMapSender::cMapSender (cConnectionManager& connectionManager, int toPlayerNr, const std::filesystem::path& mapFilename) :
	connectionManager (connectionManager),
	toPlayerNr (toPlayerNr),
	mapFilename (mapFilename)
{
}

//------------------------------------------------------------------------------
cMapSender::~cMapSender()
{
	if (thread.joinable())
	{
		canceled = true;
		thread.join();
	}
	if (!sendBuffer.empty())
	{
		// the thread was not finished yet
		// (else it would have deleted sendBuffer already)
		// send a canceled msg to the client
		Log.debug ("MapSender: Canceling an unfinished upload thread");
		sendMsg (cMuMsgCanceledMapDownload());
	}
}

//------------------------------------------------------------------------------
void cMapSender::runInThread()
{
	// the thread will quit, when it finished uploading the map
	thread = std::thread ([this]() {
		try
		{
			run();
		}
		catch (const std::exception& ex)
		{
			Log.error (std::string ("Exception: ") + ex.what());
		}
	});
}

//------------------------------------------------------------------------------
bool cMapSender::getMapFileContent()
{
	// read map file in memory
	auto filename = cSettings::getInstance().getMapsPath() / mapFilename;
	std::ifstream file (filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open() && !cSettings::getInstance().getUserMapsDir().empty())
	{
		// try to open the map from the user's maps dir
		filename = cSettings::getInstance().getUserMapsDir() / mapFilename;
		file.open (filename, std::ios::in | std::ios::binary | std::ios::ate);
	}
	if (!file.is_open())
	{
		Log.warn (std::string ("MapSender: could not read the map \"") + filename.u8string() + "\" into memory.");
		return false;
	}
	const std::size_t mapSize = narrow_cast<std::size_t> (file.tellg());
	sendBuffer.resize (mapSize);
	file.seekg (0, std::ios::beg);
	file.read (sendBuffer.data(), mapSize);
	file.close();
	Log.debug (std::string ("MapSender: read the map \"") + filename.u8string() + "\" into memory.");
	return true;
}

//------------------------------------------------------------------------------
void cMapSender::run()
{
	if (canceled) return;
	getMapFileContent();
	if (canceled) return;

	{
		sendMsg (cMuMsgStartMapDownload (mapFilename, sendBuffer.size()));
	}
	int msgCount = 0;
	const std::size_t MAX_MESSAGE_SIZE = 10 * 1024;

	while (bytesSent < sendBuffer.size())
	{
		if (canceled) return;

		cMuMsgMapDownloadData msg;
		int bytesToSend = std::min (sendBuffer.size() - bytesSent, MAX_MESSAGE_SIZE);
		for (int i = 0; i < bytesToSend; i++)
			msg.data.push_back (sendBuffer[bytesSent + i]);
		bytesSent += bytesToSend;
		sendMsg (msg);

		msgCount++;
		if (msgCount % 10 == 0)
			SDL_Delay (100);
	}

	// finished
	sendBuffer.clear();

	sendMsg (cMuMsgFinishedMapDownload());

	connectionManager.sendToServer (cMuMsgFinishedMapDownload().From (toPlayerNr));
}

//------------------------------------------------------------------------------
void cMapSender::sendMsg (cNetMessage& message)
{
	message.playerNr = -1;

	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("MapSender: --> " + json.dump (-1) + " to " + std::to_string (toPlayerNr));

	connectionManager.sendToPlayer (message, toPlayerNr);
}

//------------------------------------------------------------------------------
void cMapSender::sendMsg (cNetMessage&& msg)
{
	sendMsg (static_cast<cNetMessage&> (msg));
}
