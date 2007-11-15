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
#include "math.h"
#include "buildings.h"
#include "main.h"
#include "game.h"
#include "fonts.h"
#include "mouse.h"

// Funktionen der Vehicle Klasse /////////////////////////////////////////////
cBuilding::cBuilding ( sBuilding *b,cPlayer *Owner,cBase *Base )
{
	PosX=0;PosY=0;
	DirtTyp=0;
	DirtValue=0;
	EffectAlpha=0;
	dir=0;
	StartUp=0;
	IsWorking=false;
	IsLocked=false;
	typ=b;
	owner=Owner;
	base=Base;

	if ( Owner==NULL||b==NULL )
	{
		if ( b!=NULL )
		{
			data=b->data;
		}
		else
		{
			memset ( &data,0,sizeof ( sUnitData ) );
		}
		BuildList=NULL;
		StoredVehicles=NULL;
		detected=true;
		return;
	}
	data=owner->BuildingData[typ->nr];
	selected=false;
	MenuActive=false;
	AttackMode=false;
	Transfer=false;
	BaseN=false;
	BaseE=false;
	BaseS=false;
	BaseW=false;
	Attacking=false;
	LoadActive=false;
	ActivatingVehicle=false;
	RepeatBuild=false;
	if ( data.is_expl_mine )
	{
		detected=false;
	}
	else
	{
		detected=true;
	}
	MetalProd=0;
	GoldProd=0;
	OilProd=0;
	MaxMetalProd=0;
	MaxGoldProd=0;
	MaxOilProd=0;
	Disabled=0;
	data.hit_points=data.max_hit_points;
	data.ammo=data.max_ammo;
	SubBase=NULL;
	BuildSpeed=0;
	BuildList=NULL;
	if ( data.can_build )
	{
		BuildList=new TList;
	}
	StoredVehicles=NULL;
	if ( data.can_load==TRANS_VEHICLES||data.can_load==TRANS_MEN||data.can_load==TRANS_AIR )
	{
		StoredVehicles=new TList;
	}
	if ( data.is_big )
	{
		DamageFXPointX=random ( 64,0 ) +32;
		DamageFXPointY=random ( 64,0 ) +32;
		DamageFXPointX2=random ( 64,0 ) +32;
		DamageFXPointY2=random ( 64,0 ) +32;
	}
	else
	{
		DamageFXPointX=random ( 64-24,0 );
		DamageFXPointY=random ( 64-24,0 );
	}
	RefreshData();
}

cBuilding::~cBuilding ( void )
{
	if ( BuildList )
	{
		while ( BuildList->Count )
		{
			sBuildList *ptr;
			ptr=BuildList->BuildListItems[0];
			delete ptr;
			BuildList->DeleteBuildList ( 0 );
		}
	}
	if ( StoredVehicles )
	{
		while ( StoredVehicles->Count )
		{
			cVehicle *v;
			v=StoredVehicles->VehicleItems[0];

			if ( v->prev )
			{
				cVehicle *vp;
				vp=v->prev;
				vp->next=v->next;
				if ( v->next ) v->next->prev=vp;
			}
			else
			{
				v->owner->VehicleList=v->next;
				if ( v->next ) v->next->prev=NULL;
			}
			delete v;
			StoredVehicles->DeleteVehicle ( 0 );
		}
		delete StoredVehicles;
	}
	if ( data.can_attack )
	{
		owner->DeleteWachpostenB ( this );
	}
	if ( IsLocked )
	{
		cPlayer *p;
		int i;
		for ( i=0;i<game->PlayerList->Count;i++ )
		{
			p=game->PlayerList->PlayerItems[i];
			p->DeleteLock ( this );
		}
	}
}

