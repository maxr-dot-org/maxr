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

#ifndef serialization_serializationH
#define serialization_serializationH

#include "nvp.h"
#include "utility/color.h"
#include "utility/flatset.h"
#include "utility/log.h"
#include "utility/position.h"
#include "utility/ranges.h"

#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <forward_list>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <typeinfo>
#include <variant>
#include <vector>

//used to constrain a template definition to use with in-archive types only
#define ENABLE_ARCHIVE_IN std::enable_if_t<Archive::isWriter == false, int> = 0

//used to constrain a template definition to use with out-archive types only
#define ENABLE_ARCHIVE_OUT std::enable_if_t<Archive::isWriter == true, int> = 0

//used to constrain a template definition to use with archive types only
#define ENABLE_ARCHIVES std::enable_if_t<std::is_same<decltype (Archive::isWriter), bool>::value, int> = 0

namespace serialization
{
	// Customisation point for enum serialization
	template <typename E>
	struct sEnumStringMapping
	{};
	// template <> struct serialization::sEnumStringMapping<MyEnum>
	// {
	//     static const std::vector<std::pair<MyEnum, const char*>> m;
	// };
	// const std::vector<std::pair<MyEnum, const char*>> serialization::sEnumStringMapping<MyEnum>::m =
	// {
	//     { MyEnum::value1, "value1" },
	//     // ...
	// };

	template <typename E, typename Enabler = void>
	struct sEnumSerializer
	{
		static constexpr bool hasStringRepresentation = false;
		static std::string toString (E e) { return std::to_string (std::underlying_type_t<E> (e)); }
		static E fromString (const std::string& s)
		{
			std::stringstream ss (s);
			ss.imbue (std::locale ("C"));
			std::underlying_type_t<E> underlying = 0;
			ss >> underlying;

			if (ss.fail() || !ss.eof()) //test eof, because all characters in the string should belong to the converted value
				throw std::runtime_error ("Could not convert value " + s + " to " + typeid (E).name());
			return static_cast<E> (underlying);
		}
	};

	template <typename E>
	struct sEnumSerializer<E, decltype (sEnumStringMapping<E>::m, void())>
	{
		static constexpr bool hasStringRepresentation = true;
		static std::string toString (E e)
		{
			auto it = ranges::find_if (sEnumStringMapping<E>::m, [&] (const auto& p) { return p.first == e; });
			if (it != sEnumStringMapping<E>::m.end()) return it->second;
			Log.warn (std::string ("Unknown ") + typeid (E).name() + " " + std::to_string (static_cast<int> (e)));
			return std::to_string (static_cast<int> (e));
		}
		static E fromString (const std::string& s)
		{
			auto it = ranges::find_if (sEnumStringMapping<E>::m, [&] (const auto& p) { return p.second == s; });
			if (it != sEnumStringMapping<E>::m.end()) return it->first;

			Log.warn (std::string ("Unknown ") + typeid (E).name() + " value " + s);
			throw std::runtime_error (std::string ("Unknown ") + typeid (E).name() + " value " + s);
		}
	};

	template <typename E>
	decltype (sEnumStringMapping<E>::m, std::string{}) enumToString (E e)
	{
		return sEnumSerializer<E>::toString (e);
	}

	namespace detail
	{
		template <typename Archive, typename T>
		void splitFree (Archive& archive, T& value);
	} // namespace detail

	//
	// default serialize implementations
	//
	template <typename Archive, typename T>
	auto serialize (Archive& archive, T& object) -> decltype (object.serialize (archive))
	{
		object.serialize (archive);
	}

	//
	// free serialization functions (for e. g. STL types, pointers)
	//

