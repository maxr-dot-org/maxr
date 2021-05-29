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


#ifndef serialization_textarchiveH
#define serialization_textarchiveH

#include <sstream>

#include "serialization.h"

class cTextArchiveIn
{
public:
	cTextArchiveIn();

	static const bool isWriter = true;

	template<typename T>
	cTextArchiveIn& operator<<(const T& value);
	template<typename T>
	cTextArchiveIn& operator&(const T& value);

	const std::string data() const;
private:
	std::stringstream buffer;
	bool nextCommaNeeded;

	void openBracket();
	void closeBracket();
	void addComma();

	template<typename T, typename = std::enable_if_t <!std::is_enum<T>::value>>
	void pushValue(const T& value);
	template<typename T>
	void pushValue(const serialization::sNameValuePair<T>& nvp);


	//
	// push fundamental types
	//
	void pushValue(bool value);
	void pushValue(char value);
	void pushValue(signed char value);
	void pushValue(unsigned char value);
	void pushValue(signed short value);
	void pushValue(unsigned short value);
	void pushValue(signed int value);
	void pushValue(unsigned int value);
	void pushValue(signed long value);
	void pushValue(unsigned long value);
	void pushValue(signed long long value);
	void pushValue(unsigned long long value);
	void pushValue(float value);
	void pushValue(double value);

	//
	// push types that need a special handling in text archives
	//
	void pushValue(const std::string& value);
	template <typename T, typename = std::enable_if_t <std::is_enum<T>::value>>
	void pushValue(T value);
};
//------------------------------------------------------------------------------
template<typename T>
cTextArchiveIn& cTextArchiveIn::operator<<(const T& value)
{
	pushValue(value);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T>
cTextArchiveIn& cTextArchiveIn::operator&(const T& value)
{
	pushValue(value);
	return *this;
}
//------------------------------------------------------------------------------
template<typename T, typename>
void cTextArchiveIn::pushValue(const T& value)
{
	openBracket();

	T& valueNonConst = const_cast<T&>(value);
	serialization::serialize(*this, valueNonConst);

	closeBracket();
}
//------------------------------------------------------------------------------
template<typename T>
void cTextArchiveIn::pushValue(const serialization::sNameValuePair<T>& nvp)
{
	pushValue(nvp.value);
}


//------------------------------------------------------------------------------
template <typename T, typename /*= std::enable_if_t <std::is_enum<T>::value>*/>
void cTextArchiveIn::pushValue(T value)
{
	addComma();
	buffer << enumToString(value);
	nextCommaNeeded = true;
}
 
#endif //serialization_binaryarchiveH
