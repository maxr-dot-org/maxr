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

#include "utility/crc.h"
#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/jsonarchive.h"

#include <3rd/doctest/doctest.h>
#include <iostream>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
template <typename T>
std::ostream& operator<< (std::ostream& os, const std::vector<T>& v)
{
	os << '{';
	const char* sep = "";
	for (const auto& e : v)
	{
		os << sep << e;
		sep = ", ";
	}
	return os << '}';
}

//------------------------------------------------------------------------------
template <typename T, std::size_t N>
std::ostream& operator<< (std::ostream& os, const std::array<T, N>& a)
{
	os << '{';
	const char* sep = "";
	for (const auto& e : a)
	{
		os << sep << e;
		sep = ", ";
	}
	return os << '}';
}

//------------------------------------------------------------------------------
template <typename T1, typename T2>
std::ostream& operator<< (std::ostream& os, const std::pair<T1, T2>& p)
{
	return os << '{' << p.first << ", " << p.second << '}';
}

//------------------------------------------------------------------------------
template <typename T>
std::ostream& operator<< (std::ostream& os, const std::optional<T>& o)
{
	if (o)
	{
		return os << o.value();
	}
	return os << "{empty}";
}

//------------------------------------------------------------------------------
template <typename K, typename V>
std::ostream& operator<< (std::ostream& os, const std::map<K, V>& m)
{
	os << '{';
	const char* sep = "";
	for (const auto& p : m)
	{
		os << sep << p;
		sep = ", ";
	}
	return os << '}';
}

//------------------------------------------------------------------------------
template <typename T>
std::ostream& operator<< (std::ostream& os, const std::forward_list<T>& list)
{
	os << '{';
	const char* sep = "";
	for (const auto& e : list)
	{
		os << sep << e;
		sep = ", ";
	}
	return os << '}';
}

//------------------------------------------------------------------------------
template <typename... Ts>
std::ostream& operator<< (std::ostream& os, const std::variant<Ts...>& var)
{
	os << R"({ "type": )" << var.index << R"(, "value": )";
	std::visit ([&] (const auto& value) { os << value; });
	return os << '}';
}

namespace
{

	//--------------------------------------------------------------------------
	template <typename T>
	void checkBinarySaveLoadNvp (T expected)
	{
		std::vector<unsigned char> buffer;

		cBinaryArchiveOut out (buffer);
		out << serialization::makeNvp ("value", expected);

		cBinaryArchiveIn in (buffer.data(), buffer.size());
		T value;
		in >> serialization::makeNvp ("value", value);
		CHECK (expected == value);
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void checkBinarySaveLoad (T expected)
	{
		std::vector<unsigned char> buffer;

		cBinaryArchiveOut out (buffer);
		out << expected;

		cBinaryArchiveIn in (buffer.data(), buffer.size());
		T value;
		in >> value;
		CHECK (expected == value);
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void checkJsonSaveLoadNvp (T expected)
	{
		nlohmann::json json;
		cJsonArchiveOut out (json);
		cJsonArchiveIn in (json);

		T value;
		out << serialization::makeNvp ("value", expected);
		//std::cout << json.dump() << std::endl;
		in >> serialization::makeNvp ("value", value);

		CHECK (expected == value);
	}

	//--------------------------------------------------------------------------
	template <typename T>
	void checkJsonSaveLoad (T expected)
	{
		nlohmann::json json;
		cJsonArchiveOut out (json);
		cJsonArchiveIn in (json);

		T value;
		out << expected;
		//std::cout << json.dump() << std::endl;
		in >> value;

		CHECK (expected == value);
	}

	constexpr int n = 42;
	constexpr float f = 1.f / 3;
	constexpr double d = 1. / 3;
	const std::string s = "Maxr";
	const std::filesystem::path path = "/usr/local/file.txt";
	const std::vector<int> v{1, 1, 2, 3, 5, 8};
	constexpr std::array<int, 6> a{4, 8, 15, 16, 23};
	constexpr std::pair<int, int> p{40, 2};
	const std::optional<int> empty{};
	const std::optional<int> o = 42;
	const std::forward_list<int> list{1, 2, 3};
	const std::map<int, std::string> m{{1, "I"}, {2, "II"}, {5, "V"}};
	const std::variant<int, std::string> var1 = 42;
	const std::variant<int, std::string> var2 = "s";
} // namespace

//------------------------------------------------------------------------------
TEST_CASE ("BinarySerializationNvp")
{
	checkBinarySaveLoadNvp (n);
	checkBinarySaveLoadNvp (f);
	checkBinarySaveLoadNvp (d);
	checkBinarySaveLoadNvp (s);
	checkBinarySaveLoadNvp (path);
	checkBinarySaveLoadNvp (v);
	checkBinarySaveLoadNvp (a);
	checkBinarySaveLoadNvp (p);
	checkBinarySaveLoadNvp (empty);
	checkBinarySaveLoadNvp (o);
	checkBinarySaveLoadNvp (m);
	checkBinarySaveLoadNvp (list);
	checkBinarySaveLoadNvp (var1);
	checkBinarySaveLoadNvp (var2);
}

//------------------------------------------------------------------------------
TEST_CASE ("BinarySerialization")
{
	checkBinarySaveLoad (n);
	checkBinarySaveLoad (f);
	checkBinarySaveLoad (d);
	checkBinarySaveLoad (s);
	checkBinarySaveLoad (path);
	checkBinarySaveLoad (v);
	checkBinarySaveLoad (a);
	checkBinarySaveLoad (p);
	checkBinarySaveLoad (empty);
	checkBinarySaveLoad (o);
	checkBinarySaveLoad (m);
	checkBinarySaveLoad (list);
	checkBinarySaveLoadNvp (var1);
	checkBinarySaveLoadNvp (var2);
}

//------------------------------------------------------------------------------
TEST_CASE ("JsonSerializationNvp")
{
	checkJsonSaveLoadNvp (n);
	checkJsonSaveLoadNvp (f);
	checkJsonSaveLoadNvp (d);
	checkJsonSaveLoadNvp (s);
	checkJsonSaveLoadNvp (path);
	checkJsonSaveLoadNvp (v);
	checkJsonSaveLoadNvp (a);
	checkJsonSaveLoadNvp (p);
	checkJsonSaveLoadNvp (empty);
	checkJsonSaveLoadNvp (o);
	checkJsonSaveLoadNvp (m);
	checkJsonSaveLoadNvp (list);
	checkBinarySaveLoadNvp (var1);
	checkBinarySaveLoadNvp (var2);
}

//------------------------------------------------------------------------------
TEST_CASE ("JsonSerialization")
{
	checkJsonSaveLoad (n);
	checkJsonSaveLoad (f);
	checkJsonSaveLoad (d);
	checkJsonSaveLoad (s);
	checkJsonSaveLoad (path);
	checkJsonSaveLoad (v);
	checkJsonSaveLoad (a);
	checkJsonSaveLoad (p);
	checkJsonSaveLoad (empty);
	checkJsonSaveLoad (o);
	checkJsonSaveLoad (m);
	checkJsonSaveLoad (list);
	checkBinarySaveLoadNvp (var1);
	checkBinarySaveLoadNvp (var2);
}
