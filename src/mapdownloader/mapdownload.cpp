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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

#include "defines.h"
#include "utility/crc.h"
#include "utility/files.h"
#include "utility/language.h"
#include "utility/log.h"
#include "settings.h"
#include "utility/string/tolower.h"
#include "protocol/lobbymessage.h"
#include "utility/string/toString.h"
#include "connectionmanager.h"
#include "protocol/netmessage.h"

using namespace std;

//------------------------------------------------------------------------------
bool MapDownload::isMapOriginal (const std::string& mapName, int32_t checksum)
{
	std::string lowerMapName (to_lower_copy (mapName));

	const struct
	{
		const char* filename;
		int32_t checksum;
	} maps[] =
	{
		{ "bottleneck.wrl"        , 344087468},
		{ "flash point.wrl"       , 1702427970},
		{ "freckles.wrl"          , 1401869069},
		{ "frigia.wrl"            , 1612651246},
		{ "great circle.wrl"      , 1041139234},
		{ "great divide.wrl"      , 117739146},
		{ "hammerhead.wrl"        , 1969035068},
		{ "high impact.wrl"       , 268073155},
		{ "ice berg.wrl"          , 1382754034},
		{ "iron cross.wrl"        , 1704409466},
		{ "islandia.wrl"          , 1893077128},
		{ "long floes.wrl"        , 289119678},
		{ "long passage.wrl"      , 231873358},
		{ "middle sea.wrl"        , 959897984},
		{ "new luzon.wrl"         , 1422663356},
		{ "peak-a-boo.wrl"        , 2072925938},
		{ "sanctuary.wrl"         , 1286420600},
		{ "sandspit.wrl"          , 2040193020},
		{ "snowcrab.wrl"          , 10554807},
		{ "splatterscape.wrl"     , 486474018},
		{ "the cooler.wrl"        , 451439582},
		{ "three rings.wrl"       , 1682525072},
		{ "ultima thule.wrl"      , 1397392934},
		{ "valentine's planet.wrl", 280492815}
	};

	for (int i = 0; i != sizeof (maps) / sizeof (*maps); ++i)
	{
		if (lowerMapName.compare (maps[i].filename) == 0)
		{
			return true;
		}
	}
	if (checksum == 0)
		checksum = calculateCheckSum (lowerMapName);
	for (int i = 0; i != sizeof (maps) / sizeof (*maps); ++i)
	{
		if (maps[i].checksum == checksum)
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
std::string MapDownload::getExistingMapFilePath (const std::string& mapName)
{
	string filenameFactory = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + mapName;
	if (FileExists (filenameFactory.c_str()))
		return filenameFactory;
	if (!getUserMapsDir().empty())
	{
		string filenameUser = getUserMapsDir() + mapName;
		if (FileExists (filenameUser.c_str()))
			return filenameUser;
	}
	return "";
}

//------------------------------------------------------------------------------
uint32_t MapDownload::calculateCheckSum (const std::string& mapName)
{
	uint32_t result = 0;
	string filename = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + mapName;
	ifstream file (filename.c_str(), ios::in | ios::binary | ios::ate);
	if (!file.is_open() && !getUserMapsDir().empty())
	{
		// try to open the map from the user's maps dir
		filename = getUserMapsDir() + mapName.c_str();
		file.open (filename.c_str(), ios::in | ios::binary | ios::ate);
	}
	if (file.is_open())
	{
		const int mapSize = (int) file.tellg();
		std::vector<char> data (mapSize);
		file.seekg (0, ios::beg);

		file.read (data.data(), 9);  // read only header
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
cMapReceiver::cMapReceiver (const std::string& mapName, int mapSize) :
	mapName (mapName),
	bytesReceived (0),
	readBuffer (mapSize)
{
}

//------------------------------------------------------------------------------
bool cMapReceiver::receiveData (cMuMsgMapDownloadData& message)
{
	const int bytesInMsg = message.data.size();
	if (bytesInMsg <= 0 || bytesReceived + bytesInMsg > readBuffer.size())
		return false;

	for (int i = 0; i < bytesInMsg; i++)
		readBuffer[bytesReceived + i] = message.data[i];

	bytesReceived += bytesInMsg;
	std::ostringstream os;
	os << "MapReceiver: Received Data for map " << mapName << lngPack.i18n ("Text~Punctuation~Colon")
	   << bytesReceived << "/" << readBuffer.size();
	Log.write (os.str(), cLog::eLOG_TYPE_DEBUG);
	return true;
}

//------------------------------------------------------------------------------
bool cMapReceiver::finished()
{
	Log.write ("MapReceiver: Received complete map", cLog::eLOG_TYPE_DEBUG);

	if (bytesReceived != readBuffer.size())
		return false;
	std::string mapsFolder = getUserMapsDir();
	if (mapsFolder.empty())
		mapsFolder = cSettings::getInstance().getMapsPath() + PATH_DELIMITER;
	const std::string filename = mapsFolder + mapName;
	std::ofstream newMapFile (filename.c_str(), ios::out | ios::binary);
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
int mapSenderThreadFunction (void* data)
{
	cMapSender* mapSender = reinterpret_cast<cMapSender*> (data);
	mapSender->run();
	return 0;
}

//------------------------------------------------------------------------------
cMapSender::cMapSender(cConnectionManager& connectionManager, int toPlayerNr,
						const std::string& mapName,
						const std::string& receivingPlayerName) :
	connectionManager (connectionManager),
	toPlayerNr (toPlayerNr),
	receivingPlayerName (receivingPlayerName),
	mapName (mapName),
	bytesSent (0),
	sendBuffer(),
	thread (nullptr),
	canceled (false)
{
}

//------------------------------------------------------------------------------
cMapSender::~cMapSender()
{
	if (thread != nullptr)
	{
		canceled = true;
		SDL_WaitThread (thread, nullptr);
		thread = nullptr;
	}
	if (!sendBuffer.empty())
	{
		// the thread was not finished yet
		// (else it would have deleted sendBuffer already)
		// send a canceled msg to the client
		Log.write ("MapSender: Canceling an unfinished upload thread", cLog::eLOG_TYPE_DEBUG);
		sendMsg (cMuMsgCanceledMapDownload());
	}
}

//------------------------------------------------------------------------------
void cMapSender::runInThread()
{
	// the thread will quit, when it finished uploading the map
	thread = SDL_CreateThread (mapSenderThreadFunction, "mapSender", this);
}

//------------------------------------------------------------------------------
bool cMapSender::getMapFileContent()
{
	// read map file in memory
	string filename = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + mapName.c_str();
	ifstream file (filename.c_str(), ios::in | ios::binary | ios::ate);
	if (!file.is_open() && !getUserMapsDir().empty())
	{
		// try to open the map from the user's maps dir
		filename = getUserMapsDir() + mapName.c_str();
		file.open (filename.c_str(), ios::in | ios::binary | ios::ate);
	}
	if (!file.is_open())
	{
		Log.write (string ("MapSender: could not read the map \"") + filename + "\" into memory.", cLog::eLOG_TYPE_WARNING);
		return false;
	}
	const std::size_t mapSize = file.tellg();
	sendBuffer.resize (mapSize);
	file.seekg (0, ios::beg);
	file.read (sendBuffer.data(), mapSize);
	file.close();
	Log.write (string ("MapSender: read the map \"") + filename + "\" into memory.", cLog::eLOG_TYPE_DEBUG);
	return true;
}

//------------------------------------------------------------------------------
void cMapSender::run()
{
	if (canceled) return;
	getMapFileContent();
	if (canceled) return;

	{
		sendMsg(cMuMsgStartMapDownload(mapName, sendBuffer.size()));
	}
	int msgCount = 0;
	const int MAX_MESSAGE_SIZE = 10 * 1024;

	while (bytesSent < sendBuffer.size())
	{
		if (canceled) return;

		cMuMsgMapDownloadData msg;
		int bytesToSend = sendBuffer.size() - bytesSent;
		if (bytesToSend  > MAX_MESSAGE_SIZE)
			bytesToSend = MAX_MESSAGE_SIZE;
		for (int i = 0; i < bytesToSend; i++)
			msg.data.push_back(sendBuffer[bytesSent + i]);
		bytesSent += bytesToSend;
		sendMsg (msg);

		msgCount++;
		if (msgCount % 10 == 0)
			SDL_Delay (100);
	}

	// finished
	sendBuffer.clear();

	sendMsg (cMuMsgFinishedMapDownload(receivingPlayerName));

	// Push message also to client, that belongs to the host,
	// to give feedback about the finished upload state.
	// The EventHandler mechanism is used,
	// because this code runs in another thread than the code,
	// that must display the msg.

	// FIXME: may use a signal here? (and protect it by mutexes?!)

	//if (eventHandling)
	//{
	//	cNetMessage* message = new cNetMessage (MU_MSG_FINISHED_MAP_DOWNLOAD);
	//	message->pushString (receivingPlayerName);
	//	eventHandling->pushEvent (message);
	//}
}

//------------------------------------------------------------------------------
void cMapSender::sendMsg (cNetMessage2& msg)
{
	msg.playerNr = -1;

	cTextArchiveIn archive;
	archive << msg;
	Log.write ("MapSender: --> " + archive.data() + " to " + toString(toPlayerNr), cLog::eLOG_TYPE_NET_DEBUG);

	connectionManager.sendToPlayer(msg, toPlayerNr);
}

//------------------------------------------------------------------------------
void cMapSender::sendMsg (cNetMessage2&& msg)
{
	sendMsg(static_cast<cNetMessage2&>(msg));
}
