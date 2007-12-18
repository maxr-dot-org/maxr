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
#include "game.h"
#include "networkmessages.h"

// Funktionen der Base Klasse ////////////////////////////////////////////////
cBase::cBase ( cPlayer *Owner )
{
	owner=Owner;
	SubBases=new cList<sSubBase*>;
}

cBase::~cBase ( void )
{
	while ( SubBases->iCount )
	{
		sSubBase *sb;
		sb=SubBases->Items[0];
		delete sb->buildings;
		delete sb;
		SubBases->Delete ( 0 );
	}
	delete SubBases;
}

// Fügt ein neues Building in die Base ein:
void cBase::AddBuilding ( cBuilding *b )
{
	cList<sSubBase*> *NeighbourList;
	int pos;
	if ( b->data.is_base ) return;
	pos=b->PosX+b->PosY*map->size;
	NeighbourList=new cList<sSubBase*>;
	b->SubBase= ( sSubBase* ) 1;
	// Prüfen, ob ein Gebäude in in der Nähe steht:
#define CHECK_NEIGHBOUR(a) if(a>=0&&a<map->size*map->size&&map->GO[a].top&&map->GO[a].top->owner==b->owner&&map->GO[a].top->SubBase){NeighbourList->Add(map->GO[a].top->SubBase);map->GO[a].top->CheckNeighbours();}
	if ( !b->data.is_big )
	{
		// Kleines Gebäude:
		CHECK_NEIGHBOUR ( pos-map->size )
		CHECK_NEIGHBOUR ( pos+1 )
		CHECK_NEIGHBOUR ( pos+map->size )
		CHECK_NEIGHBOUR ( pos-1 )
	}
	else
	{
		// Großes Gebäude:
		CHECK_NEIGHBOUR ( pos-map->size )
		CHECK_NEIGHBOUR ( pos-map->size+1 )
		CHECK_NEIGHBOUR ( pos+2 )
		CHECK_NEIGHBOUR ( pos+2+map->size )
		CHECK_NEIGHBOUR ( pos+map->size*2 )
		CHECK_NEIGHBOUR ( pos+map->size*2+1 )
		CHECK_NEIGHBOUR ( pos-1 )
		CHECK_NEIGHBOUR ( pos-1+map->size )
	}
	if ( NeighbourList->iCount )
	{
		int i,k;
		// Nachbarn gefunden:
		b->CheckNeighbours();
		// Die doppelten Einträge löschen:
		for ( i=0;i<NeighbourList->iCount;i++ )
		{
			for ( k=i+1;k<NeighbourList->iCount;k++ )
			{
				if ( NeighbourList->Items[i]==NeighbourList->Items[k] )
				{
					NeighbourList->Delete ( i );
					i--;
					break;
				}
			}
		}
		// Prüfen, ob Subbases zusammengefügt werden müssen:
		if ( NeighbourList->iCount>1 )
		{
			// Die Basen zu einer zusammenfügen:
			sSubBase *n,*sb;
			cBuilding *sbb;
			// neue Subbase anlegen:
			n=new sSubBase;
			memset ( n,0,sizeof ( sSubBase ) );
			b->SubBase=n;
			n->buildings=new cList<cBuilding*>;
			AddBuildingToSubBase ( b,n );
			SubBases->Add ( n );
			// Alle gefundenen Subbases durchgehen:
			while ( NeighbourList->iCount )
			{
				sb=NeighbourList->Items[0];
				// Alle Buildungs der Subbase durchgehen:
				while ( sb->buildings->iCount )
				{
					sbb=sb->buildings->Items[0];
					AddBuildingToSubBase ( sbb,n );
					sbb->SubBase=n;
					sb->buildings->Delete ( 0 );
				}
				delete sb->buildings;
				// Die Subbase aus der Subbaseliste löschen:
				for ( i=0;i<SubBases->iCount;i++ )
				{
					if ( SubBases->Items[i]==sb )
					{
						SubBases->Delete ( i );
						break;
					}
				}
				delete sb;
				NeighbourList->Delete ( 0 );
			}
		}
		else
		{
			sSubBase *sb;
			// Das Building nur der Base hinzufügen:
			sb=NeighbourList->Items[0];
			AddBuildingToSubBase ( b,sb );
			b->SubBase=sb;
		}
	}
	else
	{
		sSubBase *n;
		// Keine Nachbarn gefunden:
		b->BaseBE=false;
		b->BaseBN=false;
		b->BaseBS=false;
		b->BaseBW=false;
		b->BaseE=false;
		b->BaseN=false;
		b->BaseS=false;
		b->BaseW=false;
		// Neue Subbase anlegen:
		n=new sSubBase;
		memset ( n,0,sizeof ( sSubBase ) );
		b->SubBase=n;
		n->buildings=new cList<cBuilding*>;
		AddBuildingToSubBase ( b,n );
		SubBases->Add ( n );
	}
	delete NeighbourList;
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
	for ( i=0;i<sb->buildings->iCount;i++ )
	{
		sb->buildings->Items[i]->SubBase=NULL;
	}
	for ( i=0;i<SubBases->iCount;i++ )
	{
		if ( SubBases->Items[i]==sb )
		{
			SubBases->Delete ( i );
			break;
		}
	}
	// Alle Gebäude neu einsetzen:
	for ( i=0;i<sb->buildings->iCount;i++ )
	{
		n=sb->buildings->Items[i];
		if ( n==b ) continue;
		AddBuilding ( n );
	}
	if ( b->IsWorking&&b->data.can_research ) b->owner->StopAReserach();
	delete sb->buildings;
	delete sb;
}

