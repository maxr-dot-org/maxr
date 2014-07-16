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

#include "game/data/report/unit/savedreportsurveyoraisenseless.h"

//------------------------------------------------------------------------------
cSavedReportSurveyorAiSenseless::cSavedReportSurveyorAiSenseless (const cUnit& unit) :
	cSavedReportUnit (unit)
{}

//------------------------------------------------------------------------------
cSavedReportSurveyorAiSenseless::cSavedReportSurveyorAiSenseless (cNetMessage& message) :
	cSavedReportUnit (message)
{}

//------------------------------------------------------------------------------
cSavedReportSurveyorAiSenseless::cSavedReportSurveyorAiSenseless (const tinyxml2::XMLElement& element) :
	cSavedReportUnit (element)
{}

//------------------------------------------------------------------------------
void cSavedReportSurveyorAiSenseless::pushInto (cNetMessage& message) const
{
	cSavedReportUnit::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportSurveyorAiSenseless::pushInto (tinyxml2::XMLElement& element) const
{
	cSavedReportUnit::pushInto (element);
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportSurveyorAiSenseless::getType () const
{
	return eSavedReportType::SurveyorAiSenseless;
}

//------------------------------------------------------------------------------
std::string cSavedReportSurveyorAiSenseless::getText () const
{
	return "Surveyor AI: My life is so senseless. I've nothing to do...";
}