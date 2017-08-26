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

#include "destroyjob.h"
#include "utility/crc.h"
#include "game/data/units/unit.h"
#include "game/logic/fxeffects.h"
#include "game/data/model.h"


//------------------------------------------------------------------------------
cDestroyJob::cDestroyJob(cUnit& unit, cModel& model) :
	cJob(unit),
	counter(0)
{
	createDestroyFx(model);
}

//------------------------------------------------------------------------------
void cDestroyJob::run(cModel& model)
{
	if (counter > 0)
	{
		counter--;
	}
	else
	{
		deleteUnit(model);
		finished = true;
	}
}

//------------------------------------------------------------------------------
eJobType cDestroyJob::getType() const
{
	return eJobType::DESTROY;
}

//------------------------------------------------------------------------------
uint32_t cDestroyJob::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(getType(), crc);
	crc = calcCheckSum(unit ? unit->getId() : 0, crc);
	crc = calcCheckSum(counter, crc);

	return crc;
}

//------------------------------------------------------------------------------
void cDestroyJob::createDestroyFx(cModel& model)
{
	const cMap& map = *model.getMap();

	std::shared_ptr<cFx> fx;
	if (unit->isAVehicle())
	{
		auto& vehicle = *static_cast<cVehicle*>(unit);

		if (vehicle.getIsBig())
		{
			fx = std::make_shared<cFxExploBig>(vehicle.getPosition() * 64 + 64, map.isWaterOrCoast(vehicle.getPosition()));
		}
		else if (vehicle.getStaticUnitData().factorAir > 0 && vehicle.getFlightHeight() != 0)
		{
			fx = std::make_shared<cFxExploAir>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32);
		}
		else if (map.isWaterOrCoast(vehicle.getPosition()))
		{
			fx = std::make_shared<cFxExploWater>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32);
		}
		else
		{
			fx = std::make_shared<cFxExploSmall>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32);
		}
		counter = fx->getLength() / 2;
		model.addFx(fx);

		if (vehicle.uiData->hasCorpse)
		{
			// add corpse
			model.addFx(std::make_shared<cFxCorpse>(vehicle.getPosition() * 64 + vehicle.getMovementOffset() + 32));
		}
	}
	else
	{
		auto& building = *static_cast<cBuilding*>(unit);

		const cBuilding* topBuilding = map.getField(building.getPosition()).getBuilding();
		if (topBuilding && topBuilding->getIsBig())
		{
			fx = std::make_shared<cFxExploBig>(topBuilding->getPosition() * 64 + 64, map.isWaterOrCoast(topBuilding->getPosition()));
		}
		else
		{
			fx = std::make_shared<cFxExploSmall>(building.getPosition() * 64 + 32);
		}

		counter = fx->getLength() / 2;
		model.addFx(fx);
	}
}

//------------------------------------------------------------------------------
void cDestroyJob::deleteUnit(cModel& model)
{
	bool isVehicle = false;
	const auto position = unit->getPosition();
	auto& map = *model.getMap();
	auto& field = map.getField(position);

	//delete planes immediately
	if (unit->isAVehicle())
	{
		isVehicle = true;
		auto& vehicle = *static_cast<cVehicle*> (unit);
		if (vehicle.getStaticUnitData().factorAir > 0 && vehicle.getFlightHeight() > 0)
		{
			model.deleteUnit(&vehicle);
			return;
		}
	}

	//check, if there is a big unit on the field
	bool bigUnit = false;
	auto topBuilding = field.getTopBuilding();
	if ((topBuilding && topBuilding->getIsBig()) || unit->getIsBig())
		bigUnit = true;


	//delete unit
	int rubbleValue = 0;
	if (!unit->getStaticUnitData().isHuman)
	{
		rubbleValue += unit->data.getBuildCost();
		// stored material is always added completely to the rubble
		if (unit->getStaticUnitData().storeResType == eResourceType::Metal)
			rubbleValue += unit->getStoredResources() * 2;
	}
	model.deleteUnit(unit);

	//delete all buildings (except connectors, when a vehicle is destroyed)
	rubbleValue += deleteAllBuildingsOnField(field, !isVehicle, model);
	if (bigUnit)
	{
		rubbleValue += deleteAllBuildingsOnField(map.getField(position + cPosition(1, 0)), !isVehicle, model);
		rubbleValue += deleteAllBuildingsOnField(map.getField(position + cPosition(0, 1)), !isVehicle, model);
		rubbleValue += deleteAllBuildingsOnField(map.getField(position + cPosition(1, 1)), !isVehicle, model);
	}

	//add rubble
	auto rubble = field.getRubble();
	if (rubble)
	{
		rubble->setRubbleValue(rubble->getRubbleValue() + rubbleValue / 2, model.randomGenerator);
	}
	else
	{
		if (rubbleValue > 2)
			model.addRubble(position, rubbleValue / 2, bigUnit);
	}
}

//------------------------------------------------------------------------------
int cDestroyJob::deleteAllBuildingsOnField(cMapField& field, bool deleteConnector, cModel& model)
{
	auto buildings = field.getBuildings();
	int rubble = 0;

	for (auto b_it = buildings.begin(); b_it != buildings.end(); ++b_it)
	{
		if ((*b_it)->getStaticUnitData().surfacePosition == cStaticUnitData::SURFACE_POS_ABOVE && deleteConnector == false) continue;
		if ((*b_it)->isRubble()) continue;

		if ((*b_it)->getStaticUnitData().surfacePosition != cStaticUnitData::SURFACE_POS_ABOVE) //no rubble for connectors
			rubble += (*b_it)->data.getBuildCost();
		if ((*b_it)->getStaticUnitData().storeResType == eResourceType::Metal)
			rubble += (*b_it)->getStoredResources() * 2; // stored material is always added completely to the rubble

		model.deleteUnit(*b_it);
	}

	return rubble;
}


