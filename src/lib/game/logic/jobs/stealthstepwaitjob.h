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

#ifndef game_logic_jobs_stealthstepwaitjobH
#define game_logic_jobs_stealthstepwaitjobH

#include "game/logic/jobs/job.h"
#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/jsonarchive.h"

#include <vector>

class cAction;
class cVehicle;

/**
* An undetected enemy stealth unit steps aside to free a field (see cModel::sideStepStealthUnit):
* the action, that needs the field, waits for the step and is executed again, when the step is done
* (like a move, that waits for the step, see cMoveJob::handleCollision).
* A stealth unit, that is still in the way then, is detected.
* Without an action, the vehicle goes on with its path building (see cVehicle::moveOnBuildPath).
*/
class cStealthStepWaitJob : public cJob
{
public:
	/**
	* @param unit the unit, whose action waits (the unit, that unloads, builds, clears, ...)
	* @param stealthVehicles the stealth units, that step aside
	* @param action the action to execute again or nullptr for the path building of unit
	*/
	cStealthStepWaitJob (const cUnit& unit, const std::vector<cVehicle*>& stealthVehicles, const cAction* action);
	template <ArchiveInOrOut Archive>
	explicit cStealthStepWaitJob (Archive& archive)
	{
		serializeThis (archive);
	}

	void run (cModel&) override;
	eJobType getType() const override;

	void serialize (cBinaryArchiveOut& archive) override
	{
		archive << serialization::makeNvp ("type", getType());
		serializeThis (archive);
	}
	void serialize (cJsonArchiveOut& archive) override
	{
		archive << serialization::makeNvp ("type", getType());
		serializeThis (archive);
	}

	void postLoad (const cModel&) override;

	uint32_t getChecksum (uint32_t crc) const override;

private:
	template <ArchiveInOrOut Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (unitId);
		archive & NVP (stealthVehicleIds);
		archive & NVP (action);
		archive & NVP (ticks);
		// clang-format on
	}

	std::vector<unsigned int> stealthVehicleIds;
	/** the serialized action (see cNetMessage::createFromBuffer), empty for the path building */
	std::vector<unsigned char> action;
	int ticks = 0;
};

#endif
