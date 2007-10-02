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

#define LOG_TYPE_WARNING 1
#define LOG_TYPE_ERROR 2
#define LOG_TYPE_DEBUG 3

/**
* Log class. Simple log class :-)
*
* @author Bernd "beko" Kosmahl
*/
class cLog
{
private:

	int writeMessage( char *);
public:
	cLog();
	/**
	* Init the Logfile. This should be done only once 
	* at the beginning of the app
	*
	* @return 0 on success
	*/
	int init();

	/**
	* Writes message to the logfile
	*
	* @param str Message for the log
	* @param TYPE Type for the log.<br>
	* 1 		== warning 	(WW):<br>
	* 2 		== error	(EE):<br>
	* 3		== debug	(DD):<br>
	* else		== information	(II):
	*
	* @return 0 on success
	*/
	int write(char *str, int TYPE);

	/**
	* Writes message with default type (II) to the logfile
	*
	* @param str Message for the log
	*
	* @return 0 on success
	*/
	int write(char *str);

	/**
	* Closes the logfile. This should only be done on
	* application exit (end or error.)
	*
	* @return 0 SDL <= 1.2.9 always returns 0
	*/
	int close();
};
