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

#include <math.h>
#include <iostream>
#include <stdio.h>
#include <sstream>

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <SDL_getenv.h>

#define __main__
#include "defines.h"
#include "main.h"
#include "files.h"
#include "mouse.h"
#include "pcx.h"
#include "keys.h"
#include "sound.h"
#include "map.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "base.h"
#include "network.h"
#include "log.h"
#include "loaddata.h"
#include "tinyxml.h"
#include "events.h"
#include "client.h"
#include "server.h"
#include "savegame.h"
#include "mveplayer.h"
#include "input.h"
#include "unifonts.h"
#include "menus.h"
#include "clans.h"
#include "settings.h"

int main ( int argc, char *argv[] )
{
	setPaths(); //first thing: figure out paths

	if ( initSDL() == -1 ) return -1;  //stop on error during init of SDL basics. WARNINGS will be ignored!

	{
		string sVersion = PACKAGE_NAME; sVersion += " ";
		sVersion += PACKAGE_VERSION; sVersion += " ";
		sVersion += PACKAGE_REV; sVersion += " ";
		Log.write ( sVersion, cLog::eLOG_TYPE_INFO );
		string sBuild = "Build: "; sBuild += MAX_BUILD_DATE;
		Log.write ( sBuild , cLog::eLOG_TYPE_INFO );
		#if HAVE_AUTOVERSION_H
			string sBuildVerbose = "On: ";
			sBuildVerbose += BUILD_UNAME_S; 
			sBuildVerbose += " "; 
			sBuildVerbose += BUILD_UNAME_R;
			Log.write ( sBuildVerbose, cLog::eLOG_TYPE_INFO );

			sBuildVerbose = "From: ";
			sBuildVerbose += BUILD_USER;
			sBuildVerbose += " at "; 
			sBuildVerbose += BUILD_UNAME_N;
			Log.write ( sBuildVerbose, cLog::eLOG_TYPE_INFO );
		#endif
		Log.mark();
		Log.write ( sVersion, cLog::eLOG_TYPE_NET_DEBUG );
		Log.write ( sBuild , cLog::eLOG_TYPE_NET_DEBUG );
	}

	srand ( ( unsigned ) time ( NULL ) ); //start random number generator

	if( ReadMaxXml() == -1 )
	{
		Quit();
		return -1;
	}

	showSplash(); //show splashscreen
	initSound(); //now config is loaded and we can init sound and net
	initNet();

	// load files
	SDL_Thread *DataThread = NULL;
	DataThread = SDL_CreateThread ( LoadData,NULL );

	SDL_Event event;
	while ( LoadingData != LOAD_FINISHED )
	{
		if ( LoadingData == LOAD_ERROR )
		{
			Log.write ( "Error while loading data!", cLog::eLOG_TYPE_ERROR );
			SDL_WaitThread ( DataThread, NULL );
			Quit();
		}
		while ( SDL_PollEvent ( &event ) )
		{
			if ( event.type == SDL_ACTIVEEVENT )
			{
				SDL_UpdateRect ( screen,0,0,0,0 );
			}
		}
		SDL_Delay ( 100 );
	}

	// play intro if we're supposed to and the file exists
	if(SettingsData.bIntro)
	{
		if(FileExists((SettingsData.sMVEPath + PATH_DELIMITER + "MAXINT.MVE").c_str()))
		{
			// Close maxr sound for intro movie
			CloseSound();

			char mvereturn;
			Log.write ( "Starting movie " + SettingsData.sMVEPath + PATH_DELIMITER + "MAXINT.MVE", cLog::eLOG_TYPE_DEBUG );
			mvereturn = MVEPlayer((SettingsData.sMVEPath + PATH_DELIMITER + "MAXINT.MVE").c_str(), SettingsData.iScreenW, SettingsData.iScreenH, !SettingsData.bWindowMode, !SettingsData.SoundMute);
			Log.write("MVEPlayer returned " + iToStr(mvereturn), cLog::eLOG_TYPE_DEBUG);
		//FIXME: make this case sensitive - my mve is e.g. completly lower cases -- beko

			// reinit maxr sound
			if ( SettingsData.bSoundEnabled && !InitSound ( SettingsData.iFrequency, SettingsData.iChunkSize ) )
			{
				Log.write("Can't reinit sound after playing intro" + iToStr(mvereturn), cLog::eLOG_TYPE_DEBUG);
			}
		}
		else
		{
			Log.write ( "Couldn't find movie " + SettingsData.sMVEPath + PATH_DELIMITER + "MAXINT.MVE", cLog::eLOG_TYPE_WARNING );
		}
	}
	else
	{
		Log.write ( "Skipped intro movie due settings", cLog::eLOG_TYPE_DEBUG );
	}

	SDL_WaitThread ( DataThread, NULL );

	showGameWindow(); //start game-window
	mouse = new cMouse;
	InputHandler = new cInput;
	EventHandler = new cEventHandling;
	cStartMenu mainMenu;
	mainMenu.show();

	Quit();
	return 0;
}

