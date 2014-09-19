#ifndef LUAINTELLIGENCE_H
#define LUAINTELLIGENCE_H

#include <memory>
#include <string>

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "lua/lunar.h"

class cClient;
struct lua_State;

class cLuaIntelligence
{
public:
    static const char className[];
    static Lunar<cLuaIntelligence>::RegType methods[];

public:
    cLuaIntelligence(std::shared_ptr<cClient> client);
    cLuaIntelligence(lua_State *);
    ~cLuaIntelligence();
    std::string openLuaFile(std::string luaFilename);

    // Lua interface
    int getSettings(lua_State *L);
    int move(lua_State *L);

    // SIGNALS
public:
    mutable cSignal<void (const std::string&)> showMessage;

    // SLOTS
private:
    void newTurn();
    void moveJobsFinished();

private:
    cSignalConnectionManager signalConnectionManager;
    std::shared_ptr<cClient> client;
    lua_State* L;
};

#endif // LUAINTELLIGENCE_H
