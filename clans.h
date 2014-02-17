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

#ifndef clansH
#define clansH

#include <map>
#include <string>
#include <vector>

#include "main.h"

//-------------------------------------------------------------------------
class cClanUnitStat
{
public:
	cClanUnitStat (sID unitId_) : unitId (unitId_) {}

	void addModification (const std::string& area, int value);

	sID getUnitId() const { return unitId; }

	int getModificationValue (const std::string& key) const;
	bool hasModification (const std::string& key) const;

	std::string getClanStatsDescription() const;

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
	~cClan();

	void setDescription (const std::string& newDescription);
	const std::string& getDescription() const { return description; }

	std::vector<std::string> getClanStatsDescription() const;

	void setName (const std::string& newName);
	const std::string& getName() const { return name; }

	int getClanID() const { return num; }

	cClanUnitStat* getUnitStat (sID id) const;
	cClanUnitStat* getUnitStat (unsigned int index) const;
	cClanUnitStat* addUnitStat (sID id);
	int getNrUnitStats() const { return static_cast<int> (stats.size()); }

	//-------------------------------------------------------------------------
private:
	int num;
	std::string description;
	std::string name;
	std::vector<cClanUnitStat*> stats;
};

//-------------------------------------------------------------------------
class cClanData
{
public:
	static cClanData& instance();
	~cClanData();

	cClan* addClan();
	cClan* getClan (unsigned int num);
	int getNrClans() const { return static_cast<int> (clans.size()); }

	//-------------------------------------------------------------------------
private:
	cClanData() {}
	std::vector<cClan*> clans;
};

#endif // clansH
