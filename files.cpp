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

#include <SDL.h>

#include "files.h"
#include "log.h"
#include "settings.h"

#ifdef _WIN32
	#include <io.h>
#else
	#include <dirent.h>
#endif

#ifdef WIN32
#else
	#include <sys/stat.h>
	#include <unistd.h>
#endif


// Pr¸ft ob die Datei 'name' existiert
bool FileExists ( const char* name )
{
	SDL_RWops *file;
	file=SDL_RWFromFile ( name, "r" );
	if ( file==NULL )
	{
		Log.write(SDL_GetError(), cLog::eLOG_TYPE_WARNING);
		return false;
	}
	SDL_RWclose ( file );
	return true;
}

int CheckFile(const char* directory, const char* filename)
{
	std::string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
		return 0;
	return 1;
}

cList<std::string> *getFilesOfDirectory(std::string sDirectory)
{
	cList<std::string> *List = new cList<std::string>;
#ifdef _WIN32
	_finddata_t DataFile;
	long lFile = (long)_findfirst ( (sDirectory + PATH_DELIMITER + "*.*").c_str(), &DataFile );

	if( lFile == -1 )
	{
		_findclose ( lFile );
	}
	else
	{
		do
		{
			if ( !( DataFile.attrib & _A_SUBDIR ) && DataFile.name[0] != '.' )
			{
				List->Add( DataFile.name );
			}
		}
		while ( _findnext ( lFile, &DataFile ) == 0 );
		_findclose ( lFile );
	}
#elif __amigaos4__
	DIR* hDir = opendir ( sDirectory.c_str() );
	struct dirent* entry;
	if( hDir == NULL )
	{
		closedir( hDir );
	}
	else
	{
		do
		{
			entry = readdir ( hDir );
			if( entry != NULL && entry->d_name[0] != '.' )
			{
				List->Add( entry->d_name );
			}
		}
		while ( entry != NULL );

		closedir( hDir );
	}
#else
	DIR* hDir = opendir ( sDirectory.c_str() );
	struct dirent* entry;
	if( hDir == NULL )
	{
		closedir( hDir );
	}
	else
	{
		do
		{
			entry = readdir ( hDir );
			if( entry != NULL && entry->d_name[0] != '.' )
			{
				List->Add( entry->d_name );
			}
		}
		while ( entry != NULL );

		closedir( hDir );
	}
#endif
	return List;
}

std::string getUserMapsDir()
{
#ifdef WIN32
	return "";
#else
	#ifdef __amigaos4__
		return "";
	#else
		if (SettingsData.sHome.empty ())
			return "";
		std::string mapFolder = SettingsData.sHome + "maps";
		if (FileExists (mapFolder.c_str ()) == false)
		{
			if (mkdir (mapFolder.c_str (), 0755) == 0)
				return mapFolder + PATH_DELIMITER;
			return "";
		}
		return mapFolder + PATH_DELIMITER;
	#endif
#endif
	return "";
}