// Liefert einen String mit dem aktuellen Status zurück:
char *cBuilding::GetStatusStr ( void )
{
	if ( IsWorking )
	{
		// Fabrik:
		if ( data.can_build&&BuildList&&BuildList->Count&&owner==game->ActivePlayer )
		{
			sBuildList *ptr;
			ptr=BuildList->BuildListItems[0];
			if ( ptr->metall_remaining>0 )
			{
				static char str[50];
				int r;
				//r = ( int ) ( floor ( (double) ptr->metall_remaining/ ( data.metal_need*BuildSpeed*1 ) ) );
				r = ( int ) ceil ( ptr->metall_remaining / ( double ) MetalPerRound );
				//if ( !r ) r++;
				sprintf ( str,"beim bau: %s (%d)",owner->VehicleData[ptr->typ->nr].name,r ); //"Runden"?
				if ( fonts->GetTextLenSmall ( str ) >126 )
				{
					sprintf ( str,"beim bau:\n%s (%d)",owner->VehicleData[ptr->typ->nr].name,r ); //"Runden"?
				}
				return str;
			}
			else
			{
				return "bau abgeschlossen";
			}
		}
		// Forschungszentrum:
		if ( data.can_research&&owner==game->ActivePlayer )
		{
			static char str[400];
			char tmp[50];
			int i;
			sprintf ( str,"arbeitet\n" );
			for ( i=0;i<8;i++ )
			{
				if ( owner->ResearchTechs[i].working_on )
				{
					switch ( i )
					{
						case 0:sprintf ( tmp,"Angriff: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 1:sprintf ( tmp,"Schüsse: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 2:sprintf ( tmp,"Reichweite: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 3:sprintf ( tmp,"Panzer: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 4:sprintf ( tmp,"Treffer: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 5:sprintf ( tmp,"Geschw.: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 6:sprintf ( tmp,"Scan: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
						case 7:sprintf ( tmp,"Kosten: %d\n", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );strcat ( str,tmp );break;
					}
				}
			}
			return str;
		}
		// Goldraffinerie:
		if ( data.gold_need&&owner==game->ActivePlayer )
		{
			static char str[50];
			sprintf ( str,"arbeitet\nCredits: %d",owner->Credits );
			return str;
		}
		return "arbeitet";
	}
	if ( Disabled )
	{
		static char str[50];
		sprintf ( str,"außer Gefecht (%d)",Disabled );
		return str;
	}
	return "wartet";
}

// Aktalisiert alle Daten auf ihre Max-Werte:
void cBuilding::RefreshData ( void )
{
	if ( data.ammo>=data.max_shots )
	{
		data.shots=data.max_shots;
	}
	else
	{
		data.shots=data.ammo;
	}
}

// Erzeugt den Namen für das Buildings aus der Versionsnummer:
void cBuilding::GenerateName ( void )
{
	string rome;
	int nr,tmp;
	rome="";
	nr=data.version;

	// Römische Versionsnummer erzeugen (ist bis 899 richtig):
	if ( nr>100 )
	{
		tmp=nr/100;
		nr%=100;
		while ( tmp-- )
		{
			rome+="C";
		}
	}
	if ( nr>=90 )
	{
		rome+="XC";
		nr-=90;
	}
	if ( nr>=50 )
	{
		nr-=50;
		rome+="L";
	}
	if ( nr>=40 )
	{
		nr-=40;
		rome+="XL";
	}
	if ( nr>=10 )
	{
		tmp=nr/10;
		nr%=10;
		while ( tmp-- )
		{
			rome+="X";
		}
	}
	if ( nr==9 )
	{
		nr-=9;
		rome+="IX";
	}
	if ( nr>=5 )
	{
		nr-=5;
		rome+="V";
	}
	if ( nr==4 )
	{
		nr-=4;
		rome+="IV";
	}
	while ( nr-- )
	{
		rome+="I";
	}
	// Den Namen zusammenbauen:
	// name=(string)data.name + " MK "+rome;
	name += ( string ) data.name; name += " MK "; name += rome;
}

void cBuilding::Draw ( SDL_Rect *dest )
{
	SDL_Rect scr,tmp;
	tmp=*dest;
	scr.x=0;
	scr.y=0;

	// Den Schadenseffekt machen:
	if ( timer1&&!data.is_base&&!data.is_connector&&data.hit_points<data.max_hit_points&&SettingsData.bDamageEffects&& ( owner==game->ActivePlayer||game->ActivePlayer->ScanMap[PosX+PosY*game->map->size] ) )
	{
		int intense= ( int ) ( 200-200* ( ( float ) data.hit_points/data.max_hit_points ) );
		game->AddFX ( fxDarkSmoke,PosX*64+DamageFXPointX,PosY*64+DamageFXPointY,intense );
		if ( data.is_big&&intense>50 )
		{
			intense-=50;
			game->AddFX ( fxDarkSmoke,PosX*64+DamageFXPointX2,PosY*64+DamageFXPointY2,intense );
		}
	}

	// Prüfen, ob die Mine gemalt werden soll:
	if ( data.is_expl_mine&&owner!=game->ActivePlayer&&!detected ) return;
	// Prüfen, ob es Dreck ist:
	if ( !owner )
	{
		if ( BigDirt ) scr.w=scr.h=dest->h=dest->w=UnitsData.dirt_big->h;
		else scr.w=scr.h=dest->h=dest->w=UnitsData.dirt_small->h;
		scr.x=scr.w*DirtTyp;
		scr.y=0;
		// Den Schatten malen:
		if ( SettingsData.bShadows )
		{
			if ( BigDirt ) SDL_BlitSurface ( UnitsData.dirt_big_shw,&scr,buffer,&tmp );
			else SDL_BlitSurface ( UnitsData.dirt_small_shw,&scr,buffer,&tmp );
		}
		// Das Building malen:
		tmp=*dest;
		if ( BigDirt ) SDL_BlitSurface ( UnitsData.dirt_big,&scr,buffer,&tmp );
		else SDL_BlitSurface ( UnitsData.dirt_small,&scr,buffer,&tmp );
		return;
	}

	// Größe auslesen:
	if ( !data.is_connector )
	{
		if ( data.has_frames )
		{
			dest->w=scr.w=game->hud->Zoom;
			dest->h=scr.h=game->hud->Zoom;
		}
		else
		{
			scr.w=dest->w=typ->img->w;
			scr.h=dest->h=typ->img->h;
		}
	}
	else
	{
		scr.y=0;
		scr.x=0;
		scr.h=dest->h=scr.w=dest->w=typ->img->h;
		if ( BaseN )
		{
			if ( BaseE )
			{
				if ( BaseS )
				{
					if ( BaseW )
					{
						// N,E,S,W
						scr.x+=15*scr.h;
					}
					else
					{
						// N,E,S
						scr.x+=13*scr.h;
					}
				}
				else
				{
					// N,E
					if ( BaseW )
					{
						// N,E,W
						scr.x+=12*scr.h;
					}
					else
					{
						// N,E
						scr.x+=8*scr.h;
					}
				}
			}
			else
			{
				// N
				if ( BaseS )
				{
					// N,S
					if ( BaseW )
					{
						// N,S,W
						scr.x+=11*scr.h;
					}
					else
					{
						// N,S
						scr.x+=5*scr.h;
					}
				}
				else
				{
					// N
					if ( BaseW )
					{
						// N,W
						scr.x+=7*scr.h;
					}
					else
					{
						// N
						scr.x+=1*scr.h;
					}
				}
			}
		}
		else
		{
			if ( BaseE )
			{
				// E
				if ( BaseS )
				{
					// E,S
					if ( BaseW )
					{
						// E,S,W
						scr.x+=14*scr.h;
					}
					else
					{
						// E,S
						scr.x+=9*scr.h;
					}
				}
				else
				{
					// E
					if ( BaseW )
					{
						// E,W
						scr.x+=6*scr.h;
					}
					else
					{
						// E
						scr.x+=2*scr.h;
					}
				}
			}
			else
			{
				if ( BaseS )
				{
					// S
					if ( BaseW )
					{
						// S,W
						scr.x+=10*scr.h;
					}
					else
					{
						// S
						scr.x+=3*scr.h;
					}
				}
				else
				{
					if ( BaseW )
					{
						// W
						scr.x+=4*scr.h;
					}
					else
					{
						scr.x=0;
					}
				}
			}
		}
	}

	// Den Beton malen:
	if ( !data.build_on_water&&!data.is_expl_mine )
	{
		if ( data.is_big )
		{
			if ( StartUp&&SettingsData.bAlphaEffects )
			{
				SDL_SetAlpha ( GraphicsData.gfx_big_beton,SDL_SRCALPHA,StartUp );
				SDL_BlitSurface ( GraphicsData.gfx_big_beton,NULL,buffer,&tmp );
				SDL_SetAlpha ( GraphicsData.gfx_big_beton,SDL_SRCALPHA,255 );
			}
			else
			{
				SDL_SetAlpha ( GraphicsData.gfx_big_beton,SDL_SRCALPHA,255 );
				SDL_BlitSurface ( GraphicsData.gfx_big_beton,NULL,buffer,&tmp );
			}
		}
		else if ( !data.is_road&&!data.is_connector )
		{
			if ( StartUp&&SettingsData.bAlphaEffects )
			{
				SDL_SetAlpha ( UnitsData.ptr_small_beton,SDL_SRCALPHA,StartUp );
				SDL_BlitSurface ( UnitsData.ptr_small_beton,NULL,buffer,&tmp );
				SDL_SetAlpha ( UnitsData.ptr_small_beton,SDL_SRCALPHA,255 );
			}
			else
			{
				SDL_SetAlpha ( UnitsData.ptr_small_beton,SDL_SRCALPHA,255 );
				SDL_BlitSurface ( UnitsData.ptr_small_beton,NULL,buffer,&tmp );
			}
		}
	}
	tmp=*dest;

	// Die Connectoranschlüsse malen:
	if ( !data.is_connector&&!data.is_base&&!StartUp )
	{
		DrawConnectors ( *dest );
	}

	// Den Schatten malen:
	if ( SettingsData.bShadows )
	{
		if ( StartUp&&SettingsData.bAlphaEffects )
		{
			SDL_SetAlpha ( typ->shw,SDL_SRCALPHA,StartUp/5 );
			if ( !data.is_connector )
			{
				SDL_BlitSurface ( typ->shw,NULL,buffer,&tmp );
			}
			else
			{
				SDL_BlitSurface ( typ->shw,&scr,buffer,&tmp );
			}
			SDL_SetAlpha ( typ->shw,SDL_SRCALPHA,50 );
		}
		else
		{
			if ( !data.is_connector )
			{
				SDL_BlitSurface ( typ->shw,NULL,buffer,&tmp );
			}
			else
			{
				SDL_BlitSurface ( typ->shw,&scr,buffer,&tmp );
			}
		}
	}
	// Die Spielerfarbe blitten:
	if ( !data.is_connector&&!data.is_road )
	{
		SDL_BlitSurface ( owner->color,NULL,GraphicsData.gfx_tmp,NULL );
		if ( !data.is_connector )
		{
			if ( data.has_frames )
			{
				if ( data.is_annimated&&SettingsData.bAnimations&&!Disabled )
				{
					scr.x= ( game->Frame%data.has_frames ) *game->hud->Zoom;
				}
				else
				{
					scr.x=dir*game->hud->Zoom;
				}
				SDL_BlitSurface ( typ->img,&scr,GraphicsData.gfx_tmp,NULL );
				scr.x=0;
			}
			else
			{
				SDL_BlitSurface ( typ->img,NULL,GraphicsData.gfx_tmp,NULL );
			}
		}
		else
		{
			SDL_BlitSurface ( typ->img,&scr,GraphicsData.gfx_tmp,NULL );
		}
	}
	else
	{
		SDL_FillRect ( GraphicsData.gfx_tmp,NULL,0xFF00FF );
		if ( !data.is_connector )
		{
			SDL_BlitSurface ( typ->img,NULL,GraphicsData.gfx_tmp,NULL );
		}
		else
		{
			SDL_BlitSurface ( typ->img,&scr,GraphicsData.gfx_tmp,NULL );
		}
	}

	// Das Building malen:
	tmp=*dest;
	scr.x=0;
	scr.y=0;
	if ( StartUp&&SettingsData.bAlphaEffects )
	{
		SDL_SetAlpha ( GraphicsData.gfx_tmp,SDL_SRCALPHA,StartUp );
		SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,buffer,&tmp );
		SDL_SetAlpha ( GraphicsData.gfx_tmp,SDL_SRCALPHA,255 );
		if ( timer0 ) StartUp+=25;
		if ( StartUp>=255 ) StartUp=0;
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,buffer,&tmp );
	}

	// Ggf den Effekt malen:
	if ( data.has_effect&&SettingsData.bAnimations&& ( IsWorking||!data.can_work ) )
	{
		tmp=*dest;
		SDL_SetAlpha ( typ->eff,SDL_SRCALPHA,EffectAlpha );
		SDL_BlitSurface ( typ->eff,NULL,buffer,&tmp );
		if ( timer0 )
		{
			if ( EffectInc )
			{
				EffectAlpha+=30;
				if ( EffectAlpha>220 )
				{
					EffectAlpha=255;
					EffectInc=!EffectInc;
				}
			}
			else
			{
				EffectAlpha-=30;
				if ( EffectAlpha<30 )
				{
					EffectAlpha=0;
					EffectInc=!EffectInc;
				}
			}
		}
	}

	// Ggf Markierung malen, wenn der Bauvorgang abgeschlossen ist:
	if ( BuildList&&BuildList->Count&&!IsWorking&&BuildList->BuildListItems[0]->metall_remaining<=0&&owner==game->ActivePlayer )
	{
		SDL_Rect d,t;
		int max,nr;
		nr=0xFF00- ( ( game->Frame%0x8 ) *0x1000 );
		max= ( game->hud->Zoom-2 ) *2;
		d.x=dest->x+2;
		d.y=dest->y+2;
		d.w=max;
		d.h=1;
		t=d;
		SDL_FillRect ( buffer,&d,nr );
		d=t;
		d.y+=max-1;
		t=d;
		SDL_FillRect ( buffer,&d,nr );
		d=t;
		d.y=dest->y+2;
		d.w=1;
		d.h=max;
		t=d;
		SDL_FillRect ( buffer,&d,nr );
		d=t;
		d.x+=max-1;
		SDL_FillRect ( buffer,&d,nr );
	}

	// Ggf den farbigen Rahmen malen:
	if ( game->hud->Farben )
	{
		SDL_Rect d,t;
		int max,nr;
		nr=* ( unsigned int* ) owner->color->pixels;
		if ( data.is_big )
		{
			max= ( game->hud->Zoom-1 ) *2;
		}
		else
		{
			max=game->hud->Zoom-1;
		}
		d.x=dest->x+1;
		d.y=dest->y+1;
		d.w=max;
		d.h=1;
		t=d;
		SDL_FillRect ( buffer,&d,nr );
		d=t;
		d.y+=max-1;
		t=d;
		SDL_FillRect ( buffer,&d,nr );
		d=t;
		d.y=dest->y+1;
		d.w=1;
		d.h=max;
		t=d;
		SDL_FillRect ( buffer,&d,nr );
		d=t;
		d.x+=max-1;
		SDL_FillRect ( buffer,&d,nr );
	}

	// Ggf den Rahmen malen:
	if ( selected )
	{
		SDL_Rect d,t;
		int len,max;
		if ( data.is_big )
		{
			max=game->hud->Zoom*2;
		}
		else
		{
			max=game->hud->Zoom;
		}
		len=max/4;
		d.x=dest->x+1;
		d.y=dest->y+1;
		d.w=len;
		d.h=1;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.x+=max-len-1;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.y+=max-2;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.x=dest->x+1;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.y=dest->y+1;
		d.w=1;
		d.h=len;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.x+=max-2;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.y+=max-len-1;
		t=d;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
		d=t;
		d.x=dest->x+1;
		SDL_FillRect ( buffer,&d,game->BlinkColor );
	}
}

// Liefert die Anzahl der Menüpunkte:
int cBuilding::GetMenuPointAnz ( void )
{
	int nr=2;
	if ( !typ->data.is_road ) nr++;
	if ( typ->data.can_build ) nr++;
	if ( typ->data.can_load==TRANS_METAL||typ->data.can_load==TRANS_OIL||typ->data.can_load==TRANS_GOLD ) nr++;
	if ( typ->data.can_attack&&data.shots ) nr++;
	if ( typ->data.can_work&& ( IsWorking||typ->data.can_build==BUILD_NONE ) ) nr++;
	if ( typ->data.is_mine ) nr++;
	if ( typ->data.can_load==TRANS_VEHICLES||typ->data.can_load==TRANS_MEN||typ->data.can_load==TRANS_AIR ) nr+=2;
	if ( typ->data.can_research&&IsWorking ) nr++;
	if ( data.version!=owner->BuildingData[typ->nr].version&&SubBase->Metal>=2 ) nr+=2;
	if ( data.gold_need ) nr++;
	return nr;
}

// Liefert die Größe des Menüs und Position zurück:
SDL_Rect cBuilding::GetMenuSize ( void )
{
	SDL_Rect dest;
	int i,size;
	dest.x=GetScreenPosX();
	dest.y=GetScreenPosY();
	dest.h=i=GetMenuPointAnz() *22;
	dest.w=42;
	size=game->hud->Zoom;
	if ( data.is_big ) size*=2;

	if ( dest.x+size+42>=SettingsData.iScreenW-12 )
	{
		dest.x-=42;
	}
	else
	{
		dest.x+=size;
	}
	if ( dest.y- ( i-size ) /2<=24 )
	{
		dest.y-= ( i-size ) /2;
		dest.y+=- ( dest.y-24 );
	}
	else if ( dest.y- ( i-size ) /2+i>=SettingsData.iScreenH-24 )
	{
		dest.y-= ( i-size ) /2;
		dest.y-= ( dest.y+i )- ( SettingsData.iScreenH-24 );
	}
	else
	{
		dest.y-= ( i-size ) /2;
	}

	return dest;
}

// Liefert true zurück, wenn die Koordinaten in dem Menübereich liegen:
bool cBuilding::MouseOverMenu ( int mx,int my )
{
	SDL_Rect r;
	r=GetMenuSize();
	if ( mx<r.x||mx>r.x+r.w ) return false;
	if ( my<r.y||my>r.y+r.h ) return false;
	return true;
}

// Zeigt das Selbstzerstörungsmenü an:
void cBuilding::SelfDestructionMenu ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b;
	SDL_Rect scr,dest;
	bool AbbruchPressed=false;
	bool ScharfPressed=false;
	bool DestroyPressed=true;
	bool Scharf=false;
	int GlasHeight=56;

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	game->DrawMap();
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	if ( SettingsData.bAlphaEffects ) SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,buffer,NULL );
	dest.x=233;scr.x=0;
	dest.y=199;scr.y=0;
	scr.w=dest.w=GraphicsData.gfx_destruction->w;
	scr.h=dest.h=GraphicsData.gfx_destruction->h/2;
	SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );

	dest.w=59;
	dest.h=56;
	dest.x=233+15;
	dest.y=199+13;
	SDL_BlitSurface ( GraphicsData.gfx_destruction_glas,NULL,buffer,&dest );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );
	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Abbruch-Button:
		if ( x>=233+89&&x<233+89+71&&y>=199+46&&y<199+46+21 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=232;
				scr.y=231;
				dest.w=scr.w=75;
				dest.h=scr.h=24;
				dest.x=233+89;
				dest.y=199+46;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				return;
			}
		}
		else if ( AbbruchPressed )
		{
			scr.x=89;
			scr.y=46;
			dest.w=scr.w=75;
			dest.h=scr.h=24;
			dest.x=233+89;
			dest.y=199+46;
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}
		// Scharf-Button:
		if ( !Scharf&&x>=233+89&&x<233+89+71&&y>=199+14&&y<199+14+21 )
		{
			if ( b&&!ScharfPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=156;
				scr.y=231;
				dest.w=scr.w=75;
				dest.h=scr.h=24;
				dest.x=233+89;
				dest.y=199+14;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				ScharfPressed=true;
			}
			else if ( !b&&LastB )
			{
				Scharf=true;
				PlayFX ( SoundData.SNDArm );
			}
		}
		else if ( !Scharf&&ScharfPressed )
		{
			scr.x=89;
			scr.y=14;
			dest.w=scr.w=75;
			dest.h=scr.h=24;
			dest.x=233+89;
			dest.y=199+14;
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			ScharfPressed=false;
		}
		// Das Schutzglas hochfahren:
		if ( Scharf&&GlasHeight>0&&timer0 )
		{
			scr.x=15;
			scr.y=13;
			scr.w=dest.w=59;
			scr.h=dest.h=56;
			dest.x=233+15;
			dest.y=199+13;
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
			GlasHeight-=10;
			if ( GlasHeight>0 )
			{
				scr.x=0;scr.y=0;
				scr.h=dest.h=GlasHeight;
				SDL_BlitSurface ( GraphicsData.gfx_destruction_glas,&scr,buffer,&dest );
			}
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Zerstören-Button:
		if ( GlasHeight<=0&&x>=233+15&&x<233+15+59&&y>=199+13&&y<199+13+56 )
		{
			if ( b&&!DestroyPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=15;
				scr.y=95;
				dest.w=scr.w=59;
				dest.h=scr.h=56;
				dest.x=233+15;
				dest.y=199+13;
				SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				DestroyPressed=true;
			}
			else if ( !b&&LastB )
			{
				// Nur SP/Server:
				game->engine->DestroyObject ( PosX+PosY*game->map->size,false );
				return;
			}
		}
		else if ( GlasHeight<=0&&DestroyPressed )
		{
			scr.x=15;
			scr.y=13;
			dest.w=scr.w=59;
			dest.h=scr.h=56;
			dest.x=233+15;
			dest.y=199+13;
			SDL_BlitSurface ( GraphicsData.gfx_destruction,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DestroyPressed=false;
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
}

// Zeigt die Details mit großen Symbolen an:
void cBuilding::ShowBigDetails ( void )
{
	SDL_Rect dest;
	char str[6];
	int y;
	y=297;
	if ( data.max_shield )
	{
		// Range:
		sprintf ( str,"%d",data.range );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Reichw.",42,y,buffer );
		DrawSymbolBig ( SBRange,95,y-2,160,data.range,typ->data.range,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	if ( data.can_attack )
	{
		// Damage:
		sprintf ( str,"%d",data.damage );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Angriff",42,y,buffer );
		DrawSymbolBig ( SBAttack,95,y-3,160,data.damage,typ->data.damage,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
		if ( !data.is_expl_mine )
		{
			// Shots:
			sprintf ( str,"%d",data.max_shots );
			fonts->OutTextCenter ( str,27,y,buffer );
			fonts->OutText ( "Schüsse",42,y,buffer );
			DrawSymbolBig ( SBShots,95,y+2,160,data.max_shots,typ->data.max_shots,buffer );
			dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
			SDL_FillRect ( buffer,&dest,0xFC0000 );
			y+=19;
			// Range:
			sprintf ( str,"%d",data.range );
			fonts->OutTextCenter ( str,27,y,buffer );
			fonts->OutText ( "Reichw.",42,y,buffer );
			DrawSymbolBig ( SBRange,95,y-2,160,data.range,typ->data.range,buffer );
			dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
			SDL_FillRect ( buffer,&dest,0xFC0000 );
			y+=19;
			// Ammo:
			sprintf ( str,"%d",data.ammo );
			fonts->OutTextCenter ( str,27,y,buffer );
			fonts->OutText ( "Munni.",42,y,buffer );
			DrawSymbolBig ( SBAmmo,95,y-2,160,data.ammo,typ->data.max_ammo,buffer );
			dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
			SDL_FillRect ( buffer,&dest,0xFC0000 );
			y+=19;
		}
	}
	if ( data.can_load==TRANS_METAL||data.can_load==TRANS_OIL||data.can_load==TRANS_GOLD )
	{
		// Metall:
		sprintf ( str,"%d",data.max_cargo );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Ladung",42,y,buffer );
		switch ( data.can_load )
		{
			case TRANS_METAL:
				DrawSymbolBig ( SBMetal,95,y-2,160,data.max_cargo,typ->data.max_cargo,buffer );
				break;
			case TRANS_OIL:
				DrawSymbolBig ( SBOil,95,y-2,160,data.max_cargo,typ->data.max_cargo,buffer );
				break;
			case TRANS_GOLD:
				DrawSymbolBig ( SBGold,95,y-2,160,data.max_cargo,typ->data.max_cargo,buffer );
				break;
		}
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	if ( data.energy_prod )
	{
		// Energieproduktion:
		sprintf ( str,"%d",data.energy_prod );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Prod.",42,y,buffer );
		DrawSymbolBig ( SBEnergy,95,y-2,160,data.energy_prod,typ->data.energy_prod,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
		// Verbrauch:
		sprintf ( str,"%d",data.oil_need );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Verbr.",42,y,buffer );
		DrawSymbolBig ( SBOil,95,y-2,160,data.oil_need,typ->data.oil_need,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	if ( data.human_prod )
	{
		// Humanproduktion:
		sprintf ( str,"%d",data.human_prod );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Prod.",42,y,buffer );
		DrawSymbolBig ( SBHuman,95,y-2,160,data.human_prod,typ->data.human_prod,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	// Armor:
	sprintf ( str,"%d",data.armor );
	fonts->OutTextCenter ( str,27,y,buffer );
	fonts->OutText ( "Panzer",42,y,buffer );
	DrawSymbolBig ( SBArmor,95,y-2,160,data.armor,typ->data.armor,buffer );
	dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
	SDL_FillRect ( buffer,&dest,0xFC0000 );
	y+=19;
	// Hitpoints:
	sprintf ( str,"%d",data.max_hit_points );
	fonts->OutTextCenter ( str,27,y,buffer );
	fonts->OutText ( "Treffer",42,y,buffer );
	DrawSymbolBig ( SBHits,95,y-1,160,data.max_hit_points,typ->data.max_hit_points,buffer );
	dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
	SDL_FillRect ( buffer,&dest,0xFC0000 );
	y+=19;
	// Scan:
	if ( data.scan )
	{
		sprintf ( str,"%d",data.scan );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Scan",42,y,buffer );
		DrawSymbolBig ( SBScan,95,y-2,160,data.scan,typ->data.scan,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	// Energieverbrauch:
	if ( data.energy_need )
	{
		sprintf ( str,"%d",data.energy_need );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Verbr.",42,y,buffer );
		DrawSymbolBig ( SBEnergy,95,y-2,160,data.energy_need,typ->data.energy_need,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	// Humanverbrauch:
	if ( data.human_need )
	{
		sprintf ( str,"%d",data.human_need );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Prod.",42,y,buffer );
		DrawSymbolBig ( SBHuman,95,y-2,160,data.human_need,typ->data.human_need,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	// Metallverbrauch:
	if ( data.metal_need )
	{
		sprintf ( str,"%d",data.metal_need );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Verbr.",42,y,buffer );
		DrawSymbolBig ( SBMetal,95,y-2,160,data.metal_need,typ->data.metal_need,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	// Goldverbrauch:
	if ( data.gold_need )
	{
		sprintf ( str,"%d",data.gold_need );
		fonts->OutTextCenter ( str,27,y,buffer );
		fonts->OutText ( "Verbr.",42,y,buffer );
		DrawSymbolBig ( SBGold,95,y-2,160,data.gold_need,typ->data.gold_need,buffer );
		dest.y=y+14;dest.x=13;dest.w=242;dest.h=1;
		SDL_FillRect ( buffer,&dest,0xFC0000 );
		y+=19;
	}
	// Costs:
	sprintf ( str,"%d",data.costs );
	fonts->OutTextCenter ( str,27,y,buffer );
	fonts->OutText ( "Kosten",42,y,buffer );
	DrawSymbolBig ( SBMetal,95,y-2,160,data.costs,typ->data.costs,buffer );
}

// Prüft, ob es Nachbarn gibt:
void cBuilding::CheckNeighbours ( void )
{
	int pos;
	pos=PosX+PosY*game->map->size;
#define CHECK_NEIGHBOUR(a,m) if(a>=0&&a<game->map->size*game->map->size&&abs((a)%game->map->size-PosX)<4&&abs((a)/game->map->size-PosY)<4&&game->map->GO[a].top&&game->map->GO[a].top->owner==owner&&game->map->GO[a].top->SubBase){m=true;}else{m=false;}
	if ( !data.is_big )
	{
		CHECK_NEIGHBOUR ( pos-game->map->size,BaseN )
		CHECK_NEIGHBOUR ( pos+1,BaseE )
		CHECK_NEIGHBOUR ( pos+game->map->size,BaseS )
		CHECK_NEIGHBOUR ( pos-1,BaseW )
	}
	else
	{
		CHECK_NEIGHBOUR ( pos-game->map->size,BaseN )
		CHECK_NEIGHBOUR ( pos-game->map->size+1,BaseBN )
		CHECK_NEIGHBOUR ( pos+2,BaseE )
		CHECK_NEIGHBOUR ( pos+2+game->map->size,BaseBE )
		CHECK_NEIGHBOUR ( pos+game->map->size*2,BaseS )
		CHECK_NEIGHBOUR ( pos+game->map->size*2+1,BaseBS )
		CHECK_NEIGHBOUR ( pos-1,BaseW )
		CHECK_NEIGHBOUR ( pos-1+game->map->size,BaseBW )
	}
}

// Malt die Anschlüsse an das Gebäude:
void cBuilding::DrawConnectors ( SDL_Rect dest )
{
	SDL_Rect scr,tmp;
	int zoom;
	zoom=game->hud->Zoom;
	tmp=dest;
	scr.y=0;
	scr.h=scr.w=zoom;
	if ( BaseN )
	{
		scr.x=zoom;
		SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
		tmp=dest;
	}
	if ( BaseW )
	{
		scr.x=zoom*4;
		SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
		tmp=dest;
	}
	if ( !data.is_big )
	{
		if ( BaseE )
		{
			scr.x=zoom*2;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
		if ( BaseS )
		{
			scr.x=zoom*3;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
	}
	else
	{
		if ( BaseBN )
		{
			scr.x=zoom;
			tmp.x+=zoom;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
		if ( BaseBW )
		{
			scr.x=zoom*4;
			tmp.y+=zoom;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
		if ( BaseE )
		{
			scr.x=zoom*2;
			tmp.x+=zoom;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
		if ( BaseBE )
		{
			scr.x=zoom*2;
			tmp.x+=zoom;
			tmp.y+=zoom;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
		if ( BaseS )
		{
			scr.x=zoom*3;
			tmp.y+=zoom;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
		if ( BaseBS )
		{
			scr.x=zoom*3;
			tmp.y+=zoom;
			tmp.x+=zoom;
			SDL_BlitSurface ( UnitsData.ptr_connector,&scr,buffer,&tmp );
			tmp=dest;
		}
	}
}

// Startet die Arbeit des Gebäudes:
bool cBuilding::StartWork ( bool engine_call )
{
	cBuilding *b;
	int i;

	if ( IsWorking ) return false;
	if ( Disabled ) return false;
	if ( engine_call )
	{
		IsWorking=true;
		EffectAlpha=0;
		return true;
	}

	// Humanverbraucher:
	if ( data.human_need )
	{
		if ( SubBase->HumanProd<SubBase->HumanNeed+data.human_need )
		{
			game->AddMessage ( "Zu wenig Teams, Verbraucher werden ausgeschaltet!" );
			return false;
		}
		SubBase->HumanNeed+=data.human_need;
	}

	// Energiegeneratoren:
	if ( data.energy_prod )
	{
		if ( data.oil_need+SubBase->OilNeed>SubBase->Oil+SubBase->OilProd )
		{
			game->AddMessage ( "Nicht genug Treibstoff zum Einschalten vorhanden!" );
			return false;
		}
		else
		{
			SubBase->EnergyProd+=data.energy_prod;
			SubBase->OilNeed+=data.oil_need;
		}
	}
	// Energieverbraucher:
	else if ( data.energy_need )
	{
		if ( data.energy_need+SubBase->EnergyNeed>SubBase->MaxEnergyProd )
		{
			game->AddMessage ( "Nicht genug Energie zum Einschalten vorhanden!" );
			return false;
		}
		else
		{
			if ( data.energy_need+SubBase->EnergyNeed>SubBase->EnergyProd )
			{
				game->AddMessage ( "Zu wenig Energie, Generator wird eingeschaltet!" );
				for ( i=0;i<SubBase->buildings->Count;i++ )
				{
					b=SubBase->buildings->BuildItems[i];
					if ( !b->data.energy_prod||b->data.is_big ) continue;
					b->StartWork();
					if ( data.energy_need+SubBase->EnergyNeed<=SubBase->EnergyProd ) break;
				}
				for ( i=0;i<SubBase->buildings->Count;i++ )
				{
					if ( data.energy_need+SubBase->EnergyNeed<=SubBase->EnergyProd ) break;
					b=SubBase->buildings->BuildItems[i];
					if ( !b->data.energy_prod ) continue;
					b->StartWork();
				}
				if ( data.energy_need+SubBase->EnergyNeed>SubBase->EnergyProd )
				{
					return false;
				}
				SubBase->EnergyNeed+=data.energy_need;
			}
			else
			{
				SubBase->EnergyNeed+=data.energy_need;
			}
		}
	}

	// Rohstoffverbraucher:
	if ( data.metal_need )
	{
		SubBase->MetalNeed+= min( MetalPerRound, BuildList->BuildListItems[0]->metall_remaining);
	}

	// Goldverbraucher:
	if ( data.gold_need )
	{
		SubBase->GoldNeed+=data.gold_need;
	}

	// Minen:
	if ( data.is_mine )
	{
		SubBase->MetalProd+=MetalProd;
		SubBase->OilProd+=OilProd;
		SubBase->GoldProd+=GoldProd;
	}

	// Allgemeines:
	IsWorking=true;
	EffectAlpha=0;
	StopFXLoop ( game->ObjectStream );
	PlayFX ( typ->Start );
	game->ObjectStream=PlayStram();
	ShowDetails();

	if ( data.can_research ) owner->StartAResearch();
	return true;
}

// Stoppt die Arbeit des Gebäudes:
void cBuilding::StopWork ( bool override,bool engine_call )
{
	if ( !IsWorking ) return;
	if ( engine_call )
	{
		IsWorking=false;
		return;
	}
	// Energiegeneratoren:
	if ( data.energy_prod )
	{
		if ( SubBase->EnergyNeed>SubBase->EnergyProd-data.energy_prod&&!override )
		{
			game->AddMessage ( "Kann Generator nicht abschalten, Energie wird benötigt!" );
			return;
		}
		SubBase->EnergyProd-=data.energy_prod;
		SubBase->OilNeed-=data.oil_need;
	}
	// Energieverbraucher:
	else if ( data.energy_need )
	{
		SubBase->EnergyNeed-=data.energy_need;
	}

	// Rohstoffverbraucher:
	if ( data.metal_need )
	{
		SubBase->MetalNeed-=min( MetalPerRound, BuildList->BuildListItems[0]->metall_remaining);
	}

	// Goldverbraucher:
	if ( data.gold_need )
	{
		SubBase->GoldNeed-=data.gold_need;
	}

	// Humanverbraucher:
	if ( data.human_need )
	{
		SubBase->HumanNeed-=data.human_need;
	}

	// Minen:
	if ( data.is_mine )
	{
		SubBase->MetalProd-=MetalProd;
		SubBase->OilProd-=OilProd;
		SubBase->GoldProd-=GoldProd;
	}

	// Schildgeneratoren:
	if ( data.max_shield )
	{
		data.shield=0;
		owner->CalcShields();
	}

	// Allgemeines:
	IsWorking=false;
	StopFXLoop ( game->ObjectStream );
	PlayFX ( typ->Stop );
	game->ObjectStream=PlayStram();
	ShowDetails();

	if ( data.can_research ) owner->StopAReserach();
}

// Prüft, ob Rohstoffe zu dem GO transferiert werden können:
bool cBuilding::CanTransferTo ( sGameObjects *go )
{
	cBuilding *b;
	cVehicle *v;
	int x,y,i;
	mouse->GetKachel ( &x,&y );
	if ( go->vehicle )
	{
		v=go->vehicle;
		if ( v->owner!=game->ActivePlayer ) return false;
		if ( v->data.can_transport!=data.can_load ) return false;

		for ( i=0;i<SubBase->buildings->Count;i++ )
		{
			b=SubBase->buildings->BuildItems[i];
			if ( b->data.is_big )
			{
				if ( x<b->PosX-1||x>b->PosX+2||y<b->PosY-1||y>b->PosY+2 ) continue;
			}
			else
			{
				if ( x<b->PosX-1||x>b->PosX+1||y<b->PosY-1||y>b->PosY+1 ) continue;
			}
			return true;
		}
		return false;
	}
	else if ( go->top )
	{
		b=go->top;
		if ( b==this ) return false;
		if ( b->SubBase!=SubBase ) return false;
		if ( b->owner!=game->ActivePlayer ) return false;
		if ( data.can_load!=b->data.can_load ) return false;
		return true;
	}
	return false;
}

// Zeigt das Transfer-Menü an:
void cBuilding::ShowTransfer ( sGameObjects *target )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b;//i;
	SDL_Rect scr,dest;
	bool AbbruchPressed=false;
	bool FertigPressed=false;
	bool IncPressed=false;
	bool DecPressed=false;
	bool MouseHot=false;
	int MaxTarget,Target;
	int Transf=0;
	SDL_Surface *img;
	cVehicle *pv=NULL;
	cBuilding *pb=NULL;

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	game->DrawMap();
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	if ( SettingsData.bAlphaEffects ) SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,buffer,NULL );
	dest.x=166;
	dest.y=159;
	dest.w=GraphicsData.gfx_transfer->w;
	dest.h=GraphicsData.gfx_transfer->h;
	SDL_BlitSurface ( GraphicsData.gfx_transfer,NULL,buffer,&dest );

	// Die Images erstellen:
	if ( data.is_big )
	{
		ScaleSurfaceAdv2 ( typ->img_org,typ->img,typ->img_org->w/4,typ->img_org->h/4 );
	}
	else
	{
		ScaleSurfaceAdv2 ( typ->img_org,typ->img,typ->img_org->w/2,typ->img_org->h/2 );
	}
	img=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,typ->img->w,typ->img->h,32,0,0,0,0 );
	SDL_SetColorKey ( img,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( game->ActivePlayer->color,NULL,img,NULL );
	SDL_BlitSurface ( typ->img,NULL,img,NULL );
	dest.x=88+166;
	dest.y=20+159;
	dest.h=img->h;
	dest.w=img->w;
	SDL_BlitSurface ( img,NULL,buffer,&dest );
	SDL_FreeSurface ( img );

	if ( target->vehicle ) pv=target->vehicle;
	pb=target->top;
	if ( pv )
	{
		ScaleSurfaceAdv2 ( pv->typ->img_org[0],pv->typ->img[0],pv->typ->img_org[0]->w/2,pv->typ->img_org[0]->h/2 );
		img=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,pv->typ->img[0]->w,pv->typ->img[0]->h,32,0,0,0,0 );
		SDL_SetColorKey ( img,SDL_SRCCOLORKEY,0xFF00FF );
		SDL_BlitSurface ( game->ActivePlayer->color,NULL,img,NULL );
		SDL_BlitSurface ( pv->typ->img[0],NULL,img,NULL );
	}
	else
	{
		if ( pb->data.is_big )
		{
			ScaleSurfaceAdv2 ( pb->typ->img_org,pb->typ->img,pb->typ->img_org->w/4,pb->typ->img_org->h/4 );
			img=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,pb->typ->img->w,pb->typ->img->h,32,0,0,0,0 );
			SDL_SetColorKey ( img,SDL_SRCCOLORKEY,0xFF00FF );
			SDL_BlitSurface ( game->ActivePlayer->color,NULL,img,NULL );
			SDL_BlitSurface ( pb->typ->img,NULL,img,NULL );
		}
		else
		{
			ScaleSurfaceAdv2 ( pb->typ->img_org,pb->typ->img,pb->typ->img_org->w/2,pb->typ->img_org->h/2 );
			if ( pb->data.has_frames||pb->data.is_connector )
			{
				pb->typ->img->h=pb->typ->img->w=32;
			}
			img=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,pb->typ->img->w,pb->typ->img->h,32,0,0,0,0 );
			SDL_SetColorKey ( img,SDL_SRCCOLORKEY,0xFF00FF );
			if ( !pb->data.is_connector )
			{
				SDL_BlitSurface ( game->ActivePlayer->color,NULL,img,NULL );
			}
			SDL_BlitSurface ( pb->typ->img,NULL,img,NULL );
		}
	}
	dest.x=192+166;
	dest.y=20+159;
	dest.h=dest.w=32;
	SDL_BlitSurface ( img,NULL,buffer,&dest );
	SDL_FreeSurface ( img );

	// Text ausgeben:
	fonts->OutTextCenter ( typ->data.name,102+166,64+159,buffer );
	if ( pv )
	{
		fonts->OutTextCenter ( pv->typ->data.name,208+166,64+159,buffer );
		MaxTarget=pv->data.max_cargo;
		Target=pv->data.cargo;
	}
	else
	{
		fonts->OutTextCenter ( pb->typ->data.name,208+166,64+159,buffer );
		MaxTarget=pb->data.max_cargo;
		Target=pb->data.cargo;
	}

	Transf=MaxTarget;
	// Den Bar malen:
	MakeTransBar ( &Transf,MaxTarget,Target );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		if ( !b ) MouseHot=true;
		if ( !MouseHot ) b=0;
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Abbruch-Button:
		if ( x>=82+166&&x<141+166&&y>=125+159&&y<145+159 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=0;
				scr.y=263;
				dest.w=scr.w=64;
				dest.h=scr.h=23;
				dest.x=82+166;
				dest.y=125+159;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				break;
			}
		}
		else if ( AbbruchPressed )
		{
			scr.x=82;
			scr.y=125;
			dest.w=scr.w=64;
			dest.h=scr.h=23;
			dest.x=82+166;
			dest.y=125+159;
			SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}
		// Fertig-Button:
		if ( x>=174+166&&x<174+56+166&&y>=125+159&&y<125+23+159 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=65;
				scr.y=263;
				dest.w=scr.w=56;
				dest.h=scr.h=23;
				dest.x=174+166;
				dest.y=125+159;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				if ( !Transf ) break;
				if ( pv )
				{
					switch ( data.can_load )
					{
						case TRANS_METAL:
							owner->base->AddMetal ( SubBase,-Transf );
							break;
						case TRANS_OIL:
							owner->base->AddOil ( SubBase,-Transf );
							break;
						case TRANS_GOLD:
							owner->base->AddGold ( SubBase,-Transf );
							break;
					}
					pv->data.cargo+=Transf;
				}
				else
				{
					if ( data.cargo>Transf )
					{
						data.cargo-=Transf;
					}
					else
					{
						Transf=data.cargo;
						data.cargo=0;
					}
					pb->data.cargo+=Transf;
				}
				ShowDetails();
				PlayVoice ( VoiceData.VOITransferDone );
				break;
			}
		}
		else if ( FertigPressed )
		{
			scr.x=174;
			scr.y=125;
			dest.w=scr.w=56;
			dest.h=scr.h=23;
			dest.x=174+166;
			dest.y=125+159;
			SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Inc-Button:
		if ( x>=277+166&&x<277+19+166&&y>=88+159&&y<88+18+159&&b&&!IncPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=257;
			scr.y=177;
			dest.w=scr.w=19;
			dest.h=scr.h=18;
			dest.x=277+166;
			dest.y=88+159;
			Transf++;
			MakeTransBar ( &Transf,MaxTarget,Target );
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncPressed=true;
		}
		else if ( !b&&IncPressed )
		{
			scr.x=277;
			scr.y=88;
			dest.w=scr.w=19;
			dest.h=scr.h=18;
			dest.x=277+166;
			dest.y=88+159;
			SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncPressed=false;
		}
		// Dec-Button:
		if ( x>=16+166&&x<16+19+166&&y>=88+159&&y<88+18+159&&b&&!DecPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=237;
			scr.y=177;
			dest.w=scr.w=19;
			dest.h=scr.h=18;
			dest.x=16+166;
			dest.y=88+159;
			Transf--;
			MakeTransBar ( &Transf,MaxTarget,Target );
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecPressed=true;
		}
		else if ( !b&&DecPressed )
		{
			scr.x=16;
			scr.y=88;
			dest.w=scr.w=19;
			dest.h=scr.h=18;
			dest.x=16+166;
			dest.y=88+159;
			SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecPressed=false;
		}
		// Klick auf den Bar:
		if ( x>=44+166&&x<44+223+166&&y>=86+159&&y<86+20+159&&b&&!LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Transf = Round( ( x -( 44+  166 ) ) * ( MaxTarget / 223.0 ) - Target);
			MakeTransBar ( &Transf,MaxTarget,Target );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
	// Die Images wiederherstellen:
	float newzoom = game->hud->Zoom / 64.0;

	ScaleSurfaceAdv2 ( typ->img_org,typ->img,( int ) ( typ->img_org->w* newzoom ) ,( int ) ( typ->img_org->h* newzoom ) );
	if ( pv )
	{
		ScaleSurfaceAdv2 ( pv->typ->img_org[0],pv->typ->img[0],( int ) ( pv->typ->img_org[0]->w* newzoom ),( int ) ( pv->typ->img_org[0]->h* newzoom ) );
	}
	else
	{
		ScaleSurfaceAdv2 ( pb->typ->img_org,pb->typ->img,( int ) ( pb->typ->img_org->w* newzoom ),( int ) ( pb->typ->img_org->h* newzoom ) );
	}
	Transfer=false;
}

// Malt den Transfer-Bar (len von 0-223):
void cBuilding::DrawTransBar ( int len )
{
	SDL_Rect scr,dest;
	if ( len<0 ) len=0;
	if ( len>223 ) len=223;
	scr.x=44;
	scr.y=90;
	dest.w=scr.w=223;
	dest.h=scr.h=16;
	dest.x=44+166;
	dest.y=90+159;
	SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
	scr.x=156+ ( 223-len );
	dest.w=scr.w=223- ( 223-len );
	if ( data.can_load==TRANS_METAL )
	{
		scr.y=256;
	}
	else if ( data.can_load==TRANS_OIL )
	{
		scr.y=273;
	}
	else
	{
		scr.y=290;
	}
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
}

// Erzeugt den Transfer Balken:
void cBuilding::MakeTransBar ( int *trans,int MaxTarget,int Target )
{
	int cargo,max_cargo;
	SDL_Rect scr,dest;
	char str[10];

	switch ( data.can_load )
	{
		case TRANS_METAL:
			cargo=SubBase->Metal;
			max_cargo=SubBase->MaxMetal;
			break;
		case TRANS_OIL:
			cargo=SubBase->Oil;
			max_cargo=SubBase->MaxOil;
			break;
		case TRANS_GOLD:
			cargo=SubBase->Gold;
			max_cargo=SubBase->MaxGold;
			break;
	}

	// Den trans-Wert berichtigen:
	if ( cargo-*trans<0 )
	{
		*trans+=cargo-*trans;
	}
	if ( Target+*trans<0 )
	{
		*trans-=Target+*trans;
	}
	if ( Target+*trans>MaxTarget )
	{
		*trans-= ( Target+*trans )-MaxTarget;
	}
	if ( cargo-*trans>max_cargo )
	{
		*trans+= ( cargo-*trans )-max_cargo;
	}
	// Die Nummern machen:
	scr.x=4;
	scr.y=30;
	dest.x=4+166;
	dest.y=30+159;
	dest.w=scr.w=78;
	dest.h=scr.h=14;
	SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
	sprintf ( str,"%d",cargo-*trans );
	fonts->OutTextCenter ( str,4+39+166,30+159,buffer );
	scr.x=229;
	dest.x=229+166;
	SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
	sprintf ( str,"%d",Target+*trans );
	fonts->OutTextCenter ( str,229+39+166,30+159,buffer );
	scr.x=141;
	scr.y=15;
	dest.x=141+166;
	dest.y=15+159;
	dest.w=scr.w=29;
	dest.h=scr.h=21;
	SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
	sprintf ( str,"%d",abs ( *trans ) );
	fonts->OutTextCenter ( str,155+166,21+159,buffer );
	// Den Pfeil malen:
	if ( *trans<0 )
	{
		scr.x=122;
		scr.y=263;
		dest.x=143+166;
		dest.y=44+159;
		dest.w=scr.w=30;
		dest.h=scr.h=16;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		scr.x=143;
		scr.y=44;
		dest.x=143+166;
		dest.y=44+159;
		dest.w=scr.w=30;
		dest.h=scr.h=16;
		SDL_BlitSurface ( GraphicsData.gfx_transfer,&scr,buffer,&dest );
	}
	DrawTransBar ( (int) (223 * ( float ) ( Target+*trans ) / MaxTarget ) );
}

// Mal die Exitpoints für ein Vehicle des Übergebenen Typs:
void cBuilding::DrawExitPoints ( sVehicle *typ )
{
	int spx,spy,size;
	spx=GetScreenPosX();
	spy=GetScreenPosY();
	size=game->map->size;
	if ( PosX-1>=0&&PosY-1>=0&&CanExitTo ( PosX-1+ ( PosY-1 ) *size,typ ) ) game->DrawExitPoint ( spx-game->hud->Zoom,spy-game->hud->Zoom );
	if ( PosY-1>=0&&CanExitTo ( PosX+ ( PosY-1 ) *size,typ ) ) game->DrawExitPoint ( spx,spy-game->hud->Zoom );
	if ( PosY-1>=0&&PosX+1<size&&CanExitTo ( PosX+1+ ( PosY-1 ) *size,typ ) ) game->DrawExitPoint ( spx+game->hud->Zoom,spy-game->hud->Zoom );
	if ( PosX+2<size&&PosY-1>=0&&CanExitTo ( PosX+2+ ( PosY-1 ) *size,typ ) ) game->DrawExitPoint ( spx+game->hud->Zoom*2,spy-game->hud->Zoom );

	if ( PosX-1>=0&&CanExitTo ( PosX-1+ ( PosY ) *size,typ ) ) game->DrawExitPoint ( spx-game->hud->Zoom,spy );
	if ( PosX+2<size&&CanExitTo ( PosX+2+ ( PosY ) *size,typ ) ) game->DrawExitPoint ( spx+game->hud->Zoom*2,spy );

	if ( PosX-1>=0&&CanExitTo ( PosX-1+ ( PosY+1 ) *size,typ ) ) game->DrawExitPoint ( spx-game->hud->Zoom,spy+game->hud->Zoom );
	if ( PosX+2<size&&CanExitTo ( PosX+2+ ( PosY+1 ) *size,typ ) ) game->DrawExitPoint ( spx+game->hud->Zoom*2,spy+game->hud->Zoom );

	if ( PosX-1>=0&&PosY+2<size&&CanExitTo ( PosX-1+ ( PosY+2 ) *size,typ ) ) game->DrawExitPoint ( spx-game->hud->Zoom,spy+game->hud->Zoom*2 );
	if ( PosY+2<size&&CanExitTo ( PosX+ ( PosY+2 ) *size,typ ) ) game->DrawExitPoint ( spx,spy+game->hud->Zoom*2 );
	if ( PosY+2<size&&PosX+1<size&&CanExitTo ( PosX+1+ ( PosY+2 ) *size,typ ) ) game->DrawExitPoint ( spx+game->hud->Zoom,spy+game->hud->Zoom*2 );
	if ( PosX+2<size&&PosY+2<size&&CanExitTo ( PosX+2+ ( PosY+2 ) *size,typ ) ) game->DrawExitPoint ( spx+game->hud->Zoom*2,spy+game->hud->Zoom*2 );
}

// Prüft, ob das Vehicle das Gebäude zu dem übergebenen Feld verlassen kann:
bool cBuilding::CanExitTo ( int off,sVehicle *typ )
{
	int boff;
	if ( off<0||off>=game->map->size*game->map->size ) return false;
	if ( abs ( ( off%game->map->size )-PosX ) >8||abs ( ( off/game->map->size )-PosY ) >8 ) return false;
	boff=PosX+PosY*game->map->size;
	if ( ( off>game->map->size*off>game->map->size ) ||
	        ( off>=boff-1-game->map->size&&off<=boff+2-game->map->size ) ||
	        ( off>=boff-1&&off<=boff+2 ) ||
	        ( off>=boff-1+game->map->size&&off<=boff+2+game->map->size ) ||
	        ( off>=boff-1+game->map->size*2&&off<=boff+2+game->map->size*2 ) )
	{}
	else return false;
	if ( ( typ->data.can_drive!=DRIVE_AIR&& ( ( game->map->GO[off].top&&!game->map->GO[off].top->data.is_connector ) ||game->map->GO[off].vehicle||TerrainData.terrain[game->map->Kacheln[off]].blocked ) ) ||
	        ( typ->data.can_drive==DRIVE_AIR&&game->map->GO[off].plane ) ||
	        ( typ->data.can_drive==DRIVE_SEA&&!game->map->IsWater ( off,true ) ) ||
	        ( typ->data.can_drive==DRIVE_LAND&&game->map->IsWater ( off ) ) )
	{
		return false;
	}
	return true;
}

// Prüft, ob das vehicle an der Position geladen werden kann:
bool cBuilding::CanLoad ( int off )
{
	int boff;
	if ( data.cargo==data.max_cargo ) return false;
	boff=PosX+PosY*game->map->size;
	if ( ( off>game->map->size*off>game->map->size ) ||
	        ( off>=boff-1-game->map->size&&off<=boff+2-game->map->size ) ||
	        ( off>=boff-1&&off<=boff+2 ) ||
	        ( off>=boff-1+game->map->size&&off<=boff+2+game->map->size ) ||
	        ( off>=boff-1+game->map->size*2&&off<=boff+2+game->map->size*2 ) )
	{}
	else return false;
	switch ( data.can_load )
	{
		case TRANS_VEHICLES:
			if ( game->map->GO[off].vehicle&&game->map->GO[off].vehicle->owner==owner&&!game->map->GO[off].vehicle->data.is_human&&!game->map->GO[off].vehicle->IsBuilding&&!game->map->GO[off].vehicle->IsClearing&& ( data.build_on_water? ( game->map->GO[off].vehicle->data.can_drive==DRIVE_SEA ) : ( game->map->GO[off].vehicle->data.can_drive!=DRIVE_SEA ) ) ) return true;
			break;
		case TRANS_MEN:
			if ( game->map->GO[off].vehicle&&game->map->GO[off].vehicle->data.is_human&&game->map->GO[off].vehicle->owner==owner&&!game->map->GO[off].vehicle->IsBuilding&&!game->map->GO[off].vehicle->IsClearing ) return true;
			break;
		case TRANS_AIR:
			if ( game->map->GO[off].plane&&game->map->GO[off].plane->owner==owner ) return true;
			break;
	}
	return false;
}

// Läd das Vehicle an der Position ein:
void cBuilding::StoreVehicle ( int off )
{
	cVehicle *v=NULL;

	if ( data.cargo==data.max_cargo ) return;
	if ( data.can_load==TRANS_VEHICLES )
	{
		v=game->map->GO[off].vehicle;
		game->map->GO[off].vehicle=NULL;
	}
	else if ( data.can_load==TRANS_MEN )
	{
		v=game->map->GO[off].vehicle;
		game->map->GO[off].vehicle=NULL;
	}
	else if ( data.can_load==TRANS_AIR )
	{
		v=game->map->GO[off].plane;
		game->map->GO[off].plane=NULL;
	}
	if ( !v ) return;

	if ( v->mjob )
	{
		if ( v->moving||v->rotating||v->Attacking||v->MoveJobActive ) return;
		if ( v->mjob )
		{
			v->mjob->finished=true;
			v->mjob=NULL;
			v->MoveJobActive=false;
		}
	}
	v->Loaded=true;
	StoredVehicles->AddVehicle ( v );
	data.cargo++;
	if ( game->SelectedBuilding&&game->SelectedBuilding==this )
	{
		ShowDetails();
	}
	if ( v==game->SelectedVehicle )
	{
		v->Deselct();
		game->SelectedVehicle=NULL;
	}
	owner->DoScan();
}

// Zeigt den Lagerbildschirm an:
void cBuilding::ShowStorage ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b,i,to;
	SDL_Surface *sf;
	SDL_Rect scr,dest;
	bool FertigPressed=false;
	bool DownPressed=false,DownEnabled=false;
	bool UpPressed=false,UpEnabled=false;
	bool AlleAktivierenEnabled=false;
	bool AlleAufladenEnabled=false;
	bool AlleReparierenEnabled=false;
	bool AlleUpgradenEnabled=false;
	int offset=0;

	LoadActive=false;
	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	if ( data.can_load==TRANS_AIR )
	{
		SDL_BlitSurface ( GraphicsData.gfx_storage,NULL,buffer,NULL );
		to=4;
	}
	else
	{
		scr.x=480;
		scr.y=0;
		scr.w=640-480;
		scr.h=480;
		SDL_BlitSurface ( GraphicsData.gfx_storage,&scr,buffer,&scr );
		SDL_BlitSurface ( GraphicsData.gfx_storage_ground,NULL,buffer,NULL );
		to=6;
	}

	// Alle Buttons machen:
	// Fertig-Button:
	scr.w=dest.w=94;
	scr.h=dest.h=23;
	scr.x=0;scr.y=468;
	dest.x=510;dest.y=371;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	// Down:
	if ( StoredVehicles->Count>to )
	{
		DownEnabled=true;
		scr.x=103;scr.y=452;
		dest.h=scr.h=dest.w=scr.w=25;
		dest.x=530;dest.y=426;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	// Alle Aktivieren:
	scr.x=0;scr.y=376;
	dest.w=scr.w=94;
	dest.h=scr.h=23;
	dest.x=511;dest.y=251;
	if ( StoredVehicles->Count )
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		AlleAktivierenEnabled=true;
		MakeStorageButtonsAlle ( &AlleAufladenEnabled,&AlleReparierenEnabled,&AlleUpgradenEnabled );
	}

	// Vehicles anzeigen:
	DrawStored ( offset );

	// Metallreserven anzeigen:
	ShowStorageMetalBar();

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback=false;

	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Down-Button:
		if ( DownEnabled )
		{
			if ( x>=530&&x<530+25&&y>=426&&y<426+25&&b&&!DownPressed )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				scr.x=530;
				scr.y=426;
				dest.w=scr.w=25;
				dest.h=scr.h=25;
				dest.x=530;
				dest.y=426;

				offset+=to;
				if ( StoredVehicles->Count<=offset+to ) DownEnabled=false;
				DrawStored ( offset );

				SDL_BlitSurface ( GraphicsData.gfx_storage,&scr,buffer,&dest );

				scr.x=130;
				scr.y=452;
				dest.w=scr.w=25;
				dest.h=scr.h=25;
				dest.x=504;
				dest.y=426;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				UpEnabled=true;

				SHOW_SCREEN
				mouse->draw ( false,screen );
				DownPressed=true;
			}
			else if ( !b&&DownPressed&&DownEnabled )
			{
				scr.x=103;
				scr.y=452;
				dest.w=scr.w=25;
				dest.h=scr.h=25;
				dest.x=530;
				dest.y=426;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				DownPressed=false;
			}
		}
		// Up-Button:
		if ( UpEnabled )
		{
			if ( x>=504&&x<504+25&&y>=426&&y<426+25&&b&&!UpPressed )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				scr.x=504;
				scr.y=426;
				dest.w=scr.w=25;
				dest.h=scr.h=25;
				dest.x=504;
				dest.y=426;

				offset-=to;
				if ( offset==0 ) UpEnabled=false;
				DrawStored ( offset );

				SDL_BlitSurface ( GraphicsData.gfx_storage,&scr,buffer,&dest );
				mouse->draw ( false,screen );
				UpPressed=true;

				if ( StoredVehicles->Count>to )
				{
					DownEnabled=true;
					scr.x=103;scr.y=452;
					dest.h=scr.h=dest.w=scr.w=25;
					dest.x=530;dest.y=426;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				}
				SHOW_SCREEN
			}
			else if ( !b&&UpPressed&&UpEnabled )
			{
				scr.x=130;
				scr.y=452;
				dest.w=scr.w=25;
				dest.h=scr.h=25;
				dest.x=504;
				dest.y=426;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				UpPressed=false;
			}
		}
		// Fertig-Button:
		if ( x>=510&&x<510+94&&y>=371&&y<371+23 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.w=dest.w=94;
				scr.h=dest.h=23;
				scr.x=510;scr.y=371;
				dest.x=510;dest.y=371;
				SDL_BlitSurface ( GraphicsData.gfx_storage,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				break;
			}
		}
		else if ( FertigPressed )
		{
			scr.w=dest.w=94;
			scr.h=dest.h=23;
			scr.x=0;scr.y=468;
			dest.x=510;dest.y=371;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Alle Aktivieren:
		if ( x>=511&&x<511+94&&y>=251&&y<251+23&&b&&!LastB&&AlleAktivierenEnabled )
		{
			sVehicle *typ;
			int size;
			PlayFX ( SoundData.SNDMenuButton );
			dest.w=94;dest.h=23;
			dest.x=511;dest.y=251;
			SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			while ( b )
			{
				SDL_PumpEvents();
				b=mouse->GetMouseButton();
			}
			game->OverObject=NULL;
			mouse->MoveCallback=true;

			PlayFX ( SoundData.SNDActivate );
			size=game->map->size;
			for ( i=0;i<StoredVehicles->Count; )
			{
				typ=StoredVehicles->VehicleItems[i]->typ;
				if ( PosX-1>=0&&PosY-1>=0&&CanExitTo ( PosX-1+ ( PosY-1 ) *size,typ ) ) {ExitVehicleTo ( i,PosX-1+ ( PosY-1 ) *size,false );continue;}
				if ( PosY-1>=0&&CanExitTo ( PosX+ ( PosY-1 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+ ( PosY-1 ) *size,false );continue;}
				if ( PosY-1>=0&&CanExitTo ( PosX+1+ ( PosY-1 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+1+ ( PosY-1 ) *size,false );continue;}
				if ( PosX+2<size&&PosY-1>=0&&CanExitTo ( PosX+2+ ( PosY-1 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+2+ ( PosY-1 ) *size,false );continue;}

				if ( PosX-1>=0&&CanExitTo ( PosX-1+ ( PosY ) *size,typ ) ) {ExitVehicleTo ( i,PosX-1+ ( PosY ) *size,false );continue;}
				if ( PosX+2<size&&CanExitTo ( PosX+2+ ( PosY ) *size,typ ) ) {ExitVehicleTo ( i,PosX+2+ ( PosY ) *size,false );continue;}

				if ( PosX-1>=0&&CanExitTo ( PosX-1+ ( PosY+1 ) *size,typ ) ) {ExitVehicleTo ( i,PosX-1+ ( PosY+1 ) *size,false );continue;}
				if ( PosX+2<size&&CanExitTo ( PosX+2+ ( PosY+1 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+2+ ( PosY+1 ) *size,false );continue;}

				if ( PosX-1>=0&&PosY+2<size&&CanExitTo ( PosX-1+ ( PosY+2 ) *size,typ ) ) {ExitVehicleTo ( i,PosX-1+ ( PosY+2 ) *size,false );continue;}
				if ( PosY+2<size&&CanExitTo ( PosX+ ( PosY+2 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+ ( PosY+2 ) *size,false );continue;}
				if ( PosY+2<size&&CanExitTo ( PosX+1+ ( PosY+2 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+1+ ( PosY+2 ) *size,false );continue;}
				if ( PosX+2<size&&PosY+2<size&&CanExitTo ( PosX+2+ ( PosY+2 ) *size,typ ) ) {ExitVehicleTo ( i,PosX+2+ ( PosY+2 ) *size,false );continue;}
				i++;
			}
			return;
		}
		// Alle Aufladen:
		if ( x>=511&&x<511+94&&y>=251+25&&y<251+25+23&&b&&!LastB&&AlleAufladenEnabled )
		{
			PlayFX ( SoundData.SNDMenuButton );
			dest.w=94;dest.h=23;
			dest.x=511;dest.y=251+25;

			for ( i=0;i<StoredVehicles->Count;i++ )
			{
				cVehicle *v;
				v=StoredVehicles->VehicleItems[i];
				if ( v->data.ammo!=v->data.max_ammo )
				{
					v->data.ammo=v->data.max_ammo;
					owner->base->AddMetal ( SubBase,-2 );
					SendUpdateStored ( i );
					if ( SubBase->Metal<2 ) break;
				}
			}
			DrawStored ( offset );
			PlayVoice ( VoiceData.VOILoaded );
			ShowStorageMetalBar();

			SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AlleAufladenEnabled=false;
		}
		// Alle Reparieren:
		if ( x>=511&&x<511+94&&y>=251+25*2&&y<251+25*2+23&&b&&!LastB&&AlleReparierenEnabled )
		{
			PlayFX ( SoundData.SNDMenuButton );
			dest.w=94;dest.h=23;
			dest.x=511;dest.y=251+25*2;

			for ( i=0;i<StoredVehicles->Count;i++ )
			{
				cVehicle *v;
				v=StoredVehicles->VehicleItems[i];
				if ( v->data.hit_points!=v->data.max_hit_points )
				{
					v->data.hit_points=v->data.max_hit_points;
					owner->base->AddMetal ( SubBase,-2 );
					SendUpdateStored ( i );
					if ( SubBase->Metal<2 ) break;
				}
			}
			DrawStored ( offset );
			PlayVoice ( VoiceData.VOIRepaired );
			ShowStorageMetalBar();

			SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AlleReparierenEnabled=false;
		}
		// Alle Upgraden:
		if ( x>=511&&x<511+94&&y>=251+25*3&&y<251+25*3+23&&b&&!LastB&&AlleUpgradenEnabled )
		{
			PlayFX ( SoundData.SNDMenuButton );
			dest.w=94;dest.h=23;
			dest.x=511;dest.y=251+25*3;

			for ( i=0;i<StoredVehicles->Count;i++ )
			{
				cVehicle *v;
				v=StoredVehicles->VehicleItems[i];
				if ( v->data.version!=owner->VehicleData[v->typ->nr].version )
				{

					Update ( v->data,owner->VehicleData[v->typ->nr] )

					v->GenerateName();
					owner->base->AddMetal ( SubBase,-2 );
					SendUpdateStored ( i );
					if ( SubBase->Metal<2 ) break;
				}
			}
			DrawStored ( offset );
			ShowStorageMetalBar();

			SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AlleUpgradenEnabled=false;
		}

		// Buttons unter den Vehicles:
		dest.w=73;dest.h=23;
		if ( data.can_load==TRANS_AIR )
		{
			sf=GraphicsData.gfx_storage;
		}
		else
		{
			sf=GraphicsData.gfx_storage_ground;
		}
		for ( i=0;i<to;i++ )
		{
			cVehicle *v;

			if ( StoredVehicles->Count<=i+offset ) break;
			v=StoredVehicles->VehicleItems[i+offset];
			if ( data.can_load==TRANS_AIR )
			{
				if ( i==0 )
				{
					dest.x=44;
					dest.y=191;
				}
				else if ( i==1 )
				{
					dest.x=270;
					dest.y=191;
				}
				else if ( i==2 )
				{
					dest.x=44;
					dest.y=426;
				}
				else
				{
					dest.x=270;
					dest.y=426;
				}
			}
			else
			{
				if ( i==0 )
				{
					dest.x=8;
					dest.y=191;
				}
				else if ( i==1 )
				{
					dest.x=163;
					dest.y=191;
				}
				else if ( i==2 )
				{
					dest.x=318;
					dest.y=191;
				}
				else if ( i==3 )
				{
					dest.x=8;
					dest.y=426;
				}
				else if ( i==4 )
				{
					dest.x=163;
					dest.y=426;
				}
				else if ( i==5 )
				{
					dest.x=318;
					dest.y=426;
				}
			}
			// Aktivieren:
			if ( x>=dest.x&&x<dest.x+73&&y>=dest.y&&y<dest.y+23&&b&&!LastB )
			{
				PlayFX ( SoundData.SNDMenuButton );
				ActivatingVehicle=true;
				VehicleToActivate=i+offset;
				SDL_BlitSurface ( sf,&dest,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				while ( b )
				{
					SDL_PumpEvents();
					b=mouse->GetMouseButton();
				}
				game->OverObject=NULL;
				mouse->MoveCallback=true;
				return;
			}
			// Reparatur:
			dest.x+=75;
			if ( x>=dest.x&&x<dest.x+73&&y>=dest.y&&y<dest.y+23&&b&&!LastB&&v->data.hit_points<v->data.max_hit_points )
			{
				PlayFX ( SoundData.SNDMenuButton );

				owner->base->AddMetal ( SubBase,-2 );
				v->data.hit_points=v->data.max_hit_points;
				DrawStored ( offset );
				PlayVoice ( VoiceData.VOIRepaired );
				MakeStorageButtonsAlle ( &AlleAufladenEnabled,&AlleReparierenEnabled,&AlleUpgradenEnabled );

				SDL_BlitSurface ( sf,&dest,buffer,&dest );
				ShowStorageMetalBar();
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SendUpdateStored ( i+offset );
			}
			// Aufladen:
			dest.x-=75;
			dest.y+=25;
			if ( x>=dest.x&&x<dest.x+73&&y>=dest.y&&y<dest.y+23&&b&&!LastB&&v->data.ammo<v->data.max_ammo )
			{
				PlayFX ( SoundData.SNDMenuButton );

				owner->base->AddMetal ( SubBase,-2 );
				v->data.ammo=v->data.max_ammo;
				DrawStored ( offset );
				PlayVoice ( VoiceData.VOILoaded );
				MakeStorageButtonsAlle ( &AlleAufladenEnabled,&AlleReparierenEnabled,&AlleUpgradenEnabled );

				SDL_BlitSurface ( sf,&dest,buffer,&dest );
				ShowStorageMetalBar();
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SendUpdateStored ( i+offset );
			}
			// Upgrade:
			dest.x+=75;
			if ( x>=dest.x&&x<dest.x+73&&y>=dest.y&&y<dest.y+23&&b&&!LastB&&v->data.version!=owner->VehicleData[v->typ->nr].version )
			{
				PlayFX ( SoundData.SNDMenuButton );

				Update ( v->data,owner->VehicleData[v->typ->nr] )
				v->GenerateName();
				owner->base->AddMetal ( SubBase,-2 );
				DrawStored ( offset );
				MakeStorageButtonsAlle ( &AlleAufladenEnabled,&AlleReparierenEnabled,&AlleUpgradenEnabled );

				SDL_BlitSurface ( sf,&dest,buffer,&dest );
				ShowStorageMetalBar();
				SHOW_SCREEN
				mouse->draw ( false,screen );
				SendUpdateStored ( i+offset );
			}
		}

		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
	mouse->MoveCallback=true;
}

// Malt alle Bilder der geladenen Vehicles:
void cBuilding::DrawStored ( int off )
{
	SDL_Rect scr,dest;
	SDL_Surface *sf;
	cVehicle *v;
	int i,to;

	if ( data.can_load==TRANS_AIR )
	{
		to=4;
		sf=GraphicsData.gfx_storage;
	}
	else
	{
		to=6;
		sf=GraphicsData.gfx_storage_ground;
	}

	for ( i=0;i<to;i++ )
	{
		if ( i+off>=StoredVehicles->Count )
		{
			v=NULL;
		}
		else
		{
			v=StoredVehicles->VehicleItems[i+off];
		}

		// Das Bild malen:
		if ( data.can_load==TRANS_AIR )
		{
			if ( i==0 ) {dest.x=17;dest.y=9;}
			else if ( i==1 ) {dest.x=243;dest.y=9;}
			else if ( i==2 ) {dest.x=17;dest.y=244;}
			else{dest.x=243;dest.y=244;}
			dest.w=200;dest.h=128;
		}
		else
		{
			if ( i==0 ) {dest.x=17;dest.y=9;}
			else if ( i==1 ) {dest.x=172;dest.y=9;}
			else if ( i==2 ) {dest.x=327;dest.y=9;}
			else if ( i==3 ) {dest.x=17;dest.y=244;}
			else if ( i==4 ) {dest.x=172;dest.y=244;}
			else if ( i==5 ) {dest.x=327;dest.y=244;}
			dest.w=128;dest.h=128;
		}

		SDL_BlitSurface ( sf,&dest,buffer,&dest );
		if ( v )
		{
			SDL_BlitSurface ( v->typ->storage,NULL,buffer,&dest );
			// Den Namen ausgeben:
			if ( fonts->GetTextLen ( ( char * ) v->name.c_str() ) >dest.w-10 )
			{
				char str[100];
				strcpy ( str,v->name.c_str() );
				str[strlen ( str )-1]=0;
				while ( fonts->GetTextLen ( str ) >dest.w-10 )
				{
					str[strlen ( str )-1]=0;
				}
				fonts->OutText ( str,dest.x+5,dest.y+5,buffer );
			}
			else
			{
				fonts->OutText ( ( char * ) v->name.c_str(),dest.x+5,dest.y+5,buffer );
			}
			if ( v->data.version!=v->owner->VehicleData[v->typ->nr].version )
			{
				fonts->OutTextSmall ( "(veraltet)",dest.x+5,dest.y+15,ClWhite,buffer );
			}
		}
		else if ( data.build_on_water )
		{
			SDL_BlitSurface ( GraphicsData.gfx_edock,NULL,buffer,&dest );
		}

		// Die Buttons malen:
		// Aktivieren:
		if ( to==4 )
		{
			dest.x+=27;
		}
		else
		{
			dest.x-=9;
		}
		dest.y+=182;
		scr.w=dest.w=73;
		scr.h=dest.h=23;
		if ( v )
		{
			scr.x=156;
			scr.y=431;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}
		else
		{
			SDL_BlitSurface ( sf,&dest,buffer,&dest );
		}
		// Reparieren:
		dest.x+=75;
		if ( v&&v->data.hit_points!=v->data.max_hit_points&&SubBase->Metal>=2 )
		{
			scr.x=304;
			scr.y=431;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}
		else
		{
			SDL_BlitSurface ( sf,&dest,buffer,&dest );
		}
		// Aufladen:
		dest.x-=75;
		dest.y+=25;
		if ( v&&v->data.ammo!=v->data.max_ammo&&SubBase->Metal>=2 )
		{
			scr.x=230;
			scr.y=431;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}
		else
		{
			SDL_BlitSurface ( sf,&dest,buffer,&dest );
		}
		// Upgrade:
		dest.x+=75;
		if ( v&&v->data.version!=v->owner->VehicleData[v->typ->nr].version&&SubBase->Metal>=2 )
		{
			scr.x=156;
			scr.y=455;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}
		else
		{
			SDL_BlitSurface ( sf,&dest,buffer,&dest );
		}

		// Die zusätzlichen Infos anzeigen:
		dest.x-=66;
		dest.y-=69-6;
		dest.w=128;
		dest.h=30;
		SDL_BlitSurface ( sf,&dest,buffer,&dest );
		dest.x+=6;
		if ( v )
		{
			// Die Hitpoints anzeigen:
			DrawNumber ( dest.x+13,dest.y,v->data.hit_points,v->data.max_hit_points,buffer );
			fonts->OutTextSmall ( "Treffer",dest.x+27,dest.y,ClWhite,buffer );
			DrawSymbol ( SHits,dest.x+60,dest.y,58,v->data.hit_points,v->data.max_hit_points,buffer );
			// Die Munition anzeigen:
			if ( v->data.can_attack )
			{
				dest.y+=15;
				DrawNumber ( dest.x+13,dest.y,v->data.ammo,v->data.max_ammo,buffer );
				fonts->OutTextSmall ( "Munni",dest.x+27,dest.y,ClWhite,buffer );
				DrawSymbol ( SAmmo,dest.x+60,dest.y,58,v->data.ammo,v->data.max_ammo,buffer );
			}
		}
	}
}

// Zeigt den Metallbalken im Storage-Menü an:
void cBuilding::ShowStorageMetalBar ( void )
{
	SDL_Rect scr,dest;
	char str[50];
	scr.x=490;scr.y=80;
	scr.w=136;scr.h=145;
	SDL_BlitSurface ( GraphicsData.gfx_storage,&scr,buffer,&scr );

	scr.x=135;
	scr.y=335;
	dest.x=546;
	dest.y=106;
	scr.w=dest.w=20;
	scr.h=dest.h=115* ( int ) ( SubBase->Metal/ ( float ) SubBase->MaxMetal );
	dest.y+=115- ( 115* ( int ) ( SubBase->Metal/ ( float ) SubBase->MaxMetal ) );
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );

	sprintf ( str,"Metall: %d",SubBase->Metal );
	fonts->OutTextCenter ( str,557,86,buffer );
}

// Läd ein Vehicle aus:
void cBuilding::ExitVehicleTo ( int nr,int off,bool engine_call )
{
	cVehicle *ptr;
	if ( !StoredVehicles||StoredVehicles->Count<=nr ) return;

	ptr=StoredVehicles->VehicleItems[nr];
	StoredVehicles->DeleteVehicle ( nr );
	data.cargo--;
	ActivatingVehicle=false;
	if ( this==game->SelectedBuilding )
	{
		ShowDetails();
	}
	if ( ptr->data.can_drive==DRIVE_AIR )
	{
		game->map->GO[off].plane=ptr;
	}
	else
	{
		game->map->GO[off].vehicle=ptr;
	}
	ptr->PosX=off%game->map->size;
	ptr->PosY=off/game->map->size;
	ptr->Loaded=false;
	ptr->InWachRange();

	owner->DoScan();
}

void cBuilding::MakeStorageButtonsAlle ( bool *AlleAufladenEnabled,bool *AlleReparierenEnabled,bool *AlleUpgradenEnabled )
{
	SDL_Rect scr,dest;
	int i;
	scr.x=0;scr.y=376+23;
	dest.w=scr.w=94;
	dest.h=scr.h=23;
	dest.x=511;dest.y=251+25;

	*AlleAufladenEnabled=false;
	*AlleReparierenEnabled=false;
	*AlleUpgradenEnabled=false;

	if ( SubBase->Metal>=2 )
	{
		for ( i=0;i<StoredVehicles->Count;i++ )
		{
			if ( StoredVehicles->VehicleItems[i]->data.ammo!=StoredVehicles->VehicleItems[i]->data.max_ammo )
			{
				*AlleAufladenEnabled=true;
			}
			if ( StoredVehicles->VehicleItems[i]->data.hit_points!=StoredVehicles->VehicleItems[i]->data.max_hit_points )
			{
				*AlleReparierenEnabled=true;
			}
			if ( StoredVehicles->VehicleItems[i]->data.version!=StoredVehicles->VehicleItems[i]->owner->VehicleData[StoredVehicles->VehicleItems[i]->typ->nr].version&&SubBase->Metal>=2 )
			{
				*AlleUpgradenEnabled=true;
			}
		}
	}
	// Alle Aufladen:
	if ( *AlleAufladenEnabled )
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
	}
	scr.y+=23;
	dest.y+=25;
	// Alle Reparieren:
	if ( *AlleReparierenEnabled )
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
	}
	scr.y+=23;
	dest.y+=25;
	// Alle Upgraden:
	if ( *AlleUpgradenEnabled )
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_storage,&dest,buffer,&dest );
	}
}

// Bring eine Mine zur Detonation:
void cBuilding::Detonate ( void )
{
	int off;
	off=PosX+PosY*game->map->size;

	// Das Ziel beschädigen/zerstören:
	game->engine->AddAttackJob ( off,off,false,false,false,true );
}

// Zeigt den Researchbildschirm an:
void cBuilding::ShowResearch ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b;
	SDL_Rect scr,dest;
	bool AbbruchPressed=false;
	bool FertigPressed=false;

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	game->DrawMap();
	SDL_BlitSurface ( GraphicsData.gfx_hud,NULL,buffer,NULL );
	if ( SettingsData.bAlphaEffects ) SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,buffer,NULL );
	dest.x=140;
	dest.y=74;
	dest.w=GraphicsData.gfx_research->w;
	dest.h=GraphicsData.gfx_research->h;
	SDL_BlitSurface ( GraphicsData.gfx_research,NULL,buffer,&dest );

	// Schieber malen:
	ShowResearchSchieber();

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Abbruch-Button:
		if ( x>=92+140&&x<92+140+75&&y>=294+74&&y<294+74+26 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=0;
				scr.y=492;
				dest.w=scr.w=75;
				dest.h=scr.h=26;
				dest.x=92+140;
				dest.y=294+74;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				break;
			}
		}
		else if ( AbbruchPressed )
		{
			scr.x=92;
			scr.y=294;
			dest.w=scr.w=75;
			dest.h=scr.h=26;
			dest.x=92+140;
			dest.y=294+74;
			SDL_BlitSurface ( GraphicsData.gfx_research,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}
		// Fertig-Button:
		if ( x>=194+140&&x<194+140+75&&y>=294+74&&y<294+26+74 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=76;
				scr.y=492;
				dest.w=scr.w=75;
				dest.h=scr.h=26;
				dest.x=194+140;
				dest.y=294+74;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				break;
			}
		}
		else if ( FertigPressed )
		{
			scr.x=194;
			scr.y=294;
			dest.w=scr.w=75;
			dest.h=scr.h=26;
			dest.x=194+140;
			dest.y=294+74;
			SDL_BlitSurface ( GraphicsData.gfx_research,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Die Schieber machen:
		if ( b&&!LastB ) MakeResearchSchieber ( x,y );
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
}

// Zeigt die Schieber an:
void cBuilding::ShowResearchSchieber ( void )
{
	SDL_Rect scr,dest;
	char str[20];
	int i;

	for ( i=0;i<8;i++ )
	{
		scr.x=20;
		scr.y=70+i*28;
		dest.x=20+140;
		dest.y=70+74+i*28;
		dest.w=scr.w=316;
		dest.h=scr.h=18;
		SDL_BlitSurface ( GraphicsData.gfx_research,&scr,buffer,&dest );

		// Texte ausgeben:
		sprintf ( str,"%d",owner->ResearchTechs[i].working_on );
		fonts->OutTextCenter ( str,dest.x+21+2,dest.y+3,buffer );
		sprintf ( str,"+%.0f%",owner->ResearchTechs[i].level*100 );
		fonts->OutTextCenter ( str,258+140,dest.y+3,buffer );
		if ( owner->ResearchTechs[i].working_on )
		{
			sprintf ( str,"%d", ( int ) ceil ( owner->ResearchTechs[i].RoundsRemaining/ ( double ) owner->ResearchTechs[i].working_on ) );
			fonts->OutTextCenter ( str,313+140,dest.y+3,buffer );
		}

		// Den Pfeil nach links:
		if ( owner->ResearchTechs[i].working_on==0 )
		{
			dest.w=scr.w=19;
			dest.h=scr.h=18;
			scr.x=237;
			scr.y=177;
			dest.x=71+140;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}

		// Den Pfeil nach rechts:
		if ( owner->UnusedResearch<=0 )
		{
			dest.w=scr.w=19;
			dest.h=scr.h=18;
			scr.x=257;
			scr.y=177;
			dest.x=143+140;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}

		// Die Schieber malen:
		dest.w=scr.w=14;
		dest.h=scr.h=17;
		scr.x=412;
		scr.y=46;
		dest.x=90+140+36* ( int ) ( ( ( float ) ( owner->ResearchTechs[i].working_on ) /owner->ResearchCount ) );
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
}

// Prüft, ob die Schieber geändert wurden:
void cBuilding::MakeResearchSchieber ( int x,int y )
{
	bool changed=false;
	int i;

	for ( i=0;i<8;i++ )
	{
		// Den Pfeil nach links:
		if ( x>=71+140&&x<71+140+19&&y>=70+74+i*28&&y<70+74+i*28+18&&owner->ResearchTechs[i].working_on )
		{
			owner->ResearchTechs[i].working_on--;
			owner->UnusedResearch++;
			changed=true;
		}

		// Den Pfeil nach rechts:
		if ( x>=143+140&&x<143+140+19&&y>=70+74+i*28&&y<70+74+i*28+18&&owner->UnusedResearch>0 )
		{
			owner->ResearchTechs[i].working_on++;

			owner->UnusedResearch--;
			changed=true;
		}
	}
	if ( changed )
	{
		ShowResearchSchieber();
		SHOW_SCREEN
		mouse->draw ( false,screen );
	}
}

// Struktur für die Upgrade-List:
struct sUpgradeStruct
{
	SDL_Surface *sf;
	bool vehicle;
	int id;
	sUpgrades upgrades[8];
};
// Zeigt den Upgradeschirm an:
void cBuilding::ShowUpgrade ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b,i,k;
	SDL_Rect scr,dest;
	bool AbbruchPressed=false;
	bool FertigPressed=false;
	bool Beschreibung=SettingsData.bShowDescription;
	bool DownPressed=false;
	bool UpPressed=false;
	TList *images,*selection;
	int selected=0,offset=0;
	int StartCredits=owner->Credits;

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,NULL,buffer,NULL );

	// Der Haken:
	if ( Beschreibung )
	{
		dest.x=scr.x=291;
		dest.y=scr.y=264;
		dest.w=scr.w=17;
		dest.h=scr.h=17;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
	}
	else
	{
		scr.x=393;
		scr.y=46;
		dest.x=291;
		dest.y=264;
		dest.w=scr.w=18;
		dest.h=scr.h=17;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}

	// Die Images erstellen:
	images=new TList;
	float newzoom = ( game->hud->Zoom/64.0 );
	for ( i=0;i<UnitsData.vehicle_anz;i++ )
	{
		sUpgradeStruct *n;
		SDL_Surface *sf;
		ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[0],UnitsData.vehicle[i].img[0],UnitsData.vehicle[i].img_org[0]->w/2,UnitsData.vehicle[i].img_org[0]->h/2 );
		sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,UnitsData.vehicle[i].img[0]->w,UnitsData.vehicle[i].img[0]->h,32,0,0,0,0 );
		SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
		SDL_BlitSurface ( game->ActivePlayer->color,NULL,sf,NULL );
		SDL_BlitSurface ( UnitsData.vehicle[i].img[0],NULL,sf,NULL );
		ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[0],UnitsData.vehicle[i].img[0],( int ) ( UnitsData.vehicle[i].img_org[0]->w* newzoom ),( int ) ( UnitsData.vehicle[i].img_org[0]->h* newzoom ) );
		n=new sUpgradeStruct;
		n->sf=sf;
		n->id=i;
		n->vehicle=true;
		MakeUpgradeSliderVehicle ( n->upgrades,i );
		images->AddUpgraStr ( n );
	}
	for ( i=0;i<UnitsData.building_anz;i++ )
	{
		sUpgradeStruct *n;
		SDL_Surface *sf;
		if ( UnitsData.building[i].data.is_big )
		{
			ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img,UnitsData.building[i].img_org->w/4,UnitsData.building[i].img_org->h/4 );
		}
		else
		{
			ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img,UnitsData.building[i].img_org->w/2,UnitsData.building[i].img_org->h/2 );
		}
		sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,UnitsData.building[i].img->w,UnitsData.building[i].img->h,32,0,0,0,0 );
		SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
		if ( !UnitsData.building[i].data.is_connector&&!UnitsData.building[i].data.is_road )
		{
			SDL_BlitSurface ( game->ActivePlayer->color,NULL,sf,NULL );
		}
		else
		{
			SDL_FillRect ( sf,NULL,0xFF00FF );
		}
		SDL_BlitSurface ( UnitsData.building[i].img,NULL,sf,NULL );
		ScaleSurfaceAdv2 ( UnitsData.building[i].img_org,UnitsData.building[i].img,( int ) (UnitsData.building[i].img_org->w* newzoom ),( int ) (UnitsData.building[i].img_org->h* newzoom ) );
		n=new sUpgradeStruct;
		n->sf=sf;
		n->id=i;
		n->vehicle=false;
		MakeUpgradeSliderBuilding ( n->upgrades,i );
		images->AddUpgraStr ( n );
	}
	selection=new TList;
	CreateUpgradeList ( selection,images,&selected,&offset );
	ShowUpgradeList ( selection,selected,offset,Beschreibung );
	MakeUpgradeSubButtons();

	// Credits anzeigen:
	ShowGoldBar ( StartCredits );

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback=false;

	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Down-Button:
		if ( x>=491&&x<491+18&&y>=386&&y<386+17&&b&&!DownPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=491;
			dest.y=386;
			if ( offset<selection->Count-9 )
			{
				offset++;
				if ( selected<offset ) selected=offset;
				ShowUpgradeList ( selection,selected,offset,Beschreibung );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=true;
		}
		else if ( !b&&DownPressed )
		{
			scr.x=491;
			scr.y=386;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=491;
			dest.y=386;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=false;
		}
		// Up-Button:
		if ( x>=470&&x<470+18&&y>=386&&y<386+17&&b&&!UpPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=470;
			dest.y=386;
			if ( offset!=0 )
			{
				offset--;
				if ( selected>=offset+9 ) selected=offset+8;
				ShowUpgradeList ( selection,selected,offset,Beschreibung );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=true;
		}
		else if ( !b&&UpPressed )
		{
			scr.x=470;
			scr.y=386;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=470;
			dest.y=386;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=false;
		}
		// Abbruch-Button:
		if ( x>=357&&x<357+61&&y>=452&&y<452+23 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=364;
				scr.y=231;
				dest.w=scr.w=62;
				dest.h=scr.h=24;
				dest.x=357;
				dest.y=452;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				// Alle Upgrades zurücksetzen:
				owner->Credits=StartCredits;
				for ( i=0;i<images->Count;i++ )
				{
					sUpgradeStruct *ptr;
					ptr=images->UpgraStrItems[i];
					for ( k=0;k<8;k++ )
					{
						if ( !ptr->upgrades[k].active||!ptr->upgrades[k].Purchased ) continue;
						* ( ptr->upgrades[k].value ) =ptr->upgrades[k].StartValue;
					}
				}
				break;
			}
		}
		else if ( AbbruchPressed )
		{
			scr.x=357;
			scr.y=452;
			dest.w=scr.w=62;
			dest.h=scr.h=24;
			dest.x=357;
			dest.y=452;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}
		// Fertig-Button:
		if ( x>=447&&x<447+54&&y>=452&&y<452+23 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=308;
				scr.y=231;
				dest.w=scr.w=55;
				dest.h=scr.h=24;
				dest.x=447;
				dest.y=452;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				// Alle Upgrades durchführen:
				for ( i=0;i<images->Count;i++ )
				{
					bool up=false;
					sUpgradeStruct *ptr;
					ptr=images->UpgraStrItems[i];
					for ( k=0;k<8;k++ )
					{
						if ( !ptr->upgrades[k].active||!ptr->upgrades[k].Purchased ) continue;
						if ( !ptr->vehicle )
						{
							owner->BuildingData[ptr->id].version++;
						}
						up=true;
						break;
					}
				}
				break;
			}
		}
		else if ( FertigPressed )
		{
			scr.x=447;
			scr.y=452;
			dest.w=scr.w=55;
			dest.h=scr.h=24;
			dest.x=447;
			dest.y=452;
			SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Beschreibung Haken:
		if ( x>=292&&x<292+16&&y>=265&&y<265+15&&b&&!LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Beschreibung=!Beschreibung;
			SettingsData.bShowDescription=Beschreibung;
			if ( Beschreibung )
			{
				dest.x=scr.x=291;
				dest.y=scr.y=264;
				dest.w=scr.w=17;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
			}
			else
			{
				scr.x=393;
				scr.y=46;
				dest.x=291;
				dest.y=264;
				dest.w=scr.w=18;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			}
			ShowUpgradeList ( selection,selected,offset,Beschreibung );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Klick in die Liste:
		if ( x>=490&&x<490+70&&y>=60&&y<60+315&&b&&!LastB )
		{
			int nr;
			nr= ( y-60 ) / ( 32+2 );
			if ( selection->Count<9 )
			{
				if ( nr>=selection->Count ) nr=-1;
			}
			else
			{
				if ( nr>=10 ) nr=-1;
				nr+=offset;
			}
			if ( nr!=-1 )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				selected=nr;
				ShowUpgradeList ( selection,selected,offset,Beschreibung );
				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Klick auf einen Upgrade-Slider:
		if ( b&&!LastB&&x>=283&&x<301+18&&selection->Count )
		{
			sUpgradeStruct *ptr=selection->UpgraStrItems[selected];
			for ( i=0;i<8;i++ )
			{
				if ( !ptr->upgrades[i].active ) continue;
				if ( ptr->upgrades[i].Purchased&&x<283+18&&y>=293+i*19&&y<293+i*19+19 )
				{
					int variety;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "hitpoints" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "armor" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "ammo" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "damage" ) == 0 )
						variety = 0;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "speed" ) == 0 )
						variety = 1;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "shots" ) == 0 )
						variety = 2;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "range" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "scan" ) == 0 )
						variety = 3;
					* ( ptr->upgrades[i].value )-=CalcSteigerung ( ptr->upgrades[i].StartValue,variety );
					ptr->upgrades[i].NextPrice=CalcPrice ( * ( ptr->upgrades[i].value ),ptr->upgrades[i].StartValue,variety );
					owner->Credits+=ptr->upgrades[i].NextPrice;
					ptr->upgrades[i].Purchased--;

					PlayFX ( SoundData.SNDObjectMenu );
					ShowUpgradeList ( selection,selected,offset,Beschreibung );
					ShowGoldBar ( StartCredits );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					break;
				}
				else if ( ptr->upgrades[i].NextPrice<=owner->Credits&&x>=301&&y>=293+i*19&&y<293+i*19+19 )
				{
					int variety;
					owner->Credits-=ptr->upgrades[i].NextPrice;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "hitpoints" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "armor" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "ammo" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "damage" ) == 0 )
						variety = 0;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "speed" ) == 0 )
						variety = 1;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "shots" ) == 0 )
						variety = 2;
					if ( strcmp ( ptr->upgrades[i].name.c_str(), "range" ) == 0 || strcmp ( ptr->upgrades[i].name.c_str(), "scan" ) == 0 )
						variety = 3;
					* ( ptr->upgrades[i].value ) +=CalcSteigerung ( ptr->upgrades[i].StartValue,variety );
					ptr->upgrades[i].NextPrice=CalcPrice ( * ( ptr->upgrades[i].value ),ptr->upgrades[i].StartValue,variety );
					ptr->upgrades[i].Purchased++;

					PlayFX ( SoundData.SNDObjectMenu );
					ShowUpgradeList ( selection,selected,offset,Beschreibung );
					ShowGoldBar ( StartCredits );
					SHOW_SCREEN
					mouse->draw ( false,screen );
					break;
				}
			}
		}
		// Klick auf einen der SubSelctionButtons:
		if ( b&&!LastB&&x>=467&&x<467+32&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			game->UpShowTank=!game->UpShowTank;
			CreateUpgradeList ( selection,images,&selected,&offset );
			ShowUpgradeList ( selection,selected,offset,Beschreibung );
			MakeUpgradeSubButtons();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!LastB&&x>=467+33&&x<467+32+33&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			game->UpShowPlane=!game->UpShowPlane;
			CreateUpgradeList ( selection,images,&selected,&offset );
			ShowUpgradeList ( selection,selected,offset,Beschreibung );
			MakeUpgradeSubButtons();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!LastB&&x>=467+33*2&&x<467+32+33*2&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			game->UpShowShip=!game->UpShowShip;
			CreateUpgradeList ( selection,images,&selected,&offset );
			ShowUpgradeList ( selection,selected,offset,Beschreibung );
			MakeUpgradeSubButtons();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!LastB&&x>=467+33*3&&x<467+32+33*3&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			game->UpShowBuild=!game->UpShowBuild;
			CreateUpgradeList ( selection,images,&selected,&offset );
			ShowUpgradeList ( selection,selected,offset,Beschreibung );
			MakeUpgradeSubButtons();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		else if ( b&&!LastB&&x>=467+33*4&&x<467+32+33*4&&y>=411&&y<411+31 )
		{
			PlayFX ( SoundData.SNDHudSwitch );
			game->UpShowTNT=!game->UpShowTNT;
			CreateUpgradeList ( selection,images,&selected,&offset );
			ShowUpgradeList ( selection,selected,offset,Beschreibung );
			MakeUpgradeSubButtons();
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
	// Alles Images löschen:
	while ( images->Count )
	{
		sUpgradeStruct *ptr;
		ptr=images->UpgraStrItems[0];
		SDL_FreeSurface ( ptr->sf );
		delete ptr;
		images->DeleteUpgraStr ( 0 );
	}
	delete images;
	delete selection;
	mouse->MoveCallback=true;
}

// Zeigt die Liste mit den Images an:
void cBuilding::ShowUpgradeList ( TList *list,int selected,int offset,bool beschreibung )
{
	sUpgradeStruct *ptr;
	SDL_Rect dest,scr,text;
	char str[100];
	int i,t,k;
	scr.x=479;scr.y=52;
	scr.w=150;scr.h=330;
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=490;dest.y=58;
	dest.w=32;dest.h=32;
	text.x=530;text.y=70;
	if ( list->Count==0 )
	{
		scr.x=0;scr.y=0;
		scr.w=316;scr.h=256;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
		scr.x=11;scr.y=290;
		scr.w=346;scr.h=176;
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&scr );
		return;
	}
	for ( i=offset;i<list->Count;i++ )
	{
		if ( i>=offset+9 ) break;
		// Das Bild malen:
		ptr=list->UpgraStrItems[i];
		SDL_BlitSurface ( ptr->sf,&scr,buffer,&dest );
		// Ggf noch Rahmen drum:
		if ( selected==i )
		{
			SDL_Rect tmp;
			tmp=dest;
			tmp.x-=4;
			tmp.y-=4;
			tmp.h=1;
			tmp.w=8;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x+=30;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y+=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x-=30;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y=dest.y-4;
			tmp.w=1;
			tmp.h=8;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x+=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.y+=31;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			tmp.x-=38;
			SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			// Das Bild neu malen:
			tmp.x=11;tmp.y=13;
			if ( ptr->vehicle )
			{
				tmp.w=UnitsData.vehicle[ptr->id].info->w;
				tmp.h=UnitsData.vehicle[ptr->id].info->h;
				SDL_BlitSurface ( UnitsData.vehicle[ptr->id].info,NULL,buffer,&tmp );
			}
			else
			{
				tmp.w=UnitsData.building[ptr->id].info->w;
				tmp.h=UnitsData.building[ptr->id].info->h;
				SDL_BlitSurface ( UnitsData.building[ptr->id].info,NULL,buffer,&tmp );
			}
			// Ggf die Beschreibung ausgeben:
			if ( beschreibung )
			{
				tmp.x+=10;tmp.y+=10;
				tmp.w-=20;tmp.h-=20;
				if ( ptr->vehicle )
				{
					fonts->OutTextBlock ( UnitsData.vehicle[ptr->id].text,tmp,buffer );
				}
				else
				{
					fonts->OutTextBlock ( UnitsData.building[ptr->id].text,tmp,buffer );
				}
			}
			// Die Details anzeigen:
			{
				cVehicle *tv;
				cBuilding *tb;
				tmp.x=11;
				tmp.y=290;
				tmp.w=346;
				tmp.h=176;
				SDL_BlitSurface ( GraphicsData.gfx_upgrade,&tmp,buffer,&tmp );
				if ( ptr->vehicle )
				{
					tv=new cVehicle ( UnitsData.vehicle+ptr->id,game->ActivePlayer );
					tv->ShowBigDetails();
					delete tv;
				}
				else
				{
					tb=new cBuilding ( UnitsData.building+ptr->id,game->ActivePlayer,NULL );
					tb->ShowBigDetails();
					delete tb;
				}
			}
			// Die Texte anzeigen/Slider machen:
			for ( k=0;k<8;k++ )
			{
				SDL_Rect scr,dest;
				if ( !ptr->upgrades[k].active ) continue;
				sprintf ( str,"%d",ptr->upgrades[k].NextPrice );
				fonts->OutText ( str,322,296+k*19,buffer );

				if ( ptr->upgrades[k].Purchased )
				{
					scr.x=380;scr.y=256;
					dest.w=scr.w=18;dest.h=scr.h=17;
					dest.x=283;dest.y=293+k*19;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				}
				if ( ptr->upgrades[k].NextPrice<=owner->Credits )
				{
					scr.x=399;scr.y=256;
					dest.w=scr.w=18;dest.h=scr.h=17;
					dest.x=301;dest.y=293+k*19;
					SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				}
			}
		}
		// Text ausgeben:
		t=0;
		str[0]=0;
		if ( ptr->vehicle )
		{
			while ( UnitsData.vehicle[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <85 )
			{
				str[t]=UnitsData.vehicle[ptr->id].data.name[t];str[++t]=0;
			}
		}
		else
		{
			while ( UnitsData.building[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <85 )
			{
				str[t]=UnitsData.building[ptr->id].data.name[t];str[++t]=0;
			}
		}
		str[t]='.';
		str[t+1]=0;
		fonts->OutText ( str,text.x,text.y,buffer );
		text.y+=32+2;
		dest.y+=32+2;
	}
}

// Zeigt die Anzahl der Credits an:
void cBuilding::ShowGoldBar ( int StartCredits )
{
	char str[50];
	SDL_Rect scr,dest;
	scr.x=dest.x=371;
	scr.y=dest.y=301;
	scr.w=dest.w=22;
	scr.h=dest.h=115;
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
	scr.x=dest.x=312;
	scr.y=dest.y=265;
	scr.w=dest.w=150;
	scr.h=dest.h=26;
	SDL_BlitSurface ( GraphicsData.gfx_upgrade,&scr,buffer,&dest );
	sprintf ( str,"Credits: %d",owner->Credits );
	fonts->OutTextCenter ( str,381,275,buffer );

	scr.x=118;
	scr.y=336;
	scr.w=dest.w=16;
	scr.h=dest.h=115* ( int ) ( ( owner->Credits/ ( float ) StartCredits ) );
	dest.x=375;
	dest.y=301+115-dest.h;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
}

// Macht die Upgradeschieber für Vehicle:
void cBuilding::MakeUpgradeSliderVehicle ( sUpgrades *u,int nr )
{
	sUnitData *d;
	int i;
	for ( i=0;i<8;i++ )
	{
		u[i].active=false;
		u[i].Purchased=0;
		u[i].value=NULL;
	}
	d=owner->VehicleData+nr;
	i=0;

	if ( d->can_attack )
	{
		// Damage:
		u[i].active=true;
		u[i].value=& ( d->damage );
		u[i].NextPrice=2*CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.damage, 0 );
		u[i].name = "damage";
		i++;
		// Shots:
		u[i].active=true;
		u[i].value=& ( d->max_shots );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_shots, 2 );
		u[i].name = "shots";
		i++;
		// Range:
		u[i].active=true;
		u[i].value=& ( d->range );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.range, 3 );
		u[i].name = "range";
		i++;
		// Ammo:
		u[i].active=true;
		u[i].value=& ( d->max_ammo );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_ammo, 0 );
		u[i].name = "ammo";
		i++;
	}
	if ( d->can_transport==TRANS_METAL||d->can_transport==TRANS_OIL||d->can_transport==TRANS_GOLD )
	{
		i++;
	}
	// Armor:
	u[i].active=true;
	u[i].value=& ( d->armor );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.armor, 0 );
	u[i].name = "armor";
	i++;
	// Hitpoints:
	u[i].active=true;
	u[i].value=& ( d->max_hit_points );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_hit_points, 0 );
	u[i].name = "hitpoints";
	i++;
	// Scan:
	u[i].active=true;
	u[i].value=& ( d->scan );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.scan, 3 );
	u[i].name = "scan";
	i++;
	// Speed:
	u[i].active=true;
	u[i].value=& ( d->max_speed );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.vehicle[nr].data.max_speed, 1 );
	u[i].name = "speed";
	i++;
	// Costs:
	i++;

	for ( i=0;i<8;i++ )
	{
		if ( u[i].value==NULL ) continue;
		u[i].StartValue=* ( u[i].value );
	}
}

