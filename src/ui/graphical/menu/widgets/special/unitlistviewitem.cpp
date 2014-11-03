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

#include "ui/graphical/menu/widgets/special/unitlistviewitem.h"

#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/data/player/player.h"
#include "utility/drawing.h"

//------------------------------------------------------------------------------
cUnitListViewItem::cUnitListViewItem (unsigned int width, const sID& unitId_, const cPlayer& owner) :
	unitId (unitId_)
{
	const int unitImageSize = 32;
	AutoSurface surface (SDL_CreateRGBSurface (0, unitImageSize, unitImageSize, Video.getColDepth (), 0, 0, 0, 0));
	SDL_SetColorKey (surface.get (), SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (surface.get (), nullptr, 0x00FF00FF);
	SDL_Rect dest = {0, 0, 0, 0};

	const auto& data = *unitId.getUnitDataOriginalVersion ();
	if (unitId.isAVehicle ())
	{
		const float zoomFactor = unitImageSize / 64.0f;
		const auto& uiData = *UnitsData.getVehicleUI (unitId);
		cVehicle::render_simple (surface.get (), dest, zoomFactor, data, uiData, &owner);
		cVehicle::drawOverlayAnimation (surface.get (), dest, zoomFactor, data, uiData);
	}
	else if (unitId.isABuilding ())
	{
		const float zoomFactor = unitImageSize / (data.isBig ? 128.0f : 64.0f);
		const auto& uiData = *UnitsData.getBuildingUI (unitId);
		cBuilding::render_simple (surface.get (), dest, zoomFactor, data, uiData, &owner);
	}
	else surface = nullptr;

	unitImage = addChild (std::make_unique<cImage> (cPosition (0, 0), surface.get ()));
	unitImage->setConsumeClick (false);

	nameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImage->getEndPosition ().x ()+3, 0), cPosition (width, unitImage->getEndPosition ().y ())), owner.getUnitDataCurrentVersion (unitId)->name, FONT_LATIN_SMALL_WHITE, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
	nameLabel->setWordWrap (true);

	auto size = unitImage->getArea ();
	size.add (nameLabel->getArea ());

	resize (size.getSize());
}

//------------------------------------------------------------------------------
void cUnitListViewItem::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	cAbstractListViewItem::draw (destination, clipRect);

	if (isSelected ())
	{
		auto dest = unitImage->getArea ();
		dest.getMinCorner () -= cPosition (1, 1);
		dest.getMaxCorner () += cPosition (1, 1);
		drawSelectionCorner (destination, dest, cRgbColor (224, 224, 224), 8, cBox<cPosition> (clipRect.getMinCorner () - 1, clipRect.getMaxCorner () + 1));
	}
}

//------------------------------------------------------------------------------
const sID& cUnitListViewItem::getUnitId () const
{
	return unitId;
}
