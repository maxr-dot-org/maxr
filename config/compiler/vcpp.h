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

#if _MSC_VER < 1600
#   ifndef MAXR_NO_ASSERT_CONFIG
#       error "Visual C++ version < 16.0 (Visual Studio < 2010) will not be able to compile this code because of missing C++11 features."
#   endif
#endif

#ifndef MAXR_NO_OVERRIDE_FUNCTION
#   if _MSC_VER < 1700
#       define MAXR_NO_OVERRIDE_FUNCTION   1
#   endif
#endif

#ifndef MAXR_NO_DELETE_FUNCTION
#   if _MSC_VER < 1800
#       define MAXR_NO_DELETE_FUNCTION     1
#   endif
#endif

#ifndef MAXR_NO_NOEXCEPT
#   if _MSC_FULL_VER < 180021114 // Nov 2013 CTP
#       define MAXR_NO_NOEXCEPT            1
#   endif
#endif

#ifndef MAXR_NO_NOEXCEPT_FUNCTION
//#   if _MSC_VER < ??? // Not yet implemented by any version
#       define MAXR_NO_NOEXCEPT_FUNCTION   1
//#   endif
#endif
