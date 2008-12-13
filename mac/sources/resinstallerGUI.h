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

// author: Paul Grathwohl

#include <string>

std::string askForCDPath (bool (*validationFunction)(std::string&));
std::string askForOutputPath (bool (*validationFunction)(std::string&));
int askForLanguage (bool germanAvailable, bool italianAvailable, bool frenchAvailable);

void updateProgressWindow (std::string taskDescription, int totalFiles, int installedFiles);
void displayProgressWindow (bool& finishedInstalling);

void showIntroductionMAC (std::string strText);
void showMessage (std::string msgText, std::string informativeText);