// generate SplashScreen
void showSplash()
{
        const SDL_VideoInfo *vInfo = SDL_GetVideoInfo();
        Uint8 uBpp = vInfo->vfmt->BitsPerPixel;
	
	buffer = LoadPCX(SPLASH_BACKGROUND);
	SDL_WM_SetIcon ( SDL_LoadBMP ( MAXR_ICON ), NULL );
	
	//set window to center of screen.
	char cVideoPos[21] = "SDL_VIDEO_CENTERED=1";
	if(putenv( cVideoPos)!=0)
	{
		Log.write("Couldn't export SDL_VIDEO_CENTERED", cLog::eLOG_TYPE_WARNING);
	}

	if(SettingsData.iColourDepth > (Uint32)uBpp)
	{
	  Log.write("Requested colordepth from config is higher than the display has!", cLog::eLOG_TYPE_WARNING);
	}
	
	//made it far enough to start game
	screen=SDL_SetVideoMode ( SPLASHWIDTH, SPLASHHEIGHT, SettingsData.iColourDepth, OtherData.iSurface|SDL_NOFRAME );
	SDL_BlitSurface ( buffer,NULL,screen,NULL );
	
	string sVersion = PACKAGE_NAME; sVersion += " ";
	sVersion += PACKAGE_VERSION; sVersion += " ";
	sVersion += PACKAGE_REV; sVersion += " ";
	SDL_WM_SetCaption ( sVersion.c_str(), NULL );
	SDL_UpdateRect ( screen,0,0,0,0 );
}

void showGameWindow()
{
	SDL_FreeSurface(buffer); //delete splash image
	buffer=SDL_CreateRGBSurface ( OtherData.iSurface|SDL_SRCCOLORKEY,SettingsData.iScreenW,SettingsData.iScreenH,SettingsData.iColourDepth,0,0,0,0 );

	//set window to center of screen.
	char cVideoPos[21] = "SDL_VIDEO_CENTERED=1";
	if(putenv( cVideoPos)!=0)
	{
		Log.write("Couldn't export SDL_VIDEO_CENTERED", cLog::eLOG_TYPE_WARNING);
	}

	screen=SDL_SetVideoMode ( buffer->w,buffer->h,buffer->format->BitsPerPixel,OtherData.iSurface|(SettingsData.bWindowMode?0:SDL_FULLSCREEN) );

	if ( screen == NULL )
	{
		Log.write("Couldn't set video mode w: " + iToStr( buffer->w ) +  " h: " + iToStr ( buffer->h ) + " bpp: " + iToStr ( buffer->format->BitsPerPixel ) + (SettingsData.bWindowMode?" window":" fullscreen"), cLog::eLOG_TYPE_ERROR);
		Quit();
	}

	SDL_FillRect ( buffer,NULL,SDL_MapRGB (buffer->format, 0, 0, 0) );

	string sVersion = PACKAGE_NAME; sVersion += " ";
	sVersion += PACKAGE_VERSION; sVersion += " ";
	sVersion += PACKAGE_REV; sVersion += " ";
	SDL_WM_SetCaption ( sVersion.c_str(), NULL ); //set caption
	SDL_ShowCursor ( 0 );
}

