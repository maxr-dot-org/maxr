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

#ifndef utility_rangesH
#define utility_rangesH

#include <algorithm>
#include <iterator>
#include <vector>

namespace ranges
{
	//------------------------------------------------------------------------------
	template <typename T, typename U>
	bool contains (const std::vector<T>& container, const U& elem)
	{
		return std::ranges::find (container, elem) != container.end();
	}

	//------------------------------------------------------------------------------
	template <typename Range, typename Projection>
	auto Transform (Range&& range, Projection&& proj)
		-> std::vector<std::decay_t<decltype (proj (*std::begin (range)))>>
	{
		std::vector<std::decay_t<decltype (proj (*std::begin (range)))>> res;
		res.reserve (range.size());
		std::transform (std::begin (range), std::end (range), std::back_inserter (res), proj);
		return res;
	}

} // namespace ranges

#endif
