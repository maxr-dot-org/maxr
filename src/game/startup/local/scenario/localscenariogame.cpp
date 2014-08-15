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
#include "ui/graphical/menu/dialogs/dialogscenarioend.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/application.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/map/map.h"
#include "game/logic/clientevents.h"
#include "game/logic/turnclock.h"
#include "utility/files.h"
#include "utility/log.h"
#include "loaddata.h"

#include "lua/lua.hpp"
#include "game/startup/local/scenario/luaposition.h"
#include "game/startup/local/scenario/luagame.h"
#include "game/startup/local/scenario/luaplayer.h"
#include "game/startup/local/scenario/luasettings.h"

cLocalScenarioGame::cLocalScenarioGame(cApplication* application) :
    m_application(application),
    m_startStatus(Ready),
    m_scenarioFinished(false)
{
    // Build the server
    m_server = std::make_unique<cServer>(nullptr);

    // Build the GUI client and player
    m_guiClient = std::make_shared<cClient>(m_server.get(), nullptr);
    cPlayerBasicData player(cSettings::getInstance().getPlayerName(), cPlayerColor(cPlayerColor::predefinedColors[0]), 0);
    player.setLocal ();
    m_server->addPlayer(std::make_unique<cPlayer>(player));
    m_players.push_back(player);

    // Connect to the new turn signal
    m_signalConnectionManager.connect(m_server->getTurnClock()->turnChanged, std::bind (&cLocalScenarioGame::turnChanged, this));
}

