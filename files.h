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
#ifndef filesH
#define filesH
#include <SDL.h>
#include "defines.h"
#include "main.h"

#ifndef PATH_DELIMITER
#	define PATH_DELIMITER "/"
#endif


/**
* Checks whether a file exists or not
* @author beko
* @param name Filename to check for
* @return true if exists (as in readable)
* @return false if does not exist (as in not readable)
*/
bool FileExists(const char* name);

/**
* Checks whether a file exits
* @author alzi
* @param directory Directory of the file
* @param filename Name of the file
* @return 1 on success
*/
int CheckFile(const char* directory, const char* filename);

/**
* Gets the filenames of all files in the directory
* @author alzi
* @param sDirectory Directory in which to search
* @return A List with all filenames
*/
cList<string> *getFilesOfDirectory(string sDirectory);
#endif
