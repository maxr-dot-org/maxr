AC_INIT([maxr], [0.2.4], [bugs@maxr.org])
AC_PREREQ(2.61)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_PROG_SED
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_LIBTOOL
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_CHECK_PROGS([SDLCONFIG], [sdl-config], [:])
if test "$SDLCONFIG" = :; then
	AC_MSG_ERROR([This package needs sdl-config.])
fi

#check where ssdl geaders are located
LIBSDL_CFLAGS="`$SDLCONFIG --cflags`"
AC_SUBST(LIBSDL_CFLAGS)

AC_CHECK_LIB([SDL],
             [SDL_Init],
	     [if test -n "$LDPREFIX" -a -r `$SDLCONFIG --prefix`/lib/libSDL.la
then SDL_LIBS=`$SDLCONFIG --prefix`/lib/libSDL.la
else SDL_LIBS=-lSDL
fi],
        [AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])])
AC_CHECK_HEADER([SDL/SDL.h],,AC_MSG_ERROR([Cannot find headers for SDL]))


AC_CHECK_LIB([SDL_mixer],
             [Mix_OpenAudio],
	     [if test -n "$LDPREFIX" -a -r `$SDLCONFIG --prefix`/lib/libSDL_mixer.la
then SDL_MIXER_LIBS=`$SDLCONFIG --prefix`/lib/libSDL_mixer.la
else SDL_MIXER_LIBS=-lSDL_mixer
fi],
             [AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])])
AC_CHECK_HEADER([SDL/SDL_mixer.h],,AC_MSG_ERROR([Cannot find headers for SDL_mixer]))

AC_CHECK_LIB([SDL_net],
             [SDLNet_Init],
	     [if test -n "$LDPREFIX" -a -r `$SDLCONFIG --prefix`/lib/libSDL_net.la
then SDL_NET_LIBS=`$SDLCONFIG --prefix`/lib/libSDL_net.la
else SDL_NET_LIBS=-lSDL_net
fi],
             [AC_MSG_ERROR([*** SDL_net lib not found! Get SDL_net from
http://www.libsdl.org/projects/SDL_net/index.html])])
AC_CHECK_HEADER([SDL/SDL_net.h],,AC_MSG_ERROR([Cannot find headers for SDL_net]))

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_NET_LIBS])


AC_OUTPUT

