//////////////////////////////////////////////////////////////////////////////
// M.A.X. - files.cpp
//////////////////////////////////////////////////////////////////////////////
#include "files.h"
#include "log.h"
#include "string.h"

// Prüft ob die Datei 'name' existiert
bool FileExists ( const char* name )
{
	SDL_RWops *file;
	file=SDL_RWFromFile ( name, "r" );
	if ( file==NULL )
	{
		cLog::write(SDL_GetError(), cLog::eLOG_TYPE_WARNING);
		return false;
	}
	SDL_RWclose ( file );
	return true;
}

int CheckFile(const char* directory, const char* filename)
{
	string filepath;
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
