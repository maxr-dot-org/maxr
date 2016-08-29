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

#ifndef serialization_xmlarchiveH
#define  serialization_xmlarchiveH

#include "tinyxml2.h"

#include "serialization.h"

class serialization::cPointerLoader;


class cXmlArchiveIn
{
public:
	cXmlArchiveIn(tinyxml2::XMLElement& rootElement);

	static const bool isWriter = true;

	template<typename T>
	cXmlArchiveIn& operator<<(const serialization::sNameValuePair<T>& nvp);
	template<typename T>
	cXmlArchiveIn& operator&(const serialization::sNameValuePair<T>& nvp);

	void openNewChild(const std::string& name);
	void closeChild();

	serialization::cPointerLoader* getPointerLoader() const { return nullptr; };
private:
	tinyxml2::XMLElement& buffer;

	tinyxml2::XMLElement* currentElement;

	// xml handling methods
	void addToCurrentElement(const std::string& name, const std::string& value);
	void convertAttributeToChild(const std::string& name);

	
	template<typename T>
	void pushValue(const serialization::sNameValuePair<T>& nvp);

	//
	// push fundamental types
	//
	void pushValue(const serialization::sNameValuePair<bool>& nvp);
	void pushValue(const serialization::sNameValuePair<char>& nvp);
	void pushValue(const serialization::sNameValuePair<signed char>& nvp);
	void pushValue(const serialization::sNameValuePair<unsigned char>& nvp);
	void pushValue(const serialization::sNameValuePair<signed short>& nvp);
	void pushValue(const serialization::sNameValuePair<unsigned short>& nvp);
	void pushValue(const serialization::sNameValuePair<signed int>& nvp);
	void pushValue(const serialization::sNameValuePair<unsigned int>& nvp);
	void pushValue(const serialization::sNameValuePair<signed long>& nvp);
	void pushValue(const serialization::sNameValuePair<unsigned long>& nvp);
	void pushValue(const serialization::sNameValuePair<signed long long>& nvp);
	void pushValue(const serialization::sNameValuePair<unsigned long long>& nvp);
	void pushValue(const serialization::sNameValuePair<float>& nvp);
	void pushValue(const serialization::sNameValuePair<double>& nvp);

	//
	// push STL types
	//
	void pushValue(const serialization::sNameValuePair<std::string>& nvp);
};

class cXmlArchiveOut
{
public:
	cXmlArchiveOut(const tinyxml2::XMLElement& rootElement, serialization::cPointerLoader* pointerLoader = NULL);

	static const bool isWriter = false;

	template<typename T>
	cXmlArchiveOut& operator>>(serialization::sNameValuePair<T>& nvp);
	template<typename T>
	cXmlArchiveOut& operator&(serialization::sNameValuePair<T>& nvp);

	serialization::cPointerLoader* getPointerLoader() const;

	void enterChild(const std::string& name);
	void leaveChild();

private:
	const tinyxml2::XMLElement& buffer;

	const tinyxml2::XMLElement* currentElement;
	std::vector<const tinyxml2::XMLElement*> visitedElements;

	serialization::cPointerLoader* pointerLoader;

	// xml handling methods
	template <typename T>
	void getFromCurrentElement(serialization::sNameValuePair<T>& nvp);
	std::string getStringFromCurrentElement(const std::string& name);

	template<typename T>
	void popValue(serialization::sNameValuePair<T>& nvp);

	//
	// pop fundamental types
	//
	void popValue(serialization::sNameValuePair<bool>& nvp);
	void popValue(serialization::sNameValuePair<char>& nvp);
	void popValue(serialization::sNameValuePair<signed char>& nvp);
	void popValue(serialization::sNameValuePair<unsigned char>& nvp);
	void popValue(serialization::sNameValuePair<signed short>& nvp);
	void popValue(serialization::sNameValuePair<unsigned short>& nvp);
	void popValue(serialization::sNameValuePair<signed int>& nvp);
	void popValue(serialization::sNameValuePair<unsigned int>& nvp);
	void popValue(serialization::sNameValuePair<signed long>& nvp);
	void popValue(serialization::sNameValuePair<unsigned long>& nvp);
	void popValue(serialization::sNameValuePair<signed long long>& nvp);
	void popValue(serialization::sNameValuePair<unsigned long long>& nvp);
	void popValue(serialization::sNameValuePair<float>& nvp);
	void popValue(serialization::sNameValuePair<double>& nvp);

	//
	// pop STL types
	//
	void popValue(serialization::sNameValuePair<std::string>& nvp);
};
//------------------------------------------------------------------------------
template<typename T>
cXmlArchiveIn& cXmlArchiveIn::operator<<(const serialization::sNameValuePair<T>& nvp)
{
	pushValue(nvp);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T>
cXmlArchiveIn& cXmlArchiveIn::operator&(const serialization::sNameValuePair<T>& nvp)
{
	pushValue(nvp);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T>
void cXmlArchiveIn::pushValue(const serialization::sNameValuePair<T>& nvp)
{
	//check invalid characters in element and attribute names
	assert(nvp.name.find_first_of("<>\"= []?!&") == std::string::npos);

	if (std::is_class<T>::value)
		openNewChild(nvp.name);

	serialization::sNameValuePair<T>& valueNonConst = const_cast<serialization::sNameValuePair<T>&>(nvp);
	serialize(*this, valueNonConst);
	
	if (std::is_class<T>::value)
		closeChild();
}
//------------------------------------------------------------------------------
template<typename T>
cXmlArchiveOut& cXmlArchiveOut::operator>>(serialization::sNameValuePair<T>& nvp)
{
	popValue(nvp);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T>
cXmlArchiveOut& cXmlArchiveOut::operator&(serialization::sNameValuePair<T>& nvp)
{
	popValue(nvp);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T>
void cXmlArchiveOut::getFromCurrentElement(serialization::sNameValuePair<T>& nvp)
{
	std::string value = getStringFromCurrentElement(nvp.name);

	std::stringstream ss(value);
	ss.imbue(std::locale("C"));
	ss >> nvp.value;

	if (ss.fail() || !ss.eof()) //test eof, because all characters in the string should belong to the converted value
		throw std::runtime_error("Could not convert value of node " + printXMLPath(currentElement) + "~" + nvp.name + " to " + typeid(T).name());

}
//------------------------------------------------------------------------------
template<typename T>
void cXmlArchiveOut::popValue(serialization::sNameValuePair<T>& nvp)
{
	if (std::is_class<T>::value)
		enterChild(nvp.name);

	serialize(*this, nvp);

	if (std::is_class<T>::value)
		leaveChild();
}

#endif
