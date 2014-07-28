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
#include "network.h"

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData ()
{}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData (const std::string& name_, cPlayerColor color_, int nr_, int socketIndex_) :
	name (name_),
	color (std::move(color_)),
	Nr (nr_),
	socketIndex (socketIndex_),
	ready (false)
{}

//------------------------------------------------------------------------------
cPlayerBasicData::cPlayerBasicData (const cPlayerBasicData& other) :
	name (other.name),
	color (other.color),
	Nr (other.Nr),
	socketIndex (other.socketIndex),
	ready (other.ready)
{}

//------------------------------------------------------------------------------
cPlayerBasicData& cPlayerBasicData::operator=(const cPlayerBasicData& other)
{
	name = other.name;
	color = other.color;
	Nr = other.Nr;
	socketIndex = other.socketIndex;
	ready = other.ready;
	return *this;
}

//------------------------------------------------------------------------------
const std::string& cPlayerBasicData::getName () const
{
	return name;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setName (std::string name_)
{
	std::swap (name, name_);
	if (name != name_) nameChanged ();
}

//------------------------------------------------------------------------------
int cPlayerBasicData::getNr () const
{
	return Nr;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setNr (int nr)
{
	std::swap (Nr, nr);
	if (Nr != nr) numberChanged ();
}

//------------------------------------------------------------------------------
int cPlayerBasicData::getSocketIndex () const
{
	return socketIndex;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setSocketIndex (int index)
{
	std::swap (socketIndex, index);
	if (socketIndex != index) socketIndexChanged ();
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setLocal()
{
	socketIndex = MAX_CLIENTS;
}

//------------------------------------------------------------------------------
bool cPlayerBasicData::isLocal() const
{
	return socketIndex == MAX_CLIENTS;
}

//------------------------------------------------------------------------------
void cPlayerBasicData::onSocketIndexDisconnected (int socketIndex_)
{
	if (isLocal() || socketIndex == -1) return;
	if (socketIndex == socketIndex_)
	{
		socketIndex = -1;
	}
	else if (socketIndex > socketIndex_)
	{
		--socketIndex;
	}
}
//------------------------------------------------------------------------------
void cPlayerBasicData::setColor (cPlayerColor color_)
{
	std::swap(color, color_);
	if (color != color_) colorChanged ();
}

//------------------------------------------------------------------------------
void cPlayerBasicData::setReady (bool ready_)
{
	std::swap (ready, ready_);
	if (ready != ready_) readyChanged ();
}

//------------------------------------------------------------------------------
bool cPlayerBasicData::isReady () const
{
	return ready;
}