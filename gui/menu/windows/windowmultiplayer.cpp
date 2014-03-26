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

#include <functional>

#include "windowmultiplayer.h"
#include "windowgamesettings/gamesettings.h"
#include "windowgamesettings/windowgamesettings.h"
#include "windowclanselection/windowclanselection.h"
#include "windowlandingunitselection/windowlandingunitselection.h"
#include "windowlandingpositionselection/windowlandingpositionselection.h"
#include "windownetworklobbyhost/windownetworklobbyhost.h"
#include "windownetworklobbyclient/windownetworklobbyclient.h"
#include "../widgets/pushbutton.h"
#include "../../application.h"
#include "../../../main.h"
#include "../../../log.h"
#include "../../../player.h"
#include "../../../network.h"
#include "../../../menus.h"
#include "../../../game/network/host/networkhostgamenew.h"
#include "../../../game/network/host/networkhostgamesaved.h"
#include "../../../game/logic/landingpositionmanager.h"

//------------------------------------------------------------------------------
cWindowMultiPlayer::cWindowMultiPlayer () :
	cWindowMain (lngPack.i18n ("Text~Others~Multi_Player"))
{
	using namespace std::placeholders;

	auto hastButton = addChild (std::make_unique<cPushButton> (getPosition () +cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~TCPIP_Host")));
	signalConnectionManager.connect (hastButton->clicked, std::bind (&cWindowMultiPlayer::tcpHostClicked, this));

	auto clientButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~TCPIP_Client")));
	signalConnectionManager.connect (clientButton->clicked, std::bind (&cWindowMultiPlayer::tcpClientClicked, this));

#ifndef RELEASE
	auto newHotSeatButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace * 2), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~HotSeat_New")));
	signalConnectionManager.connect (newHotSeatButton->clicked, std::bind (&cWindowMultiPlayer::newHotSeatClicked, this));

	auto loadHotSeatButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace * 3), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~HotSeat_Load")));
	signalConnectionManager.connect (loadHotSeatButton->clicked, std::bind (&cWindowMultiPlayer::loadHotSeatClicked, this));
#endif

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowMultiPlayer::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowMultiPlayer::~cWindowMultiPlayer ()
{}

// FIXME: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (int clan, const cGameSettings& gameSettings);

//------------------------------------------------------------------------------
void cWindowMultiPlayer::tcpHostClicked ()
{
	auto application = getActiveApplication ();

	if (!application) return;

	auto windowNetworkLobby = getActiveApplication ()->show (std::make_shared<cWindowNetworkLobbyHost> ());
	windowNetworkLobby->start.connect ([windowNetworkLobby, application]()
	{
		if ((!windowNetworkLobby->getGameSettings () || !windowNetworkLobby->getStaticMap ()) && windowNetworkLobby->getSaveGameNumber () == -1)
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Missing_Settings"));
			return;
		}

		auto players = windowNetworkLobby->getPlayers ();
		auto notReadyPlayerIter = std::find_if (players.begin (), players.end (), [ ](const std::shared_ptr<sPlayer>& player) { return !player->isReady (); });

		if (notReadyPlayerIter != players.end ())
		{
			windowNetworkLobby->addInfoEntry ((*notReadyPlayerIter)->getName () + " " + lngPack.i18n ("Text~Multiplayer~Not_Ready"));
			return;
		}

		if (windowNetworkLobby->getNetwork ().getConnectionStatus () == 0)
		{
			windowNetworkLobby->addInfoEntry (lngPack.i18n ("Text~Multiplayer~Server_Not_Running"));
			return;
		}

		if (windowNetworkLobby->getSaveGameNumber () != -1)
		{
			//auto game = std::make_shared<cNetworkHostGameSaved> ();
			// run save game
			windowNetworkLobby->close ();
		}
		else
		{
			auto gameSettings = windowNetworkLobby->getGameSettings ();
			auto staticMap = windowNetworkLobby->getStaticMap ();

			auto players = windowNetworkLobby->getPlayers ();
			auto localPlayer = windowNetworkLobby->getLocalPlayer ();

			auto game = std::make_shared<cNetworkHostGameNew> ();
			game->setPlayers (players, *localPlayer);
			game->setGameSettings (gameSettings);
			game->setStaticMap (staticMap);

			if (gameSettings->getClansEnabled ())
			{
				auto windowClanSelection = application->show (std::make_shared<cWindowClanSelection> ());

				windowClanSelection->done.connect ([=]()
				{
					game->setLocalPlayerClan (windowClanSelection->getSelectedClan ());

					auto initialLandingUnits = createInitialLandingUnitsList (windowClanSelection->getSelectedClan (), *gameSettings);

					auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (0, windowClanSelection->getSelectedClan (), initialLandingUnits, gameSettings->getStartCredits ()));

					windowLandingUnitSelection->done.connect ([=]()
					{
						game->setLocalPlayerLandingUnits (windowLandingUnitSelection->getLandingUnits ());
						game->setLocalPlayerUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());

						auto landingPositionManager = std::make_shared<cLandingPositionManager>(players);

						auto windowLandingPositionSelection = application->show (std::make_shared<cWindowLandingPositionSelection> (staticMap));

						windowLandingPositionSelection->selectedPosition.connect ([=](cPosition landingPosition)
						{
							landingPositionManager->setLandingPosition (*localPlayer, landingPosition);
						});

						landingPositionManager->allPositionsValid.connect ([=]()
						{
							game->setLocalPlayerLandingPosition (windowLandingPositionSelection->getSelectedPosition());

							game->start (*application);

							windowLandingPositionSelection->close ();
							windowLandingUnitSelection->close ();
							windowClanSelection->close ();
							windowNetworkLobby->close ();
						});
					});
				});
			}
			else
			{
				auto initialLandingUnits = createInitialLandingUnitsList (-1, *gameSettings);

				auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (0, -1, initialLandingUnits, gameSettings->getStartCredits ()));

				windowLandingUnitSelection->done.connect ([=]()
				{
					game->setLocalPlayerLandingUnits (windowLandingUnitSelection->getLandingUnits ());
					game->setLocalPlayerUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());

					auto landingPositionManager = std::make_shared<cLandingPositionManager> (players);

					auto windowLandingPositionSelection = application->show (std::make_shared<cWindowLandingPositionSelection> (staticMap));

					windowLandingPositionSelection->selectedPosition.connect ([=](cPosition landingPosition)
					{
						landingPositionManager->setLandingPosition (*localPlayer, landingPosition);
					});

					landingPositionManager->allPositionsValid.connect ([=]()
					{
						game->setLocalPlayerLandingPosition (windowLandingPositionSelection->getSelectedPosition ());

						game->start (*application);

						windowLandingPositionSelection->close ();
						windowLandingUnitSelection->close ();
						windowNetworkLobby->close ();
					});
				});
			}
		}
	});
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::tcpClientClicked ()
{
	auto application = getActiveApplication ();

	if (!application) return;

	auto windowNetworkLobby = getActiveApplication ()->show (std::make_shared<cWindowNetworkLobbyClient> ());
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::newHotSeatClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::loadHotSeatClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::backClicked ()
{
	close ();
}
