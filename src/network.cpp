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
#include "utility/log.h"

#include "netmessage2.h"
#include "connectionmanager.h"
#include "utility/listhelpers.h"
#include "utility/string/toString.h"

#define START_WORD 0x4D415852
#define HEADER_LENGTH 8

//------------------------------------------------------------------------
// cSocket implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cSocket::cSocket(TCPsocket socket):
	sdlSocket(socket)
{}

//------------------------------------------------------------------------
cSocket::~cSocket()
{
	if (sdlSocket != nullptr) SDLNet_TCP_Close (sdlSocket);
}


//------------------------------------------------------------------------
// cDataBuffer implementation
//------------------------------------------------------------------------
cDataBuffer::cDataBuffer() :
	length(0),
	capacity(0),
	data(nullptr)
{}

//------------------------------------------------------------------------------
void cDataBuffer::reserve(unsigned int i)
{
	if (getFreeSpace() < i && length < UINT32_MAX - i)
	{
		capacity = length + i;
		data = (unsigned char*) realloc(data, capacity);
	}
}

//------------------------------------------------------------------------
unsigned char* cDataBuffer::getWritePointer()
{
	return data + length;
}

//------------------------------------------------------------------------
uint32_t cDataBuffer::getFreeSpace() const
{
	return capacity - length;
}

//------------------------------------------------------------------------
void cDataBuffer::deleteFront (uint32_t n)
{
	memmove (data, data + n, length - n);
	length -= n;
}


//------------------------------------------------------------------------
// cNetwork implementation
//------------------------------------------------------------------------


cNetwork::cNetwork(cConnectionManager& connectionManager, cMutex& mutex) :
	exit(false),
	connectionManager(connectionManager),
	serverSocket(nullptr),
	socketsChanged(false),
	tcpMutex(mutex)
{
	socketSet = SDLNet_AllocSocketSet(0);
	tcpHandleThread = SDL_CreateThread(networkThreadCallback, "network", this);
}

//------------------------------------------------------------------------
cNetwork::~cNetwork()
{
	exit = true;
	SDL_WaitThread(tcpHandleThread, nullptr);
	SDLNet_FreeSocketSet(socketSet);
	if (serverSocket)
	{
		SDLNet_TCP_Close(serverSocket);
	}
	cleanupClosedSockets();
	for (auto socket : sockets)
	{
		delete socket;
	}
}

//------------------------------------------------------------------------
int cNetwork::openServer(int port)
{
	cLockGuard<cMutex> tl(tcpMutex);
	
	Log.write("Network: Open server on port: " + toString(port), cLog::eLOG_TYPE_NET_DEBUG);

	IPaddress ipaddr;
	if (SDLNet_ResolveHost(&ipaddr, nullptr, port) == -1) 
	{ 
		return -1;
	}

	TCPsocket socket = SDLNet_TCP_Open(&ipaddr);
	if (socket == nullptr)
	{
		return -1;
	}

	serverSocket = socket;
	socketsChanged = true;

	return 0;
}

//------------------------------------------------------------------------
void cNetwork::closeServer()
{
	cLockGuard<cMutex> tl(tcpMutex);

	SDLNet_TCP_Close(serverSocket);
	serverSocket = nullptr;
	socketsChanged = true;
}

//------------------------------------------------------------------------
void cNetwork::connectToServer(const std::string& ip, int port)
{
	cLockGuard<cMutex> tl(tcpMutex);
	
	if (!connectToIp.empty())
	{
		Log.write("Network: Can only handle one connection attempt at once", cLog::eLOG_TYPE_NET_ERROR);
		connectionManager.connectionResult(nullptr);
		return;
	}

	connectToIp = ip;
	connectToPort = port;
}

