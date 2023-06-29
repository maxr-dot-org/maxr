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

#include "chatcommand.h"

//------------------------------------------------------------------------------
/*static*/ bool cChatCommand::isCommand (const std::string& command)
{
	if (command.empty()) return false;
	if (command[0] != '/') return false;

	return true;
}

//------------------------------------------------------------------------------
cChatCommand::cChatCommand (std::string name, std::function<std::string()> description) :
	name (std::move (name)),
	description (std::move (description))
{}

//------------------------------------------------------------------------------
cChatCommand& cChatCommand::setShouldBeReported (bool value)
{
	shouldBeReported = value;
	return *this;
}

//------------------------------------------------------------------------------
cChatCommand& cChatCommand::setIsServerOnly (bool value)
{
	isServerOnly = value;
	return *this;
}
