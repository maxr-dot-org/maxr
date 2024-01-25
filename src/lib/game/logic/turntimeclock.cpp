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

#include "game/logic/turntimeclock.h"

#include "game/data/model.h"
#include "game/logic/gametimer.h"

#include <algorithm>
#include <iomanip>

const std::chrono::seconds cTurnTimeClock::alertRemainingTime (20);

//------------------------------------------------------------------------------
std::string to_MM_ss (std::chrono::milliseconds time)
{
	const auto minutes = std::chrono::duration_cast<std::chrono::minutes> (time);
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds> (time) - std::chrono::duration_cast<std::chrono::seconds> (minutes);

	std::stringstream text;

	text << std::setw (2) << std::setfill ('0') << minutes.count()
		 << ":"
		 << std::setw (2) << std::setfill ('0') << seconds.count();

	return text.str();
}

//------------------------------------------------------------------------------
cTurnTimeDeadline::cTurnTimeDeadline (unsigned int startGameTime_, const std::chrono::milliseconds& deadline_, unsigned int id_) :
	startGameTime (startGameTime_),
	deadline (deadline_),
	id (id_)
{}

//------------------------------------------------------------------------------
unsigned int cTurnTimeDeadline::getStartGameTime() const
{
	return startGameTime;
}

//------------------------------------------------------------------------------
const std::chrono::milliseconds& cTurnTimeDeadline::getDeadline() const
{
	return deadline;
}

//------------------------------------------------------------------------------
unsigned int cTurnTimeDeadline::getId() const
{
	return id;
}

//------------------------------------------------------------------------------
void cTurnTimeDeadline::changeDeadline (const std::chrono::milliseconds& deadline_)
{
	deadline = deadline_;
}

//------------------------------------------------------------------------------
uint32_t cTurnTimeDeadline::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (startGameTime, crc);
	crc = calcCheckSum (deadline.count(), crc);
	crc = calcCheckSum (id, crc);

	return crc;
}

//------------------------------------------------------------------------------
cTurnTimeClock::cTurnTimeClock (const cModel& model) :
	model (model)
{
	std::chrono::seconds lastCheckedSeconds (0);
	std::chrono::seconds lastTimeTillFirstDeadline (std::numeric_limits<std::chrono::seconds::rep>::max());
	signalConnectionManager.connect (model.gameTimeChanged, [lastCheckedSeconds, lastTimeTillFirstDeadline, this]() mutable {
		const std::chrono::seconds currentSeconds (this->model.getGameTime() / 100);
		if (currentSeconds != lastCheckedSeconds)
		{
			lastCheckedSeconds = currentSeconds;
			secondChanged();

			if (hasDeadline())
			{
				const auto currentTimeTillFirstDeadline = std::chrono::duration_cast<std::chrono::seconds> (getTimeTillFirstDeadline());
				if (lastTimeTillFirstDeadline > alertRemainingTime && currentTimeTillFirstDeadline <= alertRemainingTime)
				{
					alertTimeReached();
				}
				lastTimeTillFirstDeadline = currentTimeTillFirstDeadline;
			}
		}
	});
}

//------------------------------------------------------------------------------
void cTurnTimeClock::restartFromNow()
{
	startTurnGameTime = model.getGameTime();

	secondChanged();
}

//------------------------------------------------------------------------------
unsigned int cTurnTimeClock::getStartGameTime() const
{
	return startTurnGameTime;
}

//------------------------------------------------------------------------------
void cTurnTimeClock::clearAllDeadlines()
{
	deadlines.clear();
	deadlinesChanged();
}

//------------------------------------------------------------------------------
unsigned int cTurnTimeClock::startNewDeadlineFromNow (const std::chrono::milliseconds& duration)
{
	return startNewDeadlineFrom (model.getGameTime(), duration);
}

//------------------------------------------------------------------------------
unsigned int cTurnTimeClock::startNewDeadlineFrom (unsigned int gameTime, const std::chrono::milliseconds& duration)
{
	cTurnTimeDeadline deadline (gameTime, duration, nextDeadlineId);
	nextDeadlineId++;

	deadlines.push_back (deadline);
	deadlinesChanged();
	return deadline.getId();
}

//------------------------------------------------------------------------------
void cTurnTimeClock::removeDeadline (unsigned int id)
{
	auto it = ranges::find_if (deadlines, [id] (const auto& deadline) { return deadline.getId() == id; });
	if (it != deadlines.end())
	{
		deadlines.erase (it);
		deadlinesChanged();
	}
}

//------------------------------------------------------------------------------
void cTurnTimeClock::changeDeadline (unsigned int id, const std::chrono::seconds& duration)
{
	auto it = ranges::find_if (deadlines, [id] (const auto& deadline) { return deadline.getId() == id; });
	if (it != deadlines.end())
	{
		it->changeDeadline (duration);
		deadlinesChanged();
	}
}

//------------------------------------------------------------------------------
std::chrono::milliseconds cTurnTimeClock::getTimeSinceStart() const
{
	if (startTurnGameTime > model.getGameTime()) return std::chrono::milliseconds (0);

	const auto ticksSinceStart = model.getGameTime() - startTurnGameTime;
	return std::chrono::milliseconds (ticksSinceStart * GAME_TICK_TIME);
}

//------------------------------------------------------------------------------
std::chrono::milliseconds cTurnTimeClock::getTimeTillFirstDeadline() const
{
	if (deadlines.empty()) return std::chrono::milliseconds (0);

	auto minTime = getTimeTillDeadlineReached (deadlines[0]);
	for (auto it = deadlines.begin() + 1; it != deadlines.end(); ++it)
	{
		minTime = std::min (minTime, getTimeTillDeadlineReached (*it));
	}
	return minTime;
}

//------------------------------------------------------------------------------
bool cTurnTimeClock::hasReachedAnyDeadline() const
{
	return ranges::find_if (deadlines, [this] (const auto& deadline) { return this->getTimeTillDeadlineReached (deadline) <= std::chrono::milliseconds (0); }) != deadlines.end();
}

//------------------------------------------------------------------------------
uint32_t cTurnTimeClock::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (deadlines, crc);
	crc = calcCheckSum (startTurnGameTime, crc);
	crc = calcCheckSum (nextDeadlineId, crc);

	return crc;
}

//------------------------------------------------------------------------------
bool cTurnTimeClock::hasDeadline() const
{
	return !deadlines.empty();
}

//------------------------------------------------------------------------------
std::chrono::milliseconds cTurnTimeClock::getTimeTillDeadlineReached (const cTurnTimeDeadline& deadline) const
{
	const auto deadlineEndMilliseconds = deadline.getStartGameTime() * GAME_TICK_TIME + deadline.getDeadline().count();
	const auto currentTimeMilliseconds = model.getGameTime() * GAME_TICK_TIME;

	return std::chrono::milliseconds (deadlineEndMilliseconds < currentTimeMilliseconds ? 0 : deadlineEndMilliseconds - currentTimeMilliseconds);
}
