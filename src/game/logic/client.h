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

#include "SDL_flic.h"
#include "defines.h"
#include "game/logic/jobs.h"
#include "game/logic/gametimer.h"
#include "main.h"
#include "connectionmanager.h"
#include "game/data/units/unit.h" // sUnitLess
#include "utility/thread/concurrentqueue.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/flatset.h"
#include "netmessage2.h"
#include "game/data/model.h"

class cBuilding;
class cCasualtiesTracker;
class cAttackJob;
class cClientMoveJob;
class cAutoMJob;
class cFx;
class cFxContainer;
class cJob;
class cMap;
class cNetMessage;
class cPlayer;
class cStaticMap;
class cPlayerBasicData;
class cGameSettings;
class cPosition;
class cTurnClock;
class cTurnTimeClock;
class cTurnTimeDeadline;
class cGameGuiState;
class cSubBase;
class cSavegame;

Uint32 TimerCallback (Uint32 interval, void* arg);


class cClient : public INetMessageReceiver
{
	friend class cDebugOutputWidget;
	friend class cPlayer;
public:
	cClient (std::shared_ptr<cConnectionManager> connectionManager);
	~cClient();

	const cModel& getModel() const { return model; };

	const cPlayer& getActivePlayer() const { return *activePlayer; }
	void setActivePlayer(cPlayer* player) { activePlayer = player; }

	void setUnitsData(std::shared_ptr<const cUnitsData> unitsData);
	void setGameSettings(const cGameSettings& gameSettings);
	void setMap(std::shared_ptr<cStaticMap> staticMap);
	void setPlayers(const std::vector<cPlayerBasicData>& splayers, size_t activePlayerNr);

	unsigned int getNetMessageQueueSize() const { return static_cast<unsigned int>(eventQueue.safe_size()); };
	virtual void pushMessage(std::unique_ptr<cNetMessage2> message) MAXR_OVERRIDE_FUNCTION;

	void enableFreezeMode (eFreezeMode mode, int playerNumber = -1);
	void disableFreezeMode (eFreezeMode mode);
	bool isFreezed() const;
	int getFreezeInfoPlayerNumber() const { return freezeModes.getPlayerNumber(); }
	bool getFreezeMode (eFreezeMode mode) const;

	/**
	* handles the end of a turn
	*@author alzi alias DoctorDeath
	*/
	void handleEnd();

	void addJob (cJob* job);

	/**
	* handles the game relevant actions
	* (for example moving the current position of a rocket)
	* of the fx-effects,
	* so that they are handled also, when the effects are not drawn.
	*/
	void runFx();

	/**
	* creates a new moveJob and transmits it to the server
	* @param vehicle the vehicle to be moved
	* @param iDestOffset the Destination
	*/
	bool addMoveJob (cVehicle& vehicle, const cPosition& destination, const std::vector<cVehicle*>* group = nullptr);
	void startGroupMove (const std::vector<cVehicle*>& group_, const cPosition& mainDestination);
	/**
	* adds a new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob (cClientMoveJob& MJob);

	void addAutoMoveJob (std::weak_ptr<cAutoMJob> autoMoveJob);

	/**
	* sends the netMessage to the server.
	*@author Eiko
	*@param message The netMessage to be send.
	*/
	void sendNetMessage (std::unique_ptr<cNetMessage> message) const;
	/**
	* sends a serialized copy of the netmessage to the server.
	*/
	void sendNetMessage(cNetMessage2& message) const;

	/**
	* gets the vehicle with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the vehicle
	*/
	cVehicle* getVehicleFromID (unsigned int id) const;
	cBuilding* getBuildingFromID (unsigned int id) const;
	cUnit* getUnitFromID (unsigned int id) const;

	/**
	* handles move and attack jobs
	* this function should be called in all menu loops
	*/
	void doGameActions();

	void handleNetMessages();

	/**
	* processes everything that is need for this netMessage
	*@author alzi alias DoctorDeath
	*@param message The netMessage to be handled.
	*@return 0 for success
	*/
	int handleNetMessage (cNetMessage& message);

	void addFx (std::shared_ptr<cFx> fx, bool playSound = true);

	/**
	* destroys a unit
	* play FX, add rubble and delete Unit
	*/
	void addDestroyFx (cVehicle& vehicle);
	void addDestroyFx (cBuilding& building);

	void deletePlayer (cPlayer& player);

	const std::shared_ptr<cCasualtiesTracker>& getCasualtiesTracker() { return casualtiesTracker; }
	std::shared_ptr<const cCasualtiesTracker> getCasualtiesTracker() const { return casualtiesTracker; }

	std::shared_ptr<const cTurnClock> getTurnClock() const { return turnClock; }
	std::shared_ptr<const cTurnTimeClock> getTurnTimeClock() const { return turnTimeClock; }


	const std::shared_ptr<cGameTimerClient>& getGameTimer() const { return gameTimer; }

	void loadModel(int saveGameNumber);


	mutable cSignal<void (int fromPlayerNr, std::unique_ptr<cSavedReport>& report, int toPlayerNr)> reportMessageReceived;
	mutable cSignal<void (int savingID)> guiSaveInfoRequested;
	mutable cSignal<void(const cNetMessageGUISaveInfo& guiInfo)> guiSaveInfoReceived;

	//TODO: move signals to model
	mutable cSignal<void (int, int)> playerFinishedTurn;

	mutable cSignal<void (eFreezeMode)> freezeModeChanged;

	mutable cSignal<void (const cUnit& storingUnit, const cUnit& storedUnit)> unitStored;
	mutable cSignal<void (const cUnit& storingUnit, const cUnit&storedUnit)> unitActivated;