// Fügt ein Gebäude in eine Subbase ein:
void cBase::AddBuildingToSubBase ( cBuilding *b,sSubBase *sb )
{
	sb->buildings->Add ( b );
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
			sb->MetalNeed+=min( b->MetalPerRound, b->BuildList->Items[0]->metall_remaining);
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
	for ( i=0;i<sb->buildings->iCount;i++ )
	{
		b=sb->buildings->Items[i];
		if ( b->data.can_load!=TRANS_METAL ) continue;
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
		if ( value==0 ) break;
	}
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
	for ( i=0;i<sb->buildings->iCount;i++ )
	{
		b=sb->buildings->Items[i];
		if ( b->data.can_load!=TRANS_OIL ) continue;
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
		if ( value==0 ) break;
	}
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
	for ( i=0;i<sb->buildings->iCount;i++ )
	{
		b=sb->buildings->Items[i];
		if ( b->data.can_load!=TRANS_GOLD ) continue;
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
		if ( value==0 ) break;
	}
}

// Alle Aktionen zum Rundenende durchführen:
void cBase::Rundenende ( void )
{
	int i,k;
	sSubBase *sb;

	for ( i=0;i<SubBases->iCount;i++ )
	{
		sb=SubBases->Items[i];
		// Öl produzieren/abziehen:
		if ( sb->OilProd-sb->OilNeed<0&&sb->Oil+ ( sb->OilProd-sb->OilNeed ) <0 )
		{
			// Generator muss abgeschaltet werden:
			game->AddMessage ( lngPack.i18n( "Text~Comp~Fuel_Low") );
			for ( k=0;k<sb->buildings->iCount&&sb->EnergyProd;k++ )
			{
				cBuilding *b;
				b=sb->buildings->Items[k];
				if ( !b->data.energy_prod ) continue;
				b->StopWork ( true );
				if ( sb->OilProd-sb->OilNeed<0&&sb->Oil+ ( sb->OilProd-sb->OilNeed ) <0 ) continue;
				break;
			}
		}
		AddOil ( sb,sb->OilProd-sb->OilNeed );
		// Energieverbraucher prüfen:
		if ( sb->EnergyNeed>sb->EnergyProd )
		{
			game->AddMessage ( lngPack.i18n( "Text~Comp~Energy_Low") );
			for ( k=0;k<sb->buildings->iCount;k++ )
			{
				cBuilding *b;
				b=sb->buildings->Items[k];
				if ( !b->data.energy_need ) continue;
				b->StopWork ( true );
				if ( sb->EnergyNeed>sb->EnergyProd ) continue;
				break;
			}
		}

		// Metall produzieren/abziehen:
		if ( sb->Metal+ ( sb->MetalProd-sb->MetalNeed ) <0 )
		{
			game->AddMessage ( lngPack.i18n( "Text~Comp~Metal_Low") );
			for ( k=0;k<sb->buildings->iCount;k++ )
			{
				cBuilding *b;
				b=sb->buildings->Items[k];
				if ( !b->data.metal_need ) continue;
				b->StopWork ( true );
				if ( sb->Metal+ ( sb->MetalProd-sb->MetalNeed ) <0 ) continue;
				break;
			}
		}
		AddMetal ( sb,sb->MetalProd-sb->MetalNeed );

		// Gold produzieren/abziehen:
		if ( sb->Gold+ ( sb->GoldProd-sb->GoldNeed ) <0 )
		{
			game->AddMessage ( lngPack.i18n( "Text~Comp~Gold_Low") );
			for ( k=0;k<sb->buildings->iCount;k++ )
			{
				cBuilding *b;
				b=sb->buildings->Items[k];
				if ( !b->data.gold_need ) continue;
				b->StopWork ( true );
				if ( sb->Gold+ ( sb->GoldProd-sb->GoldNeed ) <0 ) continue;
				break;
			}
		}
		AddGold ( sb,sb->GoldProd-sb->GoldNeed );
		// Credits erzeugen:
		owner->Credits+=sb->GoldNeed;

		// Humanneed prüfen:
		if ( sb->HumanNeed>sb->HumanProd )
		{
			game->AddMessage ( lngPack.i18n( "Text~Comp~Team_Low") );
			for ( k=0;k<sb->buildings->iCount;k++ )
			{
				cBuilding *b;
				b=sb->buildings->Items[k];
				if ( !b->data.human_need ) continue;
				b->StopWork ( true );
				if ( sb->HumanNeed>sb->HumanProd ) continue;
				break;
			}
		}

		// Energieoptimierungen:
		if ( OptimizeEnergy ( sb ) )
		{
			game->AddMessage (lngPack.i18n( "Text~Comp~Energy_Optimize"));
		}

		// Reparaturen durchführen/bauen/aufladen:
		for ( k=0;k<sb->buildings->iCount&&sb->Metal;k++ )
		{
			cBuilding *b;
			b=sb->buildings->Items[k];
			// Reparatur:
			if ( b->data.hit_points<b->data.max_hit_points&&sb->Metal>0 )
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

				if( game->engine->network )
				{
					SendReloadRepair( true, false, b->PosX + b->PosY * map->size, b->data.max_hit_points, MSG_REPAIR );
				}
			}
			// Aufladen:
			if ( b->data.can_attack&&b->data.ammo==0&&sb->Metal>=2 )
			{
				b->data.ammo=b->data.max_ammo;
				AddMetal ( sb,-2 );

				if( game->engine->network )
				{
					SendReloadRepair( true, false, b->PosX + b->PosY * map->size, b->data.max_ammo, MSG_RELOAD );
				}
			}
			// Bauen:
			if ( b->IsWorking&&b->data.can_build&&b->BuildList->iCount )
			{
				sBuildList *BuildListItem;
				BuildListItem=b->BuildList->Items[0];
				if ( BuildListItem->metall_remaining > 0 )
				{
					//in this block the metal consumption of the factory in the next round can change
					//so we first substract the old value from MetalNeed and then add the new one, to hold the base up to date
					sb->MetalNeed -= min( b->MetalPerRound, BuildListItem->metall_remaining);

					BuildListItem->metall_remaining -= min( b->MetalPerRound, BuildListItem->metall_remaining);
					if (BuildListItem->metall_remaining < 0) BuildListItem->metall_remaining = 0;

					sb->MetalNeed += min( b->MetalPerRound, BuildListItem->metall_remaining);
				}
				if ( BuildListItem->metall_remaining<=0 )
				{
					// if(b->owner==game->ActivePlayer)game->engine->AddReport(ptr->typ->data.name,true);
					b->StopWork ( false );
				}
			}
		}
	}
	if ( game->SelectedBuilding ) game->SelectedBuilding->ShowDetails();
}

