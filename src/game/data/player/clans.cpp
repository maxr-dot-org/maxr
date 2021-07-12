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

#include "game/data/player/clans.h"

#include "game/data/units/unitdata.h"

cClanData ClanDataGlobal;

//--------------------------------------------------
void cClanUnitStat::addModification (EClanModification area, int value)
{
	modifications[area] = value;
}

//--------------------------------------------------
bool cClanUnitStat::hasModification (EClanModification key) const
{
	return modifications.find (key) != modifications.end();
}

//--------------------------------------------------
int cClanUnitStat::getModificationValue (EClanModification key) const
{
	auto it = modifications.find (key);
	if (it != modifications.end())
		return it->second;
	return 0;
}

//--------------------------------------------------
const cClanUnitStat* cClan::getUnitStat (sID id) const
{
	for (const auto& stat : stats)
		if (stat.getUnitId() == id)
			return &stat;
	return nullptr;
}

//--------------------------------------------------
const cClanUnitStat* cClan::getUnitStat (unsigned int index) const
{
	if (index < stats.size())
		return &stats[index];
	return nullptr;
}

//--------------------------------------------------
cClanUnitStat* cClan::addUnitStat (sID id)
{
	stats.emplace_back (id);
	return &stats.back();
}

//--------------------------------------------------
void cClan::setDefaultDescription (const std::string& newDescription)
{
	description = newDescription;
}

//--------------------------------------------------
void cClan::setDefaultName (const std::string& newName)
{
	name = newName;
}

//------------------------------------------------------------------------------
cClan& cClanData::addClan()
{
	clans.emplace_back ((int) clans.size());
	return clans.back();
}
