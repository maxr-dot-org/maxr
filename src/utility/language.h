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

#ifndef utility_languageH
#define utility_languageH

#include <map>
#include <string>
#include <vector>

#include <3rd/tinyxml2/tinyxml2.h>

struct sID;

class cLanguage
{
public:
	cLanguage();

	const std::string& GetCurrentLanguage() const;
	int SetCurrentLanguage (const std::string& szLanguageCode);

	int ReadLanguagePack();
	std::vector<std::string> getAvailableLanguages() const;

	std::string i18n (const std::string& szInputText) const;
	// Translation with replace %s
	std::string i18n (const std::string& szMainText, const std::string& szInsertText) const;

	std::string getUnitName(const sID& id) const;
	std::string getUnitDescription(const sID& id) const;

	std::string getClanName(int num) const;
	std::string getClanDescription(int num) const;

private:
	typedef std::map<std::string, std::string> StrStrMap;

	int ReadSingleTranslation (const char* pszCurrent, ...);
	std::string ReadSingleTranslation (const std::string& strInput);
	int ReadLanguagePackFooter();
	int ReadLanguagePackFooter (const std::string& strLanguageCode);
	int ReadLanguageMaster();
	int ReadRecursiveLanguagePack (tinyxml2::XMLElement* xmlElement, std::string strNodePath);

	int checkTimeStamp (std::string rstrData);

	tinyxml2::XMLDocument m_XmlDoc;
	// Use ISO 639-2 codes to identify languages
	// (http://www.loc.gov/standards/iso639-2/php/code_list.php)
	std::string m_szLanguage;
	std::string m_szLanguageFile;
	std::string m_szLanguageFileMaster;
	std::string m_szEncoding;
	std::string m_szLastEditor;
	StrStrMap   m_mpLanguage;
	bool m_bLeftToRight;
	bool m_bErrorMsgTranslationLoaded;
};

extern cLanguage lngPack;

#endif // utility_languageH
