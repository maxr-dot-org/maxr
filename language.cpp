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

#include "language.h"

extern cLog fLog;

cLanguage::cLanguage(void)
{
	m_szLanguage = "";
	m_szLanguageFile = "";
	m_szEncoding = "";
	m_bLeftToRight = true;
	m_szLastEditor = "";
	m_bErrorMsgTranslationLoaded = false;

	m_szLanguageFileMaster = LANGUAGE_FILE_FOLDER ;
	m_szLanguageFileMaster += PATH_DELIMITER;
	m_szLanguageFileMaster += LANGUAGE_FILE_NAME;
	m_szLanguageFileMaster += "eng";
	m_szLanguageFileMaster += LANGUAGE_FILE_EXT;
}

cLanguage::~cLanguage(void)
{
}

std::string cLanguage::GetCurrentLanguage(void)
{
	return m_szLanguage;
}

int cLanguage::SetCurrentLanguage(std::string szLanguageCode)
{
	std::string szTemp;
	if( szLanguageCode.length() != 3 )
	{
		return -1;
	}
	if( szLanguageCode.find_first_not_of
		( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos )
	{
		return -1;
	}

	m_szLanguage = szLanguageCode;
	for(int i=0 ; i<=2 ; i++)
	{
		if( m_szLanguage[i] < 97 )
		{
			m_szLanguage[i] += 32;
		}
	}

	m_szLanguageFile = LANGUAGE_FILE_FOLDER ;
	m_szLanguageFile += PATH_DELIMITER;
	m_szLanguageFile += LANGUAGE_FILE_NAME;
	m_szLanguageFile +=	m_szLanguage + LANGUAGE_FILE_EXT;
	return 0;
}

std::string cLanguage::i18n(std::string szInputText)
{
	StrStrMap :: const_iterator impTranslation;
	impTranslation = m_mpLanguage.find( szInputText );

	if( impTranslation == m_mpLanguage.end() )
	{
		if( m_bErrorMsgTranslationLoaded == true )
		{
			return i18n( "Text~Error_Messages~ERROR_Missing_Translation" , szInputText );
		}else
		{
			return std::string("missing translation: ") + szInputText;
		}
	}else
	{
		if ( impTranslation->second != "" )
			return impTranslation->second ;
		else
			return "";
	}
}

// Translation with replace %s
std::string cLanguage::i18n(std::string  szMainText, std::string szInsertText)
{
	std::string  szMainTextNew;
	std::size_t iPos;

	szMainTextNew = this->i18n( szMainText );
	iPos = szMainTextNew.find( "%s" );
	if( iPos == std::string::npos )
	{
		return szMainTextNew + szInsertText;
	}
	szMainTextNew.replace( iPos, 2, szInsertText );
	return szMainTextNew;
}



int cLanguage::ReadLanguagePack()
{
	std::string strResult;
	ExTiXmlNode * pXmlStartingNode;

	// First let's load the English language pack and use it as master
	if( ReadLanguagePackHeader() != 0 )
	{
		return -1;
	}

	// Read the complete <Text> - section
	pXmlStartingNode = NULL;
	try
	{
		pXmlStartingNode = pXmlStartingNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_TEXT );
		if( pXmlStartingNode == NULL ) throw std::string("No <Text> section found!");

		pXmlStartingNode = pXmlStartingNode->XmlGetFirstNodeChild();
		if( pXmlStartingNode == NULL ) throw std::string("<Text> section is empty!");

	}
	catch( std::string strMsg )
	{
		fLog.write( "Language file (eng): " + strMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	ReadRecursiveLanguagePack( pXmlStartingNode , "Text" );

	// Read the complete <Graphic> - section
	pXmlStartingNode = NULL;
	try
	{
		pXmlStartingNode = pXmlStartingNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_GRAPHIC );
		if( pXmlStartingNode == NULL ) throw std::string("No <Graphic> section found!");

		pXmlStartingNode = pXmlStartingNode->XmlGetFirstNodeChild();
		if( pXmlStartingNode == NULL ) throw std::string("<Graphic> section is empty!") ;

	}
	catch( std::string strMsg )
	{
		fLog.write( "Language file (eng): " + strMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	ReadRecursiveLanguagePack( pXmlStartingNode , "Graphic" );


	// Read the complete <Speech> - section
	pXmlStartingNode = NULL;
	try
	{
		pXmlStartingNode = pXmlStartingNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_SPEECH );
		if( pXmlStartingNode == NULL ) throw std::string("No <Speech> section found!");

		pXmlStartingNode = pXmlStartingNode->XmlGetFirstNodeChild();
		if( pXmlStartingNode == NULL ) throw std::string("<Speech> section is empty!");

	}
	catch( std::string strMsg )
	{
		fLog.write( "Language file (eng): " + strMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	ReadRecursiveLanguagePack( pXmlStartingNode , "Speech" );

	// Second step: Load the selected language pack
	if( m_szLanguage == "eng" )
	{
		StrStrMap :: const_iterator impTranslation;
		impTranslation = m_mpLanguage.find( "Text~Error_Messages~ERROR_Missing_Translation" );
		if( impTranslation == m_mpLanguage.end() )
		{
			m_mpLanguage["Text~Error_Messages~ERROR_Missing_Translation"] = "missing translation: %s";
		}
		m_bErrorMsgTranslationLoaded = true;
		return 0;
	}
	if( ReadLanguagePackHeader( m_szLanguage ) != 0 )
	{
		return -1;
	}

	// Now - finaly - let's get the translations.
	if( ReadSingleTranslation( strResult, XNP_MAX_LANG_FILE_TEXT_ERROR_MSG, "ERROR_Missing_Translation", NULL ) == 0)
	{
		m_bErrorMsgTranslationLoaded = true;
	}
	StrStrMap::iterator impPosition;
	impPosition = m_mpLanguage.begin();
	while( impPosition !=  m_mpLanguage.end() )
	{
		strResult = ReadSingleTranslation( impPosition->first );
		if( strResult != "" )
		{
			m_mpLanguage[ impPosition->first ] = strResult;
		}else
		{
			// ToDo ?
		}
		impPosition++;
	}


	fLog.write(this->i18n("Text~Error_Messages~INFO_Language_initialised"), cLog::eLOG_TYPE_INFO );

	return 0;
}

int cLanguage::CheckCurrentLanguagePack(bool bInsertMissingEntries)
{
	// ToDo - JCK: Check and correct a language pack
	return 0;
}

int cLanguage::ReadSingleTranslation( std::string & strResult, const char * pszCurrent, ... )
{
	va_list pvaArg;
	va_start(pvaArg, pszCurrent);
	StrStrMap :: const_iterator impTranslation;
	std::string szErrorMsg;

	TiXmlNode * pXmlNode;
	std::string szXmlNodePath;

	for(;;)
	{
		if( m_XmlDoc.Value() == NULL )
		{
			break;
		}

		pXmlNode = m_XmlDoc.RootElement();
		if( pXmlNode == NULL )
		{
			break;
		}

		if( strcmp(pXmlNode->Value(), pszCurrent) != 0 )
		{
			break;
		}

		do
		{
			pszCurrent = va_arg(pvaArg, char * );
			if( pszCurrent != NULL )
			{
				szXmlNodePath += "~";
				szXmlNodePath += pszCurrent;
				pXmlNode = pXmlNode->FirstChild( pszCurrent );
				if( pXmlNode == NULL )
				{
					break;
				}
			}
		}while( pszCurrent != NULL );
		break;
	}
	szXmlNodePath.erase(0,1);

	if( pXmlNode != NULL )
	{
		if( ((ExTiXmlNode *)pXmlNode)->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "localized" ) != NULL )
		{
			va_end( pvaArg );
			impTranslation = m_mpLanguage.find( szXmlNodePath );
			if( impTranslation == m_mpLanguage.end() )
			{
				szErrorMsg = "Language file: translation for >";
				szErrorMsg += szXmlNodePath + "< is read more than once!";
				fLog.write( szErrorMsg, cLog::eLOG_TYPE_WARNING );
				return -1;
			}
			m_mpLanguage[szXmlNodePath] = strResult;
			return 0;
		}
	}

	szErrorMsg = "Language file: translation for >";
	if( pXmlNode != NULL )
	{
		if( ((ExTiXmlNode *)pXmlNode)->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "ENG" ) != NULL )
		{
			m_mpLanguage[szXmlNodePath] = strResult;
			szErrorMsg += strResult + "< is missing";
		}else
		{
			if( m_bErrorMsgTranslationLoaded == true )
			{
				m_mpLanguage[szXmlNodePath] = i18n( "Text~Error_Messages~ERROR_Missing_Translation" ) + szXmlNodePath;
			}else
			{
				m_mpLanguage[szXmlNodePath] = std::string("missing translation: ") + szXmlNodePath;
			}
			szErrorMsg += szXmlNodePath + "< is missing";
		}
	}else
	{
		if( m_bErrorMsgTranslationLoaded == true )
		{
			m_mpLanguage[szXmlNodePath] = i18n( "Text~Error_Messages~ERROR_Missing_Translation" , szXmlNodePath);
		}else
		{
			m_mpLanguage[szXmlNodePath] = std::string("missing translation: ") + szXmlNodePath;
		}
		szErrorMsg += szXmlNodePath + "< is missing";
	}
	fLog.write( szErrorMsg, cLog::eLOG_TYPE_WARNING );
	va_end( pvaArg );
	return -1;
}

	/////////////
	//ExTiXmlNode * node;
	//node = XmlGetFirstNode(...);
	//while( node != NULL )
	//{
	//	data = node->XmlReadNodeData(...);
	//	node = node-XmlGetFirstNodeChild();
	//child = child->NextSibling() )
	/////////////

int cLanguage::ReadLanguagePackHeader()
{
	return ReadLanguagePackHeader( "" );
}


int cLanguage::ReadLanguagePackHeader( std::string szLanguageCode )
{
	ExTiXmlNode * pXmlNode = NULL;
	std::string strResult;
	std::string strErrorMsg;
	std::string strFileName;

	if( szLanguageCode == "" )
	{
		strFileName = m_szLanguageFileMaster;
		szLanguageCode = "eng";
	}else
	{
		strFileName = m_szLanguageFile;
	}

	// Load the file
	if( !m_XmlDoc.LoadFile( strFileName.c_str() ))
	{
		strErrorMsg = "Can't open language file :" + strFileName;
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	// Is the main node correct ?
	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE );
	if( pXmlNode == NULL )
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): missing main node!";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}

	// Who is responsible for the file ? (Who is to blame in case of errors?)
	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_HEADER_AUTHOR );
	if( pXmlNode == NULL )
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): missing author node!";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_ERROR );
	}else
	{
		pXmlNode->XmlGetLastEditor( strResult, pXmlNode );
	}

	// Check the lang attribute of the main node
	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE );
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "lang" ) == NULL )
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): language attribut missing! Language can not be identified";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	if( szLanguageCode  != strResult )
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): language attribut mismatch file name!";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_ERROR );
		return -1;
	}

	// Writing is left-to-right oder vice versa ?
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "direction" ) == NULL )
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): language attribut 'direction' is missing! Writing direction will be set to 'Left-To-Right'";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
		m_bLeftToRight = true;
	}
	if( strResult == "left-to-right" )
	{
		m_bLeftToRight = true;
	}else if( strResult == "right-to-left" )
	{
		m_bLeftToRight = false;
	}else
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): language attribut 'direction' can not interpreted! Writing direction will be set to 'Left-To-Right'";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
		m_bLeftToRight = true;
	}

	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_HEADER_GAMEVERSION );
	if( pXmlNode == NULL )
	{
		strErrorMsg = "Language file (" + szLanguageCode+ "): missing game version node!";
		fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
	}else
	{
		if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "time" ) == NULL )
		{
			strErrorMsg = "Language file (" + szLanguageCode+ "): game version attribute 'time' is missing!";
			fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
		}else
		{
			int iTestResult = pXmlNode->CheckTimeStamp( strResult );
			switch( iTestResult )
			{
				case -1 :
					strErrorMsg = "Language file (" + szLanguageCode+ "): game version attribute has wrong format!";
					fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
					break;
				case 0 :
					strErrorMsg = "Language file (" + szLanguageCode+ "): may be outdated!";
					fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
					break;
				case 1 :
					strErrorMsg = "Language file (" + szLanguageCode+ "): is newer than the game!";
					fLog.write( strErrorMsg, cLog::eLOG_TYPE_WARNING );
					break;
				case 2 :
					// Timestamps match
					break;
			}
		}
	}
	return 0;
}

