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

#ifndef ui_graphical_game_temp_animationtimerH
#define ui_graphical_game_temp_animationtimerH

#include <SDL_timer.h>

#include "utility/signal/signal.h"
#include "utility/runnable.h"

// TODO: may remove this classes!
//       replace it by something a lot more generic and clean

class cAnimationTimeFlags
{
public:
	cAnimationTimeFlags ();

	bool is10ms () const;
	bool is50ms () const;
	bool is100ms () const;
	bool is400ms () const;

	void set10ms (bool flag);
	void set50ms (bool flag);
	void set100ms (bool flag);
	void set400ms (bool flag);

private:
	bool is10msFlag;
	bool is50msFlag;
	bool is100msFlag;
	bool is400msFlag;
};

class cAnimationTimer : public cRunnable
{
public:
	cAnimationTimer ();
	~cAnimationTimer ();

	void increaseTimer ();

	unsigned long long getAnimationTime () const;

	virtual void run () MAXR_OVERRIDE_FUNCTION;
	const cAnimationTimeFlags& getAnimationFlags () const;

	cSignal<void ()> triggered10ms;
	cSignal<void ()> triggered50ms;
	cSignal<void ()> triggered100ms;
	cSignal<void ()> triggered400ms;
private:
	const Uint32 sdlTimerInterval;
	SDL_TimerID timerId;

	unsigned long long timerTime;

	unsigned long long nextTrigger10msTime;
	unsigned long long nextTrigger50msTime;
	unsigned long long nextTrigger100msTime;
	unsigned long long nextTrigger400msTime;

	cAnimationTimeFlags animationFlags;
};

#endif // ui_graphical_game_temp_animationtimerH
