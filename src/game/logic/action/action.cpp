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

#include "action.h"
#include "utility/log.h"
#include "utility/string/toString.h"
#include "main.h"
#include "actioninitnewgame.h"

std::unique_ptr<cAction> cAction::createFromBuffer(cBinaryArchiveOut& archive)
{
	eActiontype type;
	archive >> type;

	switch (type)
	{
	case eActiontype::ACTION_INIT_NEW_GAME:
		return std::make_unique<cActionInitNewGame>(archive);
	default:
		//TODO: to throw or not to throw...
		Log.write("Unknown action type " + iToStr(static_cast<int>(type)), cLog::eLOG_TYPE_NET_ERROR);
		return nullptr;
	}
}

//------------------------------------------------------------------------------
std::string enumToString(cAction::eActiontype value)
{
	switch (value)
	{
	case cAction::eActiontype::ACTION_INIT_NEW_GAME:
		return "ACTION_INIT_NEW_GAME";
		break;
	default:
		assert(false);
		return toString(static_cast<int>(value));
		break;
	}
}
