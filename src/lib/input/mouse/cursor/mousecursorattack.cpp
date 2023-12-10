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

#include "input/mouse/cursor/mousecursorattack.h"

#include "game/data/map/map.h"
#include "game/data/units/unit.h"
#include "game/logic/attackjob.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "utility/position.h"

#include <cassert>

//------------------------------------------------------------------------------
cMouseCursorAttack::cMouseCursorAttack() :
	currentHealthPercent (-1),
	newHealthPercent (-1),
	inRange (true)
{}

//------------------------------------------------------------------------------
cMouseCursorAttack::cMouseCursorAttack (const cUnit& sourceUnit, const cPosition& targetPosition, const cMapView& map) :
	currentHealthPercent (-1),
	newHealthPercent (-1),
	inRange (sourceUnit.isInRange (targetPosition))
{
	const cUnit* target = cAttackJob::selectTarget (targetPosition, sourceUnit.getStaticUnitData().canAttack, map, sourceUnit.getOwner());

	if (target && (target != &sourceUnit))
	{
		currentHealthPercent = 100 * target->data.getHitpoints() / target->data.getHitpointsMax();
		newHealthPercent = 100 * target->calcHealth (sourceUnit.data.getDamage()) / target->data.getHitpointsMax();
	}
	assert (currentHealthPercent >= newHealthPercent);
}

//------------------------------------------------------------------------------
cMouseCursorAttack::cMouseCursorAttack (int currentHealthPercent_, int newHealthPercent_, bool inRange_) :
	currentHealthPercent (currentHealthPercent_),
	newHealthPercent (newHealthPercent_),
	inRange (inRange_)
{
	assert (currentHealthPercent >= newHealthPercent);
}

//------------------------------------------------------------------------------
SDL_Surface* cMouseCursorAttack::getSurface() const
{
	if (surface == nullptr) generateSurface();
	return surface.get();
}

//------------------------------------------------------------------------------
cPosition cMouseCursorAttack::getHotPoint() const
{
	return cPosition (19, 19);
}

//------------------------------------------------------------------------------
bool cMouseCursorAttack::equal (const cMouseCursor& other) const
{
	auto other2 = dynamic_cast<const cMouseCursorAttack*> (&other);
	return other2 && other2->currentHealthPercent == currentHealthPercent && other2->newHealthPercent == newHealthPercent && other2->inRange == inRange;
}

//------------------------------------------------------------------------------
void cMouseCursorAttack::generateSurface() const
{
	SDL_Surface* sourceSurface = inRange ? GraphicsData.gfx_Cattack.get() : GraphicsData.gfx_Cattackoor.get();
	surface = UniqueSurface (SDL_CreateRGBSurface (0, sourceSurface->w, sourceSurface->h, Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	SDL_BlitSurface (sourceSurface, nullptr, surface.get(), nullptr);

	const int barWidth = 35;

	if (currentHealthPercent < 0 || currentHealthPercent > 100 || newHealthPercent < 0 || newHealthPercent > 100)
	{
		SDL_Rect rect = {1, 29, barWidth, 3};
		SDL_FillRect (surface.get(), &rect, 0);
	}
	else
	{
		const auto currentHealthWidth = static_cast<int> (currentHealthPercent / 100. * barWidth);
		const auto newHealthWidth = static_cast<int> (newHealthPercent / 100. * barWidth);

		SDL_Rect rect = {1, 29, newHealthWidth, 3};
		SDL_FillRect (surface.get(), &rect, 0x00FF00);

		rect.x += rect.w;
		rect.w = currentHealthWidth - newHealthWidth;

		SDL_FillRect (surface.get(), &rect, 0xFF0000);

		rect.x += rect.w;
		rect.w = barWidth - currentHealthWidth;

		SDL_FillRect (surface.get(), &rect, 0);
	}
}
