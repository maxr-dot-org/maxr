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

#ifndef game_logic_clientH
#define game_logic_clientH

#include <memory>

#include "game/logic/gametimer.h"
#include "game/connectionmanager.h"
#include "utility/thread/concurrentqueue.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "protocol/netmessage.h"
#include "game/data/model.h"

class cBuilding;
class cCasualtiesTracker;
class cAttackJob;
class cFx;
class cFxContainer;
class cJob;
class cMap;
class cPlayer;
class cStaticMap;
class cPlayerBasicData;
class cGameSettings;
class cPosition;
class cTurnCounter;
class cTurnTimeClock;
class cTurnTimeDeadline;
class cSubBase;
class cSavegame;
class cSurveyorAi;

struct sLobbyPreparationData;

Uint32 TimerCallback (Uint32 interval, void* arg);


class cClient : public INetMessageReceiver
{
	friend class cDebugOutputWidget;
public:
	cClient (std::shared_ptr<cConnectionManager> connectionManager);
	~cClient();

	const cModel& getModel() const { return model; };

	const cPlayer& getActivePlayer() const { return *activePlayer; }

	void setPreparationData (const sLobbyPreparationData&);
	void setMap(std::shared_ptr<cStaticMap> staticMap);
	void setPlayers(const std::vector<cPlayerBasicData>& splayers, size_t activePlayerNr);

	unsigned int getNetMessageQueueSize() const { return static_cast<unsigned int>(eventQueue2.safe_size()); };
	void pushMessage(std::unique_ptr<cNetMessage> message) override;

	//
	void enableFreezeMode (eFreezeMode mode);
	void disableFreezeMode (eFreezeMode mode);
	const cFreezeModes& getFreezeModes () const;
	const std::map<int, ePlayerConnectionState>& getPlayerConnectionStates() const;
	//

	void addSurveyorMoveJob (const cVehicle& vehicle);
	void removeSurveyorMoveJob (const cVehicle& vehicle);
	void recreateSurveyorMoveJobs();

	/**
	* sends a serialized copy of the netmessage to the server.
	*/
	void sendNetMessage(cNetMessage& message) const;
	void sendNetMessage(cNetMessage&& message) const;


	void handleNetMessages();

	void runClientJobs(const cModel& model);

	const std::shared_ptr<cGameTimerClient>& getGameTimer() const { return gameTimer; }

	void loadModel(int saveGameNumber, int playerNr);


	mutable cSignal<void (int fromPlayerNr, std::unique_ptr<cSavedReport>& report, int toPlayerNr)> reportMessageReceived;
	mutable cSignal<void (int savingID)> guiSaveInfoRequested;
	mutable cSignal<void (const cNetMessageGUISaveInfo& guiInfo)> guiSaveInfoReceived;
	mutable cSignal<void ()> freezeModeChanged;
	mutable cSignal<void ()> connectionToServerLost;
	mutable cSignal<void (const cVehicle& vehicle)> surveyorAiConfused;

	void run();
private:

	void handleSurveyorMoveJobs();

	cModel model;

	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cConnectionManager> connectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue2;

	std::shared_ptr<cGameTimerClient> gameTimer;

	cPlayer* activePlayer; /** the active Player */

	cFreezeModes freezeModes;
	std::map<int, ePlayerConnectionState> playerConnectionStates;

	std::vector<std::unique_ptr<cSurveyorAi>> surveyorAiJobs;
};

#endif // game_logic_clientH
