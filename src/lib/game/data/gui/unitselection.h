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

#ifndef game_data_gui_unitselectionH
#define game_data_gui_unitselectionH

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <utility>
#include <vector>

class cPosition;
class cMapView;
class cPlayer;
class cMapField;
class cUnit;
class cVehicle;
class cBuilding;
class cMapFieldView;
template <typename T>
class cBox;

class cUnitSelection
{
public:
	bool selectUnitAt (const cMapFieldView&, bool base);
	bool selectVehiclesAt (const cBox<cPosition>&, const cMapView&, const cPlayer&);
	bool selectUnit (cUnit&, bool add = false);
	bool selectNextUnit (const cPlayer&, const std::vector<unsigned int>& doneList);
	bool selectPrevUnit (const cPlayer&, const std::vector<unsigned int>& doneList);

	void deselectUnit (const cUnit&);
	void deselectUnits();

	cUnit* getSelectedUnit() const;
	cVehicle* getSelectedVehicle() const;
	cBuilding* getSelectedBuilding() const;

	std::vector<cUnit*> getSelectedUnits() const;
	std::vector<cVehicle*> getSelectedVehicles() const;
	std::vector<cBuilding*> getSelectedBuildings() const;

	size_t getSelectedUnitsCount() const;
	size_t getSelectedVehiclesCount() const;
	size_t getSelectedBuildingsCount() const;

	bool isSelected (const cUnit&) const;

	bool canSelect (const cUnit*) const;

	bool canSelectNextUnit (const cPlayer&, const std::vector<unsigned int>& doneList) const;

	mutable cSignal<void()> selectionChanged;
	mutable cSignal<void()> mainSelectionChanged;
	mutable cSignal<void()> groupSelectionChanged;

private:
	std::vector<std::pair<cUnit*, cSignalConnectionManager>> selectedUnits;

	void addSelectedUnitBack (cUnit&);
	void addSelectedUnitFront (cUnit&);

	void removeSelectedUnit (const cUnit&);
	void removeAllSelectedUnits();

	cVehicle* getNextVehicle (const cPlayer&, const std::vector<unsigned int>& doneList, const cVehicle* start) const;
	cBuilding* getNextBuilding (const cPlayer&, const std::vector<unsigned int>& doneList, const cBuilding* start) const;
	cBuilding* getNextMiningStation (const cPlayer&, const cBuilding* start) const;
	cUnit* getNextUnit (const cPlayer&, const std::vector<unsigned int>& doneList, cUnit* start) const;

	cVehicle* getPrevVehicle (const cPlayer&, const std::vector<unsigned int>& doneList, const cVehicle* start) const;
	cBuilding* getPrevBuilding (const cPlayer&, const std::vector<unsigned int>& doneList, const cBuilding* start) const;
	cBuilding* getPrevMiningStation (const cPlayer&, const cBuilding* start) const;
	cUnit* getPrevUnit (const cPlayer&, const std::vector<unsigned int>& doneList, cUnit* start) const;
};

#endif
