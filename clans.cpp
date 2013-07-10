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

using namespace std;


//--------------------------------------------------
void cClanUnitStat::addModification (const string& area, int value)
{
	modifications[area] = value;
}

//--------------------------------------------------
bool cClanUnitStat::hasModification (const string& key) const
{
	return modifications.find (key) != modifications.end();
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
static string GetModificatorString (int original, int modified)
{
	const int diff = modified - original;
	if (diff > 0)
		return " +" + iToStr (diff);
	else if (diff < 0)
		return " -" + iToStr (-diff);
	else // diff == 0
		return " =" + iToStr (modified);
}

//--------------------------------------------------
string cClanUnitStat::getClanStatsDescription() const
{
	const sUnitData* data = unitId.getUnitDataOriginalVersion();

	if (data == NULL) return "Unknown";

	string result = string (data->name) + ": ";
	const char* const commaSep = ", ";
	const char* sep = "";

	struct {
		const char* type;
		const char* textToTranslate;
		int originalValue;
	} t[] = {
		{"Damage", "Text~Vehicles~Damage", data->damage},
		// here ~Hud~ to avoid letter overlay in german - blutroter pfad - kanonenboot
		{"Range", "Text~Hud~Range", data->range},
		{"Armor", "Text~Vehicles~Armor", data->armor},
		{"Hitpoints", "Text~Vehicles~Hitpoints", data->hitpointsMax},
		{"Scan", "Text~Vehicles~Scan", data->scan},
		{"Speed", "Text~Vehicles~Speed", data->speedMax / 4},
	};

	for (int i = 0; i != sizeof (t) / sizeof (*t); ++i)
	{
		if (hasModification (t[i].type) == false) continue;
		result += sep;
		result += lngPack.i18n (t[i].textToTranslate);
		result += GetModificatorString (t[i].originalValue, getModificationValue (t[i].type));
		sep = commaSep;
	}
	if (hasModification ("Built_Costs"))
	{
		result += sep;
		int nrTurns = getModificationValue ("Built_Costs");
		if (data->isHuman == false) nrTurns /= unitId.iFirstPart == 0 ? 3 : 2;

		result += iToStr (nrTurns) + " " + lngPack.i18n ("Text~Comp~Turns");
	}
	return result;
}

//--------------------------------------------------
cClan::~cClan()
{
	for (size_t i = 0; i != stats.size(); ++i)
	{
		delete stats[i];
	}
}

//--------------------------------------------------
cClanUnitStat* cClan::getUnitStat (sID id) const
{
	for (size_t statIdx = 0; statIdx != stats.size(); ++statIdx)
		if (stats[statIdx]->getUnitId() == id)
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
cClanUnitStat* cClan::addUnitStat (sID id)
{
	cClanUnitStat* newStat = new cClanUnitStat (id);
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
	for (int i = 0; i != getNrUnitStats(); ++i)
	{
		cClanUnitStat* stat = getUnitStat (i);
		result.push_back (stat->getClanStatsDescription());
	}
	return result;
}

//--------------------------------------------------
/*static*/ cClanData& cClanData::instance()
{
	static cClanData _instance;
	return _instance;
}

//--------------------------------------------------
cClanData::~cClanData()
{
	for (size_t i = 0; i != clans.size(); ++i)
	{
		delete clans[i];
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
