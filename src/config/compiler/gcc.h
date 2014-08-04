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

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
#   ifndef MAXR_NO_ASSERT_CONFIG
#       error "GCC version < 4.6 will not be able to compile this code because of missing C++11 features."
#   endif
#endif

#ifndef MAXR_NO_OVERRIDE_FUNCTION
#   if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)) || !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#       define MAXR_NO_OVERRIDE_FUNCTION   1
#   endif
#endif

#ifndef MAXR_NO_DELETE_FUNCTION
#   if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 4)) || !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#       define MAXR_NO_DELETE_FUNCTION     1
#   endif
#endif

#ifndef MAXR_NO_NOEXCEPT
#   if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6)) || !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#       define MAXR_NO_NOEXCEPT            1
#   endif
#endif

#ifndef MAXR_NO_NOEXCEPT_FUNCTION
#   if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6)) || !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#       define MAXR_NO_NOEXCEPT_FUNCTION   1
#   endif
#endif

#ifndef MAXR_NO_VARIADIC_TEMPLATES
#   if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 4)) || !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
#       define MAXR_NO_VARIADIC_TEMPLATES  1
#   endif
#endif
