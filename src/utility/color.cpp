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

#include "utility/color.h"

//------------------------------------------------------------------------------
cRgbColor::cRgbColor () :
	r (0),
	g (0),
	b (0),
	a (0xFF)
{}

//------------------------------------------------------------------------------
cRgbColor::cRgbColor (unsigned char red_, unsigned char green_, unsigned char blue_, unsigned char alpha_) :
	r (red_),
	g (green_),
	b (blue_),
	a (alpha_)
{}

//------------------------------------------------------------------------------
bool cRgbColor::operator==(const cRgbColor& other) const
{
	return r == other.r && g == other.g && b == other.b && a == other.a;
}

//------------------------------------------------------------------------------
bool cRgbColor::operator!=(const cRgbColor& other) const
{
	return !(*this == other);
}

//------------------------------------------------------------------------------
SDL_Color cRgbColor::toSdlColor () const
{
	return SDL_Color{r, g, b, a};
}

//------------------------------------------------------------------------------
Uint32 cRgbColor::toMappedSdlRGBColor (const SDL_PixelFormat* format) const
{
	return SDL_MapRGB (format, r, g, b);
}

//------------------------------------------------------------------------------
Uint32 cRgbColor::toMappedSdlRGBAColor (const SDL_PixelFormat* format) const
{
	return SDL_MapRGBA (format, r, g, b, a);
}

//------------------------------------------------------------------------------
cRgbColor cRgbColor::exchangeRed (unsigned char red_) const
{
	return cRgbColor (red_, g, b, a);
}
//------------------------------------------------------------------------------
cRgbColor cRgbColor::exchangeGreen (unsigned char green_) const
{
	return cRgbColor (r, green_, b, a);
}
//------------------------------------------------------------------------------
cRgbColor cRgbColor::exchangeBlue (unsigned char blue_) const
{
	return cRgbColor (r, g, blue_, a);
}
//------------------------------------------------------------------------------
cRgbColor cRgbColor::exchangeAlpha (unsigned char alpha_) const
{
	return cRgbColor (r, g, b, alpha_);
}

//------------------------------------------------------------------------------
cHsvColor cRgbColor::toHsv () const
{
	const auto maxValue = std::max (r, std::max (g, b));
	const auto minValue = std::min (r, std::min (g, b));

	const auto delta = maxValue - minValue;

	short hTemp = 0;
	if (maxValue == minValue) hTemp = 0;
	else if (maxValue == r) hTemp = (60 * ((int)g-b) / delta);
	else if (maxValue == g) hTemp = (120 + 60 * ((int)b-r) / delta);
	else if (maxValue == b) hTemp = (240 + 60 * ((int)r-g) / delta);

	if (hTemp < 0) hTemp += 360;

	cHsvColor result;

	result.h = (unsigned short)hTemp;

	if (maxValue == 0) result.s = 0;
	else result.s = delta * 100 / maxValue;

	result.v = maxValue * 100 / 255;

	assert (result.h >= 0 && result.h < 360);
	assert (result.s >= 0 && result.s <= 100);
	assert (result.v >= 0 && result.v <= 100);

	return result;
}

//------------------------------------------------------------------------------
cHsvColor::cHsvColor () :
	h (0),
	s (0),
	v (0),
	a (0xFF)
{}

//------------------------------------------------------------------------------
cHsvColor::cHsvColor (unsigned short hue, unsigned char saturation, unsigned char value, unsigned char alpha_) :
	h (hue),
	s (saturation),
	v (value),
	a (alpha_)
{}

//------------------------------------------------------------------------------
bool cHsvColor::operator==(const cHsvColor& other) const
{
	return h == other.h && s == other.s && v == other.v && a == other.a;
}

//------------------------------------------------------------------------------
bool cHsvColor::operator!=(const cHsvColor& other) const
{
	return !(*this == other);
}

//------------------------------------------------------------------------------
cRgbColor cHsvColor::toRgb () const
{
	assert (h >= 0 && h < 360);
	assert (s >= 0 && s <= 100);
	assert (v >= 0 && v <= 100);

	cRgbColor result;
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