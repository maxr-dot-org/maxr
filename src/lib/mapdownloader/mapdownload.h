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

#ifndef mapdownloader_mapdownloadH
#define mapdownloader_mapdownloadH

#include <atomic>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

class cConnectionManager;
class cMuMsgMapDownloadData;
class cNetMessage;

namespace MapDownload
{

	/** @return is this a map that originates from the original M.A.X. ? */
	bool isMapOriginal (const std::filesystem::path& mapFilename, int32_t checksum = 0);

	/** @return the path to the map (in user or factory maps directory),
 *          or empty string if not found */
	std::filesystem::path getExistingMapFilePath (const std::filesystem::path& mapFilename);

	/** @return a 32 bit checksum of the given map */
	uint32_t calculateCheckSum (const std::filesystem::path& mapFilename);

} // namespace MapDownload

//--------------------------------------------------------------------
class cMapReceiver
{
public:
	cMapReceiver (const std::filesystem::path& mapFilename, int mapSize);

	bool receiveData (const cMuMsgMapDownloadData&);
	bool finished();

	const std::filesystem::path& getMapFilename() const { return mapFilename; }
	std::size_t getBytesReceivedPercent() const { return (100 * bytesReceived) / readBuffer.size(); }

private:
	std::filesystem::path mapFilename;
	std::size_t bytesReceived = 0;
	std::vector<char> readBuffer;
};

//--------------------------------------------------------------------
class cMapSender
{
public:
	cMapSender (cConnectionManager&, int toPlayerNr, const std::filesystem::path& mapFilename);
	~cMapSender();

	int getToPlayerNr() const { return toPlayerNr; }

	void runInThread();

private:
	void run();

	bool getMapFileContent();
	void sendMsg (cNetMessage&);
	void sendMsg (cNetMessage&&);

private:
	cConnectionManager& connectionManager;
	int toPlayerNr;
	std::filesystem::path mapFilename;
	std::size_t bytesSent = 0;
	std::vector<char> sendBuffer;

	std::thread thread;
	std::atomic<bool> canceled{false};
};

#endif
