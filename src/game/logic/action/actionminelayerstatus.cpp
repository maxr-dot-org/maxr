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

#include "actionminelayerstatus.h"
#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionMinelayerStatus::cActionMinelayerStatus(const cVehicle& vehicle, bool layMines, bool clearMines) :
	cAction(eActiontype::ACTION_MINELAYER_STATUS), 
	vehicleId(vehicle.getId()),
	layMines(layMines),
	clearMines(clearMines)
{};

//------------------------------------------------------------------------------
cActionMinelayerStatus::cActionMinelayerStatus(cBinaryArchiveOut& archive)
	: cAction(eActiontype::ACTION_MINELAYER_STATUS)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionMinelayerStatus::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	cVehicle* vehicle = model.getVehicleFromID(vehicleId);
	if (vehicle == nullptr) return;
	if (vehicle->getOwner()->getId() != playerNr) return;

	if (layMines && clearMines) return;
	if (!vehicle->getStaticUnitData().canPlaceMines) return;

	vehicle->setClearMines(clearMines);
	vehicle->setLayMines(layMines);

	if (vehicle->isUnitClearingMines())
	{
		vehicle->clearMine(model);
	}
	else if (vehicle->isUnitLayingMines())
	{
		vehicle->layMine(model);
	}
}