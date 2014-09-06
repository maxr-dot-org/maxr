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

const char LuaIntelligence::className[] = "LuaIntelligence";

Lunar<LuaIntelligence>::RegType LuaIntelligence::methods[] = {
    LUNAR_DECLARE_METHOD(LuaIntelligence, getSettings),
    LUNAR_DECLARE_METHOD(LuaIntelligence, move),
    {0,0}
};

LuaIntelligence::LuaIntelligence(std::shared_ptr<cClient> client) :
    m_client(client)
{
//    Log.write("Lua Intelligence construction *****************************************************************", cLog::eLOG_TYPE_DEBUG);

    // Connect to the new turn signal
    m_signalConnectionManager.connect(m_client->finishedTurnEndProcess, std::bind (&LuaIntelligence::newTurn, this));
}

LuaIntelligence::LuaIntelligence(lua_State *L)
{
//    Log.write("Lua Intelligence creation from LUA ERROR *****************************************************************", cLog::eLOG_TYPE_DEBUG);
    // This does not have to be created from Lua !
}

LuaIntelligence::~LuaIntelligence()
{
    Log.write("LuaIntelligence destructor", cLog::eLOG_TYPE_DEBUG);

    // Close Lua context
    lua_close(L);
}

void LuaIntelligence::openLuaFile(std::string luaFilename)
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
        Log.write("Cannot load intelligence file: \"" + luaFilename + "\"", cLog::eLOG_TYPE_ERROR);
        return;
    }
    Sint64 fileSize = SDL_RWsize(fpLuaFile);
    char* luaData = new char[fileSize];
    SDL_RWread(fpLuaFile, luaData, 1, fileSize);
    SDL_RWclose (fpLuaFile);

    // Create a lua context and run Lua interpreter
    L = luaL_newstate();
    luaL_openlibs(L);
    Lunar<LuaPlayer>::Register(L);
    Lunar<LuaSettings>::Register(L);
    Lunar<LuaIntelligence>::Register(L);

    // Create the global variable "game" that will point to this
    Lunar<LuaIntelligence>::push(L, this);
    lua_setglobal(L, "game");

    // Add the AI player (that is us, the active player from the client) and others to the Lua script.
    const std::vector<std::shared_ptr<cPlayer>>& players = m_client->getPlayerList();
    lua_newtable(L);
    for (unsigned int i = 0; i < players.size(); i++) {
        LuaPlayer *lp = new LuaPlayer(players[i].get());
        if (players[i]->getNr() == m_client->getActivePlayer().getNr()) {
            Lunar<LuaPlayer>::push(L, lp, true);
            lua_setglobal(L, "ai");
        }
        else {
            lua_pushstring(L, lp->getName().c_str());
            Lunar<LuaPlayer>::push(L, lp, true);
            lua_settable(L, -3);
        }
    }
    lua_setglobal(L, "ennemies");

    // TODO_M: should schedule the call to newTurn to be able to react on first game turn

    // Try to understand where to begin to make a unit move :p

    // Load the Lua script
    int error = luaL_loadbuffer(L, luaData, fileSize, "luaIntelligence") || lua_pcall(L, 0, 0, 0);
    if (error) {
        Log.write("Cannot run intelligence file, check lua syntax: \"" + luaFilename + "\"" +
                  "\nLua Error: \n" + lua_tostring(L, -1), cLog::eLOG_TYPE_ERROR);
        lua_pop(L, 1);
    }
    else {
        Log.write("Intelligence loaded successfully.", cLog::eLOG_TYPE_INFO);
    }
    delete[] luaData;
}

int LuaIntelligence::getSettings(lua_State *L)
{
//    Log.write("Lua Intelligence get settings *****************************************************************", cLog::eLOG_TYPE_DEBUG);
    LuaSettings *ls = new LuaSettings(m_client->getGameSettings());
    Lunar<LuaSettings>::push(L, ls, true);
    return 1;
}

// Take iID and destination as argument (LuaPosition or x, y)
int LuaIntelligence::move(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams < 2 || !lua_isnumber(L, 1)) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Move needs at least 2 args and first is number");
        return 2;
    }

    unsigned int iID = (unsigned int)lua_tointeger(L, 1);

    cPosition pos;
    bool posOk = LuaPosition::getPosition(L, pos);
    cVehicle *v = m_client->getVehicleFromID(iID);
    if (!posOk || v == 00) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Move could not get vehicle or position");
        return 2;
    }

    bool success = m_client->addMoveJob(*v, pos);
    lua_pushboolean(L, success);
    if (success) return 1;
    else {
        lua_pushstring(L, "Move client addMoveJob error");
        return 2;
    }
}

void LuaIntelligence::newTurn()
{
    Log.write("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", cLog::eLOG_TYPE_DEBUG);
    Log.write("New turn begin, will call ai script : " + std::to_string(m_client->getTurnClock()->getTurn()), cLog::eLOG_TYPE_DEBUG);

    // Add turn count as a global variable
    lua_pushinteger(L, m_client->getTurnClock()->getTurn());
    lua_setglobal(L, "turnCount");

    // Call Lua function "newTurn" from ai;
    lua_getglobal(L, "newTurn");
    if (lua_pcall(L, 0, 1, 0) != 0) {
        // TODO_M: find better way for visibility of Lua Errors: should popup a dialog in MAXR
        Log.write("****************************************************************************************************************************************************** \n"
                  "Error calling newTurn : " + std::string(lua_tostring(L, -1)), cLog::eLOG_TYPE_ERROR);
        return;
    }
}
