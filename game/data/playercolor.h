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

#ifndef game_data_playercolorH
#define game_data_playercolorH

#include <SDL.h>
#include "../../utility/color.h"

class cPlayerColor
{
public:
	explicit cPlayerColor (size_t index);

	//assert (colorIndex < PLAYERCOLORS);
	const cColor& getColor () const;
	SDL_Surface* getTexture () const;
	size_t getIndex () const; // DO NOT USE THIS IN GENERAL. For now it is here for only one reason: Serialization!

	bool operator==(const cPlayerColor& other) const;
	bool operator!=(const cPlayerColor& other) const;
private:
	cColor color;
	size_t index;
	SDL_Surface* texture;
};

#endif // game_data_playercolorH
