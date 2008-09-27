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

#include "map.h"

cVehicleIterator::cVehicleIterator(cList<cVehicle*>* list)
{
	index = 0;
	vehicleList = list;

	if ( list->Size() <= 0 )
	{
		end = true;
		rend = true;
	}
	else
	{
		end = false;
		rend = false;
	}
}

unsigned int cVehicleIterator::size()
{
	return vehicleList->Size();
}

cVehicle* cVehicleIterator::operator->() const
{
	if ( !end && !rend )
		return (*vehicleList)[index];
	else
		return NULL;
}

cVehicle& cVehicleIterator::operator*() const
{
	cVehicle* vehicle = NULL;
	if (!end && !rend ) vehicle = (*vehicleList)[index];

	return *vehicle;
}

cVehicleIterator cVehicleIterator::operator++(int)
{
	cVehicleIterator vehicles = *this;
	if (end) return vehicles;
	
	if (rend)
	{
		rend = false;
		index = 0;
	}
	else
	{
		index++;
	}
	if ( index >= vehicleList->Size() ) end = true;

	return vehicles;
}

cVehicleIterator cVehicleIterator::operator--(int)
{
	cVehicleIterator vehicles = *this;
	if (rend) return vehicles;
	
	if ( end )
	{
		index = vehicleList->Size() - 1;
		end = false;
	}
	else
	{
		index--;
	}
	if ( index < 0 ) rend = true;

	return vehicles;
}

bool cVehicleIterator::operator ==(cVehicle* v) const
{
	if ( v == NULL && (end || rend) ) return true;
	if ( (*vehicleList)[index] == v ) return true;

	return false;
}

cVehicleIterator::operator cVehicle *() const
{
	if ( end || rend ) return NULL;
	return (*vehicleList)[index];
}

cBuildingIterator::cBuildingIterator(cList<cBuilding*>* list)
{
	buildingList = list;
	index = 0;

	if ( buildingList->Size() <= 0 )
	{
		end = true;
		rend = true;
	}
	else
	{
		end = false;
		rend = false;
	}
}

unsigned int cBuildingIterator::size()
{
	return buildingList->Size();
}

cBuilding* cBuildingIterator::operator->() const
{
	if ( !end && !rend )
		return (*buildingList)[index];
	else
		return NULL;
}

cBuilding& cBuildingIterator::operator*() const
{
	cBuilding* building = NULL;
	if (!end && !rend ) building = (*buildingList)[index];

	return *building;
}

cBuildingIterator cBuildingIterator::operator++(int)
{
	cBuildingIterator buildings = *this;
	if (end) return buildings;
	
	if (rend)
	{
		rend = false;
		index = 0;
	}
	else
	{
		index++;
	}
	if ( index >= buildingList->Size() ) end = true;
	
	return buildings;
}

cBuildingIterator cBuildingIterator::operator--(int)
{
	cBuildingIterator buildings = *this;
	if (rend) return buildings;
	
	if ( end ) 
	{
		end = false;
		index = buildingList->Size() - 1;
	}
	else
	{
		index--;
	}
	if ( index < 0 ) rend = true;

	return buildings;
}

bool cBuildingIterator::operator ==(cBuilding* b) const
{
	if ( b == NULL && (end || rend) ) return true;
	if ( (*buildingList)[index] == b ) return true;

	return false;
}

cBuildingIterator::operator cBuilding*() const
{
	if ( end || rend ) return NULL;
	return (*buildingList)[index];
}

cVehicleIterator cMapField::getVehicles()
{
	cVehicleIterator v(&vehicles);
	return v;
}

cVehicleIterator cMapField::getPlanes()
{
	cVehicleIterator v(&planes);
	return v;
}

cBuildingIterator cMapField::getBuildings()
{
	cBuildingIterator b(&buildings);
	return b;
}