	//--------------------------------------------------------------------------
	template <typename Archive, typename T, size_t SIZE>
	void serialize (Archive& archive, std::array<T, SIZE>& value)
	{
		for (size_t i = 0; i < SIZE; i++)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & makeNvp ("item", value[i]);
			// clang-format on
		}
	}

	//-------------------------------------------------------------------------
	template <typename Archive, typename T1, typename T2>
	void serialize (Archive& archive, std::pair<T1, T2>& value)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & makeNvp ("first", value.first);
		archive & makeNvp ("second", value.second);
		// clang-format on
	}

	//-------------------------------------------------------------------------
	template <typename Archive>
	void serialize (Archive& archive, cRgbColor& color)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & makeNvp ("r", color.r);
		archive & makeNvp ("g", color.g);
		archive & makeNvp ("b", color.b);
		archive & makeNvp ("a", color.a);
		// clang-format on
	}

	template <typename Archive>
	void serialize (Archive& archive, cPosition& position)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & serialization::makeNvp ("X", position[0]);
		archive & serialization::makeNvp ("Y", position[1]);
		// clang-format on
	}

	template <typename Archive>
	void serialize (Archive& archive, cVector2& vec)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive& serialization::makeNvp ("X", vec[0]);
		archive& serialization::makeNvp ("Y", vec[1]);
		// clang-format on
	}

	//-------------------------------------------------------------------------
	template <typename Archive, typename T>
	void save (Archive& archive, const std::shared_ptr<T>& value)
	{
		if (value)
		{
			serialize (archive, *value);
		}
		else
		{
			throw std::runtime_error ("Unexpected null shared_ptr");
		}
	}
	template <typename Archive, typename T>
	void load (Archive& archive, std::shared_ptr<T>& value)
	{
		value = T::createFrom (archive);
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, std::shared_ptr<T>& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive, typename T>
	void save (Archive& archive, const std::unique_ptr<T>& value)
	{
		if (value)
		{
			serialize (archive, *value);
		}
		else
		{
			throw std::runtime_error ("Unexpected null unique_ptr");
		}
	}
	template <typename Archive, typename T>
	void load (Archive& archive, std::unique_ptr<T>& value)
	{
		value = T::createFrom (archive);
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, std::unique_ptr<T>& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive, typename T>
	void save (Archive& archive, const std::vector<T>& value)
	{
		uint32_t length = static_cast<uint32_t> (value.size());
		archive << NVP (length);
		for (auto& item : value)
		{
			archive << NVP (item);
		}
	}
	template <typename Archive, typename T>
	void load (Archive& archive, std::vector<T>& value)
	{
		uint32_t length;
		archive >> NVP (length);
		value.resize (length);
		for (size_t i = 0; i < length; i++)
		{
			T c;
			archive >> makeNvp ("item", c);
			value[i] = std::move (c);
		}
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, std::vector<T>& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive>
	void save (Archive& archive, const std::string& value)
	{
		uint32_t length = static_cast<uint32_t> (value.length());
		archive << NVP (length);
		for (char c : value)
		{
			archive << c;
		}
	}
	template <typename Archive>
	void load (Archive& archive, std::string& value)
	{
		uint32_t length;
		archive >> length;
		value.clear();
		value.reserve (length);
		for (size_t i = 0; i < length; i++)
		{
			char c;
			archive >> c;
			value.push_back (c);
		}
	}
	template <typename Archive>
	void serialize (Archive& archive, std::string& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive>
	void save (Archive& archive, const std::filesystem::path& value)
	{
		archive << value.u8string();
	}
	template <typename Archive>
	void load (Archive& archive, std::filesystem::path& value)
	{
		std::string s;
		archive >> s;
		value = std::filesystem::u8path (s);
	}
	template <typename Archive>
	void serialize (Archive& archive, std::filesystem::path& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive>
	void save (Archive& archive, const std::chrono::milliseconds& value)
	{
		archive << makeNvp ("milliseconds", value.count());
	}
	template <typename Archive>
	void load (Archive& archive, std::chrono::milliseconds& value)
	{
		long long tmp;
		archive >> makeNvp ("milliseconds", tmp);
		value = std::chrono::milliseconds (tmp);
	}
	template <typename Archive>
	void serialize (Archive& archive, std::chrono::milliseconds& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive>
	void save (Archive& archive, const std::chrono::seconds& value)
	{
		archive << makeNvp ("seconds", value.count());
	}
	template <typename Archive>
	void load (Archive& archive, std::chrono::seconds& value)
	{
		long long tmp;
		archive >> makeNvp ("seconds", tmp);
		value = std::chrono::seconds (tmp);
	}
	template <typename Archive>
	void serialize (Archive& archive, std::chrono::seconds& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive, typename... Ts>
	void save (Archive& archive, const std::variant<Ts...>& var)
	{
		archive << makeNvp ("type", std::uint32_t (var.index()));
		std::visit ([&] (const auto& value) { archive << makeNvp ("value", value); }, var);
	}
	namespace detail
	{
		// Some version of compilers (gcc)
		// Doesn't like expanding lambda
		// So create regular template function
		template <typename T, typename Archive, typename Variant>
		void loadT (Archive& archive, Variant& var)
		{
			T value;
			archive >> makeNvp ("value", value);
			var = value;
		}

	} // namespace detail
	template <typename Archive, typename... Ts>
	void load (Archive& archive, std::variant<Ts...>& var)
	{
		std::uint32_t type;
		archive >> makeNvp ("type", type);
		using LoaderType = void (Archive&, std::variant<Ts...>&);
		LoaderType* loaders[] = {&detail::loadT<Ts>...};
		loaders[type](archive, var);
	}
	template <typename Archive, typename... Ts>
	void serialize (Archive& archive, std::variant<Ts...>& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//------------------------------------------------------------------------------
	template <typename Archive, typename T, typename Cmp>
	void save (Archive& archive, const cFlatSet<T, Cmp>& value)
	{
		uint32_t length = static_cast<uint32_t> (value.size());
		archive << NVP (length);
		for (const auto& item : value)
		{
			archive << NVP (item);
		}
	}
	template <typename Archive, typename T, typename Cmp>
	void load (Archive& archive, cFlatSet<T, Cmp>& value)
	{
		uint32_t length;
		archive >> NVP (length);
		for (size_t i = 0; i < length; i++)
		{
			T item;
			archive >> NVP (item);
			value.insert (std::move (item));
		}
	}
	template <typename Archive, typename T, typename Cmp>
	void serialize (Archive& archive, cFlatSet<T, Cmp>& value)
	{
		serialization::detail::splitFree (archive, value);
	}
	//------------------------------------------------------------------------------
	template <typename Archive, typename K, typename T>
	void save (Archive& archive, const std::map<K, T>& value)
	{
		uint32_t length = static_cast<uint32_t> (value.size());
		archive << NVP (length);
		for (auto& pair : value)
		{
			archive << NVP (pair);
		}
	}
	template <typename Archive, typename K, typename T>
	void load (Archive& archive, std::map<K, T>& value)
	{
		uint32_t length;
		archive >> NVP (length);
		for (size_t i = 0; i < length; i++)
		{
			std::pair<K, T> c;
			archive >> makeNvp ("pair", c);
			value.insert (c);
		}
	}
	template <typename Archive, typename K, typename T>
	void serialize (Archive& archive, std::map<K, T>& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	template <typename Archive, typename T>
	void save (Archive& archive, const std::forward_list<T>& value)
	{
		const uint32_t length = std::distance (value.begin(), value.end());
		archive << NVP (length);

		for (const auto& item : value)
		{
			archive << NVP (item);
		}
	}
	template <typename Archive, typename T>
	void load (Archive& archive, std::forward_list<T>& value)
	{
		uint32_t length;
		archive >> NVP (length);
		value.resize (length);

		for (auto& item : value)
		{
			archive >> NVP (item);
		}
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, std::forward_list<T>& value)
	{
		serialization::detail::splitFree (archive, value);
	}
	//-------------------------------------------------------------------------
	template <typename Archive, typename T>
	void save (Archive& archive, const std::optional<T>& value)
	{
		archive << makeNvp ("valid", static_cast<bool> (value));
		if (value)
		{
			archive << makeNvp ("data", *value);
		}
	}
	template <typename Archive, typename T>
	void load (Archive& archive, std::optional<T>& value)
	{
		bool valid = false;
		archive >> makeNvp ("valid", valid);
		if (valid)
		{
			value = T{};
			archive >> makeNvp ("data", *value);
		}
		else
		{
			value = std::nullopt;
		}
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, std::optional<T>& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	//-------------------------------------------------------------------------
	namespace detail
	{
		template <typename Archive, typename T>
		void splitMember (Archive& archive, T& value)
		{
			if constexpr (Archive::isWriter)
			{
				value.save (archive);
			}
			else
			{
				value.load (archive);
			}
		}

		template <typename Archive, typename T>
		void splitFree (Archive& archive, T& value)
		{
			if constexpr (Archive::isWriter)
			{
				::serialization::save (archive, value);
			}
			else
			{
				::serialization::load (archive, value);
			}
		}

	} //namespace detail

	//--------------------------------------------------------------------------
	template <typename Archive, typename T>
	void serialize (Archive& archive, const sNameValuePair<T>& nvp)
	{
		serialization::serialize (archive, nvp.value);
	}

#define SERIALIZATION_SPLIT_MEMBER() \
 template <typename Archive> \
 void serialize (Archive& archive) \
 { \
  serialization::detail::splitMember (archive, *this); \
 }

#define SERIALIZATION_SPLIT_FREE(T) \
 namespace serialization \
 { \
  template <typename Archive> \
  void serialize (Archive& archive, T& value) \
  { \
   serialization::detail::splitFree (archive, value); \
  } \
 }

} //namespace serialization

#endif //serialization_serializationH
