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
#include "events.h"
#include "log.h"
#include "main.h"
#include "netmessage.h"

//------------------------------------------------------------------------
// sSocket implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
sSocket::sSocket()
{
	iType = FREE_SOCKET;
	iState = STATE_UNUSED;
	messagelength = 0;
	buffer.clear();
}

//------------------------------------------------------------------------
sSocket::~sSocket()
{
	if (iType != FREE_SOCKET) SDLNet_TCP_Close (socket);
}


//------------------------------------------------------------------------
// sDataBuffer implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
void sDataBuffer::clear()
{
	iLength = 0;
}

//------------------------------------------------------------------------
char* sDataBuffer::getWritePointer()
{
	return data + iLength;
}

//------------------------------------------------------------------------
int sDataBuffer::getFreeSpace() const
{
	return PACKAGE_LENGTH - iLength;
}

//------------------------------------------------------------------------
void sDataBuffer::deleteFront (int n)
{
	memmove (data, data + n, iLength - n);
	iLength -= n;
}


//------------------------------------------------------------------------
// cTCP implementation
//------------------------------------------------------------------------

//------------------------------------------------------------------------
cTCP::cTCP() :
	TCPMutex(),
	messageReceiver (NULL)
{
	SocketSet = SDLNet_AllocSocketSet (MAX_CLIENTS);

	iLast_Socket = 0;
	bHost = false;

	bExit = false;
	TCPHandleThread = SDL_CreateThread (CallbackHandleNetworkThread, "network", this);
}

//------------------------------------------------------------------------
cTCP::~cTCP()
{
	bExit = true;
	SDL_WaitThread (TCPHandleThread, NULL);
}

//------------------------------------------------------------------------
int cTCP::create (int iPort)
{
	cMutex::Lock tl (TCPMutex);
	if (SDLNet_ResolveHost (&ipaddr, NULL, iPort) == -1) { return -1; }

	const int iNum = getFreeSocket();
	if (iNum == -1) { return -1; }

	Sockets[iNum].socket = SDLNet_TCP_Open (&ipaddr);
	if (!Sockets[iNum].socket)
	{
		deleteSocket (iNum);
		return -1;
	}

	Sockets[iNum].iType = SERVER_SOCKET;
	Sockets[iNum].iState = STATE_NEW;

	bHost = true; // is the host

	return 0;
}

//------------------------------------------------------------------------
int cTCP::connect (const std::string& sIP, int iPort)
{
	cMutex::Lock tl (TCPMutex);
	if (SDLNet_ResolveHost (&ipaddr, sIP.c_str(), iPort) == -1) { return -1; }

	const int socketIndex = getFreeSocket();
	if (socketIndex == -1) { return -1; }

	Sockets[socketIndex].socket = SDLNet_TCP_Open (&ipaddr);
	if (!Sockets[socketIndex].socket)
	{
		deleteSocket (socketIndex);
		return -1;
	}

	Sockets[socketIndex].iType = CLIENT_SOCKET;
	Sockets[socketIndex].iState = STATE_NEW;

	bHost = false; // is not the host
	return 0;
}

bool cTCP::isConnected (unsigned int socketIndex) const
{
	if (socketIndex == MAX_CLIENTS) return true;
	const sSocket& socket = Sockets[socketIndex];
	return socket.iState == STATE_NEW || socket.iState == STATE_READY;
}

