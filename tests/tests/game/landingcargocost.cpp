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

#include "game/startup/gamepreparation.h"

#include <doctest.h>

//------------------------------------------------------------------------------
TEST_CASE ("computeLandingCargoCost")
{
	// bought units: one credit per started 5 units
	CHECK (computeLandingCargoCost (0, 0) == 0);
	CHECK (computeLandingCargoCost (1, 0) == 1);
	CHECK (computeLandingCargoCost (5, 0) == 1);
	CHECK (computeLandingCargoCost (15, 0) == 3);
	CHECK (computeLandingCargoCost (16, 0) == 4); // a mine layer: storage 16
	CHECK (computeLandingCargoCost (50, 0) == 10);
	// free units: only the cargo above the free cargo is paid
	CHECK (computeLandingCargoCost (40, 40) == 0);
	CHECK (computeLandingCargoCost (41, 40) == 1);
	CHECK (computeLandingCargoCost (60, 40) == 4);
	CHECK (computeLandingCargoCost (30, 40) == 0);
}
