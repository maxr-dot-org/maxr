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

#ifndef ui_graphical_game_widgets_unitrenamewidgetH
#define ui_graphical_game_widgets_unitrenamewidgetH

#include <memory>

#include "ui/graphical/widget.h"

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cPosition;

template<typename>
class cBox;

class cUnit;

class cLabel;
class cLineEdit;
class cPlayer;
class cUnitsData;

class cUnitRenameWidget : public cWidget
{
public:
	cUnitRenameWidget (const cPosition& position, int width);

	void setUnit (const cUnit* unit, const cUnitsData& unitsData);
	const cUnit* getUnit() const;

	void setPlayer (const cPlayer* player, const cUnitsData& unitsData);

	const std::string& getUnitName() const;

	bool isAt (const cPosition& position) const override;

	cSignal<void ()> unitRenameTriggered;
private:
	cLabel* selectedUnitStatusLabel;
	cLabel* selectedUnitNamePrefixLabel;
	cLineEdit* selectedUnitNameEdit;

	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager unitSignalConnectionManager;

	const cUnit* activeUnit;
	const cPlayer* player;
};

#endif // ui_graphical_game_widgets_unitvideowidgetH
