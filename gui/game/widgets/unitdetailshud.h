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

#ifndef gui_game_widgets_unitdetailshudH
#define gui_game_widgets_unitdetailshudH

#include <array>

#include "../../widget.h"

#include "../../menu/widgets/special/unitdatasymboltype.h"

class cLabel;
class cUnit;
class cPlayer;

class cUnitDetailsHud : public cWidget
{
public:
	explicit cUnitDetailsHud (const cBox<cPosition>& area);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	void setUnit (const cUnit* unit, const cPlayer* player = nullptr);
private:
	AutoSurface surface;

	void reset ();

	void drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, int maximalAmount, const std::string& name);

	void drawSmallSymbols (eUnitDataSymbolType symbolType, const cPosition& position, int value1, int value2);

	cBox<cPosition> getSmallSymbolPosition (eUnitDataSymbolType symbolType);

	static const size_t maxRows = 4;
	static const int rowHeight = 12;

	std::array<cLabel*, maxRows> amountLabels;
	std::array<cLabel*, maxRows> nameLabels;

	const cUnit* unit;
	const cPlayer* player;
};

#endif // gui_game_widgets_unitdetailshudH
