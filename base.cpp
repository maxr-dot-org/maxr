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
#include "base.h"
#include "map.h"
#include "serverevents.h"
#include "server.h"


sSubBase::sSubBase( int iNextID ) :
	buildings(),
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
	MetalProd(),
	OilProd(),
	GoldProd(),
	HumanProd(),
	HumanNeed(),
	MaxHumanNeed(),
	iID( iNextID )
{}

sSubBase::sSubBase( const sSubBase& sb ) :
	buildings(),
	MaxMetal( sb.MaxMetal ),
	Metal( sb.Metal ),
	MaxOil( sb.MaxOil ),
	Oil( sb.Oil ),
	MaxGold( sb.MaxGold ),
	Gold( sb.Gold ),
	MaxEnergyProd( sb.MaxEnergyProd ),
	EnergyProd( sb.EnergyProd ),
	MaxEnergyNeed( sb.MaxEnergyNeed ),
	EnergyNeed( sb.EnergyNeed ),
	MetalNeed( sb.MetalNeed ),
	OilNeed( sb.OilNeed ),
	GoldNeed( sb.GoldNeed ),
	MaxMetalNeed( sb.MaxMetalNeed ),
	MaxOilNeed( sb.MaxOilNeed ),
	MaxGoldNeed( sb.MaxGoldNeed ),
	MetalProd( sb.MetalProd ),
	OilProd( sb.OilProd ),
	GoldProd( sb.GoldProd ),
	HumanProd( sb.HumanProd ),
	HumanNeed( sb.HumanNeed ),
	MaxHumanNeed( sb.MaxHumanNeed ),
	iID( sb.iID )
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

