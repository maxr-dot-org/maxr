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

#include "connectionmanager.h"

#include "game/network.h"
#include "game/protocol/lobbymessage.h"
#include "game/protocol/netmessage.h"
#include "maxrversion.h"
#include "utility/log.h"
#include "utility/ranges.h"

#include <mutex>
#include <string>

// this can be used, when debugging the connection handshake and timeouts are not wanted
#define DISABLE_TIMEOUTS 0

constexpr auto HANDSHAKE_TIMEOUT_MS = 3000;

//------------------------------------------------------------------------------
class cHandshakeTimeout
{
public:
	//--------------------------------------------------------------------------
	cHandshakeTimeout (const cSocket* socket, cConnectionManager* connectionManager) :
		connectionManager (connectionManager),
		socket (socket)
	{
		timer = SDL_AddTimer (HANDSHAKE_TIMEOUT_MS, callback, this);
	}

	//--------------------------------------------------------------------------
	~cHandshakeTimeout()
	{
		SDL_RemoveTimer (timer);
	}

	//--------------------------------------------------------------------------
	const cSocket* getSocket() const { return socket; }

private:
	//--------------------------------------------------------------------------
	static uint32_t callback (uint32_t intervall, void* arg)
	{
		cHandshakeTimeout* thisPtr = reinterpret_cast<cHandshakeTimeout*> (arg);
		thisPtr->connectionManager->handshakeTimeoutCallback (*thisPtr);

		return 0;
	}

	cConnectionManager* connectionManager = nullptr;
	SDL_TimerID timer;
	const cSocket* socket = nullptr;
};

//------------------------------------------------------------------------------
cConnectionManager::cConnectionManager() = default;

//------------------------------------------------------------------------------
cConnectionManager::~cConnectionManager() = default;

//------------------------------------------------------------------------------
int cConnectionManager::openServer (int port)
{
	assert (localServer != nullptr);

	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (serverOpen) return -1;

	if (!network)
		network = std::make_unique<cNetwork> (*this, mutex);

	int result = network->openServer (port);
	if (result == 0)
		serverOpen = true;

	return result;
}

//------------------------------------------------------------------------------
void cConnectionManager::closeServer()
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (!network || !serverOpen) return;

	network->closeServer();
	serverOpen = false;
}

//------------------------------------------------------------------------------
bool cConnectionManager::isServerOpen() const
{
	return serverOpen;
}

//------------------------------------------------------------------------------
void cConnectionManager::acceptConnection (const cSocket& socket, int playerNr)
{
	assert (localServer != nullptr);

	std::unique_lock<std::recursive_mutex> tl (mutex);

	stopTimeout (socket);

	auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.first == &socket; });
	if (it == clientSockets.end())
	{
		//looks like the connection was disconnected during the handshake
		NetLog.warn ("ConnectionManager: accept called for unknown socket");
		localServer->pushMessage (std::make_unique<cNetMessageTcpClose> (playerNr));

		return;
	}

	NetLog.debug ("ConnectionManager: Accepted connection and assigned playerNr: " + std::to_string (playerNr));

	//assign playerNr to the socket
	it->second = playerNr;

	cNetMessageTcpConnected message (playerNr);

	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("ConnectionManager: --> " + json.dump (-1));

	sendMessage (socket, message);
}

//------------------------------------------------------------------------------
void cConnectionManager::declineConnection (const cSocket& socket, eDeclineConnectionReason reason)
{
	assert (localServer != nullptr);

	std::unique_lock<std::recursive_mutex> tl (mutex);

	stopTimeout (socket);

	auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.first == &socket; });
	if (it == clientSockets.end())
	{
		//looks like the connection was disconnected during the handshake
		NetLog.warn ("ConnectionManager: decline called for unknown socket");
		return;
	}

	cNetMessageTcpConnectFailed message (reason);

	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("ConnectionManager: --> " + json.dump (-1));

	sendMessage (socket, message);

	network->close (socket);
}

//------------------------------------------------------------------------------
void cConnectionManager::connectToServer (const sNetworkAddress& address)
{
	assert (localClient != nullptr);

	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (!network)
		network = std::make_unique<cNetwork> (*this, mutex);

	NetLog.debug ("ConnectionManager: Connecting to " + address.toString());

	network->connectToServer (address);

	connecting = true;
}

//------------------------------------------------------------------------------
bool cConnectionManager::isConnectedToServer() const
{
	if (localServer) return true;
	std::unique_lock<std::recursive_mutex> tl (mutex);

	return connecting || serverSocket != nullptr;
}

