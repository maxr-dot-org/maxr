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

/* Author: Paul Grathwohl */

#include "game/logic/casualtiestracker.h"

#include "game/data/player/player.h"
#include "game/data/units/unit.h"
#include "game/data/units/unitdata.h"
#include "utility/log.h"

using namespace std;

//--------------------------------------------------------------------------
void cCasualtiesTracker::logCasualty (const cUnit& unit)
{
	if (!unit.getOwner()) return;
	if (unit.isABuilding() && unit.data.getBuildCost() <= 2) return;

	const auto& unitType = unit.data.getId();
	const int playerNr = unit.getOwner()->getId();
	setCasualty (unitType, getCasualtiesOfUnitType (unitType, playerNr) + 1, playerNr);

	casualtiesChanged();
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::setCasualty (sID unitType, int numberOfLosses, int playerNr)
{
	auto signalCaller = makeScopedOperation ([=]() { casualtyChanged (unitType, playerNr); });

	vector<Casualty>& casualties = getCasualtiesOfPlayer (playerNr);

	for (auto& casualty : casualties)
	{
		if (unitType == casualty.unitID)
		{
			casualty.numberOfLosses = numberOfLosses;
			return;
		}
	}
	Casualty newCasualtyEntry;
	newCasualtyEntry.numberOfLosses = numberOfLosses;
	newCasualtyEntry.unitID = unitType;
	for (size_t i = 0; i != casualties.size(); ++i)
	{
		if (unitType.less_vehicleFirst (casualties[i].unitID))
		{
			vector<Casualty>::iterator it = casualties.begin();
			casualties.insert (it + i, newCasualtyEntry);
			return;
		}
	}
	casualties.push_back (newCasualtyEntry);
}

//--------------------------------------------------------------------------
int cCasualtiesTracker::getCasualtiesOfUnitType (sID unitType, int playerNr) const
{
	const vector<Casualty>& casualties = getCasualtiesOfPlayer (playerNr);
	for (unsigned int i = 0; i < casualties.size(); i++)
	{
		if (unitType == casualties[i].unitID)
			return casualties[i].numberOfLosses;
	}
	return 0;
}

//--------------------------------------------------------------------------
vector<sID> cCasualtiesTracker::getUnitTypesWithLosses() const
{
	vector<sID> result;

	for (const auto& casualtiesForPlayer : casualtiesPerPlayer)
	{
		const vector<Casualty>& casualties = casualtiesForPlayer.casualties;
		for (const Casualty& casualty : casualties)
		{
			if (ranges::any_of (result, [&] (const sID& id) { return id == casualty.unitID; })) continue;

			bool inserted = false;
			for (size_t j = 0; j != result.size(); ++j)
			{
				const sID unitID = casualty.unitID;

				// buildings should be inserted first
				if (unitID.less_buildingFirst (result[j]))
				{
					result.insert (result.begin() + j, casualty.unitID);
					inserted = true;
					break;
				}
			}
			if (inserted == false)
				result.push_back (casualty.unitID);
		}
	}
	return result;
}

//------------------------------------------------------------------------------
uint32_t cCasualtiesTracker::getChecksum (uint32_t crc) const
{
	return calcCheckSum (casualtiesPerPlayer, crc);
}

//--------------------------------------------------------------------------
vector<cCasualtiesTracker::Casualty>& cCasualtiesTracker::getCasualtiesOfPlayer (int playerNr) const
{
	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		if (casualtiesPerPlayer[i].playerNr == playerNr)
			return casualtiesPerPlayer[i].casualties;
	}

	CasualtiesOfPlayer newCasualtiesOfPlayer;
	newCasualtiesOfPlayer.playerNr = playerNr;
	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		if (playerNr < casualtiesPerPlayer[i].playerNr)
		{
			vector<CasualtiesOfPlayer>::iterator it = casualtiesPerPlayer.begin();
			casualtiesPerPlayer.insert (it + i, newCasualtiesOfPlayer);
			return casualtiesPerPlayer[i].casualties;
		}
	}
	casualtiesPerPlayer.push_back (newCasualtiesOfPlayer);
	return casualtiesPerPlayer.back().casualties;
}

uint32_t cCasualtiesTracker::CasualtiesOfPlayer::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (casualties, crc);
	crc = calcCheckSum (playerNr, crc);

	return crc;
}

uint32_t cCasualtiesTracker::Casualty::getChecksum (uint32_t crc) const
{
	crc = calcCheckSum (unitID, crc);
	crc = calcCheckSum (numberOfLosses, crc);

	return crc;
}
