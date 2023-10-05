/***************************************************************************
 *              Resinstaller - installs missing GFX for MAXR               *
 *              This file is part of the resinstaller project              *
 *   Copyright (C) 2007, 2008 Eiko Oltmanns                                *
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

#ifndef ResinstallerH
#define ResinstallerH
#include "defines.h"

#include <SDL.h>
#include <filesystem>
#include <iostream>
#include <locale>
#include <string>

#ifdef WIN32
# include <conio.h>
# include <windows.h>
#endif

extern std::filesystem::path sOutputPath;
extern SDL_RWops* res;
extern SDL_RWops* logFile;

extern Uint32 lPosBegin;
extern Uint32 lEndOfFile;

extern int iErrors;
extern int iInstalledFiles;
extern int iTotalFiles;
extern bool wasError;

extern bool oggEncode;

//this exception is thrown, when the installation of the current file failed
class InstallException
{
public:
	std::string message;
	InstallException (std::string m) { message = m; };
};

void trimSpaces (std::string& str, const std::locale& loc = std::locale());

void trimQuotes (std::string& str);

// makes all necessary actions after a successful
// or unsuccessful attempt to install a file
#define END_INSTALL_FILE(file) \
 catch (const InstallException& e) \
 { \
  writeLog ("Error while installing file '" + std::filesystem::path (file).u8string() + "'" + TEXT_FILE_LF + e.message); \
  iErrors++; \
  wasError = true; \
 } \
 catch (const std::exception& e) \
 { \
  writeLog ("Error while installing file '" + std::filesystem::path (file).u8string() + "'" + TEXT_FILE_LF + e.what() + TEXT_FILE_LF); \
  iErrors++; \
  wasError = true; \
 } \
 iInstalledFiles++; \
 updateProgressbar();

#endif // ResinstallerH
