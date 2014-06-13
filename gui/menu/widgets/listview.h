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

#ifndef gui_menu_widgets_listviewH
#define gui_menu_widgets_listviewH

#include "../../../maxrconfig.h"
#include "clickablewidget.h"
#include "../../../settings.h"
#include "../../../video.h"
#include "../../../sound.h"
#include "../../../input/mouse/mouse.h"

#include "../../application.h"

class cAbstractListViewItem;

template<typename ItemType>
class cListView : public cClickableWidget
{
	static_assert(std::is_base_of<cAbstractListViewItem, ItemType>::value, "Items in list view have to inherit from cAbstractListViewItem");
public:
	explicit cListView (const cBox<cPosition>& area, bool allowMultiSelection = false, sSOUND* clickSound = SoundData.SNDObjectMenu.get ());

	void disableSelectable ();
	void enableSelectable ();

	void setBeginMargin (const cPosition& margin);
	void setEndMargin (const cPosition& margin);
	void setItemDistance (const cPosition& distance);

	const cPosition& getBeginMargin ();
	const cPosition& getEndMargin ();
	const cPosition& getItemDistance ();

	ItemType* addItem (std::unique_ptr<ItemType> item);

	std::unique_ptr<ItemType> removeItem (ItemType& item);

	// TODO: provide iterator support instead of index access.
	size_t getItemsCount () const;
	ItemType& getItem (size_t index);
	const ItemType& getItem (size_t index) const;

	void clearItems ();

	ItemType* getSelectedItem ();
	const std::vector<ItemType*>& getSelectedItems ();

	void setSelectedItem (const ItemType* item);
	void deselectAll ();

	void scrollToItem (const ItemType* item);

	void scrollDown ();
	void scrollUp ();

	void pageDown ();
	void pageUp ();

	cSignal<void ()> itemRemoved;
	cSignal<void ()> itemAdded;

	cSignal<void ()> selectionChanged;

	cSignal<void (ItemType&)> itemClicked;

	virtual cWidget* getChildAt (const cPosition& position) const MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleGetKeyFocus (cApplication& application) MAXR_OVERRIDE_FUNCTION;
	virtual void handleLooseKeyFocus (cApplication& application) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key) MAXR_OVERRIDE_FUNCTION;

	virtual void handleMoved (const cPosition& offset);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual void handleResized (const cPosition& oldSize) MAXR_OVERRIDE_FUNCTION;
protected:

	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
private:
	cPosition beginMargin;
	cPosition endMargin;
	cPosition itemDistance;

	sSOUND* clickSound;

	bool removeTookPlace;

	std::vector<std::unique_ptr<ItemType>> items;

	std::vector<ItemType*> selectedItems;

	size_t beginDisplayItem;
	size_t endDisplayItem;

	bool selectable;

	bool hasKeyFocus;

	void updateItems ();
};

