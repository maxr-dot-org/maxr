#include "luaplayer.h"

#include "game/startup/local/scenario/luaposition.h"
#include "game/logic/clientevents.h"
#include "game/data/player/player.h"
#include "game/data/units/unitdata.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "utility/log.h"
#include "main.h"

const char cLuaPlayer::className[] = "LuaPlayer";

Lunar<cLuaPlayer>::RegType cLuaPlayer::methods[] = {
    LUNAR_DECLARE_METHOD(cLuaPlayer, getName),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getLandingPosition),
    LUNAR_DECLARE_METHOD(cLuaPlayer, setLandingPosition),
    LUNAR_DECLARE_METHOD(cLuaPlayer, addLandingUnit),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getClan),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getVehicleCount),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getVehicleIdList),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getVehicleById),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getBuildingCount),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getBuildingIdList),
    LUNAR_DECLARE_METHOD(cLuaPlayer, getBuildingById),
    LUNAR_DECLARE_METHOD(cLuaPlayer, setClan),
    LUNAR_DECLARE_METHOD(cLuaPlayer, addUnit),
    LUNAR_DECLARE_METHOD(cLuaPlayer, addBuilding),
    LUNAR_DECLARE_METHOD(cLuaPlayer, setIaScript),
    {0,0}
};

cLuaPlayer::cLuaPlayer(lua_State *) :
    player(00)
{
    // Only here for Lunar registration, should not be created from Lua code
}

cLuaPlayer::cLuaPlayer(cPlayer *player) :
    player(player)
{
}

int cLuaPlayer::getName(lua_State *L)
{
    lua_pushstring(L, getName().c_str());
    return 1;
}

int cLuaPlayer::getLandingPosition(lua_State *L)
{
    cLuaPosition::pushPosition(L, player->getLandingPosX(), player->getLandingPosY());
    return 1;
}

int cLuaPlayer::setLandingPosition(lua_State *L)
{
    int x, y;
    if (cLuaPosition::getPosition(L, x, y)) {
        player->setLandingPos(x, y);
    }
    return 0;
}

int cLuaPlayer::getClan(lua_State *L)
{
    lua_pushinteger(L, player->getClan());
    return 1;
}

int cLuaPlayer::getVehicleCount(lua_State *L)
{
    lua_pushinteger(L, player->getVehicles().size());
    return 1;
}

