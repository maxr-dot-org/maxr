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

#include "freezemode.h"

#include <cassert>

namespace serialization
{
	//--------------------------------------------------------------------------
	const std::vector<std::pair<ePlayerConnectionState, const char*>>
	sEnumStringMapping<ePlayerConnectionState>::m =
	{
		{ePlayerConnectionState::INACTIVE, "Inactive"},
		{ePlayerConnectionState::CONNECTED, "Connected"},
		{ePlayerConnectionState::NOT_RESPONDING, "Not responding"},
		{ePlayerConnectionState::DISCONNECTED, "Disconnected"}
	};

	//--------------------------------------------------------------------------
	const std::vector<std::pair<eFreezeMode, const char*>>
	sEnumStringMapping<eFreezeMode>::m =
	{
		{eFreezeMode::WAIT_FOR_TURNEND, "WAIT_FOR_TURNEND"},
		{eFreezeMode::PAUSE, "PAUSE"},
		{eFreezeMode::WAIT_FOR_CLIENT, "WAIT_FOR_CLIENT"},
		{eFreezeMode::WAIT_FOR_SERVER, "WAIT_FOR_SERVER"}
	};
}
//------------------------------------------------------------------------------
void cFreezeModes::enable (eFreezeMode mode)
{
	switch (mode)
	{
	case eFreezeMode::WAIT_FOR_TURNEND:
		waitForTurnEnd = true;
		return;
	case eFreezeMode::PAUSE:
		pause = true;
		return;
	case eFreezeMode::WAIT_FOR_CLIENT:
		waitForClient = true;
		return;
	case eFreezeMode::WAIT_FOR_SERVER:
		waitForServer = true;
		return;
	}
	assert (false);
}

//------------------------------------------------------------------------------
void cFreezeModes::disable (eFreezeMode mode)
{
	switch (mode)
	{
	case eFreezeMode::WAIT_FOR_TURNEND:
		waitForTurnEnd = false;
		return;
	case eFreezeMode::PAUSE:
		pause = false;
		return;
	case eFreezeMode::WAIT_FOR_CLIENT:
		waitForClient = false;
		return;
	case eFreezeMode::WAIT_FOR_SERVER:
		waitForServer = false;
		return;
	}
	assert (false);
}

//------------------------------------------------------------------------------
bool cFreezeModes::isFreezed() const
{
	return pause | waitForTurnEnd | waitForClient | waitForServer;
}

//------------------------------------------------------------------------------
bool cFreezeModes::gameTimePaused() const
{
	return waitForClient | pause;
}

//------------------------------------------------------------------------------
bool cFreezeModes::isEnabled (eFreezeMode mode) const
{
	switch (mode)
	{
		case eFreezeMode::PAUSE: return pause;
		case eFreezeMode::WAIT_FOR_TURNEND: return waitForTurnEnd;
		case eFreezeMode::WAIT_FOR_CLIENT: return waitForClient;
		case eFreezeMode::WAIT_FOR_SERVER: return waitForServer;
	}
	assert (0); // Incorrect parameter
	return false;
}
