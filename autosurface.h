#ifndef AUTOSURFACE_H
#define AUTOSURFACE_H

#include <memory>
#include <SDL.h>

#include "maxrconfig.h"

namespace detail {

struct SdlSurfaceDeleter
{
	void operator()(SDL_Surface* surface)
	{
		SDL_FreeSurface (surface);
	}
};

}

typedef std::unique_ptr<SDL_Surface, detail::SdlSurfaceDeleter> AutoSurface;

/* Prevent accidentally freeing the SDL_Surface owned by an AutoSurface */
void SDL_FreeSurface (const AutoSurface&) MAXR_DELETE_FUNCTION;

#endif
