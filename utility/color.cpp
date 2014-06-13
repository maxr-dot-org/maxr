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

#include "color.h"

//------------------------------------------------------------------------------
cColor::cColor () :
	r (0),
	g (0),
	b (0),
	a (0xFF)
{}

//------------------------------------------------------------------------------
cColor::cColor (unsigned char red_, unsigned char green_, unsigned char blue_, unsigned char alpha_) :
	r (red_),
	g (green_),
	b (blue_),
	a (alpha_)
{}

//------------------------------------------------------------------------------
bool cColor::operator==(const cColor& other) const
{
	return r == other.r && g == other.g && b == other.b && a == other.a;
}

//------------------------------------------------------------------------------
bool cColor::operator!=(const cColor& other) const
{
	return !(*this == other);
}

//------------------------------------------------------------------------------
SDL_Color cColor::toSdlColor () const
{
	return SDL_Color{r, g, b, a};
}

//------------------------------------------------------------------------------
Uint32 cColor::toMappedSdlRGBColor (const SDL_PixelFormat* format) const
{
	return SDL_MapRGB (format, r, g, b);
}

//------------------------------------------------------------------------------
Uint32 cColor::toMappedSdlRGBAColor (const SDL_PixelFormat* format) const
{
	return SDL_MapRGBA (format, r, g, b, a);
}

//------------------------------------------------------------------------------
cColor cColor::exchangeRed (unsigned char red_)
{
	return cColor (red_, g, b, a);
}
//------------------------------------------------------------------------------
cColor cColor::exchangeGreen (unsigned char green_)
{
	return cColor (r, green_, b, a);
}
//------------------------------------------------------------------------------
cColor cColor::exchangeBlue (unsigned char blue_)
{
	return cColor (r, g, blue_, a);
}
//------------------------------------------------------------------------------
cColor cColor::exchangeAlpha (unsigned char alpha_)
{
	return cColor (r, g, b, alpha_);
}
