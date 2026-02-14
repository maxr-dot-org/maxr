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

#include "gamesettings.h"

#include "utility/crc.h"

//------------------------------------------------------------------------------
uint32_t cGameSettings::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (alienEnabled, crc);
	crc = calcCheckSum (bridgeheadType, crc);
	crc = calcCheckSum (clansEnabled, crc);
	crc = calcCheckSum (gameType, crc);
	crc = calcCheckSum (goldAmount, crc);
	crc = calcCheckSum (metalAmount, crc);
	crc = calcCheckSum (oilAmount, crc);
	crc = calcCheckSum (resourceDensity, crc);
	crc = calcCheckSum (startCredits, crc);
	crc = calcCheckSum (turnEndDeadline.count(), crc);
	crc = calcCheckSum (turnEndDeadlineActive, crc);
	crc = calcCheckSum (turnLimit.count(), crc);
	crc = calcCheckSum (turnLimitActive, crc);
	crc = calcCheckSum (victoryConditionType, crc);
	crc = calcCheckSum (victoryPoints, crc);
	crc = calcCheckSum (victoryTurns, crc);

	return crc;
}
