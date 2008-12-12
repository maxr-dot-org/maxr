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
	if( iOff >= 0 && iOff < map->size*map->size && map->GO[iOff].top && map->GO[iOff].top->owner == Building->owner && map->GO[iOff].top->SubBase )
	{
		map->GO[iOff].top->CheckNeighbours( map );
		return map->GO[iOff].top->SubBase ;
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
	int i;
	if ( b->data.is_road||b->data.is_platform||b->data.is_bridge||b->data.is_expl_mine ) return;
	sb=b->SubBase;
	// Alle SubBases auf NULL setzen:
	for (i = 0; i < sb->buildings.Size(); i++)
	{
		sb->buildings[i]->SubBase = NULL;
	}
	for (i = 0; i < SubBases.Size(); ++i)
	{
		if (SubBases[i] == sb)
		{
			SubBases.Delete(i);
			break;
		}
	}
	// Alle Gebäude neu einsetzen:
	for (i = 0; i < sb->buildings.Size(); i++)
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
	int i;
	if ( sb->Metal+value>sb->MaxMetal ) value-= ( sb->Metal+value )-sb->MaxMetal;
	if ( sb->Metal+value<0 ) value-=sb->Metal+value;
	if ( !value ) return;
	sb->Metal+=value;
	for (i = 0; i < sb->buildings.Size(); i++)
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
	int i;
	if ( sb->Oil+value>sb->MaxOil ) value-= ( sb->Oil+value )-sb->MaxOil;
	if ( sb->Oil+value<0 ) value-=sb->Oil+value;
	if ( !value ) return;
	sb->Oil+=value;
	for (i = 0; i < sb->buildings.Size(); i++)
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
	int i;
	if ( sb->Gold+value>sb->MaxGold ) value-= ( sb->Gold+value )-sb->MaxGold;
	if ( sb->Gold+value<0 ) value-=sb->Gold+value;
	if ( !value ) return;
	sb->Gold+=value;
	for (i = 0; i < sb->buildings.Size(); i++)
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
			//sendChatMessage ( lngPack.i18n( "Text~Comp~Fuel_Low") );
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
			//sendChatMessage ( lngPack.i18n( "Text~Comp~Energy_Low") );
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
			//sendChatMessage ( lngPack.i18n( "Text~Comp~Metal_Low") );
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
		AddMetal ( SubBase, SubBase->MetalProd-SubBase->MetalNeed );

		// produce/reduce gold
		if ( SubBase->Gold + ( SubBase->GoldProd-SubBase->GoldNeed ) < 0 )
		{
			//sendChatMessage ( lngPack.i18n( "Text~Comp~Gold_Low") );
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
			//sendChatMessage ( lngPack.i18n( "Text~Comp~Team_Low") );
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
		for (unsigned int k = 0; k < SubBase->buildings.Size() && SubBase->Metal; k++)
		{
			cBuilding *Building = SubBase->buildings[k];
			// Reparatur:
			/*if ( b->data.hit_points<b->data.max_hit_points&&sb->Metal>0 )
			{
				if ( b->data.max_hit_points/10>2 )
				{
					b->data.hit_points+=b->data.max_hit_points/10;
				}
				else
				{
					b->data.hit_points+=2;
				}
				if ( b->data.hit_points>b->data.max_hit_points )
				{
					b->data.hit_points=b->data.max_hit_points;
				}
				AddMetal ( sb,-1 );
			}
			// Aufladen:
			if ( b->data.can_attack&&b->data.ammo==0&&sb->Metal>=2 )
			{
				b->data.ammo=b->data.max_ammo;
				AddMetal ( sb,-2 );
			}*/

			// Bauen:
			if (Building->IsWorking && Building->data.can_build && Building->BuildList->Size())
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
					Server->addReport ( BuildListItem->typ->nr, true, Building->owner->Nr );
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
	int i;

	if ( sb->EnergyProd==0 ) return false;

	cList<cBuilding*> eb;
	cList<cBuilding*> es;

	for (i = 0; i < sb->buildings.Size(); i++)
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
		int bneed,sneed,i,pre;

		pre=sb->EnergyProd;
		bneed=sb->EnergyNeed/6;
		sneed=sb->EnergyNeed%6;
		if (sneed >= 3 && bneed < eb.Size())
		{
			bneed++;
			sneed=0;
		}

		if (sneed > es.Size() && bneed < eb.Size())
		{
			sneed=0;
			bneed++;
		}

		for (i = 0; i < eb.Size(); ++i)
		{
			if ( i>=bneed ) break;
			//eb[i]->StartWork();
			eb.Delete(i);
			i--;
			bneed--;
		}
		for (i = 0; i < es.Size(); ++i)
		{
			if ( i>=sneed&&sb->EnergyNeed<=sb->EnergyProd ) break;
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
	int i;

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
		for (i = 0; i < sb->buildings.Size(); i++)
		{
			sb->buildings[i]->SubBase=NULL;
		}
		// Alle Gebäude neu einsetzen:
		for (i = 0; i < sb->buildings.Size(); i++)
		{
			n=sb->buildings[i];
			AddBuilding ( n );
		}

		delete sb;
		OldSubBases.Delete(0);
	}
}
