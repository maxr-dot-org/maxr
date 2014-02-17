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

#ifndef utility_signal_signalcalliteratorH
#define utility_signal_signalcalliteratorH

#include "../invoke.h"

template<typename ResultType, typename ArgumentPackType, typename IterType>
struct sSignalCallIterator
{
	typedef ResultType value_type;

	sSignalCallIterator (const ArgumentPackType& arguments_, IterType iter_, IterType end_) :
		arguments (arguments_),
		iter (iter_),
		end (end_)
	{}

	bool operator==(const sSignalCallIterator& other)
	{
		return iter == other.iter;
	}
	bool operator!=(const sSignalCallIterator& other)
	{
		return !(*this == other);
	}

	value_type operator*() const
	{
		return invoke (iter->function, arguments);
	}
	//pointer operator->() const;

	sSignalCallIterator& operator++()
	{
		++iter;
		while (iter != end && iter->disconnected) ++iter; // skip disconnected slots
		return *this;
	}

	sSignalCallIterator operator++(int)
	{
		sCallIterator tmp (*this);
		++*this;
		return tmp;
	}

private:
	const ArgumentPackType& arguments;
	IterType iter;
	IterType end;
};

#endif // utility_signal_signalcalliteratorH
