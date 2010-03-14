CONFIG ?= config.default
-include $(CONFIG)


BINARY ?= maxr

SDL_CONFIG  ?= sdl-config
ifndef CFLAGS_SDL
CFLAGS_SDL  := $(shell $(SDL_CONFIG) --cflags)
endif
ifndef LDFLAGS_SDL
LDFLAGS_SDL := $(shell $(SDL_CONFIG) --libs)
endif

CFLAGS += $(CFLAGS_SDL)

CCFLAGS += $(CFLAGS)

CXXFLAGS += $(CFLAGS)

LDFLAGS += $(LDFLAGS_SDL)
LDFLAGS += -lSDL_mixer
LDFLAGS += -lSDL_net

SRCS :=
SRCS += SDL_flic.c
SRCS += attackJobs.cpp
SRCS += automjobs.cpp
SRCS += base.cpp
SRCS += buildings.cpp
SRCS += clans.cpp
SRCS += client.cpp
SRCS += clientevents.cpp
SRCS += dialog.cpp
SRCS += drawingcache.cpp
SRCS += events.cpp
SRCS += extendedtinyxml.cpp
SRCS += files.cpp
SRCS += hud.cpp
SRCS += input.cpp
SRCS += keys.cpp
SRCS += language.cpp
SRCS += loaddata.cpp
SRCS += log.cpp
SRCS += main.cpp
SRCS += map.cpp
SRCS += mapdownload.cpp
SRCS += menuevents.cpp
SRCS += menuitems.cpp
SRCS += menus.cpp
SRCS += mouse.cpp
SRCS += movejobs.cpp
SRCS += mveplayer.cpp
SRCS += netmessage.cpp
SRCS += network.cpp
SRCS += pcx.cpp
SRCS += player.cpp
SRCS += savegame.cpp
SRCS += server.cpp
SRCS += serverevents.cpp
SRCS += settings.cpp
SRCS += sound.cpp
SRCS += tinystr.cpp
SRCS += tinyxml.cpp
SRCS += tinyxmlerror.cpp
SRCS += tinyxmlparser.cpp
SRCS += unifonts.cpp
SRCS += upgradecalculator.cpp
SRCS += vehicles.cpp
SRCS += video.cpp

OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

.SUFFIXES:
.SUFFIXES: .c .cpp .d .o

Q ?= @

all: $(BINARY)

-include $(DEPS)

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CCFLAGS) -c -MMD -o $@ $<

.cpp.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<

clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(DEPS) $(OBJS) $(BINARY)
