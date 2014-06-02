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

#include <algorithm>

#include "protectionglass.h"
#include "../../../game/temp/animationtimer.h"
#include "../../../../main.h" // GraphicsData
#include "../../../../video.h"

//------------------------------------------------------------------------------
cProtectionGlass::cProtectionGlass (const cPosition& position, std::shared_ptr<cAnimationTimer> animationTimer_, double percentClosed_) :
	cWidget (position),
	animationTimer (std::move(animationTimer_)),
	percentClosed (percentClosed_),
	openStep (100. * 10 / (400)) // open in 400 ms
{
	percentClosed = std::max (0., percentClosed);
	percentClosed = std::min (100., percentClosed);

	assert (animationTimer != nullptr);

	resize (cPosition (GraphicsData.gfx_destruction_glas->w, GraphicsData.gfx_destruction_glas->h));
}

//------------------------------------------------------------------------------
void cProtectionGlass::open ()
{
	signalConnectionManager.connect (animationTimer->triggered10ms, std::bind (&cProtectionGlass::doOpenStep, this));
}

//------------------------------------------------------------------------------
void cProtectionGlass::draw ()
{
	animationTimer->updateAnimationFlags (); // TODO: remove this

	const auto offset = (int)(getSize ().y () * (100. - percentClosed) / 100);

	SDL_Rect source = {0, offset, GraphicsData.gfx_destruction_glas->w, GraphicsData.gfx_destruction_glas->h - offset};

	SDL_Rect destination = {getPosition ().x (), getPosition ().y (), source.w, source.h};
	
	SDL_BlitSurface (GraphicsData.gfx_destruction_glas.get (), &source, cVideo::buffer, &destination);
}

//------------------------------------------------------------------------------
void cProtectionGlass::doOpenStep ()
{
	percentClosed -= openStep;

	if (percentClosed <= 0.)
	{
		percentClosed = std::max (0., percentClosed);
		signalConnectionManager.disconnectAll ();
		disable ();
		hide ();
		opened ();
	}
}