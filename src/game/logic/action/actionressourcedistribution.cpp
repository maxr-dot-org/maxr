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

#include "actionressourcedistribution.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionRessourceDistribution::cActionRessourceDistribution(const cBuilding& building,	int goldProd, int oilProd, int metalProd) :
	cAction(eActiontype::ACTION_RESSOURCE_DISTRIBUTION),
	buildingId(building.getId()),
	goldProd(goldProd),
	oilProd(oilProd),
	metalProd(metalProd)
{};

//------------------------------------------------------------------------------
cActionRessourceDistribution::cActionRessourceDistribution(cBinaryArchiveOut& archive) :
	cAction(eActiontype::ACTION_RESSOURCE_DISTRIBUTION)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionRessourceDistribution::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto building = model.getBuildingFromID(buildingId);
	if (building == nullptr) return;

	cSubBase& subBase = *building->subBase;

	subBase.setMetalProd(0);
	subBase.setOilProd(0);
	subBase.setGoldProd(0);

	// no need to verify the values.
	// They will be reduced automatically, if necessary
	subBase.setMetalProd(metalProd);
	subBase.setGoldProd(goldProd);
	subBase.setOilProd(oilProd);
}