// Fill a table indexed by iId of units of the player and content is unit type name
int cLuaPlayer::getVehicleIdList(lua_State *L)
{
    lua_newtable(L);
    for (auto it = player->getVehicles().begin(); it != player->getVehicles().end(); ++it) {
        std::shared_ptr<cVehicle> vehicle = *it;
        lua_pushnumber(L, vehicle->iID);
        std::string unitName = UnitsData.vehiclesNames.at(vehicle->data.ID);
        lua_pushstring(L, unitName.c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int cLuaPlayer::getVehicleById(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams < 1 || !lua_isnumber(L, 1)) return 0;
    unsigned int iID = lua_tointeger(L, 1);
    cVehicle *v = player->getVehicleFromId(iID);
    if (v == 00) return 0;

    // Build a table with all vehicle data inside
    lua_newtable(L);
    pushUnitData(L, v);

    lua_pushstring(L, "type");
    std::string unitName = UnitsData.vehiclesNames.at(v->data.ID);
    lua_pushstring(L, unitName.c_str());
    lua_settable(L, -3);

    return 1;
}

void cLuaPlayer::pushUnitData(lua_State *L, cUnit *unit)
{
    lua_pushstring(L, player->getName().c_str());
    lua_setfield(L, -2, "owner");

    cLuaPosition::pushPosition(L, unit->getPosition());
    lua_setfield(L, -2, "pos");

    lua_pushboolean(L, unit->isDisabled());
    lua_setfield(L, -2, "disabled");

    lua_pushstring(L, unit->getName().c_str());
    lua_setfield(L, -2, "name");

    lua_pushinteger(L, unit->getDisabledTurns());
    lua_setfield(L, -2, "disabledTurns");

    lua_pushinteger(L, unit->iID);
    lua_setfield(L, -2, "iID");

    lua_pushinteger(L, unit->data.getHitpoints());
    lua_setfield(L, -2, "hitPoints");

    lua_pushinteger(L, unit->data.hitpointsMax);
    lua_setfield(L, -2, "hitPointsMax");

    lua_pushinteger(L, unit->data.speedCur);
    lua_setfield(L, -2, "speed");

    lua_pushinteger(L, unit->data.speedMax);
    lua_setfield(L, -2, "speedMax");

    lua_pushinteger(L, unit->data.getScan());
    lua_setfield(L, -2, "scan");

    lua_pushinteger(L, unit->data.getRange());
    lua_setfield(L, -2, "range");

    lua_pushinteger(L, unit->data.getShots());
    lua_setfield(L, -2, "shots");

    lua_pushinteger(L, unit->data.getAmmo());
    lua_setfield(L, -2, "ammo");

    lua_pushinteger(L, unit->data.ammoMax);
    lua_setfield(L, -2, "ammoMax");

    lua_pushinteger(L, unit->data.getDamage());
    lua_setfield(L, -2, "damage");

    lua_pushinteger(L, unit->data.getArmor());
    lua_setfield(L, -2, "armor");

    lua_pushinteger(L, unit->data.getStoredResources());
    lua_setfield(L, -2, "storedRessources");

    lua_pushinteger(L, unit->data.getStoredUnits());
    lua_setfield(L, -2, "storedUnits");
}

int cLuaPlayer::getBuildingCount(lua_State *L)
{
    lua_pushinteger(L, player->getBuildings().size());
    return 1;
}

// Return table {iID, unitName}
int cLuaPlayer::getBuildingIdList(lua_State *L)
{
    lua_newtable(L);
    for (auto it = player->getBuildings().cbegin(); it != player->getBuildings().cend(); ++it) {
        std::shared_ptr<cBuilding> building = *it;
        lua_pushnumber(L, building->iID);
        std::string unitName = UnitsData.buildingsNames.at(building->data.ID);
        lua_pushstring(L, unitName.c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int cLuaPlayer::getBuildingById(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams < 1 || !lua_isnumber(L, 1)) return 0;
    unsigned int iID = lua_tointeger(L, 1);
    cBuilding *b = player->getBuildingFromId(iID);

    // Build a table with all vehicle data inside
    lua_newtable(L);
    pushUnitData(L, b);

    lua_pushstring(L, "type");
    std::string unitName = UnitsData.buildingsNames.at(b->data.ID);
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

int cLuaPlayer::addLandingUnit(lua_State *L)
{
    int nbParams = lua_gettop(L);
    sID unitId;
    int cargo = 0;

    // Id
    if (nbParams > 0 && lua_isstring(L, 1)) {
        // Get the sID from the unit string
        std::string unitName = lua_tostring(L, 1);
        try {
            unitId = UnitsData.vehiclesIDs.at(unitName);
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
    landingUnits.push_back(unit);

    return 0;
}

// TODO_M: it would be nice if these 2 could return the iId of the unit just created !
int cLuaPlayer::addUnit(lua_State *L)
{
    std::string unitName;

    int nbParams = lua_gettop(L);
    if (nbParams >= 2 && lua_isstring(L, 1)) {
        unitName = lua_tostring(L, 1);

        sPlayerUnit pu;
        if (!cLuaPosition::getPosition(L, pu.position)) {
            Log.write("addUnit parameters error ", cLog::eLOG_TYPE_WARNING);
            return 0;
        }

        pu.unitID = UnitsData.vehiclesIDs.at(unitName);
        otherUnits.push_back(pu);
    }

    return 0;
}

int cLuaPlayer::addBuilding(lua_State *L)
{
    std::string buildingName;

    int nbParams = lua_gettop(L);
    if (nbParams >= 2 && lua_isstring(L, 1)) {
        buildingName = lua_tostring(L, 1);

        sPlayerUnit pu;
        if (!cLuaPosition::getPosition(L, pu.position)) {
            Log.write("addBuilding parameters error ", cLog::eLOG_TYPE_WARNING);
            return 0;
        }

        pu.unitID = UnitsData.buildingsIDs.at(buildingName);
        buildings.push_back(pu);
    }

    return 0;
}

int cLuaPlayer::setIaScript(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isstring(L, 1)) {
        iaScriptName = lua_tostring(L, 1);
    }
    return 0;
}

std::string cLuaPlayer::getName() const
{
    return player->getName();
}

int cLuaPlayer::getClan() const
{
    return player->getClan();
}

cPosition cLuaPlayer::landingPosition() const
{
    return cPosition(player->getLandingPosX(), player->getLandingPosY());
}

int cLuaPlayer::setClan(lua_State *L)
{
    int nbParams = lua_gettop(L);
    if (nbParams == 1 && lua_isnumber(L, 1)) {
        int clan = (int)lua_tonumber(L, 1);
        if (clan >= 0 && clan < 8) {
            player->setClan(clan);
        }
    }
    return 0;
}

void cLuaPlayer::sendInformations(const cClient &client)
{
    sendClan(client);
    sendLandingUnits(client, landingUnits);
    sendUnitUpgrades(client);
    sendLandingCoords(client, cPosition(player->getLandingPosX(), player->getLandingPosY()));
    sendReadyToStart(client);
}