// Macht die Upgradeschieber für Buildings:
void cBuilding::MakeUpgradeSliderBuilding ( sUpgrades *u,int nr )
{
	sUnitData *d;
	int i;
	for ( i=0;i<8;i++ )
	{
		u[i].active=false;
		u[i].Purchased=0;
		u[i].value=NULL;
	}
	d=owner->BuildingData+nr;
	i=0;

	if ( d->can_attack )
	{
		// Damage:
		u[i].active=true;
		u[i].value=& ( d->damage );
		u[i].NextPrice=2*CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.damage, 0 );
		u[i].name = "damage";
		i++;
		if ( !d->is_expl_mine )
		{
			// Shots:
			u[i].active=true;
			u[i].value=& ( d->max_shots );
			u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.max_shots, 2 );
			u[i].name = "shots";
			i++;
			// Range:
			u[i].active=true;
			u[i].value=& ( d->range );
			u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.range, 3 );
			u[i].name = "range";
			i++;
			// Ammo:
			u[i].active=true;
			u[i].value=& ( d->max_ammo );
			u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.max_ammo, 0 );
			u[i].name = "ammo";
			i++;
		}
	}
	if ( d->max_shield )
	{
		// Range:
		u[i].active=true;
		u[i].value=& ( d->range );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.range, 3 );
		u[i].name = "range";
		i++;
	}
	if ( d->can_load==TRANS_METAL||d->can_load==TRANS_OIL||d->can_load==TRANS_GOLD )
	{
		i++;
	}
	if ( d->energy_prod )
	{
		i+=2;
	}
	if ( d->human_prod )
	{
		i++;
	}
	// Armor:
	u[i].active=true;
	u[i].value=& ( d->armor );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.armor, 0 );
	u[i].name = "armor";
	i++;
	// Hitpoints:
	u[i].active=true;
	u[i].value=& ( d->max_hit_points );
	u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.max_hit_points, 0 );
	u[i].name = "hitpoints";
	i++;
	// Scan:
	if ( d->scan )
	{
		u[i].active=true;
		u[i].value=& ( d->scan );
		u[i].NextPrice=CalcPrice ( * ( u[i].value ),UnitsData.building[nr].data.scan, 3 );
		u[i].name = "scan";
		i++;
	}
	// Energieverbrauch:
	if ( d->energy_need )
	{
		i++;
	}
	// Humanverbrauch:
	if ( d->human_need )
	{
		i++;
	}
	// Metallverbrauch:
	if ( d->metal_need )
	{
		i++;
	}
	// Goldverbrauch:
	if ( d->gold_need )
	{
		i++;
	}
	// Costs:
	i++;

	for ( i=0;i<8;i++ )
	{
		if ( u[i].value==NULL ) continue;
		u[i].StartValue=* ( u[i].value );
	}
}

