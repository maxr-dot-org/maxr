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

#ifndef serialization_nvpH
#define serialization_nvpH

#include <string_view>

namespace serialization
{
	template <typename T>
	struct sNameValuePair
	{
		sNameValuePair (std::string_view name, T& value) :
			name (name),
			value (value)
		{}

		std::string_view name;
		T& value;
	};

	template <typename T>
	sNameValuePair<T> makeNvp (std::string_view name, T& value)
	{
		return sNameValuePair<T> (name, value);
	}
	template <typename T>
	sNameValuePair<T> makeNvp (std::string_view name, const T& value)
	{
		T& value_nonconst = const_cast<T&> (value);
		return sNameValuePair<T> (name, value_nonconst);
	}

	void makeNvp (std::string_view, std::string_view) = delete;

#define NVP_QUOTE(x) #x
#define NVP(value) serialization::makeNvp (NVP_QUOTE (value), value)
#define NVP_MEMBER(obj, value) serialization::makeNvp (NVP_QUOTE (value), obj.value)
} // namespace serialization

#endif
