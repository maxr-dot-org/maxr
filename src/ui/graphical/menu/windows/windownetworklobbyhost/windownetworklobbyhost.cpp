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

#include "ui/graphical/menu/windows/windownetworklobbyhost/windownetworklobbyhost.h"

#include "ui/graphical/menu/widgets/pushbutton.h"
#include "game/data/player/player.h"
#include "game/data/savegame.h"
#include "game/data/map/map.h"
#include "mapdownloader/mapdownload.h"
#include "utility/language.h"
#include "../../../application.h"
#include "../../dialogs/dialogok.h"

//------------------------------------------------------------------------------
cWindowNetworkLobbyHost::cWindowNetworkLobbyHost() :
	cWindowNetworkLobby (lngPack.i18n ("Text~Others~TCPIP_Host"), true)
{
	auto mapButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 42), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Choose_Planet")));
	signalConnectionManager.connect (mapButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleMapClicked, this));

	auto settingsButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 77), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Options")));
	signalConnectionManager.connect (settingsButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleSettingsClicked, this));

	auto loadButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 120), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Game_Load")));
	signalConnectionManager.connect (loadButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleLoadClicked, this));

	auto startButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 200), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Host_Start")));
	signalConnectionManager.connect (startButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleStartClicked, this));

	auto okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 450), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleOkClicked, this));

	signalConnectionManager.connect (staticMapChanged, [this]() {setSaveGame (cSaveGameInfo(-1), nullptr); });
	signalConnectionManager.connect(gameSettingsChanged, [this]() {setSaveGame(cSaveGameInfo(-1), nullptr); });
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleMapClicked()
{
	triggeredSelectMap();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleSettingsClicked()
{
	triggeredSelectSettings();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleLoadClicked()
{
	triggeredSelectSaveGame();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleStartClicked()
{
	triggeredStartHost();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleOkClicked()
{
	triggeredStartGame();
}

//------------------------------------------------------------------------------
bool cWindowNetworkLobbyHost::setSaveGame(const cSaveGameInfo& saveGameInfo_, cApplication* application)
{

	if (saveGameInfo_.number >= 0)
	{
		staticMap = std::make_shared<cStaticMap>();
		if (!staticMap->loadMap(saveGameInfo_.mapName))
		{
			staticMap = nullptr;
			application->show(std::make_shared<cDialogOk>("Map \"" + saveGameInfo_.mapName + "\" not found"));
			return false;
		}
		else if (MapDownload::calculateCheckSum(saveGameInfo_.mapName) != saveGameInfo_.mapCrc)
		{
			staticMap = nullptr;
			application->show(std::make_shared<cDialogOk>("The map \"" + saveGameInfo_.mapName + "\" does not match the map the game was started with")); // TODO: translate
			return false;
		}
	}

	saveGameInfo = saveGameInfo_;

	updateMap();
	updateSettingsText();
	saveGameChanged();

	return true;
}
