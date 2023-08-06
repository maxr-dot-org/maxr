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

#ifndef ui_graphical_menu_control_local_singleplayer_localsingleplayergamenewH
#define ui_graphical_menu_control_local_singleplayer_localsingleplayergamenewH

#include "game/connectionmanager.h"
#include "game/startup/initplayerdata.h"
#include "game/startup/lobbyclient.h"
#include "game/startup/lobbyserver.h"
#include "ui/graphical/menu/control/initgamepreparation.h"
#include "ui/graphical/menu/control/local/singleplayer/localsingleplayergame.h"
#include "utility/signal/signalconnectionmanager.h"

#include <filesystem>
#include <memory>

class cApplication;
class cStaticMap;
class cGameSettings;
class cPlayerBasicData;

class cLocalSingleplayerGameNew : public cLocalSingleplayerGame
{
public:
	cLocalSingleplayerGameNew();

	void run() override;

	void runGamePreparation (cApplication&);
	void start (cApplication&, cServer&);

	void setGameSettings (const cGameSettings&);
	void selectMapFilename (const std::filesystem::path&);

	static cPlayerBasicData createPlayer();

private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cConnectionManager> connectionManager = std::make_shared<cConnectionManager>();
	cLobbyServer lobbyServer;
	cLobbyClient lobbyClient;
	std::shared_ptr<cInitGamePreparation> initGamePreparation;
};

#endif
