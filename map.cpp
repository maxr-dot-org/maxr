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
	TerrainInUse=new cList<sTerrain*>;
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

SDL_Surface *cMap::LoadTerrGraph ( SDL_RWops *fpMapFile, int iGraphicsPos, sColor Palette[256], int iNum, int iOffToWater, int iWaterCount, bool &overlay  )
{
	// Create new surface and copy palette
	overlay = false;
	SDL_Surface *surface;
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64,8,0,0,0,0);
	surface->pitch = surface->w;

	surface->format->palette->ncolors = 256;
	for (int i = 0; i < 256; i++ )
	{
		surface->format->palette->colors[i].r = Palette[i].cBlue;
		surface->format->palette->colors[i].g = Palette[i].cGreen;
		surface->format->palette->colors[i].b = Palette[i].cRed;
		// If is not a water graphic it may could be an overlay graphic so index 96 must hava an other color
		if ( i == 96 && ( iNum >= iOffToWater+iWaterCount || iNum < iOffToWater ) )
		{
			surface->format->palette->colors[i].r = 255;
			surface->format->palette->colors[i].g = 0;
			surface->format->palette->colors[i].b = 255;
		}
	}

	// Go to position of filedata
	SDL_RWseek ( fpMapFile, iGraphicsPos + 64*64*( iNum ), SEEK_SET );

	// Read pixel data and write to surface
	for( int iY = 64-1; iY >= 0; iY-- )
	{
		for( int iX = 0; iX < 64; iX++ )
		{
			unsigned char cColorOffset;
			SDL_RWread ( fpMapFile, &cColorOffset, 1, 1 );
			Uint8 *pixel = (Uint8*) surface->pixels  + (iY * 64 + iX);
			// If is not a water graphic set all pixels in water color to index 96 with will be a color key
			if( cColorOffset > 96 && cColorOffset <= 127 && ( iNum >= iOffToWater+iWaterCount || iNum < iOffToWater ) )
			{
				*pixel = 96;
				overlay = true;
			}
			else
			{
				*pixel = cColorOffset;
			}
		}
	}
	return surface;
}

void cMap::CopySrfToTerData ( SDL_Surface *surface, int iNum, int iSizeX )
{
	TerrainData.terrain[iNum].sf_org = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_BlitSurface( surface, NULL, TerrainData.terrain[iNum].sf_org, NULL );

	TerrainData.terrain[iNum].sf = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect( TerrainData.terrain[iNum].sf, NULL, 0xFF00FF );
	SDL_BlitSurface( TerrainData.terrain[iNum].sf_org, NULL, TerrainData.terrain[iNum].sf, NULL );

	TerrainData.terrain[iNum].shw_org = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect( TerrainData.terrain[iNum].shw_org, NULL, 0xFF00FF );
	SDL_BlitSurface( TerrainData.terrain[iNum].sf_org, NULL, TerrainData.terrain[iNum].shw_org, NULL );

	SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,TerrainData.terrain[iNum].shw_org,NULL );

	TerrainData.terrain[iNum].shw = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect( TerrainData.terrain[iNum].shw, NULL, 0xFF00FF );
	SDL_BlitSurface( TerrainData.terrain[iNum].shw_org, NULL, TerrainData.terrain[iNum].shw, NULL );
}

