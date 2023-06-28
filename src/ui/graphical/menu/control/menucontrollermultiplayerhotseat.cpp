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

#include "menucontrollermultiplayerhotseat.h"

#include "game/data/gamesettings.h"
#include "game/data/map/map.h"
#include "game/data/player/playerbasicdata.h"
#include "game/data/units/landingunit.h"
#include "game/logic/upgradecalculator.h"
#include "game/startup/gamepreparation.h"
#include "resources/playercolor.h"
#include "resources/uidata.h"
#include "ui/widgets/application.h"
#include "ui/graphical/menu/control/local/hotseat/localhotseatgamenew.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windowplayerselection/windowplayerselection.h"
#include "utility/language.h"

#include <cassert>

//------------------------------------------------------------------------------
cMenuControllerMultiplayerHotSeat::cMenuControllerMultiplayerHotSeat (cApplication& application_) :
	application (application_)
{}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::start()
{
	game = std::make_shared<cLocalHotSeatGameNew>();

	//initialize copy of unitsData that will be used in game
	game->setUnitsData (std::make_shared<const cUnitsData> (UnitsDataGlobal));
	game->setClanData (std::make_shared<const cClanData> (ClanDataGlobal));

	selectGameSettings();
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::reset()
{
	game = nullptr;
	windowPlayerSelection = nullptr;
	firstWindow = nullptr;
	landingPositionManager = nullptr;
	nextInvalidLandingPositionPlayers.clear();
	invalidLandingPositionPlayers.clear();
	playerLandingSelectionWindows.clear();
	landingSelectionWindowConnections.disconnectAll();
	application.removeRunnable (shared_from_this()); // finish lifetime
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectGameSettings()
{
	if (!game) return;

	application.addRunnable (shared_from_this()); // ensure lifetime

	auto windowGameSettings = application.show (std::make_shared<cWindowGameSettings> (true));
	windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->terminated.connect ([this]() { reset(); });

	firstWindow = windowGameSettings;

	windowGameSettings->done.connect ([this] (const cGameSettings& gameSettings) {
		game->setGameSettings (std::make_shared<cGameSettings> (gameSettings));

		selectMap();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectMap()
{
	if (!game) return;

	auto windowMapSelection = application.show (std::make_shared<cWindowMapSelection>());

	windowMapSelection->done.connect ([this] (const std::string& mapName) {
		auto staticMap = std::make_shared<cStaticMap>();
		if (!staticMap->loadMap (mapName))
		{
			// TODO: error dialog: could not load selected map!
			return;
		}
		game->setStaticMap (staticMap);

		selectPlayers();
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectPlayers()
{
	if (!game) return;

	windowPlayerSelection = application.show (std::make_shared<cWindowPlayerSelection>());

	windowPlayerSelection->done.connect ([=]() {
		const auto& playerTypes = windowPlayerSelection->getPlayerTypes();

		const std::string playerNames[] =
			{
				lngPack.i18n ("Multiplayer~Player1"),
				lngPack.i18n ("Multiplayer~Player2"),
				lngPack.i18n ("Multiplayer~Player3"),
				lngPack.i18n ("Multiplayer~Player4"),
				lngPack.i18n ("Multiplayer~Player5"),
				lngPack.i18n ("Multiplayer~Player6"),
				lngPack.i18n ("Multiplayer~Player7"),
				lngPack.i18n ("Multiplayer~Player8")};

		std::vector<cPlayerBasicData> players;
		int playerNum = 0;
		for (size_t i = 0; i < playerTypes.size(); ++i)
		{
			if (playerTypes[i] == ePlayerType::Human)
			{
				cPlayerBasicData player ({playerNames[i], cPlayerColor::predefinedColors[i]}, playerNum++, false);
				players.push_back (player);
			}
		}
		assert (players.size() > 0);

		game->setPlayers (players);

		playerLandingSelectionWindows.resize (players.size());
		landingPositionManager = std::make_unique<cLandingPositionManager> (players);
		landingPositionManager->landingPositionStateChanged.connect ([this] (const cPlayerBasicData& player, eLandingPositionState state) {
			if (state == eLandingPositionState::TooClose || state == eLandingPositionState::Warning)
			{
				for (size_t i = 0; i < game->getPlayerCount(); ++i)
				{
					if (game->getPlayer (i).getNr() == player.getNr())
					{
						nextInvalidLandingPositionPlayers.push_back (std::make_pair (i, state));
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
	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Multiplayer~Player_Turn", game->getPlayer (playerIndex).getName()), eWindowBackgrounds::Black));
	dialog->done.connect ([this, playerIndex]() {
		if (game->getGameSettings()->clansEnabled)
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

	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> (game->getUnitsData(), game->getClanData()));

	windowClanSelection->done.connect ([=]() {
		game->setPlayerClan (playerIndex, windowClanSelection->getSelectedClan());

		selectLandingUnits (playerIndex, false);
	});

	if (firstForPlayer)
	{
		windowClanSelection->canceled.connect ([=]() {
			application.closeTill (*windowPlayerSelection);
		});
	}
	else
	{
		windowClanSelection->canceled.connect ([=]() { windowClanSelection->close(); });
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectLandingUnits (size_t playerIndex, bool firstForPlayer)
{
	if (!game) return;

	auto initialLandingUnits = computeInitialLandingUnits (game->getPlayerClan (playerIndex), *game->getGameSettings(), *game->getUnitsData());

	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (game->getPlayer (playerIndex).getColor(), game->getPlayerClan (playerIndex), initialLandingUnits, game->getGameSettings()->startCredits, game->getUnitsData()));

	windowLandingUnitSelection->done.connect ([=]() {
		game->setLandingUnits (playerIndex, windowLandingUnitSelection->getLandingUnits());
		game->setUnitUpgrades (playerIndex, windowLandingUnitSelection->getUnitUpgrades());

		selectLandingPosition (playerIndex);
	});

	if (firstForPlayer)
	{
		windowLandingUnitSelection->canceled.connect ([=]() {
			application.closeTill (*windowPlayerSelection);
		});
	}
	else
	{
		windowLandingUnitSelection->canceled.connect ([=]() { windowLandingUnitSelection->close(); });
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::selectLandingPosition (size_t playerIndex)
{
	if (!game) return;

	auto& map = game->getStaticMap();
	bool fixedBridgeHead = game->getGameSettings()->bridgeheadType == eGameSettingsBridgeheadType::Definite;
	auto& landingUnits = game->getLandingUnits (playerIndex);
	auto unitsData = game->getUnitsData();
	playerLandingSelectionWindows[playerIndex] = std::make_shared<cWindowLandingPositionSelection> (map, fixedBridgeHead, landingUnits, unitsData, true);

	auto windowLandingPositionSelection = application.show (playerLandingSelectionWindows[playerIndex]);

	windowLandingPositionSelection->canceled.connect ([=]() { windowLandingPositionSelection->close(); });
	landingSelectionWindowConnections.connect (windowLandingPositionSelection->selectedPosition, [=] (cPosition landingPosition) {
		landingPositionManager->setLandingPosition (game->getPlayer (playerIndex), landingPosition);
		game->setLandingPosition (playerIndex, landingPosition);

		if (playerIndex == game->getPlayerCount() - 1)
		{
			checkAllLandingPositions();
		}
		else
		{
			startNextPlayerGamePreperation (playerIndex + 1);
		}
	});
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::checkAllLandingPositions()
{
	std::swap (invalidLandingPositionPlayers, nextInvalidLandingPositionPlayers);
	nextInvalidLandingPositionPlayers.clear();

	landingSelectionWindowConnections.disconnectAll();

	if (!invalidLandingPositionPlayers.empty())
	{
		reselectLandingPosition (0);
	}
	else
	{
		application.closeTill (*firstWindow);
		firstWindow->close();
		signalConnectionManager.connect (firstWindow->terminated, [this]() { firstWindow = nullptr; });

		game->start (application);
	}
}

//------------------------------------------------------------------------------
void cMenuControllerMultiplayerHotSeat::reselectLandingPosition (size_t reselectIndex)
{
	auto playerIndex = invalidLandingPositionPlayers[reselectIndex].first;
	auto landingState = invalidLandingPositionPlayers[reselectIndex].second;

	auto dialog = application.show (std::make_shared<cDialogOk> (lngPack.i18n ("Multiplayer~Player_Turn", game->getPlayer (playerIndex).getName()), eWindowBackgrounds::Black));
	dialog->done.connect ([=]() {
		auto windowLandingPositionSelection = application.show (playerLandingSelectionWindows[playerIndex]);

		windowLandingPositionSelection->applyReselectionState (landingState);

		landingSelectionWindowConnections.connect (windowLandingPositionSelection->selectedPosition, [=] (cPosition landingPosition) {
			landingPositionManager->setLandingPosition (game->getPlayer (playerIndex), landingPosition);
			game->setLandingPosition (playerIndex, landingPosition);

			if (reselectIndex == invalidLandingPositionPlayers.size() - 1)
			{
				checkAllLandingPositions();
			}
			else
			{
				reselectLandingPosition (reselectIndex + 1);
			}
		});
	});
}
