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

#include "unitlistviewitem.h"

#include "../image.h"
#include "../label.h"
#include "../../../../buildings.h"
#include "../../../../vehicles.h"
#include "../../../../player.h"

//------------------------------------------------------------------------------
cUnitListViewItem::cUnitListViewItem (unsigned int width, const sID& unitId_, const cPlayer& owner) :
	unitId (unitId_)
{
	const int unitImageSize = 32;
	auto surface = SDL_CreateRGBSurface (0, unitImageSize, unitImageSize, Video.getColDepth (), 0, 0, 0, 0);
	SDL_SetColorKey (surface, SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (surface, NULL, 0x00FF00FF);
	SDL_Rect dest = {0, 0, 0, 0};

	// FIXME: very very bad...
	//        why do we need to create a full vehicle/building object to draw it?!
	//        Currently the const_cast is okay here because the player state is not changed during the render
	//        method of the units.
	if (unitId.isAVehicle ())
	{
		cVehicle vehicle (*unitId.getUnitDataOriginalVersion (), const_cast<cPlayer*>(&owner), 0);
		const float zoomFactor = unitImageSize / 64.0f;
		vehicle.render_simple (surface, dest, zoomFactor);
		vehicle.drawOverlayAnimation (surface, dest, zoomFactor, 0);
	}
	else if (unitId.isABuilding ())
	{
		cBuilding building (unitId.getUnitDataOriginalVersion (), const_cast<cPlayer*>(&owner), 0);
		const float zoomFactor = unitImageSize / (building.data.isBig ? 128.0f : 64.0f);
		building.render_simple (surface, dest, zoomFactor, 0);
	}
	else surface = NULL;

	unitImage = addChild (std::make_unique<cImage> (cPosition (0, 0), surface));
	unitImage->setConsumeClick (false);

	nameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (unitImage->getEndPosition ().x ()+3, 0), cPosition (width, unitImage->getEndPosition ().y ())), owner.getUnitDataCurrentVersion (unitId)->name, FONT_LATIN_SMALL_WHITE, toEnumFlag (eAlignmentType::Left) | eAlignmentType::CenterVerical));
	nameLabel->setWordWrap (true);

	auto size = unitImage->getArea ();
	size.add (nameLabel->getArea ());

	resize (size.getSize());
}

// TODO: find a nice place for this function
//------------------------------------------------------------------------------
void drawSelectionCorner (SDL_Surface* surface, const SDL_Rect& rectangle, Uint16 cornerSize, Uint32 color)
{
	SDL_Rect line_h = {rectangle.x, rectangle.y, cornerSize, 1};

	SDL_FillRect (surface, &line_h, color);
	line_h.x += rectangle.w - 1 - cornerSize;
	SDL_FillRect (surface, &line_h, color);
	line_h.x = rectangle.x;
	line_h.y += rectangle.h - 1;
	SDL_FillRect (surface, &line_h, color);
	line_h.x += rectangle.w - 1 - cornerSize;
	SDL_FillRect (surface, &line_h, color);

	SDL_Rect line_v = {rectangle.x, rectangle.y, 1, cornerSize};
	SDL_FillRect (surface, &line_v, color);
	line_v.y += rectangle.h - 1 - cornerSize;
	SDL_FillRect (surface, &line_v, color);
	line_v.x += rectangle.w - 1;
	line_v.y = rectangle.y;
	SDL_FillRect (surface, &line_v, color);
	line_v.y += rectangle.h - 1 - cornerSize;
	SDL_FillRect (surface, &line_v, color);
}

//------------------------------------------------------------------------------
void cUnitListViewItem::draw ()
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
const sID& cUnitListViewItem::getUnitId () const
{
	return unitId;
}
