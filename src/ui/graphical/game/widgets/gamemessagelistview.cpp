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

#include "ui/graphical/game/widgets/gamemessagelistview.h"

#include "ui/graphical/menu/widgets/listview.h"

//------------------------------------------------------------------------------
cGameMessageListView::cGameMessageListView (const cBox<cPosition>& area) :
	cWidget (area),
	maximalDisplayTime (15)
{
	listView = emplaceChild<cListView<cGameMessageListViewItem>> (area, false, nullptr);
	listView->setBeginMargin (cPosition (0, 0));
	listView->setEndMargin (cPosition (0, 0));
	listView->setItemDistance (0);
}

//------------------------------------------------------------------------------
void cGameMessageListView::addMessage (const std::string& message, eGameMessageListViewItemBackgroundColor backgroundColor)
{
	listView->addItem (std::make_unique<cGameMessageListViewItem> (message, backgroundColor), eAddListItemScrollType::Always);
}

//------------------------------------------------------------------------------
void cGameMessageListView::removeOldMessages()
{
	while (listView->getItemsCount() > 0 && (std::chrono::steady_clock::now() - listView->getItem (0).getCreationTime()) >= maximalDisplayTime)
	{
		listView->removeItem (listView->getItem (0));
	}
}

//------------------------------------------------------------------------------
void cGameMessageListView::clear()
{
	listView->clearItems();
}

//------------------------------------------------------------------------------
bool cGameMessageListView::isAt (const cPosition&) const /* override */
{
	return false; // fully transparent. Do not take any input events
}

//------------------------------------------------------------------------------
void cGameMessageListView::handleResized (const cPosition& oldSize)
{
	cWidget::handleResized (oldSize);

	listView->setArea (getArea());
}
