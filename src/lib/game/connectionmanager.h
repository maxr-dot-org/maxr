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

#ifndef game_connectionmanagerH
#define game_connectionmanagerH

#include <memory>
#include <mutex>
#include <vector>
#include <stdexcept>

class cNetwork;
class cSocket;
class cNetMessage;
class cHandshakeTimeout;
struct sNetworkAddress;

/**
* Interface called each time a message is received by network.
*/
class INetMessageReceiver
{
public:
	virtual ~INetMessageReceiver() {}
	virtual void pushMessage (std::unique_ptr<cNetMessage> message) = 0;
	virtual std::unique_ptr<cNetMessage> popMessage() { throw std::runtime_error ("Method not implemented"); }
};

//------------------------------------------------------------------------------
enum class eDeclineConnectionReason
{
	NotPartOfTheGame,
	AlreadyConnected,
	Other
};

class cConnectionManager
{
public:
	cConnectionManager();
	~cConnectionManager();

	int openServer (int port);
	void closeServer();
	bool isServerOpen() const;

	void acceptConnection (const cSocket&, int playerNr);
	void declineConnection (const cSocket&, eDeclineConnectionReason);
	void connectToServer (const sNetworkAddress&);
	bool isConnectedToServer() const;
	void changePlayerNumber (int currentNr, int newNr);
	bool isPlayerConnected (int playerNr) const;

	void setLocalClient (INetMessageReceiver* client, int playerNr);
	void setLocalServer (INetMessageReceiver* server);
	void setLocalClients (std::vector<INetMessageReceiver*>&&);

	void sendToServer (const cNetMessage&);
	void sendToPlayer (const cNetMessage&, int playerNr);
	void sendToPlayers (const cNetMessage&);

	void disconnect (int player);
	void disconnectAll();

	//callbacks from network thread
	void connectionClosed (const cSocket&);
	void incomingConnection (const cSocket&);
	void messageReceived (const cSocket&, unsigned char* data, int length);
	void connectionResult (const cSocket*);

	//callback from timeout timer
	void handshakeTimeoutCallback (cHandshakeTimeout&);

private:
	void startTimeout (const cSocket&);
	void stopTimeout (const cSocket&);
	int sendMessage (const cSocket&, const cNetMessage&);
	bool handeConnectionHandshake (const std::unique_ptr<cNetMessage>&, const cSocket&, int playerOnSocket);

private:
	std::unique_ptr<cNetwork> network;
	std::vector<INetMessageReceiver*> localClients; // Hotseat
	INetMessageReceiver* localClient = nullptr;
	INetMessageReceiver* localServer = nullptr;
	mutable std::recursive_mutex mutex;

	int localPlayer = -1;
	std::vector<std::pair<const cSocket*, int>> clientSockets;
	const cSocket* serverSocket = nullptr;

	std::vector<std::unique_ptr<cHandshakeTimeout>> timeouts;

	bool serverOpen = false;

	bool connecting = false;
};

#endif
