//////////////////////////////////////////////////////////////////////////////
// M.A.X. - files.h
//////////////////////////////////////////////////////////////////////////////
#ifndef filesH
#define filesH
#include <SDL.h>
#include "defines.h"
#include "main.h"

// Standard //////////////////////////////////////////////////////////////////

bool FileExists(const char* name);
long SearchFor(const char* searchstr, long startpos, const char* filename);
int FileEraseData(long startpos, long endpos, const char* filename);
int FileInsertData(long pos, const char* data, const char* filename);
string LoadFileToString(const char* filename);
long StrPosToFilePos(string filestring, long strpos);

// Ini-Dateien ///////////////////////////////////////////////////////////////
TList *ReadIniSections(const char* name);
char *ReadIniString(const char* section, const char* key, const char* defaultValue, const char* filename);
int ReadIniInteger(const char* section, const char* key, int defaultValue, const char* filename);
bool ReadIniBool(const char* section, const char* key, bool defaultValue, const char* filename);

void WriteIniString(const char* section, const char* key, const char* Value, const char* filename);
void WriteIniInteger(const char* section, const char* key, int Value, const char* filename);
void WriteIniBool(const char* section, const char* key, bool Value, const char* filename);

#endif
