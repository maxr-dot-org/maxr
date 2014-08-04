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
// Workaround for compilers that do not support std::underlying_type
#ifndef MAXR_NO_UNDERLYING_TYPE
#   define MAXR_NO_UNDERLYING_TYPE         0
#endif

#if MAXR_NO_UNDERLYING_TYPE

#include <type_traits>

namespace std {

template<typename E>
struct underlying_type
{
    typedef typename conditional
    <
        E(-1) < E(0),
        typename make_signed<E>::type,
        typename make_unsigned<E>::type
    >::type type;
};

}

#endif // MAXR_NO_UNDERLYING_TYPE
