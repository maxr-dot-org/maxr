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

#include "game/startup/local/singleplayer/localsingleplayergame.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/logic/savegame.h"
#include "loaddata.h"

//------------------------------------------------------------------------------
cLocalSingleplayerGame::~cLocalSingleplayerGame ()
{
	if (server)
	{
		server->stop ();
		reloadUnitValues ();
	}
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGame::run ()
{
	if (client) client->getGameTimer ()->run ();
}

//------------------------------------------------------------------------------
void cLocalSingleplayerGame::save (int saveNumber, const std::string& saveName)
{
	if (!server) throw std::runtime_error ("Game not started!"); // should never happen (hence a translation is not necessary).

	cSavegame savegame (saveNumber);
	savegame.save (*server, saveName);
	server->makeAdditionalSaveRequest (saveNumber);
}
