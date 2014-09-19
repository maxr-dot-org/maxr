#ifndef LUAGAME_H
#define LUAGAME_H

#include <memory>
#include <vector>

#include "lua/lua.hpp"
#include "lua/lunar.h"
#include "game/startup/local/scenario/localscenariogame.h"
#include "game/startup/local/scenario/luaplayer.h"

class cWindowClanSelection;

class cLuaGame
{
public:
    static const char className[];
    static Lunar<cLuaGame>::RegType methods[];

public:
    cLuaGame(cLocalScenarioGame* game);
    cLuaGame(lua_State *);
    ~cLuaGame();

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
    cLocalScenarioGame* game;
    std::vector<std::string> availableMaps;
    std::vector<cLuaPlayer*> players;

    bool mapLoaded;
    std::shared_ptr<cWindowClanSelection> clanWindow;
    int humanClan;
};

#endif // LUAGAME_H
