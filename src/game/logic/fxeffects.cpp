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

#include <SDL.h>

#include "game/logic/fxeffects.h"

#include "game/logic/client.h"
#include "main.h"
#include "game/data/player/player.h"
#include "video.h"
#include "sound.h"
#include "game/data/map/map.h"
#include "utility/random.h"
#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectposition.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"

cFx::cFx (bool bottom_, const cPosition& position_) :
	position (position_),
	tick (0),
	length (-1),
	bottom (bottom_)
{}

cFx::~cFx()
{}

int cFx::getLength() const
{
	return length;
}

const cPosition& cFx::getPosition()
{
	return position;
}

bool cFx::isFinished() const
{
	return tick >= length;
}

void cFx::playSound (cSoundManager& /*soundManager*/) const
{}

void cFx::run()
{
	tick++;
}

//------------------------------------------------------------------------------
void cFxContainer::push_back (std::shared_ptr<cFx> fx)
{
	fxs.push_back (std::move (fx));
}

void cFxContainer::push_front (std::shared_ptr<cFx> fx)
{
	fxs.insert (fxs.begin(), std::move (fx));
}

size_t cFxContainer::size() const
{
	return fxs.size();
}

void cFxContainer::run()
{
	for (auto it = fxs.begin(); it != fxs.end();)
	{
		auto& fx = * (*it);

		fx.run();
		if (fx.isFinished())
		{
			it = fxs.erase (it);
		}
		else ++it;
	}
}

//------------------------------------------------------------------------------
cFxMuzzle::cFxMuzzle (const cPosition& position_, int dir_, sID id_) :
	cFx (false, position_),
	pImages (nullptr),
	dir (dir_),
	id (id_)
{}

void cFxMuzzle::draw (float zoom, const cPosition& destination) const
{
	if (pImages == nullptr) return;
	AutoSurface (&images) [2] (*pImages);
	CHECK_SCALING (*images[1], *images[0], zoom);

	SDL_Rect src;
	src.x = (int) (images[0]->w * zoom * dir / 8);
	src.y = 0;
	src.w = images[1]->w / 8;
	src.h = images[1]->h;
	SDL_Rect dest = {destination.x(), destination.y(), 0, 0};

	SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
}

void cFxMuzzle::playSound (cSoundManager& soundManager) const
{
	if (id.isABuilding())
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, UnitsData.getBuildingUI (id)->Attack, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
	else
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, UnitsData.getVehicleUI (id)->Attack, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
}

//------------------------------------------------------------------------------
cFxMuzzleBig::cFxMuzzleBig (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	pImages = &EffectsData.fx_muzzle_big;
	length = 6;
}

//------------------------------------------------------------------------------
cFxMuzzleMed::cFxMuzzleMed (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	pImages = &EffectsData.fx_muzzle_med;
	length = 6;
}

//------------------------------------------------------------------------------
cFxMuzzleMedLong::cFxMuzzleMedLong (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	length = 16;
	pImages = &EffectsData.fx_muzzle_med;
}

//------------------------------------------------------------------------------
cFxMuzzleSmall::cFxMuzzleSmall (const cPosition& position_, int dir_, sID id_) :
	cFxMuzzle (position_, dir_, id_)
{
	length = 6;
	pImages = &EffectsData.fx_muzzle_small;
}

//------------------------------------------------------------------------------
cFxExplo::cFxExplo (const cPosition& position_, int frames_) :
	cFx (false, position_),
	pImages (nullptr),
	frames (frames_)
{}

void cFxExplo::draw (float zoom, const cPosition& destination) const
{
	if (!pImages) return;
	AutoSurface (&images) [2] (*pImages);
	CHECK_SCALING (*images[1], *images[0], zoom);

	const int frame = tick * frames / length;

	SDL_Rect src;
	src.x = (int) (images[0]->w * zoom * frame / frames);
	src.y = 0;
	src.w = images[1]->w / frames;
	src.h = images[1]->h;
	SDL_Rect dest = {destination.x() - static_cast<int> ((images[0]->w / (frames * 2)) * zoom), destination.y() - static_cast<int> ((images[0]->h / 2) * zoom), 0, 0};

	SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
}

//------------------------------------------------------------------------------
cFxExploSmall::cFxExploSmall (const cPosition& position_) :
	cFxExplo (position_, 14)
{
	length = 140;
	pImages = &EffectsData.fx_explo_small;
}

