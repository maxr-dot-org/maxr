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

#include "windowadvancedhangar.h"

#include "../../widgets/pushbutton.h"
#include "../../widgets/listview.h"
#include "../../widgets/special/unitlistviewitembuy.h"
#include "../../widgets/special/unitlistviewitemcargo.h"

//------------------------------------------------------------------------------
cWindowAdvancedHangar::cWindowAdvancedHangar (SDL_Surface* surface, int playerColor, int playerClan) :
	cWindowHangar (surface, playerColor, playerClan)
{
	const auto& menuPosition = getArea ().getMinCorner ();

	using namespace std::placeholders;

	selectedUnitList = addChild (std::make_unique<cListView<cUnitListViewItemCargo>> (cBox<cPosition> (menuPosition + cPosition (330, 14), menuPosition + cPosition (330 + 130, 12 + 220))));
	signalConnectionManager.connect (selectedUnitList->itemClicked, std::bind (&cWindowAdvancedHangar::selectedUnitClicked, this, _1));
	signalConnectionManager.connect (selectedUnitList->selectionChanged, std::bind (&cWindowAdvancedHangar::handleSelectionChanged, this));

	auto upButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (327, 240), ePushButtonType::ArrowUpSmall, SoundData.SNDObjectMenu));
	signalConnectionManager.connect (upButton->clicked, std::bind (&cListView<cUnitListViewItemCargo>::pageUp, selectedUnitList));

	auto downButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (348, 240), ePushButtonType::ArrowDownSmall, SoundData.SNDObjectMenu));
	signalConnectionManager.connect (downButton->clicked, std::bind (&cListView<cUnitListViewItemCargo>::pageDown, selectedUnitList));

	using namespace std::placeholders;

	signalConnectionManager.connect (selectionUnitClickedSecondTime, std::bind (&cWindowAdvancedHangar::handleSelectionUnitClickedSecondTime, this, _1));
}

//------------------------------------------------------------------------------
cWindowAdvancedHangar::~cWindowAdvancedHangar ()
{}

//------------------------------------------------------------------------------
bool cWindowAdvancedHangar::tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const
{
	return true;
}

//------------------------------------------------------------------------------
bool cWindowAdvancedHangar::tryRemoveSelectedUnit (const cUnitListViewItemCargo& unitItem) const
{
	return true;
}

//------------------------------------------------------------------------------
cUnitListViewItemCargo& cWindowAdvancedHangar::addSelectedUnit (const sID& unitId)
{
	auto addedItem = selectedUnitList->addItem (std::make_unique<cUnitListViewItemCargo> (selectedUnitList->getSize ().x () - 9, unitId, getPlayer ()));
	return *addedItem;
}

//------------------------------------------------------------------------------
size_t cWindowAdvancedHangar::getSelectedUnitsCount () const
{
	return selectedUnitList->getItemsCount ();
}

//------------------------------------------------------------------------------
cUnitListViewItemCargo& cWindowAdvancedHangar::getSelectedUnit (size_t index)
{
	return selectedUnitList->getItem (index);
}

//------------------------------------------------------------------------------
const cUnitListViewItemCargo& cWindowAdvancedHangar::getSelectedUnit (size_t index) const
{
	return selectedUnitList->getItem (index);
}

//------------------------------------------------------------------------------
void cWindowAdvancedHangar::handleSelectionChanged ()
{
	auto selectedItem = selectedUnitList->getSelectedItem ();
	if (selectedItem != nullptr)
	{
		setActiveUnit (selectedItem->getUnitId ());
	}
	selectedUnitSelectionChanged (selectedItem);
}

//------------------------------------------------------------------------------
void cWindowAdvancedHangar::handleSelectionUnitClickedSecondTime (const cUnitListViewItemBuy& unitItem)
{
	if (tryAddSelectedUnit (unitItem))
	{
		auto& addedItem = addSelectedUnit (unitItem.getUnitId ());
		selectedUnitList->setSelectedItem (&addedItem);
		selectedUnitList->scroolToItem (&addedItem);
	}
}

//------------------------------------------------------------------------------
void cWindowAdvancedHangar::selectedUnitClicked (cUnitListViewItemCargo& unitItem)
{
	if (&unitItem == selectedUnitList->getSelectedItem () && tryRemoveSelectedUnit (unitItem))
	{
		selectedUnitList->removeItem (unitItem);
	}
}
