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

#include <vector>
#include "extendedtinyxml.h"
#include "utility/log.h"

using namespace tinyxml2;
using namespace std;

XMLElement* XmlGetFirstElementVa (XMLDocument& xmlDoc, const char* first, va_list vaList)
{
	XMLElement* xmlElement;

	xmlElement = xmlDoc.RootElement();
	if (xmlElement == NULL)
	{
		return NULL;
	}

	if (strcmp (xmlElement->Value(), first) != 0)
	{
		return NULL;
	}

	char* elementName;
	while ((elementName = va_arg (vaList, char*)) != NULL)
	{
		xmlElement = xmlElement->FirstChildElement (elementName);
		if (xmlElement == NULL)
		{
			return NULL;
		}
	}

	return xmlElement;
}
//-------------------------------------------------------------------------------
XMLElement* XmlGetFirstElement (XMLDocument& xmlDoc, const char* first, ...)
{
	va_list list;
	va_start (list, first);

	XMLElement* element = XmlGetFirstElementVa (xmlDoc, first, list);
	va_end (list);

	return element;
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
int getXMLAttributeInt (tinyxml2::XMLDocument& document, const char* first, ...)
{
	va_list list;
	va_start (list, first);
	XMLElement* element = XmlGetFirstElementVa (document, first, list);
	va_end (list);

	if (element == NULL) return 0;


	if (element->Attribute ("Num"))
	{
		return element->IntAttribute ("Num");
	}
	else
	{
		va_start (list, first);
		string pathText = string (first);
		char* elementName;
		while ((elementName = va_arg (list, char*)) != NULL)
			pathText += string ("~") + elementName;
		va_end (list);

		Log.write (((string) "Can't read \"Num\" from \"") + pathText + "\"", cLog::eLOG_TYPE_WARNING);
		return 0;
	}
}

//------------------------------------------------------------------------------
float getXMLAttributeFloat (tinyxml2::XMLDocument& document, const char* first, ...)
{
	va_list list;
	va_start (list, first);
	XMLElement* element = XmlGetFirstElementVa (document, first, list);
	va_end (list);

	if (element == NULL) return 0;

	if (element->Attribute ("Num"))
	{
		return element->FloatAttribute ("Num");
	}
	else
	{
		va_start (list, first);
		string pathText = string (first);
		char* elementName;
		while ((elementName = va_arg (list, char*)) != NULL)
			pathText += string ("~") + elementName;
		va_end (list);

		Log.write (((string) "Can't read \"Num\" from \"") + pathText + "\"", cLog::eLOG_TYPE_WARNING);
		return 0;
	}
}

//------------------------------------------------------------------------------
string getXMLAttributeString (tinyxml2::XMLDocument& document, const char* attribut, const char* first, ...)
{
	va_list list;
	va_start (list, first);
	XMLElement* element = XmlGetFirstElementVa (document, first, list);
	va_end (list);

	if (element == NULL) return "";

	const char* text = element->Attribute (attribut);
	if (text == NULL)
	{
		va_start (list, first);
		string pathText = string (first);
		char* elementName;
		while ((elementName = va_arg (list, char*)) != NULL)
			pathText += string ("~") + elementName;
		va_end (list);

		Log.write (((string) "Can't read \"") + attribut + "\" from \"" + pathText + "\"", cLog::eLOG_TYPE_WARNING);
		return "";
	}

	return text;
}

//------------------------------------------------------------------------------
bool getXMLAttributeBoolFromElement(const tinyxml2::XMLElement* element, const char* name)
{
	string value = element->Attribute(name);
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
	Log.write((string)"Error reading boolen attribute of element \"" + element->Name() + "\": Illegal value \"" + value + "\"", cLog::eLOG_TYPE_WARNING);
	return false;
}
//------------------------------------------------------------------------------
bool getXMLAttributeBool (tinyxml2::XMLDocument& document, const char* first, ...)
{
	va_list list;
	va_start (list, first);
	XMLElement* element = XmlGetFirstElementVa (document, first, list);
	va_end (list);

	if (element == NULL) return false;

	if (element->Attribute("YN"))
	{
		return getXMLAttributeBoolFromElement(element,"YN");
	}
	else
	{
		va_start (list, first);
		string pathText = string (first);
		char* elementName;
		while ((elementName = va_arg (list, char*)) != NULL)
			pathText += string ("~") + elementName;
		va_end (list);

		Log.write (((string) "Can't read \"YN\" from \"") + pathText + "\"", cLog::eLOG_TYPE_WARNING);
		return false;
	}
}

string printXMLPath(const tinyxml2::XMLElement* element)
{
	string path = element->Name();
	while ((element = element->Parent()->ToElement()))
	{
		path = string(element->Name()) + "~" + path;
	}

	return path;
}

string getXMLErrorMsg(const tinyxml2::XMLDocument& document)
{
	string msg;
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
		msg += (std::string)" " + reason;

	if (const char* reason = document.GetErrorStr2())
		msg += (std::string)" " + reason;

	return msg;
}
