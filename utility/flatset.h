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

#include <vector>
#include <algorithm>

template<typename Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
class cFlatSet : private std::vector<Key, Allocator>
{
	typedef std::vector<Key, Allocator> Base;
public:
	typedef Key key_type;
	typedef Key value_type;
	typedef typename Allocator::size_type size_type;
	typedef typename Allocator::difference_type difference_type;
	typedef Compare key_compare;
	typedef Compare value_compare;
	typedef Allocator allocator_type;
	typedef typename Allocator::pointer pointer;
	typedef typename Allocator::const_pointer const_pointer;
	typedef typename Allocator::reference reference;
	typedef typename Allocator::const_reference const_reference;
	typedef typename Base::iterator iterator;
	typedef typename Base::const_iterator const_iterator;
	typedef typename Base::reverse_iterator reverse_iterator;
	typedef typename Base::const_reverse_iterator const_reverse_iterator;

	cFlatSet () :
		Base (),
		compare ()
	{}
	explicit cFlatSet (const Compare& comp, const allocator_type& alloc = allocator_type ()) :
		Base (alloc),
		compare (comp)
	{}
	template<typename InputIterator>
	cFlatSet (InputIterator first, InputIterator last, const Compare& comp = Compare (), const allocator_type& alloc = allocator_type ()) :
		Base (first, last, alloc),
		compare (comp)
	{
		std::sort (begin (), end (), compare);
	}
	cFlatSet (const cFlatSet& other) :
		Base (other),
		compare (other.compare)
	{}
	cFlatSet (cFlatSet&& other) :
		Base (std::move(other)),
		compare (std::move(other.compare))
	{}
	//cFlatSet (const cFlatSet& other, const allocator_type& alloc);
	//cFlatSet (cFlatSet&& other, const allocator_type& alloc);
	//cFlatSet (std::initializer_list<value_type> init, const Compare& comp = Compare (), const allocator_type& alloc = allocator_type ());

	cFlatSet& operator=(const cFlatSet& other)
	{
		cFlatSet (other).swap (*this);
		return *this;
	}
	cFlatSet& operator=(cFlatSet&& other)
	{
		Base::operator=(std::move (other));
		compare = std::move (other.compare);
		return *this;
	}

	allocator_type get_allocator () const { return Base::get_allocator (); }

	// iterators
	iterator begin () { return Base::begin (); }
	const_iterator begin () const { return Base::begin (); }
	iterator end () { return Base::end (); }
	const_iterator end () const { return Base::end (); }
	const_iterator cbegin () const { return Base::cbegin (); }
	const_iterator cend () const { return Base::cend (); }

	reverse_iterator rbegin () { return Base::rbegin (); }
	const_reverse_iterator rbegin () const { return Base::rbegin (); }
	reverse_iterator rend () { return Base::rend (); }
	const_reverse_iterator rend () const { return Base::rend (); }
	const_reverse_iterator crbegin () const { return Base::crbegin (); }
	const_reverse_iterator crend () const { return Base::crend (); }

	// capacity
	bool empty () const { return Base::empty (); }
	size_type size () const { return Base::size (); }
	size_type max_size () const { return Base::max_size (); }
	size_type capacity () const { return Base::capacity (); }
	void reserve (size_type cap) { Base::reserve (cap); }
	void shrink_to_fit () { Base::shrink_to_fit (); }

	// modifiers
	std::pair<iterator, bool> insert (const value_type& value)
	{
		bool found = true;
		auto i = lower_bound (value);

		if (i == end () || compare (value, *i))
		{
			i = Base::insert (i, value);
			found = false;
		}
		return std::make_pair (i, !found);
	}
	std::pair<iterator, bool> insert (value_type&& value)
	{
		bool found = true;
		auto i = lower_bound (value);

		if (i == end () || compare (value, *i))
		{
			i = Base::insert (i, std::move(value));
			found = false;
		}
		return std::make_pair (i, !found);
	}
	iterator insert (const_iterator hint, const value_type& value)
	{
		if ((hint == begin () || compare (*(hint-1), value)) && (hint == end () || compare (value, pos)))
		{
			return Base::insert (pos, value);
		}
		return insert (value).first;
	}
	iterator insert (const_iterator hint, value_type&& value)
	{
		if ((hint == begin () || compare (*(hint-1), value)) && (hint == end () || compare (value, pos)))
		{
			return Base::insert (pos, std::move(value));
		}
		return insert (std::move(value)).first;
	}
	template<typename InputIterator>
	void insert (InputIterator first, InputIterator last)
	{
		for (; first != last; ++first) insert (*first);
	}
	//void insert (std::initializer_list<value_type> ilist);

	// TODO: emplace?!

	iterator erase (const_iterator position) { return Base::erase (position); }
	size_type erase (const key_type& x)
	{
		auto i = find (x);
		if (i == end ()) return 0;
		erase (i);
		return 1;
	}
	iterator erase (const_iterator first, const_iterator last) { return Base::erase (first, last); }

	void swap (cFlatSet& other)
	{
		Base::swap (other);
		std::swap (compare, other.compare);
	}

	void clear () { Base::clear (); }

	// observers
	key_compare key_comp () const { return compare; }

	value_compare value_comp () const { return compare; }

	// lookup
	iterator find (const key_type& k)
	{
		auto i  = lower_bound (k);
		if (i != end () && compare (k, *i))
		{
			i = end ();
		}
		return i;
	}
	const_iterator find (const key_type& k) const
	{
		auto i = lower_bound (k);
		if (i != end () && compare (k, *i))
		{
			i = end ();
		}
		return i;
	}
	template<typename K>
	iterator find (const K& k)
	{
		auto i = lower_bound (k);
		if (i != end () && compare (k, *i))
		{
			i = end ();
		}
		return i;
	}
	template<typename K>
	const_iterator find (const K& k) const
	{
		auto i = lower_bound (k);
		if (i != end () && compare (k, *i))
		{
			i = end ();
		}
		return i;
	}
	
	size_type count (const key_type& k) const
	{
		return find (k) != end () ? 1 : 0;
	}
	template<typename K>
	size_type count (const K& k) const
	{
		return find (k) != end () ? 1 : 0;
	}

	iterator lower_bound (const key_type& k)
	{
		return std::lower_bound (begin (), end (), k, compare);
	}
	const_iterator lower_bound (const key_type& k) const
	{
		return std::lower_bound (begin (), end (), k, compare);
	}
	template<typename K>
	iterator lower_bound (const K& k)
	{
		return std::lower_bound (begin (), end (), k, compare);
	}
	template<typename K>
	const_iterator lower_bound (const K& k) const
	{
		return std::lower_bound (begin (), end (), k, compare);
	}

	iterator upper_bound (const key_type& k)
	{
		return std::upper_bound (begin (), end (), k, compare);
	}
	const_iterator upper_bound (const key_type& k) const
	{
		return std::upper_bound (begin (), end (), k, compare);
	}
	template<typename K>
	iterator upper_bound (const K& k)
	{
		return std::upper_bound (begin (), end (), k, compare);
	}
	template<typename K>
	const_iterator upper_bound (const K& k) const
	{
		return std::upper_bound (begin (), end (), k, compare);
	}

	std::pair<iterator, iterator> equal_range (const key_type& k)
	{
		return std::equal_range (begin (), end (), k, compare);
	}
	std::pair<const_iterator, const_iterator> equal_range (const key_type& k) const
	{
		return std::equal_range (begin (), end (), k, compare);
	}

private:
	Compare compare;
};

#endif // utility_flatsetH
