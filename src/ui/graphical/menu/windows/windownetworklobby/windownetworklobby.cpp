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

#include "game/data/gamesettings.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/graphical/menu/widgets/special/lobbyplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/tools/validatorint.h"
#include "ui/graphical/menu/dialogs/dialogcolorpicker.h"
#include "game/data/player/player.h"
#include "game/startup/lobbyclient.h"
#include "utility/language.h"
#include "resources/pcx.h"
#include "utility/string/toString.h"
#include "game/data/map/map.h"
#include "output/video/video.h"
#include "game/data/savegame.h"
#include "maxrversion.h"

//------------------------------------------------------------------------------
cWindowNetworkLobby::cWindowNetworkLobby (const std::string title, bool disableIp) :
	cWindow (LoadPCX (GFXOD_MULT)),
	localPlayer (std::make_shared<cPlayerBasicData> (cPlayerBasicData::fromSettings())),
	saveGameInfo(-1)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 11), getPosition() + cPosition (getArea().getMaxCorner().x(), 11 + 10)), title, FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	mapImage = addChild (std::make_unique<cImage> (getPosition() + cPosition (33, 106)));
	mapNameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (90 - 70, 65), getPosition() + cPosition (90 + 70, 65 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	settingsTextLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (192, 52), getPosition() + cPosition (192 + 246, 52 + 175)), ""));
	settingsTextLabel->setWordWrap (true);

	chatLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (20, 424), getPosition() + cPosition (20 + 430, 424 + 10))));
	signalConnectionManager.connect (chatLineEdit->returnPressed, std::bind (&cWindowNetworkLobby::triggerChatMessage, this, true));
	auto sendButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 416), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Send")));
	signalConnectionManager.connect (sendButton->clicked, std::bind (&cWindowNetworkLobby::triggerChatMessage, this, false));
	chatList = addChild (std::make_unique<cListView<cLobbyChatBoxListViewItem>> (cBox<cPosition> (getPosition() + cPosition (14, 284), getPosition() + cPosition (14 + 439, 284 + 124)), eScrollBarStyle::Classic));
	chatList->disableSelectable();
	chatList->setBeginMargin (cPosition (12, 12));
	chatList->setEndMargin (cPosition (10, 10));
	chatList->setScrollOffset (cUnicodeFont::font->getFontHeight() + 3);

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (20, 245), getPosition() + cPosition (20 + 170, 245 + 10)), lngPack.i18n ("Text~Title~IP"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (228, 245), getPosition() + cPosition (228 + 90, 245 + 10)), lngPack.i18n ("Text~Title~Port"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (352, 245), getPosition() + cPosition (352 + 90, 245 + 10)), lngPack.i18n ("Text~Title~Player_Name"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (500, 245), getPosition() + cPosition (500 + 90, 245 + 10)), lngPack.i18n ("Text~Title~Color"), FONT_LATIN_NORMAL, eAlignmentType::Left));

	ipLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (20, 260), getPosition() + cPosition (20 + 178, 260 + 10))));
	if (disableIp)
	{
		ipLineEdit->setText ("-");
		ipLineEdit->setReadOnly (true);
	}
	else
	{
		ipLineEdit->setText(cSettings::getInstance().getIP());
	}
	portLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (230, 260), getPosition() + cPosition (230 + 95, 260 + 10))));
	portLineEdit->setText (iToStr (cSettings::getInstance().getPort()));
	portLineEdit->setValidator (std::make_unique<cValidatorInt> (0, 65535));
	restoreDefaultPortButton = addChild(std::make_unique<cImage>(getPosition() + cPosition(230 + 82, 254), GraphicsData.gfx_Cpfeil2.get()));
	signalConnectionManager.connect(restoreDefaultPortButton->clicked, [this]()
	{
		portLineEdit->setText(iToStr(DEFAULTPORT));
	});

	auto nameLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (353, 260), getPosition() + cPosition (353 + 95, 260 + 10))));
	nameLineEdit->setText (localPlayer->getName());
	signalConnectionManager.connect (nameLineEdit->returnPressed, [this, nameLineEdit]()
	{
		auto application = getActiveApplication();
		if (application) application->releaseKeyFocus (*nameLineEdit);
	});
	signalConnectionManager.connect (nameLineEdit->editingFinished, [&, nameLineEdit] (eValidatorState) {localPlayer->setName (nameLineEdit->getText()); });

	mapButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 42), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Choose_Planet")));
	signalConnectionManager.connect (mapButton->clicked, [this]() { triggeredSelectMap(); });

	settingsButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 77), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Options")));
	signalConnectionManager.connect (settingsButton->clicked, [this]() { triggeredSelectSettings(); });

	loadButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (470, 120), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Game_Load")));
	signalConnectionManager.connect (loadButton->clicked, [this]() { triggeredSelectSaveGame(); });

	playersList = addChild (std::make_unique<cListView<cLobbyPlayerListViewItem>> (cBox<cPosition> (getPosition() + cPosition (465, 284), getPosition() + cPosition (465 + 167, 284 + 124)), eScrollBarStyle::Classic));
	playersList->disableSelectable();
	playersList->setBeginMargin (cPosition (12, 12));
	playersList->setEndMargin (cPosition (10, 10));
	playersList->setItemDistance (4);

	colorImage = addChild (std::make_unique<cImage> (getPosition() + cPosition (505, 260)));
	signalConnectionManager.connect (colorImage->clicked, [this]()
	{
		auto application = getActiveApplication();

		if (!application) return;

		auto dialog = application->show (std::make_shared<cDialogColorPicker> (localPlayer->getColor().getColor()));
		dialog->done.connect ([this, dialog]()
		{
			localPlayer->setColor (cPlayerColor (dialog->getSelectedColor()));
			dialog->close();
		});
		dialog->canceled.connect ([dialog]() { dialog->close(); });
	});
	auto nextColorButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (596, 256), ePushButtonType::ArrowRightSmall, &SoundData.SNDObjectMenu));
	signalConnectionManager.connect (nextColorButton->clicked, [this]()
	{
		const auto localPlayerColorIndex = (cPlayerColor::findClosestPredefinedColor (localPlayer->getColor().getColor()) + 1) % cPlayerColor::predefinedColorsCount;
		localPlayer->setColor (cPlayerColor (cPlayerColor::predefinedColors[localPlayerColorIndex]));
	});
	auto prevColorButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (478, 256), ePushButtonType::ArrowLeftSmall, &SoundData.SNDObjectMenu));
	signalConnectionManager.connect (prevColorButton->clicked, [this]()
	{
		const auto localPlayerColorIndex = (cPlayerColor::findClosestPredefinedColor (localPlayer->getColor().getColor()) + cPlayerColor::predefinedColorsCount - 1) % cPlayerColor::predefinedColorsCount;
		localPlayer->setColor (cPlayerColor (cPlayerColor::predefinedColors[localPlayerColorIndex]));
	});

	okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 450), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, [this]() { triggeredStartGame(); });

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (50, 450), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, [&]() { backClicked(); });

	updateSettingsText();
	updateMap();
	updatePlayerColor();

	localPlayer->colorChanged.connect (std::bind (&cWindowNetworkLobby::updatePlayerColor, this));

	addPlayer (localPlayer);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::bindConnections (cLobbyClient& lobbyClient)
{
	signalConnectionManager.connect (lobbyClient.onLocalPlayerConnected, [this](){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Connected"));
	});
	signalConnectionManager.connect (lobbyClient.onDifferentVersion, [this](const std::string& version, const std::string& revision){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Gameversion_Warning_Client", version + " " + revision));
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Gameversion_Own", (std::string)PACKAGE_VERSION + " " + PACKAGE_REV));
	});
	signalConnectionManager.connect (lobbyClient.onConnectionFailed, [this](const std::string& reason){
		if (reason.empty())
		{
			addInfoEntry (lngPack.i18n ("Text~Multiplayer~Network_Error_Connect", "server"));
		}
		else
		{
			addInfoEntry (lngPack.i18n (reason));
		}
		enablePortEdit();
		enableIpEdit();
	});
	signalConnectionManager.connect (lobbyClient.onConnectionClosed, [this](){
		removePlayers();
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server"));

		enablePortEdit();
		enableIpEdit();
	});

	signalConnectionManager.connect (lobbyClient.onNoMapNoReady, [this](const std::string& mapName){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", mapName));
	});
	signalConnectionManager.connect (lobbyClient.onIncompatibleMap, [this](const std::string& mapName, const std::string& localPath){
		addInfoEntry ("You have an incompatible version of the");  //TODO: translate
		addInfoEntry (std::string ("map \"") + mapName + "\" at");
		addInfoEntry (std::string ("\"") + localPath + "\" !");
		addInfoEntry ("Move it away or delete it, then reconnect.");
	});
	signalConnectionManager.connect (lobbyClient.onMapDownloadRequest, [this](const std::string& mapName){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequest"));
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Download", mapName));
	});

	signalConnectionManager.connect (lobbyClient.onMissingOriginalMap, [this](const std::string& mapName){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequestInvalid"));
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadInvalid", mapName));
	});

	signalConnectionManager.connect (lobbyClient.onDownloadMapPercentChanged, [this](std::size_t percent){
		setMapDownloadPercent (percent);
	});
	signalConnectionManager.connect (lobbyClient.onDownloadMapCancelled, [this](){
		setMapDownloadCanceled();
	});
	signalConnectionManager.connect (lobbyClient.onDownloadMapFinished, [this](std::shared_ptr<cStaticMap> staticMap){
		setStaticMap (staticMap);
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~MapDL_Finished"));
	});

	signalConnectionManager.connect (lobbyClient.onPlayersList, [this](const cPlayerBasicData& localPlayer, const std::vector<cPlayerBasicData>& players){
		updatePlayerList (localPlayer, players);
	});
	signalConnectionManager.connect (lobbyClient.onDuplicatedPlayerColor, [this](){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Color_Taken"));
	});
	signalConnectionManager.connect (lobbyClient.onDuplicatedPlayerName, [this](){
		addInfoEntry (lngPack.i18n ("Text~Multiplayer~Player_Name_Taken"));
	});

	signalConnectionManager.connect (lobbyClient.onOptionsChanged, [this](std::shared_ptr<cGameSettings> settings, std::shared_ptr<cStaticMap> map, const cSaveGameInfo& saveGameInfo){
		setGameSettings (settings ? std::make_unique<cGameSettings> (*settings) : nullptr);
		setStaticMap (std::move (map));
	});

	signalConnectionManager.connect (triggeredChatMessage, [&lobbyClient, this](){
		const auto& chatMessage = getChatMessage();

		if (chatMessage.empty()) return;

		lobbyClient.sendChatMessage (chatMessage);

		const auto& localPlayer = getLocalPlayer();

		if (localPlayer)
		{
			addChatEntry (localPlayer->getName(), chatMessage);
		}
	});

	signalConnectionManager.connect (wantLocalPlayerReadyChange, [&lobbyClient](){ lobbyClient.tryToSwitchReadyState(); });

	auto handleLocalPlayerAttributesChanged = [&lobbyClient, this](){
		const auto& player = getLocalPlayer();

		lobbyClient.changeLocalPlayerProperties (player->getName(), player->getColor(), player->isReady());
	};
	signalConnectionManager.connect (getLocalPlayer()->nameChanged, handleLocalPlayerAttributesChanged);
	signalConnectionManager.connect (getLocalPlayer()->colorChanged, handleLocalPlayerAttributesChanged);
	signalConnectionManager.connect (getLocalPlayer()->readyChanged, handleLocalPlayerAttributesChanged);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updateSettingsText()
{
	std::string text = lngPack.i18n ("Text~Main~Version", PACKAGE_VERSION) + "\n";

	if (saveGameInfo.number >= 0)
	{
		text += lngPack.i18n("Text~Title~Savegame") + ":\n  " + saveGameInfo.gameName + "\n\n" + lngPack.i18n("Text~Title~Players") + "\n";
		for (size_t i = 0; i < saveGameInfo.players.size(); ++i)
		{
			text += saveGameInfo.players[i].getName() + "\n";
		}
		text += "\n";
	}
	if (staticMap != nullptr)
	{
		text += lngPack.i18n ("Text~Title~Map") + lngPack.i18n ("Text~Punctuation~Colon") + staticMap->getName();
		text += " (" + iToStr (staticMap->getSize().x()) + "x" + iToStr (staticMap->getSize().y()) + ")\n";
	}
	else if (saveGameInfo.number < 0)
	{
		text += lngPack.i18n ("Text~Multiplayer~Map_NoSet") + "\n";
	}

	text += "\n";

	if (saveGameInfo.number < 0)
	{
		if (gameSettings)
		{
			auto additionalGameEndString = gameSettings->getVictoryCondition() == eGameSettingsVictoryCondition::Turns ? (" " + iToStr (gameSettings->getVictoryTurns()) + " ") : (gameSettings->getVictoryCondition() == eGameSettingsVictoryCondition::Points ? (" " + iToStr (gameSettings->getVictoryPoints()) + " ") : " ");
			text += lngPack.i18n ("Text~Comp~GameEndsAt") + additionalGameEndString + toTranslatedString (gameSettings->getVictoryCondition()) + "\n";
			text += lngPack.i18n ("Text~Title~Metal") + lngPack.i18n ("Text~Punctuation~Colon") + toTranslatedString (gameSettings->getMetalAmount()) + "\n";
			text += lngPack.i18n ("Text~Title~Oil") + lngPack.i18n ("Text~Punctuation~Colon") + toTranslatedString (gameSettings->getOilAmount()) + "\n";
			text += lngPack.i18n ("Text~Title~Gold") + lngPack.i18n ("Text~Punctuation~Colon") + toTranslatedString (gameSettings->getGoldAmount()) + "\n";
			text += lngPack.i18n ("Text~Title~Resource_Density") + lngPack.i18n ("Text~Punctuation~Colon") + toTranslatedString (gameSettings->getResourceDensity()) + "\n";
			text += lngPack.i18n ("Text~Title~Credits")  + lngPack.i18n ("Text~Punctuation~Colon") + iToStr (gameSettings->getStartCredits()) + "\n";
			text += lngPack.i18n ("Text~Title~BridgeHead") + lngPack.i18n ("Text~Punctuation~Colon") + toTranslatedString (gameSettings->getBridgeheadType()) + "\n";
			text += lngPack.i18n ("Text~Title~Clans") + lngPack.i18n ("Text~Punctuation~Colon") + (gameSettings->getClansEnabled() ? lngPack.i18n ("Text~Option~On") : lngPack.i18n ("Text~Option~Off")) + "\n";
			text += lngPack.i18n ("Text~Title~Game_Type") + lngPack.i18n ("Text~Punctuation~Colon") + toTranslatedString (gameSettings->getGameType()) + "\n";
			text += lngPack.i18n ("Text~Title~Turn_limit") + lngPack.i18n ("Text~Punctuation~Colon") + (gameSettings->isTurnLimitActive() ? iToStr (gameSettings->getTurnLimit().count()) + "s" : lngPack.i18n ("Text~Settings~Unlimited_11")) + "\n";
			if (gameSettings->getGameType() == eGameSettingsGameType::Simultaneous)
			{
				text += lngPack.i18n ("Text~Title~Turn_end") + lngPack.i18n ("Text~Punctuation~Colon") + (gameSettings->isTurnEndDeadlineActive() ? iToStr (gameSettings->getTurnEndDeadline().count()) + "s" : lngPack.i18n ("Text~Settings~Unlimited_11")) + "\n";
			}
		}
		else text += lngPack.i18n ("Text~Multiplayer~Option_NoSet") + "\n";
	}
	if (saveGameInfo.number >= 0) {
		text += lngPack.i18n("Text~Comp~Turn_5") + lngPack.i18n ("Text~Punctuation~Colon") + iToStr(saveGameInfo.turn) + "\n";
	}
	settingsTextLabel->setText (text);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updateMap()
{
	if (!staticMap || !staticMap->isValid())
	{
		mapImage->setImage (nullptr);
		mapNameLabel->setText ("");
		return;
	}

	AutoSurface surface (cStaticMap::loadMapPreview (staticMap->getName()));
	if (surface != nullptr)
	{
		mapImage->setImage (surface.get());
	}

	auto mapName = staticMap->getName();
	const auto size = staticMap->getSize();

	if (cUnicodeFont::font->getTextWide (">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size.x()) + "x" + iToStr (size.y()) + ")<") > 140)
	{
		while (cUnicodeFont::font->getTextWide (">" + mapName + "... (" + iToStr (size.x()) + "x" + iToStr (size.y()) + ")<") > 140)
		{
			mapName.erase (mapName.length() - 1, mapName.length());
		}
		mapName = mapName + "... (" + iToStr (size.x()) + "x" + iToStr (size.y()) + ")";
	}
	else mapName = mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size.x()) + "x" + iToStr (size.y()) + ")";

	mapNameLabel->setText (mapName);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updatePlayerColor()
{
	SDL_Rect src = {0, 0, 83, 10};
	AutoSurface colorSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_BlitSurface (localPlayer->getColor().getTexture(), &src, colorSurface.get(), nullptr);
	colorImage->setImage (colorSurface.get());
}
//------------------------------------------------------------------------------
void cWindowNetworkLobby::triggerChatMessage (bool keepFocus)
{
	if (!chatLineEdit->getText().empty())
	{
		triggeredChatMessage();
		chatLineEdit->setText ("");
	}
	if (!keepFocus)
	{
		auto application = getActiveApplication();
		if (application)
		{
			application->releaseKeyFocus (*chatLineEdit);
		}
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addChatEntry (const std::string& playerName, const std::string& message)
{
	chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message), eAddListItemScrollType::IfAtBottom);
	cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addInfoEntry (const std::string& message)
{
	chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (message), eAddListItemScrollType::IfAtBottom);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addPlayer (const std::shared_ptr<cPlayerBasicData>& player)
{
	auto item = playersList->addItem (std::make_unique<cLobbyPlayerListViewItem> (player));
	if (player == localPlayer)
	{
		signalConnectionManager.connect (item->readyClicked, [player, this]()
		{
			cSoundDevice::getInstance().playSoundEffect (SoundData.SNDHudButton);
			wantLocalPlayerReadyChange();
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
		if (item.getPlayer().get() == &player)
		{
			playersList->removeItem (item);
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::removePlayers()
{
	for (size_t i = 0; i < playersList->getItemsCount();)
	{
		auto& item = playersList->getItem(i);
		playersList->removeItem (item);
	}
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = std::move (staticMap_);

	updateMap();
	updateSettingsText();
	staticMapChanged();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setGameSettings (std::unique_ptr<cGameSettings> gameSettings_)
{
	gameSettings = std::move (gameSettings_);

	updateSettingsText();
	gameSettingsChanged();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setMapDownloadPercent (int percent)
{
	mapNameLabel->setText (lngPack.i18n ("Text~Multiplayer~MapDL_Percent", iToStr (percent)));
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setMapDownloadCanceled()
{
	mapNameLabel->setText (lngPack.i18n ("Text~Multiplayer~MapDL_Cancel"));
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGameSettings>& cWindowNetworkLobby::getGameSettings() const
{
	return gameSettings;
}

//------------------------------------------------------------------------------
const cSaveGameInfo& cWindowNetworkLobby::getSaveGameInfo() const
{
	return saveGameInfo;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cStaticMap>& cWindowNetworkLobby::getStaticMap() const
{
	return staticMap;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cPlayerBasicData>& cWindowNetworkLobby::getLocalPlayer() const
{
	return localPlayer;
}

//------------------------------------------------------------------------------
std::vector<std::shared_ptr<cPlayerBasicData>> cWindowNetworkLobby::getPlayers() const
{
	std::vector<std::shared_ptr<cPlayerBasicData>> result;
	for (size_t i = 0; i < playersList->getItemsCount(); ++i)
	{
		const auto& item = playersList->getItem (i);
		result.push_back (item.getPlayer());
	}
	return result;
}

//------------------------------------------------------------------------------
std::vector<cPlayerBasicData> cWindowNetworkLobby::getPlayersNotShared() const
{
	std::vector<cPlayerBasicData> result;
	for (size_t i = 0; i < playersList->getItemsCount(); ++i)
	{
		const auto& item = playersList->getItem (i);
		result.push_back (*item.getPlayer());
	}
	return result;
}

//------------------------------------------------------------------------------
std::shared_ptr<cPlayerBasicData> cWindowNetworkLobby::getPlayer(int playerNr) const
{
	for (size_t i = 0; i < playersList->getItemsCount(); ++i)
	{
		const auto& item = playersList->getItem(i);
		if (item.getPlayer()->getNr() == playerNr)
		{
			return item.getPlayer();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
unsigned short cWindowNetworkLobby::getPort() const
{
	return atoi (portLineEdit->getText().c_str());
}

//------------------------------------------------------------------------------
const std::string& cWindowNetworkLobby::getIp() const
{
	return ipLineEdit->getText();
}

//------------------------------------------------------------------------------
const std::string& cWindowNetworkLobby::getChatMessage() const
{
	return chatLineEdit->getText();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::disablePortEdit()
{
	portLineEdit->disable();
	restoreDefaultPortButton->disable();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::disableIpEdit()
{
	ipLineEdit->disable();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::enablePortEdit()
{
	portLineEdit->enable();
	restoreDefaultPortButton->enable();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::enableIpEdit()
{
	ipLineEdit->enable();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updatePlayerList (const cPlayerBasicData& local, const std::vector<cPlayerBasicData>& players)
{
	// Cannot use *localPlayer = local because of signal.
	localPlayer->setColor(local.getColor());
	localPlayer->setName(local.getName());
	localPlayer->setNr(local.getNr());
	localPlayer->setReady(local.isReady());

	removePlayers();
	for (const auto& playerData : players)
	{
		addPlayer (playerData.getNr() == localPlayer->getNr() ? localPlayer : std::make_shared<cPlayerBasicData> (playerData));
	}
	updatePlayerListView();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updatePlayerListView()
{
	for (unsigned int i = 0; i < playersList->getItemsCount(); i++)
	{
		playersList->getItem(i).update();
	}
}
