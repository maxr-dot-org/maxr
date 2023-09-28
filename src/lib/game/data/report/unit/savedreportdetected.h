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

#ifndef game_data_reports_savedreportdetectedH
#define game_data_reports_savedreportdetectedH

#include "game/data/report/savedreportunit.h"

class cModel;

class cSavedReportDetected : public cSavedReportUnit
{
public:
	explicit cSavedReportDetected (const cUnit&);

	template <typename Archive, ENABLE_ARCHIVE_IN>
	explicit cSavedReportDetected (Archive& archive) :
		cSavedReportUnit (archive)
	{
		serializeThis (archive);
	}

	eSavedReportType getType() const override;
	bool isSubmarine (const cModel&) const;

	void serialize (cBinaryArchiveIn& archive) override
	{
		cSavedReportUnit::serialize (archive);
		serializeThis (archive);
	}
	void serialize (cJsonArchiveOut& archive) override
	{
		cSavedReportUnit::serialize (archive);
		serializeThis (archive);
	}

	std::string getPlayerOwnerName() const { return playerOwnerName; }

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (playerOwnerName);
		// clang-format on
	}

private:
	std::string playerOwnerName;
};

#endif
