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

#ifndef game_startup_initplayerdataH
#define game_startup_initplayerdataH

#include "game/data/units/id.h"
#include "game/data/units/landingunit.h"
#include "game/logic/upgradecalculator.h"
#include "utility/position.h"
#include "utility/serialization/nvp.h"

#include <utility>
#include <vector>

struct sInitPlayerData
{
	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (clan);
		archive & NVP (landingPosition);
		archive & NVP (landingUnits);
		archive & NVP (unitUpgrades);
		// clang-format on
	}

	int clan = -1;
	std::vector<sLandingUnit> landingUnits;
	std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades;
	cPosition landingPosition;
};

#endif
