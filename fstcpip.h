//////////////////////////////////////////////////////////////////////////////
// FailSafe TCP/IP
//////////////////////////////////////////////////////////////////////////////
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