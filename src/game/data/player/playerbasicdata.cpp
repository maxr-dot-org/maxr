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
	const auto& settings = cSettings::getInstance().getPlayerSettings();
	return { settings.name, settings.color, -1, false };
}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData() :
	nr (0),
	ready (false),
	defeated (false)
{}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData (const std::string& name, const cRgbColor& color, int nr, bool defeated) :
	name (name),
	color (color),
	nr (nr),
	ready (false),
	defeated (defeated)
{}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData (const cPlayerBasicData& other) :
	name (other.name),
	color (other.color),
	nr (other.nr),
	ready (other.ready),
	defeated (other.defeated)
{}

//------------------------------------------------------------------------------
cPlayerBasicData& cPlayerBasicData::operator= (const cPlayerBasicData& other)
{
	name = other.name;
	color = other.color;
	nr = other.nr;
	ready = other.ready;
	defeated = other.defeated;
	return *this;
}

//------------------------------------------------------------------------------
bool cPlayerBasicData::operator == (const cPlayerBasicData& rhs) const
{
	return name == rhs.name
		&& color == rhs.color
		&& nr == rhs.nr
		&& ready == rhs.ready
		&& defeated == rhs.defeated;
}

//------------------------------------------------------------------------------
const std::string& cPlayerBasicData::getName() const
{
	return name;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setName (std::string name_)
{
	std::swap (name, name_);
	if (name != name_) nameChanged();
}

//------------------------------------------------------------------------------
int cPlayerBasicData::getNr() const
{
	return nr;
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
	std::swap (color, color_);
	if (color != color_) colorChanged();
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setReady (bool ready_)
{
	std::swap (ready, ready_);
	if (ready != ready_) readyChanged();
}

//------------------------------------------------------------------------------
bool cPlayerBasicData::isReady() const
{
	return ready;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setDefeated (bool defeated_)
{
	std::swap (defeated_, defeated);
	if (defeated != defeated_) isDefeatedChanged();
}

//------------------------------------------------------------------------------
bool cPlayerBasicData::isDefeated() const
{
	return defeated;
}
