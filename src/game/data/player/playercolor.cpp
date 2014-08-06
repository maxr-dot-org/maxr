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

#include <cassert>

#include "defines.h"
#include "main.h" // OtherData
#include "game/data/player/playercolor.h"
#include "utility/random.h"

const cRgbColor cPlayerColor::predefinedColors[predefinedColorsCount] =
{
	cRgbColor (0xFF, 0x00, 0x00), // red
	cRgbColor (0x00, 0xFF, 0x00), // green
	cRgbColor (0x00, 0x00, 0xFF), // blue
	cRgbColor (0x7F, 0x7F, 0x7F), // gray
	cRgbColor (0xFF, 0x7F, 0x00), // orange
	cRgbColor (0xFF, 0xFF, 0x00), // yellow
	cRgbColor (0xFF, 0x00, 0xFE), // purple
	cRgbColor (0x00, 0xFF, 0xFF)  // aqua
};

//------------------------------------------------------------------------------
cPlayerColor::cPlayerColor ()
{
	color = cRgbColor::red ();
	createTexture ();
}

//------------------------------------------------------------------------------
cPlayerColor::cPlayerColor (const cRgbColor& color_) :
	color (color_)
{
	createTexture ();
}

//------------------------------------------------------------------------------
cPlayerColor::cPlayerColor (const cPlayerColor& other) :
	color (other.color),
	texture (AutoSurface (other.texture.get ()))
{
	++texture->refcount;
}

//------------------------------------------------------------------------------
cPlayerColor::cPlayerColor (cPlayerColor&& other) :
	color (std::move (other.color)),
	texture (std::move (other.texture))
{}

//------------------------------------------------------------------------------
cPlayerColor& cPlayerColor::operator=(const cPlayerColor& other)
{
	color = other.color;

	texture = AutoSurface (other.texture.get ());
	++texture->refcount;

	return *this;
}

//------------------------------------------------------------------------------
cPlayerColor& cPlayerColor::operator=(cPlayerColor&& other)
{
	color = std::move (other.color);
	texture = std::move (other.texture);

	return *this;
}

//------------------------------------------------------------------------------
const cRgbColor& cPlayerColor::getColor () const
{
	return color;
}

//------------------------------------------------------------------------------
SDL_Surface* cPlayerColor::getTexture () const
{
	return texture.get();
}

//------------------------------------------------------------------------------
void cPlayerColor::createTexture ()
{
	texture = AutoSurface(SDL_CreateRGBSurface (0, 128, 128, 32, 0, 0, 0, 0));

	SDL_FillRect (texture.get (), nullptr, color.toMappedSdlRGBAColor (texture->format));

	auto hsvColor = color.toHsv ();

	const size_t boxes = 400;

	std::array<cRgbColor, 7> randomColors;

	randomColors[0] = color;

	for (size_t i = 1; i < randomColors.size (); ++i)
	{
		do
		{
			const auto hChange = random (0, 5);
			const auto sChange = random (10, 30);
			const auto vChange = random (10, 30);

			unsigned short changedH;
			unsigned char changedS, changedV;

			if ((int)(hsvColor.h)+hChange >= 360 || ((int)(hsvColor.h)-hChange >= 0 && randomBernoulli ())) changedH = hsvColor.h - (unsigned short)hChange;
			else changedH = hsvColor.h + (unsigned short)hChange;

			if ((int)(hsvColor.s)+sChange > 100 || ((int)(hsvColor.s)-sChange >= 0 && randomBernoulli ())) changedS = hsvColor.s - (unsigned char)sChange;
			else changedS = hsvColor.s + (unsigned char)sChange;

			if ((int)(hsvColor.v)+vChange > 100 || ((int)(hsvColor.v)-vChange >= 0 && randomBernoulli ())) changedV = hsvColor.v - (unsigned char)vChange;
			else changedV = hsvColor.v + (unsigned char)vChange;

			randomColors[i] = cHsvColor (changedH, changedS, changedV).toRgb();
		}
		while (randomColors[i] == cRgbColor (0xFF, 0, 0xFF)); // 0xFF00FF is our "transparent color". Hence we do not want to select this color as player color.
	}

	for (size_t j = 0; j < boxes; ++j)
	{
		int width = random (7, 20);
		int height = random (7, 20);
		int xPos = random (-5, 128);
		int yPos = random (-5, 128);

		if (xPos < 0)
		{
			width += xPos;
			xPos = 0;
		}
		if (yPos < 0)
		{
			height += yPos;
			yPos = 0;
		}
		SDL_Rect dest = {xPos, yPos, width, height};

		SDL_FillRect (texture.get (), &dest, getRandom(randomColors).toMappedSdlRGBAColor (texture->format));
	}
}

//------------------------------------------------------------------------------
bool cPlayerColor::operator==(const cPlayerColor& other) const
{
	return color == other.color;
}

//------------------------------------------------------------------------------
bool cPlayerColor::operator!=(const cPlayerColor& other) const
{
	return !(*this == other);
}
