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

#include "ui/graphical/control/menucontrollermultiplayerhotseat.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windowplayerselection/windowplayerselection.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "game/local/hotseat/localhotseatgamenew.h"
#include "game/data/player/playerbasicdata.h"
#include "map.h"
#include "menus.h"
#include "upgradecalculator.h"

// TODO: remove
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (int clan, const cGameSettings& gameSettings); // defined in windowsingleplayer.cpp

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHotSeat::cMenuControllerMultiplayerHotSeat (cApplication& application_) :
	application (application_)
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::start ()
{
	game = std::make_shared<cLocalHotSeatGameNew> ();

	selectGameSettings ();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::reset ()
{
	game = nullptr;
	windowPlayerSelection = nullptr;
	firstWindow = nullptr;
	landingPositionManager = nullptr;
	nextInvalidLandingPositionPlayers.clear ();
	invalidLandingPositionPlayers.clear ();
	playerLandingSelectionWindows.clear ();
	landingSelectionWindowConnections.disconnectAll ();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectGameSettings ()
{
	if (!game) return;

	auto windowGameSettings = application.show (std::make_shared<cWindowGameSettings> (true));
	windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->terminated.connect (std::bind (&cMenuControllerMultiplayerHotSeat::reset, this));

	firstWindow = windowGameSettings;

	windowGameSettings->done.connect ([=]()
	{
		auto gameSettings = std::make_shared<cGameSettings> (windowGameSettings->getGameSettings ());
		game->setGameSettings (gameSettings);

		selectMap ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectMap ()
{
	if (!game) return;

	auto windowMapSelection = application.show (std::make_shared<cWindowMapSelection> ());

	windowMapSelection->done.connect ([=]()
	{
		auto staticMap = std::make_shared<cStaticMap> ();
		if (!windowMapSelection->loadSelectedMap (*staticMap))
		{
			// TODO: error dialog: could not load selected map!
			return;
		}
		game->setStaticMap (staticMap);

		selectPlayers ();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectPlayers ()
{
	if (!game) return;

	windowPlayerSelection = application.show (std::make_shared<cWindowPlayerSelection> ());

	windowPlayerSelection->done.connect ([=]()
	{
		const auto& playerTypes = windowPlayerSelection->getPlayerTypes ();

		const char* const playerNames[] =
		{
			"Text~Multiplayer~Player1",
			"Text~Multiplayer~Player2",
			"Text~Multiplayer~Player3",
			"Text~Multiplayer~Player4",
			"Text~Multiplayer~Player5",
			"Text~Multiplayer~Player6",
			"Text~Multiplayer~Player7",
			"Text~Multiplayer~Player8"
		};

		std::vector<cPlayerBasicData> players;
		int playerNum = 0;
		for (size_t i = 0; i < playerTypes.size (); ++i)
		{
			if (playerTypes[i] == ePlayerType::Human)
			{
				cPlayerBasicData player (lngPack.i18n (playerNames[i]), cPlayerColor (i), playerNum++);
				players.push_back (player);
			}
		}
		assert (players.size () > 0);

		game->setPlayers (players);

		playerLandingSelectionWindows.resize (players.size ());
		landingPositionManager = std::make_unique<cLandingPositionManager> (players);
		landingPositionManager->landingPositionStateChanged.connect ([this](const cPlayerBasicData& player, eLandingPositionState state)
		{
			if (state == eLandingPositionState::TooClose || state == eLandingPositionState::Warning)
			{
				for (size_t i = 0; i < game->getPlayerCount (); ++i)
				{
					if (game->getPlayer (i).getNr () == player.getNr ())
					{
						nextInvalidLandingPositionPlayers.push_back (std::make_pair(i, state));
						break;
					}
				}
			}
		});

		startNextPlayerGamePreperation (0);
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::startNextPlayerGamePreperation (size_t playerIndex)
{
	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Multiplayer~Player_Turn", game->getPlayer (playerIndex).getName ()), eWindowBackgrounds::Black));
	dialog->done.connect ([this, playerIndex]()
	{
		if (game->getGameSettings ()->getClansEnabled ())
		{
			selectClan (playerIndex, true);
		}
		else
		{
			selectLandingUnits (playerIndex, true);
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectClan (size_t playerIndex, bool firstForPlayer)
{
	if (!game) return;

	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> ());

	windowClanSelection->done.connect ([=]()
	{
		game->setPlayerClan (playerIndex, windowClanSelection->getSelectedClan ());

		selectLandingUnits (playerIndex, false);
	});

	if (firstForPlayer)
	{
		windowClanSelection->canceled.connect ([=]()
		{
			application.closeTill (*windowPlayerSelection);
		});
	}
	else
	{
		windowClanSelection->canceled.connect ([=](){ windowClanSelection->close (); });
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectLandingUnits (size_t playerIndex, bool firstForPlayer)
{
	if (!game) return;

	auto initialLandingUnits = createInitialLandingUnitsList (game->getPlayerClan(playerIndex), *game->getGameSettings());

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (game->getPlayer (playerIndex).getColor (), game->getPlayerClan (playerIndex), initialLandingUnits, game->getGameSettings ()->getStartCredits ()));

	windowLandingUnitSelection->done.connect ([=]()
	{
		game->setLandingUnits (playerIndex, windowLandingUnitSelection->getLandingUnits ());
		game->setUnitUpgrades (playerIndex, windowLandingUnitSelection->getUnitUpgrades ());

		selectLandingPosition (playerIndex);
	});

	if (firstForPlayer)
	{
		windowLandingUnitSelection->canceled.connect ([=]()
		{
			application.closeTill (*windowPlayerSelection);
		});
	}
	else
	{
		windowLandingUnitSelection->canceled.connect ([=](){ windowLandingUnitSelection->close (); });
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectLandingPosition (size_t playerIndex)
{
	if (!game) return;

	playerLandingSelectionWindows[playerIndex] = std::make_shared<cWindowLandingPositionSelection> (game->getStaticMap ());

	auto windowLandingPositionSelection = application.show (playerLandingSelectionWindows[playerIndex]);

	windowLandingPositionSelection->canceled.connect ([=]() {windowLandingPositionSelection->close (); });
	landingSelectionWindowConnections.connect(windowLandingPositionSelection->selectedPosition, [=](cPosition landingPosition)
	{
		landingPositionManager->setLandingPosition (game->getPlayer (playerIndex), landingPosition);
		game->setLandingPosition (playerIndex, landingPosition);

		if (playerIndex == game->getPlayerCount () - 1)
		{
			checkAllLandingPositions ();
		}
		else
		{
			startNextPlayerGamePreperation (playerIndex + 1);
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::checkAllLandingPositions ()
{
	std::swap (invalidLandingPositionPlayers, nextInvalidLandingPositionPlayers);
	nextInvalidLandingPositionPlayers.clear ();

	landingSelectionWindowConnections.disconnectAll ();

	if (!invalidLandingPositionPlayers.empty ())
	{
		reselectLandingPosition (0);
	}
	else
	{
		game->start (application);

		game->terminated.connect ([&]()
		{
			application.closeTill (*firstWindow);
			firstWindow->close ();
			firstWindow = nullptr;
		});
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::reselectLandingPosition (size_t reselectIndex)
{
	auto playerIndex = invalidLandingPositionPlayers[reselectIndex].first;
	auto landingState = invalidLandingPositionPlayers[reselectIndex].second;

	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Multiplayer~Player_Turn", game->getPlayer(playerIndex).getName()), eWindowBackgrounds::Black));
	dialog->done.connect ([=]()
	{
		auto windowLandingPositionSelection = application.show (playerLandingSelectionWindows[playerIndex]);

		windowLandingPositionSelection->applyReselectionState (landingState);

		landingSelectionWindowConnections.connect(windowLandingPositionSelection->selectedPosition, [=](cPosition landingPosition)
		{
			landingPositionManager->setLandingPosition (game->getPlayer (playerIndex), landingPosition);
			game->setLandingPosition (playerIndex, landingPosition);

			if (reselectIndex == invalidLandingPositionPlayers.size () - 1)
			{
				checkAllLandingPositions ();
			}
			else
			{
				reselectLandingPosition (reselectIndex + 1);
			}
		});
	});
}