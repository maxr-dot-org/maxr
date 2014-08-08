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

#include "game/startup/local/scenario/localscenariogame.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/logic/savegame.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/map/map.h"
#include "game/logic/clientevents.h"
#include "utility/files.h"
#include "utility/log.h"
#include "loaddata.h"

cLocalScenarioGame::cLocalScenarioGame()
{
    // Build the server
    m_server = std::make_unique<cServer>(nullptr);

    // Build the GUI client and player
    m_guiClient = std::make_shared<cClient>(m_server.get(), nullptr);
    cPlayerBasicData player(cSettings::getInstance().getPlayerName(), cPlayerColor(cPlayerColor::predefinedColors[0]), 0);
    player.setLocal ();
    m_server->addPlayer(std::make_unique<cPlayer>(player));
    m_players.push_back(player);
}

cLocalScenarioGame::~cLocalScenarioGame()
{
    Log.write("cLocalScenarioGame destructor", cLog::eLOG_TYPE_DEBUG);
    if (m_server)
    {
        m_server->stop ();
        reloadUnitValues ();
    }
}

void cLocalScenarioGame::run()
{
    if (m_guiClient) m_guiClient->getGameTimer()->run();
    for (size_t i = 0; i != m_iaClients.size(); ++i) {
        m_iaClients[i]->getGameTimer()->run();
    }
}

void cLocalScenarioGame::save(int saveNumber, const std::string &saveName)
{
    if (!m_server) throw std::runtime_error ("Game not started!"); // should never happen (hence a translation is not necessary).

    cSavegame savegame(saveNumber);
    savegame.save(*m_server, saveName);
    m_server->makeAdditionalSaveRequest(saveNumber);
}

bool cLocalScenarioGame::loadMap(std::string mapName)
{
    if (!m_map) m_map = std::make_shared<cStaticMap>();
    if (!m_map->loadMap(mapName)) {
        Log.write ("couldn't load map: " + mapName, cLog::eLOG_TYPE_ERROR);
        return false;
    }
    else {
        if (m_server) m_server->setMap(m_map);
        if (m_guiClient) m_guiClient->setMap(m_map);
        for (size_t i = 0; i != m_iaClients.size(); ++i) {
            m_iaClients[i]->setMap(m_map);
        }
    }
    return true;
}

void cLocalScenarioGame::addPlayer(std::string playerName)
{
    // Create client and player
    std::shared_ptr<cClient> client = std::make_shared<cClient>(m_server.get(), nullptr);
    if (m_map) client->setMap(m_map);
    cPlayerBasicData ia_player(playerName, cPlayerColor(cPlayerColor::predefinedColors[m_players.size()]), m_players.size());
    ia_player.setLocal();
    m_server->addPlayer (std::make_unique<cPlayer>(ia_player));
    m_players.push_back(ia_player);
    m_iaClients.push_back(std::move(client));
}

void cLocalScenarioGame::addUnit(const sID &id, const std::string &playerName, const cPosition &pos)
{
    m_server->addVehicle(pos, id, m_server->getPlayerFromString(playerName), false);
    // FUTURE: return vehicle reference into LuaVehicle class for Lua being able to move, attack etc.
}

void cLocalScenarioGame::addBuilding(const sID &id, const std::string &playerName, const cPosition &pos)
{
    m_server->addBuilding(pos, id, m_server->getPlayerFromString(playerName), false);
}

void cLocalScenarioGame::setPlayerClan(std::string playerName, int clan)
{
    Log.write (playerName + " set clan : " + std::to_string(clan), cLog::eLOG_TYPE_DEBUG);
    m_server->getPlayerFromString(playerName)->setClan(clan);
    // BUG_M: clan does not seems to work...
    // They work but only on units added after clan is set ! Not on landing units...
}

void cLocalScenarioGame::startServer()
{
    if (!m_map) {
        Log.write ("Scenario game error: no map loaded !", cLog::eLOG_TYPE_ERROR);
        return;
    }

    // Load default settings if no settings have been loaded by the script
    if (!m_gameSettings) {
        m_gameSettings = std::make_shared<cGameSettings>();
        m_gameSettings->setStartCredits(0);
        m_gameSettings->setClansEnabled(true);
        m_gameSettings->setBridgeheadType(eGameSettingsBridgeheadType::Mobile);      // Mobile will remove the initial mining station building
    }
    m_server->setGameSettings(*m_gameSettings);
    m_guiClient->setGameSettings(*m_gameSettings);
    for (size_t i = 0; i != m_iaClients.size(); ++i) m_iaClients[i]->setGameSettings(*m_gameSettings);

    // Set player list to the clients
    m_guiClient->setPlayers(m_players, 0);
    for (size_t i = 0; i != m_iaClients.size(); ++i) m_iaClients[i]->setPlayers(m_players, i+1);

    // Start server and send data to clients
    m_server->start();
}

void cLocalScenarioGame::startGame(cApplication &application)
{
    // Everybody is Ready, let's go !
    m_server->startTurnTimers();

    // Show GUI
    gameGuiController = std::make_unique<cGameGuiController> (application, m_map);
    gameGuiController->setSingleClient (m_guiClient);

    cGameGuiState playerGameGuiState;
    const auto& player = m_guiClient->getPlayerList()[0];
    playerGameGuiState.setMapPosition (m_guiPosition);
    gameGuiController->addPlayerGameGuiState (*player, std::move (playerGameGuiState));

    gameGuiController->start ();

    using namespace std::placeholders;
    m_signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cLocalScenarioGame::save, this, _1, _2));

    terminate = false;

    application.addRunnable (shared_from_this ());

    m_signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

const cClient &cLocalScenarioGame::getClient(int index)
{
    if (index == 0) return *m_guiClient;
    else return *m_iaClients[index - 1];
}

