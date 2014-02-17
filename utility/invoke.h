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

#ifndef utility_invokeH
#define utility_invokeH

#include <functional>
#include <tuple>

#include "../maxrconfig.h"

#include "functiontraits.h"

namespace detail {

#if MAXR_NO_VARIADIC_TEMPLATES

template<typename F>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<>& args)
{
	static_assert(sFunctionTraits<F>::arity == 0, "Tuple size does not match functions argument count");
	return func();
}

template<typename F, typename Arg1>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<Arg1>& args)
{
	static_assert(sFunctionTraits<F>::arity == 1, "Tuple size does not match functions argument count");
	return func(std::get<0>(args));
}

template<typename F, typename Arg1, typename Arg2>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<Arg1, Arg2>& args)
{
	static_assert(sFunctionTraits<F>::arity == 2, "Tuple size does not match functions argument count");
	return func (std::get<0> (args), std::get<1> (args));
}

template<typename F, typename Arg1, typename Arg2, typename Arg3>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<Arg1, Arg2, Arg3>& args)
{
	static_assert(sFunctionTraits<F>::arity == 3, "Tuple size does not match functions argument count");
	return func (std::get<0> (args), std::get<1> (args), std::get<2> (args));
}

template<typename F, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<Arg1, Arg2, Arg3, Arg4>& args)
{
	static_assert(sFunctionTraits<F>::arity == 4, "Tuple size does not match functions argument count");
	return func (std::get<0> (args), std::get<1> (args), std::get<2> (args), std::get<3> (args));
}

template<typename F, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<Arg1, Arg2, Arg3, Arg4, Arg5>& args)
{
	static_assert(sFunctionTraits<F>::arity == 5, "Tuple size does not match functions argument count");
	return func (std::get<0> (args), std::get<1> (args), std::get<2> (args), std::get<3> (args), std::get<4> (args));
}

#else

template<int ...>
struct sequence {};

template<int N, int ...S>
struct generate_sequence : generate_sequence<N-1, N-1, S...> {};

template<int ...S>
struct generate_sequence<0, S...>
{
    typedef sequence<S...> type;
};

template<typename F, typename ArgsTuple, int ...S>
typename sFunctionTraits<F>::result_type invoke_impl (const std::function<F>& func, const ArgsTuple& args, sequence<S...>)
{
    return func (std::get<S> (args)...);
}

} // namespace detail

template<typename F, typename... Args>
typename sFunctionTraits<F>::result_type invoke (const std::function<F>& func, const std::tuple<Args...>& args)
{
	static_assert(sFunctionTraits<F>::arity == sizeof...(Args), "Tuple size does not match functions argument count");
    return detail::invoke_impl (func, args, typename detail::generate_sequence<sizeof...(Args)>::type ());
}

#endif // MAXR_NO_VARIADIC_TEMPLATES

#endif // utility_invokeH
