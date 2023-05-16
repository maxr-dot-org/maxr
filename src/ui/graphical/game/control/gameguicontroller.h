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

#ifndef ui_graphical_game_control_gameguicontrollerH
#define ui_graphical_game_control_gameguicontrollerH

#include "game/data/gui/gameguistate.h"
#include "game/data/gui/playerguiinfo.h"
#include "game/logic/upgradecalculator.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class cAnimationTimer;
class cBuildListItem;
class cBuilding;
class cApplication;
class cCasualtiesTracker;
class cChatCommandExecutor;
class cClient;
class cFreezeModes;
class cGameGui;
class cGameSettings;
class cKeySequence;
class cMapView;
class cPlayer;
class cSavedReport;
class cServer;
class cSoundManager;
class cStaticMap;
class cTurnCounter;
class cTurnTimeClock;
class cUnit;
class cUnitUpgrade;
class cUnitsData;
class cVehicle;
class cWindowUpgradesFilterState;

enum class ePlayerConnectionState;
enum class eResourceType;
enum class eStart;

struct sID;
struct sMiningResource;

class cGameGuiController
{
public:
	cGameGuiController (cApplication&, std::shared_ptr<const cStaticMap>);
	~cGameGuiController();

	cGameGuiController (const cGameGuiController&) = delete;
	cGameGuiController& operator= (const cGameGuiController&) = delete;

	void start();

	void addPlayerGameGuiState (int playerNr, cGameGuiState playerGameGuiState);

	void addSavedReport (std::unique_ptr<cSavedReport>, int playerNr);
	const std::vector<std::unique_ptr<cSavedReport>>& getSavedReports (int playerNr) const;

	void setSingleClient (std::shared_ptr<cClient>);
	void setClients (std::vector<std::shared_ptr<cClient>>, int activePlayerNumber);
	void setServer (cServer*);

	mutable cSignal<void()> terminated;

private:
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager guiSignalConnectionManager;
	cSignalConnectionManager clientSignalConnectionManager;
	cSignalConnectionManager allClientsSignalConnectionManager;
	cSignalConnectionManager buildPositionSelectionConnectionManager;

	cApplication& application;

	std::shared_ptr<cSoundManager> soundManager;
	std::shared_ptr<cAnimationTimer> animationTimer;

	std::shared_ptr<cGameGui> gameGui;
	std::shared_ptr<cClient> activeClient;
	std::vector<std::shared_ptr<cClient>> clients;
	std::shared_ptr<const cMapView> mapView;
	cServer* server = nullptr;

	std::vector<std::unique_ptr<cChatCommandExecutor>> chatCommands;

	std::map<int, sPlayerGuiInfo> playerGameGuiStates;

	std::optional<cPosition> savedReportPosition;
	std::shared_ptr<cWindowUpgradesFilterState> upgradesFilterState;

	template <typename Action>
	void addShortcut (cKeySequence, Action);
	void initShortcuts();
	void initChatCommands();

	void connectGuiStaticCommands();

	void setActiveClient (std::shared_ptr<cClient>);

	void connectClient (cClient&);
	void connectReportSources (cClient&);

	void showNextPlayerDialog();

	void showFilesWindow();
	void showPreferencesDialog();
	void showReportsWindow();

	void showUnitHelpWindow (const cUnit&);
	void showUnitTransferDialog (const cUnit& sourceUnit, const cUnit& destinationUnit);
	void showBuildBuildingsWindow (const cVehicle&);
	void showBuildVehiclesWindow (const cBuilding&);
	void showResourceDistributionDialog (const cUnit&);
	void showResearchDialog (const cUnit&);
	void showUpgradesWindow (const cUnit&);
	void showStorageWindow (const cUnit&);
	void showSelfDestroyDialog (const cBuilding&);

	void handleChatCommand (const std::string& command);

	void handleReportForActivePlayer (const cSavedReport&);

	void selectNextUnit();
	void selectPreviousUnit();
	void markSelectedUnitAsDone();

	void centerSelectedUnit();

	void savePosition (size_t index);
	void jumpToSavedPosition (size_t index);

	void sendStartGroupMoveAction (std::vector<cVehicle*> group, const cPosition& destination, eStart);

	void updateChangeAllowed();
	void updateEndButtonState();
	void updateGuiInfoTexts();

	std::vector<std::shared_ptr<const cPlayer>> getPlayers() const;
	std::shared_ptr<const cPlayer> getActivePlayer() const;
	std::shared_ptr<const cTurnCounter> getTurnCounter() const;
	std::shared_ptr<const cTurnTimeClock> getTurnTimeClock() const;
	std::shared_ptr<const cGameSettings> getGameSettings() const;
	std::shared_ptr<const cUnitsData> getUnitsData() const;

	mutable cSignal<void (const cUnit&, const cUnit&, int, eResourceType)> transferTriggered;
	mutable cSignal<void (const cVehicle&, const cPosition&, const sID&, int)> buildBuildingTriggered;
	mutable cSignal<void (const cVehicle&, const cPosition&, const sID&, int)> buildBuildingPathTriggered;
	mutable cSignal<void (const cBuilding&, const std::vector<sID>&, int, bool)> buildVehiclesTriggered;
	mutable cSignal<void (const cUnit& unit, size_t index, const cPosition&)> activateAtTriggered;
	mutable cSignal<void (const cUnit&, const cUnit&)> reloadTriggered;
	mutable cSignal<void (const cUnit&, const cUnit&)> repairTriggered;
	mutable cSignal<void (const cBuilding&, size_t index)> upgradeTriggered;
	mutable cSignal<void (const cBuilding&)> upgradeAllTriggered;
	mutable cSignal<void (const cBuilding&, const sMiningResource&)> changeResourceDistributionTriggered;
	mutable cSignal<void (const std::array<int, cResearch::kNrResearchAreas>&)> changeResearchSettingsTriggered;
	mutable cSignal<void (const std::vector<std::pair<sID, cUnitUpgrade>>&)> takeUnitUpgradesTriggered;
	mutable cSignal<void (const cBuilding&)> selfDestructionTriggered;
	mutable cSignal<void (const cVehicle& vehicle)> resumeMoveJobTriggered;
	mutable cSignal<void()> resumeAllMoveJobsTriggered;
};

#endif // ui_graphical_game_control_gameguicontrollerH
