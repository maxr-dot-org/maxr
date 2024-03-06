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

#include "jobcontainer.h"

#include "game/data/model.h"
#include "game/data/units/unit.h"
#include "job.h"
#include "utility/crc.h"
#include "utility/log.h"
#include "utility/narrow_cast.h"
#include "utility/ranges.h"

#include <algorithm>

//------------------------------------------------------------------------------
void cJobContainer::addJob (cModel& model, std::unique_ptr<cJob> job)
{
	auto* unit = model.getUnitFromID (job->unitId);

	unit->jobActive = true;
	jobs.push_back (std::move (job));
}

//------------------------------------------------------------------------------
void cJobContainer::postLoad (const cModel& model)
{
	for (auto& job : jobs)
	{
		job->postLoad (model);
	}
}

//------------------------------------------------------------------------------
void cJobContainer::run (cModel& model)
{
	for (auto it = jobs.begin(); it != jobs.end();)
	{
		cJob& job = **it;

		if (!job.finished) job.run (model);

		if (job.finished)
			it = releaseJob (model, it);
		else
			++it;
	}
}

//------------------------------------------------------------------------------
uint32_t cJobContainer::getChecksum (uint32_t crc) const
{
	return calcCheckSum (jobs, crc);
}

//------------------------------------------------------------------------------
std::vector<std::unique_ptr<cJob>>::iterator cJobContainer::releaseJob (const cModel& model, std::vector<std::unique_ptr<cJob>>::iterator it)
{
	if (it == jobs.end()) return jobs.end();
	cJob& job = **it;
	auto* unit = model.getUnitFromID (job.unitId);

	if (unit)
	{
		auto nr = ranges::count_if (jobs, [&] (const auto& x) {
			return narrow_cast<unsigned> (x->unitId) == unit->getId();
		});
		if (nr <= 1)
		{
			unit->jobActive = false;
		}
	}
	return jobs.erase (it);
}

//------------------------------------------------------------------------------
void cJobContainer::onRemoveUnit (const cUnit& unit)
{
	for (auto& job : jobs)
	{
		if (narrow_cast<unsigned> (job->unitId) == unit.getId())
		{
			job->unitId = -1;
			job->finished = true;
		}
	}
}
