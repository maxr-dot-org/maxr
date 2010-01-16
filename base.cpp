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
#include <assert.h>
#include "base.h"
#include "map.h"
#include "serverevents.h"
#include "server.h"



sSubBase::sSubBase( cPlayer* owner_ ) :
	buildings(),
	owner(owner_),
	MaxMetal(),
	Metal(),
	MaxOil(),
	Oil(),
	MaxGold(),
	Gold(),
	MaxEnergyProd(),
	EnergyProd(),
	MaxEnergyNeed(),
	EnergyNeed(),
	MetalNeed(),
	OilNeed(),
	GoldNeed(),
	MaxMetalNeed(),
	MaxOilNeed(),
	MaxGoldNeed(),
	HumanProd(),
	HumanNeed(),
	MaxHumanNeed(),
	MetalProd(),
	OilProd(),
	GoldProd()
{}

sSubBase::sSubBase( const sSubBase& sb ) :
	buildings(),
	owner(sb.owner),
	MaxMetal(sb.MaxMetal),
	Metal(sb.Metal),
	MaxOil(sb.MaxOil),
	Oil(sb.Oil),
	MaxGold(sb.MaxGold),
	Gold(sb.Gold),
	MaxEnergyProd(sb.MaxEnergyProd),
	EnergyProd(sb.EnergyProd),
	MaxEnergyNeed(sb.MaxEnergyNeed),
	EnergyNeed(sb.EnergyNeed),
	MetalNeed(sb.MetalNeed),
	OilNeed(sb.OilNeed),
	GoldNeed(sb.GoldNeed),
	MaxMetalNeed(sb.MaxMetalNeed),
	MaxOilNeed(sb.MaxOilNeed),
	MaxGoldNeed(sb.MaxGoldNeed),
	HumanProd(sb.HumanProd),
	HumanNeed(sb.HumanNeed),
	MaxHumanNeed(sb.MaxHumanNeed),
	MetalProd(sb.MetalProd),
	OilProd(sb.OilProd),
	GoldProd(sb.GoldProd)
{
	for ( unsigned int i = 0; i < sb.buildings.Size(); i++ )
	{
		buildings.Add( sb.buildings[i] );
	}
}

int sSubBase::getMaxMetalProd()
{
	return calcMaxProd(RES_METAL);
}

int sSubBase::getMaxGoldProd()
{
	return calcMaxProd(RES_GOLD);
}

int sSubBase::getMaxOilProd()
{
	return calcMaxProd(RES_OIL);
}

int sSubBase::getMaxAllowedMetalProd()
{
	return calcMaxAllowedProd( RES_METAL );
}

int sSubBase::getMaxAllowedGoldProd()
{
	return calcMaxAllowedProd( RES_GOLD );
}

int sSubBase::getMaxAllowedOilProd()
{
	return calcMaxAllowedProd( RES_OIL );
}


int sSubBase::getMetalProd()
{
	return MetalProd;
}

int sSubBase::getGoldProd()
{
	return GoldProd;
}

int sSubBase::getOilProd()
{
	return OilProd;
}

void sSubBase::setMetalProd( int i )
{
	int max = getMaxAllowedMetalProd();

	if ( i < 0 )
		i = 0;

	if ( i > max )
		i = max;

	MetalProd = i;
}

void sSubBase::setGoldProd( int i )
{
	int max = getMaxAllowedGoldProd();

	if ( i < 0 )
		i = 0;

	if ( i > max )
		i = max;

	GoldProd = i;
}

void sSubBase::setOilProd( int i )
{
	int max = getMaxAllowedOilProd();

	if ( i < 0 )
		i = 0;

	if ( i > max )
		i = max;

	OilProd = i;
}

void sSubBase::changeMetalProd( int i )
{
	setMetalProd( MetalProd + i );
}

void sSubBase::changeOilProd( int i )
{
	setOilProd( OilProd + i );
}

void sSubBase::changeGoldProd( int i )
{
	setGoldProd( GoldProd + i );
}

int sSubBase::calcMaxProd( int ressourceType )
{
	int maxProd = 0;
	for ( unsigned int i = 0; i < buildings.Size(); i++ )
	{
		cBuilding* building = buildings[i];

		if ( !(building->data.canMineMaxRes > 0 && building->IsWorking) ) continue;

		switch ( ressourceType )
		{
		case RES_METAL:
			maxProd += building->MaxMetalProd;
			break;
		case RES_OIL:
			maxProd += building->MaxOilProd;
			break;
		case RES_GOLD:
			maxProd += building->MaxGoldProd;
			break;
		}
	}

	return maxProd;
}