// Stellt die Selectionlist zusammen:
void cBuilding::CreateUpgradeList ( TList *selection,TList *images,int *selected,int *offset )
{
	sUnitData *bd;
	sUnitData *vd;
	int i;
	while ( selection->Count )
	{
		selection->Delete ( 0 );
	}
	for ( i=0;i<images->Count;i++ )
	{
		if ( images->UpgraStrItems[i]->vehicle )
		{
			if ( ! ( game->UpShowTank||game->UpShowShip||game->UpShowPlane ) ) continue;
			vd=& ( UnitsData.vehicle[images->UpgraStrItems[i]->id].data );
			if ( game->UpShowTNT&&!vd->can_attack ) continue;
			if ( vd->can_drive==DRIVE_AIR&&!game->UpShowPlane ) continue;
			if ( vd->can_drive==DRIVE_SEA&&!game->UpShowShip ) continue;
			if ( ( vd->can_drive==DRIVE_LAND||vd->can_drive==DRIVE_LANDnSEA ) &&!game->UpShowTank ) continue;
			selection->AddUpgraStr ( images->UpgraStrItems[i] );
		}
		else
		{
			if ( !game->UpShowBuild ) continue;
			bd=& ( UnitsData.building[images->UpgraStrItems[i]->id].data );
			if ( game->UpShowTNT&&!bd->can_attack ) continue;
			selection->AddUpgraStr ( images->UpgraStrItems[i] );
		}
	}
	if ( *offset>=selection->Count-9 )
	{
		*offset=selection->Count-9;
		if ( *offset<0 ) *offset=0;
	}
	if ( *selected>=selection->Count )
	{
		*selected=selection->Count-1;
		if ( *selected<0 ) *selected=0;
	}
}

