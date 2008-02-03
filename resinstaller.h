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
#include <iostream>

using namespace std;


#ifdef _WIN32
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif


EX string sMAXPath;
EX string sPalettePath;
EX string sOutputPath;
EX SDL_RWops *res ZERO;
EX SDL_RWops *logFile ZERO;


EX Uint32 lPosBegin;
EX Uint32 lEndOfFile;

EX int iErrors, iInstalledFiles, iTotalFiles;
EX bool wasError;



//this exception is thrown, when the installation of the current file failed
class InstallException
{
public:
	string message;
	InstallException( string m ) { message = m; };
};


//makes all nessesary aktions after a succsessfull 
//or unsuccessfull attempt to install a file
#define END_INSTALL_FILE( file )\
			catch ( InstallException e )\
			{\
				writeLog("Error while installing file '" + file + "'" + TEXT_FILE_LF + e.message );\
				iErrors++;\
				wasError = true;\
			}\
			iInstalledFiles++;\
			updateProgressbar();
		



#endif // ResinstallerH
