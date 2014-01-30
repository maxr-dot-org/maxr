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

#ifndef MAXR_PLATFORM_CONFIG

#if (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC)
#   define MAXR_PLATFORM_CONFIG "config/platform/linux.h"
#elif defined(__CYGWIN__)
#   define MAXR_PLATFORM_CONFIG "config/platform/cygwin.h"
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   define MAXR_PLATFORM_CONFIG "config/platform/win32.h"
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#   define MAXR_PLATFORM_CONFIG "config/platform/macos.h"
#elif defined(__amigaos__)
#   define MAXR_PLATFORM_CONFIG "config/platform/amigaos.h"
#else
#   ifndef MAXR_NO_ASSERT_CONFIG
#       error "Unknown platform! Please add the configuration for your platform!"
#   endif
#endif

#endif // MAXR_PLATFORM_CONFIG
