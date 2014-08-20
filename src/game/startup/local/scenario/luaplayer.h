#ifndef LUAPLAYER_H
#define LUAPLAYER_H

#include <string>
#include <vector>
#include "utility/position.h"
#include "game/data/units/landingunit.h"
#include "lua/lua.hpp"
#include "lua/lunar.h"

class cClient;
struct sLandingUnit;
class cUnit;

struct sPlayerUnit {
    sID unitID;
    cPosition position;
};

class LuaPlayer
{
public:
    static const char className[];
    static Lunar<LuaPlayer>::RegType methods[];

public:
    LuaPlayer(lua_State *L);
    LuaPlayer(cPlayer* player);

    // Lua interface
    int getName(lua_State *L);
    int getLandingPosition(lua_State *L);
    int getClan(lua_State *L);
    int getVehicleCount(lua_State *L);
    int getVehicleIdList(lua_State *L);
    int getVehicleById(lua_State *L);
    int getBuildingCount(lua_State *L);
    int getBuildingIdList(lua_State *L);
    int getBuildingById(lua_State *L);

    // Extra interface for scenario construction
    int setClan(lua_State *L);
    int setLandingPosition(lua_State *L);
    int addLandingUnit(lua_State *L);
    int addUnit(lua_State *L);
    int addBuilding(lua_State *L);
    int setIaScript(lua_State *L);

    // C interface
    std::string getName() const;
    int getClan() const;
    cPosition landingPosition() const;
    std::vector<sPlayerUnit> getUnits() const { return m_otherUnits; }
    std::vector<sPlayerUnit> getBuildings() const { return m_buildings; }
    std::string getIaScript() const { return m_iaScriptName; }
    void sendInformations(const cClient& client);

private:
    void pushUnitData(lua_State *L, cUnit *unit);

private:
    cPlayer *m_player;
    std::vector<sLandingUnit> m_landingUnits;
    std::vector<sPlayerUnit> m_otherUnits;
    std::vector<sPlayerUnit> m_buildings;
    std::string m_iaScriptName;
};

#endif // LUAPLAYER_H
