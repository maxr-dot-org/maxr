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

#include "ui/graphical/menu/widgets/special/reportdisadvantageslistviewitem.h"

#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "utility/string/toString.h"

const int cReportDisadvantagesListViewItem::unitImageWidth = 32;
const int cReportDisadvantagesListViewItem::unitImageHeight = 32;
const int cReportDisadvantagesListViewItem::unitNameWidth = 110;
const int cReportDisadvantagesListViewItem::casualityLabelWidth = 75;
const int cReportDisadvantagesListViewItem::maxItemsInRow = 4;

//------------------------------------------------------------------------------
cReportDisadvantagesListViewItem::cReportDisadvantagesListViewItem (const cStaticUnitData& data, std::vector<int> disadvantages_) :
	unitId (data.ID),
	disadvantages (disadvantages_)
{
	auto font = cUnicodeFont::font.get();

	const auto totalHeight = std::max (unitImageHeight, (int) (disadvantages.size() / maxItemsInRow + 1) * font->getFontHeight());

	AutoSurface unitSurface (SDL_CreateRGBSurface (0, unitImageWidth, unitImageHeight, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (unitSurface.get(), SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (unitSurface.get(), nullptr, 0x00FF00FF);
	SDL_Rect dest = {0, 0, 0, 0};

	if (unitId.isAVehicle())
	{
		const float zoomFactor = unitImageWidth / 64.0f;
		const auto& uiData = *UnitsUiData.getVehicleUI (unitId);
		cVehicle::render_simple (unitSurface.get(), dest, zoomFactor, uiData, nullptr);
		cVehicle::drawOverlayAnimation (unitSurface.get(), dest, zoomFactor, uiData);
	}
	else if (unitId.isABuilding())
	{
		const float zoomFactor = unitImageWidth / (data.isBig ? 128.0f : 64.0f);
		const auto& uiData = *UnitsUiData.getBuildingUI (unitId);
		cBuilding::render_simple (unitSurface.get(), dest, zoomFactor, uiData, nullptr);
	}
	auto unitImage = addChild (std::make_unique<cImage> (cPosition (0, (totalHeight - unitImageHeight) / 2), unitSurface.get()));

	auto nameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImage->getEndPosition().x(), 0), cPosition (unitImage->getEndPosition().x() + unitNameWidth, totalHeight)), data.getName(), FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
	nameLabel->setWordWrap (true);
	nameLabel->setConsumeClick (false);

	for (size_t i = 0; i < disadvantages.size(); ++i)
	{
		const auto disadvantage = disadvantages[i];

		const int row = static_cast<int> (i / maxItemsInRow);
		const int col = static_cast<int> (i % maxItemsInRow);

		auto disadvantageLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (nameLabel->getEndPosition().x() + casualityLabelWidth * col + (row % 2 == 0 ? 15 : 0), font->getFontHeight()*row), cPosition (nameLabel->getEndPosition().x() + casualityLabelWidth * (col + 1) + (row % 2 == 0 ? 15 : 0), font->getFontHeight() * (row + 1))), iToStr (disadvantage), FONT_LATIN_NORMAL, eAlignmentType::Center));
		disadvantageLabel->setConsumeClick (false);
	}

	fitToChildren();
}
