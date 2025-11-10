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

#ifndef game_data_rangemapH
#define game_data_rangemapH

#include "utility/position.h"
#include "utility/signal/signal.h"

#include <optional>
#include <vector>

/**
* This class is used to track, whether a position is a specific range to any
* object on the map.
*/
class cRangeMap
{
public:
	cRangeMap() = default;

	void reset();
	void resize (const cPosition& size);
	void subtract (const std::vector<uint16_t>& scanMapCopy);

	void add (int range, const cPosition& position, int unitSize, bool square = false);
	void update (int range, const cPosition& oldPosition, const cPosition& newPosition, int oldUnitSize, int newUnitSize, bool square = false);
	void update (int oldRange, int newRange, const cPosition& position, int unitSize, bool square = false);
	void remove (int range, const cPosition& position, int unitSize, bool square = false);

	/** returns true, when position is in range of any of the objects in the map */
	bool get (const cPosition& position) const;

	/** returns access to the full map */
	std::vector<uint16_t> getMap() const;

	uint32_t getChecksum (uint32_t crc) const;

	/** Triggered, when a position comes in range or goes out of range, by an add/update/remove */
	mutable cSignal<void (const std::vector<cPosition>&)> positionsInRange;
	mutable cSignal<void (const std::vector<cPosition>&)> positionsOutOfRange;
	/** Triggered after an operation changed at least the status of one position */
	mutable cSignal<void()> changed;

private:
	bool isInRange (int x, int y, const cPosition&, int range, int unitSize, bool square) const;
	int getOffset (int x, int y) const;

	cPosition size;
	std::vector<uint16_t> map;

	mutable std::optional<uint32_t> crcCache;
};

#endif
