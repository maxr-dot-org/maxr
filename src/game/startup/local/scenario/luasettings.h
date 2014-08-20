#ifndef LUASETTINGS_H
#define LUASETTINGS_H

#include "lua/lua.hpp"
#include "lua/lunar.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"

class LuaSettings
{
public:
    static const char className[];
    static Lunar<LuaSettings>::RegType methods[];

public:
    // Lua interface
    LuaSettings(lua_State *L);
    LuaSettings(std::shared_ptr<const cGameSettings> settings);
    ~LuaSettings();
    int setHumanChooseClan(lua_State *L);
    int setStartingCredits(lua_State *L);
    int setClansEnabled(lua_State *L);
    int setBridgeHeadDefinite(lua_State *L);
    int getStartingCredits(lua_State *L);
    int getClansEnabled(lua_State *L);
    int getBridgeHeadDefinite(lua_State *L);

    // C++ interface
    std::shared_ptr<cGameSettings> getGameSettings() const { return m_gameSettings; }
    bool humanChooseClan() const { return m_humanChooseClan; }

private:
    std::shared_ptr<cGameSettings> m_gameSettings;
    std::shared_ptr<const cGameSettings> m_constSettings;
    bool m_humanChooseClan;
};

#endif // LUASETTINGS_H
