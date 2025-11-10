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

#include "id.h"

#include "utility/crc.h"

#include <cstdio>

//------------------------------------------------------------------------------
std::string sID::getText() const
{
	char tmp[6];
	snprintf (tmp, sizeof (tmp), "%.2d %.2d", firstPart, secondPart);
	return tmp;
}

//------------------------------------------------------------------------------
bool sID::less_buildingFirst (const sID& ID) const
{
	return firstPart == ID.firstPart ? secondPart < ID.secondPart : firstPart > ID.firstPart;
}

//------------------------------------------------------------------------------
uint32_t sID::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (firstPart, crc);
	crc = calcCheckSum (secondPart, crc);

	return crc;
}

//------------------------------------------------------------------------------
bool sID::less_vehicleFirst (const sID& ID) const
{
	return firstPart == ID.firstPart ? secondPart < ID.secondPart : firstPart < ID.firstPart;
}
