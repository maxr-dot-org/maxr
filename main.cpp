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

#define __main__
#include "defines.h"
#include "main.h"
#include "files.h"
#include "mouse.h"
#include "menu.h"
#include "pcx.h"
#include "fonts.h"
#include "keyinp.h"
#include "keys.h"
#include "sound.h"
#include "map.h"
#include "game.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "base.h"
#include "ajobs.h"
#include "mjobs.h"
#include "fstcpip.h"
#include "log.h"
#include "loaddata.h"
#include "tinyxml.h"

TList::TList ( void )
{
	Count = 0;
}


int main ( int argc, char *argv[] )
{
	{
		cLog::write ( MAXVERSION );
		std::string str = "Build : ";
		str += MAX_BUILD_DATE;
		cLog::write ( str , cLog::eLOG_TYPE_INFO );
		cLog::mark();
		cLog::write ( MAXVERSION , cLog::eLOG_TYPE_NETWORK );
		cLog::write ( str , cLog::eLOG_TYPE_NETWORK );
	}
	if ( initSDL() == -1 ) return -1;  //stop on error during init of SDL basics. WARNINGS will be ignored!

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
			cLog::write ( "Error while loading data!",LOG_TYPE_ERROR );
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

	SDL_WaitThread ( DataThread, NULL );
//	SDL_Delay ( 3000 ); //debug only
	
	//screen = SDL_SetVideoMode(640,480,8,SDL_FULLSCREEN);

	showGameWindow(); //start game-window

	// Die Maus erzeugen:
	mouse = new cMouse;


	//SDL_Thread *EventThread = NULL;
	//EventThread = SDL_CreateThread ( runEventChecker, NULL);
	
	// Das Menü starten:
	RunMainMenu();
	
	Quit();
	return 0;
}

//IMPORTANT: this is highly temporary! -- beko
int runEventChecker( void *)
{
	bool exit = false;
	SDL_Event event;
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, 2);
	cout << "Eventing\n";
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	
	#define SYM event.key.keysym.sym
	#define MOD event.key.keysym.mod
	#define ETYPE event.type
	while(!exit)
	{
		
		SDL_PollEvent ( &event );

		switch (event.type)
		{
		case SDL_QUIT:
			cout << "Quit \n";
			exit=true;
			break;
		
		case SDL_PRESSED:		
		if ( SYM == SDLK_RETURN)
		{
			cout << "Toggled Enter\n";
			if(MOD &KMOD_ALT && ETYPE == SDL_KEYUP) //alt+enter makes us go fullscreen|windowmode
			{
				cout << "Toggled Fullscreen\n";
				SettingsData.bWindowMode = !SettingsData.bWindowMode;
				//SDL_LockSurface(screen);
				screen = SDL_SetVideoMode(SettingsData.iScreenW,SettingsData.iScreenH,SettingsData.iColourDepth,SDL_HWSURFACE|(SettingsData.bWindowMode?0:SDL_FULLSCREEN));

				//SDL_UnlockSurface(screen);
				SHOW_SCREEN
			}
		}

		if ( SYM == SDLK_ESCAPE )
		{
			cout << "Toggled ESC\n";
		}

		if ( SYM == SDLK_F4 )
		{
			if(MOD &KMOD_ALT) //alt+f4 pressed
			{ //TODO: implement me proper. make game ask for really? and exit no matter where we are!
					cout << "Toggled Alt+F4\n";
					SDL_Event quit;
					quit.type = SDL_QUIT;
					SDL_PushEvent(&quit);
					Quit();
			}
		}
		break;
		}
		//if ( SYM == SDLK_LEFT )
// 		SDL_SaveBMP
		//if ( SYM == SDLK_RIGHT)
	



		
		SDL_Delay ( 1 );
	}
	return 0;
}

