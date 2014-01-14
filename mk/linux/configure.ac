AC_INIT([maxr], [0.2.8], [bugs@maxr.org])
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

AC_ARG_ENABLE([dedicated],
	AS_HELP_STRING([--enable-dedicated], [Build as dedicted server version]))

AS_IF([test "x$enable_dedicated" = "xyes"], [
        DDEDICATED="-DDEDICATED_SERVER_APPLICATION"
        AC_SUBST(DDEDICATED)
])

AC_ARG_ENABLE([win32],
	AS_HELP_STRING([--enable-win32], [Enable WIN32 flag (needed for cross compiling)]))

AS_IF([test "x$enable_win32" = "xyes"], [
	DWIN="-DWIN32"
	AC_SUBST(DWIN)
])

AC_CHECK_PROGS([SDLCONFIG], [sdl2-config], [:])
if test "$SDLCONFIG" = :; then
	AC_MSG_ERROR([This package needs sdl-config.])
fi

LIBSDL_CFLAGS="`$SDLCONFIG --cflags`"
LIBS="$LIBS `$SDLCONFIG --libs`"
AC_SUBST(LIBSDL_CFLAGS)

AC_CHECK_HEADER([SDL2/SDL.h],,AC_MSG_ERROR([SDL headers not found.]))
AC_CHECK_HEADER(SDL2/SDL_net.h, , AC_MSG_ERROR([SDL_net header not found.]))
AC_CHECK_HEADER(SDL2/SDL_mixer.h, , AC_MSG_ERROR([SDL_mixer header not found.]))

case "$host" in
	i686-pc-mingw32|x86_64-pc-mingw64|i686-w64-mingw32)
		#simply set libs - don't check for usability since I don't know a way of doing so during cross compiling -- beko
		AC_SUBST([SDL_LIBS], [-lSDL2])
		AC_SUBST([SDL_NET_LIBS], [-lSDL2_net])
		AC_SUBST([SDL_MIXER_LIBS], [-lSDL2_mixer])
	;;
	*)
		AC_CHECK_LIB(SDL2, SDL_Init, AC_SUBST([SDL_LIBS], [-lSDL2]), AC_MSG_ERROR([SDL not found.]))
		AC_CHECK_LIB(SDL2_net, SDLNet_Init, AC_SUBST([SDL_NET_LIBS], [-lSDL2_net]), AC_MSG_ERROR([SDL_net not found.]), [${SDL_LIBS}])
		AC_CHECK_LIB(SDL2_mixer, Mix_OpenAudio, AC_SUBST([SDL_MIXER_LIBS], [-lSDL2_mixer]), AC_MSG_ERROR([SDL_mixer not found.]), [${SDL_LIBS}])
	;;
esac

AC_OUTPUT
