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

#include "ui/graphical/game/widgets/gamemessagelistviewitem.h"

#include "SDLutility/tosdl.h"
#include "ui/widgets/label.h"

//------------------------------------------------------------------------------
cGameMessageListViewItem::cGameMessageListViewItem (const std::string& message, eGameMessageListViewItemBackgroundColor backgroundColor_) :
	cAbstractListViewItem (cPosition (50, 0)),
	backgroundColor (backgroundColor_),
	beginMargin (2, 2),
	endMargin (2, 2)
{
	const cBox<cPosition> labelArea (getPosition() + beginMargin, getPosition() + cPosition (getSize().x() - 1, 50) - endMargin);
	messageLabel = emplaceChild<cLabel> (labelArea, message);
	messageLabel->setWordWrap (true);
	messageLabel->resizeToTextHeight();
	messageLabel->setConsumeClick (false);

	resize (cPosition (getSize().x(), messageLabel->getSize().y() - 1 + beginMargin.y() + endMargin.y()));

	creationTime = std::chrono::steady_clock::now();

	createBackground();
}

//------------------------------------------------------------------------------
std::chrono::steady_clock::time_point cGameMessageListViewItem::getCreationTime() const
{
	return creationTime;
}

//------------------------------------------------------------------------------
void cGameMessageListViewItem::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (cSettings::getInstance().isAlphaEffects())
	{
		auto rect = toSdlRect (getArea());

		if (background != nullptr)
			SDL_BlitSurface (background.get(), nullptr, &destination, &rect);
		else
			Video.applyShadow (&rect, destination);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cGameMessageListViewItem::handleResized (const cPosition& oldSize)
{
	cAbstractListViewItem::handleResized (oldSize);

	if (oldSize.x() == getSize().x()) return;

	const cBox<cPosition> labelArea (getPosition() + beginMargin, getPosition() + cPosition (getSize().x() - 1, 50) - endMargin);
	messageLabel->setArea (labelArea);
	messageLabel->resizeToTextHeight();

	resize (cPosition (getSize().x(), messageLabel->getSize().y() - 1 + beginMargin.y() + endMargin.y()));

	createBackground();
}

//------------------------------------------------------------------------------
void cGameMessageListViewItem::createBackground()
{
	const Uint8 alpha = 50;
	const auto size = getSize();
	background = AutoSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
	switch (backgroundColor)
	{
		default:
		case eGameMessageListViewItemBackgroundColor::DarkGray:
			SDL_FillRect (background.get(), nullptr, toSdlAlphaColor(cRgbColor::black(alpha), *background));
			break;
		case eGameMessageListViewItemBackgroundColor::LightGray:
			SDL_FillRect (background.get(), nullptr, toSdlAlphaColor (cRgbColor::white(alpha), *background));
			break;
		case eGameMessageListViewItemBackgroundColor::Red:
			SDL_FillRect (background.get(), nullptr, toSdlAlphaColor (cRgbColor::red(alpha), *background));
			break;
	}
}
