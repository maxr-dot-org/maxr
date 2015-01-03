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

#ifndef game_data_reports_savedreportcapturedbyenemyH
#define game_data_reports_savedreportcapturedbyenemyH

#include "maxrconfig.h"

#include "game/data/report/savedreportunit.h"

class cUnit;

class cSavedReportCapturedByEnemy : public cSavedReportUnit
{
public:
	cSavedReportCapturedByEnemy (const cUnit& unit);
	explicit cSavedReportCapturedByEnemy (cNetMessage& message);
	explicit cSavedReportCapturedByEnemy (const tinyxml2::XMLElement& element);

	virtual void pushInto (cNetMessage& message) const MAXR_OVERRIDE_FUNCTION;
	virtual void pushInto (tinyxml2::XMLElement& element) const MAXR_OVERRIDE_FUNCTION;

	virtual eSavedReportType getType() const MAXR_OVERRIDE_FUNCTION;

	virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;

protected:
	virtual std::string getText() const MAXR_OVERRIDE_FUNCTION;

private:
	std::string unitName;
};

#endif // game_data_reports_savedreportcapturedbyenemyH
