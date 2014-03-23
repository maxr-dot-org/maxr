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

#include "gamemessagelistviewitem.h"
#include "../../menu/widgets/label.h"
#include "../../../main.h"

//------------------------------------------------------------------------------
cGameMessageListViewItem::cGameMessageListViewItem (int width, const std::string& message, bool alert)
{
	const cPosition beginMargin (2, 2);
	const cPosition endMargin (2, 2);

	const cBox<cPosition> labelArea (getPosition () + beginMargin, getPosition () + cPosition (width-1, 50) - endMargin);
	messageLabel = addChild (std::make_unique<cLabel> (labelArea, message));
	messageLabel->setWordWrap (true);
	messageLabel->resizeToTextHeight ();

	resize (cPosition (width, messageLabel->getSize ().y () + beginMargin.y () + endMargin.y ()));

	if (alert)
	{
		const auto size = getSize ();
		redShadow = SDL_CreateRGBSurface (0, size.x (), size. y(), Video.getColDepth (), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_FillRect (redShadow, nullptr, SDL_MapRGBA (redShadow->format, 0xFF, 0, 0, 50));
	}

	creationTime = std::chrono::system_clock::now ();
}

//------------------------------------------------------------------------------
std::chrono::system_clock::time_point cGameMessageListViewItem::getCreationTime () const
{
	return creationTime;
}

//------------------------------------------------------------------------------
void cGameMessageListViewItem::draw ()
{
	if (cSettings::getInstance ().isAlphaEffects ())
	{
		auto rect = getArea ().toSdlRect ();

		if (redShadow) SDL_BlitSurface (redShadow, nullptr, Video.buffer, &rect);
		else Video.applyShadow (&rect);
	}

	cWidget::draw ();
}