// Malt die SubButtons im Upgradefenster:
void cBuilding::MakeUpgradeSubButtons ( void )
{
	SDL_Rect scr,dest;
	scr.x=152;scr.y=479;
	dest.x=467;dest.y=411;
	dest.w=scr.w=32;dest.h=scr.h=31;
	// Tank:
	if ( !game->UpShowTank )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// Plane:
	if ( !game->UpShowPlane )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// Ship:
	if ( !game->UpShowShip )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// Building:
	if ( !game->UpShowBuild )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
	// TNT:
	if ( !game->UpShowTNT )
	{
		SDL_BlitSurface ( GraphicsData.gfx_upgrade,&dest,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	dest.x+=33;
	scr.x+=33;
}

// Berechnet den Preis für ein Upgrade:
int cBuilding::CalcPrice ( int value,int org, int variety )
{
	int tmp;
	double a, b, c;
	switch ( variety )
	{
			// Treffer, Panzerung, Munition & Angriff
		case 0:
			switch ( org )
			{
				case 2:
					if ( value==2 ) return 39;
					if ( value==3 ) return 321;
					break;
				case 4:
					a=0.0016091639;
					b=-0.073815318;
					c=6.0672869;
					break;
				case 6:
					a=0.000034548596;
					b=-0.27217472;
					c=6.3695123;
					break;
				case 8:
					a=0.00037219059;
					b=2.5148748;
					c=5.0938608;
					break;
				case 9:
					a=0.000059941694;
					b=1.3962889;
					c=4.6045196;
					break;
				case 10:
					a=0.000033736018;
					b=1.4674423;
					c=5.5606209;
					break;
				case 12:
					a=0.0000011574058;
					b=0.23439586;
					c=6.113616;
					break;
				case 14:
					a=0.0000012483447;
					b=1.4562373;
					c=5.8250952;
					break;
				case 15:
					a=0.00000018548742;
					b=-0.33519669;
					c=6.3333527;
					break;
				case 16:
					a=0.000010898263;
					b=5.0297434;
					c=5.0938627;
					break;
				case 18:
					a=0.00000017182818;
					b=2.0009536;
					c=5.8937153;
					break;
				case 20:
					a=0.00000004065782;
					b=1.6533066;
					c=6.0601538;
					break;
				case 22:
					a=0.0000000076942857;
					b=-0.45461813;
					c=6.4148588;
					break;
				case 24:
					a=0.00000076484313;
					b=8.0505377;
					c=5.1465019;
					break;
				case 28:
					a=0.00000015199858;
					b=5.1528048;
					c=5.4700225;
					break;
				case 32:
					a=0.00000030797077;
					b=8.8830596;
					c=5.1409486;
					break;
				case 56:
					a=0.000000004477053;
					b=11.454622;
					c=5.4335099;
					break;
				default:
					return -1;
			}
			break;
			// Geschwindgigkeit
		case 1:
			org=org/2;
			value=value/2;
			switch ( org )
			{
				case 5:
					a=0.00040716128;
					b=-0.16662054;
					c=6.2234362;
					break;
				case 6:
					a=0.00038548127;
					b=0.48236948;
					c=5.827724;
					break;
				case 7:
					a=0.000019798772;
					b=-0.31204765;
					c=6.3982628;
					break;
				case 9:
					a=0.0000030681294;
					b=-0.25372812;
					c=6.3995668;
					break;
				case 10:
					a=0.0000062019158;
					b=-0.23774407;
					c=6.1901333;
					break;
				case 12:
					a=0.0000064901101;
					b=0.93320705;
					c=5.8395847;
					break;
				case 14:
					a=0.0000062601892;
					b=2.1588132;
					c=5.5866699;
					break;
				case 15:
					a=0.00000027748628;
					b=-0.0031671959;
					c=6.2349744;
					break;
				case 16:
					a=0.0000011401659;
					b=1.8660343;
					c=5.7884287;
					break;
				case 18:
					a=0.00000093928003;
					b=2.9224069;
					c=5.6503159;
					break;
				case 20:
					a=0.00000003478867;
					b=0.44735558;
					c=6.2388156;
					break;
				case 24:
					a=0.0000000038623391;
					b=-0.4486039;
					c=6.4245686;
					break;
				case 28:
					a=0.000000039660207;
					b=1.6425505;
					c=5.8842817;
					break;
				default:
					return -2;
			}
			break;
			// Schüsse
		case 2:
			switch ( org )
			{
				case 1:
					return 720;
					break;
				case 2:
					if ( value==2 ) return 79;
					if ( value==3 ) return 641;
					break;
				default:
					return -3;
			}
			break;
			// Reichweite, Scan
		case 3:
			switch ( org )
			{
				case 3:
					if ( value==3 ) return 61;
					if ( value==4 ) return 299;
					break;
				case 4:
					a=0.010226741;
					b=-0.001141961;
					c=5.8477272;
					break;
				case 5:
					a=0.00074684696;
					b=-0.24064936;
					c=6.2377712;
					break;
				case 6:
					a=0.0000004205569;
					b=-2.5074874;
					c=8.1868728;
					break;
				case 7:
					a=0.00018753949;
					b=0.42735532;
					c=5.9259322;
					break;
				case 8:
					a=0.000026278484;
					b=0.0026600724;
					c=6.2281618;
					break;
				case 9:
					a=0.000017724816;
					b=0.35087138;
					c=6.1028354;
					break;
				case 10:
					a=0.000011074461;
					b=-0.41358078;
					c=6.2067919;
					break;
				case 11:
					a=0.0000022011968;
					b=-0.97456761;
					c=6.4502985;
					break;
				case 12:
					a=0.0000000034515189;
					b=-4.4597674;
					c=7.9715326;
					break;
				case 14:
					a=0.0000028257552;
					b=0.78730358;
					c=5.9483863;
					break;
				case 18:
					a=0.00000024289322;
					b=0.64536566;
					c=6.11706;
					break;
				default:
					return -4;
			}
			break;
	}

	tmp = Round((a*pow((value-b),c)));
	return tmp;
}

// Berechnet die Steigerung bei eim Upgrade:
int cBuilding::CalcSteigerung ( int org, int variety )
{
	int tmp = 0;
	switch ( variety )
	{
		case 0:
		{
			if ( org == 2 || org == 4 || org == 6 || org == 8 )
				tmp = 1;
			if ( org == 9 || org == 10 || org == 12 || org == 14 || org == 15 || org == 16 || org == 18 || org == 20 || org == 22 || org == 24 )
				tmp = 2;
			if ( org == 28 || org == 32 )
				tmp = 5;
			if ( org == 56 )
				tmp = 10;
			break;
		}
		case 1:
		{
			org=org/2;
			if ( org == 5 || org == 6 || org == 7 || org == 9 )
				tmp = 1;
			if ( org == 10 || org == 12 || org ==14 || org == 15 || org == 16 || org == 18 || org == 20 )
				tmp = 2;
			if ( org == 28 )
				tmp = 5;
			break;
		}
		case 2:
			tmp = 1;
			break;
		case 3:
		{
			if ( org == 3 || org ==4 || org == 5 || org == 6 || org == 7 || org == 8 || org == 9 )
				tmp = 1;
			if ( org == 10 || org ==11 || org == 12 || org == 14 || org == 18 )
				tmp = 2;
			break;
		}
	}
	return tmp;
}

// Malt große Symbole für das Info-Fenster:
void cBuilding::DrawSymbolBig ( eSymbolsBig sym,int x,int y,int maxx,int value,int orgvalue,SDL_Surface *sf )
{
	SDL_Rect scr,dest;
	int i,offx;
	switch ( sym )
	{
		case SBSpeed:
			scr.x=0;
			scr.y=109;
			scr.w=11;
			scr.h=12;
			break;
		case SBHits:
			scr.x=11;
			scr.y=109;
			scr.w=7;
			scr.h=11;
			break;
		case SBAmmo:
			scr.x=18;
			scr.y=109;
			scr.w=9;
			scr.h=14;
			break;
		case SBAttack:
			scr.x=27;
			scr.y=109;
			scr.w=10;
			scr.h=14;
			break;
		case SBShots:
			scr.x=37;
			scr.y=109;
			scr.w=15;
			scr.h=7;
			break;
		case SBRange:
			scr.x=52;
			scr.y=109;
			scr.w=13;
			scr.h=13;
			break;
		case SBArmor:
			scr.x=65;
			scr.y=109;
			scr.w=11;
			scr.h=14;
			break;
		case SBScan:
			scr.x=76;
			scr.y=109;
			scr.w=13;
			scr.h=13;
			break;
		case SBMetal:
			scr.x=89;
			scr.y=109;
			scr.w=12;
			scr.h=15;
			break;
		case SBOil:
			scr.x=101;
			scr.y=109;
			scr.w=11;
			scr.h=12;
			break;
		case SBGold:
			scr.x=112;
			scr.y=109;
			scr.w=13;
			scr.h=10;
			break;
		case SBEnergy:
			scr.x=125;
			scr.y=109;
			scr.w=13;
			scr.h=17;
			break;
		case SBHuman:
			scr.x=138;
			scr.y=109;
			scr.w=12;
			scr.h=16;
			break;
	}

	maxx-=scr.w;
	if ( orgvalue<value )
	{
		maxx-=scr.w+3;
	}
	offx=scr.w;
	while ( offx*value>=maxx )
	{
		offx--;
		if ( offx<4 )
		{
			value/=2;
			orgvalue/=2;
			offx=scr.w;
		}
	}

	dest.x=x;
	dest.y=y;
	dest.w=scr.w;
	dest.h=scr.h;
	for ( i=0;i<value;i++ )
	{
		if ( i==orgvalue )
		{
			SDL_Rect mark;
			dest.x+=scr.w+3;
			mark.x=dest.x-scr.w/2;
			mark.y=dest.y;
			mark.w=1;
			mark.h=dest.h;
			SDL_FillRect ( sf,&mark,0xFC0000 );
		}
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,sf,&dest );
		dest.x+=offx;
	}
}

// Prüft die Ressourcen unter der Mine:
void cBuilding::CheckRessourceProd ( void )
{
	int pos,max_cap;
	pos=PosX+PosY*game->map->size;
	if ( game->map->Resources[pos].typ==RES_METAL )
	{
		MaxMetalProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_OIL )
	{
		MaxOilProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_GOLD )
	{
		MaxGoldProd+=game->map->Resources[pos].value;
	}
	pos++;
	if ( game->map->Resources[pos].typ==RES_METAL )
	{
		MaxMetalProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_OIL )
	{
		MaxOilProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_GOLD )
	{
		MaxGoldProd+=game->map->Resources[pos].value;
	}
	pos+=game->map->size;
	if ( game->map->Resources[pos].typ==RES_METAL )
	{
		MaxMetalProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_OIL )
	{
		MaxOilProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_GOLD )
	{
		MaxGoldProd+=game->map->Resources[pos].value;
	}
	pos--;
	if ( game->map->Resources[pos].typ==RES_METAL )
	{
		MaxMetalProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_OIL )
	{
		MaxOilProd+=game->map->Resources[pos].value;
	}
	else if ( game->map->Resources[pos].typ==RES_GOLD )
	{
		MaxGoldProd+=game->map->Resources[pos].value;
	}

	if ( data.is_alien )
	{
		max_cap=24;
	}
	else
	{
		max_cap=16;
	}

	// Rohstoffe verteilen:
	pos=max_cap;
	MetalProd=MaxMetalProd;
	if ( MetalProd>max_cap )
	{
		MetalProd=max_cap;
		pos=0;
	}
	else
	{
		pos-=MaxMetalProd;
	}
	if ( pos>0 )
	{
		if ( MaxOilProd-pos<0 )
		{
			OilProd=MaxOilProd;
			pos-=MaxOilProd;
		}
		else
		{
			OilProd=pos;
			pos=0;
		}
	}
	if ( pos>0 )
	{
		if ( MaxGoldProd-pos<0 )
		{
			GoldProd=MaxGoldProd;
			pos-=MaxGoldProd;
		}
		else
		{
			GoldProd=pos;
			pos=0;
		}
	}
}

// Zeigt den Minenmanager an:
void cBuilding::ShowMineManager ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b,i;
	SDL_Rect scr,dest;
	bool FertigPressed=false;
	bool IncMetalPressed=false;
	bool DecMetalPressed=false;
	bool IncOilPressed=false;
	bool DecOilPressed=false;
	bool IncGoldPressed=false;
	bool DecGoldPressed=false;
	int MaxM=0,MaxO=0,MaxG=0;
	int FreeM=0,FreeO=0,FreeG=0;
	TList *mines;

	SDL_BlitSurface ( GraphicsData.gfx_mine_manager,NULL,buffer,NULL );
	mouse->SetCursor ( CHand );

	// Liste mit Minen erstellen:
	mines=new TList;
	for ( x=0;x<SubBase->buildings->Count;x++ )
	{
		if ( SubBase->buildings->BuildItems[x]->data.is_mine&&SubBase->buildings->BuildItems[x]->IsWorking )
		{
			cBuilding *b;
			b=SubBase->buildings->BuildItems[x];
			mines->AddBuild ( b );
			MaxM+=b->MaxMetalProd;
			MaxO+=b->MaxOilProd;
			MaxG+=b->MaxGoldProd;
		}
	}
//#define DO_MINE_INC(a,b) for(i=0;i<mines->Count;i++){if(mines->BuildItems[i]->MetalProd+mines->BuildItems[i]->OilProd+mines->BuildItems[i]->GoldProd<16&&mines->BuildItems[i]->a<mines->BuildItems[i]->b){mines->BuildItems[i]->a++;break;}}
#define DO_MINE_INC(a,b) for(i=0;i<mines->Count;i++){if(mines->BuildItems[i]->MetalProd+mines->BuildItems[i]->OilProd+mines->BuildItems[i]->GoldProd<(mines->BuildItems[i]->data.is_alien?24:16)&&mines->BuildItems[i]->a<mines->BuildItems[i]->b){mines->BuildItems[i]->a++;break;}}
#define DO_MINE_DEC(a) for(i=0;i<mines->Count;i++){if(mines->BuildItems[i]->a>0){mines->BuildItems[i]->a--;break;}}
//#define CALC_MINE_FREE FreeM=0;FreeO=0;FreeG=0;for(i=0;i<mines->Count;i++){int ges=mines->BuildItems[i]->MetalProd+mines->BuildItems[i]->OilProd+mines->BuildItems[i]->GoldProd;if(ges<16){int t;ges=16-ges;t=mines->BuildItems[i]->MaxMetalProd-mines->BuildItems[i]->MetalProd;FreeM+=(ges<t?ges:t);t=mines->BuildItems[i]->MaxOilProd-mines->BuildItems[i]->OilProd;FreeO+=(ges<t?ges:t);t=mines->BuildItems[i]->MaxGoldProd-mines->BuildItems[i]->GoldProd;FreeG+=(ges<t?ges:t);}}
#define CALC_MINE_FREE FreeM=0;FreeO=0;FreeG=0;for(i=0;i<mines->Count;i++){int ges=mines->BuildItems[i]->MetalProd+mines->BuildItems[i]->OilProd+mines->BuildItems[i]->GoldProd;if(ges<(mines->BuildItems[i]->data.is_alien?24:16)){int t;ges=(mines->BuildItems[i]->data.is_alien?24:16)-ges;t=mines->BuildItems[i]->MaxMetalProd-mines->BuildItems[i]->MetalProd;FreeM+=(ges<t?ges:t);t=mines->BuildItems[i]->MaxOilProd-mines->BuildItems[i]->OilProd;FreeO+=(ges<t?ges:t);t=mines->BuildItems[i]->MaxGoldProd-mines->BuildItems[i]->GoldProd;FreeG+=(ges<t?ges:t);}}

	CALC_MINE_FREE
	MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );

	// Die Reserve malen:
	DrawMineBar ( TRANS_METAL,SubBase->Metal,SubBase->MaxMetal,2,true,0 );
	DrawMineBar ( TRANS_OIL,SubBase->Oil,SubBase->MaxOil,2,true,0 );
	DrawMineBar ( TRANS_GOLD,SubBase->Gold,SubBase->MaxGold,2,true,0 );

	SHOW_SCREEN
	mouse->GetBack ( buffer );
	mouse->draw ( false,screen );

	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}

		// Fertig-Button:
		if ( x>=514&&x<514+109&&y>=438&&y<438+40 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=0;
				scr.y=287;
				dest.w=scr.w=109;
				dest.h=scr.h=40;
				dest.x=514;
				dest.y=438;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				break;
			}
		}
		else if ( FertigPressed )
		{
			scr.x=514;
			scr.y=438;
			dest.w=scr.w=109;
			dest.h=scr.h=40;
			dest.x=514;
			dest.y=438;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Aufs Metall geklickt:
		if ( x>=174&&x<174+240&&y>=70&&y<70+30&&b&&!LastB )
		{
			int t;
			PlayFX ( SoundData.SNDObjectMenu );
			t=  Round (( x-174 ) * ( MaxM/240.0 ));
			if ( t<SubBase->MetalProd )
			{
				for ( ;abs ( SubBase->MetalProd-t ) &&SubBase->MetalProd>0; )
				{
					SubBase->MetalProd--;
					DO_MINE_DEC ( MetalProd )
				}
				CALC_MINE_FREE
			}
			else
			{
				for ( ;abs ( SubBase->MetalProd-t ) &&FreeM; )
				{
					SubBase->MetalProd++;
					DO_MINE_INC ( MetalProd,MaxMetalProd )
					CALC_MINE_FREE
				}
			}
			MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Aufs Öl geklickt:
		if ( x>=174&&x<174+240&&y>=190&&y<190+30&&b&&!LastB )
		{
			int t;
			PlayFX ( SoundData.SNDObjectMenu );
			t= Round ( ( x-174 ) * ( MaxO/240.0 ));
			if ( t<SubBase->OilProd )
			{
				for ( ;abs ( SubBase->OilProd-t ) &&SubBase->OilProd>0; )
				{
					SubBase->OilProd--;
					DO_MINE_DEC ( OilProd )
				}
				CALC_MINE_FREE
			}
			else
			{
				for ( ;abs ( SubBase->OilProd-t ) &&FreeO; )
				{
					SubBase->OilProd++;
					DO_MINE_INC ( OilProd,MaxOilProd )
					CALC_MINE_FREE
				}
			}
			MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Aufs Gold geklickt:
		if ( x>=174&&x<174+240&&y>=310&&y<310+30&&b&&!LastB )
		{
			int t;
			PlayFX ( SoundData.SNDObjectMenu );
			t= Round ( ( x-174 ) * ( MaxG/240.0 ));
			if ( t<SubBase->GoldProd )
			{
				for ( ;abs ( SubBase->GoldProd-t ) &&SubBase->GoldProd>0; )
				{
					SubBase->GoldProd--;
					DO_MINE_DEC ( GoldProd )
				}
				CALC_MINE_FREE
			}
			else
			{
				for ( ;abs ( SubBase->GoldProd-t ) &&FreeG; )
				{
					SubBase->GoldProd++;
					DO_MINE_INC ( GoldProd,MaxGoldProd )
					CALC_MINE_FREE
				}
			}
			MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// IncMetal-Button:
		if ( x>=421&&x<421+26&&y>=71&&y<71+27&&b&&!IncMetalPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=122;
			scr.y=308;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=421;
			dest.y=71;
			if ( FreeM )
			{
				SubBase->MetalProd++;
				DO_MINE_INC ( MetalProd,MaxMetalProd )
				CALC_MINE_FREE
				MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncMetalPressed=true;
		}
		else if ( !b&&IncMetalPressed )
		{
			scr.x=421;
			scr.y=71;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=421;
			dest.y=71;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncMetalPressed=false;
		}
		// DecMetal-Button:
		if ( x>=139&&x<139+26&&y>=71&&y<71+27&&b&&!DecMetalPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=122;
			scr.y=280;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=139;
			dest.y=71;
			if ( SubBase->MetalProd>0 )
			{
				SubBase->MetalProd--;
				DO_MINE_DEC ( MetalProd )
				CALC_MINE_FREE
				MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecMetalPressed=true;
		}
		else if ( !b&&DecMetalPressed )
		{
			scr.x=139;
			scr.y=71;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=139;
			dest.y=71;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecMetalPressed=false;
		}
		// IncOil-Button:
		if ( x>=421&&x<421+26&&y>=191&&y<191+27&&b&&!IncOilPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=122;
			scr.y=308;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=421;
			dest.y=191;
			if ( FreeO )
			{
				SubBase->OilProd++;
				DO_MINE_INC ( OilProd,MaxOilProd )
				CALC_MINE_FREE
				MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncOilPressed=true;
		}
		else if ( !b&&IncOilPressed )
		{
			scr.x=421;
			scr.y=191;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=421;
			dest.y=191;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncOilPressed=false;
		}
		// DecOil-Button:
		if ( x>=139&&x<139+26&&y>=191&&y<191+27&&b&&!DecOilPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=122;
			scr.y=280;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=139;
			dest.y=191;
			if ( SubBase->OilProd>0 )
			{
				SubBase->OilProd--;
				DO_MINE_DEC ( OilProd )
				CALC_MINE_FREE
				MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecOilPressed=true;
		}
		else if ( !b&&DecOilPressed )
		{
			scr.x=139;
			scr.y=191;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=139;
			dest.y=191;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecOilPressed=false;
		}
		// IncGold-Button:
		if ( x>=421&&x<421+26&&y>=311&&y<311+27&&b&&!IncGoldPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=122;
			scr.y=308;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=421;
			dest.y=311;
			if ( FreeG )
			{
				SubBase->GoldProd++;
				DO_MINE_INC ( GoldProd,MaxGoldProd )
				CALC_MINE_FREE
				MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncGoldPressed=true;
		}
		else if ( !b&&IncGoldPressed )
		{
			scr.x=421;
			scr.y=311;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=421;
			dest.y=311;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			IncGoldPressed=false;
		}
		// DecGold-Button:
		if ( x>=139&&x<139+26&&y>=311&&y<311+27&&b&&!DecGoldPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=122;
			scr.y=280;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=139;
			dest.y=311;
			if ( SubBase->GoldProd>0 )
			{
				SubBase->GoldProd--;
				DO_MINE_DEC ( GoldProd )
				CALC_MINE_FREE
				MakeMineBars ( MaxM,MaxO,MaxG,&FreeM,&FreeO,&FreeG );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecGoldPressed=true;
		}
		else if ( !b&&DecGoldPressed )
		{
			scr.x=139;
			scr.y=311;
			dest.w=scr.w=26;
			dest.h=scr.h=27;
			dest.x=139;
			dest.y=311;
			SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DecGoldPressed=false;
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
	delete mines;
}

// Malt die Minenmanager-Bars:
void cBuilding::MakeMineBars ( int MaxM,int MaxO,int MaxG,int *FreeM,int *FreeO,int *FreeG )
{
	char str[20];
	DrawMineBar ( TRANS_METAL,SubBase->MetalProd,MaxM,0,true,MaxM-SubBase->MetalProd-*FreeM );
	DrawMineBar ( TRANS_OIL,SubBase->OilProd,MaxO,0,true,MaxO-SubBase->OilProd-*FreeO );
	DrawMineBar ( TRANS_GOLD,SubBase->GoldProd,MaxG,0,true,MaxG-SubBase->GoldProd-*FreeG );

	DrawMineBar ( TRANS_METAL,SubBase->MetalNeed,SubBase->MaxMetalNeed,1,false,0 );
	sprintf ( str,"%d (%+d / Runde)",SubBase->MetalNeed,SubBase->MetalProd-SubBase->MetalNeed );
	fonts->OutTextBigCenter ( str,174+120,70+8+37,buffer );
	DrawMineBar ( TRANS_OIL,SubBase->OilNeed,SubBase->MaxOilNeed,1,false,0 );
	sprintf ( str,"%d (%+d / Runde)",SubBase->OilNeed,SubBase->OilProd-SubBase->OilNeed );
	fonts->OutTextBigCenter ( str,174+120,190+8+37,buffer );
	DrawMineBar ( TRANS_GOLD,SubBase->GoldNeed,SubBase->MaxGoldNeed,1,false,0 );
	sprintf ( str,"%d (%+d / Runde)",SubBase->GoldNeed,SubBase->GoldProd-SubBase->GoldNeed );
	fonts->OutTextBigCenter ( str,174+120,310+8+37,buffer );
}

// Malt einen Rohstoffbalken:
void cBuilding::DrawMineBar ( int typ,int value,int max_value,int offy,bool number,int fixed )
{
	SDL_Rect scr,dest;
	char str[10];
	switch ( typ )
	{
		case TRANS_METAL:
			scr.y=339;
			dest.y=70;
			break;
		case TRANS_OIL:
			scr.y=369;
			dest.y=190;
			break;
		case TRANS_GOLD:
			scr.y=400;
			dest.y=310;
			break;
	}
	dest.x=174;
	dest.w=240;
	dest.h=scr.h=30;
	dest.y+=offy*37;
	SDL_BlitSurface ( GraphicsData.gfx_mine_manager,&dest,buffer,&dest );
	if ( max_value==0 )
	{
		dest.w=scr.w=0;
		scr.x=156;
	}
	else
	{
		dest.w=scr.w= ( int ) ( ( ( float ) value / max_value ) * 240 );
		scr.x=156+ ( 240- ( int ) ( ( ( float ) value / max_value ) * 240 ) );
	}
	dest.x=174;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	if ( fixed &&scr.w!=240&&max_value!=0 )
		{
			dest.w=scr.w= ( int ) ( ( ( float ) fixed /  max_value ) * 240 );
			dest.x=174+240-scr.w;
			scr.x=156;
			scr.y=307;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
		}
	else if ( max_value==0 )
	{
		dest.w=scr.w=240;
		dest.x=174;
		scr.x=156;
		scr.y=307;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	if ( number )
	{
		sprintf ( str,"%d",value );
		fonts->OutTextBigCenter ( str,174+120,dest.y+8,buffer );
	}
}

// Prüft, ob das Ziel innerhalb der Reichweite liegt:
bool cBuilding::IsInRange ( int off )
{
	int x,y;
	x=off%game->map->size;
	y=off/game->map->size;
	x-=PosX;
	y-=PosY;
	if ( sqrt ( ( double ) ( x*x+y*y ) ) <=data.range )
	{
		return true;
	}
	return false;
}

// Prüft, ob das Building das Objekt angreifen kann:
bool cBuilding::CanAttackObject ( int off,bool override )
{
	cVehicle *v=NULL;
	cBuilding *b=NULL;
	if ( !data.can_attack ) return false;
	if ( !data.shots ) return false;
	if ( Attacking ) return false;
	if ( off<0 ) return false;
	if ( !owner->ScanMap[off] ) return false;
	if ( !IsInRange ( off ) ) return false;
	if ( data.can_attack!=ATTACK_AIR )
	{
		v=game->map->GO[off].vehicle;
		if ( game->map->GO[off].top )
		{
			b=game->map->GO[off].top;
		}
		else if ( game->map->GO[off].base&&game->map->GO[off].base->owner&&game->map->GO[off].base->detected )
		{
			b=game->map->GO[off].base;
		}
	}
	else
	{
		v=game->map->GO[off].plane;
	}
	if ( v&&v->data.is_stealth_sea&&data.can_attack!=ATTACK_SUB_LAND ) return false;
	if ( override ) return true;
	if ( v&&v->detected )
	{
		if ( v==game->SelectedVehicle||v->owner==game->ActivePlayer ) return false;
	}
	else if ( b )
	{
		if ( b==game->SelectedBuilding||b->owner==game->ActivePlayer ) return false;
	}
	else
	{
		return false;
	}
	return true;
}

// Malt den Attack-Cursor:
void cBuilding::DrawAttackCursor ( struct sGameObjects *go,int can_attack )
{
	SDL_Rect r;
	int wp,wc,t;
	cVehicle *v=NULL;
	cBuilding *b=NULL;

	if ( can_attack==ATTACK_LAND||can_attack==ATTACK_SUB_LAND )
	{
		if ( !go->vehicle&&!go->base&&!go->top )
		{
			r.x=1;r.y=29;r.h=3;r.w=35;
			SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0 );
			return;
		}
		if ( go->top )
		{
			b=go->top;
		}
		else if ( go->vehicle )
		{
			v=go->vehicle;
		}
		else if ( go->base&&go->base->owner )
		{
			b=go->base;
		}
		if ( v&&v->data.is_stealth_sea&&can_attack!=ATTACK_SUB_LAND ) v=NULL;
	}
	else if ( can_attack==ATTACK_AIR )
	{
		if ( !go->plane )
		{
			r.x=1;r.y=29;r.h=3;r.w=35;
			SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0 );
			return;
		}
		v=go->plane;
	}
	if ( ( v&&v==game->SelectedVehicle ) || ( b&&b==game->SelectedBuilding ) )
	{
		r.x=1;r.y=29;r.h=3;r.w=35;
		SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0 );
		return;
	}

	if ( v ) t=v->data.hit_points;else t=b->data.hit_points;
	if ( t )
	{
		if ( v ) wc= (int)( ( float ) t/v->data.max_hit_points *35 );
		else wc= (int)( ( float ) t/b->data.max_hit_points *35 );
	}
	else
	{
		wc=0;
	}
	if ( v ) t=v->CalcHelth ( data.damage );else t=b->CalcHelth ( data.damage );
	if ( t )
	{
		if ( v ) wp= (int)( ( float ) t/v->data.max_hit_points *35 );
		else wp= (int)( ( float ) t/b->data.max_hit_points *35 );
	}
	else
	{
		wp=0;
	}

	r.x=1;
	r.y=29;
	r.h=3;
	r.w=wp;
	if ( r.w ) SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0x00FF00 );
	r.x+=r.w;
	r.w=wc-wp;
	if ( r.w ) SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0xFF0000 );
	r.x+=r.w;
	r.w=35-wc;
	if ( r.w ) SDL_FillRect ( GraphicsData.gfx_Cattack,&r,0 );
}

// Dreht das Building in die angegebene Richtung:
void cBuilding::RotateTo ( int Dir )
{
	int i,t,dest;
	if ( dir==Dir ) return;

	t=dir;
	for ( i=0;i<8;i++ )
	{
		if ( t==Dir ) {dest=i;break;}
		t++;
		if ( t>7 ) t=0;
	}
	if ( dest<4 ) dir++;
	else dir--;
	if ( dir<0 ) dir+=8;
	else if ( dir>7 ) dir-=8;
}

// Struktur für die Build-List:
struct sBuildStruct
{
	SDL_Surface *sf;
	int id;
	int iRemainingMetal;
};
#include "pcx.h"
// Zeigt das Build-Menü an:
void cBuilding::ShowBuildMenu ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b,i,k;
	SDL_Rect scr,dest;
	bool AbbruchPressed=false;
	bool FertigPressed=false;
	bool Wiederholen=false;
	bool DownPressed=false;
	bool UpPressed=false;
	bool Down2Pressed=false;
	bool Up2Pressed=false;
	bool BauenPressed=false;
	bool EntfernenPressed=false;
	TList *images;
	TList *to_build;
	int selected=0,offset=0,BuildSpeed;
	int build_selected=0,build_offset=0;
	int  iTurboBuildRounds[3];		//Costs and
	int  iTurboBuildCosts[3];		//durations of the tree build speeds
	bool showDetailsBuildlist=true; //wenn false, stattdessen die Details der in der toBuild Liste gewählen Einheit anzeigen

	int last_b_ticks=0;

	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,NULL,buffer,NULL );

	// Der Haken:
	if ( SettingsData.bShowDescription )
	{
		dest.x=scr.x=291;
		dest.y=scr.y=264;
		dest.w=scr.w=17;
		dest.h=scr.h=17;
		SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
	}
	else
	{
		scr.x=393;
		scr.y=46;
		dest.x=291;
		dest.y=264;
		dest.w=scr.w=18;
		dest.h=scr.h=17;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}

	// Die Images erstellen:
	images=new TList;
	float newzoom = ( game->hud->Zoom/64.0 );
	for ( i=0;i<UnitsData.vehicle_anz;i++ )
	{
		sBuildStruct *n;
		SDL_Surface *sf;

		if ( data.can_build==BUILD_AIR&&UnitsData.vehicle[i].data.can_drive!=DRIVE_AIR ) continue;
		else if ( data.can_build==BUILD_BIG&&!UnitsData.vehicle[i].data.build_by_big ) continue;
		else if ( data.can_build==BUILD_SEA&&UnitsData.vehicle[i].data.can_drive!=DRIVE_SEA ) continue;
		else if ( data.can_build==BUILD_SMALL&& ( UnitsData.vehicle[i].data.can_drive==DRIVE_AIR||UnitsData.vehicle[i].data.can_drive==DRIVE_SEA||UnitsData.vehicle[i].data.build_by_big||UnitsData.vehicle[i].data.is_human ) ) continue;
		else if ( data.can_build==BUILD_MAN&&!UnitsData.vehicle[i].data.is_human ) continue;
		else if ( !data.build_alien&&UnitsData.vehicle[i].data.is_alien ) continue;
		else if ( data.build_alien&&!UnitsData.vehicle[i].data.is_alien ) continue;
		ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[0],UnitsData.vehicle[i].img[0],UnitsData.vehicle[i].img_org[0]->w/2,UnitsData.vehicle[i].img_org[0]->h/2 );
		sf=SDL_CreateRGBSurface ( SDL_SRCCOLORKEY,UnitsData.vehicle[i].img[0]->w,UnitsData.vehicle[i].img[0]->h,32,0,0,0,0 );
		SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
		SDL_BlitSurface ( game->ActivePlayer->color,NULL,sf,NULL );
		SDL_BlitSurface ( UnitsData.vehicle[i].img[0],NULL,sf,NULL );
		ScaleSurfaceAdv2 ( UnitsData.vehicle[i].img_org[0],UnitsData.vehicle[i].img[0],( int ) (UnitsData.vehicle[i].img_org[0]->w* newzoom ),( int ) (UnitsData.vehicle[i].img_org[0]->h* newzoom ) );
		n=new sBuildStruct;
		n->sf=sf;
		n->id=i;
		images->AddBuildStruct ( n );
	}
	

	// Die Bauliste anlegen:
	to_build=new TList;
	for ( i=0;i<BuildList->Count;i++ )
	{
		sBuildList *ptr;
		ptr=BuildList->BuildListItems[i];

		sBuildStruct *n;
		n = new sBuildStruct;
		n->iRemainingMetal = ptr->metall_remaining;

		//für jeden Eintrag in der toBuild-Liste das bereits erstellte Bild in der Auswahlliste suchen
		//und in die toBuild-Liste kopieren.
		for ( k=0;k<images->Count;k++ )
		{
			sBuildStruct *bs;
			bs=images->BuildStructItems[k];
			if ( UnitsData.vehicle[bs->id].nr==ptr->typ->nr )
			{
				n->id = images->BuildStructItems[k]->id;
				n->sf = images->BuildStructItems[k]->sf;
				to_build->AddBuildStruct ( n );

				break;
			}
		}
	}
	BuildSpeed=this->BuildSpeed;
	
	//show details of the first item in to_build list, if it exists
	if (to_build->Count > 0)
	{
		showDetailsBuildlist=false;
	}
	else
	{
		showDetailsBuildlist=true;
	}

	ShowBuildList ( images,selected,offset, showDetailsBuildlist );
	DrawBuildButtons ( BuildSpeed );
	ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );

	if ( !RepeatBuild )
	{
		// Den Wiederholen Haken machen:
		scr.x=393;scr.y=46;
		dest.x=447;dest.y=322;
		dest.w=scr.w=18;
		dest.h=scr.h=17;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		Wiederholen=true;
	}

	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );

	mouse->MoveCallback=false;

	while ( 1 )
	{
		if ( game->SelectedBuilding==NULL ) break;
		// Die Engine laufen lassen:
		game->engine->Run();
		//game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}
		//if ( timer0 ) last_b_ticks++;

		// Down-Button:
		if ( x>=491&&x<491+18&&y>=440&&y<440+17&&b&&!DownPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=491;
			dest.y=440;
			if ( offset<images->Count-9 )
			{
				offset++;
				//if ( selected<offset ) selected=offset;
				ShowBuildList ( images,selected,offset, showDetailsBuildlist );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=true;
		}
		else if ( !b&&DownPressed )
		{
			scr.x=491;
			scr.y=440;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=491;
			dest.y=440;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			DownPressed=false;
		}
		// Up-Button:
		if ( x>=471&&x<471+18&&y>=440&&y<440+17&&b&&!UpPressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=471;
			dest.y=440;
			if ( offset!=0 )
			{
				offset--;
				//if ( selected>=offset+9 ) selected=offset+8;
				ShowBuildList ( images,selected,offset, showDetailsBuildlist );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=true;
		}
		else if ( !b&&UpPressed )
		{
			scr.x=471;
			scr.y=440;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=471;
			dest.y=440;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			UpPressed=false;
		}
		// Down2-Button:
		if ( x>=327&&x<327+18&&y>=293&&y<293+17&&b&&!Down2Pressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=230;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=327;
			dest.y=293;
			if ( build_offset!=0 )
			{
				build_offset--;
				//if ( build_selected>=build_offset+5 ) build_selected=build_offset+4;
				ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Down2Pressed=true;
		}
		else if ( !b&&Down2Pressed )
		{
			scr.x=327;
			scr.y=293;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=327;
			dest.y=293;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Down2Pressed=false;
		}
		// Up2-Button:
		if ( x>=347&&x<347+18&&y>=293&&y<293+17&&b&&!Up2Pressed )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			scr.x=249;
			scr.y=151;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=347;
			dest.y=293;
			if ( build_offset<to_build->Count-5 )
			{
				build_offset++;
				//if ( build_selected<build_offset ) build_selected=build_offset;
				ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );
			}
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Up2Pressed=true;
		}
		else if ( !b&&Up2Pressed )
		{
			scr.x=347;
			scr.y=293;
			dest.w=scr.w=18;
			dest.h=scr.h=17;
			dest.x=347;
			dest.y=293;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			Up2Pressed=false;
		}
		// Bauen-Button:
		if ( x>=548&&x<548+78&&y>=442&&y<442+23&&b&&!BauenPressed )
		{
			PlayFX ( SoundData.SNDMenuButton );
			scr.x=0;
			scr.y=328;
			dest.w=scr.w=78;
			dest.h=scr.h=23;
			dest.x=548;
			dest.y=442;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			BauenPressed=true;
		}
		else if ( BauenPressed&&!b&&LastB )
		{
			// Vehicle in die Bauliste aufnehmen:
			sBuildStruct *n = new sBuildStruct;
			n->id = images->BuildStructItems[selected]->id;
			n->sf = images->BuildStructItems[selected]->sf;
			n->iRemainingMetal = -1;

			to_build->AddBuildStruct ( n );
			if ( build_selected<0 ) build_selected=0;
			ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );

			scr.x=548;
			scr.y=442;
			dest.w=scr.w=78;
			dest.h=scr.h=23;
			dest.x=548;
			dest.y=442;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			BauenPressed=false;
		}
		// Entfernen-Button:
		if ( x>=412&&x<412+53&&y>=293&&y<293+23&&b&&!EntfernenPressed )
		{
			PlayFX ( SoundData.SNDMenuButton );
			scr.x=0;
			scr.y=352;
			dest.w=scr.w=53;
			dest.h=scr.h=23;
			dest.x=412;
			dest.y=293;
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			EntfernenPressed=true;
		}
		else if ( EntfernenPressed&&!b&&LastB )
		{
			// Vehicle aus der Bauliste entfernen:
			if ( to_build->Count&&to_build->Count>build_selected&&build_selected>=0 )
			{
				delete to_build->BuildStructItems[build_selected];
				to_build->DeleteBuildStruct ( build_selected );
				//ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );
				if ( build_selected>=to_build->Count )
				{
					build_selected--;
				}
				if ( to_build->Count-build_offset<5&&build_offset>0 )
				{
					build_offset--;
				}
				ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );
			}

			scr.x=412;
			scr.y=293;
			dest.w=scr.w=53;
			dest.h=scr.h=23;
			dest.x=412;
			dest.y=293;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			EntfernenPressed=false;
		}
		// Abbruch-Button:
		if ( x>=307&&x<307+61&&y>=452&&y<452+23 )
		{
			if ( b&&!AbbruchPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=364;
				scr.y=231;
				dest.w=scr.w=62;
				dest.h=scr.h=24;
				dest.x=307;
				dest.y=452;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				AbbruchPressed=true;
			}
			else if ( !b&&LastB )
			{
				//RepeatBuild=Wiederholen;  ?? warum?
				break;
			}
		}
		else if ( AbbruchPressed )
		{
			scr.x=307;
			scr.y=452;
			dest.w=scr.w=62;
			dest.h=scr.h=24;
			dest.x=307;
			dest.y=452;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			AbbruchPressed=false;
		}
		// Fertig-Button:
		if ( x>=397&&x<397+54&&y>=452&&y<452+23 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=308;
				scr.y=231;
				dest.w=scr.w=55;
				dest.h=scr.h=24;
				dest.x=397;
				dest.y=452;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				//perform all changes

				//first set the metal consumption of the factory
				if (IsWorking)
				{
					StopWork(false);
				}

				if (BuildSpeed == 0) MetalPerRound =  3;
				if (BuildSpeed == 1) MetalPerRound = 12;
				if (BuildSpeed == 2) MetalPerRound = 36;

				this->BuildSpeed=BuildSpeed;
				RepeatBuild=Wiederholen;
				
				//delete old BuildList
				while ( BuildList->Count )
				{
						sBuildList *ptr;
						ptr=BuildList->BuildListItems[0];
						delete ptr;
						BuildList->DeleteBuildList ( 0 );
				}

				//calculate actual costs of the vehicles
				//and add is to the BuildList
				for (int counter = 0; counter < to_build->Count; counter++)
				{
					sBuildStruct *bs = to_build->BuildStructItems[counter];

					CalcTurboBuild( iTurboBuildRounds, iTurboBuildCosts, owner->VehicleData[bs->id].iBuilt_Costs, bs->iRemainingMetal);

					sBuildList *bl = new sBuildList;
					bl->metall_remaining = iTurboBuildCosts[BuildSpeed];
					bl->typ = &UnitsData.vehicle[bs->id];

					BuildList->AddBuildList ( bl );
				}
				
				//start facrory, if there is something in the build queue
				if ( BuildList->Count > 0 )
				{
					StartWork();
				}

				break;
				
				//old
				/*i=0;
				if ( BuildList->Count )
				{
					if ( UnitsData.vehicle[to_build->BuildStructItems[0]->id].nr == BuildList->BuildListItems[0]->typ->nr )
					{
						while ( BuildList->Count>1 )
						{
							sBuildList *ptr;
							ptr=BuildList->BuildListItems[1];
							delete ptr;
							BuildList->DeleteBuildList ( 1 );
						}
						i=1;
					}
					else
					{
						while ( BuildList->Count )
						{
							sBuildList *ptr;
							ptr=BuildList->BuildListItems[0];
							delete ptr;
							BuildList->DeleteBuildList ( 0 );
						}
					}
				}

				for ( ;i<to_build->Count;i++ )
				{
					sBuildList *n;
					n=new sBuildList;
					n->typ=UnitsData.vehicle+to_build->BuildStructItems[i]->id;
					n->metall_remaining=owner->VehicleData[n->typ->nr].costs;
					BuildList->AddBuildList ( n );
				}
				break;
				*/
			}
		}
		else if ( FertigPressed )
		{
			scr.x=397;
			scr.y=452;
			dest.w=scr.w=55;
			dest.h=scr.h=24;
			dest.x=397;
			dest.y=452;
			SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}
		// Beschreibung Haken:
		if ( x>=292&&x<292+16&&y>=265&&y<265+15&&b&&!LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			
			SettingsData.bShowDescription=!SettingsData.bShowDescription;
			if ( SettingsData.bShowDescription )
			{
				dest.x=scr.x=291;
				dest.y=scr.y=264;
				dest.w=scr.w=17;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			}
			else
			{
				scr.x=393;
				scr.y=46;
				dest.x=291;
				dest.y=264;
				dest.w=scr.w=18;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			}
			ShowBuildList ( images,selected,offset, showDetailsBuildlist );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// Wiederholen Haken:
		if ( x>=447&&x<447+16&&y>=322&&y<322+15&&b&&!LastB )
		{
			PlayFX ( SoundData.SNDObjectMenu );
			Wiederholen=!Wiederholen;
			if ( Wiederholen )
			{
				dest.x=scr.x=447;
				dest.y=scr.y=322;
				dest.w=scr.w=18;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&dest );
			}
			else
			{
				scr.x=393;
				scr.y=46;
				dest.x=447;
				dest.y=322;
				dest.w=scr.w=18;
				dest.h=scr.h=17;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
			}

			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// 1x Button:
		if ( x>=292&&x<292+76&&y>=345&&y<345+22&&b&&!LastB )
		{
			PlayFX ( SoundData.SNDMenuButton );
			BuildSpeed=0;
			DrawBuildButtons ( BuildSpeed );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}
		// 2x Button:
		if ( x>=292&&x<292+76&&y>=369&&y<369+22&&b&&!LastB&&data.can_build!=BUILD_MAN )
		{
			PlayFX ( SoundData.SNDMenuButton );
			BuildSpeed=1;
			DrawBuildButtons ( BuildSpeed );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			
		}
		// 4x Button:
		if ( x>=292&&x<292+76&&y>=394&&y<394+22&&b&&!LastB&&data.can_build!=BUILD_MAN )
		{
			PlayFX ( SoundData.SNDMenuButton );
			BuildSpeed=2;
			DrawBuildButtons ( BuildSpeed );
			SHOW_SCREEN
			mouse->draw ( false,screen );
		}

		// Klick in die Liste:
		if ( x>=490&&x<490+70&&y>=60&&y<60+368&&b&&!LastB )
		{
			int nr;
			nr= ( y-60 ) / ( 32+10 );
			if ( images->Count<9 )
			{
				if ( nr>=images->Count ) nr=-1;
			}
			else
			{
				if ( nr>=10 ) nr=-1;
				nr+=offset;
			}
			if ( nr!=-1 )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				
				// second klick on the Unit?
				if ( (nr == selected)&&showDetailsBuildlist )
				{
					//insert selected Vehicle in to_build list
					sBuildStruct *n = new sBuildStruct;
					n->id = images->BuildStructItems[selected]->id;
					n->sf = images->BuildStructItems[selected]->sf;
					n->iRemainingMetal = -1;

					to_build->AddBuildStruct ( n );
					if ( build_selected<0 ) build_selected=0;
				}
				selected=nr;
				showDetailsBuildlist=true;
				ShowBuildList ( images,selected,offset, showDetailsBuildlist );
				ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );

				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		// Klick in die to_build Liste:
		if ( x>=330&&x<330+128&&y>=60&&y<60+210&&b&&!LastB )
		{
			int nr;
			nr= ( y-60 ) / ( 32+10 );
			if ( to_build->Count<5 )
			{
				if ( nr>=to_build->Count ) nr=-1;
			}
			else
			{
				if ( nr>=10 ) nr=-1;
				nr+=build_offset;
			}
			if ( nr!=-1 )
			{
				PlayFX ( SoundData.SNDObjectMenu );
				
				// second klick on the Unit?
				if ( (build_selected == nr) &&!showDetailsBuildlist )
				{
					if ( to_build->Count&&to_build->Count>build_selected&&build_selected>=0 )
					{
						delete to_build->BuildStructItems[build_selected];
						to_build->DeleteBuildStruct ( build_selected );
						
						//to_build->Delete ( build_selected );

						//ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );
						if ( build_selected>=to_build->Count )
						{
							build_selected--;
						}
						if ( to_build->Count-build_offset<5&&build_offset>0 )
						{
							build_offset--;
						}
					}
				}

				build_selected=nr;
				showDetailsBuildlist=false;
				ShowBuildList ( images,selected,offset, showDetailsBuildlist );
				ShowToBuildList ( to_build,build_selected,build_offset, !showDetailsBuildlist );

				SHOW_SCREEN
				mouse->draw ( false,screen );
			}
		}
		LastMouseX=x;LastMouseY=y;
		LastB=b;
		//if ( b ) last_b_ticks=0;
	}
	// Alles Images löschen:
	while ( images->Count )
	{
		sBuildStruct *ptr;
		ptr=images->BuildStructItems[0];
		SDL_FreeSurface ( ptr->sf );
		delete ptr;
		images->DeleteBuildStruct ( 0 );
	}
	delete images;

	while ( to_build->Count )
	{
		delete  to_build->BuildStructItems[0];
		to_build->DeleteBuildStruct ( 0 );
	}
	delete to_build;

	//dat geit so nich...
	/*
	if ( BuildList->Count&&!IsWorking )
	{
		StartWork();
	}
	else if ( !BuildList->Count&&IsWorking )
	{
		StopWork ( false );
	} */

	mouse->MoveCallback=true;
}

