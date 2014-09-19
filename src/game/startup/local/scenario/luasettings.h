#ifndef LUASETTINGS_H
#define LUASETTINGS_H

#include "lua/lua.hpp"
#include "lua/lunar.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"

class cLuaSettings
{
public:
    static const char className[];
    static Lunar<cLuaSettings>::RegType methods[];

public:
    // Lua interface
    cLuaSettings(lua_State *);
    cLuaSettings(std::shared_ptr<const cGameSettings> settings);
    ~cLuaSettings();
    int setHumanChooseClan(lua_State *L);
    int setStartingCredits(lua_State *L);
    int setClansEnabled(lua_State *L);
    int setBridgeHeadDefinite(lua_State *L);
    int getStartingCredits(lua_State *L);
    int getClansEnabled(lua_State *L);
    int getBridgeHeadDefinite(lua_State *L);

    // C++ interface
    std::shared_ptr<cGameSettings> getGameSettings() const { return gameSettings; }
    bool isHumanChooseClan() const { return humanChooseClan; }

private:
    std::shared_ptr<cGameSettings> gameSettings;
    std::shared_ptr<const cGameSettings> constSettings;
    bool humanChooseClan;
};

#endif // LUASETTINGS_H
