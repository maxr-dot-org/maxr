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

#include "utility/log.h"

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

//------------------------------------------------------------------------------
std::vector<std::string> getFilesOfDirectory (const std::filesystem::path& directory)
{
	std::vector<std::string> files;
#ifdef _WIN32
	_finddata_t DataFile;
	intptr_t const lFile = _findfirst ((directory / "*.*").string().c_str(), &DataFile);
	if (lFile != -1)
	{
		do
		{
			if (DataFile.attrib & _A_SUBDIR) continue;
			if (DataFile.name[0] == '.') continue;
			files.push_back (DataFile.name);
		} while (_findnext (lFile, &DataFile) == 0);
		_findclose (lFile);
	}
#else
	if (DIR* const dir = opendir (directory.string().c_str()))
	{
		while (struct dirent* const entry = readdir (dir))
		{
			char const* const name = entry->d_name;
			if (name[0] == '.') continue;
			files.push_back (name);
		}
		closedir (dir);
	}
#endif
	return files;
}

//------------------------------------------------------------------------------
std::filesystem::path getHomeDir()
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
	return home;
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
std::filesystem::path getCurrentExeDir()
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
	return exe;
#elif __amigaos4__
	return "";
#else
	// determine full path to application
	// this needs /proc support that should be available
	// on most linux installations
	if (std::filesystem::exists ("/proc/self/exe"))
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
				// snip garbage after last '/' + executable itself
				// (is reported on some linux systems
				//  as well using /proc/self/exe
				if (cPathToExe[i] == '/')
					iPos = i;
			}

			std::string exePath = cPathToExe;
			exePath = exePath.substr (0, iPos);
			exePath += "/";

			// check for binary itself in bin folder
			if (std::filesystem::exists (exePath + "maxr"))
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
					if (std::filesystem::exists (exePath + "maxr"))
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
