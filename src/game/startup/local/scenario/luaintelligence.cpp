#include "luaintelligence.h"

#include "game/logic/client.h"
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
    {0,0}
};

LuaIntelligence::LuaIntelligence(std::shared_ptr<cClient> client) :
    m_client(client)
{
//    Log.write("Lua Intelligence construction *****************************************************************", cLog::eLOG_TYPE_DEBUG);
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
    Lunar<LuaPosition>::Register(L);
    Lunar<LuaPlayer>::Register(L);
    Lunar<LuaSettings>::Register(L);
    Lunar<LuaIntelligence>::Register(L);

    // Create the global variable "game" that will point to this
    Lunar<LuaIntelligence>::push(L, this);
    lua_setglobal(L, "game");

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
