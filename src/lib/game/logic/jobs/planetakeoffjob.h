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

#ifndef game_logic_jobs_planetakeoffjobH
#define game_logic_jobs_planetakeoffjobH

#include "game/data/units/unit.h"
#include "game/logic/jobs/job.h"
#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/jsonarchive.h"
#include "utility/signal/signalconnectionmanager.h"

class cVehicle;

class cPlaneTakeoffJob : public cJob
{
public:
	cPlaneTakeoffJob (cVehicle& vehicle);
	template <typename Archive>
	cPlaneTakeoffJob (Archive& archive)
	{
		serializeThis (archive);
	}

	void run (cModel& model) override;
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
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (unitId);
		// clang-format on
	}

	cSignalConnectionManager connectionManager;
};

#endif
