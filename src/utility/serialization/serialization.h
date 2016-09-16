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

#include <string>
#include <vector>
#include <chrono>

#include "game/logic/jobs.h"

class cModel;
class cPlayer;
class cBuilding;
class cVehicle;
class cUnit;
class cStaticUnitData;
struct sID;

//used to constrain a template definition to use with out-archive types only
#define ENABLE_ARCHIVE_OUT                       \
	typename = std::enable_if_t<                 \
	std::is_same<T, cBinaryArchiveOut>::value || \
	std::is_same<T, cXmlArchiveOut>::value>

//used to constrain a template definition to use with in-archive types only
#define ENABLE_ARCHIVE_IN                        \
	typename = std::enable_if_t<                 \
	std::is_same<T, cBinaryArchiveIn>::value  || \
	std::is_same<T, cXmlArchiveIn>::value     || \
	std::is_same<T, cTextArchiveIn>::value>

//used to constrain a template definition to use with archive types only
#define ENABLE_ARCHIVES                          \
	typename = std::enable_if_t<                 \
	std::is_same<T, cBinaryArchiveOut>::value || \
	std::is_same<T, cXmlArchiveOut>::value    || \
	std::is_same<T, cBinaryArchiveIn>::value  || \
	std::is_same<T, cXmlArchiveIn>::value     || \
	std::is_same<T, cTextArchiveIn>::value>

template<typename T, typename = std::enable_if_t <std::is_enum<T>::value>>
std::string enumToString(T value)
{
	std::stringstream ss;
	ss.imbue(std::locale("C"));
	ss << static_cast<int>(value);

	return ss.str();
}

namespace serialization
{
	//--------------------------------------------------------------------------
	// name value pair
	//--------------------------------------------------------------------------
	template <typename T>
	struct sNameValuePair
	{
		sNameValuePair(const std::string& name, T& value) :
			name(name),
			value(value)
		{}

		const std::string& name;
		T& value;
	};

	template<typename T>
	sNameValuePair<T> makeNvp(const std::string& name, T& value)
	{
		return sNameValuePair<T>(name, value);
	}
	template<typename T>
	const sNameValuePair<T> makeNvp(const std::string& name, const T& value)
	{
		T& value_nonconst = const_cast<T&>(value);
		return sNameValuePair<T>(name, value_nonconst);
	}
	#define NVP_QUOTE(x) #x
	#define NVP(value) serialization::makeNvp(NVP_QUOTE(value), value)

	//-------------------------------------------------------------------------
	namespace detail
	{
		struct sSplitMemberWriter
		{
			template<typename Archive, typename T>
			static void apply(Archive& archive, T& value)
			{
				value.save(archive);
			}
		};

		struct sSplitMemberReader
		{
			template<typename Archive, typename T>
			static void apply(Archive& archive, T& value)
			{
				value.load(archive);
			}
		};

		template<typename Archive, typename T>
		void splitMember(Archive& archive, T& value)
		{
			typedef std::conditional
				<
				Archive::isWriter,
				sSplitMemberWriter,
				sSplitMemberReader
				>::type operation;

			operation::apply(archive, value);
		}

		struct sSplitFreeWriter
		{
			template<typename Archive, typename T>
			static void apply(Archive& archive, T& value)
			{
				::serialization::save(archive, value);
			}
		};

		struct sSplitFreeReader
		{
			template<typename Archive, typename T>
			static void apply(Archive& archive, T& value)
			{
				::serialization::load(archive, value);
			}
		};

		template<typename Archive, typename T>
		void splitFree(Archive& archive, T& value)
		{
			typedef typename std::conditional
				<
				Archive::isWriter,
				sSplitFreeWriter,
				sSplitFreeReader
				>::type operation;

			operation::apply(archive, value);
		}

		struct sSerializeMember
		{
			template<typename T, typename A>
			static void serialize(A& archive, T& object)
			{
				object.serialize(archive);
			}
			template<typename T, typename A>
			static void serialize(A& archive, sNameValuePair<T>& nvp)
			{
				serialization::serialize(archive, nvp.value);
			}
		};

		struct sSerializeEnum
		{
			template<typename T, typename A>
			static void serialize(A& archive, T& enumValue)
			{
				if (archive.isWriter)
				{
					int tmp = static_cast<int>(enumValue);
					archive & tmp;
					//TODO: operator<< does not work
				}
				else
				{
					int tmp;
					archive & tmp;
					//TODO: operator>> does not work
					enumValue = static_cast<T>(tmp);
				}
			}
			template<typename T, typename A>
			static void serialize(A& archive, sNameValuePair<T>& nvp)
			{
				if (archive.isWriter)
				{
					int tmp = static_cast<int>(nvp.value);
					archive & makeNvp(nvp.name, tmp);
					//TODO: operator<< does not work
				}
				else
				{
					int tmp;
					archive & makeNvp(nvp.name, tmp);
					nvp.value = static_cast<T>(tmp);
					//TODO: operator>> does not work
				}
			}
		};
	} //namespace detail

	#define SERIALIZATION_SPLIT_MEMBER()                        \
	template<typename A>                                  \
	void serialize(A& archive)                            \
	{                                                           \
		serialization::detail::splitMember(archive, *this);     \
	}

	#define SERIALIZATION_SPLIT_FREE(T)                         \
	namespace serialization {                                   \
	template<typename A>                                  \
	void serialize(A& archive, T & value)                 \
	{                                                           \
		serialization::detail::splitFree(archive, value);       \
	}                                                           \
	}

	