void cFxExploSmall::playSound (cSoundManager& soundManager) const
{
	soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPSmall), cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
}

//------------------------------------------------------------------------------
cFxExploBig::cFxExploBig (const cPosition& position_, bool onWater_) :
	cFxExplo (position_, 28),
	onWater (onWater_)
{
	length = 280;
	pImages = &EffectsData.fx_explo_big;
}


void cFxExploBig::playSound (cSoundManager& soundManager) const
{
	if (onWater)
	{
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPBigWet), cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
	}
	else
	{
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPBig), cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
	}
}

//------------------------------------------------------------------------------
cFxExploAir::cFxExploAir (const cPosition& position_) :
	cFxExplo (position_, 14)
{
	length = 140;
	pImages = &EffectsData.fx_explo_air;
}

void cFxExploAir::playSound (cSoundManager& soundManager) const
{
	soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPSmall), cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
}

//------------------------------------------------------------------------------
cFxExploWater::cFxExploWater (const cPosition& position_) :
	cFxExplo (position_, 14)
{
	length = 140;
	pImages = &EffectsData.fx_explo_water;
}

void cFxExploWater::playSound (cSoundManager& soundManager) const
{
	soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPSmallWet), cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
}

//------------------------------------------------------------------------------
cFxHit::cFxHit (const cPosition& position_, bool targetHit_, bool big_) :
	cFxExplo (position_, 5),
	targetHit (targetHit_),
	big (big_)
{
	length = 50;
	pImages = &EffectsData.fx_hit;
}

void cFxHit::playSound (cSoundManager& soundManager) const
{
	if (targetHit)
	{
		if (big)
			soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDHitLarge, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
		else
			soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDHitMed, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
	}
	else
	{
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDHitSmall, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
	}
}

//------------------------------------------------------------------------------
cFxAbsorb::cFxAbsorb (const cPosition& position_) :
	cFxExplo (position_, 10)
{
	length = 100;
	pImages = &EffectsData.fx_absorb;
}

void cFxAbsorb::playSound (cSoundManager& soundManager) const
{
	soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDAbsorb, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
}

//------------------------------------------------------------------------------
cFxFade::cFxFade (const cPosition& position_, bool bottom, int start, int end) :
	cFx (bottom, position_),
	pImages (nullptr),
	alphaStart (start),
	alphaEnd (end)
{}

void cFxFade::draw (float zoom, const cPosition& destination) const
{
	if (!pImages) return;
	AutoSurface (&images) [2] (*pImages);
	CHECK_SCALING (*images[1], *images[0], zoom);

	const int alpha = (alphaEnd - alphaStart) * tick / length + alphaStart;
	SDL_SetSurfaceAlphaMod (images[1].get(), alpha);

	SDL_Rect dest = {destination.x() - static_cast<int> ((images[0]->w / 2) * zoom), destination.y() - static_cast<int> ((images[0]->h / 2) * zoom), 0, 0};
	SDL_BlitSurface (images[1].get(), nullptr, cVideo::buffer, &dest);
}

//------------------------------------------------------------------------------
cFxSmoke::cFxSmoke (const cPosition& position_, bool bottom) :
	cFxFade (position_, bottom, 100, 0)
{
	length = 50;
	pImages = &EffectsData.fx_smoke;
}

//------------------------------------------------------------------------------
cFxCorpse::cFxCorpse (const cPosition& position_) :
	cFxFade (position_, true, 255, 0)
{
	length = 1024;
	pImages = &EffectsData.fx_corpse;
}

//------------------------------------------------------------------------------
cFxTracks::cFxTracks (const cPosition& position_, int dir_) :
	cFx (true, position_),
	pImages (nullptr),
	alphaStart (100),
	alphaEnd (0),
	dir (dir_)
{
	length = 1024;
	pImages = &EffectsData.fx_tracks;
}

void cFxTracks::draw (float zoom, const cPosition& destination) const
{
	if (!pImages) return;
	AutoSurface (&images) [2] (*pImages);
	CHECK_SCALING (*images[1], *images[0], zoom);

	const int alpha = (alphaEnd - alphaStart) * tick / length + alphaStart;
	SDL_SetSurfaceAlphaMod (images[1].get(), alpha);

	SDL_Rect src;
	src.y = 0;
	src.x = images[1]->w * dir / 4;
	src.w = images[1]->w / 4;
	src.h = images[1]->h;
	SDL_Rect dest = {destination.x(), destination.y(), 0, 0};

	SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
}

