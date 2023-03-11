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

#ifndef cpp17_workaround_applyH
#define cpp17_workaround_applyH

#if __cplusplus < 201700

# include <tuple>

namespace std
{

	inline namespace compatibility_cpp17
	{
		namespace details
		{
			template <typename F, typename Tuple, std::size_t... Is>
			auto apply (F func, Tuple&& tuple, std::index_sequence<Is...>)
			{
				return func (std::get<Is> (std::forward<Tuple> (tuple))...);
			}
		} // namespace details

		template <typename F, typename Tuple>
		auto apply (F func, Tuple&& tuple)
		{
			using Seq = std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>;
			return details::apply (func, std::forward<Tuple> (tuple), Seq{});
		}

	} // namespace compatibility_cpp17

} // namespace std

#endif
#endif
