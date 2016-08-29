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
#include "xmlarchive.h"
#include "utility/listhelpers.h"
#include "utility/string/toString.h"


cXmlArchiveIn::cXmlArchiveIn(tinyxml2::XMLElement& rootElement) :
	buffer(rootElement),
	currentElement(&rootElement)
{}
//------------------------------------------------------------------------------
void cXmlArchiveIn::convertAttributeToChild(const std::string& name)
{
	const char* value = currentElement->Attribute(name.c_str());

	openNewChild(name);
	currentElement->SetText(value);
	closeChild();

	currentElement->DeleteAttribute(name.c_str());
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::addToCurrentElement(const std::string& name, const std::string& value)
{
	// This method tries to add a value as attribute. If there is more then one value
	// with the same name (e. g. a vector), the value needs to be stored as child.
	
	if (currentElement->Attribute(name.c_str()))
	{
		convertAttributeToChild(name);
	}

	if (currentElement->FirstChildElement(name.c_str()))
	{
		openNewChild(name.c_str());
		currentElement->SetText(value.c_str());
		closeChild();
	}
	else
	{
		currentElement->SetAttribute(name.c_str(), value.c_str());
	}
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::openNewChild(const std::string& name)
{
	tinyxml2::XMLElement* child = buffer.GetDocument()->NewElement(name.c_str());
	currentElement->LinkEndChild(child);
	currentElement = child;
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::closeChild()
{
	currentElement = currentElement->Parent()->ToElement();
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<bool>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<char>& nvp)
{
	addToCurrentElement(nvp.name, toString(static_cast<int>(nvp.value)));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<signed char>& nvp)
{
	addToCurrentElement(nvp.name, toString(static_cast<int>(nvp.value)));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<unsigned char>& nvp)
{
	addToCurrentElement(nvp.name, toString(static_cast<int>(nvp.value)));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<signed short>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<unsigned short>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<signed int>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<unsigned int>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<signed long>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<unsigned long>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<signed long long>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<unsigned long long>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<float>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<double>& nvp)
{
	addToCurrentElement(nvp.name, toString(nvp.value));
}
//------------------------------------------------------------------------------
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<std::string>& nvp)
{
	std::string value = nvp.value;

	//replace line breaks in string by "\n"
	size_t pos = 0;
	while ((pos = value.find('\n', pos)) != std::string::npos)
	{
		value.replace(pos, 1, "\\n");
		pos += 2;
	}

	addToCurrentElement(nvp.name, value);
}

//------------------------------------------------------------------------------
cXmlArchiveOut::cXmlArchiveOut(const tinyxml2::XMLElement& rootElement, serialization::cPointerLoader* pointerLoader) :
	buffer(rootElement),
	currentElement(&rootElement),
	pointerLoader(pointerLoader)
{}
//------------------------------------------------------------------------------
void cXmlArchiveOut::enterChild(const std::string& name)
{
	const tinyxml2::XMLElement* nextElement;
	nextElement = currentElement->FirstChildElement(name.c_str());

	while (Contains(visitedElements, nextElement))
	{
		nextElement = nextElement->NextSiblingElement(name.c_str());
	}

	if (nextElement == nullptr)
		throw std::runtime_error("Attribute or element not found: " + printXMLPath(currentElement) + "~" + name);

	visitedElements.push_back(nextElement);
	currentElement = nextElement;
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::leaveChild()
{
	currentElement = currentElement->Parent()->ToElement();
}
//------------------------------------------------------------------------------
serialization::cPointerLoader* cXmlArchiveOut::getPointerLoader() const
{
	return pointerLoader;
}
//------------------------------------------------------------------------------
std::string cXmlArchiveOut::getStringFromCurrentElement(const std::string& name)
{
	std::string value;
	if (currentElement->FindAttribute(name.c_str()))
	{
		if (currentElement->FirstChildElement(name.c_str()))
			throw std::runtime_error(std::string("XML Element \"") + currentElement->Name() + "\" has an attribure and child with the same name");

		value = currentElement->Attribute(name.c_str());
	}
	else
	{
		enterChild(name.c_str());
		value = currentElement->GetText();
		leaveChild();
	}
	return value;
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<bool>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<char>& nvp)
{
	auto tmp = serialization::makeNvp(nvp.name, (int)nvp.value);
	getFromCurrentElement(tmp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<signed char>& nvp)
{
	auto tmp = serialization::makeNvp(nvp.name, (int)nvp.value);
	getFromCurrentElement(tmp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<unsigned char>& nvp)
{
	auto tmp = serialization::makeNvp(nvp.name, (int)nvp.value);
	getFromCurrentElement(tmp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<signed short>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<unsigned short>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<signed int>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<unsigned int>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<signed long>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<unsigned long>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<signed long long>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<unsigned long long>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<float>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<double>& nvp)
{
	getFromCurrentElement(nvp);
}
//------------------------------------------------------------------------------
void cXmlArchiveOut::popValue(serialization::sNameValuePair<std::string>& nvp)
{
	nvp.value = getStringFromCurrentElement(nvp.name);
	//replace "\n" in string by real line breaks
	size_t pos = 0;
	while ((pos = nvp.value.find("\\n", pos)) != std::string::npos)
	{
		nvp.value.replace(pos, 2, "\n");
		pos += 1;
	}
}
