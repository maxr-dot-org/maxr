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
		{ePlayerConnectionState::Inactive, "Inactive"},
		{ePlayerConnectionState::Connected, "Connected"},
		{ePlayerConnectionState::NotResponding, "Not responding"},
		{ePlayerConnectionState::Disconnected, "Disconnected"}
	};

	//--------------------------------------------------------------------------
	const std::vector<std::pair<eFreezeMode, const char*>>
	sEnumStringMapping<eFreezeMode>::m =
	{
		{eFreezeMode::WaitForTurnend, "WAIT_FOR_TURNEND"},
		{eFreezeMode::Pause, "PAUSE"},
		{eFreezeMode::WaitForClient, "WAIT_FOR_CLIENT"},
		{eFreezeMode::WaitForServer, "WAIT_FOR_SERVER"}
	};
} // namespace serialization
//------------------------------------------------------------------------------
void cFreezeModes::enable (eFreezeMode mode)
{
	switch (mode)
	{
		case eFreezeMode::WaitForTurnend:
			waitForTurnEnd = true;
			return;
		case eFreezeMode::Pause:
			pause = true;
			return;
		case eFreezeMode::WaitForClient:
			waitForClient = true;
			return;
		case eFreezeMode::WaitForServer:
			waitForServer = true;
			return;
	}
	throw std::runtime_error ("unreachable");
}

//------------------------------------------------------------------------------
void cFreezeModes::disable (eFreezeMode mode)
{
	switch (mode)
	{
		case eFreezeMode::WaitForTurnend:
			waitForTurnEnd = false;
			return;
		case eFreezeMode::Pause:
			pause = false;
			return;
		case eFreezeMode::WaitForClient:
			waitForClient = false;
			return;
		case eFreezeMode::WaitForServer:
			waitForServer = false;
			return;
	}
	throw std::runtime_error ("unreachable");
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
		case eFreezeMode::Pause: return pause;
		case eFreezeMode::WaitForTurnend: return waitForTurnEnd;
		case eFreezeMode::WaitForClient: return waitForClient;
		case eFreezeMode::WaitForServer: return waitForServer;
	}
	throw std::runtime_error ("unreachable");
}
