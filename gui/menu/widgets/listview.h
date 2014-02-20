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
#include "../../../input/mouse/mouse.h"

#include "../../application.h"

class cAbstractListViewItem;

template<typename ItemType>
class cListView : public cClickableWidget
{
	static_assert(std::is_base_of<cAbstractListViewItem, ItemType>::value, "Items in list view have to inherit from cAbstractListViewItem");
public:
	explicit cListView (const cBox<cPosition>& area, bool allowMultiSelection = false);

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

	void scroolToItem (const ItemType* item);

	void scrollDown ();
	void scrollUp ();

	void pageDown ();
	void pageUp ();

	cSignal<void ()> selectionChanged;

	cSignal<void (ItemType&)> itemClicked;

	virtual bool handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount) MAXR_OVERRIDE_FUNCTION;

	virtual void handleMoved (const cPosition& offset);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

protected:

	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
private:
	const cPosition beginMargin;
	const cPosition endMargin;
	const cPosition itemDistance;

	bool removeTookPlace;

	std::vector<std::unique_ptr<ItemType>> items;

	std::vector<ItemType*> selectedItems;

	size_t beginDisplayItem;
	size_t endDisplayItem;

	void updateItems ();
};

//------------------------------------------------------------------------------
template<typename ItemType>
cListView<ItemType>::cListView (const cBox<cPosition>& area, bool allowMultiSelection) :
	cClickableWidget (area),
	beginDisplayItem (0),
	endDisplayItem (0),
	beginMargin (3, 4),
	endMargin (2, 2),
	itemDistance (0, 3)
{
	assert (!allowMultiSelection); // multi selection not yet implemented
}

//------------------------------------------------------------------------------
template<typename ItemType>
ItemType* cListView<ItemType>::addItem (std::unique_ptr<ItemType> item)
{
	items.push_back (std::move (item));

	updateItems ();

	return items.back ().get();
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
bool cListView<ItemType>::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		for (auto i = beginDisplayItem; i < endDisplayItem; ++i)
		{
			auto& item = *items[i];

			if (item.isAt (mouse.getPosition ()))
			{
				removeTookPlace = false;
				itemClicked (item);
				if (removeTookPlace) break;

				if (selectedItems.size () != 1 || selectedItems[0] != &item)
				{
					deselectAll ();

					selectedItems.push_back (&item);
					item.select ();

					selectionChanged ();
				}
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
void cListView<ItemType>::scroolToItem (const ItemType* item)
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
		const cPosition itemEndPosition = currentItemPosition + itemDistance + item.getSize ();
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

		currentItemPosition.y () = itemEndPosition.y ();
	}
}

#endif // gui_menu_widgets_listviewH
