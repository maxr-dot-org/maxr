#include "luaintelligence.h"

#include "game/data/units/vehicle.h"
#include "game/data/player/player.h"
#include "game/logic/client.h"
#include "game/logic/turnclock.h"
#include "utility/files.h"
#include "utility/log.h"

#include "lua/lua.hpp"
#include "game/startup/local/scenario/luaposition.h"
#include "game/startup/local/scenario/luagame.h"
#include "game/startup/local/scenario/luaplayer.h"
#include "game/startup/local/scenario/luasettings.h"

const char cLuaIntelligence::className[] = "LuaIntelligence";

Lunar<cLuaIntelligence>::RegType cLuaIntelligence::methods[] = {
    LUNAR_DECLARE_METHOD(cLuaIntelligence, getSettings),
    LUNAR_DECLARE_METHOD(cLuaIntelligence, move),
    {0,0}
};

cLuaIntelligence::cLuaIntelligence(std::shared_ptr<cClient> client) :
    client(client)
{
    // Connect to the client signals
    signalConnectionManager.connect(client->finishedTurnEndProcess, std::bind (&cLuaIntelligence::newTurn, this));
    signalConnectionManager.connect(client->moveJobsFinished, std::bind (&cLuaIntelligence::moveJobsFinished, this));
}

cLuaIntelligence::cLuaIntelligence(lua_State *)
{
    // This does not have to be created from Lua !
}

cLuaIntelligence::~cLuaIntelligence()
{
    Log.write("LuaIntelligence destructor", cLog::eLOG_TYPE_DEBUG);

    // Close Lua context
    lua_close(L);
}

std::string cLuaIntelligence::openLuaFile(std::string luaFilename)
{
    // Open Lua File
    std::string fullFilename = cSettings::getInstance().getScenariosPath() + PATH_DELIMITER + luaFilename;
    SDL_RWops* fpLuaFile = SDL_RWFromFile(fullFilename.c_str(), "rb");
    if (fpLuaFile == NULL)
    {
        // TODO_M: hugh seems like a bad copy paste, we can't find scenarios in map directory !
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
        return "Cannot load intelligence file: \"" + luaFilename + "\"";
    }
    Sint64 fileSize = SDL_RWsize(fpLuaFile);
    char* luaData = new char[(size_t)fileSize];
    SDL_RWread(fpLuaFile, luaData, 1, (size_t)fileSize);
    SDL_RWclose (fpLuaFile);

    // Create a lua context and run Lua interpreter
    L = luaL_newstate();
    luaL_openlibs(L);
    Lunar<cLuaPlayer>::Register(L);
    Lunar<cLuaSettings>::Register(L);
    Lunar<cLuaIntelligence>::Register(L);

    // Create the global variable "game" that will point to this
    Lunar<cLuaIntelligence>::push(L, this);
    lua_setglobal(L, "game");

    // Set the path to the AI modules
    lua_getglobal( L, "package" );
    lua_getfield( L, -1, "path" );
    std::string curPath = lua_tostring(L, -1);
    std::string aiPath = cSettings::getInstance().getScenariosPath() + "/ia/?.lua;";
    std::string newPath = aiPath + curPath;
    lua_pop(L, 1);      // pops last path value
    lua_pushstring(L, newPath.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);     // pops the package table


    // Add the AI player (that is us, the active player from the client) and others to the Lua script.
    const std::vector<std::shared_ptr<cPlayer>>& players = client->getPlayerList();
    lua_newtable(L);
    for (unsigned int i = 0; i < players.size(); i++) {
        cLuaPlayer *lp = new cLuaPlayer(players[i].get());
        if (players[i]->getNr() == client->getActivePlayer().getNr()) {
            Lunar<cLuaPlayer>::push(L, lp, true);
            lua_setglobal(L, "ai");
        }
        else {
            lua_pushstring(L, lp->getName().c_str());
            Lunar<cLuaPlayer>::push(L, lp, true);
            lua_settable(L, -3);
        }
    }
    lua_setglobal(L, "ennemies");

    // TODO_M: should schedule the call to newTurn to be able to react on first game turn

    // Load the Lua script
    int error = luaL_loadbuffer(L, luaData, (size_t)fileSize, "luaIntelligence") || lua_pcall(L, 0, 0, 0);
    if (error) {
        std::string errStr = lua_tostring(L, -1);
        lua_pop(L, 1);
        return "Cannot run intelligence file, check lua syntax: \"" + luaFilename + "\"" + "\nLua Error: \n" + errStr;
    }
    else {
        Log.write("Intelligence loaded successfully.", cLog::eLOG_TYPE_INFO);
    }
    delete[] luaData;

    return "";
}

int cLuaIntelligence::getSettings(lua_State *L)
{
    cLuaSettings *ls = new cLuaSettings(client->getGameSettings());
    Lunar<cLuaSettings>::push(L, ls, true);
    return 1;
}

// Take iID and destination as argument (LuaPosition or x, y)
int cLuaIntelligence::move(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams < 2 || !lua_isnumber(L, 1)) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Move needs at least 2 args and first is number");
        return 2;
    }

    unsigned int iID = (unsigned int)lua_tointeger(L, 1);

    cPosition pos;
    bool posOk = cLuaPosition::getPosition(L, pos);
    cVehicle *v = client->getVehicleFromID(iID);
    if (!posOk || v == 00) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Move could not get vehicle or position");
        return 2;
    }

    bool success = client->addMoveJob(*v, pos);
    lua_pushboolean(L, success);
    if (success) return 1;
    else {
        lua_pushstring(L, "Move client addMoveJob error");
        return 2;
    }
}

void cLuaIntelligence::newTurn()
{
    Log.write("New turn begin, will call ai script : " + std::to_string(client->getTurnClock()->getTurn()), cLog::eLOG_TYPE_DEBUG);

    // Add turn count as a global variable
    lua_pushinteger(L, client->getTurnClock()->getTurn());
    lua_setglobal(L, "turnCount");

    // Call Lua function "newTurn" from ai;
    lua_getglobal(L, "newTurn");
    if (lua_pcall(L, 0, 1, 0) != 0) {
        showMessage("Error calling newTurn : " + std::string(lua_tostring(L, -1)));
        return;
    }
}

void cLuaIntelligence::moveJobsFinished()
{
    // Call Lua function "movesFinished" from ai;
    lua_getglobal(L, "movesFinished");
    if (lua_pcall(L, 0, 1, 0) != 0) {
        showMessage("Error calling movesFinished : " + std::string(lua_tostring(L, -1)));
        return;
    }
}
