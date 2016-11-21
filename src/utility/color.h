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

#ifndef utility_colorH
#define utility_colorH

#include <random>
#include <algorithm>
#include <cassert>

#include <SDL.h>

#include "serialization/serialization.h"

class cHsvColor;
class cLabColor;

/**
 * Simple RGBA Color class (RGBA = Red-Green-Blue-Alpha).
 */
class cRgbColor
{
public:
	cRgbColor();
	cRgbColor (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xFF);

	bool operator== (const cRgbColor& other) const;
	bool operator!= (const cRgbColor& other) const;

	SDL_Color toSdlColor() const;
	Uint32 toMappedSdlRGBColor (const SDL_PixelFormat* format) const;
	Uint32 toMappedSdlRGBAColor (const SDL_PixelFormat* format) const;

	cHsvColor toHsv() const;
	cLabColor toLab() const;

	cRgbColor exchangeRed (unsigned char red) const;
	cRgbColor exchangeGreen (unsigned char green) const;
	cRgbColor exchangeBlue (unsigned char blue) const;
	cRgbColor exchangeAlpha (unsigned char alpha) const;

	unsigned char r, g, b;
	unsigned char a;

	uint32_t getChecksum(uint32_t crc) const;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(r);
		archive & NVP(g);
		archive & NVP(b);
		archive & NVP(a);
	}

	// predefined colors
	inline static cRgbColor red (unsigned char alpha_ = 0xFF) { return cRgbColor (0xFF, 0, 0, alpha_); }
	inline static cRgbColor green (unsigned char alpha_ = 0xFF) { return cRgbColor (0, 0xFF, 0, alpha_); }
	inline static cRgbColor blue (unsigned char alpha_ = 0xFF) { return cRgbColor (0, 0, 0xFF, alpha_); }
	inline static cRgbColor yellow (unsigned char alpha_ = 0xFF) { return cRgbColor (0xFF, 0xFF, 0, alpha_); }
	inline static cRgbColor black (unsigned char alpha_ = 0xFF) { return cRgbColor (0, 0, 0, alpha_); }
	inline static cRgbColor white (unsigned char alpha_ = 0xFF) { return cRgbColor (0xFF, 0xFF, 0xFF, alpha_); }
	inline static cRgbColor transparent() { return cRgbColor (0, 0, 0, 0); }

	inline static cRgbColor random (unsigned char alpha_ = 0xFF)
	{
		static std::random_device rd;
		static std::mt19937 engine (rd());
		// Workaround for C++11 bug: uniform_int_distribution does not allow char types
		std::uniform_int_distribution<unsigned int> distribution (0, (unsigned int)std::numeric_limits<unsigned char>::max());
		return cRgbColor ((unsigned char)distribution (engine), (unsigned char)distribution (engine), (unsigned char)distribution (engine), alpha_);
	}
};

class cHsvColor
{
public:
	cHsvColor();
	cHsvColor (unsigned short hue, unsigned char saturation, unsigned char value, unsigned char alpha = 0xFF);

	bool operator== (const cHsvColor& other) const;
	bool operator!= (const cHsvColor& other) const;

	cRgbColor toRgb() const;

	unsigned short h;
	unsigned char s, v;
	unsigned char a;
};

class cLabColor
{
public:
	cLabColor();
	cLabColor (double L, double a, double b);

	bool operator== (const cLabColor& other) const;
	bool operator!= (const cLabColor& other) const;

	double deltaE (const cLabColor& other) const;

	double L;
	double a;
	double b;
};

#endif // utility_colorH
