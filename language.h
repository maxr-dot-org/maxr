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

#define LANGUAGE_FILE_FOLDER SettingsData.sLangPath
#define LANGUAGE_FILE_NAME   "lang_"
#define LANGUAGE_FILE_EXT    ".xml"

////////////////////////////////////////////////////////////////////////////////
// XML-Node paths

// With NULL as ending sign
#define XNP_MAX_LANG_FILE "MAX_Language_File", NULL
#define XNP_MAX_LANG_FILE_HEADER_AUTHOR "MAX_Language_File", "Header", "Author", NULL
#define XNP_MAX_LANG_FILE_HEADER_AUTHOR_EDITOR "MAX_Language_File", "Header", "Author", "Editor", NULL
#define XNP_MAX_LANG_FILE_HEADER_GAMEVERSION "MAX_Language_File", "Header", "Game_Version", NULL
#define XNP_MAX_LANG_FILE_TEXT "MAX_Language_File", "Text", NULL
#define XNP_MAX_LANG_FILE_GRAPHIC "MAX_Language_File", "Graphic", NULL
#define XNP_MAX_LANG_FILE_SPEECH "MAX_Language_File", "Speech", NULL

// Without NULL as ending sign. Do not forget it in parameter list !
#define XNP_MAX_LANG_FILE_TEXT_MAIN "MAX_Language_File", "Text", "Main"
#define XNP_MAX_LANG_FILE_TEXT_ERROR_MSG "MAX_Language_File", "Text", "Error_Messages"

////////////////////////////////////////////////////////////////////////////////


#include <map>
#include <string>
#include "log.h"
#include "defines.h"

#include "tinyxml.h"
#include "extendedtinyxml.h"

class cLanguage
{
public:
	cLanguage(void);

protected:
	typedef std::map<std::string, std::string> StrStrMap;

	TiXmlDocument m_XmlDoc;
	// please use the ISO 639-2 Codes to identify a language ( http://www.loc.gov/standards/iso639-2/php/code_list.php )
	std::string m_szLanguage;
	std::string m_szLanguageFile;
	std::string m_szLanguageFileMaster;
	std::string m_szEncoding;
	std::string m_szLastEditor;
	bool m_bLeftToRight;
	bool m_bErrorMsgTranslationLoaded;
	StrStrMap m_mpLanguage;
	int ReadSingleTranslation( std::string & strResult, const char * pszCurrent, ... );
	std::string ReadSingleTranslation( std::string strInput );
	int ReadLanguagePackHeader( );
	int ReadLanguagePackHeader( std::string szLanguageCode );
	int ReadLanguageMaster();
	int ReadRecursiveLanguagePack( ExTiXmlNode * pXmlStartingNode , std::string strNodePath );
public:
	std::string GetCurrentLanguage(void);
	int SetCurrentLanguage(std::string szLanguageCode);
	std::string i18n(std::string szInputText);
	std::string i18n(std::string szMainText, std::string szInsertText); 	// Translation with replace %s
	int ReadLanguagePack();
	int CheckCurrentLanguagePack(bool bInsertMissingEntries);
};

#endif
