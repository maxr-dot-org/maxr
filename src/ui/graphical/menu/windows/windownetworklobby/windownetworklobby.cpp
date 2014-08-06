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

#include "ui/graphical/menu/windows/windownetworklobby/windownetworklobby.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbyplayerlistviewitem.h"
#include "ui/graphical/menu/dialogs/dialogcolorpicker.h"
#include "game/data/player/player.h"
#include "pcx.h"
#include "main.h"
#include "network.h"
#include "game/data/map/map.h"
#include "video.h"
#include "game/logic/savegame.h"
#include "network.h"
#include "menuevents.h"

//------------------------------------------------------------------------------
cWindowNetworkLobby::cWindowNetworkLobby (const std::string title, bool disableIp) :
	cWindow (LoadPCX (GFXOD_MULT)),
	localPlayer (std::make_shared<cPlayerBasicData> (cSettings::getInstance ().getPlayerName (), cPlayerColor(cSettings::getInstance ().getPlayerColor ()), 0, MAX_CLIENTS)),
	saveGameNumber (-1)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 11), getPosition () + cPosition (getArea ().getMaxCorner ().x (), 11 + 10)), title, FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	mapImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (33, 106)));
	mapNameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (90 - 70, 65), getPosition () + cPosition (90 + 70, 65 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	settingsTextLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (192, 52), getPosition () + cPosition (192 + 246, 52 + 146)), ""));
	settingsTextLabel->setWordWrap (true);

	chatLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition () + cPosition (20, 424), getPosition () + cPosition (20 + 430, 424 + 10))));
	signalConnectionManager.connect (chatLineEdit->returnPressed, std::bind (&cWindowNetworkLobby::triggerChatMessage, this, true));
	auto sendButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 416), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Send")));
	signalConnectionManager.connect (sendButton->clicked, std::bind (&cWindowNetworkLobby::triggerChatMessage, this, false));
	chatList = addChild (std::make_unique<cListView<cLobbyChatBoxListViewItem>> (cBox<cPosition> (getPosition () + cPosition (14, 284), getPosition () + cPosition (14 + 439, 284 + 124))));
	chatList->disableSelectable ();
	chatList->setBeginMargin (cPosition (12, 12));
	chatList->setEndMargin (cPosition (10, 10));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (20, 245), getPosition () + cPosition (20 + 170, 245 + 10)), lngPack.i18n ("Text~Title~IP"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (228, 245), getPosition () + cPosition (228 + 90, 245 + 10)), lngPack.i18n ("Text~Title~Port"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (352, 245), getPosition () + cPosition (352 + 90, 245 + 10)), lngPack.i18n ("Text~Title~Player_Name"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (500, 245), getPosition () + cPosition (500 + 90, 245 + 10)), lngPack.i18n ("Text~Title~Color"), FONT_LATIN_NORMAL, eAlignmentType::Left));

	ipLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition () + cPosition (20, 260), getPosition () + cPosition (20 + 178, 260 + 10))));
	if (disableIp)
	{
		ipLineEdit->setText ("-");
		ipLineEdit->setReadOnly (true);
	}
	portLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition () + cPosition (230, 260), getPosition () + cPosition (230 + 95, 260 + 10))));
	portLineEdit->setText (iToStr (cSettings::getInstance ().getPort ()));
	
	auto nameLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition () + cPosition (353, 260), getPosition () + cPosition (353 + 95, 260 + 10))));
	nameLineEdit->setText (localPlayer->getName ());
	signalConnectionManager.connect (nameLineEdit->editingFinished, [&, nameLineEdit](eValidatorState){localPlayer->setName (nameLineEdit->getText ()); });

	playersList = addChild (std::make_unique<cListView<cLobbyPlayerListViewItem>> (cBox<cPosition> (getPosition () + cPosition (465, 284), getPosition () + cPosition (465 + 167, 284 + 124))));
	playersList->disableSelectable ();
	playersList->setBeginMargin (cPosition (12, 12));
	playersList->setEndMargin (cPosition (10, 10));
	playersList->setItemDistance (cPosition (0, 4));

	auto selectColorButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (596, 256), ePushButtonType::ArrowRightSmall, &SoundData.SNDObjectMenu));
	signalConnectionManager.connect (selectColorButton->clicked, [this]()
	{
		auto application = getActiveApplication ();

		if (!application) return;

		auto dialog = application->show (std::make_shared<cDialogColorPicker> (localPlayer->getColor ().getColor()));
		dialog->done.connect ([this, dialog]()
		{
			localPlayer->setColor (cPlayerColor(dialog->getSelectedColor ()));
			dialog->close ();
		});
		dialog->canceled.connect ([dialog](){ dialog->close (); });
	});
	colorImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (505, 260)));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (50, 450), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, [&]() { close (); });

	updateSettingsText ();
	updateMap ();
	updatePlayerColor ();

	localPlayer->colorChanged.connect (std::bind (&cWindowNetworkLobby::updatePlayerColor, this));

	addPlayer (localPlayer);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updateSettingsText ()
{
	std::string text = lngPack.i18n ("Text~Main~Version", PACKAGE_VERSION) + "\n";
	//text += "Checksum: " + iToStr (cSettings::getInstance().Checksum) + "\n\n";

	if (saveGameNumber != -1)
	{
		text += lngPack.i18n ("Text~Title~Savegame") + ":\n  " + saveGameName +  "\n\n" + lngPack.i18n ("Text~Title~Players") + "\n";
		for (size_t i = 0; i < saveGamePlayers.size (); ++i)
		{
			text += saveGamePlayers[i].getName() + "\n";
		}
		text += "\n";
	}
	if (staticMap != nullptr)
	{
		text += lngPack.i18n ("Text~Title~Map") + ": " + staticMap->getName ();
		text += " (" + iToStr (staticMap->getSize ().x ()) + "x" + iToStr (staticMap->getSize ().y ()) + ")\n";
	}
	else if (saveGameNumber == -1) text += lngPack.i18n ("Text~Multiplayer~Map_NoSet") + "\n";

	text += "\n";

	if (saveGameNumber == -1)
	{
		if (gameSettings)
		{
			auto additionalGameEndString = gameSettings->getVictoryCondition () == eGameSettingsVictoryCondition::Turns ? (" " + iToStr (gameSettings->getVictoryTurns ()) + " ") : (gameSettings->getVictoryCondition () == eGameSettingsVictoryCondition::Points ? (" " + iToStr (gameSettings->getVictoryPoints ()) + " ") : " ");
			text += lngPack.i18n ("Text~Comp~GameEndsAt") + additionalGameEndString + gameSettingsVictoryConditionToString (gameSettings->getVictoryCondition (), true) + "\n";
			text += lngPack.i18n ("Text~Title~Metal") + ": " + gameSettingsResourceAmountToString (gameSettings->getMetalAmount (), true) + "\n";
			text += lngPack.i18n ("Text~Title~Oil") + ": " + gameSettingsResourceAmountToString (gameSettings->getOilAmount (), true) + "\n";
			text += lngPack.i18n ("Text~Title~Gold") + ": " + gameSettingsResourceAmountToString (gameSettings->getGoldAmount (), true) + "\n";
			text += lngPack.i18n ("Text~Title~Resource_Density") + ": " + gameSettingsResourceDensityToString (gameSettings->getResourceDensity (), true) + "\n";
			text += lngPack.i18n ("Text~Title~Credits")  + ": " + iToStr (gameSettings->getStartCredits()) + "\n";
			text += lngPack.i18n ("Text~Title~BridgeHead") + ": " + gameSettingsBridgeheadTypeToString (gameSettings->getBridgeheadType(), true) + "\n";
			text += std::string ("Clans") + ": " + (gameSettings->getClansEnabled() ? lngPack.i18n ("Text~Option~On") : lngPack.i18n ("Text~Option~Off")) + "\n";
			text += lngPack.i18n ("Text~Title~Game_Type") + ": " + gameSettingsGameTypeToString (gameSettings->getGameType (), true) + "\n";
		}
		else text += lngPack.i18n ("Text~Multiplayer~Option_NoSet") + "\n";
	}
	settingsTextLabel->setText (text);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updateMap ()
{
	if (!staticMap || !staticMap->isValid ())
	{
		mapImage->setImage (nullptr);
		mapNameLabel->setText ("");
		return;
	}

	AutoSurface surface (cStaticMap::loadMapPreview (staticMap->getName ()));
	if (surface != nullptr)
	{
		mapImage->setImage (surface.get ());
	}

	auto mapName = staticMap->getName ();
	const auto size = staticMap->getSize ();

	if (font->getTextWide (">" + mapName.substr (0, mapName.length () - 4) + " (" + iToStr (size.x ()) + "x" + iToStr (size.y ()) + ")<") > 140)
	{
		while (font->getTextWide (">" + mapName + "... (" + iToStr (size.x ()) + "x" + iToStr (size.y ()) + ")<") > 140)
		{
			mapName.erase (mapName.length () - 1, mapName.length ());
		}
		mapName = mapName + "... (" + iToStr (size.x ()) + "x" + iToStr (size.y ()) + ")";
	}
	else mapName = mapName.substr (0, mapName.length () - 4) + " (" + iToStr (size.x ()) + "x" + iToStr (size.y ()) + ")";

	mapNameLabel->setText (mapName);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updatePlayerColor ()
{
	SDL_Rect src = {0, 0, 83, 10};
	AutoSurface colorSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth (), 0, 0, 0, 0));
	SDL_BlitSurface (localPlayer->getColor ().getTexture(), &src, colorSurface.get (), NULL);
	colorImage->setImage (colorSurface.get ());
}
//------------------------------------------------------------------------------
void cWindowNetworkLobby::triggerChatMessage (bool refocusChatLine)
{
	if (!chatLineEdit->getText ().empty ())
	{
		triggeredChatMessage ();
		chatLineEdit->setText ("");
	}
	if (refocusChatLine)
	{
		auto application = getActiveApplication ();
		if (application)
		{
			application->grapKeyFocus (*chatLineEdit);
		}
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addChatEntry (const std::string& playerName, const std::string& message)
{
	auto addedItem = chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message));
	chatList->scrollToItem (addedItem);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addInfoEntry (const std::string& message)
{
	auto addedItem = chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (message));
	chatList->scrollToItem (addedItem);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addPlayer (const std::shared_ptr<cPlayerBasicData>& player)
{
	auto item = playersList->addItem (std::make_unique<cLobbyPlayerListViewItem> (player));
	if (player == localPlayer)
	{
		signalConnectionManager.connect (item->readyClicked, [player, this]()
		{
            cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (SoundData.SNDHudButton);
			wantLocalPlayerReadyChange ();
		});
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::removePlayer (const cPlayerBasicData& player)
{
	if (&player == localPlayer.get()) return; // do never remove the local player

	for (size_t i = 0; i < playersList->getItemsCount(); ++i)
	{
		auto& item = playersList->getItem (i);
		if (item.getPlayer ().get() == &player)
		{
			playersList->removeItem (item);
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::removeNonLocalPlayers ()
{
	for (size_t i = 0; i < playersList->getItemsCount ();)
	{
		auto& item = playersList->getItem (i);
		if (item.getPlayer ().get () != localPlayer.get ())
		{
			playersList->removeItem (item);
		}
		else
		{
			++i;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = std::move (staticMap_);

	saveGameNumber = -1;

	updateMap ();
	updateSettingsText ();
	staticMapChanged ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setGameSettings (std::unique_ptr<cGameSettings> gameSettings_)
{
	gameSettings = std::move (gameSettings_);

	saveGameNumber = -1;

	updateSettingsText ();
	gameSettingsChanged ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setSaveGame (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;

	if (saveGameNumber >= 0)
	{
		cSavegame saveGame (saveGameNumber_);

		saveGame.loadHeader (&saveGameName, nullptr, nullptr);
		saveGamePlayers = saveGame.loadPlayers ();

		staticMap = std::make_shared<cStaticMap> ();
		if (!staticMap->loadMap (saveGame.loadMapName ()))
		{
			// error dialog
			staticMap = nullptr;
		}
	}

	updateMap ();
	updateSettingsText ();
	saveGameChanged ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setMapDownloadPercent (int percent)
{
	mapNameLabel->setText (lngPack.i18n ("Text~Multiplayer~MapDL_Percent", iToStr (percent)));
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setMapDownloadCanceled ()
{
	mapNameLabel->setText (lngPack.i18n ("Text~Multiplayer~MapDL_Cancel"));
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cWindowNetworkLobby::getGameSettings () const
{
	return gameSettings;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cWindowNetworkLobby::getStaticMap () const
{
	return staticMap;
}

//------------------------------------------------------------------------------
int cWindowNetworkLobby::getSaveGameNumber () const
{
	return saveGameNumber;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cPlayerBasicData>& cWindowNetworkLobby::getLocalPlayer () const
{
	return localPlayer;
}

//------------------------------------------------------------------------------
std::vector<std::shared_ptr<cPlayerBasicData>> cWindowNetworkLobby::getPlayers () const
{
	std::vector<std::shared_ptr<cPlayerBasicData>> result;
	for (size_t i = 0; i < playersList->getItemsCount (); ++i)
	{
		const auto& item = playersList->getItem (i);
		result.push_back (item.getPlayer ());
	}
	return result;
}

//------------------------------------------------------------------------------
std::vector<cPlayerBasicData> cWindowNetworkLobby::getPlayersNotShared () const
{
	std::vector<cPlayerBasicData> result;
	for (size_t i = 0; i < playersList->getItemsCount (); ++i)
	{
		const auto& item = playersList->getItem (i);
		result.push_back (*item.getPlayer ());
	}
	return result;
}

//------------------------------------------------------------------------------
unsigned short cWindowNetworkLobby::getPort () const
{
	return atoi (portLineEdit->getText ().c_str());
}

//------------------------------------------------------------------------------
const std::string& cWindowNetworkLobby::getIp () const
{
	return ipLineEdit->getText ();
}

//------------------------------------------------------------------------------
const std::string& cWindowNetworkLobby::getChatMessage () const
{
	return chatLineEdit->getText ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::disablePortEdit ()
{
	portLineEdit->disable ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::disableIpEdit ()
{
	ipLineEdit->disable ();
}
