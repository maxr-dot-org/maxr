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

#include "utility/serialization/nvp.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <chrono>
#include <memory>
#include <string>

class cModel;

class cTurnTimeDeadline
{
public:
	cTurnTimeDeadline() = default;
	cTurnTimeDeadline (unsigned int startGameTime, const std::chrono::milliseconds& deadline, unsigned int id);

	unsigned int getStartGameTime() const;
	const std::chrono::milliseconds& getDeadline() const;
	unsigned int getId() const;

	void changeDeadline (const std::chrono::milliseconds& deadline);

	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (startGameTime);
		archive & NVP (deadline);
		archive & NVP (id);
		// clang-format on
	}

private:
	unsigned int startGameTime = 0;
	std::chrono::milliseconds deadline{0};
	unsigned int id = 0;
};

class cTurnTimeClock
{
public:
	static const std::chrono::seconds alertRemainingTime;

	explicit cTurnTimeClock (const cModel& model);

	void restartFromNow();

	unsigned int getStartGameTime() const;

	void clearAllDeadlines();

	unsigned int startNewDeadlineFromNow (const std::chrono::milliseconds& duration);
	unsigned int startNewDeadlineFrom (unsigned int gameTime, const std::chrono::milliseconds& duration);

	void removeDeadline (unsigned int id);
	void changeDeadline (unsigned int turnEndDeadline, const std::chrono::seconds& duration);

	std::chrono::milliseconds getTimeSinceStart() const;
	std::chrono::milliseconds getTimeTillFirstDeadline() const;

	bool hasDeadline() const;

	bool hasReachedAnyDeadline() const;

	mutable cSignal<void()> secondChanged;
	mutable cSignal<void()> deadlinesChanged;
	mutable cSignal<void()> alertTimeReached;

	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (deadlines);
		archive & NVP (startTurnGameTime);
		archive & NVP (nextDeadlineId);
		// clang-format on
	}

private:
	cSignalConnectionManager signalConnectionManager;

	const cModel& model;
	std::vector<cTurnTimeDeadline> deadlines;

	unsigned int nextDeadlineId = 1;
	unsigned int startTurnGameTime = 0;

	std::chrono::milliseconds getTimeTillDeadlineReached (const cTurnTimeDeadline& deadline) const;
};

std::string to_MM_ss (std::chrono::milliseconds);

#endif // game_logic_turnclockH
