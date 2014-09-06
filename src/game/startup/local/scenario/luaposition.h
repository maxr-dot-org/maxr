#ifndef LUAPOSITION_H
#define LUAPOSITION_H

#include "utility/position.h"
#include "lua/lua.hpp"

class LuaPosition
{
public:
    static void pushPosition(lua_State *L, const cPosition &pos);
    static void pushPosition(lua_State *L, int x, int y);
    static bool getPosition(lua_State *L, cPosition &pos);
    static bool getPosition(lua_State *L, int &x, int &y);
};

#endif // LUAPOSITION_H