cLocalScenarioGame::~cLocalScenarioGame()
{
    Log.write("cLocalScenarioGame destructor", cLog::eLOG_TYPE_DEBUG);
    if (m_server)
    {
        m_server->stop ();
        reloadUnitValues ();
    }

    // Close Lua context
    lua_close(L);
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

void cLocalScenarioGame::loadLuaScript(std::string luaFilename)
{
    // Open Lua File
    std::string fullFilename = cSettings::getInstance().getScenariosPath() + PATH_DELIMITER + luaFilename;
    SDL_RWops* fpLuaFile = SDL_RWFromFile(fullFilename.c_str(), "rb");
    if (fpLuaFile == NULL)
    {
        // now try in the user's map directory
        std::string userMapsDir = getUserMapsDir();
        if (!userMapsDir.empty())
        {
            fullFilename = userMapsDir + luaFilename;
            fpLuaFile = SDL_RWFromFile (fullFilename.c_str(), "rb");
        }
    }
    if (fpLuaFile == NULL)
    {
        Log.write("Cannot load scenario file: \"" + luaFilename + "\"", cLog::eLOG_TYPE_ERROR);
        return;
    }
    Sint64 fileSize = SDL_RWsize(fpLuaFile);
    char* luaData = new char[fileSize];
    SDL_RWread(fpLuaFile, luaData, 1, fileSize);
    SDL_RWclose (fpLuaFile);

    // Create a lua context and run Lua interpreter
    L = luaL_newstate();
    luaL_openlibs(L);
    Lunar<LuaPosition>::Register(L);
    Lunar<LuaGame>::Register(L);
    Lunar<LuaPlayer>::Register(L);
    Lunar<LuaSettings>::Register(L);

    // Create the game in a global variable "game"
    m_luaGame = std::make_shared<LuaGame>(this);
    Lunar<LuaGame>::push(L, m_luaGame.get());               // luaGame has to be deleted from C++
    lua_setglobal(L, "game");

    // Load the Lua script
    int error = luaL_loadbuffer(L, luaData, fileSize, "luaScenario") || lua_pcall(L, 0, 0, 0);
    if (error) {
        Log.write("Cannot run scenario file, check lua syntax: \"" + luaFilename + "\"" +
                  "\nLua Error: \n" + lua_tostring(L, -1), cLog::eLOG_TYPE_ERROR);
        lua_pop(L, 1);
    }
    else {
        // Load scenario name from global variable of the lua script
        std::string scenarioName = "Unknown";
        lua_getglobal (L, "scenarioName");                              // Push global scenarioName var on the stack
        if (lua_isstring(L, 1)) scenarioName = lua_tostring(L, 1);      // Retreive the value
        lua_pop(L, 1);                                                  // Pop the value from the stack
        Log.write("Scenario loaded successfully : " + scenarioName, cLog::eLOG_TYPE_INFO);
    }
    delete[] luaData;

    // Add this as a runnable to application
    terminate = false;
    m_application->addRunnable (shared_from_this ());
}

void cLocalScenarioGame::openClanWindow()
{
    m_startStatus = WaitingHuman;
    m_clanWindow = std::make_shared<cWindowClanSelection>();
    m_application->show (m_clanWindow);

    // Connection to further code: but the functions returns immediately
    m_clanWindow->done.connect ([&]()
    {
        // We go here ONLY after the user clicked the ok button
        m_luaGame->setHumanClan(m_clanWindow->getSelectedClan());
        m_startStatus = Ready;
        m_clanWindow->close ();
        m_luaGame->gameReady();
    });
    m_clanWindow->canceled.connect ([&]()
    {
        m_startStatus = Cancelled;
        m_clanWindow->close ();
    });

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

void cLocalScenarioGame::startGame()
{
    // Everybody is Ready, let's go !
    m_server->startTurnTimers();

    // Show GUI
    m_gameGuiController = std::make_unique<cGameGuiController> (*m_application, m_map);
    m_gameGuiController->setSingleClient (m_guiClient);

    cGameGuiState playerGameGuiState;
    const auto& player = m_guiClient->getPlayerList()[0];
    playerGameGuiState.setMapPosition (m_guiPosition);
    m_gameGuiController->addPlayerGameGuiState (*player, std::move (playerGameGuiState));

    m_gameGuiController->start ();

    using namespace std::placeholders;
    m_signalConnectionManager.connect (m_gameGuiController->triggeredSave, std::bind (&cLocalScenarioGame::save, this, _1, _2));
    m_signalConnectionManager.connect (m_gameGuiController->terminated, [&]() { terminate = true; });
}

void cLocalScenarioGame::exit()
{
    m_gameGuiController->exit();
}

const cClient &cLocalScenarioGame::getClient(int index)
{
    if (index == 0) return *m_guiClient;
    else return *m_iaClients[index - 1];
}

void cLocalScenarioGame::turnChanged()
{
    if (m_scenarioFinished) return;

    Log.write("New turn begin, will call scenario victory condition script : " + std::to_string(m_server->getTurnClock()->getTurn()), cLog::eLOG_TYPE_DEBUG);

    // Add turn count as a global variable
    lua_pushinteger(L, m_server->getTurnClock()->getTurn());
    lua_setglobal(L, "turnCount");

    /* TODO_M: list of other datas that could be added to the context for the victory condition
     * List vehicles of each player, their position and caracteristics, if disabled
     * List of buildings of each player, their position and caracteristics, if disabled
     * Ecosphere count and score of players
     * Research state of players, player credits
     * Casualties: les pertes de chaque joueur, afin de savoir combien d'unités chacun s'est fait dégommé, on pourrait demander plus de dommage AI que humain au tour 50 par ex.
     * /


    // Call Lua function "victoryCondition" from scenario; return value
    // -1 defeat, 0 continue, 1 victory
    lua_getglobal(L, "victoryCondition");
    if (lua_pcall(L, 0, 1, 0) != 0) {
        Log.write("Error calling victoryCondition : " + std::string(lua_tostring(L, -1)), cLog::eLOG_TYPE_ERROR);
        return;
    }

    // Get result value
    if (!lua_isnumber(L, -1)) Log.write("Error victoryCondition should return a number: ", cLog::eLOG_TYPE_ERROR);
    else {
        int v = (int)lua_tonumber(L, -1);
        lua_pop(L, 1);
        Log.write("Victory condition returned : " + std::to_string(v), cLog::eLOG_TYPE_DEBUG);

        // Scenario end !
        if (v != 0) {
            // Avoid to display the dialog endlessly in case of user want to continue the game
            m_scenarioFinished = true;

            // Option to either return to game (endless play) or exit scenario.
            auto scenarioDialog = m_application->show (std::make_shared<cDialogScenarioEnd> (v > 0));
            m_signalConnectionManager.connect (scenarioDialog->exitClicked, [&]()
            {
                m_gameGuiController->exit ();
            });
        }
    }
}

