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

#ifndef utility_string_toStringH
#define utility_string_toStringH

#include <sstream>

template<typename T>
std::string toString(const T& x)
{
	std::stringstream ss;
	ss.imbue(std::locale("C"));
	ss << x;
	return ss.str();
}

/**Converts integer to string
*/
std::string iToStr (int x);
/**Converts integer to string in hex representation
*/
std::string iToHex (unsigned int x);

/**Converts pointer to string
*/
std::string pToStr (const void* x);
/**Converts bool to string
*/
std::string bToStr (bool x);

std::string getHexValue(unsigned char byte);
unsigned char getByteValue(const std::string& str, int index);

#endif // utility_string_toStringH
