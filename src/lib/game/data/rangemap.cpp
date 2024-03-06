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

#include "rangemap.h"

#include "utility/crc.h"
#include "utility/narrow_cast.h"

#include <cassert>

//------------------------------------------------------------------------------
void cRangeMap::reset()
{
	std::fill (map.begin(), map.end(), 0);
	crcCache = std::nullopt;
}

//------------------------------------------------------------------------------
void cRangeMap::resize (const cPosition& size_)
{
	size = size_;
	map.resize (size.x() * size.y());
	reset();
}

//------------------------------------------------------------------------------
void cRangeMap::add (int range, const cPosition& position, int unitSize, bool square /*= false*/)
{
	std::vector<cPosition> positions;

	const int minx = std::max (position.x() - range, 0);
	const int maxx = std::min (position.x() + range, size.x() - 1);
	const int miny = std::max (position.y() - range, 0);
	const int maxy = std::min (position.y() + range, size.y() - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			if (isInRange (x, y, position, range, unitSize, square))
			{
				auto& value = map[getOffset (x, y)];
				value++;
				if (value == 1)
				{
					positions.push_back (cPosition (x, y));
				}
			}
		}
	}

	positionsInRange (positions);
	crcCache = std::nullopt;
	changed();
}

//------------------------------------------------------------------------------
void cRangeMap::update (int range, const cPosition& oldPosition, const cPosition& newPosition, int oldUnitSize, int newUnitSize, bool square /*= false*/)
{
	std::vector<cPosition> inPositions;
	std::vector<cPosition> outPositions;

	const int minx = std::max (std::min (oldPosition.x(), newPosition.x()) - range, 0);
	const int maxx = std::min (std::max (oldPosition.x(), newPosition.x()) + range, size.x() - 1);
	const int miny = std::max (std::min (oldPosition.y(), newPosition.y()) - range, 0);
	const int maxy = std::min (std::max (oldPosition.y(), newPosition.y()) + range, size.y() - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			bool newInRange = isInRange (x, y, newPosition, range, newUnitSize, square);
			bool oldInRange = isInRange (x, y, oldPosition, range, oldUnitSize, square);

			if (newInRange && !oldInRange)
			{
				auto& value = map[getOffset (x, y)];
				++value;
				if (value == 1)
				{
					inPositions.push_back (cPosition (x, y));
				}
			}
			else if (!newInRange && oldInRange)
			{
				auto& value = map[getOffset (x, y)];
				assert (value > 0);
				--value;
				if (value == 0)
				{
					outPositions.push_back (cPosition (x, y));
				}
			}
		}
	}

	positionsInRange (inPositions);
	positionsOutOfRange (outPositions);
	crcCache = std::nullopt;
	changed();
}

//------------------------------------------------------------------------------
void cRangeMap::update (int oldRange, int newRange, const cPosition& position, int unitSize, bool square /*= false*/)
{
	std::vector<cPosition> inPositions;
	std::vector<cPosition> outPositions;

	const int maxRange = std::max (oldRange, newRange);
	const int minx = std::max (position.x() - maxRange, 0);
	const int maxx = std::min (position.x() + maxRange, size.x() - 1);
	const int miny = std::max (position.y() - maxRange, 0);
	const int maxy = std::min (position.y() + maxRange, size.y() - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			bool newInRange = isInRange (x, y, position, newRange, unitSize, square);
			bool oldInRange = isInRange (x, y, position, oldRange, unitSize, square);

			if (newInRange && !oldInRange)
			{
				auto& value = map[getOffset (x, y)];
				++value;
				if (value == 1)
				{
					inPositions.push_back (cPosition (x, y));
				}
			}
			else if (!newInRange && oldInRange)
			{
				auto& value = map[getOffset (x, y)];
				assert (value > 0);
				--value;
				if (value == 0)
				{
					outPositions.push_back (cPosition (x, y));
				}
			}
		}
	}

	positionsInRange (inPositions);
	positionsOutOfRange (outPositions);
	crcCache = std::nullopt;
	changed();
}

//------------------------------------------------------------------------------
void cRangeMap::remove (int range, const cPosition& position, int unitSize, bool square /*= false*/)
{
	std::vector<cPosition> positions;

	const int minx = std::max (position.x() - range, 0);
	const int maxx = std::min (position.x() + range, size.x() - 1);
	const int miny = std::max (position.y() - range, 0);
	const int maxy = std::min (position.y() + range, size.y() - 1);

	for (int x = minx; x <= maxx; ++x)
	{
		for (int y = miny; y <= maxy; ++y)
		{
			if (isInRange (x, y, position, range, unitSize, square))
			{
				auto& value = map[getOffset (x, y)];
				--value;
				if (value == 0)
				{
					positions.push_back (cPosition (x, y));
				}
			}
		}
	}

	positionsOutOfRange (positions);
	crcCache = std::nullopt;
	changed();
}

//------------------------------------------------------------------------------
bool cRangeMap::get (const cPosition& position) const
{
	if (position.x() < 0 || position.x() >= size.x()) return false;
	if (position.y() < 0 || position.y() >= size.y()) return false;

	return map[getOffset (position.x(), position.y())] > 0;
}

//------------------------------------------------------------------------------
uint32_t cRangeMap::getChecksum (uint32_t crc) const
{
	if (!crcCache)
	{
		crcCache = calcCheckSum (map, 0);
	}

	return calcCheckSum (*crcCache, crc);
}

//------------------------------------------------------------------------------
void cRangeMap::subtract (const std::vector<uint16_t>& data)
{
	assert (map.size() == data.size());

	std::vector<cPosition> positions;

	for (size_t i = 0; i < data.size(); i++)
	{
		auto oldValue = map[i];
		map[i] = narrow_cast<std::uint16_t> (std::max (oldValue - data[i], 0));
		if (map[i] == 0 && oldValue > 0)
		{
			positions.push_back (cPosition (i % size.x(), i / size.x()));
		}
	}

	positionsOutOfRange (positions);
	crcCache = std::nullopt;
	changed();
}

//------------------------------------------------------------------------------
bool cRangeMap::isInRange (int x, int y, const cPosition& position, int range, int unitSize, bool square) const
{
	// calc distance from center of unit.
	// to prevent fractional numbers, store the double distance
	cPosition delta2x = (cPosition (x, y) - position) * 2 - unitSize + 1;

	if (square)
	{
		return delta2x.abs().max() <= 2 * range;
	}
	else
	{
		return delta2x.l2NormSquared() <= 4 * range * range;
	}
}

//------------------------------------------------------------------------------
int cRangeMap::getOffset (int x, int y) const
{
	return x + y * size.x();
}

//------------------------------------------------------------------------------
std::vector<uint16_t> cRangeMap::getMap() const
{
	return map;
}