	//
	// default serialize implementations
	//
	template<typename A, typename T>
	void serialize(A& archive, T& value)
	{
		typedef std::conditional<std::is_enum<T>::value, detail::sSerializeEnum, detail::sSerializeMember>::type serializeWrapper;
		serializeWrapper::serialize(archive, value);
	}

	template<typename A, typename T>
	void serialize(A& archive, sNameValuePair<T>& value)
	{
		typedef std::conditional<std::is_enum<T>::value, detail::sSerializeEnum, detail::sSerializeMember>::type serializeWrapper;
		serializeWrapper::serialize(archive, value);
	}

	//
	// free serialization functions (for e. g. STL types, pointers)
	//
	//-------------------------------------------------------------------------
	template<typename A, typename T1, typename T2>
	void serialize(A& archive, std::pair<T1, T2>& value)
	{
		archive & makeNvp("first", value.first);
		archive & makeNvp("second", value.second);
	}
	//-------------------------------------------------------------------------
	template<typename A, typename T>
	void save(A& archive, const std::vector<T>& value)
	{
		uint32_t length = value.size();
		archive << NVP(length);
		for (auto item : value)
		{
			archive << NVP(item);
		}
	}
	template<typename A, typename T>
	void load(A& archive, std::vector<T>& value)
	{
		uint32_t length;
		archive >> NVP(length);
		value.resize(length);
		for (size_t i = 0; i < length; i++)
		{
			T c;
			archive >> makeNvp("item", c);
			value[i] = c;
		}
	}
	template<typename A, typename T>
	void serialize(A& archive, std::vector<T>& value)
	{
		serialization::detail::splitFree(archive, value);
	}
	//-------------------------------------------------------------------------
	template<typename A>
	void save(A& archive, const std::string& value)
	{
		uint32_t length = value.length();
		archive << NVP(length);
		for (char c : value)
		{
			archive << c;
		}
	}
	template<typename A>
	void load(A& archive, std::string& value)
	{
		uint32_t length;
		archive >> length;
		value.clear();
		value.reserve(length);
		for (size_t i = 0; i < length; i++)
		{
			char c;
			archive >> c;
			value.push_back(c);
		}
	}
	template<typename A>
	void serialize(A& archive, std::string& value)
	{
		serialization::detail::splitFree(archive, value);
	}
	//-------------------------------------------------------------------------
	template<typename A>
	void save(A& archive, const std::chrono::seconds& value)
	{
		archive << makeNvp("seconds", value.count());
	}
	template<typename A>
	void load(A& archive, std::chrono::seconds& value)
	{
		long long tmp;
		archive >> makeNvp("seconds", tmp);
		value = std::chrono::seconds(tmp);
	}
	template<typename A>
	void serialize(A& archive, std::chrono::seconds& value)
	{
		serialization::detail::splitFree(archive, value);
	}
	//-------------------------------------------------------------------------
	/**
	* The class cPointerLoader is used to translate IDs into pointer values during deserialisation.
	* It looks for an object with the given ID in the model and returns the pointer.
	* The object with the ID must exist in the model when the pointer is deserialized, otherwiese an
	* exception is thrown.
	* Serialization/Deserialisatoin of a pointer needs the target class of the pointer to have
	* a getId() method. That method should return an unique ID to identify the
	* pointer target.
	*/
	class cPointerLoader
	{
	public:
		cPointerLoader(cModel& model);
		void get(int id, cJob*& value);
		void get(int id, cPlayer*& value);
		void get(int id, cBuilding*& value);
		void get(int id, cVehicle*& value);
		void get(int id, cUnit*& value);
		void get(sID id, const cStaticUnitData*& value);

	private:
		cModel& model;
	};

	template<typename A, typename T>
	void save(A& archive, const T* value)
	{
		int id = value ? value->getId() : -1;
		archive << id;
	}
	template<typename A, typename T>
	void load(A& archive, T*& value)
	{
		assert(archive.getPointerLoader() != nullptr);

		int id;
		archive >> id;
		if (id == -1)
		{
			value = nullptr;
			return;
		}

		archive.getPointerLoader()->get(id, value);

		if (value == nullptr)
			throw std::runtime_error("Error creating pointer to object from id");
	}
	template<typename A, typename T>
	void serialize(A& archive, T*& value)
	{
		serialization::detail::splitFree(archive, value);
	}

	template<typename A, typename T>
	void save(A& archive, const sNameValuePair<T*>& nvp)
	{
		int id = nvp.value ? nvp.value->getId() : -1;
		archive << makeNvp(nvp.name, id);
	}
	template<typename A, typename T>
	void load(A& archive, sNameValuePair<T*>& nvp)
	{
		assert(archive.getPointerLoader() != nullptr);
		
		int id;
		archive >> makeNvp(nvp.name, id);
		if (id == -1)
		{
			nvp.value = nullptr;
			return;
		}

		archive.getPointerLoader()->get(id, nvp.value);

		if (nvp.value == nullptr)
			throw std::runtime_error("Error creating pointer to object from id");
	}
	template<typename A, typename T>
	void serialize(A& archive, sNameValuePair<T*> nvp)
	{
		serialization::detail::splitFree(archive, nvp);
	}

} //namespace sersialization

#endif //serialization_serializationH