// Zeigt die Liste mit den baubaren Einheiten und wenn showInfo==true auch sämtliche Infos zur ausgewählten Einheit 
void cBuilding::ShowBuildList ( TList *list,int selected,int offset, bool showInfo )
{
	sBuildStruct *ptr;
	SDL_Rect dest,scr,text;
	char str[100];
	int i,t;
	scr.x=479;scr.y=52;
	scr.w=150;scr.h=378;
	SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&scr );
	scr.x=373;scr.y=344;
	scr.w=77;scr.h=72;
	SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&scr );
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=490;dest.y=58;
	dest.w=32;dest.h=32;
	text.x=530;text.y=70;
	for ( i=offset;i<list->Count;i++ )
	{
		if ( i>=offset+9 ) break;
		// Das Bild malen:
		ptr=list->BuildStructItems[i];
		SDL_BlitSurface ( ptr->sf,&scr,buffer,&dest );
		// Ggf noch Rahmen drum:
		if ( selected==i )
		{
			if (showInfo == true)
			{
				//doppelten Rahmen drum malen
				SDL_Rect tmp;
				tmp=dest;
				tmp.x-=3;
				tmp.y-=3;
				tmp.h=1;
				tmp.w=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=28;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=36;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=28;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y=dest.y-3;
				tmp.w=1;
				tmp.h=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=36;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=29;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=36;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );

				tmp=dest;
				tmp.x-=5;
				tmp.y-=5;
				tmp.h=1;
				tmp.w=10;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=40;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y=dest.y-5;
				tmp.w=1;
				tmp.h=10;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=40;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=31;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=40;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );

				// Das große Bild neu malen:
				tmp.x=11;tmp.y=13;
				tmp.w=UnitsData.vehicle[ptr->id].info->w;
				tmp.h=UnitsData.vehicle[ptr->id].info->h;
				SDL_BlitSurface ( UnitsData.vehicle[ptr->id].info,NULL,buffer,&tmp );
				
				
				// Ggf die Beschreibung ausgeben:
				if ( SettingsData.bShowDescription )
				{
					tmp.x+=10;tmp.y+=10;
					tmp.w-=20;tmp.h-=20;
					fonts->OutTextBlock ( UnitsData.vehicle[ptr->id].text,tmp,buffer );
				}
				// Die Details anzeigen:
				{
					cVehicle *tv;
					tmp.x=11;
					tmp.y=290;
					tmp.w=260;
					tmp.h=176;
					SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&tmp,buffer,&tmp );
					tv=new cVehicle ( UnitsData.vehicle+ptr->id,game->ActivePlayer );
					tv->ShowBigDetails();
					delete tv;
				}
				// Die Bauzeiten eintragen:
				int iTurboBuildRounds[3];
				int iTurboBuildCosts[3];
				CalcTurboBuild(iTurboBuildRounds, iTurboBuildCosts, owner->VehicleData[ptr->id].iBuilt_Costs );

				sprintf ( str,"%d",iTurboBuildRounds[0]) ;
				fonts->OutTextCenter ( str,389,350,buffer );
				sprintf ( str,"%d",iTurboBuildCosts[0] );
				fonts->OutTextCenter ( str,429,350,buffer );

				if ( iTurboBuildRounds[1] > 0 )
				{
					sprintf ( str,"%d", iTurboBuildRounds[1] );
					fonts->OutTextCenter ( str,389,375,buffer );
					sprintf ( str,"%d", iTurboBuildCosts[1] );
					fonts->OutTextCenter ( str,429,375,buffer );
				}
				if ( iTurboBuildRounds[2] > 0 )
				{
					sprintf ( str,"%d", iTurboBuildRounds[2] );
					fonts->OutTextCenter ( str,389,400,buffer );
					sprintf ( str,"%d", iTurboBuildCosts[2] );
					fonts->OutTextCenter ( str,429,400,buffer );
				}
			}
			else //showInfo == false
			{
				//einfachen Rahmen drum malen
				SDL_Rect tmp;
				tmp=dest;
				tmp.x-=4;
				tmp.y-=4;
				tmp.h=1;
				tmp.w=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=38;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y=dest.y-4;
				tmp.w=1;
				tmp.h=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=38;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=31;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=38;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			}
		}

		// Text ausgeben:
		t=0;
		str[0]=0;
		while ( UnitsData.vehicle[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <70 )
		{
			str[t]=UnitsData.vehicle[ptr->id].data.name[t];str[++t]=0;
		}
		str[t]='.';
		str[t+1]=0;
		fonts->OutText ( str,text.x,text.y,buffer );
		sprintf ( str,"%d",owner->VehicleData[ptr->id].costs );
		fonts->OutTextCenter ( str,616,text.y,buffer );
		text.y+=32+10;
		dest.y+=32+10;
	}
}

