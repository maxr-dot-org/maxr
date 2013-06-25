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

#include "clans.h"
#include "main.h"

using namespace std;


//--------------------------------------------------
void cClanUnitStat::addModification (const string& area, int value)
{
	modifications[area] = value;
}

//--------------------------------------------------
bool cClanUnitStat::hasModification (const string& key) const
{
	return (modifications.find (key) != modifications.end());
}

//--------------------------------------------------
int cClanUnitStat::getModificationValue (const string& key) const
{
	map<string, int>::const_iterator it = modifications.find (key);
	if (it != modifications.end())
		return it->second;
	return 0;
}

//--------------------------------------------------
string cClanUnitStat::getClanStatsDescription() const
{
	sID unitID;
	unitID.iFirstPart = unitIdFirstPart;
	unitID.iSecondPart = unitIdSecPart;
	sUnitData* data = unitID.getUnitDataOriginalVersion();
	string result = "Unknown";
	//TODO: for positive values the › + " +" + ‹-construct should be added to the iToStr-value - if there are more mod's in future... else it is e.g. displayed: <xyz> +-<value> if you lower something -- nonsinn
	if (data)
	{
		result = string (data->name) + ": ";

		bool first = true;
		if (hasModification ("Damage"))
		{
			if (!first)
				result += ", ";
			result += lngPack.i18n ("Text~Vehicles~Damage") + " +" + iToStr (getModificationValue ("Damage") - data->damage);
			first = false;
		}
		if (hasModification ("Range"))
		{
			if (!first)
				result += ", ";
			result += lngPack.i18n ("Text~Hud~Range")/*here ~Hud~ to avoid letter overlay in german - blutroter pfad - kanonenboot */ + " +" + iToStr (getModificationValue ("Range") - data->range);
			first = false;
		}
		if (hasModification ("Armor"))
		{
			if (!first)
				result += ", ";
			result += lngPack.i18n ("Text~Vehicles~Armor") + " +" + iToStr (getModificationValue ("Armor") - data->armor);
			first = false;
		}
		if (hasModification ("Hitpoints"))
		{
			if (!first)
				result += ", ";
			result += lngPack.i18n ("Text~Vehicles~Hitpoints") + " +" + iToStr (getModificationValue ("Hitpoints") - data->hitpointsMax);
			first = false;
		}
		if (hasModification ("Scan"))
		{
			if (!first)
				result += ", ";
			result += lngPack.i18n ("Text~Vehicles~Scan") + " +" + iToStr (getModificationValue ("Scan") - data->scan);
			first = false;
		}
		if (hasModification ("Speed"))
		{
			if (!first)
				result += ", ";
			result += lngPack.i18n ("Text~Vehicles~Speed") + " +" + iToStr (getModificationValue ("Speed") - (data->speedMax / 4));
			first = false;
		}
		if (hasModification ("Built_Costs"))
		{
			if (!first)
				result += ", ";
			int nrTurns = (getModificationValue ("Built_Costs")) / (unitIdFirstPart == 0 ? 3 : 2);
			if (data->isHuman)
				nrTurns = getModificationValue ("Built_Costs");
			result += iToStr (nrTurns) + " " + lngPack.i18n ("Text~Comp~Turns");
			first = false;
		}
	}
	return result;
}

//--------------------------------------------------
cClan::~cClan()
{
	for (unsigned int i = 0; i < stats.size(); i++)
	{
		delete stats[i];
		stats[i] = 0;
	}
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (int idFirstPart, int idSecPart) const
{
	for (unsigned int statIdx = 0; statIdx < stats.size(); statIdx++)
		if (stats[statIdx]->getUnitIdFirstPart() == idFirstPart && stats[statIdx]->getUnitIdSecPart() == idSecPart)
			return stats[statIdx];
	return 0;
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (unsigned int index) const
{
	if (index < stats.size())
		return stats[index];
	return 0;
}

//--------------------------------------------------
cClanUnitStat* cClan::addUnitStat (int idFirstPart, int idSecPart)
{
	cClanUnitStat* newStat = new cClanUnitStat (idFirstPart, idSecPart);
	stats.push_back (newStat);
	return newStat;
}

//--------------------------------------------------
void cClan::setDescription (const string& newDescription)
{
	description = newDescription;
}
//--------------------------------------------------
void cClan::setName (const string& newName)
{
	name = newName;
}

//--------------------------------------------------
vector<string> cClan::getClanStatsDescription() const
{
	vector<string> result;
	for (int i = 0; i < getNrUnitStats(); i++)
	{
		cClanUnitStat* stat = getUnitStat (i);
		result.push_back (stat->getClanStatsDescription());
	}
	return result;
}


//--------------------------------------------------
cClanData& cClanData::instance()
{
	static cClanData _instance;
	return _instance;
}

//--------------------------------------------------
cClanData::~cClanData()
{
	for (unsigned int i = 0; i < clans.size(); i++)
	{
		delete clans[i];
		clans[i] = 0;
	}
}

//--------------------------------------------------
cClan* cClanData::addClan()
{
	cClan* clan = new cClan ( (int) clans.size());
	clans.push_back (clan);
	return clan;
}

//--------------------------------------------------
cClan* cClanData::getClan (unsigned int num)
{
	if (num < clans.size())
		return clans[num];

	return NULL;
}
