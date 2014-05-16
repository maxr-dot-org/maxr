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

#ifndef gui_control_menucontrollermultiplayerclientH
#define gui_control_menucontrollermultiplayerclientH

#include <memory>

#include "../../utility/signal/signalconnectionmanager.h"
#include "../../network.h"
#include "../../utility/runnable.h"
#include "../../utility/concurrentqueue.h"

class cApplication;
class cWindowNetworkLobbyClient;
class cWindowLandingPositionSelection;
class cNetworkClientGameNew;
class cNetMessage;

class cTCP;

class cMenuControllerMultiplayerClient : public INetMessageReceiver, public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerClient>
{
public:
	cMenuControllerMultiplayerClient (cApplication& application);

	void start ();

	virtual void pushEvent (std::unique_ptr<cNetMessage> message) MAXR_OVERRIDE_FUNCTION;

	virtual void run () MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage>> messageQueue;

	std::shared_ptr<cTCP> network;

	cApplication& application;

	std::shared_ptr<cWindowNetworkLobbyClient> windowNetworkLobby;

	std::shared_ptr<cWindowLandingPositionSelection> windowLandingPositionSelection;

	std::shared_ptr<cNetworkClientGameNew> newGame;

	std::string triedLoadMapName;
	std::string lastRequestedMapName;

	void reset ();

	void handleWantLocalPlayerReadyChange ();
	void handleChatMessageTriggered ();
	void handleLocalPlayerAttributesChanged ();

	void connect ();

	void startSavedGame ();

	void startGamePreparation ();

	void startClanSelection ();
	void startLandingUnitSelection ();
	void startLandingPositionSelection ();
	void startNewGame ();

	void handleNetMessage (cNetMessage& message);

	void handleNetMessage_TCP_CLOSE (cNetMessage& message);
	void handleNetMessage_MU_MSG_CHAT (cNetMessage& message);
	void handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (cNetMessage& message);
	void handleNetMessage_MU_MSG_PLAYER_NUMBER (cNetMessage& message);
	void handleNetMessage_MU_MSG_PLAYERLIST (cNetMessage& message);
	void handleNetMessage_MU_MSG_OPTINS (cNetMessage& message);
	void handleNetMessage_MU_MSG_GO (cNetMessage& message);
	void handleNetMessage_MU_MSG_LANDING_STATE (cNetMessage& message);
	void handleNetMessage_MU_MSG_ALL_LANDED (cNetMessage& message);
	//void initMapDownload (cNetMessage& message);
	//void receiveMapData (cNetMessage& message);
	//void canceledMapDownload (cNetMessage& message);
	//void finishedMapDownload (cNetMessage& message);
	void handleNetMessage_GAME_EV_REQ_RECON_IDENT (cNetMessage& message);
	void handleNetMessage_GAME_EV_RECONNECT_ANSWER (cNetMessage& message);
};

#endif // gui_control_menucontrollermultiplayerclientH
