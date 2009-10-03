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

#include <string>
#include "defines.h"
#include "clist.h"

#ifndef PATH_DELIMITER
#	define PATH_DELIMITER "/"
#endif


/**
* Checks whether a file exists or not
* @author beko
* @param path Filename to check for
* @return true if exists (as in readable)
* @return false if does not exist (as in not readable)
*/
bool FileExists(const char* path);

/**
* Gets the filenames of all files in the directory
* @author alzi
* @param sDirectory Directory in which to search
* @return A new list with all filenames (the caller is owner of the list)
*/
cList<std::string> *getFilesOfDirectory(std::string sDirectory);

/**
* Gets the map folder of the user's custom maps.
* @author pagra
* @return an absolute path to the user's maps directory or empty string, if no user maps folder is defined on the system
*/
std::string getUserMapsDir();

/**
 * Gets the folder, where screenshots made by the user should be saved.
 * @author pagra
 * @return an absolute path to the user's screenshots directory or empty string, if no user screenshots folder is defined on the system
 */
std::string getUserScreenshotsDir();

/**
* @author pagra
* @return a checksum of all bytes in the given data chunk
*/
Sint32 calcCheckSum(const char* data, size_t dataSize);

#endif
