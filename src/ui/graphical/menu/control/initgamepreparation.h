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

#ifndef ui_graphical_menu_control_initgamepreparationH
#define ui_graphical_menu_control_initgamepreparationH

#include <memory>
#include <utility>
#include <vector>

#include "game/data/units/landingunit.h"
#include "game/data/units/unitdata.h"
#include "game/logic/upgradecalculator.h"
#include "game/startup/initplayerdata.h"
#include "game/startup/lobbypreparationdata.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "utility/signal/signalconnectionmanager.h"

class cApplication;
class cLobbyClient;
class cWindow;
class cWindowLandingPositionSelection;

class cInitGamePreparation
{
public:
	cInitGamePreparation (cApplication& application, cLobbyClient& lobbyClient);

	void bindConnections (cLobbyClient&);

	void onChatMessage (const std::string& playerName, const std::string& message);

	void startGamePreparation (const sLobbyPreparationData&);

	const sInitPlayerData& getInitPlayerData() const { return initPlayerData; }

private:
	void startClanSelection (bool isFirstWindowOnGamePreparation);
	void startLandingUnitSelection (bool isFirstWindowOnGamePreparation);
	void startLandingPositionSelection();
	void back();
	void checkReallyWantsToQuit();

private:
	sLobbyPreparationData lobbyPreparationData;
	cApplication& application;

	cLobbyClient& lobbyClient;
	std::vector<cWindow*> windows;
	std::shared_ptr<cWindowLandingPositionSelection> windowLandingPositionSelection;
	std::vector<std::unique_ptr<cPlayerLandingStatus>> playersLandingStatus;

	cSignalConnectionManager signalConnectionManager;

	sInitPlayerData initPlayerData;
};

#endif
