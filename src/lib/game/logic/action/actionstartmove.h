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
#include "game/logic/endmoveaction.h"

#include <forward_list>

enum class eStopOn;

enum class eStart
{
	Deferred,
	Immediate
};

class cActionStartMove : public cActionT<cAction::eActiontype::StartMove>
{
public:
	cActionStartMove (const cVehicle&, const std::forward_list<cPosition>& path, eStart, eStopOn, cEndMoveAction);
	cActionStartMove (cBinaryArchiveIn& archive);

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

	void execute (cModel& model) const override;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (unitId);
		archive & NVP (path);
		archive & NVP (endMoveAction);
		archive & NVP (start);
		archive & NVP (stopOn);
		// clang-format on
	}

	std::forward_list<cPosition> path;
	unsigned int unitId;
	cEndMoveAction endMoveAction;
	eStart start;
	eStopOn stopOn;
};

#endif // game_logic_actionStartMoveH
