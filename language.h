////////////////////////////////////////////////////////////////////////////////
//
//  File:   language.h
//  Date:   07-10-01
//  Author: JCK
//
////////////////////////////////////////////////////////////////////////////////
//  Description:
//  This class handles the support for different language packs in XML-Format.
////////////////////////////////////////////////////////////////////////////////

#ifndef LANGUAGE_H
#define LANGUAGE_H

////////////////////////////////////////////////////////////////////////////////

#define LANGUAGE_FILE_FOLDER "languages"
#define LANGUAGE_FILE_NAME   "lang_"
#define LANGUAGE_FILE_EXT    ".xml"

////////////////////////////////////////////////////////////////////////////////
// XML-Node paths

// With NULL as ending sign
#define XNP_MAX_LANG_FILE "MAX_Language_File", NULL
#define XNP_MAX_LANG_FILE_HEADER_AUTHOR "MAX_Language_File", "Header", "Author", NULL
#define XNP_MAX_LANG_FILE_HEADER_AUTHOR_EDITOR "MAX_Language_File", "Header", "Author", "Editor", NULL
#define XNP_MAX_LANG_FILE_HEADER_GAMEVERSION "MAX_Language_File", "Header", "Game_Version", NULL

// Without NULL as ending sign. Do not forget it in parameter list !
#define XNP_MAX_LANG_FILE_TEXT_MAIN "MAX_Language_File", "Text", "Main"
#define XNP_MAX_LANG_FILE_TEXT_ERROR_MSG "MAX_Language_File", "Text", "Error_Messages"

////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <tinyxml.h>
#ifndef LOG_H
	#include "log.h"
#endif
#include "defines.h"

#include "ExtendedTinyXml.h"

typedef std::map < std::string, std::string > StrStrMap; 

class cLanguage
{
public:
	cLanguage(void);
	~cLanguage(void);
protected:
	TiXmlDocument m_XmlDoc;
	// please use the ISO 639-2 Codes to identify a language ( http://www.loc.gov/standards/iso639-2/php/code_list.php )
	std::string m_szLanguage;
	std::string m_szLanguageFile;
	std::string m_szEncoding;
	bool m_bLeftToRight;
	bool m_bErrorMsgTranslationLoaded;
	StrStrMap m_mpLanguage;
	std::string m_szLastEditor;
	int ReadSingleTranslation( std::string & strResult, const char * pszCurrent, ... );
public:
	std::string GetCurrentLanguage(void);
	int SetCurrentLanguage(std::string szLanguageCode);
	std::string Translate(std::string szInputText);
	std::string Translate(std::string szMainText, std::string szInsertText); 	// Translation with replace %s
	int ReadLanguagePack();
	int CheckCurrentLanguagePack(bool bInsertMissingEntries);
};

#endif
