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

#ifndef ui_graphical_menu_widgets_special_unitdetailsH
#define ui_graphical_menu_widgets_special_unitdetailsH

#include <array>

#include "ui/graphical/menu/widgets/special/unitdatasymboltype.h"

#include "maxrconfig.h"
#include "ui/graphical/widget.h"
#include "main.h"

class cLabel;
class cUnitUpgrade;
struct sUnitData;
struct sID;

class cUnitDetails : public cWidget
{
public:
	explicit cUnitDetails (const cPosition& position);

	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;;

	const sID* getCurrentUnitId ();

	void setUnit (const sID& unitId, const cPlayer& owner, const sUnitData* unitObjectCurrentData = nullptr, const cUnitUpgrade* upgrades = nullptr);
	void setUpgrades (const cUnitUpgrade* upgrades);
private:
	AutoSurface surface;

	void reset ();

	void drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, const std::string& name, int value1, int value2);

	void drawBigSymbols (eUnitDataSymbolType symbolType, const cPosition& position, int value1, int value2);

	cBox<cPosition> getBigSymbolPosition (eUnitDataSymbolType symbolType);

	static const size_t maxRows = 9;
	static const int rowHeight = 19;

	std::array<cLabel*, maxRows> amountLabels;
	std::array<cLabel*, maxRows> nameLabels;

	sID unitId;
	const sUnitData* playerOriginalData;
	const sUnitData* playerCurrentData;
	const sUnitData* unitObjectCurrentData;
	const cUnitUpgrade* upgrades;
};

#endif // ui_graphical_menu_widgets_special_unitdetailsH
