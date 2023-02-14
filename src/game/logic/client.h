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

#include "game/connectionmanager.h"
#include "game/data/model.h"
#include "game/logic/gametimer.h"
#include "game/protocol/netmessage.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/concurrentqueue.h"

#include <memory>

class cAttackJob;
class cBuilding;
class cCasualtiesTracker;
class cGameSettings;
class cJob;
class cMap;
class cPlayer;
class cPlayerBasicData;
class cPosition;
class cStaticMap;
class cTurnCounter;
class cTurnTimeClock;
class cTurnTimeDeadline;
class cSavegame;
class cSubBase;
class cSurveyorAi;

struct sInitPlayerData;
struct sLobbyPreparationData;

class cClient : public INetMessageReceiver
{
	friend class cDebugOutputWidget;

public:
	cClient (std::shared_ptr<cConnectionManager>);
	~cClient();

	const cModel& getModel() const { return model; }
	const cPlayer& getActivePlayer() const { return *activePlayer; }

	void setPreparationData (const sLobbyPreparationData&);
	void setMap (std::shared_ptr<cStaticMap>);
	void setPlayers (const std::vector<cPlayerBasicData>&, size_t activePlayerNr);

	unsigned int getNetMessageQueueSize() const { return static_cast<unsigned int> (eventQueue.safe_size()); }
	void pushMessage (std::unique_ptr<cNetMessage>) override;

	//
	void enableFreezeMode (eFreezeMode);
	void disableFreezeMode (eFreezeMode);
	const cFreezeModes& getFreezeModes() const;
	const std::map<int, ePlayerConnectionState>& getPlayerConnectionStates() const;
	//

	void addSurveyorMoveJob (const cVehicle&);
	void removeSurveyorMoveJob (const cVehicle&);
	void recreateSurveyorMoveJobs();

	void activateUnit (const cUnit& containingUnit, const cVehicle& activatedVehicle, const cPosition&);
	void attack (const cUnit& aggressor, const cPosition& targetPosition, const cUnit* targetUnit);
	void buyUpgrades (const std::vector<std::pair<sID, cUnitUpgrade>>&);
	void changeBuildList (const cBuilding&, const std::vector<sID>& buildList, int buildSpeed, bool repeat);
	void changeManualFire (const cUnit&);
	void changeResearch (const std::array<int, cResearch::kNrResearchAreas>&);
	void changeSentry (const cUnit&);
	void changeUnitName (const cUnit&, const std::string&);
	void startClearRubbles (const cVehicle&);
	void endTurn();
	void finishBuild (const cUnit&, const cPosition& escapePosition);
	void initNewGame (const sInitPlayerData&);
	void load (const cUnit& loadingUnit, const cVehicle& loadedVehicle);
	void toggleLayMines (const cVehicle&);
	void toggleCollectMines (const cVehicle&);

	/**
	* sends a serialized copy of the netmessage to the server.
	*/
	void sendNetMessage (cNetMessage&) const;
	void sendNetMessage (cNetMessage&&) const;

	void handleNetMessages();

	void runClientJobs (const cModel&);

	const std::shared_ptr<cGameTimerClient>& getGameTimer() const { return gameTimer; }

	void loadModel (int saveGameNumber, int playerNr);

	cSignal<void (int fromPlayerNr, std::unique_ptr<cSavedReport>&, int toPlayerNr)> reportMessageReceived;
	cSignal<void (int slot, int savingID)> guiSaveInfoRequested;
	cSignal<void (const cNetMessageGUISaveInfo&)> guiSaveInfoReceived;
	cSignal<void()> freezeModeChanged;
	cSignal<void()> connectionToServerLost;
	cSignal<void (const cVehicle&)> surveyorAiConfused;

	void run();

private:
	void handleSurveyorMoveJobs();

private:
	cModel model;
	cSignalConnectionManager signalConnectionManager;
	std::shared_ptr<cConnectionManager> connectionManager;
	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue;
	std::shared_ptr<cGameTimerClient> gameTimer;
	cPlayer* activePlayer;
	cFreezeModes freezeModes;
	std::map<int, ePlayerConnectionState> playerConnectionStates;
	std::vector<std::unique_ptr<cSurveyorAi>> surveyorAiJobs;
};

#endif // game_logic_clientH
