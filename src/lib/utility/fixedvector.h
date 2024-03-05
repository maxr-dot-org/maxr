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

#ifndef utility_fixedvectorH
#define utility_fixedvectorH

#include <algorithm> // for std::max
#include <cassert>
#include <cmath>
#include <type_traits>

/**
 * A simple fixed size vector that provides basic mathematical operations.
 *
 * @tparam T Type of stored elements. Should be an arithmetic type.
 * @tparam D Dimension/size of the vector.
 */
template <typename T, std::size_t D>
class cFixedVector
{
public:
	using value_type = T;
	using reference_type = T&;
	using const_reference_type = const T&;
	using point_type = T*;
	using const_point_type = const T*;
	using const_size = std::integral_constant<std::size_t, D>;

	using self_type = cFixedVector<T, D>;

	cFixedVector();
	template <typename U>
	cFixedVector (const cFixedVector<U, D>& other);

	/**
	 * Returns a single element of the vector.
	 * @param index The index of the vector. Should be 0 <= index <= size()
	 * @return The element with the given index.
	 */
	reference_type operator[] (std::size_t index);
	const_reference_type operator[] (std::size_t index) const;

	//
	// arithmetic operations
	//

	self_type& operator= (const value_type& value);

	self_type operator+ (const value_type& value) const;
	self_type operator- (const value_type& value) const;
	self_type operator* (const value_type& value) const;
	self_type operator/ (const value_type& value) const;

	self_type& operator+= (const value_type& value);
	self_type& operator-= (const value_type& value);
	self_type& operator*= (const value_type& value);
	self_type& operator/= (const value_type& value);

	self_type operator+ (const self_type& other) const;
	self_type operator- (const self_type& other) const;
	self_type operator* (const self_type& other) const; // element wise
	self_type operator/ (const self_type& other) const; // element wise

	self_type& operator+= (const self_type& other);
	self_type& operator-= (const self_type& other);
	self_type& operator*= (const self_type& other); // element wise
	self_type& operator/= (const self_type& other); // element wise

	bool operator== (const value_type& value) const;
	bool operator!= (const value_type& value) const;

	bool operator== (const self_type& other) const;
	bool operator!= (const self_type& other) const;

	value_type dotProduct (const self_type& other) const;

	double l2Norm() const;
	value_type l2NormSquared() const;

	value_type max() const;
	self_type abs() const;

	/**
	 * Returns the fixed size of the vector.
	 * @return
	 */
	std::size_t size() const;

	/**
	 * Access to the underlying data.
	 * @return
	 */
	point_type data();
	const_point_type data() const;

private:
	T data_[D]{};
};

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
cFixedVector<T, D>::cFixedVector()
{}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
template <typename U>
cFixedVector<T, D>::cFixedVector (const cFixedVector<U, D>& other)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] = other[d];
	}
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::reference_type cFixedVector<T, D>::operator[] (std::size_t index)
{
	assert (index < D);
	return data_[index];
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::const_reference_type cFixedVector<T, D>::operator[] (std::size_t index) const
{
	assert (index < D);
	return data_[index];
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator+ (const value_type& value) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] + value;
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator- (const value_type& value) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] - value;
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator* (const value_type& value) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] * value;
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator/ (const value_type& value) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] / value;
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator= (const value_type& value)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] = value;
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator+= (const value_type& value)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] += value;
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator-= (const value_type& value)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] -= value;
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator*= (const value_type& value)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] *= value;
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator/= (const value_type& value)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] /= value;
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator+ (const self_type& other) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] + other[d];
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator- (const self_type& other) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] - other[d];
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator/ (const self_type& other) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] / other[d];
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::operator* (const self_type& other) const
{
	cFixedVector<T, D> result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = data_[d] * other[d];
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator+= (const self_type& other)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] += other[d];
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator-= (const self_type& other)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] -= other[d];
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator/= (const self_type& other)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] /= other[d];
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type& cFixedVector<T, D>::operator*= (const self_type& other)
{
	for (std::size_t d = 0; d < D; ++d)
	{
		data_[d] *= other[d];
	}
	return *this;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
bool cFixedVector<T, D>::operator== (const value_type& other) const
{
	for (std::size_t d = 0; d < D; ++d)
	{
		if (data_[d] != other) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
bool cFixedVector<T, D>::operator!= (const value_type& other) const
{
	return !((*this) == other);
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
bool cFixedVector<T, D>::operator== (const self_type& other) const
{
	for (std::size_t d = 0; d < D; ++d)
	{
		if (data_[d] != other[d]) return false;
	}
	return true;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
bool cFixedVector<T, D>::operator!= (const self_type& other) const
{
	return !((*this) == other);
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::value_type cFixedVector<T, D>::dotProduct (const self_type& other) const
{
	value_type result = 0;
	for (std::size_t d = 0; d < D; ++d)
	{
		result += data_[d] * other[d];
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
double cFixedVector<T, D>::l2Norm() const
{
	return std::sqrt (dotProduct (*this));
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::value_type cFixedVector<T, D>::l2NormSquared() const
{
	return dotProduct (*this);
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::value_type cFixedVector<T, D>::max() const
{
	value_type result = data_[0];
	for (std::size_t d = 1; d < D; ++d)
	{
		result = std::max (result, data_[d]);
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::self_type cFixedVector<T, D>::abs() const
{
	self_type result;
	for (std::size_t d = 0; d < D; ++d)
	{
		result[d] = std::abs (data_[d]);
	}
	return result;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
std::size_t cFixedVector<T, D>::size() const
{
	return D;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::point_type cFixedVector<T, D>::data()
{
	return data_;
}

//------------------------------------------------------------------------------
template <typename T, std::size_t D>
typename cFixedVector<T, D>::const_point_type cFixedVector<T, D>::data() const
{
	return data_;
}

#endif // utility_fixedvectorH
