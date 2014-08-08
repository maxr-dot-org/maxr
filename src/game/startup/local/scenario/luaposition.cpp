#include "luaposition.h"

const char LuaPosition::className[] = "LuaPosition";

Lunar<LuaPosition>::RegType LuaPosition::methods[] = {
    LUNAR_DECLARE_METHOD(LuaPosition, x),
    LUNAR_DECLARE_METHOD(LuaPosition, y),
    LUNAR_DECLARE_METHOD(LuaPosition, setX),
    LUNAR_DECLARE_METHOD(LuaPosition, setY),
    {0,0}
};

LuaPosition::LuaPosition(lua_State *L) :
    m_position(-1, -1)
{
    int nbParams = lua_gettop(L);

    // If two args, build a new position from x, y coordinates
    if (nbParams == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        int x = (int)lua_tonumber(L, 1);
        int y = (int)lua_tonumber(L, 2);
        m_position = cPosition(x, y);
    }
}

LuaPosition::LuaPosition(const cPosition &position) :
    m_position(position)
{
}

int LuaPosition::x(lua_State *L)
{
    lua_pushinteger(L, m_position.x());
    return 1;       // return the number of values pushed on the stack
}

int LuaPosition::y(lua_State *L)
{
    lua_pushinteger(L, m_position.y());
    return 1;
}

int LuaPosition::setX(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        m_position.x() = (int)lua_tonumber(L, 1);
    }
    return 0;       // here we pushed nothing on the stack
}

int LuaPosition::setY(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        m_position.y() = (int)lua_tonumber(L, 1);
    }
    return 0;
}
