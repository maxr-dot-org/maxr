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

#include "mapdownload.h"
#include "netmessage.h"
#include "menus.h"
#include "menuevents.h"
#include "settings.h"
#include "defines.h"
#include "log.h"
#include "files.h"

bool MapDownload::isMapOriginal (std::string mapName)
{
	return (mapName == "Bottleneck.wrl"
			|| mapName == "Flash Point.wrl"
			|| mapName == "Freckles.wrl"
			|| mapName == "Frigia.wrl"
			|| mapName == "Great Circle.wrl"
			|| mapName == "Great divide.wrl"
			|| mapName == "Hammerhead.wrl"
			|| mapName == "High Impact.wrl"
			|| mapName == "Ice Berg.wrl"
			|| mapName == "Iron Cross.wrl"
			|| mapName == "Islandia.wrl"
			|| mapName == "Long Floes.wrl"
			|| mapName == "Long Passage.wrl"
			|| mapName == "Middle Sea.wrl"
			|| mapName == "New Luzon.wrl"
			|| mapName == "Peak-a-boo.wrl"
			|| mapName == "Sanctuary.wrl"
			|| mapName == "Sandspit.wrl"
			|| mapName == "Snowcrab.wrl"
			|| mapName == "Splatterscape.wrl"
			|| mapName == "The Cooler.wrl"
			|| mapName == "Three Rings.wrl"
			|| mapName == "Ultima Thule.wrl"
			|| mapName == "Valentine's Planet.wrl");
}


//-------------------------------------------------------------------------------
/** @return a 32 bit checksum of the given data */
//-------------------------------------------------------------------------------
/*Sint32 calcCheckSum (char* data, int dataSize)
{
	Sint32 checksum = 0;
	for (int i = 0; i < dataSize; i++)
	{
		if (checksum > 1073741824) // 2^30
		{
			checksum *= 2;
			checksum += 1;
		}
		else
			checksum *= 2;
		checksum += data[i];

// pretty checksum, but not working the same way on big and little endian machines...
//		checksum = (checksum >> 1) + ((checksum & 1) << 30); // don't get in the area 
//		checksum += data[i];
//		checksum &= 0xffffffff;       // Keep it within bounds.
	}
	return checksum;
}*/

//-------------------------------------------------------------------------------
// cMapReceiver implementation
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
cMapReceiver::cMapReceiver (std::string mapName, int mapSize)
: mapName (mapName)
, mapSize (mapSize)
, bytesReceived (0)
, readBuffer (0)
{
	if (mapSize > 0)
		readBuffer = new char [mapSize];
}

//-------------------------------------------------------------------------------
cMapReceiver::~cMapReceiver ()
{
	if (readBuffer != 0)
		delete[] readBuffer;
}

//-------------------------------------------------------------------------------
bool cMapReceiver::receiveData (cNetMessage* message, int bytesInMsg)
{
	if (readBuffer == 0 || message == 0 || bytesInMsg <= 0 || bytesReceived + bytesInMsg > mapSize)
		return false;
	
	for (int i = bytesInMsg - 1; i >= 0; i--)
		readBuffer[bytesReceived + i] = message->popChar ();
	
	bytesReceived += bytesInMsg;
	cout << "Received Data: " << bytesReceived << "/" << mapSize;
	return true;
}

//-------------------------------------------------------------------------------
bool cMapReceiver::finished ()
{
	cout << "Received Complete Map";
	if (bytesReceived != mapSize)
		return false;
	
	std::string mapsFolder = getUserMapsDir ();
	if (mapsFolder.empty ())
		mapsFolder = SettingsData.sMapsPath + PATH_DELIMITER;
	std::string filename = mapsFolder + mapName.c_str();
	std::ofstream newMapFile;
	newMapFile.open (filename.c_str(), ios::out | ios::binary);
	if (newMapFile.bad ())
		return false;
	newMapFile.write (readBuffer, mapSize);
	if (newMapFile.bad ())
		return false;
	newMapFile.close();
	if (newMapFile.bad ())
		return false;
	
	return true;
}


//-------------------------------------------------------------------------------
// cMapSender implementation
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
int mapSenderThreadFunction (void* data)
{
	cMapSender* mapSender = (cMapSender*) data;
	mapSender->run ();
	return 0;
}

//-------------------------------------------------------------------------------
cMapSender::cMapSender (int toSocket, std::string mapName)
: toSocket (toSocket)
, mapName (mapName)
, mapSize (0)
, bytesSent (0)
, sendBuffer (0)
, hostMenu (0)
, thread (0)
{
}

//-------------------------------------------------------------------------------
cMapSender::~cMapSender ()
{
	if (sendBuffer != 0)
	{
		delete[] sendBuffer;
		// the thread was not finished yet (else it would have deleted sendBuffer already)
		// send a canceled msg to the client
		cNetMessage* msg = new cNetMessage (MU_MSG_CANCELED_MAP_DOWNLOAD);
		sendMsg (msg);
		Log.write ("MapSender: Canceling an unfinsed upload thread", cLog::eLOG_TYPE_DEBUG);
	}
	if (thread != 0)
		SDL_KillThread (thread);
}

//-------------------------------------------------------------------------------
void cMapSender::runInThread (cNetworkHostMenu* hostMenu)
{
	thread = SDL_CreateThread (mapSenderThreadFunction, this); // the thread will quit, when it finished uploading the map
}

//-------------------------------------------------------------------------------
void cMapSender::run ()
{
	// read map file in memory
	string filename = SettingsData.sMapsPath + PATH_DELIMITER + mapName.c_str();
	
	ifstream file (filename.c_str(), ios::in | ios::binary | ios::ate);
	if (file.is_open ())
	{
		mapSize = (int) file.tellg();
		sendBuffer = new char [mapSize];
		file.seekg (0, ios::beg);
		file.read (sendBuffer, mapSize);
		file.close();
		Log.write (string ("MapSender: read the map \"") + filename + "\" into memory.", cLog::eLOG_TYPE_DEBUG);
	}
	else 
	{
		Log.write (string ("MapSender: could not read the map \"") + filename + "\" into memory.", cLog::eLOG_TYPE_WARNING);
		return;
	}

	cNetMessage* msg = new cNetMessage (MU_MSG_START_MAP_DOWNLOAD);
	msg->pushString (mapName.c_str());
	msg->pushInt32 (mapSize);
	sendMsg (msg);
	
	while (bytesSent < mapSize)
	{
		msg = new cNetMessage (MU_MSG_MAP_DOWNLOAD_DATA);
		int bytesToSend = mapSize - bytesSent;
		if (msg->iLength + bytesToSend + 4 > PACKAGE_LENGTH)
			bytesToSend = PACKAGE_LENGTH - msg->iLength - 4;
		for (int i = 0; i < bytesToSend; i++)
			msg->pushChar (sendBuffer[bytesSent + i]);
		bytesSent += bytesToSend;
		msg->pushInt32 (bytesToSend);
		sendMsg (msg);
		SDL_Delay (10);
	}
	
	// finished
	delete[] sendBuffer;
	sendBuffer = 0;
	msg = new cNetMessage (MU_MSG_FINISHED_MAP_DOWNLOAD);
	sendMsg (msg);
}

//-------------------------------------------------------------------------------
bool cMapSender::sendMsg (cNetMessage* msg)
{
	if (network == 0) 
		return false;
	
	msg->iPlayerNr = -1;
	network->sendTo (toSocket, msg->iLength, msg->serialize ());

	Log.write ("MapSender: <-- " + msg->getTypeAsString() + ", Hexdump: " + msg->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );
	delete msg;
	
	return true;
}