// Optimiert den Energieverbrauch der Basis:
bool cBase::OptimizeEnergy ( sSubBase *sb )
{
	bool changed=false;
	int i;
	cList<cBuilding*> *eb,*es;

	if ( sb->EnergyProd==0 ) return false;

	eb=new cList<cBuilding*>;
	es=new cList<cBuilding*>;

	for ( i=0;i<sb->buildings->iCount;i++ )
	{
		cBuilding *b;
		b=sb->buildings->Items[i];
		if ( !b->data.energy_prod ) continue;

		if ( b->data.energy_prod==1 ) es->Add ( b );
		else eb->Add ( b );
	}

	if ( !sb->EnergyNeed&&es->iCount )
	{
		while ( es->iCount )
		{
			es->Items[0]->StopWork ( false );
			es->Delete ( 0 );
			changed=true;
		}
	}
	if ( !sb->EnergyNeed&&eb->iCount )
	{
		while ( es->iCount )
		{
			eb->Items[0]->StopWork ( false );
			eb->Delete ( 0 );
			changed=true;
		}
	}

	if ( es->iCount&&!eb->iCount )
	{
		while ( sb->EnergyNeed<sb->EnergyProd&&es->iCount )
		{
			es->Items[0]->StopWork ( false );
			es->Delete ( 0 );
			changed=true;
		}
	}
	else if ( eb->iCount&&!es->iCount )
	{
		while ( sb->EnergyProd>=sb->EnergyNeed+6&&eb->iCount )
		{
			eb->Items[0]->StopWork ( false );
			eb->Delete ( 0 );
			changed=true;
		}
	}
	else if ( es->iCount&&eb->iCount )
	{
		int bneed,sneed,i,pre;

		pre=sb->EnergyProd;
		bneed=sb->EnergyNeed/6;
		sneed=sb->EnergyNeed%6;
		if ( sneed>=3&&bneed<eb->iCount )
		{
			bneed++;
			sneed=0;
		}

		if ( sneed>es->iCount&&bneed<eb->iCount )
		{
			sneed=0;
			bneed++;
		}

		for ( i=0;i<eb->iCount;i++ )
		{
			if ( i>=bneed ) break;
			eb->Items[i]->StartWork();
			eb->Delete ( i );
			i--;
			bneed--;
		}
		for ( i=0;i<es->iCount;i++ )
		{
			if ( i>=sneed&&sb->EnergyNeed<=sb->EnergyProd ) break;
			es->Items[i]->StartWork();
			es->Delete ( i );
			i--;
			sneed--;
		}
		while ( eb->iCount )
		{
			eb->Items[0]->StopWork ( false );
			eb->Delete ( 0 );
		}
		while ( es->iCount )
		{
			es->Items[0]->StopWork ( false );
			es->Delete ( 0 );
		}

		changed=pre!=sb->EnergyProd;
	}

	delete eb;
	delete es;
	return changed;
}

// Berechnet alle Subbases neu (für ein Load):
void cBase::RefreshSubbases ( void )
{
	cList<sSubBase*> *OldSubBases;
	sSubBase *sb;
	cBuilding *n;
	int i;

	OldSubBases=new cList<sSubBase*>;
	while ( SubBases->iCount )
	{
		sb=SubBases->Items[0];
		OldSubBases->Add ( sb );
		SubBases->Delete ( 0 );
	}

	while ( OldSubBases->iCount )
	{
		sb=OldSubBases->Items[0];

		// Alle SubBases auf NULL setzen:
		for ( i=0;i<sb->buildings->iCount;i++ )
		{
			sb->buildings->Items[i]->SubBase=NULL;
		}
		// Alle Gebäude neu einsetzen:
		for ( i=0;i<sb->buildings->iCount;i++ )
		{
			n=sb->buildings->Items[i];
			AddBuilding ( n );
		}

		delete sb;
		OldSubBases->Delete ( 0 );
	}
	delete OldSubBases;
}
