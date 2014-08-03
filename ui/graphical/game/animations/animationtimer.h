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

#ifndef ui_graphical_game_animations_animationtimerH
#define ui_graphical_game_animations_animationtimerH

#include <SDL_timer.h>

#include "utility/signal/signal.h"
#include "utility/runnable.h"

class cAnimationTimer : public cRunnable
{
public:
	cAnimationTimer ();
	~cAnimationTimer ();

	void increaseTimer ();

	unsigned long long getAnimationTime () const;

	virtual void run () MAXR_OVERRIDE_FUNCTION;

	cSignal<void ()> triggered10ms;
	cSignal<void ()> triggered50ms;
	cSignal<void ()> triggered100ms;
	cSignal<void ()> triggered400ms;

	cSignal<void ()> triggered10msCatchUp;
	cSignal<void ()> triggered50msCatchUp;
	cSignal<void ()> triggered100msCatchUp;
	cSignal<void ()> triggered400msCatchUp;
private:
	const Uint32 sdlTimerInterval;
	SDL_TimerID timerId;

	unsigned long long timerTime;

	unsigned long long nextTrigger10msTime;
	unsigned long long nextTrigger50msTime;
	unsigned long long nextTrigger100msTime;
	unsigned long long nextTrigger400msTime;
};

#endif // ui_graphical_game_animations_animationtimerH
