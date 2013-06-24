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
//  File:   ExtendedTinyXml.cpp
//  Date:   07-10-01
//  Author: JCK
//
////////////////////////////////////////////////////////////////////////////////
//  Description:
//  Improves the TinyXML family by adding ExTiXmlNode. This class is a bid more
//  user-friendly.
//
//	Example for usage is added in "ExtendedTinyXml.h"
//
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include "extendedtinyxml.h"
#include "defines.h"
#include "log.h"
#include "tinyxml2.h"

using namespace tinyxml2;

#if 0
void debugToLog (const std::string& szMsg);
void debugToLog (void* pointer , const char* pname);
#endif

ExTiXmlNode* ExTiXmlNode::XmlGetFirstNode (TiXmlDocument& rTiXmlDoc, const char* pszCurrent, ...)
{
	va_list pvaArg;
	va_start (pvaArg, pszCurrent);

	TiXmlNode* pXmlNode;

	if (rTiXmlDoc.Value() == NULL)
	{
		va_end (pvaArg);
		return NULL;
	}

	pXmlNode = rTiXmlDoc.RootElement();
	if (pXmlNode == NULL)
	{
		va_end (pvaArg);
		return NULL;
	}

	if (strcmp (pXmlNode->Value(), pszCurrent) != 0)
	{
		va_end (pvaArg);
		return NULL;
	}

	do
	{
		pszCurrent = va_arg (pvaArg, char*);
		if (pszCurrent != NULL)
		{
			pXmlNode = pXmlNode->FirstChild (pszCurrent);
			if (pXmlNode == NULL)
			{
				va_end (pvaArg);
				return NULL;
			}
		}
	}
	while (pszCurrent != NULL);

	return (ExTiXmlNode*) pXmlNode;
}

ExTiXmlNode* ExTiXmlNode::XmlGetFirstNodeChild()
{
	TiXmlNode* pXmlNode;
	if (this == NULL)
	{
		return NULL;
	}
	pXmlNode = this;

	pXmlNode = pXmlNode->FirstChild();

	return (ExTiXmlNode*) pXmlNode;
}

ExTiXmlNode* ExTiXmlNode::XmlGetNextNodeSibling()
{
	TiXmlNode* pXmlNode;
	if (this == NULL)
	{
		return NULL;
	}
	pXmlNode = this;

	pXmlNode = pXmlNode->NextSibling();

	return (ExTiXmlNode*) pXmlNode;
}


ExTiXmlNode* ExTiXmlNode::XmlReadNodeData (std::string& rstrData, XML_NODE_TYPE eType,  const char* pszAttributeName)
{
	TiXmlNode* pXmlNode = (TiXmlNode*) this;
	TiXmlElement* pXmlElement;
	const char* pszTemp;

	rstrData = "";

	if (this == NULL)
	{
		return NULL;
	}
	switch (eType)
	{
		case ExTiXmlNode::eXML_ATTRIBUTE :
			if (pXmlNode->Type() != TiXmlNode::TINYXML_ELEMENT) return NULL;
			pXmlElement = pXmlNode->ToElement();// FirstChildElement();
			if (pXmlElement == NULL)
			{
				return NULL;
			}
			pszTemp =  pXmlElement->Attribute (pszAttributeName);
			if (pszTemp == 0)
			{
				return NULL;
			}
			else
			{
				rstrData = pszTemp;
			}
			break;
		case ExTiXmlNode::eXML_COMMENT :
			return NULL;
			break;
		case ExTiXmlNode::eXML_TEXT :
			return NULL;
			break;
		default :
			return NULL;
	}
	return (ExTiXmlNode*) pXmlNode;
}

int ExTiXmlNode::XmlGetLastEditor (std::string& rstrData, ExTiXmlNode* pXmlAuthorNode)
{
	rstrData = "";

	// ToDo - JCK: Find the last editor of the XML file
	return 0;
}

bool ExTiXmlNode::XmlDataToBool (std::string& rstrData)
{
	// Default value = true !!!

	// is it a number ?
	if (rstrData.find_first_not_of (" 01234567890,.+-") == rstrData.npos)
	{
		if (atoi (rstrData.c_str()) == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else  // no number ! only first letter is important !
	{
		std::string szTemp = rstrData;
		while (szTemp[0] == ' ')
		{
			szTemp.erase (0);
		}
		if (szTemp[0] == 'f' || szTemp[0] == 'F' || szTemp[0] == 'n' || szTemp[0] == 'N')
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

XMLElement* XmlGetFirstElement (XMLDocument& xmlDoc, const char* rootElement, ...)
{
	va_list vaList;
	va_start (vaList, rootElement);

	XMLElement* xmlElement;

	xmlElement = xmlDoc.RootElement();
	if (xmlElement == NULL)
	{
		va_end (vaList);
		return NULL;
	}

	if (strcmp (xmlElement->Value(), rootElement) != 0)
	{
		va_end (vaList);
		return NULL;
	}

	char* elementName;
	while ((elementName = va_arg (vaList, char*)) != NULL)
	{
		xmlElement = xmlElement->FirstChildElement (elementName);
		if (xmlElement == NULL)
		{
			va_end (vaList);
			return NULL;
		}
	}

	va_end (vaList);
	return xmlElement;
}

//------------------------------------------------------------------------------
XMLElement* getOrCreateXmlElement (XMLDocument& xmlDoc, const std::string& path)
{
	std::vector<std::string> parts;
	size_t i = 0, j;
	do
	{
		j = path.find ('~', i);
		if (j == std::string::npos) j = path.length();
		parts.push_back (path.substr (i, j - i));
		i = j + 1;
	}
	while (j != path.length());

	XMLElement* xmlElement = NULL;
	XMLElement* lastElement = xmlDoc.FirstChildElement(parts[0].c_str());
	if (lastElement == NULL)
		lastElement = xmlDoc.LinkEndChild (xmlDoc.NewElement(parts[0].c_str()))->ToElement();

	for (unsigned i = 1; i < parts.size(); ++i)
	{
		xmlElement = lastElement->FirstChildElement (parts[i].c_str());
		if (xmlElement == NULL) 
			xmlElement = lastElement->LinkEndChild (xmlDoc.NewElement (parts[i].c_str()))->ToElement();
		lastElement = xmlElement;
	}

	return xmlElement;
}

