//////////////////////////////////////////////////////////////////////////////
// M.A.X. - files.h
//////////////////////////////////////////////////////////////////////////////
#ifndef filesH
#define filesH
#include <SDL.h>
#include "defines.h"
#include "main.h"

#ifdef WIN32
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "\\"
	#endif
#else
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "//"
	#endif
#endif


/**
* Checks whether a file exists or not
* @author beko
* @param name Filename to check for
* @return true if exists (as in readable)
* @return false if does not exist (as in not readable)
*/
bool FileExists(const char* name);

/**
* Checks whether a file exits
* @author alzi
* @param directory Directory of the file
* @param filename Name of the file
* @return 1 on success
*/
int CheckFile(const char* directory, const char* filename);
#endif
