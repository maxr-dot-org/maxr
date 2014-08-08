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
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/gamegui.h"
#include "loaddata.h"

const char LuaGame::className[] = "LuaGame";

Lunar<LuaGame>::RegType LuaGame::methods[] = {
    LUNAR_DECLARE_METHOD(LuaGame, getAvailableMaps),
    LUNAR_DECLARE_METHOD(LuaGame, loadMap),
    LUNAR_DECLARE_METHOD(LuaGame, getHumanPlayer),
    LUNAR_DECLARE_METHOD(LuaGame, addPlayer),
    LUNAR_DECLARE_METHOD(LuaGame, setSettings),
    LUNAR_DECLARE_METHOD(LuaGame, start),
    {0,0}
};

LuaGame::LuaGame(lua_State *L) :
    m_mapLoaded(false),
    m_game(std::make_shared<cLocalScenarioGame>()),
    m_humanClan(-1),
    m_humanChooseClan(false)
{
    Log.write("LuaGame construction from Lua, should never happen !", cLog::eLOG_TYPE_ERROR);
}

// LuaGame needs to be instantiated by C++ code, not by Lua !
LuaGame::LuaGame(cApplication *app) :
    m_mapLoaded(false),
    m_game(std::make_shared<cLocalScenarioGame>()),
    m_application(app),
    m_humanClan(-1),
    m_humanChooseClan(false)
{
    // List all available map : this is copy-paste from cWindowMapSelection --> refactoring
    m_availableMaps = getFilesOfDirectory (cSettings::getInstance ().getMapsPath ());
    if (!getUserMapsDir ().empty ())
    {
        std::vector<std::string> userMaps (getFilesOfDirectory (getUserMapsDir ()));
        for (size_t i = 0; i != userMaps.size (); ++i)
        {
            if (std::find (m_availableMaps.begin (), m_availableMaps.end (), userMaps[i]) == m_availableMaps.end ())
            {
                m_availableMaps.push_back (userMaps[i]);
            }
        }
    }
    for (size_t i = 0; i != m_availableMaps.size (); ++i)
    {
        const std::string& map = m_availableMaps[i];
        if (map.compare (map.length () - 3, 3, "WRL") != 0 && map.compare (map.length () - 3, 3, "wrl") != 0)
        {
            m_availableMaps.erase (m_availableMaps.begin () + i);
            i--;
        }
    }

    LuaPlayer* humanPlayer = new LuaPlayer(cSettings::getInstance().getPlayerName());
    m_players.push_back(humanPlayer);
}

LuaGame::~LuaGame()
{
    Log.write("LuaGame destructor", cLog::eLOG_TYPE_DEBUG);
    for (unsigned int i = 0; i < m_players.size(); i++) {
        LuaPlayer *lp = m_players[i];
        delete lp;
    }
}

int LuaGame::getAvailableMaps(lua_State *L)
{
    lua_newtable(L);
    for (unsigned int i = 0; i < m_availableMaps.size(); i++) {
        lua_pushnumber(L, i);
        lua_pushstring(L, m_availableMaps[i].c_str());
        lua_settable(L, -3);
    }
    return 1;
}

// Load the map, set it in server and clients
// param string filename of the map
int LuaGame::loadMap(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isstring(L, 1)) {
        std::string mapName = lua_tostring(L, 1);
        Log.write("Scenario map to load : " + mapName, cLog::eLOG_TYPE_INFO);
        m_mapLoaded |= m_game->loadMap(mapName);
    }
    return 0;
}

int LuaGame::getHumanPlayer(lua_State *L)
{
    Lunar<LuaPlayer>::push(L, m_players[0]);
    return 1;
}

// Add a AI player (and its client)
// optionnal param string: name of the player
int LuaGame::addPlayer(lua_State *L)
{
    // Default name
    std::string playerName = "IA_Player_";
    char tmp[2];
    TIXML_SNPRINTF(tmp, sizeof(tmp), "%.1d", m_game->playerCount());
    playerName.append(tmp);

    // Lua name if provided
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isstring(L, 1)) {
        playerName = lua_tostring(L, 1);
    }

    // Add player to the game
    m_game->addPlayer(playerName);

    // Return a LuaPlayer object
    LuaPlayer* lp = new LuaPlayer(playerName);
    Lunar<LuaPlayer>::push(L, lp);      // silent false means we are deleting LuaPlayer from C++
    m_players.push_back(lp);
    return 1;
}

// Load settings in server and clients
int LuaGame::setSettings(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isuserdata(L, 1)) {
        LuaSettings* settings = Lunar<LuaSettings>::check(L, 1);
        m_game->setGameSettings(settings->getGameSettings());
        m_humanChooseClan = settings->humanChooseClan();
    }
    return 0;
}

// Start the game
int LuaGame::start(lua_State *L)
{
    // Load the first map by default if it has not been loaded by the script
    if (!m_mapLoaded && m_availableMaps.size() > 0) {
        if (! m_game->loadMap(m_availableMaps.at(0))) return 0;
    }

    // Let human choose his clan if set from LuaSettings script
    if (m_humanChooseClan) {
        m_clanWindow = std::make_shared<cWindowClanSelection> ();
        m_application->show (m_clanWindow);
        m_clanWindow->done.connect ([&]()
        {
            // humanClan is set when script let user choose its own clan, otherwise script may choose clan as for each player
            m_humanClan = m_clanWindow->getSelectedClan();
            buildGame();
            m_clanWindow->close ();
        });
        m_clanWindow->canceled.connect ([&]()
        {
            m_clanWindow->close ();
            // TODO_M: cancel scenatio launch, should delete luaGame
        });
    }
    else {
        buildGame();
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

    return 0;
}

void LuaGame::buildGame()
{
    m_game->startServer();

    for (unsigned int i = 0; i < m_players.size(); i++) {
        LuaPlayer *lp = m_players[i];

        // Set clan
        if (i == 0 && m_humanClan >= 0) m_game->setPlayerClan(lp->getName(), m_humanClan);
        else m_game->setPlayerClan(lp->getName(), lp->getClan());

        // Add buildings to the game
        std::vector<sPlayerUnit> buildings = lp->getBuildings();
        for (unsigned int j = 0; j < buildings.size(); j++) {
            sPlayerUnit pu = buildings[j];
            m_game->addBuilding(pu.unitID, lp->getName(), pu.position);
        }

        // Add other units to the game
        std::vector<sPlayerUnit> units = lp->getUnits();
        for (unsigned int j = 0; j < units.size(); j++) {
            sPlayerUnit pu = units[j];
            m_game->addUnit(pu.unitID, lp->getName(), pu.position);
        }

        // Send player informations: landing units etc...
        lp->sendInformations(m_game->getClient(i));
    }

    m_game->setGuiPosition(m_players[0]->landingPosition());

    // Start definitely the game
    m_game->startGame(*m_application);
    Log.write("Scenario game started successfully", cLog::eLOG_TYPE_INFO);

    // TODO_M: Handle end of game condition, victory (destruction, ecosphere...) or defeat (destruction, particular unit destruction)
    // Let a lua function call evaluate this from the script
}