sSubBase::~sSubBase()
{}

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
	//TODO: the energy producion and fuel consumption of generators and stations are hardcoded here.
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
		sendChatMessageToClient("Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, buildings[0]->owner->Nr );
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

// Funktionen der Base Klasse ////////////////////////////////////////////////
cBase::cBase ( cPlayer *Owner )
{
	owner=Owner;
	iNextSubBaseID = 0;
}

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

// Fügt ein neues Building in die Base ein:
void cBase::AddBuilding ( cBuilding *Building )
{
	int pos;
	if ( !Building->data.connectsToBase ) return;
	pos = Building->PosX+Building->PosY*map->size;
	cList<sSubBase*> NeighbourList;
	Building->SubBase = ( sSubBase* ) 1;
	// Prüfen, ob ein Gebäude in in der Nähe steht:
	if ( !Building->data.isBig )
	{
		// small building
		sSubBase *SubBase;
		if ( ( SubBase = checkNeighbour ( pos-map->size, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos+1, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos+map->size, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos-1, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
	}
	else
	{
		// big building
		sSubBase *SubBase;
		if ( ( SubBase = checkNeighbour ( pos-map->size, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos-map->size+1, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos+2, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos+2+map->size, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos+map->size*2, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos+map->size*2+1, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos-1, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
		if ( ( SubBase = checkNeighbour ( pos-1+map->size, Building ) ) != NULL ) NeighbourList.Add ( SubBase );
	}
	if (NeighbourList.Size() > 0)
	{
		// found neighbours
		Building->CheckNeighbours( map );
		// remove duplicate entrys
		for (unsigned int i = 0; i < NeighbourList.Size(); i++)
		{
			for (unsigned int k = i + 1; k < NeighbourList.Size(); k++)
			{
				if (NeighbourList[i] == NeighbourList[k])
				{
					NeighbourList.Delete(i);
					i--;
					break;
				}
			}
		}
		// check whether there have to merge subbases
		if (NeighbourList.Size() > 1)
		{
			// merge the subbases to one
			sSubBase *NewSubBase;
			cBuilding *SubBaseBuilding;
			// generate new subbase
			NewSubBase = new sSubBase( iNextSubBaseID );
			iNextSubBaseID++;
			Building->SubBase = NewSubBase;
			AddBuildingToSubBase ( Building, NewSubBase );
			SubBases.Add( NewSubBase );

			//store the mine produktion values, to restore them after merging the subbases
			int metalProd = NewSubBase->getMetalProd();
			int oilProd = NewSubBase->getOilProd();
			int goldProd = NewSubBase->getGoldProd();


			sendNewSubbase ( NewSubBase, Building->owner->Nr );
			// go through all found subbases
			while (NeighbourList.Size())
			{
				sSubBase* const SubBase = NeighbourList[0];
				// go through all buildings of the subbases
				while (SubBase->buildings.Size())
				{
					SubBaseBuilding = SubBase->buildings[0];
					AddBuildingToSubBase ( SubBaseBuilding, NewSubBase );
					SubBaseBuilding->SubBase = NewSubBase;
					SubBase->buildings.Delete ( 0 );
				}

				//store the mine produktion values, to restore them after merging the subbases
				metalProd += SubBase->getMetalProd();
				oilProd   += SubBase->getOilProd();
				goldProd  += SubBase->getGoldProd();

				// delete the subbase from the subbase list
				for (unsigned int i = 0; i < SubBases.Size(); i++)
				{
					if (SubBases[i] == SubBase)
					{
						SubBases.Delete( i );
						break;
					}
				}
				sendDeleteSubbase ( SubBase, Building->owner->Nr );
				delete SubBase;
				NeighbourList.Delete( 0 );
			}

			//restore mine production values
			NewSubBase->setMetalProd(0);
			NewSubBase->setOilProd(0);
			NewSubBase->setGoldProd(0);
			NewSubBase->setMetalProd( metalProd );
			NewSubBase->setOilProd( oilProd );
			NewSubBase->setGoldProd( goldProd );

			sendAddSubbaseBuildings ( NULL, NewSubBase, Building->owner->Nr );
			sendSubbaseValues ( NewSubBase, Building->owner->Nr );
		}
		else
		{
			// just add the building to the subbase
			sSubBase* const SubBase = NeighbourList[0];
			AddBuildingToSubBase ( Building, SubBase );
			Building->SubBase = SubBase;
			sendAddSubbaseBuildings ( Building, SubBase, Building->owner->Nr );
			sendSubbaseValues ( SubBase, Building->owner->Nr );
		}
	}
	else
	{
		sSubBase *NewSubBase;
		// no neigbours found
		Building->BaseBE = false;
		Building->BaseBN = false;
		Building->BaseBS = false;
		Building->BaseBW = false;
		Building->BaseE = false;
		Building->BaseN = false;
		Building->BaseS = false;
		Building->BaseW = false;
		// generate new subbase
		NewSubBase = new sSubBase ( iNextSubBaseID );
		iNextSubBaseID++;
		Building->SubBase = NewSubBase;
		AddBuildingToSubBase ( Building, NewSubBase );
		SubBases.Add( NewSubBase );
		sendNewSubbase ( NewSubBase, Building->owner->Nr );
		sendAddSubbaseBuildings ( NULL, NewSubBase, Building->owner->Nr );
		sendSubbaseValues ( NewSubBase, Building->owner->Nr );
	}
}

// Löscht ein Building aus der Base:
void cBase::DeleteBuilding ( cBuilding *b )
{
	sSubBase *sb;
	cBuilding *n;
	if ( !b->data.connectsToBase ) return;
	sb=b->SubBase;
	// Alle SubBases auf NULL setzen:
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
	// add all the buildings again
	//FIXME: with deleting the old subbase, the ressouce configuration is lost and will be set to a default distribution
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		n = sb->buildings[i];
		if ( n==b ) continue;
		AddBuilding ( n );		//TODO: this causes a lot of unnessesary net traffic, when deleting a building from a big subbase
	}
	if (b->IsWorking && b->data.canReasearch)
		b->owner->stopAResearch (b->researchArea);
	delete sb;
}

// Fügt ein Gebäude in eine Subbase ein:
void cBase::AddBuildingToSubBase ( cBuilding *b,sSubBase *sb )
{
	sb->buildings.Add ( b );
	// Ladung ausrechnen:
	switch ( b->data.storeResType )
	{
	case sUnitData::STORE_RES_METAL:
		sb->MaxMetal += b->data.storageResMax;
		sb->Metal += b->data.storageResCur;
		break;
	case sUnitData::STORE_RES_OIL:
		sb->MaxOil += b->data.storageResMax;
		sb->Oil += b->data.storageResCur;
		break;
	case sUnitData::STORE_RES_GOLD:
		sb->MaxGold += b->data.storageResMax;
		sb->Gold += b->data.storageResCur;
		break;
	}
	// Energiehaushalt ausrechnen:
	if ( b->data.produceEnergy )
	{
		sb->MaxEnergyProd+=b->data.produceEnergy;
		sb->MaxOilNeed+=b->data.needsOil;
		if ( b->IsWorking )
		{
			sb->EnergyProd+=b->data.produceEnergy;
			sb->OilNeed+=b->data.needsOil;
		}
	}
	else if ( b->data.needsEnergy )
	{
		sb->MaxEnergyNeed+=b->data.needsEnergy;
		if ( b->IsWorking )
		{
			sb->EnergyNeed+=b->data.needsEnergy;
		}
	}
	// Rohstoffhaushalt ausrechnen:
	if ( b->data.needsMetal )
	{
		sb->MaxMetalNeed+=b->data.needsMetal*12;
		if ( b->IsWorking )
		{
			sb->MetalNeed += min(b->MetalPerRound, (*b->BuildList)[0]->metall_remaining);
		}
	}
	// Goldhaushalt ausrechnen:
	if ( b->data.convertsGold )
	{
		sb->MaxGoldNeed+=b->data.convertsGold;
		if ( b->IsWorking )
		{
			sb->GoldNeed+=b->data.convertsGold;
		}
	}
	// Rohstoffproduktion ausrechnen:
	if ( b->data.canMineMaxRes > 0 && b->IsWorking )
	{
		int mineFree = b->data.canMineMaxRes;
		sb->changeMetalProd( b->MaxMetalProd );
		mineFree -= b->MaxMetalProd;

		sb->changeOilProd( min ( b->MaxOilProd, mineFree));
		mineFree -= min ( b->MaxOilProd, mineFree);

		sb->changeGoldProd( min ( b->MaxGoldProd, mineFree));
	}
	// Human-Haushalt ausrechnen:
	if ( b->data.produceHumans )
	{
		sb->HumanProd+=b->data.produceHumans;
	}
	if ( b->data.needsHumans )
	{
		sb->MaxHumanNeed+=b->data.needsHumans;
		if ( b->IsWorking )
		{
			sb->HumanNeed+=b->data.needsHumans;
		}
	}
}

// Fügt Metall zu der Subbase hinzu:
void cBase::AddMetal ( sSubBase *sb,int value )
{
	cBuilding *b;
	if ( sb->Metal+value>sb->MaxMetal ) value-= ( sb->Metal+value )-sb->MaxMetal;
	if ( sb->Metal+value<0 ) value-=sb->Metal+value;
	if ( !value ) return;
	sb->Metal+=value;
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		b = sb->buildings[i];
		if ( b->data.storeResType != sUnitData::STORE_RES_METAL ) continue;
		int iStartValue = value;
		if ( value<0 )
		{
			if ( b->data.storageResCur>-value )
			{
				b->data.storageResCur+=value;
				value=0;
			}
			else
			{
				value+=b->data.storageResCur;
				b->data.storageResCur=0;
			}
		}
		else
		{
			if ( b->data.storageResMax-b->data.storageResCur>value )
			{
				b->data.storageResCur+=value;
				value=0;
			}
			else
			{
				value-=b->data.storageResMax-b->data.storageResCur;
				b->data.storageResCur=b->data.storageResMax;
			}
		}
		if ( iStartValue != value ) sendUnitData ( b, owner->Nr );
		if ( value==0 ) break;
	}
	sendSubbaseValues ( sb, owner->Nr );
}

// Fügt Öl zu der Subbase hinzu:
void cBase::AddOil ( sSubBase *sb,int value )
{
	cBuilding *b;
	if ( sb->Oil+value>sb->MaxOil ) value-= ( sb->Oil+value )-sb->MaxOil;
	if ( sb->Oil+value<0 ) value-=sb->Oil+value;
	if ( !value ) return;
	sb->Oil+=value;
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		b = sb->buildings[i];
		if ( b->data.storeResType != sUnitData::STORE_RES_OIL ) continue;
		int iStartValue = value;
		if ( value<0 )
		{
			if ( b->data.storageResCur>-value )
			{
				b->data.storageResCur+=value;
				value=0;
			}
			else
			{
				value+=b->data.storageResCur;
				b->data.storageResCur=0;
			}
		}
		else
		{
			if ( b->data.storageResMax-b->data.storageResCur>value )
			{
				b->data.storageResCur+=value;
				value=0;
			}
			else
			{
				value-=b->data.storageResMax-b->data.storageResCur;
				b->data.storageResCur=b->data.storageResMax;
			}
		}
		if ( iStartValue != value ) sendUnitData ( b, owner->Nr );
		if ( value==0 ) break;
	}
	sendSubbaseValues ( sb, owner->Nr );
}

// Fügt Gold zu der Subbase hinzu:
void cBase::AddGold ( sSubBase *sb,int value )
{
	cBuilding *b;
	if ( sb->Gold+value>sb->MaxGold ) value-= ( sb->Gold+value )-sb->MaxGold;
	if ( sb->Gold+value<0 ) value-=sb->Gold+value;
	if ( !value ) return;
	sb->Gold+=value;
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		b = sb->buildings[i];
		if ( b->data.storeResType != sUnitData::STORE_RES_GOLD ) continue;
		int iStartValue = value;
		if ( value<0 )
		{
			if ( b->data.storageResCur>-value )
			{
				b->data.storageResCur+=value;
				value=0;
			}
			else
			{
				value+=b->data.storageResCur;
				b->data.storageResCur=0;
			}
		}
		else
		{
			if ( b->data.storageResMax-b->data.storageResCur>value )
			{
				b->data.storageResCur+=value;
				value=0;
			}
			else
			{
				value-=b->data.storageResMax-b->data.storageResCur;
				b->data.storageResCur=b->data.storageResMax;
			}
		}
		if ( iStartValue != value ) sendUnitData ( b, owner->Nr );
		if ( value==0 ) break;
	}
	sendSubbaseValues ( sb, owner->Nr );
}

void cBase::handleTurnend ()
{
	bool bSendCreditsUpdate = false;
	
	for (unsigned int i = 0; i < SubBases.Size(); ++i)
	{
		sSubBase* const SubBase = SubBases[i];
		// produce/reduce oil
		if ( SubBase->OilProd-SubBase->OilNeed < 0 && SubBase->Oil + ( SubBase->OilProd-SubBase->OilNeed ) < 0 )
		{
			// generator has to stop work
			sendChatMessageToClient ( "Text~Comp~Fuel_Low", SERVER_INFO_MESSAGE, owner->Nr );
			for (unsigned int k = 0; k < SubBase->buildings.Size() && SubBase->EnergyProd; k++)
			{
				cBuilding *Building;
				Building = SubBase->buildings[k];
				if ( !Building->data.produceEnergy ) continue;
				Building->ServerStopWork ( true );
				if ( SubBase->OilProd-SubBase->OilNeed < 0 && SubBase->Oil + ( SubBase->OilProd-SubBase->OilNeed ) < 0 ) continue;
				break;
			}
		}
		AddOil ( SubBase, SubBase->OilProd-SubBase->OilNeed );
		// check energy consumers
		if ( SubBase->EnergyNeed > SubBase->EnergyProd )
		{
			sendChatMessageToClient ( "Text~Comp~Energy_Low", SERVER_INFO_MESSAGE, owner->Nr );
			for (unsigned int k = 0; k < SubBase->buildings.Size(); k++)
			{
				cBuilding *Building;
				Building = SubBase->buildings[k];
				if ( !Building->data.needsEnergy ) continue;
				Building->ServerStopWork ( true );
				if ( SubBase->EnergyNeed>SubBase->EnergyProd ) continue;
				break;
			}
		}

		// produce/reduce metal
		if ( SubBase->Metal + ( SubBase->MetalProd-SubBase->MetalNeed ) <0 )
		{
			sendChatMessageToClient ( "Text~Comp~Metal_Low", SERVER_INFO_MESSAGE, owner->Nr );
			for (unsigned int k = 0; k < SubBase->buildings.Size(); k++)
			{
				cBuilding *Building;
				Building = SubBase->buildings[k];
				if ( !Building->data.needsMetal ) continue;
				Building->ServerStopWork ( true );
				if ( SubBase->Metal + ( SubBase->MetalProd-SubBase->MetalNeed ) < 0 ) continue;
				break;
			}
		}
		int overproducedMetal = SubBase->MetalProd-SubBase->MetalNeed+SubBase->Metal-SubBase->MaxMetal;
		AddMetal ( SubBase, SubBase->MetalProd-SubBase->MetalNeed );

		// produce/reduce gold
		if ( SubBase->Gold + ( SubBase->GoldProd-SubBase->GoldNeed ) < 0 )
		{
			sendChatMessageToClient ( "Text~Comp~Gold_Low", SERVER_INFO_MESSAGE, owner->Nr );
			for (unsigned int k = 0; k < SubBase->buildings.Size(); k++)
			{
				cBuilding *Building;
				Building = SubBase->buildings[k];
				if ( !Building->data.convertsGold ) continue;
				Building->ServerStopWork ( true );
				if ( SubBase->Gold + ( SubBase->GoldProd-SubBase->GoldNeed ) < 0 ) continue;
				break;
			}
		}
		AddGold ( SubBase, SubBase->GoldProd-SubBase->GoldNeed );
		
		// get credits
		if (SubBase->GoldNeed > 0)
		{
			owner->Credits += SubBase->GoldNeed;
			bSendCreditsUpdate = true;
		}
		
		// check humanneed
		if ( SubBase->HumanNeed > SubBase->HumanProd )
		{
			sendChatMessageToClient ( "Text~Comp~Team_Low", SERVER_INFO_MESSAGE, owner->Nr );
			for (unsigned int k = 0; k < SubBase->buildings.Size(); k++)
			{
				cBuilding *Building;
				Building = SubBase->buildings[k];
				if ( !Building->data.needsHumans ) continue;
				Building->ServerStopWork ( true );
				if ( SubBase->HumanNeed > SubBase->HumanProd ) continue;
				break;
			}
		}

		// Optimize energy
		/*if ( OptimizeEnergy ( sb ) )
		{
			//sendChatMessage (lngPack.i18n( "Text~Comp~Energy_Optimize"));
		}*/

		// make repairs/build/reload
		for (unsigned int k = 0; k < SubBase->buildings.Size(); k++)
		{
			cBuilding *Building = SubBase->buildings[k];
			// repair:
			if ( Building->data.hitpointsCur < Building->data.hitpointsMax && ( SubBase->Metal > 0 || overproducedMetal > 0 ) )
			{
				// do not repair buildings that have been attacked in this turn
				if ( !Building->hasBeenAttacked )
				{
					// calc new hitpoints
					Building->data.hitpointsCur += Round ( ((float)Building->data.hitpointsMax/Building->data.buildCosts)*4 );
					if ( Building->data.hitpointsCur > Building->data.hitpointsMax ) Building->data.hitpointsCur = Building->data.hitpointsMax;
					// first use overproduced metal to repair units, and use the stored metal afterwards
					if ( overproducedMetal > 0 ) overproducedMetal--;
					else AddMetal ( SubBase, -1 );
					sendUnitData ( Building, owner->Nr );
					for ( unsigned int j = 0; j < Building->SeenByPlayerList.Size(); j++ )
					{
						sendUnitData ( Building, Building->SeenByPlayerList[j]->Nr );
					}
				}
			}
			// reload:
			if ( Building->data.canAttack && Building->data.ammoCur == 0 && ( SubBase->Metal > 0 || overproducedMetal > 0 ) )
			{
				Building->data.ammoCur = Building->data.ammoMax;
				// first use overproduced metal to reload units
				if ( overproducedMetal > 0 ) overproducedMetal--;
				else AddMetal ( SubBase, -1 );
				sendUnitData ( Building, owner->Nr );
				for ( unsigned int j = 0; j < Building->SeenByPlayerList.Size(); j++ )
				{
					sendUnitData ( Building, Building->SeenByPlayerList[j]->Nr );
				}
			}
			if ( Building->hasBeenAttacked ) Building->hasBeenAttacked = false;

			// build:
			if (Building->IsWorking && !Building->data.canBuild.empty() && Building->BuildList->Size() && SubBase->Metal )
			{
				sBuildList *BuildListItem;
				BuildListItem = (*Building->BuildList)[0];
				if ( BuildListItem->metall_remaining > 0 )
				{
					//in this block the metal consumption of the factory in the next round can change
					//so we first substract the old value from MetalNeed and then add the new one, to hold the base up to date
					SubBase->MetalNeed -= min( Building->MetalPerRound, BuildListItem->metall_remaining);

					BuildListItem->metall_remaining -= min( Building->MetalPerRound, BuildListItem->metall_remaining);
					if ( BuildListItem->metall_remaining < 0 ) BuildListItem->metall_remaining = 0;

					SubBase->MetalNeed += min( Building->MetalPerRound, BuildListItem->metall_remaining );
					sendBuildList ( Building );
					sendSubbaseValues ( SubBase, Building->owner->Nr );
				}
				if ( BuildListItem->metall_remaining <= 0 )
				{
					Server->addReport ( BuildListItem->typ->data.ID, true, Building->owner->Nr );
					Building->ServerStopWork ( false );
				}
			}
		}
	}
	if (bSendCreditsUpdate)
		sendCredits (owner->Credits, owner->Nr);
}

// Optimiert den Energieverbrauch der Basis:
bool cBase::OptimizeEnergy ( sSubBase *sb )
{
	bool changed=false;

	if ( sb->EnergyProd==0 ) return false;

	cList<cBuilding*> eb;
	cList<cBuilding*> es;

	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		cBuilding *b;
		b = sb->buildings[i];
		if ( !b->data.produceEnergy ) continue;

		if (b->data.produceEnergy == 1) es.Add(b);
		else eb.Add(b);
	}

	if (!sb->EnergyNeed && es.Size() != 0)
	{
		while (es.Size())
		{
			//es[0]->StopWork(false);
			es.Delete(0);
			changed=true;
		}
	}
	if (!sb->EnergyNeed && eb.Size() != 0)
	{
		while (es.Size() != 0)
		{
			//eb[0]->StopWork(false);
			eb.Delete(0);
			changed=true;
		}
	}

	if (es.Size() != 0 && eb.Size() == 0)
	{
		while (sb->EnergyNeed < sb->EnergyProd && es.Size() != 0)
		{
			//es[0]->StopWork(false);
			es.Delete(0);
			changed=true;
		}
	}
	else if (eb.Size() != 0&& es.Size() == 0)
	{
		while (sb->EnergyProd >= sb->EnergyNeed + 6 && eb.Size() != 0)
		{
			//eb[0]->StopWork(false);
			eb.Delete(0);
			changed=true;
		}
	}
	else if (es.Size() != 0 && eb.Size() != 0)
	{
		int bneed,sneed,pre;

		pre=sb->EnergyProd;
		bneed=sb->EnergyNeed/6;
		sneed=sb->EnergyNeed%6;
		if (sneed >= 3 && bneed < (int)eb.Size())
		{
			bneed++;
			sneed=0;
		}

		if (sneed > (int)es.Size() && bneed < (int)eb.Size())
		{
			sneed=0;
			bneed++;
		}

		for ( unsigned int i = 0; i < eb.Size(); ++i)
		{
			if ( (int)i>=bneed ) break;
			//eb[i]->StartWork();
			eb.Delete(i);
			i--;
			bneed--;
		}
		for (unsigned  int i = 0; i < es.Size(); ++i)
		{
			if ( (int)i>=sneed&&sb->EnergyNeed<=sb->EnergyProd ) break;
			//es[i]->StartWork();
			es.Delete(i);
			i--;
			sneed--;
		}
		while (eb.Size())
		{
			//eb[0]->StopWork(false);
			eb.Delete(0);
		}
		while (es.Size() != 0)
		{
			//es[0]->StopWork(false);
			es.Delete(0);
		}

		changed=pre!=sb->EnergyProd;
	}

	return changed;
}

// Berechnet alle Subbases neu (für ein Load):
void cBase::RefreshSubbases ( void )
{
	cBuilding *n;

	cList<sSubBase*> OldSubBases;
	while (SubBases.Size() != 0)
	{
		sSubBase* const sb = SubBases[0];
		OldSubBases.Add(sb);
		SubBases.Delete(0);
	}

	while (OldSubBases.Size() != 0)
	{
		sSubBase* const sb = OldSubBases[0];

		// Alle SubBases auf NULL setzen:
		for (unsigned int i = 0; i < sb->buildings.Size(); i++)
		{
			sb->buildings[i]->SubBase=NULL;
		}
		// Alle Gebäude neu einsetzen:
		for (unsigned int i = 0; i < sb->buildings.Size(); i++)
		{
			n=sb->buildings[i];
			AddBuilding ( n );
		}

		delete sb;
		OldSubBases.Delete(0);
	}
}