cBuilding* cMapField::getTopBuilding()
{
	cBuildingIterator buildingIterator( &buildings );


	if ( buildingIterator && buildingIterator->data.is_base )
	{
		return buildingIterator;
	}
	else
	{
		return NULL;
	}
}

cBuilding* cMapField::getBaseBuilding()
{
	cBuildingIterator building (&buildings);
	
	while ( !building.end )
	{
		if ( building->data.is_base ) return building;
		building++;
	}
	
	return NULL; 
}

// Funktionen der Map-Klasse /////////////////////////////////////////////////
cMap::cMap ( void )
{
	Kacheln=NULL;
	NewMap ( 32, 32 );
	MapName="";
}

cMap::~cMap ( void )
{
	DeleteMap();
}

cMapField& cMap::operator[]( unsigned int offset ) const
{
	return fields[offset];
}

// Gibt zurück, ob eine Kachel als Wasser gilt, oder nicht:
bool cMap::IsWater ( int off,bool not_coast,bool is_ship )
{
	if ( !terrain[Kacheln[off]].water&&!terrain[Kacheln[off]].coast ) return false;


//  if(game->ActivePlayer->ScanMap[off]&&GO[off].base&&(GO[off].base->data.is_platform||GO[off].base->data.is_bridge)){
	/*if(game->ActivePlayer->ScanMap[off]&&GO[off].base&&(GO[off].base->data.is_platform||(is_ship?0:GO[off].base->data.is_bridge))){
	  return false;
	}*/
	if ( not_coast )
	{
		return terrain[Kacheln[off]].water;
	}
	else
	{
		return terrain[Kacheln[off]].water||terrain[Kacheln[off]].coast;
	}
}

// Struktur zum laden:
struct sTuple
{
	int from;
	int to;
};

