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

#ifndef utility_enumflagH
#define utility_enumflagH

#include "utility/tounderlyingtype.h"

#include <type_traits>

/**
 * Simple class for an enumeration based "bit-flag".
 *
 * @tparam E The enum type to be used for the flags.
 *           NOTE: all the values in the flag must be different powers of 2!
 *           HINT: 0 is not a power of 2 ;)
 */
template <typename E>
class cEnumFlag
{
	static_assert (std::is_enum<E>::value, "Template parameter has to be an enumerator type!");

public:
	cEnumFlag() = default;
	cEnumFlag (E value);
	explicit cEnumFlag (std::underlying_type_t<E>);
	cEnumFlag (const cEnumFlag&) = default;

	cEnumFlag& operator= (E other);
	cEnumFlag& operator= (const cEnumFlag&) = default;

	operator bool() const;

	bool operator== (const cEnumFlag&) const = default;

	cEnumFlag<E> operator| (E other) const;
	cEnumFlag<E> operator| (cEnumFlag<E> other) const;

	cEnumFlag<E>& operator|= (E other);
	cEnumFlag<E>& operator|= (cEnumFlag<E> other);

	cEnumFlag<E> operator& (E other) const;
	cEnumFlag<E> operator& (cEnumFlag<E> other) const;

	cEnumFlag<E>& operator&= (E other);
	cEnumFlag<E>& operator&= (cEnumFlag<E> other);

	std::underlying_type_t<E> data() const;

private:
	std::underlying_type_t<E> data_ = 0;
};

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E> toEnumFlag (E value)
{
	return cEnumFlag<E> (value);
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>::cEnumFlag (E value) :
	data_ (toUnderlyingType (value))
{}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>::cEnumFlag (std::underlying_type_t<E> value) :
	data_ (value)
{}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>& cEnumFlag<E>::operator= (E other)
{
	data_ = toUnderlyingType (other);
	return *this;
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>::operator bool() const
{
	return data_ != (std::underlying_type_t<E>) (0);
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E> cEnumFlag<E>::operator| (E other) const
{
	return cEnumFlag<E> (data_ | toUnderlyingType (other));
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E> cEnumFlag<E>::operator| (cEnumFlag<E> other) const
{
	return cEnumFlag<E> (data_ | other.data_);
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>& cEnumFlag<E>::operator|= (E other)
{
	data_ |= toUnderlyingType (other);
	return *this;
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>& cEnumFlag<E>::operator|= (cEnumFlag<E> other)
{
	data_ |= other.data_;
	return *this;
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E> cEnumFlag<E>::operator& (E other) const
{
	return cEnumFlag<E> (data_ & toUnderlyingType (other));
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E> cEnumFlag<E>::operator& (cEnumFlag<E> other) const
{
	return cEnumFlag<E> (data_ & other.data_);
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>& cEnumFlag<E>::operator&= (E other)
{
	data_ &= toUnderlyingType (other);
	return *this;
}

//------------------------------------------------------------------------------
template <typename E>
cEnumFlag<E>& cEnumFlag<E>::operator&= (cEnumFlag<E> other)
{
	data_ &= other.data_;
	return *this;
}

//------------------------------------------------------------------------------
template <typename E>
std::underlying_type_t<E> cEnumFlag<E>::data() const
{
	return data_;
}

#endif // utility_enumflagH
