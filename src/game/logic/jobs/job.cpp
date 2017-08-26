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
#include <cassert>

#include "job.h"

#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/xmlarchive.h"
#include "startbuildjob.h"
#include "planetakeoffjob.h"
#include "destroyjob.h"
#include "utility/string/toString.h"

cJob::cJob (cUnit& unit) :
	finished (false),
	unit (&unit)
{}

cJob::cJob() :
	finished(false),
	unit(nullptr)
{}


cJob* cJob::createFrom(cBinaryArchiveOut& archive, const std::string& name)
{
	return createFromImpl(archive);
}

cJob* cJob::createFrom(cXmlArchiveOut& archive, const std::string& name)
{
	archive.enterChild(name);
	auto job = createFromImpl(archive);
	archive.leaveChild();
	return job;
}

template <typename T>
cJob* cJob::createFromImpl(T& archive)
{
	eJobType type;
	archive >> NVP(type);

	switch (type)
	{
	case eJobType::START_BUILD:
		return new cStartBuildJob(archive);
	case eJobType::PLANE_TAKEOFF:
		return new cPlaneTakeoffJob(archive);
	case eJobType::DESTROY:
		return new cDestroyJob(archive);
	default:
		throw std::runtime_error("Unknown job type " + toString(static_cast<int>(type)));
		break;
	}
}