// Läd das Mapfile:
bool cMap::LoadMap ( string filename )
{
	if( filename.substr( filename.length()-4, filename.length() ).compare( ".WRL") == 0 || filename.substr( filename.length()-4, filename.length() ).compare( ".wrl") == 0 )
	{
		SDL_RWops *fpMapFile;
		short sWidth, sHeight;
		short sGraphCount;		// Number of terrain graphics for this map
		sColor Palette[256];	// Palette with all Colors for the terrain graphics
		int iPalettePos, iGraphicsPos, iInfoPos, iDataPos;	// Positions in map-file
		int iWaterCount = 0, iOffToWater = 0;	// Number of water graphics
		unsigned char cByte;	// one Byte

		// Open File
		MapName = filename;
		filename = SettingsData.sMapsPath + PATH_DELIMITER + filename;
		fpMapFile = SDL_RWFromFile ( filename.c_str(),"rb" );
		if ( !fpMapFile )
		{
			cLog::write("Cannot load map file!", cLog::eLOG_TYPE_WARNING);
			return false;
		}

		// Delete old Map
		DeleteMap();

		// Read informations and get positions from the map-file
		SDL_RWseek ( fpMapFile, 5, SEEK_SET );					// Ignore WRL-Typ
		SDL_RWread ( fpMapFile, &sWidth, 2, 1 );
		SDL_RWread ( fpMapFile, &sHeight, 2, 1 );
		SDL_RWseek ( fpMapFile, sWidth * sHeight, SEEK_CUR );	// Ignore Mini-Map
		iDataPos = SDL_RWtell( fpMapFile );						// Map-Data
		SDL_RWseek ( fpMapFile, sWidth * sHeight * 2, SEEK_CUR );
		SDL_RWread ( fpMapFile, &sGraphCount, 2, 1 );			// Read PicCount	
		iGraphicsPos = SDL_RWtell( fpMapFile );					// Terrain Graphics
		iPalettePos = iGraphicsPos + sGraphCount * 64*64;		// Palette
		iInfoPos = iPalettePos + 256*3;							// Special informations

		if ( sWidth != sHeight )
		{
			cLog::write("Map must be quadratic!", cLog::eLOG_TYPE_WARNING);
			return false;
		}
		size = sWidth;

		// Generate new Map
		NewMap ( size );

		// Load Color Palette
		SDL_RWseek ( fpMapFile, iPalettePos , SEEK_SET );
		SDL_RWread ( fpMapFile, &Palette, 3, 256 );

		// alloc memory for terrains
		TerrainData.terrain_anz = sGraphCount;
		TerrainData.terrain = ( sTerrain * ) malloc ( sizeof( sTerrain ) * sGraphCount );

		// First load water graphics
		SDL_RWseek ( fpMapFile, iInfoPos , SEEK_SET );
		SDL_RWread ( fpMapFile, &cByte, 1, 1 );
		// Get number of water graphics
		while ( cByte != 1 )
		{
			iOffToWater++;
			SDL_RWread ( fpMapFile, &cByte, 1, 1 );
		}
		while ( cByte == 1 )
		{
			iWaterCount++;
			SDL_RWread ( fpMapFile, &cByte, 1, 1 );
		}
		// Create new surface for the graphics
		SDL_Surface *surface;
		surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 64*iWaterCount, 64,32,0,0,0,0);
		// Load single graphics and put them together
		for ( int j = iOffToWater; j < iOffToWater+iWaterCount; j++ )
		{
			SDL_Rect rect = { 64*(j-iOffToWater), 0, 64, 64 };
			SDL_Surface *loadsurface;
			loadsurface = LoadTerrGraph ( fpMapFile, iGraphicsPos, Palette, j, iOffToWater, iWaterCount, TerrainData.terrain[0].overlay );
			SDL_BlitSurface( loadsurface, NULL, surface, &rect );
			SDL_FreeSurface ( loadsurface );
		}
		CopySrfToTerData ( surface, 0, 64*iWaterCount );
		SDL_FreeSurface ( surface );

		// Set special informations dor water
		TerrainData.terrain[0].frames = TerrainData.terrain[0].sf_org->w/64;
		TerrainData.terrain[0].water = true;
		TerrainData.terrain[0].blocked = false;
		TerrainData.terrain[0].coast = false;
		TerrainInUse->Add( TerrainData.terrain+0 );
		DefaultWater = 0;

		// Load necessary Terrain Graphics
		for ( int iNum = 0; iNum < sGraphCount-iWaterCount; iNum++ )
		{
			// Load graphic
			if( iNum < iOffToWater)
			{
				CopySrfToTerData ( LoadTerrGraph ( fpMapFile, iGraphicsPos, Palette, iNum, iOffToWater, iWaterCount, TerrainData.terrain[iNum+1].overlay), iNum+1,64 );
			}
			else
			{
				CopySrfToTerData ( LoadTerrGraph ( fpMapFile, iGraphicsPos, Palette, iNum + iWaterCount, iOffToWater, iWaterCount, TerrainData.terrain[iNum+1].overlay), iNum+1,64 );
			}
			TerrainData.terrain[iNum+1].frames = 1;

			// This Terrain will be used
			TerrainInUse->Add( TerrainData.terrain+iNum+1 );

			// Set ColorKeys if necessary
			if ( TerrainData.terrain[iNum+1].overlay )
			{
				int t=0xFFCD00CD;
				SDL_SetColorKey ( TerrainData.terrain[iNum+1].sf_org,SDL_SRCCOLORKEY,0xFF00FF );
				SDL_SetColorKey ( TerrainData.terrain[iNum+1].shw_org,SDL_SRCCOLORKEY,t );
				SDL_SetColorKey ( TerrainData.terrain[iNum+1].sf,SDL_SRCCOLORKEY,0xFF00FF );
				SDL_SetColorKey ( TerrainData.terrain[iNum+1].shw,SDL_SRCCOLORKEY,t );
			}
		}
		// Read special informations about terrain graphics
		SDL_RWseek ( fpMapFile, iInfoPos+iWaterCount , SEEK_SET );
		for ( int k = 1; k < sGraphCount-iWaterCount; k++ )
		{
			SDL_RWread ( fpMapFile, &cByte, 1, 1 );

			if ( cByte == 2 ) TerrainData.terrain[k].coast = true;
			else TerrainData.terrain[k].coast = false;
			if ( cByte == 3 ) TerrainData.terrain[k].blocked = true;
			else TerrainData.terrain[k].blocked = false;
			TerrainData.terrain[k].water = false;
		}

		// Load map data
		SDL_RWseek ( fpMapFile, iDataPos , SEEK_SET );
		for ( int iY = size-1; iY >= 0; iY-- )
		{
			for ( int iX = 0; iX < size; iX++ )
			{
				SDL_RWread ( fpMapFile, &Kacheln[iY*size+iX], 2, 1 );
				if ( Kacheln[iY*size+iX] >= iWaterCount + iOffToWater )
				{
					Kacheln[iY*size+iX]-=iWaterCount-1;
				}
				else
				{
					if ( Kacheln[iY*size+iX] >= iOffToWater )
					{
						Kacheln[iY*size+iX] = 0;
					}
					else
					{
						Kacheln[iY*size+iX]++;
					}
				}
			}
		}

		SDL_RWclose( fpMapFile );
		return true;
	}
	else
	{
		FILE *fp;
		char str[100];
		cList<sTuple*> *index;
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
		index=new cList<sTuple*>;
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
					index->Add( t );
					TerrainInUse->Add( TerrainData.terrain+i );
					break;
				}
			}
			if ( i==TerrainData.terrain_anz )
			{
				ErrorStr="terrain not found";
				cLog::write((string)"terrain not found: " + str,LOG_TYPE_WARNING);
				while ( index->iCount )
				{
					delete index->Items[index->iCount - 1];
					index->Delete( index->iCount - 1 );
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
			for ( k=0;k<index->iCount;k++ )
			{
				t=index->Items[k];
				if ( t->from==nr ) break;
			}
			Kacheln[i]=t->to;
		}

		while ( index->iCount )
		{
			delete index->Items[0];
			index->Delete ( 0 );
		}
		delete index;
		fclose ( fp );
		return true;
	}
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
	while ( TerrainInUse->iCount )
	{
		TerrainInUse->Delete( 0 );
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
			PosMap[i]=random ( size*size - 1,0 );
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
