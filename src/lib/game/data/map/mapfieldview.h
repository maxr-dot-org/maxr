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

#ifndef game_data_map_mapfieldviewH
#define game_data_map_mapfieldviewH

#include "utility/signal/signal.h"

#include <vector>

class cMapField;
class cVehicle;
class cBuilding;
class cPlayer;
class cUnit;
struct sTerrain;

class cMapFieldView
{
public:
	cMapFieldView (const cMapField&, const sTerrain&, const cPlayer*);

	/** returns the top vehicle on this field */
	cVehicle* getVehicle() const;
	/** returns a Iterator for the planes on this field */
	cVehicle* getPlane() const;
	/** returns a pointer for the buildings on this field */
	cBuilding* getBuilding() const;
	/** returns a pointer to the top building or nullptr if the first building is a base type */
	cBuilding* getTopBuilding() const;
	/** returns a pointer to the first base building or nullptr if there is no base building */
	cBuilding* getBaseBuilding() const;
	/** returns a pointer to a rubble object, if there is one. */
	cBuilding* getRubble() const;
	/** returns a pointer to an expl. mine, if there is one */
	cBuilding* getMine() const;
	/** checks if there is a building that allows gorund units on water fields */
	bool hasBridgeOrPlattform() const;

	//TODO: maybe use iterators here, to prevent copying the unit vectors
	/** returns the buildings on this field */
	std::vector<cBuilding*> getBuildings() const;
	/** returns the vehicles on this field */
	std::vector<cVehicle*> getVehicles() const;
	/** returns the planes on this field */
	std::vector<cVehicle*> getPlanes() const;
	std::vector<cUnit*> getUnits() const;

	cSignal<void()>& unitsChanged;

private:
	const cMapField& mapField;
	const sTerrain& terrain;
	const cPlayer* player = nullptr; // may be null
};

#endif
