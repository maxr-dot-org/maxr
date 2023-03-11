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

#include "game/logic/turncounter.h"

#include "utility/crc.h"

//------------------------------------------------------------------------------
cTurnCounter::cTurnCounter (int turn_) :
	turn (turn_)
{}

//------------------------------------------------------------------------------
int cTurnCounter::getTurn() const
{
	return turn;
}

//------------------------------------------------------------------------------
void cTurnCounter::setTurn (int turn_)
{
	std::swap (turn, turn_);
	if (turn != turn_) turnChanged();
}

//------------------------------------------------------------------------------
void cTurnCounter::increaseTurn()
{
	++turn;
	turnChanged();
}

//------------------------------------------------------------------------------
uint32_t cTurnCounter::getChecksum (uint32_t crc) const
{
	return calcCheckSum (turn, crc);
}
