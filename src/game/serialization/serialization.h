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

#include <array>
#include <cassert>
#include <chrono>
#include <forward_list>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include "config/workaround/cpp17/optional.h"

#include "nvp.h"
#include "utility/color.h"
#include "utility/position.h"

class cBuilding;
class cJob;
class cModel;
class cPlayer;
class cStaticUnitData;
class cUnit;
class cVehicle;
struct sID;

//used to constrain a template definition to use with out-archive types only
#define ENABLE_ARCHIVE_OUT                             \
	typename = std::enable_if_t<                       \
	std::is_same<Archive, cBinaryArchiveOut>::value || \
	std::is_same<Archive, cXmlArchiveOut>::value>

//used to constrain a template definition to use with in-archive types only
#define ENABLE_ARCHIVE_IN                              \
	typename = std::enable_if_t<                       \
	std::is_same<Archive, cBinaryArchiveIn>::value  || \
	std::is_same<Archive, cXmlArchiveIn>::value     || \
	std::is_same<Archive, cTextArchiveIn>::value>

//used to constrain a template definition to use with archive types only
#define ENABLE_ARCHIVES                                \
	typename = std::enable_if_t<                       \
	std::is_same<Archive, cBinaryArchiveOut>::value || \
	std::is_same<Archive, cXmlArchiveOut>::value    || \
	std::is_same<Archive, cBinaryArchiveIn>::value  || \
	std::is_same<Archive, cXmlArchiveIn>::value     || \
	std::is_same<Archive, cTextArchiveIn>::value>

namespace serialization
{
	// Customisation point for enum serialization
	template <typename E>
	struct sEnumSerializer
	{
		static std::string toString (E e) { return std::to_string (std::underlying_type_t<E> (e)); }
		static E fromString (const std::string& s)
		{
			std::stringstream ss (s);
			ss.imbue (std::locale ("C"));
			std::underlying_type_t<E> underlying = 0;
			ss >> underlying;

			if (ss.fail() || !ss.eof()) //test eof, because all characters in the string should belong to the converted value
				throw std::runtime_error ("Could not convert value " + s + " to " + typeid (E).name());
			return static_cast<E>(underlying);
		}
	};

