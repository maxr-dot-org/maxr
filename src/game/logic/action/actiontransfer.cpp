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

#include "actiontransfer.h"

#include "game/data/model.h"
#include "game/data/resourcetype.h"

#include "utility/log.h"

//------------------------------------------------------------------------------
cActionTransfer::cActionTransfer(const cUnit& sourceUnit, const cUnit& destinationUnit, int transferValue_, eResourceType resourceType_) :
	cAction(eActiontype::ACTION_TRANSFER),
	sourceUnitId(sourceUnit.getId()),
	destinationUnitId(destinationUnit.getId()),
	transferValue(transferValue_),
	resourceType(resourceType_)
{}

//------------------------------------------------------------------------------
cActionTransfer::cActionTransfer(cBinaryArchiveOut& archive) :
	cAction(eActiontype::ACTION_TRANSFER)
{
	serializeThis(archive);
}

//------------------------------------------------------------------------------
void cActionTransfer::execute(cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	const auto sourceUnit = model.getUnitFromID(sourceUnitId);
	if(sourceUnit == nullptr) return;

	const auto destinationUnit = model.getUnitFromID(destinationUnitId);
	if(destinationUnit == nullptr) return;

	if(sourceUnit->isABuilding())
	{
		const auto sourceBuilding = static_cast<cBuilding*>(sourceUnit);
		if(destinationUnit->isABuilding())
		{
			const auto destinationBuilding = static_cast<cBuilding*>(destinationUnit);

			if(sourceBuilding->subBase != destinationBuilding->subBase) return;
			if(sourceBuilding->getOwner() != destinationBuilding->getOwner()) return;
			if(sourceBuilding->getStaticUnitData().storeResType != resourceType) return;
			if(sourceBuilding->getStaticUnitData().storeResType != destinationBuilding->getStaticUnitData().storeResType) return;
			if(destinationBuilding->getStoredResources() + transferValue > destinationBuilding->getStaticUnitData().storageResMax || destinationBuilding->getStoredResources() + transferValue < 0) return;
			if(sourceBuilding->getStoredResources() - transferValue > sourceBuilding->getStaticUnitData().storageResMax || sourceBuilding->getStoredResources() - transferValue < 0) return;

			destinationBuilding->setStoredResources(destinationBuilding->getStoredResources() + transferValue);
			sourceBuilding->setStoredResources(sourceBuilding->getStoredResources() - transferValue);
		}
		else
		{
			const auto destinationVehicle = static_cast<cVehicle*>(destinationUnit);

			if(destinationVehicle->isUnitBuildingABuilding() || destinationVehicle->isUnitClearing()) return;
			if(destinationVehicle->getStaticUnitData().storeResType != resourceType) return;
			if(destinationVehicle->getStoredResources() + transferValue > destinationVehicle->getStaticUnitData().storageResMax || destinationVehicle->getStoredResources() + transferValue < 0) return;

			bool breakSwitch = false;
			switch(resourceType)
			{
			case eResourceType::None: break;
			case eResourceType::Metal:
				{
					if(sourceBuilding->subBase->getMetalStored() - transferValue > sourceBuilding->subBase->getMaxMetalStored() || sourceBuilding->subBase->getMetalStored() - transferValue < 0) breakSwitch = true;
					if(!breakSwitch) sourceBuilding->subBase->addMetal(-transferValue);
				}
				break;
			case eResourceType::Oil:
				{
					if(sourceBuilding->subBase->getOilStored() - transferValue > sourceBuilding->subBase->getMaxOilStored() || sourceBuilding->subBase->getOilStored() - transferValue < 0) breakSwitch = true;
					if(!breakSwitch) sourceBuilding->subBase->addOil(-transferValue);
				}
				break;
			case eResourceType::Gold:
				{
					if(sourceBuilding->subBase->getGoldStored() - transferValue > sourceBuilding->subBase->getMaxGoldStored() || sourceBuilding->subBase->getGoldStored() - transferValue < 0) breakSwitch = true;
					if(!breakSwitch) sourceBuilding->subBase->addGold(-transferValue);
				}
				break;
			}
			if(breakSwitch) return;

			destinationVehicle->setStoredResources(destinationVehicle->getStoredResources() + transferValue);
		}
	}
	else
	{
		const auto sourceVehicle = static_cast<cVehicle*>(sourceUnit);

		if(sourceVehicle->getStaticUnitData().storeResType != resourceType) return;
		if(sourceVehicle->isUnitBuildingABuilding() || sourceVehicle->isUnitClearing()) return;
		if(sourceVehicle->getStoredResources() - transferValue > sourceVehicle->getStaticUnitData().storageResMax || sourceVehicle->getStoredResources() - transferValue < 0) return;

		if(destinationUnit->isABuilding())
		{
			const auto destinationBuilding = static_cast<cBuilding*>(destinationUnit);

			bool breakSwitch = false;
			switch(resourceType)
			{
			case eResourceType::None: break;
			case eResourceType::Metal:
				{
					if(destinationBuilding->subBase->getMetalStored() + transferValue > destinationBuilding->subBase->getMaxMetalStored() || destinationBuilding->subBase->getMetalStored() + transferValue < 0) breakSwitch = true;
					if(!breakSwitch) destinationBuilding->subBase->addMetal(transferValue);
				}
				break;
			case eResourceType::Oil:
				{
					if(destinationBuilding->subBase->getOilStored() + transferValue > destinationBuilding->subBase->getMaxOilStored() || destinationBuilding->subBase->getOilStored() + transferValue < 0) breakSwitch = true;
					if(!breakSwitch) destinationBuilding->subBase->addOil(transferValue);
				}
				break;
			case eResourceType::Gold:
				{
					if(destinationBuilding->subBase->getGoldStored() + transferValue > destinationBuilding->subBase->getMaxGoldStored() || destinationBuilding->subBase->getGoldStored() + transferValue < 0) breakSwitch = true;
					if(!breakSwitch) destinationBuilding->subBase->addGold(transferValue);
				}
				break;
			}
			if(breakSwitch) return;
		}
		else
		{
			const auto destinationVehicle = static_cast<cVehicle*>(destinationUnit);

			if(destinationVehicle->isUnitBuildingABuilding() || destinationVehicle->isUnitClearing()) return;
			if(destinationVehicle->getStaticUnitData().storeResType != resourceType) return;
			if(destinationVehicle->getStoredResources() + transferValue > destinationVehicle->getStaticUnitData().storageResMax || destinationVehicle->getStoredResources() + transferValue < 0) return;
			destinationVehicle->setStoredResources(destinationVehicle->getStoredResources() + transferValue);
		}

		sourceVehicle->setStoredResources(sourceVehicle->getStoredResources() - transferValue);
	}
}
