/***************************************************************************
 *   Copyright (C) 2007 by Bernd Kosmahl   *
 *   beko@duke.famkos.net   *
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

#include <stdio.h>
#include <string.h>

#include "SDL_rwops.h"
#include "log.h"

/*
mode kann folgende Werte annehmen:
r  Datei zum Lesen öffnen, die Datei muss existieren.
w  Datei zum Schreiben öffnen, die Datei muss existieren.
a  Hinzufügen zu der Datei, Daten werden am Ende der Datei hinzugefügt.
r+  Datei zum Lesen und Schreiben öffnen, die Datei muss existieren.
w+  Öffnet eine leere Datei zum Lesen und Schreiben. Wenn eine Datei mit dem Namen existiert, wird sie überschrieben.
a+  Öffnet eine Datei zum Lesen und Hinzufügen. Alle Schreiboperationen geschehen am Ende der Datei.

Zusätzlich kann noch einer der folgenden Buchstaben zu mode hinzugefügt werden.
t  Textmodus: Das Ende der Datei ist das STRG+Z-Zeichen.
b  Binäre Modus: Das Ende der Datei ist am letzen Byte der Datei erreicht.*/

char *ee="(EE): "; //errors
char *ww="(WW): "; //warnings
char *ii="(II): "; //informations
//TODO: DEBUG (DD):

SDL_RWops *logfile=SDL_RWFromFile ( "max.log","w+t" );

Log::Log()
{}

//Send str to logfile and add error/warning tag
//TYPEs:
// 1 		== warning 	(WW):
// 2 		== error	(EE):
// else		== information	(II):
int Log::write ( char *str, int TYPE )
{
	if ( logfile==NULL )
	{
		fprintf ( stderr,"Couldn't open max.log\n" );
		return ( 1 );
	}
	
	if ( TYPE == 1 )
	{
		SDL_RWwrite ( logfile,ww,1,6 );
	}
	else if ( TYPE == 2 )
	{
		SDL_RWwrite ( logfile,ee,1,6 );
	}
	else
	{
		SDL_RWwrite ( logfile,ii,1,6 );
	}
	return writeMessage( str );

}

// noTYPE	== information 	(II):
int Log::write ( char *str )
{
	SDL_RWwrite ( logfile,ii,1,6 );
	return writeMessage( str );
}

int Log::writeMessage( char *str )
{
	int wrote;
	wrote=SDL_RWwrite ( logfile,str,1,strlen ( str ) );

	if ( wrote<0 )
	{
		fprintf ( stderr,"Couldn't write to max.log\n" );
		return ( 2 );
	}

	fprintf ( stderr,"Wrote %d 1-byte blocks\n",wrote );
	return ( 0 );
}

//Am Programmende oder bei Programmabbruch ausführen
int Log::close()
{
	//Die Funktion SDL_RWclose liefert aktuell immer 0 zurück, auf Fehler wird intern noch nicht geprüft (SDL 1.2.9).
	SDL_RWclose ( logfile );

}
