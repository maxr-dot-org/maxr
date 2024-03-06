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

#ifndef serialization_jsonarchiveH
#define serialization_jsonarchiveH

#include "serialization.h"
#include "utility/log.h"
#include "utility/narrow_cast.h"

#include <nlohmann/json.hpp>

class cJsonArchiveOut
{
public:
	static const bool isWriter = true;

	explicit cJsonArchiveOut (nlohmann::json& json);

	//--------------------------------------------------------------------------
	template <typename T>
	cJsonArchiveOut& operator<< (const T& nvp)
	{
		pushValue (nvp);
		return *this;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	cJsonArchiveOut& operator& (const T& nvp)
	{
		pushValue (nvp);
		return *this;
	}

private:
	//--------------------------------------------------------------------------
	template <typename T>
	void pushValue (const serialization::sNameValuePair<T>& nvp)
	{
		//check invalid characters in element and attribute names
		assert (nvp.name.find_first_of ("<>\"= []?!&") == std::string::npos);

		if (json.contains (nvp.name))
		{
			Log.error ("Entry " + nvp.name + " already present. old data will be overwritten");
		}
		cJsonArchiveOut (json[nvp.name]) << nvp.value;
	}

	//--------------------------------------------------------------------------
	template <typename T, std::enable_if_t<std::is_class<T>::value, int> = 0>
	void pushValue (const T& object)
	{
		json = nlohmann::json::object();
		serialization::serialize (*this, const_cast<T&> (object));
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void pushValue (const T* ptr)
	{
		serialization::serialize (*this, ptr);
	}

	//--------------------------------------------------------------------------
	template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
	void pushValue (E e)
	{
		if (serialization::sEnumSerializer<E>::hasStringRepresentation)
		{
			json = serialization::sEnumSerializer<E>::toString (e);
		}
		else
		{
			static_assert (sizeof (E) <= sizeof (int), "!");
			json = static_cast<int> (e);
		}
	}

	//
	// push fundamental types
	//
	void pushValue (bool b) { json = b; }
	void pushValue (char c) { json = static_cast<int> (c); }
	void pushValue (signed char c) { json = static_cast<int> (c); }
	void pushValue (unsigned char c) { json = static_cast<int> (c); }
	void pushValue (signed short n) { json = n; }
	void pushValue (unsigned short n) { json = n; }
	void pushValue (signed int n) { json = n; }
	void pushValue (unsigned int n) { json = n; }
	void pushValue (signed long n) { json = n; }
	void pushValue (unsigned long n) { json = n; }
	void pushValue (signed long long n) { json = n; }
	void pushValue (unsigned long long n) { json = n; }
	void pushValue (float f) { json = f; }
	void pushValue (double d) { json = d; }

	//
	// push STL types
	//
	void pushValue (const std::string& s) { json = s; }

	//--------------------------------------------------------------------------
	template <typename T>
	void pushValue (const std::vector<T>& v)
	{
		auto arr = nlohmann::json::array();
		for (const auto& e : v)
		{
			cJsonArchiveOut (arr.emplace_back()) << e;
		}
		json = std::move (arr);
	}

	//--------------------------------------------------------------------------
	template <typename T, std::size_t N>
	void pushValue (const std::array<T, N>& a)
	{
		auto arr = nlohmann::json::array();
		for (const auto& e : a)
		{
			cJsonArchiveOut (arr.emplace_back()) << e;
		}
		json = std::move (arr);
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void pushValue (const std::forward_list<T>& list)
	{
		auto arr = nlohmann::json::array();
		for (const auto& e : list)
		{
			cJsonArchiveOut (arr.emplace_back()) << e;
		}
		json = std::move (arr);
	}

	//--------------------------------------------------------------------------
	template <typename K, typename V>
	void pushValue (const std::map<K, V>& m)
	{
		auto arr = nlohmann::json::array();
		for (const auto& p : m)
		{
			cJsonArchiveOut (arr.emplace_back()) << p;
		}
		json = std::move (arr);
	}

	//--------------------------------------------------------------------------
	template <typename T, typename Cmp>
	void pushValue (const cFlatSet<T, Cmp>& v)
	{
		auto arr = nlohmann::json::array();
		for (const auto& e : v)
		{
			cJsonArchiveOut (arr.emplace_back()) << e;
		}
		json = std::move (arr);
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void pushValue (const std::optional<T>& o)
	{
		if (o)
		{
			*this << *o;
		}
		else
		{
			json = nullptr;
		}
	}

private:
	nlohmann::json& json;
};

class cJsonArchiveIn
{
public:
	static const bool isWriter = false;

	explicit cJsonArchiveIn (const nlohmann::json& json, bool strict = true);

	//--------------------------------------------------------------------------
	template <typename T>
	cJsonArchiveIn& operator>> (T& t)
	{
		popValue (t);
		return *this;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	cJsonArchiveIn& operator& (T& t)
	{
		popValue (t);
		return *this;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	cJsonArchiveIn& operator>> (const serialization::sNameValuePair<T>& nvp)
	{
		popValue (nvp);
		return *this;
	}

	//--------------------------------------------------------------------------
	template <typename T>
	cJsonArchiveIn& operator& (const serialization::sNameValuePair<T>& nvp)
	{
		popValue (nvp);
		return *this;
	}

private:
	//--------------------------------------------------------------------------
	template <typename T>
	void popValue (const serialization::sNameValuePair<T>& nvp)
	{
		//check invalid characters in element and attribute names
		assert (nvp.name.find_first_of ("<>\"= []?!&") == std::string::npos);

		if (strict)
		{
			cJsonArchiveIn (json.at (nvp.name), strict) >> nvp.value;
		}
		else {
			auto it = json.find (nvp.name);

			if (it != json.end()) {
				cJsonArchiveIn (*it, strict) >> nvp.value;
			}
			else {
				Log.warn ("Entry " + nvp.name + " is missing.");
			}
		}
	}

	//--------------------------------------------------------------------------
	template <typename T, std::enable_if_t<std::is_class<T>::value, int> = 0>
	void popValue (T& object)
	{
		serialization::serialize (*this, object);
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void popValue (T*& ptr)
	{
		serialization::serialize (*this, ptr);
	}

	//--------------------------------------------------------------------------
	template <typename E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
	void popValue (E& e)
	{
		if (json.is_string())
		{
			assert (serialization::sEnumSerializer<E>::hasStringRepresentation);
			e = serialization::sEnumSerializer<E>::fromString (json);
		}
		else
		{
			static_assert (sizeof (E) <= sizeof (int), "!");
			int tmp = json;
			e = static_cast<E> (tmp);
		}
	}

	//
	// pop fundamental types
	//
	void popValue (bool& b) { b = json; }
	void popValue (char& c) { c = narrow_cast<char> (json.get<int>()); }
	void popValue (signed char& c) { c = narrow_cast<signed char> (json.get<int>()); }
	void popValue (unsigned char& c) { c = narrow_cast<unsigned char> (json.get<int>()); }
	void popValue (signed short& n) { n = json; }
	void popValue (unsigned short& n) { n = json; }
	void popValue (signed int& n) { n = json; }
	void popValue (unsigned int& n) { n = json; }
	void popValue (signed long& n) { n = json; }
	void popValue (unsigned long& n) { n = json; }
	void popValue (signed long long& n) { n = json; }
	void popValue (unsigned long long& n) { n = json; }
	void popValue (float& f) { f = json; }
	void popValue (double& d) { d = json; }

	//
	// pop STL types
	//
	void popValue (std::string& s) { s = json; }

	//--------------------------------------------------------------------------
	template <typename T>
	void popValue (std::vector<T>& v)
	{
		v.resize (json.size());
		std::size_t i = 0;
		for (const auto& e : json)
		{
			cJsonArchiveIn (e, strict) >> v[i++];
		}
	}

	//--------------------------------------------------------------------------
	template <typename T, std::size_t N>
	void popValue (std::array<T, N>& a)
	{
		std::size_t i = 0;

		assert (json.size() == N);
		for (const auto& e : json)
		{
			cJsonArchiveIn (e, strict) >> a[i++];
		}
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void popValue (std::forward_list<T>& list)
	{
		list.resize (json.size());
		auto it = list.begin();
		for (const auto& e : json)
		{
			cJsonArchiveIn (e, strict) >> *it++;
		}
	}

	//--------------------------------------------------------------------------
	template <typename K, typename V>
	void popValue (std::map<K, V>& m)
	{
		for (const auto& e : json)
		{
			std::pair<K, V> p;
			cJsonArchiveIn (e, strict) >> p;
			m.insert (p);
		}
	}

	//--------------------------------------------------------------------------
	template <typename T, typename Cmp>
	void popValue (cFlatSet<T, Cmp>& v)
	{
		for (const auto& e : json)
		{
			T item;
			cJsonArchiveIn (e, strict) >> item;
			v.insert (std::move (item));
		}
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void popValue (std::optional<T>& value)
	{
		if (json.is_null())
		{
			value = std::nullopt;
		}
		else
		{
			value.emplace();
			*this >> *value;
		}
	}

private:
	const nlohmann::json& json;
	bool strict;
};

#endif
