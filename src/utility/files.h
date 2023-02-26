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

#ifndef utility_filesH
#define utility_filesH

#include <config/workaround/cpp17/filesystem.h>
#include <string>
#include <vector>

/**
* Gets the filenames of all files in the directory
* @author alzi
* @param directory Directory in which to search
* @return A new list with all filenames
*/
std::vector<std::string> getFilesOfDirectory (const std::filesystem::path& directory);

/**
* Gets the map folder of the user's custom maps.
* @author pagra
* @return an absolute path to the user's maps directory or empty string, if no user maps folder is defined on the system
*/
std::filesystem::path getUserMapsDir();

/**
 * Gets the folder, where screenshots made by the user should be saved.
 * @author pagra
 * @return an absolute path to the user's screenshots directory or empty string, if no user screenshots folder is defined on the system
 */
std::filesystem::path getUserScreenshotsDir();

std::filesystem::path getUserLogDir();
std::string getHomeDir();
std::string getCurrentExeDir();

#endif // utility_filesH
