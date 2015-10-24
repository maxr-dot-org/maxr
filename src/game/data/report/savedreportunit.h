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

#include "maxrconfig.h"

#include "game/data/report/savedreport.h"
#include "main.h" // sID
#include "utility/position.h"

class cUnit;

class cSavedReportUnit : public cSavedReport
{
public:
	explicit cSavedReportUnit (const cUnit& unit);
	template <typename T, ENABLE_ARCHIVE_OUT>
	explicit cSavedReportUnit(T& archive)
	{
		serializeThis(archive);
	}

	virtual std::string getMessage() const MAXR_OVERRIDE_FUNCTION;

	virtual bool isAlert() const MAXR_OVERRIDE_FUNCTION;

	virtual void serialize(cBinaryArchiveIn& archive) { cSavedReport::serialize(archive); serializeThis(archive); }
	virtual void serialize(cXmlArchiveIn& archive) { cSavedReport::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { cSavedReport::serialize(archive); serializeThis(archive); }

	virtual bool hasUnitId() const MAXR_OVERRIDE_FUNCTION;
	virtual const sID& getUnitId() const MAXR_OVERRIDE_FUNCTION;

	virtual bool hasPosition() const MAXR_OVERRIDE_FUNCTION;
	virtual const cPosition& getPosition() const MAXR_OVERRIDE_FUNCTION;

protected:
	virtual std::string getText() const = 0;

private:
	template <typename T>
	void serializeThis(T& archive)
	{
		archive & NVP(unitId);
		archive & NVP(position);
	}

	sID unitId;
	cPosition position;
};

#endif // game_data_reports_savedreportunitH
