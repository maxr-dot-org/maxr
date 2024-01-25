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

#ifndef game_logic_endmoveaction_h
#define game_logic_endmoveaction_h

#include "utility/serialization/nvp.h"

#include <stdint.h>

class cVehicle;
class cModel;
class cUnit;


class cEndMoveAction
{
public:
	static cEndMoveAction None();
	static cEndMoveAction Attacking (const cUnit& destUnit);
	static cEndMoveAction Load (const cUnit& destUnit);
	static cEndMoveAction GetIn (const cUnit& destUnit);

	void execute (cModel&, cVehicle&);
	bool isAttacking() const { return endMoveAction == eEndMoveActionType::Attack; }
	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (endMoveAction);
		archive & NVP (destID);
		// clang-format on
	}

private:
	enum class eEndMoveActionType
	{
		None,
		Load,
		GetIn,
		Attack
	};

	cEndMoveAction();
	cEndMoveAction (const cUnit& destUnit, eEndMoveActionType);

	void executeGetInAction (cModel&, cVehicle&);
	void executeLoadAction (cModel&, cVehicle&);
	void executeAttackAction (cModel&, cVehicle&);

private:
	eEndMoveActionType endMoveAction;
	int destID = -1;
};

#endif // !game_logic_endmoveaction_h
