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

// Outside of header guards due to circular dependencies
#include "chatcommand/chatcommand.h"

#ifndef chatcommand_chatcommandparserH
# define chatcommand_chatcommandparserH

# include <sstream>
# include <string>
# include <tuple>

void skipWhiteSpace (std::string_view command, size_t& position);

template <typename... Arguments>
class cChatCommandParser;

template <>
class cChatCommandParser<>
{
public:
	using ArgumentValueTypes = std::tuple<>;

	cChatCommandParser (cChatCommand command_);

	size_t parse (std::string_view command, size_t position) const;
	void printArguments (std::ostream& result) const;

	const cChatCommand& getCommand() const;

	ArgumentValueTypes getArgumentValues() const;

private:
	cChatCommand command;
};

template <typename Argument, typename... LastArguments>
class cChatCommandParser<Argument, LastArguments...>
{
public:
	using ArgumentValueTypes = std::tuple<typename LastArguments::ValueTypes..., typename Argument::ValueType>;

	cChatCommandParser (cChatCommandParser<LastArguments...> lastParser_, Argument argument_);
	size_t parse (std::string_view command, size_t position) const;

	template <typename NewArgument, typename... Args>
	cChatCommandParser<NewArgument, Argument, LastArguments...> addArgument (Args&&... args) &&;
	template <typename F>
	std::unique_ptr<cChatCommandExecutor> setAction (F function) &&;

	void printArguments (std::ostream& result) const;

	const cChatCommand& getCommand() const;
	ArgumentValueTypes getArgumentValues() const;

private:
	cChatCommandParser<LastArguments...> lastParser;
	mutable Argument argument;
};

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
cChatCommandParser<Argument, LastArguments...>::cChatCommandParser (cChatCommandParser<LastArguments...> lastParser_, Argument argument_) :
	lastParser (std::move (lastParser_)),
	argument (std::move (argument_))
{}

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
size_t cChatCommandParser<Argument, LastArguments...>::parse (std::string_view command, size_t position) const
{
	position = lastParser.parse (command, position);
	try
	{
		position = argument.parse (command, position);
	}
	catch (const std::runtime_error& e)
	{
		std::stringstream errorString;
		// TODO: translate
		errorString << std::string ("Invalid value for argument: ") << e.what();
		throw std::runtime_error (errorString.str());
	}
	skipWhiteSpace (command, position);

	return position;
}

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
template <typename NewArgument, typename... Args>
cChatCommandParser<NewArgument, Argument, LastArguments...> cChatCommandParser<Argument, LastArguments...>::addArgument (Args&&... args) &&
{
	return cChatCommandParser<NewArgument, Argument, LastArguments...> (std::move (*this), NewArgument (std::forward<Args> (args)...));
}

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
template <typename F>
std::unique_ptr<cChatCommandExecutor> cChatCommandParser<Argument, LastArguments...>::setAction (F function) &&
{
	return std::make_unique<cChatCommandExecutorImpl<F, Argument, LastArguments...>> (std::move (function), std::move (*this));
}

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
void cChatCommandParser<Argument, LastArguments...>::printArguments (std::ostream& result) const
{
	lastParser.printArguments (result);
	const auto argumentString = argument.toString();
	if (!argumentString.empty())
	{
		result << " " << argumentString;
	}
}

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
const cChatCommand& cChatCommandParser<Argument, LastArguments...>::getCommand() const
{
	return lastParser.getCommand();
}

//------------------------------------------------------------------------------
template <typename Argument, typename... LastArguments>
typename cChatCommandParser<Argument, LastArguments...>::ArgumentValueTypes cChatCommandParser<Argument, LastArguments...>::getArgumentValues() const
{
	return std::tuple_cat (lastParser.getArgumentValues(), std::tuple<typename Argument::ValueType> (argument.getValue()));
}

#endif
