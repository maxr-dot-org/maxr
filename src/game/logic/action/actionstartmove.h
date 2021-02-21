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

#ifndef game_logic_actionStartMoveH
#define game_logic_actionStartMoveH

#include "action.h"
#include <forward_list>
#include "game/logic/endmoveaction.h"

class cActionStartMove : public cAction
{
public:
	cActionStartMove(const cVehicle& vehicle, const std::forward_list<cPosition>& path, cEndMoveAction emat);
	cActionStartMove(const cVehicle& vehicle, const std::forward_list<cPosition>& path, bool stopOnDetectResource = false);
	cActionStartMove(cBinaryArchiveOut& archive);

	void serialize(cBinaryArchiveIn& archive) override { cAction::serialize(archive); serializeThis(archive); }
	void serialize(cTextArchiveIn& archive) override { cAction::serialize(archive); serializeThis(archive); }

	void execute(cModel& model) const override;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & unitId;
		archive & path;
		archive & endMoveAction;
		archive & stopOnDetectResource;
	}

	std::forward_list<cPosition> path;
	unsigned int unitId;
	cEndMoveAction endMoveAction;
	bool stopOnDetectResource;
};

#endif // game_logic_actionStartMoveH
