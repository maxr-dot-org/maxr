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

#include "game/data/units/unitdata.h"
#include "utility/serialization/serialization.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

enum class eClanModification
{
	Damage,
	Range,
	Armor,
	Hitpoints,
	Scan,
	Speed,
	Built_Costs
};
namespace serialization
{
	template <>
	struct sEnumStringMapping<eClanModification>
	{
		static const std::vector<std::pair<eClanModification, const char*>> m;
	};
} // namespace serialization
//------------------------------------------------------------------------------
class cClanUnitStat
{
public:
	cClanUnitStat (sID unitId) :
		unitId (unitId) {}
	cClanUnitStat() {}

	void addModification (eClanModification, int value);

	sID getUnitId() const { return unitId; }

	std::optional<int> getModificationValue (eClanModification) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (unitId);
		archive & NVP (modifications);
		// clang-format on
	}

private:
	sID unitId;
	std::map<eClanModification, int> modifications;
};

//------------------------------------------------------------------------------
class cClan
{
public:
	cClan (int num) :
		num (num) {}
	cClan() = default;

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

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (num);
		archive & NVP (description);
		archive & NVP (name);
		archive & NVP (stats);
		// clang-format on
	}

private:
	int num = -1;
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

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (clans);
		// clang-format on
	}

private:
	std::vector<cClan> clans;
};

extern cClanData ClanDataGlobal;

#endif // game_data_player_clansH