//------------------------------------------------------------------------
void cNetwork::close(cSocket* socket)
{
	cLockGuard<cMutex> tl(tcpMutex);

	if (!Contains(sockets, socket))
	{
		Log.write("Network: Unable to close socket. Invalid socket", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	connectionManager.connectionClosed(socket);

	// socket will be cleaned up later by the networkthread. This cannot be done
	// immediately, because the network thread may be still using the socket in SDLNet_CheckSockets
	closingSockets.push_back(socket);
	Remove(sockets, socket);
	socketsChanged = true;
}

//------------------------------------------------------------------------
int cNetwork::sendMessage(cSocket* socket, unsigned int length, const unsigned char* buffer)
{
	cLockGuard<cMutex> tl(tcpMutex);

	if (!Contains(sockets, socket))
	{
		Log.write("Network: Unable to send message. Invalid socket", cLog::eLOG_TYPE_NET_ERROR);
		return -1;
	}

	// send message header
	unsigned char header[HEADER_LENGTH];
	reinterpret_cast<int32_t*>(header)[0] = SDL_SwapLE32(START_WORD);
	reinterpret_cast<int32_t*>(header)[1] = SDL_SwapLE32(length);

	if (send(socket, header, HEADER_LENGTH) == -1) return -1;

	// send message data
	return send(socket, buffer, length);

}

//------------------------------------------------------------------------
int cNetwork::send(cSocket* socket, const unsigned char* buffer, unsigned int length)
{
	const unsigned int bytesSent = SDLNet_TCP_Send(socket->sdlSocket, buffer, length);

	// delete socket when sending fails
	if (bytesSent != length)
	{
		// delete socket when sending fails
		Log.write("Network: Error while sending message. Closing socket...", cLog::eLOG_TYPE_NET_WARNING);
		close(socket);
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------
void cNetwork::handleNetworkThread()
{
	while (!exit)
	{
		const int timeoutMilliseconds = 10;
		int readySockets = SDLNet_CheckSockets(socketSet, timeoutMilliseconds);
		
		if (exit) break;
		
		if (readySockets == -1)
		{
			//return value of -1 means that most likely the socket set is empty
			SDL_Delay(10);
		}

		if (readySockets > 0 || socketsChanged || !connectToIp.empty())
		{
			cLockGuard<cMutex> tl(tcpMutex);

			//handle incoming data
			for (size_t i = 0; i < sockets.size();) // erease in loop
			{
				auto socket = sockets[i];
				if (SDLNet_SocketReady(socket->sdlSocket))
				{
					socket->buffer.reserve(1024);
					int recvlength;
					recvlength = SDLNet_TCP_Recv(socket->sdlSocket, socket->buffer.getWritePointer(), socket->buffer.getFreeSpace());
					if (recvlength <= 0)
					{
						close(socket);
						continue;
					}

					socket->buffer.length += recvlength;

					pushReadyMessages(socket);
				}
				i++;
			}

			//handle incoming connections
			if (serverSocket && SDLNet_SocketReady(serverSocket))
			{
				//TODO: close server socket, when max clients reached?
				TCPsocket sdlSocket = SDLNet_TCP_Accept(serverSocket);

				if (sdlSocket != nullptr)
				{
					{
						// log
						IPaddress* remoteAddress = SDLNet_TCP_GetPeerAddress(sdlSocket);
						std::string ip = toString(remoteAddress->host & 0xFF) + '.';
						ip += toString((remoteAddress->host >>  8) & 0xFF) + '.';
						ip += toString((remoteAddress->host >> 16) & 0xFF) + '.';
						ip += toString((remoteAddress->host >> 24) & 0xFF);
						
						Log.write("Network: Incoming connection from " + ip, cLog::eLOG_TYPE_NET_DEBUG);
					}
					cSocket* socket = new cSocket(sdlSocket);
					sockets.push_back(socket);
					socketsChanged = true;
					connectionManager.incomingConnection(socket);
				}
			}

			//handle connection request from client
			if (!connectToIp.empty())
			{
				IPaddress ipaddr;
				if (SDLNet_ResolveHost(&ipaddr, connectToIp.c_str(), connectToPort) == -1)
				{
					Log.write("Network: Couldn't resolve host", cLog::eLOG_TYPE_WARNING);
					connectionManager.connectionResult(nullptr);
				}
				else
				{
					TCPsocket tcpSocket = SDLNet_TCP_Open(&ipaddr);
					if (tcpSocket == nullptr)
					{
						Log.write("Network: Couldn't connect to host", cLog::eLOG_TYPE_WARNING);
						connectionManager.connectionResult(nullptr);
					}
					else
					{
						cSocket* socket = new cSocket(tcpSocket);
						sockets.push_back(socket);
						socketsChanged = true;
						connectionManager.connectionResult(socket);
					}
				}
				connectToIp.clear();
			}

			//update socket management structures
			if (socketsChanged)
			{
				cleanupClosedSockets();
				updateSocketSet();
				socketsChanged = false;
			}
		}		
	}
}

//------------------------------------------------------------------------
int cNetwork::networkThreadCallback(void* arg)
{
	cNetwork* network = static_cast<cNetwork*>(arg);
	network->handleNetworkThread();
	return 0;
}

//------------------------------------------------------------------------
void cNetwork::pushReadyMessages(cSocket* socket)
{
	//push all received messages
	int readPos = 0;
	for (;;)
	{
		if (socket->buffer.length - readPos < HEADER_LENGTH) break;

		//check message delimiter
		uint32_t startWord = SDL_SwapLE32(*reinterpret_cast<uint32_t*>(socket->buffer.data + readPos));
		if (startWord != START_WORD)
		{
			//something went terribly wrong. We are unable to continue the communication.
			Log.write("Network: Wrong start character in received message. Socket closed!", cLog::eLOG_TYPE_NET_ERROR);
			close(socket);
			break;
		}

		// read message length
		uint32_t messageLength = SDL_SwapLE32(*reinterpret_cast<uint32_t*>(socket->buffer.data + readPos + 4));
		if (messageLength > PACKAGE_LENGTH)
		{
			Log.write("Network: Length of received message exceeds PACKAGE_LENGTH. Socket closed!", cLog::eLOG_TYPE_NET_ERROR);
			close(socket);
			break;
		}

		//check if there is a complete message in buffer
		if (socket->buffer.length - readPos - HEADER_LENGTH < messageLength) break;

		//push message
		connectionManager.messageReceived(socket, socket->buffer.data + readPos + HEADER_LENGTH, messageLength);

		//save position of next message
		readPos += messageLength + HEADER_LENGTH;
	}

	socket->buffer.deleteFront(readPos);
}

//------------------------------------------------------------------------
void cNetwork::updateSocketSet()
{
	SDLNet_FreeSocketSet(socketSet);

	SDLNet_AllocSocketSet(sockets.size() + 1);

	for (const auto& socket : sockets)
	{
		SDLNet_TCP_AddSocket (socketSet, socket->sdlSocket);
	}
	if (serverSocket != nullptr)
	{
		SDLNet_TCP_AddSocket(socketSet, serverSocket);
	}
}

//------------------------------------------------------------------------
void cNetwork::cleanupClosedSockets()
{
	for (cSocket* socket : closingSockets)
	{
		delete socket;
	}
	closingSockets.clear();
}
