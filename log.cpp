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

#define LOGFILE "max.log"
/** errors */
#define EE "(EE): "
/** warnings */
#define WW "(WW): "
/** informations */
#define II "(II): "
/** debuginformations */
#define DD "(DD): "

static SDL_RWops *logfile = NULL;

bool cLog::open()
{
	if (logfile != NULL) 
	{
		logfile = SDL_RWFromFile ( LOGFILE,"a+t" ); //Reopen log and write at end of file
	} 
	else //Log not yet started (should only happen at game start)
	{
		logfile = SDL_RWFromFile ( LOGFILE,"w+t" ); //Start new log and write at beginning of file
	}

	int blocks; //sanity check - is file readable?
	char buf[256];
	blocks=SDL_RWread(logfile,buf,16,256/16);
	if(blocks<0)
	{
		fprintf(stderr,"(EE): Couldn't open max.log!\n Please check file/directory permissions\n");

		if ( logfile != NULL ) return true;
		else return false;
	} 
	else return true;
	
}

int cLog::write ( const char *str, int TYPE )
{
	if (open())
	{
		char tmp[264] = "(XX): "; //placeholder
		if (strlen ( str ) > 264 - 7) //message max is 256chars
		{ 
			std::string str = "(EE): sLog recieved to long log message!";
			str += TEXT_FILE_LF;
			str += "(EE): Message had more than 256 chars! That should not happen!";
			str += TEXT_FILE_LF;
			return writeMessage( str );
		}
		else 
		{
			switch ( TYPE ) //Attach log message type to tmp
			{
				case LOG_TYPE_WARNING : strcpy(tmp, WW); break;
				case LOG_TYPE_ERROR : strcpy(tmp, EE); break;
				case LOG_TYPE_DEBUG : strcpy(tmp, DD); break;
				case LOG_TYPE_INFO : strcpy(tmp, II); break;
				default : strcpy(tmp, II);
			}
		}
		strcat(tmp, str);
		return writeMessage ( strcat(tmp, TEXT_FILE_LF ) ); //add log message itself to tmp and send it for writing
	}
	else return -1;
}

int cLog::write ( std::string str, int TYPE )
{

	if (open())
	{
		switch ( TYPE ) //Attach log message type to tmp
		{
			case LOG_TYPE_WARNING : str = str.insert( 0 , WW ); break;
			case LOG_TYPE_ERROR :   str = str.insert( 0 , EE ); break;
			case LOG_TYPE_DEBUG :   str = str.insert( 0 , DD ); break;
			case LOG_TYPE_INFO :    str = str.insert( 0 , II ); break;
			default :				str = str.insert( 0 , II );
		}
		str += TEXT_FILE_LF;
		return writeMessage ( str ); //add log message itself to tmp and send it for writing
	}
	else return -1;
}

int cLog::write (const char *str )
{
	return write ( str, LOG_TYPE_INFO );
}

void cLog::mark()
{
	std::string str = "==============================(MARK)==============================";
	str += TEXT_FILE_LF;
	if(open()) writeMessage( str );
}

int cLog::writeMessage ( char *str )
{
	std::size_t wrote;
	wrote = SDL_RWwrite ( logfile,str,1, (int)strlen ( str ) );

	if ( wrote<0 ) //sanity check - was file writable?
	{
		fprintf ( stderr,"Couldn't write to max.log\nPlease check permissions for max.log\nLog message was:\n%s", str );
		return -1;
	}
	else close(); //after successful writing of all information we close log here and nowhere else!
	return 0;
}

int cLog::writeMessage ( std::string str )
{
	std::size_t wrote;
	wrote = SDL_RWwrite ( logfile , str.c_str() , 1 , (int) str.length() );

	if ( wrote<0 ) //sanity check - was file writable?
	{
		fprintf ( stderr,"Couldn't write to max.log\nPlease check permissions for max.log\nLog message was:\n%s", str.c_str() );
		return -1;
	}
	else close(); //after successful writing of all information we close log here and nowhere else!
	return 0;
}


void cLog::close()
{
	SDL_RWclose ( logfile ); //function RWclose always returns 0 in SDL <= 1.2.9 - no sanity check possible
}
