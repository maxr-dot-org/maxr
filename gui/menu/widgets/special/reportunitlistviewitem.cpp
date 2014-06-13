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
#include "../image.h"
#include "../label.h"
#include "../../../game/widgets/unitdetailshud.h"
#include "../../../../unit.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"

//------------------------------------------------------------------------------
cReportUnitListViewItem::cReportUnitListViewItem (cUnit& unit_) :
	cAbstractListViewItem (),
	unit (unit_)
{
	const int unitImageSize = 32;
	auto surface = SDL_CreateRGBSurface (0, unitImageSize, unitImageSize, Video.getColDepth (), 0, 0, 0, 0);
	SDL_SetColorKey (surface, SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (surface, NULL, 0x00FF00FF);
	SDL_Rect dest = {0, 0, 0, 0};

	if (unit.data.ID.isAVehicle ())
	{
		const auto& vehicle = static_cast<const cVehicle&>(unit);
		const float zoomFactor = unitImageSize / 64.0f;
		vehicle.render_simple (surface, dest, zoomFactor);
		vehicle.drawOverlayAnimation (surface, dest, zoomFactor, 0);
	}
	else if (unit.data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(unit);
		const float zoomFactor = unitImageSize / (building.data.isBig ? 128.0f : 64.0f);
		building.render_simple (surface, dest, zoomFactor, 0);
	}
	else surface = NULL;

	auto unitDetails = addChild (std::make_unique<cUnitDetailsHud> (cBox<cPosition> (cPosition (unitImageSize+3+75+3, 0), cPosition (unitImageSize+3+75+3 + 155, 48)), true));
	unitDetails->setUnit (&unit, unit.owner);

	unitImage = addChild (std::make_unique<cImage> (cPosition (0, (unitDetails->getSize ().y () - unitImageSize)/2), surface));
	unitImage->setConsumeClick (false);

	auto nameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImage->getEndPosition ().x ()+3, 0), cPosition (unitDetails->getPosition ().x ()-3, unitDetails->getEndPosition ().y ())), unit.getDisplayName (), FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
	nameLabel->setWordWrap (true);

	auto positionLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitDetails->getEndPosition ().x ()+5, 0), cPosition (unitDetails->getEndPosition ().x ()+5+50, unitDetails->getEndPosition ().y ())), iToStr (unit.getPosition ().x ()) + "," + iToStr (unit.getPosition ().y ()), FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::CenterVerical));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (positionLabel->getEndPosition ().x (), 0), cPosition (positionLabel->getEndPosition ().x ()+120, unitDetails->getEndPosition ().y ())), unit.getStatusStr (unit.owner), FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));

	fitToChildren ();
}

void drawSelectionCorner (SDL_Surface* surface, const SDL_Rect& rectangle, Uint16 cornerSize, Uint32 color);

//------------------------------------------------------------------------------
void cReportUnitListViewItem::draw ()
{
	cAbstractListViewItem::draw ();

	if (isSelected ())
	{
		auto dest = unitImage->getArea ();
		dest.getMinCorner () -= cPosition (1, 1);
		dest.getMaxCorner () += cPosition (1, 1);
		drawSelectionCorner (cVideo::buffer, dest.toSdlRect (), 8, 0xFFE0E0E0);
	}
}

//------------------------------------------------------------------------------
cUnit& cReportUnitListViewItem::getUnit () const
{
	return unit;
}