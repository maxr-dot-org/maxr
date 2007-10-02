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
#include "language.h"
#include <tinyxml.h>


cLanguage::cLanguage(void)
: szLanguage(NULL)
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
	TiXmlDocument doc;
	TiXmlNode* rootnode;
	if(!doc.LoadFile("max.xml")){
		MessageBoxA(NULL,"max.xml kann nicht geladen werden!","Error",MB_ICONERROR);
		return 0;
	}
	rootnode = doc.FirstChildElement("MAXOptions")->FirstChildElement("StartOptions");

	FastMode=GetXMLBool(rootnode,"fastmode");;



	return 0;
}

int cLanguage::CheckCurrentLanguagePack(bool bInsertMissingEntries)
{
	return 0;
}
