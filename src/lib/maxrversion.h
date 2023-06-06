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

#if HAVE_AUTOVERSION_H
# include "autoversion.h" //include autoversion created by buildinfo.sh or cmake
# define BUILD_DATADIR "/usr/share/maxr" // TODO: reimplement setting this path in autoversion.h
#else // We have no autoversion => take care of these manually!
//default path to data dir only used on linux/other
# define BUILD_DATADIR "/usr/share/maxr"
// Builddate: Mmm DD YYYY HH:MM:SS
# define MAX_BUILD_DATE ((std::string) __DATE__ + " " + __TIME__)
# ifdef NDEBUG
#  define PACKAGE_REV "Releaseversion"
# else
#  define PACKAGE_REV "GIT Hash unknown"
# endif
#endif

#if HAVE_CONFIG_H
# include "config.h"
#else // We have no config.h => take care of these manually
# ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "0.2.11"
# endif
# define PACKAGE_NAME "M.A.X.R."
#endif

void logMAXRVersion();

#endif
