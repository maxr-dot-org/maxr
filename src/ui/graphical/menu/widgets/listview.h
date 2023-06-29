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

#ifndef ui_graphical_menu_widgets_listviewH
#define ui_graphical_menu_widgets_listviewH

#include "input/mouse/mouse.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/sound.h"
#include "ui/graphical/menu/widgets/scrollbar.h"
#include "ui/widgets/application.h"
#include "ui/widgets/clickablewidget.h"
#include "utility/ranges.h"
#include "utility/signal/signalconnectionmanager.h"

#include <cassert>

class cAbstractListViewItem;

enum class eAddListItemScrollType
{
	None,
	Always,
	IfAtBottom
};

template <typename ItemType>
class cListView : public cClickableWidget
{
	static_assert (std::is_base_of<cAbstractListViewItem, ItemType>::value, "Items in list view have to inherit from cAbstractListViewItem");

public:
	explicit cListView (const cBox<cPosition>& area, bool allowMultiSelection = false, cSoundChunk* clickSound = &SoundData.SNDObjectMenu);
	explicit cListView (const cBox<cPosition>& area, eScrollBarStyle, bool allowMultiSelection = false, cSoundChunk* clickSound = &SoundData.SNDObjectMenu);

	void disableSelectable();
	void enableSelectable();

	void setBeginMargin (const cPosition& margin);
	void setEndMargin (const cPosition& margin);
	void setItemDistance (int distance);

	void setScrollOffset (int offset);

	const cPosition& getBeginMargin() const;
	const cPosition& getEndMargin() const;
	int getItemDistance() const;

	ItemType* addItem (std::unique_ptr<ItemType>, eAddListItemScrollType = eAddListItemScrollType::None);

	std::unique_ptr<ItemType> removeItem (ItemType&);

	// TODO: provide iterator support instead of index access.
	size_t getItemsCount() const;
	ItemType& getItem (size_t index);
	const ItemType& getItem (size_t index) const;

	void clearItems();

	ItemType* getSelectedItem();
	const std::vector<ItemType*>& getSelectedItems();

	void setSelectedItem (const ItemType*);
	void deselectAll();

	void scrollToItem (const ItemType*);

	void scrollDown();
	void scrollUp();

	void pageDown();
	void pageUp();

	bool isAtMinScroll() const;
	bool isAtMaxScroll() const;

	cSignal<void()> itemRemoved;
	cSignal<void()> itemAdded;

	cSignal<void()> selectionChanged;

	cSignal<void (ItemType&)> itemClicked;

	cWidget* getChildAt (const cPosition&) const override;
	bool handleMouseWheelMoved (cApplication&, cMouse&, const cPosition& amount) override;
	bool handleGetKeyFocus (cApplication&) override;
	void handleLooseKeyFocus (cApplication&) override;
	bool handleKeyPressed (cApplication&, cKeyboard&, SDL_Keycode) override;
	void handleMoved (const cPosition& offset) override;
	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	void handleResized (const cPosition& oldSize) override;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	cSignalConnectionManager signalConnectionManager;

	cScrollBar* scrollBar;

	cPosition beginMargin;
	cPosition endMargin;
	int itemDistance;

	cSoundChunk* clickSound;

	bool removeTookPlace;

	std::vector<std::pair<int, std::unique_ptr<ItemType>>> items;

	std::vector<ItemType*> selectedItems;

	int pixelOffset;
	int pixelScrollOffset;
	bool pixelScrollOffsetInitialized;

	size_t beginDisplayItem;
	size_t endDisplayItem;

	bool selectable;

	bool hasKeyFocus;

	void updateDisplayItems();
};

//------------------------------------------------------------------------------
template <typename ItemType>
cListView<ItemType>::cListView (const cBox<cPosition>& area, bool allowMultiSelection, cSoundChunk* clickSound_) :
	cClickableWidget (area),
	scrollBar (nullptr),
	beginMargin (3, 4),
	endMargin (2, 2),
	itemDistance (3),
	clickSound (clickSound_),
	pixelOffset (0),
	pixelScrollOffset (0),
	pixelScrollOffsetInitialized (false),
	beginDisplayItem (0),
	endDisplayItem (0),
	selectable (true),
	hasKeyFocus (false)
{
	assert (!allowMultiSelection); // multi selection not yet implemented
}