int initSDL()
{
	if ( SDL_Init ( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE ) == -1 ) // start SDL basics
	{
		Log.write ( "Could not init SDL",cLog::eLOG_TYPE_ERROR );
		Log.write ( SDL_GetError(),cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	else
	{
		Log.write ( "Initalized SDL basics - looks good!",cLog::eLOG_TYPE_INFO );
		Log.mark();
		//made it - enough to start game
		return 0;
	}
}

int initSound()
{
	if (!SettingsData.bSoundEnabled)
	{
		Log.write ( "Sound disabled due configuration", cLog::eLOG_TYPE_INFO);
		return 1;
	}

	if ( SDL_Init ( SDL_INIT_AUDIO ) < 0 ) //start sound
	{
		Log.write ( "Could not init SDL_INIT_AUDIO",cLog::eLOG_TYPE_WARNING );
		Log.write ( "Sound won't  be avaible!",cLog::eLOG_TYPE_WARNING );
		Log.write ( SDL_GetError(),cLog::eLOG_TYPE_WARNING );
		SettingsData.bSoundEnabled=false;
		return -1;
	}

	if ( !InitSound ( SettingsData.iFrequency, SettingsData.iChunkSize ) )
        {
		Log.write ( "Could not access mixer",cLog::eLOG_TYPE_WARNING );
		Log.write ( "Sound won't  be avaible!",cLog::eLOG_TYPE_WARNING );
		SettingsData.bSoundEnabled = false;
                return -1;
        }
	Log.write ( "Sound started", cLog::eLOG_TYPE_INFO);
	return 0;
}

int initNet()
{
	if ( SDLNet_Init() == -1 ) // start SDL_net
	{
		Log.write ( "Could not init SDLNet_Init\nNetwork games won' be avaible! ",cLog::eLOG_TYPE_WARNING );
		Log.write ( SDL_GetError(),cLog::eLOG_TYPE_WARNING );
		return -1;
	}
	Log.write ( "Net started", cLog::eLOG_TYPE_INFO);
	return 0;
}

void Quit()
{
	delete mouse;
	delete font;
	//unload files here
	CloseSound();
	SDLNet_Quit();
	SDL_FreeSurface(buffer);
	SDL_FreeSurface(screen);
	SDL_Quit();
	Log.write ( "EOF" );
	exit ( 0 );
}

template<typename Type> void drawStetchedLine ( Type *srcPixelData, int srcWidth, Type *destPixelData, int destWidth )
{
	int i = 0;
	int width = destWidth;
	Type pixel = 0;
	Type* srcEnd = srcPixelData + srcWidth;
	// go trough all pixel in this line
	while( srcPixelData < srcEnd )
	{
		pixel = *srcPixelData;
	drawpixel:
		// copy the pixel
		*destPixelData++ = pixel;
		width--;
		i += srcWidth;
		if ( !width ) break;
		// draw pixel once more if necessary
		if ( i < destWidth ) goto drawpixel;
		// skip pixels when necessary
		do
		{
			i -= destWidth;
			srcPixelData++;
		}
		while ( i >= destWidth );
	};
	return; 
}

SDL_Surface *scaleSurface( SDL_Surface *scr, SDL_Surface *dest, int width, int height )
{
	if ( width <= 0 || height <= 0 || !scr ) return NULL;
	SDL_Surface *surface;

	//can not enlage an existing surface
	if ( width > scr->w && dest ) width = scr->w;
	if ( height > scr->h && dest ) height = scr->h;

	// generate new surface if necessary
	if ( dest == NULL ) surface = SDL_CreateRGBSurface(scr->flags, width, height, scr->format->BitsPerPixel, scr->format->Rmask, scr->format->Gmask, scr->format->Bmask, scr->format->Amask);
	else
	{
		// else set the size of the old one
		surface = dest;
		surface->w = width;
		surface->h = height;
	}

	if ( SDL_MUSTLOCK(scr) ) SDL_LockSurface( scr );
	if ( SDL_MUSTLOCK(surface)) SDL_LockSurface( surface );

	// just blit the surface when the new size is identic to the old one
	/*if ( scr->w == width && scr->h == height )
	{
		SDL_BlitSurface ( scr, NULL, surface, NULL );
		return surface;
	}*/
	// copy palette when necessary
	if ( scr->format->BitsPerPixel == 8 && !dest )
	{
		for (int i = 0; i < 256; i++ )
		{
			surface->format->palette->colors[i].r = scr->format->palette->colors[i].r;
			surface->format->palette->colors[i].g = scr->format->palette->colors[i].g;
			surface->format->palette->colors[i].b = scr->format->palette->colors[i].b;
		}
	}

	int srcRow = 0;
	int destRow = 0;
	int i = 0;
	Uint8* srcPixelData;
	Uint8* destPixelData;
	// go trough all rows
	while ( srcRow < scr->h )
	{
		srcPixelData = (Uint8*)scr->pixels+(srcRow*scr->pitch);
		// draw the complete line
	drawline:
		destPixelData = (Uint8*)surface->pixels+(destRow*surface->pitch);

		// pay attention to diffrent surface formats
		switch ( scr->format->BytesPerPixel )
		{
		case 1:
		    drawStetchedLine<Uint8>( srcPixelData, scr->w, destPixelData, surface->w );
		    break;
		case 2:
		    drawStetchedLine<Uint16>( (Uint16*)srcPixelData, scr->w, (Uint16*)destPixelData, surface->w );
		    break;
		case 3:
		    // not yet supported
		    break;
		case 4:
		    drawStetchedLine<Uint32>( (Uint32*)srcPixelData, scr->w, (Uint32 *)destPixelData, surface->w );
		    break;
		}
		destRow++;
		i += scr->h;
		// break when we have already finished
	    if ( destRow == surface->h ) break;
		// draw the line once more when the destiniation surface has a bigger height then the source surface
	    if ( i < surface->h ) goto drawline;
		// skip lines in the source surface when the destiniation surface has a smaller height then the source surface
	    do
		{
			i -= surface->h;
			srcRow++;
		}
		while ( i >= surface->h );
	}

	if ( SDL_MUSTLOCK(scr) ) SDL_UnlockSurface( scr );
	if ( SDL_MUSTLOCK(surface)) SDL_UnlockSurface( surface );

	return surface;
}

// CreatePfeil ////////////////////////////////////////////////////////////////
// Erzeigt ein Pfeil-Surface:
SDL_Surface *CreatePfeil ( int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size )
{
	SDL_Surface *sf;
	float fak;
	sf=SDL_CreateRGBSurface ( OtherData.iSurface|SDL_SRCCOLORKEY,size,size,SettingsData.iColourDepth,0,0,0,0 );
	SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_FillRect ( sf,NULL,0xFF00FF );
	SDL_LockSurface ( sf );

	fak=(float)(size/64.0);
	line ( ( int )Round ( p1x*fak ),( int )Round ( p1y*fak ),( int )Round ( p2x*fak ),( int )Round ( p2y*fak ),color,sf );
	line ( ( int )Round ( p2x*fak ),( int )Round ( p2y*fak ),( int )Round ( p3x*fak ),( int )Round ( p3y*fak ),color,sf );
	line ( ( int )Round ( p3x*fak ),( int )Round ( p3y*fak ),( int )Round ( p1x*fak ),( int )Round ( p1y*fak ),color,sf );

	SDL_UnlockSurface ( sf );
	return sf;
}


void line ( int x1,int y1,int x2,int y2,unsigned int color,SDL_Surface *sf )
{
	int dx,dy,dir=1,error=0,*ptr;
	ptr= ( int* ) ( sf->pixels );
	if ( x2<x1 )
	{
		dx=x1;dy=y1;
		x1=x2;y1=y2;
		x2=dx;y2=dy;
	}
	dx=x2-x1;
	dy=y2-y1;
	if ( dy<0 ) {dy=-dy;dir=-1;}
	if ( dx>dy )
	{
		for ( ;x1!=x2;x1++,error+=dy )
		{
			if ( error>dx ) {error-=dx;y1+=dir;}
			if ( x1<sf->w&&x1>=0&&y1>=0&&y1<sf->h )
				ptr[x1+y1*sf->w]=color;
		}
		return;
	}
	for ( ;y1!=y2;y1+=dir,error+=dx )
	{
		if ( error>dy ) {error-=dy;x1++;}
		if ( x1<sf->w&&x1>=0&&y1>=0&&y1<sf->h )
			ptr[x1+y1*sf->w]=color;
	}
}
void drawCircle( int iX, int iY, int iRadius, int iColor, SDL_Surface *surface )
{
	int d,da,db,xx,yy,bry;
	unsigned int *ptr;
	if ( iX + iRadius < 0 || iX - iRadius > SettingsData.iScreenW || iY + iRadius < 0 || iY - iRadius > SettingsData.iScreenH ) return;
	SDL_LockSurface ( surface );

	ptr = ( unsigned int* ) surface->pixels;

	d = 0;
	xx = 0;
	yy = iRadius;
	bry = ( int ) Round ( 0.70710678*iRadius,0 );
	while ( yy > bry )
	{
		da=d+ ( xx<<1 ) +1;
		db=da- ( yy<<1 ) +1;
		if ( abs ( da ) <abs ( db ) )
		{
			d=da;
			xx++;
		}
		else
		{
			d=db;
			xx++;
			yy--;
		}
		setPixel ( surface, iX + xx, iY + yy, iColor );
		setPixel ( surface, iX + yy, iY + xx, iColor );
		setPixel ( surface, iX + yy, iY - xx, iColor );
		setPixel ( surface, iX + xx, iY - yy, iColor );
		setPixel ( surface, iX - xx, iY + yy, iColor );
		setPixel ( surface, iX - yy, iY + xx, iColor );
		setPixel ( surface, iX - yy, iY - xx, iColor );
		setPixel ( surface, iX - xx, iY - yy, iColor );
	}
	SDL_UnlockSurface ( surface );
}

void setPixel( SDL_Surface* surface, int x, int y, int iColor )
{
	//check the surface size
	if ( x < 0 || x >= surface->w || y < 0 || y >= surface->h ) return;
	//check the clip rect
	if ( x < surface->clip_rect.x || x >= surface->clip_rect.x + surface->clip_rect.w || y < surface->clip_rect.y || y >= surface->clip_rect.y + surface->clip_rect.h ) return;
	
	((unsigned int*) surface->pixels)[x + y * surface->w] = iColor;
}

int random(int const x)
{
	return (int)((double)rand() / RAND_MAX * x);
}

string iToStr(int x)
{
 	stringstream strStream;
 	strStream << x;
 	return strStream.str();
}

string dToStr(double x)
{
 	stringstream strStream;
 	strStream << x;
 	return strStream.str();
}

std::string pToStr(void *x)
{
	stringstream strStream;
 	strStream << x;
 	return "0x" + strStream.str();
}

// Round //////////////////////////////////////////////////////////////////////
// Rounds a Number to 'iDecimalPlace' digits after the comma:
double Round ( double dValueToRound, unsigned int iDecimalPlace )
{
	dValueToRound *= pow ( ( double ) 10, ( int ) iDecimalPlace );
	if ( dValueToRound >= 0 )
		dValueToRound = floor ( dValueToRound + 0.5 );
	else
		dValueToRound = ceil ( dValueToRound - 0.5 );
	dValueToRound /= pow ( ( double ) 10, ( int ) iDecimalPlace );
	return dValueToRound;
}

int Round ( double dValueToRound )
{
	return ( int ) Round ( dValueToRound,0 );
}


//----------------------------------------------------------------------------------
// ----------- sID Implementation --------------------------------------------------
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
string sID::getText()
{
	char tmp[6];
	sprintf ( tmp, "%0.2d %0.2d", iFirstPart, iSecondPart );
	return tmp;
}

//----------------------------------------------------------------------------------
void sID::generate ( string text )
{
	iFirstPart = atoi ( text.substr( 0, text.find( " ", 0 ) ).c_str() );
	iSecondPart = atoi ( text.substr( text.find( " ", 0 ), text.length() ).c_str() );
}

//----------------------------------------------------------------------------------
sUnitData *sID::getUnitDataOriginalVersion( cPlayer *Owner )
{
	switch ( iFirstPart )
	{
		case 0:
			if ( !getVehicle (Owner) ) return NULL;
			return &(getVehicle (Owner)->data);
		case 1:
			if ( !getBuilding (Owner) ) return NULL;
			return &(getBuilding (Owner)->data);
		default:
		return NULL;
	}
	return NULL;
}

//----------------------------------------------------------------------------------
sUnitData *sID::getUnitDataCurrentVersion( cPlayer *Owner )
{
	if (iFirstPart == 0)
	{
		for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
		{
			if ( Owner->VehicleData[i].ID == *this ) 
				return &Owner->VehicleData[i];
		}
	}
	else if (iFirstPart == 1)
	{
		for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
		{
			if ( Owner->BuildingData[i].ID == *this ) 
				return &Owner->BuildingData[i];
		}		
	}
	return 0;	
}

//----------------------------------------------------------------------------------
sVehicle *sID::getVehicle (cPlayer* Owner)
{
	if ( iFirstPart != 0 ) return NULL;
	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++ )
	{
		sVehicle* result = Owner ? &UnitsData.getVehicle (i, Owner->getClan ()) : &UnitsData.getVehicle (i);
		if ( result->data.ID == *this ) return result;
	}
	return NULL;
}

//----------------------------------------------------------------------------------
sBuilding *sID::getBuilding (cPlayer* Owner)
{
	if ( iFirstPart != 1 ) return NULL;
	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++ )
	{
		sBuilding* result = Owner ? &UnitsData.getBuilding (i, Owner->getClan ()) : &UnitsData.getBuilding (i);
		if ( result->data.ID == *this ) return result;
	}
	return NULL;
}

