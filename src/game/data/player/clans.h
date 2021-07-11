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

//-------------------------------------------------------------------------
class cClanUnitStat
{
public:
	cClanUnitStat (sID unitId_) : unitId (unitId_) {}
	cClanUnitStat() {}

	void addModification (const std::string& area, int value);

	sID getUnitId() const { return unitId; }

	int getModificationValue (const std::string& key) const;
	bool hasModification (const std::string& key) const;

	template <typename T>
	void serialize (T& archive)
	{
		archive & unitId;
		archive & modifications;
	}

	//-------------------------------------------------------------------------
private:
	sID unitId;
	std::map<std::string, int> modifications;
};

//-------------------------------------------------------------------------
class cClan
{
public:
	cClan (int num) : num (num) {}
	cClan() : num (-1) {}

	cClan (const cClan&);

	void setDefaultDescription (const std::string& newDescription);
	const std::string& getDefaultDescription() const;

	void setDefaultName (const std::string& newName);
	const std::string& getDefaultName() const;

	int getClanID() const { return num; }

	cClanUnitStat* getUnitStat (sID id) const;
	cClanUnitStat* getUnitStat (unsigned int index) const;
	cClanUnitStat* addUnitStat (sID id);
	int getNrUnitStats() const { return static_cast<int> (stats.size()); }

	template <typename T>
	void save (T& archive) const
	{
		archive << num;
		archive << description;
		archive << name;

		archive << static_cast<uint32_t> (stats.size());
		for (const auto& stat : stats)
			archive << *stat;
	}
	template <typename T>
	void load (T& archive)
	{
		archive >> num;
		archive >> description;
		archive >> name;

		uint32_t length;
		archive >> length;
		stats.clear();
		for (size_t i = 0; i < length; i++)
		{
			cClanUnitStat stat;
			archive >> stat;
			stats.push_back (std::make_unique<cClanUnitStat> (stat));
		}
	}
	SERIALIZATION_SPLIT_MEMBER()

	//-------------------------------------------------------------------------
private:
	int num;
	std::string description;
	std::string name;
	std::vector<std::unique_ptr<cClanUnitStat>> stats;
};

//-------------------------------------------------------------------------
class cClanData
{
public:
	cClanData() {};
	cClanData (const cClanData&);

	cClan* addClan();
	cClan* getClan (unsigned int num) const;
	int getNrClans() const { return static_cast<int> (clans.size()); }

	template <typename T>
	void save (T& archive)
	{
		archive << static_cast<uint32_t> (clans.size());
		for (const auto& clan : clans)
			archive << *clan;
	}
	template <typename T>
	void load (T& archive)
	{
		uint32_t length;
		archive >> length;
		clans.clear();
		for (size_t i = 0; i < length; i++)
		{
			cClan clan;
			archive >> clan;
			clans.push_back (std::make_unique<cClan> (clan));
		}
	}
	SERIALIZATION_SPLIT_MEMBER()

	//-------------------------------------------------------------------------
private:

	std::vector<std::unique_ptr<cClan>> clans;
};

extern cClanData ClanDataGlobal;

#endif // game_data_player_clansH
