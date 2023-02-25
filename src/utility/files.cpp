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

#include "defines.h"
#include "settings.h"
#include "utility/log.h"

#include <SDL.h>
#include <SDL_endian.h>
#include <iostream>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#else
# include <dirent.h>
#endif

#ifdef WIN32
# include <direct.h>
# include <shlobj.h>
# include <sys/stat.h>
# include <sys/types.h>
#else
# include <sys/stat.h>
# include <unistd.h>
#endif

//--------------------------------------------------------------
/** @return exists a file at path */
//------------------------------------------------------------------------------
bool FileExists (const std::string& path)
{
	SDL_RWops* file = SDL_RWFromFile (path.c_str(), "r");

	if (file == nullptr)
	{
		return false;
	}
	SDL_RWclose (file);
	return true;
}

//------------------------------------------------------------------------------
bool makeDir (const std::string& path)
{
#ifdef WIN32
	return _mkdir (path.c_str()) == 0;
#else
	return mkdir (path.c_str(), 0755) == 0;
#endif
}

//------------------------------------------------------------------------------
// std::make_directories is C++17
void makeDirectories (const std::string& path)
{
	std::size_t prev_pos = 0;

	while (prev_pos != std::string::npos)
	{
		const auto pos = path.find_first_of ("/\\", prev_pos);
		const auto subPath = path.substr (0, pos);
		if (!DirExists (subPath))
		{
			makeDir (subPath);
		}
		prev_pos = pos == std::string::npos ? pos : pos + 1;
	}
}

//------------------------------------------------------------------------------
bool DirExists (const std::string& path)
{
#ifdef WIN32
	if (_access (path.c_str(), 0) == 0)
	{
		struct stat status;
		stat (path.c_str(), &status);

		if (status.st_mode & S_IFDIR)
			return true;
		else
			return false; // The path is not a directory
	}
	else
		return false;
#else
	return FileExists (path); // on linux everything is a file
#endif
}

//------------------------------------------------------------------------------
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
			if (DataFile.name[0] == '.') continue;
			List.push_back (DataFile.name);
		} while (_findnext (lFile, &DataFile) == 0);
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

//------------------------------------------------------------------------------
std::string getUserMapsDir()
{
#ifdef __amigaos4__
	return "";
#else
	if (cSettings::getInstance().getHomeDir().empty()) return "";
	std::string mapFolder = cSettings::getInstance().getHomeDir() + "maps";
	if (!DirExists (mapFolder))
	{
		if (makeDir (mapFolder))
			return mapFolder + PATH_DELIMITER;
		return "";
	}
	return mapFolder + PATH_DELIMITER;
#endif
}

//------------------------------------------------------------------------------
std::string getUserScreenshotsDir()
{
#ifdef __amigaos4__
	return "";
#elif defined(MAC)
	char* cHome = getenv ("HOME"); //get $HOME on mac
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
		if (makeDir (screenshotsFolder))
			return screenshotsFolder + PATH_DELIMITER;
		return "";
	}
	return screenshotsFolder + PATH_DELIMITER;
#endif
}

//------------------------------------------------------------------------------
std::string getHomeDir()
{
#if WIN32
	// this is where windowsuser should set their %HOME%
	//set home dir
	TCHAR szPath[MAX_PATH];
	SHGetFolderPath (nullptr, CSIDL_PERSONAL, nullptr, 0, szPath);
# ifdef UNICODE
	std::wstring home = szPath;
# else
	std::string home = szPath;
# endif
	return std::string (home.begin(), home.end());
#elif __amigaos4__
	return "";
#elif MAC
	char* cHome = getenv ("HOME"); //get $HOME on mac
	return cHome != nullptr ? cHome : "";
#else
	char* cHome = getenv ("HOME"); // get $HOME on linux
	return cHome != nullptr ? cHome : "";
#endif
}

//------------------------------------------------------------------------------
std::string getCurrentExeDir()
{
#if MAC
	return ""; // TODO
#elif WIN32
	//set exe path
	TCHAR szPath[MAX_PATH];
	HMODULE hModule = GetModuleHandle (nullptr);

	GetModuleFileName (hModule, szPath, MAX_PATH);
# ifdef UNICODE
	std::wstring exe = szPath;
	const auto backslashString = L"\\";
# else
	std::string exe = szPath;
	const auto backslashString = "\\";
# endif
	exe.erase (exe.rfind (backslashString), std::string::npos);
	return std::string (exe.begin(), exe.end());
#elif __amigaos4__
	return "";
#else
	// determine full path to application
	// this needs /proc support that should be available
	// on most linux installations
	if (FileExists ("/proc/self/exe"))
	{
		char cPathToExe[255];
		int iSize = readlink ("/proc/self/exe", cPathToExe, sizeof (cPathToExe));
		if (iSize < 0)
		{
			Log.write ("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?", cLog::eLogType::Warning);
			return "";
		}
		else if (iSize >= 255)
		{
			Log.write ("Can't resolve full path to program since my array is to small and my programmer is to lame to write a buffer for me!", cLog::eLogType::Warning);
			return "";
		}
		else
		{
			int iPos = 0;
			for (int i = 0; i < 255 && cPathToExe[i] != '\0'; i++)
			{
				// snip garbage after last PATH_DELIMITER + executable itself
				// (is reported on some linux systems
				//  as well using /proc/self/exe
				if (cPathToExe[i] == '/')
					iPos = i;
			}

			std::string exePath = cPathToExe;
			exePath = exePath.substr (0, iPos);
			exePath += PATH_DELIMITER;

			// check for binary itself in bin folder
			if (FileExists (exePath + "maxr"))
			{
				Log.write ("Path to binary is: " + exePath, cLog::eLogType::Info);
			}
			else
			{
				// perhaps we got ourself a trailing maxr
				// in the path like /proc
				// seems to do it sometimes. remove it and try again!
				if (cPathToExe[iPos - 1] == 'r' && cPathToExe[iPos - 2] == 'x' && cPathToExe[iPos - 3] == 'a' && cPathToExe[iPos - 4] == 'm')
				{
					exePath = exePath.substr (0, iPos - 5);
					if (FileExists (exePath + "maxr"))
					{
						Log.write ("Path to binary is: " + exePath, cLog::eLogType::Info);
					}
				}
			}
			return exePath;
		}
	}
	else
	{
		std::cerr << "Can't resolve full path to program. Doesn't this system feature /proc/self/exe?";
		return "";
	}
#endif
}

//------------------------------------------------------------------------------
std::string getUserLogDir()
{
#ifdef __amigaos4__
	return "";
#elif defined(MAC)
	char* cHome = getenv ("HOME"); //get $HOME on mac
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
	std::string LogDir = cSettings::getInstance().getHomeDir() + "log_files";

	if (!DirExists (LogDir))
	{
		if (makeDir (LogDir))
			return LogDir + PATH_DELIMITER;
		return "";
	}
	return LogDir + PATH_DELIMITER;
#endif
}

//------------------------------------------------------------------------------
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
	std::vector<unsigned char> buffer (size);

	SDL_RWseek (sourceFile, 0, SEEK_SET);
	SDL_RWread (sourceFile, buffer.data(), 1, size);

	SDL_RWwrite (destFile, buffer.data(), 1, size);

	buffer.clear();

	if (sourceFile) SDL_RWclose (sourceFile);
	if (destFile) SDL_RWclose (destFile);
}
