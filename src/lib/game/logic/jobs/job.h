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

#ifndef game_logic_jobs_jobH
#define game_logic_jobs_jobH

#include <cstdint>
#include <memory>
#include <string>

class cJobContainer;
class cUnit;
class cModel;
class cBinaryArchiveIn;
class cBinaryArchiveOut;
class cJsonArchiveIn;
class cJsonArchiveOut;

enum class eJobType
{
	START_BUILD,
	PLANE_TAKEOFF,
	DESTROY,
	AIR_TRANSPORT_LOAD,
	GET_IN
};

/**
* little helper jobs for game time synchronous actions,
* like rotating a unit to a specific direction or landing/takeoff
*/
class cJob
{
	friend class cJobContainer;

protected:
	cJob() = default;
	explicit cJob (const cUnit&);

public:
	virtual ~cJob() = default;
	virtual void run (cModel&) = 0;
	virtual eJobType getType() const = 0;

	static std::unique_ptr<cJob> createFrom (cBinaryArchiveIn&);
	static std::unique_ptr<cJob> createFrom (cJsonArchiveIn&);

	virtual void serialize (cBinaryArchiveOut&) = 0;
	virtual void serialize (cJsonArchiveOut&) = 0;

	virtual void postLoad (const cModel&) {}

	virtual uint32_t getChecksum (uint32_t crc) const = 0;

protected:
	bool finished = false;
	int unitId = -1;

private:
	template <typename Archive>
	static std::unique_ptr<cJob> createFromImpl (Archive&);
};

#endif // game_logic_jobsH
