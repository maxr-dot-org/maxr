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

AC_CHECK_PROGS([SDLCONFIG], [sdl-config], [:])
if test "$SDLCONFIG" = :; then
	AC_MSG_ERROR([This package needs sdl-config.])
fi

LIBSDL_CFLAGS="`$SDLCONFIG --cflags`"
AC_SUBST(LIBSDL_CFLAGS)

AC_MSG_CHECKING([for lib SDL])
if test -r `$SDLCONFIG --prefix`/lib/libSDL.la; then
	SDL_LIBS=`$SDLCONFIG --libs`
	AC_MSG_RESULT([yes])
else
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check its in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])
fi

AC_MSG_CHECKING([for lib SDL_net])
if test -r `$SDLCONFIG --prefix`/lib/libSDL_net.la; then
	SDL_NET_LIBS=`$SDLCONFIG --prefix`/lib/libSDL_net.la
	AC_MSG_RESULT([yes])
else
        AC_MSG_RESULT([no])
	AC_MSG_ERROR([*** SDL_net lib not found! Get SDL_net from
http://www.libsdl.org/projects/SDL_net/index.html])
fi

AC_MSG_CHECKING([for lib SDL_mixer])
if test -r `$SDLCONFIG --prefix`/lib/libSDL_mixer.la; then
        SDL_MIXER_LIBS=`$SDLCONFIG --prefix`/lib/libSDL_mixer.la
        AC_MSG_RESULT([yes])
else
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])
fi

AC_CHECK_HEADER([SDL/SDL.h],,AC_MSG_ERROR([Cannot find headers for SDL]))
AC_CHECK_HEADER([SDL/SDL_net.h],,AC_MSG_ERROR([Cannot find headers for SDL_net]))
AC_CHECK_HEADER([SDL/SDL_mixer.h],,AC_MSG_ERROR([Cannot find headers for SDL_mixer]))

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_NET_LIBS])

AC_OUTPUT
