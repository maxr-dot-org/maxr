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
SRCS += ajobs.cpp
SRCS += attackJobs.cpp
SRCS += automjobs.cpp
SRCS += base.cpp
SRCS += buildings.cpp
SRCS += buttons.cpp
SRCS += client.cpp
SRCS += clientevents.cpp
SRCS += dialog.cpp
SRCS += engine.cpp
SRCS += events.cpp
SRCS += extendedtinyxml.cpp
SRCS += files.cpp
SRCS += fonts.cpp
SRCS += game.cpp
SRCS += hud.cpp
SRCS += keyinp.cpp
SRCS += keys.cpp
SRCS += language.cpp
SRCS += loaddata.cpp
SRCS += log.cpp
SRCS += main.cpp
SRCS += map.cpp
SRCS += menu.cpp
SRCS += mjobs.cpp
SRCS += mouse.cpp
SRCS += netmessage.cpp
SRCS += network.cpp
SRCS += pcx.cpp
SRCS += player.cpp
SRCS += server.cpp
SRCS += serverevents.cpp
SRCS += sound.cpp
SRCS += tinystr.cpp
SRCS += tinyxml.cpp
SRCS += tinyxmlerror.cpp
SRCS += tinyxmlparser.cpp
SRCS += vehicles.cpp

DEPS = $(filter %.d, $(SRCS:.c=.d) $(SRCS:.cpp=.d))
OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o))

.SUFFIXES:
.SUFFIXES: .c .cpp .d .o

Q ?= @

all: $(BINARY)

ifndef NO_DEPS
depend: $(DEPS)

ifeq ($(findstring $(MAKECMDGOALS), clean depend deinstall distclean),)
-include $(DEPS)
endif
endif

$(BINARY): $(OBJS)
	@echo '===> LD $@'
	$(Q)$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	@echo '===> CC $<'
	$(Q)$(CC) $(CCFLAGS) -c $< -o $@

.cpp.o:
	@echo '===> CXX $<'
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

.c.d:
	@echo '===> DEP $<'
	$(Q)$(CC) $(CCFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@

.cpp.d:
	@echo '===> DEP $<'
	$(Q)$(CXX) $(CXXFLAGS) -MM $< | sed 's#^$(@F:%.d=%.o):#$@ $(@:%.d=%.o):#' > $@


clean distclean:
	@echo '===> CLEAN'
	$(Q)rm -fr $(DEPS) $(OBJS) $(BINARY)
