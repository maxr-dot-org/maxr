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

#include <string>
#include <tinyxml.h>
#include "log.h"
#include "language.h"
#include "ExtendedTinyXml.h"
#include "defines.h"

extern cLog fLog;

cLanguage::cLanguage(void)
{
	m_szLanguage = "";
	m_szLanguageFile = "";
	m_szEncoding = "";
	m_bLeftToRight = true;
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
	TiXmlDocument XmlDoc;
	ExTiXmlNode * pXmlNode = NULL;
	std::string strResult;

	if( !XmlDoc.LoadFile( m_szLanguageFile.c_str() ))
	{
		fLog.write( "Can't open language file", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, XNP_MAX_LANG_FILE );
	if( pXmlNode == NULL )
	{
		fLog.write( "Language file: missing main node!", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, XNP_MAX_LANG_FILE_HEADER_AUTHOR );
	if( pXmlNode == NULL )
	{
		fLog.write( "Language file: missing main node!", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	pXmlNode->XmlGetLastEditor( strResult, pXmlNode );




	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "lang" ) == NULL )
	{
		fLog.write( "Language file: language attribut missing! Language can not be identified", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	if( m_szLanguage  != strResult )
	{
		fLog.write( "Language file: language attribut missing! Language can not be identified", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "lang" ) == NULL )
	{
		fLog.write( "Language file: language attribut 'lang' missing! Language can not be identified", cLog::eLOG_TYPE_ERROR );
		return -1;
	}
	if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "direction" ) == NULL )
	{
		fLog.write( "Language file: language attribut 'direction' missing! Writing direction will be set to 'Left-To-Right'", cLog::eLOG_TYPE_WARNING );
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


	return 0;
}

int cLanguage::CheckCurrentLanguagePack(bool bInsertMissingEntries)
{
	return 0;
}
