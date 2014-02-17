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

#ifndef MAXR_STDLIB_CONFIG

#include <cstddef>

#if defined(_LIBCPP_VERSION)
#   define MAXR_STDLIB_CONFIG "config/stdlib/libcpp.h"
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
#   define MAXR_STDLIB_CONFIG "config/stdlib/libstdcpp.h"
#elif (defined(_YVALS) && !defined(__IBMCPP__)) || defined(_CPPLIB_VER)
#   define MAXR_STDLIB_CONFIG "config/stdlib/dinkumware.h"
#else
#   ifndef MAXR_NO_ASSERT_CONFIG
#       error "Unknown C++ standard library! Please add the configuration for your library!"
#   endif
#endif

#endif // MAXR_STDLIB_CONFIG