//draws the Buildspeed-Buttons
void cBuilding::DrawBuildButtons ( int speed )
{
	SDL_Rect scr,dest;
	dest.w=scr.w=78;
	dest.h=scr.h=23;
	dest.x=292;dest.y=345;
	if ( speed==0 )
	{
		scr.x=39;scr.y=126;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&dest,buffer,&dest );
	}
	dest.y+=24;
	if ( speed==1 )
	{
		scr.x=118;scr.y=126;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&dest,buffer,&dest );
	}
	dest.y+=25;
	if ( speed==2 )
	{
		scr.x=216;scr.y=106;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
	}
	else
	{
		SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&dest,buffer,&dest );
	}
}

// Zeigt die Liste mit den Bauaufträgen an, und wenn show Info==true auch sämtliche Details zur gewählten Einheit 
void cBuilding::ShowToBuildList ( TList *list,int selected,int offset, bool showInfo )
{
	sBuildStruct *ptr;
	SDL_Rect scr,dest,text;
	char str[100];
	int i,t;
	scr.x=330;scr.y=49;
	scr.w=128;scr.h=233;
	SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&scr,buffer,&scr );

	
	scr.x=0;scr.y=0;
	scr.w=32;scr.h=32;
	dest.x=340;dest.y=58;
	dest.w=32;dest.h=32;
	text.x=375;text.y=70;
	for ( i=offset;i<list->Count;i++ )
	{
		if ( i>=offset+5 ) break;
		ptr=list->BuildStructItems[i];
		// Das Bild malen:
		SDL_BlitSurface ( ptr->sf,&scr,buffer,&dest );
		// Ggf noch Rahmen drum:
		if ( selected==i )
		{
			if (showInfo == true)
			{
				
				//dopelten Rahmen drum malen
				SDL_Rect tmp;
				tmp=dest;
				tmp.x-=3;
				tmp.y-=3;
				tmp.h=1;
				tmp.w=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=28;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=36;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=28;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y=dest.y-3;
				tmp.w=1;
				tmp.h=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=36;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=29;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=36;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );

				tmp=dest;
				tmp.x-=5;
				tmp.y-=5;
				tmp.h=1;
				tmp.w=10;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=40;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y=dest.y-5;
				tmp.w=1;
				tmp.h=10;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=40;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=31;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=40;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );

				// Das große Bild neu malen:
				tmp.x=11;tmp.y=13;
				tmp.w=UnitsData.vehicle[ptr->id].info->w;
				tmp.h=UnitsData.vehicle[ptr->id].info->h;
				SDL_BlitSurface ( UnitsData.vehicle[ptr->id].info,NULL,buffer,&tmp );
				
				
				// Ggf die Beschreibung ausgeben:
				if ( SettingsData.bShowDescription )
				{
					tmp.x+=10;tmp.y+=10;
					tmp.w-=20;tmp.h-=20;
					fonts->OutTextBlock ( UnitsData.vehicle[ptr->id].text,tmp,buffer );
				}
				// Die Details anzeigen:
				{
					cVehicle *tv;
					tmp.x=11;
					tmp.y=290;
					tmp.w=260;
					tmp.h=176;
					SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&tmp,buffer,&tmp );
					tv=new cVehicle ( UnitsData.vehicle+ptr->id,game->ActivePlayer );
					tv->ShowBigDetails();
					delete tv;
				}
				
				// Die Bauzeiten eintragen:
				int iTurboBuildRounds[3];
				int iTurboBuildCosts[3];
				CalcTurboBuild(iTurboBuildRounds, iTurboBuildCosts, owner->VehicleData[ptr->id].iBuilt_Costs, ptr->iRemainingMetal );

				tmp.x=373;tmp.y=344;
				tmp.w=77;tmp.h=72;
				SDL_BlitSurface ( GraphicsData.gfx_fac_build_screen,&tmp,buffer,&tmp );

				sprintf ( str,"%d",iTurboBuildRounds[0]);
				fonts->OutTextCenter ( str,389,350,buffer );
				sprintf ( str,"%d",iTurboBuildCosts[0] );
				fonts->OutTextCenter ( str,429,350,buffer );

				if ( iTurboBuildRounds[1] > 0 )
				{
					sprintf ( str,"%d", iTurboBuildRounds[1] );
					fonts->OutTextCenter ( str,389,375,buffer );
					sprintf ( str,"%d", iTurboBuildCosts[1] );
					fonts->OutTextCenter ( str,429,375,buffer );
				}
				if ( iTurboBuildRounds[2] > 0 )
				{
					sprintf ( str,"%d", iTurboBuildRounds[2] );
					fonts->OutTextCenter ( str,389,400,buffer );
					sprintf ( str,"%d", iTurboBuildCosts[2] );
					fonts->OutTextCenter ( str,429,400,buffer );
				}
			}
			else
			{
				SDL_Rect tmp;
				tmp=dest;
				tmp.x-=4;
				tmp.y-=4;
				tmp.h=1;
				tmp.w=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=38;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=30;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y=dest.y-4;
				tmp.w=1;
				tmp.h=8;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x+=38;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.y+=31;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
				tmp.x-=38;
				SDL_FillRect ( buffer,&tmp,0xE0E0E0 );
			}
		}
		// Text ausgeben:
		t=0;
		str[0]=0;
		while ( UnitsData.vehicle[ptr->id].data.name[t]&&fonts->GetTextLen ( str ) <70 )
		{
			str[t]=UnitsData.vehicle[ptr->id].data.name[t];str[++t]=0;
		}
		str[t]='.';
		str[t+1]=0;
		fonts->OutText ( str,text.x,text.y,buffer );
		text.y+=32+10;
		dest.y+=32+10;
	}
}

//calculates the costs and the duration of the 3 buildspeeds for the vehicle with the given id
//iRemainingMetal is only needed for recalculating costs of vehicles in the Buildqueue and is set per default to -1
void cBuilding::CalcTurboBuild(int *iTurboBuildRounds, int *iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal)
{
	//first calc costs for a new Vehical
	iTurboBuildCosts[0] = iVehicleCosts;
	
	iTurboBuildCosts[1] = iTurboBuildCosts[0];
	while (iTurboBuildCosts[1] + 6 <= 2*iTurboBuildCosts[0]) 
	{
		iTurboBuildCosts[1] += 6;
	}

	iTurboBuildCosts[2] = iTurboBuildCosts[1];
	while (iTurboBuildCosts[2] + 12 <= 3*iTurboBuildCosts[0]) 
	{
		iTurboBuildCosts[2] += 12;
	}

	//now this is a litle bit tricky ...
	//trying to calculate a plausible value, if we are changing the speed of an already started build-job
	if ( iRemainingMetal >= 0 )
	{
		float WorkedRounds;
		switch (BuildSpeed)  //BuildSpeed here is the previous build speed
		{
			case 0:
				WorkedRounds = ( iTurboBuildCosts[0] - iRemainingMetal )/ (float) 3;
				iTurboBuildCosts[0] -= 1    *  3 * WorkedRounds;
				iTurboBuildCosts[1] -= 0.5  * 12 * WorkedRounds;
				iTurboBuildCosts[2] -= 0.25 * 36 * WorkedRounds;
				break;
			case 1:
				WorkedRounds = ( iTurboBuildCosts[1] - iRemainingMetal )/ (float) 12;
				iTurboBuildCosts[0] -= 2   *  3 * WorkedRounds;
				iTurboBuildCosts[1] -= 1   * 12 * WorkedRounds;
				iTurboBuildCosts[2] -= 0.5 * 36 * WorkedRounds;
				break;
			case 2:
				WorkedRounds = ( iTurboBuildCosts[2] - iRemainingMetal )/ (float) 36;
				iTurboBuildCosts[0] -= 4 *  3 * WorkedRounds;
				iTurboBuildCosts[1] -= 2 * 12 * WorkedRounds;
				iTurboBuildCosts[2] -= 1 * 36 * WorkedRounds;
				break;
		}
	}

	//TODO: men always need 9 month ;)
	//calc needed Rounds
	iTurboBuildRounds[0] = ( int ) ceil ( iTurboBuildCosts[0] / ( double ) 3  );
	iTurboBuildRounds[1] = ( int ) ceil ( iTurboBuildCosts[1] / ( double ) 12 );
	iTurboBuildRounds[2] = ( int ) ceil ( iTurboBuildCosts[2] / ( double ) 36 );
}

