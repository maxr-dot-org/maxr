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

#ifndef ui_graphical_game_widgets_unitdetailsstoredH
#define ui_graphical_game_widgets_unitdetailsstoredH

#include <array>

#include "ui/graphical/widget.h"

#include "ui/graphical/menu/widgets/special/unitdatasymboltype.h"
#include "utility/signal/signalconnectionmanager.h"

class cLabel;
class cUnit;
class cPlayer;

class cUnitDetailsStored : public cWidget
{
public:
	explicit cUnitDetailsStored (const cBox<cPosition>& area);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;

	void setUnit (const cUnit* unit);
private:
	cSignalConnectionManager unitSignalConnectionManager;

	AutoSurface surface;

	void reset();

	void drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, int maximalAmount, const std::string& name);

	static const size_t maxRows = 2;
	static const int rowHeight = 15;

	std::array<cLabel*, maxRows> amountLabels;
	std::array<cLabel*, maxRows> nameLabels;

	const cUnit* unit;
};

#endif // ui_graphical_game_widgets_unitdetailsstoredH
