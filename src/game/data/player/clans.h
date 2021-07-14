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

#ifndef game_data_player_clansH
#define game_data_player_clansH

#include <map>
#include <string>
#include <vector>

#include "game/data/units/unitdata.h"
#include "game/serialization/serialization.h"

enum class EClanModification
{
	Damage,
	Range,
	Armor,
	Hitpoints,
	Scan,
	Speed,
	Built_Costs
};

//------------------------------------------------------------------------------
class cClanUnitStat
{
public:
	cClanUnitStat (sID unitId) : unitId (unitId) {}
	cClanUnitStat() {}

	void addModification (EClanModification, int value);

	sID getUnitId() const { return unitId; }

	int getModificationValue (EClanModification) const;
	bool hasModification (EClanModification) const;

	template <typename T>
	void serialize (T& archive)
	{
		archive & unitId;
		archive & modifications;
	}

private:
	sID unitId;
	std::map<EClanModification, int> modifications;
};

//------------------------------------------------------------------------------
class cClan
{
public:
	cClan (int num) : num (num) {}
	cClan() : num (-1) {}

	cClan (const cClan&) = default;

	void setDefaultDescription (const std::string&);
	const std::string& getDefaultDescription() const { return description; }

	void setDefaultName (const std::string&);
	const std::string& getDefaultName() const { return name; }

	int getClanID() const { return num; }

	const cClanUnitStat* getUnitStat (sID) const;
	const cClanUnitStat* getUnitStat (unsigned int index) const;
	cClanUnitStat* addUnitStat (sID);
	int getNrUnitStats() const { return static_cast<int> (stats.size()); }

	template <typename T>
	void serialize (T& archive)
	{
		archive & num;
		archive & description;
		archive & name;
		archive & stats;
	}

private:
	int num;
	std::string description;
	std::string name;
	std::vector<cClanUnitStat> stats;
};

//------------------------------------------------------------------------------
class cClanData
{
public:
	cClanData() = default;
	cClanData (const cClanData&) = default;

	cClan& addClan();
	const std::vector<cClan>& getClans() const { return clans; }

	template <typename T>
	void serialize (T& archive)
	{
		archive & clans;
	}

private:
	std::vector<cClan> clans;
};

extern cClanData ClanDataGlobal;

#endif // game_data_player_clansH
