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

#ifndef chatcommand_chatcommandargumentsH
#define chatcommand_chatcommandargumentsH

#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

size_t getNextWordLength (const std::string& s, size_t position);

class cPlayer;
class cClient;
class cServer;

class cChatCommandArgumentBool
{
public:
	using ValueType = bool;

	static const char* const trueName;
	static const char* const falseName;

	explicit cChatCommandArgumentBool (bool isOptional = false, ValueType defaultValue = false);

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	ValueType value;
	const ValueType defaultValue;
};

template <typename T>
class cChatCommandArgumentInt
{
public:
	using ValueType = T;

	explicit cChatCommandArgumentInt (std::string name, bool isOptional = false, ValueType defaultValue = 0) 
:		isOptional (isOptional),
		name (std::move (name)),
		defaultValue (std::move (defaultValue))
	{}

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const { return value; }

private:
	bool isOptional;
	std::string name;
	ValueType value{};
	const ValueType defaultValue;
};

class cChatCommandArgumentChoice
{
public:
	using ValueType = std::string;

	explicit cChatCommandArgumentChoice (std::vector<std::string> choices, bool isOptional = false, size_t defaultSelection = 0);

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	std::vector<std::string> choices;
	size_t currentSelection;
	size_t defaultSelection;
};

class cChatCommandArgumentString
{
public:
	using ValueType = std::string;

	explicit cChatCommandArgumentString (std::string name, bool isOptional = false, ValueType defaultValue = "");

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	std::string name;
	ValueType value;
	const ValueType defaultValue;
};

class cChatCommandArgumentServer
{
public:
	using ValueType = cServer*;

	explicit cChatCommandArgumentServer (cServer*& serverPointer, bool isOptional = false, ValueType defaultValue = nullptr);

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	ValueType value;
	const ValueType defaultValue;
	cServer*& serverPointer;
};

class cChatCommandArgumentClient
{
public:
	using ValueType = cClient*;

	explicit cChatCommandArgumentClient (const std::shared_ptr<cClient>& activeClientPointer, bool isOptional = false, ValueType defaultValue = nullptr);

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	ValueType value;
	const ValueType defaultValue;
	const std::shared_ptr<cClient>& activeClientPointer;
};

class cChatCommandArgumentServerPlayer
{
public:
	using ValueType = const cPlayer*;

	explicit cChatCommandArgumentServerPlayer (cServer*& serverPointer, bool isOptional = false, ValueType defaultValue = nullptr);

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	ValueType value;
	const ValueType defaultValue;
	cServer*& serverPointer;
};

class cChatCommandArgumentClientPlayer
{
public:
	using ValueType = const cPlayer*;

	explicit cChatCommandArgumentClientPlayer (const std::shared_ptr<cClient>& activeClientPointer, bool isOptional = false, ValueType defaultValue = nullptr);

	size_t parse (const std::string& command, size_t position);

	std::string toString() const;

	const ValueType& getValue() const;

private:
	bool isOptional;
	ValueType value;
	const ValueType defaultValue;
	const std::shared_ptr<cClient>& activeClientPointer;
};

//------------------------------------------------------------------------------
template <typename T>
size_t cChatCommandArgumentInt<T>::parse (const std::string& command, size_t position)
{
	const auto nextWordLength = getNextWordLength (command, position);

	bool success = true;
	size_t pos;
	long long longValue;
	try
	{
		longValue = std::stoll (command.substr (position, nextWordLength), &pos);
	}
	catch (const std::exception&)
	{
		success = false;
	}

	if (pos != nextWordLength)
	{
		success = false;
	}

	if (success && (longValue < static_cast<long long> (std::numeric_limits<ValueType>::min()) || longValue > static_cast<long long> (std::numeric_limits<ValueType>::max())))
	{
		success = false;
	}

	if (!success)
	{
		if (isOptional)
		{
			value = defaultValue;
			return position;
		}
		else
		{
			std::stringstream errorString;
			if (nextWordLength == 0)
			{
				throw std::runtime_error ("Missing integer argument <" + name + ">");
			}
			else
			{
				// TODO: translate
				errorString << "'" << command.substr (position, nextWordLength) << "' is not a valid integer";
			}
			throw std::runtime_error (errorString.str());
		}
	}
	else
	{
		value = static_cast<ValueType> (longValue);
	}
	return position + nextWordLength;
}

//------------------------------------------------------------------------------
template <typename T>
std::string cChatCommandArgumentInt<T>::toString() const
{
	std::stringstream result;
	if (isOptional) result << "[";
	result << "<" << name << ">";
	if (isOptional) result << "]";
	return result.str();
}

#endif