//----------------------------------------------------------------------------------
bool sID::operator ==(const sID &ID) const
{
	if ( iFirstPart == ID.iFirstPart && iSecondPart == ID.iSecondPart ) return true;
	return false;
}

//----------------------------------------------------------------------------------
cUnitsData::cUnitsData ()
: initializedClanUnitData (false) 
{
}

//----------------------------------------------------------------------------------
sVehicle& cUnitsData::getVehicle (int nr, int clan)
{
	if (initializedClanUnitData == false)
		initializeClanUnitData ();
	
	if (clan < 0 || clan > (int) clanUnitDataVehicles.size ())
	{
		return vehicle[nr];
	}
	return clanUnitDataVehicles.at (clan).at (nr);//[clan][nr];
}

//----------------------------------------------------------------------------------
sBuilding& cUnitsData::getBuilding (int nr, int clan)
{
	if (initializedClanUnitData == false)
		initializeClanUnitData ();
	
	if (clan < 0 || clan > (int) clanUnitDataBuildings.size ())
	{
		return building[nr];
	}
	return clanUnitDataBuildings.at (clan).at (nr);//[clan][nr];
}

//----------------------------------------------------------------------------------
void cUnitsData::initializeClanUnitData ()
{
	cClanData& clanData = cClanData::instance ();
	for (int clanIdx = 0; clanIdx < clanData.getNrClans (); clanIdx++)
	{
		cClan* clan = clanData.getClan (clanIdx);
		if (clan == 0)
			continue; 
		
		vector<sVehicle> clanListVehicles;
		for (unsigned int vehicleIdx = 0; vehicleIdx < vehicle.Size (); vehicleIdx++)
		{
			// make a copy of the vehicle's stats
			sVehicle curVehicle = vehicle[vehicleIdx];
			clanListVehicles.push_back (curVehicle);
			
			cClanUnitStat* changedStat = clan->getUnitStat (curVehicle.data.ID.iFirstPart, curVehicle.data.ID.iSecondPart);
			if (changedStat != 0)
			{
				if (changedStat->hasModification ("Damage"))
					clanListVehicles.back ().data.damage = changedStat->getModificationValue ("Damage");
				if (changedStat->hasModification ("Range"))
					clanListVehicles.back ().data.range = changedStat->getModificationValue ("Range");
				if (changedStat->hasModification ("Armor"))
					clanListVehicles.back ().data.armor = changedStat->getModificationValue ("Armor");
				if (changedStat->hasModification ("Hitpoints"))
					clanListVehicles.back ().data.hitpointsMax = changedStat->getModificationValue ("Hitpoints");
				if (changedStat->hasModification ("Scan"))
					clanListVehicles.back ().data.scan = changedStat->getModificationValue ("Scan");
				if (changedStat->hasModification ("Speed"))
					clanListVehicles.back ().data.speedMax = changedStat->getModificationValue ("Speed") * 4;
				if (changedStat->hasModification ("Built_Costs"))
					clanListVehicles.back ().data.buildCosts = changedStat->getModificationValue ("Built_Costs");
			}
		}
		clanUnitDataVehicles.push_back (clanListVehicles);
		
		vector<sBuilding> clanListBuildings;
		for (unsigned int buildingIdx = 0; buildingIdx < building.Size (); buildingIdx++)
		{
			// make a copy of the building's stats
			sBuilding curBuilding = building[buildingIdx];
			clanListBuildings.push_back (curBuilding);
			
			cClanUnitStat* changedStat = clan->getUnitStat (curBuilding.data.ID.iFirstPart, curBuilding.data.ID.iSecondPart);
			if (changedStat != 0)
			{
				if (changedStat->hasModification ("Damage"))
					clanListBuildings.back ().data.damage = changedStat->getModificationValue ("Damage");
				if (changedStat->hasModification ("Range"))
					clanListBuildings.back ().data.range = changedStat->getModificationValue ("Range");
				if (changedStat->hasModification ("Armor"))
					clanListBuildings.back ().data.armor = changedStat->getModificationValue ("Armor");
				if (changedStat->hasModification ("Hitpoints"))
					clanListBuildings.back ().data.hitpointsMax = changedStat->getModificationValue ("Hitpoints");
				if (changedStat->hasModification ("Scan"))
					clanListBuildings.back ().data.scan = changedStat->getModificationValue ("Scan");
				if (changedStat->hasModification ("Speed"))
					clanListBuildings.back ().data.speedMax = changedStat->getModificationValue ("Speed") * 4;
				if (changedStat->hasModification ("Built_Costs"))
					clanListBuildings.back ().data.buildCosts = changedStat->getModificationValue ("Built_Costs");
			}
		}
		clanUnitDataBuildings.push_back (clanListBuildings);
	}
	initializedClanUnitData = true;
}

