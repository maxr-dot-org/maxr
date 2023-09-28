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
#include "game/startup/initplayerdata.h"

class cVehicle;
class cBuilding;
class cStaticMap;

class cActionInitNewGame : public cActionT<cAction::eActiontype::InitNewGame>
{
public:
	explicit cActionInitNewGame (sInitPlayerData);
	explicit cActionInitNewGame (cBinaryArchiveIn&);

	void serialize (cBinaryArchiveOut& archive) override
	{
		cAction::serialize (archive);
		serializeThis (archive);
	}
	void serialize (cJsonArchiveOut& archive) override
	{
		cAction::serialize (archive);
		serializeThis (archive);
	}

	void execute (cModel&) const override;

	static bool isValidLandingPosition (cPosition, const cStaticMap&, bool fixedBridgeHead, const std::vector<sLandingUnit>&, const cUnitsData&);

	sInitPlayerData initPlayerData;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		initPlayerData.serialize (archive);
	}
	void makeLanding (cPlayer&, const std::vector<sLandingUnit>&, cModel&) const;
	cVehicle* landVehicle (const cPosition&, int radius, const sID&, cPlayer&, cModel&) const;
	static bool findPositionForStartMine (cPosition&, const cUnitsData&, const cStaticMap&);
};

#endif
