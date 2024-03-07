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

#include "chatcommandarguments.h"

#include "game/logic/client.h"
#include "game/logic/server.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>

/*static*/ const char* const cChatCommandArgumentBool::trueName = "on";
/*static*/ const char* const cChatCommandArgumentBool::falseName = "off";

//------------------------------------------------------------------------------
size_t getNextWordLength (const std::string& s, size_t position)
{
	const auto begin = s.begin() + position;
	const auto end = std::find_if (begin, s.end(), [] (unsigned char c) { return std::isspace (c); });
	return end - begin;
}

//------------------------------------------------------------------------------
cChatCommandArgumentBool::cChatCommandArgumentBool (bool isOptional, ValueType defaultValue) :
	isOptional (isOptional),
	value (defaultValue),
	defaultValue (defaultValue)
{}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentBool::parse (const std::string& command, size_t position)
{
	const auto nextWordLength = getNextWordLength (command, position);
	if (command.compare (position, nextWordLength, trueName) == 0)
	{
		value = true;
		return position + nextWordLength;
	}
	else if (command.compare (position, nextWordLength, falseName) == 0)
	{
		value = false;
		return position + nextWordLength;
	}
	else
	{
		if (this->isOptional)
		{
			value = defaultValue;
			return position;
		}
		else
		{
			std::stringstream errorString;
			// TODO: translate
			if (nextWordLength == 0)
			{
				errorString << "Missing boolean argument (" << trueName << "/" << falseName << ")";
			}
			else
			{
				errorString << "'" << command.substr (position, nextWordLength) << "' could not be recognized as boolean argument (" << trueName << "/" << falseName << ")";
			}
			throw std::runtime_error (errorString.str());
		}
	}
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentBool::toString() const
{
	std::stringstream result;
	if (this->isOptional) result << "[";
	result << "{" << trueName << "/" << falseName << "}";
	if (this->isOptional) result << "]";
	return result.str();
}

//------------------------------------------------------------------------------
const cChatCommandArgumentBool::ValueType& cChatCommandArgumentBool::getValue() const
{
	return value;
}

//------------------------------------------------------------------------------
cChatCommandArgumentChoice::cChatCommandArgumentChoice (std::vector<std::string> choices, bool isOptional, size_t defaultSelection) :
	isOptional (isOptional),
	choices (std::move (choices)),
	currentSelection (defaultSelection),
	defaultSelection (defaultSelection)
{
	assert (defaultSelection < this->choices.size());
}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentChoice::parse (const std::string& command, size_t position)
{
	const auto nextWordLength = getNextWordLength (command, position);
	bool success = false;
	for (size_t i = 0; i < choices.size(); ++i)
	{
		if (nextWordLength != choices[i].size()) continue;
		if (command.compare (position, nextWordLength, choices[i]) == 0)
		{
			currentSelection = i;
			success = true;
			break;
		}
	}

	if (!success)
	{
		if (isOptional)
		{
			currentSelection = defaultSelection;
			return position;
		}
		else
		{
			std::stringstream errorString;
			if (nextWordLength == 0)
			{
				errorString << "Missing argument (";
			}
			else
			{
				// TODO: translate
				errorString << "'" << command.substr (position, nextWordLength) << "' does not match any of the allowed values (";
			}
			if (!choices.empty())
			{
				errorString << choices.front();
				for (size_t i = 1; i < choices.size(); ++i)
				{
					errorString << ", " << choices[i];
				}
			}
			errorString << ")";
			throw std::runtime_error (errorString.str());
		}
	}
	return position + nextWordLength;
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentChoice::toString() const
{
	std::stringstream result;
	if (isOptional) result << "[";
	result << "{";
	if (!choices.empty())
	{
		result << choices.front();
		for (size_t i = 1; i < choices.size(); ++i)
		{
			result << "/" << choices[i];
		}
	}
	result << "}";
	if (isOptional) result << "]";
	return result.str();
}

//------------------------------------------------------------------------------
const cChatCommandArgumentChoice::ValueType& cChatCommandArgumentChoice::getValue() const
{
	return choices[currentSelection];
}

//------------------------------------------------------------------------------
cChatCommandArgumentString::cChatCommandArgumentString (std::string name, bool isOptional, ValueType defaultValue) :
	isOptional (isOptional),
	name (std::move (name)),
	defaultValue (std::move (defaultValue))
{}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentString::parse (const std::string& command, size_t position)
{
	value = command.substr (position);

	if (value.empty())
	{
		if (isOptional)
		{
			value = defaultValue;
		}
		else
		{
			// TODO: translate
			throw std::runtime_error ("Missing string argument <" + name + ">");
		}
	}

	return command.size();
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentString::toString() const
{
	std::stringstream result;
	if (isOptional) result << "[";
	result << "<" << name << ">";
	if (isOptional) result << "]";
	return result.str();
}

//------------------------------------------------------------------------------
const cChatCommandArgumentString::ValueType& cChatCommandArgumentString::getValue() const
{
	return value;
}

//------------------------------------------------------------------------------
cChatCommandArgumentServer::cChatCommandArgumentServer (cServer*& serverPointer, bool isOptional, ValueType defaultValue) :
	isOptional (isOptional),
	value (defaultValue),
	defaultValue (defaultValue),
	serverPointer (serverPointer)
{}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentServer::parse (const std::string&, size_t position)
{
	value = serverPointer;
	if (value == nullptr)
	{
		if (isOptional)
		{
			value = defaultValue;
		}
		else
		{
			// TODO: translate
			throw std::runtime_error ("Command can only be executed on server");
		}
	}
	return position;
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentServer::toString() const
{
	return "";
}

//------------------------------------------------------------------------------
const cChatCommandArgumentServer::ValueType& cChatCommandArgumentServer::getValue() const
{
	return value;
}

//------------------------------------------------------------------------------
cChatCommandArgumentClient::cChatCommandArgumentClient (const std::shared_ptr<cClient>& activeClientPointer, bool isOptional, ValueType defaultValue) :
	isOptional (isOptional),
	value (defaultValue),
	defaultValue (defaultValue),
	activeClientPointer (activeClientPointer)
{}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentClient::parse (const std::string&, size_t position)
{
	if (activeClientPointer == nullptr)
	{
		if (isOptional)
		{
			value = defaultValue;
		}
		else
		{
			// TODO: translate
			throw std::runtime_error ("Command can not be executed when there is no active client");
		}
	}
	else
	{
		value = activeClientPointer.get();
	}
	return position;
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentClient::toString() const
{
	return "";
}

//------------------------------------------------------------------------------
const cChatCommandArgumentClient::ValueType& cChatCommandArgumentClient::getValue() const
{
	return value;
}

//------------------------------------------------------------------------------
cChatCommandArgumentServerPlayer::cChatCommandArgumentServerPlayer (cServer*& serverPointer, bool isOptional, ValueType defaultValue) :
	isOptional (isOptional),
	value (defaultValue),
	defaultValue (defaultValue),
	serverPointer (serverPointer)
{}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentServerPlayer::parse (const std::string& command, size_t position)
{
	const auto server = serverPointer;
	if (server == nullptr)
	{
		// TODO: translate
		throw std::runtime_error ("Command can only be executed on server");
	}

	const auto nextWordLength = getNextWordLength (command, position);

	std::optional<int> playerNumber;
	try
	{
		size_t pos{};
		playerNumber = std::stoi (command.substr (position, nextWordLength), &pos);
		if (pos != nextWordLength)
		{
			playerNumber.reset();
		}
	}
	catch (const std::invalid_argument&)
	{
		playerNumber.reset();
	}
	catch (const std::out_of_range&)
	{
		// TODO: translate
		throw std::runtime_error ("Invalid player number");
	}
	if (playerNumber)
	{
		try
		{
			value = server->getModel().getPlayer (*playerNumber);
		}
		catch (std::exception&)
		{
			// TODO: translate
			throw std::runtime_error ("Could not find player with number " + std::to_string (*playerNumber));
		}
	}
	else
	{
		const auto& playerName = command.substr (position, nextWordLength);

		value = server->getModel().getPlayer (playerName);

		if (value == nullptr)
		{
			if (nextWordLength == 0 && this->isOptional)
			{
				value = defaultValue;
				return position;
			}
			else
			{
				// TODO: translate
				throw std::runtime_error ("Could not find player with name '" + playerName + "'");
			}
		}
	}

	return position + nextWordLength;
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentServerPlayer::toString() const
{
	std::stringstream result;
	if (isOptional) result << "[";
	result << "<playerID>";
	if (isOptional) result << "]";
	return result.str();
}

//------------------------------------------------------------------------------
const cChatCommandArgumentServerPlayer::ValueType& cChatCommandArgumentServerPlayer::getValue() const
{
	return value;
}

//------------------------------------------------------------------------------
cChatCommandArgumentClientPlayer::cChatCommandArgumentClientPlayer (const std::shared_ptr<cClient>& activeClientPointer, bool isOptional, ValueType defaultValue) :
	isOptional (isOptional),
	value (defaultValue),
	defaultValue (defaultValue),
	activeClientPointer (activeClientPointer)
{}

//------------------------------------------------------------------------------
size_t cChatCommandArgumentClientPlayer::parse (const std::string& command, size_t position)
{
	if (activeClientPointer == nullptr)
	{
		// TODO: translate
		throw std::runtime_error ("Command can not be executed when there is no active client");
	}

	const auto nextWordLength = getNextWordLength (command, position);

	std::optional<int> playerNumber;
	try
	{
		size_t pos{};
		playerNumber = std::stoi (command.substr (position, nextWordLength), &pos);
		if (pos != nextWordLength)
		{
			playerNumber.reset();
		}
	}
	catch (const std::invalid_argument&)
	{
		playerNumber.reset();
	}
	catch (const std::out_of_range&)
	{
		// TODO: translate
		throw std::runtime_error ("Invalid player number");
	}

	if (playerNumber)
	{
		value = activeClientPointer->getModel().getPlayer (*playerNumber);
		if (value == nullptr)
		{
			// TODO: translate
			throw std::runtime_error ("Could not find player with number " + std::to_string (*playerNumber));
		}
	}
	else
	{
		const auto& playerName = command.substr (position, nextWordLength);

		value = activeClientPointer->getModel().getPlayer (playerName);
		if (value == nullptr)
		{
			if (nextWordLength == 0 && this->isOptional)
			{
				value = defaultValue;
				return position;
			}
			else
			{
				// TODO: translate
				throw std::runtime_error ("Could not find player with name '" + playerName + "'");
			}
		}
	}

	return position + nextWordLength;
}

//------------------------------------------------------------------------------
std::string cChatCommandArgumentClientPlayer::toString() const
{
	std::stringstream result;
	if (isOptional) result << "[";
	result << "<playerID>";
	if (isOptional) result << "]";
	return result.str();
}

//------------------------------------------------------------------------------
const cChatCommandArgumentClientPlayer::ValueType& cChatCommandArgumentClientPlayer::getValue() const
{
	return value;
}
