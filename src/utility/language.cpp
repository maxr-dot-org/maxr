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
//  File:   language.cpp
//  Date:   07-10-01
//  Author: JCK
//
////////////////////////////////////////////////////////////////////////////////
//  Description:
//  This class handles the support for language packs in XML-format.
////////////////////////////////////////////////////////////////////////////////

#include "utility/language.h"
#include "maxrversion.h"
#include "utility/extendedtinyxml.h"
#include "utility/log.h"
#include "settings.h"
#include "utility/files.h"
#include "utility/string/toupper.h"
#include "game/data/units/unitdata.h"
#include "string/toString.h"

using namespace tinyxml2;

cLanguage lngPack;

cLanguage::cLanguage() :
	m_bLeftToRight (true),
	m_bErrorMsgTranslationLoaded (false)
{}

const std::string& cLanguage::GetCurrentLanguage() const
{
	return m_szLanguage;
}

int cLanguage::SetCurrentLanguage (const std::string& szLanguageCode)
{
	// don't do this in constructor
	// because language folder isn't known yet in programm start.
	// since the first thing we do with language files is setting
	// our language we can init the master lang file here too -- beko
	m_szLanguageFileMaster = LANGUAGE_FILE_FOLDER;
	m_szLanguageFileMaster += PATH_DELIMITER LANGUAGE_FILE_NAME "eng" LANGUAGE_FILE_EXT;

	if (szLanguageCode.length() != 3)
	{
		return -1;
	}
	if (szLanguageCode.find_first_not_of
		("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") != std::string::npos)
	{
		return -1;
	}

	m_szLanguage = szLanguageCode;
	for (int i = 0; i <= 2; i++)
	{
		if (m_szLanguage[i] < 'a')
		{
			m_szLanguage[i] += 'a' - 'A';
		}
	}

	m_szLanguageFile = LANGUAGE_FILE_FOLDER;
	m_szLanguageFile += PATH_DELIMITER LANGUAGE_FILE_NAME;
	m_szLanguageFile += m_szLanguage + LANGUAGE_FILE_EXT;
	return 0;
}

std::string cLanguage::i18n (const std::string& szInputText)
{
	StrStrMap::const_iterator impTranslation;
	impTranslation = m_mpLanguage.find (szInputText);

	if (impTranslation == m_mpLanguage.end())
	{
		if (m_bErrorMsgTranslationLoaded)
		{
			return i18n ("Text~Error_Messages~ERROR_Missing_Translation", szInputText);
		}
		else
		{
			return std::string ("missing translation: ") + szInputText;
		}
	}
	else
	{
		return impTranslation->second;
	}
}

// Translation with replace %s
std::string cLanguage::i18n (const std::string& szMainText, const std::string& szInsertText)
{
	std::string szMainTextNew;
	std::size_t iPos;

	szMainTextNew = this->i18n (szMainText);
	iPos = szMainTextNew.find ("%s");
	if (iPos == std::string::npos)
	{
		Log.write ("Found no place holder in language string. Update language file!", cLog::eLOG_TYPE_WARNING);
		Log.write ("*-> String in question is: \"" + szMainText + "\"", cLog::eLOG_TYPE_WARNING);
		return szMainTextNew + szInsertText;
	}
	else
	{
		szMainTextNew.replace (iPos, 2, szInsertText);
		return szMainTextNew;
	}
}

