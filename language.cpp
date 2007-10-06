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
}

cLanguage::~cLanguage(void)
{
}

std::string cLanguage::GetCurrentLanguage(void)
{
	return NULL;
}

int cLanguage::SetCurrentLanguage(std::string szLanguageCode)
{
	std::string szTemp;
	if( szLanguageCode.length() != 3 )
	{
		return -1;
	}
	m_szLanguage = szLanguageCode;
	m_szLanguageFile = LANGUAGE_FILE_FOLDER ;
	m_szLanguageFile += PATH_DELIMITER;
	m_szLanguageFile += LANGUAGE_FILE_NAME + szLanguageCode + LANGUAGE_FILE_EXT;
	return 0;
}

std::string cLanguage::Translate(std::string szInputText)
{
	return NULL;
}

int cLanguage::ReadLanguagePack(std::string szLanguageCode)
{
	ExTiXmlNode * pXmlNode = NULL;
	std::string strResult;

	// Load the file
	if( !m_XmlDoc.LoadFile( m_szLanguageFile.c_str() ))
	{
		fLog.write( "Can't open language file", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	// Is the main node correct ?
	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE );
	if( pXmlNode == NULL )
	{
		fLog.write( "Language file: missing main node!", cLog::eLOG_TYPE_ERROR );
		return -1;
	}

	// Who is responsible for the file ? (Who is to blame in case of errors?)
	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_HEADER_AUTHOR );
	if( pXmlNode == NULL )
	{
		fLog.write( "Language file: missing author node!", cLog::eLOG_TYPE_WARNING );
	}else
	{
		pXmlNode->XmlGetLastEditor( strResult, pXmlNode );
	}

	// Check the lang attribute of the main node
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "lang" ) == NULL )
	{
		fLog.write( "Language file: language attribut missing! Language can not be identified", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	if( m_szLanguage  != strResult )
	{
		fLog.write( "Language file: language attribut mismatch the selected language", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "lang" ) == NULL )
	{
		fLog.write( "Language file: language attribut 'lang' missing! Language can not be identified", cLog::eLOG_TYPE_ERROR );
		return -1;
	}

	// Writing is left-to-right oder vice versa ?
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "direction" ) == NULL )
	{
		fLog.write( "Language file: language attribut 'direction' is missing! Writing direction will be set to 'Left-To-Right'", cLog::eLOG_TYPE_WARNING );
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
		fLog.write( "Language file: language attribut 'direction' can not interpreted! Writing direction will be set to 'Left-To-Right'", cLog::eLOG_TYPE_WARNING );
		m_bLeftToRight = true;
	}

	pXmlNode = pXmlNode->XmlGetFirstNode( m_XmlDoc, XNP_MAX_LANG_FILE_HEADER_GAMEVERSION );
	if( pXmlNode == NULL )
	{
		fLog.write( "Language file: missing game version node!", cLog::eLOG_TYPE_WARNING );
	}else
	{
		if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "time" ) == NULL )
		{
			fLog.write( "Language file: game version attribute 'time' is missing!", cLog::eLOG_TYPE_WARNING );
		}else
		{
			int iTestResult = pXmlNode->CheckTimeStamp( strResult );
			switch( iTestResult )
			{
				case -1 : 
					fLog.write( "Language file: game version attribute has wrong format!", cLog::eLOG_TYPE_WARNING );
					break;
				case 0 : 
					fLog.write( "Language file may be outdated!", cLog::eLOG_TYPE_WARNING );
					break;
				case 1 :
					fLog.write( "Language file is newer than the game!", cLog::eLOG_TYPE_WARNING );
					break;
				case 2 :
					// Timestamps match
					break;
			}
		}
	}

	// Now - finaly - let's get the translations.
	ReadSingleTranslation( strResult, XNP_MAX_LANG_FILE_TEXT_MAIN, "Game_Title" , "" );

	return 0;
}

int cLanguage::CheckCurrentLanguagePack(bool bInsertMissingEntries)
{
	return 0;
}

int cLanguage::ReadSingleTranslation( std::string & strResult, const char * pszCurrent, ... )
{
	va_list pvaArg;
	va_start(pvaArg, pszCurrent);

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
			if( pszCurrent != "" )
			{
				szXmlNodePath += "_";
				szXmlNodePath += pszCurrent;
				pXmlNode = pXmlNode->FirstChild( pszCurrent );
				if( pXmlNode == NULL )
				{
					break;
				}
			}
		}while( pszCurrent != "" );
		break;
	}
	szXmlNodePath.erase(0,1);

	if( pXmlNode != NULL )
	{
		if( ((ExTiXmlNode *)pXmlNode)->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "localized" ) != NULL )
		{
			va_end( pvaArg );
			m_mpLanguage[szXmlNodePath] = strResult;
			return 0;
		}
	}

	std::string szErrorMsg = "Language file: translation for >";
	if( pXmlNode != NULL )
	{
		if( ((ExTiXmlNode *)pXmlNode)->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "ENG" ) != NULL )
		{
			m_mpLanguage[szXmlNodePath] = strResult;
		}else
		{
			m_mpLanguage[szXmlNodePath] = szXmlNodePath;
		}
	}
	szErrorMsg += strResult + "< is missing";
	fLog.write( szErrorMsg.c_str(), cLog::eLOG_TYPE_WARNING );
	va_end( pvaArg );
	return -1;
}

