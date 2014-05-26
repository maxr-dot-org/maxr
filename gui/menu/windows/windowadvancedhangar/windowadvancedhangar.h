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

#ifndef gui_menu_windows_windowadvancedhangar_windowadvancedhangarH
#define gui_menu_windows_windowadvancedhangar_windowadvancedhangarH

#include "../windowhangar/windowhangar.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/special/unitlistviewitembuy.h"

template<typename SelectedUnitItemType>
class cWindowAdvancedHangar : public cWindowHangar
{
public:
	cWindowAdvancedHangar (SDL_Surface* surface, int playerColor, int playerClan);
	cWindowAdvancedHangar (SDL_Surface* surface, const cPlayer& player);
	~cWindowAdvancedHangar ();

protected:
	SelectedUnitItemType& addSelectedUnit (const sID& unitId);

	size_t getSelectedUnitsCount () const;
	SelectedUnitItemType& getSelectedUnit (size_t index);
	const SelectedUnitItemType& getSelectedUnit (size_t index) const;

	virtual bool tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const;
	virtual bool tryRemoveSelectedUnit (const SelectedUnitItemType& unitItem) const;

	// TODO: the following widgets should be private instead.
	// They are protect at the moment because some inheriting windows need to move/resize the widgets.
	cListView<SelectedUnitItemType>* selectedUnitList;
	cPushButton* selectedListUpButton;
	cPushButton* selectedListDownButton;

	cSignal<void (SelectedUnitItemType*)> selectedUnitSelectionChanged;
private:
	cSignalConnectionManager signalConnectionManager;

	void initialize ();

	void handleSelectionChanged ();

	void handleSelectionUnitClickedSecondTime (const cUnitListViewItemBuy& item);

	void selectedUnitClicked (SelectedUnitItemType& unitItem);
};

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
cWindowAdvancedHangar<SelectedUnitItemType>::cWindowAdvancedHangar (SDL_Surface* surface, int playerColor, int playerClan) :
	cWindowHangar (surface, playerColor, playerClan)
{
	initialize ();
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
cWindowAdvancedHangar<SelectedUnitItemType>::cWindowAdvancedHangar (SDL_Surface* surface, const cPlayer& player) :
	cWindowHangar (surface, player)
{
	initialize ();
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::initialize ()
{
	using namespace std::placeholders;

	selectedUnitList = addChild (std::make_unique<cListView<SelectedUnitItemType>> (cBox<cPosition> (getPosition () + cPosition (330, 14), getPosition () + cPosition (330 + 130, 12 + 220))));
	signalConnectionManager.connect (selectedUnitList->itemClicked, std::bind (&cWindowAdvancedHangar<SelectedUnitItemType>::selectedUnitClicked, this, _1));
	signalConnectionManager.connect (selectedUnitList->selectionChanged, std::bind (&cWindowAdvancedHangar<SelectedUnitItemType>::handleSelectionChanged, this));

	selectedListUpButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (327, 240), ePushButtonType::ArrowUpSmall, SoundData.SNDObjectMenu.get ()));
	signalConnectionManager.connect (selectedListUpButton->clicked, std::bind (&cListView<SelectedUnitItemType>::pageUp, selectedUnitList));

	selectedListDownButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (348, 240), ePushButtonType::ArrowDownSmall, SoundData.SNDObjectMenu.get ()));
	signalConnectionManager.connect (selectedListDownButton->clicked, std::bind (&cListView<SelectedUnitItemType>::pageDown, selectedUnitList));

	using namespace std::placeholders;

	signalConnectionManager.connect (selectionUnitClickedSecondTime, std::bind (&cWindowAdvancedHangar<SelectedUnitItemType>::handleSelectionUnitClickedSecondTime, this, _1));
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
cWindowAdvancedHangar<SelectedUnitItemType>::~cWindowAdvancedHangar ()
{}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
bool cWindowAdvancedHangar<SelectedUnitItemType>::tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const
{
	return true;
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
bool cWindowAdvancedHangar<SelectedUnitItemType>::tryRemoveSelectedUnit (const SelectedUnitItemType& unitItem) const
{
	return true;
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
SelectedUnitItemType& cWindowAdvancedHangar<SelectedUnitItemType>::addSelectedUnit (const sID& unitId)
{
	auto addedItem = selectedUnitList->addItem (std::make_unique<SelectedUnitItemType> (selectedUnitList->getSize ().x () - 9, unitId, getPlayer ()));
	return *addedItem;
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
size_t cWindowAdvancedHangar<SelectedUnitItemType>::getSelectedUnitsCount () const
{
	return selectedUnitList->getItemsCount ();
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
SelectedUnitItemType& cWindowAdvancedHangar<SelectedUnitItemType>::getSelectedUnit (size_t index)
{
	return selectedUnitList->getItem (index);
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
const SelectedUnitItemType& cWindowAdvancedHangar<SelectedUnitItemType>::getSelectedUnit (size_t index) const
{
	return selectedUnitList->getItem (index);
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::handleSelectionChanged ()
{
	auto selectedItem = selectedUnitList->getSelectedItem ();
	if (selectedItem != nullptr)
	{
		setActiveUnit (selectedItem->getUnitId ());
	}
	selectedUnitSelectionChanged (selectedItem);
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::handleSelectionUnitClickedSecondTime (const cUnitListViewItemBuy& unitItem)
{
	if (tryAddSelectedUnit (unitItem))
	{
		auto& addedItem = addSelectedUnit (unitItem.getUnitId ());
		selectedUnitList->setSelectedItem (&addedItem);
		selectedUnitList->scroolToItem (&addedItem);
	}
}

//------------------------------------------------------------------------------
template<typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::selectedUnitClicked (SelectedUnitItemType& unitItem)
{
	if (&unitItem == selectedUnitList->getSelectedItem () && tryRemoveSelectedUnit (unitItem))
	{
		selectedUnitList->removeItem (unitItem);
	}
}

#endif // gui_menu_windows_windowadvancedhangar_windowadvancedhangarH
