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

#include "windownetworklobby.h"
#include "../windowgamesettings/gamesettings.h"
#include "../../widgets/label.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/lineedit.h"
#include "../../widgets/image.h"
#include "../../widgets/listview.h"
#include "../../widgets/special/lobbychatboxlistviewitem.h"
#include "../../widgets/special/lobbyplayerlistviewitem.h"
#include "../../../../player.h"
#include "../../../../pcx.h"
#include "../../../../main.h"
#include "../../../../network.h"
#include "../../../../map.h"
#include "../../../../video.h"
#include "../../../../savegame.h"
#include "../../../../network.h"

//------------------------------------------------------------------------------
cWindowNetworkLobby::cWindowNetworkLobby (const std::string title, bool disableIp) :
	cWindow (LoadPCX (GFXOD_MULT)),
	player (std::make_shared<sPlayer>(cSettings::getInstance ().getPlayerName (), cSettings::getInstance ().getPlayerColor (), 0, MAX_CLIENTS)),
	network (std::make_shared<cTCP> ()),
	saveGameNumber (-1)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 11), getPosition () + cPosition (getArea ().getMaxCorner ().x (), 11 + 10)), title, FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	mapImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (33, 106)));
	mapNameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (90 - 70, 65), getPosition () + cPosition (90 + 70, 65 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	settingsTextLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (192, 52), getPosition () + cPosition (192 + 246, 52 + 146)), ""));
	settingsTextLabel->setWordWrap (true);

	chatLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition () + cPosition (20, 424), getPosition () + cPosition (20 + 430, 424 + 10))));
	signalConnectionManager.connect (chatLineEdit->returnPressed, std::bind (&cWindowNetworkLobby::sendChatMessage, this, true));
	auto sendButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (470, 416), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Title~Send")));
	signalConnectionManager.connect (sendButton->clicked, std::bind (&cWindowNetworkLobby::sendChatMessage, this, false));
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
	nameLineEdit->setText (player->getName());
	signalConnectionManager.connect (nameLineEdit->editingFinished, [&, nameLineEdit](eValidatorState){player->setName (nameLineEdit->getText ()); });

	playersList = addChild (std::make_unique<cListView<cLobbyPlayerListViewItem>> (cBox<cPosition> (getPosition () + cPosition (465, 284), getPosition () + cPosition (465 + 167, 284 + 124))));
	playersList->disableSelectable ();
	playersList->setBeginMargin (cPosition (12, 12));
	playersList->setEndMargin (cPosition (10, 10));
	playersList->setItemDistance (cPosition (0, 4));

	auto prevColorButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (478, 256), ePushButtonType::ArrowLeftSmall, SoundData.SNDObjectMenu));
	signalConnectionManager.connect (prevColorButton->clicked, [&]() { player->setToPrevColorIndex (); });
	auto nextColorButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (596, 256), ePushButtonType::ArrowRightSmall, SoundData.SNDObjectMenu));
	signalConnectionManager.connect (nextColorButton->clicked, [&]() { player->setToNextColorIndex (); });
	colorImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (505, 260)));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (50, 450), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, [&]() { close (); });

	updateSettingsText ();
	updateMap ();
	updatePlayerColor ();

	player->colorChanged.connect (std::bind (&cWindowNetworkLobby::updatePlayerColor, this));

	addPlayer (player);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::updateSettingsText ()
{
	std::string text = lngPack.i18n ("Text~Main~Version", PACKAGE_VERSION) + "\n";
	//text += "Checksum: " + iToStr (cSettings::getInstance().Checksum) + "\n\n";

	if (saveGameNumber != -1)
	{
		text += lngPack.i18n ("Text~Title~Savegame") + ":\n  " + saveGameName +  "\n\n" + lngPack.i18n ("Text~Title~Players") + "\n" + saveGamePlayers + "\n";
	}
	if (staticMap != nullptr)
	{
		text += lngPack.i18n ("Text~Title~Map") + ": " + staticMap->getName ();
		text += " (" + iToStr (staticMap->getSizeNew ().x ()) + "x" + iToStr (staticMap->getSizeNew ().y ()) + ")\n";
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
		mapImage->setImage (surface);
	}

	auto mapName = staticMap->getName ();
	const auto size = staticMap->getSizeNew ();

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
	SDL_BlitSurface (OtherData.colors[player->getColorIndex ()], &src, colorSurface, NULL);
	colorImage->setImage (colorSurface);
}

void cWindowNetworkLobby::sendChatMessage (bool refocusChatLine)
{
	if (!chatLineEdit->getText ().empty ())
	{
		addChatEntry (player->getName (), chatLineEdit->getText ());
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
	auto addedItem = chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message, chatList->getSize ().x () - chatList->getBeginMargin ().x () - chatList->getEndMargin ().x ()));
	chatList->scroolToItem (addedItem);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addInfoEntry (const std::string& message)
{
	auto addedItem = chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (message, chatList->getSize ().x () - chatList->getBeginMargin ().x () - chatList->getEndMargin ().x ()));
	chatList->scroolToItem (addedItem);
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::addPlayer (const std::shared_ptr<sPlayer>& player)
{
	playersList->addItem (std::make_unique<cLobbyPlayerListViewItem> (player, playersList->getSize ().x () - playersList->getBeginMargin ().x () - playersList->getEndMargin ().x ()));
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setStaticMap (std::shared_ptr<cStaticMap> staticMap_)
{
	staticMap = std::move (staticMap_);

	saveGameNumber = -1;

	updateMap ();
	updateSettingsText ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setGameSettings (std::unique_ptr<cGameSettings> gameSettings_)
{
	gameSettings = std::move (gameSettings_);

	saveGameNumber = -1;

	updateSettingsText ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::setSaveGame (int saveGameNumber_)
{
	saveGameNumber = saveGameNumber_;

	cSavegame saveGame(saveGameNumber_);

	saveGame.loadHeader (&saveGameName, nullptr, nullptr);
	saveGamePlayers = saveGame.getPlayerNames ();

	staticMap = std::make_shared<cStaticMap> ();
	if (!staticMap->loadMap (saveGame.getMapName ()))
	{
		// error dialog
		staticMap = nullptr;
	}

	updateMap ();
	updateSettingsText ();
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
const std::shared_ptr<sPlayer>& cWindowNetworkLobby::getLocalPlayer () const
{
	return player;
}

//------------------------------------------------------------------------------
std::vector<std::shared_ptr<sPlayer>> cWindowNetworkLobby::getPlayers () const
{
	std::vector<std::shared_ptr<sPlayer>> result;
	for (size_t i = 0; i < playersList->getItemsCount (); ++i)
	{
		const auto& item = playersList->getItem (i);
		result.push_back (item.getPlayer ());
	}
	return result;
}

//------------------------------------------------------------------------------
cTCP& cWindowNetworkLobby::getNetwork ()
{
	return *network;
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
void cWindowNetworkLobby::disablePortEdit ()
{
	portLineEdit->disable ();
}

//------------------------------------------------------------------------------
void cWindowNetworkLobby::disableIpEdit ()
{
	ipLineEdit->disable ();
}