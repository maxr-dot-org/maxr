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

//--------------------------------------------------
void cClanUnitStat::addModification (const std::string& area, int value)
{
	modifications[area] = value;
}

//--------------------------------------------------
bool cClanUnitStat::hasModification (const std::string& key) const
{
	return modifications.find (key) != modifications.end();
}

//--------------------------------------------------
int cClanUnitStat::getModificationValue (const std::string& key) const
{
	auto it = modifications.find (key);
	if (it != modifications.end())
		return it->second;
	return 0;
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (sID id) const
{
	for (const auto& stat : stats)
		if (stat->getUnitId() == id)
			return stat.get();
	return nullptr;
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (unsigned int index) const
{
	if (index < stats.size())
		return stats[index].get();
	return nullptr;
}

//--------------------------------------------------
cClanUnitStat* cClan::addUnitStat (sID id)
{
	stats.push_back (std::make_unique<cClanUnitStat> (id));
	return stats.back().get();
}

//---------------------------------------------------
cClan::cClan(const cClan& other) :
	num(other.num),
	description(other.description),
	name(other.name)
{
	for (const auto& stat : other.stats)
	{
		stats.push_back(std::make_unique<cClanUnitStat>(*stat));
	}
}

//--------------------------------------------------
void cClan::setDefaultDescription (const std::string& newDescription)
{
	description = newDescription;
}

//---------------------------------------------------
const std::string& cClan::getDefaultDescription() const
{
	return description;
}

//--------------------------------------------------
void cClan::setDefaultName (const std::string& newName)
{
	name = newName;
}

//--------------------------------------------------
const std::string& cClan::getDefaultName() const
{
	return name;
}

//---------------------------------------------------
cClanData::cClanData(const cClanData& other)
{
	for (const auto& clan : other.clans)
	{
		clans.push_back(std::make_unique<cClan>(*clan));
	}
}

//--------------------------------------------------
cClan* cClanData::addClan()
{
	clans.push_back (std::make_unique<cClan> ((int) clans.size()));
	return clans.back().get();
}

//--------------------------------------------------
cClan* cClanData::getClan (unsigned int num) const
{
	if (num < clans.size())
		return clans[num].get();
	return nullptr;
}
