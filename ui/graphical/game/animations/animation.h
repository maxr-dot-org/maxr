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

#ifndef ui_graphical_game_animations_animationH
#define ui_graphical_game_animations_animationH

#include "utility/signal/signal.h"

class cPosition;
template<typename> class cBox;

class cAnimation
{
public:
	cAnimation () :
		finished (false),
		running (false)
	{}
	virtual ~cAnimation () {}

	virtual bool isLocatedIn (const cBox<cPosition>& box) const = 0;

	bool isFinished () const { return finished; }
	bool isRunning () const { return running; }

protected:
	bool finished;
	bool running;
};

#endif // ui_graphical_game_animations_animationH
