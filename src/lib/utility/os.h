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

#ifndef utility_osH
#define utility_osH

#include <filesystem>
#include <string>
#include <vector>

namespace os
{

	/**
	* Gets the filenames of all files in the directory
	* @param directory Directory in which to search
	* @return A new list with all filenames
	*/
	std::vector<std::string> getFilesOfDirectory (const std::filesystem::path& directory);
	std::vector<std::string> getDirectories (const std::filesystem::path& directory);

	std::filesystem::path getHomeDir();
	std::filesystem::path getCurrentExeDir();

	std::string getUserName();

	/* Get current time with format (strftime)
	*/
	std::string formattedNow (const char* format);

} // namespace os

#endif // utility_osH
