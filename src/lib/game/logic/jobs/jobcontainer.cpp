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
#include "utility/narrow_cast.h"

#include <algorithm>

//------------------------------------------------------------------------------
void cJobContainer::addJob (cModel& model, std::unique_ptr<cJob> job)
{
	auto* unit = model.getUnitFromID (job->unitId);

	unit->jobActive = true;
	if (running)
		addedJobs.push_back (std::move (job));
	else
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
	running = true;
	for (auto it = jobs.begin(); it != jobs.end();)
	{
		cJob& job = **it;

		if (!job.finished) job.run (model);

		if (job.finished)
			it = releaseJob (model, it);
		else
			++it;
	}
	running = false;

	for (auto& job : addedJobs)
	{
		// (releaseJob may have reset the flag of the unit of a job added during the run)
		if (auto* unit = model.getUnitFromID (job->unitId)) unit->jobActive = true;
		jobs.push_back (std::move (job));
	}
	addedJobs.clear();
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
		auto nr = std::ranges::count_if (jobs, [&] (const auto& x) {
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
	for (auto* list : {&jobs, &addedJobs})
	{
		for (auto& job : *list)
		{
			if (narrow_cast<unsigned> (job->unitId) == unit.getId())
			{
				job->unitId = -1;
				job->finished = true;
			}
		}
	}
}