int sSubBase::calcMaxAllowedProd( int ressourceType )
{
	//initialise needed Variables and element pointers, so the algorithm itself is independent from the ressouce type
	int maxAllowedProd;
	int ressourceToDistributeB;
	int ressourceToDistributeC;

	int cBuilding::* ressourceProdA;
	int cBuilding::* ressourceProdB;
	int cBuilding::* ressourceProdC;

	switch ( ressourceType )
	{
	case RES_METAL:
		maxAllowedProd = getMaxMetalProd();
		ressourceToDistributeB = GoldProd;
		ressourceToDistributeC = OilProd;
		ressourceProdA = &cBuilding::MaxMetalProd;
		ressourceProdB = &cBuilding::MaxGoldProd;
		ressourceProdC = &cBuilding::MaxOilProd;
		break;
	case RES_OIL:
		maxAllowedProd = getMaxOilProd();
		ressourceToDistributeB = MetalProd;
		ressourceToDistributeC = GoldProd;
		ressourceProdA = &cBuilding::MaxOilProd;
		ressourceProdB = &cBuilding::MaxMetalProd;
		ressourceProdC = &cBuilding::MaxGoldProd;
		break;
	case RES_GOLD:
		maxAllowedProd = getMaxGoldProd();
		ressourceToDistributeB = MetalProd;
		ressourceToDistributeC = OilProd;
		ressourceProdA = &cBuilding::MaxGoldProd;
		ressourceProdB = &cBuilding::MaxMetalProd;
		ressourceProdC = &cBuilding::MaxOilProd;
		break;
	default:
		return 0;
	}


	//when calculating the maximum allowed production for ressource A, the algo tries to distribute
	// the ressources B and C so that the maximum possible production capacity is left over for A.
	//the actual production values of each mine are not saved, because they are not needed.

	//step one:
	//distribute ressources, that do not decrease the possible production of the others
	for ( unsigned int i = 0; i < buildings.Size(); i++ )
	{
		cBuilding* building = buildings[i];

		if ( !(building->data.canMineMaxRes > 0 && building->IsWorking )) continue;

		//how much of B can be produced in this mine, without decreasing the possible production of A and C?
		int amount = min( building->*ressourceProdB, building->data.canMineMaxRes - building->*ressourceProdA - building->*ressourceProdC );
		if ( amount > 0 ) ressourceToDistributeB -= amount;

		//how much of C can be produced in this mine, without decreasing the possible production of A and B?
		amount = min( building->*ressourceProdC, building->data.canMineMaxRes - building->*ressourceProdA - building->*ressourceProdB );
		if ( amount > 0 ) ressourceToDistributeC -= amount;

	}

	if ( ressourceToDistributeB < 0 ) ressourceToDistributeB = 0;
	if ( ressourceToDistributeC < 0 ) ressourceToDistributeC = 0;

	//step two:
	//distribute ressources, that do not decrease the possible production of A
	for ( unsigned int i = 0; i < buildings.Size(); i++ )
	{
		cBuilding* building = buildings[i];

		if ( !(building->data.canMineMaxRes > 0 && building->IsWorking )) continue;

		int freeB = min ( building->data.canMineMaxRes - building->*ressourceProdA, building->*ressourceProdB);
		int freeC = min ( building->data.canMineMaxRes - building->*ressourceProdA, building->*ressourceProdC);

		//substract values from step 1
		freeB -= min( max(building->data.canMineMaxRes - building->*ressourceProdA - building->*ressourceProdC, 0), building->*ressourceProdB);
		freeC -= min( max(building->data.canMineMaxRes - building->*ressourceProdA - building->*ressourceProdB, 0), building->*ressourceProdC);

		if ( ressourceToDistributeB > 0 )
		{
			int value = min( freeB, ressourceToDistributeB );
			freeC -= value;
			ressourceToDistributeB -= value;
		}
		if ( ressourceToDistributeC > 0 )
		{
			ressourceToDistributeC -= min( freeC, ressourceToDistributeC );
		}
	}

	//step three:
	//the remaining amount of B and C have to be subtracted from the maximum allowed production of A
	maxAllowedProd -= ( ressourceToDistributeB + ressourceToDistributeC );

	if ( maxAllowedProd < 0 ) maxAllowedProd = 0;

	return maxAllowedProd;
}

