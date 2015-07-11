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

#ifndef game_logic_actionH
#define game_logic_actionH

#include "netmessage2.h"
#include "game/data/units/landingunit.h"
#include "upgradecalculator.h"
#include "utility/position.h"

class cAction : public cNetMessage2
{
public:
	enum eActiontype {
		ACTION_INIT_NEW_GAME
	};
	static std::unique_ptr<cAction> createFromBuffer(cArchiveOut& archive);

	eActiontype getType() const;

	template<typename T>
	void serializeThis(T& archive)
	{
		cNetMessage2::serialize(archive);
		archive & type;
	}
	virtual void serialize(cArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cArchiveOut& archive) { serializeThis(archive); }

	virtual void execute(cModel& model) const = NULL;
protected:
	cAction(eActiontype type) : cNetMessage2(ACTION), type(type){};
private:
	cAction(const cAction&) MAXR_DELETE_FUNCTION;
	cAction& operator=(const cAction&)MAXR_DELETE_FUNCTION;
	eActiontype type;
};

class cVehicle;
class cBuilding;

class cActionInitNewGame : public cAction
{
public:
	cActionInitNewGame() : cAction(ACTION_INIT_NEW_GAME), clan(-1){};

	template<typename T>
	void serializeThis(T& archive)
	{
		cAction::serialize(archive);
		archive & landingUnits;
		archive & clan;
		archive & unitUpgrades;
		archive & landingPosition;
	}
	virtual void serialize(cArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cArchiveOut& archive) { serializeThis(archive); }

	virtual void execute(cModel& model) const override;


	std::vector<sLandingUnit> landingUnits;
	int clan;
	std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades;
	cPosition landingPosition;
private:
	void makeLanding(const cPosition& landingPosition, cPlayer& player, const std::vector<sLandingUnit>& landingUnits, cModel& model) const;
	cVehicle* landVehicle(const cPosition& landingPosition, int iWidth, int iHeight, const sUnitData& unitData, cPlayer& player, cModel& model) const;
};


#endif
