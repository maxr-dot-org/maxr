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

#ifndef utility_flatsetH
#define utility_flatsetH

#include <algorithm>
#include <functional>
#include <vector>

/**
 * Set class based on the ideas from "AssocVector" of "Modern C++ by Andrei Alexandrescu".
 *
 * This class provides a very similar interface to std::set but uses a continuous memory layout instead of a tree.
 *
 * This results in some differences:
 *  - insert and erase are in O(N) instead of O(log N)
 *  - iterators are invalidated on insert and erase
 *  - iterators fulfill the random-access-iterator concept
 *
 * A very important feature of this class is that it supports (a part of)
 * N3657 - "Adding heterogeneous comparison lookup to associative containers" of C++14.
 * This means the key type of .find() is a template which allows to look up elements in the set
 * without the construction of a value_type of the set, as long as the Compare-Predicate supports the
 * comparison with the passed key type.
 *
 * TODO: add check for is_transparent in Compare for the template versions of find/upper_bound/lower_bound.
 *
 * @tparam Key The key and value type of the set.
 * @tparam Compare The less-compare function to use for the sorting.
 * @tparam Allocator The allocator to use.
 */
template <typename Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
class cFlatSet
{
	using DataType = std::vector<Key, Allocator>;

public:
	using key_type = Key;
	using value_type = Key;
	using size_type = typename Allocator::size_type;
	using difference_type = typename Allocator::difference_type;
	using key_compare = Compare;
	using value_compare = Compare;
	using allocator_type = Allocator;
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
	using iterator = typename DataType::iterator;
	using const_iterator = typename DataType::const_iterator;
	using reverse_iterator = typename DataType::reverse_iterator;
	using const_reverse_iterator = typename DataType::const_reverse_iterator;

	cFlatSet() :
		compare(),
		data()
	{}
	explicit cFlatSet (const Compare& comp, const allocator_type& alloc = allocator_type()) :
		compare (comp),
		data (alloc)
	{}
	template <typename InputIterator>
	cFlatSet (InputIterator first, InputIterator last, const Compare& comp = Compare(), const allocator_type& alloc = allocator_type()) :
		compare (comp),
		data (first, last, alloc)
	{
		std::sort (begin(), end(), compare);
	}

	//cFlatSet (const cFlatSet& other, const allocator_type& alloc);
	//cFlatSet (cFlatSet&& other, const allocator_type& alloc);
	//cFlatSet (std::initializer_list<value_type> init, const Compare& comp = Compare(), const allocator_type& alloc = allocator_type());

	cFlatSet (const cFlatSet&) = default;
	cFlatSet (cFlatSet&&) = default;
	cFlatSet& operator= (const cFlatSet&) = default;
	cFlatSet& operator= (cFlatSet&&) = default;

	allocator_type get_allocator() const { return data.get_allocator(); }

	// iterators
	iterator begin() { return data.begin(); }
	const_iterator begin() const { return data.begin(); }
	iterator end() { return data.end(); }
	const_iterator end() const { return data.end(); }
	const_iterator cbegin() const { return data.cbegin(); }
	const_iterator cend() const { return data.cend(); }

	reverse_iterator rbegin() { return data.rbegin(); }
	const_reverse_iterator rbegin() const { return data.rbegin(); }
	reverse_iterator rend() { return data.rend(); }
	const_reverse_iterator rend() const { return data.rend(); }
	const_reverse_iterator crbegin() const { return data.crbegin(); }
	const_reverse_iterator crend() const { return data.crend(); }

	// capacity
	bool empty() const { return data.empty(); }
	size_type size() const { return data.size(); }
	size_type max_size() const { return data.max_size(); }
	size_type capacity() const { return data.capacity(); }
	void reserve (size_type cap) { data.reserve (cap); }
	void shrink_to_fit() { data.shrink_to_fit(); }

