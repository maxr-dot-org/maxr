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

#include <memory>

#include "main.h"
#include "netmessage2.h"

std::unique_ptr<cNetMessage2> cNetMessage2::createFromBuffer(cBinaryArchiveOut& archive)
{
	eNetMessageType type;
	archive >> type;
	std::unique_ptr<cNetMessage2> message;
	switch (type)
	{
	//case cNetMessage2::ACTION:
	//	break;
	//case cNetMessage2::SYNC:
	//	break;
	//case cNetMessage2::PLAYERSTATE:
	//	break;
	case cNetMessage2::CHAT:
		message = std::make_unique<cNetMessageChat>();
		break;
	default:
		throw std::runtime_error("Unknown action type " + iToStr(type));
		break;
	}

	archive.rewind();
	archive >> *message;

	return std::move(message);
}
