#include "luasettings.h"

const char LuaSettings::className[] = "LuaSettings";

Lunar<LuaSettings>::RegType LuaSettings::methods[] = {
    LUNAR_DECLARE_METHOD(LuaSettings, setHumanChooseClan),
    LUNAR_DECLARE_METHOD(LuaSettings, setStartingCredits),
    LUNAR_DECLARE_METHOD(LuaSettings, setClansEnabled),
    LUNAR_DECLARE_METHOD(LuaSettings, setBridgeHeadDefinite),
    {0,0}
};

LuaSettings::LuaSettings(lua_State *L) :
    m_gameSettings(std::make_shared<cGameSettings>()),
    m_humanChooseClan(false)
{
    // Default game settings
    m_gameSettings->setStartCredits(0);
    m_gameSettings->setClansEnabled(true);
    m_gameSettings->setBridgeheadType(eGameSettingsBridgeheadType::Mobile);      // Mobile will remove the initial mining station building
}

int LuaSettings::setHumanChooseClan(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        m_humanChooseClan = lua_toboolean(L, 1) > 0;
    }
    return 0;
}

int LuaSettings::setStartingCredits(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        m_gameSettings->setStartCredits((int)lua_tonumber(L, 1));
    }
    return 0;
}

int LuaSettings::setClansEnabled(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        m_gameSettings->setClansEnabled(lua_toboolean(L, 1) > 0);
    }
    return 0;
}

int LuaSettings::setBridgeHeadDefinite(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isboolean(L, 1)) {
        m_gameSettings->setBridgeheadType(lua_toboolean(L, 1) ? eGameSettingsBridgeheadType::Definite : eGameSettingsBridgeheadType::Mobile);
    }
    return 0;
}
