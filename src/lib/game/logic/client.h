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
#include "game/logic/action/actionstartmove.h"
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

	void pushMessage (std::unique_ptr<cNetMessage>) override;

	//
	void enableFreezeMode (eFreezeMode);
	void disableFreezeMode (eFreezeMode);
	const cFreezeModes& getFreezeModes() const;
	const std::map<int, ePlayerConnectionState>& getPlayerConnectionStates() const;
	//

	void sendSyncMessage (unsigned int gameTime, bool crcOK, unsigned int timeBuffer, unsigned int ticksPerFrame, unsigned int eventCounter) const;

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
	void rearm (const cUnit& sourceUnit, const cUnit& destUnit);
	void repair (const cUnit& sourceUnit, const cUnit& destUnit);
	void changeResourceDistribution (const cBuilding&, const sMiningResource&);
	void resumeMoveJob (const cVehicle&);
	void resumeAllMoveJobs();
	void selfDestroy (const cBuilding&);
	void setAutoMove (const cVehicle&, bool);
	void startBuild (const cVehicle&, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition);
	void startBuildPath (const cVehicle&, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition, const cPosition& pathEndPosition);
	void startMove (const cVehicle&, const std::forward_list<cPosition>& path, eStart, eStopOn, cEndMoveAction);
	void startTurn();
	void startWork (const cBuilding&);
	void disable (const cVehicle& infiltrator, const cUnit& target);
	void steal (const cVehicle& infiltrator, const cUnit& target);
	void stopWork (const cUnit&);
	void transfer (const cUnit& sourceUnit, const cUnit& destinationUnit, int transferValue, eResourceType);
	void upgradeAllBuildings (const cBuilding&);
	void upgradeBuilding (const cBuilding&);
	void upgradeAllVehicles (const cBuilding& containingBuilding);
	void upgradeVehicle (const cBuilding& containingBuilding, const cVehicle&);

	void report (std::unique_ptr<cSavedReport>);

	void sendGUISaveInfo (int slot, int savingId, const sPlayerGuiInfo&, std::optional<cGameGuiState>);

	void handleNetMessages();

	void runClientJobs (const cModel&);

	const std::shared_ptr<cGameTimerClient>& getGameTimer() const { return gameTimer; }

	void loadModel (int saveGameNumber, int playerNr);

	cSignal<void (int fromPlayerNr, std::unique_ptr<cSavedReport>&, int toPlayerNr)> reportMessageReceived;
	cSignal<void (int slot, int savingID)> guiSaveInfoRequested;
	cSignal<void (const cNetMessageGUISaveInfo&)> guiSaveInfoReceived;
	cSignal<void()> freezeModeChanged;
	cSignal<void()> resynced;
	cSignal<void()> connectionToServerLost;
	cSignal<void (const cVehicle&)> surveyorAiConfused;

	void run();

private:
	/**
	* sends a serialized copy of the netmessage to the server.
	*/
	void sendNetMessage (cNetMessage&&) const;

	void handleSurveyorMoveJobs();

private:
	cModel model;
	cSignalConnectionManager signalConnectionManager;
	std::shared_ptr<cConnectionManager> connectionManager;
	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue;
	std::shared_ptr<cGameTimerClient> gameTimer;
	cPlayer* activePlayer = nullptr;
	cFreezeModes freezeModes;
	std::map<int, ePlayerConnectionState> playerConnectionStates;
	std::vector<std::unique_ptr<cSurveyorAi>> surveyorAiJobs;
};

#endif // game_logic_clientH
