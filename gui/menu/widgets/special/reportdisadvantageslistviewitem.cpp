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

#include "reportdisadvantageslistviewitem.h"
#include "../image.h"
#include "../label.h"
#include "../../../../buildings.h"
#include "../../../../vehicles.h"

//------------------------------------------------------------------------------
cReportDisadvantagesListViewItem::cReportDisadvantagesListViewItem (const sID& unitId_, std::vector<int> disadvantages_) :
	unitId (unitId_),
	disadvantages (disadvantages_)
{
	const auto totalHeight = std::max (unitImageHeight, (int)(disadvantages.size () / maxItemsInRow + 1) * font->getFontHeight ());

	AutoSurface unitSurface;
	if (unitId.isABuilding ())
	{
        unitSurface = AutoSurface (scaleSurface (UnitsData.getBuildingUI (unitId)->img_org.get (), nullptr, unitImageWidth, unitImageHeight));
	}
	else if (unitId.isAVehicle ())
	{
        unitSurface = AutoSurface (scaleSurface (UnitsData.getVehicleUI (unitId)->img_org[0].get (), nullptr, unitImageWidth, unitImageHeight));
	}
	auto unitImage = addChild (std::make_unique<cImage> (cPosition (0, (totalHeight - unitImageHeight) / 2), unitSurface.get ()));

	auto nameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImage->getEndPosition ().x (), 0), cPosition (unitImage->getEndPosition ().x () + unitNameWidth, totalHeight)), unitId.getUnitDataOriginalVersion ()->name, FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
	nameLabel->setWordWrap (true);

	for (size_t i = 0; i < disadvantages.size (); ++i)
	{
		const auto disadvantage = disadvantages[i];

		const int row = static_cast<int>(i / maxItemsInRow);
		const int col = static_cast<int>(i % maxItemsInRow);

		addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (nameLabel->getEndPosition ().x () + casualityLabelWidth * col + (row % 2 == 0 ? 15 : 0), font->getFontHeight ()*row), cPosition (nameLabel->getEndPosition ().x () + casualityLabelWidth * (col+1) + (row % 2 == 0 ? 15 : 0), font->getFontHeight ()*(row+1))), iToStr (disadvantage), FONT_LATIN_NORMAL, eAlignmentType::Center));
	}

	fitToChildren ();
}