//------------------------------------------------------------------------------
void cConnectionManager::changePlayerNumber (int currentNr, int newNr)
{
	if (currentNr == newNr) return;
	NetLog.debug ("Connection Manager: ChangePlayerNumber " + std::to_string (currentNr) + " to " + std::to_string (newNr));
	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (localPlayer == currentNr)
	{
		localPlayer = newNr;
		return;
	}

	auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.second == currentNr; });
	if (it == clientSockets.end())
	{
		NetLog.error ("Connection Manager: Can't change playerNr. Unknown player " + std::to_string (currentNr));
		NetLog.debug ("Connection Manager: Known players are:");
		for (const auto& p : clientSockets)
		{
			NetLog.debug ("player " + std::to_string (p.second));
		}
		return;
	}
	it->second = newNr;
}

//------------------------------------------------------------------------------
void cConnectionManager::setLocalClient (INetMessageReceiver* client, int playerNr)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	//move pending messages in the event queue to new receiver
	if (localClient && client)
	{
		while (auto message = localClient->popMessage())
		{
			client->pushMessage (std::move (message));
		}
	}

	localPlayer = playerNr;
	localClient = client;
}

//------------------------------------------------------------------------------
void cConnectionManager::setLocalServer (INetMessageReceiver* server)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	//move pending messages in the event queue to new receiver
	if (localServer && server)
	{
		while (auto message = localServer->popMessage())
		{
			server->pushMessage (std::move (message));
		}
	}

	localServer = server;
}

//------------------------------------------------------------------------------
void cConnectionManager::setLocalClients (std::vector<INetMessageReceiver*>&& clients)
{
	localClients = std::move (clients);
}

//------------------------------------------------------------------------------
void cConnectionManager::sendToServer (const cNetMessage& message)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (localServer)
	{
		localServer->pushMessage (message.clone());
	}
	else if (serverSocket)
	{
		sendMessage (*serverSocket, message);
	}
	else
	{
		NetLog.error ("Connection Manager: Can't send message. No local server and no connection to server");
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::sendToPlayer (const cNetMessage& message, int playerNr)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (playerNr == localPlayer)
	{
		localClient->pushMessage (message.clone());
	}
	else if (static_cast<std::size_t> (playerNr) < localClients.size())
	{
		localClients[playerNr]->pushMessage (message.clone());
	}
	else
	{
		auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.second == playerNr; });
		if (it == clientSockets.end())
		{
			NetLog.error ("Connection Manager: Can't send message. No connection to player " + std::to_string (playerNr));
			return;
		}

		sendMessage (*it->first, message);
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::sendToPlayers (const cNetMessage& message)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (localPlayer != -1)
	{
		localClient->pushMessage (message.clone());
	}
	for (auto& client : localClients)
	{
		client->pushMessage (message.clone());
	}

	// serialize...
	std::vector<unsigned char> buffer;
	cBinaryArchiveOut archive (buffer);
	archive << message;

	for (const auto& client : clientSockets)
	{
		network->sendMessage (*client.first, buffer.size(), buffer.data());
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::disconnect (int player)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.second == player; });
	if (it == clientSockets.end())
	{
		NetLog.error ("ConnectionManager: Can't disconnect player. No connection to player " + std::to_string (player));
		return;
	}

	network->close (*it->first);
}

//------------------------------------------------------------------------------
void cConnectionManager::disconnectAll()
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	if (serverSocket)
	{
		network->close (*serverSocket);
	}

	while (clientSockets.size() > 0)
	{
		//erease in loop
		network->close (*clientSockets[0].first);
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::connectionClosed (const cSocket& socket)
{
	stopTimeout (socket);

	if (&socket == serverSocket)
	{
		if (localClient)
		{
			localClient->pushMessage (std::make_unique<cNetMessageTcpClose> (-1));
		}
		serverSocket = nullptr;
	}
	else
	{
		auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.first == &socket; });
		if (it == clientSockets.end())
		{
			NetLog.error ("ConnectionManager: An unknown connection was closed");
			return;
		}

		int playerNr = it->second;
		if (playerNr != -1 && localServer) //is a player associated with the socket?
		{
			localServer->pushMessage (std::make_unique<cNetMessageTcpClose> (playerNr));
		}

		clientSockets.erase (it);
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::incomingConnection (const cSocket& socket)
{
	startTimeout (socket);

	clientSockets.emplace_back (&socket, -1);

	cNetMessageTcpHello message;

	nlohmann::json json;
	cJsonArchiveOut jsonarchive (json);
	jsonarchive << message;
	NetLog.debug ("ConnectionManager: --> " + json.dump (-1));

	sendMessage (socket, message);
}

//------------------------------------------------------------------------------
int cConnectionManager::sendMessage (const cSocket& socket, const cNetMessage& message)
{
	// serialize...
	std::vector<unsigned char> buffer;
	cBinaryArchiveOut archive (buffer);
	archive << message;

	return network->sendMessage (socket, buffer.size(), buffer.data());
}

//------------------------------------------------------------------------------
void cConnectionManager::messageReceived (const cSocket& socket, unsigned char* data, int length)
{
	std::unique_ptr<cNetMessage> message;
	try
	{
		message = cNetMessage::createFromBuffer (data, length);
	}
	catch (std::runtime_error& e)
	{
		NetLog.error (std::string{"ConnectionManager: Can't deserialize net message: "} + e.what());
		return;
	}

	//compare the sender playerNr of the message with the playerNr that is expected behind the socket
	int playerOnSocket = -1;
	auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.first == &socket; });
	if (it != clientSockets.end())
	{
		playerOnSocket = it->second;
		if (message->playerNr != playerOnSocket)
		{
			NetLog.warn ("ConnectionManager: Discarding message with wrong sender id");
			return;
		}
	}

	// handle messages for the connection handshake
	if (handeConnectionHandshake (message, socket, playerOnSocket))
	{
		return;
	}

	if (localServer)
	{
		localServer->pushMessage (std::move (message));
	}
	else if (localClient)
	{
		localClient->pushMessage (std::move (message));
	}
	else
	{
		NetLog.error ("ConnectionManager: Cannot handle message. No message receiver.");
	}
}

