TEMPLATE = app
TARGET = maxr
INCLUDEPATH += src

# SDL
INCLUDEPATH += C:/SDL2-2.0.3/include
LIBS += -LC:/SDL2-2.0.3/lib/x86 -lSDL2 -lSDL2main
INCLUDEPATH += C:/SDL2_net-2.0.0/include
LIBS += -LC:/SDL2_net-2.0.0/lib/x86 -lSDL2_net
INCLUDEPATH += C:/SDL2_mixer-2.0.0/include
LIBS += -LC:/SDL2_mixer-2.0.0/lib/x86 -lSDL2_mixer

# remove entry point of qt because SDL has its own
release:QMAKE_LIBS_QT_ENTRY -= -lqtmain
debug:QMAKE_LIBS_QT_ENTRY -= -lqtmaind

# missing library for windows
win32: LIBS += -lshell32

# Define to avoid min/max macro of windows.h to clash with std
DEFINES += NOMINMAX

HEADERS += $$files(src/*.h) \
    src/game/startup/local/scenario/luaintelligence.h
SOURCES += $$files(src/*.cpp) \
    src/game/startup/local/scenario/luaintelligence.cpp
SOURCES += $$files(src/*.c)

HEADERS += $$files(src/config/*.h)
HEADERS += $$files(src/config/compiler/*.h)
HEADERS += $$files(src/config/platform/*.h)
HEADERS += $$files(src/config/stdlib/*.h)
HEADERS += $$files(src/config/workaround/*.h)

HEADERS += $$files(src/events/*.h)
SOURCES += $$files(src/events/*.cpp)

HEADERS += $$files(src/game/logic/*.h)
SOURCES += $$files(src/game/logic/*.cpp)
HEADERS += $$files(src/game/data/base/*.h)
SOURCES += $$files(src/game/data/base/*.cpp)
HEADERS += $$files(src/game/data/map/*.h)
SOURCES += $$files(src/game/data/map/*.cpp)
HEADERS += $$files(src/game/data/player/*.h)
SOURCES += $$files(src/game/data/player/*.cpp)
HEADERS += $$files(src/game/data/report/*.h)
SOURCES += $$files(src/game/data/report/*.cpp)
HEADERS += $$files(src/game/data/report/special/*.h)
SOURCES += $$files(src/game/data/report/special/*.cpp)
HEADERS += $$files(src/game/data/report/unit/*.h)
SOURCES += $$files(src/game/data/report/unit/*.cpp)
HEADERS += $$files(src/game/data/units/*.h)
SOURCES += $$files(src/game/data/units/*.cpp)
HEADERS += $$files(src/game/startup/*.h)
HEADERS += $$files(src/game/startup/local/hotseat/*.h)
SOURCES += $$files(src/game/startup/local/hotseat/*.cpp)
HEADERS += $$files(src/game/startup/local/singleplayer/*.h)
SOURCES += $$files(src/game/startup/local/singleplayer/*.cpp)
HEADERS += $$files(src/game/startup/local/scenario/*.h)
SOURCES += $$files(src/game/startup/local/scenario/*.cpp)
HEADERS += $$files(src/game/startup/network/client/*.h)
SOURCES += $$files(src/game/startup/network/client/*.cpp)
HEADERS += $$files(src/game/startup/network/host/*.h)
SOURCES += $$files(src/game/startup/network/host/*.cpp)

HEADERS += $$files(src/input/keyboard/*.h)
SOURCES += $$files(src/input/keyboard/*.cpp)
HEADERS += $$files(src/input/mouse/*.h)
SOURCES += $$files(src/input/mouse/*.cpp)
HEADERS += $$files(src/input/mouse/cursor/*.h)
SOURCES += $$files(src/input/mouse/cursor/*.cpp)

HEADERS += $$files(src/output/sound/*.h)
SOURCES += $$files(src/output/sound/*.cpp)

HEADERS += $$files(src/ui/graphical/*.h)
SOURCES += $$files(src/ui/graphical/*.cpp)
HEADERS += $$files(src/ui/graphical/game/*.h)
SOURCES += $$files(src/ui/graphical/game/*.cpp)
HEADERS += $$files(src/ui/graphical/game/animations/*.h)
SOURCES += $$files(src/ui/graphical/game/animations/*.cpp)
HEADERS += $$files(src/ui/graphical/game/control/*.h)
SOURCES += $$files(src/ui/graphical/game/control/*.cpp)
HEADERS += $$files(src/ui/graphical/game/temp/*.h)
SOURCES += $$files(src/ui/graphical/game/temp/*.cpp)
HEADERS += $$files(src/ui/graphical/game/widgets/*.h)
SOURCES += $$files(src/ui/graphical/game/widgets/*.cpp)
HEADERS += $$files(src/ui/graphical/game/widgets/mouseaction/*.h)
SOURCES += $$files(src/ui/graphical/game/widgets/mouseaction/*.cpp)
HEADERS += $$files(src/ui/graphical/game/widgets/mousemode/*.h)
SOURCES += $$files(src/ui/graphical/game/widgets/mousemode/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/control/*.h)
SOURCES += $$files(src/ui/graphical/menu/control/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/dialogs/*.h)
SOURCES += $$files(src/ui/graphical/menu/dialogs/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/widgets/*.h)
SOURCES += $$files(src/ui/graphical/menu/widgets/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/widgets/special/*.h)
SOURCES += $$files(src/ui/graphical/menu/widgets/special/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/widgets/tools/*.h)
SOURCES += $$files(src/ui/graphical/menu/widgets/tools/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowadvancedhangar/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowadvancedhangar/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowbuildbuildings/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowbuildbuildings/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowbuildvehicles/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowbuildvehicles/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowclanselection/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowclanselection/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowgamesettings/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowgamesettings/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowhangar/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowhangar/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowlandingpositionselection/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowlandingpositionselection/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowlandingunitselection/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowlandingunitselection/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowload/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowload/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowloadsave/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowloadsave/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowmapselection/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowmapselection/*.cpp)
SOURCES += $$files(src/ui/graphical/menu/windows/windownetworklobby/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windownetworklobby/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windownetworklobbyclient/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windownetworklobbyclient/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windownetworklobbyhost/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windownetworklobbyhost/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowplayerselection/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowplayerselection/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowreports/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowreports/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowresourcedistribution/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowresourcedistribution/*.h)
HEADERS += $$files(src/ui/graphical/menu/windows/windowstorage/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowstorage/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowscenario/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowscenario/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowunitinfo/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowunitinfo/*.cpp)
HEADERS += $$files(src/ui/graphical/menu/windows/windowupgrades/*.h)
SOURCES += $$files(src/ui/graphical/menu/windows/windowupgrades/*.cpp)

HEADERS += $$files(src/ui/sound/*.h)
SOURCES += $$files(src/ui/sound/*.cpp)
HEADERS += $$files(src/ui/sound/effects/*.h)
SOURCES += $$files(src/ui/sound/effects/*.cpp)

HEADERS += $$files(src/utility/*.h)
SOURCES += $$files(src/utility/*.cpp)
HEADERS += $$files(src/utility/signal/*.h)
SOURCES += $$files(src/utility/signal/*.cpp)
HEADERS += $$files(src/utility/signal/novariadic/*.h)
SOURCES += $$files(src/utility/signal/novariadic/*.cpp)
HEADERS += $$files(src/utility/string/*.h)
SOURCES += $$files(src/utility/string/*.cpp)

HEADERS += $$files(src/lua/*.h)
SOURCES += $$files(src/lua/*.c)
