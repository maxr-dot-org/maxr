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

#ifndef ui_graphical_menu_windows_windowadvancedhangar_windowadvancedhangarH
#define ui_graphical_menu_windows_windowadvancedhangar_windowadvancedhangarH

#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"
#include "ui/graphical/menu/windows/windowhangar/windowhangar.h"
#include "utility/color.h"

template <typename SelectedUnitItemType>
class cWindowAdvancedHangar : public cWindowHangar
{
public:
	cWindowAdvancedHangar (UniqueSurface, std::shared_ptr<const cUnitsData>, cRgbColor playerColor, int playerClan);
	cWindowAdvancedHangar (UniqueSurface, std::shared_ptr<const cUnitsData>, const cPlayer&);
	~cWindowAdvancedHangar() = default;

protected:
	SelectedUnitItemType& addSelectedUnit (const sID& unitId);

	size_t getSelectedUnitsCount() const;
	SelectedUnitItemType& getSelectedUnit (size_t index);
	const SelectedUnitItemType& getSelectedUnit (size_t index) const;

	virtual bool tryAddSelectedUnit (const cUnitListViewItemBuy&) const { return true; }
	virtual bool tryRemoveSelectedUnit (const SelectedUnitItemType&) const { return true; }

	// TODO: the following widgets should be private instead.
	// They are protect at the moment because some inheriting windows need to move/resize the widgets.
	cListView<SelectedUnitItemType>* selectedUnitList = nullptr;
	cPushButton* selectedListUpButton = nullptr;
	cPushButton* selectedListDownButton = nullptr;

	cSignal<void (SelectedUnitItemType*)> selectedUnitSelectionChanged;

private:
	cSignalConnectionManager signalConnectionManager;

	void initialize();

	void handleSelectionChanged();

	void handleSelectionUnitClickedSecondTime (const cUnitListViewItemBuy& item);

	void selectedUnitClicked (SelectedUnitItemType& unitItem);
};

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
cWindowAdvancedHangar<SelectedUnitItemType>::cWindowAdvancedHangar (UniqueSurface surface, std::shared_ptr<const cUnitsData> unitsData, cRgbColor playerColor, int playerClan) :
	cWindowHangar (std::move (surface), unitsData, playerColor, playerClan)
{
	initialize();
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
cWindowAdvancedHangar<SelectedUnitItemType>::cWindowAdvancedHangar (UniqueSurface surface, std::shared_ptr<const cUnitsData> unitsData, const cPlayer& player) :
	cWindowHangar (std::move (surface), unitsData, player)
{
	initialize();
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::initialize()
{
	selectedUnitList = emplaceChild<cListView<SelectedUnitItemType>> (cBox<cPosition> (getPosition() + cPosition (330, 14), getPosition() + cPosition (330 + 130, 12 + 220)));
	selectedUnitList->setEndMargin (cPosition (2, 9));
	signalConnectionManager.connect (selectedUnitList->itemClicked, [this] (SelectedUnitItemType& unitItem) { selectedUnitClicked (unitItem); });
	signalConnectionManager.connect (selectedUnitList->selectionChanged, [this]() { handleSelectionChanged(); });

	selectedListUpButton = emplaceChild<cPushButton> (getPosition() + cPosition (327, 240), ePushButtonType::ArrowUpSmall, &SoundData.SNDObjectMenu);
	signalConnectionManager.connect (selectedListUpButton->clicked, [this]() { selectedUnitList->pageUp(); });

	selectedListDownButton = emplaceChild<cPushButton> (getPosition() + cPosition (348, 240), ePushButtonType::ArrowDownSmall, &SoundData.SNDObjectMenu);
	signalConnectionManager.connect (selectedListDownButton->clicked, [this]() { selectedUnitList->pageDown(); });

	signalConnectionManager.connect (selectionUnitClickedSecondTime, [this] (const cUnitListViewItemBuy& unitItem) { handleSelectionUnitClickedSecondTime (unitItem); });
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
SelectedUnitItemType& cWindowAdvancedHangar<SelectedUnitItemType>::addSelectedUnit (const sID& unitId)
{
	auto addedItem = selectedUnitList->addItem (std::make_unique<SelectedUnitItemType> (selectedUnitList->getSize().x() - 9, unitId, getPlayer(), *unitsData));
	return *addedItem;
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
size_t cWindowAdvancedHangar<SelectedUnitItemType>::getSelectedUnitsCount() const
{
	return selectedUnitList->getItemsCount();
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
SelectedUnitItemType& cWindowAdvancedHangar<SelectedUnitItemType>::getSelectedUnit (size_t index)
{
	return selectedUnitList->getItem (index);
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
const SelectedUnitItemType& cWindowAdvancedHangar<SelectedUnitItemType>::getSelectedUnit (size_t index) const
{
	return selectedUnitList->getItem (index);
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::handleSelectionChanged()
{
	auto selectedItem = selectedUnitList->getSelectedItem();
	if (selectedItem != nullptr)
	{
		setActiveUnit (selectedItem->getUnitId());
	}
	selectedUnitSelectionChanged (selectedItem);
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::handleSelectionUnitClickedSecondTime (const cUnitListViewItemBuy& unitItem)
{
	if (tryAddSelectedUnit (unitItem))
	{
		auto& addedItem = addSelectedUnit (unitItem.getUnitId());
		selectedUnitList->setSelectedItem (&addedItem);
		selectedUnitList->scrollToItem (&addedItem);
	}
}

//------------------------------------------------------------------------------
template <typename SelectedUnitItemType>
void cWindowAdvancedHangar<SelectedUnitItemType>::selectedUnitClicked (SelectedUnitItemType& unitItem)
{
	if (&unitItem == selectedUnitList->getSelectedItem() && tryRemoveSelectedUnit (unitItem))
	{
		selectedUnitList->removeItem (unitItem);
	}
}

#endif // ui_graphical_menu_windows_windowadvancedhangar_windowadvancedhangarH
