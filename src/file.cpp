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

#include "file.h"

#include "converter.h"
#include "resinstaller.h"

#include <string>

namespace
{
	std::string toLower (std::string s)
	{
		for (char& c : s)
		{
			c = tolower (c);
		}
		return s;
	}

	std::string toUpper (std::string s)
	{
		for (char& c : s)
		{
			c = toupper (c);
		}
		return s;
	}

} // namespace

//first tries to open a file with lowercase name
//if this fails, tries to open the file with uppercase name
SDL_RWops* openFile (const std::filesystem::path& path, const char* mode)
{
	const std::string fileName = toUpper (path.filename().string());
	const std::string lowerCaseFileName = toLower (fileName);
	const auto dir = path.parent_path();

	//try to open with lower case file name
	SDL_RWops* file = SDL_RWFromFile ((dir / fileName).string().c_str(), mode);
	if (file != nullptr)
	{
		return file;
	}

	//try to open with upper case file name
	file = SDL_RWFromFile ((path / fileName).string().c_str(), mode);
	if (file != nullptr)
	{
		return file;
	}

	throw InstallException ("Couldn't open file '" + (dir / fileName).u8string() + "' or '" + lowerCaseFileName + "'" + TEXT_FILE_LF);
	return nullptr;
}

void copyFile (const std::filesystem::path& source, const std::filesystem::path& dest)
{
	try
	{
		auto lower_path = source.parent_path() / toLower (source.filename().string());
		auto upper_path = source.parent_path() / toUpper (source.filename().string());

		std::filesystem::create_directories(dest.parent_path());
		if (std::filesystem::exists (lower_path))
		{
			std::filesystem::copy_file (lower_path, dest, std::filesystem::copy_options::skip_existing);
		}
		else if (std::filesystem::exists (upper_path))
		{
			std::filesystem::copy_file (upper_path, dest, std::filesystem::copy_options::skip_existing);
		}
		else
		{
			throw InstallException ("Couldn't copy file '" + source.u8string() + "' to '" + dest.u8string() + "'" + TEXT_FILE_LF);
		}
	}
	END_INSTALL_FILE (dest);
}

//--------------------------------------------------------------
bool DirExists (const std::filesystem::path& path)
{
	return std::filesystem::exists (path) && std::filesystem::is_directory (path);
}
