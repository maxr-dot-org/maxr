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

#ifndef utility_boxH
#define utility_boxH

#include <algorithm>

#include <SDL.h>

/**
 * An axis aligned bounding box (AABB).
 *
 * @tparam PointType The point type to be used in the box.
 */
template<typename PointType>
class cBox
{
public:
	cBox();
	cBox(const PointType& minCorner, const PointType& maxCorner);

	PointType& getMinCorner();
	PointType& getMaxCorner();

	const PointType& getMinCorner() const;
	const PointType& getMaxCorner() const;

	void add (const PointType& point);
	void add (const cBox<PointType>& box);

	/**
	 * Checks whether the point lies within the box.
	 *
	 * @param point The point to check against.
	 * @return True if the point is in the box.
	 *         False if it is outside or on the boundary of the box.
	 */
	bool within(const PointType& point) const;

	/**
	 * Checks whether the point lies within the box or on its boundary.
	 *
	 * @param point The point to check against.
	 * @return True if the point is on the box or on the boundary of the box.
	 *         False if it is outside of the box.
	 */
	bool withinOrTouches(const PointType& point) const;

	/**
	 * Checks whether the box intersects the other one.
	 *
	 * @param other The second box to check against.
	 * @return True if the other box intersects the current one.
	 *         False if the other box lies completely outside the current box
	 *         or if the two boxes only touch at their boundaries.
	 */
	bool intersects(const cBox<PointType>& other) const;

	SDL_Rect toSdlRect () const;

	void fromSdlRect (const SDL_Rect& rect);
private:
	PointType minCorner;
	PointType maxCorner;
};

//------------------------------------------------------------------------------
template<typename PointType>
cBox<PointType>::cBox()
{}

//------------------------------------------------------------------------------
template<typename PointType>
cBox<PointType>::cBox(const PointType& minCorner_, const PointType& maxCorner_) :
	minCorner(minCorner_),
	maxCorner(maxCorner_)
{}

//------------------------------------------------------------------------------
template<typename PointType>
PointType& cBox<PointType>::getMinCorner()
{
	return minCorner;
}

//------------------------------------------------------------------------------
template<typename PointType>
PointType& cBox<PointType>::getMaxCorner()
{
	return maxCorner;
}

//------------------------------------------------------------------------------
template<typename PointType>
const PointType& cBox<PointType>::getMinCorner() const
{
	return minCorner;
}

//------------------------------------------------------------------------------
template<typename PointType>
const PointType& cBox<PointType>::getMaxCorner() const
{
	return maxCorner;
}

//------------------------------------------------------------------------------
template<typename PointType>
void cBox<PointType>::add (const PointType& point)
{
	for (size_t d = 0; d < point.size (); ++d)
	{
		minCorner[d] = std::min (minCorner[d], point[d]);
		maxCorner[d] = std::max (maxCorner[d], point[d]);
	}
}

//------------------------------------------------------------------------------
template<typename PointType>
void cBox<PointType>::add (const cBox<PointType>& box)
{
	for (size_t d = 0; d < PointType::const_size::value; ++d)
	{
		minCorner[d] = std::min (minCorner[d], box.minCorner[d]);
		maxCorner[d] = std::max (maxCorner[d], box.maxCorner[d]);
	}
}

//------------------------------------------------------------------------------
template<typename PointType>
bool cBox<PointType>::within(const PointType& point) const
{
	for(size_t d = 0; d < point.size(); ++d)
	{
		if(point[d] <= minCorner[d] || point[d] >= maxCorner[d]) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
template<typename PointType>
bool cBox<PointType>::withinOrTouches(const PointType& point) const
{
	for(size_t d = 0; d < point.size(); ++d)
	{
		if(point[d] < minCorner[d] || point[d] > maxCorner[d]) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
template<typename PointType>
bool cBox<PointType>::intersects(const cBox<PointType>& point) const
{
	for(size_t d = 0; d < point.size(); ++d)
	{
		if(std::min(maxCorner[d], other.getMaxCorner()[d]) < std::max(minCorner[d], other.getMinCorner()[d])) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
template<typename PointType>
SDL_Rect cBox<PointType>::toSdlRect () const
{
	static_assert(PointType::const_size::value == 2, "Converting to SDL_Rect not support in dimension other than 2.");
	static_assert(std::is_same<typename PointType::value_type, int>::value, "Converting to SDL_Rect not support if point scalar value is other than int."); // NOTE: we may could allow all non-narrowing casts here (e.g. short to int).

	auto diff = maxCorner - minCorner;
	return SDL_Rect{minCorner[0], minCorner[1], diff[0], diff[1]};
}

//------------------------------------------------------------------------------
template<typename PointType>
void cBox<PointType>::fromSdlRect (const SDL_Rect& rect)
{
	minCorner[0] = rect.x;
	minCorner[1] = rect.y;

	maxCorner[0] = rect.x + rect.w;
	maxCorner[1] = rect.y + rect.h;
}

#endif // utility_boxH
