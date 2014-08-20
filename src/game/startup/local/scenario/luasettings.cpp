#include "luasettings.h"

#include "utility/log.h"

const char LuaSettings::className[] = "LuaSettings";

Lunar<LuaSettings>::RegType LuaSettings::methods[] = {
    LUNAR_DECLARE_METHOD(LuaSettings, setHumanChooseClan),
    LUNAR_DECLARE_METHOD(LuaSettings, setStartingCredits),
    LUNAR_DECLARE_METHOD(LuaSettings, setClansEnabled),
    LUNAR_DECLARE_METHOD(LuaSettings, setBridgeHeadDefinite),
    LUNAR_DECLARE_METHOD(LuaSettings, getStartingCredits),
    LUNAR_DECLARE_METHOD(LuaSettings, getClansEnabled),
    LUNAR_DECLARE_METHOD(LuaSettings, getBridgeHeadDefinite),
    {0,0}
};

// Creates a new game settings from Lua
LuaSettings::LuaSettings(lua_State *L) :
    m_gameSettings(std::make_shared<cGameSettings>()),
    m_humanChooseClan(false)
{
//    Log.write("Lua Settings creation from LUA *****************************************************************", cLog::eLOG_TYPE_DEBUG);
    // Default game settings
    m_gameSettings->setStartCredits(0);
    m_gameSettings->setClansEnabled(true);
    m_gameSettings->setBridgeheadType(eGameSettingsBridgeheadType::Mobile);      // Mobile will remove the initial mining station building
}

LuaSettings::LuaSettings(std::shared_ptr<const cGameSettings> settings) :
    m_constSettings(settings),
    m_humanChooseClan(false)
{
//    Log.write("Lua settings creation from C++ *****************************************************************", cLog::eLOG_TYPE_DEBUG);
}

LuaSettings::~LuaSettings()
{
//    Log.write("Lua Settings destruction *****************************************************************", cLog::eLOG_TYPE_DEBUG);
}
int LuaSettings::setHumanChooseClan(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        m_humanChooseClan = lua_toboolean(L, 1) > 0;
    }
    return 0;
}

// Only works when settings are created from Lua unless they are read only
int LuaSettings::setStartingCredits(lua_State *L)
{
    if (!m_gameSettings) return 0;
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        m_gameSettings->setStartCredits((int)lua_tonumber(L, 1));
    }
    return 0;
}

// Only works when settings are created from Lua unless they are read only
int LuaSettings::setClansEnabled(lua_State *L)
{
    if (!m_gameSettings) return 0;
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        m_gameSettings->setClansEnabled(lua_toboolean(L, 1) > 0);
    }
    return 0;
}

// Only works when settings are created from Lua unless they are read only
int LuaSettings::setBridgeHeadDefinite(lua_State *L)
{
    if (!m_gameSettings) return 0;
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        m_gameSettings->setBridgeheadType(lua_toboolean(L, 1) ? eGameSettingsBridgeheadType::Definite : eGameSettingsBridgeheadType::Mobile);
    }
    return 0;
}

int LuaSettings::getStartingCredits(lua_State *L)
{
    if (m_gameSettings) lua_pushinteger(L, m_gameSettings->getStartCredits());
    else lua_pushinteger(L, m_constSettings->getStartCredits());
    return 1;
}

int LuaSettings::getClansEnabled(lua_State *L)
{
    if (m_gameSettings) lua_pushboolean(L, m_gameSettings->getClansEnabled());
    else lua_pushboolean(L, m_constSettings->getClansEnabled());
    return 1;
}

int LuaSettings::getBridgeHeadDefinite(lua_State *L)
{
    if (m_gameSettings) lua_pushboolean(L, m_gameSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite);
    else lua_pushboolean(L, m_constSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite);
    return 1;
}
