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

#include <algorithm>

#include "game/logic/turntimeclock.h"
#include "gametimer.h"


//------------------------------------------------------------------------------
cTurnTimeDeadline::cTurnTimeDeadline (unsigned int startGameTime_, const std::chrono::milliseconds& deadline_) :
	startGameTime (startGameTime_),
	deadline (deadline_)
{}

//------------------------------------------------------------------------------
unsigned int cTurnTimeDeadline::getStartGameTime () const
{
	return startGameTime;
}

//------------------------------------------------------------------------------
void cTurnTimeDeadline::changeDeadline (const std::chrono::milliseconds& deadline_)
{
	deadline = deadline_;
}

//------------------------------------------------------------------------------
std::chrono::milliseconds cTurnTimeDeadline::getTimeTillReached (unsigned int currentGameTime)
{
	const auto deadlineEndMilliseconds = startGameTime * GAME_TICK_TIME + deadline.count ();
	const auto currentTimeMilliseconds = currentGameTime * GAME_TICK_TIME;

	return std::chrono::milliseconds (deadlineEndMilliseconds < currentTimeMilliseconds ? 0 : deadlineEndMilliseconds - currentTimeMilliseconds);
}

//------------------------------------------------------------------------------
cTurnTimeClock::cTurnTimeClock (std::shared_ptr<cGameTimer> gameTimer_) :
	gameTimer (std::move(gameTimer_)),
	startTurnGameTime (0)
{
	unsigned int lastCheckedSeconds = 0;
	signalConnectionManager.connect (gameTimer->gameTimeChanged, [lastCheckedSeconds, this]() mutable
	{
		const auto currentSeconds = gameTimer->gameTime / 100;
		if (currentSeconds > lastCheckedSeconds)
		{
			lastCheckedSeconds = currentSeconds;
			secondChanged ();
		}
	});
}

//------------------------------------------------------------------------------
void cTurnTimeClock::restartFromNow ()
{
	restartFrom (gameTimer->gameTime);
}

//------------------------------------------------------------------------------
void cTurnTimeClock::restartFrom (unsigned int gameTime)
{
	startTurnGameTime = gameTime;
	secondChanged ();
}

//------------------------------------------------------------------------------
unsigned int cTurnTimeClock::getStartGameTime () const
{
	return startTurnGameTime;
}

//------------------------------------------------------------------------------
void cTurnTimeClock::clearAllDeadlines ()
{
	deadlines.clear ();
	deadlinesChanged ();
}

//------------------------------------------------------------------------------
std::shared_ptr<cTurnTimeDeadline> cTurnTimeClock::startNewDeadlineFromNow (const std::chrono::milliseconds& deadline)
{
	return startNewDeadlineFrom (gameTimer->gameTime, deadline);
}

//------------------------------------------------------------------------------
std::shared_ptr<cTurnTimeDeadline> cTurnTimeClock::startNewDeadlineFrom (unsigned int gameTime, const std::chrono::milliseconds& deadline)
{
	auto turnTimeDeadline = std::make_shared<cTurnTimeDeadline> (gameTime, deadline);
	deadlines.push_back (turnTimeDeadline);
	deadlinesChanged ();
	return turnTimeDeadline;
}

//------------------------------------------------------------------------------
void cTurnTimeClock::removeDeadline (const std::shared_ptr<cTurnTimeDeadline>& deadline)
{
	for (auto i = deadlines.begin (); i != deadlines.end (); ++i)
	{
		if ((*i) == deadline)
		{
			deadlines.erase (i);
			deadlinesChanged ();
			return;
		}
	}
}

//------------------------------------------------------------------------------
std::chrono::milliseconds cTurnTimeClock::getTimeSinceStart () const
{
	return std::chrono::milliseconds ((gameTimer->gameTime - startTurnGameTime) * GAME_TICK_TIME);
}

//------------------------------------------------------------------------------
std::chrono::milliseconds cTurnTimeClock::getTimeTillFirstDeadline () const
{
	if (deadlines.empty ()) return std::chrono::milliseconds (0);

	auto minTime = deadlines[0]->getTimeTillReached (gameTimer->gameTime);
	for (auto i = deadlines.begin () + 1; i != deadlines.end (); ++i)
	{
		minTime = std::min (minTime, (*i)->getTimeTillReached (gameTimer->gameTime));
	}
	return minTime;
}

//------------------------------------------------------------------------------
bool cTurnTimeClock::hasReachedAnyDeadline () const
{
	for (auto i = deadlines.begin (); i != deadlines.end (); ++i)
	{
		if ((*i)->getTimeTillReached (gameTimer->gameTime) <= std::chrono::milliseconds (0))
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
bool cTurnTimeClock::hasDeadline () const
{
	return !deadlines.empty();
}