int cLanguage::ReadLanguagePack()
{
	// First let's load the English language pack and use it as master
	if (ReadLanguagePackFooter() != 0)
	{
		return -1;
	}

	// Read the complete <Text> - section
	XMLElement* xmlStartingElement = nullptr;
	try
	{
		xmlStartingElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE_TEXT);
		if (xmlStartingElement == nullptr) throw std::string ("No <Text> section found!");

		xmlStartingElement = xmlStartingElement->FirstChildElement();
		if (xmlStartingElement == nullptr) throw std::string ("<Text> section is empty!");
	}
	catch (const std::string& strMsg)
	{
		Log.write ("Language file (eng): " + strMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}
	ReadRecursiveLanguagePack (xmlStartingElement, "Text");

	// Read the complete <Graphic> - section
	xmlStartingElement = nullptr;
	try
	{
		xmlStartingElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE_GRAPHIC);
		if (xmlStartingElement == nullptr) throw std::string ("No <Graphic> section found!");

		xmlStartingElement = xmlStartingElement->FirstChildElement();
		if (xmlStartingElement == nullptr) throw std::string ("<Graphic> section is empty!");
	}
	catch (const std::string& strMsg)
	{
		Log.write ("Language file (eng): " + strMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}
	ReadRecursiveLanguagePack (xmlStartingElement, "Graphic");

	// Read the complete <Speech> - section
	xmlStartingElement = nullptr;
	try
	{
		xmlStartingElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE_SPEECH);
		if (xmlStartingElement == nullptr) throw std::string ("No <Speech> section found!");

		xmlStartingElement = xmlStartingElement->FirstChildElement();
		if (xmlStartingElement == nullptr) throw std::string ("<Speech> section is empty!");
	}
	catch (const std::string& strMsg)
	{
		Log.write ("Language file (eng): " + strMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}
	ReadRecursiveLanguagePack (xmlStartingElement, "Speech");

	// Second step: Load the selected language pack
	if (m_szLanguage == "eng")
	{
		StrStrMap::const_iterator impTranslation;
		impTranslation = m_mpLanguage.find ("Text~Error_Messages~ERROR_Missing_Translation");
		if (impTranslation == m_mpLanguage.end())
		{
			m_mpLanguage["Text~Error_Messages~ERROR_Missing_Translation"] = "missing translation: %s";
		}
		m_bErrorMsgTranslationLoaded = true;
		return 0;
	}
	if (ReadLanguagePackFooter (m_szLanguage) != 0)
	{
		return -1;
	}

	// Now - finally - let's get the translations.
	if (ReadSingleTranslation (XNP_MAX_LANG_FILE_TEXT_ERROR_MSG, "ERROR_Missing_Translation", nullptr) == 0)
	{
		m_bErrorMsgTranslationLoaded = true;
	}

	for (StrStrMap::const_iterator impPosition = m_mpLanguage.begin();
		 impPosition !=  m_mpLanguage.end();
		 ++impPosition)
	{
		std::string strResult = ReadSingleTranslation (impPosition->first);
		if (strResult != "")
		{
			m_mpLanguage[impPosition->first] = strResult;
		}
		else
		{
			// TODO: ?
		}
	}

	Log.write (this->i18n ("Text~Error_Messages~INFO_Language_initialised"), cLog::eLOG_TYPE_INFO);

	return 0;
}

std::vector<std::string> cLanguage::getAvailableLanguages() const
{
	std::vector<std::string> languageCodes;

	const auto fileNames = getFilesOfDirectory (LANGUAGE_FILE_FOLDER);

	const auto languageFileNameSize = strlen (LANGUAGE_FILE_NAME);
	const auto languageFileExtensionSize = strlen (LANGUAGE_FILE_EXT);

	for (size_t i = 0; i < fileNames.size(); ++i)
	{
		const auto& fileName = fileNames[i];

		if (fileName.size() <= languageFileNameSize + languageFileExtensionSize) continue;

		const auto prefix = fileName.substr (0, languageFileNameSize);

		if (prefix.compare (LANGUAGE_FILE_NAME) != 0) continue;

		const auto suffix = fileName.substr (fileName.size() - languageFileExtensionSize);

		if (suffix.compare (LANGUAGE_FILE_EXT) != 0) continue;

		const auto languageCode = fileName.substr (languageFileNameSize, fileName.size() - languageFileExtensionSize - languageFileNameSize);

		languageCodes.push_back (to_upper_copy (languageCode));
	}

	return languageCodes;
}

std::string cLanguage::getUnitName(const sID& id) const
{
	const XMLElement* xmlElement = m_XmlDoc.RootElement()->FirstChildElement("Units");
	if (xmlElement == nullptr) return "";

	xmlElement = xmlElement->FirstChildElement();
	while (xmlElement != nullptr)
	{
		const char* value = xmlElement->Attribute("ID");
		if (value == nullptr) continue;

		sID elementID;
		elementID.generate(value);

		if (elementID == id)
		{
			const char* value;
			if (cSettings::getInstance().getLanguage() != "ENG")
				value = xmlElement->Attribute("localized");
			else
				value = xmlElement->Attribute("ENG");
			if (value == nullptr) return "";

			return std::string(value);
		}
		xmlElement = xmlElement->NextSiblingElement();
	}
	return "";
}

std::string cLanguage::getUnitDescription(const sID& id) const
{
	const XMLElement* xmlElement = m_XmlDoc.RootElement()->FirstChildElement("Units");
	if (xmlElement == nullptr) return "";

	xmlElement = xmlElement->FirstChildElement();
	while (xmlElement != nullptr)
	{
		const char* value = xmlElement->Attribute("ID");
		if (value == nullptr) continue;

		sID elementID;
		elementID.generate(value);

		if (elementID == id)
		{
			const char* value = xmlElement->GetText();
			if (value == nullptr) return "";

			std::string description = value;
			size_t pos;
			while ((pos = description.find("\\n")) != std::string::npos)
			{
				description.replace(pos, 2, "\n");
			}
			return description;
		}
		xmlElement = xmlElement->NextSiblingElement();
	}
	return "";
}

