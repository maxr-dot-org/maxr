#include "luaposition.h"

void LuaPosition::pushPosition(lua_State *L, const cPosition &pos)
{
    pushPosition(L, pos.x(), pos.y());
}

void LuaPosition::pushPosition(lua_State *L, int x, int y)
{
    lua_newtable(L);

    lua_pushinteger(L, x);
    lua_setfield(L, -2, "x");

    lua_pushinteger(L, y);
    lua_setfield(L, -2, "y");
}

bool LuaPosition::getPosition(lua_State *L, cPosition &pos)
{
    return getPosition(L, pos.x(), pos.y());
}

bool LuaPosition::getPosition(lua_State *L, int &x, int &y)
{
    int nbParams = lua_gettop(L);
    if (nbParams >= 1 && lua_istable(L, -1)) {
        lua_getfield(L, -1, "x");
        lua_getfield(L, -2, "y");
        if (lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
            x = (int)lua_tonumber(L, -2);
            y = (int)lua_tonumber(L, -1);
            lua_pop(L, 2);
            return true;
        }
        lua_pop(L, 2);
    }
    if (nbParams >= 2 && lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
        x = (int)lua_tonumber(L, -2);
        y = (int)lua_tonumber(L, -1);
        return true;
    }
    return false;
}

