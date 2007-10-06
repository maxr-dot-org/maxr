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

#define LANGUAGE_FILE_FOLDER "languages"
#define LANGUAGE_FILE_NAME   "Language "
#define LANGUAGE_FILE_EXT    ".xml"

////////////////////////////////////////////////////////////////////////////////
// XML-Node paths

#define XNP_MAX_LANG_FILE "MAX_Language_File", ""
#define XNP_MAX_LANG_FILE_HEADER_AUTHOR "MAX_Language_File", "Header", "Author", ""
#define XNP_MAX_LANG_FILE_HEADER_AUTHOR_EDITOR "MAX_Language_File", "Header", "Author", "Editor", ""

////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>

typedef std::map < std::string, std::string > StrStrMap; 

class cLanguage
{
public:
	cLanguage(void);
	~cLanguage(void);
protected:
	// please use the ISO 639-2 Codes to identify a language ( http://www.loc.gov/standards/iso639-2/php/code_list.php )
	std::string m_szLanguage;
	std::string m_szLanguageFile;
	std::string m_szEncoding;
	bool m_bLeftToRight;
	StrStrMap m_mpLanguage;
	std::string m_szLastEditor;
public:
	std::string GetCurrentLanguage(void);
	int SetCurrentLanguage(std::string szLanguageCode);
	std::string Translate(std::string szInputText);
	int ReadLanguagePack(std::string szLanguageCode);
	int CheckCurrentLanguagePack(bool bInsertMissingEntries);
};