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

#ifndef game_logic_actionStealDisableH
#define game_logic_actionStealDisableH

#include "action.h"

class cUnit;
class cVehicle;

enum class eInfiltratorAction
{
	STEAL,
	DISABLE
};

class cActionStealDisable : public cActionT<cAction::eActiontype::StealDisable>
{
public:
	cActionStealDisable (const cVehicle& infiltrator, const cUnit& target, bool steal);
	cActionStealDisable (cBinaryArchiveOut& archive);

	void serialize (cBinaryArchiveIn& archive) override
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
	void changeUnitOwner (cUnit& unit, cPlayer& newOwner, cModel& model) const;

	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (infiltratorId);
		archive & NVP (targetId);
		archive & NVP (steal);
		// clang-format on
	}

	int infiltratorId;
	int targetId;
	bool steal;
};

#endif // game_logic_actionStealDisableH
