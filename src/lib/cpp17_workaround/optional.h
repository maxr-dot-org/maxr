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

#ifndef cpp17_workaround_optionalH
#define cpp17_workaround_optionalH

#if __cplusplus < 201700

# include <type_traits>
# include <utility>

namespace std
{

	inline namespace compatibility_cpp17
	{

		struct nullopt_t
		{
			enum class Construct
			{
				Token
			};

			explicit constexpr nullopt_t (Construct) {}
		};

		extern const nullopt_t nullopt;

		template <typename T>
		class optional
		{
			static_assert (!std::is_reference<T>::value, "!");

		public:
			optional() = default;
			optional (nullopt_t) {}
			optional (const T& data) :
				valid (true) { new (&u.data) T (data); }
			optional (T&& data) :
				valid (true) { new (&u.data) T (std::move (data)); }
			optional (const optional& rhs) :
				valid (rhs.valid)
			{
				if (valid) new (&u.data) T (rhs.u.data);
			}
			optional (optional&& rhs) :
				valid (rhs.valid)
			{
				if (valid) new (&u.data) T (std::move (rhs.u.data));
			}
			~optional() { deconstruct(); }

			optional& operator= (const optional& rhs)
			{
				deconstruct();
				valid = rhs.valid;
				if (valid) new (&u.data) T (rhs.u.data);
				return *this;
			}
			optional& operator= (optional&& rhs) noexcept
			{
				deconstruct();
				valid = rhs.valid;
				if (valid) new (&u.data) T (std::move (rhs.u.data));
				return *this;
			}
			optional& operator= (const T& data)
			{
				*this = optional (std::move (data));
				return *this;
			}
			optional& operator= (T&& data)
			{
				*this = optional (std::move (data));
				return *this;
			}

			const T& operator*() const { return u.data; }
			T& operator*() { return u.data; }
			const T* operator->() const { return &u.data; }
			T* operator->() { return &u.data; }

			explicit operator bool() const { return valid; }

			bool has_value() const { return valid; }
			const T& value() const { return u.data; }
			T& value() { return u.data; }

			template <typename U>
			T value_or (U&& u) const
			{
				return valid ? value() : T (std::forward<U> (u));
			}

			template <typename... Ts>
			T& emplace (Ts&&... args)
			{
				deconstruct();
				valid = true;
				new (&u.data) T (std::forward<Ts> (args)...);
				return value();
			}

			bool operator== (const optional& rhs) const
			{
				if (valid != rhs.valid) return false;
				if (!valid) return true;
				return value() == rhs.value();
			}

			bool operator!= (const optional& rhs) const { return !(*this == rhs); }

		private:
			void deconstruct()
			{
				if (valid) u.data.~T();
			}

		private:
			bool valid = false;
			union U
			{
				char dummy;
				T data;
				U() :
					dummy (0) {}
				~U() {}
			} u;
		};

		template <typename T>
		std::optional<std::decay_t<T>> make_optional (T&& t)
		{
			return {std::forward<T> (t)};
		}

	} // namespace compatibility_cpp17

} // namespace std
#else
# include <optional>
#endif

#endif
