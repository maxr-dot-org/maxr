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

#include "savedreport.h"

#include "savedreportchat.h"
#include "savedreportsimple.h"
#include "savedreporttranslated.h"
#include "savedreportunit.h"

#include "../../../netmessage.h"
#include "../../../utility/tounderlyingtype.h"
#include "../../../main.h"

void cSavedReport::pushInto (cNetMessage& message) const
{
	message.pushInt32 (toUnderlyingType(getType()));
}

void cSavedReport::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("type", iToStr (toUnderlyingType (getType ())).c_str ());
}

std::unique_ptr<cSavedReport> cSavedReport::createFrom (cNetMessage& message)
{
	auto type = (eSavedReportType)message.popInt32 ();

	switch (type)
	{
	case eSavedReportType::Chat:
		return std::make_unique<cSavedReportChat> (message);
	case eSavedReportType::Unit:
		return std::make_unique<cSavedReportUnit> (message);
	case eSavedReportType::Simple:
		return std::make_unique<cSavedReportSimple> (message);
	case eSavedReportType::Translated:
		return std::make_unique<cSavedReportTranslated> (message);
	default:
		return nullptr;
	}
}

std::unique_ptr<cSavedReport> cSavedReport::createFrom (const tinyxml2::XMLElement& element)
{
	auto type = (eSavedReportType)element.IntAttribute ("type");

	switch (type)
	{
	case eSavedReportType::Chat:
		return std::make_unique<cSavedReportChat> (element);
	case eSavedReportType::Unit:
		return std::make_unique<cSavedReportUnit> (element);
	case eSavedReportType::Simple:
		return std::make_unique<cSavedReportSimple> (element);
	case eSavedReportType::Translated:
		return std::make_unique<cSavedReportTranslated> (element);
	default:
		return nullptr;
	}
}
