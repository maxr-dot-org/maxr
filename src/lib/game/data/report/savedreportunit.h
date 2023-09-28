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

#ifndef game_data_reports_savedreportunitH
#define game_data_reports_savedreportunitH

#include "game/data/report/savedreport.h"
#include "game/data/units/id.h"
#include "game/data/units/unit.h"
#include "utility/position.h"

class cUnit;

class cSavedReportUnit : public cSavedReport
{
public:
	explicit cSavedReportUnit (const cUnit&);

	template <typename Archive, ENABLE_ARCHIVE_IN>
	explicit cSavedReportUnit (Archive& archive)
	{
		serializeThis (archive);
	}

	bool isAlert() const override;

	void serialize (cBinaryArchiveIn& archive) override
	{
		cSavedReport::serialize (archive);
		serializeThis (archive);
	}
	void serialize (cJsonArchiveOut& archive) override
	{
		cSavedReport::serialize (archive);
		serializeThis (archive);
	}

	sID getUnitId() const { return unitId; }
	int getUnitVersion() const { return version; }
	std::optional<std::string> getUnitCustomName() const { return customName; }
	std::optional<cPosition> getPosition() const override;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (unitId);
		archive & NVP (version);
		archive & NVP (customName);
		archive & NVP (position);
		// clang-format on
	}

	sID unitId;
	unsigned int version;
	std::optional<std::string> customName;
	cPosition position;
};

#endif
