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

#include "game/data/report/savedreportchat.h"

#include "game/data/player/player.h"

//------------------------------------------------------------------------------
cSavedReportChat::cSavedReportChat (const cPlayer& player, std::string text_) :
	playerName (player.getName()),
	playerNumber (player.getId()),
	text (std::move (text_))
{}

//------------------------------------------------------------------------------
cSavedReportChat::cSavedReportChat (std::string playerName_, std::string text_) :
	playerName (std::move (playerName_)),
	text (std::move (text_))
{}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportChat::getType() const
{
	return eSavedReportType::Chat;
}

//------------------------------------------------------------------------------
bool cSavedReportChat::isAlert() const
{
	return false;
}

//------------------------------------------------------------------------------
int cSavedReportChat::getPlayerNumber() const
{
	return playerNumber;
}

//------------------------------------------------------------------------------
const std::string& cSavedReportChat::getText() const
{
	return text;
}

//------------------------------------------------------------------------------
const std::string& cSavedReportChat::getPlayerName() const
{
	return playerName;
}
