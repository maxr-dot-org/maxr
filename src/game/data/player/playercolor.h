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

#ifndef game_data_player_playercolorH
#define game_data_player_playercolorH

#include <SDL.h>
#include "utility/color.h"
#include "utility/autosurface.h"
#include "utility/serialization/serialization.h"

class cPlayerColor
{
public:
	static const size_t predefinedColorsCount = 8;
	static const cRgbColor predefinedColors[predefinedColorsCount];

	static size_t findClosestPredefinedColor (const cRgbColor& color);

	cPlayerColor();
	explicit cPlayerColor (const cRgbColor& color);
	cPlayerColor (const cPlayerColor& other);
	cPlayerColor (cPlayerColor&& other);

	cPlayerColor& operator= (const cPlayerColor& other);
	cPlayerColor& operator= (cPlayerColor && other);

	const cRgbColor& getColor() const;
	SDL_Surface* getTexture() const;

	bool operator== (const cPlayerColor& other) const;
	bool operator!= (const cPlayerColor& other) const;

	template<typename T>
	void save(T& archive)
	{
		archive & NVP(color);
	}
	template<typename T>
	void load(T& archive)
	{
		archive & NVP(color);
		createTexture();
	}
	SERIALIZATION_SPLIT_MEMBER();
private:
	cRgbColor color;
	AutoSurface texture;

	void createTexture();
};

#endif // game_data_player_playercolorH
