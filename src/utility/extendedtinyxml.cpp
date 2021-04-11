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

#include "extendedtinyxml.h"

#include "utility/log.h"

#include <algorithm>
#include <vector>

using namespace tinyxml2;

namespace
{
	//--------------------------------------------------------------------------
	void LogWarning (const char* attribute, const char* root, std::initializer_list<const char*> elementNames)
	{
		std::string pathText = root;
		for (const auto* elementName : elementNames)
		{
			pathText += std::string ("~") + elementName;
		}

		Log.write (std::string ("Can't read \"") + attribute + "\" from \"" + pathText + "\"", cLog::eLOG_TYPE_WARNING);
	}
}

//------------------------------------------------------------------------------
XMLElement* XmlGetFirstElement (XMLDocument& xmlDoc, const char* root, std::initializer_list<const char*> elementNames)
{
	XMLElement* xmlElement = xmlDoc.RootElement();

	if (xmlElement == nullptr) return nullptr;
	if (strcmp (xmlElement->Value(), root) != 0) return nullptr;

	for (const char* elementName : elementNames)
	{
		xmlElement = xmlElement->FirstChildElement (elementName);
		if (xmlElement == nullptr)
		{
			return nullptr;
		}
	}
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
	XMLElement* lastElement = xmlDoc.FirstChildElement (parts[0].c_str());
	if (lastElement == NULL)
		lastElement = xmlDoc.LinkEndChild (xmlDoc.NewElement (parts[0].c_str()))->ToElement();

	for (unsigned i = 1; i < parts.size(); ++i)
	{
		xmlElement = lastElement->FirstChildElement (parts[i].c_str());
		if (xmlElement == NULL)
			xmlElement = lastElement->LinkEndChild (xmlDoc.NewElement (parts[i].c_str()))->ToElement();
		lastElement = xmlElement;
	}

	return xmlElement;
}


//------------------------------------------------------------------------------
int getXMLAttributeInt (tinyxml2::XMLDocument& document, const char* root, std::initializer_list<const char*> elementNames)
{
	XMLElement* element = XmlGetFirstElement (document, root, elementNames);

	if (element == nullptr) return 0;

	const char* attribut = "Num";
	if (element->Attribute (attribut))
	{
		return element->IntAttribute (attribut);
	}
	else
	{
		LogWarning (attribut, root, elementNames);
		return 0;
	}
}

//------------------------------------------------------------------------------
float getXMLAttributeFloat (tinyxml2::XMLDocument& document, const char* root, std::initializer_list<const char*> elementNames)
{
	XMLElement* element = XmlGetFirstElement (document, root, elementNames);

	if (element == nullptr) return 0;

	const char* attribut = "Num";
	if (element->Attribute (attribut))
	{
		return element->FloatAttribute (attribut);
	}
	else
	{
		LogWarning (attribut, root, elementNames);
		return 0;
	}
}

//------------------------------------------------------------------------------
std::string getXMLAttributeString (tinyxml2::XMLDocument& document, const char* attribut, const char* root, std::initializer_list<const char*> elementNames)
{
	XMLElement* element = XmlGetFirstElement (document, root, elementNames);

	if (element == nullptr) return {};

	if (const char* text = element->Attribute (attribut))
	{
		return text;
	}
	else
	{
		LogWarning (attribut, root, elementNames);
		return "";
	}
}

//------------------------------------------------------------------------------
bool getXMLAttributeBoolFromElement(const tinyxml2::XMLElement* element, const char* name)
{
	std::string value = element->Attribute(name);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (value == "true" ||
		value == "y" ||
		value == "yes")
	{
		return true;
	}
	else if (value == "false" ||
		value == "n" ||
		value == "no")
	{
		return false;
	}
	Log.write(std::string("Error reading boolean attribute of element \"") + element->Name() + "\": Illegal value \"" + value + "\"", cLog::eLOG_TYPE_WARNING);
	return false;
}
//------------------------------------------------------------------------------
bool getXMLAttributeBool (tinyxml2::XMLDocument& document, const char* root, std::initializer_list<const char*> elementNames)
{
	XMLElement* element = XmlGetFirstElement (document, root, elementNames);

	if (element == nullptr) return false;

	const char* attribut = "YN";
	if (element->Attribute (attribut))
	{
		return getXMLAttributeBoolFromElement (element, attribut);
	}
	else
	{
		LogWarning (attribut, root, elementNames);
		return false;
	}
}

std::string printXMLPath(const tinyxml2::XMLElement* element)
{
	std::string path = element->Name();
	while ((element = element->Parent()->ToElement()))
	{
		path = std::string(element->Name()) + "~" + path;
	}

	return path;
}

std::string getXMLErrorMsg(const tinyxml2::XMLDocument& document)
{
	std::string msg;
	switch (document.ErrorID())
	{
		case XML_NO_ERROR: msg = "XML_NO_ERROR"; break;
		case XML_NO_ATTRIBUTE: msg = "XML_NO_ATTRIBUTE"; break;
		case XML_WRONG_ATTRIBUTE_TYPE: msg = "XML_WRONG_ATTRIBUTE_TYPE"; break;
		case XML_ERROR_FILE_NOT_FOUND: msg = "XML_ERROR_FILE_NOT_FOUND"; break;
		case XML_ERROR_FILE_COULD_NOT_BE_OPENED: msg = "XML_ERROR_FILE_COULD_NOT_BE_OPENED"; break;
		case XML_ERROR_FILE_READ_ERROR: msg = "XML_ERROR_FILE_READ_ERROR"; break;
		case XML_ERROR_ELEMENT_MISMATCH: msg = "XML_ERROR_ELEMENT_MISMATCH"; break;
		case XML_ERROR_PARSING_ELEMENT: msg = "XML_ERROR_PARSING_ELEMENT"; break;
		case XML_ERROR_PARSING_ATTRIBUTE: msg = "XML_ERROR_PARSING_ATTRIBUTE"; break;
		case XML_ERROR_IDENTIFYING_TAG: msg = "XML_ERROR_IDENTIFYING_TAG"; break;
		case XML_ERROR_PARSING_TEXT: msg = "XML_ERROR_PARSING_TEXT"; break;
		case XML_ERROR_PARSING_CDATA: msg = "XML_ERROR_PARSING_CDATA"; break;
		case XML_ERROR_PARSING_COMMENT: msg = "XML_ERROR_PARSING_COMMENT"; break;
		case XML_ERROR_PARSING_DECLARATION: msg = "XML_ERROR_PARSING_DECLARATION"; break;
		case XML_ERROR_PARSING_UNKNOWN: msg = "XML_ERROR_PARSING_UNKNOWN"; break;
		case XML_ERROR_EMPTY_DOCUMENT: msg = "XML_ERROR_EMPTY_DOCUMENT"; break;
		case XML_ERROR_MISMATCHED_ELEMENT: msg = "XML_ERROR_MISMATCHED_ELEMENT"; break;
		case XML_ERROR_PARSING: msg = "XML_ERROR_PARSING"; break;
		case XML_CAN_NOT_CONVERT_TEXT: msg = "XML_CAN_NOT_CONVERT_TEXT"; break;
		case XML_NO_TEXT_NODE: msg = "XML_NO_TEXT_NODE"; break;
		default: msg = "Unknown error";	break;
	}

	if (const char* reason = document.GetErrorStr1())
		msg += std::string(" ") + reason;

	if (const char* reason = document.GetErrorStr2())
		msg += std::string(" ") + reason;

	return msg;
}
