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

#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/report/savedreport.h"
#include "game/data/report/savedreportunit.h"
#include "resources/buildinguidata.h"
#include "resources/vehicleuidata.h"
#include "SDLutility/drawing.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "utility/color.h"

//------------------------------------------------------------------------------
cReportMessageListViewItem::cReportMessageListViewItem (const cSavedReport& report_, const cUnitsData& unitsData) :
	report (report_)
{
	const int unitImageSize = 32;

	auto textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImageSize, 0), cPosition (450, 0)), report.getMessage(unitsData), FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
	textLabel->setWordWrap (true);
	textLabel->resizeToTextHeight();
	textLabel->setConsumeClick (false);

	if (report.hasUnitId())
	{
		const auto& unitId = report.getUnitId();

		const auto totalHeight = std::max (unitImageSize, textLabel->getSize().y());

		AutoSurface unitSurface (SDL_CreateRGBSurface (0, unitImageSize, unitImageSize, Video.getColDepth(), 0, 0, 0, 0));
		SDL_SetColorKey (unitSurface.get(), SDL_TRUE, 0x00FF00FF);
		SDL_FillRect (unitSurface.get(), nullptr, 0x00FF00FF);
		SDL_Rect dest = {0, 0, 0, 0};

		const cStaticUnitData& data = unitsData.getStaticUnitData(unitId);
		if (unitId.isAVehicle())
		{
			const float zoomFactor = unitImageSize / 64.0f;
			const auto& uiData = *UnitsUiData.getVehicleUI (unitId);
			uiData.render_simple (*unitSurface, dest, zoomFactor, nullptr);
			uiData.drawOverlayAnimation (*unitSurface, dest, zoomFactor);
		}
		else if (unitId.isABuilding())
		{
			const float zoomFactor = unitImageSize / (data.buildingData.isBig ? 128.0f : 64.0f);
			const auto& uiData = *UnitsUiData.getBuildingUI (unitId);
			uiData.render_simple (*unitSurface, dest, zoomFactor, nullptr);
		}
		addChild (std::make_unique<cImage> (cPosition (0, (totalHeight - unitImageSize) / 2), unitSurface.get()));
	}

	fitToChildren();
}

//------------------------------------------------------------------------------
void cReportMessageListViewItem::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	cAbstractListViewItem::draw (destination, clipRect);

	if (isSelected())
	{
		auto dest = getArea();
		dest.getMinCorner() -= cPosition (1, 1);
		dest.getMaxCorner() += cPosition (1, 1);
		drawRectangle (destination, dest, cRgbColor (0xE0, 0xE0, 0xE0));
	}
}

//------------------------------------------------------------------------------
const cSavedReport& cReportMessageListViewItem::getReport() const
{
	return report;
}