//------------------------------------------------------------------------
int cTCP::sendTo (unsigned int iClientNumber, unsigned int iLength, const char* buffer)
{
	cMutex::Lock tl (TCPMutex);

	if (iClientNumber >= iLast_Socket ||
		isConnected (iClientNumber) == false ||
		Sockets[iClientNumber].iType != CLIENT_SOCKET ||
		iLength == 0)
	{
		return 0;
	}
	// if the message is too long, cut it.
	// this will result in an error in nearly all cases
	if (iLength > PACKAGE_LENGTH)
	{
		Log.write ("Cut size of message!", LOG_TYPE_NET_ERROR);
		iLength = PACKAGE_LENGTH;
	}

	// send the message
	const unsigned int iSendLength = SDLNet_TCP_Send (Sockets[iClientNumber].socket, buffer, iLength);

	// delete socket when sending fails
	if (iSendLength != iLength)
	{
		Sockets[iClientNumber].iState = STATE_DYING;
		pushEventTCP_Close (iClientNumber);
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------
int cTCP::send (unsigned int iLength, const char* buffer)
{
	cMutex::Lock tl (TCPMutex);
	int iReturnVal = 0;
	for (int i = 0; i < getSocketCount(); i++)
	{
		if (sendTo (i, iLength, buffer) == -1)
		{
			iReturnVal = -1;
		}
	}
	return iReturnVal;
}

//------------------------------------------------------------------------
int CallbackHandleNetworkThread (void* arg)
{
	cTCP* TCP = reinterpret_cast<cTCP*> (arg);
	TCP->HandleNetworkThread();
	return 0;
}

void cTCP::HandleNetworkThread_STATE_NEW (unsigned int socketIndex)
{
	sSocket& socket = Sockets[socketIndex];
	assert (socket.iState == STATE_NEW);
	cMutex::Lock tl (TCPMutex);
	if (SDLNet_TCP_AddSocket (SocketSet, socket.socket) != -1)
	{
		socket.iState = STATE_READY;
	}
	else
	{
		socket.iState = STATE_DELETE;
	}
}

void cTCP::HandleNetworkThread_SERVER (unsigned int socketIndex)
{
	assert (Sockets[socketIndex].iType == SERVER_SOCKET);
	cMutex::Lock tl (TCPMutex);
	TCPsocket socket = SDLNet_TCP_Accept (Sockets[socketIndex].socket);

	if (socket == NULL) return;

	Log.write ("Incoming connection!", cLog::eLOG_TYPE_NET_DEBUG);
	const int iNum = getFreeSocket();
	if (iNum != -1)
	{
		Log.write ("Connection accepted and assigned socket number " + iToStr (iNum), cLog::eLOG_TYPE_NET_DEBUG);
		Sockets[iNum].socket = socket;

		Sockets[iNum].iType = CLIENT_SOCKET;
		Sockets[iNum].iState = STATE_NEW;
		Sockets[iNum].buffer.clear();
		cNetMessage* message = new cNetMessage (TCP_ACCEPT);
		message->pushInt16 (iNum);
		pushEvent (message);
	}
	else SDLNet_TCP_Close (socket);
}

void cTCP::HandleNetworkThread_CLIENT (unsigned int socketIndex)
{
	sSocket& s = Sockets[socketIndex];
	assert (s.iType == CLIENT_SOCKET && s.iState == STATE_READY);
	{
		cMutex::Lock tl (TCPMutex);

		//read available data from the socket to the buffer
		int recvlength;
		recvlength = SDLNet_TCP_Recv (s.socket, s.buffer.getWritePointer(), s.buffer.getFreeSpace());
		if (recvlength < 0) //TODO: gleich 0???
		{
			pushEventTCP_Close (socketIndex);
			s.iState = STATE_DYING;
			return;
		}

		s.buffer.iLength += recvlength;
	}
	HandleNetworkThread_CLIENT_pushReadyMessage (socketIndex);
}

void cTCP::HandleNetworkThread_CLIENT_pushReadyMessage (unsigned int socketIndex)
{
	sSocket& s = Sockets[socketIndex];
	//push all received messages
	int readPos = 0;
	for (;;)
	{
		if (s.buffer.iLength - readPos < 3) break;

		//get length of next message
		if (s.messagelength == 0)
		{
			if (s.buffer.data[readPos] != START_CHAR)
			{
				//something went terribly wrong. We are unable to continue the communication.
				Log.write ("Wrong start character in received message. Socket closed!", LOG_TYPE_NET_ERROR);
				pushEventTCP_Close (socketIndex);
				s.iState = STATE_DYING;
				break;
			}
			// Use temporary variable to avoid gcc warning:
			// "dereferencing type-punned pointer will break strict-aliasing rules"
			const Sint16* data16 = reinterpret_cast<Sint16*> (s.buffer.data + readPos + 1);
			s.messagelength = SDL_SwapLE16 (*data16);
			if (s.messagelength > PACKAGE_LENGTH)
			{
				Log.write ("Length of received message exceeds PACKAGE_LENGTH", LOG_TYPE_NET_ERROR);
				pushEventTCP_Close (socketIndex);
				s.iState = STATE_DYING;
				break;
			}
		}

		//check if there is a complete message in buffer
		if (s.buffer.iLength - readPos < s.messagelength) break;

		//push message
		cNetMessage* message = new cNetMessage (s.buffer.data + readPos);
		pushEvent (message);

		//save position of next message
		readPos += s.messagelength;
		s.messagelength = 0;
	}

	s.buffer.deleteFront (readPos);
}

//------------------------------------------------------------------------
void cTCP::HandleNetworkThread()
{
	while (!bExit)
	{
		const int timeout = 10;
		SDLNet_CheckSockets (SocketSet, timeout);

		// Check all Sockets
		for (unsigned int i = 0; !bExit && i < iLast_Socket; i++)
		{
			if (Sockets[i].iState == STATE_NEW)
			{
				// there has to be added a new socket
				HandleNetworkThread_STATE_NEW (i);
			}
			else if (Sockets[i].iState == STATE_DELETE)
			{
				// there has to be deleted a socket
				cMutex::Lock tl (TCPMutex);
				SDLNet_TCP_DelSocket (SocketSet, Sockets[i].socket);
				deleteSocket (i);
				i--;
			}
			else if (Sockets[i].iType == SERVER_SOCKET && SDLNet_SocketReady (Sockets[i].socket))
			{
				// there is a new connection
				HandleNetworkThread_SERVER (i);
			}
			else if (Sockets[i].iType == CLIENT_SOCKET && Sockets[i].iState == STATE_READY && SDLNet_SocketReady (Sockets[i].socket))
			{
				// there has to be received new data
				HandleNetworkThread_CLIENT (i);
			}
		}
	}
}

//------------------------------------------------------------------------
void cTCP::pushEvent (cNetMessage* message)
{
	if (messageReceiver == NULL)
	{
		Log.write ("Discarded message: no receiver!", LOG_TYPE_NET_ERROR);
		delete message;
		return;
	}
	messageReceiver->pushEvent (message);
}

//------------------------------------------------------------------------
void cTCP::pushEventTCP_Close (unsigned int socketIndex)
{
	cNetMessage* message = new cNetMessage (TCP_CLOSE);
	message->pushInt16 (socketIndex);
	pushEvent (message);
}

//------------------------------------------------------------------------
void cTCP::close (unsigned int iClientNumber)
{
	cMutex::Lock tl (TCPMutex);
	if (iClientNumber < iLast_Socket && (Sockets[iClientNumber].iType == CLIENT_SOCKET || Sockets[iClientNumber].iType == SERVER_SOCKET))
	{
		Sockets[iClientNumber].iState = STATE_DELETE;
	}
}

//------------------------------------------------------------------------
void cTCP::deleteSocket (int iNum)
{
	Sockets[iNum].~sSocket();
	for (unsigned int i = iNum; i + 1 < iLast_Socket; ++i)
	{
		Sockets[i] = Sockets[i + 1];
		memcpy (Sockets[i].buffer.data, Sockets[i + 1].buffer.data, Sockets[i].buffer.iLength);
	}
	Sockets[iLast_Socket - 1].iType = FREE_SOCKET;
	Sockets[iLast_Socket - 1].iState = STATE_UNUSED;
	Sockets[iLast_Socket - 1].messagelength = 0;
	Sockets[iLast_Socket - 1].buffer.clear();
	iLast_Socket--;
}

//------------------------------------------------------------------------
void cTCP::setMessageReceiver (INetMessageReceiver* messageReceiver_)
{
	cMutex::Lock lock (TCPMutex);

	this->messageReceiver = messageReceiver_;
}

//------------------------------------------------------------------------
int cTCP::getSocketCount() const
{
	return iLast_Socket;
}

//------------------------------------------------------------------------
int cTCP::getConnectionStatus() const
{
	if (iLast_Socket > 0)
		return 1;
	return 0;
}

//------------------------------------------------------------------------
bool cTCP::isHost() const
{
	return bHost;
}

//------------------------------------------------------------------------
int cTCP::getFreeSocket()
{
	if (iLast_Socket == MAX_CLIENTS) return -1;
	for (unsigned int iNum = 0; iNum < iLast_Socket; iNum++)
	{
		if (Sockets[iNum].iType == FREE_SOCKET) return iNum;
	}
	Sockets[iLast_Socket].iType = FREE_SOCKET;
	iLast_Socket++;
	return iLast_Socket - 1;
}
