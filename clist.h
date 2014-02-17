#ifndef CLIST_H
#define CLIST_H

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

#endif
