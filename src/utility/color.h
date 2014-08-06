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

/**
 * Simple RGBA Color class (RGBA = Red-Green-Blue-Alpha).
 */
class cColor
{
public:
	cColor ();
	cColor (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xFF);

	bool operator==(const cColor& other) const;
	bool operator!=(const cColor& other) const;

	SDL_Color toSdlColor () const;
	Uint32 toMappedSdlRGBColor (const SDL_PixelFormat* format) const;
	Uint32 toMappedSdlRGBAColor (const SDL_PixelFormat* format) const;

	void toHsv (unsigned short& h, unsigned char& s, unsigned char& v) const
	{
		const auto maxValue = std::max (r, std::max (g, b));
		const auto minValue = std::min (r, std::min (g, b));

		const auto delta = maxValue - minValue;

		short hTemp = 0;
		if (maxValue == minValue) hTemp = 0;
		else if (maxValue == r) hTemp = (      60 * ((int)g-b) / delta);
		else if (maxValue == g) hTemp = (120 + 60 * ((int)b-r) / delta);
		else if (maxValue == b) hTemp = (240 + 60 * ((int)r-g) / delta);

		if (hTemp < 0) hTemp += 360;

		h = (unsigned short)hTemp;

		if (maxValue == 0) s = 0;
		else s = delta * 100 / maxValue;

		v = maxValue * 100 / 255;

		assert (h >= 0 && h < 360);
		assert (s >= 0 && s <= 100);
		assert (v >= 0 && v <= 100);
	}
	static cColor fromHsv (unsigned short h, unsigned char s, unsigned char v)
	{
		assert (h >= 0 && h < 360);
		assert (s >= 0 && s <= 100);
		assert (v >= 0 && v <= 100);

		cColor result;
		if (s == 0)
		{
			result.r = result.g = result.b = (unsigned char)v * 255 / 100;
		}
		else
		{
			const auto hh = (double)h / 60;
			const auto i = (int)hh;

			const auto f = hh - i;

			const auto p = v * (100 - s) * 255 / 10000;
			const auto q = v * (100 - (s * f)) * 255 / 10000;
			const auto t = v * (100 - (s * (1. - f))) * 255 / 10000;

			switch (i)
			{
			default:
			case 0:
				result.r = (unsigned char)v * 255 / 100;
				result.g = (unsigned char)t;
				result.b = (unsigned char)p;
				break;
			case 1:
				result.r = (unsigned char)q;
				result.g = (unsigned char)v * 255 / 100;
				result.b = (unsigned char)p;
				break;
			case 2:
				result.r = (unsigned char)p;
				result.g = (unsigned char)v * 255 / 100;
				result.b = (unsigned char)t;
				break;
			case 3:
				result.r = (unsigned char)p;
				result.g = (unsigned char)q;
				result.b = (unsigned char)v * 255 / 100;
				break;
			case 4:
				result.r = (unsigned char)t;
				result.g = (unsigned char)p;
				result.b = (unsigned char)v * 255 / 100;
				break;
			case 5:
				result.r = (unsigned char)v * 255 / 100;
				result.g = (unsigned char)p;
				result.b = (unsigned char)q;
				break;
			}
		}
		return result;
	}

	cColor exchangeRed (unsigned char red) const;
	cColor exchangeGreen (unsigned char green) const;
	cColor exchangeBlue (unsigned char blue) const;
	cColor exchangeAlpha (unsigned char alpha) const;

	unsigned char r, g, b;
	unsigned char a;

	// predefined colors
	inline static cColor red (unsigned char alpha_ = 0xFF) { return cColor (0xFF, 0, 0, alpha_); }
	inline static cColor green (unsigned char alpha_ = 0xFF) { return cColor (0, 0xFF, 0, alpha_); }
	inline static cColor blue (unsigned char alpha_ = 0xFF) { return cColor (0, 0, 0xFF, alpha_); }
	inline static cColor yellow (unsigned char alpha_ = 0xFF) { return cColor (0xFF, 0xFF, 0, alpha_); }
	inline static cColor black (unsigned char alpha_ = 0xFF) { return cColor (0, 0, 0, alpha_); }
	inline static cColor white (unsigned char alpha_ = 0xFF) { return cColor (0xFF, 0xFF, 0xFF, alpha_); }
	inline static cColor transparent () { return cColor (0, 0, 0, 0); }

	inline static cColor random (unsigned char alpha_ = 0xFF)
	{
		static std::random_device rd;
		static std::mt19937 engine (rd());
		// Workaround for C++11 bug: uniform_int_distribution does not allow char types
		std::uniform_int_distribution<unsigned int> distribution (0, (unsigned int)std::numeric_limits<unsigned char>::max ());
		return cColor ((unsigned char)distribution (engine), (unsigned char)distribution (engine), (unsigned char)distribution (engine), alpha_);
	}
};

#endif // utility_colorH
