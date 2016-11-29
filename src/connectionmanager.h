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

#ifndef connectionmanagerH
#define connectionmanagerH

#include <memory>
#include <vector>
#include "utility/color.h"
#include "utility/thread/mutex.h"

class cNetwork;
class cSocket;
class cNetMessage2;
class cPlayerBasicData;
class cHandshakeTimeout;

/**
* Interface called each time a message is received by network.
*/
class INetMessageReceiver
{
public:
	virtual ~INetMessageReceiver() {}
	virtual void pushMessage(std::unique_ptr<cNetMessage2> message) = 0;
};

class cConnectionManager
{
public:
	cConnectionManager();
	~cConnectionManager();

	int openServer(int port);
	void closeServer();
	bool isServerOpen() const;

	void acceptConnection(const cSocket* socket, int playerNr);
	void declineConnection(const cSocket* socket);
	void connectToServer(const std::string& host, int port, const cPlayerBasicData& player);
	bool isConnectedToServer() const;

	void setLocalClient(INetMessageReceiver* client, int playerNr);
	void setLocalServer(INetMessageReceiver* server);

	int sendToServer(const cNetMessage2& message);
	int sendToPlayer(const cNetMessage2& message, int playerNr);
	int sendToPlayers(const cNetMessage2& message);

	void disconnectAll();


	//callbacks from network thread
	void connectionClosed(const cSocket* socket);
	void incomingConnection(const cSocket* socket);
	void messageReceived(const cSocket* socket, unsigned char* data, int length);
	void connectionResult(const cSocket* socket);

	//callback from timeout timer
	void handshakeTimeoutCallback(cHandshakeTimeout* timer);
private:
	void startTimeout(const cSocket* socket);
	void stopTimeout(const cSocket* socket);
	int sendMessage(const cSocket* socket, const cNetMessage2& message);
	bool handeConnectionHandshake(const std::unique_ptr<cNetMessage2> &message, const cSocket* socket, int playerOnSocket);

	cNetwork* network;
	INetMessageReceiver* localClient;
	INetMessageReceiver* localServer;
	mutable cMutex mutex;

	int localPlayer;
	std::vector<std::pair<const cSocket*, int>> clientSockets;
	const cSocket* serverSocket;

	std::vector<cHandshakeTimeout*> timeouts;

	bool serverOpen;

	// temporary save player credentials (used during non blocking connection attempt)
	bool connecting;
	std::string connectingPlayerName;
	cRgbColor connectingPlayerColor;
	bool connectingPlayerReady;

};

#endif
