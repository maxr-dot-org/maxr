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
#ifndef fstcpipH
#define fstcpipH
#include <SDL_net.h>
#include <SDL_thread.h>
#include "defines.h"
#include "main.h"

#define STAT_OPENED		100
#define STAT_CLOSED		101
#define STAT_WAITING	102
#define STAT_CONNECTED	103

#define BUFF_TYP_DATA	200
#define BUFF_TYP_OK		201
#define BUFF_TYP_RESEND	202
#define BUFF_TYP_NEWID	203

/**
* Message class with message, lenght and typ
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
class cNetMessage{
public:
	int typ;
	int lenght;
	char msg[256];
};

/**
* Buffer for incomming and outcomming packages
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
struct sNetBuffer{
	unsigned int iID;	// ID of this Message
	int iTyp;			// Typ of Buffer: see BUFF_TYP_XYZ
	int iPart;			// Partnumber of message
	int iMax_parts;		// Maximal number of parts
	cNetMessage msg;	// Message for Data-Packages
};

struct sIDList{
	sIDList()
	{
		iCount = 0;
	}
	unsigned int iID[64];  // This isn't good but it hasn't worked with pointer. But normaly there shouldn't be more then 64 ids
	int iCount;

	void Add(unsigned int iSrcID)
	{
		if(iCount > 64) return; // Saves from bufferoverflow
		iID[iCount] = iSrcID;
		iCount++;
	}
	void Delete(int iIndex)
	{
		if(iIndex >= iCount) iIndex = iCount-1;
		iID[iIndex] = 0;
		iCount--;
		for (int i = iIndex; i <= iCount; i++)
			iID[i] = iID[i + 1];
	}
};

/**
* Failsafe TcpIp class.
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
class cFSTcpIp{
public:
	cFSTcpIp(bool server);
	~cFSTcpIp();
	SDL_Thread *FSTcpIpReceiveThread;	// Thread for message receiving
	bool bReceiveThreadFinished;		// Has the Receive-Thread finished?
	bool bServer;						// Is this a server?
	int iStatus;						// Status of connection
	int iMax_clients;					// Maximal clients that can connect
	int iMin_clients;					// Minimal clients needed to run stable

	bool FSTcpIpOpen(void);
	bool FSTcpIpCreate(void);
	void FSTcpIpClose(void);
	bool FSTcpIpSend(int typ, const char *msg);
	bool FSTcpIpReceive(void);

	void SetTcpIpPort(int port);
	void SetIp(string ip);
	int GetConnectionCount(void);

private:
	int GenerateNewID();
	void SendNewID(unsigned int iNewID, int iClientNum);
	void SendOK(unsigned int iID, int iClientNum /* -1 For server*/ );

	unsigned int iNextMessageID;	// ID of next Message
	sIDList *UsedIDs;				// A List with all currently used message IDs (server only)
	sIDList *WaitOKList;			// A List with all IDs of messages, the game is waiting for an Reseive-OK
	sNetBuffer *NetBuffer;			// Buffer for incominig and outcoming Packages
	int iPlayerId;					// ID of this Player
	int iNum_clients;				// Number of current clients
	int iPort;						// Current port
	string sIp;						// Current ip or hostname
	IPaddress addr;					// Address for SDL_Net
	TCPsocket sock_server, sock_client[16];	// Sockets of Server (clients only) or for maximal 16 clients (server only)
	SDLNet_SocketSet SocketSet;		// The socket-set with all currently connected clients
};

/**
* Receive funktion for FSTcpIpReceiveThread.
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
int Receive(void *);
/**
* Open funktion for FSTcpIpReceiveThread (server only).
* In this funktion receiving of messages is integrated while the server is waiting for clients.
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
int Open(void *);

#endif