bool sSubBase::increaseEnergyProd( int i )
{

	//TODO: the energy production and fuel consumption of generators and stations are hardcoded in this function
	cList<cBuilding*> onlineStations;
	cList<cBuilding*> onlineGenerators;
	cList<cBuilding*> offlineStations;
	cList<cBuilding*> offlineGenerators;
	int availableStations = 0;
	int availableGenerators = 0;

	//generate lists with energy producers
	for ( unsigned int n = 0; n < buildings.Size(); n++ )
	{
		cBuilding* b = buildings[n];

		if ( !b->data.produceEnergy ) continue;

		if ( b->data.produceEnergy == 1 )
		{
			availableGenerators++;

			if ( b->IsWorking )
				onlineGenerators.Add( b );
			else
				offlineGenerators.Add( b );
		}
		else
		{
			availableStations++;

			if ( b->IsWorking )
				onlineStations.Add( b );
			else
				offlineStations.Add( b );
		}

	}

	//calc the optimum amount of energy stations and generators
	int energy = EnergyProd + i;

	int stations   = min( (energy + 3) / 6, availableStations );
	int generators = max(  energy - stations * 6, 0 );

	if ( generators > availableGenerators )
	{
		stations++;
		generators = 0;
	}

	if ( stations > availableStations )
	{
		return false;	//not enough free energy production capacity
	}

	//check available fuel
	int neededFuel = stations * 6 + generators * 2;
	if ( neededFuel > Oil + getMaxOilProd() )
	{
		//not possible to produce enough fuel
		sendChatMessageToClient("Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
		return false;
	}

	//stop unneeded buildings
	for ( int i = (int)onlineStations.Size() - stations; i > 0; i-- )
	{
		onlineStations[0]->ServerStopWork(true);
		onlineStations.Delete(0);
	}
	for ( int i = (int)onlineGenerators.Size() - generators; i > 0; i-- )
	{
		onlineGenerators[0]->ServerStopWork(true);
		onlineGenerators.Delete(0);
	}

	//start needed buildings
	for ( int i = stations - (int)onlineStations.Size(); i > 0; i-- )
	{
		offlineStations[0]->ServerStartWork();
		offlineStations.Delete(0);
	}
	for ( int i = generators - (int)onlineGenerators.Size(); i > 0; i-- )
	{
		offlineGenerators[0]->ServerStartWork();
		offlineGenerators.Delete(0);
	}

	return true;
}


void sSubBase::addMetal( int i )
{
	addRessouce( sUnitData::STORE_RES_METAL, i );
}

void sSubBase::addOil( int i )
{
	addRessouce( sUnitData::STORE_RES_OIL, i );
}

void sSubBase::addGold( int i )
{
	addRessouce( sUnitData::STORE_RES_GOLD, i );
}

void sSubBase::addRessouce( sUnitData::eStorageResType storeResType, int value )
{
	cBuilding *b;
	int* storedRessources = NULL;
	switch ( storeResType )
	{
	case sUnitData::STORE_RES_METAL:
		storedRessources = &Metal;
		break;
	case sUnitData::STORE_RES_OIL:
		storedRessources = &Oil;
		break;
	case sUnitData::STORE_RES_GOLD:
		storedRessources = &Gold;
		break;
	}


	if ( *storedRessources + value < 0 ) value -= *storedRessources + value;
	if ( !value ) return;

	*storedRessources += value;

	for (unsigned int i = 0; i < buildings.Size(); i++)
	{
		b = buildings[i];
		if ( b->data.storeResType != storeResType ) continue;
		int iStartValue = value;
		if ( value < 0 )
		{
			if ( b->data.storageResCur > -value )
			{
				b->data.storageResCur += value;
				value = 0;
			}
			else
			{
				value += b->data.storageResCur;
				b->data.storageResCur = 0;
			}
		}
		else
		{
			if ( b->data.storageResMax - b->data.storageResCur > value )
			{
				b->data.storageResCur += value;
				value = 0;
			}
			else
			{
				value -= b->data.storageResMax - b->data.storageResCur;
				b->data.storageResCur = b->data.storageResMax;
			}
		}
		if ( iStartValue != value ) sendUnitData ( b, owner->Nr );
		if ( value == 0 ) break;
	}
	sendSubbaseValues ( this, owner->Nr );
}

void sSubBase::refresh()
{
	//copy buildings list
	cList<cBuilding*> buildingsCopy;
	for ( unsigned int i = 0; i < buildings.Size(); i++ )
	{
		buildingsCopy.Add( buildings[i] );
	}

	//reset subbase
	while ( buildings.Size() ) buildings.Delete( buildings.Size() - 1 );
	MaxMetal = 0;
	Metal = 0;
	MaxOil = 0;
	Oil = 0;
	MaxGold = 0;
	Gold = 0;
	MaxEnergyProd = 0;
	EnergyProd = 0;
	MaxEnergyNeed = 0;
	EnergyNeed = 0;
	MetalNeed = 0;
	OilNeed = 0;
	GoldNeed = 0;
	MaxMetalNeed = 0;
	MaxOilNeed = 0;
	MaxGoldNeed = 0;
	MetalProd = 0;
	OilProd = 0;
	GoldProd = 0;
	HumanProd = 0;
	HumanNeed = 0;
	MaxHumanNeed = 0;

	//readd all buildings
	for ( unsigned int i = 0; i < buildingsCopy.Size(); i++ )
	{
		addBuilding( buildingsCopy[i] );
	}

}

bool sSubBase::checkHumanConsumer()
{
	if ( HumanNeed > HumanProd )
	{
		for ( unsigned int i = 0; i < buildings.Size(); i++ )
		{
			cBuilding& building = *buildings[i];
			if ( !building.data.needsHumans || !building.IsWorking ) continue;

			building.ServerStopWork(false);

			if ( HumanNeed <= HumanProd ) break;
		}

		return true;
	}

	return false;
};

bool sSubBase::checkGoldConsumer()
{
	if ( GoldNeed > GoldProd + Gold )
	{
		for ( unsigned int i = 0; i < buildings.Size(); i++ )
		{
			cBuilding& building = *buildings[i];
			if ( !building.data.convertsGold || !building.IsWorking ) continue;

			building.ServerStopWork(false);

			if ( GoldNeed <= GoldProd + Gold ) break;
		}

		return true;
	}

	return false;
};

bool sSubBase::checkMetalConsumer()
{
	if ( MetalNeed > MetalProd + Metal )
	{
		for ( unsigned int i = 0; i < buildings.Size(); i++ )
		{
			cBuilding& building = *buildings[i];
			if ( !building.data.needsMetal || !building.IsWorking ) continue;

			building.ServerStopWork(false);

			if ( MetalNeed <= MetalProd + Metal ) break;
		}

		return true;
	}

	return false;
};

bool sSubBase::checkOil()
{
	//TODO: the energy production and fuel consumption of generators and stations are hardcoded in this function
	cList<cBuilding*> onlineStations;
	cList<cBuilding*> onlineGenerators;
	cList<cBuilding*> offlineStations;
	cList<cBuilding*> offlineGenerators;
	int availableStations = 0;
	int availableGenerators = 0;
	bool oilMissing = false;

	//generate lists with energy producers
	for ( unsigned int n = 0; n < buildings.Size(); n++ )
	{
		cBuilding* b = buildings[n];

		if ( !b->data.produceEnergy ) continue;

		if ( b->data.produceEnergy == 1 )
		{
			availableGenerators++;

			if ( b->IsWorking )
				onlineGenerators.Add( b );
			else
				offlineGenerators.Add( b );
		}
		else
		{
			availableStations++;

			if ( b->IsWorking )
				onlineStations.Add( b );
			else
				offlineStations.Add( b );
		}

	}

	//calc the optimum amount of energy stations and generators
	int stations   = min( (EnergyNeed + 3) / 6, availableStations );
	int generators = max(  EnergyNeed - stations * 6, 0 );

	if ( generators > availableGenerators )
	{
		if ( stations < availableStations )
		{
			stations++;
			generators = 0;
		}
		else
		{
			generators = availableGenerators;
		}
	}

	//check needed oil
	int neededOil = stations * 6 + generators * 2;
	int availableOil = getMaxOilProd() + Oil;
	if ( neededOil > availableOil )
	{
		//reduce energy production to maximum possible value
		stations = min( (availableOil)/6, availableStations );
		generators = min( (availableOil - ( stations * 6 )) / 2, availableGenerators );

		oilMissing = true;
	}

	//increase oil production, if nessesary
	neededOil = stations * 6 + generators * 2;
	if ( neededOil > OilProd + Oil )
	{
		//temporary decrease gold and metal production
		int missingOil = neededOil - OilProd - Oil;
		int gold = GoldProd;
		int metal = MetalProd;
		setMetalProd(0);
		setGoldProd(0);

		changeOilProd( missingOil );

		setGoldProd(gold);
		setMetalProd(metal);

		sendChatMessageToClient ( "Text~Comp~Adjustments_Fuel_Increased", SERVER_INFO_MESSAGE, owner->Nr, iToStr(missingOil) );
		if ( getMetalProd() < metal )
			sendChatMessageToClient ( "Text~Comp~Adjustments_Metal_Decreased", SERVER_INFO_MESSAGE, owner->Nr, iToStr(metal - MetalProd) );
		if ( getGoldProd() < gold )
			sendChatMessageToClient ( "Text~Comp~Adjustments_Gold_Decreased", SERVER_INFO_MESSAGE, owner->Nr, iToStr(gold - GoldProd) );
	}

	//stop unneeded buildings
	for ( int i = (int)onlineStations.Size() - stations; i > 0; i-- )
	{
		onlineStations[0]->ServerStopWork(true);
		onlineStations.Delete(0);
	}
	for ( int i = (int)onlineGenerators.Size() - generators; i > 0; i-- )
	{
		onlineGenerators[0]->ServerStopWork(true);
		onlineGenerators.Delete(0);
	}

	//start needed buildings
	for ( int i = stations - (int)onlineStations.Size(); i > 0; i-- )
	{
		offlineStations[0]->ServerStartWork();
		offlineStations.Delete(0);
	}
	for ( int i = generators - (int)onlineGenerators.Size(); i > 0; i-- )
	{
		offlineGenerators[0]->ServerStartWork();
		offlineGenerators.Delete(0);
	}

	//temporary debug check
	if ( getGoldProd() < getMaxAllowedGoldProd() ||
		 getMetalProd() < getMaxAllowedMetalProd() ||
		 getOilProd() < getMaxAllowedOilProd() )
	{
		Log.write(" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}

	return oilMissing;
};

bool sSubBase::checkEnergy()
{
	if ( EnergyNeed > EnergyProd )
	{
		for ( unsigned int i = 0; i < buildings.Size(); i++ )
		{
			cBuilding& building = *buildings[i];
			if ( !building.data.needsEnergy || !building.IsWorking ) continue;

			//do not shut down ressource producers in the first run
			if ( building.MaxOilProd > 0 ||
				 building.MaxGoldProd > 0 ||
				 building.MaxMetalProd > 0 )	continue;

			building.ServerStopWork(false);

			if ( EnergyNeed <= EnergyProd )	return true;

		}

		for ( unsigned int i = 0; i < buildings.Size(); i++ )
		{
			cBuilding& building = *buildings[i];
			if ( !building.data.needsEnergy || !building.IsWorking ) continue;

			//do not shut down oil producers in the second run
			if ( building.MaxOilProd > 0 ) continue;

			building.ServerStopWork(false);

			if ( EnergyNeed <= EnergyProd )	return true;

		}

		//if energy is still missing, shut down also oil producers
		for ( unsigned int i = 0; i < buildings.Size(); i++ )
		{
			cBuilding& building = *buildings[i];
			if ( !building.data.needsEnergy || !building.IsWorking ) continue;

			building.ServerStopWork(false);

			if ( EnergyNeed <= EnergyProd ) return true;
		}
		return true;
	}

	return false;
}

void sSubBase::prepareTurnend()
{

	if ( checkMetalConsumer() )
		sendChatMessageToClient("Text~Comp~Metal_Low", SERVER_INFO_MESSAGE, owner->Nr );

	if ( checkHumanConsumer() )
		sendChatMessageToClient("Text~Comp~Team_Low", SERVER_INFO_MESSAGE, owner->Nr );

	if ( checkGoldConsumer() )
		sendChatMessageToClient("Text~Comp~Gold_Low", SERVER_INFO_MESSAGE, owner->Nr );

	//there is a loop around checkOil/checkEnergy, because a lack of energy can lead to less fuel,
	//that can lead to less energy, etc...
	bool oilMissing = false;
	bool energyMissing = false;
	bool changed = true;
	while ( changed )
	{
		changed = false;
		if ( checkOil() )
		{
			changed = true;
			oilMissing = true;
		}

		if ( checkEnergy() )
		{
			changed = true;
			energyMissing = true;
		}
	}
	if ( oilMissing    ) sendChatMessageToClient("Text~Comp~Fuel_Low", SERVER_INFO_MESSAGE, owner->Nr );
	if ( energyMissing ) sendChatMessageToClient("Text~Comp~Energy_Low", SERVER_INFO_MESSAGE, owner->Nr );

	//recheck metal and gold, because metal and gold producers could have been shut down, due to a lack of energy
	if ( checkMetalConsumer() )
		sendChatMessageToClient("Text~Comp~Metal_Low", SERVER_INFO_MESSAGE, owner->Nr );

	if ( checkGoldConsumer() )
		sendChatMessageToClient("Text~Comp~Gold_Low", SERVER_INFO_MESSAGE, owner->Nr );
}

void sSubBase::makeTurnend()
{
	prepareTurnend();

	//produce ressources
	addOil ( OilProd - OilNeed );
	addMetal (MetalProd - MetalNeed );
	addGold ( GoldProd - GoldNeed );

	//produce credits
	if ( GoldNeed )
	{
		owner->Credits += GoldNeed;
		sendCredits (owner->Credits, owner->Nr);
	}

	// make repairs/build/reload
	for (unsigned int k = 0; k < buildings.Size(); k++)
	{
		cBuilding *Building = buildings[k];

		// repair (do not repair buildings that have been attacked in this turn):
		if ( Building->data.hitpointsCur < Building->data.hitpointsMax && Metal > 0 && !Building->hasBeenAttacked )
		{
			// calc new hitpoints
			Building->data.hitpointsCur += Round ( ((float)Building->data.hitpointsMax/Building->data.buildCosts)*4 );
			if ( Building->data.hitpointsCur > Building->data.hitpointsMax ) Building->data.hitpointsCur = Building->data.hitpointsMax;
			addMetal ( -1 );
			sendUnitData ( Building, owner->Nr );
			for ( unsigned int j = 0; j < Building->SeenByPlayerList.Size(); j++ )
			{
				sendUnitData ( Building, Building->SeenByPlayerList[j]->Nr );
			}
		}
		if ( Building->hasBeenAttacked ) Building->hasBeenAttacked = false;

		// reload:
		if ( Building->data.canAttack && Building->data.ammoCur == 0 && Metal > 0 )
		{
			Building->data.ammoCur = Building->data.ammoMax;
			addMetal ( -1 );
			//ammo is not visible to enemies. So only send to the owner
			sendUnitData ( Building, owner->Nr );
		}

		// build:
		if (Building->IsWorking && !Building->data.canBuild.empty() && Building->BuildList->Size() )
		{
			sBuildList *BuildListItem;
			BuildListItem = (*Building->BuildList)[0];
			if ( BuildListItem->metall_remaining > 0 )
			{
				//in this block the metal consumption of the factory in the next round can change
				//so we first substract the old value from MetalNeed and then add the new one, to hold the base up to date
				MetalNeed -= min( Building->MetalPerRound, BuildListItem->metall_remaining);

				BuildListItem->metall_remaining -= min( Building->MetalPerRound, BuildListItem->metall_remaining);
				if ( BuildListItem->metall_remaining < 0 ) BuildListItem->metall_remaining = 0;

				MetalNeed += min( Building->MetalPerRound, BuildListItem->metall_remaining );
				sendBuildList ( Building );
				sendSubbaseValues ( this, owner->Nr );
			}
			if ( BuildListItem->metall_remaining <= 0 )
			{
				Server->addReport ( BuildListItem->typ->data.ID, true, owner->Nr );
				Building->ServerStopWork ( false );
			}
		}
	}

	//check maximum storage limits
	if ( Metal > MaxMetal ) Metal = MaxMetal;
	if ( Oil   > MaxOil   ) Oil   = MaxOil;
	if ( Gold  > MaxGold  ) Gold  = MaxGold;

	//should not happen, but to be sure:
	if ( Metal < 0 ) Metal = 0;
	if ( Oil   < 0 ) Oil   = 0;
	if ( Gold  < 0 ) Gold  = 0;

	sendSubbaseValues( this, owner->Nr );
}

void sSubBase::merge(sSubBase* sb )
{
	//merge ressource allocation
	int metal = MetalProd;
	int oil = OilProd;
	int gold = GoldProd;

	metal += sb->getMetalProd();
	gold  += sb->getGoldProd();
	oil   += sb->getOilProd();

	//merge buildings
	while( sb->buildings.Size() )
	{
		cBuilding* building = sb->buildings[0];
		addBuilding( building );
		building->SubBase = this;
		sb->buildings.Delete ( 0 );
	}

	//set ressource allocation
	setMetalProd(0);
	setOilProd(0);
	setGoldProd(0);

	setMetalProd(metal);
	setGoldProd(gold);
	setOilProd(oil);

	// delete the subbase from the subbase list
	cList<sSubBase*>& SubBases = owner->base.SubBases;
	for (unsigned int i = 0; i < SubBases.Size(); i++)
	{
		if (SubBases[i] == sb)
		{
			SubBases.Delete( i );
			break;
		}
	}
}

int sSubBase::getID()
{
	assert( buildings.Size() );
	
	return buildings[0]->iID;
}

void sSubBase::addBuilding( cBuilding *b )
{
	buildings.Add ( b );
	// calculate storage level
	switch ( b->data.storeResType )
	{
	case sUnitData::STORE_RES_METAL:
		MaxMetal += b->data.storageResMax;
		Metal += b->data.storageResCur;
		break;
	case sUnitData::STORE_RES_OIL:
		MaxOil += b->data.storageResMax;
		Oil += b->data.storageResCur;
		break;
	case sUnitData::STORE_RES_GOLD:
		MaxGold += b->data.storageResMax;
		Gold += b->data.storageResCur;
		break;
	}
	// calculate energy
	if ( b->data.produceEnergy )
	{
		MaxEnergyProd += b->data.produceEnergy;
		MaxOilNeed += b->data.needsOil;
		if ( b->IsWorking )
		{
			EnergyProd += b->data.produceEnergy;
			OilNeed += b->data.needsOil;
		}
	}
	else if ( b->data.needsEnergy )
	{
		MaxEnergyNeed += b->data.needsEnergy;
		if ( b->IsWorking )
		{
			EnergyNeed += b->data.needsEnergy;
		}
	}
	// calculate ressource consumption
	if ( b->data.needsMetal )
	{
		MaxMetalNeed += b->data.needsMetal*12;
		if ( b->IsWorking )
		{
			MetalNeed += min(b->MetalPerRound, (*b->BuildList)[0]->metall_remaining);
		}
	}
	// calculate gold
	if ( b->data.convertsGold )
	{
		MaxGoldNeed += b->data.convertsGold;
		if ( b->IsWorking )
		{
			GoldNeed += b->data.convertsGold;
		}
	}
	// calculate ressource production
	if ( b->data.canMineMaxRes > 0 && b->IsWorking )
	{
		int mineFree = b->data.canMineMaxRes;
		changeMetalProd( b->MaxMetalProd );
		mineFree -= b->MaxMetalProd;

		changeOilProd( min ( b->MaxOilProd, mineFree));
		mineFree -= min ( b->MaxOilProd, mineFree);

		changeGoldProd( min ( b->MaxGoldProd, mineFree));
	}
	// calculate humans
	if ( b->data.produceHumans )
	{
		HumanProd += b->data.produceHumans;
	}
	if ( b->data.needsHumans )
	{
		MaxHumanNeed += b->data.needsHumans;
		if ( b->IsWorking )
		{
			HumanNeed += b->data.needsHumans;
		}
	}

	//temporary debug check
	if ( getGoldProd() < getMaxAllowedGoldProd() ||
		 getMetalProd() < getMaxAllowedMetalProd() ||
		 getOilProd() < getMaxAllowedOilProd() )
	{
		Log.write(" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}
}


cBase::cBase(): map()
{};

cBase::~cBase ( void )
{
	while (SubBases.Size() != 0)
	{
		sSubBase* const sb = SubBases[0];
		delete sb;
		SubBases.Delete(0);
	}
}

sSubBase *cBase::checkNeighbour ( int iOff, cBuilding *Building )
{
	if( iOff < 0 || iOff >= map->size*map->size ) return NULL;
	cBuilding* b = map->fields[iOff].getBuildings();

	if ( b && b->owner == Building->owner && b->SubBase )
	{
		b->CheckNeighbours( map );
		return b->SubBase ;
	}
	else return NULL;
}

void cBase::addBuilding ( cBuilding *building, bool bServer )
{
	int pos;
	if ( !building->data.connectsToBase ) return;
	pos = building->PosX + building->PosY * map->size;
	cList<sSubBase*> NeighbourList;

	//check for neighbours
	if ( !building->data.isBig )
	{
		// small building
		sSubBase *SubBase;
		if ( (SubBase = checkNeighbour( pos-map->size, building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos+1        , building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos+map->size, building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos-1        , building )) ) NeighbourList.Add( SubBase );
	}
	else
	{
		// big building
		sSubBase *SubBase;
		if ( (SubBase = checkNeighbour( pos-map->size,     building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos-map->size+1,   building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos+2,             building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos+2+map->size,   building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos+map->size*2,   building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos+map->size*2+1, building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos-1,             building )) ) NeighbourList.Add( SubBase );
		if ( (SubBase = checkNeighbour( pos-1+map->size,   building )) ) NeighbourList.Add( SubBase );
	}
	building->CheckNeighbours( map );

	NeighbourList.RemoveDuplicates();

	if (NeighbourList.Size() == 0)
	{
		// no neigbours found, just generate new subbase and add the building
		sSubBase *NewSubBase;
		NewSubBase = new sSubBase ( building->owner );
		building->SubBase = NewSubBase;
		NewSubBase->addBuilding( building );
		SubBases.Add( NewSubBase );
			
		if ( bServer ) sendSubbaseValues ( NewSubBase, NewSubBase->owner->Nr );

		return;
	}

	// found neighbours, so add the building to the first neighbour subbase
	sSubBase* const firstNeighbour = NeighbourList[0];
	firstNeighbour->addBuilding( building );
	building->SubBase = firstNeighbour;
	NeighbourList.Delete(0);

	// now merge the other neigbours to the first one, if nessesary
	while (NeighbourList.Size() > 0)
	{
		sSubBase* const SubBase = NeighbourList[0];
		firstNeighbour->merge( SubBase );
		
		delete SubBase;
		NeighbourList.Delete(0);
	}

	if ( bServer ) sendSubbaseValues ( firstNeighbour, building->owner->Nr );
}

void cBase::deleteBuilding ( cBuilding *building, bool bServer )
{
	if ( !building->data.connectsToBase ) return;
	sSubBase *sb = building->SubBase;

	// remove the current subbase
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		sb->buildings[i]->SubBase = NULL;
	}
	for (unsigned int i = 0; i < SubBases.Size(); ++i)
	{
		if (SubBases[i] == sb)
		{
			SubBases.Delete(i);
			break;
		}
	}

	//save ressource allocation
	int metal = sb->getMetalProd();
	int gold = sb->getGoldProd();
	int oil = sb->getOilProd();

	// add all the buildings again
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		cBuilding *n = sb->buildings[i];
		if ( n == building ) continue;
		addBuilding ( n, false );
	}

	//generate list, with the new subbases
	cList<sSubBase*> newSubBases;
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		cBuilding *n = sb->buildings[i];
		if ( n == building ) continue;
		newSubBases.Add(n->SubBase);
	}
	newSubBases.RemoveDuplicates();

	//try to restore ressource allocation
	for ( unsigned int i = 0; i < newSubBases.Size(); i++ )
	{
		newSubBases[i]->setMetalProd(metal);
		newSubBases[i]->setGoldProd(gold);
		newSubBases[i]->setOilProd(oil);
		
		metal -= newSubBases[i]->getMetalProd();
		gold -= newSubBases[i]->getGoldProd();
		oil -= newSubBases[i]->getOilProd();
	}
	
	if ( building->IsWorking && building->data.canResearch )
		building->owner->stopAResearch(building->researchArea);

	if ( bServer )
	{
		//send subbase values to client
		for ( unsigned int i = 0; i < newSubBases.Size(); i++ )
		{
			sendSubbaseValues( newSubBases[i], building->owner->Nr);
		}
	}

	delete sb;
}



void cBase::handleTurnend ()
{

	for (unsigned int i = 0; i < SubBases.Size(); ++i)
	{
		SubBases[i]->makeTurnend();
	}
}

// recalculates all subbase values (after a load)
void cBase::refreshSubbases ( void )
{
	for ( unsigned int i = 0; i < SubBases.Size(); i++ )
	{
		SubBases[i]->refresh();
	}
}
