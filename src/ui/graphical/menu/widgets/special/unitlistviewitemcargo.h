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

#ifndef ui_graphical_menu_widgets_special_unitlistviewitemcargoH
#define ui_graphical_menu_widgets_special_unitlistviewitemcargoH

#include "ui/graphical/menu/widgets/special/unitlistviewitem.h"

class cUnitListViewItemCargo : public cUnitListViewItem
{
public:
	cUnitListViewItemCargo (unsigned int width, const sID& unitId, const cPlayer& owner, const cUnitsData&);

	int getCargo() const { return cargo; }
	void setCargo (int);

private:
	void updateCargoLabel();

private:
	cLabel* cargoLabel = nullptr;
	int cargo = 0;
	int cargoMax = 0;
};

#endif // ui_graphical_menu_widgets_special_unitlistviewitemcargoH
