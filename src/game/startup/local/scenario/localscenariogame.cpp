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
#include "ui/graphical/menu/dialogs/dialogok.h"
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
#include "game/startup/local/scenario/luaintelligence.h"

cLocalScenarioGame::cLocalScenarioGame(cApplication* application) :
    application(application),
    L(00),
    startStatus(Ready),
    scenarioFinished(false)
{
    // Build the server
    server = std::make_unique<cServer>(nullptr);

    // Build the GUI client and player
    guiClient = std::make_shared<cClient>(server.get(), nullptr);
    cPlayerBasicData player(cSettings::getInstance().getPlayerName(), cPlayerColor(cPlayerColor::predefinedColors[0]), 0);
    player.setLocal ();
    server->addPlayer(std::make_unique<cPlayer>(player));
    players.push_back(player);

    // Connect to the new turn signal
    signalConnectionManager.connect(server->getTurnClock()->turnChanged, std::bind (&cLocalScenarioGame::turnChanged, this));
}

cLocalScenarioGame::~cLocalScenarioGame()
{
    Log.write("cLocalScenarioGame destructor", cLog::eLOG_TYPE_DEBUG);
    if (server)
    {
        server->stop ();
        reloadUnitValues ();
    }

    // Close Lua context
    if (L) lua_close(L);
}

void cLocalScenarioGame::run()
{
    if (guiClient) guiClient->getGameTimer()->run();
    for (size_t i = 0; i != iaClients.size(); ++i) {
        iaClients[i]->getGameTimer()->run();
    }
}