int cLanguage::ReadRecursiveLanguagePack( ExTiXmlNode * pXmlNode, std::string strNodePath )
{
	ExTiXmlNode * pXmlNodeTMP;
	std::string strResult;

	if( pXmlNode == NULL )
	{
		return -1;
	}

	if( strNodePath[ strNodePath.length()-1 ] != '~' )
	{
		strNodePath += "~";
	}

	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "ENG" ) != NULL )
	{
		m_mpLanguage[strNodePath + pXmlNode->Value()] = strResult;
		fLog.write( strNodePath + pXmlNode->Value() + " : " + strResult, cLog::eLOG_TYPE_DEBUG );
	}

	pXmlNodeTMP = pXmlNode->XmlGetFirstNodeChild( );
	if( pXmlNodeTMP != NULL )
	{
		this->ReadRecursiveLanguagePack( pXmlNodeTMP , strNodePath + pXmlNode->Value() );
	}

	pXmlNode = pXmlNode->XmlGetNextNodeSibling();
	if( pXmlNode != NULL )
	{
		this->ReadRecursiveLanguagePack( pXmlNode , strNodePath );
	}

	return 0;
}
std::string cLanguage::ReadSingleTranslation( std::string strInput )
{
	std::string strPath, strCurrent, strResult, szErrorMsg;
	TiXmlNode * pXmlNode;
	std::size_t iPosBegin, iPosEnd;

	strPath = strInput;
	if( strPath == "" ) return "";

	pXmlNode = m_XmlDoc.RootElement();
	try
	{
		if( pXmlNode == NULL ) throw std::string("Can't excess root node");
		if( strcmp(pXmlNode->Value(), "MAX_Language_File") != 0 ) throw std::string("Root node mismatch");
	}
	catch( std::string strMsg )
	{
		fLog.write( "Language file (" + m_szLanguage + "): "+ strMsg , cLog::eLOG_TYPE_WARNING );
		return "";
	}
	iPosBegin = 0;
	try
	{
		do
		{
			iPosEnd = strPath.find( '~', iPosBegin + 1);
			strCurrent = strPath.substr(iPosBegin , iPosEnd - iPosBegin );
			if( strCurrent[0] == '~' ) strCurrent.erase(0,1);
			pXmlNode = pXmlNode->FirstChild( strCurrent.c_str() );
			if( pXmlNode == NULL )
				throw i18n("Text~Error_Messages~ERROR_Missing_Translation" , m_mpLanguage[strInput] );

			iPosBegin = iPosEnd;
		}while( iPosBegin != std::string::npos );
	}
	catch( std::string strMsg )
	{
		fLog.write( "Language file (" + m_szLanguage + "): "+ strMsg , cLog::eLOG_TYPE_WARNING );
		return m_mpLanguage[strInput];
	}

	if( ((ExTiXmlNode *)pXmlNode)->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "localized" ) != NULL )
	{
		return strResult;
	}

	szErrorMsg = "Language file: translation for >";
	szErrorMsg += strInput + "< is missing";
	fLog.write( szErrorMsg, cLog::eLOG_TYPE_WARNING );

	return "";
}
