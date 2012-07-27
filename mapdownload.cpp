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

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> // std::transform
#include <cctype> // std::tolower

#include "mapdownload.h"
#include "netmessage.h"
#include "menus.h"
#include "menuevents.h"
#include "events.h"
#include "settings.h"
#include "defines.h"
#include "log.h"
#include "files.h"

using namespace std;

//-------------------------------------------------------------------------------
bool MapDownload::isMapOriginal (const std::string& mapName, Sint32 checksum)
{
	std::string lowerMapName (mapName);
	std::transform (lowerMapName.begin(), lowerMapName.end(), lowerMapName.begin(), static_cast<int (*) (int) > (std::tolower));
	if (lowerMapName == "bottleneck.wrl"
		|| lowerMapName == "flash point.wrl"
		|| lowerMapName == "freckles.wrl"
		|| lowerMapName == "frigia.wrl"
		|| lowerMapName == "great circle.wrl"
		|| lowerMapName == "great divide.wrl"
		|| lowerMapName == "hammerhead.wrl"
		|| lowerMapName == "high impact.wrl"
		|| lowerMapName == "ice berg.wrl"
		|| lowerMapName == "iron cross.wrl"
		|| lowerMapName == "islandia.wrl"
		|| lowerMapName == "long floes.wrl"
		|| lowerMapName == "long passage.wrl"
		|| lowerMapName == "middle sea.wrl"
		|| lowerMapName == "new luzon.wrl"
		|| lowerMapName == "peak-a-boo.wrl"
		|| lowerMapName == "sanctuary.wrl"
		|| lowerMapName == "sandspit.wrl"
		|| lowerMapName == "snowcrab.wrl"
		|| lowerMapName == "splatterscape.wrl"
		|| lowerMapName == "the cooler.wrl"
		|| lowerMapName == "three rings.wrl"
		|| lowerMapName == "ultima thule.wrl"
		|| lowerMapName == "valentine's planet.wrl")
	{
		return true;
	}
	if (checksum == 0)
		checksum = calculateCheckSum (lowerMapName);
	if (checksum == 344087468
		|| checksum == 1702427970
		|| checksum == 1401869069
		|| checksum == 1612651246
		|| checksum == 1041139234
		|| checksum == 117739146
		|| checksum == 1969035068
		|| checksum == 268073155
		|| checksum == 1382754034
		|| checksum == 1704409466
		|| checksum == 1893077128
		|| checksum == 289119678
		|| checksum == 231873358
		|| checksum == 959897984
		|| checksum == 1422663356
		|| checksum == 2072925938
		|| checksum == 1286420600
		|| checksum == 2040193020
		|| checksum == 10554807
		|| checksum == 486474018
		|| checksum == 451439582
		|| checksum == 1682525072
		|| checksum == 1397392934
		|| checksum == 280492815)
	{
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------
Sint32 MapDownload::calculateCheckSum (const std::string& mapName)
{
	Sint32 result = 0;
	string filename = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + mapName;
	ifstream* file = new ifstream (filename.c_str(), ios::in | ios::binary | ios::ate);
	if (!file->is_open() && !getUserMapsDir().empty())
	{
		// try to open the map from the user's maps dir
		filename = getUserMapsDir() + mapName.c_str();
		delete file;
		file = new ifstream (filename.c_str(), ios::in | ios::binary | ios::ate);
	}
	if (file->is_open())
	{
		int mapSize = (int) file->tellg();
		char* data = new char [mapSize];
		file->seekg (0, ios::beg);

		file->read (data, 9);  // read only header
		int width = data[5] + data[6] * 256;
		int height = data[7] + data[8] * 256;
		int relevantMapDataSize = width * height * 3; // the information after this is only for graphic stuff and not necessary for comparing two maps

		if (relevantMapDataSize + 9 <= mapSize)
		{
			file->read (data + 9, relevantMapDataSize);
			if (!file->bad() && !file->eof())
				result = calcCheckSum (data, relevantMapDataSize + 9);
		}
		file->close();
		delete[] data;
	}
	delete file;
	return result;
}

//-------------------------------------------------------------------------------
// cMapReceiver implementation
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
cMapReceiver::cMapReceiver (const std::string& mapName, int mapSize)
	: mapName (mapName)
	, mapSize (mapSize)
	, bytesReceived (0)
	, readBuffer (0)
{
	if (mapSize > 0)
		readBuffer = new char [mapSize];
}

//-------------------------------------------------------------------------------
cMapReceiver::~cMapReceiver()
{
	delete[] readBuffer;
}

//-------------------------------------------------------------------------------
bool cMapReceiver::receiveData (cNetMessage* message, int bytesInMsg)
{
	if (readBuffer == 0 || message == 0 || bytesInMsg <= 0 || bytesReceived + bytesInMsg > mapSize)
		return false;

	for (int i = bytesInMsg - 1; i >= 0; i--)
		readBuffer[bytesReceived + i] = message->popChar();

	bytesReceived += bytesInMsg;
	std::ostringstream os;
	os << "MapReceiver: Received Data for map " << mapName << ": " << bytesReceived << "/" << mapSize;
	Log.write (os.str(), cLog::eLOG_TYPE_DEBUG);
	return true;
}

//-------------------------------------------------------------------------------
bool cMapReceiver::finished()
{
	Log.write ("MapReceiver: Received complete map", cLog::eLOG_TYPE_DEBUG);
	if (bytesReceived != mapSize)
		return false;

	std::string mapsFolder = getUserMapsDir();
	if (mapsFolder.empty())
		mapsFolder = cSettings::getInstance().getMapsPath() + PATH_DELIMITER;
	std::string filename = mapsFolder + mapName;
	std::ofstream newMapFile;
	newMapFile.open (filename.c_str(), ios::out | ios::binary);
	if (newMapFile.bad())
		return false;
	newMapFile.write (readBuffer, mapSize);
	if (newMapFile.bad())
		return false;
	newMapFile.close();
	if (newMapFile.bad())
		return false;

	return true;
}


//-------------------------------------------------------------------------------
// cMapSender implementation
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
int mapSenderThreadFunction (void* data)
{
	cMapSender* mapSender = reinterpret_cast<cMapSender*> (data);
	mapSender->run();
	return 0;
}

//-------------------------------------------------------------------------------
cMapSender::cMapSender (int toSocket, const std::string& mapName, const std::string& receivingPlayerName)
	: toSocket (toSocket)
	, receivingPlayerName (receivingPlayerName)
	, mapName (mapName)
	, mapSize (0)
	, bytesSent (0)
	, sendBuffer (0)
	, hostMenu (0)
	, thread (0)
	, canceled (false)
{
}

//-------------------------------------------------------------------------------
cMapSender::~cMapSender()
{
	if (thread != 0)
	{
		canceled = true;
		SDL_WaitThread (thread, NULL);
		thread = 0;
	}
	if (sendBuffer != 0)
	{
		delete[] sendBuffer;
		// the thread was not finished yet (else it would have deleted sendBuffer already)
		// send a canceled msg to the client
		cNetMessage* msg = new cNetMessage (MU_MSG_CANCELED_MAP_DOWNLOAD);
		sendMsg (msg);
		Log.write ("MapSender: Canceling an unfinished upload thread", cLog::eLOG_TYPE_DEBUG);
	}
}

//-------------------------------------------------------------------------------
void cMapSender::runInThread (cNetworkHostMenu* hostMenu)
{
	thread = SDL_CreateThread (mapSenderThreadFunction, this);  // the thread will quit, when it finished uploading the map
}

//-------------------------------------------------------------------------------
void cMapSender::run()
{
	if (canceled) return;

	// read map file in memory
	string filename = cSettings::getInstance().getMapsPath() + PATH_DELIMITER + mapName.c_str();
	ifstream* file = new ifstream (filename.c_str(), ios::in | ios::binary | ios::ate);
	if (!file->is_open() && !getUserMapsDir().empty())
	{
		// try to open the map from the user's maps dir
		filename = getUserMapsDir() + mapName.c_str();
		delete file;
		file = new ifstream (filename.c_str(), ios::in | ios::binary | ios::ate);
	}
	if (file->is_open())
	{
		mapSize = (int) file->tellg();
		sendBuffer = new char [mapSize];
		file->seekg (0, ios::beg);
		file->read (sendBuffer, mapSize);
		file->close();
		delete file;
		Log.write (string ("MapSender: read the map \"") + filename + "\" into memory.", cLog::eLOG_TYPE_DEBUG);
	}
	else
	{
		Log.write (string ("MapSender: could not read the map \"") + filename + "\" into memory.", cLog::eLOG_TYPE_WARNING);
		delete file;
		return;
	}

	if (canceled) return;

	cNetMessage* msg = new cNetMessage (MU_MSG_START_MAP_DOWNLOAD);
	msg->pushString (mapName);
	msg->pushInt32 (mapSize);
	sendMsg (msg);

	int msgCount = 0;
	while (bytesSent < mapSize)
	{
		if (canceled) return;

		msg = new cNetMessage (MU_MSG_MAP_DOWNLOAD_DATA);
		int bytesToSend = mapSize - bytesSent;
		if (msg->iLength + bytesToSend + 4 > PACKAGE_LENGTH)
			bytesToSend = PACKAGE_LENGTH - msg->iLength - 4;
		for (int i = 0; i < bytesToSend; i++)
			msg->pushChar (sendBuffer[bytesSent + i]);
		bytesSent += bytesToSend;
		msg->pushInt32 (bytesToSend);
		sendMsg (msg);

		msgCount++;
		if (msgCount % 10 == 0)
			SDL_Delay (100);
	}

	// finished
	delete[] sendBuffer;
	sendBuffer = 0;
	msg = new cNetMessage (MU_MSG_FINISHED_MAP_DOWNLOAD);
	msg->pushString (receivingPlayerName);
	sendMsg (msg);
	// Push message also to client, that belongs to the host, to give feedback about the finished upload state.
	// The EventHandler mechanism is used, because this code runs in another thread than the code, that must display the msg.
	if (EventHandler)
	{
		cNetMessage* message = new cNetMessage (MU_MSG_FINISHED_MAP_DOWNLOAD);
		message->pushString (receivingPlayerName);
		EventHandler->pushEvent (message);
	}
}

//-------------------------------------------------------------------------------
bool cMapSender::sendMsg (cNetMessage* msg)
{
	if (network == 0)
		return false;

	msg->iPlayerNr = -1;
	network->sendTo (toSocket, msg->iLength, msg->serialize());

	Log.write ("MapSender: <-- " + msg->getTypeAsString() + ", Hexdump: " + msg->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	delete msg;

	return true;
}