//------------------------------------------------------------------------------
template <typename ItemType>
cListView<ItemType>::cListView (const cBox<cPosition>& area, eScrollBarStyle scrollBarStyle, bool allowMultiSelection, cSoundChunk* clickSound_) :
	cListView<ItemType> (area, allowMultiSelection, clickSound_)
{
	scrollBar = emplaceChild<cScrollBar> (getPosition(), getSize().y(), scrollBarStyle, eOrientationType::Vertical);
	scrollBar->move (cPosition (getSize().x() - scrollBar->getSize().x() + 1, 0));

	signalConnectionManager.connect (scrollBar->forwardClicked, [this]() { scrollDown(); });
	signalConnectionManager.connect (scrollBar->backClicked, [this]() { scrollUp(); });
	signalConnectionManager.connect (scrollBar->offsetChanged, [this]() {
		if (this->scrollBar->getOffset() == this->pixelOffset) return;

		this->pixelOffset = std::max (this->scrollBar->getOffset(), 0);

		this->updateDisplayItems();
	});
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::disableSelectable()
{
	selectable = false;
	deselectAll();
	setConsumeClick (false);
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::enableSelectable()
{
	selectable = true;
	setConsumeClick (true);
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::setBeginMargin (const cPosition& margin)
{
	beginMargin = margin;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::setEndMargin (const cPosition& margin)
{
	endMargin = margin;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::setItemDistance (int distance)
{
	itemDistance = distance;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::setScrollOffset (int offset)
{
	pixelScrollOffset = offset;
	pixelScrollOffsetInitialized = true;
}

//------------------------------------------------------------------------------
template <typename ItemType>
const cPosition& cListView<ItemType>::getBeginMargin() const
{
	return beginMargin;
}

//------------------------------------------------------------------------------
template <typename ItemType>
const cPosition& cListView<ItemType>::getEndMargin() const
{
	return endMargin;
}

//------------------------------------------------------------------------------
template <typename ItemType>
int cListView<ItemType>::getItemDistance() const
{
	return itemDistance;
}

//------------------------------------------------------------------------------
template <typename ItemType>
ItemType* cListView<ItemType>::addItem (std::unique_ptr<ItemType> item, eAddListItemScrollType scrollType)
{
	if (item == nullptr) return nullptr;

	const auto wasAtMaxScroll = isAtMaxScroll();

	auto newItemPos = items.empty() ? 0 : items.back().first + items.back().second->getSize().y() + getItemDistance();
	items.emplace_back (newItemPos, std::move (item));

	auto& addedItem = *items.back().second;

	if (!pixelScrollOffsetInitialized)
	{
		pixelScrollOffset = addedItem.getSize().y() + getItemDistance();
		pixelScrollOffsetInitialized = true;
	}

	addedItem.resize (cPosition (getSize().x() - getBeginMargin().x() - getEndMargin().x() - (scrollBar ? scrollBar->getSize().x() : 0), addedItem.getSize().y()));

	addedItem.setParent (this);

	const auto itemPtr = &addedItem;
	signalConnectionManager.connect (addedItem.resized, [this, itemPtr] (const cPosition& oldSize) {
		const auto offset = itemPtr->getSize().y() - oldSize.y();
		if (offset == 0) return;

		auto iter = ranges::find_if (items, [=] (const std::pair<int, std::unique_ptr<ItemType>>& entry) { return entry.second.get() == itemPtr; });
		if (iter != items.end()) ++iter;
		for (; iter != items.end(); ++iter)
		{
			iter->first += offset;
		}
		if (scrollBar)
		{
			const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();
			scrollBar->setRange (std::max (items.back().first + items.back().second->getSize().y() - pixelSize, 0));
		}
	});

	if (scrollBar)
	{
		const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();
		scrollBar->setRange (std::max (items.back().first + items.back().second->getSize().y() - pixelSize, 0));
	}

	if (scrollType == eAddListItemScrollType::Always || (scrollType == eAddListItemScrollType::IfAtBottom && wasAtMaxScroll))
	{
		scrollToItem (&addedItem);
	}

	updateDisplayItems();

	itemAdded();

	return &addedItem;
}

//------------------------------------------------------------------------------
template <typename ItemType>
std::unique_ptr<ItemType> cListView<ItemType>::removeItem (ItemType& item)
{
	auto iter = ranges::find_if (items, [&] (const std::pair<int, std::unique_ptr<ItemType>>& entry) { return entry.second.get() == &item; });

	if (iter != items.end())
	{
		auto removedItem = std::move (iter->second);

		const size_t index = iter - items.begin();

		iter = items.erase (iter);

		const auto offset = removedItem->getSize().y() + getItemDistance();
		for (; iter != items.end(); ++iter)
		{
			iter->first -= offset;
		}

		if (scrollBar)
		{
			if (!items.empty())
			{
				const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();
				scrollBar->setRange (std::max (items.back().first + items.back().second->getSize().y() - pixelSize, 0));
			}
			else
			{
				scrollBar->setRange (0);
			}
		}

		removeTookPlace = true;

		auto iter2 = ranges::find (selectedItems, removedItem.get());
		if (iter2 != selectedItems.end())
		{
			removedItem->deselect();
			selectedItems.erase (iter2);

			if (selectedItems.empty() && !items.empty())
			{
				// TODO: make this functionality optional
				auto itemToSelect = index >= items.size() ? items.back().second.get() : items[index].second.get();

				selectedItems.push_back (itemToSelect);
				itemToSelect->select();
			}

			selectionChanged();
		}

		updateDisplayItems();

		itemRemoved();

		return removedItem;
	}
	else
		return nullptr;
}

//------------------------------------------------------------------------------
template <typename ItemType>
size_t cListView<ItemType>::getItemsCount() const
{
	return items.size();
}

//------------------------------------------------------------------------------
template <typename ItemType>
ItemType& cListView<ItemType>::getItem (size_t index)
{
	return *items[index].second;
}

//------------------------------------------------------------------------------
template <typename ItemType>
const ItemType& cListView<ItemType>::getItem (size_t index) const
{
	return *items[index].second;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::clearItems()
{
	const auto hadSelection = !selectedItems.empty();
	for (auto& selectedItem : selectedItems)
	{
		selectedItem->deselect();
	}
	selectedItems.clear();

	beginDisplayItem = endDisplayItem = 0;
	pixelOffset = 0;

	items.clear();

	if (hadSelection) selectionChanged();

	if (scrollBar) scrollBar->setRange (0);

	itemRemoved();
}

//------------------------------------------------------------------------------
template <typename ItemType>
cWidget* cListView<ItemType>::getChildAt (const cPosition& position) const
{
	for (auto i = beginDisplayItem; i < endDisplayItem; ++i)
	{
		auto& item = *items[i].second;
		if (item.isAt (position))
		{
			auto itemChild = item.getChildAt (position);
			return itemChild ? itemChild : &item;
		}
	}
	return cClickableWidget::getChildAt (position);
}

//------------------------------------------------------------------------------
template <typename ItemType>
bool cListView<ItemType>::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
{
	if (amount.y() > 0)
	{
		scrollUp();
		return true;
	}
	else if (amount.y() < 0)
	{
		scrollDown();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
template <typename ItemType>
bool cListView<ItemType>::handleGetKeyFocus (cApplication& application)
{
	if (!isEnabled()) return false;

	hasKeyFocus = true;

	return true;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::handleLooseKeyFocus (cApplication& application)
{
	hasKeyFocus = false;
}

//------------------------------------------------------------------------------
template <typename ItemType>
bool cListView<ItemType>::handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	if (!hasKeyFocus) return false;

	switch (key)
	{
		case SDLK_UP:
			if (selectable)
			{
				if (selectedItems.size() == 1)
				{
					for (size_t i = 0; i < items.size(); ++i)
					{
						if (selectedItems[0] == items[i].second.get() && i != 0)
						{
							setSelectedItem (items[i - 1].second.get());
							scrollToItem (items[i - 1].second.get());
							break;
						}
					}
				}
				return true;
			}
			return false;
		case SDLK_DOWN:
			if (selectable)
			{
				if (selectedItems.size() == 1)
				{
					for (size_t i = 0; i < items.size(); ++i)
					{
						if (selectedItems[0] == items[i].second.get() && i != items.size() - 1)
						{
							setSelectedItem (items[i + 1].second.get());
							scrollToItem (items[i + 1].second.get());
							break;
						}
					}
				}
				return true;
			}
			return false;
		default:
			return false;
	}
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::handleMoved (const cPosition& offset)
{
	for (auto& p : items)
	{
		p.second->move (offset);
	}

	cClickableWidget::handleMoved (offset);
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	cBox<cPosition> itemBox;
	for (auto i = beginDisplayItem; i < endDisplayItem && i < items.size(); ++i)
	{
		auto& item = *items[i].second;

		itemBox = item.getArea();
		if (i == beginDisplayItem)
		{
			itemBox.getMinCorner().y() = std::max (itemBox.getMinCorner().y(), getPosition().y() + getBeginMargin().y());
		}
		if (i == endDisplayItem - 1)
		{
			itemBox.getMaxCorner().y() = std::min (itemBox.getMaxCorner().y(), getEndPosition().y() - getEndMargin().y());
		}

		item.draw (destination, itemBox);
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::handleResized (const cPosition& oldSize)
{
	if (scrollBar)
	{
		scrollBar->resize (cPosition (scrollBar->getSize().x(), getSize().y()));
		scrollBar->moveTo (getPosition() + cPosition (getSize().x() - scrollBar->getSize().x() + 1, 0));
	}

	for (auto& item : items)
	{
		item.second->resize (cPosition (getSize().x() - getBeginMargin().x() - getEndMargin().x() - (scrollBar ? scrollBar->getSize().x() : 0), item.second->getSize().y()));
	}

	if (scrollBar && !items.empty())
	{
		const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();
		scrollBar->setRange (std::max (items.back().first + items.back().second->getSize().y() - pixelSize, 0));
	}

	updateDisplayItems();

	cClickableWidget::handleResized (oldSize);
}

//------------------------------------------------------------------------------
template <typename ItemType>
bool cListView<ItemType>::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (!selectable) return false;

	if (button == eMouseButtonType::Left)
	{
		for (auto i = beginDisplayItem; i < endDisplayItem; ++i)
		{
			auto& item = *items[i].second;

			if (item.isAt (mouse.getPosition()))
			{
				removeTookPlace = false;
				itemClicked (item);
				if (clickSound) cSoundDevice::getInstance().playSoundEffect (*clickSound);
				if (removeTookPlace) break;

				if (selectedItems.size() != 1 || selectedItems[0] != &item)
				{
					deselectAll();

					selectedItems.push_back (&item);
					item.select();

					selectionChanged();
				}
				break;
			}
		}
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
template <typename ItemType>
ItemType* cListView<ItemType>::getSelectedItem()
{
	return selectedItems.empty() ? nullptr : selectedItems[0];
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::setSelectedItem (const ItemType* item)
{
	if (!selectable) return;

	if (selectedItems.size() == 1 && selectedItems[0] == item) return;

	if (item == nullptr)
	{
		deselectAll();
	}
	else
	{
		auto iter = ranges::find_if (items, [=] (const std::pair<int, std::unique_ptr<ItemType>>& entry) { return entry.second.get() == item; });

		if (iter != items.end())
		{
			deselectAll();

			selectedItems.push_back (iter->second.get());
			iter->second->select();

			selectionChanged();
		}
	}
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::deselectAll()
{
	for (auto& selectedItem : selectedItems)
	{
		selectedItem->deselect();
	}
	selectedItems.clear();
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::scrollToItem (const ItemType* item)
{
	auto iter = ranges::find_if (items, [=] (const std::pair<int, std::unique_ptr<ItemType>>& entry) { return entry.second.get() == item; });

	if (iter != items.end())
	{
		if (iter->first < pixelOffset)
		{
			pixelOffset = iter->first;

			updateDisplayItems();
		}
		else if (iter->first + iter->second->getSize().y() > pixelOffset + getSize().y() - getBeginMargin().y() - getEndMargin().y())
		{
			const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();
			pixelOffset = std::max (iter->first + iter->second->getSize().y() - pixelSize, 0);

			updateDisplayItems();
		}
	}
}

//------------------------------------------------------------------------------
template <typename ItemType>
const std::vector<ItemType*>& cListView<ItemType>::getSelectedItems()
{
	return selectedItems;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::scrollDown()
{
	//if (endDisplayItem >= items.size()) return;

	pixelOffset += pixelScrollOffset;

	updateDisplayItems();
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::scrollUp()
{
	if (pixelOffset <= 0) return;

	pixelOffset = std::max (pixelOffset - pixelScrollOffset, 0);

	updateDisplayItems();
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::pageDown()
{
	//if (endDisplayItem >= items.size()) return;

	const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();

	pixelOffset += pixelSize;

	updateDisplayItems();
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::pageUp()
{
	if (pixelOffset <= 0) return;

	const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();

	pixelOffset = std::max (pixelOffset - pixelSize, 0);

	updateDisplayItems();
}

//------------------------------------------------------------------------------
template <typename ItemType>
bool cListView<ItemType>::isAtMinScroll() const
{
	return pixelOffset <= 0;
}

//------------------------------------------------------------------------------
template <typename ItemType>
bool cListView<ItemType>::isAtMaxScroll() const
{
	if (items.empty()) return true;

	const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();

	return pixelOffset >= items.back().first + items.back().second->getSize().y() - pixelSize;
}

//------------------------------------------------------------------------------
template <typename ItemType>
void cListView<ItemType>::updateDisplayItems()
{
	if (items.empty())
	{
		beginDisplayItem = endDisplayItem = 0;
		pixelOffset = 0;
		return;
	}

	const auto pixelSize = getSize().y() - getBeginMargin().y() - getEndMargin().y();

	const auto endPixelOffset = pixelOffset + pixelSize;

	auto iter = std::lower_bound (items.begin(), items.end(), endPixelOffset, [] (const std::pair<int, std::unique_ptr<ItemType>>& entry, int offset) { return entry.first + entry.second->getSize().y() < offset; });
	if (iter == items.end())
	{
		endDisplayItem = items.size();
		pixelOffset = std::max (items[items.size() - 1].first + items[items.size() - 1].second->getSize().y() - pixelSize, 0);
	}
	else
	{
		endDisplayItem = iter - items.begin() + 1;
	}

	iter = std::upper_bound (items.begin(), items.end(), pixelOffset, [] (int offset, const std::pair<int, std::unique_ptr<ItemType>>& entry) { return offset < entry.first; });
	if (iter == items.end())
	{
		beginDisplayItem = items.size() - 1;
	}
	else
	{
		assert (iter != items.begin());
		--iter;
		beginDisplayItem = iter - items.begin();
	}

	for (size_t i = 0; i < items.size(); ++i)
	{
		if (i < beginDisplayItem || i >= endDisplayItem)
		{
			items[i].second->hide();
			items[i].second->disable();
		}
		else
		{
			items[i].second->show();
			items[i].second->enable();

			items[i].second->moveTo (getPosition() + getBeginMargin() + cPosition (0, items[i].first - pixelOffset));
		}
	}

	if (scrollBar)
	{
		scrollBar->setOffset (pixelOffset);
	}
}

#endif // ui_graphical_menu_widgets_listviewH