bool cConnectionManager::handeConnectionHandshake (const std::unique_ptr<cNetMessage>& message, const cSocket& socket, int playerOnSocket)
{
	switch (message->getType())
	{
		case eNetMessageType::TCP_HELLO:
		{
			nlohmann::json json;
			cJsonArchiveOut jsonarchive (json);
			jsonarchive << *message;
			NetLog.debug ("ConnectionManager: <-- " + json.dump (-1));

			if (localServer)
			{
				// server shouldn't get this message
				return true;
			}

			//check compatible game version
			const auto& msgTcpHello = static_cast<cNetMessageTcpHello&> (*message);
			if (msgTcpHello.packageVersion != PACKAGE_VERSION)
			{
				network->close (socket);
			}
			return false;
		}
		case eNetMessageType::TCP_WANT_CONNECT:
		{
			nlohmann::json json;
			cJsonArchiveOut jsonarchive (json);
			jsonarchive << *message;
			NetLog.debug ("ConnectionManager: <-- " + json.dump (-1));

			if (!localServer)
			{
				// clients shouldn't get this message
				return true;
			}

			if (playerOnSocket != -1)
			{
				NetLog.error ("ConnectionManager: Received TCP_WANT_CONNECT from already connected player");
				return true;
			}

			auto& msgTcpWantConnect = static_cast<cNetMessageTcpWantConnect&> (*message);
			msgTcpWantConnect.socket = &socket;

			//check compatible game version
			if (msgTcpWantConnect.packageVersion != PACKAGE_VERSION)
			{
				network->close (socket);
				return true;
			}
			break;
		}
		case eNetMessageType::TCP_CONNECTED:
		{
			if (localServer)
			{
				// server shouldn't get this message
				return true;
			}
			nlohmann::json json;
			cJsonArchiveOut jsonarchive (json);
			jsonarchive << *message;
			NetLog.debug ("ConnectionManager: <-- " + json.dump (-1));

			stopTimeout (socket);

			localPlayer = message->playerNr;
			break;
		}
		default:
			break;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cConnectionManager::isPlayerConnected (int playerNr) const
{
	if (playerNr == localPlayer || static_cast<std::size_t> (playerNr) < localClients.size())
	{
		return true;
	}

	auto it = ranges::find_if (clientSockets, [&] (const std::pair<const cSocket*, int>& p) { return p.second == playerNr; });
	return it != clientSockets.end();
}

//------------------------------------------------------------------------------
void cConnectionManager::connectionResult (const cSocket* socket)
{
	assert (localClient);
	if (socket != nullptr)
	{
		startTimeout (*socket);
	}

	connecting = false;
	serverSocket = socket;

	if (socket == nullptr)
	{
		NetLog.warn ("ConnectionManager: Connect to server failed");
		auto message = std::make_unique<cNetMessageTcpConnectFailed> (eDeclineConnectionReason::Other);
		localClient->pushMessage (std::move (message));
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::startTimeout ([[maybe_unused]] const cSocket& socket)
{
#if DISABLE_TIMEOUTS == 0
	timeouts.push_back (std::make_unique<cHandshakeTimeout> (&socket, this));
#endif
}

//------------------------------------------------------------------------------
void cConnectionManager::stopTimeout (const cSocket& socket)
{
	auto it = ranges::find_if (timeouts, [&] (const auto& timer) { return timer->getSocket() == &socket; });
	if (it != timeouts.end())
	{
		timeouts.erase (it);
	}
}

//------------------------------------------------------------------------------
void cConnectionManager::handshakeTimeoutCallback (cHandshakeTimeout& timer)
{
	std::unique_lock<std::recursive_mutex> tl (mutex);

	NetLog.warn ("ConnectionManager: Handshake timed out");

	auto it = ranges::find_if (timeouts, [&] (const auto& timeout) { return timeout.get() == &timer; });
	if (it != timeouts.end())
	{
		network->close (*timer.getSocket());
		timeouts.erase (it);
	}
}
