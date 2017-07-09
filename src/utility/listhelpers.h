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

#include <algorithm>
#include <vector>

template <typename T>
struct trait_add_const
{
	typedef const T type;
};

template <typename T>
struct trait_add_const<T*>
{
	typedef const T* type;
};

template <typename T>
bool Contains (const std::vector<T>& container, const typename trait_add_const<T>::type& elem)
{
	return std::find (container.begin(), container.end(), elem) != container.end();
}

template <typename T>
void Remove (std::vector<T>& container, const typename trait_add_const<T>::type& elem)
{
	container.erase (std::remove (container.begin(), container.end(), elem), container.end());
}

template <typename T>
void RemoveEmpty(std::vector<T>& container)
{
	container.erase(std::remove_if(container.begin(), container.end(), [](const T& elem){return elem.empty(); }), container.end());
}

template<typename T>
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

#endif // utility_listhelpersH
