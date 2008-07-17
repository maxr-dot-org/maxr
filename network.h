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
#ifndef networkH
#define networkH
#include <SDL_net.h>
#include "defines.h"
#include "main.h"

#define MAX_CLIENTS 10
#define PACKAGE_LENGHT 256

// the first client message must be smaller then the first menu message!
#define FIRST_CLIENT_MESSAGE 50
#define FIRST_MENU_MESSAGE 100

// All three chars have to be different!
#define NETMESSAGE_CONTROLCHAR 0xFF
#define NETMESSAGE_STARTCHAR 0x00
#define NETMESSAGE_NOTSTARTCHAR 0xEE

/**
* Callback for the networkthread
*@author alzi alias DoctorDeath
*/
int CallbackHandleNetworkThread( void *arg );

enum EVENT_TYPES
{
	TCP_ACCEPTEVENT,
	TCP_RECEIVEEVENT,
	TCP_CLOSEEVENT
};

enum SOCKET_TYPES
{
	FREE_SOCKET,
	SERVER_SOCKET,
	CLIENT_SOCKET
};

enum SOCKET_STATES
{
	STATE_UNUSED,
	STATE_READY,
	STATE_NEW,
	STATE_DYING,
	STATE_DELETE
};

/**
* Structure with data and its lenght.
*@author alzi alias DoctorDeath
*/
struct sDataBuffer
{
	Uint32 iLenght;
	char data[PACKAGE_LENGHT];

	/**
	* Clears the data buffer and sets his lenght to 0.
	*@author alzi alias DoctorDeath
	*/
	void clear();
};

/**
* Structure for Sockets used by the TCP-Class.
*@author alzi alias DoctorDeath
*/
struct sSocket
{
	sSocket();
	~sSocket();

	int iType;
	int iState;

	TCPsocket socket;
	sDataBuffer buffer;
	int iLeftBytes;
};

/**
* Class for the handling of events over TCP/IP
*@author alzi alias DoctorDeath
*/
class cTCP
{
public:
	/**
	 * Creates the mutexes, initialises some variables and the sockets and starts the network thread.
	 *@author alzi alias DoctorDeath
	 */
	cTCP();

	/**
	 * Destroys the mutexes, the sockets and exits the network thread.
	 *@author alzi alias DoctorDeath
	 */
	~cTCP();

private:
	SDL_mutex *TCPMutex;
	bool bDataLocked;
	bool bTCPLocked;
	bool bWaitForRead;

	SDL_Thread *TCPHandleThread;
	bool bExit;
	bool bHost;

	int iPort;
	int iLast_Socket;
	string sIP;
	sSocket Sockets[MAX_CLIENTS];
	SDLNet_SocketSet SocketSet;
	IPaddress ipaddr;

	/**
	* Searchs for the first unused socket and allocates memory for a new one if there are no free sockets.
	*@author alzi alias DoctorDeath
	*@return index of found socket
	*/
	int getFreeSocket();
	/**
	* Deletes the socket, frees its memory and sorts the rest sockets in the list.
	*@author alzi alias DoctorDeath
	*/
	void deleteSocket( int iNum );

	/**
	* Locks the network mutex.
	*@author alzi alias DoctorDeath
	*/
	void lockTCP();
	/**
	* Unlocks the network mutex.
	*@author alzi alias DoctorDeath
	*/
	void unlockTCP();
	/**
	* Locks the data mutex.
	*@author alzi alias DoctorDeath
	*/
	void lockData();
	/**
	* Unlocks the data mutex.
	*@author alzi alias DoctorDeath
	*/
	void unlockData();
	/**
	* Sends a signal to the conditional variable.
	*@author alzi alias DoctorDeath
	*/
	void sendReadFinished();
	/**
	* waits until data will be read
	*@author alzi alias DoctorDeath
	*/
	void waitForRead();

	/**
	* Pushes an event to the event handling or to the server eventqueue
	*@author alzi alias DoctorDeath
	*@param iEventType Typ of the event to push ( see EVENT_TYPES ).
	*@param data1 first data of the event.
	*@param data2 second data of the event.
	*@return Allways 0 for success since it waits until the event can be pushed.
	*/
	int pushEvent( int iEventType, void *data1, void *data2 );
	/**
	* gets the position of the next NETMESSAGE_CONTROLCHAR character
	*@author alzi alias DoctorDeath
	*@param iStartPos The start position from which the search should beginn.
	*@param data Data in which the search should take place.
	*@param iLength The length of the data.
	*@return Position of the first NETMESSAGE_CONTROLCHAR charcter or -1 if non has been found
	*/
	int findNextMessageStart ( int iStartPos, char *data, int iLength );
public:
	/**
	* Creates a new server on the port which has to be set before.
	*@author alzi alias DoctorDeath
	*return 0 on succes, -1 if an error occurs
	*/
	int create();
	/**
	* Connects as client to the IP on the port which both have to be set before.
	*@author alzi alias DoctorDeath
	*return 0 on succes, -1 if an error occurs
	*/
	int connect();
	/**
	* Closes the connection to the socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket to which the connection should be closed.
	*/
	void close( int iClientNumber );

	/**
	* Sends data of an given lenght to the client/socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket to which the data should be send.
	*param iLenght Lenght of data to be send.
	*param buffer buffer with data to be send.
	*return 0 on succes, -1 if an error occurs
	*/
	int sendTo( int iClientNumber, int iLenght, char *buffer );
	/**
	* Sends the data to all sockets to which this machine is connected.
	*@author alzi alias DoctorDeath
	*param iLenght Lenght of data to be send.
	*param buffer buffer with data to be send.
	*return 0 on succes, -1 if an error occurs
	*/
	int send( int iLenght, char *buffer );
	/**
	* Reads data of an given lenght from the client/socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket form which the data should be read.
	*param iLenght Lenght of data to be read.
	*param buffer buffer with data to be read.
	*return 0 on succes, -1 if an error occurs
	*/
	int read( int iClientNumber, int iLenght, char *buffer );

	/**
	* Sets a new port.
	*@author alzi alias DoctorDeath
	*param iPort New port number.
	*/
	void setPort( int iPort );
	/**
	* Sets a new IP.
	*@author alzi alias DoctorDeath
	*param iPort New IP.
	*/
	void setIP ( string sIP );
	/**
	* Gets the number of currently connected sockets.
	*@author alzi alias DoctorDeath
	*return Number of sockets.
	*/
	int getSocketCount();
	/**
	* Gets the status of the connection.
	*@author alzi alias DoctorDeath
	*return 1 if connected, 0 if not.
	*/
	int getConnectionStatus();
	/**
	* looks if this machine is the host.
	*@author alzi alias DoctorDeath
	*return true if is host, false if not.
	*/
	bool isHost();

	/**
	* Thread funktion which new incomming connections and data.
	*@author alzi alias DoctorDeath
	*/
	void HandleNetworkThread();
} EX *network;
#endif // networkH