// Liefert die X-Position des Buildings auf dem Screen zurück:
int cBuilding::GetScreenPosX ( void )
{
	return 180- ( ( int ) ( ( game->hud->OffX ) / ( 64.0/game->hud->Zoom ) ) ) +game->hud->Zoom*PosX;
}

// Liefert die Y-Position des Buildings auf dem Screen zurück:
int cBuilding::GetScreenPosY ( void )
{
	return 18- ( ( int ) ( ( game->hud->OffY ) / ( 64.0/game->hud->Zoom ) ) ) +game->hud->Zoom*PosY;
}

// Berechnet die Hitpoints nach einem Treffer:
int cBuilding::CalcHelth ( int damage )
{
	int hp;
	hp=data.hit_points+data.armor-damage;
	if ( hp>data.hit_points ) hp=data.hit_points;
	if ( hp<0 ) return 0;
	return hp;
}

// Malt das Buildingmenü:
void cBuilding::DrawMenu ( void )
{
	int nr=0,SelMenu=-1,ExeNr=-1;
	static int LastNr=-1;
	SDL_Rect scr,dest;
	dest=GetMenuSize();
	dest.w=scr.w=42;
	dest.h=scr.h=21;

	if ( ActivatingVehicle ) {MenuActive=false;return;}
	if ( BuildList&&BuildList->Count&&!IsWorking&& ( BuildList->BuildListItems[0] )->metall_remaining<=0 ) return;

	if ( mouse->GetMouseButton() &&MouseOverMenu ( mouse->x,mouse->y ) )
	{
		SelMenu= ( mouse->y-dest.y ) /22;
		LastNr=SelMenu;
	}
	else if ( MouseOverMenu ( mouse->x,mouse->y ) )
	{
		ExeNr=LastNr;
		LastNr=-1;
	}
	else
	{
		SelMenu=-1;
		LastNr=-1;
	}

	// Angriff:
	if ( typ->data.can_attack&&data.shots )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			AttackMode=true;
			game->hud->CheckScroll();
			MouseMoveCallback ( true );
			return;
		}
		scr.x=588;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Bauen:
	if ( typ->data.can_build )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			ShowBuildMenu();
			return;
		}
		scr.x=0;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Verteilen:
	if ( typ->data.is_mine&&IsWorking )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			ShowMineManager();
			return;
		}
		scr.x=504;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Transfer:
	if ( typ->data.can_load==TRANS_METAL||typ->data.can_load==TRANS_OIL||typ->data.can_load==TRANS_GOLD )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			Transfer=true;
			return;
		}
		scr.x=42;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Start:
	if ( typ->data.can_work&&!IsWorking&& ( ( BuildList&&BuildList->Count ) ||typ->data.can_build==BUILD_NONE ) )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			if ( !StartWork() )
			{
				PlayFX ( SoundData.SNDQuitsch );
			}
			return;
		}
		scr.x=672;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Stop:
	if ( IsWorking )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			StopWork ( false );
			return;
		}
		scr.x=210;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Aktivieren/Laden:
	if ( typ->data.can_load==TRANS_VEHICLES||typ->data.can_load==TRANS_MEN||typ->data.can_load==TRANS_AIR )
	{
		// Aktivieren:
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			ShowStorage();
			return;
		}
		scr.x=462;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
		// Laden:
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			LoadActive=true;
			return;
		}
		scr.x=420;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Forschen:
	if ( typ->data.can_research&&IsWorking )
	{
		// Aktivieren:
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			ShowResearch();
			return;
		}
		scr.x=714;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Upgradeschirm:
	if ( data.gold_need )
	{
		// Dies Updaten:
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			ShowUpgrade();
			return;
		}
		scr.x=798;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Updates:
	if ( data.version!=owner->BuildingData[typ->nr].version&&SubBase->Metal>=2 )
	{
		// Alle Updaten:
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			int i,k,sum=0,count=0;
			char str[50];
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );

			for ( i=0;i<owner->base->SubBases->Count;i++ )
			{
				sSubBase *sb;
				sb=owner->base->SubBases->SubBaseItems[i];
				for ( k=0;k<sb->buildings->Count;k++ )
				{
					cBuilding *b;
					b=sb->buildings->BuildItems[k];
					if ( b->typ!=typ ) continue;
					sum++;
					if ( SubBase->Metal<2 ) continue;

					UpdateBuilding ( b->data,owner->BuildingData[typ->nr] );
					b->GenerateName();
					if ( b==game->SelectedBuilding ) ShowDetails();
					owner->base->AddMetal ( SubBase,-2 );
					count++;
				}
			}
			owner->DoScan();
			sprintf ( str,"%d von %d aufgerüstet.",count,sum );
			game->AddMessage ( str );

			return;
		}
		scr.x=756;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;

		// Dies Updaten:
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );

			UpdateBuilding ( data,owner->BuildingData[typ->nr] );
			GenerateName();
			owner->base->AddMetal ( SubBase,-2 );
			if ( this==game->SelectedBuilding ) ShowDetails();
			owner->DoScan();

			return;
		}
		scr.x=798;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Entfernen:
	if ( !data.is_road )
	{
		if ( SelMenu==nr ) scr.y=21;else scr.y=0;
		if ( ExeNr==nr )
		{
			MenuActive=false;
			PlayFX ( SoundData.SNDObjectMenu );
			SelfDestructionMenu();
			return;
		}
		scr.x=252;
		SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
		dest.y+=22;nr++;
	}
	// Info:
	if ( SelMenu==nr ) scr.y=21;else scr.y=0;
	if ( ExeNr==nr )
	{
		MenuActive=false;
		PlayFX ( SoundData.SNDObjectMenu );
		ShowHelp();
		return;
	}
	scr.x=840;
	SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
	dest.y+=22;nr++;
	// Fertig:
	if ( SelMenu==nr ) scr.y=21;else scr.y=0;
	if ( ExeNr==nr )
	{
		MenuActive=false;
		PlayFX ( SoundData.SNDObjectMenu );
		return;
	}
	scr.x=126;
	SDL_BlitSurface ( GraphicsData.gfx_object_menu,&scr,buffer,&dest );
}

// Zentriert auf dieses Building:
void cBuilding::Center ( void )
{
	game->hud->OffX=PosX*64- ( ( int ) ( ( ( float ) 224/game->hud->Zoom ) *64 ) ) +32;
	game->hud->OffY=PosY*64- ( ( int ) ( ( ( float ) 224/game->hud->Zoom ) *64 ) ) +32;
	game->fDrawMap=true;
	game->hud->DoScroll ( 0 );
}

// Malt die Munitionsanzeige über das Buildings:
void cBuilding::DrawMunBar ( void )
{
	SDL_Rect r1,r2;
	int w=game->hud->Zoom;
	if ( data.is_big ) w<<=1;
	r2.x= ( r1.x=GetScreenPosX() ) +1;
	r1.w=w;
	r2.h= ( r1.h=w/6 )-2;
	r2.y= (int)( r1.y=w-r1.h+GetScreenPosY() ) +1;
	r2.w= (int)( ( ( float ) ( r1.w-2 ) ) /data.max_ammo ) *data.ammo;
	if ( data.ammo>data.max_ammo/2 )
	{
		SDL_FillRect ( buffer,&r1,0 );
		SDL_FillRect ( buffer,&r2,0x04AE04 );
	}
	else if ( data.ammo>data.max_ammo/4 )
	{
		SDL_FillRect ( buffer,&r1,0 );
		SDL_FillRect ( buffer,&r2,0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer,&r1,0 );
		SDL_FillRect ( buffer,&r2,0xE60000 );
	}
}

// Malt die Trefferanzeige über das Buildings:
void cBuilding::DrawHelthBar ( void )
{
	SDL_Rect r1,r2;
	int w=game->hud->Zoom;
	if ( data.is_big ) w<<=1;
	r2.x= ( r1.x=GetScreenPosX() ) +1;
	r1.w=w;
	r2.h= ( r1.h=w/6 )-2;
	r2.y= (int)( r1.y=GetScreenPosY() ) +1;
	r2.w= (int)( ( ( float ) ( r1.w-2 ) ) /data.max_hit_points ) *data.hit_points;
	if ( data.hit_points>data.max_hit_points/2 )
	{
		SDL_FillRect ( buffer,&r1,0 );
		SDL_FillRect ( buffer,&r2,0x04AE04 );
	}
	else if ( data.hit_points>data.max_hit_points/4 )
	{
		SDL_FillRect ( buffer,&r1,0 );
		SDL_FillRect ( buffer,&r2,0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer,&r1,0 );
		SDL_FillRect ( buffer,&r2,0xE60000 );
	}
}

void cBuilding::Select ( void )
{
	selected=true;
	// Das Video laden:
	if ( game->FLC!=NULL )
	{
		FLI_Close ( game->FLC );
		game->FLC=NULL;
		game->FLCname="";
	}
	if ( game->video!=typ->video )
	{
		game->video=typ->video;
	}
	// Sound abspielen:
	if ( !IsWorking ) PlayFX ( SoundData.SNDHudButton );
	// Die Eigenschaften anzeigen:
	ShowDetails();
}

void cBuilding::Deselct ( void )
{
	SDL_Rect scr,dest;
	selected=false;
	MenuActive=false;
	AttackMode=false;
	Transfer=false;
	LoadActive=false;
	ActivatingVehicle=false;
	// Den Hintergrund wiederherstellen:
	scr.x=0;
	scr.y=215;
	dest.w=scr.w=155;
	dest.h=scr.h=48;
	dest.x=8;
	dest.y=171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	StopFXLoop ( game->ObjectStream );
	game->ObjectStream=-1;
}

void cBuilding::ShowDetails ( void )
{
	SDL_Rect scr,dest;
	// Den Hintergrund wiederherstellen:
	scr.x=0;
	scr.y=215;
	dest.w=scr.w=155;
	dest.h=scr.h=48;
	dest.x=8;
	dest.y=171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,GraphicsData.gfx_hud,&dest );
	// Die Hitpoints anzeigen:
	DrawNumber ( 31,177,data.hit_points,data.max_hit_points,GraphicsData.gfx_hud );
	fonts->OutTextSmall ( "Treffer",55,177,ClWhite,GraphicsData.gfx_hud );
	DrawSymbol ( SHits,88,174,70,data.hit_points,data.max_hit_points,GraphicsData.gfx_hud );
	// Zusätzliche Werte:
	if ( data.max_shield )
	{
		// Schild:
		DrawNumber ( 31,189,data.shield,data.max_shield,GraphicsData.gfx_hud );
		fonts->OutTextSmall ( "Schild",55,189,ClWhite,GraphicsData.gfx_hud );
		DrawSymbol ( SShield,88,186,70,data.shield,data.max_shield,GraphicsData.gfx_hud );
	}
	if ( data.can_load&&owner==game->ActivePlayer )
	{
		// Load:
		DrawNumber ( 31,189,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
		fonts->OutTextSmall ( "Ladung",55,189,ClWhite,GraphicsData.gfx_hud );
		switch ( data.can_load )
		{
			case TRANS_METAL:
				DrawSymbol ( SMetal,88,186,70,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
				break;
			case TRANS_OIL:
				DrawSymbol ( SOil,88,186,70,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
				break;
			case TRANS_GOLD:
				DrawSymbol ( SGold,88,187,70,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
				break;
			case TRANS_VEHICLES:
				DrawSymbol ( STrans,88,186,70,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
				break;
			case TRANS_MEN:
				DrawSymbol ( SHuman,88,186,70,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
				break;
			case TRANS_AIR:
				DrawSymbol ( SAir,88,186,50,data.cargo,data.max_cargo,GraphicsData.gfx_hud );
				break;
		}
		// Gesamt:
		if ( data.can_load==TRANS_METAL||data.can_load==TRANS_OIL||data.can_load==TRANS_GOLD )
		{
			fonts->OutTextSmall ( "Gesamt",55,201,ClWhite,GraphicsData.gfx_hud );
			switch ( data.can_load )
			{
				case TRANS_METAL:
					DrawNumber ( 31,201,SubBase->Metal,SubBase->MaxMetal,GraphicsData.gfx_hud );
					DrawSymbol ( SMetal,88,198,70,SubBase->Metal,SubBase->MaxMetal,GraphicsData.gfx_hud );
					break;
				case TRANS_OIL:
					DrawNumber ( 31,201,SubBase->Oil,SubBase->MaxOil,GraphicsData.gfx_hud );
					DrawSymbol ( SOil,88,198,70,SubBase->Oil,SubBase->MaxOil,GraphicsData.gfx_hud );
					break;
				case TRANS_GOLD:
					DrawNumber ( 31,201,SubBase->Gold,SubBase->MaxGold,GraphicsData.gfx_hud );
					DrawSymbol ( SGold,88,199,70,SubBase->Gold,SubBase->MaxGold,GraphicsData.gfx_hud );
					break;
			}
		}
	}
	else if ( data.can_attack&&!data.is_expl_mine )
	{
		if ( owner==game->ActivePlayer )
		{
			// Munition:
			DrawNumber ( 31,189,data.ammo,data.max_ammo,GraphicsData.gfx_hud );
			fonts->OutTextSmall ( "Munni",55,189,ClWhite,GraphicsData.gfx_hud );
			DrawSymbol ( SAmmo,88,187,70,data.ammo,data.max_ammo,GraphicsData.gfx_hud );
		}
		// Schüsse:
		DrawNumber ( 31,212,data.shots,data.max_shots,GraphicsData.gfx_hud );
		fonts->OutTextSmall ( "Schüss",55,212,ClWhite,GraphicsData.gfx_hud );
		DrawSymbol ( SShots,88,212,70,data.shots,data.max_shots,GraphicsData.gfx_hud );
	}
	else if ( data.energy_prod )
	{
		// EnergieProduktion:
		DrawNumber ( 31,189, ( IsWorking?data.energy_prod:0 ),data.energy_prod,GraphicsData.gfx_hud );
		fonts->OutTextSmall ( "Energie",55,189,ClWhite,GraphicsData.gfx_hud );
		DrawSymbol ( SEnergy,88,187,70, ( IsWorking?data.energy_prod:0 ),data.energy_prod,GraphicsData.gfx_hud );
		if ( owner==game->ActivePlayer )
		{
			// Gesammt:
			fonts->OutTextSmall ( "Gesamt",55,201,ClWhite,GraphicsData.gfx_hud );
			DrawNumber ( 31,201,SubBase->EnergyProd,SubBase->MaxEnergyProd,GraphicsData.gfx_hud );
			DrawSymbol ( SEnergy,88,199,70,SubBase->EnergyProd,SubBase->MaxEnergyProd,GraphicsData.gfx_hud );
			// Verbrauch:
			fonts->OutTextSmall ( "Verbr.",55,212,ClWhite,GraphicsData.gfx_hud );
			DrawNumber ( 31,212,SubBase->EnergyNeed,SubBase->MaxEnergyNeed,GraphicsData.gfx_hud );
			DrawSymbol ( SEnergy,88,212,70,SubBase->EnergyNeed,SubBase->MaxEnergyNeed,GraphicsData.gfx_hud );
		}
	}
	else if ( data.human_prod )
	{
		// HumanProduktion:
		DrawNumber ( 31,189,data.human_prod,data.human_prod,GraphicsData.gfx_hud );
		fonts->OutTextSmall ( "Teams",55,189,ClWhite,GraphicsData.gfx_hud );
		DrawSymbol ( SHuman,88,187,70,data.human_prod,data.human_prod,GraphicsData.gfx_hud );
		if ( owner==game->ActivePlayer )
		{
			// Gesammt:
			fonts->OutTextSmall ( "Gesamt",55,201,ClWhite,GraphicsData.gfx_hud );
			DrawNumber ( 31,201,SubBase->HumanProd,SubBase->HumanProd,GraphicsData.gfx_hud );
			DrawSymbol ( SHuman,88,199,70,SubBase->HumanProd,SubBase->HumanProd,GraphicsData.gfx_hud );
			// Verbrauch:
			fonts->OutTextSmall ( "Verbr.",55,212,ClWhite,GraphicsData.gfx_hud );
			DrawNumber ( 31,212,SubBase->HumanNeed,SubBase->MaxHumanNeed,GraphicsData.gfx_hud );
			DrawSymbol ( SHuman,88,210,70,SubBase->HumanNeed,SubBase->MaxHumanNeed,GraphicsData.gfx_hud );
		}
	}
	else if ( data.human_need )
	{
		// HumanNeed:
		if ( IsWorking )
		{
			DrawNumber ( 31,189,data.human_need,data.human_need,GraphicsData.gfx_hud );
			fonts->OutTextSmall ( "Verbr.",55,189,ClWhite,GraphicsData.gfx_hud );
			DrawSymbol ( SHuman,88,187,70,data.human_need,data.human_need,GraphicsData.gfx_hud );
		}
		else
		{
			DrawNumber ( 31,189,0,data.human_need,GraphicsData.gfx_hud );
			fonts->OutTextSmall ( "Verbr.",55,189,ClWhite,GraphicsData.gfx_hud );
			DrawSymbol ( SHuman,88,187,70,0,data.human_need,GraphicsData.gfx_hud );
		}
		if ( owner==game->ActivePlayer )
		{
			// Gesammt:
			fonts->OutTextSmall ( "Gesamt",55,201,ClWhite,GraphicsData.gfx_hud );
			DrawNumber ( 31,201,SubBase->HumanNeed,SubBase->MaxHumanNeed,GraphicsData.gfx_hud );
			DrawSymbol ( SHuman,88,199,70,SubBase->HumanNeed,SubBase->MaxHumanNeed,GraphicsData.gfx_hud );
		}
	}
}

// Malt eine Reihe von Symbolen:
void cBuilding::DrawSymbol ( eSymbols sym,int x,int y,int maxx,int value,int maxvalue,SDL_Surface *sf )
{
	SDL_Rect full,empty,dest;
	int i,to,step,offx;
	switch ( sym )
	{
		case SSpeed:
			full.x=0;
			empty.y=full.y=98;
			empty.w=full.w=7;
			empty.h=full.h=7;
			empty.x=7;
			break;
		case SHits:
			empty.y=full.y=98;
			empty.w=full.w=6;
			empty.h=full.h=9;
			if ( value>maxvalue/2 )
			{
				full.x=14;
				empty.x=20;
			}
			else if ( value>maxvalue/4 )
			{
				full.x=26;
				empty.x=32;
			}
			else
			{
				full.x=38;
				empty.x=44;
			}
			break;
		case SAmmo:
			full.x=50;
			empty.y=full.y=98;
			empty.w=full.w=5;
			empty.h=full.h=7;
			empty.x=55;
			break;
		case SMetal:
			full.x=60;
			empty.y=full.y=98;
			empty.w=full.w=7;
			empty.h=full.h=10;
			empty.x=67;
			break;
		case SEnergy:
			full.x=74;
			empty.y=full.y=98;
			empty.w=full.w=7;
			empty.h=full.h=7;
			empty.x=81;
			break;
		case SShots:
			full.x=88;
			empty.y=full.y=98;
			empty.w=full.w=8;
			empty.h=full.h=4;
			empty.x=96;
			break;
		case SOil:
			full.x=104;
			empty.y=full.y=98;
			empty.w=full.w=8;
			empty.h=full.h=9;
			empty.x=112;
			break;
		case SGold:
			full.x=120;
			empty.y=full.y=98;
			empty.w=full.w=9;
			empty.h=full.h=8;
			empty.x=129;
			break;
		case STrans:
			full.x=138;
			empty.y=full.y=98;
			empty.w=full.w=16;
			empty.h=full.h=8;
			empty.x=154;
			break;
		case SHuman:
			full.x=170;
			empty.y=full.y=98;
			empty.w=full.w=8;
			empty.h=full.h=9;
			empty.x=178;
			break;
		case SAir:
			full.x=186;
			empty.y=full.y=98;
			empty.w=full.w=21;
			empty.h=full.h=8;
			empty.x=207;
			break;
		case SShield:
			full.x=228;
			empty.y=full.y=98;
			empty.w=full.w=8;
			empty.h=full.h=8;
			empty.x=236;
			break;
	}

	to=maxvalue;
	step=1;
	offx=full.w;
	while ( offx*to>maxx )
	{
		offx--;
		if ( offx<4 )
		{
			to/=2;
			step*=2;
			offx=full.w;
		}
	}

	dest.x=x;
	dest.y=y;
	dest.w=full.w;
	dest.h=full.h;
	for ( i=0;i<to;i++ )
	{
		if ( value>0 )
		{
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&full,sf,&dest );
		}
		else
		{
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&empty,sf,&dest );
		}
		dest.x+=offx;
		value-=step;
	}
}

// Malt eine Nummer/Nummer auf das Surface:
void cBuilding::DrawNumber ( int x,int y,int value,int maxvalue,SDL_Surface *sf )
{
	char str[20];
	sprintf ( str,"%d/%d",value,maxvalue );
	if ( value>maxvalue/2 )
	{
		fonts->OutTextSmallCenter ( str,x,y,ClGreen,sf );
	}
	else if ( value>maxvalue/4 )
	{
		fonts->OutTextSmallCenter ( str,x,y,ClYellow,sf );
	}
	else
	{
		fonts->OutTextSmallCenter ( str,x,y,ClRed,sf );
	}
}

// Spielt den Soundstream am, der zu diesem Vehicle gehört:
int cBuilding::PlayStram ( void )
{
	if ( IsWorking ) return PlayFXLoop ( typ->Running );
	return 0;
}

// Zeigt den Hilfebildschirm an:
void cBuilding::ShowHelp ( void )
{
	int LastMouseX=0,LastMouseY=0,LastB=0,x,b,y;
	bool FertigPressed=false;
	SDL_Rect scr,dest;

	PlayFX ( SoundData.SNDHudButton );
	mouse->SetCursor ( CHand );
	mouse->draw ( false,buffer );
	// Den Hilfebildschirm blitten:
	SDL_BlitSurface ( GraphicsData.gfx_help_screen,NULL,buffer,NULL );
	// Das Infobild blitten:
	dest.x=11;
	dest.y=13;
	dest.w=typ->info->w;
	dest.h=typ->info->h;
	SDL_BlitSurface ( typ->info,NULL,buffer,&dest );
	// Den Text anzeigen:
	dest.x=345;
	dest.y=62;
	dest.w=277;
	dest.h=181;
	fonts->OutTextBlock ( typ->text,dest,buffer );
	// Die Details anzeigen:
	ShowBigDetails();
	// Den Buffer anzeigen:
	SHOW_SCREEN
	mouse->GetBack ( buffer );
	while ( 1 )
	{
		// Die Engine laufen lassen:
		game->engine->Run();
		game->HandleTimer();

		// Events holen:
		SDL_PumpEvents();

		// Die Maus machen:
		mouse->GetPos();
		b=mouse->GetMouseButton();
		x=mouse->x;y=mouse->y;
		if ( x!=LastMouseX||y!=LastMouseY )
		{
			mouse->draw ( true,screen );
		}
		// Fertig-Button:
		if ( x>=484&&x<484+63&&y>=452&&y<452+24 )
		{
			if ( b&&!FertigPressed )
			{
				PlayFX ( SoundData.SNDMenuButton );
				scr.x=68;
				scr.y=172;
				dest.w=scr.w=61;
				dest.h=scr.h=22;
				dest.x=484;
				dest.y=452;
				SDL_BlitSurface ( GraphicsData.gfx_hud_stuff,&scr,buffer,&dest );
				SHOW_SCREEN
				mouse->draw ( false,screen );
				FertigPressed=true;
			}
			else if ( !b&&LastB )
			{
				return;
			}
		}
		else if ( FertigPressed )
		{
			scr.x=484;
			scr.y=452;
			dest.w=scr.w=61;
			dest.h=scr.h=22;
			dest.x=484;
			dest.y=452;
			SDL_BlitSurface ( GraphicsData.gfx_help_screen,&scr,buffer,&dest );
			SHOW_SCREEN
			mouse->draw ( false,screen );
			FertigPressed=false;
		}

		LastMouseX=x;LastMouseY=y;
		LastB=b;
	}
}

// Sendet die Update-Nachricht für das gespeicherte Vehicle mit dem Index:
void cBuilding::SendUpdateStored ( int index )
{
	/*if(!game->engine->fstcpip||game->engine->fstcpip->server)*/return;
	/*unsigned char msg[3+12+sizeof(sUnitData)];
	msg[0]='#';
	msg[1]=3+12+sizeof(sUnitData);
	msg[2]=MSG_UPDATE_STORED;
	((int*)(msg+3))[0]=owner->Nr;
	((int*)(msg+3))[1]=index;
	((int*)(msg+3))[2]=PosX+PosY*game->map->size; 
	((sUnitData*)(msg+3+12))[0]=((cVehicle*)(StoredVehicles->Items[index]))->data;
	game->engine->fstcpip->Send(msg,msg[1]);*/
}
