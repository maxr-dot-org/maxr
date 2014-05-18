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

#ifndef game_data_reports_savedreporttranslatedH
#define game_data_reports_savedreporttranslatedH

#include "../../../maxrconfig.h"

#include "savedreport.h"

class cSavedReportTranslated : public cSavedReport
{
public:
	cSavedReportTranslated (std::string translationText, bool isAlert = false);
	cSavedReportTranslated (std::string translationText, std::string insertText, bool isAlert = false);
	explicit cSavedReportTranslated (cNetMessage& message);
	explicit cSavedReportTranslated (const tinyxml2::XMLElement& element);

	virtual eSavedReportType getType () const MAXR_OVERRIDE_FUNCTION;

	virtual std::string getMessage () const MAXR_OVERRIDE_FUNCTION;

	virtual bool isAlert () const MAXR_OVERRIDE_FUNCTION;

	virtual void pushInto (cNetMessage& message) const MAXR_OVERRIDE_FUNCTION;
	virtual void pushInto (tinyxml2::XMLElement& element) const MAXR_OVERRIDE_FUNCTION;

private:
	std::string translationText;
	std::string insertText;

	bool alert;
};

#endif // game_data_reports_savedreporttranslatedH
