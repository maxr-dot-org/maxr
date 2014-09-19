#include "luasettings.h"

#include "utility/log.h"

const char cLuaSettings::className[] = "LuaSettings";

Lunar<cLuaSettings>::RegType cLuaSettings::methods[] = {
    LUNAR_DECLARE_METHOD(cLuaSettings, setHumanChooseClan),
    LUNAR_DECLARE_METHOD(cLuaSettings, setStartingCredits),
    LUNAR_DECLARE_METHOD(cLuaSettings, setClansEnabled),
    LUNAR_DECLARE_METHOD(cLuaSettings, setBridgeHeadDefinite),
    LUNAR_DECLARE_METHOD(cLuaSettings, getStartingCredits),
    LUNAR_DECLARE_METHOD(cLuaSettings, getClansEnabled),
    LUNAR_DECLARE_METHOD(cLuaSettings, getBridgeHeadDefinite),
    {0,0}
};

// Creates a new game settings from Lua
cLuaSettings::cLuaSettings(lua_State *) :
    gameSettings(std::make_shared<cGameSettings>()),
    humanChooseClan(false)
{
//    Log.write("Lua Settings creation from LUA *****************************************************************", cLog::eLOG_TYPE_DEBUG);
    // Default game settings
    gameSettings->setStartCredits(0);
    gameSettings->setClansEnabled(true);
    gameSettings->setBridgeheadType(eGameSettingsBridgeheadType::Mobile);      // Mobile will remove the initial mining station building
}

cLuaSettings::cLuaSettings(std::shared_ptr<const cGameSettings> settings) :
    constSettings(settings),
    humanChooseClan(false)
{
//    Log.write("Lua settings creation from C++ *****************************************************************", cLog::eLOG_TYPE_DEBUG);
}

cLuaSettings::~cLuaSettings()
{
//    Log.write("Lua Settings destruction *****************************************************************", cLog::eLOG_TYPE_DEBUG);
}
int cLuaSettings::setHumanChooseClan(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        humanChooseClan = lua_toboolean(L, 1) > 0;
    }
    return 0;
}

// Only works when settings are created from Lua unless they are read only
int cLuaSettings::setStartingCredits(lua_State *L)
{
    if (!gameSettings) return 0;
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        gameSettings->setStartCredits((int)lua_tonumber(L, 1));
    }
    return 0;
}

// Only works when settings are created from Lua unless they are read only
int cLuaSettings::setClansEnabled(lua_State *L)
{
    if (!gameSettings) return 0;
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        gameSettings->setClansEnabled(lua_toboolean(L, 1) > 0);
    }
    return 0;
}

// Only works when settings are created from Lua unless they are read only
int cLuaSettings::setBridgeHeadDefinite(lua_State *L)
{
    if (!gameSettings) return 0;
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        gameSettings->setBridgeheadType(lua_toboolean(L, 1) ? eGameSettingsBridgeheadType::Definite : eGameSettingsBridgeheadType::Mobile);
    }
    return 0;
}

int cLuaSettings::getStartingCredits(lua_State *L)
{
    if (gameSettings) lua_pushinteger(L, gameSettings->getStartCredits());
    else lua_pushinteger(L, constSettings->getStartCredits());
    return 1;
}

int cLuaSettings::getClansEnabled(lua_State *L)
{
    if (gameSettings) lua_pushboolean(L, gameSettings->getClansEnabled());
    else lua_pushboolean(L, constSettings->getClansEnabled());
    return 1;
}

int cLuaSettings::getBridgeHeadDefinite(lua_State *L)
{
    if (gameSettings) lua_pushboolean(L, gameSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite);
    else lua_pushboolean(L, constSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite);
    return 1;
}
