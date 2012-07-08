#bin_PROGRAMS = resinstaller

# set the include path found by configure
AM_CPPFLAGS =  $(LIBSDL_CFLAGS) $(all_includes)

# the library search path.
resinstaller_LDFLAGS = $(LIBSDL_RPATH)
resinstaller_LDADD = $(LIBSDL_LIBS) $(VORBIS_LIBS) $(VORBISENC_LIBS)
#-lvorbisenc
resinstaller_SOURCES = SDL_flic.c file.cpp ogg_encode.cpp pcx.cpp wave.cpp \
	converter.cpp resinstaller.cpp
noinst_HEADERS = SDL_flic.h file.h ogg_encode.h pcx.h wave.h \
	converter.h resinstaller.h defines.h palette.h
noinst_PROGRAMS = resinstaller
