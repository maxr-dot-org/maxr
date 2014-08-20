#include "luaplayer.h"

#include "game/startup/local/scenario/luaposition.h"
#include "game/logic/clientevents.h"
#include "game/data/player/player.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "utility/log.h"
#include "main.h"

const char LuaPlayer::className[] = "LuaPlayer";

Lunar<LuaPlayer>::RegType LuaPlayer::methods[] = {
    LUNAR_DECLARE_METHOD(LuaPlayer, getName),
    LUNAR_DECLARE_METHOD(LuaPlayer, getLandingPosition),
    LUNAR_DECLARE_METHOD(LuaPlayer, setLandingPosition),
    LUNAR_DECLARE_METHOD(LuaPlayer, addLandingUnit),
    LUNAR_DECLARE_METHOD(LuaPlayer, getClan),
    LUNAR_DECLARE_METHOD(LuaPlayer, getVehicleCount),
    LUNAR_DECLARE_METHOD(LuaPlayer, getVehicleIdList),
    LUNAR_DECLARE_METHOD(LuaPlayer, getVehicleById),
    LUNAR_DECLARE_METHOD(LuaPlayer, getBuildingCount),
    LUNAR_DECLARE_METHOD(LuaPlayer, getBuildingIdList),
    LUNAR_DECLARE_METHOD(LuaPlayer, getBuildingById),
    LUNAR_DECLARE_METHOD(LuaPlayer, setClan),
    LUNAR_DECLARE_METHOD(LuaPlayer, addUnit),
    LUNAR_DECLARE_METHOD(LuaPlayer, addBuilding),
    LUNAR_DECLARE_METHOD(LuaPlayer, setIaScript),
    {0,0}
};

LuaPlayer::LuaPlayer(lua_State *L) :
    m_player(00)
{
    // Only here for Lunar registration, should not be created from Lua code
}

LuaPlayer::LuaPlayer(cPlayer *player) :
    m_player(player)
{
}

int LuaPlayer::getName(lua_State *L)
{
    lua_pushstring(L, getName().c_str());
    return 1;
}

int LuaPlayer::getLandingPosition(lua_State *L)
{
    LuaPosition* lpos = new LuaPosition(m_player->getLandingPosX(), m_player->getLandingPosY());
    Lunar<LuaPosition>::push(L, lpos, true);        // true: use the lua gc
    return 1;
}

int LuaPlayer::setLandingPosition(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isuserdata(L, 1)) {
        LuaPosition* lpos = Lunar<LuaPosition>::check(L, 1);
        m_player->setLandingPos(lpos->x(), lpos->y());
    }
    if (nbParams == 2 && lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        int x = (int)lua_tonumber(L, 1);
        int y = (int)lua_tonumber(L, 2);
        m_player->setLandingPos(x, y);
    }
    return 0;
}

int LuaPlayer::getClan(lua_State *L)
{
    lua_pushinteger(L, m_player->getClan());
    return 1;
}

int LuaPlayer::getVehicleCount(lua_State *L)
{
    lua_pushinteger(L, m_player->getVehicles().size());
    return 1;
}

