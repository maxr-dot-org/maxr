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

#include "utility/os.h"

#include "utility/log.h"

#include <filesystem>
#include <iostream>

#ifndef _WIN32
# include <dirent.h>
#endif

#ifdef WIN32
# include <shlobj.h>
#else
# include <sys/stat.h>
# include <unistd.h>
#endif
#include <ctime>

namespace os
{

	//--------------------------------------------------------------------------
	std::vector<std::filesystem::path> getFilesOfDirectory (const std::filesystem::path& directory)
	{
		std::vector<std::filesystem::path> filenames;

		for (auto it = std::filesystem::directory_iterator{directory}; it != std::filesystem::directory_iterator{}; ++it)
		{
			if (!std::filesystem::is_directory (it->path()))
			{
				filenames.push_back (it->path().filename());
			}
		}
		return filenames;
	}

	//--------------------------------------------------------------------------
	std::vector<std::string> getDirectories (const std::filesystem::path& directory)
	{
		std::vector<std::string> files;

		for (auto it = std::filesystem::directory_iterator{directory}; it != std::filesystem::directory_iterator{}; ++it)
		{
			if (std::filesystem::is_directory (it->path()))
			{
				files.push_back (it->path().filename().string());
			}
		}
		return files;
	}

	//--------------------------------------------------------------------------
	std::filesystem::path getHomeDir()
	{
#if WIN32
		// this is where windowsuser should set their %HOME%
		//set home dir
		TCHAR szPath[MAX_PATH];
		SHGetFolderPath (nullptr, CSIDL_PERSONAL, nullptr, 0, szPath);
		return szPath;
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

	//--------------------------------------------------------------------------
	std::filesystem::path getCurrentExeDir()
	{
#if MAC
		return ""; // TODO
#elif WIN32
		//set exe path
		TCHAR szPath[MAX_PATH];
		HMODULE hModule = GetModuleHandle (nullptr);

		GetModuleFileName (hModule, szPath, MAX_PATH);
		const std::filesystem::path exe = szPath;
		return exe.parent_path();
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
				Log.warn ("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?");
				return "";
			}
			else if (iSize >= 255)
			{
				Log.warn ("Can't resolve full path to program since my array is to small and my programmer is to lame to write a buffer for me!");
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
					Log.info ("Path to binary is: " + exePath);
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
							Log.info ("Path to binary is: " + exePath);
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

	//--------------------------------------------------------------------------
	std::string getUserName()
	{
#ifdef WIN32
		return getenv ("USERNAME");
#elif __amigaos4__
		return "AmigaOS4-User";
#else
		return getenv ("USER"); //get $USER on linux
#endif
	}

	//--------------------------------------------------------------------------
	std::string formattedNow (const char* format)
	{
		// TODO: may use std::chrono here.
		//       Problem is that std::put_time is not supported in gcc < 4.8 iirc

		time_t tTime = time (nullptr);
#if defined(_MSC_VER)
		tm tm_;
		tm* tmTime = &tm_;
		localtime_s (tmTime, &tTime);
#else
		// Not thread safe
		tm* tmTime = localtime (&tTime);
#endif
		char timestr[1024];
		strftime (timestr, 1024, format, tmTime);
		return timestr;
	}

} // namespace os
