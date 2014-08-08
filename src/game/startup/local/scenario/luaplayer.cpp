#include "luaplayer.h"

#include "game/startup/local/scenario/luaposition.h"
#include "game/logic/clientevents.h"
#include "game/data/units/unitdata.h"
#include "utility/log.h"
#include "main.h"

const char LuaPlayer::className[] = "LuaPlayer";

Lunar<LuaPlayer>::RegType LuaPlayer::methods[] = {
    LUNAR_DECLARE_METHOD(LuaPlayer, getName),
    LUNAR_DECLARE_METHOD(LuaPlayer, getLandingPosition),
    LUNAR_DECLARE_METHOD(LuaPlayer, setLandingPosition),
    LUNAR_DECLARE_METHOD(LuaPlayer, addLandingUnit),
    LUNAR_DECLARE_METHOD(LuaPlayer, setClan),
    LUNAR_DECLARE_METHOD(LuaPlayer, addUnit),
    LUNAR_DECLARE_METHOD(LuaPlayer, addBuilding),
    {0,0}
};

LuaPlayer::LuaPlayer(lua_State *L) :
    m_name("__LuaPlayer"),
    m_clan(-1),
    m_landingPosition(0, 0)
{
    // Only here for Lunar registration, should not be created from Lua code
}

LuaPlayer::LuaPlayer(std::string name) :
    m_name(name),
    m_clan(-1),
    m_landingPosition(0, 0)
{
}

int LuaPlayer::getName(lua_State *L)
{
    lua_pushstring(L, m_name.c_str());
    return 1;
}

int LuaPlayer::getLandingPosition(lua_State *L)
{
    LuaPosition* lpos = new LuaPosition(m_landingPosition);
    Lunar<LuaPosition>::push(L, lpos, true);        // true: use the lua gc
    return 1;
}

int LuaPlayer::setLandingPosition(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isuserdata(L, 1)) {
        LuaPosition* lpos = Lunar<LuaPosition>::check(L, 1);
        m_landingPosition = lpos->getPosition();
    }
    if (nbParams == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        int x = (int)lua_tonumber(L, 1);
        int y = (int)lua_tonumber(L, 2);
        m_landingPosition = cPosition(x, y);
    }
    return 0;
}

int LuaPlayer::addLandingUnit(lua_State *L)
{
    int nbParams = lua_gettop(L);
    sID unitId;
    int cargo = 0;

    // Id
    if (nbParams > 0 && lua_isstring(L, 1)) {
        // Get the sID from the unit string
        std::string unitName = lua_tostring(L, 1);
        try {
            unitId = UnitsData.m_vehiclesIDs.at(unitName);
        }
        catch (const std::out_of_range& oor) {
            Log.write("Impossible to add landing unit, does not exists : " + unitName + "\n" + oor.what(), cLog::eLOG_TYPE_WARNING);
            return 0;
        }

        // Forbid units that can't stand on the ground nor the air
        if (UnitsData.getUnit(unitId).factorGround == 0.0 && UnitsData.getUnit(unitId).factorAir == 0.0) return 0;
    }
    else return 0;

    // Cargo
    if (nbParams == 2 && lua_isnumber(L, 2)) {
        cargo = (int)lua_tonumber(L, 2);
    }

    sLandingUnit unit = { unitId, cargo };
    m_landingUnits.push_back(unit);

    return 0;
}

int LuaPlayer::addUnit(lua_State *L)
{
    std::string unitName;

    int nbParams = lua_gettop(L);
    if (nbParams >= 2 && lua_isstring(L, 1)) {
        unitName = lua_tostring(L, 1);

        sPlayerUnit pu;
        if (nbParams == 2 && lua_isuserdata(L, 2)) {
            LuaPosition* lpos = Lunar<LuaPosition>::check(L, 2);
            pu.position = lpos->getPosition();
        }
        else if (lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
            pu.position.x() = (int)lua_tonumber(L, 2);
            pu.position.y() = (int)lua_tonumber(L, 3);
        }
        else {
            Log.write("addUnit parameters error ", cLog::eLOG_TYPE_WARNING);
            return 0;
        }

        pu.unitID = UnitsData.m_vehiclesIDs.at(unitName);
        m_otherUnits.push_back(pu);
    }

    return 0;
}

int LuaPlayer::addBuilding(lua_State *L)
{
    std::string buildingName;

    int nbParams = lua_gettop(L);
    if (nbParams >= 2 && lua_isstring(L, 1)) {
        buildingName = lua_tostring(L, 1);

        sPlayerUnit pu;
        if (nbParams == 2 && lua_isuserdata(L, 2)) {
            LuaPosition* lpos = Lunar<LuaPosition>::check(L, 2);
            pu.position = lpos->getPosition();
        }
        else if (lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
            pu.position.x() = (int)lua_tonumber(L, 2);
            pu.position.y() = (int)lua_tonumber(L, 3);
        }
        else {
            Log.write("addUnit parameters error ", cLog::eLOG_TYPE_WARNING);
            return 0;
        }

        pu.unitID = UnitsData.m_buildingsIDs.at(buildingName);
        m_buildings.push_back(pu);
    }

    return 0;
}

int LuaPlayer::setClan(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        int clan = (int)lua_tonumber(L, 1);
        if (clan >= 0 && clan < 8) {
            m_clan = clan;
        }
    }
    return 0;
}

void LuaPlayer::sendInformations(const cClient &client)
{
    sendClan(client);
    sendLandingUnits(client, m_landingUnits);
    sendUnitUpgrades(client);
    sendLandingCoords(client, m_landingPosition);
    sendReadyToStart(client);
}