// generate SplashScreen
void showSplash()
{
	buffer = LoadPCX("init.pcx", false); //load splash with SDL_HWSURFACE
	if (buffer == NULL)
	{ //TODO: at flag for gamewide handling of SDL_HWSURFACE in case it doesn't work
		cLog::write("Couldn't use hardware acceleration for images", cLog::eLOG_TYPE_ERROR);
		cLog::write("This is currently not supported. Expect M.A.X. to crash!", cLog::eLOG_TYPE_ERROR);
		buffer = LoadPCX("init.pcx", true);
		if (buffer == NULL)
		{
			cLog::write("Couldn't use software acceleration, too", cLog::eLOG_TYPE_ERROR);
			cLog::write("That's it. Tried my best. Bye!", cLog::eLOG_TYPE_ERROR);
			Quit();
		}

	}
	
	SDL_WM_SetIcon ( SDL_LoadBMP ( "MaxIcon.bmp" ), NULL ); //JCK: Icon for frame and taskmanager is set
	screen=SDL_SetVideoMode ( SPLASHWIDTH, SPLASHHEIGHT, SettingsData.iColourDepth, SDL_HWSURFACE|SDL_NOFRAME );
	SDL_BlitSurface ( buffer,NULL,screen,NULL );
	SDL_WM_SetCaption ( MAXVERSION, NULL );
	SDL_UpdateRect ( screen,0,0,0,0 );
	SDL_ShowCursor ( 0 ); // hide cursor during splash
}

void showGameWindow()
{
	SDL_FreeSurface(buffer); //delete splash image 
	buffer=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,SettingsData.iScreenW,SettingsData.iScreenH,SettingsData.iColourDepth,0,0,0,0 );
	
	screen=SDL_SetVideoMode ( buffer->w,buffer ->h,buffer->format->BitsPerPixel,SDL_HWSURFACE|(SettingsData.bWindowMode?0:SDL_FULLSCREEN) );
	
	SDL_FillRect ( buffer,NULL,SDL_MapRGB (buffer->format, 0, 0, 0) );
	SDL_WM_SetCaption ( MAXVERSION, NULL ); //set caption
}

