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

#include "drawfxeffect.h"

#include "game/logic/fxeffects.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "utility/position.h"

namespace
{
	//--------------------------------------------------------------------------
	void drawFxMuzzle (UniqueSurface (&images)[2], float zoom, const cPosition& destination, const cFxMuzzle& fx)
	{
		CHECK_SCALING (*images[1], *images[0], zoom);

		SDL_Rect src;
		src.x = (int) (images[0]->w * zoom * fx.getDir() / 8);
		src.y = 0;
		src.w = images[1]->w / 8;
		src.h = images[1]->h;
		SDL_Rect dest = {destination.x(), destination.y(), 0, 0};

		SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
	}

	//--------------------------------------------------------------------------
	void drawFxExplo (UniqueSurface (&images)[2], float zoom, const cPosition& destination, const cFxExplo& fx)
	{
		CHECK_SCALING (*images[1], *images[0], zoom);

		const auto frames = fx.getFrames();
		const int frame = fx.getTick() * frames / fx.getLength();

		SDL_Rect src;
		src.x = (int) (images[0]->w * zoom * frame / frames);
		src.y = 0;
		src.w = images[1]->w / frames;
		src.h = images[1]->h;
		SDL_Rect dest = {destination.x() - static_cast<int> ((images[0]->w / (frames * 2)) * zoom), destination.y() - static_cast<int> ((images[0]->h / 2) * zoom), 0, 0};

		SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
	}

	//--------------------------------------------------------------------------
	void drawFxFade (UniqueSurface (&images)[2], float zoom, const cPosition& destination, const cFxFade& fx)
	{
		CHECK_SCALING (*images[1], *images[0], zoom);

		const int alpha = (fx.alphaEnd - fx.alphaStart) * fx.getTick() / fx.getLength() + fx.alphaStart;
		SDL_SetSurfaceAlphaMod (images[1].get(), alpha);

		SDL_Rect dest = {destination.x() - static_cast<int> ((images[0]->w / 2) * zoom), destination.y() - static_cast<int> ((images[0]->h / 2) * zoom), 0, 0};
		SDL_BlitSurface (images[1].get(), nullptr, cVideo::buffer, &dest);
	}

	//--------------------------------------------------------------------------
	void drawFxRocket (UniqueSurface (&images)[2], float zoom, const cPosition& destination, const cFxRocket& fx)
	{
		//draw smoke effect
		for (auto& subEffect : fx.getSubEffects())
		{
			const cPosition offset = (subEffect->getPixelPosition() - fx.getPixelPosition());
			drawFx (*subEffect, zoom, destination + cPosition (static_cast<int> (offset.x() * zoom), static_cast<int> (offset.y() * zoom)));
		}

		//draw rocket
		if (fx.getTick() >= fx.getLength()) return;
		CHECK_SCALING (*images[1], *images[0], zoom);

		SDL_Rect src;
		src.x = fx.getDir() * images[1]->w / 8;
		src.y = 0;
		src.h = images[1]->h;
		src.w = images[1]->w / 8;
		SDL_Rect dest = {destination.x() - static_cast<int> ((images[0]->w / 16) * zoom), destination.y() - static_cast<int> ((images[0]->h / 2) * zoom), 0, 0};

		SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
	}

	//--------------------------------------------------------------------------
	void drawFxDarkSmoke (UniqueSurface (&images)[2], float zoom, const cPosition& destination, const cFxDarkSmoke& fx)
	{
		CHECK_SCALING (*images[1], *images[0], zoom);

		const auto frames = fx.frames;
		const auto tick = fx.getTick();
		const int frame = tick * frames / fx.getLength();

		SDL_Rect src;
		src.x = (int) (images[0]->w * zoom * frame / frames);
		src.y = 0;
		src.w = images[1]->w / frames;
		src.h = images[1]->h;
		SDL_Rect dest = {destination.x() + static_cast<int> ((tick * fx.getDx()) * zoom), destination.y() + static_cast<int> ((tick * fx.getDy()) * zoom), 0, 0};

		const int alpha = (fx.alphaEnd - fx.alphaStart) * tick / fx.getLength() + fx.alphaStart;
		SDL_SetSurfaceAlphaMod (images[1].get(), alpha);
		SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
	}

	//--------------------------------------------------------------------------
	void drawFxTracks (UniqueSurface (&images)[2], float zoom, const cPosition& destination, const cFxTracks& fx)
	{
		CHECK_SCALING (*images[1], *images[0], zoom);

		const int alpha = (fx.alphaEnd - fx.alphaStart) * fx.getTick() / fx.getLength() + fx.alphaStart;
		SDL_SetSurfaceAlphaMod (images[1].get(), alpha);

		SDL_Rect src;
		src.y = 0;
		src.x = images[1]->w * fx.dir / 4;
		src.w = images[1]->w / 4;
		src.h = images[1]->h;
		SDL_Rect dest = {destination.x(), destination.y(), 0, 0};

		SDL_BlitSurface (images[1].get(), &src, cVideo::buffer, &dest);
	}

	//--------------------------------------------------------------------------
	class cFxDrawerVisitor : public IFxVisitor
	{
		float zoom;
		const cPosition& screenPosition;

	public:
		cFxDrawerVisitor (float zoom, const cPosition& screenPosition) :
			zoom (zoom), screenPosition (screenPosition) {}

		//----------------------------------------------------------------------
		void visit (const cFxMuzzleBig& fx) override
		{
			drawFxMuzzle (EffectsData.fx_muzzle_big, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxMuzzleMed& fx) override
		{
			drawFxMuzzle (EffectsData.fx_muzzle_med, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxMuzzleMedLong& fx) override
		{
			drawFxMuzzle (EffectsData.fx_muzzle_med, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxMuzzleSmall& fx) override
		{
			drawFxMuzzle (EffectsData.fx_muzzle_small, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploAir& fx) override
		{
			drawFxExplo (EffectsData.fx_explo_air, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploBig& fx) override
		{
			drawFxExplo (EffectsData.fx_explo_big, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploSmall& fx) override
		{
			drawFxExplo (EffectsData.fx_explo_small, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploWater& fx) override
		{
			drawFxExplo (EffectsData.fx_explo_water, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxAbsorb& fx) override
		{
			drawFxExplo (EffectsData.fx_absorb, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxRocket& fx) override
		{
			drawFxRocket (EffectsData.fx_rocket, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxCorpse& fx) override
		{
			drawFxFade (EffectsData.fx_corpse, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxSmoke& fx) override
		{
			drawFxFade (EffectsData.fx_smoke, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxDarkSmoke& fx) override
		{
			drawFxDarkSmoke (EffectsData.fx_dark_smoke, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxHit& fx) override
		{
			drawFxExplo (EffectsData.fx_hit, zoom, screenPosition, fx);
		}
		//----------------------------------------------------------------------
		void visit (const cFxTracks& fx) override
		{
			drawFxTracks (EffectsData.fx_tracks, zoom, screenPosition, fx);
		}
	};
} // namespace

//------------------------------------------------------------------------------
void drawFx (const cFx& fx, float zoom, const cPosition& screenPosition)
{
	cFxDrawerVisitor visitor{zoom, screenPosition};
	fx.accept (visitor);
}
