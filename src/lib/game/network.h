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
#ifndef game_networkH
#define game_networkH

#include "game/networkaddress.h"

#include <SDL_net.h>
#include <atomic>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

//this is probably the maximum of the underlying os 'select' call
#define MAX_TCP_CONNECTIONS 64

class cConnectionManager;

//TODO: remove the need for a fixed arbitrary maximum message size.
const uint32_t PACKAGE_LENGTH = 1024 * 1024 * 10;

class cDataBuffer
{
public:
	cDataBuffer() = default;

	void reserve (uint32_t i);
	unsigned char* getWritePointer();
	uint32_t getFreeSpace() const;
	void deleteFront (uint32_t n);

	uint32_t capacity = 0;
	uint32_t length = 0;
	unsigned char* data = nullptr;
};

class cSocket
{
public:
	explicit cSocket (TCPsocket socket);

	const TCPsocket sdlSocket;
	cDataBuffer buffer;
};

//------------------------------------------------------------------------
class cNetwork
{
public:
	cNetwork (cConnectionManager&, std::recursive_mutex&);
	~cNetwork();

	int openServer (int port);
	void closeServer();
	void connectToServer (const sNetworkAddress&);

	void close (const cSocket&);
	int sendMessage (const cSocket&, unsigned int length, const unsigned char* buffer);

private:
	void handleNetworkThread();

	void pushReadyMessages (cSocket&);
	int send (const cSocket&, const unsigned char* buffer, unsigned int length);

	void cleanupClosedSockets();

private:
	std::recursive_mutex& tcpMutex;

	TCPsocket serverSocket;
	std::vector<std::unique_ptr<cSocket>> sockets;
	SDLNet_SocketSet socketSet;
	std::vector<TCPsocket> closingSockets; //list of sockets to be closed. This needs to be done inside the network thread.

	cConnectionManager& connectionManager;

	// save infos for non blocking connection attempt
	std::optional<sNetworkAddress> connectTo;

	std::atomic<bool> exit{false};
	std::thread tcpHandleThread;
};

#endif