int initSDL()
{
	putenv ( "SDL_VIDEO_WINDOW_POS=center" ); //Set env for SDL - must be done _before_ init_sdl
	putenv ( "SDL_VIDEO_CENTERED=1" );

	if ( SDL_Init ( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE ) == -1 ) // start SDL basics
	{
		cLog::write ( "Could not init SDL",cLog::eLOG_TYPE_ERROR );
		cLog::write ( SDL_GetError(),cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	else
	{
		cLog::write ( "Initalized SDL basics - looks good!",cLog::eLOG_TYPE_INFO ); 
		cLog::mark();
		//made it - enough to start game
		return 0;
	}
}

int initSound()
{
	if (!SettingsData.bSoundEnabled)
	{
		cLog::write ( "Sound disabled due configuration", cLog::eLOG_TYPE_INFO);
		return 1;
	}

	if ( SDL_Init ( SDL_INIT_AUDIO ) == -1 ) //start sound
	{
		cLog::write ( "Could not init SDL_INIT_AUDIO\nSound won't  be avaible!",cLog::eLOG_TYPE_WARNING );
		cLog::write ( SDL_GetError(),cLog::eLOG_TYPE_WARNING );
		SettingsData.bSoundEnabled=false;
		return -1;
	}

	if ( !InitSound ( SettingsData.iFrequency, SettingsData.iChunkSize ) )
        {
                return -1;
        }
	cLog::write ( "Sound started", cLog::eLOG_TYPE_INFO);
	return 0;
}

int initNet()
{
	if ( SDLNet_Init() == -1 ) // start SDL_net
	{
		cLog::write ( "Could not init SDLNet_Init\nNetwork games won' be avaible! ",cLog::eLOG_TYPE_WARNING );
		cLog::write ( SDL_GetError(),cLog::eLOG_TYPE_WARNING );
		return -1;
	}
	cLog::write ( "Net started", cLog::eLOG_TYPE_INFO);
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
	cLog::write ( "EOF" );
	exit ( 0 );
}

// ScaleSurface //////////////////////////////////////////////////////////////
// Skaliert ein Surface in ein Anderes:
void ScaleSurface ( SDL_Surface *scr,SDL_Surface **dest,int size )
{
	int x,y,rx,ry,dx,dy,sizex=size;
	unsigned int *s,*d;

	if ( scr->w>scr->h )
	{
		sizex=scr->w/64*size;
	}
	*dest=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,sizex,size,SettingsData.iColourDepth,0,0,0,0 );
	dx=rx=0;
	dy=0;
	SDL_LockSurface ( *dest );
	SDL_LockSurface ( scr );
	s= ( unsigned int* ) scr->pixels;
	d= ( unsigned int* ) ( *dest )->pixels;
	ry=scr->h;
	for ( y=0;y<scr->h;y++ )
	{
		if ( ry>=scr->h )
		{
			ry-=scr->h;
			dx=0;
			rx=scr->w;
			for ( x=0;x<scr->w;x++ )
			{
				if ( rx>=scr->w )
				{
					rx-=scr->w;
					d[dx+dy*sizex]=s[x+y*scr->w];
					dx++;
				}
				if ( dx>=sizex ) break;
				rx+=sizex;
			}
			dy++;
			if ( dy>=size ) break;
		}
		ry+=size;
	}
	SDL_UnlockSurface ( *dest );
	SDL_UnlockSurface ( scr );
}

void ScaleSurface2 ( SDL_Surface *scr,SDL_Surface *dest,int size )
{
	int x,y,rx,ry,dx,dy,sizex=size;
	unsigned int *s,*d;
	if ( scr->w>scr->h )
	{
		sizex=scr->w/64*size;
	}
	dest->w=scr->w;
	dest->h=scr->h;
	dx=rx=0;
	dy=0;
	SDL_LockSurface ( dest );
	SDL_LockSurface ( scr );
	s= ( unsigned int* ) scr->pixels;
	d= ( unsigned int* ) dest->pixels;
	ry=scr->h;
	for ( y=0;y<scr->h;y++ )
	{
		if ( ry>=scr->h )
		{
			ry-=scr->h;
			dx=0;
			rx=scr->w;
			for ( x=0;x<scr->w;x++ )
			{
				if ( rx>=scr->w )
				{
					rx-=scr->w;
					d[dx+dy*dest->w]=s[x+y*scr->w];
					dx++;
				}
				if ( dx>=sizex ) break;
				rx+=sizex;
			}
			dy++;
			if ( dy>=size ) break;
		}
		ry+=size;
	}
	SDL_UnlockSurface ( dest );
	SDL_UnlockSurface ( scr );
	dest->w=sizex;
	dest->h=size;
}

// ScaleSurfaceAdv ///////////////////////////////////////////////////////////
// Skaliert ein Surface in ein Anderes:
void ScaleSurfaceAdv ( SDL_Surface *scr,SDL_Surface **dest,int sizex,int sizey )
{
	int x,y,rx,ry,dx,dy;
	unsigned int *s,*d;
	*dest=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,sizex,sizey,SettingsData.iColourDepth,0,0,0,0 );
	dx=rx=0;
	dy=0;
	SDL_LockSurface ( *dest );
	SDL_LockSurface ( scr );
	s= ( unsigned int* ) scr->pixels;
	d= ( unsigned int* ) ( *dest )->pixels;
	ry=scr->h;
	for ( y=0;y<scr->h;y++ )
	{
		if ( ry>=scr->h )
		{
			ry-=scr->h;
			dx=0;
			rx=scr->w;
			for ( x=0;x<scr->w;x++ )
			{
				if ( rx>=scr->w )
				{
					rx-=scr->w;
					d[dx+dy*sizex]=s[x+y*scr->w];
					dx++;
				}
				if ( dx>=sizex ) break;
				rx+=sizex;
			}
			dy++;
			if ( dy>=sizey ) break;
		}
		ry+=sizey;
	}
	SDL_UnlockSurface ( *dest );
	SDL_UnlockSurface ( scr );
}

void ScaleSurfaceAdv2 ( SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey )
{
	int x,y,rx,ry,dx,dy;
	unsigned int *s,*d;
	dx=rx=0;
	dy=0;
	dest->w=scr->w;
	dest->h=scr->h;
	SDL_LockSurface ( dest );
	SDL_LockSurface ( scr );
	s= ( unsigned int* ) scr->pixels;
	d= ( unsigned int* ) dest->pixels;
	ry=scr->h;
	for ( y=0;y<scr->h;y++ )
	{
		if ( ry>=scr->h )
		{
			ry-=scr->h;
			dx=0;
			rx=scr->w;
			for ( x=0;x<scr->w;x++ )
			{
				if ( rx>=scr->w )
				{
					rx-=scr->w;
					d[dx+dy*dest->w]=s[x+y*scr->w];
					dx++;
				}
				if ( dx>=sizex ) break;
				rx+=sizex;
			}
			dy++;
			if ( dy>=sizey ) break;
		}
		ry+=sizey;
	}
	SDL_UnlockSurface ( dest );
	SDL_UnlockSurface ( scr );
	dest->w=sizex;
	dest->h=sizey;
}

