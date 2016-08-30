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

#ifndef game_logic_actionInitNewGameH
#define game_logic_actionInitNewGameH

#include "action.h"
#include "game/data/units/landingunit.h"
#include "game/logic/upgradecalculator.h"
#include "utility/position.h"

class cVehicle;
class cBuilding;

class cActionInitNewGame : public cAction
{
public:
	cActionInitNewGame();
	cActionInitNewGame(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive) { cAction::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cAction::serialize(archive); serializeThis(archive); }

	virtual void execute(cModel& model) const override;

	std::vector<sLandingUnit> landingUnits;
	int clan;
	std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades;
	cPosition landingPosition;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & landingUnits;
		archive & clan;
		archive & unitUpgrades;
		archive & landingPosition;
	}
	void makeLanding(const cPosition& landingPosition, cPlayer& player, const std::vector<sLandingUnit>& landingUnits, cModel& model) const;
	cVehicle* landVehicle(const cPosition& landingPosition, int iWidth, int iHeight, const sID& id, cPlayer& player, cModel& model) const;
};


#endif
