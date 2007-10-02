/***************************************************************************
 *   Copyright (C) 2007 by Bernd 'beko' Kosmahl   *
 *   beko@dmaxthegame.de   *
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

#include <iostream>
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

const char *ee="(EE): "; //errors
const char *ww="(WW): "; //warnings
const char *ii="(II): "; //informations
const char *dd="(DD): "; //debug

static SDL_RWops *logfile;

cLog::cLog()
{}

int cLog::init ( )
{
	logfile = SDL_RWFromFile ( "max.log","w+t" );
	if ( logfile==NULL )
	{
		fprintf ( stderr,"Couldn't open max.log\n" );
		return ( 1 );
	}
	return 0;
}

//Send str to logfile and add error/warning tag
//TYPEs:
// 1 		== warning 	(WW):
// 2 		== error	(EE):
// 3		== debug	(DD):
// else		== information	(II):
int cLog::write ( char *str, int TYPE )
{
	switch(TYPE)
	{
		case LOG_TYPE_WARNING : SDL_RWwrite ( logfile,ww,1,6 ); break;
		case LOG_TYPE_ERROR : SDL_RWwrite ( logfile,ee,1,6 ); break;
		case LOG_TYPE_DEBUG : SDL_RWwrite ( logfile,dd,1,6 ); break;
		default : SDL_RWwrite ( logfile,ii,1,6 );
	
	}
	return writeMessage( str );
}

// noTYPE	== information 	(II):
int cLog::write ( char *str )
{
	SDL_RWwrite ( logfile,ii,1,6 );
	return writeMessage( str );
}

int cLog::writeMessage( char *str )
{
	int wrote;
	wrote=SDL_RWwrite ( logfile,str,1,strlen ( str ) );

	if ( wrote<0 )
	{
		fprintf ( stderr,"Couldn't write to max.log\n" );
		return ( 2 );
	}

	//fprintf ( stderr,"Wrote %d 1-byte blocks\n",wrote );
	return ( 0 );
}

//Am Programmende oder bei Programmabbruch ausführen
int cLog::close()
{
	//Die Funktion SDL_RWclose liefert aktuell immer 0 zurück, auf Fehler wird intern noch nicht geprüft (SDL 1.2.9).
	return SDL_RWclose ( logfile );
}
