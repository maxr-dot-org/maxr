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
#include "log.h"
#include "tinyxml2.h"

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
bool getXMLAttributeBool (tinyxml2::XMLDocument& document, const char* first, ...)
{
	va_list list;
	va_start (list, first);
	XMLElement* element = XmlGetFirstElementVa (document, first, list);
	va_end (list);

	if (element == NULL) return false;


	if (element->Attribute ("YN"))
	{
		return element->BoolAttribute ("YN");
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
