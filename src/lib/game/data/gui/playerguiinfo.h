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

#ifndef game_data_gui_playerguiinfoH
#define game_data_gui_playerguiinfoH

#include "game/data/gui/gameguistate.h"
#include "game/data/report/savedreport.h"
#include "utility/position.h"
#include "utility/serialization/nvp.h"

#include <array>
#include <memory>
#include <vector>

struct sPlayerGuiInfo
{
	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (gameGuiState);
		archive & serialization::makeNvp ("reports", *reports);
		archive & NVP (savedPositions);
		archive & NVP (doneList);
		// clang-format on
	}

	cGameGuiState gameGuiState;
	std::shared_ptr<std::vector<std::unique_ptr<cSavedReport>>> reports = std::make_shared<std::vector<std::unique_ptr<cSavedReport>>>();
	std::array<std::optional<cPosition>, 4> savedPositions;
	std::vector<unsigned int> doneList;
};

#endif
