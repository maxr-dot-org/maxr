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
#include <tuple>

class cHsvColor;
class cLabColor;

/**
 * Simple RGBA Color class (RGBA = Red-Green-Blue-Alpha).
 */
class cRgbColor
{
public:
	constexpr cRgbColor() = default;
	constexpr cRgbColor (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xFF) :
		r (red),
		g (green),
		b (blue),
		a (alpha)
	{
	}

	bool operator== (const cRgbColor&) const = default;

	cHsvColor toHsv() const;
	cLabColor toLab() const;

	cRgbColor exchangeRed (unsigned char red) const;
	cRgbColor exchangeGreen (unsigned char green) const;
	cRgbColor exchangeBlue (unsigned char blue) const;
	cRgbColor exchangeAlpha (unsigned char alpha) const;

	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 0xFF;

	uint32_t getChecksum (uint32_t crc) const;

	// predefined colors
	static constexpr cRgbColor red (unsigned char alpha_ = 0xFF) { return cRgbColor (0xFF, 0, 0, alpha_); }
	static constexpr cRgbColor green (unsigned char alpha_ = 0xFF) { return cRgbColor (0, 0xFF, 0, alpha_); }
	static constexpr cRgbColor blue (unsigned char alpha_ = 0xFF) { return cRgbColor (0, 0, 0xFF, alpha_); }
	static constexpr cRgbColor yellow (unsigned char alpha_ = 0xFF) { return cRgbColor (0xFF, 0xFF, 0, alpha_); }
	static constexpr cRgbColor black (unsigned char alpha_ = 0xFF) { return cRgbColor (0, 0, 0, alpha_); }
	static constexpr cRgbColor white (unsigned char alpha_ = 0xFF) { return cRgbColor (0xFF, 0xFF, 0xFF, alpha_); }
	static constexpr cRgbColor transparent() { return cRgbColor (0, 0, 0, 0); }

	static cRgbColor random (unsigned char alpha_ = 0xFF)
	{
		static std::random_device rd;
		static std::mt19937 engine (rd());
		// Workaround for C++11 bug: uniform_int_distribution does not allow char types
		std::uniform_int_distribution<unsigned int> distribution (0, (unsigned int) std::numeric_limits<unsigned char>::max());
		return cRgbColor ((unsigned char) distribution (engine), (unsigned char) distribution (engine), (unsigned char) distribution (engine), alpha_);
	}
};

//------------------------------------------------------------------------------
struct sLessRgbColor
{
	bool operator() (const cRgbColor& lhs, const cRgbColor& rhs) const
	{
		return std::tie (lhs.r, lhs.g, lhs.b, lhs.a) < std::tie (rhs.r, rhs.g, rhs.b, rhs.a);
	}
};

//------------------------------------------------------------------------------
class cHsvColor
{
public:
	cHsvColor() = default;
	cHsvColor (unsigned short hue, unsigned char saturation, unsigned char value, unsigned char alpha = 0xFF);

	bool operator== (const cHsvColor& other) const;
	bool operator!= (const cHsvColor& other) const;

	cRgbColor toRgb() const;

	unsigned short h = 0;
	unsigned char s = 0;
	unsigned char v = 0;
	unsigned char a = 0xFF;
};

//------------------------------------------------------------------------------
class cLabColor
{
public:
	cLabColor() = default;
	cLabColor (double L, double a, double b);

	bool operator== (const cLabColor& other) const;
	bool operator!= (const cLabColor& other) const;

	double deltaE (const cLabColor& other) const;

	double L = 0.;
	double a = 0.;
	double b = 0.;
};

#endif // utility_colorH