std::string cLanguage::getClanName(int num) const
{
	const XMLElement* xmlElement = m_XmlDoc.RootElement()->FirstChildElement("Clans");
	if (!xmlElement)
	{
		Log.write("Can't find clan node in language file. Please report this to your translation team!", cLog::eLOG_TYPE_WARNING);
		return "";
	}

	for (xmlElement = xmlElement->FirstChildElement("Clan"); xmlElement; xmlElement = xmlElement->NextSiblingElement("Clan"))
	{
		int id;
		if (xmlElement->QueryIntAttribute("ID", &id) != XML_NO_ERROR) continue;
		if (id != num) continue;

		if (cSettings::getInstance().getLanguage() != "ENG")
		{
			const char* name = xmlElement->Attribute("localized");
			if (!name) continue;
			return std::string(name);
		}
		else
		{
			const char* name = xmlElement->Attribute("ENG");
			if (!name) continue;
			return std::string(name);
		}
	}

	Log.write("Can't find clan translation for clan " + toString(num), cLog::eLOG_TYPE_WARNING);
	return "";
}

std::string cLanguage::getClanDescription(int num) const
{
	const XMLElement* xmlElement = m_XmlDoc.RootElement()->FirstChildElement("Clans");
	if (!xmlElement)
	{
		Log.write("Can't find clan node in language file. Please report this to your translation team!", cLog::eLOG_TYPE_WARNING);
		return "";
	}

	for (xmlElement = xmlElement->FirstChildElement("Clan"); xmlElement; xmlElement = xmlElement->NextSiblingElement("Clan"))
	{
		int id;
		if (xmlElement->QueryIntAttribute("ID", &id) != XML_NO_ERROR) continue;
		if (id != num) continue;

		const char* description = xmlElement->GetText();
		if (description != nullptr)
			return std::string(description);
	}

	Log.write("Can't find clan translation for clan " + toString(num), cLog::eLOG_TYPE_WARNING);
	return "";
}

int cLanguage::ReadSingleTranslation(const char* pszCurrent, ...)
{
	va_list pvaArg;
	va_start (pvaArg, pszCurrent);
	StrStrMap::const_iterator impTranslation;
	std::string szErrorMsg;

	XMLElement* xmlElement = nullptr;
	std::string szXmlNodePath;

	for (;;)
	{
		xmlElement = m_XmlDoc.RootElement();
		if (xmlElement == nullptr)
		{
			break;
		}

		if (strcmp (xmlElement->Value(), pszCurrent) != 0)
		{
			break;
		}

		do
		{
			pszCurrent = va_arg (pvaArg, char*);
			if (pszCurrent != nullptr)
			{
				szXmlNodePath += "~";
				szXmlNodePath += pszCurrent;
				xmlElement = xmlElement->FirstChildElement (pszCurrent);
				if (xmlElement == nullptr)
				{
					break;
				}
			}
		}
		while (pszCurrent != nullptr);
		break;
	}
	szXmlNodePath.erase (0, 1);

	if (xmlElement != nullptr)
	{
		const char* value = xmlElement->Attribute ("localized");
		if (value)
		{
			va_end (pvaArg);
			impTranslation = m_mpLanguage.find (szXmlNodePath);
			if (impTranslation == m_mpLanguage.end())
			{
				szErrorMsg = "Language file: translation for >";
				szErrorMsg += szXmlNodePath + "< is read more than once!";
				Log.write (szErrorMsg, cLog::eLOG_TYPE_WARNING);
				return -1;
			}
			m_mpLanguage[szXmlNodePath] = value;
			return 0;
		}
	}

	szErrorMsg = "Language file: translation for >";
	if (xmlElement != nullptr)
	{
		const char* value = xmlElement->Attribute ("ENG");
		if (value != nullptr)
		{
			m_mpLanguage[szXmlNodePath] = value;
			szErrorMsg += std::string (value) + "< is missing";
		}
		else
		{
			if (m_bErrorMsgTranslationLoaded)
			{
				m_mpLanguage[szXmlNodePath] = i18n ("Text~Error_Messages~ERROR_Missing_Translation") + szXmlNodePath;
			}
			else
			{
				m_mpLanguage[szXmlNodePath] = std::string ("missing translation: ") + szXmlNodePath;
			}
			szErrorMsg += szXmlNodePath + "< is missing";
		}
	}
	else
	{
		if (m_bErrorMsgTranslationLoaded)
		{
			m_mpLanguage[szXmlNodePath] = i18n ("Text~Error_Messages~ERROR_Missing_Translation", szXmlNodePath);
		}
		else
		{
			m_mpLanguage[szXmlNodePath] = std::string ("missing translation: ") + szXmlNodePath;
		}
		szErrorMsg += szXmlNodePath + "< is missing";
	}
	Log.write (szErrorMsg, cLog::eLOG_TYPE_WARNING);
	va_end (pvaArg);
	return -1;
}

