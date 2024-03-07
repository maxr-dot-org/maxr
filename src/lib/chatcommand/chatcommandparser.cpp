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

#include "chatcommandparser.h"

#include <cctype>
#include <string>

//------------------------------------------------------------------------------
void skipWhiteSpace (const std::string& command, size_t& position)
{
	while (position < command.size() && std::isspace (static_cast<unsigned char>(command[position])))
	{
		++position;
	}
}

//------------------------------------------------------------------------------
cChatCommandParser<>::cChatCommandParser (cChatCommand command_) :
	command (std::move (command_))
{}

//------------------------------------------------------------------------------
size_t cChatCommandParser<>::parse (const std::string&, size_t position) const
{
	return position;
}

//------------------------------------------------------------------------------
void cChatCommandParser<>::printArguments (std::ostream&) const
{}

//------------------------------------------------------------------------------
const cChatCommand& cChatCommandParser<>::getCommand() const
{
	return command;
}

//------------------------------------------------------------------------------
cChatCommandParser<>::ArgumentValueTypes cChatCommandParser<>::getArgumentValues() const
{
	return ArgumentValueTypes();
}
