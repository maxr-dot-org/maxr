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

#include "game/startup/lobbyclient.h"
#include "game/startup/gamepreparation.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/dialogs/dialogyesno.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cInitGamePreparation::cInitGamePreparation (cApplication& application, cLobbyClient& lobbyClient) :
	application (application),
	lobbyClient (lobbyClient)
{}

//------------------------------------------------------------------------------
void cInitGamePreparation::bindConnections (cLobbyClient& lobbyClient)
{
	signalConnectionManager.connect (lobbyClient.onPlayerEnterLeaveLandingSelectionRoom, [this](const cPlayerBasicData& player, bool isIn){
		if (isIn)
		{
			playersLandingStatus.push_back (std::make_unique<cPlayerLandingStatus> (player));
			if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*playersLandingStatus.back()));
		}
		else
		{
			if (windowLandingPositionSelection) windowLandingPositionSelection->getChatBox()->removePlayerEntry (player.getNr());
			playersLandingStatus.erase (std::remove_if (playersLandingStatus.begin(), playersLandingStatus.end(), [&] (const std::unique_ptr<cPlayerLandingStatus>& status) { return status->getPlayer().getNr() == player.getNr(); }), playersLandingStatus.end());
		}
	});

	signalConnectionManager.connect (lobbyClient.onPlayerSelectLandingPosition, [this](const cPlayerBasicData& player){
		auto it = ranges::find_if (playersLandingStatus, [&] (const std::unique_ptr<cPlayerLandingStatus>& entry) { return entry->getPlayer().getNr() == player.getNr(); });

		if (it == playersLandingStatus.end()) return;

		auto& playerLandingStatus = **it;

		playerLandingStatus.setHasSelectedPosition (true);
	});

	signalConnectionManager.connect (lobbyClient.onLandingDone, [this](eLandingPositionState state){
		if (!windowLandingPositionSelection) return;

		windowLandingPositionSelection->applyReselectionState (state);
	});

}

//------------------------------------------------------------------------------
void cInitGamePreparation::onChatMessage(const std::string& playerName, bool translate, const std::string& message, const std::string& insertText)
{
	if (!windowLandingPositionSelection) return;

	auto* chatBox = windowLandingPositionSelection->getChatBox();
	if (translate)
	{
		chatBox->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (lngPack.i18n (message, insertText)));
	}
	else
	{
		chatBox->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message));
		cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
	}
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startGamePreparation (const sLobbyPreparationData& lobbyData)
{
	lobbyPreparationData = lobbyData;
	if (lobbyData.gameSettings->getClansEnabled())
	{
		startClanSelection (true);
	}
	else
	{
		startLandingUnitSelection (true);
	}
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startClanSelection (bool isFirstWindowOnGamePreparation)
{
	auto windowClanSelection = application.show (std::make_shared<cWindowClanSelection> (lobbyPreparationData.unitsData, lobbyPreparationData.clanData));
	windows.push_back (windowClanSelection);

	signalConnectionManager.connect (windowClanSelection->canceled, [this]() { back(); });
	signalConnectionManager.connect (windowClanSelection->done, [this, windowClanSelection]()
	{
		clan = windowClanSelection->getSelectedClan();

		startLandingUnitSelection (false);
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startLandingUnitSelection (bool isFirstWindowOnGamePreparation)
{
	const auto& gameSettings = *lobbyPreparationData.gameSettings;
	const auto unitsData = lobbyPreparationData.unitsData;
	const auto initialLandingUnits = computeInitialLandingUnits (clan, gameSettings, *unitsData);
	auto windowLandingUnitSelection = application.show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), clan, initialLandingUnits, gameSettings.getStartCredits(), unitsData));
	windows.push_back (windowLandingUnitSelection);

	signalConnectionManager.connect (windowLandingUnitSelection->canceled, [this]() { back(); });
	signalConnectionManager.connect (windowLandingUnitSelection->done, [this, windowLandingUnitSelection]()
	{
		landingUnits = windowLandingUnitSelection->getLandingUnits();
		unitUpgrades = windowLandingUnitSelection->getUnitUpgrades();

		startLandingPositionSelection();
	});
}

//------------------------------------------------------------------------------
void cInitGamePreparation::startLandingPositionSelection()
{
	if (!lobbyPreparationData.staticMap) return;
	const auto& map = lobbyPreparationData.staticMap;
	const bool fixedBridgeHead = lobbyPreparationData.gameSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
	const auto& unitsData = lobbyPreparationData.unitsData;

	windowLandingPositionSelection = std::make_shared<cWindowLandingPositionSelection> (map, fixedBridgeHead, landingUnits, unitsData, true);
	application.show (windowLandingPositionSelection);
	windows.push_back (windowLandingPositionSelection.get());

	for (const auto& status : playersLandingStatus)
	{
		windowLandingPositionSelection->getChatBox()->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (*status));
	}
	signalConnectionManager.connect (windowLandingPositionSelection->opened, [this]()
	{
		lobbyClient.enterLandingSelection();
	});
	signalConnectionManager.connect (windowLandingPositionSelection->closed, [this]()
	{
		lobbyClient.exitLandingSelection();
	});

	signalConnectionManager.connect (windowLandingPositionSelection->canceled, [=]() { back(); });
	signalConnectionManager.connect (windowLandingPositionSelection->selectedPosition, [this] (cPosition landingPosition)
	{
		this->landingPosition = landingPosition;
		lobbyClient.selectLandingPosition (landingPosition);
	});

	signalConnectionManager.connect (windowLandingPositionSelection->getChatBox()->commandEntered, [=] (const std::string& text)
	{
		const std::string& playerName = lobbyClient.getLocalPlayerName();
		windowLandingPositionSelection->getChatBox()->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerName, text));
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
	auto yesNoDialog = application.show (std::make_shared<cDialogYesNo> ("Are you sure you want to abort the game preparation?")); // TODO: translate

	signalConnectionManager.connect(yesNoDialog->yesClicked, [this]()
	{
		lobbyClient.abortGamePreparation();
		for (auto window : windows)
		{
			window->close();
		}
		windows.clear();
		windowLandingPositionSelection.reset();
	});
}