void ScaleSurfaceAdv2Spec ( SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey )
{
	int x,y,rx,ry,dx,dy;
	unsigned int *s,*d;
	dx=rx=0;
	dy=0;
	dest->w=scr->w;
	dest->h=scr->h;
	SDL_LockSurface ( dest );
	SDL_LockSurface ( scr );
	s= ( unsigned int* ) scr->pixels;
	d= ( unsigned int* ) dest->pixels;
	ry=scr->h;
	for ( y=0;y<scr->h;y++ )
	{
		if ( ry>=scr->h )
		{
			ry-=scr->h;
			dx=0;
			rx=scr->w;
			for ( x=0;x<scr->w;x++ )
			{
				if ( rx>=scr->w )
				{
					unsigned int t,sc,de;
					rx-=scr->w;

					sc=x+y*scr->w;
					de=dx+dy*dest->w;
					t=d[de]=s[sc];

					if ( t==0xFF00FF )
					{
						if ( x>0&&s[sc-1]!=0xFF00FF ) d[de]=s[sc-1];
						else if ( x<scr->w-1&&s[sc+1]!=0xFF00FF ) d[de]=s[sc+1];
					}

					dx++;
				}
				if ( dx>=sizex ) break;
				rx+=sizex;
			}
			dy++;
			if ( dy>=sizey ) break;
		}
		ry+=sizey;
	}
	SDL_UnlockSurface ( dest );
	SDL_UnlockSurface ( scr );
	dest->w=sizex;
	dest->h=sizey;
}

// CreatePfeil ////////////////////////////////////////////////////////////////
// Erzeigt ein Pfeil-Surface:
SDL_Surface *CreatePfeil ( int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size )
{
	SDL_Surface *sf;
	float fak;
	sf=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,size,size,SettingsData.iColourDepth,0,0,0,0 );
	SDL_SetColorKey ( sf,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_FillRect ( sf,NULL,0xFF00FF );
	SDL_LockSurface ( sf );

	fak=size/64.0;
	line ( ( int )Round ( p1x*fak ),( int )Round ( p1y*fak ),( int )Round ( p2x*fak ),( int )Round ( p2y*fak ),color,sf );
	line ( ( int )Round ( p2x*fak ),( int )Round ( p2y*fak ),( int )Round ( p3x*fak ),( int )Round ( p3y*fak ),color,sf );
	line ( ( int )Round ( p3x*fak ),( int )Round ( p3y*fak ),( int )Round ( p1x*fak ),( int )Round ( p1y*fak ),color,sf );

	SDL_UnlockSurface ( sf );
	return sf;
}

// CreatePfeil ////////////////////////////////////////////////////////////////
// Malt eine Linie auf dem Surface (muss vorher gelocked sein):
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

// CreatePfeil ////////////////////////////////////////////////////////////////
// Erzeugt eine Schield-Farbe:
void MakeShieldColor ( SDL_Surface **dest,SDL_Surface *scr )
{
	SDL_Rect r;
	int i;
	*dest=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,64,64,SettingsData.iColourDepth,0,0,0,0 );
	SDL_BlitSurface ( scr,NULL,*dest,NULL );
	SDL_SetColorKey ( *dest,SDL_SRCCOLORKEY,0xFF00FF );
	r.w=64;
	r.h=1;
	r.x=0;
	for ( i=1;i<64;i+=2 )
	{
		r.y=i;
		SDL_FillRect ( *dest,&r,0xFF00FF );
	}
	r.w=1;
	r.h=64;
	r.y=0;
	for ( i=1;i<64;i+=2 )
	{
		r.x=i;
		SDL_FillRect ( *dest,&r,0xFF00FF );
	}
}

// random ////////////////////////////////////////////////////////////////
// returns a random number between 'y' and 'x':
int random ( int x, int y )
{
	return ( ( int ) ( ( ( double ) rand() /RAND_MAX ) * ( ( x-y ) +y ) ) );
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

int min (int a, int b )
{
	if (a > b)
	{
		return b;
	}
	else
	{
		return a;
	}
}
