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

// Standard //////////////////////////////////////////////////////////////////

/**
* Checks whether a file exists or not
* @author MM
* @param name Filename to check for
* @return true if exists(readable) and false if not
*/
bool FileExists(const char* name);

/**
* Sucht in einer der Datei 'filename', ab der Position 'startpos', nach 'searchstr'
* und gibt, wenn gefunden, die Position zurück. Ansonnsten wird -1 zurückgegeben.
* @author MM
* @param searchstr String to search for
* @param startpos Position to start search from
* @param filename Name of file for search 
* @return position in file on hit or -1 if searchstr could not be found
*/
long SearchFor(const char* searchstr, long startpos, const char* filename);

/**
* Löscht ab der Position 'startpos' bis zur position 'endpos' alle Daten der Datei 'filename'.
* Ist 'endpos' gleich 0, so wird bis zum ende der Datei gelöscht.
* Ist ein Fehler aufgetreten, so wird 0 zurückgegeben.
* @param startpos Position to start deleting
* @param endpos Position to stop deleting
* @param filename File to delete data from
* @author MM
* @return returns 0 on error and 1 on success
*/
int FileEraseData(long startpos, long endpos, const char* filename);

/**
* Fügt ab der Position 'pos' alle Daten 'data' in die Datei 'filename' ein.
* Ist ein Fehler aufgetreten, so wird 0 zurückgegeben.
* @param pos position to start writing data in file
* @param data data to write into file
* @param filename file to open for writing
* @author MM
* @return returns 0 on error and 1 on success
*/
int FileInsertData(long pos, const char* data, const char* filename);

/**
* Opens a file for reading and returns data from file as string
* @author MM
* @param filename File to read data from
* @return Data from file as string or NULL on error
*/
string LoadFileToString(const char* filename);
long StrPosToFilePos(string filestring, long strpos);

// Ini-Dateien ///////////////////////////////////////////////////////////////
TList *ReadIniSections(const char* name);

/**
* Gets string of key 'key' in section 'section' and returns the string.
* If no string exists in 'filename' defaultValue will be returned
* @author MM
* @param section Section with key
* @param key Key for value
* @param defaultValue Value returned in case no value can be found in file.
* @param filename INI-File to open
* @return Found value or defaultValue on error
* @deprecated
*/
char *ReadIniString(const char* section, const char* key, const char* defaultValue, const char* filename);

/**
* Gets integer of key 'key' in section 'section' and returns the value.
* If no value exists in 'filename' defaultValue will be returned
* @author MM
* @param section Section with key
* @param key Key for value
* @param defaultValue Value returned in case no value can be found in file.
* @param filename INI-File to open
* @return Found value or defaultValue on error
* @deprecated
*/
int ReadIniInteger(const char* section, const char* key, int defaultValue, const char* filename);

/**
* Gets bool of key 'key' in section 'section' and returns the value.
* If no value exists in 'filename' defaultValue will be returned
* @author MM
* @param section Section with key
* @param key Key for value
* @param defaultValue Value returned in case no value can be found in file.
* @param filename INI-File to open
* @return Found value or defaultValue on error
* @deprecated
*/
bool ReadIniBool(const char* section, const char* key, bool defaultValue, const char* filename);

/**
* Writes string 'Value' after key 'key' of section 'section'.
* Creates section of key if they don't exist yet. Old values will be overwritten!
* @author MM
* @param section Section with key
* @param key Key for value
* @param Value Value to write
* @param filename INI-File to open
* @deprecated
*/
void WriteIniString(const char* section, const char* key, const char* Value, const char* filename);

/**
* Writes integer 'Value' after key 'key' of section 'section'.
* Creates section of key if they don't exist yet. Old values will be overwritten!
* @author MM
* @param section Section with key
* @param key Key for value
* @param Value Value to write
* @param filename INI-File to open
* @deprecated
*/
void WriteIniInteger(const char* section, const char* key, int Value, const char* filename);

/**
* Writes bool 'Value' after key 'key' of section 'section'.
* Creates section of key if they don't exist yet. Old values will be overwritten!
* @author MM
* @param section Section with key
* @param key Key for value
* @param Value Value to write
* @param filename INI-File to open
* @deprecated
*/
void WriteIniBool(const char* section, const char* key, bool Value, const char* filename);

#endif
