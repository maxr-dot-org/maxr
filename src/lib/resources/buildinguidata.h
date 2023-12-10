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

#ifndef resources_buildinguidataH
#define resources_buildinguidataH

#include "SDLutility/autosurface.h"
#include "game/data/units/id.h"
#include "resources/sound.h"
#include "utility/serialization/serialization.h"

#include <SDL.h>

class cBuilding;
class cPlayer;

//------------------------------------------------------------------------------
struct sBuildingUIStaticData
{
	bool hasBetonUnderground = false;
	bool hasClanLogos = false;
	bool hasDamageEffect = false;
	bool hasOverlay = false;
	bool hasPlayerColor = false;
	bool isAnimated = false;
	bool powerOnGraphic = false;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (hasBetonUnderground);
		archive & NVP (hasClanLogos);
		archive & NVP (hasDamageEffect);
		archive & NVP (hasOverlay);
		archive & NVP (hasPlayerColor);
		archive & NVP (isAnimated);
		archive & NVP (powerOnGraphic);
		// clang-format on
	}
};

//--------------------------------------------------------------------------
/** struct for the images and sounds */
//--------------------------------------------------------------------------
struct sBuildingUIData
{
	sBuildingUIData() = default;
	sBuildingUIData (const sBuildingUIData&) = delete;
	sBuildingUIData (sBuildingUIData&&) = default;
	sBuildingUIData& operator= (const sBuildingUIData&) = delete;
	sBuildingUIData& operator= (sBuildingUIData&&) = default;

	/**
	* draws the main image of the building onto the given surface
	*/
	void render (unsigned long long animationTime, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, const cBuilding&, bool drawShadow, bool drawConcrete) const;
	void render_simple (SDL_Surface&, const SDL_Rect& dest, float zoomFactor, int clan, std::optional<cRgbColor> playerColor, int frameNr = 0, int alpha = 254) const;
	void render_simple (SDL_Surface&, const SDL_Rect& dest, float zoomFactor, const cPlayer* owner, int frameNr = 0, int alpha = 254) const;
	void render_simple (SDL_Surface&, const SDL_Rect& dest, float zoomFactor, const cBuilding&, unsigned long long animationTime = 0, int alpha = 254) const;

private:
	void drawConnectors (SDL_Surface&, SDL_Rect dest, float zoomFactor, const cBuilding&, bool drawShadow) const;

public:
	sID id;

	sBuildingUIStaticData staticData;

	bool isConnectorGraphic = false;
	int hasFrames = 0;

	UniqueSurface img, img_org; // Surface of the building
	UniqueSurface shw, shw_org; // Surfaces of the shadow
	UniqueSurface eff, eff_org; // Surfaces of the effects
	UniqueSurface video; // video
	UniqueSurface info; // info image

	// Die Sounds:
	cSoundChunk Start;
	cSoundChunk Running;
	cSoundChunk Stop;
	cSoundChunk Attack;
	cSoundChunk Wait;
};

void render (const cBuilding&, unsigned long long animationTime, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete);
void render_simple (const cBuilding&, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, unsigned long long animationTime = 0, int alpha = 254);

#endif
