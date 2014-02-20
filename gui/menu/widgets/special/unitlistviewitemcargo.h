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

#ifndef gui_menu_widgets_special_unitlistviewitemcargoH
#define gui_menu_widgets_special_unitlistviewitemcargoH

#include "unitlistviewitem.h"

struct sUnitData;

class cUnitListViewItemCargo : public cUnitListViewItem
{
public:
	cUnitListViewItemCargo (unsigned int width, const sID& unitId, cPlayer& owner);

	int getCargo () const;
	void setCargo (int cargo);
private:
	const sUnitData* unitData;
	int cargo;

	cLabel* cargoLabel;

	void updateCargoLabel ();
};

#endif // gui_menu_widgets_special_unitlistviewitemcargoH