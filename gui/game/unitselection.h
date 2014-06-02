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

#ifndef gui_game_unitselectionH
#define gui_game_unitselectionH

#include <vector>

#include "../../utility/signal/signalconnectionmanager.h"
#include "../../utility/signal/signal.h"

class cPosition;
class cMap;
class cPlayer;
class cMapField;
class cUnit;
class cVehicle;
class cBuilding;
template<typename T> class cBox;

class cUnitSelection
{
public:
	bool selectUnitAt (const cMapField& field, bool base);

	bool selectVehiclesAt (const cBox<cPosition>& box, const cMap& map, const cPlayer& player);

	bool selectUnit (cUnit& unit, bool add = false);

	void deselectUnit (const cUnit& unit);
	void deselectUnits ();

	cUnit* getSelectedUnit () const;
	cVehicle* getSelectedVehicle () const;
	cBuilding* getSelectedBuilding () const;

	std::vector<cUnit*> getSelectedUnits () const;
	std::vector<cVehicle*> getSelectedVehicles () const;
	std::vector<cBuilding*> getSelectedBuildings () const;

	size_t getSelectedUnitsCount () const;
	size_t getSelectedVehiclesCount () const;
	size_t getSelectedBuildingsCount () const;

	bool isSelected (const cUnit& unit) const;

	cSignal<void ()> selectionChanged;
	cSignal<void ()> mainSelectionChanged;
	cSignal<void ()> groupSelectionChanged;
private:
	cSignalConnectionManager selectedUnitsSignalConnectionManager;

	std::vector<std::pair<cUnit*, cSignalConnection>> selectedUnits;

	void addSelectedUnitBack (cUnit& unit);
	void addSelectedUnitFront (cUnit& unit);

	void removeSelectedUnit (const cUnit& unit);

	void removeAllSelectedUnits ();
};

#endif // gui_game_unitselectionH
