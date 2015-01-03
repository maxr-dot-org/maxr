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

#include "utility/files.h"

#include <iostream>
#include <SDL.h>
#include <SDL_endian.h>

#include "utility/log.h"
#include "settings.h"

#ifdef _WIN32
# include <io.h>
# include <direct.h>
#else
# include <dirent.h>
#endif

#ifdef WIN32
# include <sys/types.h>
# include <sys/stat.h>
#else
# include <sys/stat.h>
# include <unistd.h>
#endif


//--------------------------------------------------------------
/** @return exists a file at path */
//--------------------------------------------------------------
bool FileExists (const char* path)
{
	SDL_RWops* file = SDL_RWFromFile (path, "r");

	if (file == nullptr)
	{
		if(Log.isInitialized())
		{
			Log.write (SDL_GetError(), cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			std::cout << SDL_GetError();
		}
		return false;
	}
	SDL_RWclose (file);
	return true;
}

//--------------------------------------------------------------
bool makeDir (const std::string& path)
{
#ifdef WIN32
	return _mkdir (path.c_str()) == 0;
#else
	return mkdir (path.c_str(), 0755) == 0;
#endif
}

//--------------------------------------------------------------
bool DirExists (const std::string& path)
{
#ifdef WIN32
	if (_access (path.c_str(), 0) == 0)
	{
		struct stat status;
		stat (path.c_str(), &status);

		if (status.st_mode & S_IFDIR) return true;
		else return false; // The path is not a directory
	}
	else return false;
#else
	return FileExists (path.c_str()); // on linux everything is a file
#endif
}

//--------------------------------------------------------------
std::vector<std::string> getFilesOfDirectory (const std::string& sDirectory)
{
	std::vector<std::string> List;
#ifdef _WIN32
	_finddata_t DataFile;
	intptr_t const lFile = _findfirst ((sDirectory + PATH_DELIMITER "*.*").c_str(), &DataFile);
	if (lFile != -1)
	{
		do
		{
			if (DataFile.attrib & _A_SUBDIR) continue;
			if (DataFile.name[0] == '.')     continue;
			List.push_back (DataFile.name);
		}
		while (_findnext (lFile, &DataFile) == 0);
		_findclose (lFile);
	}
#else
	if (DIR* const dir = opendir (sDirectory.c_str()))
	{
		while (struct dirent* const entry = readdir (dir))
		{
			char const* const name = entry->d_name;
			if (name[0] == '.') continue;
			List.push_back (name);
		}
		closedir (dir);
	}
#endif
	return List;
}

//--------------------------------------------------------------
std::string getUserMapsDir()
{
#ifdef __amigaos4__
	return "";
#else
	if (cSettings::getInstance().getHomeDir().empty()) return "";
	std::string mapFolder = cSettings::getInstance().getHomeDir() + "maps";
	if (!DirExists (mapFolder.c_str()))
	{
		if (makeDir (mapFolder.c_str()))
			return mapFolder + PATH_DELIMITER;
		return "";
	}
	return mapFolder + PATH_DELIMITER;
#endif
}

//--------------------------------------------------------------
std::string getUserScreenshotsDir()
{
#ifdef __amigaos4__
	return "";
#elif defined(MAC)
	char* cHome = getenv ("HOME");  //get $HOME on mac
	if (cHome == nullptr)
		return "";
	std::string homeFolder = cHome;
	if (homeFolder.empty())
		return "";
	// store screenshots directly on the desktop of the user
	return homeFolder + PATH_DELIMITER "Desktop" PATH_DELIMITER;
#else
	if (cSettings::getInstance().getHomeDir().empty())
		return "";
	std::string screenshotsFolder = cSettings::getInstance().getHomeDir() + "screenies";
	if (!DirExists (screenshotsFolder))
	{
		if (makeDir (screenshotsFolder.c_str()))
			return screenshotsFolder + PATH_DELIMITER;
		return "";
	}
	return screenshotsFolder + PATH_DELIMITER;
#endif
}

//--------------------------------------------------------------
std::string getUserLogDir()
{
#ifdef __amigaos4__
	return "";
#elif defined(MAC)
	char* cHome = getenv ("HOME");  //get $HOME on mac
	if (cHome == nullptr)
		return "";
	std::string homeFolder = cHome;
	if (homeFolder.empty())
		return "";
	// store Log directly on the desktop of the user
	return homeFolder + PATH_DELIMITER "Desktop" PATH_DELIMITER;
#else
	if (cSettings::getInstance().getHomeDir().empty())
		return "";
	std::string LogDir = cSettings::getInstance().getHomeDir() + MAX_LOG_DIR;

	if (!DirExists (LogDir))
	{
		if (makeDir (LogDir.c_str()))
			return LogDir + PATH_DELIMITER;
		return "";
	}
	return LogDir + PATH_DELIMITER;
#endif
}

//--------------------------------------------------------------
uint32_t calcCheckSum (uint32_t data, uint32_t checksum)
{
	data = SDL_SwapLE32 (data);// The calculation must be endian safe.
	return calcCheckSum (reinterpret_cast<char*> (&data), 4, checksum);
}

uint32_t calcCheckSum (const char* data, size_t dataSize, uint32_t checksum)
{
	for (const char* i = data; i != data + dataSize; ++i)
	{
		checksum = checksum << 1 | checksum >> 31; // Rotate left by one.
		checksum += *i;
	}
	return checksum;
}

void copyFile (const std::string& source, const std::string& dest)
{
	SDL_RWops* sourceFile = SDL_RWFromFile (source.c_str(), "rb");
	SDL_RWops* destFile = SDL_RWFromFile (dest.c_str(), "wb");
	if (destFile == nullptr)
	{
		return;
	}

	SDL_RWseek (sourceFile, 0, SEEK_END);
	const long int size = SDL_RWtell (sourceFile);
	unsigned char* buffer = new unsigned char [size];
	if (buffer == nullptr)
	{
		std::cout << "Out of memory\n";
		exit (-1);
	}

	SDL_RWseek (sourceFile, 0, SEEK_SET);
	SDL_RWread (sourceFile, buffer, 1, size);

	SDL_RWwrite (destFile, buffer, 1, size);

	delete [] buffer;

	if (sourceFile) SDL_RWclose (sourceFile);
	if (destFile)   SDL_RWclose (destFile);
}