	mutable cSignal<void (const cUnit&)> unitHasStolenSuccessfully;
	mutable cSignal<void (const cUnit&)> unitHasDisabledSuccessfully;
	mutable cSignal<void (const cUnit&)> unitStealDisableFailed;

	mutable cSignal<void (const cUnit&)> unitSuppliedWithAmmo;
	mutable cSignal<void (const cUnit&)> unitRepaired;

	mutable cSignal<void (const cUnit&)> unitDisabled;
	mutable cSignal<void (const cUnit&)> unitStolen; //TODO: was in addUnit()

	mutable cSignal<void (const cUnit&)> unitDetected; //TODO: was in addUnit()

	mutable cSignal<void (const cVehicle&)> moveJobBlocked;

	mutable cSignal<void (const std::shared_ptr<cFx>&, bool)> addedEffect;

	void run();
private:

	/**
	* adds the unit to the map and player.
	*@author alzi alias DoctorDeath
	*@param iPosX The X were the unit should be added.
	*@param iPosY The Y were the unit should be added.
	*@param addedVehicle Vehicle which should be added.
	*@param addedBuilding Building which should be added.
	*/
	void addUnit (const cPosition& position, cVehicle& addedVehicle, bool addToMap = true);
	void addUnit (const cPosition& position, cBuilding& addedBuilding);

	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs();

	void handleAutoMoveJobs();

	/**
	* gets the subbase with the id
	*@author alzi alias DoctorDeath
	*@param iID Id of the subbase
	*/
	cSubBase* getSubBaseFromID (int iID);

	void runJobs();

	void HandleNetMessage_TCP_CLOSE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_PLAYER_CLANS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_ENEM_BUILDING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_ENEM_VEHICLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_WAIT_FOR (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MAKE_TURNEND (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FINISHED_TURN (cNetMessage& message);
	void HandleNetMessage_GAME_EV_TURN_START_TIME (cNetMessage& message);
	void HandleNetMessage_GAME_EV_TURN_END_DEADLINE_START_TIME (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNIT_DATA (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SPECIFIC_UNIT_DATA (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MOVE_JOB_SERVER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NEXT_MOVE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RESOURCES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_BUILD_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STOP_BUILD (cNetMessage& message);
	void HandleNetMessage_GAME_EV_BUILDLIST (cNetMessage& message);
	void HandleNetMessage_GAME_EV_MARK_LOG (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SUPPLY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_ADD_RUBBLE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DETECTION_STATE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CLEAR_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STOP_CLEARING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NOFOG (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEFEATED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FREEZE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNFREEZE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DEL_PLAYER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_TURN (cNetMessage& message);
	void HandleNetMessage_GAME_EV_STORE_UNIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_EXIT_UNIT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_DELETE_EVERYTHING (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNIT_UPGRADE_VALUES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_CREDITS_CHANGED (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UPGRADED_BUILDINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UPGRADED_VEHICLES (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RESEARCH_SETTINGS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_RESEARCH_LEVEL (cNetMessage& message);
	void HandleNetMessage_GAME_EV_FINISHED_RESEARCH_AREAS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_REFRESH_RESEARCH_COUNT (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SET_AUTOMOVE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_COMMANDO_ANSWER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SCORE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_NUM_ECOS (cNetMessage& message);
	void HandleNetMessage_GAME_EV_UNIT_SCORE (cNetMessage& message);
	void HandleNetMessage_GAME_EV_SELFDESTROY (cNetMessage& message);
	void HandleNetMessage_GAME_EV_END_MOVE_ACTION_SERVER (cNetMessage& message);
	void HandleNetMessage_GAME_EV_REVEAL_MAP (cNetMessage& message);
private:
	cModel model;

	cSignalConnectionManager signalConnectionManager;

	cServer* server;
	std::shared_ptr<cConnectionManager> connectionManager;

	cConcurrentQueue<std::unique_ptr<cNetMessage>> eventQueue;
	cConcurrentQueue<std::unique_ptr<cNetMessage2>> eventQueue2;

	std::shared_ptr<cGameTimerClient> gameTimer;

	/** the active Player */
	cPlayer* activePlayer;

	cJobContainer helperJobs; //TODO: move to cModel

	/** list with buildings without owner, e. g. rubble fields */
	cFlatSet<std::shared_ptr<cBuilding>, sUnitLess<cBuilding>> neutralBuildings; //TODO: move to cModel
	/** true if the player has been defeated */
	bool bDefeated;                                          //TODO: move to cPlayer

	std::shared_ptr<cTurnClock> turnClock;                   //TODO: move to cModel
	std::shared_ptr<cTurnTimeClock> turnTimeClock;           //TODO: move to cModel
	std::shared_ptr<cTurnTimeDeadline> turnLimitDeadline;    //TODO: move to cModel
	std::shared_ptr<cTurnTimeDeadline> turnEndDeadline;      //TODO: move to cModel

	std::shared_ptr<cCasualtiesTracker> casualtiesTracker; //TODO: move to cModel

	sFreezeModes freezeModes;

	/** lists with all FX-Animation */
	std::unique_ptr<cFxContainer> effectsList; //TODO: move to cModel

	std::list<std::weak_ptr<cAutoMJob>> autoMoveJobs; //TODO: move to cModel
public:
	/** list with the running clientAttackJobs */
	std::vector<cAttackJob*> attackJobs; //TODO: move to cModel
	/** List with all active movejobs */
	std::vector<cClientMoveJob*> ActiveMJobs; //TODO: move to cModel
};

#endif // game_logic_clientH
