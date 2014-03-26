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

#include "windownetworklobbyhost.h"
#include "../windowgamesettings/gamesettings.h"
#include "../windowgamesettings/windowgamesettings.h"
#include "../windowmapselection/windowmapselection.h"
#include "../windowload/windowload.h"
#include "../../dialogs/dialogok.h"
#include "../../widgets/pushbutton.h"
#include "../../../application.h"
#include "../../../../main.h"
#include "../../../../map.h"
#include "../../../../network.h"
#include "../../../../log.h"

//------------------------------------------------------------------------------
cWindowNetworkLobbyHost::cWindowNetworkLobbyHost () :
	cWindowNetworkLobby (lngPack.i18n ("Text~Others~TCPIP_Host"), true)
{
	auto mapButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 42), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Choose_Planet")));
	signalConnectionManager.connect (mapButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleMapClicked, this));
	
	auto settingsButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 77), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Options")));
	signalConnectionManager.connect (settingsButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleSettingsClicked, this));

	auto loadButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 120), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Game_Load")));
	signalConnectionManager.connect (loadButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleLoadClicked, this));

	auto startButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 200), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Host_Start")));
	signalConnectionManager.connect (startButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleStartClicked, this));

	auto okButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 450), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowNetworkLobbyHost::handleOkClicked, this));
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleMapClicked ()
{
	if (!getActiveApplication ()) return;

	auto application = getActiveApplication ();

	auto windowMapSelection = application->show (std::make_shared<cWindowMapSelection> ());
	windowMapSelection->done.connect ([&, application, windowMapSelection]()
	{
		auto staticMap = std::make_shared<cStaticMap>();
		if (windowMapSelection->loadSelectedMap (*staticMap))
		{
			setStaticMap (std::move (staticMap));
			windowMapSelection->close ();
		}
		else
		{
			application->show (std::make_shared<cDialogOk> ("Error while loading map!")); // TODO: translate
		}
	});
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleSettingsClicked ()
{
	if (!getActiveApplication ()) return;

	auto application = getActiveApplication ();

	auto windowGameSettings = application->show (std::make_shared<cWindowGameSettings> ());

	if (getGameSettings ())	windowGameSettings->applySettings (*getGameSettings ());
	else windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->done.connect ([&, windowGameSettings]()
	{
		setGameSettings (std::make_unique<cGameSettings> (windowGameSettings->getGameSettings ()));
		windowGameSettings->close ();
	});
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleLoadClicked ()
{
	if (!getActiveApplication ()) return;

	auto application = getActiveApplication ();

	auto windowLoad = application->show (std::make_shared<cWindowLoad> ());
	windowLoad->load.connect ([&, windowLoad](int saveGameNumber)
	{
		setSaveGame (saveGameNumber);
		windowLoad->close ();
	});
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleStartClicked ()
{
	if (getNetwork ().getConnectionStatus () != 0) return;

	if (getNetwork ().create (getPort ()))
	{
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Socket"));
		Log.write ("Error opening socket", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n ("Text~Title~Port") + ": "  + iToStr (getPort ()) + ")");
		Log.write ("Game open (Port: " + iToStr (getPort ()) + ")", cLog::eLOG_TYPE_INFO);
		disablePortEdit ();
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobbyHost::handleOkClicked ()
{
	start ();
}