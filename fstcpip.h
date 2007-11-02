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

// Defines ///////////////////////////////////////////////////////////////////
#define STAT_OPENED		100
#define STAT_CLOSED		101
#define STAT_WAITING		102
#define STAT_CONNECTED	103

// Globale ///////////////////////////////////////////////////////////////////
EX SDL_Thread *FSTcpIpReceiveThread;
EX SDL_Thread *FSTcpIpSendThread;

// Nachrichten Klasse ////////////////////////////////////////////////////////
class cNetMessage{
public:
	int typ;
	int playerid;
	int lenght;
	unsigned char msg[1024];
};

// FailSafe TCP/IP Klasse ////////////////////////////////////////////////////
class cFSTcpIp{
public:
	cFSTcpIp(bool server);
	~cFSTcpIp();
	bool server;
	int status;
	int max_clients;
	int min_clients;

	bool FSTcpIpOpen(void);
	bool FSTcpIpCreate(void);
	void FSTcpIpClose(void);
	bool FSTcpIpSend(int typ, const void *msg, int len, int num);
	bool FSTcpIpReceive(void);

	void SetTcpIpPort(int port);
	void SetIp(string ip);
	int GetConnectionCount(void);

private:
	cNetMessage *msg;
	int PlayerId;
	int num_clients;
	int port;
	string ip;
	IPaddress addr;
	TCPsocket sock_server, sock_client[16];
	SDLNet_SocketSet SocketSet;
};

// Vordeklerationen //////////////////////////////////////////////////////////
int Receive(void *);
int Open(void *);

#endif
