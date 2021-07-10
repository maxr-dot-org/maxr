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

#include "ui/graphical/game/control/chatcommand/chatcommand.h"

 //------------------------------------------------------------------------------
/*static*/ bool cChatCommand::isCommand (const std::string& command)
{
	if (command.empty()) return false;
	if (command[0] != '/') return false;

	return true;
}

//------------------------------------------------------------------------------
cChatCommand::cChatCommand (const std::string name_, const std::string description_) :
	name (std::move (name_)),
	description (std::move (description_)),
	shouldBeReported (false),
	isServerOnly (false)
{}

//------------------------------------------------------------------------------
const std::string& cChatCommand::getName() const
{
	return name;
}

//------------------------------------------------------------------------------
const std::string& cChatCommand::getDescription() const
{
	return description;
}

//------------------------------------------------------------------------------
cChatCommand& cChatCommand::setShouldBeReported (bool value)
{
	shouldBeReported = value;
	return *this;
}

//------------------------------------------------------------------------------
bool cChatCommand::getShouldBeReported() const
{
	return shouldBeReported;
}

//------------------------------------------------------------------------------
cChatCommand& cChatCommand::setIsServerOnly (bool value)
{
	isServerOnly = value;
	return *this;
}

//------------------------------------------------------------------------------
bool cChatCommand::getIsServerOnly() const
{
	return isServerOnly;
}
