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

#ifndef ui_graphical_game_control_chatcommand_chatcommandexecutorH
#define ui_graphical_game_control_chatcommand_chatcommandexecutorH

#include <string>
#include <sstream>
#include <exception>

class cChatCommand;

void skipWhiteSpace(const std::string& command, size_t& position);

class cChatCommandExecutor
{
public:
	virtual ~cChatCommandExecutor() = default;
	virtual bool tryExecute(const std::string& command) const = 0;
	virtual void printArguments(std::ostream& result) const = 0;
	virtual const cChatCommand& getCommand() const = 0;
};

template<typename F, typename... Arguments>
class cChatCommandExecutorImpl : public cChatCommandExecutor
{
public:
	cChatCommandExecutorImpl(F function_, cChatCommandParser<Arguments...> parser_);

	virtual bool tryExecute(const std::string& command) const override;
	virtual void printArguments(std::ostream& result) const override;
	virtual const cChatCommand& getCommand() const override;
private:
	F function;
	cChatCommandParser<Arguments...> argumentParser;
};


#include "utility/invoke.h"

template<typename F, typename... Arguments>
cChatCommandExecutorImpl<F, Arguments...>::cChatCommandExecutorImpl(F function_, cChatCommandParser<Arguments...> parser_) :
	function(std::move(function_)),
	argumentParser(std::move(parser_))
{}

template<typename F, typename... Arguments>
bool cChatCommandExecutorImpl<F, Arguments...>::tryExecute(const std::string& command) const
{
	if(!cChatCommand::isCommand(command)) return false;

	const auto& commandName = argumentParser.getCommand().getName();

	if(command.compare(1, commandName.size(), commandName) == 0)
	{
		auto position = 1 + commandName.size();
		skipWhiteSpace(command, position);

		// Name has either be to be followed by at least one white space or it has to be the end of the command string
		if(position == 1 + commandName.size() && position != command.size())
		{
			return false;
		}

		position = argumentParser.parse(command, position);
		skipWhiteSpace(command, position);

		if(position != command.size())
		{
			std::stringstream errorString;
			// TODO: translate
			errorString << "Command is followed by non-empty string '" << command.substr(position) << "'";
			throw std::runtime_error(errorString.str());
		}

		invoke(function, argumentParser.getArgumentValues());

		return true;
	}

	return false;
}

template<typename F, typename... Arguments>
void cChatCommandExecutorImpl<F, Arguments...>::printArguments(std::ostream& result) const
{
	argumentParser.printArguments(result);
}

template<typename F, typename... Arguments>
const cChatCommand& cChatCommandExecutorImpl<F, Arguments...>::getCommand() const
{
	return argumentParser.getCommand();
}

#endif // ui_graphical_game_control_chatcommand_chatcommandexecutorH
