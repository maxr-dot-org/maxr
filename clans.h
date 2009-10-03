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

#include "clist.h"
#include <string>
#include <map>
#include <vector>

//-------------------------------------------------------------------------
class cClanUnitStat
{
public:
	cClanUnitStat (int unitIdFirstPart, int unitIdSecPart) : unitIdFirstPart (unitIdFirstPart), unitIdSecPart (unitIdSecPart) {}

	void addModification (std::string area, int value);

	int getUnitIdFirstPart () const { return unitIdFirstPart; }
	int getUnitIdSecPart () const { return unitIdSecPart; }

	int getModificationValue (std::string key) const;
	bool hasModification (std::string key) const;

	std::string getClanStatsDescription () const;

	//-------------------------------------------------------------------------
private:
	int unitIdFirstPart;
	int unitIdSecPart;
	std::map<std::string, int> modifications;
};

//-------------------------------------------------------------------------
class cClan
{
public:
	cClan (int num) : num (num) {}
	virtual ~cClan ();

	void setDescription (std::string newDescription);
	const std::string& getDescription () const { return description; }

	std::vector<std::string> getClanStatsDescription () const;

	void setName (std::string newName);
	const std::string& getName () const { return name; }

	cClanUnitStat* getUnitStat (int idFirstPart, int idSecPart) const;
	cClanUnitStat* getUnitStat (unsigned int index) const;
	cClanUnitStat* addUnitStat (int idFirstPart, int idSecPart);
	int getNrUnitStats () const { return (int) stats.Size (); }

	//-------------------------------------------------------------------------
private:
	int num;
	std::string description;
	std::string name;
	cList<cClanUnitStat*> stats;
};

//-------------------------------------------------------------------------
class cClanData
{
public:
	static cClanData& instance ();
	virtual ~cClanData ();

	cClan* addClan ();
	cClan* getClan (unsigned int num);
	int getNrClans () const { return (int) clans.Size (); }

	//-------------------------------------------------------------------------
private:
	cClanData () {}

	cList<cClan*> clans;
};



#endif // clansH
