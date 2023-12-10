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

#include "reportunitlistviewitem.h"

#include "SDLutility/drawing.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "game/data/units/vehicle.h"
#include "resources/buildinguidata.h"
#include "resources/vehicleuidata.h"
#include "ui/graphical/game/widgets/unitdetailshud.h"
#include "ui/translations.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/color.h"

//------------------------------------------------------------------------------
cReportUnitListViewItem::cReportUnitListViewItem (cUnit& unit_, const cUnitsData& unitsData_) :
	cAbstractListViewItem(),
	unit (unit_),
	unitsData (unitsData_)
{
	const int unitImageSize = 32;
	UniqueSurface surface (SDL_CreateRGBSurface (0, unitImageSize, unitImageSize, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (surface.get(), nullptr, 0x00FF00FF);
	SDL_Rect dest = {0, 0, 0, 0};

	if (unit.data.getId().isAVehicle())
	{
		const auto& vehicle = static_cast<const cVehicle&> (unit);
		const float zoomFactor = unitImageSize / 64.0f;
		render_simple (vehicle, *surface, dest, zoomFactor);
		drawOverlayAnimation (vehicle, *surface, dest, zoomFactor, 0);
	}
	else if (unit.data.getId().isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (unit);
		const float zoomFactor = unitImageSize / (building.getIsBig() ? 128.0f : 64.0f);
		render_simple (building, *surface, dest, zoomFactor, 0);
	}
	else
		surface = nullptr;

	auto unitDetails = emplaceChild<cUnitDetailsHud> (cBox<cPosition> (cPosition (unitImageSize + 3 + 75 + 3, 0), cPosition (unitImageSize + 3 + 75 + 3 + 155, 48)), true);
	unitDetails->setPlayer (unit.getOwner());
	unitDetails->setUnit (&unit);

	unitImage = emplaceChild<cImage> (cPosition (0, (unitDetails->getSize().y() - unitImageSize) / 2), surface.get());
	unitImage->setConsumeClick (false);

	auto nameLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (unitImage->getEndPosition().x() + 3, 0), cPosition (unitDetails->getPosition().x() - 3, unitDetails->getEndPosition().y())), getDisplayName (unit), eUnicodeFontType::LatinNormal, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical);
	nameLabel->setWordWrap (true);
	nameLabel->setConsumeClick (false);

	auto positionLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (unitDetails->getEndPosition().x() + 5, 0), cPosition (unitDetails->getEndPosition().x() + 5 + 50, unitDetails->getEndPosition().y())), std::to_string (unit.getPosition().x()) + "," + std::to_string (unit.getPosition().y()), eUnicodeFontType::LatinNormal, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::CenterVerical);
	positionLabel->setConsumeClick (false);

	statusLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (positionLabel->getEndPosition().x(), 0), cPosition (positionLabel->getEndPosition().x() + 120, unitDetails->getEndPosition().y())), getStatusStr (unit, unit.getOwner(), unitsData), eUnicodeFontType::LatinNormal, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical);
	statusLabel->setConsumeClick (false);

	fitToChildren();
}

//------------------------------------------------------------------------------
void cReportUnitListViewItem::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	cAbstractListViewItem::draw (destination, clipRect);

	if (isSelected())
	{
		auto dest = unitImage->getArea();
		dest.getMinCorner() -= cPosition (1, 1);
		dest.getMaxCorner() += cPosition (1, 1);
		drawSelectionCorner (destination, dest, cRgbColor (224, 224, 224), 8, cBox<cPosition> (clipRect.getMinCorner() - 1, clipRect.getMaxCorner() + 1));
	}
}

//------------------------------------------------------------------------------
void cReportUnitListViewItem::retranslate()
{
	cAbstractListViewItem::retranslate();
	statusLabel->setText (getStatusStr (unit, unit.getOwner(), unitsData));
}

//------------------------------------------------------------------------------
cUnit& cReportUnitListViewItem::getUnit() const
{
	return unit;
}
