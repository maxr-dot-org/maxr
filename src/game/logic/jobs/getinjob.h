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

#ifndef game_logic_jobs_getinjobH
#define game_logic_jobs_getinjobH

#include "job.h"

#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/xmlarchive.h"
#include "game/data/units/unit.h"
#include "utility/signal/signalconnectionmanager.h"

class cVehicle;

/**
* creates the animation, when a vehicle is loaded. This is a
* job, not a gui animation, because it triggers an action on the model.
*/
class cGetInJob : public cJob
{
public:
	cGetInJob (cVehicle& loadedVehicle, cUnit& loadingUnit);
	template <typename T>
	cGetInJob(T& archive);

	void run (cModel& model) override;
	eJobType getType() const override;

	void serialize(cBinaryArchiveIn& archive) override { archive << serialization::makeNvp("type", getType()); serializeThis(archive); }
	void serialize(cXmlArchiveIn& archive) override { archive << serialization::makeNvp("type", getType()); serializeThis(archive); }

	uint32_t getChecksum(uint32_t crc) const override;
private:
	template <typename T>
	void serializeThis(T& archive)
	{
		archive & NVP(unit);
		archive & NVP(loadingUnit);
		archive & NVP(counter);
		archive & NVP(startFlightHeight);
	}

	cUnit* loadingUnit;
	cSignalConnectionManager connectionManager;
	int counter;
	int startFlightHeight;
};

template <typename T>
cGetInJob::cGetInJob(T& archive)
{
	serializeThis(archive);
	if (!unit || !loadingUnit)
	{
		finished = true;
		return;
	}
	connectionManager.connect(loadingUnit->destroyed, [&](){finished = true; });
	unit->jobActive = true;
}

#endif
