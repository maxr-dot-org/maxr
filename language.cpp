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

extern cLog fLog;

cLanguage::cLanguage(void)
: m_szLanguage(NULL)
{
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
	return 0;
}

std::string cLanguage::Translate(std::string szInputText)
{
	return NULL;
}

int cLanguage::ReadLanguagePack(std::string szLanguageCode)
{
	TiXmlDocument xmlDoc;
	TiXmlNode * xmlRoot;
	TiXmlNode * xmlNode;

	if( !xmlDoc.LoadFile( m_szLanguageFile.data() ))
	{
		fLog.write( "Can't open language file", LOG_TYPE_ERROR );
		return 0;
	}
	xmlRoot = xmlDoc.FirstChildElement("MAX_Language_File");


//	FastMode=GetXMLBool(rootnode,"fastmode");



	return 0;
}

int cLanguage::CheckCurrentLanguagePack(bool bInsertMissingEntries)
{
	return 0;
}
