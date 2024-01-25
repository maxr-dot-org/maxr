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

#ifndef ui_graphical_menu_widgets_special_unitlistviewitemH
#define ui_graphical_menu_widgets_special_unitlistviewitemH

#include "ui/graphical/menu/widgets/abstractlistviewitem.h"
#include "game/data/units/id.h"

class cImage;
class cLabel;
class cPlayer;
class cUnitsData;

class cUnitListViewItem : public cAbstractListViewItem
{
public:
	// TODO: remove the player parameter.
	//       The only reason it is here is because we need to know the
	//       color of the unit.
	//       Take note of the TODO in the constructor implementation as well.
	cUnitListViewItem (unsigned int width, const sID& unitId, const cPlayer& owner, const cUnitsData&);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

	const sID& getUnitId() const { return unitId; }

protected:
	cImage* unitImage = nullptr;
	cLabel* nameLabel = nullptr;

private:
	sID unitId;
};

#endif // ui_graphical_menu_widgets_special_unitlistviewitemH
