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

#ifndef gui_game_temp_animationtimerH
#define gui_game_temp_animationtimerH

#include <SDL_timer.h>

// TODO: may remove this class! replace it by something a lot more generic and clean

class cAnimationTimeFlags
{
public:
	cAnimationTimeFlags ();

	bool is50ms () const;
	bool is100ms () const;
	bool is400ms () const;

	void set50ms (bool flag);
	void set100ms (bool flag);
	void set400ms (bool flag);

private:
	bool is50msFlag;
	bool is100msFlag;
	bool is400msFlag;
};

class cAnimationTimer
{
public:
	cAnimationTimer ();
	~cAnimationTimer ();

	void increaseTimer ();

	unsigned long long getAnimationTime () const;

	void updateAnimationFlags ();
	const cAnimationTimeFlags& getAnimationFlags () const;
private:
	SDL_TimerID timerId;
	unsigned long long timerTime;
	unsigned long long lastUpdateTimerTime;

	cAnimationTimeFlags animationFlags;
};

#endif // gui_game_temp_animationtimerH
