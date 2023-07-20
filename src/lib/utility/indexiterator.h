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

#ifndef utility_indexiteratorH
#define utility_indexiteratorH

template <typename PointType>
class cIndexIterator
{
public:
	cIndexIterator (const PointType& begin, const PointType& end);

	bool hasMore() const;
	bool next();

	const PointType& operator*();
	const PointType* operator->();

private:
	const PointType begin;
	const PointType end;

	PointType current;
};

template <typename PointType>
cIndexIterator<PointType> makeIndexIterator (const PointType& begin, const PointType& end)
{
	return cIndexIterator<PointType> (begin, end);
}

//------------------------------------------------------------------------------
template <typename PointType>
cIndexIterator<PointType>::cIndexIterator (const PointType& begin_, const PointType& end_) :
	begin (begin_),
	end (end_),
	current (begin_)
{}

//------------------------------------------------------------------------------
template <typename PointType>
bool cIndexIterator<PointType>::hasMore() const
{
	for (size_t d = 0; d < current.size(); ++d)
	{
		if (current[d] >= end[d]) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
template <typename PointType>
bool cIndexIterator<PointType>::next()
{
	size_t k = 0;

	while (true)
	{
		++current[k];
		if (end[k] > current[k])
		{
			return true;
		}
		else
		{
			if (k == current.size() - 1)
			{
				return false;
			}
			else
			{
				current[k] = begin[k];
				++k;
			}
		}
	}
}

//------------------------------------------------------------------------------
template <typename PointType>
const PointType& cIndexIterator<PointType>::operator*()
{
	return current;
}

//------------------------------------------------------------------------------
template <typename PointType>
const PointType* cIndexIterator<PointType>::operator->()
{
	return &current;
}

#endif // utility_indexiteratorH
