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


sSubBase::~sSubBase()
{}


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
	if( iOff < 0 && iOff <= map->size*map->size ) return NULL;
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
	if ( Building->data.is_base ) return;
	pos = Building->PosX+Building->PosY*map->size;
	cList<sSubBase*> NeighbourList;
	Building->SubBase = ( sSubBase* ) 1;
	// Prüfen, ob ein Gebäude in in der Nähe steht:
	if ( !Building->data.is_big )
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
	if ( b->data.is_road||b->data.is_platform||b->data.is_bridge||b->data.is_expl_mine ) return;
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
	// Alle Gebäude neu einsetzen:
	for (unsigned int i = 0; i < sb->buildings.Size(); i++)
	{
		n = sb->buildings[i];
		if ( n==b ) continue;
		AddBuilding ( n );
	}
	if ( b->IsWorking&&b->data.can_research ) b->owner->StopAReserach();
	delete sb;
}

// Fügt ein Gebäude in eine Subbase ein:
void cBase::AddBuildingToSubBase ( cBuilding *b,sSubBase *sb )
{
	sb->buildings.Add ( b );
	// Ladung ausrechnen:
	switch ( b->data.can_load )
	{
		case TRANS_NONE:
			break;
		case TRANS_METAL:
			sb->MaxMetal+=b->data.max_cargo;
			sb->Metal+=b->data.cargo;
			break;
		case TRANS_OIL:
			sb->MaxOil+=b->data.max_cargo;
			sb->Oil+=b->data.cargo;
			break;
		case TRANS_GOLD:
			sb->MaxGold+=b->data.max_cargo;
			sb->Gold+=b->data.cargo;
			break;
	}
	// Energiehaushalt ausrechnen:
	if ( b->data.energy_prod )
	{
		sb->MaxEnergyProd+=b->data.energy_prod;
		sb->MaxOilNeed+=b->data.oil_need;
		if ( b->IsWorking )
		{
			sb->EnergyProd+=b->data.energy_prod;
			sb->OilNeed+=b->data.oil_need;
		}
	}
	else if ( b->data.energy_need )
	{
		sb->MaxEnergyNeed+=b->data.energy_need;
		if ( b->IsWorking )
		{
			sb->EnergyNeed+=b->data.energy_need;
		}
	}
	// Rohstoffhaushalt ausrechnen:
	if ( b->data.metal_need )
	{
		sb->MaxMetalNeed+=b->data.metal_need*12;
		if ( b->IsWorking )
		{
			sb->MetalNeed += min(b->MetalPerRound, (*b->BuildList)[0]->metall_remaining);
		}
	}
	// Goldhaushalt ausrechnen:
	if ( b->data.gold_need )
	{
		sb->MaxGoldNeed+=b->data.gold_need;
		if ( b->IsWorking )
		{
			sb->GoldNeed+=b->data.gold_need;
		}
	}
	// Rohstoffproduktion ausrechnen:
	if ( b->data.is_mine&&b->IsWorking )
	{
		sb->MetalProd+=b->MetalProd;
		sb->OilProd+=b->OilProd;
		sb->GoldProd+=b->GoldProd;
	}
	// Human-Haushalt ausrechnen:
	if ( b->data.human_prod )
	{
		sb->HumanProd+=b->data.human_prod;
	}
	if ( b->data.human_need )
	{
		sb->MaxHumanNeed+=b->data.human_need;
		if ( b->IsWorking )
		{
			sb->HumanNeed+=b->data.human_need;
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
		if ( b->data.can_load!=TRANS_METAL ) continue;
		int iStartValue = value;
		if ( value<0 )
		{
			if ( b->data.cargo>-value )
			{
				b->data.cargo+=value;
				value=0;
			}
			else
			{
				value+=b->data.cargo;
				b->data.cargo=0;
			}
		}
		else
		{
			if ( b->data.max_cargo-b->data.cargo>value )
			{
				b->data.cargo+=value;
				value=0;
			}
			else
			{
				value-=b->data.max_cargo-b->data.cargo;
				b->data.cargo=b->data.max_cargo;
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
		if ( b->data.can_load!=TRANS_OIL ) continue;
		int iStartValue = value;
		if ( value<0 )
		{
			if ( b->data.cargo>-value )
			{
				b->data.cargo+=value;
				value=0;
			}
			else
			{
				value+=b->data.cargo;
				b->data.cargo=0;
			}
		}
		else
		{
			if ( b->data.max_cargo-b->data.cargo>value )
			{
				b->data.cargo+=value;
				value=0;
			}
			else
			{
				value-=b->data.max_cargo-b->data.cargo;
				b->data.cargo=b->data.max_cargo;
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
		if ( b->data.can_load!=TRANS_GOLD ) continue;
		int iStartValue = value;
		if ( value<0 )
		{
			if ( b->data.cargo>-value )
			{
				b->data.cargo+=value;
				value=0;
			}
			else
			{
				value+=b->data.cargo;
				b->data.cargo=0;
			}
		}
		else
		{
			if ( b->data.max_cargo-b->data.cargo>value )
			{
				b->data.cargo+=value;
				value=0;
			}
			else
			{
				value-=b->data.max_cargo-b->data.cargo;
				b->data.cargo=b->data.max_cargo;
			}
		}
		if ( iStartValue != value ) sendUnitData ( b, owner->Nr );
		if ( value==0 ) break;
	}
	sendSubbaseValues ( sb, owner->Nr );
}

void cBase::handleTurnend ()
{
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
				if ( !Building->data.energy_prod ) continue;
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
				if ( !Building->data.energy_need ) continue;
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
				if ( !Building->data.metal_need ) continue;
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
				if ( !Building->data.gold_need ) continue;
				Building->ServerStopWork ( true );
				if ( SubBase->Gold + ( SubBase->GoldProd-SubBase->GoldNeed ) < 0 ) continue;
				break;
			}
		}
		AddGold ( SubBase, SubBase->GoldProd-SubBase->GoldNeed );
		// get credits
		//owner->Credits += SubBase->GoldNeed;

		// check humanneed
		if ( SubBase->HumanNeed > SubBase->HumanProd )
		{
			sendChatMessageToClient ( "Text~Comp~Team_Low", SERVER_INFO_MESSAGE, owner->Nr );
			for (unsigned int k = 0; k < SubBase->buildings.Size(); k++)
			{
				cBuilding *Building;
				Building = SubBase->buildings[k];
				if ( !Building->data.human_need ) continue;
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
			if ( Building->data.hit_points < Building->data.max_hit_points && ( SubBase->Metal > 0 || overproducedMetal > 0 ) )
			{
				// do not repair buildings that have been attacked in this turn
				if ( !Building->hasBeenAttacked )
				{
					// calc new hitpoints
					Building->data.hit_points += Round ( ((float)Building->data.max_hit_points/Building->data.iBuilt_Costs)*4 );
					if ( Building->data.hit_points > Building->data.max_hit_points ) Building->data.hit_points = Building->data.max_hit_points;
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
			if ( Building->data.can_attack && Building->data.ammo == 0 && ( SubBase->Metal > 0 || overproducedMetal > 0 ) )
			{
				Building->data.ammo = Building->data.max_ammo;
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
			if (Building->IsWorking && Building->data.can_build && Building->BuildList->Size() && SubBase->Metal )
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
		if ( !b->data.energy_prod ) continue;

		if (b->data.energy_prod == 1) es.Add(b);
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
