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

#include "combobox.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/special/textlistviewitem.h"
#include "ui/widgets/lineedit.h"
#include "utility/color.h"

//------------------------------------------------------------------------------
cComboBox::cComboBox (const cBox<cPosition>& area) :
	cWidget (area),
	maxVisibleItems (5)
{
	auto font = cUnicodeFont::font.get();

	const cBox<cPosition> listViewArea (cPosition (area.getMinCorner().x(), area.getMaxCorner().y() - 1), cPosition (area.getMaxCorner().x() - 3, area.getMaxCorner().y() + 5));
	listView = emplaceChild<cListView<cTextListViewItem>> (listViewArea, eScrollBarStyle::Modern);
	listView->setBeginMargin (cPosition (2, 2));
	listView->setEndMargin (cPosition (2, 0));
	listView->setItemDistance (0);
	listView->hide();
	listView->disable();

	downButton = emplaceChild<cCheckBox> (getEndPosition(), eCheckBoxType::ArrowDownSmall);
	downButton->move (downButton->getSize() * -1);

	const cBox<cPosition> lineEditArea (getPosition() + cPosition (2, (area.getSize().y() - font->getFontHeight (eUnicodeFontType::LatinNormal)) / 2) + 1, cPosition (getEndPosition().x() - downButton->getSize().x() - 2, getPosition().y() + (area.getSize().y() - font->getFontHeight (eUnicodeFontType::LatinNormal)) / 2 + font->getFontHeight (eUnicodeFontType::LatinNormal)));
	lineEdit = emplaceChild<cLineEdit> (lineEditArea);
	lineEdit->setReadOnly (true);

	signalConnectionManager.connect (lineEdit->clicked, [this, area]() {
		downButton->toggle();
	});

	signalConnectionManager.connect (downButton->toggled, [this, area]() {
		if (downButton->isChecked())
		{
			listView->show();
			listView->enable();
			fitToChildren();
		}
		else
		{
			listView->hide();
			listView->disable();
			resize (area.getSize());
		}
	});

	signalConnectionManager.connect (listView->itemClicked, [this] (cTextListViewItem&) {
		downButton->setChecked (false);
	});

	signalConnectionManager.connect (listView->selectionChanged, [this]() {
		const auto selectedItem = listView->getSelectedItem();
		if (selectedItem)
		{
			lineEdit->setText (selectedItem->getText());
			onItemChanged (selectedItem->getText());
		}
	});

	signalConnectionManager.connect (listView->itemAdded, [this]() { updateListViewSize(); });
	signalConnectionManager.connect (listView->itemRemoved, [this]() { updateListViewSize(); });

	updateListViewSize();
	updateLineEditBackground();
}

//------------------------------------------------------------------------------
void cComboBox::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (!listView->isHidden() && listViewBackground != nullptr)
	{
		SDL_Rect position = toSdlRect (listView->getArea());
		SDL_BlitSurface (listViewBackground.get(), nullptr, &destination, &position);
	}
	if (lineEditBackground != nullptr)
	{
		SDL_Rect position = toSdlRect (getArea());
		position.w = lineEditBackground->w;
		position.h = lineEditBackground->h;
		SDL_BlitSurface (lineEditBackground.get(), nullptr, &destination, &position);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cComboBox::setMaxVisibleItems (size_t count)
{
	maxVisibleItems = count;
	updateListViewSize();
}

//------------------------------------------------------------------------------
void cComboBox::updateListViewSize()
{
	const auto visibleItems = static_cast<int> (std::min (maxVisibleItems, listView->getItemsCount()));

	const auto itemHeight = cUnicodeFont::font->getFontHeight (eUnicodeFontType::LatinNormal) + 1;

	const auto requiredSize = listView->getBeginMargin().y() + listView->getEndMargin().y() + itemHeight * visibleItems + (visibleItems > 0 ? (listView->getItemDistance() * (visibleItems - 1)) : 0) + 1;

	const auto currentSize = listView->getSize();

	if (currentSize.y() != requiredSize)
	{
		listView->resize (cPosition (currentSize.x(), requiredSize));
		updateListViewBackground();
	}
}

//------------------------------------------------------------------------------
void cComboBox::updateLineEditBackground()
{
	lineEditBackground = UniqueSurface (SDL_CreateRGBSurface (0, getSize().x() - downButton->getSize().x(), downButton->getSize().y(), Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (lineEditBackground.get(), nullptr, 0x181818);

	drawRectangle (*lineEditBackground, cBox<cPosition> (cPosition (0, 0), cPosition (lineEditBackground->w, lineEditBackground->h)), cRgbColor::black());
}

//------------------------------------------------------------------------------
void cComboBox::updateListViewBackground()
{
	const auto size = listView->getSize();

	listViewBackground = UniqueSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (listViewBackground.get(), nullptr, 0x181818);

	drawRectangle (*listViewBackground, cBox<cPosition> (cPosition (0, 0), size), cRgbColor::black());
}

//------------------------------------------------------------------------------
void cComboBox::addItem (std::string text)
{
	listView->addItem (std::make_unique<cTextListViewItem> (text));
}

//------------------------------------------------------------------------------
void cComboBox::removeItem (size_t index)
{
	listView->removeItem (listView->getItem (index));
}

//------------------------------------------------------------------------------
size_t cComboBox::getItemsCount() const
{
	return listView->getItemsCount();
}

//------------------------------------------------------------------------------
const std::string& cComboBox::getItem (size_t index) const
{
	return listView->getItem (index).getText();
}

//------------------------------------------------------------------------------
void cComboBox::clearItems()
{
	listView->clearItems();
}

//------------------------------------------------------------------------------
const std::string& cComboBox::getSelectedText() const
{
	return lineEdit->getText();
}

//------------------------------------------------------------------------------
void cComboBox::setSelectedIndex (size_t index)
{
	listView->setSelectedItem (&listView->getItem (index));
}