//------------------------------------------------------------------------------
cFxRocket::cFxRocket (const cPosition& startPosition_, const cPosition& endPosition_, int dir_, bool bottom, sID id_) :
	cFx (bottom, startPosition_),
	speed (8),
	pImages (&EffectsData.fx_rocket),
	dir (dir_),
	distance (0),
	startPosition (startPosition_),
	endPosition (endPosition_),
	id (id_)
{
	distance = static_cast<int> ((endPosition - startPosition).l2Norm());
	length = distance / speed;
}

void cFxRocket::playSound (cSoundManager& soundManager) const
{
	if (id.isABuilding())
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, UnitsData.getBuildingUI (id)->Attack, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
	else
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, UnitsData.getVehicleUI (id)->Attack, cPosition (position.x() / cStaticMap::tilePixelWidth, position.y() / cStaticMap::tilePixelHeight)));
}

cFxRocket::~cFxRocket()
{
	for (size_t i = 0; i != subEffects.size(); ++i)
		delete subEffects[i];
}

void cFxRocket::draw (float zoom, const cPosition& destination) const
{
	//draw smoke effect
	for (unsigned i = 0; i < subEffects.size(); i++)
	{
		const cPosition offset = (subEffects[i]->getPosition() - position);
		subEffects[i]->draw (zoom, destination + cPosition (static_cast<int> (offset.x() * zoom), static_cast<int> (offset.y() * zoom)));
	}

	//draw rocket
	if (!pImages) return;
	if (tick >= length) return;
	AutoSurface (&images) [2] (*pImages);
	CHECK_SCALING (*images[1], *images[0], zoom);

	SDL_Rect src;
	src.x = dir * images[1]->w / 8;
	src.y = 0;
	src.h = images[1]->h;
	src.w = images[1]->w / 8;
	SDL_Rect dest = {destination.x() - static_cast<int> ((images[0]->w / 16) * zoom), destination.y() - static_cast<int> ((images[0]->h / 2) * zoom), 0, 0};

	SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
}

void cFxRocket::run()
{
	//run smoke effect
	for (unsigned i = 0; i < subEffects.size(); i++)
	{
		subEffects[i]->run();
		if (subEffects[i]->isFinished())
		{
			delete subEffects[i];
			subEffects.erase (subEffects.begin() + i);
			i--;
		}
	}

	//add new smoke
	if (tick >= length) return;
	if (cSettings::getInstance().isAlphaEffects())
	{
		subEffects.push_back (new cFxSmoke (position, bottom));
	}

	//update rocket position
	tick++;
	position = startPosition + (endPosition - startPosition) * speed * tick / distance;
}

bool cFxRocket::isFinished() const
{
	return tick >= length && subEffects.empty();
}

cFxDarkSmoke::cFxDarkSmoke (const cPosition& position_, int alpha, float windDir) :
	cFx (false, position_),
	dx (0),
	dy (0),
	alphaStart (alpha),
	alphaEnd (0),
	frames (50),
	pImages (&EffectsData.fx_dark_smoke)
{
	length = 200;

	const float ax = abs (sinf (windDir));
	const float ay = abs (cosf (windDir));
	if (ax > ay)
	{
		dx = (ax +  random (5)       / 20.0f) / 2.f;
		dy = (ay + (random (15) - 7) / 28.0f) / 2.f;
	}
	else
	{
		dx = (ax + (random (15) - 7) / 28.0f) / 2.f;
		dy = (ay +  random (5)       / 20.0f) / 2.f;
	}
}

void cFxDarkSmoke::draw (float zoom, const cPosition& destination) const
{
	//if (!client.getActivePlayer().ScanMap[posX / 64 + posY / 64 * client.getMap()->size]) return;
	if (!pImages) return;
	AutoSurface (&images) [2] (*pImages);
	CHECK_SCALING (*images[1], *images[0], zoom);

	const int frame = tick * frames / length;

	SDL_Rect src;
	src.x = (int) (images[0]->w * zoom * frame / frames);
	src.y = 0;
	src.w = images[1]->w / frames;
	src.h = images[1]->h;
	SDL_Rect dest = {destination.x() + static_cast<int> ((tick * dx) * zoom), destination.y() + static_cast<int> ((tick * dy) * zoom), 0, 0};

	const int alpha = (alphaEnd - alphaStart) * tick / length + alphaStart;
	SDL_SetSurfaceAlphaMod (images[1].get(), alpha);
	SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
}
