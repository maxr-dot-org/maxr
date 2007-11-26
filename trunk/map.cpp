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
//#include <time.h>
#include <stdlib.h>
#include "map.h"
#include "game.h"

// Funktionen der Map-Klasse /////////////////////////////////////////////////
cMap::cMap ( void )
{
	TerrainInUse=new TList;
	Kacheln=NULL;
	NewMap ( 32 );
	MapName="";
}

cMap::~cMap ( void )
{
	DeleteMap();
	delete TerrainInUse;
}

// Gibt zurück, ob eine Kachel als Wasser gilt, oder nicht:
bool cMap::IsWater ( int off,bool not_coast,bool is_ship )
{
	if ( !TerrainData.terrain[Kacheln[off]].water&&!TerrainData.terrain[Kacheln[off]].coast ) return false;


//  if(game->ActivePlayer->ScanMap[off]&&GO[off].base&&(GO[off].base->data.is_platform||GO[off].base->data.is_bridge)){
	/*if(game->ActivePlayer->ScanMap[off]&&GO[off].base&&(GO[off].base->data.is_platform||(is_ship?0:GO[off].base->data.is_bridge))){
	  return false;
	}*/
	if ( not_coast )
	{
		return TerrainData.terrain[Kacheln[off]].water;
	}
	else
	{
		return TerrainData.terrain[Kacheln[off]].water||TerrainData.terrain[Kacheln[off]].coast;
	}
}

// Struktur zum laden:
struct sTuple
{
	int from;
	int to;
};

// Läd das Mapfile:
bool cMap::LoadMap ( string filename )
{
	FILE *fp;
	char str[100];
	TList *index;
	int nr,i,k;

	MapName = filename;
	filename = SettingsData.sMapsPath + PATH_DELIMITER + filename;
	ErrorStr="";
	fp=fopen ( filename.c_str(),"rb" );
	if ( !fp )
	{
		ErrorStr="file not found";
		return false;
	}

	// Header:
	fread ( str,1,21,fp );
	if ( strncmp ( str,"[MMs M.A.X.-Map-File]",21 ) !=0 )
	{
		fclose ( fp );
		ErrorStr="wrong format";
		return false;
	}

	DeleteMap();

	// Mapgröße:
	fread ( &size,sizeof ( int ),1,fp );
	NewMap ( size );
	// DefaultWater:
	fread ( &DefaultWater,sizeof ( int ),1,fp );
	// Den Index erstellen:
	index=new TList;
	fseek ( fp,sizeof ( int ) *size*size,SEEK_CUR );
	while ( 1 )
	{
		if ( !fread ( &nr,sizeof ( int ),1,fp ) ) break;
		i=0;
		while ( 1 )
		{
			str[i]=fgetc ( fp );
			if ( str[i]==0 ) break;
			i++;
		}

		for ( i=0;i<TerrainData.terrain_anz;i++ )
		{
			if ( strcmp ( TerrainData.terrain[i].id,str ) ==0 )
			{
				sTuple *t;
				t=new sTuple;
				t->from=nr;
				t->to=i;
				index->AddTuple ( t );
				TerrainInUse->AddTerrain ( TerrainData.terrain+i );
				break;
			}
		}
		if ( i==TerrainData.terrain_anz )
		{
			ErrorStr="terrain not found";
			cLog::write((string)"terrain not found: " + str,LOG_TYPE_WARNING);
			while ( index->Count )
			{
				index->DeleteTuple ( index->Count );
			}
			delete index;
			fclose ( fp );
			NewMap ( 16 );
			return false;
		}
	}
	// Die Karte laden:
	fseek ( fp,21+4+4,SEEK_SET );
	for ( i=0;i<size*size;i++ )
	{
		sTuple *t;
		fread ( &nr,sizeof ( int ),1,fp );
		for ( k=0;k<index->Count;k++ )
		{
			t=index->TupleItems[k];
			if ( t->from==nr ) break;
		}
		Kacheln[i]=t->to;
	}

	while ( index->Count )
	{
		// delete (sTuple*)(index->Items[index->Count]);
		index->Delete ( index->Count );
	}
	delete index;
	fclose ( fp );
	return true;
}

// Erstellt eine neue Map:
void cMap::NewMap ( int size )
{

	if ( size<16 ) size=16;
	if ( size>256 ) size=256;

	DeleteMap();
	this->size=size;
	Kacheln= ( int* ) malloc ( sizeof ( int ) *size*size );
	memset ( Kacheln,0,sizeof ( int ) *size*size );

	DefaultWater=0;

	GO= ( sGameObjects* ) malloc ( sizeof ( sGameObjects ) *size*size );
	memset ( GO,0,sizeof ( sGameObjects ) *size*size );
	Resources= ( sResources* ) malloc ( sizeof ( Resources ) *size*size );
}

// Löscht die aktuelle Map:
void cMap::DeleteMap ( void )
{
	if ( !Kacheln ) return;
	free ( Kacheln );
	free ( GO );
	free ( Resources );
	Kacheln=NULL;
	while ( TerrainInUse->Count )
	{
		TerrainInUse->DeleteTerrain ( 0 );
	}
}

