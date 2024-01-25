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

#include "game/data/player/playerbasicdata.h"

#include "game/network.h"
#include "settings.h"
#include "utility/crc.h"

//------------------------------------------------------------------------------
/*static*/ cPlayerBasicData cPlayerBasicData::fromSettings()
{
	return {cSettings::getInstance().getPlayerSettings(), -1, false};
}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData (const sPlayerSettings& player, int nr, bool defeated) :
	player (player),
	nr (nr),
	defeated (defeated)
{}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData (const cPlayerBasicData& other) :
	player (other.player),
	nr (other.nr),
	ready (other.ready),
	defeated (other.defeated)
{}

//------------------------------------------------------------------------------
cPlayerBasicData& cPlayerBasicData::operator= (const cPlayerBasicData& other)
{
	player = other.player;
	nr = other.nr;
	ready = other.ready;
	defeated = other.defeated;
	return *this;
}

//------------------------------------------------------------------------------
bool cPlayerBasicData::operator== (const cPlayerBasicData& rhs) const
{
	return player == rhs.player
	    && nr == rhs.nr
	    && ready == rhs.ready
	    && defeated == rhs.defeated;
}

//------------------------------------------------------------------------------
const std::string& cPlayerBasicData::getName() const
{
	return player.name;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setName (std::string name_)
{
	std::swap (player.name, name_);
	if (player.name != name_) nameChanged();
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setNr (int newNr)
{
	std::swap (newNr, nr);
	if (newNr != nr) numberChanged();
}
//------------------------------------------------------------------------------
void cPlayerBasicData::setColor (cRgbColor color_)
{
	std::swap (player.color, color_);
	if (player.color != color_) colorChanged();
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setReady (bool ready_)
{
	std::swap (ready, ready_);
	if (ready != ready_) readyChanged();
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setDefeated (bool defeated_)
{
	std::swap (defeated_, defeated);
	if (defeated != defeated_) isDefeatedChanged();
}
