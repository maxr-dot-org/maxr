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

#include "ui/graphical/menu/widgets/special/textlistviewitem.h"

#include "ui/graphical/menu/widgets/label.h"

//------------------------------------------------------------------------------
cTextListViewItem::cTextListViewItem (const std::string& text) :
	cAbstractListViewItem (cPosition (50, font->getFontHeight (FONT_LATIN_NORMAL)))
{
	label = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (0, 0), cPosition (getSize().x() - 1, font->getFontHeight (FONT_LATIN_NORMAL))), text));
}

//------------------------------------------------------------------------------
const std::string& cTextListViewItem::getText() const
{
	return label->getText();
}

//------------------------------------------------------------------------------
void cTextListViewItem::handleResized (const cPosition& oldSize)
{
	cAbstractListViewItem::handleResized (oldSize);

	label->resize (getSize());
}
