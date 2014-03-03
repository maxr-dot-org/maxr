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

//--------------------------------------------------------------------------
// Workaround for compilers that do not support std::make_unique
#ifndef MAXR_NO_MAKE_UNIQUE
#   define MAXR_NO_MAKE_UNIQUE         0
#endif

#if MAXR_NO_MAKE_UNIQUE

#include <type_traits>
#include <utility>
#include <memory>

namespace std
{

template<class T, class... Args>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (Args&& ... args)
{
	return (unique_ptr<T> (new T (forward<Args> (args)...)));
}

template<class T>
inline typename enable_if < is_array<T>::value&& extent<T>::value == 0, unique_ptr<T >>::type make_unique (size_t size)
{
	typedef typename remove_extent<T>::type E;
	return (unique_ptr<T> (new E[size]()));
}

template<class T, class... Args>
typename enable_if < extent<T>::value != 0, void >::type make_unique (Args&& ...) MAXR_DELETE_FUNCTION;

}

#endif // MAXR_NO_MAKE_UNIQUE
