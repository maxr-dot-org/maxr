#ifndef LOCALSCENARIOGAME_H
#define LOCALSCENARIOGAME_H

#include <memory>
#include <vector>
#include <utility>

#include "ui/graphical/game/control/gameguicontroller.h"
#include "game/startup/game.h"
#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"
#include "maxrconfig.h"

class cApplication;
class cStaticMap;
class cGameSettings;
class cPlayerBasicData;
class cPlayer;
class cUnitUpgrade;
class cServer;
class cClient;
class LuaGame;
struct lua_State;
class cWindowClanSelection;

enum StartingStatus {
    WaitingHuman,
    Ready,
    Cancelled
};

// TODO_M: where to place translations
// TODO_M: where to place scenarios
class cLocalScenarioGame : public cGame
{
public:
    cLocalScenarioGame(cApplication* application);
    ~cLocalScenarioGame();

    // From cGame
    virtual void run () MAXR_OVERRIDE_FUNCTION;
    virtual void save (int saveNumber, const std::string& saveName) MAXR_OVERRIDE_FUNCTION;

    void loadLuaScript(std::string luaFilename);
    void setGameSettings(std::shared_ptr<cGameSettings> settings) { m_gameSettings = settings; }
    void openClanWindow();
    StartingStatus startingStatus() const { return m_startStatus; }

    bool loadMap(std::string mapName);
    void addPlayer(std::string playerName);
    int playerCount() { return m_players.size(); }
    void setGuiPosition(const cPosition& pos) { m_guiPosition = pos; }
    void addUnit(const sID &id, const std::string &playerName, const cPosition &pos);
    void addBuilding(const sID &id, const std::string &playerName, const cPosition &pos);
    void setPlayerClan(std::string playerName, int clan);
    void startServer();
    void startGame();
    void exit();

    const cClient& getClient(int index);


private:
    cApplication* m_application;
    std::shared_ptr<LuaGame> m_luaGame;
    lua_State* L;
    StartingStatus m_startStatus;

    cSignalConnectionManager m_signalConnectionManager;
    std::unique_ptr<cServer> m_server;
    std::shared_ptr<cClient> m_guiClient;
    std::vector< std::shared_ptr<cClient> > m_iaClients;
    std::vector<cPlayerBasicData> m_players;        // First one is the GUI player

    std::vector<std::string> m_availableMaps;
    std::shared_ptr<cStaticMap> m_map;

    std::shared_ptr<cGameSettings> m_gameSettings;
    std::unique_ptr<cGameGuiController> gameGuiController;
    cPosition m_guiPosition;
    std::shared_ptr<cWindowClanSelection> m_clanWindow;
};

#endif // LOCALSCENARIOGAME_H