//------------------------------------------------------------------------------
template<typename ItemType>
cListView<ItemType>::cListView (const cBox<cPosition>& area, bool allowMultiSelection, sSOUND* clickSound_) :
	cClickableWidget (area),
	beginMargin (3, 4),
	endMargin (2, 2),
	itemDistance (0, 3),
	clickSound (clickSound_),
	beginDisplayItem (0),
	endDisplayItem (0),
	selectable (true),
	hasKeyFocus (false)
{
	assert (!allowMultiSelection); // multi selection not yet implemented
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::disableSelectable ()
{
	selectable = false;
	deselectAll ();
	setConsumeClick (false);
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::enableSelectable ()
{
	selectable = true;
	setConsumeClick (true);
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::setBeginMargin (const cPosition& margin)
{
	beginMargin = margin;
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::setEndMargin (const cPosition& margin)
{
	endMargin = margin;
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::setItemDistance (const cPosition& distance)
{
	itemDistance = distance;
}

//------------------------------------------------------------------------------
template<typename ItemType>
const cPosition& cListView<ItemType>::getBeginMargin ()
{
	return beginMargin;
}

//------------------------------------------------------------------------------
template<typename ItemType>
const cPosition& cListView<ItemType>::getEndMargin ()
{
	return endMargin;
}

//------------------------------------------------------------------------------
template<typename ItemType>
const cPosition& cListView<ItemType>::getItemDistance ()
{
	return itemDistance;
}


//------------------------------------------------------------------------------
template<typename ItemType>
ItemType* cListView<ItemType>::addItem (std::unique_ptr<ItemType> item)
{
	if (item == nullptr) return nullptr;

	items.push_back (std::move (item));

	auto& addedItem = *items.back ();

	addedItem.resize (cPosition (getSize ().x () - getBeginMargin ().x () - getEndMargin ().x (), addedItem.getSize().y()));

	addedItem.setParent (this);

	updateItems ();

	itemAdded ();

	return &addedItem;
}

//------------------------------------------------------------------------------
template<typename ItemType>
std::unique_ptr<ItemType> cListView<ItemType>::removeItem (ItemType& item)
{
	auto iter = std::find_if (items.begin (), items.end (), [&](const std::unique_ptr<ItemType>& entry){ return entry.get () == &item; });

	if (iter != items.end ())
	{
		auto removedItem = std::move (*iter);

		const size_t index = iter - items.begin();

		items.erase (iter);
		removeTookPlace = true;

		auto iter2 = std::find (selectedItems.begin (), selectedItems.end (), removedItem.get ());
		if (iter2 != selectedItems.end ())
		{
			removedItem->deselect ();
			selectedItems.erase (iter2);

			if (selectedItems.empty () && !items.empty())
			{
				// TODO: make this functionality optional
				auto itemToSelect = index >= items.size () ? items.back ().get () : items[index].get ();

				selectedItems.push_back (itemToSelect);
				itemToSelect->select ();
			}

			selectionChanged ();
		}

		updateItems ();

		itemRemoved ();

		return std::move (removedItem);
	}
	else return nullptr;
}

//------------------------------------------------------------------------------
template<typename ItemType>
size_t cListView<ItemType>::getItemsCount () const
{
	return items.size ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
ItemType& cListView<ItemType>::getItem (size_t index)
{
	return *items[index];
}

//------------------------------------------------------------------------------
template<typename ItemType>
const ItemType& cListView<ItemType>::getItem (size_t index) const
{
	return *items[index];
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::clearItems ()
{
	const auto hadSelection = !selectedItems.empty ();
	for (size_t j = 0; j != selectedItems.size (); ++j)
	{
		selectedItems[j]->deselect ();
	}
	selectedItems.clear ();

	beginDisplayItem = endDisplayItem = 0;

	items.clear ();

	if (hadSelection) selectionChanged ();

	itemRemoved ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
cWidget* cListView<ItemType>::getChildAt (const cPosition& position) const
{
	for (auto i = beginDisplayItem; i < endDisplayItem; ++i)
	{
		auto& item = *items[i];
		if (item.isAt (position))
		{
			auto itemChild = item.getChildAt (position);
			return itemChild ? itemChild : &item;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
template<typename ItemType>
bool cListView<ItemType>::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
{
	if (amount.y () > 0)
	{
		scrollUp ();
		return true;
	}
	else if (amount.y () < 0)
	{
		scrollDown ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
template<typename ItemType>
bool cListView<ItemType>::handleGetKeyFocus (cApplication& application)
{
	if (!isEnabled()) return false;

	hasKeyFocus = true;

	return true;
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::handleLooseKeyFocus (cApplication& application)
{
	hasKeyFocus = false;
}

//------------------------------------------------------------------------------
template<typename ItemType>
bool cListView<ItemType>::handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	if (!hasKeyFocus) return false;

	switch (key)
	{
	//case SDLK_ESC:
	//	return true;
	case SDLK_UP:
		if (selectable)
		{
			if (selectedItems.size () == 1)
			{
				for (size_t i = 0; i < items.size (); ++i)
				{
					if (selectedItems[0] == items[i].get() && i != 0)
					{
						setSelectedItem (items[i-1].get ());
						scrollToItem (items[i-1].get ());
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
			if (selectedItems.size () == 1)
			{
				for (size_t i = 0; i < items.size (); ++i)
				{
					if (selectedItems[0] == items[i].get () && i != items.size ()-1)
					{
						setSelectedItem (items[i+1].get ());
						scrollToItem (items[i+1].get ());
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
	return false;
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::handleMoved (const cPosition& offset)
{
	for (auto i = items.begin (); i != items.end (); ++i)
	{
		(*i)->move (offset);
	}

	cClickableWidget::handleMoved (offset);
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::draw ()
{
	for (auto i = beginDisplayItem; i < endDisplayItem && i < items.size (); ++i)
	{
		auto& item = *items[i];

		item.draw ();
	}

	cClickableWidget::draw ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::handleResized (const cPosition& oldSize)
{
	for (size_t i = 0; i < items.size (); ++i)
	{
		items[i]->resize (cPosition (getSize ().x () - getBeginMargin ().x () - getEndMargin ().x (), items[i]->getSize ().y ()));
	}
	updateItems ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
bool cListView<ItemType>::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (!selectable) return false;

	if (button == eMouseButtonType::Left)
	{
		for (auto i = beginDisplayItem; i < endDisplayItem; ++i)
		{
			auto& item = *items[i];

			if (item.isAt (mouse.getPosition ()))
			{
				removeTookPlace = false;
				itemClicked (item);
				PlayFX (clickSound);
				if (removeTookPlace) break;

				if (selectedItems.size () != 1 || selectedItems[0] != &item)
				{
					deselectAll ();

					selectedItems.push_back (&item);
					item.select ();

					selectionChanged ();
				}
				break;
			}
		}
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
template<typename ItemType>
ItemType* cListView<ItemType>::getSelectedItem ()
{
	return selectedItems.empty () ? nullptr : selectedItems[0];
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::setSelectedItem (const ItemType* item)
{
	if (!selectable) return;

	if (selectedItems.size () == 1 && selectedItems[0] == item) return;

	if (item == nullptr)
	{
		deselectAll ();
	}
	else
	{
		auto iter = std::find_if (items.begin (), items.end (), [=](const std::unique_ptr<ItemType>& entry){ return entry.get () == item; });

		if (iter != items.end ())
		{
			deselectAll ();

			selectedItems.push_back (iter->get());
			(*iter)->select ();

			selectionChanged ();
		}
	}
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::deselectAll ()
{
	for (size_t j = 0; j != selectedItems.size (); ++j)
	{
		selectedItems[j]->deselect ();
	}
	selectedItems.clear ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::scrollToItem (const ItemType* item)
{
	auto iter = std::find_if (items.begin (), items.end (), [=](const std::unique_ptr<ItemType>& entry){ return entry.get () == item; });

	if (iter != items.end ())
	{
		const size_t index = iter - items.begin ();

		if (index < beginDisplayItem)
		{
			beginDisplayItem = index;
			updateItems ();
		}
		if (index >= endDisplayItem)
		{
			beginDisplayItem += index - (endDisplayItem - 1);
			updateItems ();
		}
	}
}

//------------------------------------------------------------------------------
template<typename ItemType>
const std::vector<ItemType*>& cListView<ItemType>::getSelectedItems ()
{
	return selectedItems;
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::scrollDown ()
{
	if (endDisplayItem >= items.size ()) return;

	++beginDisplayItem;

	updateItems ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::scrollUp ()
{
	if (beginDisplayItem == 0) return;

	--beginDisplayItem;

	updateItems ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::pageDown ()
{
	if (endDisplayItem >= items.size ()) return;

	beginDisplayItem = endDisplayItem;

	updateItems ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::pageUp ()
{
	if (beginDisplayItem == 0) return;

	const auto maxScroll = getSize ().y () - beginMargin.y () - endMargin.y ();
	int scrolledHeight = 0;
	while (scrolledHeight < maxScroll)
	{
		scrolledHeight += items[beginDisplayItem-1]->getSize ().y () + itemDistance.y();
		if (scrolledHeight < maxScroll)
		{
			--beginDisplayItem;
			if (beginDisplayItem == 0) break;
		}
	}

	updateItems ();
}

//------------------------------------------------------------------------------
template<typename ItemType>
void cListView<ItemType>::updateItems ()
{
	// TODO: make this more efficient.
	//       We do not need to iterate over all items every time.
	//       In most cases only a single element has been changed
	//       (e.g. one has been added or we moved one down).


	cPosition currentItemPosition = getPosition () + beginMargin;
	endDisplayItem = items.size ();
	for (size_t i = 0; i < items.size (); ++i)
	{
		auto& item = *items[i];
		if (i < beginDisplayItem || i >= endDisplayItem)
		{
			item.disable ();
			item.hide ();
			continue;
		}
		const cPosition itemEndPosition = currentItemPosition + item.getSize ();
		if (itemEndPosition.y () > getEndPosition ().y () - endMargin.y ())
		{
			endDisplayItem = i;
			item.disable ();
			item.hide ();
			continue;
		}

		item.enable ();
		item.show ();
		item.moveTo (currentItemPosition);

		currentItemPosition.y () = itemEndPosition.y () + itemDistance.y ();
	}
}

#endif // gui_menu_widgets_listviewH
