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
	enum class eActiontype {
		ACTION_INIT_NEW_GAME
	};
	static std::unique_ptr<cAction> createFromBuffer(cBinaryArchiveOut& archive);

	eActiontype getType() const;

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	virtual void execute(cModel& model) const = NULL;
protected:
	cAction(eActiontype type) : cNetMessage2(eNetMessageType::ACTION), type(type){};
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & type;
	}

	cAction(const cAction&) MAXR_DELETE_FUNCTION;
	cAction& operator=(const cAction&)MAXR_DELETE_FUNCTION;

	eActiontype type;
};

std::string enumToString(cAction::eActiontype value);

//TODOO: new file
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
