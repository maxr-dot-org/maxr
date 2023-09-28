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

#ifndef game_data_reports_special_savedreportupgradedH
#define game_data_reports_special_savedreportupgradedH

#include "game/data/report/savedreport.h"
#include "game/data/units/id.h"

class cSavedReportUpgraded : public cSavedReport
{
public:
	cSavedReportUpgraded (const sID& unitId, int unitsCount, int costs);
	template <typename Archive, ENABLE_ARCHIVE_IN>
	explicit cSavedReportUpgraded (Archive& archive)
	{
		serializeThis (archive);
	}

	void serialize (cBinaryArchiveOut& archive) override
	{
		cSavedReport::serialize (archive);
		serializeThis (archive);
	}
	void serialize (cJsonArchiveOut& archive) override
	{
		cSavedReport::serialize (archive);
		serializeThis (archive);
	}

	eSavedReportType getType() const override;

	bool isAlert() const override;

	sID getUnitId() const { return unitId; }
	int getUnitsCount() const { return unitsCount; }
	int getCosts() const { return costs; }

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (unitId);
		archive & NVP (unitsCount);
		archive & NVP (costs);
		// clang-format on
	}

	sID unitId;
	int unitsCount;
	int costs;
};

#endif // game_data_reports_special_savedreportupgradedH
