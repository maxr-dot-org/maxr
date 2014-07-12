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

#ifndef game_logic_turntimeclockH
#define game_logic_turntimeclockH

#include <memory>
#include <chrono>

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cGameTimer;

class cTurnTimeDeadline
{
public:
	cTurnTimeDeadline (unsigned int startGameTime, const std::chrono::milliseconds& deadline);

	unsigned int getStartGameTime () const;

	void changeDeadline (const std::chrono::milliseconds& deadline);

	std::chrono::milliseconds getTimeTillReached (unsigned int currentGameTime);

private:
	unsigned int startGameTime;
	std::chrono::milliseconds deadline;
};

class cTurnTimeClock
{
public:
	explicit cTurnTimeClock (std::shared_ptr<cGameTimer> gameTimer);

	void restartFromNow ();
	void restartFrom (unsigned int gameTime);

	unsigned int getStartGameTime () const;

	void clearAllDeadlines ();

	std::shared_ptr<cTurnTimeDeadline> startNewDeadlineFromNow (const std::chrono::milliseconds& deadline);
	std::shared_ptr<cTurnTimeDeadline> startNewDeadlineFrom (unsigned int gameTime, const std::chrono::milliseconds& deadline);

	void removeDeadline (const std::shared_ptr<cTurnTimeDeadline>& deadline);

	std::chrono::milliseconds getTimeSinceStart () const;
	std::chrono::milliseconds getTimeTillFirstDeadline () const;

	bool hasDeadline () const;

	bool hasReachedAnyDeadline () const;

	mutable cSignal<void ()> secondChanged;
	mutable cSignal<void ()> deadlinesChanged;
private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cGameTimer> gameTimer;
	std::vector<std::shared_ptr<cTurnTimeDeadline>> deadlines;

	unsigned int startTurnGameTime;
};

#endif // game_logic_turnclockH