// Platziert die Ressourcen (0-wenig,3-viel):
void cMap::PlaceRessources ( int Metal,int Oil,int Gold,int Dichte )
{
	int *GaussMap;
	int amount;
	int PosMap[10];
	int i,k,pos,index;
	int next=0;
	int x,y;
	int nest=0;

	memset ( Resources,0,sizeof ( Resources ) *size*size );
	if ( Metal>3 ) Metal=3;
	if ( Oil>3 ) Oil=3;
	if ( Gold>3 ) Gold=3;
	if ( Dichte>3 ) Dichte=3;
	GaussMap= ( int* ) malloc ( size*size*sizeof ( int ) );
	memset ( GaussMap,0,sizeof ( int ) *size*size );

	// Dafür sorgen, dass mehr Rohstoffe auf dem Land sind:
	for ( i=0;i<size*size;i++ )
	{
		if ( TerrainData.terrain[Kacheln[i]].water ) GaussMap[i]=5;
	}

	// Berechnen, wieviele Quellen es geben soll:
	if ( Dichte==0 ) amount=size/14;
	else if ( Dichte==1 ) amount=size/10;
	else if ( Dichte==2 ) amount=size/8;
	else amount=size/6;
	amount*=amount;
	if ( amount<4 ) amount=4;

	// Rohstoffe platzieren:
	while ( amount-- )
	{
		for ( i=0;i<10;i++ )
		{
			PosMap[i]=random ( size*size,0 );
		}
		pos=GaussMap[PosMap[0]];
		index=0;
		for ( i=1;i<10;i++ )
		{
			if ( GaussMap[PosMap[i]]<pos )
			{
				pos=GaussMap[PosMap[i]];
				index=i;
			}
		}
		pos=PosMap[index];

		if ( TerrainData.terrain[Kacheln[pos]].blocked )
		{
			amount++;
			continue;
		}

		// Ggf ein Nest erstellen:
		if ( random ( 15,0 ) ==0 )
			nest=random ( 3,0 ) +1;

		do
		{
			if ( next==0 )
			{
				Resources[pos].typ=RES_METAL;
				Resources[pos].value+=4*Metal+4+random ( 5,0 )-2;
			}
			else if ( next==1 )
			{
				Resources[pos].typ=RES_OIL;
				Resources[pos].value+=4*Oil+4+random ( 5,0 )-2;
			}
			else
			{
				Resources[pos].typ=RES_GOLD;
				Resources[pos].value+=4*Gold+4+random ( 5,0 )-2;
			}
			if ( Resources[pos].value>16 )
			{
				Resources[pos].value=16;
			}
			next++;
			if ( next>2 ) next=0;

			// Ressurcen dumherum platzieren:
			if ( pos>30000 )
				int k = 1;
			x=pos%size;
			y=pos/size;
			if ( x>0&&y>0 )
			{
				x--;y--;
				Resources[x+y*size].typ=random ( 3,0 ) +1;
				Resources[x+y*size].value+=random ( 4,0 );
				if ( Resources[x+y*size].value>16 )
				{
					Resources[x+y*size].value=16;
				}
				x++;y++;
			}
			if ( x<size-1&&y>0 )
			{
				x++;y--;
				Resources[x+y*size].typ=random ( 3,0 ) +1;
				Resources[x+y*size].value+=random ( 4,0 );
				if ( Resources[x+y*size].value>16 )
				{
					Resources[x+y*size].value=16;
				}
				x--;y++;
			}
			if ( x>0&&y<size-1 )
			{
				x--;y++;
				Resources[x+y*size].typ=random ( 3,0 ) +1;
				Resources[x+y*size].value+=random ( 4,0 );
				if ( Resources[x+y*size].value>16 )
				{
					Resources[x+y*size].value=16;
				}
				x++;y--;
			}
			if ( x<size-1&&y<size-1 )
			{
				x++;y++;
				Resources[x+y*size].typ=random ( 3,0 ) +1;
				Resources[x+y*size].value=1+random ( 4,0 );
				x--;y--;
			}

			if ( nest )
			{
				int old=pos;
				do
				{
					pos=old;
					pos+=random ( 6,0 )-3;
					pos+= ( random ( 6,0 )-3 ) *size;
				}
				while ( pos<0||pos>=size*size );
			}
		}
		while ( ( nest-- ) >0 );

		// GaussMap machen:
		int dick;
		dick=4*6- ( Dichte+1 ) *6;
		for ( i=1;i<dick;i++ )
		{
			x=pos%size;
			for ( k=1;k<dick;k++ )
			{
				y=pos/size;
				if ( y-k>=0 )
				{
					if ( x+i<size ) GaussMap[x+i+ ( y-k ) *size]+=4*4-k;
					if ( x-i>=0 ) GaussMap[x-i+ ( y-k ) *size]+=4*4-k;
				}
				if ( y+k<size )
				{
					if ( x+i<size ) GaussMap[x+i+ ( y+k ) *size]+=4*4-k;
					if ( x-i>=0 ) GaussMap[x-i+ ( y+k ) *size]+=4*4-k;
				}
			}
		}
		GaussMap[pos]+=999;

	}
	free ( GaussMap );
}
