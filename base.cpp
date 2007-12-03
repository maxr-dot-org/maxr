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

// Funktionen der Base Klasse ////////////////////////////////////////////////
cBase::cBase ( cPlayer *Owner )
{
	owner=Owner;
	SubBases=new TList;
}

cBase::~cBase ( void )
{
	while ( SubBases->Count )
	{
		sSubBase *sb;
		sb=SubBases->SubBaseItems[0];
		delete sb->buildings;
		delete sb;
		SubBases->DeleteSubBase ( 0 );
	}
}

// Fügt ein neues Building in die Base ein:
void cBase::AddBuilding ( cBuilding *b )
{
	TList *NeighbourList;
	int pos;
	if ( b->data.is_base ) return;
	pos=b->PosX+b->PosY*map->size;
	NeighbourList=new TList;
	b->SubBase= ( sSubBase* ) 1;
	// Prüfen, ob ein Gebäude in in der Nähe steht:
#define CHECK_NEIGHBOUR(a) if(a>=0&&a<map->size*map->size&&map->GO[a].top&&map->GO[a].top->owner==b->owner&&map->GO[a].top->SubBase){NeighbourList->AddSubBase(map->GO[a].top->SubBase);map->GO[a].top->CheckNeighbours();}
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
	if ( NeighbourList->Count )
	{
		int i,k;
		// Nachbarn gefunden:
		b->CheckNeighbours();
		// Die doppelten Einträge löschen:
		for ( i=0;i<NeighbourList->Count;i++ )
		{
			for ( k=i+1;k<NeighbourList->Count;k++ )
			{
				if ( NeighbourList->SubBaseItems[i]==NeighbourList->SubBaseItems[k] )
				{
					NeighbourList->DeleteSubBase ( i );
					i--;
					break;
				}
			}
		}
		// Prüfen, ob Subbases zusammengefügt werden müssen:
		if ( NeighbourList->Count>1 )
		{
			// Die Basen zu einer zusammenfügen:
			sSubBase *n,*sb;
			cBuilding *sbb;
			// neue Subbase anlegen:
			n=new sSubBase;
			memset ( n,0,sizeof ( sSubBase ) );
			b->SubBase=n;
			n->buildings=new TList;
			AddBuildingToSubBase ( b,n );
			SubBases->AddSubBase ( n );
			// Alle gefundenen Subbases durchgehen:
			while ( NeighbourList->Count )
			{
				sb=NeighbourList->SubBaseItems[0];
				// Alle Buildungs der Subbase durchgehen:
				while ( sb->buildings->Count )
				{
					sbb=sb->buildings->BuildItems[0];
					AddBuildingToSubBase ( sbb,n );
					sbb->SubBase=n;
					sb->buildings->DeleteBuilding ( 0 );
				}
				// Die Subbase aus der Subbaseliste löschen:
				for ( i=0;i<SubBases->Count;i++ )
				{
					if ( SubBases->SubBaseItems[i]==sb )
					{
						SubBases->DeleteSubBase ( i );
						break;
					}
				}
				delete sb;
				NeighbourList->DeleteSubBase ( 0 );
			}
		}
		else
		{
			sSubBase *sb;
			// Das Building nur der Base hinzufügen:
			sb=NeighbourList->SubBaseItems[0];
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
		n->buildings=new TList;
		AddBuildingToSubBase ( b,n );
		SubBases->AddSubBase ( n );
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
	for ( i=0;i<sb->buildings->Count;i++ )
	{
		sb->buildings->BuildItems[i]->SubBase=NULL;
	}
	for ( i=0;i<SubBases->Count;i++ )
	{
		if ( SubBases->SubBaseItems[i]==sb )
		{
			SubBases->DeleteSubBase ( i );
			break;
		}
	}
	// Alle Gebäude neu einsetzen:
	for ( i=0;i<sb->buildings->Count;i++ )
	{
		n=sb->buildings->BuildItems[i];
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
	sb->buildings->AddBuild ( b );
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
			sb->MetalNeed+=min( b->MetalPerRound, b->BuildList->BuildListItems[0]->metall_remaining);
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
	for ( i=0;i<sb->buildings->Count;i++ )
	{
		b=sb->buildings->BuildItems[i];
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
	for ( i=0;i<sb->buildings->Count;i++ )
	{
		b=sb->buildings->BuildItems[i];
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
	for ( i=0;i<sb->buildings->Count;i++ )
	{
		b=sb->buildings->BuildItems[i];
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

	for ( i=0;i<SubBases->Count;i++ )
	{
		sb=SubBases->SubBaseItems[i];
		// Öl produzieren/abziehen:
		if ( sb->OilProd-sb->OilNeed<0&&sb->Oil+ ( sb->OilProd-sb->OilNeed ) <0 )
		{
			// Generator muss abgeschaltet werden:
			game->AddMessage ( lngPack.i18n( "Text~Comp~Fuel_Low") );
			for ( k=0;k<sb->buildings->Count&&sb->EnergyProd;k++ )
			{
				cBuilding *b;
				b=sb->buildings->BuildItems[k];
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
			for ( k=0;k<sb->buildings->Count;k++ )
			{
				cBuilding *b;
				b=sb->buildings->BuildItems[k];
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
			for ( k=0;k<sb->buildings->Count;k++ )
			{
				cBuilding *b;
				b=sb->buildings->BuildItems[k];
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
			for ( k=0;k<sb->buildings->Count;k++ )
			{
				cBuilding *b;
				b=sb->buildings->BuildItems[k];
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
			for ( k=0;k<sb->buildings->Count;k++ )
			{
				cBuilding *b;
				b=sb->buildings->BuildItems[k];
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
		for ( k=0;k<sb->buildings->Count&&sb->Metal;k++ )
		{
			cBuilding *b;
			b=sb->buildings->BuildItems[k];
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

			}
			// Aufladen:
			if ( b->data.can_attack&&b->data.ammo==0&&sb->Metal>=2 )
			{
				b->data.ammo=b->data.max_ammo;
				AddMetal ( sb,-2 );

			}
			// Bauen:
			if ( b->IsWorking&&b->data.can_build&&b->BuildList->Count )
			{
				sBuildList *BuildListItem;
				BuildListItem=b->BuildList->BuildListItems[0];
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
	TList *eb,*es;

	if ( sb->EnergyProd==0 ) return false;

	eb=new TList;
	es=new TList;

	for ( i=0;i<sb->buildings->Count;i++ )
	{
		cBuilding *b;
		b=sb->buildings->BuildItems[i];
		if ( !b->data.energy_prod ) continue;

		if ( b->data.energy_prod==1 ) es->AddBuild ( b );
		else eb->AddBuild ( b );
	}

	if ( !sb->EnergyNeed&&es->Count )
	{
		while ( es->Count )
		{
			es->BuildItems[0]->StopWork ( false );
			es->DeleteBuilding ( 0 );
			changed=true;
		}
	}
	if ( !sb->EnergyNeed&&eb->Count )
	{
		while ( es->Count )
		{
			eb->BuildItems[0]->StopWork ( false );
			eb->DeleteBuilding ( 0 );
			changed=true;
		}
	}

	if ( es->Count&&!eb->Count )
	{
		while ( sb->EnergyNeed<sb->EnergyProd&&es->Count )
		{
			es->BuildItems[0]->StopWork ( false );
			es->DeleteBuilding ( 0 );
			changed=true;
		}
	}
	else if ( eb->Count&&!es->Count )
	{
		while ( sb->EnergyProd>=sb->EnergyNeed+6&&eb->Count )
		{
			eb->BuildItems[0]->StopWork ( false );
			eb->DeleteBuilding ( 0 );
			changed=true;
		}
	}
	else if ( es->Count&&eb->Count )
	{
		int bneed,sneed,i,pre;

		pre=sb->EnergyProd;
		bneed=sb->EnergyNeed/6;
		sneed=sb->EnergyNeed%6;
		if ( sneed>=3&&bneed<eb->Count )
		{
			bneed++;
			sneed=0;
		}

		if ( sneed>es->Count&&bneed<eb->Count )
		{
			sneed=0;
			bneed++;
		}

		for ( i=0;i<eb->Count;i++ )
		{
			if ( i>=bneed ) break;
			eb->BuildItems[i]->StartWork();
			eb->DeleteBuilding ( i );
			i--;
			bneed--;
		}
		for ( i=0;i<es->Count;i++ )
		{
			if ( i>=sneed&&sb->EnergyNeed<=sb->EnergyProd ) break;
			es->BuildItems[i]->StartWork();
			es->DeleteBuilding ( i );
			i--;
			sneed--;
		}
		while ( eb->Count )
		{
			eb->BuildItems[0]->StopWork ( false );
			eb->DeleteBuilding ( 0 );
		}
		while ( es->Count )
		{
			es->BuildItems[0]->StopWork ( false );
			es->DeleteBuilding ( 0 );
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
	TList *OldSubBases;
	sSubBase *sb;
	cBuilding *n;
	int i;

	OldSubBases=new TList;
	while ( SubBases->Count )
	{
		sb=SubBases->SubBaseItems[0];
		OldSubBases->AddSubBase ( sb );
		SubBases->DeleteSubBase ( 0 );
	}

	while ( OldSubBases->Count )
	{
		sb=OldSubBases->SubBaseItems[0];

		// Alle SubBases auf NULL setzen:
		for ( i=0;i<sb->buildings->Count;i++ )
		{
			sb->buildings->BuildItems[i]->SubBase=NULL;
		}
		// Alle Gebäude neu einsetzen:
		for ( i=0;i<sb->buildings->Count;i++ )
		{
			n=sb->buildings->BuildItems[i];
			AddBuilding ( n );
		}

		delete sb;
		OldSubBases->DeleteSubBase ( 0 );
	}
	delete OldSubBases;
}
