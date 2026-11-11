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

#include "stealthstepwaitjob.h"

#include "game/data/model.h"
#include "game/data/units/vehicle.h"
#include "game/logic/action/action.h"
#include "game/logic/movejob.h"
#include "utility/crc.h"

#include <algorithm>

namespace
{
	// a step takes some ticks (rotation and move), the wait must end in any case
	constexpr int maxWaitTicks = 300;

	//--------------------------------------------------------------------------
	class cStealthStepWaitedGuard
	{
	public:
		explicit cStealthStepWaitedGuard (cModel& model) :
			model (model)
		{
			model.setStealthStepWaited (true);
		}
		~cStealthStepWaitedGuard() { model.setStealthStepWaited (false); }

		cStealthStepWaitedGuard (const cStealthStepWaitedGuard&) = delete;
		cStealthStepWaitedGuard& operator= (const cStealthStepWaitedGuard&) = delete;

	private:
		cModel& model;
	};
} // namespace

//------------------------------------------------------------------------------
cStealthStepWaitJob::cStealthStepWaitJob (const cUnit& unit, const std::vector<cVehicle*>& stealthVehicles, const cAction* action_) :
	cJob (unit)
{
	for (const auto* stealthVehicle : stealthVehicles)
	{
		stealthVehicleIds.push_back (stealthVehicle->getId());
	}
	if (action_)
	{
		cBinaryArchiveOut archive (action);
		archive << *action_;
	}
}

//------------------------------------------------------------------------------
void cStealthStepWaitJob::postLoad (const cModel& model)
{
	if (auto* unit = model.getUnitFromID (unitId))
	{
		unit->jobActive = true;
	}
}

//------------------------------------------------------------------------------
void cStealthStepWaitJob::run (cModel& model)
{
	++ticks;
	const bool stepping = std::ranges::any_of (stealthVehicleIds, [&] (unsigned int id) {
		const cVehicle* stealthVehicle = model.getVehicleFromID (id);
		return stealthVehicle && stealthVehicle->getMoveJob() && stealthVehicle->getMoveJob()->isActive();
	});
	if (stepping && ticks < maxWaitTicks) return;

	finished = true;

	cStealthStepWaitedGuard guard (model);
	if (action.empty())
	{
		if (auto* vehicle = model.getVehicleFromID (unitId))
		{
			vehicle->moveOnBuildPath (model);
		}
		return;
	}
	auto message = cNetMessage::createFromBuffer (action.data(), static_cast<int> (action.size()));
	if (message && message->getType() == eNetMessageType::ACTION)
	{
		static_cast<const cAction&> (*message).execute (model);
	}
}

//------------------------------------------------------------------------------
eJobType cStealthStepWaitJob::getType() const
{
	return eJobType::STEALTH_STEP_WAIT;
}

//------------------------------------------------------------------------------
uint32_t cStealthStepWaitJob::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (getType(), crc);
	crc = calcCheckSum (unitId, crc);
	crc = calcCheckSum (stealthVehicleIds, crc);
	crc = calcCheckSum (action, crc);
	crc = calcCheckSum (ticks, crc);

	return crc;
}
