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

#ifndef game_logic_casualtiestrackerH
#define game_logic_casualtiestrackerH

#include "game/data/units/unitdata.h"
#include "utility/signal/signal.h"

#include <vector>

class cUnit;

//-------------------------------------------------------------------------------
class cCasualtiesTracker
{
public:
	cCasualtiesTracker() {}

	void logCasualty (const cUnit& unit);
	int getCasualtiesOfUnitType (sID unitType, int playerNr) const;

	std::vector<sID> getUnitTypesWithLosses() const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (casualtiesPerPlayer);
		// clang-format on
	}
	uint32_t getChecksum (uint32_t crc) const;

	mutable cSignal<void (const sID&, int)> casualtyChanged;
	mutable cSignal<void()> casualtiesChanged;
	//-------------------------------------------------------------------------------
private:
	struct Casualty
	{
		sID unitID;
		int numberOfLosses;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (unitID);
			archive & NVP (numberOfLosses);
			// clang-format on
		}
		uint32_t getChecksum (uint32_t crc) const;
	};
	struct CasualtiesOfPlayer
	{
		std::vector<Casualty> casualties;
		int playerNr;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (casualties);
			archive & NVP (playerNr);
			// clang-format on
		}
		uint32_t getChecksum (uint32_t crc) const;
	};
	mutable std::vector<CasualtiesOfPlayer> casualtiesPerPlayer;

	std::vector<Casualty>& getCasualtiesOfPlayer (int playerNr) const;
	void increaseCasualty (sID unitType, int playerNr);
};

#endif // game_logic_casualtiestrackerH