/////////////
//ExTiXmlNode* node;
//node = XmlGetFirstNode (...);
//while (node != nullptr)
//{
//	data = node->XmlReadNodeData (...);
//	node = node-XmlGetFirstNodeChild();
//child = child->NextSibling() )
/////////////

int cLanguage::ReadLanguagePackFooter()
{
	return ReadLanguagePackFooter ("");
}

int cLanguage::ReadLanguagePackFooter (const std::string& strLanguageCode)
{
	XMLElement* xmlElement = nullptr;
	std::string strErrorMsg;
	std::string strFileName;
	std::string szLanguageCode (strLanguageCode);

	if (szLanguageCode.empty())
	{
		strFileName = m_szLanguageFileMaster;
		szLanguageCode = "eng";
	}
	else
	{
		strFileName = m_szLanguageFile;
	}

	// Load the file
	if (m_XmlDoc.LoadFile (strFileName.c_str()) != XML_NO_ERROR)
	{
		strErrorMsg = "Can't open language file :" + strFileName;
		Log.write (strErrorMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}
	// Is the main node correct ?
	xmlElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE);
	if (xmlElement == nullptr)
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): missing main node!";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}

	// Who is responsible for the file ? (Who is to blame in case of errors?)
	xmlElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE_FOOTER_AUTHOR);
	if (xmlElement == nullptr)
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): missing author node!";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_ERROR);
	}
	else
	{
		//TODO: Find the last editor of the XML file
	}

	// Check the lang attribute of the main node
	xmlElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE);
	const char* value = xmlElement->Attribute ("lang");
	if (value == nullptr)
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): language attribut missing! Language can not be identified";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}
	else if (szLanguageCode  != std::string (value))
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): language attribut mismatch file name!";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_ERROR);
		return -1;
	}

	// Writing is left-to-right or vice versa ?
	value = xmlElement->Attribute ("direction");
	if (value == nullptr)
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): language attribut 'direction' is missing! Writing direction will be set to 'Left-To-Right'";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
		m_bLeftToRight = true;
	}
	else if (std::string (value) == "left-to-right")
	{
		m_bLeftToRight = true;
	}
	else if (std::string (value) == "right-to-left")
	{
		m_bLeftToRight = false;
	}
	else
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): language attribut 'direction' can not interpreted! Writing direction will be set to 'Left-To-Right'";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
		m_bLeftToRight = true;
	}

	xmlElement = XmlGetFirstElement (m_XmlDoc, XNP_MAX_LANG_FILE_FOOTER_GAMEVERSION);
	if (xmlElement == nullptr)
	{
		strErrorMsg = "Language file (" + szLanguageCode + "): missing game version node!";
		Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		const char* value = xmlElement->Attribute ("time");
		if (value == nullptr)
		{
			strErrorMsg = "Language file (" + szLanguageCode + "): game version attribute 'time' is missing!";
			Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			int iTestResult = checkTimeStamp (value);
			switch (iTestResult)
			{
				case -1 :
					strErrorMsg = "Language file (" + szLanguageCode + "): game version attribute has wrong format!";
					Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
					break;
				case 0 :
					strErrorMsg = "Language file (" + szLanguageCode + "): may be outdated!";
					Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
					break;
				case 1 :
					strErrorMsg = "Language file (" + szLanguageCode + "): is newer than the game!";
					Log.write (strErrorMsg, cLog::eLOG_TYPE_WARNING);
					break;
				case 2 :
					// Timestamps match
					break;
			}
		}
	}
	return 0;
}

