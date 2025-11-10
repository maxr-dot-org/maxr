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

// Might be constexpr inline in C++17
/* static */ const unsigned int cGameSettings::defaultVictoryTurnsOptions[3]{100, 200, 400};
/* static */ const unsigned int cGameSettings::defaultVictoryPointsOptions[3]{200, 400, 800};

const std::chrono::seconds cGameSettings::defaultTurnLimitOption0 (60);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption1 (120);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption2 (180);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption3 (240);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption4 (300);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption5 (350);

const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption0 (15);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption1 (30);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption2 (45);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption3 (60);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption4 (75);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption5 (90);

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
