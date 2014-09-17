#ifndef LUAGAME_H
#define LUAGAME_H

#include <memory>
#include <vector>

#include "lua/lua.hpp"
#include "lua/lunar.h"
#include "game/startup/local/scenario/localscenariogame.h"
#include "game/startup/local/scenario/luaplayer.h"

class cWindowClanSelection;

class LuaGame
{
public:
    static const char className[];
    static Lunar<LuaGame>::RegType methods[];

public:
    LuaGame(cLocalScenarioGame* game);
    LuaGame(lua_State *);
    ~LuaGame();

    // Lua interface
    int getAvailableMaps(lua_State *L);
    int loadMap(lua_State *L);
    int getHumanPlayer(lua_State *L);
    int addPlayer(lua_State *L);
    int setSettings(lua_State *L);
    int start(lua_State *);
    int message(lua_State *L);

    // C interface
    void setHumanClan(int clan);
    void gameReady();

private:
    void buildGame();

private:
    cLocalScenarioGame* m_game;
    std::vector<std::string> m_availableMaps;
    std::vector<LuaPlayer*> m_players;

    bool m_mapLoaded;
    std::shared_ptr<cWindowClanSelection> m_clanWindow;
    int m_humanClan;
};

#endif // LUAGAME_H
