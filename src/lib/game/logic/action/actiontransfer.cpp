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
cActionTransfer::cActionTransfer (const cUnit& sourceUnit, const cUnit& destinationUnit, int transferValue_, eResourceType resourceType_) :
	sourceUnitId (sourceUnit.getId()),
	destinationUnitId (destinationUnit.getId()),
	transferValue (transferValue_),
	resourceType (resourceType_)
{}

//------------------------------------------------------------------------------
cActionTransfer::cActionTransfer (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionTransfer::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	const auto sourceUnit = model.getUnitFromID (sourceUnitId);
	if (sourceUnit == nullptr) return;

	const auto destinationUnit = model.getUnitFromID (destinationUnitId);
	if (destinationUnit == nullptr) return;

	if (auto* sourceBuilding = dynamic_cast<cBuilding*> (sourceUnit))
	{
		if (auto* destinationBuilding = dynamic_cast<cBuilding*> (destinationUnit))
		{
			if (sourceBuilding->subBase != destinationBuilding->subBase) return;
			if (sourceBuilding->getOwner() != destinationBuilding->getOwner()) return;
			if (sourceBuilding->getStaticUnitData().storeResType != resourceType) return;
			if (sourceBuilding->getStaticUnitData().storeResType != destinationBuilding->getStaticUnitData().storeResType) return;
			if (destinationBuilding->getStoredResources() + transferValue > destinationBuilding->getStaticUnitData().storageResMax || destinationBuilding->getStoredResources() + transferValue < 0) return;
			if (sourceBuilding->getStoredResources() - transferValue > sourceBuilding->getStaticUnitData().storageResMax || sourceBuilding->getStoredResources() - transferValue < 0) return;

			destinationBuilding->setStoredResources (destinationBuilding->getStoredResources() + transferValue);
			sourceBuilding->setStoredResources (sourceBuilding->getStoredResources() - transferValue);
		}
		else if (auto* destinationVehicle = dynamic_cast<cVehicle*> (destinationUnit))
		{
			if (destinationVehicle->isUnitBuildingABuilding() || destinationVehicle->isUnitClearing()) return;
			if (destinationVehicle->getStaticUnitData().storeResType != resourceType) return;
			if (destinationVehicle->getStoredResources() + transferValue > destinationVehicle->getStaticUnitData().storageResMax || destinationVehicle->getStoredResources() + transferValue < 0) return;

			bool breakSwitch = false;
			const sMiningResource& sourceStored = sourceBuilding->subBase->getResourcesStored();
			const sMiningResource& sourceMaxStored = sourceBuilding->subBase->getMaxResourcesStored();

			switch (resourceType)
			{
				case eResourceType::None: break;
				case eResourceType::Metal:
				{
					if (sourceStored.metal - transferValue > sourceMaxStored.metal || sourceStored.metal - transferValue < 0) breakSwitch = true;
					if (!breakSwitch) sourceBuilding->subBase->addMetal (-transferValue);
				}
				break;
				case eResourceType::Oil:
				{
					if (sourceStored.oil - transferValue > sourceMaxStored.oil || sourceStored.oil - transferValue < 0) breakSwitch = true;
					if (!breakSwitch) sourceBuilding->subBase->addOil (-transferValue);
				}
				break;
				case eResourceType::Gold:
				{
					if (sourceStored.gold - transferValue > sourceMaxStored.gold || sourceStored.gold - transferValue < 0) breakSwitch = true;
					if (!breakSwitch) sourceBuilding->subBase->addGold (-transferValue);
				}
				break;
			}
			if (breakSwitch) return;

			destinationVehicle->setStoredResources (destinationVehicle->getStoredResources() + transferValue);
		}
	}
	else if (auto* sourceVehicle = dynamic_cast<cVehicle*> (sourceUnit))
	{
		if (sourceVehicle->getStaticUnitData().storeResType != resourceType) return;
		if (sourceVehicle->isUnitBuildingABuilding() || sourceVehicle->isUnitClearing()) return;
		if (sourceVehicle->getStoredResources() - transferValue > sourceVehicle->getStaticUnitData().storageResMax || sourceVehicle->getStoredResources() - transferValue < 0) return;

		if (auto destinationBuilding = dynamic_cast<cBuilding*> (destinationUnit))
		{
			const sMiningResource& destinationStored = destinationBuilding->subBase->getResourcesStored();
			const sMiningResource& destinationMaxStored = destinationBuilding->subBase->getMaxResourcesStored();

			bool breakSwitch = false;
			switch (resourceType)
			{
				case eResourceType::None: break;
				case eResourceType::Metal:
				{
					if (destinationStored.metal + transferValue > destinationMaxStored.metal || destinationStored.metal + transferValue < 0) breakSwitch = true;
					if (!breakSwitch) destinationBuilding->subBase->addMetal (transferValue);
				}
				break;
				case eResourceType::Oil:
				{
					if (destinationStored.oil + transferValue > destinationMaxStored.oil || destinationStored.oil + transferValue < 0) breakSwitch = true;
					if (!breakSwitch) destinationBuilding->subBase->addOil (transferValue);
				}
				break;
				case eResourceType::Gold:
				{
					if (destinationStored.gold + transferValue > destinationMaxStored.gold || destinationStored.gold + transferValue < 0) breakSwitch = true;
					if (!breakSwitch) destinationBuilding->subBase->addGold (transferValue);
				}
				break;
			}
			if (breakSwitch) return;
		}
		else if (auto destinationVehicle = dynamic_cast<cVehicle*> (destinationUnit))
		{
			if (destinationVehicle->isUnitBuildingABuilding() || destinationVehicle->isUnitClearing()) return;
			if (destinationVehicle->getStaticUnitData().storeResType != resourceType) return;
			if (destinationVehicle->getStoredResources() + transferValue > destinationVehicle->getStaticUnitData().storageResMax || destinationVehicle->getStoredResources() + transferValue < 0) return;
			destinationVehicle->setStoredResources (destinationVehicle->getStoredResources() + transferValue);
		}

		sourceVehicle->setStoredResources (sourceVehicle->getStoredResources() - transferValue);
	}
}
