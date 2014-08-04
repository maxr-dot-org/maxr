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

#ifndef MAXR_COMPILER_CONFIG

#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)
#   define MAXR_COMPILER_CONFIG "config/compiler/intel.h"
#elif defined(__clang__)
#   define MAXR_COMPILER_CONFIG "config/compiler/clang.h"
# elif defined(__GNUC__)
#   define MAXR_COMPILER_CONFIG "config/compiler/gcc.h"
#elif defined(_MSC_VER)
#   define MAXR_COMPILER_CONFIG "config/compiler/vcpp.h"
#else
#   ifndef MAXR_NO_ASSERT_CONFIG
#       error "Unknown compiler! Please add the configuration for your compiler!"
#   endif
#endif

#endif // MAXR_COMPILER_CONFIG
