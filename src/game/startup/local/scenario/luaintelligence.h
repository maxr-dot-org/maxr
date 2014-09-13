#ifndef LUAINTELLIGENCE_H
#define LUAINTELLIGENCE_H

#include <memory>
#include <string>

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "lua/lunar.h"

class cClient;
struct lua_State;

class LuaIntelligence
{
public:
    static const char className[];
    static Lunar<LuaIntelligence>::RegType methods[];

public:
    LuaIntelligence(std::shared_ptr<cClient> client);
    LuaIntelligence(lua_State *);
    ~LuaIntelligence();
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
    cSignalConnectionManager m_signalConnectionManager;
    std::shared_ptr<cClient> m_client;
    lua_State* L;
};

#endif // LUAINTELLIGENCE_H
