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

class cUnitListViewItemCargo;

class cWindowAdvancedHangar : public cWindowHangar
{
public:
	explicit cWindowAdvancedHangar (SDL_Surface* surface, int playerColor, int playerClan);
	~cWindowAdvancedHangar ();

protected:
	cUnitListViewItemCargo& addSelectedUnit (const sID& unitId);

	size_t getSelectedUnitsCount () const;
	cUnitListViewItemCargo& getSelectedUnit (size_t index);
	const cUnitListViewItemCargo& getSelectedUnit (size_t index) const;

	virtual bool tryAddSelectedUnit (const cUnitListViewItemBuy& unitItem) const;
	virtual bool tryRemoveSelectedUnit (const cUnitListViewItemCargo& unitItem) const;

	cSignal<void (cUnitListViewItemCargo*)> selectedUnitSelectionChanged;
private:
	cSignalConnectionManager signalConnectionManager;

	cListView<cUnitListViewItemCargo>* selectedUnitList;

	void handleSelectionChanged ();

	void handleSelectionUnitClickedSecondTime (const cUnitListViewItemBuy& item);

	void selectedUnitClicked (cUnitListViewItemCargo& unitItem);
};

#endif // gui_menu_windows_windowadvancedhangar_windowadvancedhangarH