//----------------------------------------------------------------------------------
sUnitData::sUnitData()
{
	version = 0;
	muzzleType = MUZZLE_TYPE_NONE;

	ammoMax = 0;
	ammoCur = 0;
	shotsMax = 0;
	shotsCur = 0;
	range = 0;
	damage = 0;	
	canAttack = 0;
	canDriveAndFire = false;

	buildCosts = 0;
	maxBuildFactor = 0;
	
	canBuildPath = false;
	canBuildRepeat = false;
	buildIntern = false;
	
	// Movement
	speedMax = 0;
	speedCur = 0;
	
	factorGround = 0.0f;
	factorSea = 0.0f;
	factorAir = 0.0f;
	factorCoast = 0.0f;
	
	// Abilities
	isBig = false;
	connectsToBase = false;
	armor = 0;
	hitpointsMax = 0;
	hitpointsCur = 0;
	scan = 0;
	modifiesSpeed = 0.0f;
	canClearArea = false;
	canBeCaptured = false;
	canBeDisabled = false;
	canCapture = false;
	canDisable = false;
	canRepair = false;
	canRearm = false;
	canResearch = false;
	canPlaceMines = false;
	canSurvey = false;
	doesSelfRepair = false;
	convertsGold = 0;
	canSelfDestroy = false;
		
	canMineMaxRes = 0;		
	needsMetal = 0;
	needsOil = 0;
	needsEnergy = 0;
	needsHumans = 0;
	produceEnergy = 0;
	produceHumans = 0;
		
	isStealthOn = 0;
	canDetectStealthOn = 0;

	surfacePosition = SURFACE_POS_BENEATH_SEA;
	canBeOverbuild = OVERBUILD_TYPE_NO;
		
	canBeLandedOn = false;
	canWork = false;
	explodesOnContact = false;
	isHuman = false;
		
	// Storage
	storageResMax = 0;
	storageResCur = 0;
	storeResType = STORE_RES_NONE;		
	storageUnitsMax = 0;
	storageUnitsCur = 0;
	storeUnitsImageType = STORE_UNIT_IMG_NONE;

	// Graphic
	hasClanLogos = false;
	hasCorpse = false;
	hasDamageEffect = false;
	hasBetonUnderground = false;
	hasPlayerColor = false;
	hasOverlay = false;
		
	buildUpGraphic = false;
	animationMovement = false;
	powerOnGraphic = false;
	isAnimated = false;
	makeTracks = false;
		
	isConnectorGraphic = false;
	hasFrames = 0;
}

