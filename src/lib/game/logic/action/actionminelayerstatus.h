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

#ifndef game_logic_actionMinelayerStatusH
#define game_logic_actionMinelayerStatusH

#include "action.h"

class cVehicle;

class cActionMinelayerStatus : public cActionT<cAction::eActiontype::MinelayerStatus>
{
public:
	cActionMinelayerStatus (const cVehicle& vehicle, bool layMines, bool clearMines);
	cActionMinelayerStatus (cBinaryArchiveOut& archive);

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
	unsigned int vehicleId;
	bool layMines;
	bool clearMines;

	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (vehicleId);
		archive & NVP (layMines);
		archive & NVP (clearMines);
		// clang-format on
	}
};

#endif
