#include "luagame.h"

#include "utility/files.h"
#include "utility/log.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/logic/savegame.h"
#include "game/data/player/player.h"
#include "game/data/map/map.h"
#include "game/startup/local/scenario/luaposition.h"
#include "game/startup/local/scenario/luasettings.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/gamegui.h"
#include "loaddata.h"

const char cLuaGame::className[] = "LuaGame";

Lunar<cLuaGame>::RegType cLuaGame::methods[] = {
    LUNAR_DECLARE_METHOD(cLuaGame, getAvailableMaps),
    LUNAR_DECLARE_METHOD(cLuaGame, loadMap),
    LUNAR_DECLARE_METHOD(cLuaGame, getHumanPlayer),
    LUNAR_DECLARE_METHOD(cLuaGame, addPlayer),
    LUNAR_DECLARE_METHOD(cLuaGame, setSettings),
    LUNAR_DECLARE_METHOD(cLuaGame, start),
    LUNAR_DECLARE_METHOD(cLuaGame, message),
    {0,0}
};

cLuaGame::cLuaGame(lua_State *) :
    mapLoaded(false),
    game(00),
    humanClan(-1)
{
    Log.write("LuaGame construction from Lua, should never happen !", cLog::eLOG_TYPE_ERROR);
}

// LuaGame needs to be instantiated by C++ code, not by Lua !
cLuaGame::cLuaGame(cLocalScenarioGame *game) :
    mapLoaded(false),
    game(game),
    humanClan(-1)
{
    // List all available map : this is copy-paste from cWindowMapSelection --> refactoring
    availableMaps = getFilesOfDirectory (cSettings::getInstance ().getMapsPath ());
    if (!getUserMapsDir ().empty ())
    {
        std::vector<std::string> userMaps (getFilesOfDirectory (getUserMapsDir ()));
        for (size_t i = 0; i != userMaps.size (); ++i)
        {
            if (std::find (availableMaps.begin (), availableMaps.end (), userMaps[i]) == availableMaps.end ())
            {
                availableMaps.push_back (userMaps[i]);
            }
        }
    }
    for (size_t i = 0; i != availableMaps.size (); ++i)
    {
        const std::string& map = availableMaps[i];
        if (map.compare (map.length () - 3, 3, "WRL") != 0 && map.compare (map.length () - 3, 3, "wrl") != 0)
        {
            availableMaps.erase (availableMaps.begin () + i);
            i--;
        }
    }

    cLuaPlayer* humanPlayer = new cLuaPlayer(game->humanPlayer());
    players.push_back(humanPlayer);
}

cLuaGame::~cLuaGame()
{
    Log.write("LuaGame destructor", cLog::eLOG_TYPE_DEBUG);
    for (unsigned int i = 0; i < players.size(); i++) {
        cLuaPlayer *lp = players[i];
        delete lp;
    }
}

int cLuaGame::getAvailableMaps(lua_State *L)
{
    lua_newtable(L);
    for (unsigned int i = 0; i < availableMaps.size(); i++) {
        lua_pushnumber(L, i);
        lua_pushstring(L, availableMaps[i].c_str());
        lua_settable(L, -3);
    }
    return 1;
}

// Load the map, set it in server and clients
// param string filename of the map
int cLuaGame::loadMap(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isstring(L, 1)) {
        std::string mapName = lua_tostring(L, 1);
        Log.write("Scenario map to load : " + mapName, cLog::eLOG_TYPE_INFO);
        mapLoaded |= game->loadMap(mapName);
    }
    return 0;
}

int cLuaGame::getHumanPlayer(lua_State *L)
{
    Lunar<cLuaPlayer>::push(L, players[0]);
    return 1;
}

// Add a AI player (and its client)
// optionnal param string: name of the player
int cLuaGame::addPlayer(lua_State *L)
{
    // Default name
    std::string playerName = "IA_Player_";
    char tmp[2];
    TIXML_SNPRINTF(tmp, sizeof(tmp), "%.1d", game->playerCount());
    playerName.append(tmp);

    // Lua name if provided
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isstring(L, 1)) {
        playerName = lua_tostring(L, 1);
    }

    // Add player to the game
    cPlayer *player = game->addPlayer(playerName);

    // Return a LuaPlayer object
    cLuaPlayer* lp = new cLuaPlayer(player);
    Lunar<cLuaPlayer>::push(L, lp);      // silent false means we are deleting LuaPlayer from C++
    players.push_back(lp);
    return 1;
}

// Load settings in server and clients
int cLuaGame::setSettings(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isuserdata(L, 1)) {
        cLuaSettings* settings = Lunar<cLuaSettings>::check(L, 1);
        game->setGameSettings(settings->getGameSettings());
        if (settings->isHumanChooseClan()) game->openClanWindow();
    }
    return 0;
}

// Start the game
int cLuaGame::start(lua_State *)
{
    // Load the first map by default if it has not been loaded by the script
    if (!mapLoaded && availableMaps.size() > 0) {
        if (! game->loadMap(availableMaps.at(0))) return 0;
    }

    // TODO_M: script unit upgrades from LUA
    // Lua may also permit additionnal configuration by player from the credit allowed
    // Where to put this ??
    if (false) {
//        auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(0), playClan, initialLandingUnits, gameSettings->getStartCredits ()));
//        windowLandingUnitSelection->done.connect ([=]()
//        {
//            game->setLandingUnits (windowLandingUnitSelection->getLandingUnits ());
//            game->setUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());
//        });
//        windowLandingUnitSelection->close();
    }

    if (game->startingStatus() == Ready) buildGame();             // Start game immediately
    if (game->startingStatus() == Cancelled) game->exit();      // Release ressource
    if (game->startingStatus() == WaitingHuman) { }               // Wait for human to interract (choose clan, ...)

    return 0;
}

int cLuaGame::message(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams > 0 || lua_isstring(L, 1)) {
        std::string m = lua_tostring(L, 1);
        game->popupMessage(m);
    }
    return 0;
}

void cLuaGame::setHumanClan(int clan)
{
    humanClan = clan;
}

void cLuaGame::gameReady()
{
    buildGame();
}

void cLuaGame::buildGame()
{
    game->startServer();

    for (unsigned int i = 0; i < players.size(); i++) {
        cLuaPlayer *lp = players[i];

        // Set clan
        if (i == 0 && humanClan >= 0) game->setPlayerClan(lp->getName(), humanClan);
        else game->setPlayerClan(lp->getName(), lp->getClan());

        // Add buildings to the game
        std::vector<sPlayerUnit> buildings = lp->getBuildings();
        for (unsigned int j = 0; j < buildings.size(); j++) {
            sPlayerUnit pu = buildings[j];
            game->addBuilding(pu.unitID, lp->getName(), pu.position);
        }

        // Add other units to the game
        std::vector<sPlayerUnit> units = lp->getUnits();
        for (unsigned int j = 0; j < units.size(); j++) {
            sPlayerUnit pu = units[j];
            game->addUnit(pu.unitID, lp->getName(), pu.position);
        }

        // Send player informations: landing units etc...
        lp->sendInformations(game->getClient(i));

        // Load AI scripts for each player
        if (lp->getIaScript() != "") game->loadAiScript(lp->getName(), lp->getIaScript());
    }

    game->setGuiPosition(players[0]->landingPosition());

    // Start definitely the game
    game->startGame();
    Log.write("Scenario game started successfully", cLog::eLOG_TYPE_INFO);
}

