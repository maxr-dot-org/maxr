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
#include "game/data/player/playerbasicdata.h"

class cApplication;
class cStaticMap;
class cGameSettings;
class cPlayerBasicData;
class cPlayer;
class cUnitUpgrade;
class cServer;
class cClient;
class cLuaGame;
struct lua_State;
class cWindowClanSelection;
class cLuaIntelligence;

enum StartingStatus {
    WaitingHuman,
    Ready,
    Cancelled
};

class cLocalScenarioGame : public cGame
{
public:
    cLocalScenarioGame(cApplication* application);
    ~cLocalScenarioGame();

    // From cGame
    virtual void run () MAXR_OVERRIDE_FUNCTION;
    virtual void save (int saveNumber, const std::string& saveName) MAXR_OVERRIDE_FUNCTION;

    void loadLuaScript(std::string luaFilename);
    void setGameSettings(std::shared_ptr<cGameSettings> settings) { gameSettings = settings; }
    void openClanWindow();
    StartingStatus startingStatus() const { return startStatus; }

    bool loadMap(std::string mapName);
    cPlayer *humanPlayer();
    cPlayer *addPlayer(std::string playerName);
    int playerCount() { return players.size(); }
    void setGuiPosition(const cPosition& pos) { guiPosition = pos; }
    unsigned int addUnit(const sID &id, const std::string &playerName, const cPosition &pos);
    unsigned int addBuilding(const sID &id, const std::string &playerName, const cPosition &pos);
    void setPlayerClan(std::string playerName, int clan);
    void startServer();
    void startGame();
    void exit();
    void loadAiScript(std::string playerName, std::string luaFileName);

    const cClient& getClient(int index);

    // Slots:
    void turnChanged();
    void popupMessage(std::string message);

private:
    cApplication* application;
    std::shared_ptr<cLuaGame> luaGame;
    lua_State* L;
    StartingStatus startStatus;
    bool scenarioFinished;
    std::string aiErrMsg;

    cSignalConnectionManager signalConnectionManager;
    std::unique_ptr<cServer> server;
    std::shared_ptr<cClient> guiClient;
    std::vector< std::shared_ptr<cClient> > iaClients;
    std::vector<cPlayerBasicData> players;        // First one is the GUI player
    std::vector< std::shared_ptr<cLuaIntelligence> > intelligences;

    std::vector<std::string> availableMaps;
    std::shared_ptr<cStaticMap> map;

    std::shared_ptr<cGameSettings> gameSettings;
    std::unique_ptr<cGameGuiController> gameGuiController;
    cPosition guiPosition;
    std::shared_ptr<cWindowClanSelection> clanWindow;
};

#endif // LOCALSCENARIOGAME_H
