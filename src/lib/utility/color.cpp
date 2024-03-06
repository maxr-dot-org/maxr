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

#include "crc.h"
#include "utility/comparison.h"
#include "utility/narrow_cast.h"

#include <algorithm>
#include <cassert>

//------------------------------------------------------------------------------
bool cRgbColor::operator== (const cRgbColor& other) const
{
	return r == other.r && g == other.g && b == other.b && a == other.a;
}

//------------------------------------------------------------------------------
bool cRgbColor::operator!= (const cRgbColor& other) const
{
	return !(*this == other);
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
uint32_t cRgbColor::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (r, crc);
	crc = calcCheckSum (g, crc);
	crc = calcCheckSum (b, crc);
	crc = calcCheckSum (a, crc);

	return crc;
}

//------------------------------------------------------------------------------
cHsvColor cRgbColor::toHsv() const
{
	const auto maxValue = std::max ({r, g, b});
	const auto minValue = std::min ({r, g, b});

	const auto delta = maxValue - minValue;

	short hTemp = 0;
	if (maxValue == minValue)
		hTemp = 0;
	else if (maxValue == r)
		hTemp = narrow_cast<short> (60 * ((int) g - b) / delta);
	else if (maxValue == g)
		hTemp = narrow_cast<short> (120 + 60 * ((int) b - r) / delta);
	else if (maxValue == b)
		hTemp = narrow_cast<short> (240 + 60 * ((int) r - g) / delta);

	if (hTemp < 0) hTemp += 360;

	cHsvColor result;

	result.h = (unsigned short) hTemp;

	if (maxValue == 0)
		result.s = 0;
	else
		result.s = narrow_cast<unsigned char> (delta * 100 / maxValue);

	result.v = maxValue * 100 / 255;

	assert (result.h < 360);
	assert (result.s <= 100);
	assert (result.v <= 100);

	return result;
}

//------------------------------------------------------------------------------
cLabColor cRgbColor::toLab() const
{
	// Convert from RGB to XYZ color space
	auto r2 = (r / 255.0);
	auto g2 = (g / 255.0);
	auto b2 = (b / 255.0);

	if (r2 > 0.04045)
		r2 = std::pow (((r2 + 0.055) / 1.055), 2.4);
	else
		r2 = r2 / 12.92;
	if (g2 > 0.04045)
		g2 = std::pow (((g2 + 0.055) / 1.055), 2.4);
	else
		g2 = g2 / 12.92;
	if (b2 > 0.04045)
		b2 = std::pow (((b2 + 0.055) / 1.055), 2.4);
	else
		b2 = b2 / 12.92;

	const auto x = r2 * 0.4124 + g2 * 0.3576 + b2 * 0.1805;
	const auto y = r2 * 0.2126 + g2 * 0.7152 + b2 * 0.0722;
	const auto z = r2 * 0.0193 + g2 * 0.1192 + b2 * 0.9505;

	// Convert from XYZ to Lab color space
	auto x2 = x / 0.95047;
	auto y2 = y / 1.;
	auto z2 = z / 1.08883;

	if (x2 > 0.008856)
		x2 = std::cbrt (x2);
	else
		x2 = (7.787 * x2) + (16. / 116.);
	if (y2 > 0.008856)
		y2 = std::cbrt (y2);
	else
		y2 = (7.787 * y2) + (16. / 116.);
	if (z2 > 0.008856)
		z2 = std::cbrt (z2);
	else
		z2 = (7.787 * z2) + (16. / 116.);

	return cLabColor ((116 * y2) - 16, 500 * (x2 - y2), 200 * (y2 - z2));
}

//------------------------------------------------------------------------------
cHsvColor::cHsvColor (unsigned short hue, unsigned char saturation, unsigned char value, unsigned char alpha_) :
	h (hue),
	s (saturation),
	v (value),
	a (alpha_)
{}

//------------------------------------------------------------------------------
bool cHsvColor::operator== (const cHsvColor& other) const
{
	return h == other.h && s == other.s && v == other.v && a == other.a;
}

//------------------------------------------------------------------------------
bool cHsvColor::operator!= (const cHsvColor& other) const
{
	return !(*this == other);
}

//------------------------------------------------------------------------------
cRgbColor cHsvColor::toRgb() const
{
	assert (h < 360);
	assert (s <= 100);
	assert (v <= 100);

	cRgbColor result;
	if (s == 0)
	{
		result.r = result.g = result.b = (unsigned char) v * 255 / 100;
	}
	else
	{
		const auto hh = (double) h / 60;
		const auto i = (int) hh;

		const auto f = hh - i;

		const auto p = v * (100 - s) * 255 / 10000;
		const auto q = v * (100 - (s * f)) * 255 / 10000;
		const auto t = v * (100 - (s * (1. - f))) * 255 / 10000;

		switch (i)
		{
			default:
			case 0:
				result.r = (unsigned char) v * 255 / 100;
				result.g = (unsigned char) t;
				result.b = (unsigned char) p;
				break;
			case 1:
				result.r = (unsigned char) q;
				result.g = (unsigned char) v * 255 / 100;
				result.b = (unsigned char) p;
				break;
			case 2:
				result.r = (unsigned char) p;
				result.g = (unsigned char) v * 255 / 100;
				result.b = (unsigned char) t;
				break;
			case 3:
				result.r = (unsigned char) p;
				result.g = (unsigned char) q;
				result.b = (unsigned char) v * 255 / 100;
				break;
			case 4:
				result.r = (unsigned char) t;
				result.g = (unsigned char) p;
				result.b = (unsigned char) v * 255 / 100;
				break;
			case 5:
				result.r = (unsigned char) v * 255 / 100;
				result.g = (unsigned char) p;
				result.b = (unsigned char) q;
				break;
		}
	}
	return result;
}

//------------------------------------------------------------------------------
cLabColor::cLabColor (double L_, double a_, double b_) :
	L (L_),
	a (a_),
	b (b_)
{}

//------------------------------------------------------------------------------
bool cLabColor::operator== (const cLabColor& other) const
{
	return equals (L, other.L) && equals (a, other.a) && equals (b, other.b);
}

//------------------------------------------------------------------------------
bool cLabColor::operator!= (const cLabColor& other) const
{
	return !(*this == other);
}

//------------------------------------------------------------------------------
double cLabColor::deltaE (const cLabColor& other) const
{
	const auto dL = L - other.L;
	const auto da = a - other.a;
	const auto db = b - other.b;
	return std::sqrt (dL * dL + da * da + db * db);
}
