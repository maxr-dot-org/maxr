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

#ifndef utility_functiontraitsH
#define utility_functiontraitsH

#include <tuple>

/**
 * Provides traits for function types.
 *
 * NOTE: maybe we should name this 'function_traits' to be more consistent with
 *       the standard libraries traits names.
 *
 * @tparam F
 */
template <typename F>
struct sFunctionTraits : public sFunctionTraits<decltype (&F::operator())>
{};

template <typename R, typename... Args>
struct sFunctionTraits<R (Args...)>
{
	using result_type = R;

	static const size_t arity = sizeof...(Args);

	template <size_t N>
	struct argument
	{
		static_assert (N < arity, "Invalid argument index.");
		using type = std::tuple_element_t<N, std::tuple<Args...>>;
	};
};

template <typename R, typename... Args>
struct sFunctionTraits<R (*) (Args...)> : public sFunctionTraits<R (Args...)>
{};

template <typename C, typename R, typename... Args>
struct sFunctionTraits<R (C::*) (Args...)> : public sFunctionTraits<R (Args...)>
{};

template <typename C, typename R, typename... Args>
struct sFunctionTraits<R (C::*) (Args...) const> : public sFunctionTraits<R (Args...)>
{};

#endif // utility_functiontraitsH
