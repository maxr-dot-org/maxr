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

#include "initgamepreparation.h"

#include "game/startup/gamepreparation.h"
#include "game/startup/lobbyclient.h"
#include "ui/widgets/application.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "utility/language.h"
#include "utility/listhelpers.h"

#include <cassert>

//------------------------------------------------------------------------------
cInitGamePreparation::cInitGamePreparation (cApplication& application, cLobbyClient& lobbyClient) :
	application (application),
	lobbyClient (lobbyClient)
{}

//------------------------------------------------------------------------------
void cInitGamePreparation::bindConnections (cLobbyClient& lobbyClient)
{
	signalConnectionManager.connect (lobbyClient.onPlayerEnterLeaveLandingSelectionRoom, [this] (const cPlayerBasicData& player, bool isIn) {
		if (isIn)
		{
			playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (player));
			if (windowLandingPositionSelection) windowLandingPositionSelection->addChatPlayerEntry (*playersLandingStatus.back());
		}
		else
		{
			if (windowLandingPositionSelection) windowLandingPositionSelection->removeChatPlayerEntry (player.getNr());
			EraseIf (playersLandingStatus, [&] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == player.getNr(); });
		}
	});

	signalConnectionManager.connect (lobbyClient.onPlayerSelectLandingPosition, [this] (const cPlayerBasicData& player) {
		auto it = ranges::find_if (playersLandingStatus, [&] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == player.getNr(); });

		if (it == playersLandingStatus.end()) return;

		auto& playerLandingStatus = **it;

		playerLandingStatus.setHasSelectedPosition (true);
	});

	signalConnectionManager.connect (lobbyClient.onLandingDone, [this] (eLandingPositionState state) {
		if (!windowLandingPositionSelection) return;

		windowLandingPositionSelection->applyReselectionState (state);
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::onChatMessage (const std::string& playerName, const std::string& message)
{
	if (!windowLandingPositionSelection) return;

	windowLandingPositionSelection->addChatEntry (playerName, message);
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startGamePreparation()
{
	lobbyPreparationData = lobbyClient.getLobbyPreparationData();
	if (lobbyPreparationData.gameSettings->clansEnabled)
	{
		startClanSelection();
	}
	else
	{
		startLandingUnitSelection();
	}
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startClanSelection()
{
	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> (lobbyPreparationData.unitsData, lobbyPreparationData.clanData));
	windows.push_back (windowClanSelection);

	signalConnectionManager.connect (windowClanSelection->canceled, [this]() { back(); });
	signalConnectionManager.connect (windowClanSelection->done, [this, windowClanSelection]() {
		initPlayerData.clan = windowClanSelection->getSelectedClan();

		startLandingUnitSelection();
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startLandingUnitSelection()
{
	const auto& gameSettings = *lobbyPreparationData.gameSettings;
	const auto unitsData = lobbyPreparationData.unitsData;
	const auto initialLandingUnits = computeInitialLandingUnits (initPlayerData.clan, gameSettings, *unitsData);
	const auto& playerColor = lobbyClient.getLocalPlayer().getColor();
	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (playerColor, initPlayerData.clan, initialLandingUnits, gameSettings.startCredits, unitsData));
	windows.push_back (windowLandingUnitSelection);

	signalConnectionManager.connect (windowLandingUnitSelection->canceled, [this]() { back(); });
	signalConnectionManager.connect (windowLandingUnitSelection->done, [this, windowLandingUnitSelection]() {
		initPlayerData.landingUnits = windowLandingUnitSelection->getLandingUnits();
		initPlayerData.unitUpgrades = windowLandingUnitSelection->getUnitUpgrades();

		startLandingPositionSelection();
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startLandingPositionSelection()
{
	if (!lobbyPreparationData.staticMap) return;
	const auto& map = lobbyPreparationData.staticMap;
	const bool fixedBridgeHead = lobbyPreparationData.gameSettings->bridgeheadType == eGameSettingsBridgeheadType::Definite;
	const auto& unitsData = lobbyPreparationData.unitsData;

	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (map, fixedBridgeHead, initPlayerData.landingUnits, unitsData, !lobbyClient.isUniquePlayer());
	application.show (windowLandingPositionSelection);
	windows.push_back (windowLandingPositionSelection.get());

	for (const auto& status : playersLandingStatus)
	{
		windowLandingPositionSelection->addChatPlayerEntry (*status);
	}
	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]() {
		lobbyClient.enterLandingSelection();
	});
	// nothing for windowLandingPositionSelection->closed (see `canceled` below)

	signalConnectionManager.connect (windowLandingPositionSelection->canceled, [this]() {
		// Call `exitLandingSelection` only when "back" is clicked.
		// not in `closed` which is also called when window is auto-closed
		// once game **starts**.
		lobbyClient.exitLandingSelection();
		back();
	});
	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this] (cPosition landingPosition) {
		initPlayerData.landingPosition = landingPosition;
		lobbyClient.selectLandingPosition (landingPosition);
	});

	signalConnectionManager.connect (windowLandingPositionSelection->onCommandEntered, [this] (const std::string& text) {
		const std::string& playerName = lobbyClient.getLocalPlayer().getName();
		windowLandingPositionSelection->addChatEntry (playerName, text);
		cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
		lobbyClient.sendChatMessage (text);
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::back()
{
	assert (!windows.empty());
	if (windows.size() == 1)
	{
		checkReallyWantsToQuit();
	}
	else
	{
		windows.back()->close();
		if (windows.back() == windowLandingPositionSelection.get()) windowLandingPositionSelection.reset();
		windows.pop_back();
	}
}

//------------------------------------------------------------------------------
void cInitGamePreparation::checkReallyWantsToQuit()
{
	auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> (lngPack.i18n ("GamePreparation~Aborting?")));

	signalConnectionManager.connect (yesNoDialog->yesClicked, [this]() {
		lobbyClient.abortGamePreparation();
		close();
		windowLandingPositionSelection.reset();
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::close()
{
	for (auto window : windows)
	{
		window->close();
	}
	windows.clear();
}
