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

#ifndef maxrconfigH
#define maxrconfigH

//--------------------------------------------------------------------------
// Detect compiler, C++ Standard Library and platform

#ifdef MAXR_USER_CONFIG
#   include "config/userconfig.h"
#endif

#include "config/selectcompilerconfig.h"
#ifdef MAXR_COMPILER_CONFIG
#  include MAXR_COMPILER_CONFIG
#endif

#include "config/selectstdlibconfig.h"
#ifdef MAXR_STDLIB_CONFIG
#  include MAXR_STDLIB_CONFIG
#endif

#include "config/selectplatformconfig.h"
#ifdef MAXR_PLATFORM_CONFIG
#  include MAXR_PLATFORM_CONFIG
#endif

//--------------------------------------------------------------------------
// Workarounds for missing features

#include "config/workaround/underlyingtype.h"

#endif // maxrconfigH
