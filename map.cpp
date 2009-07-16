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
#include "player.h"

sTerrain::sTerrain()
	:sf(NULL),
	sf_org(NULL),
	shw(NULL),
	shw_org(NULL),
	water(false),
	coast(false),
	blocked(false)
{};

sTerrain::~sTerrain()
{
	if ( sf )		SDL_FreeSurface ( sf );
	if ( sf_org )	SDL_FreeSurface ( sf_org );
	if ( shw )		SDL_FreeSurface ( shw );
	if ( shw_org )	SDL_FreeSurface ( shw_org );
}

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

unsigned int cVehicleIterator::size() const
{
	return (unsigned int)vehicleList->Size();
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
	if ( index >= (int)vehicleList->Size() ) end = true;

	return vehicles;
}

cVehicleIterator cVehicleIterator::operator--(int)
{
	cVehicleIterator vehicles = *this;
	if (rend) return vehicles;
	
	if ( end )
	{
		index = (int)vehicleList->Size() - 1;
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

unsigned int cBuildingIterator::size() const
{
	return (unsigned int)buildingList->Size();
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
	if ( index >= (int)buildingList->Size() ) end = true;
	
	return buildings;
}

cBuildingIterator cBuildingIterator::operator--(int)
{
	cBuildingIterator buildings = *this;
	if (rend) return buildings;
	
	if ( end ) 
	{
		end = false;
		index = (int)buildingList->Size() - 1;
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

cMapField::cMapField()
{};

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


	if ( buildingIterator && ( buildingIterator->data.surfacePosition == sUnitData::SURFACE_POS_GROUND || buildingIterator->data.surfacePosition == sUnitData::SURFACE_POS_GROUND ) && buildingIterator->owner )
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
		if ( building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND && building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND ) return building;
		building++;
	}
	
	return NULL; 
}

cBuilding* cMapField::getRubble()
{
	cBuildingIterator building(&buildings);
	while ( !building.end )
	{
		if ( !building->owner ) return building;
		building++;
	}

	return NULL;
}

cBuilding* cMapField::getMine()
{
	cBuildingIterator building(&buildings);
	while ( building && !building->data.explodesOnContact ) building++;

	return building;
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
	if ( !terrain[Kacheln[off]].water && !terrain[Kacheln[off]].coast ) return false;

	if ( not_coast ) return terrain[Kacheln[off]].water;
	else return terrain[Kacheln[off]].water||terrain[Kacheln[off]].coast;
}

// Struktur zum laden:
struct sTuple
{
	int from;
	int to;
};

SDL_Surface *cMap::LoadTerrGraph ( SDL_RWops *fpMapFile, int iGraphicsPos, SDL_Color* Palette, int iNum )
{
	// Create new surface and copy palette
	SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64,8,0,0,0,0);
	surface->pitch = surface->w;

	SDL_SetColors( surface, Palette, 0, 256 );

	// Go to position of filedata
	SDL_RWseek ( fpMapFile, iGraphicsPos + 64*64*( iNum ), SEEK_SET );

	// Read pixel data and write to surface
	for( int iY = 0; iY < 64; iY++ )
	{
		for( int iX = 0; iX < 64; iX++ )
		{
			unsigned char cColorOffset;
			if ( SDL_RWread ( fpMapFile, &cColorOffset, 1, 1 ) == -1 )
			{
				SDL_FreeSurface( surface );
				return NULL;
			}
			Uint8 *pixel = (Uint8*) surface->pixels  + (iY * 64 + iX);
			*pixel = cColorOffset;
		}
	}
	return surface;
}

void cMap::CopySrfToTerData ( SDL_Surface *surface, int iNum )
{
	//before the surfaces are copied, the colortable of both surfaces has to be equal
	//This is needed to make sure, that the pixeldata is copied 1:1

	//copy the normal terrains
	terrain[iNum].sf_org = SDL_CreateRGBSurface( SDL_HWSURFACE , 64, 64, 8, 0, 0, 0, 0 );
	SDL_SetColors( terrain[iNum].sf_org, surface->format->palette->colors,0, 256);
	SDL_BlitSurface( surface, NULL, terrain[iNum].sf_org, NULL );

	terrain[iNum].sf = SDL_CreateRGBSurface( SDL_HWSURFACE , 64, 64, 8, 0, 0, 0, 0 );
	SDL_SetColors( terrain[iNum].sf, surface->format->palette->colors,0, 256);
	SDL_BlitSurface( surface, NULL, terrain[iNum].sf, NULL );

	//copy the terrains with fog
	terrain[iNum].shw_org = SDL_CreateRGBSurface( SDL_HWSURFACE , 64, 64, 8, 0, 0, 0, 0 );
	SDL_SetColors( terrain[iNum].shw_org, surface->format->palette->colors,0, 256);
	SDL_BlitSurface( surface, NULL, terrain[iNum].shw_org, NULL );

	terrain[iNum].shw = SDL_CreateRGBSurface( SDL_HWSURFACE , 64, 64, 8, 0, 0, 0, 0 );
	SDL_SetColors( terrain[iNum].shw, surface->format->palette->colors,0, 256);
	SDL_BlitSurface( surface, NULL, terrain[iNum].shw, NULL );

	//now set the palette for the fog terrains
	SDL_SetColors( terrain[iNum].shw_org, palette_shw,0, 256);
	SDL_SetColors( terrain[iNum].shw, palette_shw,0, 256);
}

// Läd das Mapfile:
bool cMap::LoadMap ( string filename )
{
	SDL_RWops *fpMapFile;
	short sWidth, sHeight;
	int iPalettePos, iGraphicsPos, iInfoPos, iDataPos;	// Positions in map-file
	unsigned char cByte;	// one Byte
	char szFileTyp[4];

	// Open File
	MapName = filename;
	Log.write("Loading map \"" + filename + "\"", cLog::eLOG_TYPE_DEBUG );
	filename = SettingsData.sMapsPath + PATH_DELIMITER + filename;
	fpMapFile = SDL_RWFromFile ( filename.c_str(),"rb" );
	if ( !fpMapFile )
	{
		Log.write("Cannot load map file: \"" + MapName + "\"", cLog::eLOG_TYPE_WARNING);
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
		Log.write("Wrong file format: \"" + MapName + "\"", cLog::eLOG_TYPE_WARNING);
		SDL_RWclose( fpMapFile );
		return false;
	}
	SDL_RWseek ( fpMapFile, 2, SEEK_CUR );

	// Read informations and get positions from the map-file
	sWidth = SDL_ReadLE16( fpMapFile );
	Log.write("SizeX: " + iToStr(sWidth), cLog::eLOG_TYPE_DEBUG );
	sHeight = SDL_ReadLE16( fpMapFile );
	Log.write("SizeY: " + iToStr(sHeight), cLog::eLOG_TYPE_DEBUG );
	SDL_RWseek ( fpMapFile, sWidth * sHeight, SEEK_CUR );	// Ignore Mini-Map
	iDataPos = SDL_RWtell( fpMapFile );						// Map-Data
	SDL_RWseek ( fpMapFile, sWidth * sHeight * 2, SEEK_CUR );
	iNumberOfTerrains = SDL_ReadLE16( fpMapFile );				// Read PicCount
	Log.write("Number of terrains: " + iToStr(iNumberOfTerrains), cLog::eLOG_TYPE_DEBUG );
	iGraphicsPos = SDL_RWtell( fpMapFile );					// Terrain Graphics
	iPalettePos = iGraphicsPos + iNumberOfTerrains * 64*64;		// Palette
	iInfoPos = iPalettePos + 256*3;							// Special informations

	if ( sWidth != sHeight )
	{
		Log.write("Map must be quadratic!: \"" + MapName + "\"", cLog::eLOG_TYPE_WARNING);
		SDL_RWclose( fpMapFile );
		return false;
	}
	size = sWidth;

	// Generate new Map
	NewMap ( size, iNumberOfTerrains );

	// Load Color Palette
	SDL_RWseek ( fpMapFile, iPalettePos , SEEK_SET );
	for ( int i = 0; i < 256; i++ )
	{
		SDL_RWread ( fpMapFile, palette + i, 3, 1 );
	}
	//generate palette for terrains with fog
	for ( int i = 0; i < 256; i++)
	{
		palette_shw[i].r = (unsigned char) (palette[i].r * 0.6);
		palette_shw[i].g = (unsigned char) (palette[i].g * 0.6);
		palette_shw[i].b = (unsigned char) (palette[i].b * 0.6);
	}


	// Load necessary Terrain Graphics
	for ( int iNum = 0; iNum < iNumberOfTerrains; iNum++ )
	{
		SDL_Surface *surface;	// Temporary surface for fresh loaded graphic

		// load terrain type info
		SDL_RWseek ( fpMapFile, iInfoPos+iNum, SEEK_SET );
		SDL_RWread ( fpMapFile, &cByte, 1, 1 );

		switch ( cByte )
		{
		case 0:
			//normal terrain without special property
			break; 
		case 1:
			terrain[iNum].water = true;
			break;
		case 2:
			terrain[iNum].coast = true;
			break;
		case 3:
			terrain[iNum].blocked = true;
			break;
		default:
			Log.write("unknown terrain type found", cLog::eLOG_TYPE_WARNING );
			SDL_RWclose( fpMapFile );
			return false;
		}


		//load terrain graphic
		surface = LoadTerrGraph ( fpMapFile, iGraphicsPos, palette, iNum );
		if ( surface == NULL )
		{
			Log.write("EOF while loading terrain number " + iToStr(iNum), cLog::eLOG_TYPE_WARNING );
			SDL_RWclose( fpMapFile );
			return false;
		}
		CopySrfToTerData ( surface, iNum );
		SDL_FreeSurface ( surface );
	}

	// Load map data
	SDL_RWseek ( fpMapFile, iDataPos , SEEK_SET );
	for ( int iY = 0; iY < size; iY++ )
	{
		for ( int iX = 0; iX < size; iX++ )
		{
			int Kachel = SDL_ReadLE16( fpMapFile );
			if ( Kachel >= iNumberOfTerrains )
			{
				Log.write("a map field referred to a nonexisting terrain", cLog::eLOG_TYPE_WARNING );
				SDL_RWclose( fpMapFile );
				return false;
			}
			Kacheln[iY*size+iX] = Kachel; 
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
	Kacheln = new int[size*size];
	memset ( Kacheln, 0, sizeof ( int ) *size*size );

	fields = new cMapField[size*size];
	Resources = new sResources[size*size];

	// alloc memory for terrains
	terrain = new sTerrain[iTerrainGrphCount];
}

// Löscht die aktuelle Map:
void cMap::DeleteMap ( void )
{
	if ( !Kacheln ) return;
	delete [] Kacheln;
	delete[] fields;
	delete [] Resources;
	Kacheln=NULL;
	delete[] ( terrain );
}

void cMap::generateNextAnimationFrame()
{
	//change palettes to display next frame
	SDL_Color temp = palette[127];
	memmove( palette + 97, palette + 96, 32 * sizeof(SDL_Color) );
	palette[96]  = palette[103];
	palette[103] = palette[110];
	palette[110] = palette[117];
	palette[117] = palette[123];
	palette[123] = temp;

	temp = palette_shw[127];
	memmove( palette_shw + 97, palette_shw + 96, 32 * sizeof(SDL_Color) );
	palette_shw[96]  = palette_shw[103];
	palette_shw[103] = palette_shw[110];
	palette_shw[110] = palette_shw[117];
	palette_shw[117] = palette_shw[123];
	palette_shw[123] = temp;


	//set the new palette for all terrain surfaces
	for ( int i = 0; i < iNumberOfTerrains; i++ )
	{
		SDL_SetColors( terrain[i].sf, palette + 96, 96, 127);
		//SDL_SetColors( TerrainInUse[i]->sf_org, palette + 96, 96, 127);
		SDL_SetColors( terrain[i].shw, palette_shw + 96, 96, 127);
		//SDL_SetColors( TerrainInUse[i]->shw_org, palette_shw + 96, 96, 127);	
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

	memset ( Resources,0,sizeof ( sResources ) *size*size );
	if ( Metal>3 ) Metal=3;
	if ( Oil>3 ) Oil=3;
	if ( Gold>3 ) Gold=3;
	if ( Dichte>3 ) Dichte=3;
	GaussMap = new int[size*size];
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
	delete [] GaussMap;
}


int cMap::getMapLevel( cBuilding* building ) const
{
	const sUnitData& data = building->data;

	if ( data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA ) return 9;		// seamine
	if ( data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA ) return 7;	// bridge
	if ( data.surfacePosition == sUnitData::SURFACE_POS_BASE && data.canBeOverbuild ) return 6;	// platform
	if ( data.surfacePosition == sUnitData::SURFACE_POS_BASE ) return 5;	// road
	if ( !building->owner ) return 4;	// rubble
	if ( data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE) return 3;	// landmine

	return 1;	// other buildings
}

int cMap::getMapLevel( cVehicle* vehicle ) const
{
	if ( vehicle->data.factorSea > 0 && vehicle->data.factorGround == 0 ) return 8;	// ships
	if ( vehicle->data.factorAir > 0 ) return 0;	// planes

	return 2;	// other vehicles
}

void cMap::addBuilding( cBuilding* building, unsigned int x, unsigned int y )
{
	addBuilding( building, x + y * size );
}

void cMap::addBuilding( cBuilding* building, unsigned int offset )
{
	if ( building->data.surfacePosition != sUnitData::SURFACE_POS_GROUND && building->data.isBig ) return; //big base building are not implemented

	if ( building->data.isBig )
	{
		//assumption: there is no rubble under a top building
		//so a big building will always be the first one
		fields[offset           ].buildings.Insert(0, building );
		fields[offset + 1       ].buildings.Insert(0, building );
		fields[offset + size    ].buildings.Insert(0, building );
		fields[offset + size + 1].buildings.Insert(0, building );
	}
	else
	{
		unsigned int i = 0;
		int mapLevel = getMapLevel( building );
		while ( i < fields[offset].buildings.Size() && getMapLevel(fields[offset].buildings[i]) < mapLevel )
		{
			i++;
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
	if ( vehicle->data.factorAir > 0 )
	{
		fields[offset].planes.Insert(0, vehicle );
	}
	else
	{
		fields[offset].vehicles.Insert(0, vehicle );
	}
}

void cMap::deleteBuilding( cBuilding* building )
{
	int offset = building->PosX + building->PosY * size;

	cList<cBuilding*>* buildings = &fields[offset].buildings;
	for ( unsigned int i = 0; i < buildings->Size(); i++ )
	{
		if ( (*buildings)[i] == building ) buildings->Delete(i);
	}

	if ( building->data.isBig )
	{
		//big building must be a top building or rubble
		//assumption: there is no rubble under a top building
		//so only check the first building
		offset++;
		buildings = &fields[offset].buildings;
		if ( (*buildings)[0] == building ) buildings->Delete(0);
		
		offset += size;
		buildings = &fields[offset].buildings;
		if ( (*buildings)[0] == building ) buildings->Delete(0);
		
		offset--;
		buildings = &fields[offset].buildings;
		if ( (*buildings)[0] == building ) buildings->Delete(0);
		
	}
}

void cMap::deleteVehicle( cVehicle* vehicle )
{
	int offset = vehicle->PosX + vehicle->PosY *size;

	if ( vehicle->data.factorAir > 0 )
	{
		cList<cVehicle*>& planes = fields[offset].planes;
		if ( planes.Size() <= 0 ) return;
		for ( unsigned int i = 0; i < planes.Size(); i++ )
		{
			if ( planes[i] == vehicle )
			{
				planes.Delete(i);
				break;
			}
			if ( i == planes.Size()-1 ) return;
		}
	}
	else
	{
		if ( fields[offset].vehicles.Size() > 0 && fields[offset].vehicles[0] == vehicle )
		{
			//only one vehicle per field allowed
			fields[offset].vehicles.Delete(0);

			//check, whether the vehicle is centered on 4 map fields
			if ( vehicle->data.isBig )
			{
				offset++;
				fields[offset].vehicles.Delete(0);
				offset += size;
				fields[offset].vehicles.Delete(0);
				offset--;
				fields[offset].vehicles.Delete(0);
			}
		}
		else return;
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

	if ( vehicle->data.factorAir > 0 )
	{
		cList<cVehicle*>& planes = fields[oldOffset].planes;
		for ( unsigned int i = 0; i < planes.Size(); i++ )
		{
			if ( planes[i] == vehicle ) planes.Delete(i);
		}
		fields[newOffset].planes.Insert(0, vehicle );
	}
	else
	{
		cList<cVehicle*>& vehicles = fields[oldOffset].vehicles;
		for ( unsigned int i = 0; i < vehicles.Size(); i++ )
		{
			if ( vehicles[i] == vehicle ) vehicles.Delete(i);
		}

		//check, whether the vehicle is centered on 4 map fields
		if ( vehicle->data.isBig )
		{
			oldOffset++;
			fields[oldOffset].vehicles.Delete(0);
			oldOffset += size;
			fields[oldOffset].vehicles.Delete(0);
			oldOffset--;
			fields[oldOffset].vehicles.Delete(0);

			vehicle->data.isBig = false;
		}

		fields[newOffset].vehicles.Insert(0, vehicle );
	}
}

void cMap::moveVehicleBig( cVehicle* vehicle, unsigned int x, unsigned int y)
{
	moveVehicleBig( vehicle, x + y * size );
}

void cMap::moveVehicleBig( cVehicle* vehicle, unsigned int offset )
{
	if ( vehicle->data.isBig ) 
	{
		Log.write("Calling moveVehicleBig on a big vehicle", cLog::eLOG_TYPE_NET_ERROR );
		//calling this this function twice is allways an error.
		//nevertheless try to proceed by resetting the data.isBig flag
		moveVehicle(vehicle, offset);
	}

	int oldOffset = vehicle->PosX + vehicle->PosY * size;
	fields[oldOffset].vehicles.Delete(0);

	vehicle->PosX = offset % size;
	vehicle->PosY = offset / size;

	fields[offset].vehicles.Insert(0, vehicle );
	offset++;
	fields[offset].vehicles.Insert(0, vehicle );
	offset += size;
	fields[offset].vehicles.Insert(0, vehicle );
	offset--;
	fields[offset].vehicles.Insert(0, vehicle );

	vehicle->data.isBig = true;
}

bool cMap::possiblePlace( const cVehicle* vehicle, int x, int y, const cPlayer* player ) const
{
	if ( x < 0 || x >= size || y < 0 || y >= size ) return false;
	return possiblePlaceVehicle( vehicle->data, x + y * size, player );
}

bool cMap::possiblePlace( const cVehicle* vehicle, int offset, const cPlayer* player ) const
{
	return possiblePlaceVehicle( vehicle->data, offset, player );
}

bool cMap::possiblePlaceVehicle( const sUnitData& vehicleData, int x, int y, const cPlayer* player ) const
{
	if ( x < 0 || x >= size || y < 0 || y >= size ) return false;
	return possiblePlaceVehicle( vehicleData, x + y * size, player );
}

bool cMap::possiblePlaceVehicle( const sUnitData& vehicleData, int offset, const cPlayer* player ) const
{
	if ( offset < 0 || offset >= size*size ) return false;

	//search first building, that is not a connector
	cBuildingIterator building = fields[offset].getBuildings();
	if ( building && building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE ) building++;

	if ( vehicleData.factorAir > 0 )
	{
		if ( player && !player->ScanMap[offset] ) return true;
		//only one plane per field for now
		if ( fields[offset].planes.Size() > 0 ) return false;
	}
	if ( vehicleData.factorGround > 0 )
	{
		if ( terrain[Kacheln[offset]].blocked ) return false;
		
		if ( ( terrain[Kacheln[offset]].water && vehicleData.factorSea == 0  ) || ( terrain[Kacheln[offset]].coast && vehicleData.factorCoast == 0 ) )
		{
			if ( player && !player->ScanMap[offset] ) return false;

			//vehicle can drive on water, if there is a bridge, platform or road
			if ( !building ) return false;
			if ( !( building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE ) ) return false;
		}
		if ( player && !player->ScanMap[offset] ) return true;			

		if ( fields[offset].vehicles.Size() > 0 ) return false;
		if ( building )
		{
			//only base buildings and rubbe is allowed on the same field with a vehicle (connectors have been skiped, so doesn't matter here)
			if ( !( building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || !building->owner ) ) return false;
		}
	}
	else if ( vehicleData.factorSea > 0 )
	{
		if ( terrain[Kacheln[offset]].blocked ) return false;

		if ( !terrain[Kacheln[offset]].water && ( !terrain[Kacheln[offset]].coast || vehicleData.factorCoast == 0 ) ) return false;

		if ( player && !player->ScanMap[offset] ) return true;

		if ( fields[offset].vehicles.Size() > 0 ) return false;

		//only bridge and sea mine are allowed on the same field with a ship (connectors have been skiped, so doesn't matter here)
		if ( building && !( building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA ) )
		{
			// if the building is a landmine, we have to check whether it's on a bridge or not
			if ( building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE )
			{
				building++;
				if ( building.end || building->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA ) return false;
			}
			else return false;
		}
	}

	return true;
}

bool cMap::possiblePlaceBuilding( const sUnitData& buildingData, int x, int y, cVehicle* vehicle ) const
{
	if ( x < 0 || x >= size || y < 0 || y >= size ) return false;
	return possiblePlaceBuilding( buildingData, x + y * size, vehicle );
}

bool cMap::possiblePlaceBuilding( const sUnitData& buildingData, int offset, cVehicle* vehicle ) const
{
	if ( offset < 0 || offset >= size*size ) return false;
	if ( terrain[Kacheln[offset]].blocked ) return false;
	cMapField& field = fields[offset];
	bool checkBeneathSea = false, checkAboveBase = false;

	cBuildingIterator bi = field.getBuildings();
	if ( !bi.end && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE )
	{
		//can not build connectors on connectors
		if ( buildingData.surfacePosition == sUnitData::SURFACE_POS_ABOVE ) return false;

		//use next building under the connector for the next checks
		bi++;
	}

	if ( buildingData.factorSea )
	{
		if ( !terrain[Kacheln[offset]].water && buildingData.factorGround == 0 && buildingData.factorCoast == 0 ) return false;
		if ( terrain[Kacheln[offset]].coast && buildingData.factorCoast == 0 ) return false;

		if ( !bi.end )
		{
			if ( buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE && buildingData.surfacePosition != sUnitData::SURFACE_POS_BENEATH_SEA && ( bi->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_NO ) ) return false;
			if ( buildingData.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA ) checkBeneathSea = true;
		}	
	}
	else if ( buildingData.factorGround )
	{
		if ( terrain[Kacheln[offset]].water || ( terrain[Kacheln[offset]].coast && buildingData.factorCoast == 0 ) )
		{
			//can not be built on water, but terrain is water
			//so a base building is required
			if ( buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE && buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE && ( !bi || bi->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_NO ) ) return false;
			if ( buildingData.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE ) checkAboveBase = true;
		}
		
		if ( !bi.end )
		{
			if ( buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE && buildingData.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE )
			{
				// cannot build normal buildings over normal buildings
				if ( ( bi->data.surfacePosition != sUnitData::SURFACE_POS_BASE && bi->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_SEA && bi->data.surfacePosition != sUnitData::SURFACE_POS_ABOVE_BASE) || ( bi->data.canBeOverbuild == sUnitData::OVERBUILD_TYPE_NO ) ) return false;
			}
			if ( buildingData.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE && bi->owner && !( bi->data.surfacePosition == sUnitData::SURFACE_POS_BASE || bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA ) ) return false;
		}
	}
	
	//can not build on rubble
	if (bi && !bi->owner && ! ( buildingData.surfacePosition == sUnitData::SURFACE_POS_ABOVE || buildingData.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE )) return false;

	if ( field.vehicles.Size() > 0 )
	{
		if ( !vehicle ) return false;
		if ( vehicle != field.vehicles[0] ) return false;
	}

	if ( checkBeneathSea || checkAboveBase )
	{
		do
		{
			if ( checkBeneathSea && bi->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA ) return false;
			if ( checkAboveBase && bi->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE) return false;
			bi++;
		}
		while ( !bi.end );
	}

	return true;
}
void cMap::reset()
{
	for ( int i = 0; i < size*size; i++ )
	{
		while ( fields[i].buildings.Size() ) fields[i].buildings.Delete(0);
		while ( fields[i].vehicles.Size() ) fields[i].vehicles.Delete(0);
		while ( fields[i].planes.Size() ) fields[i].planes.Delete(0);
	}
}