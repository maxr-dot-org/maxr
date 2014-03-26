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

#include <cassert>

#include "menus.h"
#include "netmessage.h"

//std::string ToString (eLandingState state)
//{
//	switch (state)
//	{
//		case LANDING_POSITION_TOO_CLOSE: return "LANDING_POSITION_TOO_CLOSE";
//		case LANDING_POSITION_WARNING: return "LANDING_POSITION_WARNING";
//		case LANDING_POSITION_OK: return "LANDING_POSITION_OK";
//		case LANDING_POSITION_CONFIRMED: return "LANDING_POSITION_COMFIRMED";
//		case LANDING_STATE_UNKNOWN: return "LANDING_STATE_UNKNOWN";
//	}
//	assert (0);
//	return "unknown";
//}