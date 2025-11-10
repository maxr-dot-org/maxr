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

#ifndef chatcommand_chatcommandexecutorH
#define chatcommand_chatcommandexecutorH

#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>

class cChatCommand;

void skipWhiteSpace (std::string_view command, size_t& position);

class cChatCommandExecutor
{
public:
	virtual ~cChatCommandExecutor() = default;
	virtual bool tryExecute (std::string_view command) const = 0;
	virtual void printArguments (std::ostream& result) const = 0;
	virtual const cChatCommand& getCommand() const = 0;
};

template <typename F, typename... Arguments>
class cChatCommandExecutorImpl : public cChatCommandExecutor
{
public:
	cChatCommandExecutorImpl (F function_, cChatCommandParser<Arguments...> parser_);

	bool tryExecute (std::string_view command) const override;
	void printArguments (std::ostream& result) const override;
	const cChatCommand& getCommand() const override;

private:
	F function;
	cChatCommandParser<Arguments...> argumentParser;
};

template <typename F, typename... Arguments>
cChatCommandExecutorImpl<F, Arguments...>::cChatCommandExecutorImpl (F function_, cChatCommandParser<Arguments...> parser_) :
	function (std::move (function_)),
	argumentParser (std::move (parser_))
{}

template <typename F, typename... Arguments>
bool cChatCommandExecutorImpl<F, Arguments...>::tryExecute (std::string_view command) const
{
	if (!cChatCommand::isCommand (command)) return false;

	const auto& commandName = argumentParser.getCommand().getName();

	if (command.compare (1, commandName.size(), commandName) == 0)
	{
		auto position = 1 + commandName.size();
		skipWhiteSpace (command, position);

		// Name has either be to be followed by at least one white space or it has to be the end of the command string
		if (position == 1 + commandName.size() && position != command.size())
		{
			return false;
		}

		position = argumentParser.parse (command, position);
		skipWhiteSpace (command, position);

		if (position != command.size())
		{
			std::stringstream errorString;
			// TODO: translate
			errorString << "Command is followed by non-empty string '" << command.substr (position) << "'";
			throw std::runtime_error (errorString.str());
		}
		std::apply (function, argumentParser.getArgumentValues());
		return true;
	}
	return false;
}

template <typename F, typename... Arguments>
void cChatCommandExecutorImpl<F, Arguments...>::printArguments (std::ostream& result) const
{
	argumentParser.printArguments (result);
}

template <typename F, typename... Arguments>
const cChatCommand& cChatCommandExecutorImpl<F, Arguments...>::getCommand() const
{
	return argumentParser.getCommand();
}

#endif
