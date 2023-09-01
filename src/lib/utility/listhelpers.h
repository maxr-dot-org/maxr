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

#ifndef utility_listhelpersH
#define utility_listhelpersH

#include "utility/ranges.h"

#include <algorithm>
#include <memory>
#include <vector>

template <typename T>
struct trait_add_const
{
	using type = const T;
};

template <typename T>
struct trait_add_const<T*>
{
	using type = const T*;
};

//--------------------------------------------------------------------------
template <typename T>
std::vector<T*> ExtractPtrs (const std::vector<std::unique_ptr<T>>& v)
{
	return ranges::Transform (v, [] (const auto& ptr) { return ptr.get(); });
}

//--------------------------------------------------------------------------
template <typename T>
void Remove (std::vector<T>& container, const typename trait_add_const<T>::type& elem)
{
	container.erase (std::remove (container.begin(), container.end(), elem), container.end());
}

template <typename Container, typename Predicate>
void EraseIf (Container& container, Predicate pred)
{
	container.erase (ranges::remove_if (container, pred), container.end());
}

template <typename T>
void RemoveEmpty (std::vector<T>& container)
{
	EraseIf (container, [] (const T& elem) { return elem.empty(); });
}

template <typename T>
void RemoveDuplicates (std::vector<T>& container)
{
	for (unsigned int i = 0; i < container.size(); i++)
	{
		for (unsigned int k = i + 1; k < container.size(); k++)
		{
			if (container[i] == container[k])
			{
				container.erase (container.begin() + k);
				k--;
			}
		}
	}
}

template <typename T, typename F>
[[nodiscard]] std::vector<T> Filter (const std::vector<T>& v, F&& filter)
{
	std::vector<T> res (v);

	EraseIf (res, [&] (const auto& e) { return !filter (e); });
	return res;
}

template <typename T>
auto ByGetTo (const T* p)
{
	return [p] (const auto& e) { return e.get() == p; };
}

#endif // utility_listhelpersH
