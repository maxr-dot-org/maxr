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

#ifndef game_data_units_commandodataH
#define game_data_units_commandodataH

#include "utility/serialization/nvp.h"

#include <cstdint>

class cMapView;
class cPosition;
class cUnit;
class cVehicle;

class cCommandoData
{
public:
	/**
	* checks whether the commando action can be performed or not
	*@author alzi alias DoctorDeath
	*/
	static bool canDoAction (const cVehicle& commando, const cPosition&, const cMapView&, bool steal);
	static bool canDoAction (const cVehicle& commando, const cUnit*, bool steal);

	static void increaseXp (cVehicle& commando);

	static int getLevel (std::uint32_t numberOfSuccess);

	/**
	* calculates the chance for disabling or stealing the target unit
	*@author alzi alias DoctorDeath
	*/
	int computeChance (const cUnit*, bool steal) const;
	int computeDisabledTurnCount (const cUnit&) const;

	std::uint32_t getSuccessCount() const { return successCount; }

	std::string getDebugString() const;

	[[nodiscard]] std::uint32_t calcCheckSum (std::uint32_t crc) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (successCount);
		// clang-format on
	}

private:
	std::uint32_t successCount = 0;
};

#endif
