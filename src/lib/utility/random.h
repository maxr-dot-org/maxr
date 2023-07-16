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

#ifndef utility_randomH
#define utility_randomH

#include <array>
#include <cassert>
#include <random>
#include <stdexcept>

/**
 * Creates a uniform distributed random number in the interval [min, max).
 *
 * It is required that min < max.
 *
 * @tparam T Has to be an arithmetic type (integral or floating point).
 *           NOTE: char is not yet supported since it is not an IntType by the definition of the C++ standard
 *                 and therefor it is not supported by std::uniform_int_distribution.
 * @param min Lower bound of the interval.
 * @param max Upper bound of the interval.
 * @return Randomly selected value.
 */
template <typename T>
T random (const T min, const T max)
{
	assert (max > min);

	static_assert (std::is_arithmetic<T>::value, "Can not generate random number for non-arithmetic types");
	using DistributionType = std::conditional_t<
		std::is_integral<T>::value,
		std::uniform_int_distribution<T>,
		std::uniform_real_distribution<T>>;

	static std::random_device rd;
	static std::mt19937 gen (rd());

	DistributionType distribution (min, std::is_integral<T>::value ? max - 1 : max);

	return distribution (gen);
}

/**
 * Creates a uniform distributed random number in the interval [0, max).
 *
 * @tparam T Has to be an arithmetic type (integral or floating point).
 *           NOTE: char is not yet supported since it is not an IntType by the definition of the C++ standard
 *                 and therefor it is not supported by std::uniform_int_distribution (see http://open-std.org/JTC1/SC22/WG21/docs/lwg-active.html#2326).
 * @param max Upper bound of the interval.
 * @return Randomly selected value.
 */
template <typename T>
T random (const T max)
{
	return random (T (0), max);
}

inline bool randomBernoulli()
{
	return random (2) == 1;
}

/**
 * Select a random element from a std::array
 */
template <typename T, size_t N>
typename std::array<T, N>::reference getRandom (std::array<T, N>& data)
{
	static_assert (N > 0, "Getting random element from empty array is not allowed");
	return data[random (N)];
}

/**
 * Select a random element from a std::array
 */
template <typename T, size_t N>
typename std::array<T, N>::const_reference getRandom (const std::array<T, N>& data)
{
	static_assert (N > 0, "Getting random element from empty array is not allowed");
	return data[random (N)];
}

/**
 * Select a random element from a std::vector
 */
template <typename T>
const T& getRandom (const std::vector<T>& v)
{
	if (v.empty()) {
		throw std::runtime_error ("Empty vector");
	}
	return v[random (v.size())];
}

#endif // utility_randomH