// not const string& since it is modified
int cLanguage::checkTimeStamp (std::string rstrData)
{
	//JCK: Should be replaced by a faster and more secure function

	// Index   : 0123456789012345678
	// Example : 2007-09-30 13:04:00

	if (rstrData.length() != 19) return -1;
	if (! isdigit (rstrData[0])) return -1;
	if (! isdigit (rstrData[1])) return -1;
	if (! isdigit (rstrData[2])) return -1;
	if (! isdigit (rstrData[3])) return -1;
	if (rstrData[4] != '-') return -1;
	if (! isdigit (rstrData[5])) return -1;
	if (! isdigit (rstrData[6])) return -1;
	if (rstrData[7] != '-') return -1;
	if (! isdigit (rstrData[8])) return -1;
	if (! isdigit (rstrData[9])) return -1;
	if (rstrData[10] != ' ') return -1;
	if (! isdigit (rstrData[11])) return -1;
	if (! isdigit (rstrData[12])) return -1;
	if (rstrData[13] != ':') return -1;
	if (! isdigit (rstrData[14])) return -1;
	if (! isdigit (rstrData[15])) return -1;
	if (rstrData[16] != ':') return -1;
	if (! isdigit (rstrData[17])) return -1;
	if (! isdigit (rstrData[18])) return -1;

	std::string szTemp1 = rstrData;

	szTemp1.erase (16, 1);
	szTemp1.erase (13, 1);
	szTemp1.erase (10, 1);
	szTemp1.erase (7, 1);
	szTemp1.erase (4, 1);

	std::string szTemp2 = MAX_BUILD_DATE;

	szTemp2.erase (16, 1);
	szTemp2.erase (13, 1);
	szTemp2.erase (10, 1);
	szTemp2.erase (7, 1);
	szTemp2.erase (4, 1);

	if (szTemp1 < szTemp2)
	{
		return 0; // XML is older than the game
	}
	if (szTemp1 > szTemp2)
	{
		return 1; // XML is newer than the game
	}
	return 2;     // XML is matching the game
}

int cLanguage::ReadRecursiveLanguagePack (XMLElement* xmlElement, std::string strNodePath)
{
	if (xmlElement == nullptr)
	{
		return -1;
	}

	if (strNodePath[ strNodePath.length() - 1 ] != '~')
	{
		strNodePath += "~";
	}

	const char* value = xmlElement->Attribute ("ENG");
	if (value != nullptr)
	{
		m_mpLanguage[strNodePath + xmlElement->Value()] = value;
		Log.write (strNodePath + xmlElement->Value() + " : " + value, cLog::eLOG_TYPE_DEBUG);
	}

	XMLElement* xmlElementTMP = xmlElement->FirstChildElement();
	if (xmlElementTMP != nullptr)
	{
		this->ReadRecursiveLanguagePack (xmlElementTMP, strNodePath + xmlElement->Value());
	}

	xmlElement = xmlElement->NextSiblingElement();
	if (xmlElement != nullptr)
	{
		this->ReadRecursiveLanguagePack (xmlElement, strNodePath);
	}

	return 0;
}

std::string cLanguage::ReadSingleTranslation (const std::string& strInput)
{
	if (strInput.empty()) return "";
	const std::string& strPath = strInput;

	XMLElement* xmlElement = m_XmlDoc.RootElement();
	try
	{
		if (xmlElement == nullptr) throw std::string ("Can't excess root node");
		if (strcmp (xmlElement->Value(), "MAX_Language_File") != 0) throw std::string ("Root node mismatch");
	}
	catch (const std::string& strMsg)
	{
		Log.write ("Language file (" + m_szLanguage + "): " + strMsg, cLog::eLOG_TYPE_WARNING);
		return "";
	}
	std::size_t iPosBegin = 0;
	try
	{
		do
		{
			std::size_t iPosEnd = strPath.find ('~', iPosBegin + 1);
			std::string strCurrent = strPath.substr (iPosBegin, iPosEnd - iPosBegin);
			if (strCurrent[0] == '~') strCurrent.erase (0, 1);
			xmlElement = xmlElement->FirstChildElement (strCurrent.c_str());
			if (xmlElement == nullptr)
				throw i18n ("Text~Error_Messages~ERROR_Missing_Translation", m_mpLanguage[strInput]);

			iPosBegin = iPosEnd;
		}
		while (iPosBegin != std::string::npos);
	}
	catch (const std::string& strMsg)
	{
		Log.write ("Language file (" + m_szLanguage + "): " + strMsg, cLog::eLOG_TYPE_WARNING);
		return m_mpLanguage[strInput];
	}

	const char* translation = xmlElement->Attribute ("localized");
	if (translation != nullptr)
	{
		return translation;
	}

	std::string szErrorMsg = "Language file: translation for >";
	szErrorMsg += strInput + "< is missing";
	Log.write (szErrorMsg, cLog::eLOG_TYPE_WARNING);

	return "";
}