SDL_Surface *cMap::LoadTerrGraph ( SDL_RWops *fpMapFile, int iGraphicsPos, sColor Palette[256], int iNum, bool bWater, bool &overlay  )
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
		if ( i == 96 && !bWater )
		{
			surface->format->palette->colors[i].r = 255;
			surface->format->palette->colors[i].g = 0;
			surface->format->palette->colors[i].b = 255;
		}
	}

	// Go to position of filedata
	SDL_RWseek ( fpMapFile, iGraphicsPos + 64*64*( iNum ), SEEK_SET );

	// Read pixel data and write to surface
	for( int iY = 0; iY < 64; iY++ )
	{
		for( int iX = 0; iX < 64; iX++ )
		{
			unsigned char cColorOffset;
			SDL_RWread ( fpMapFile, &cColorOffset, 1, 1 );
			Uint8 *pixel = (Uint8*) surface->pixels  + (iY * 64 + iX);
			// If is not a water graphic set all pixels in water color to index 96 with will be a color key
			if( ((cColorOffset > 95 && cColorOffset <= 116) || (cColorOffset > 122 && cColorOffset <= 127)) && !bWater )
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
	terrain[iNum].sf_org = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_BlitSurface( surface, NULL, terrain[iNum].sf_org, NULL );

	terrain[iNum].sf = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect( terrain[iNum].sf, NULL, 0xFF00FF );
	SDL_BlitSurface( terrain[iNum].sf_org, NULL, terrain[iNum].sf, NULL );

	terrain[iNum].shw_org = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect( terrain[iNum].shw_org, NULL, 0xFF00FF );
	SDL_BlitSurface( terrain[iNum].sf_org, NULL, terrain[iNum].shw_org, NULL );

	SDL_BlitSurface ( GraphicsData.gfx_shadow, NULL, terrain[iNum].shw_org, NULL );

	terrain[iNum].shw = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, iSizeX, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect( terrain[iNum].shw, NULL, 0xFF00FF );
	SDL_BlitSurface( terrain[iNum].shw_org, NULL, terrain[iNum].shw, NULL );
}

// Läd das Mapfile:
bool cMap::LoadMap ( string filename )
{
	SDL_RWops *fpMapFile;
	short sWidth, sHeight;
	short sGraphCount;		// Number of terrain graphics for this map
	sColor Palette[256];	// Palette with all Colors for the terrain graphics
	int iPalettePos, iGraphicsPos, iInfoPos, iDataPos;	// Positions in map-file
	unsigned char cByte;	// one Byte
	char szFileTyp[4];

	// Open File
	MapName = filename;
	filename = SettingsData.sMapsPath + PATH_DELIMITER + filename;
	fpMapFile = SDL_RWFromFile ( filename.c_str(),"rb" );
	if ( !fpMapFile )
	{
		cLog::write("Cannot load map file: \"" + MapName + "\"", cLog::eLOG_TYPE_WARNING);
		return false;
	}

	// check for typ
	SDL_RWread ( fpMapFile, &szFileTyp, 1, 3 );
	szFileTyp[3] = '\0';
	// WRL - interplays original mapformat
	// WRX - mapformat from russian mapeditor
	// DMO - for some reason some original maps have this filetype
	if( strcmp( szFileTyp, "WRL" ) != 0 && strcmp( szFileTyp, "WRX" ) != 0 && strcmp( szFileTyp, "DMO" ) != 0  )
	{
		cLog::write("Wrong file format: \"" + MapName + "\"", cLog::eLOG_TYPE_WARNING);
		SDL_RWclose( fpMapFile );
		return false;
	}
	SDL_RWseek ( fpMapFile, 2, SEEK_CUR );

	// Read informations and get positions from the map-file
	sWidth = SDL_ReadLE16( fpMapFile );
	sHeight = SDL_ReadLE16( fpMapFile );
	SDL_RWseek ( fpMapFile, sWidth * sHeight, SEEK_CUR );	// Ignore Mini-Map
	iDataPos = SDL_RWtell( fpMapFile );						// Map-Data
	SDL_RWseek ( fpMapFile, sWidth * sHeight * 2, SEEK_CUR );
	sGraphCount = SDL_ReadLE16( fpMapFile );				// Read PicCount
	iGraphicsPos = SDL_RWtell( fpMapFile );					// Terrain Graphics
	iPalettePos = iGraphicsPos + sGraphCount * 64*64;		// Palette
	iInfoPos = iPalettePos + 256*3;							// Special informations

	if ( sWidth != sHeight )
	{
		cLog::write("Map must be quadratic!: \"" + MapName + "\"", cLog::eLOG_TYPE_WARNING);
		SDL_RWclose( fpMapFile );
		return false;
	}
	size = sWidth;

	// Generate new Map
	NewMap ( size, sGraphCount );

	// Load Color Palette
	SDL_RWseek ( fpMapFile, iPalettePos , SEEK_SET );
	SDL_RWread ( fpMapFile, &Palette, 1, 768 );

	DefaultWater = -1;
	// Load necessary Terrain Graphics
	for ( int iNum = 0; iNum < sGraphCount; iNum++ )
	{
		SDL_Surface *surface;	// Temporary surface for fresh loaded graphic

		// Check for typ
		SDL_RWseek ( fpMapFile, iInfoPos+iNum, SEEK_SET );
		SDL_RWread ( fpMapFile, &cByte, 1, 1 );

		terrain[iNum].water = false;
		if ( cByte == 1 ) terrain[iNum].water = true;
		else terrain[iNum].water = false;
		if ( cByte == 2 ) terrain[iNum].coast = true;
		else terrain[iNum].coast = false;
		if ( cByte == 3 ) terrain[iNum].blocked = true;
		else terrain[iNum].blocked = false;

		if ( terrain[iNum].water )
		{
			SDL_Surface *fullsurface = SDL_CreateRGBSurface( SDL_HWSURFACE, 64* 7, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
			surface = LoadTerrGraph ( fpMapFile, iGraphicsPos, Palette, iNum, true, terrain[iNum].overlay);
			SDL_Rect dest = { 0, 0, 64, 64 };
			SDL_BlitSurface( surface, NULL, fullsurface, &dest );
			dest.x += 64;
			for ( int i = 0; i < 6; i++ )
			{
				for (int iInd = 0; iInd < 64*64; iInd++ )
				{
					Uint8 *pixel = (Uint8*) surface->pixels  + iInd;
					if ( *pixel > 96 && *pixel <= 102 ) *pixel -= 1;
					else if ( *pixel == 96 ) *pixel += 6;

					else if ( *pixel > 103 && *pixel <= 109 ) *pixel -= 1;
					else if ( *pixel == 103 ) *pixel += 6;

					else if ( *pixel > 110 && *pixel <= 116 ) *pixel -= 1;
					else if ( *pixel == 110 ) *pixel += 6;

					else if ( *pixel > 117 && *pixel <= 122 ) *pixel -= 1;
					else if ( *pixel == 117 ) *pixel += 5;

					else if ( *pixel > 123 && *pixel <= 127 ) *pixel -= 1;
					else if ( *pixel == 123 ) *pixel += 4;
				}
				SDL_BlitSurface( surface, NULL, fullsurface, &dest );
				dest.x += 64;
			}
			CopySrfToTerData ( fullsurface, iNum, 64*7 );
			SDL_FreeSurface ( fullsurface );
		}
		else
		{
			surface = LoadTerrGraph ( fpMapFile, iGraphicsPos, Palette, iNum, terrain[iNum].water, terrain[iNum].overlay);
			CopySrfToTerData ( surface, iNum, 64 );
		}
		terrain[iNum].frames = terrain[iNum].sf_org->w/64;

		// First watergraphic will be the default one for coasts
		if( terrain[iNum].water && DefaultWater == -1 ) DefaultWater = iNum;

		// This Terrain will be used
		TerrainInUse.Add( terrain+( iNum ) );

		// Set ColorKeys if necessary
		if ( terrain[iNum].overlay )
		{
			int t=0xCD00CD;
			SDL_SetColorKey ( terrain[iNum].sf_org,SDL_SRCCOLORKEY,0xFF00FF );
			SDL_SetColorKey ( terrain[iNum].shw_org,SDL_SRCCOLORKEY,t );
			SDL_SetColorKey ( terrain[iNum].sf,SDL_SRCCOLORKEY,0xFF00FF );
			SDL_SetColorKey ( terrain[iNum].shw,SDL_SRCCOLORKEY,t );
		}
		SDL_FreeSurface ( surface );
	}

	// Load map data
	SDL_RWseek ( fpMapFile, iDataPos , SEEK_SET );
	for ( int iY = 0; iY < size; iY++ )
	{
		for ( int iX = 0; iX < size; iX++ )
		{
			Kacheln[iY*size+iX] = SDL_ReadLE16( fpMapFile );
		}
	}
	SDL_RWclose( fpMapFile );
	return true;
}

// Erstellt eine neue Map:
void cMap::NewMap ( int size, int iTerrainGrphCount )
{

	if ( size<16 ) size=16;
	if ( size>256 ) size=256;

	DeleteMap();
	this->size=size;
	Kacheln= ( int* ) malloc ( sizeof ( int ) *size*size );
	memset ( Kacheln,0,sizeof ( int ) *size*size );

	DefaultWater=0;

	fields = new cMapField[size*size];
	GO= ( sGameObjects* ) malloc ( sizeof ( sGameObjects ) *size*size );
	memset ( GO,0,sizeof ( sGameObjects ) *size*size );
	Resources= ( sResources* ) malloc ( sizeof ( sResources ) *size*size );

	// alloc memory for terrains
	terrain = ( sTerrain * ) malloc ( sizeof( sTerrain ) * iTerrainGrphCount );
}

// Löscht die aktuelle Map:
void cMap::DeleteMap ( void )
{
	if ( !Kacheln ) return;
	free ( Kacheln );
	delete[] fields;
	free ( GO );
	free ( Resources );
	Kacheln=NULL;
	for (int i = 0; i < TerrainInUse.Size(); i++)
	{
		TerrainInUse.Delete(TerrainInUse.Size());
		SDL_FreeSurface ( terrain[i].sf_org );
		SDL_FreeSurface ( terrain[i].sf );
		SDL_FreeSurface ( terrain[i].shw_org );
		SDL_FreeSurface ( terrain[i].shw );
	}
	free ( terrain );
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

	memset ( Resources,0,sizeof ( sResources ) *size*size );
	if ( Metal>3 ) Metal=3;
	if ( Oil>3 ) Oil=3;
	if ( Gold>3 ) Gold=3;
	if ( Dichte>3 ) Dichte=3;
	GaussMap= ( int* ) malloc ( size*size*sizeof ( int ) );
	memset ( GaussMap,0,sizeof ( int ) *size*size );

	// Dafür sorgen, dass mehr Rohstoffe auf dem Land sind:
	for ( i=0;i<size*size;i++ )
	{
		if ( terrain[Kacheln[i]].water ) GaussMap[i]=5;
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
			PosMap[i] = random(size * size - 1);
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

		if ( terrain[Kacheln[pos]].blocked )
		{
			amount++;
			continue;
		}

		// Ggf ein Nest erstellen:
		if (random(15) == 0)
			nest = random(3) + 1;

		do
		{
			if ( next==0 )
			{
				Resources[pos].typ=RES_METAL;
				Resources[pos].value += 4 * Metal + 4 + random(5) - 2;
			}
			else if ( next==1 )
			{
				Resources[pos].typ=RES_OIL;
				Resources[pos].value += 4 * Oil + 4 + random(5) - 2;
			}
			else
			{
				Resources[pos].typ=RES_GOLD;
				Resources[pos].value += 4 * Gold + 4 + random(5) - 2;
			}
			if ( Resources[pos].value>16 )
			{
				Resources[pos].value=16;
			}
			next++;
			if ( next>2 ) next=0;

			// Ressurcen dumherum platzieren:
			x=pos%size;
			y=pos/size;
			if ( x>0&&y>0 )
			{
				x--;y--;
				Resources[x + y * size].typ    = random(3) + 1;
				Resources[x + y * size].value += random(4);
				if ( Resources[x+y*size].value>16 )
				{
					Resources[x+y*size].value=16;
				}
				x++;y++;
			}
			if ( x<size-1&&y>0 )
			{
				x++;y--;
				Resources[x + y * size].typ    = random(3) + 1;
				Resources[x + y * size].value += random(4);
				if ( Resources[x+y*size].value>16 )
				{
					Resources[x+y*size].value=16;
				}
				x--;y++;
			}
			if ( x>0&&y<size-1 )
			{
				x--;y++;
				Resources[x + y * size].typ    = random(3) + 1;
				Resources[x + y * size].value += random(4);
				if ( Resources[x+y*size].value>16 )
				{
					Resources[x+y*size].value=16;
				}
				x++;y--;
			}
			if ( x<size-1&&y<size-1 )
			{
				x++;y++;
				Resources[x + y * size].typ   = random(3) + 1;
				Resources[x + y * size].value = random(4) + 1;
				x--;y--;
			}

			if ( nest )
			{
				int old=pos;
				do
				{
					pos=old;
					pos += random(6) - 3;
					pos += (random(6) - 3) *size;
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



void cMap::addBuilding( cBuilding* building, unsigned int x, unsigned int y )
{
	addBuilding( building, x + y * size );
}


void cMap::addBuilding( cBuilding* building, unsigned int offset )
{
	if ( building->data.is_base && building->data.is_big ) return; //big base building are not implemented

	//TODO: sort order of buildings in the list

	if ( building->data.is_big )
	{
		fields[offset           ].buildings.Insert(0, building);
		fields[offset + 1       ].buildings.Insert(0, building );
		fields[offset + size    ].buildings.Insert(0, building );
		fields[offset + size + 1].buildings.Insert(0, building );

		GO[offset].top = building;
		GO[offset + 1].top = building;
		GO[offset + size].top = building;
		GO[offset + size + 1].top = building;

	}
	else
	{
		int i = 0;
		if ( building->data.is_base && !fields[offset].buildings[0]->data.is_base )
		{
			i++;
			GO[offset].subbase = GO[offset].base;
			GO[offset].base = building;
		}
		else
		{
			GO[offset].top = building;
		}

		fields[offset].buildings.Insert(i, building);
	}
}


void cMap::addVehicle(cVehicle *vehicle, unsigned int x, unsigned int y )
{
	addVehicle( vehicle, x + y * size );
}

void cMap::addVehicle(cVehicle *vehicle, unsigned int offset )
{
	if ( vehicle->data.can_drive == DRIVE_AIR )
	{
		fields[offset].planes.Insert(0, vehicle );
	}
	else
	{
		fields[offset].vehicles.Insert(0, vehicle );
	}

	//backward compatibility 
	if ( vehicle->data.can_drive == DRIVE_AIR )
	{
		GO[offset].plane = vehicle;
	}
	else
	{
		GO[offset].vehicle = vehicle;
	}
}

void cMap::deleteBuilding( cBuilding* building )
{
	int offset = building->PosX + building->PosY * size;

	cList<cBuilding*>& buildings = fields[offset].buildings;
	for ( int i = 0; i < buildings.Size(); i++ )
	{
		if ( buildings[i] == building ) buildings.Delete(i);
	}

	if ( building->data.is_big )
	{
		//big building must be a top building
		//so only check the first building
		offset++;
		buildings = fields[offset].buildings;
		if ( buildings[0] == building ) buildings.Delete(0);
		
		offset += size;
		buildings = fields[offset].buildings;
		if ( buildings[0] == building ) buildings.Delete(0);
		
		offset--;
		buildings = fields[offset].buildings;
		if ( buildings[0] == building ) buildings.Delete(0);
		
	}

	//backward compatibility
	offset = building->PosX + building->PosY * size;

	if ( GO[offset].subbase == building ) GO[offset].subbase = NULL;
	if ( GO[offset].base    == building ) GO[offset].base    = NULL;
	if ( GO[offset].top     == building ) GO[offset].top     = NULL;
	

	if ( building->data.is_big )
	{
		offset++;
		if ( GO[offset].top == building ) GO[offset].top = NULL;

		offset += size;
		if ( GO[offset].top == building ) GO[offset].top = NULL;	
		
		offset--;
		if ( GO[offset].top == building ) GO[offset].top = NULL;
	}
}

void cMap::deleteVehicle( cVehicle* vehicle )
{
	int offset = vehicle->PosX + vehicle->PosY *size;

	if ( vehicle->data.can_drive == DRIVE_AIR )
	{
		cList<cVehicle*>& planes = fields[offset].planes;
		for ( int i = 0; i < planes.Size(); i++ )
		{
			if ( planes[i] == vehicle ) planes.Delete(i);
		}
	}
	else
	{
		//only one vehicle per field allowed
		fields[offset].vehicles.Delete(0);

		//check, whether the vehicle is centered on 4 map fields
		if ( vehicle->data.is_big )
		{
			offset++;
			fields[offset].vehicles.Delete(0);
			offset += size;
			fields[offset].vehicles.Delete(0);
			offset--;
			fields[offset].vehicles.Delete(0);
		}
	}

	//backward compatibility
	if ( vehicle->data.can_drive == DRIVE_AIR ) 
	{
		if (GO[offset].plane == vehicle ) GO[offset].plane = NULL;
	}
	else
	{
		if ( GO[offset].vehicle == vehicle ) GO[offset].vehicle = NULL;
		if ( vehicle->PosX + 1 < size && vehicle->PosY + 1 < size )
		{
			offset++;
			if ( GO[offset].vehicle == vehicle ) GO[offset].vehicle = NULL;
			offset += size;
			if ( GO[offset].vehicle == vehicle ) GO[offset].vehicle = NULL;
			offset--;
			if ( GO[offset].vehicle == vehicle ) GO[offset].vehicle = NULL;
		}
	}
}

void cMap::moveVehicle( cVehicle* vehicle, unsigned int x, unsigned int y )
{
	moveVehicle( vehicle, x + y * size );
}

void cMap::moveVehicle( cVehicle* vehicle, unsigned int newOffset )
{
	int oldOffset = vehicle->PosX + vehicle->PosY * size;
	
	vehicle->PosX = newOffset % size;
	vehicle->PosY = newOffset / size;

	if ( vehicle->data.can_drive == DRIVE_AIR )
	{
		cList<cVehicle*>& planes = fields[oldOffset].planes;
		for ( int i = 0; i < planes.Size(); i++ )
		{
			if ( planes[i] == vehicle ) planes.Delete(i);
		}
		fields[newOffset].planes.Insert(0, vehicle );
	}
	else
	{
		//there will be only one vehicle per field
		fields[oldOffset].vehicles.Delete(0);
		
		//check, whether the vehicle is centered on 4 map fields
		if ( vehicle->data.is_big )
		{
			oldOffset++;
			fields[oldOffset].vehicles.Delete(0);
			oldOffset += size;
			fields[oldOffset].vehicles.Delete(0);
			oldOffset--;
			fields[oldOffset].vehicles.Delete(0);

			oldOffset -= size; //temp
			//vehicle->data.is_big = false;
		}

		fields[newOffset].vehicles.Insert(0, vehicle );
	}

	//backward compatibility
	if ( vehicle->data.can_drive == DRIVE_AIR )
	{
		GO[oldOffset].plane = NULL;
		GO[newOffset].plane = vehicle;
	}
	else
	{
		GO[oldOffset].vehicle = NULL;
		
		if ( vehicle->IsBuilding || vehicle->IsClearing || vehicle->data.is_big )
		{
			oldOffset++;
			if ( GO[oldOffset].vehicle == vehicle ) GO[oldOffset].vehicle = NULL;
			oldOffset += size;
			if ( GO[oldOffset].vehicle == vehicle ) GO[oldOffset].vehicle = NULL;
			oldOffset--;
			if ( GO[oldOffset].vehicle == vehicle ) GO[oldOffset].vehicle = NULL;
		}

		GO[newOffset].vehicle = vehicle;
	}
	vehicle->data.is_big = false;
}

void cMap::moveVehicleBig( cVehicle* vehicle, unsigned int x, unsigned int y)
{
	moveVehicleBig( vehicle, x + y * size );
}

void cMap::moveVehicleBig( cVehicle* vehicle, unsigned int offset )
{
	int oldOffset = vehicle->PosX + vehicle->PosY * size;
	fields[oldOffset].vehicles.Delete(0);
	
	vehicle->PosX = offset % size;
	vehicle->PosY = offset / size;

	offset++;
	fields[offset].vehicles.Insert(0, vehicle );
	offset += size;
	fields[offset].vehicles.Insert(0, vehicle );
	offset--;
	fields[offset].vehicles.Insert(0, vehicle );

	vehicle->data.is_big = true;

	//backward compatibility
	GO[oldOffset].vehicle = NULL;

	offset -=size;
	GO[offset].vehicle = vehicle;
	offset++;
	GO[offset].vehicle = vehicle;
	offset += size;
	GO[offset].vehicle = vehicle;
	offset--;
	GO[offset].vehicle = vehicle;

}
