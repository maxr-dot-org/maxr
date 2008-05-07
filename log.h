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
#ifndef LOG_H
#define LOG_H

#include <iostream>

#include "defines.h"

#define LOG_TYPE_WARNING 1
#define LOG_TYPE_ERROR 2
#define LOG_TYPE_DEBUG 3
#define LOG_TYPE_INFO 4
#define LOG_TYPE_MEM 5
#define LOG_TYPE_NET_DEBUG 6
#define LOG_TYPE_NET_WARNING 7
#define LOG_TYPE_NET_ERROR 8

/**
* Log class. Simple log class :-)
*
* @author Bernd "beko" Kosmahl
*/
class cLog
{
private:

	/**
	* Writes message finally to logfile
	*/
	static int writeMessage( char *);
	static int writeMessage( std::string );

	/**
	* Closes the logfile.
	*/
	static void close();

	/**
	* Opens the Logfile. 
	*
	* @return true on success
	*/
	static bool open(int TYPE);
public:
	/**
	* Writes message with given type to logfile
	*
	* @param str Message for the log
	* @param TYPE Type for the log.<br>
	* 1 		== warning 	(WW):<br>
	* 2 		== error	(EE):<br>
	* 3		== debug	(DD):<br>
	* 4		== information	(II):<br>
	* 5		== memory prob. (MM):<br>
	* else		== information	(II):
	*
	* @return 0 on success
	*/
	static int write( const char *str , int TYPE );
	static int write( std::string str , int TYPE );

	/**
	* Writes message with default type (II) to the logfile
	*
	* @param str Message for the log
	*
	* @return 0 on success
	*/
	static int write( const char *str );
	static int write( std::string str );

	enum LOG_TYPE
	{
		eLOG_TYPE_UNKNOWN = 0,
		eLOG_TYPE_WARNING = 1,
		eLOG_TYPE_ERROR   = 2,
		eLOG_TYPE_DEBUG   = 3,
		eLOG_TYPE_INFO    = 4,
		eLOG_TYPE_MEM	  = 5,
		eLOG_TYPE_NET_DEBUG = 6,
		eLOG_TYPE_NET_WARNING = 7,
		eLOG_TYPE_NET_ERROR = 8,
	};

	/**
	* Writes a marker into logfile - please use only veeeery few times!
	*/
	static void mark();
};
#endif
