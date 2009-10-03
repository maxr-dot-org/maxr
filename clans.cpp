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
void cClanUnitStat::addModification (string area, int value)
{
	modifications[area] = value;
}

//--------------------------------------------------
bool cClanUnitStat::hasModification (string key) const
{
	return (modifications.find (key) != modifications.end ());
}

//--------------------------------------------------
int cClanUnitStat::getModificationValue (string key) const
{
	map<string, int>::const_iterator it = modifications.find (key);
	if (it != modifications.end ())
		return it->second;
	return 0;
}

//--------------------------------------------------
string cClanUnitStat::getClanStatsDescription () const
{
	sID unitID;
	unitID.iFirstPart = unitIdFirstPart;
	unitID.iSecondPart = unitIdSecPart;
	sUnitData* data = unitID.getUnitDataOriginalVersion ();
	string result = "Unknown";
	if (data)
	{
		result = string (data->name) + ": ";

		bool first = true;
		if (hasModification ("Damage"))
		{
			if (first == false)
				result += ", ";
			result += "Attack +" + iToStr (getModificationValue ("Damage") - data->damage);
			first = false;
		}
		if (hasModification ("Range"))
		{
			if (first == false)
				result += ", ";
			result += "Range +" + iToStr (getModificationValue ("Range") - data->range);
			first = false;
		}
		if (hasModification ("Armor"))
		{
			if (first == false)
				result += ", ";
			result += "Armor +" + iToStr (getModificationValue ("Armor") - data->armor);
			first = false;
		}
		if (hasModification ("Hitpoints"))
		{
			if (first == false)
				result += ", ";
			result += "Hits +" + iToStr (getModificationValue ("Hitpoints") - data->hitpointsMax);
			first = false;
		}
		if (hasModification ("Scan"))
		{
			if (first == false)
				result += ", ";
			result += "Scan +" + iToStr (getModificationValue ("Scan") - data->scan);
			first = false;
		}
		if (hasModification ("Speed"))
		{
			if (first == false)
				result += ", ";
			result += "Speed +" + iToStr (getModificationValue ("Speed") - (data->speedMax / 4));
			first = false;
		}
		if (hasModification ("Built_Costs"))
		{
			if (first == false)
				result += ", ";
			int nrTurns = (getModificationValue ("Built_Costs")) / (unitIdFirstPart == 0 ? 3 : 2);
			result += iToStr (nrTurns) + " Turns";
			first = false;
		}
	}
	return result;
}


//--------------------------------------------------
cClan::~cClan ()
{
	for (unsigned int i = 0; i < stats.Size (); i++)
	{
		if (stats[i] != 0)
		{
			delete stats[i];
			stats[i] = 0;
		}
	}
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (int idFirstPart, int idSecPart) const
{
	for (unsigned int statIdx = 0; statIdx < stats.Size (); statIdx++)
		if (stats[statIdx]->getUnitIdFirstPart () == idFirstPart && stats[statIdx]->getUnitIdSecPart () == idSecPart)
			return stats[statIdx];
	return 0;
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (unsigned int index) const
{
	if (0 <= index && index < stats.Size ())
		return stats[index];
	return 0;
}

//--------------------------------------------------
cClanUnitStat* cClan::addUnitStat (int idFirstPart, int idSecPart)
{
	cClanUnitStat* newStat = new cClanUnitStat (idFirstPart, idSecPart);
	stats.Add (newStat);
	return newStat;
}

//--------------------------------------------------
void cClan::setDescription (string newDescription)
{
	description = newDescription;
}
//--------------------------------------------------
void cClan::setName (string newName)
{
	name = newName;
}

//--------------------------------------------------
vector<string> cClan::getClanStatsDescription () const
{
	vector<string> result;
	for (int i = 0; i < getNrUnitStats (); i++)
	{
		cClanUnitStat* stat = getUnitStat (i);
		result.push_back (stat->getClanStatsDescription ());
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
cClanData::~cClanData ()
{
	for (unsigned int i = 0; i < clans.Size (); i++)
	{
		if (clans[i] != 0)
		{
			delete clans[i];
			clans[i] = 0;
		}
	}
}

//--------------------------------------------------
cClan* cClanData::addClan ()
{
	cClan* clan = new cClan ( (int) clans.Size ());
	clans.Add (clan);
	return clan;
}

//--------------------------------------------------
cClan* cClanData::getClan (unsigned int num)
{
 if (0 <= num && num <= clans.Size ())
	 return clans[num];

 return NULL;
}
