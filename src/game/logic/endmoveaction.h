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

#include "game/serialization/nvp.h"

#include <stdint.h>

class cVehicle;
class cModel;
class cUnit;

enum class eEndMoveActionType
{
	None,
	Load,
	Attack
};

class cEndMoveAction
{
public:
	cEndMoveAction();
	cEndMoveAction (const cVehicle&, const cUnit& destUnit, eEndMoveActionType);

	void execute (cModel&);
	eEndMoveActionType getType() const;
	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		archive & NVP (vehicleID);
		archive & NVP (endMoveAction);
		archive & NVP (destID);
	}

private:
	void executeLoadAction (cModel&);
	void executeAttackAction (cModel&);

private:
	int vehicleID;
	eEndMoveActionType endMoveAction;
	int destID;
};

#endif // !game_logic_endmoveaction_h
