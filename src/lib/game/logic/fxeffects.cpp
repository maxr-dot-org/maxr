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

#include "game/logic/fxeffects.h"

#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/client.h"
#include "settings.h"
#include "utility/listhelpers.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFx::cFx (bool bottom_, const cPosition& position_) :
	position (position_),
	bottom (bottom_)
{}

//------------------------------------------------------------------------------
bool cFx::isFinished() const
{
	return tick >= length;
}

//------------------------------------------------------------------------------
void cFx::run()
{
	tick++;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void cFxContainer::push_back (std::shared_ptr<cFx> fx)
{
	fxs.push_back (std::move (fx));
}

//------------------------------------------------------------------------------
void cFxContainer::push_front (std::shared_ptr<cFx> fx)
{
	fxs.insert (fxs.begin(), std::move (fx));
}

//------------------------------------------------------------------------------
size_t cFxContainer::size() const
{
	return fxs.size();
}

//------------------------------------------------------------------------------
void cFxContainer::run()
{
	for (auto& fx : fxs)
	{
		fx->run();
	}
	EraseIf (fxs, [] (const auto& fx) { return fx->isFinished(); });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxMuzzle::cFxMuzzle (const cPosition& position_, int dir_, sID id_) :
	cFx (false, position_),
	dir (dir_),
	id (id_)
{}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxMuzzleBig::cFxMuzzleBig (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	length = 6;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxMuzzleMed::cFxMuzzleMed (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	length = 6;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxMuzzleMedLong::cFxMuzzleMedLong (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	length = 16;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxMuzzleSmall::cFxMuzzleSmall (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	length = 6;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxExplo::cFxExplo (const cPosition& position_, int frames_) :
	cFx (false, position_),
	frames (frames_)
{}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxExploSmall::cFxExploSmall (const cPosition& position_) :
	cFxExplo (position_, 14)
{
	length = 140;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxExploBig::cFxExploBig (const cPosition& position_, bool onWater_) :
	cFxExplo (position_, 28),
	onWater (onWater_)
{
	length = 280;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxExploAir::cFxExploAir (const cPosition& position_) :
	cFxExplo (position_, 14)
{
	length = 140;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxExploWater::cFxExploWater (const cPosition& position_) :
	cFxExplo (position_, 14)
{
	length = 140;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxHit::cFxHit (const cPosition& position_, bool targetHit_, bool big_) :
	cFxExplo (position_, 5),
	targetHit (targetHit_),
	big (big_)
{
	length = 50;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxAbsorb::cFxAbsorb (const cPosition& position_) :
	cFxExplo (position_, 10)
{
	length = 100;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxFade::cFxFade (const cPosition& position_, bool bottom, int start, int end) :
	cFx (bottom, position_),
	alphaStart (start),
	alphaEnd (end)
{}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxSmoke::cFxSmoke (const cPosition& position_, bool bottom) :
	cFxFade (position_, bottom, 100, 0)
{
	length = 50;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxCorpse::cFxCorpse (const cPosition& position_) :
	cFxFade (position_, true, 255, 0)
{
	length = 1024;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxTracks::cFxTracks (const cPosition& position_, int dir_) :
	cFx (true, position_),
	dir (dir_)
{
	length = 1024;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxRocket::cFxRocket (const cPosition& startPosition_, const cPosition& endPosition_, int dir_, bool bottom, sID id_) :
	cFx (bottom, startPosition_),
	dir (dir_),
	startPosition (startPosition_),
	endPosition (endPosition_),
	id (id_)
{
	distance = static_cast<int> ((endPosition - startPosition).l2Norm());
	length = distance / speed;
}

//------------------------------------------------------------------------------
void cFxRocket::run()
{
	//run smoke effect
	for (auto& subEffect : subEffects)
	{
		subEffect->run();
	}
	EraseIf (subEffects, [] (const auto& subEffect) { return subEffect->isFinished(); });
	//add new smoke
	if (tick >= length) return;
	if (cSettings::getInstance().isAlphaEffects())
	{
		subEffects.push_back (std::make_unique<cFxSmoke> (position, bottom));
	}

	//update rocket position
	tick++;
	position = startPosition + (endPosition - startPosition) * speed * tick / distance;
}

//------------------------------------------------------------------------------
bool cFxRocket::isFinished() const
{
	return tick >= length && subEffects.empty();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
cFxDarkSmoke::cFxDarkSmoke (const cPosition& position_, int alpha, float windDir) :
	cFx (false, position_),
	alphaStart (alpha)
{
	length = 200;

	const float ax = abs (sinf (windDir));
	const float ay = abs (cosf (windDir));
	if (ax > ay)
	{
		dx = (ax + random (5) / 20.0f) / 2.f;
		dy = (ay + (random (15) - 7) / 28.0f) / 2.f;
	}
	else
	{
		dx = (ax + (random (15) - 7) / 28.0f) / 2.f;
		dy = (ay + random (5) / 20.0f) / 2.f;
	}
}
