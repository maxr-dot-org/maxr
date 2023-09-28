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

#include "job.h"

#include "game/data/units/unit.h"
#include "game/logic/jobs/airtransportloadjob.h"
#include "game/logic/jobs/destroyjob.h"
#include "game/logic/jobs/getinjob.h"
#include "game/logic/jobs/planetakeoffjob.h"
#include "game/logic/jobs/startbuildjob.h"
#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/jsonarchive.h"

//------------------------------------------------------------------------------
cJob::cJob (const cUnit& unit) :
	unitId (unit.getId())
{}

//------------------------------------------------------------------------------
std::unique_ptr<cJob> cJob::createFrom (cBinaryArchiveIn& archive)
{
	return createFromImpl (archive);
}

//------------------------------------------------------------------------------
std::unique_ptr<cJob> cJob::createFrom (cJsonArchiveIn& archive)
{
	return createFromImpl (archive);
}

//------------------------------------------------------------------------------
template <typename Archive>
std::unique_ptr<cJob> cJob::createFromImpl (Archive& archive)
{
	eJobType type;
	archive >> NVP (type);

	switch (type)
	{
		case eJobType::START_BUILD:
			return std::make_unique<cStartBuildJob> (archive);
		case eJobType::PLANE_TAKEOFF:
			return std::make_unique<cPlaneTakeoffJob> (archive);
		case eJobType::DESTROY:
			return std::make_unique<cDestroyJob> (archive);
		case eJobType::GET_IN:
			return std::make_unique<cGetInJob> (archive);
		case eJobType::AIR_TRANSPORT_LOAD:
			return std::make_unique<cAirTransportLoadJob> (archive);
		default:
			throw std::runtime_error ("Unknown job type " + std::to_string (static_cast<int> (type)));
	}
}
