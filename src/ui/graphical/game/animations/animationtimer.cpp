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

#include "ui/graphical/game/animations/animationtimer.h"

namespace
{

	//----------------------------------------------------------------------
	Uint32 timerCallback (Uint32 interval, void* arg)
	{
		static_cast<cAnimationTimer*> (arg)->increaseTimer();
		return interval;
	}

} // namespace

//--------------------------------------------------------------------------
cAnimationTimer::cAnimationTimer()
{
	timerId = SDL_AddTimer (sdlTimerInterval, timerCallback, this);

	nextTrigger10msTime = 10 / sdlTimerInterval;
	nextTrigger50msTime = 50 / sdlTimerInterval;
	nextTrigger100msTime = 100 / sdlTimerInterval;
	nextTrigger400msTime = 400 / sdlTimerInterval;
}

//--------------------------------------------------------------------------
cAnimationTimer::~cAnimationTimer()
{
	SDL_RemoveTimer (timerId);
}

//--------------------------------------------------------------------------
void cAnimationTimer::increaseTimer()
{
	++timerTime;
}

//--------------------------------------------------------------------------
unsigned long long cAnimationTimer::getAnimationTime() const
{
	return timerTime / (100 / sdlTimerInterval); // in 100ms steps
}

//--------------------------------------------------------------------------
void cAnimationTimer::run()
{
	static const size_t maxCatchUp = 10;

	triggeredFrame();

	if (timerTime >= nextTrigger10msTime)
	{
		triggered10ms();
		size_t count = 0;
		do
		{
			triggered10msCatchUp();
			if (count++ >= maxCatchUp)
			{
				nextTrigger10msTime = timerTime + 10 / sdlTimerInterval;
				break;
			}
		} while ((nextTrigger10msTime += 10 / sdlTimerInterval) < timerTime);
	}
	if (timerTime >= nextTrigger50msTime)
	{
		triggered50ms();
		size_t count = 0;
		do
		{
			triggered50msCatchUp();
			if (count++ >= maxCatchUp)
			{
				nextTrigger10msTime = timerTime + 50 / sdlTimerInterval;
				break;
			}
		} while ((nextTrigger50msTime += 50 / sdlTimerInterval) < timerTime);
	}
	if (timerTime >= nextTrigger100msTime)
	{
		triggered100ms();
		size_t count = 0;
		do
		{
			triggered100msCatchUp();
			if (count++ >= maxCatchUp)
			{
				nextTrigger10msTime = timerTime + 100 / sdlTimerInterval;
				break;
			}
		} while ((nextTrigger100msTime += 100 / sdlTimerInterval) < timerTime);
	}
	if (timerTime >= nextTrigger400msTime)
	{
		triggered400ms();
		size_t count = 0;
		do
		{
			triggered400msCatchUp();
			if (count++ >= maxCatchUp)
			{
				nextTrigger10msTime = timerTime + 400 / sdlTimerInterval;
				break;
			}
		} while ((nextTrigger400msTime += 400 / sdlTimerInterval) < timerTime);
	}
}
