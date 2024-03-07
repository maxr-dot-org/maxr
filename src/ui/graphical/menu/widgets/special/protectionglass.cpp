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

#include "ui/graphical/menu/widgets/special/protectionglass.h"

#include "output/video/video.h"
#include "resources/uidata.h"
#include "ui/graphical/game/animations/animationtimer.h"

#include <algorithm>
#include <cassert>

//------------------------------------------------------------------------------
cProtectionGlass::cProtectionGlass (const cPosition& position, std::shared_ptr<cAnimationTimer> animationTimer_, double percentClosed_) :
	cWidget (position),
	animationTimer (std::move (animationTimer_)),
	openStep (100. * 10 / (400)), // open in 400 ms
	percentClosed (percentClosed_)
{
	percentClosed = std::max (0., percentClosed);
	percentClosed = std::min (100., percentClosed);

	assert (animationTimer != nullptr);

	resize (cPosition (GraphicsData.gfx_destruction_glas->w, GraphicsData.gfx_destruction_glas->h));
}

//------------------------------------------------------------------------------
void cProtectionGlass::open()
{
	signalConnectionManager.connect (animationTimer->triggered10msCatchUp, [this]() { doOpenStep(); });
}

//------------------------------------------------------------------------------
void cProtectionGlass::draw (SDL_Surface& destination, const cBox<cPosition>&) /* override */
{
	const auto offset = (int) (getSize().y() * (100. - percentClosed) / 100);

	SDL_Rect sourceRect = {0, offset, GraphicsData.gfx_destruction_glas->w, GraphicsData.gfx_destruction_glas->h - offset};

	SDL_Rect destinationRect = {getPosition().x(), getPosition().y(), sourceRect.w, sourceRect.h};

	SDL_BlitSurface (GraphicsData.gfx_destruction_glas.get(), &sourceRect, &destination, &destinationRect);
}

//------------------------------------------------------------------------------
void cProtectionGlass::doOpenStep()
{
	percentClosed -= openStep;

	if (percentClosed <= 0.)
	{
		percentClosed = std::max (0., percentClosed);
		signalConnectionManager.disconnectAll();
		disable();
		hide();
		opened();
	}
}
