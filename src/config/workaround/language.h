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
// Workaround for compilers that do not support = delete functions
#ifndef MAXR_NO_DELETE_FUNCTION
#   define MAXR_NO_DELETE_FUNCTION         0
#endif

#ifndef MAXR_DELETE_FUNCTION
#   if MAXR_NO_DELETE_FUNCTION
#       define MAXR_DELETE_FUNCTION
#   else
#       define MAXR_DELETE_FUNCTION = delete
#   endif
#endif

//--------------------------------------------------------------------------
// Workaround for compilers that do not support override functions
#ifndef MAXR_NO_OVERRIDE_FUNCTION
#   define MAXR_NO_OVERRIDE_FUNCTION       0
#endif

#ifndef MAXR_OVERRIDE_FUNCTION
#   if MAXR_NO_OVERRIDE_FUNCTION
#       define MAXR_OVERRIDE_FUNCTION
#   else
#       define MAXR_OVERRIDE_FUNCTION override
#   endif
#endif

//--------------------------------------------------------------------------
// Workaround for compilers that do not support noexcept functions
#ifndef MAXR_NO_NOEXCEPT
#   define MAXR_NO_NOEXCEPT                0
#endif
#ifndef MAXR_NO_NOEXCEPT_FUNCTION
#   define MAXR_NO_NOEXCEPT_FUNCTION       0
#endif

#ifndef MAXR_NOEXCEPT
#   if MAXR_NO_NOEXCEPT
#       define MAXR_NOEXCEPT
#   else
#       define MAXR_NOEXCEPT noexcept
#   endif
#endif

#ifndef MAXR_NOEXCEPT_OR_NOTHROW
#   if MAXR_NO_NOEXCEPT
#       define MAXR_NOEXCEPT_OR_NOTHROW throw()
#   else
#       define MAXR_NOEXCEPT_OR_NOTHROW noexcept
#   endif
#endif

#ifndef MAXR_NOEXCEPT_IF
#   if MAXR_NO_NOEXCEPT_FUNCTION
#       define MAXR_NOEXCEPT_IF(Predicate)
#   else
#       define MAXR_NOEXCEPT_IF(Predicate) noexcept((Predicate))
#   endif
#endif

#ifndef MAXR_NOEXCEPT_EXPR
#   if MAXR_NO_NOEXCEPT_FUNCTION
#       define MAXR_NOEXCEPT_EXPR(Expression) false
#   else
#       define MAXR_NOEXCEPT_EXPR(Expression) noexcept((Expression))
#   endif
#endif

//--------------------------------------------------------------------------
// Variadic templates - no generic workaround for this one
#ifndef MAXR_NO_VARIADIC_TEMPLATES
#   define MAXR_NO_VARIADIC_TEMPLATES      0
#endif
