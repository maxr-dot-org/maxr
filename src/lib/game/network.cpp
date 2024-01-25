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

#include "network.h"

#include "game/connectionmanager.h"
#include "game/protocol/netmessage.h"
#include "utility/listhelpers.h"
#include "utility/log.h"

namespace
{
	constexpr auto START_WORD = 0x4D415852;
	constexpr auto HEADER_LENGTH = 8;
}

//------------------------------------------------------------------------
// cSocket implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------------
cSocket::cSocket (TCPsocket socket) :
	sdlSocket (socket)
{}

//------------------------------------------------------------------------
// cDataBuffer implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------------
void cDataBuffer::reserve (std::uint32_t i)
{
	if (getFreeSpace() < i && length < UINT32_MAX - i)
	{
		capacity = length + i;
		data = (unsigned char*) realloc (data, capacity);
	}
}

//------------------------------------------------------------------------------
unsigned char* cDataBuffer::getWritePointer()
{
	return data + length;
}

//------------------------------------------------------------------------------
uint32_t cDataBuffer::getFreeSpace() const
{
	return capacity - length;
}

//------------------------------------------------------------------------------
void cDataBuffer::deleteFront (uint32_t n)
{
	memmove (data, data + n, length - n);
	length -= n;
}

//------------------------------------------------------------------------
// cNetwork implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------------
cNetwork::cNetwork (cConnectionManager& connectionManager, std::recursive_mutex& mutex) :
	tcpMutex (mutex),
	socketSet (SDLNet_AllocSocketSet (MAX_TCP_CONNECTIONS)),
	connectionManager (connectionManager),
	tcpHandleThread ([this]() {
		try
		{
			handleNetworkThread();
		}
		catch (const std::exception& ex)
		{
			Log.error (std::string ("Exception: ") + ex.what());
		}
	})
{
}

//------------------------------------------------------------------------------
cNetwork::~cNetwork()
{
	exit = true;
	tcpHandleThread.join();
	SDLNet_FreeSocketSet (socketSet);
	if (serverSocket)
	{
		SDLNet_TCP_Close (serverSocket);
	}
	cleanupClosedSockets();
	for (auto& socket : sockets)
	{
		SDLNet_TCP_Close (socket->sdlSocket);
	}
}

//------------------------------------------------------------------------------
int cNetwork::openServer (int port)
{
	std::unique_lock<std::recursive_mutex> tl (tcpMutex);

	NetLog.debug ("Network: Open server on port: " + std::to_string (port));

	IPaddress ipaddr;
	if (SDLNet_ResolveHost (&ipaddr, nullptr, port) == -1)
	{
		return -1;
	}

	TCPsocket socket = SDLNet_TCP_Open (&ipaddr);
	if (socket == nullptr)
	{
		return -1;
	}

	serverSocket = socket;
	SDLNet_TCP_AddSocket (socketSet, serverSocket);

	return 0;
}

//------------------------------------------------------------------------------
void cNetwork::closeServer()
{
	std::unique_lock<std::recursive_mutex> tl (tcpMutex);

	if (serverSocket == nullptr) return;

	closingSockets.push_back (serverSocket);
	serverSocket = nullptr;
}

//------------------------------------------------------------------------------
void cNetwork::connectToServer (const sNetworkAddress& address)
{
	std::unique_lock<std::recursive_mutex> tl (tcpMutex);

	if (connectTo)
	{
		NetLog.error ("Network: Can only handle one connection attempt at once");
		connectionManager.connectionResult (nullptr);
		return;
	}
	connectTo = address;
}

//------------------------------------------------------------------------------
void cNetwork::close (const cSocket& socket)
{
	std::unique_lock<std::recursive_mutex> tl (tcpMutex);

	if (ranges::none_of (sockets, ByGetTo (&socket)))
	{
		NetLog.error ("Network: Unable to close socket. Invalid socket");
		return;
	}
	connectionManager.connectionClosed (socket);

	// sdl socket will be cleaned up later by the networkthread. This cannot be done
	// immediately, because the network thread may be still using the socket in SDLNet_CheckSockets
	closingSockets.push_back (socket.sdlSocket);
	EraseIf (sockets, ByGetTo (&socket));
}

//------------------------------------------------------------------------------
int cNetwork::sendMessage (const cSocket& socket, unsigned int length, const unsigned char* buffer)
{
	std::unique_lock<std::recursive_mutex> tl (tcpMutex);

	if (ranges::none_of (sockets, ByGetTo (&socket)))
	{
		NetLog.error ("Network: Unable to send message. Invalid socket");
		return -1;
	}

	// send message header
	unsigned char header[HEADER_LENGTH];
	reinterpret_cast<int32_t*> (header)[0] = SDL_SwapLE32 (START_WORD);
	reinterpret_cast<int32_t*> (header)[1] = SDL_SwapLE32 (length);

	if (send (socket, header, HEADER_LENGTH) == -1) return -1;

	// send message data
	return send (socket, buffer, length);
}

