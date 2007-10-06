///////////////////////////////////////////////////////////////////////////////
//
// M.A.X. Reloaded - main.h
//
///////////////////////////////////////////////////////////////////////////////
//
// Main-declerations, -classes and structures for the game
// Contains all global varaibles needed for the game
//
///////////////////////////////////////////////////////////////////////////////

#include "loaddata.h"
#include "fonts.h"

// LoadData ///////////////////////////////////////////////////////////////////
// Loads alle relevant files and datas
int LoadData ( void * )
{
	LoadingData=true;
	for( int i = 0; i < 1000; i++)
	{
		SDL_Delay(10);
	}
	LoadingData=false;
	return 1;
}

// MakeLog ///////////////////////////////////////////////////////////////////
// Schreibt eine Nachricht auf dem SplashScreen:
void MakeLog ( char* sztxt,bool ok,int pos )
{
	if ( !ok )
		fonts->OutTextBig ( sztxt,22,152+16*pos,buffer );
	else
		fonts->OutTextBig ( "OK",250,152+16*pos,buffer );
	SDL_BlitSurface ( buffer,NULL,screen,NULL );
	SDL_UpdateRect ( screen,0,0,0,0 );
}