/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SDLutility_autosurfaceH
#define SDLutility_autosurfaceH

#include <SDL.h>
#include <memory>

namespace detail
{

	struct SdlRendererDeleter
	{
		void operator() (SDL_Renderer* renderer) const
		{
			SDL_DestroyRenderer (renderer);
		}
	};

	struct SdlSurfaceDeleter
	{
		void operator() (SDL_Surface* surface) const
		{
			SDL_FreeSurface (surface);
		}
	};

	struct SdlTextureDeleter
	{
		void operator() (SDL_Texture* texture) const
		{
			SDL_DestroyTexture (texture);
		}
	};


} // namespace detail

using UniqueRenderer = std::unique_ptr<SDL_Renderer, detail::SdlRendererDeleter>;
using AutoSurface = std::unique_ptr<SDL_Surface, detail::SdlSurfaceDeleter>;
using UniqueTexture = std::unique_ptr<SDL_Texture, detail::SdlTextureDeleter>;

/* Prevent accidentally freeing the SDL_Surface owned by an AutoSurface */
void SDL_DestroyRenderer (const UniqueRenderer&) = delete;
void SDL_DestroyTexture (const UniqueTexture&) = delete;
void SDL_FreeSurface (const AutoSurface&) = delete;

#endif
