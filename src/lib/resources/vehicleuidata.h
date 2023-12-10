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

#ifndef resources_vehicleuidataH
#define resources_vehicleuidataH

#include "SDLutility/autosurface.h"
#include "game/data/units/id.h"
#include "resources/sound.h"
#include "utility/serialization/serialization.h"

#include <SDL.h>
#include <array>
#include <filesystem>
#include <string>

class cMapView;
class cPlayer;
class cVehicle;
struct sStaticVehicleData;

struct sVehicleUIStaticData
{
	bool buildUpGraphic = false;
	bool hasDamageEffect = false;
	bool hasOverlay = false;
	bool hasPlayerColor = false;
	bool isAnimated = false;
	int hasFrames = 0;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (buildUpGraphic);
		archive & NVP (hasDamageEffect);
		archive & NVP (hasOverlay);
		archive & NVP (hasPlayerColor);
		archive & NVP (isAnimated);
		archive & NVP (hasFrames);
		// clang-format on
	}
};

//-----------------------------------------------------------------------------
// Struct for the pictures and sounds
//-----------------------------------------------------------------------------
struct sVehicleUIData
{
	friend void render (const cVehicle&, const cMapView*, unsigned long long animationTime, const cPlayer*, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, bool drawShadow);

public:
	sVehicleUIData() = default;
	sVehicleUIData (const sVehicleUIData&) = delete;
	sVehicleUIData (sVehicleUIData&&) = default;
	sVehicleUIData& operator= (const sVehicleUIData&) = delete;
	sVehicleUIData& operator= (sVehicleUIData&&) = default;

	void render_simple (SDL_Surface&, const SDL_Rect& dest, float zoomFactor, const sStaticVehicleData&, std::optional<cRgbColor> playerColor, int dir = 0, int walkFrame = 0, int alpha = 254) const;
	void render_simple (SDL_Surface&, const SDL_Rect& dest, float zoomFactor, const sStaticVehicleData&, const cPlayer* owner, int dir = 0, int walkFrame = 0, int alpha = 254) const;
	void drawOverlayAnimation (SDL_Surface&, const SDL_Rect& dest, float zoomFactor, int frameNr = 0, int alpha = 254) const;

private:
	void render_shadow (const cVehicle&, const cMapView&, SDL_Surface&, const SDL_Rect& dest, float zoomFactor) const;
	void render_BuildingOrBigClearing (const cVehicle&, const cMapView&, unsigned long long animationTime, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_smallClearing (const cVehicle&, unsigned long long animationTime, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;

public:
	sID id;

	sVehicleUIStaticData staticData;

	std::array<UniqueSurface, 8> img, img_org; // 8 Surfaces of the vehicle
	std::array<UniqueSurface, 8> shw, shw_org; // 8 Surfaces of shadows
	UniqueSurface build, build_org; // Surfaces when building
	UniqueSurface build_shw, build_shw_org; // Surfaces of shadows when building
	UniqueSurface clear_small, clear_small_org; // Surfaces when clearing
	UniqueSurface clear_small_shw, clear_small_shw_org; // Surfaces when clearing
	UniqueSurface overlay, overlay_org; // Overlays
	UniqueSurface storage; // image of the vehicle in storage
	std::filesystem::path FLCFile; // FLC-Video
	UniqueSurface info; // info image

	// Sounds:
	cSoundChunk Wait;
	cSoundChunk WaitWater;
	cSoundChunk Start;
	cSoundChunk StartWater;
	cSoundChunk Stop;
	cSoundChunk StopWater;
	cSoundChunk Drive;
	cSoundChunk DriveWater;
	cSoundChunk Attack;
};

/**
* draws the main image of the vehicle onto the passed surface
*/
void render (const cVehicle&, const cMapView*, unsigned long long animationTime, const cPlayer*, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, bool drawShadow);
void render_simple (const cVehicle&, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, int alpha = 254);

/**
* draws the overlay animation of the vehicle on the given surface
*@author: eiko
*/
void drawOverlayAnimation (const cVehicle&, unsigned long long animationTime, SDL_Surface&, const SDL_Rect& dest, float zoomFactor);
void drawOverlayAnimation (const cVehicle&, SDL_Surface&, const SDL_Rect& dest, float zoomFactor, int frameNr, int alpha = 254);

#endif