	// modifiers
	std::pair<iterator, bool> insert (const value_type& value)
	{
		bool found = true;
		auto i = lower_bound (value);

		if (i == end() || compare (value, *i))
		{
			i = data.insert (i, value);
			found = false;
		}
		return std::make_pair (i, !found);
	}
	std::pair<iterator, bool> insert (value_type&& value)
	{
		bool found = true;
		auto i = lower_bound (value);

		if (i == end() || compare (value, *i))
		{
			i = data.insert (i, std::move (value));
			found = false;
		}
		return std::make_pair (i, !found);
	}
	iterator insert (const_iterator hint, const value_type& value)
	{
		if ((hint == begin() || compare (*(hint - 1), value)) && (hint == end() || compare (value, hint)))
		{
			return data.insert (hint, value);
		}
		return insert (value).first;
	}
	iterator insert (const_iterator hint, value_type&& value)
	{
		if ((hint == begin() || compare (*(hint - 1), value)) && (hint == end() || compare (value, hint)))
		{
			return data.insert (hint, std::move (value));
		}
		return insert (std::move (value)).first;
	}
	template <typename InputIterator>
	void insert (InputIterator first, InputIterator last)
	{
		for (; first != last; ++first)
			insert (*first);
	}
	//void insert (std::initializer_list<value_type> ilist);

	// TODO: emplace?!

	iterator erase (iterator position) { return data.erase (position); }
	size_type erase (const key_type& x)
	{
		auto i = find (x);
		if (i == end()) return 0;
		erase (i);
		return 1;
	}
	iterator erase (const_iterator first, const_iterator last) { return data.erase (first, last); }

	//--------------------------------------------------------------------------
	template <typename K>
	key_type extract (const K& elem)
	{
		auto iter = find (elem);
		if (iter == end()) return {};

		auto removed = *iter;
		erase (iter);
		return removed;
	}

	//--------------------------------------------------------------------------
	void swap (cFlatSet& other)
	{
		std::swap (data, other.data);
		std::swap (compare, other.compare);
	}

	void clear() { data.clear(); }

	// observers
	key_compare key_comp() const { return compare; }

	value_compare value_comp() const { return compare; }

	// lookup
	iterator find (const key_type& k)
	{
		auto i = lower_bound (k);
		if (i != end() && compare (k, *i))
		{
			i = end();
		}
		return i;
	}
	const_iterator find (const key_type& k) const
	{
		auto i = lower_bound (k);
		if (i != end() && compare (k, *i))
		{
			i = end();
		}
		return i;
	}
	template <typename K>
	iterator find (const K& k)
	{
		auto i = lower_bound (k);
		if (i != end() && compare (k, *i))
		{
			i = end();
		}
		return i;
	}
	template <typename K>
	const_iterator find (const K& k) const
	{
		auto i = lower_bound (k);
		if (i != end() && compare (k, *i))
		{
			i = end();
		}
		return i;
	}

	size_type count (const key_type& k) const
	{
		return find (k) != end() ? 1 : 0;
	}
	template <typename K>
	size_type count (const K& k) const
	{
		return find (k) != end() ? 1 : 0;
	}

	iterator lower_bound (const key_type& k)
	{
		return std::lower_bound (begin(), end(), k, compare);
	}
	const_iterator lower_bound (const key_type& k) const
	{
		return std::lower_bound (begin(), end(), k, compare);
	}
	template <typename K>
	iterator lower_bound (const K& k)
	{
		return std::lower_bound (begin(), end(), k, compare);
	}
	template <typename K>
	const_iterator lower_bound (const K& k) const
	{
		return std::lower_bound (begin(), end(), k, compare);
	}

	iterator upper_bound (const key_type& k)
	{
		return std::upper_bound (begin(), end(), k, compare);
	}
	const_iterator upper_bound (const key_type& k) const
	{
		return std::upper_bound (begin(), end(), k, compare);
	}
	template <typename K>
	iterator upper_bound (const K& k)
	{
		return std::upper_bound (begin(), end(), k, compare);
	}
	template <typename K>
	const_iterator upper_bound (const K& k) const
	{
		return std::upper_bound (begin(), end(), k, compare);
	}

	std::pair<iterator, iterator> equal_range (const key_type& k)
	{
		return std::equal_range (begin(), end(), k, compare);
	}
	std::pair<const_iterator, const_iterator> equal_range (const key_type& k) const
	{
		return std::equal_range (begin(), end(), k, compare);
	}

private:
	Compare compare;
	DataType data;
};

#endif // utility_flatsetH