// Fill a table indexed by iId of units of the player and content is unit type name
int LuaPlayer::getVehicleIdList(lua_State *L)
{
    lua_newtable(L);
    for (auto it = m_player->getVehicles().begin(); it != m_player->getVehicles().end(); ++it) {
        std::shared_ptr<cVehicle> vehicle = *it;
        lua_pushnumber(L, vehicle->iID);
        std::string unitName = UnitsData.m_vehiclesNames.at(vehicle->data.ID);
        lua_pushstring(L, unitName.c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int LuaPlayer::getVehicleById(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams < 1 || !lua_isnumber(L, 1)) return 0;
    unsigned int iID = lua_tointeger(L, 1);
    cVehicle *v = m_player->getVehicleFromId(iID);

    // Build a table with all vehicle data inside
    lua_newtable(L);
    pushUnitData(L, v);

    lua_pushstring(L, "type");
    std::string unitName = UnitsData.m_vehiclesNames.at(v->data.ID);
    lua_pushstring(L, unitName.c_str());
    lua_settable(L, -3);

    return 1;
}

void LuaPlayer::pushUnitData(lua_State *L, cUnit *unit)
{
    lua_pushstring(L, "owner");
    lua_pushstring(L, m_player->getName().c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "pos");
    LuaPosition* lpos = new LuaPosition(unit->getPosition());
    Lunar<LuaPosition>::push(L, lpos, true);        // true: use the lua gc
    lua_settable(L, -3);

    lua_pushstring(L, "disabled");
    lua_pushboolean(L, unit->isDisabled());
    lua_settable(L, -3);

    lua_pushstring(L, "name");
    lua_pushstring(L, unit->getName().c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "disabledTurns");
    lua_pushinteger(L, unit->getDisabledTurns());
    lua_settable(L, -3);

    lua_pushstring(L, "iID");
    lua_pushinteger(L, unit->iID);
    lua_settable(L, -3);

    lua_pushstring(L, "hitPoints");
    lua_pushinteger(L, unit->data.getHitpoints());
    lua_settable(L, -3);

    lua_pushstring(L, "hitPointsMax");
    lua_pushinteger(L, unit->data.hitpointsMax);
    lua_settable(L, -3);

    lua_pushstring(L, "speed");
    lua_pushinteger(L, unit->data.speedCur);
    lua_settable(L, -3);

    lua_pushstring(L, "speedMax");
    lua_pushinteger(L, unit->data.speedMax);
    lua_settable(L, -3);

    lua_pushstring(L, "scan");
    lua_pushinteger(L, unit->data.getScan());
    lua_settable(L, -3);

    lua_pushstring(L, "range");
    lua_pushinteger(L, unit->data.getRange());
    lua_settable(L, -3);

    lua_pushstring(L, "shots");
    lua_pushinteger(L, unit->data.getShots());
    lua_settable(L, -3);

    lua_pushstring(L, "ammo");
    lua_pushinteger(L, unit->data.getAmmo());
    lua_settable(L, -3);

    lua_pushstring(L, "ammoMax");
    lua_pushinteger(L, unit->data.ammoMax);
    lua_settable(L, -3);

    lua_pushstring(L, "damage");
    lua_pushinteger(L, unit->data.getDamage());
    lua_settable(L, -3);

    lua_pushstring(L, "armor");
    lua_pushinteger(L, unit->data.getArmor());
    lua_settable(L, -3);

    lua_pushstring(L, "storedRessources");
    lua_pushinteger(L, unit->data.getStoredResources());
    lua_settable(L, -3);

    lua_pushstring(L, "storedUnits");
    lua_pushinteger(L, unit->data.getStoredUnits());
    lua_settable(L, -3);
}

int LuaPlayer::getBuildingCount(lua_State *L)
{
    lua_pushinteger(L, m_player->getBuildings().size());
    return 1;
}

int LuaPlayer::getBuildingIdList(lua_State *L)
{
    lua_newtable(L);
    for (auto it = m_player->getBuildings().cbegin(); it != m_player->getBuildings().cend(); ++it) {
        std::shared_ptr<cBuilding> building = *it;
        lua_pushnumber(L, building->iID);
        std::string unitName = UnitsData.m_buildingsNames.at(building->data.ID);
        lua_pushstring(L, unitName.c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int LuaPlayer::getBuildingById(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams < 1 || !lua_isnumber(L, 1)) return 0;
    unsigned int iID = lua_tointeger(L, 1);
    cBuilding *b = m_player->getBuildingFromId(iID);

    // Build a table with all vehicle data inside
    lua_newtable(L);
    pushUnitData(L, b);

    lua_pushstring(L, "type");
    std::string unitName = UnitsData.m_buildingsNames.at(b->data.ID);
    lua_pushstring(L, unitName.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "points");
    lua_pushinteger(L, b->points);      // eco-sphere points of the building
    lua_settable(L, -3);

    lua_pushstring(L, "big");
    lua_pushboolean(L, b->data.isBig);
    lua_settable(L, -3);

    return 1;
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

// TODO_M: it would be nice if these 2 could return the iId of the unit just created !
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

int LuaPlayer::setIaScript(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isstring(L, 1)) {
        m_iaScriptName = lua_tostring(L, 1);
    }
    return 0;
}

std::string LuaPlayer::getName() const
{
    return m_player->getName();
}

int LuaPlayer::getClan() const
{
    return m_player->getClan();
}

cPosition LuaPlayer::landingPosition() const
{
    return cPosition(m_player->getLandingPosX(), m_player->getLandingPosY());
}

int LuaPlayer::setClan(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        int clan = (int)lua_tonumber(L, 1);
        if (clan >= 0 && clan < 8) {
            m_player->setClan(clan);
        }
    }
    return 0;
}

void LuaPlayer::sendInformations(const cClient &client)
{
    sendClan(client);
    sendLandingUnits(client, m_landingUnits);
    sendUnitUpgrades(client);
    sendLandingCoords(client, cPosition(m_player->getLandingPosX(), m_player->getLandingPosY()));
    sendReadyToStart(client);
}

