#ifndef LUAPOSITION_H
#define LUAPOSITION_H

#include "utility/position.h"
#include "lua/lua.hpp"
#include "lua/lunar.h"

class LuaPosition
{
public:
    static const char className[];
    static Lunar<LuaPosition>::RegType methods[];

public:
    LuaPosition(lua_State *L);                      // Build a LuaPosition from lua code
    LuaPosition(const cPosition &position);         // Build a LuaPosition from C code (copy data)
    LuaPosition(int x, int y);                      // Build a LuaPosition from C code (copy data)

    // C interface
    cPosition getPosition() const { return m_position; }
    int x() const { return m_position.x(); }
    int y() const { return m_position.y(); }

    // Lua interface (can't be const)
    int x(lua_State *L);
    int y(lua_State *L);
    int setX(lua_State *L);
    int setY(lua_State *L);

private:
    cPosition m_position;
};

#endif // LUAPOSITION_H