//------------------------------------------------------------------------------
int cNetwork::send (const cSocket& socket, const unsigned char* buffer, unsigned int length)
{
	const unsigned int bytesSent = SDLNet_TCP_Send (socket.sdlSocket, buffer, length);

	// delete socket when sending fails
	if (bytesSent != length)
	{
		// delete socket when sending fails
		NetLog.warn ("Network: Error while sending message. Closing socket...");
		close (socket);
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------
void cNetwork::handleNetworkThread()
{
	while (!exit)
	{
		const int timeoutMilliseconds = 10;
		int readySockets = SDLNet_CheckSockets (socketSet, timeoutMilliseconds);

		if (exit) break;

		if (readySockets == -1)
		{
			//return value of -1 means that most likely the socket set is empty
			SDL_Delay (10);
		}

		if (readySockets > 0 || closingSockets.size() > 0 || connectTo)
		{
			std::unique_lock<std::recursive_mutex> tl (tcpMutex);

			//handle incoming data
			for (size_t i = 0; i < sockets.size();) // erase in loop
			{
				auto& socket = *sockets[i];
				if (SDLNet_SocketReady (socket.sdlSocket))
				{
					socket.buffer.reserve (1024);
					int recvlength;
					recvlength = SDLNet_TCP_Recv (socket.sdlSocket, socket.buffer.getWritePointer(), socket.buffer.getFreeSpace());
					if (recvlength <= 0)
					{
						close (socket);
						continue;
					}

					socket.buffer.length += recvlength;

					pushReadyMessages (socket);
				}
				i++;
			}

			//handle incoming connections
			if (serverSocket && SDLNet_SocketReady (serverSocket))
			{
				TCPsocket sdlSocket = SDLNet_TCP_Accept (serverSocket);

				if (sdlSocket != nullptr)
				{
					{
						// log
						IPaddress* remoteAddress = SDLNet_TCP_GetPeerAddress (sdlSocket);
						std::string ip = std::to_string (remoteAddress->host & 0xFF) + '.';
						ip += std::to_string ((remoteAddress->host >> 8) & 0xFF) + '.';
						ip += std::to_string ((remoteAddress->host >> 16) & 0xFF) + '.';
						ip += std::to_string ((remoteAddress->host >> 24) & 0xFF);

						NetLog.debug ("Network: Incoming connection from " + ip);
					}

					if (sockets.size() + 1 >= MAX_TCP_CONNECTIONS) // +1 for serverSocket
					{
						SDLNet_TCP_Close (sdlSocket);
						NetLog.warn ("Network: Maximum number of tcp connections reached. Connection closed.");
					}
					else
					{
						sockets.push_back (std::make_unique<cSocket> (sdlSocket));
						SDLNet_TCP_AddSocket (socketSet, sdlSocket);
						connectionManager.incomingConnection (*sockets.back());
					}
				}
			}

			//handle connection request from client
			if (connectTo)
			{
				IPaddress ipaddr;
				if (SDLNet_ResolveHost (&ipaddr, connectTo->ip.c_str(), connectTo->port) == -1)
				{
					Log.warn ("Network: Couldn't resolve host");
					connectionManager.connectionResult (nullptr);
				}
				else
				{
					TCPsocket sdlSocket = SDLNet_TCP_Open (&ipaddr);
					if (sdlSocket == nullptr)
					{
						Log.warn ("Network: Couldn't connect to host");
						connectionManager.connectionResult (nullptr);
					}
					else
					{
						sockets.push_back (std::make_unique<cSocket> (sdlSocket));
						SDLNet_TCP_AddSocket (socketSet, sdlSocket);
						connectionManager.connectionResult (sockets.back().get());
					}
				}
				connectTo = std::nullopt;
			}
			cleanupClosedSockets();
		}
	}
}

//------------------------------------------------------------------------------
void cNetwork::pushReadyMessages (cSocket& socket)
{
	//push all received messages
	int readPos = 0;
	for (;;)
	{
		if (socket.buffer.length - readPos < HEADER_LENGTH) break;

		//check message delimiter
		uint32_t startWord = SDL_SwapLE32 (*reinterpret_cast<uint32_t*> (socket.buffer.data + readPos));
		if (startWord != START_WORD)
		{
			//something went terribly wrong. We are unable to continue the communication.
			NetLog.error ("Network: Wrong start character in received message. Socket closed!");
			close (socket);
			break;
		}

		// read message length
		uint32_t messageLength = SDL_SwapLE32 (*reinterpret_cast<uint32_t*> (socket.buffer.data + readPos + 4));
		if (messageLength > PACKAGE_LENGTH)
		{
			NetLog.error ("Network: Length of received message exceeds PACKAGE_LENGTH. Socket closed!");
			close (socket);
			break;
		}

		//check if there is a complete message in buffer
		if (socket.buffer.length - readPos - HEADER_LENGTH < messageLength) break;

		//push message
		connectionManager.messageReceived (socket, socket.buffer.data + readPos + HEADER_LENGTH, messageLength);

		//socket died during handling in connectionManager
		if (ranges::find_if (sockets, ByGetTo (&socket)) == sockets.end()) return;

		//save position of next message
		readPos += messageLength + HEADER_LENGTH;
	}

	socket.buffer.deleteFront (readPos);
}

//------------------------------------------------------------------------------
void cNetwork::cleanupClosedSockets()
{
	for (TCPsocket socket : closingSockets)
	{
		if (socket != nullptr)
		{
			SDLNet_TCP_Close (socket);
			SDLNet_TCP_DelSocket (socketSet, socket);
		}
	}
	closingSockets.clear();
}
