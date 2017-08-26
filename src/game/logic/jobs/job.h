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

#include <string>
#include <stdint.h>

class cJobContainer;
class cUnit;
class cModel;
class cBinaryArchiveOut;
class cBinaryArchiveIn;
class cXmlArchiveIn;
class cXmlArchiveOut;

enum class eJobType
{
	START_BUILD,
	PLANE_TAKEOFF,
	DESTROY
};

/**
* little helper jobs for game time synchronous actions,
* like rotating a unit to a specific direction or landing/takeoff
*/
class cJob
{
	friend class cJobContainer;
protected:
	explicit cJob (cUnit& unit);
	cJob();
public:
	virtual ~cJob() {}
	virtual void run (cModel& model) = 0;
	virtual eJobType getType() const = 0;

	static cJob* createFrom(cBinaryArchiveOut& archive,  const std::string& name);
	static cJob* createFrom(cXmlArchiveOut& archive, const std::string& name);

	virtual void serialize(cBinaryArchiveIn& archive) = 0;
	virtual void serialize(cXmlArchiveIn& archive) = 0;

	virtual uint32_t getChecksum(uint32_t crc) const = 0;
protected:
	bool finished;
	cUnit* unit;
private:
	template <typename T>
	static cJob* createFromImpl(T& archive);
};

#endif // game_logic_jobsH