	namespace detail
	{
		template <typename Archive, typename T>
		void splitFree (Archive& archive, T& value);
	}

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
			archive & makeNvp ("item", value[i]);
		}
	}
	//-------------------------------------------------------------------------
	template <typename Archive, typename T1, typename T2>
	void serialize (Archive& archive, std::pair<T1, T2>& value)
	{
		archive & makeNvp ("first", value.first);
		archive & makeNvp ("second", value.second);
	}
	//-------------------------------------------------------------------------
	template <typename Archive>
	void serialize (Archive& archive, cRgbColor& color)
	{
		archive & makeNvp ("r", color.r);
		archive & makeNvp ("g", color.g);
		archive & makeNvp ("b", color.b);
		archive & makeNvp ("a", color.a);
	}

	template <typename Archive>
	void serialize (Archive& archive, cPosition& position)
	{
		archive & serialization::makeNvp ("X", position[0]);
		archive & serialization::makeNvp ("Y", position[1]);
	}

	template <typename Archive>
	void serialize (Archive& archive, cVector2& vec)
	{
		archive & serialization::makeNvp ("X", vec[0]);
		archive & serialization::makeNvp ("Y", vec[1]);
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
			value[i] = c;
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
		if (valid) {
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
	/**
	* The class cPointerLoader is used to translate IDs into pointer values during deserialization.
	* It looks for an object with the given ID in the model and returns the pointer.
	* The object with the ID must exist in the model when the pointer is deserialized, otherwise an
	* exception is thrown.
	* Serialization/deserialization of a pointer needs the target class of the pointer to have
	* a getId() method. That method should return an unique ID to identify the
	* pointer target.
	*/
	class cPointerLoader
	{
	public:
		cPointerLoader (cModel& model);

		void get (int id, cPlayer*& value) const;
		void get (int id, const cPlayer*& value) const;
		void get (int id, cBuilding*& value) const;
		void get (int id, cVehicle*& value) const;
		void get (int id, cUnit*& value) const;
		void get (int id, const cUnit*& value) const;
		void get (sID id, const cStaticUnitData*& value) const;

		const cStaticUnitData* getBigRubbleData() const;
		const cStaticUnitData* getSmallRubbleData() const;

	private:
		cModel& model;
	};

	template <typename Archive, typename T>
	void save (Archive& archive, T* const value)
	{
		int id = value ? value->getId() : -1;
		archive << id;
	}
	template <typename Archive, typename T>
	void load (Archive& archive, T*& value)
	{
		assert (archive.getPointerLoader() != nullptr);

		int id;
		archive >> id;
		if (id == -1)
		{
			value = nullptr;
			return;
		}

		archive.getPointerLoader()->get (id, value);

		if (value == nullptr)
			throw std::runtime_error ("Error creating pointer to object from id");
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, T*& value)
	{
		serialization::detail::splitFree (archive, value);
	}

	template <typename Archive, typename T>
	void save (Archive& archive, const sNameValuePair<T*>& nvp)
	{
		int id = nvp.value ? nvp.value->getId() : -1;
		archive << makeNvp (nvp.name, id);
	}
	template <typename Archive, typename T>
	void load (Archive& archive, sNameValuePair<T*>& nvp)
	{
		assert (archive.getPointerLoader() != nullptr);

		int id;
		archive >> makeNvp (nvp.name, id);
		if (id == -1)
		{
			nvp.value = nullptr;
			return;
		}

		archive.getPointerLoader()->get (id, nvp.value);

		if (nvp.value == nullptr)
			throw std::runtime_error ("Error creating pointer to object from id");
	}
	template <typename Archive, typename T>
	void serialize (Archive& archive, sNameValuePair<T*> nvp)
	{
		serialization::detail::splitFree (archive, nvp);
	}

	//-------------------------------------------------------------------------
	namespace detail
	{
		struct sSplitMemberWriter
		{
			template <typename Archive, typename T>
			static void apply (Archive& archive, T& value)
			{
				value.save (archive);
			}
		};

		struct sSplitMemberReader
		{
			template <typename Archive, typename T>
			static void apply (Archive& archive, T& value)
			{
				value.load (archive);
			}
		};

		template <typename Archive, typename T>
		void splitMember (Archive& archive, T& value)
		{
			using operation = std::conditional_t<Archive::isWriter, sSplitMemberWriter, sSplitMemberReader>;

			operation::apply (archive, value);
		}

		struct sSplitFreeWriter
		{
			template <typename Archive, typename T>
			static void apply (Archive& archive, T& value)
			{
				::serialization::save (archive, value);
			}
		};

		struct sSplitFreeReader
		{
			template <typename Archive, typename T>
			static void apply (Archive& archive, T& value)
			{
				::serialization::load (archive, value);
			}
		};

		template <typename Archive, typename T>
		void splitFree (Archive& archive, T& value)
		{
			using operation = std::conditional_t<Archive::isWriter, sSplitFreeWriter, sSplitFreeReader>;

			operation::apply (archive, value);
		}

	} //namespace detail

	//--------------------------------------------------------------------------
	template <typename Archive, typename T>
	void serialize (Archive& archive, const sNameValuePair<T>& nvp)
	{
		serialization::serialize (archive, nvp.value);
	}

	#define SERIALIZATION_SPLIT_MEMBER()                     \
	template <typename Archive>                              \
	void serialize (Archive& archive)                        \
	{                                                        \
		serialization::detail::splitMember (archive, *this); \
	}

	#define SERIALIZATION_SPLIT_FREE(T)                    \
	namespace serialization {                              \
	template <typename Archive>                            \
	void serialize (Archive& archive, T& value)            \
	{                                                      \
		serialization::detail::splitFree (archive, value); \
	}                                                      \
	}

} //namespace serialization

#endif //serialization_serializationH
