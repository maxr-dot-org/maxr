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

#ifndef maxrversionH
#define maxrversionH

#define PACKAGE_NAME "M.A.X.R."
#define PACKAGE_VERSION "0.2.14"

#if __has_include("autoversion.h")
# include "autoversion.h" // created by cmake/premake from autoversion.h.in
#elif !defined(GIT_DESC)
# define GIT_DESC "unknown"
#endif

#define PACKAGE_REV "GIT Hash " GIT_DESC

// Builddate: Mmm DD YYYY HH:MM:SS
#define MAX_BUILD_DATE ((std::string) __DATE__ + " " + __TIME__)

void logMAXRVersion();

#endif