//----------------------------------------------------------------------------------
void blittPerSurfaceAlphaToAlphaChannel(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	SDL_Rect temp1, temp2;

	if ( !dst || !src ) return;

	//check surface formats
	if ( !dst->format->Amask ) return;
	if ( src->format->Amask || !( src->flags & SDL_SRCALPHA )) return;

	if ( srcrect == NULL )
	{
		srcrect = &temp1;
		srcrect->x = 0;
		srcrect->y = 0;
		srcrect->h = src->h;
		srcrect->w = src->w;
	}

	if ( dstrect == NULL )
	{
		dstrect = &temp2;
		dstrect->x = 0;
		dstrect->y = 0;
	}

	int width  = srcrect->w;
	int height = srcrect->h;

	//clip source rect to the source surface
	if ( srcrect->x < 0 )
	{
		width += srcrect->x;
		srcrect->x = 0;
	}
	if ( srcrect->y < 0 )
	{
		height += srcrect->y;
		srcrect->y = 0;
	}
	if ( srcrect->x + width > src->w )
	{
		width = src->w - srcrect->x;
	}
	if ( srcrect->y + height > src->h )
	{
		height = src->h - srcrect->y;
	}

	//clip the dest rect to the destination clip rect
	if ( dstrect->x < dst->clip_rect.x )
	{
		width -= dst->clip_rect.x - dstrect->x;
		srcrect->x += dst->clip_rect.x - dstrect->x;
		dstrect->x = dst->clip_rect.x;
	}
	if ( dstrect->y < dst->clip_rect.y )
	{
		height -= dst->clip_rect.y - dstrect->y;
		srcrect->y += dst->clip_rect.y - dstrect->y;
		dstrect->y = dst->clip_rect.y;
	}
	if ( dstrect->x + width > dst->clip_rect.x + dst->clip_rect.w )
	{
		width -= dstrect->x + width - dst->clip_rect.x - dst->clip_rect.w;
	}
	if ( dstrect->y + height > dst->clip_rect.y + dst->clip_rect.h )
	{
		height -= dstrect->y + height - dst->clip_rect.y - dst->clip_rect.h;
	}


	if ( width <= 0 || height <= 0 ) 
	{
		dstrect->w = 0;
		dstrect->h = 0;
		return;
	}

	if ( SDL_MUSTLOCK(src) ) SDL_LockSurface( src );
	if ( SDL_MUSTLOCK(dst) ) SDL_LockSurface( dst );
	
	//setup needed variables
	Uint32 srcAlpha = src->format->alpha;
	int srmask = src->format->Rmask;
	int sgmask = src->format->Gmask;
	int sbmask = src->format->Bmask;
	int damask = dst->format->Amask;
	int drmask = dst->format->Rmask;
	int dgmask = dst->format->Gmask;
	int dbmask = dst->format->Bmask;
	int rshift = src->format->Rshift - dst->format->Rshift;
	int gshift = src->format->Gshift - dst->format->Gshift;
	int bshift = src->format->Bshift - dst->format->Bshift;
	int ashift = dst->format->Ashift;
	Uint32 colorKey = src->format->colorkey;
	bool useColorKey = (src->flags & SDL_SRCCOLORKEY) != 0;

	
	Uint32* dstPixel = ((Uint32*)dst->pixels) + dstrect->x + dstrect->y * dst->w;
	Uint32* srcPixel = ((Uint32*)src->pixels) + srcrect->x + srcrect->y * src->w;

	for ( int y = 0; y < height; y++ )
	{
		for ( int x = 0; x < width; x++ )
		{
			Uint32 dcolor = *dstPixel;
			Uint32 scolor = *srcPixel;

			if ( useColorKey && scolor == colorKey )
			{
				dstPixel++;
				srcPixel++;
				continue;
			}

			Uint32 r = (scolor & srmask) >> rshift;
			Uint32 g = (scolor & sgmask) >> gshift;
			Uint32 b = (scolor & sbmask) >> bshift;
			Uint32  dalpha = (dcolor & damask) >> ashift;

			r = ( ((dcolor & drmask)>>8) * (255 - srcAlpha) * dalpha + r * srcAlpha );
			g = ( (((dcolor & dgmask)*(255 - srcAlpha) * dalpha)>>8) + g * srcAlpha );
			b = ( (((dcolor & dbmask)*(255 - srcAlpha) * dalpha)>>8) + b * srcAlpha );

			Uint8 a = srcAlpha + dalpha - (srcAlpha * dalpha)/255;;

			if ( a > 0 )
			{
				r /= a;
				g /= a;
				b /= a;
			}
			
			r = r & drmask;
			g = g & dgmask;
			b = b & dbmask;

			*dstPixel = r | g | b | a << ashift;

			dstPixel++;
			srcPixel++;
		}

		dstPixel += dst->pitch/4 - width;
		srcPixel += src->pitch/4 - width;
	}

	if ( SDL_MUSTLOCK(src) ) SDL_UnlockSurface( src );
	if ( SDL_MUSTLOCK(dst) ) SDL_UnlockSurface( dst );
}

void blittAlphaSurface (SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	if ( dst->format->Amask && (src->flags & SDL_SRCALPHA) )
		blittPerSurfaceAlphaToAlphaChannel( src, srcrect, dst, dstrect );
	else
		SDL_BlitSurface( src, srcrect, dst, dstrect );
}
