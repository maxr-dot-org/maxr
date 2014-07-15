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

#include "ui/graphical/menu/widgets/special/reportmessagelistviewitem.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "buildings.h"
#include "vehicles.h"
#include "game/data/report/savedreport.h"
#include "game/data/report/savedreportunit.h"
#include "utility/drawing.h"
#include "utility/color.h"

//------------------------------------------------------------------------------
cReportMessageListViewItem::cReportMessageListViewItem (const cSavedReport& report_) :
	report (report_)
{
	const int unitImageSize = 32;

	auto textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImageSize, 0), cPosition (450, 0)), report.getMessage (), FONT_LATIN_NORMAL, toEnumFlag(eAlignmentType::Left) | eAlignmentType::CenterVerical));
	textLabel->setWordWrap (true);
	textLabel->resizeToTextHeight ();

	if (report.getType () == eSavedReportType::Unit)
	{
		const auto& unitReport = static_cast<const cSavedReportUnit&>(report);

		const auto& unitId = unitReport.getUnitId ();

		const auto totalHeight = std::max (unitImageSize, textLabel->getSize ().y ());

		AutoSurface unitSurface;
		if (unitId.isABuilding ())
		{
            unitSurface = AutoSurface (scaleSurface (UnitsData.getBuildingUI (unitId)->img_org.get (), nullptr, unitImageSize, unitImageSize));
		}
		else if (unitId.isAVehicle ())
		{
            unitSurface = AutoSurface (scaleSurface (UnitsData.getVehicleUI (unitId)->img_org[0].get (), nullptr, unitImageSize, unitImageSize));
		}
		addChild (std::make_unique<cImage> (cPosition (0, (totalHeight - unitImageSize) / 2), unitSurface.get ()));
	}

	fitToChildren ();
}

//------------------------------------------------------------------------------
void cReportMessageListViewItem::draw ()
{
	cAbstractListViewItem::draw ();

	if (isSelected ())
	{
		auto dest = getArea ();
		dest.getMinCorner () -= cPosition (1, 1);
		dest.getMaxCorner () += cPosition (1, 1);
		drawRectangle (*cVideo::buffer, dest, cColor (0xE0, 0xE0, 0xE0));
	}
}

//------------------------------------------------------------------------------
const cSavedReport& cReportMessageListViewItem::getReport () const
{
	return report;
}
