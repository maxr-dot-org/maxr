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

#ifndef mapdownloadH
#define mapdownloadH

#include <string>

#include <SDL.h>

class cNetMessage;
class cTCP;
struct SDL_Thread;

int mapSenderThreadFunction (void* data);

namespace MapDownload
{

/** @return is this a map that originates from the original M.A.X. ? */
bool isMapOriginal (const std::string& mapName, int32_t checksum = 0);

/** @return the path to the map (in user or factory maps directory),
 *          or empty string if not found */
std::string getExistingMapFilePath (const std::string& mapName);

/** @return a 32 bit checksum of the given map */
int32_t calculateCheckSum (const std::string& mapName);

} // namespace MapDownload

//--------------------------------------------------------------------
class cMapReceiver
{
public:
	cMapReceiver (const std::string& mapName, int mapSize);
	~cMapReceiver();

	bool receiveData (cNetMessage& message);
	bool finished();

	const std::string& getMapName() const { return mapName; }
	int getMapSize() const { return mapSize; }
	int getBytesReceived() const { return bytesReceived; }

private:
	std::string mapName;
	int mapSize;
	int bytesReceived;
	char* readBuffer;
};

//--------------------------------------------------------------------
class cMapSender
{
public:
	cMapSender (cTCP& network_, int toSocket,
				const std::string& mapName,
				const std::string& receivingPlayerName);
	~cMapSender();

	int getToSocket() const { return toSocket; }

	void runInThread();

private:
	friend int mapSenderThreadFunction (void* data);

private:
	void run();

	bool getMapFileContent();
	void sendMsg (cNetMessage& msg);

private:
	cTCP* network;
	int toSocket;
	std::string receivingPlayerName;
	std::string mapName;
	int mapSize;
	int bytesSent;
	char* sendBuffer;

	SDL_Thread* thread;
	bool canceled;
};

#endif // mapdownloadH
