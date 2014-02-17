#ifndef AUTOSURFACE_H
#define AUTOSURFACE_H

#include <SDL.h>

#include "autoobj.h"

typedef AutoObj<SDL_Surface, SDL_FreeSurface> AutoSurface;

/* Prevent accidentally freeing the SDL_Surface owned by an AutoSurface */
void SDL_FreeSurface (AutoSurface const&) DELETE_CPP11;

#endif
