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

#include "ui/widgets/runnable.h"
#include "utility/signal/signal.h"

#include <SDL_timer.h>
#include <atomic>

/**
 * Central class to get animation timers from.
 *
 * This class implements the runnable interface so that the execution
 * of animation callbacks can be put into the main game loop.
 *
 * Hence the callbacks will be synchronized with the drawing thread which
 * makes extra synchronization unnecessary.
 */
class cAnimationTimer : public cRunnable
{
public:
	cAnimationTimer();
	~cAnimationTimer();

	cAnimationTimer (const cAnimationTimer&) = delete;
	cAnimationTimer& operator= (const cAnimationTimer&) = delete;

	/**
	 * Increases the internal timer.
	 *
	 * For internal use only!
	 */
	void increaseTimer();

	/**
	 * Returns the animation time since the animation timer has been started in 100ms steps.
	 */
	unsigned long long getAnimationTime() const;

	/**
	 * Checks the time since the last call and emits the
	 * signals according to this interval.
	 */
	void run() override;

	/*
	 * The following signals get called during the run method.
	 *
	 * It gets called each time run() is executed, which means each time a frame has been drawn.
	 */
	cSignal<void()> triggeredFrame;

	/*
	 * The following signals get called during the run method.
	 *
	 * Lets take the 10ms signal as example.
	 * It gets triggered exactly once when the time since the last
	 * exceeds 10ms.
	 * To make it absolutely clear: only one call even if 95ms
	 * have been passed since the last call. But not one call if only 5ms
	 * have passed.
	 */
	cSignal<void()> triggered10ms;
	cSignal<void()> triggered50ms;
	cSignal<void()> triggered100ms;
	cSignal<void()> triggered400ms;

	/*
	 * The following signals get called during the run method.
	 *
	 * These signals do catch up if the FPS drop below the time
	 * of the signal.
	 *
	 * Lets take the 10ms signal as example.
	 * It gets triggered for every period of 10ms between the last
	 * and the current calls.
	 * To make it absolutely clear: if 95ms passed since the last call
	 * the signal will be called 9 times in a row. But not one call if only 5ms
	 * have passed.
	 */
	cSignal<void()> triggered10msCatchUp;
	cSignal<void()> triggered50msCatchUp;
	cSignal<void()> triggered100msCatchUp;
	cSignal<void()> triggered400msCatchUp;

private:
	static constexpr Uint32 sdlTimerInterval = 10;
	SDL_TimerID timerId;

	std::atomic<unsigned long long> timerTime{0};

	unsigned long long nextTrigger10msTime;
	unsigned long long nextTrigger50msTime;
	unsigned long long nextTrigger100msTime;
	unsigned long long nextTrigger400msTime;
};

#endif // ui_graphical_game_animations_animationtimerH
