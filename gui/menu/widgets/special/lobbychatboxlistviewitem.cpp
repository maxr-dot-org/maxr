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

#include "lobbychatboxlistviewitem.h"
#include "../label.h"

//------------------------------------------------------------------------------
cLobbyChatBoxListViewItem::cLobbyChatBoxListViewItem (const std::string& text, int width)
{
	auto messageLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition (), getPosition () + cPosition (width, 0)), text));
	messageLabel->setWordWrap (true);
	messageLabel->resizeToTextHeight ();

	fitToChildren ();
}

//------------------------------------------------------------------------------
cLobbyChatBoxListViewItem::cLobbyChatBoxListViewItem (const std::string& playerName, const std::string& text, int width)
{
	const auto playerNameText = playerName + ": ";
	const auto playerNameTextWidth = font->getTextWide (playerNameText);
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition (), getPosition () + cPosition (playerNameTextWidth, 10)), playerNameText));

	auto messageLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (playerNameTextWidth, 0), getPosition () + cPosition (width, 0)), text));
	messageLabel->setWordWrap (true);
	messageLabel->resizeToTextHeight ();

	fitToChildren ();
}