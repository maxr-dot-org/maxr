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
#include <string>
#include <memory>
#include "defines.h"
#include "utility/mutex.h"

class cNetMessage;

#define MAX_CLIENTS     10   // maximal number of clients that can connect to the server
#define PACKAGE_LENGTH  1024 // maximal length of a TCP/IP package
#define START_CHAR      ('\xFF') // start character in netmessages

// the first client message must be smaller then the first menu message!
#define FIRST_SERVER_MESSAGE 10
#define FIRST_CLIENT_MESSAGE 100
#define FIRST_MENU_MESSAGE   1000

/**
* Callback for the networkthread
*@author alzi alias DoctorDeath
*/
int CallbackHandleNetworkThread (void* arg);

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

//------------------------------------------------------------------------
/**
* Structure with data and its length.
*@author alzi alias DoctorDeath
*/
struct sDataBuffer
{
	uint32_t iLength;
	char data[5 * PACKAGE_LENGTH];

	char* getWritePointer();
	int getFreeSpace() const;
	void deleteFront (int n);

	/**
	* Clears the data buffer and sets his length to 0.
	*@author alzi alias DoctorDeath
	*/
	void clear();
};

//------------------------------------------------------------------------
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
	unsigned int messagelength;
};

/**
* Interface called each time a message is received by network.
*/
class INetMessageReceiver
{
public:
	virtual ~INetMessageReceiver() {}
	virtual void pushEvent (std::unique_ptr<cNetMessage> message) = 0;
};

//------------------------------------------------------------------------
/**
* Class for the handling of events over TCP/IP
*@author alzi alias DoctorDeath
*/
class cTCP
{
public:
	/**
	 * Creates the mutexes, initialises some variables
	 * and the sockets and starts the network thread.
	 *@author alzi alias DoctorDeath
	 */
	cTCP();

	/**
	 * Destroys the mutexes, the sockets and exits the network thread.
	 *@author alzi alias DoctorDeath
	 */
	~cTCP();

	/**
	* Creates a new server on the port.
	*@author alzi alias DoctorDeath
	*return 0 on succes, -1 if an error occurs
	*/
	int create (int port);
	/**
	* Connects as client to the IP on the port.
	*@author alzi alias DoctorDeath
	*return 0 on succes, -1 if an error occurs
	*/
	int connect (const std::string& sIP, int port);

	/**
	* checks whether the given socket is connected
	*/
	bool isConnected (unsigned int socketIndex) const;
	/**
	* Closes the connection to the socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket
	*                    to which the connection should be closed.
	*/
	void close (unsigned int iClientNumber);

	/**
	* Sends data of an given length to the client/socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket
	*                    to which the data should be send.
	*param iLength Length of data to be send.
	*param buffer buffer with data to be send.
	*return 0 on succes, -1 if an error occurs
	*/
	int sendTo (unsigned int iClientNumber, unsigned int iLength, const char* buffer);
	/**
	* Sends the data to all sockets to which this machine is connected.
	*@author alzi alias DoctorDeath
	*param iLength Length of data to be send.
	*param buffer buffer with data to be send.
	*return 0 on succes, -1 if an error occurs
	*/
	int send (unsigned int iLength, const char* buffer);

	/**
	* Set message receiver.
	* @author joris alias Jarod
	*/
	void setMessageReceiver (INetMessageReceiver* messageReceiver);

	/**
	* Gets the number of currently connected sockets.
	*@author alzi alias DoctorDeath
	*return Number of sockets.
	*/
	int getSocketCount() const;
	/**
	* Gets the status of the connection.
	*@author alzi alias DoctorDeath
	*return 1 if connected, 0 if not.
	*/
	int getConnectionStatus() const;
	/**
	* looks if this machine is the host.
	*@author alzi alias DoctorDeath
	*return true if is host, false if not.
	*/
	bool isHost() const;

	/**
	* Thread function which handles new incoming connections and data.
	*@author alzi alias DoctorDeath
	*/
	void HandleNetworkThread();

private:
	/**
	* Searchs for the first unused socket and allocates memory
	* for a new one if there are no free sockets.
	*@author alzi alias DoctorDeath
	*@return index of found socket
	*/
	int getFreeSocket();
	/**
	* Deletes the socket, frees its memory
	* and sorts the rest sockets in the list.
	*@author alzi alias DoctorDeath
	*/
	void deleteSocket (int socketIndex);

	void pushEvent (std::unique_ptr<cNetMessage> message);

	void pushEventTCP_Close (unsigned int socketIndex);

	void HandleNetworkThread_STATE_NEW (unsigned int socketIndex);
	void HandleNetworkThread_SERVER (unsigned int socketIndex);
	void HandleNetworkThread_CLIENT (unsigned int socketIndex);
	void HandleNetworkThread_CLIENT_pushReadyMessage (unsigned int socketIndex);

private:
	cMutex TCPMutex;

	SDL_Thread* TCPHandleThread;
	volatile bool bExit;
	bool bHost;

	unsigned int iLast_Socket;
	sSocket Sockets[MAX_CLIENTS];
	SDLNet_SocketSet SocketSet;
	IPaddress ipaddr;
	INetMessageReceiver* messageReceiver;
};

#endif // networkH