void cLocalScenarioGame::save(int saveNumber, const std::string &saveName)
{
    if (!server) throw std::runtime_error ("Game not started!"); // should never happen (hence a translation is not necessary).

    cSavegame savegame(saveNumber);
    savegame.save(*server, saveName);
    server->makeAdditionalSaveRequest(saveNumber);
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
    char* luaData = new char[(size_t)fileSize];
    SDL_RWread(fpLuaFile, luaData, 1, (size_t)fileSize);
    SDL_RWclose (fpLuaFile);

    // Create a lua context and run Lua interpreter
    L = luaL_newstate();
    luaL_openlibs(L);
    Lunar<cLuaGame>::Register(L);
    Lunar<cLuaPlayer>::Register(L);
    Lunar<cLuaSettings>::Register(L);

    // Create the game in a global variable "game"
    luaGame = std::make_shared<cLuaGame>(this);
    Lunar<cLuaGame>::push(L, luaGame.get());               // luaGame has to be deleted from C++
    lua_setglobal(L, "game");

    // Load the Lua script
    int error = luaL_loadbuffer(L, luaData, (size_t)fileSize, "luaScenario") || lua_pcall(L, 0, 0, 0);
    if (error) {
        popupMessage("Cannot run scenario file, check lua syntax: \"" + luaFilename + "\"" +
                     "\nLua Error: \n" + lua_tostring(L, -1));
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
    application->addRunnable (shared_from_this ());
}

void cLocalScenarioGame::openClanWindow()
{
    startStatus = WaitingHuman;
    clanWindow = std::make_shared<cWindowClanSelection>();
    application->show (clanWindow);

    // Connection to further code: but the functions returns immediately
    clanWindow->done.connect ([&]()
    {
        // We go here ONLY after the user clicked the ok button
        luaGame->setHumanClan(clanWindow->getSelectedClan());
        startStatus = Ready;
        clanWindow->close ();
        luaGame->gameReady();
    });
    clanWindow->canceled.connect ([&]()
    {
        startStatus = Cancelled;
        clanWindow->close ();
    });

}

bool cLocalScenarioGame::loadMap(std::string mapName)
{
    if (!map) map = std::make_shared<cStaticMap>();
    if (!map->loadMap(mapName)) {
        Log.write ("couldn't load map: " + mapName, cLog::eLOG_TYPE_ERROR);
        return false;
    }
    else {
        if (server) server->setMap(map);
        if (guiClient) guiClient->setMap(map);
        for (size_t i = 0; i != iaClients.size(); ++i) {
            iaClients[i]->setMap(map);
        }
    }
    return true;
}

cPlayer *cLocalScenarioGame::humanPlayer()
{
    return server->getPlayerFromString(players[0].getName());
}

cPlayer* cLocalScenarioGame::addPlayer(std::string playerName)
{
    // Create client and player
    std::shared_ptr<cClient> client = std::make_shared<cClient>(server.get(), nullptr);
    if (map) client->setMap(map);
    cPlayerBasicData ia_player(playerName, cPlayerColor(cPlayerColor::predefinedColors[players.size()]), players.size());
    ia_player.setLocal();
    server->addPlayer (std::make_unique<cPlayer>(ia_player));
    players.push_back(ia_player);
    iaClients.push_back(std::move(client));
    return server->getPlayerFromString(playerName);
}

unsigned int cLocalScenarioGame::addUnit(const sID &id, const std::string &playerName, const cPosition &pos)
{
    cVehicle& v = server->addVehicle(pos, id, server->getPlayerFromString(playerName), false);
    return v.iID;
}

unsigned int cLocalScenarioGame::addBuilding(const sID &id, const std::string &playerName, const cPosition &pos)
{
    cBuilding& b = server->addBuilding(pos, id, server->getPlayerFromString(playerName), false);
    return b.iID;
}

void cLocalScenarioGame::setPlayerClan(std::string playerName, int clan)
{
    Log.write (playerName + " set clan : " + std::to_string(clan), cLog::eLOG_TYPE_DEBUG);
    server->getPlayerFromString(playerName)->setClan(clan);
    // BUG_M: clan does not seems to work...
    // They work but only on units added after clan is set ! Not on landing units... maybe those need to be updated manually ?
}

void cLocalScenarioGame::startServer()
{
    if (!map) {
        Log.write ("Scenario game error: no map loaded !", cLog::eLOG_TYPE_ERROR);
        return;
    }

    // Load default settings if no settings have been loaded by the script
    if (!gameSettings) {
        gameSettings = std::make_shared<cGameSettings>();
        gameSettings->setStartCredits(0);
        gameSettings->setClansEnabled(true);
        gameSettings->setBridgeheadType(eGameSettingsBridgeheadType::Mobile);      // Mobile will remove the initial mining station building
    }
    server->setGameSettings(*gameSettings);
    guiClient->setGameSettings(*gameSettings);
    for (size_t i = 0; i != iaClients.size(); ++i) iaClients[i]->setGameSettings(*gameSettings);

    // Set player list to the clients
    guiClient->setPlayers(players, 0);
    for (size_t i = 0; i != iaClients.size(); ++i) iaClients[i]->setPlayers(players, i+1);

    // Start server and send data to clients
    server->start();
}

void cLocalScenarioGame::startGame()
{
    // Everybody is Ready, let's go !
    server->startTurnTimers();

    // Show GUI
    gameGuiController = std::make_unique<cGameGuiController> (*application, map);
    gameGuiController->setSingleClient (guiClient);

    cGameGuiState playerGameGuiState;
    const auto& player = guiClient->getPlayerList()[0];
    playerGameGuiState.setMapPosition (guiPosition);
    gameGuiController->addPlayerGameGuiState (*player, std::move (playerGameGuiState));

    gameGuiController->start ();
    if (aiErrMsg.size() > 0) popupMessage(aiErrMsg);

    using namespace std::placeholders;
    signalConnectionManager.connect (gameGuiController->triggeredSave, std::bind (&cLocalScenarioGame::save, this, _1, _2));
    signalConnectionManager.connect (gameGuiController->terminated, [&]() { terminate = true; });
}

void cLocalScenarioGame::exit()
{
    gameGuiController->exit();
}

void cLocalScenarioGame::loadAiScript(std::string playerName, std::string luaFileName)
{
    // Search the client that has this player as active player
    for (size_t i = 0; i != iaClients.size(); ++i) {
        if (iaClients[i]->getActivePlayer().getName() != playerName) continue;
        std::shared_ptr<cLuaIntelligence> ai = std::make_shared<cLuaIntelligence>(iaClients[i]);
        using namespace std::placeholders;
        signalConnectionManager.connect(ai->showMessage, std::bind (&cLocalScenarioGame::popupMessage, this, _1));
        intelligences.push_back(ai);
        aiErrMsg = ai->openLuaFile(luaFileName);
        break;
    }
}

const cClient &cLocalScenarioGame::getClient(int index)
{
    if (index == 0) return *guiClient;
    else return *iaClients[index - 1];
}

void cLocalScenarioGame::turnChanged()
{
    if (scenarioFinished) return;

    Log.write("New turn begin, will call scenario victory condition script : " + std::to_string(server->getTurnClock()->getTurn()), cLog::eLOG_TYPE_DEBUG);

    // Add turn count as a global variable
    lua_pushinteger(L, server->getTurnClock()->getTurn());
    lua_setglobal(L, "turnCount");

    /* TODO_M: list of other datas that could be added to the context for the victory condition
     * List vehicles of each player, their position and caracteristics, if disabled
     * List of buildings of each player, their position and caracteristics, if disabled
     * Ecosphere count and score of players
     * Research state of players, player credits
     * Casualties: les pertes de chaque joueur, afin de savoir combien d'unités chacun s'est fait dégommé, on pourrait demander plus de dommage AI que humain au tour 50 par ex.
     */


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
            scenarioFinished = true;

            // Option to either return to game (endless play) or exit scenario.
            auto scenarioDialog = application->show (std::make_shared<cDialogScenarioEnd> (v > 0));
            signalConnectionManager.connect (scenarioDialog->exitClicked, [&]()
            {
                gameGuiController->exit ();
            });
        }
    }
}

void cLocalScenarioGame::popupMessage(std::string message)
{
    application->show(std::make_shared<cDialogOk>(message));
}

