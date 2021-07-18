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
#define serialization_xmlarchiveH

#include "serialization.h"

#include <3rd/tinyxml2/tinyxml2.h>

#include <cassert>
#include <cstdlib>
#include <typeinfo>

class cXmlArchiveIn
{
public:
	cXmlArchiveIn (tinyxml2::XMLElement& rootElement);

	static const bool isWriter = true;

	//--------------------------------------------------------------------------
	template <typename T>
	cXmlArchiveIn& operator<< (const serialization::sNameValuePair<T>& nvp)
	{
		pushValue (nvp);
		return *this;
	}
	//--------------------------------------------------------------------------
	template <typename T>
	cXmlArchiveIn& operator& (const serialization::sNameValuePair<T>& nvp)
	{
		pushValue (nvp);
		return *this;
	}

	void openNewChild (const std::string& name);
	void closeChild();

	serialization::cPointerLoader* getPointerLoader() const { return nullptr; }
private:
	tinyxml2::XMLElement& buffer;

	tinyxml2::XMLElement* currentElement;

	// xml handling methods
	void addToCurrentElement (const std::string& name, const std::string& value);
	void convertAttributeToChild (const std::string& name);

	//--------------------------------------------------------------------------
	template <typename T, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
	void pushValue (const serialization::sNameValuePair<T>& nvp)
	{
		//check invalid characters in element and attribute names
		assert (nvp.name.find_first_of ("<>\"= []?!&") == std::string::npos);

		if (std::is_class<T>::value)
			openNewChild (nvp.name);

		serialize (*this, nvp);

		if (std::is_class<T>::value)
			closeChild();
	}

	//--------------------------------------------------------------------------
	template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
	void pushValue (const serialization::sNameValuePair<E>& nvp)
	{
		auto tmp = serialization::sEnumSerializer<E>::toString (nvp.value);
		pushValue (serialization::makeNvp (nvp.name, tmp));
	}

	//
	// push fundamental types
	//
	void pushValue (const serialization::sNameValuePair<bool>& nvp);
	void pushValue (const serialization::sNameValuePair<char>& nvp);
	void pushValue (const serialization::sNameValuePair<signed char>& nvp);
	void pushValue (const serialization::sNameValuePair<unsigned char>& nvp);
	void pushValue (const serialization::sNameValuePair<signed short>& nvp);
	void pushValue (const serialization::sNameValuePair<unsigned short>& nvp);
	void pushValue (const serialization::sNameValuePair<signed int>& nvp);
	void pushValue (const serialization::sNameValuePair<unsigned int>& nvp);
	void pushValue (const serialization::sNameValuePair<signed long>& nvp);
	void pushValue (const serialization::sNameValuePair<unsigned long>& nvp);
	void pushValue (const serialization::sNameValuePair<signed long long>& nvp);
	void pushValue (const serialization::sNameValuePair<unsigned long long>& nvp);
	void pushValue (const serialization::sNameValuePair<float>& nvp);
	void pushValue (const serialization::sNameValuePair<double>& nvp);

	//
	// push STL types
	//
	void pushValue (const serialization::sNameValuePair<std::string>& nvp);
};

class cXmlArchiveOut
{
public:
	cXmlArchiveOut (const tinyxml2::XMLElement& rootElement, serialization::cPointerLoader* pointerLoader = nullptr);

	static const bool isWriter = false;

	//--------------------------------------------------------------------------
	template <typename T>
	cXmlArchiveOut& operator>> (const serialization::sNameValuePair<T>& nvp)
	{
		popValue (nvp);
		return *this;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	cXmlArchiveOut& operator& (const serialization::sNameValuePair<T>& nvp)
	{
		popValue (nvp);
		return *this;
	}

	serialization::cPointerLoader* getPointerLoader() const;

	void enterChild (const std::string& name);
	void leaveChild();

private:
	const tinyxml2::XMLElement& buffer;

	const tinyxml2::XMLElement* currentElement;
	std::vector<const tinyxml2::XMLElement*> visitedElements;

	serialization::cPointerLoader* pointerLoader;

	// xml handling methods
	template <typename T>
	void getFromCurrentElement (const serialization::sNameValuePair<T>& nvp);
	std::string getStringFromCurrentElement (const std::string& name);

	//------------------------------------------------------------------------------
	template <typename T, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
	void popValue (const serialization::sNameValuePair<T>& nvp)
	{
		if (std::is_class<T>::value)
			enterChild (nvp.name);

		serialize (*this, nvp);

		if (std::is_class<T>::value)
			leaveChild();
	}

	//------------------------------------------------------------------------------
	template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
	void popValue (const serialization::sNameValuePair<E>& nvp)
	{
		static_assert(sizeof (E) <= sizeof (int), "!");
		std::string tmp = "";
		popValue (serialization::makeNvp (nvp.name, tmp));
		// accept both the number or the string.
		char* end = nullptr;
		const int n = std::strtol (tmp.c_str(), &end, 10);
		if (end == tmp.c_str() + tmp.size())
		{
			nvp.value = static_cast<E> (n);
		}
		else
		{
			nvp.value = serialization::sEnumSerializer<E>::fromString (tmp);
		}
	}

	//
	// pop fundamental types
	//
	void popValue (const serialization::sNameValuePair<bool>& nvp);
	void popValue (const serialization::sNameValuePair<char>& nvp);
	void popValue (const serialization::sNameValuePair<signed char>& nvp);
	void popValue (const serialization::sNameValuePair<unsigned char>& nvp);
	void popValue (const serialization::sNameValuePair<signed short>& nvp);
	void popValue (const serialization::sNameValuePair<unsigned short>& nvp);
	void popValue (const serialization::sNameValuePair<signed int>& nvp);
	void popValue (const serialization::sNameValuePair<unsigned int>& nvp);
	void popValue (const serialization::sNameValuePair<signed long>& nvp);
	void popValue (const serialization::sNameValuePair<unsigned long>& nvp);
	void popValue (const serialization::sNameValuePair<signed long long>& nvp);
	void popValue (const serialization::sNameValuePair<unsigned long long>& nvp);
	void popValue (const serialization::sNameValuePair<float>& nvp);
	void popValue (const serialization::sNameValuePair<double>& nvp);

	//
	// pop STL types
	//
	void popValue (const serialization::sNameValuePair<std::string>& nvp);
};

#endif
