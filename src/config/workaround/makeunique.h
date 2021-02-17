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

#if MAXR_NO_VARIADIC_TEMPLATES

template<class T>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique()
{
	return unique_ptr<T> (new T());
}

template<class T, class A1>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1)
{
	return unique_ptr<T> (new T (forward<A1> (arg1)));
}

template<class T, class A1, class A2>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2)));
}

template<class T, class A1, class A2, class A3>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3)));
}

template<class T, class A1, class A2, class A3, class A4>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4)));
}

template<class T, class A1, class A2, class A3, class A4, class A5>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4, A5&& arg5)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4), forward<A5> (arg5)));
}

template<class T, class A1, class A2, class A3, class A4, class A5, class A6>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4, A5&& arg5, A6&& arg6)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4), forward<A5> (arg5), forward<A6> (arg6)));
}

template<class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4, A5&& arg5, A6&& arg6, A7&& arg7)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4), forward<A5> (arg5), forward<A6> (arg6), forward<A7> (arg7)));
}

template<class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4, A5&& arg5, A6&& arg6, A7&& arg7, A8&& arg8)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4), forward<A5> (arg5), forward<A6> (arg6), forward<A7> (arg7), forward<A8> (arg8)));
}

template<class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4, A5&& arg5, A6&& arg6, A7&& arg7, A8&& arg8, A9&& arg9)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4), forward<A5> (arg5), forward<A6> (arg6), forward<A7> (arg7), forward<A8> (arg8), forward<A9> (arg9)));
}

template<class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (A1&& arg1, A2&& arg2, A3&& arg3, A4&& arg4, A5&& arg5, A6&& arg6, A7&& arg7, A8&& arg8, A9&& arg9, A10&& arg10)
{
	return unique_ptr<T> (new T (forward<A1> (arg1), forward<A2> (arg2), forward<A3> (arg3), forward<A4> (arg4), forward<A5> (arg5), forward<A6> (arg6), forward<A7> (arg7), forward<A8> (arg8), forward<A9> (arg9), forward<A10> (arg10)));
}

#else

template<class T, class... Args>
inline typename enable_if < !is_array<T>::value, unique_ptr<T >>::type make_unique (Args&& ... args)
{
	return unique_ptr<T> (new T (forward<Args> (args)...));
}

template<class T, class... Args>
typename enable_if < extent<T>::value != 0, void >::type make_unique (Args&& ...) = delete;

#endif // MAXR_NO_VARIADIC_TEMPLATES

template<class T>
inline typename enable_if < is_array<T>::value&& extent<T>::value == 0, unique_ptr<T >>::type make_unique (size_t size)
{
	typedef typename remove_extent<T>::type E;
	return unique_ptr<T> (new E[size]());
}

} // namespace std

#endif // MAXR_NO_MAKE_UNIQUE
