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

#include <cassert>
#include <cmath>
#include <sstream>

#include "menus.h"

#include "autoptr.h"
#include "buildings.h"
#include "clans.h"
#include "client.h"
#include "clientevents.h"
#include "clist.h"
#include "dialog.h"
#include "events.h"
#include "files.h"
#include "hud.h"
#include "loaddata.h"
#include "log.h"
#include "mapdownload.h"
#include "menuevents.h"
#include "netmessage.h"
#include "pcx.h"
#include "player.h"
#include "savegame.h"
#include "server.h"
#include "serverevents.h"
#include "settings.h"
#include "vehicles.h"
#include "video.h"

#include "events/eventmanager.h"
#include "input/mouse/mouse.h"
#include "input/keyboard/keyboard.h"
#include "utility/scopedoperation.h"

using namespace std;

#define MAIN_MENU_BTN_SPACE 35

//------------------------------------------------------------------------------
void sSettings::pushInto (cNetMessage& message) const
{
	message.pushBool (hotseat);
	message.pushInt16 (iTurnDeadline);
	message.pushInt16 (duration);
	message.pushChar (victoryType);
	message.pushChar (gameType);
	message.pushChar (clans);
	message.pushChar (alienTech);
	message.pushChar (bridgeHead);
	message.pushInt16 (credits);
	message.pushChar (resFrequency);
	message.pushChar (gold);
	message.pushChar (oil);
	message.pushChar (metal);
}

//------------------------------------------------------------------------------
void sSettings::popFrom (cNetMessage& message)
{
	metal = (eSettingResourceValue) message.popChar();
	oil = (eSettingResourceValue) message.popChar();
	gold = (eSettingResourceValue) message.popChar();
	resFrequency = (eSettingResFrequency) message.popChar();
	credits = message.popInt16();
	bridgeHead = (eSettingsBridgeHead) message.popChar();
	alienTech = (eSettingsAlienTech) message.popChar();
	clans = (eSettingsClans) message.popChar();
	gameType = (eSettingsGameType) message.popChar();
	victoryType = (eSettingsVictoryType) message.popChar();
	duration = message.popInt16();
	iTurnDeadline = message.popInt16();
	hotseat = message.popBool();
}

//------------------------------------------------------------------------------
string sSettings::getResValString (eSettingResourceValue type) const
{
	switch (type)
	{
		case SETTING_RESVAL_LIMITED: return lngPack.i18n ("Text~Option~Limited");
		case SETTING_RESVAL_NORMAL: return lngPack.i18n ("Text~Option~Normal");
		case SETTING_RESVAL_HIGH: return lngPack.i18n ("Text~Option~High");
		case SETTING_RESVAL_TOOMUCH: return lngPack.i18n ("Text~Option~TooMuch");
	}
	return "";
}

//------------------------------------------------------------------------------
string sSettings::getResFreqString() const
{
	switch (resFrequency)
	{
		case SETTING_RESFREQ_SPARSE: return lngPack.i18n ("Text~Option~Sparse");
		case SETTING_RESFREQ_NORMAL: return lngPack.i18n ("Text~Option~Normal");
		case SETTING_RESFREQ_DENCE: return lngPack.i18n ("Text~Option~Dense");
		case SETTING_RESFREQ_TOOMUCH: return lngPack.i18n ("Text~Option~TooMuch");
	}
	return "";
}

std::string ToString (eLandingState state)
{
	switch (state)
	{
		case LANDING_POSITION_TOO_CLOSE: return "LANDING_POSITION_TOO_CLOSE";
		case LANDING_POSITION_WARNING: return "LANDING_POSITION_WARNING";
		case LANDING_POSITION_OK: return "LANDING_POSITION_OK";
		case LANDING_POSITION_CONFIRMED: return "LANDING_POSITION_COMFIRMED";
		case LANDING_STATE_UNKNOWN: return "LANDING_STATE_UNKNOWN";
	}
	assert (0);
	return "unknown";
}

//------------------------------------------------------------------------------
/*static*/ eLandingState sClientLandData::checkLandingState (std::vector<sClientLandData>& landData, unsigned int playerNr)
{
	int posX = landData[playerNr].iLandX;
	int posY = landData[playerNr].iLandY;
	int lastPosX = landData[playerNr].iLastLandX;
	int lastPosY = landData[playerNr].iLastLandY;
	bool bPositionTooClose = false;
	bool bPositionWarning = false;

	// check distances to all other players
	for (size_t i = 0; i != landData.size(); ++i)
	{
		const sClientLandData& c = landData[i];
		if (c.receivedOK == false) continue;
		if (i == playerNr) continue;

		const int sqDistance = Square (c.iLandX - posX) + Square (c.iLandY - posY);

		if (sqDistance < Square (LANDING_DISTANCE_TOO_CLOSE))
		{
			bPositionTooClose = true;
		}
		if (sqDistance < Square (LANDING_DISTANCE_WARNING))
		{
			bPositionWarning = true;
		}
	}

	// now set the new landing state,
	// depending on the last state, the last position, the current position,
	//  bPositionTooClose and bPositionWarning
	eLandingState lastState = landData[playerNr].landingState;
	eLandingState newState = LANDING_STATE_UNKNOWN;

	if (bPositionTooClose)
	{
		newState = LANDING_POSITION_TOO_CLOSE;
	}
	else if (bPositionWarning)
	{
		if (lastState == LANDING_POSITION_WARNING)
		{
			const int sqDelta = Square (posX - lastPosX) + Square (posY - lastPosY);
			if (sqDelta <= Square (LANDING_DISTANCE_TOO_CLOSE))
			{
				// the player has chosen the same position after a warning
				// so further warnings will be ignored
				newState = LANDING_POSITION_CONFIRMED;
			}
			else
			{
				newState = LANDING_POSITION_WARNING;
			}
		}
		else if (lastState == LANDING_POSITION_CONFIRMED)
		{
			// player is in state LANDING_POSITION_CONFIRMED,
			// so ignore the warning
			newState = LANDING_POSITION_CONFIRMED;
		}
		else
		{
			newState = LANDING_POSITION_WARNING;
		}
	}
	else
	{
		if (lastState == LANDING_POSITION_CONFIRMED)
		{
			newState = LANDING_POSITION_CONFIRMED;
		}
		else
		{
			newState = LANDING_POSITION_OK;
		}
	}

	landData[playerNr].landingState = newState;
	return newState;
}

//------------------------------------------------------------------------------
string sSettings::getVictoryConditionString() const
{
	string r = iToStr (duration) + " ";

	switch (victoryType)
	{
		case SETTINGS_VICTORY_TURNS: r += lngPack.i18n ("Text~Comp~Turns");  break;
		case SETTINGS_VICTORY_POINTS: r += lngPack.i18n ("Text~Comp~Points"); break;
		case SETTINGS_VICTORY_ANNIHILATION: return lngPack.i18n ("Text~Comp~NoLimit");
	}
	return r;
}

//------------------------------------------------------------------------------
// cMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cMenu::cMenu (SDL_Surface* background_, eMenuBackgrounds backgroundType_) :
	background (background_),
	backgroundType (backgroundType_),
	lastMousePosition(-1, -1)
{
	end = false;
	terminate = false;
	activeItem = NULL;

	lastScreenResX = Video.getResolutionX();
	lastScreenResY = Video.getResolutionY();

	recalcPosition (false);
}

//------------------------------------------------------------------------------
void cMenu::mouseMoved(cMouse& mouse)
{
	const auto& currentMousePos = mouse.getPosition();

	for(unsigned int i = 0; i < menuItems.size(); i++)
	{
		cMenuItem* menuItem = menuItems[i];

		if(menuItem->overItem(lastMousePosition.x(), lastMousePosition.y()) && !menuItem->overItem(currentMousePos.x(), currentMousePos.y())) menuItem->hoveredAway(this);
		else if(!menuItem->overItem(lastMousePosition.x(), lastMousePosition.y()) && menuItem->overItem(currentMousePos.x(), currentMousePos.y())) menuItem->hoveredOn(this);
		else if(menuItem->overItem(lastMousePosition.x(), lastMousePosition.y()) && menuItem->overItem(currentMousePos.x(), currentMousePos.y())) menuItem->movedMouseOver(lastMousePosition.x(), lastMousePosition.y(), this);
		else if(menuItem == activeItem) menuItem->somewhereMoved();
	}

	lastMousePosition = currentMousePos;
}

//------------------------------------------------------------------------------
void cMenu::mouseButtonPressed(cMouse& mouse, eMouseButtonType button)
{
	const auto& currentMousePos = mouse.getPosition();

	for(size_t i = 0; i != menuItems.size(); ++i)
	{
		cMenuItem* menuItem = menuItems[i];
		if(button == eMouseButtonType::Left && menuItem->overItem(currentMousePos.x(), currentMousePos.y()))
		{
			menuItem->clicked(this);
			if(activeItem) activeItem->setActivity(false);
			activeItem = menuItem;
			activeItem->setActivity(true);
			if(!end && !terminate) draw();
		}
		if(button == eMouseButtonType::Right && menuItem->overItem(currentMousePos.x(), currentMousePos.y()))
		{
			menuItem->rightClicked(this);
		}
	}
}

//------------------------------------------------------------------------------
void cMenu::mouseButtonReleased(cMouse& mouse, eMouseButtonType button)
{
	const auto& currentMousePos = mouse.getPosition();

	for(size_t i = 0; i != menuItems.size(); ++i)
	{
		cMenuItem* menuItem = menuItems[i];
		if(button == eMouseButtonType::Left)
		{
			if(menuItem->overItem(currentMousePos.x(), currentMousePos.y())) menuItem->released(this);
			else menuItem->somewhereReleased();
		}
	}
}

//------------------------------------------------------------------------------
cMenu::~cMenu()
{
}

//------------------------------------------------------------------------------
void cMenu::recalcPosition (bool resetItemPositions)
{
	SDL_Rect oldPosition = position;

	if (background)
	{
		position.w = background->w;
		position.h = background->h;
		position.x = (Video.getResolutionX() / 2 - position.w / 2);
		position.y = (Video.getResolutionY() / 2 - position.h / 2);
	}
	else
	{
		position.x = position.y = 0;
		position.w = Video.getResolutionX();
		position.h = Video.getResolutionY();
	}

	if (resetItemPositions == false) return;

	for (size_t i = 0; i != menuItems.size(); ++i)
	{
		const int xOffset = menuItems[i]->getPosition().x - oldPosition.x;
		const int yOffset = menuItems[i]->getPosition().y - oldPosition.y;

		menuItems[i]->move (position.x + xOffset, position.y + yOffset);
	}
}

//------------------------------------------------------------------------------
void cMenu::draw (bool firstDraw, bool showScreen)
{
	if (lastScreenResX != Video.getResolutionX() || lastScreenResY != Video.getResolutionY())
	{
		recalcPosition (true);
		lastScreenResX = Video.getResolutionX();
		lastScreenResY = Video.getResolutionY();
	}

	switch (backgroundType)
	{
		case MNU_BG_BLACK:
			// fill the whole screen with black to prevent
			// old garbage from menus that don't support resolutions > 640x480
			SDL_FillRect (cVideo::buffer, NULL, 0xFF000000);
			break;
		case MNU_BG_ALPHA:
			if (cSettings::getInstance().isAlphaEffects() && firstDraw)
				Video.applyShadow (NULL);
			break;
		case MNU_BG_TRANSPARENT:
			// do nothing here
			break;
	}

	preDrawFunction();

	// draw the menu background
	if (background) SDL_BlitSurface (background, NULL, cVideo::buffer, &position);

	// show mouse
	cMouse::getInstance().show();

	// draw all menu items
	for (size_t i = 0; i != menuItems.size(); ++i)
	{
		menuItems[i]->draw();
	}

	if (showScreen)
	{
		Video.draw();
	}
}

//------------------------------------------------------------------------------
int cMenu::switchTo(cMenu& other, cClient* client)
{
	deactivate();
	auto result = other.show(client);
	activate();

	return result;
}

//------------------------------------------------------------------------------
void cMenu::activate()
{
	using namespace std::placeholders;

	signalConnectionManager.connect(cMouse::getInstance().moved, std::bind(&cMenu::mouseMoved, this, _1));
	signalConnectionManager.connect(cMouse::getInstance().pressed, std::bind(&cMenu::mouseButtonPressed, this, _1, _2));
	signalConnectionManager.connect(cMouse::getInstance().released, std::bind(&cMenu::mouseButtonReleased, this, _1, _2));

	signalConnectionManager.connect(cKeyboard::getInstance().keyPressed, std::bind(&cMenu::keyPressed, this, _1, _2));
	signalConnectionManager.connect(cKeyboard::getInstance().textEntered, std::bind(&cMenu::textEntered, this, _1, _2));
}

//------------------------------------------------------------------------------
void cMenu::deactivate()
{
	signalConnectionManager.disconnectAll();
}

//------------------------------------------------------------------------------
int cMenu::show (cClient* client)
{
	activate();
	auto deactivator = makeScopedOperation(std::bind(&cMenu::deactivate, this));

	cMouse::getInstance().setCursorType(eMouseCursorType::Hand);
	draw (true);

	int lastResX = Video.getResolutionX();
	int lastResY = Video.getResolutionY();

	while (!exiting())
	{
		cEventManager::getInstance().run();
		if (client)
		{
			client->gameTimer.run (this);
			client->getGameGUI().handleTimer();
		}
		else
			handleNetMessages();

		// check whether the resolution has been changed
		if (!exiting() && (lastResX != Video.getResolutionX() || lastResY != Video.getResolutionY()))
		{
			draw (false, true);
			lastResX = Video.getResolutionX();
			lastResY = Video.getResolutionY();
		}

		// run timer callbacks
		for (size_t i = 0; i != menuTimers.size(); ++i)
			if (menuTimers[i]->getState()) menuTimers[i]->callback();

		SDL_Delay (1);
	}

	// flush event queue before exiting menu
	if(!client) handleNetMessages();
	cEventManager::getInstance().run();

	cMouse::getInstance().setCursorType(eMouseCursorType::Hand);
	if (end) return 0;
	assert (terminate);
	return 1;
}

//------------------------------------------------------------------------------
void cMenu::close()
{
	end = true;
}

//------------------------------------------------------------------------------
bool cMenu::exiting() const
{
	return end || terminate;
}

//------------------------------------------------------------------------------
void cMenu::keyPressed(cKeyboard& keyboard, SDL_Keycode key)
{
	if (activeItem) activeItem->handleKeyInput (keyboard, key, this);
}

//------------------------------------------------------------------------------
void cMenu::textEntered(cKeyboard& keyboard, const char* text)
{
	if(activeItem) activeItem->handleTextInputEvent(keyboard, text, this);
}

//------------------------------------------------------------------------------
void cMenu::sendMessage (cTCP& network, cNetMessage* message, const sPlayer* player, int fromPlayerNr)
{
	// Attention: The playernumber will only be the real player number
	// when it is passed to this function explicitly.
	// Otherwise it is only -1!
	message->iPlayerNr = fromPlayerNr;

	if (player == NULL) network.send (message->iLength, message->serialize());
	else network.sendTo (player->getSocketIndex(), message->iLength, message->serialize());

	Log.write ("Menu: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);
	delete message;
}

//------------------------------------------------------------------------------
void cMenu::addTimer (cMenuTimerBase* timer)
{
	menuTimers.push_back (timer);
}

//------------------------------------------------------------------------------
/*static*/ void cMenu::cancelReleased (void* parent)
{
	cMenu* menu = reinterpret_cast<cMenu*> (parent);
	menu->terminate = true;
}

//------------------------------------------------------------------------------
/*static*/ void cMenu::doneReleased (void* parent)
{
	cMenu* menu = reinterpret_cast<cMenu*> (parent);
	menu->end = true;
}

//------------------------------------------------------------------------------
// cMainMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cMainMenu::cMainMenu()
	: cMenu (LoadPCX (GFXOD_MAIN))
{
	infoImage = new cMenuImage (position.x + 16, position.y + 182, getRandomInfoImage());
	infoImage->setReleasedFunction (&infoImageReleased);
	infoImage->setClickSound (SoundData.SNDHudButton);
	menuItems.push_back (infoImage.get());

	creditsLabel = new cMenuLabel (position.x + position.w / 2, position.y + 465, lngPack.i18n ("Text~Main~Credits_Reloaded") + " " + PACKAGE_VERSION);
	creditsLabel->setCentered (true);
	menuItems.push_back (creditsLabel.get());
}

//------------------------------------------------------------------------------
SDL_Surface* cMainMenu::getRandomInfoImage()
{
	int const showBuilding = random (3);
	// I want 3 possible random numbers since a chance of 50:50 is boring
	// (and vehicles are way more cool so I prefer them to be shown) -- beko
	static int lastUnitShow = -1;
	int unitShow = -1;
	SDL_Surface* surface = NULL;

	if (showBuilding == 1 && UnitsData.getNrBuildings() > 0)
	{
		// that's a 33% chance that we show a building on 1
		do
		{
			unitShow = random (UnitsData.getNrBuildings() - 1);
			// make sure we don't show same unit twice
		}
		while (unitShow == lastUnitShow && UnitsData.getNrBuildings() > 1);
		surface = UnitsData.buildingUIs[unitShow].info;
	}
	else if (UnitsData.getNrVehicles() > 0)
	{
		// and a 66% chance to show a vehicle on 0 or 2
		do
		{
			unitShow = random (UnitsData.getNrVehicles() - 1);
			// make sure we don't show same unit twice
		}
		while (unitShow == lastUnitShow && UnitsData.getNrVehicles() > 1);
		surface = UnitsData.vehicleUIs[unitShow].info;
	}
	else surface = NULL;
	lastUnitShow = unitShow; //store shown unit
	return surface;
}

//------------------------------------------------------------------------------
void cMainMenu::infoImageReleased (void* parent)
{
	cMainMenu* menu = reinterpret_cast<cMainMenu*> (parent);
	// get a new random info image
	SDL_Surface* surface = menu->getRandomInfoImage();
	// draw the new image
	menu->infoImage->setImage (surface);
	menu->infoImage->draw();
	Video.draw();
}

//------------------------------------------------------------------------------
// cStartMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cStartMenu::cStartMenu()
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 147, lngPack.i18n ("Text~Title~MainMenu"), FONT_LATIN_NORMAL);
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	singleButton = new cMenuButton (position.x + 390, position.y + 190, lngPack.i18n ("Text~Others~Single_Player"));
	singleButton->setReleasedFunction (&singlePlayerReleased);
	menuItems.push_back (singleButton.get());

	multiButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE, lngPack.i18n ("Text~Others~Multi_Player"));
	multiButton->setReleasedFunction (&multiPlayerReleased);
	menuItems.push_back (multiButton.get());

	preferenceButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE * 2, lngPack.i18n ("Text~Settings~Preferences"));
	preferenceButton->setReleasedFunction (&preferencesReleased);
	menuItems.push_back (preferenceButton.get());

	licenceButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE * 3, lngPack.i18n ("Text~Others~Mani"));
	licenceButton->setReleasedFunction (&licenceReleased);
	menuItems.push_back (licenceButton.get());

	exitButton = new cMenuButton (position.x + 415, position.y + 190 + MAIN_MENU_BTN_SPACE * 6, lngPack.i18n ("Text~Others~Exit"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL, FONT_LATIN_BIG, SoundData.SNDMenuButton);
	exitButton->setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (exitButton.get());

	PlayMusic ((cSettings::getInstance().getMusicPath() + PATH_DELIMITER + "main.ogg").c_str());
}

//------------------------------------------------------------------------------
void cStartMenu::singlePlayerReleased (void* parent)
{
	cStartMenu* menu = reinterpret_cast<cStartMenu*> (parent);
	cSinglePlayerMenu singlePlayerMenu;
	menu->switchTo(singlePlayerMenu);
	menu->draw();
}

//------------------------------------------------------------------------------
void cStartMenu::multiPlayerReleased (void* parent)
{
	cStartMenu* menu = reinterpret_cast<cStartMenu*> (parent);
	cMultiPlayersMenu multiPlayerMenu;
	menu->switchTo(multiPlayerMenu);
	menu->draw();
}

//------------------------------------------------------------------------------
void cStartMenu::preferencesReleased (void* parent)
{
	cStartMenu* menu = reinterpret_cast<cStartMenu*> (parent);
	cPlayer* activePlayer = 0;
	cDialogPreferences preferencesDialog(activePlayer);
	menu->switchTo(preferencesDialog);
	menu->draw();
}

//------------------------------------------------------------------------------
void cStartMenu::licenceReleased (void* parent)
{
	cStartMenu* menu = reinterpret_cast<cStartMenu*> (parent);
	cDialogLicence licenceDialog;
	menu->switchTo(licenceDialog);
	menu->draw();
}

//------------------------------------------------------------------------------
// cSinglePlayerMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cSinglePlayerMenu::cSinglePlayerMenu()
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 147, lngPack.i18n ("Text~Others~Single_Player"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	newGameButton = new cMenuButton (position.x + 390, position.y + 190, lngPack.i18n ("Text~Others~Game_New"));
	newGameButton->setReleasedFunction (&newGameReleased);
	menuItems.push_back (newGameButton.get());

	loadGameButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE, lngPack.i18n ("Text~Others~Game_Load"));
	loadGameButton->setReleasedFunction (&loadGameReleased);
	menuItems.push_back (loadGameButton.get());

	backButton = new cMenuButton (position.x + 415, position.y + 190 + MAIN_MENU_BTN_SPACE * 6, lngPack.i18n ("Text~Others~Back"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (backButton.get());
}

namespace
{

int settingSelection (cMenu& parent, sSettings& settings)
{
	cSettingsMenu settingsMenu (settings);

	if (parent.switchTo(settingsMenu) == 1) return -1;
	settings = settingsMenu.getSettings();
	return 1;
}

int planetSelection (cMenu& parent, std::shared_ptr<cStaticMap>& map)
{
	cPlanetsSelectionMenu planetSelectionMenu;

	if(parent.switchTo(planetSelectionMenu) == 1) return -1;
	map = planetSelectionMenu.getStaticMap ();
	return 1;
}

int clanSelection (cMenu& parent, cClient& client, bool noReturn)
{
	if (client.getGameSetting()->clans != SETTING_CLANS_ON
		|| client.getGameSetting()->hotseat) return 0;

	cPlayer& player = client.getActivePlayer();
	cClanSelectionMenu clanMenu (player.getClan(), noReturn);
	if (parent.switchTo(clanMenu) == 1)
	{
		player.setClan (-1);
		return -1;
	}
	player.setClan (clanMenu.getClan());
	return 1;
}

int landingUnitsSelection (cMenu& parent, cClient& client,
						   std::vector<sLandingUnit>& landingUnits,
						   bool noReturn)
{
	cStartupHangarMenu startupHangarMenu (client, landingUnits, noReturn);

	if (parent.switchTo(startupHangarMenu) == 1)
	{
		return -1;
	}
	sendClan (client);
	sendLandingUnits (client, landingUnits);
	sendUnitUpgrades (client);
	return 1;
}

int landingPosSelection (cMenu& parent, cClient& client, cStaticMap& map, sClientLandData& landingData)
{
	cLandingMenu landingMenu (client, map, landingData);
	if(parent.switchTo(landingMenu) == 1) return -1;
	return 1;
}

//------------------------------------------------------------------------------
int runGamePreparation (cMenu& parent, cClient& client, cStaticMap& map, sClientLandData& landingData, bool noReturn)
{
	const sSettings& settings = *client.getGameSetting();
	std::vector<sLandingUnit> landingUnits;
	int step = 0;
	int lastDir = 1;
	while (true)
	{
		int dir = 0;
		switch (step)
		{
			case -1: return -1;
			case 0: dir = clanSelection(parent, client, noReturn); break;
			case 1: dir = landingUnitsSelection(parent, client, landingUnits, noReturn && settings.clans == SETTING_CLANS_OFF); break;
			case 2: dir = landingPosSelection(parent, client, map, landingData); break;
			case 3: return 1;
			default: break;
		}
		step += dir;
		if (dir == 0) step += lastDir;
		else lastDir = dir;
	}
}

int runGamePreparation (cMenu& parent, cClient& client, cStaticMap& map, bool noReturn)
{
	sClientLandData landingData;

	return runGamePreparation (parent, client, map, landingData, noReturn);
}

}

//------------------------------------------------------------------------------
void cSinglePlayerMenu::newGameReleased (void* parent)
{
	cSinglePlayerMenu* menu = reinterpret_cast<cSinglePlayerMenu*> (parent);
	cTCP* const network = NULL;
	sSettings settings;
	sPlayer splayer (cSettings::getInstance().getPlayerName(), cl_red, 0);
	splayer.setLocal();
	cServer server (network);
	cEventHandling eventHandling;
	cClient client (&server, network, eventHandling);
	std::shared_ptr<cStaticMap> map;
	std::vector<sPlayer*> players;
	sPlayer clientPlayer (splayer);

	players.push_back (&clientPlayer);
	client.setPlayers (players, clientPlayer);

	int lastDir = 1;
	int step = 0;
	while (true)
	{
		int dir = 0;
		switch (step)
		{
			case -1: menu->draw(); return;
			case 0: dir = settingSelection (*menu, settings); break;
			case 1:
			{
				dir = planetSelection (*menu, map);
				if (dir == 1)
				{
					server.setMap (map);
					server.setGameSettings (settings);
					server.addPlayer (new cPlayer (splayer));
					server.changeStateToInitGame();
					client.setMap (map);
					client.setGameSetting (settings);
				}
				break;
			}
			case 2:
			{
				dir = runGamePreparation (*menu, client, *map, false);
				if (dir == -1 || (dir == 0 && lastDir != -1))
				{
					server.cancelStateInitGame();
				}
				break;
			}
			case 3:
			{
				menu->switchTo(client.getGameGUI(), &client);
				server.stop();
				menu->draw();
				return;
			}
			default: break;
		}
		step += dir;
		if (dir == 0) step += lastDir;
		else lastDir = dir;
	}
}

//------------------------------------------------------------------------------
void cSinglePlayerMenu::loadGameReleased (void* parent)
{
	cSinglePlayerMenu* menu = reinterpret_cast<cSinglePlayerMenu*> (parent);
	int savegameNum = -1;
	{
		cLoadMenu loadMenu;

		if (menu->switchTo(loadMenu) == 1)
		{
			menu->draw();
			return;
		}
		savegameNum = loadMenu.getLoadingSlotNumber();
	}
	menu->runSavedGame (savegameNum);
	menu->draw();
}

//------------------------------------------------------------------------------
void cSinglePlayerMenu::runSavedGame (int savegameNum)
{
	cTCP* network = NULL;
	cServer server (network);
	cSavegame savegame (savegameNum);
	if (savegame.load (server) == false) return;
	const std::vector<cPlayer*>& serverPlayerList = server.PlayerList;

	if (serverPlayerList.empty()) return;
	const int player = 0;

	// Following may be simplified according to serverGame::loadGame
	std::vector<sPlayer*> clientPlayerList;

	// copy players for client
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		const cPlayer& p = *serverPlayerList[i];
		clientPlayerList.push_back (new sPlayer (p.getName(), p.getColor(), p.getNr(), p.getSocketNum()));
	}
	// init client and his player
	cEventHandling eventHandler;
	cClient client (&server, network, eventHandler);
	client.setMap (server.Map->staticMap);
	client.setPlayers (clientPlayerList, *clientPlayerList[player]);

	for (size_t i = 0; i != clientPlayerList.size(); ++i)
	{
		delete clientPlayerList[i];
	}
	clientPlayerList.clear();


	// in singleplayer only the first player is important
	serverPlayerList[player]->setLocal();
	sendRequestResync (client, serverPlayerList[player]->getNr());

	// TODO: move that in server
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		sendHudSettings (server, *serverPlayerList[i]);
		std::vector<sSavedReportMessage>& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size(); ++j)
		{
			sendSavedReport (server, reportList[j], serverPlayerList[i]->getNr());
		}
		reportList.clear();
	}

	// start game
	server.serverState = SERVER_STATE_INGAME;
	switchTo(client.getGameGUI(), &client);

	server.stop();

	reloadUnitValues();
}

//------------------------------------------------------------------------------
// cMultiPlayersMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cMultiPlayersMenu::cMultiPlayersMenu()
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 147, lngPack.i18n ("Text~Others~Multi_Player"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	tcpHostButton = new cMenuButton (position.x + 390, position.y + 190, lngPack.i18n ("Text~Others~TCPIP_Host"));
	tcpHostButton->setReleasedFunction (&tcpHostReleased);
	menuItems.push_back (tcpHostButton.get());

	tcpClientButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE, lngPack.i18n ("Text~Others~TCPIP_Client"));
	tcpClientButton->setReleasedFunction (&tcpClientReleased);
	menuItems.push_back (tcpClientButton.get());

#ifndef RELEASE
	newHotseatButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE * 2, lngPack.i18n ("Text~Others~HotSeat_New"));
	newHotseatButton->setReleasedFunction (&newHotseatReleased);
	//newHotseatButton->setLocked(true); //disable, not implemented yet
	menuItems.push_back (newHotseatButton.get());

	loadHotseatButton = new cMenuButton (position.x + 390, position.y + 190 + MAIN_MENU_BTN_SPACE * 3, lngPack.i18n ("Text~Others~HotSeat_Load"));
	loadHotseatButton->setReleasedFunction (&loadHotseatReleased);
	//loadHotseatButton->setLocked(true); //disable, not implemented yet
	menuItems.push_back (loadHotseatButton.get());
#endif

	backButton = new cMenuButton (position.x + 415, position.y + 190 + MAIN_MENU_BTN_SPACE * 6, lngPack.i18n ("Text~Others~Back"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (backButton.get());
}

//------------------------------------------------------------------------------
void cMultiPlayersMenu::tcpHostReleased (void* parent)
{
	cMultiPlayersMenu* menu = reinterpret_cast<cMultiPlayersMenu*> (parent);
	cNetworkHostMenu networkMenu;
	if (menu->switchTo(networkMenu) == 1)
	{
		menu->draw();
		return;
	}
	menu->end = true;
}

//------------------------------------------------------------------------------
void cMultiPlayersMenu::tcpClientReleased (void* parent)
{
	cMultiPlayersMenu* menu = reinterpret_cast<cMultiPlayersMenu*> (parent);
	cNetworkClientMenu networkMenu;
	if (menu->switchTo(networkMenu) == 1)
	{
		menu->draw();
		return;
	}
	menu->end = true;
}

//------------------------------------------------------------------------------

static std::vector<sPlayer*> buildPlayerList (const cHotSeatMenu& hotSeatMenu, std::vector<int>& clans)
{
	std::vector<sPlayer*> res;
	const char* const playerNames[] = {
		"Player 1", "Player 2", "Player 3", "Player 4",
		"Player 5", "Player 6", "Player 7", "Player 8"
	};

	int nr = 0;
	for (int i = 0; i != 4; ++i)
	{
		if (hotSeatMenu.getPlayerType (i) == PLAYERTYPE_NONE) continue;
		res.push_back (new sPlayer (lngPack.i18n (playerNames[i]), i, nr++));
		res.back()->setLocal();
		clans.push_back (hotSeatMenu.getClan (i));
		// TODO return PlayerType for AI
	}
	return res;
}

//------------------------------------------------------------------------------
static int RunHostGamePreparation (cMenu& parent, std::vector<cClient*>& clients, cStaticMap& map)
{
	std::vector<sClientLandData> landingData (clients.size());
	for (size_t i = 0, size = clients.size(); i != size; ++i)
	{
		Video.clearBuffer();
		const std::string& name = clients[i]->getActivePlayer().getName();
		cDialogOK okDialog (lngPack.i18n ("Text~Multiplayer~Player_Turn", name));
		okDialog.show (NULL);
		const int dir = runGamePreparation (parent, *clients[i], map, landingData[i], true);
		if (dir == -1) return -1;
	}
	bool allOk = false;
	while (!allOk)
	{
		allOk = true;
		for (size_t i = 0, size = clients.size(); i != size; ++i)
		{
			cLandingMenu landingMenu (*clients[i], map, landingData[i]);

			landingMenu.handleNetMessages();
			if (landingData[i].landingState == LANDING_POSITION_OK
				|| landingData[i].landingState == LANDING_POSITION_CONFIRMED)
			{
				continue;
			}
			allOk = false;
			Video.clearBuffer();
			const std::string& name = clients[i]->getActivePlayer().getName();
			cDialogOK okDialog (lngPack.i18n ("Text~Multiplayer~Player_Turn", name));
			okDialog.show (NULL);
			if (landingMenu.show (clients[i]) == 1) return -1;
		}
	}
	return 1;
}

//------------------------------------------------------------------------------
void cMultiPlayersMenu::newHotseatReleased (void* parent)
{
	cMultiPlayersMenu* menu = reinterpret_cast<cMultiPlayersMenu*> (parent);

	// TODO: implement it
	cDialogOK okDialog (lngPack.i18n ("Text~Error_Messages~INFO_Not_Implemented"));
	menu->switchTo(okDialog);

	sSettings settings;
	std::shared_ptr<cStaticMap> map;
	cTCP* const network = NULL;
	cServer server (network);
	cEventHandling eventHandling;
	std::vector<cClient*> clients;

	settings.hotseat = true;

	int lastDir = 1;
	int step = 0;
	while (true)
	{
		int dir = 0;
		switch (step)
		{
			case -1: menu->draw(); return;
			case 0: dir = settingSelection (*menu, settings); break;
			case 1: dir = planetSelection (*menu, map); break;
			case 2:
			{
				cHotSeatMenu hotSeatMenu (settings);
				if(menu->switchTo(hotSeatMenu) == 1) dir = -1;
				else dir = 1;

				if (dir == 1)
				{
					server.setMap (map);
					server.setGameSettings (settings);
					std::vector<int> clans;
					std::vector<sPlayer*> splayers = buildPlayerList (hotSeatMenu, clans);
					for (size_t i = 0, size = splayers.size(); i != size; ++i)
					{
						server.addPlayer (new cPlayer (*splayers[i]));
					}
					// Create clients.
					for (size_t i = 0, size = splayers.size(); i != size; ++i)
					{
						cClient* client = new cClient (&server, network, eventHandling);
						clients.push_back (client);
						client->setPlayers(splayers, *splayers[i]);
						client->getActivePlayer().setClan (clans[i]);
						client->setMap (map);
						client->setGameSetting (settings);
					}
					for (size_t i = 0, size = splayers.size(); i != size; ++i)
						delete splayers[i];
					server.changeStateToInitGame();
				}
				break;
			}
#if 1
			case 3:
			{
				dir = RunHostGamePreparation (*menu, clients, *map);
				if (dir == -1)
				{
					// Cancel Server, clients
					server.cancelStateInitGame();
					for (size_t i = 0, size = clients.size(); i != size; ++i)
					{
						delete clients[i];
					}
					clients.clear();
				}
				break;
			}
			case 4:
			{
				// while game is not finished
				{
					for(size_t i = 0, size = clients.size(); i != size; ++i)
					{
						menu->switchTo(clients[i]->getGameGUI(), clients[i]);
					}
				}
				server.stop();
				for (size_t i = 0, size = clients.size(); i != size; ++i)
				{
					delete clients[i];
				}
				menu->draw();
				return;
			}
#endif
			default: break;
		}
		step += dir;
		if (dir == 0) step += lastDir;
		else lastDir = dir;
	}
}

//------------------------------------------------------------------------------
void cMultiPlayersMenu::loadHotseatReleased (void* parent)
{
	cMultiPlayersMenu* menu = reinterpret_cast<cMultiPlayersMenu*> (parent);
	// TODO: implement it
	cDialogOK okDialog (lngPack.i18n ("Text~Error_Messages~INFO_Not_Implemented"));
	menu->switchTo(okDialog);
	menu->draw();
}

//------------------------------------------------------------------------------
// cSettingsMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cSettingsMenu::cSettingsMenu (const sSettings& settings_) :
	cMenu (LoadPCX (GFXOD_OPTIONS)),
	settings (settings_)
{
	int iCurrentLine = 57;
	int iLineHeight = 16; //pixels after we start a new line
	// black window screen on gfx is 510 width.
	// calculation for most option fields starts at px 240x. and is 347 width.

	// Title
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 13, lngPack.i18n ("Text~Others~Game_Options"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	// OK button
	okButton = new cMenuButton (position.x + 390, position.y + 440, lngPack.i18n ("Text~Others~OK"));
	okButton->setReleasedFunction (&okReleased);
	menuItems.push_back (okButton.get());

	// Back button
	backButton = new cMenuButton (position.x + 50, position.y + 440, lngPack.i18n ("Text~Others~Back"));
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (backButton.get());

	// Resources field
	metalLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Metal") + ":");
	menuItems.push_back (metalLabel.get());
	metalGroup = new cMenuRadioGroup();
	metalGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Limited"), settings.metal == SETTING_RESVAL_LIMITED, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	metalGroup->addButton (new cMenuCheckButton (position.x + 240 + 86, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Normal"), settings.metal == SETTING_RESVAL_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	metalGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 2, position.y + iCurrentLine, lngPack.i18n ("Text~Option~High"), settings.metal == SETTING_RESVAL_HIGH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	metalGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 3, position.y + iCurrentLine, lngPack.i18n ("Text~Option~TooMuch"), settings.metal == SETTING_RESVAL_TOOMUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (metalGroup.get());
	iCurrentLine += iLineHeight;

	oilLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Oil") + ":");
	menuItems.push_back (oilLabel.get());
	oilGroup = new cMenuRadioGroup();
	oilGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Limited"), settings.oil == SETTING_RESVAL_LIMITED, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	oilGroup->addButton (new cMenuCheckButton (position.x + 240 + 86, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Normal"), settings.oil == SETTING_RESVAL_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	oilGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 2, position.y + iCurrentLine, lngPack.i18n ("Text~Option~High"), settings.oil == SETTING_RESVAL_HIGH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	oilGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 3, position.y + iCurrentLine, lngPack.i18n ("Text~Option~TooMuch"), settings.oil == SETTING_RESVAL_TOOMUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (oilGroup.get());
	iCurrentLine += iLineHeight;

	goldLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Gold") + ":");
	menuItems.push_back (goldLabel.get());
	goldGroup = new cMenuRadioGroup();
	goldGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Limited"), settings.gold == SETTING_RESVAL_LIMITED, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	goldGroup->addButton (new cMenuCheckButton (position.x + 240 + 86, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Normal"), settings.gold == SETTING_RESVAL_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	goldGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 2, position.y + iCurrentLine, lngPack.i18n ("Text~Option~High"), settings.gold == SETTING_RESVAL_HIGH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	goldGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 3, position.y + iCurrentLine, lngPack.i18n ("Text~Option~TooMuch"), settings.gold == SETTING_RESVAL_TOOMUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (goldGroup.get());
	iCurrentLine += iLineHeight;

	// Resource frequency field
	resFrequencyLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Resource_Density") + ":");
	menuItems.push_back (resFrequencyLabel.get());

	resFrequencyGroup = new cMenuRadioGroup();
	resFrequencyGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Sparse"), settings.resFrequency == SETTING_RESFREQ_SPARSE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	resFrequencyGroup->addButton (new cMenuCheckButton (position.x + 240 + 86, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Normal"), settings.resFrequency == SETTING_RESFREQ_NORMAL, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	resFrequencyGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 2, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Dense"), settings.resFrequency == SETTING_RESFREQ_DENCE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	resFrequencyGroup->addButton (new cMenuCheckButton (position.x + 240 + 86 * 3, position.y + iCurrentLine, lngPack.i18n ("Text~Option~TooMuch"), settings.resFrequency == SETTING_RESFREQ_TOOMUCH, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (resFrequencyGroup.get());
	iCurrentLine += iLineHeight * 3;

	// Bridgehead field
	bridgeheadLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~BridgeHead") + ":");
	menuItems.push_back (bridgeheadLabel.get());

	bridgeheadGroup = new cMenuRadioGroup();
	bridgeheadGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Mobile"), settings.bridgeHead == SETTING_BRIDGEHEAD_MOBILE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	bridgeheadGroup->addButton (new cMenuCheckButton (position.x + 240 + 173, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Definite"), settings.bridgeHead == SETTING_BRIDGEHEAD_DEFINITE, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (bridgeheadGroup.get());
	iCurrentLine += iLineHeight;

	// Game type field
	if (!settings_.hotseat)
	{
		gameTypeLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Game_Type") + ":");
		menuItems.push_back (gameTypeLabel.get());

		gameTypeGroup = new cMenuRadioGroup();
		gameTypeGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Type_Turns"), settings.gameType == SETTINGS_GAMETYPE_TURNS, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
		gameTypeGroup->addButton (new cMenuCheckButton (position.x + 240 + 173, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Type_Simu"), settings.gameType == SETTINGS_GAMETYPE_SIMU, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
		menuItems.push_back (gameTypeGroup.get());
	}
	iCurrentLine += iLineHeight * 3;

	// Other options (AlienTechs and Clans):
#if 0
	//alien stuff disabled until we reimplement this proper
	// -- beko Fri Jun 12 20:48:59 CEST 2009
	alienTechLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Alien_Tech") + ":");
	menuItems.push_back (alienTechLabel.get());
	aliensGroup = new cMenuRadioGroup();
	aliensGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~On"), settings.alienTech == SETTING_ALIENTECH_ON, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	aliensGroup->addButton (new cMenuCheckButton (position.x + 240 + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Off"), settings.alienTech == SETTING_ALIENTECH_OFF, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (aliensGroup.get());
	iCurrentLine += iLineHeight;
#endif

	clansLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Clans") + ":");
	menuItems.push_back (clansLabel.get());
	clansGroup = new cMenuRadioGroup();
	clansGroup->addButton (new cMenuCheckButton (position.x + 240, position.y + iCurrentLine, lngPack.i18n ("Text~Option~On"), settings.clans == SETTING_CLANS_ON, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	clansGroup->addButton (new cMenuCheckButton (position.x + 240 + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Option~Off"), settings.clans == SETTING_CLANS_OFF, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
	menuItems.push_back (clansGroup.get());
	iCurrentLine += iLineHeight * 3;

	int tmpLine = iCurrentLine;

	// Credits field - this is where the money goes
	creditsLabel = new cMenuLabel (position.x + 64, position.y + iCurrentLine, lngPack.i18n ("Text~Title~Credits_start") + ":");
	menuItems.push_back (creditsLabel.get());
	iCurrentLine += iLineHeight;
	creditsGroup = new cMenuRadioGroup();
	struct
	{
		const char* text;
		unsigned int value;
	} creditData[] =
	{
		{"Text~Option~None", SETTING_CREDITS_NONE},
		{"Text~Option~Low", SETTING_CREDITS_LOW},
		{"Text~Option~Limited", SETTING_CREDITS_LIMITED},
		{"Text~Option~Normal", SETTING_CREDITS_NORMAL},
		{"Text~Option~High", SETTING_CREDITS_HIGH},
		{"Text~Option~More", SETTING_CREDITS_MORE}
	};

	for (int i = 0; i != sizeof (creditData) / sizeof (*creditData); ++i)
	{
		const unsigned int x = position.x + 140;
		const unsigned int y = position.y + iCurrentLine;
		const unsigned int value = creditData[i].value;
		const std::string text = lngPack.i18n (creditData[i].text) + " (" + iToStr (value) + ")";
		const bool checked = settings.credits == value;
		creditsGroup->addButton (new cMenuCheckButton (x, y, text, checked, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY));
		iCurrentLine += iLineHeight;
	}
	menuItems.push_back (creditsGroup.get());

	iCurrentLine = tmpLine;

	// Victory condition
	const bool bTurns = settings.victoryType == SETTINGS_VICTORY_TURNS;
	const bool bPoints = settings.victoryType == SETTINGS_VICTORY_POINTS;
	const bool bAnnih = settings.victoryType == SETTINGS_VICTORY_ANNIHILATION;

	const bool bShort = settings.duration == SETTINGS_DUR_SHORT;
	const bool bMedi = settings.duration == SETTINGS_DUR_MEDIUM;
	const bool bLong = settings.duration == SETTINGS_DUR_LONG;

	const std::string strTurns = lngPack.i18n ("Text~Comp~Turns");
	const std::string strPoints = lngPack.i18n ("Text~Comp~Points");
	const std::string strNoLimit = lngPack.i18n ("Text~Comp~NoLimit");

	victoryLabel = new cMenuLabel (position.x + 300, position.y + iCurrentLine, lngPack.i18n ("Text~Comp~GameEndsAt"));
	menuItems.push_back (victoryLabel.get());

	tmpLine = iCurrentLine += iLineHeight;

	victoryGroup = new cMenuRadioGroup();
	victoryGroup->addButton (new cMenuCheckButton (position.x + 380, position.y + iCurrentLine, "100 " + strTurns, bTurns && bShort, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;
	victoryGroup->addButton (new cMenuCheckButton (position.x + 380, position.y + iCurrentLine, "200 " + strTurns, bTurns && bMedi,  true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;
	victoryGroup->addButton (new cMenuCheckButton (position.x + 380, position.y + iCurrentLine, "400 " + strTurns, bTurns && bLong,  true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;

	iCurrentLine = tmpLine;

	victoryGroup->addButton (new cMenuCheckButton (position.x + 500, position.y + iCurrentLine, "100 " + strPoints, bPoints && bShort, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;
	victoryGroup->addButton (new cMenuCheckButton (position.x + 500, position.y + iCurrentLine, "200 " + strPoints, bPoints && bMedi,  true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;
	victoryGroup->addButton (new cMenuCheckButton (position.x + 500, position.y + iCurrentLine, "400 " + strPoints, bPoints && bLong,  true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;

	victoryGroup->addButton (new cMenuCheckButton (position.x + 440, position.y + iCurrentLine, strNoLimit, bAnnih, true, cMenuCheckButton::RADIOBTN_TYPE_TEXT_ONLY)); iCurrentLine += iLineHeight;
	menuItems.push_back (victoryGroup.get());
}

//------------------------------------------------------------------------------
void cSettingsMenu::okReleased (void* parent)
{
	cSettingsMenu* menu = reinterpret_cast<cSettingsMenu*> (parent);
	menu->updateSettings();
	menu->end = true;
}

//------------------------------------------------------------------------------
void cSettingsMenu::updateSettings()
{
	if (metalGroup->buttonIsChecked (0)) settings.metal = SETTING_RESVAL_LIMITED;
	else if (metalGroup->buttonIsChecked (1)) settings.metal = SETTING_RESVAL_NORMAL;
	else if (metalGroup->buttonIsChecked (2)) settings.metal = SETTING_RESVAL_HIGH;
	else settings.metal = SETTING_RESVAL_TOOMUCH;

	if (oilGroup->buttonIsChecked (0)) settings.oil = SETTING_RESVAL_LIMITED;
	else if (oilGroup->buttonIsChecked (1)) settings.oil = SETTING_RESVAL_NORMAL;
	else if (oilGroup->buttonIsChecked (2)) settings.oil = SETTING_RESVAL_HIGH;
	else settings.oil = SETTING_RESVAL_TOOMUCH;

	if (goldGroup->buttonIsChecked (0)) settings.gold = SETTING_RESVAL_LIMITED;
	else if (goldGroup->buttonIsChecked (1)) settings.gold = SETTING_RESVAL_NORMAL;
	else if (goldGroup->buttonIsChecked (2)) settings.gold = SETTING_RESVAL_HIGH;
	else settings.gold = SETTING_RESVAL_TOOMUCH;

	if (resFrequencyGroup->buttonIsChecked (0)) settings.resFrequency = SETTING_RESFREQ_SPARSE;
	else if (resFrequencyGroup->buttonIsChecked (1)) settings.resFrequency = SETTING_RESFREQ_NORMAL;
	else if (resFrequencyGroup->buttonIsChecked (2)) settings.resFrequency = SETTING_RESFREQ_DENCE;
	else settings.resFrequency = SETTING_RESFREQ_TOOMUCH;

	if (creditsGroup->buttonIsChecked (0)) settings.credits = SETTING_CREDITS_NONE;
	else if (creditsGroup->buttonIsChecked (1)) settings.credits = SETTING_CREDITS_LOW;
	else if (creditsGroup->buttonIsChecked (2)) settings.credits = SETTING_CREDITS_LIMITED;
	else if (creditsGroup->buttonIsChecked (3)) settings.credits = SETTING_CREDITS_NORMAL;
	else if (creditsGroup->buttonIsChecked (4)) settings.credits = SETTING_CREDITS_HIGH;
	else if (creditsGroup->buttonIsChecked (5)) settings.credits = SETTING_CREDITS_MORE;
	else settings.credits = SETTING_CREDITS_NORMAL;

	if (bridgeheadGroup->buttonIsChecked (0)) settings.bridgeHead = SETTING_BRIDGEHEAD_MOBILE;
	else settings.bridgeHead = SETTING_BRIDGEHEAD_DEFINITE;

#if 0
	// alien stuff disabled until we reimplement this proper
	// -- beko Fri Jun 12 20:48:59 CEST 2009
	if (aliensGroup->buttonIsChecked (0)) settings.alienTech = SETTING_ALIENTECH_ON;
	else settings.alienTech = SETTING_ALIENTECH_OFF;
#endif
	settings.alienTech = SETTING_ALIENTECH_OFF;

	if (clansGroup->buttonIsChecked (0)) settings.clans = SETTING_CLANS_ON;
	else settings.clans = SETTING_CLANS_OFF;

	if (settings.hotseat) settings.gameType = SETTINGS_GAMETYPE_SIMU;
	else if (gameTypeGroup->buttonIsChecked (0)) settings.gameType = SETTINGS_GAMETYPE_TURNS;
	else settings.gameType = SETTINGS_GAMETYPE_SIMU;

	if (victoryGroup->buttonIsChecked (6))
		settings.victoryType = SETTINGS_VICTORY_ANNIHILATION;
	else
	{
		for (int i = 0; i < 6; i++)
		{
			if (victoryGroup->buttonIsChecked (i) == false) continue;

			if (i < 3)
				settings.victoryType = SETTINGS_VICTORY_TURNS;
			else
			{
				settings.victoryType = SETTINGS_VICTORY_POINTS;
				i -= 3;
			}
			if (i == 0) settings.duration = SETTINGS_DUR_SHORT;
			else if (i == 1) settings.duration = SETTINGS_DUR_MEDIUM;
			else if (i == 2) settings.duration = SETTINGS_DUR_LONG;
			break;
		}
	}
	// TODO:
	// turnLimit/ScoreLimit may be handed filled
	// some others parameters are missing (max time of a turn, ...)
}

//------------------------------------------------------------------------------
// cPlanetsSelectionMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cPlanetsSelectionMenu::cPlanetsSelectionMenu() :
	cMenu (LoadPCX (GFXOD_PLANET_SELECT))
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 11, lngPack.i18n ("Text~Title~Choose_Planet"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	okButton = new cMenuButton (position.x + 390, position.y + 440, lngPack.i18n ("Text~Others~OK"));
	okButton->setReleasedFunction (&okReleased);
	okButton->setLocked (true);
	menuItems.push_back (okButton.get());

	backButton = new cMenuButton (position.x + 50, position.y + 440, lngPack.i18n ("Text~Others~Back"));
	backButton->setReleasedFunction (&backReleased);
	menuItems.push_back (backButton.get());

	arrowUpButton = new cMenuButton (position.x + 292, position.y + 435, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG);
	arrowUpButton->setReleasedFunction (&arrowUpReleased);
	menuItems.push_back (arrowUpButton.get());

	arrowDownButton = new cMenuButton (position.x + 321, position.y + 435, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG);
	arrowDownButton->setReleasedFunction (&arrowDownReleased);
	menuItems.push_back (arrowDownButton.get());

	int index = 0;
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			planetImages[index] = new cMenuImage (position.x + 21 + 158 * x, position.y + 86 + 198 * y);
			planetImages[index]->setReleasedFunction (&mapReleased);
			planetImages[index]->setClickSound (SoundData.SNDHudButton);
			menuItems.push_back (planetImages[index].get());

			planetTitles[index] = new cMenuLabel (position.x + 77 + 158 * x, position.y + 48 + 198 * y);
			planetTitles[index]->setCentered (true);
			menuItems.push_back (planetTitles[index].get());

			index++;
		}
	}

	selectedMapIndex = -1;
	offset = 0;

	loadMaps();
	showMaps();
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::loadMaps()
{
	maps = getFilesOfDirectory (cSettings::getInstance().getMapsPath());
	if (!getUserMapsDir().empty())
	{
		std::vector<std::string> userMaps (getFilesOfDirectory (getUserMapsDir()));
		for (size_t i = 0; i != userMaps.size(); ++i)
		{
			if (!Contains (maps, userMaps[i]))
				maps.push_back (userMaps[i]);
		}
	}
	for (size_t i = 0; i != maps.size(); ++i)
	{
		const string& map = maps[i];
		if (map.compare (map.length() - 3, 3, "WRL") != 0
			&& map.compare (map.length() - 3, 3, "wrl") != 0)
		{
			maps.erase (maps.begin() + i);
			i--;
		}
	}
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::showMaps()
{
	for (int i = 0; i < 8; i++) //only 8 maps on one screen
	{
		if (i + offset < (int) maps.size())
		{
			string mapName = maps[i + offset];

			int size;
			AutoSurface mapSurface (cStaticMap::loadMapPreview (mapName, &size));

			if (mapSurface == NULL) continue;

			const int MAPWINSIZE = 112;
			const int SELECTED = 0x00C000;
			const int UNSELECTED = 0x000000;
			AutoSurface imageSurface (SDL_CreateRGBSurface (0, MAPWINSIZE + 8, MAPWINSIZE + 8, Video.getColDepth(), 0, 0, 0, 0));

			if (selectedMapIndex == i + offset)
			{
				SDL_FillRect (imageSurface, NULL, SELECTED);

				if (font->getTextWide (">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
				{
					while (font->getTextWide (">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
					{
						mapName.erase (mapName.length() - 1, mapName.length());
					}
					mapName = ">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<";
				}
				else mapName = ">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<";
			}
			else
			{
				SDL_FillRect (imageSurface, NULL, UNSELECTED);

				if (font->getTextWide (">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
				{
					while (font->getTextWide (">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
					{
						mapName.erase (mapName.length() - 1, mapName.length());
					}
					mapName = mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")";
				}
				else mapName = mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")";
			}
			SDL_Rect dest = { 4, 4, MAPWINSIZE, MAPWINSIZE };
			SDL_BlitSurface (mapSurface, NULL, imageSurface, &dest);

			planetImages[i]->setImage (imageSurface);
			planetTitles[i]->setText (mapName);
		}
		else
		{
			planetImages[i]->setImage (NULL);
			planetTitles[i]->setText ("");
		}
	}
	draw();
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::backReleased (void* parent)
{
	cPlanetsSelectionMenu* menu = reinterpret_cast<cPlanetsSelectionMenu*> (parent);
	menu->staticMap = NULL;
	menu->terminate = true;
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::okReleased (void* parent)
{
	cPlanetsSelectionMenu* menu = reinterpret_cast<cPlanetsSelectionMenu*> (parent);
	if (static_cast<unsigned int> (menu->selectedMapIndex) >= menu->maps.size()) return;

	menu->staticMap = std::make_shared<cStaticMap> ();
	menu->staticMap->loadMap (menu->maps[menu->selectedMapIndex]);

	menu->end = true;
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::arrowDownReleased (void* parent)
{
	cPlanetsSelectionMenu* menu = reinterpret_cast<cPlanetsSelectionMenu*> (parent);
	if (menu->offset + 8 < (int) menu->maps.size())
	{
		menu->offset += 8;
		menu->showMaps();
	}
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::arrowUpReleased (void* parent)
{
	cPlanetsSelectionMenu* menu = reinterpret_cast<cPlanetsSelectionMenu*> (parent);
	if (menu->offset - 8 >= 0)
	{
		menu->offset -= 8;
		menu->showMaps();
	}
}

//------------------------------------------------------------------------------
void cPlanetsSelectionMenu::mapReleased (void* parent)
{
	cPlanetsSelectionMenu* menu = reinterpret_cast<cPlanetsSelectionMenu*> (parent);
	int index = 0;
	const auto& mousePosition = cMouse::getInstance().getPosition();
	if(mousePosition.x() > menu->position.x + 160) index++;
	if(mousePosition.x() > menu->position.x + 320) index++;
	if(mousePosition.x() > menu->position.x + 470) index++;

	if(mousePosition.y() > menu->position.y + 240) index += 4;

	index += menu->offset;

	menu->selectedMapIndex = index;
	menu->showMaps();
	menu->okButton->setLocked (false);
	menu->draw();
}

//------------------------------------------------------------------------------
// cClanSelectionMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cClanSelectionMenu::cClanSelectionMenu (int clan_, bool noReturn) :
	cMenu (LoadPCX (GFXOD_CLAN_SELECT)),
	clan (std::max (0, clan_))
{
	okButton = new cMenuButton (position.x + 390, position.y + 440, lngPack.i18n ("Text~Others~OK"));
	okButton->setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (okButton.get());

	backButton = new cMenuButton (position.x + 50, position.y + 440, lngPack.i18n ("Text~Others~Back"));
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	if (noReturn) backButton->setLocked (true);
	menuItems.push_back (backButton.get());

	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 13, lngPack.i18n ("Text~Title~Choose_Clan"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	vector<string> clanLogoPaths;
	const std::string gfxPath = cSettings::getInstance().getGfxPath() + PATH_DELIMITER;
	clanLogoPaths.push_back (gfxPath + "clanlogo1.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo2.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo3.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo4.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo5.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo6.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo7.pcx");
	clanLogoPaths.push_back (gfxPath + "clanlogo8.pcx");

	int xCount = 0;
	int yCount = 0;
	for (int i = 0; i < 8; i++, xCount++)
	{
		if (i == 4)
		{
			xCount = 0;
			yCount = 1;
		}
		SDL_Surface* img = LoadPCX (clanLogoPaths[i].c_str());
		SDL_SetColorKey (img, SDL_TRUE, 0xFF00FF);
		clanImages[i] = new cMenuImage (position.x + 88 + xCount * 154 - (img ? (img->w / 2) : 0), position.y + 48 + yCount * 150, img);
		clanImages[i]->setReleasedFunction (&clanSelected);
		menuItems.push_back (clanImages[i].get());

		clanNames[i] = new cMenuLabel (position.x + 87 + xCount * 154, position.y + 144 + yCount * 150, cClanData::instance().getClan (i)->getName());
		clanNames[i]->setCentered (true);
		menuItems.push_back (clanNames[i].get());
	}
	clanNames[clan]->setText (">" + cClanData::instance().getClan (clan)->getName() + "<");

	clanDescription1 = new cMenuLabel (position.x + 47, position.y + 362, "");
	menuItems.push_back (clanDescription1.get());
	clanDescription2 = new cMenuLabel (position.x + 380, position.y + 362, "");
	menuItems.push_back (clanDescription2.get());
	clanShortDescription = new cMenuLabel (position.x + 47, position.y + 349, "");
	menuItems.push_back (clanShortDescription.get());
	updateClanDescription();
}

//------------------------------------------------------------------------------
void cClanSelectionMenu::clanSelected (void* parent)
{
	cClanSelectionMenu* menu = reinterpret_cast<cClanSelectionMenu*> (parent);

	const auto& mousePosition = cMouse::getInstance().getPosition();
	int newClan = (mousePosition.x() - menu->position.x - 47) / 154;
	if(mousePosition.y() > menu->position.y + 48 + 140)
		newClan += 4;

	if (0 <= newClan && newClan < 8 && newClan != menu->clan)
	{
		menu->clanNames[menu->clan]->setText (cClanData::instance().getClan (menu->clan)->getName());
		menu->clan = newClan;
		menu->clanNames[menu->clan]->setText (">" + cClanData::instance().getClan (menu->clan)->getName() + "<");
		menu->updateClanDescription();
		menu->draw();
	}
}

//------------------------------------------------------------------------------
void cClanSelectionMenu::updateClanDescription()
{
	cClan* clanInfo = cClanData::instance().getClan (clan);
	if (clanInfo)
	{
		vector<string> strings = clanInfo->getClanStatsDescription();

		string desc1;
		for (size_t i = 0; i < 4 && i < strings.size(); ++i)
		{
			desc1.append (strings[i]);
			desc1.append ("\n");
		}
		clanDescription1->setText (desc1);

		string desc2;
		for (size_t i = 4; i < strings.size(); ++i)
		{
			desc2.append (strings[i]);
			desc2.append ("\n");
		}
		clanDescription2->setText (desc2);

		clanShortDescription->setText (clanInfo->getDescription());
	}
	else
	{
		clanDescription1->setText ("Unknown");
		clanDescription1->setText ("");
	}
}

//------------------------------------------------------------------------------
// cHangarMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cHangarMenu::cHangarMenu (SDL_Surface* background_, cPlayer* player_, eMenuBackgrounds backgroundType_) :
	cMenu (background_, backgroundType_),
	player (player_)
{
	selectionChangedFunc = NULL;

	doneButton = new cMenuButton (position.x + 447, position.y + 452, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	menuItems.push_back (doneButton.get());

	backButton = new cMenuButton (position.x + 349, position.y + 452, lngPack.i18n ("Text~Others~Back"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	menuItems.push_back (backButton.get());

	infoImage = new cMenuImage (position.x + 11, position.y + 13);
	menuItems.push_back (infoImage.get());

	infoText = new cMenuLabel (position.x + 21, position.y + 23);
	infoText->setBox (280, 220);
	menuItems.push_back (infoText.get());

	infoTextCheckBox = new cMenuCheckButton (position.x + 291, position.y + 264, lngPack.i18n ("Text~Comp~Description"), true, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD, cMenuCheckButton::TEXT_ORIENT_LEFT);
	infoTextCheckBox->setClickedFunction (&infoCheckBoxClicked);
	menuItems.push_back (infoTextCheckBox.get());

	unitDetails = new cMenuUnitDetailsBig (position.x + 16, position.y + 297);
	menuItems.push_back (unitDetails.get());

	selectionList = new cMenuUnitsList (position.x + 477,  position.y + 50, 154, 326, this, MUL_DIS_TYPE_COSTS);
	menuItems.push_back (selectionList.get());

	selListUpButton = new cMenuButton (position.x + 471, position.y + 387, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	selListUpButton->setReleasedFunction (&selListUpReleased);
	menuItems.push_back (selListUpButton.get());

	selListDownButton = new cMenuButton (position.x + 491, position.y + 387, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	selListDownButton->setReleasedFunction (&selListDownReleased);
	menuItems.push_back (selListDownButton.get());

	selectedUnit = NULL;
}

//------------------------------------------------------------------------------
void cHangarMenu::addItem (cMenuItem* menuItem)
{
	menuItems.push_back (menuItem);
}

//------------------------------------------------------------------------------
void cHangarMenu::drawUnitInformation()
{
	if (!selectedUnit) return;
	if (selectedUnit->getUnitID().isAVehicle())
	{
		const sVehicleUIData& uiData = *UnitsData.getVehicleUI (selectedUnit->getUnitID());

		infoImage->setImage (uiData.info);
	}
	else if (selectedUnit->getUnitID().isABuilding())
	{
		const sBuildingUIData& uiData = *UnitsData.getBuildingUI (selectedUnit->getUnitID());

		infoImage->setImage (uiData.info);
	}
	if (infoTextCheckBox->isChecked())
	{
		const string description = selectedUnit->getUnitID().getUnitDataOriginalVersion()->description;
		infoText->setText (description);
	}
	else infoText->setText ("");
}

//------------------------------------------------------------------------------
void cHangarMenu::infoCheckBoxClicked (void* parent)
{
	cHangarMenu* menu = reinterpret_cast<cHangarMenu*> (parent);
	menu->drawUnitInformation();
	menu->draw();
}

//------------------------------------------------------------------------------
void cHangarMenu::selListUpReleased (void* parent)
{
	cHangarMenu* menu = reinterpret_cast<cHangarMenu*> (parent);
	menu->selectionList->scrollUp();
}

//------------------------------------------------------------------------------
void cHangarMenu::selListDownReleased (void* parent)
{
	cHangarMenu* menu = reinterpret_cast<cHangarMenu*> (parent);
	menu->selectionList->scrollDown();
}

//------------------------------------------------------------------------------
void cHangarMenu::setSelectedUnit (cMenuUnitListItem* selectedUnit_)
{
	selectedUnit = selectedUnit_;
	unitDetails->setSelection (selectedUnit);
	if (selectionChangedFunc) selectionChangedFunc (this);
	drawUnitInformation();
}

//------------------------------------------------------------------------------
cMenuUnitListItem* cHangarMenu::getSelectedUnit()
{
	return selectedUnit;
}

//------------------------------------------------------------------------------
// cAdvListHangarMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cAdvListHangarMenu::cAdvListHangarMenu (SDL_Surface* background_, cPlayer* player_, eMenuBackgrounds backgroundType_) :
	cHangarMenu (background_, player_, backgroundType_)
{
	secondList = new cMenuUnitsList (position.x + 330, position.y + 12, 130, 225, this, MUL_DIS_TYPE_CARGO);
	secondList->setDoubleClickedFunction (&secondListDoubleClicked);
	menuItems.push_back (secondList.get());

	selectionList->setDoubleClickedFunction (&selListDoubleClicked);

	secondListUpButton = new cMenuButton (position.x + 327, position.y + 240, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	secondListUpButton->setReleasedFunction (&secondListUpReleased);
	menuItems.push_back (secondListUpButton.get());

	secondListDownButton = new cMenuButton (position.x + 348, position.y + 240, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	secondListDownButton->setReleasedFunction (&secondListDownReleased);
	menuItems.push_back (secondListDownButton.get());
}

//------------------------------------------------------------------------------
void cAdvListHangarMenu::secondListUpReleased (void* parent)
{
	cAdvListHangarMenu* menu = dynamic_cast<cAdvListHangarMenu*> ((cMenu*) parent);
	if (!menu) return;
	menu->secondList->scrollUp();
}

//------------------------------------------------------------------------------
void cAdvListHangarMenu::secondListDownReleased (void* parent)
{
	cAdvListHangarMenu* menu = dynamic_cast<cAdvListHangarMenu*> ((cMenu*) parent);
	if (!menu) return;
	menu->secondList->scrollDown();
}

//------------------------------------------------------------------------------
bool cAdvListHangarMenu::selListDoubleClicked (cMenuUnitsList* list, void* parent)
{
	cAdvListHangarMenu* menu = dynamic_cast<cAdvListHangarMenu*> ((cHangarMenu*) parent);
	if (!menu || menu->selectedUnit == NULL) return false;
	if (menu->selectedUnit != menu->selectionList->getSelectedUnit()) return false;

	const sID& id = menu->selectedUnit->getUnitID();
	if (id.isAVehicle() && menu->checkAddOk (menu->selectedUnit))
	{
		if (menu->selectedUnit->getUpgrades()) menu->secondList->addUnit (id, menu->player, menu->selectedUnit->getUpgrades(), true, menu->selectedUnit->getFixedResValue());
		else menu->secondList->addUnit (menu->player->getUnitDataCurrentVersion (id), menu->player, NULL, true);
		menu->secondList->getItem (menu->secondList->getSize() - 1)->setResValue (menu->selectedUnit->getResValue(), false);
		menu->addedCallback (menu->selectedUnit);
		menu->draw();
	}
	return true;
}

//------------------------------------------------------------------------------
bool cAdvListHangarMenu::secondListDoubleClicked (cMenuUnitsList* list, void* parent)
{
	cAdvListHangarMenu* menu = dynamic_cast<cAdvListHangarMenu*> ((cHangarMenu*) parent);
	if (!menu || menu->selectedUnit == NULL) return false;
	if (menu->selectedUnit != menu->secondList->getSelectedUnit()) return false;
	if (menu->selectedUnit->getFixedStatus()) return false;

	menu->removedCallback (menu->selectedUnit);
	menu->secondList->removeUnit (menu->selectedUnit);
	if (menu->selectedUnit == NULL && menu->selectionList->getSelectedUnit()) menu->setSelectedUnit (menu->selectionList->getSelectedUnit());
	menu->draw();
	return true;
}

//------------------------------------------------------------------------------
// cStartupHangarMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cStartupHangarMenu::cStartupHangarMenu (cClient& client_,
										std::vector<sLandingUnit>& landingUnits_,
										bool noReturn) :
	cAdvListHangarMenu (LoadPCX (GFXOD_HANGAR), &client_.getActivePlayer()),
	gameSetting (client_.getGameSetting()),
	client (&client_),
	landingUnits (&landingUnits_),
	upgradeHangarContainer (this, &client_.getActivePlayer()),
	chooseUnitLabel (position.x + 552, position.y + 11, lngPack.i18n ("Text~Title~Choose_Units"))
{
	chooseUnitLabel.setCentered (true);
	menuItems.push_back (&chooseUnitLabel);

	const int credits = gameSetting->credits;

	selectionChangedFunc = &selectionChanged;

	doneButton->setReleasedFunction (&doneReleased);
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	if (noReturn) backButton->setLocked (true);

	upgradeBuyGroup = new cMenuRadioGroup();
	upgradeBuyGroup->addButton (new cMenuCheckButton (position.x + 542, position.y + 445, lngPack.i18n ("Text~Others~Buy"), true, false, cMenuCheckButton::RADIOBTN_TYPE_BTN_ROUND));
	upgradeBuyGroup->addButton (new cMenuCheckButton (position.x + 542, position.y + 445 + 17, lngPack.i18n ("Text~Others~Upgrade"), false, false, cMenuCheckButton::RADIOBTN_TYPE_BTN_ROUND));
	upgradeBuyGroup->setClickedFunction (&subButtonsChanged);
	menuItems.push_back (upgradeBuyGroup.get());

	materialBar = new cMenuMaterialBar (position.x + 421, position.y + 301, position.x + 430, position.y + 275, 0, cMenuMaterialBar::MAT_BAR_TYPE_METAL);
	materialBar->setClickedFunction (materialBarClicked);
	menuItems.push_back (materialBar.get());
	materialBarLabel = new cMenuLabel (position.x + 430, position.y + 285, lngPack.i18n ("Text~Title~Cargo"));
	materialBarLabel->setCentered (true);
	menuItems.push_back (materialBarLabel.get());

	materialBarUpButton = new cMenuButton (position.x + 413, position.y + 424, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	materialBarUpButton->setReleasedFunction (&materialBarUpReleased);
	menuItems.push_back (materialBarUpButton.get());

	materialBarDownButton = new cMenuButton (position.x + 433, position.y + 424, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	materialBarDownButton->setReleasedFunction (&materialBarDownReleased);
	menuItems.push_back (materialBarDownButton.get());

	generateSelectionList();

	upgradeHangarContainer.getGoldBar().setMaximalValue (credits);
	upgradeHangarContainer.getGoldBar().setCurrentValue (credits);
	generateInitialLandingUnits();
	addPlayerLandingUnits (*player);

	upgradeHangarContainer.computePurchased (*player);

	if (selectionList->getSize() > 0) setSelectedUnit (selectionList->getItem (0));
}

class UnitWithSameIdMoreCargo
{
public:
	UnitWithSameIdMoreCargo (sID id, int cargo) : id (id), cargo (cargo) {}
	bool operator() (const sLandingUnit& it) const
	{
		return it.unitID == id && it.cargo >= cargo;
	}
private:
	sID id;
	int cargo;
};

void cStartupHangarMenu::addPlayerLandingUnits (cPlayer& player)
{
	std::vector<sLandingUnit>& units = *landingUnits;

	if (units.empty()) return;
	int credits = upgradeHangarContainer.getGoldBar().getCurrentValue();
	std::vector<sLandingUnit>::iterator it;
	for (int i = 0; i != secondList->getSize(); ++i)
	{
		cMenuUnitListItem& unit = *secondList->getItem (i);
		UnitWithSameIdMoreCargo filter (unit.getUnitID(), unit.getResValue());
		it = std::find_if (units.begin(), units.end(), filter);
		if (it == units.end()) continue;

		credits -= (it->cargo - unit.getResValue()) / 5;
		unit.setResValue (it->cargo);
		units.erase (it);
	}

	for (size_t i = 0; i != units.size(); ++i)
	{
		cMenuUnitListItem* selectedUnit = selectionList->getItemByID (units[i].unitID);
		cMenuUnitListItem* unit = secondList->addUnit (units[i].unitID, &player, selectedUnit->getUpgrades());

		credits -= unit->getUnitID().getUnitDataOriginalVersion (&player)->buildCosts;
		credits -= units[i].cargo / 5;
		unit->setResValue (units[i].cargo);
	}
	upgradeHangarContainer.getGoldBar().setCurrentValue (credits);
}

void cStartupHangarMenu::generateInitialLandingUnits()
{
	if (gameSetting->bridgeHead != SETTING_BRIDGEHEAD_DEFINITE) return;

	const sID& constructorID = UnitsData.getConstructorID();
	const sID& engineerID = UnitsData.getEngineerID();
	const sID& surveyorID = UnitsData.getSurveyorID();

	cUnitUpgrade* constructorUpgrades = selectionList->getItemByID (constructorID)->getUpgrades();
	cUnitUpgrade* engineerUpgrades = selectionList->getItemByID (engineerID)->getUpgrades();
	cUnitUpgrade* surveyorUpgrades = selectionList->getItemByID (surveyorID)->getUpgrades();

	cMenuUnitListItem* constructor = secondList->addUnit (constructorID, player, constructorUpgrades);
	constructor->setMinResValue (40);
	constructor->setFixed (true);

	cMenuUnitListItem* engineer = secondList->addUnit (engineerID, player, engineerUpgrades);
	engineer->setMinResValue (20);
	engineer->setFixed (true);

	cMenuUnitListItem* surveyor = secondList->addUnit (surveyorID, player, surveyorUpgrades);
	surveyor->setFixed (true);

	if (gameSetting->clans != SETTING_CLANS_ON || player->getClan() != 7) return;
	// Additional Units for Axis Inc. Clan

	const int startCredits = gameSetting->credits;
	int numAddConstructors = 0;
	int numAddEngineers = 0;
	if (startCredits < 100)
		numAddEngineers = 1;
	else if (startCredits < 150)
	{
		numAddEngineers = 1;
		numAddConstructors = 1;
	}
	else if (startCredits < 200)
	{
		numAddEngineers = 2;
		numAddConstructors = 1;
	}
	else if (startCredits < 300)
	{
		numAddEngineers = 2;
		numAddConstructors = 2;
	}
	else
	{
		numAddEngineers = 3;
		numAddConstructors = 2;
	}

	for (int i = 0; i != numAddConstructors; ++i)
	{
		cMenuUnitListItem* constructor = secondList->addUnit (constructorID, player, constructorUpgrades);
		constructor->setFixed (true);
	}
	for (int i = 0; i != numAddEngineers; ++i)
	{
		cMenuUnitListItem* engineer = secondList->addUnit (engineerID, player, engineerUpgrades);
		engineer->setFixed (true);
	}
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::updateUnitData()
{
	for (unsigned int i = 0; i < UnitsData.getNrVehicles() + UnitsData.getNrBuildings(); i++)
	{
		const cUnitUpgrade& unitUpgrade = upgradeHangarContainer.getUnitUpgrade (i);
		sUnitData* data;
		if (i < UnitsData.getNrVehicles()) data = &player->VehicleData[i];
		else data = &player->BuildingData[i - UnitsData.getNrVehicles()];

		unitUpgrade.updateUnitData (*data);
	}
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::doneReleased (void* parent)
{
	cStartupHangarMenu* menu = dynamic_cast<cStartupHangarMenu*> ((cMenu*) parent);
	if (!menu) return;

	menu->updateUnitData();

	for (int i = 0; i < menu->secondList->getSize(); i++)
	{
		sLandingUnit landingUnit;
		landingUnit.unitID = menu->secondList->getItem (i)->getUnitID();
		landingUnit.cargo = menu->secondList->getItem (i)->getResValue();
		menu->landingUnits->push_back (landingUnit);
	}
	menu->end = true;
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::subButtonsChanged (void* parent)
{
	cStartupHangarMenu* menu = dynamic_cast<cStartupHangarMenu*> ((cMenu*) parent);
	if (!menu) return;
	menu->generateSelectionList();
	menu->draw();
}

void cStartupHangarMenu::materialBarUpReleased (void* parent)
{
	cStartupHangarMenu* menu = dynamic_cast<cStartupHangarMenu*> ((cMenu*) parent);
	if (!menu) return;
	cMenuUnitListItem* unit = menu->secondList->getSelectedUnit();
	if (!unit) return;
	const sUnitData* vehicle = unit->getUnitID().getUnitDataOriginalVersion (menu->player);
	cMenuMaterialBar& goldBar = menu->upgradeHangarContainer.getGoldBar();
	if (goldBar.getCurrentValue() == 0 || vehicle->storeResType == sUnitData::STORE_RES_GOLD) return;

	const int oldCargo = unit->getResValue();
	unit->setResValue (oldCargo + 5);
	menu->materialBar->setCurrentValue (unit->getResValue());
	if (oldCargo != unit->getResValue())
	{
		goldBar.increaseCurrentValue (-1);
	}
	menu->upgradeHangarContainer.getUpgradeButtons().setSelection (menu->selectedUnit);
	menu->draw();
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::materialBarDownReleased (void* parent)
{
	cStartupHangarMenu* menu = dynamic_cast<cStartupHangarMenu*> ((cMenu*) parent);
	if (!menu) return;
	cMenuUnitListItem* unit = menu->secondList->getSelectedUnit();
	if (!unit) return;
	const sUnitData* vehicle = unit->getUnitID().getUnitDataOriginalVersion (menu->player);
	if (vehicle->storeResType == sUnitData::STORE_RES_GOLD) return;

	const int oldCargo = unit->getResValue();
	unit->setResValue (oldCargo - 5);
	menu->materialBar->setCurrentValue (unit->getResValue());
	if (oldCargo != unit->getResValue())
	{
		menu->upgradeHangarContainer.getGoldBar().increaseCurrentValue (1);
	}
	menu->upgradeHangarContainer.getUpgradeButtons().setSelection (menu->selectedUnit);
	menu->draw();
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::materialBarClicked (void* parent)
{
	cStartupHangarMenu* menu = dynamic_cast<cStartupHangarMenu*> ((cMenu*) parent);
	if (!menu) return;
	cMenuUnitListItem* unit = menu->secondList->getSelectedUnit();
	if (!unit) return;
	const sUnitData* vehicle = unit->getUnitID().getUnitDataOriginalVersion (menu->player);
	if (vehicle->storeResType == sUnitData::STORE_RES_GOLD) return;

	const int oldCargo = unit->getResValue();
	int newCargo = (int)((float)(menu->position.y + 301 + 115 - cMouse::getInstance().getPosition().y()) / 115 * vehicle->storageResMax);
	if (newCargo % 5 < 3) newCargo -= newCargo % 5;
	else newCargo += 5 - newCargo % 5;

	unit->setResValue (newCargo);

	newCargo = unit->getResValue();
	int costs = (newCargo - oldCargo) / 5;
	if (costs > menu->upgradeHangarContainer.getGoldBar().getCurrentValue())
	{
		costs = menu->upgradeHangarContainer.getGoldBar().getCurrentValue();
		newCargo = costs * 5 + oldCargo;
	}

	unit->setResValue (newCargo);
	menu->materialBar->setCurrentValue (unit->getResValue());

	menu->upgradeHangarContainer.getGoldBar().increaseCurrentValue (-costs);
	menu->upgradeHangarContainer.getUpgradeButtons().setSelection (menu->selectedUnit);
	menu->draw();
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::generateSelectionList()
{
	sID oldSelectdUnit;
	if (selectionList->getSelectedUnit()) oldSelectdUnit = selectionList->getSelectedUnit()->getUnitID();

	selectionList->clear();
	const bool buy = upgradeBuyGroup->buttonIsChecked (0);
	const bool tank = upgradeHangarContainer.getUpgradeFilter().TankIsChecked();
	const bool plane = upgradeHangarContainer.getUpgradeFilter().PlaneIsChecked() && !buy;
	const bool ship = upgradeHangarContainer.getUpgradeFilter().ShipIsChecked() && !buy;
	const bool build = upgradeHangarContainer.getUpgradeFilter().BuildingIsChecked() && !buy;
	const bool tnt = upgradeHangarContainer.getUpgradeFilter().TNTIsChecked();

	if (tank || ship || plane)
	{
		for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
		{
			const sUnitData& data = UnitsData.getVehicle (i, player->getClan());
			if (data.isHuman && buy) continue;
			if (tnt && !data.canAttack) continue;
			if (data.factorAir > 0 && !plane) continue;
			if (data.factorSea > 0 && data.factorGround == 0 && !ship) continue;
			if (data.factorGround > 0 && !tank) continue;
			selectionList->addUnit (data.ID, player, &upgradeHangarContainer.getUnitUpgrade (i));
		}
	}

	if (build)
	{
		for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
		{
			const sUnitData& data = UnitsData.getBuilding (i, player->getClan());
			if (tnt && !data.canAttack) continue;
			selectionList->addUnit (data.ID, player, &upgradeHangarContainer.getUnitUpgrade (UnitsData.getNrVehicles() + i));
		}
	}

	for (int i = 0; i < selectionList->getSize(); i++)
	{
		if (oldSelectdUnit == selectionList->getItem (i)->getUnitID())
		{
			selectionList->setSelection (selectionList->getItem (i));
			break;
		}
	}
	if (selectionList->getSelectedUnit() == NULL && selectionList->getSize() > 0) selectionList->setSelection (selectionList->getItem (0));
}

//------------------------------------------------------------------------------
bool cStartupHangarMenu::isInLandingList (const cMenuUnitListItem* item) const
{
	for (int i = 0; i < secondList->getSize(); i++)
	{
		if (secondList->getItem (i) == item) return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cStartupHangarMenu::checkAddOk (const cMenuUnitListItem* item) const
{
	const sUnitData* data = item->getUnitID().getUnitDataOriginalVersion (player);
	if (!data || item->getUnitID().isAVehicle() == false) return false;

	if (data->factorGround == 0) return false;
	if (data->isHuman) return false;
	if (data->buildCosts > upgradeHangarContainer.getGoldBar().getCurrentValue()) return false;
	return true;
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::addedCallback (cMenuUnitListItem* item)
{
	const sUnitData* data = item->getUnitID().getUnitDataOriginalVersion (player);
	if (!data || item->getUnitID().isAVehicle() == false) return;

	upgradeHangarContainer.getGoldBar().increaseCurrentValue (-data->buildCosts);
	selectionChanged (this);
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::removedCallback (cMenuUnitListItem* item)
{
	const sUnitData* data = item->getUnitID().getUnitDataOriginalVersion (player);
	if (!data || item->getUnitID().isAVehicle() == false) return;

	upgradeHangarContainer.getGoldBar().increaseCurrentValue (data->buildCosts + item->getResValue() / 5);
}

//------------------------------------------------------------------------------
void cStartupHangarMenu::selectionChanged (void* parent)
{
	cStartupHangarMenu* menu;
	menu = dynamic_cast<cStartupHangarMenu*> ((cHangarMenu*) parent);
	if (!menu) menu = dynamic_cast<cStartupHangarMenu*> ((cStartupHangarMenu*) parent);
	if (!menu) return;
	const cMenuUnitListItem* unit = menu->secondList->getSelectedUnit();
	const sUnitData* vehicle = (unit && unit->getUnitID().isAVehicle()) ? unit->getUnitID().getUnitDataOriginalVersion (menu->player) : 0;
	if (vehicle && vehicle->storeResType != sUnitData::STORE_RES_NONE && vehicle->storeResType != sUnitData::STORE_RES_GOLD)
	{
		menu->materialBar->setMaximalValue (vehicle->storageResMax);
		menu->materialBar->setCurrentValue (unit->getResValue());

		cMenuMaterialBar::eMaterialBarTypes type = cMenuMaterialBar::MAT_BAR_TYPE_NONE_HORI_BIG;
		switch (vehicle->storeResType)
		{
			case sUnitData::STORE_RES_METAL: type = cMenuMaterialBar::MAT_BAR_TYPE_METAL; break;
			case sUnitData::STORE_RES_OIL: type = cMenuMaterialBar::MAT_BAR_TYPE_OIL; break;
			case sUnitData::STORE_RES_GOLD: //type = cMenuMaterialBar::MAT_BAR_TYPE_GOLD; break; // Its not allowed to store gold bought with gold. Why should the user be able to buy gold with... gold?!
			case sUnitData::STORE_RES_NONE: break;
		}
		menu->materialBar->setType (type);
	}
	else
	{
		menu->materialBar->setMaximalValue (0);
		menu->materialBar->setCurrentValue (0);
	}
	menu->upgradeHangarContainer.getUpgradeButtons().setSelection (menu->selectedUnit);
	menu->draw();
}

//------------------------------------------------------------------------------
// cLandingMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cLandingMenu::cLandingMenu (cClient& client_, cStaticMap& map_, sClientLandData& landData_) :
	cMenu (NULL),
	client (&client_),
	map (&map_),
	landData (&landData_),
	canClick (true)
{
	createMap();
	mapImage = new cMenuImage (180, 18, mapSurface);
	mapImage->setClickedFunction (&mapClicked);
	menuItems.push_back (mapImage.get());

	circlesImage = new cMenuImage (180, 18, NULL);
	menuItems.push_back (circlesImage.get());
	drawLandingPos (landData->iLandX, landData->iLandY);

	createHud();
	hudImage = new cMenuImage (0, 0, hudSurface);
	hudImage->setMovedOverFunction (&mouseMoved);
	menuItems.push_back (hudImage.get());

	infoLabel = new cMenuLabel (position.x + 180 + (position.w - 180) / 2 - (Video.getResolutionX() - 200) / 2, position.y + position.h / 2 - font->getFontHeight (FONT_LATIN_BIG), "", FONT_LATIN_BIG);
	infoLabel->setBox ((Video.getResolutionX() - 200), font->getFontHeight (FONT_LATIN_BIG) * 2);
	menuItems.push_back (infoLabel.get());

	backButton = new cMenuButton (position.x + 35, position.y + 255, lngPack.i18n ("Text~Others~Back"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (backButton.get());

	infoLabelConst = new cMenuLabel (position.x + 180 + (position.w - 180) / 2 - (Video.getResolutionX() - 200) / 2, position.y + (font->getFontHeight (FONT_LATIN_BIG)) * 3 / 2, "", FONT_LATIN_BIG);
	infoLabelConst->setBox ((Video.getResolutionX() - 200), font->getFontHeight (FONT_LATIN_BIG) * 2);
	menuItems.push_back (infoLabelConst.get());

	infoLabelConst->setText (lngPack.i18n ("Text~Comp~Landing_Select"));
	PlayRandomVoice (VoiceData.VOILanding);
}

//------------------------------------------------------------------------------
void cLandingMenu::createHud()
{
	hudSurface = cGameGUI::generateSurface();

	SDL_Rect top, bottom;
	top.x = 0;
	top.y = (Video.getResolutionY() / 2) - 479;

	bottom.x = 0;
	bottom.y = (Video.getResolutionY() / 2);

	SDL_BlitSurface (GraphicsData.gfx_panel_top, NULL, hudSurface, &top);
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom, NULL, hudSurface, &bottom);
}

//------------------------------------------------------------------------------
void cLandingMenu::createMap()
{
	mapSurface = map->createBigSurface (Video.getResolutionX() - 192, Video.getResolutionY() - 32);
}

//------------------------------------------------------------------------------
const sTerrain* cLandingMenu::getMapTile (int x, int y) const
{
	if (x < 0 || x >= Video.getResolutionX() - 192 || y < 0 || y >= Video.getResolutionY() - 32) return NULL;

	x = (int) (x * (448.0f / (Video.getResolutionX() - 180)));
	y = (int) (y * (448.0f / (Video.getResolutionY() - 32)));
	x = (int) (x * map->getSize() / 448.0f);
	y = (int) (y * map->getSize() / 448.0f);
	return &map->getTerrain (x, y);
}

//------------------------------------------------------------------------------
void cLandingMenu::drawLandingPos (int mapX, int mapY)
{
	if (mapX == -1 || mapY == -1)
	{
		circlesImage->setImage (NULL);
		return;
	}
	// pixel per field in x, y direction
	const float fakx = (Video.getResolutionX() - 192.0f) / map->getSize();
	const float faky = (Video.getResolutionY() - 32.0f) / map->getSize();

	AutoSurface circleSurface (SDL_CreateRGBSurface (0, Video.getResolutionX() - 192, Video.getResolutionY() - 32, Video.getColDepth(), 0, 0, 0, 0));
	SDL_FillRect (circleSurface, NULL, 0xFF00FF);
	SDL_SetColorKey (circleSurface, SDL_TRUE, 0xFF00FF);

	const int posX = (int) (mapX * fakx);
	const int posY = (int) (mapY * faky);
	// for non 4:3 screen resolutions, the size of the circles is
	// only correct in x dimension, because I don't draw an ellipse
	drawCircle (posX, posY, (int) ((LANDING_DISTANCE_WARNING   / 2) * fakx), SCAN_COLOR,         circleSurface);
	drawCircle (posX, posY, (int) ((LANDING_DISTANCE_TOO_CLOSE / 2) * fakx), RANGE_GROUND_COLOR, circleSurface);

	circlesImage->setImage (circleSurface);
}

//------------------------------------------------------------------------------
void cLandingMenu::mapClicked (void* parent)
{
	cLandingMenu* menu = reinterpret_cast<cLandingMenu*> (parent);

	if (!menu->canClick) return;

	if(cMouse::getInstance().getCursorType() != eMouseCursorType::Move) return;

	const auto& mousePosition = cMouse::getInstance().getPosition();
	menu->landData->iLandX = (int)((mousePosition.x() - 180) / (448.0f / menu->map->getSize()) * (448.0f / (Video.getResolutionX() - 192)));
	menu->landData->iLandY = (int)((mousePosition.y() - 18) / (448.0f / menu->map->getSize()) * (448.0f / (Video.getResolutionY() - 32)));
	menu->canClick = false;
	menu->backButton->setLocked (true);
	menu->drawLandingPos (menu->landData->iLandX, menu->landData->iLandY);
	Video.draw();
	cMouse::getInstance().setCursorType(eMouseCursorType::Hand);

	menu->hitPosition();
}

//------------------------------------------------------------------------------
void cLandingMenu::mouseMoved (void* parent)
{
	const cLandingMenu* menu = reinterpret_cast<cLandingMenu*> (parent);

	const auto& mousePosition = cMouse::getInstance().getPosition();
	if (menu->mapImage->overItem (mousePosition.x(), mousePosition.y()))
	{
		const sTerrain* terrain = menu->getMapTile(mousePosition.x() - 180, mousePosition.y() - 18);
		if(terrain && !(terrain->water || terrain->coast || terrain->blocked)) cMouse::getInstance().setCursorType(eMouseCursorType::Move);
		else cMouse::getInstance().setCursorType(eMouseCursorType::No);
	}
	else cMouse::getInstance().setCursorType(eMouseCursorType::Hand);
}

//------------------------------------------------------------------------------
//void cLandingMenu::handleKeyInput (const SDL_KeyboardEvent& key)
//{
//	if (key.keysym.sym == SDLK_ESCAPE && key.state == SDL_PRESSED)
//	{
//		// TODO: may use another text here
//		cDialogYesNo yesNoDialog (lngPack.i18n ("Text~Comp~End_Game"));
//		if (switchTo(yesNoDialog) == 0) terminate = true;
//		else draw();
//	}
//}

//------------------------------------------------------------------------------
/*virtual*/ void cLandingMenu::handleNetMessages()
{
	client->getEventHandling().handleNetMessages (client, this);
}

//------------------------------------------------------------------------------
void cLandingMenu::handleNetMessage_MU_MSG_RESELECT_LANDING (cNetMessage& message)
{
	assert (message.iType == MU_MSG_RESELECT_LANDING);
	eLandingState landingState = (eLandingState) message.popChar();

	Log.write ("Client: received MU_MSG_RESELECT_LANDING", cLog::eLOG_TYPE_NET_DEBUG);
	landData->landingState = landingState;
	backButton->setLocked (landingState == LANDING_POSITION_OK || landingState == LANDING_POSITION_CONFIRMED);

	if (landingState == LANDING_POSITION_TOO_CLOSE) infoLabel->setText (lngPack.i18n ("Text~Comp~Landing_Too_Close"));
	else if (landingState == LANDING_POSITION_WARNING) infoLabel->setText (lngPack.i18n ("Text~Comp~Landing_Warning"));

	draw();
	mouseMoved (this); // update cursor
}

//------------------------------------------------------------------------------
void cLandingMenu::handleNetMessage (cNetMessage* message)
{
	switch (message->iType)
	{
		case MU_MSG_RESELECT_LANDING: handleNetMessage_MU_MSG_RESELECT_LANDING (*message); break;
		case MU_MSG_ALL_LANDED: landData->landingState = LANDING_POSITION_OK; end = true; break;
	}
}

//------------------------------------------------------------------------------
void cLandingMenu::hitPosition()
{
	infoLabel->setText (lngPack.i18n ("Text~Multiplayer~Waiting"));
	client->getActivePlayer().setLandingPos (landData->iLandX, landData->iLandY);
	sendLandingCoords (*client, *landData);
	draw();
	if (client->getGameSetting()->hotseat) end = true;
}

//------------------------------------------------------------------------------
// cHotSeatMenu implementation
//------------------------------------------------------------------------------

cHotSeatMenu::cHotSeatMenu (const sSettings& settings) :
	//cMenu (LoadPCX (GFXOD_HOTSEAT))     // 8 players
	cMenu (LoadPCX (GFXOD_PLAYER_SELECT)) // 4 players
{
	backButton = new cMenuButton (position.x + 50, position.y + 450, lngPack.i18n ("Text~Others~Back"));
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (backButton.get());

	okButton = new cMenuButton (position.x + 390, position.y + 450, lngPack.i18n ("Text~Others~OK"));
	okButton->setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (okButton.get());

	okButton->setLocked (true);

	AutoSurface playerHumanSurface (LoadPCX (GFXOD_PLAYER_HUMAN));
	AutoSurface playerNoneSurface (LoadPCX (GFXOD_PLAYER_NONE));
	AutoSurface playerPCSurface (LoadPCX (GFXOD_PLAYER_PC));

	if (settings.clans == SETTING_CLANS_ON)
	{
		const std::string gfxPath = cSettings::getInstance().getGfxPath() + PATH_DELIMITER;
		clanSurfaces[0] = LoadPCX (gfxPath + "clanlogo1.pcx");
		clanSurfaces[1] = LoadPCX (gfxPath + "clanlogo2.pcx");
		clanSurfaces[2] = LoadPCX (gfxPath + "clanlogo3.pcx");
		clanSurfaces[3] = LoadPCX (gfxPath + "clanlogo4.pcx");
		clanSurfaces[4] = LoadPCX (gfxPath + "clanlogo5.pcx");
		clanSurfaces[5] = LoadPCX (gfxPath + "clanlogo6.pcx");
		clanSurfaces[6] = LoadPCX (gfxPath + "clanlogo7.pcx");
		clanSurfaces[7] = LoadPCX (gfxPath + "clanlogo8.pcx");

		for (int i = 0; i != 4; ++i)
		{
			clanImages[i] = new cMenuImage (position.x + 501, position.y + 67 + 92 * i);
			clanImages[i]->setReleasedFunction (onClanClicked);
			menuItems.push_back (clanImages[i].get());
			setClan (i, 0);
		}
	}
	else
	{
		for (int i = 0; i != 4; ++i)
		{
			setClan (i, -1);
		}
	}

	for (int i = 0; i != 4; ++i)
	{
		playerTypeImages[i][0] = new cMenuImage (position.x + 175 + 109 * 0, position.y + 67 + 92 * i, playerHumanSurface);
		playerTypeImages[i][0]->setReleasedFunction (onPlayerTypeClicked);
		menuItems.push_back (playerTypeImages[i][0].get());

		playerTypeImages[i][1] = new cMenuImage (position.x + 175 + 109 * 1, position.y + 67 + 92 * i, playerNoneSurface);
		playerTypeImages[i][1]->setReleasedFunction (onPlayerTypeClicked);
		menuItems.push_back (playerTypeImages[i][1].get());

		playerTypeImages[i][2] = new cMenuImage (position.x + 175 + 109 * 2, position.y + 67 + 92 * i, playerPCSurface);
		playerTypeImages[i][2]->setReleasedFunction (onPlayerTypeClicked);
		menuItems.push_back (playerTypeImages[i][2].get());

		choosePlayerType (i, PLAYERTYPE_NONE);
	}
}

//------------------------------------------------------------------------------
void cHotSeatMenu::setClan (int player, int clan)
{
	clans[player] = clan;

	if (clan == -1)
	{
		if (clanImages[player] != NULL) clanImages[player]->setImage (NULL);
	}
	else
	{
		clanImages[player]->setImage (clanSurfaces[clan]);
	}
}

//------------------------------------------------------------------------------
void cHotSeatMenu::choosePlayerType (int player, ePlayerType playerType)
{
	playerTypes[player] = playerType;

	playerTypeImages[player][0]->hide();
	playerTypeImages[player][1]->hide();
	playerTypeImages[player][2]->hide();

	switch (playerType)
	{
		case PLAYERTYPE_HUMAN: playerTypeImages[player][0]->unhide(); break;
		case PLAYERTYPE_NONE: playerTypeImages[player][1]->unhide(); break;
		case PLAYERTYPE_PC: playerTypeImages[player][2]->unhide(); break;
	}
	okButton->setLocked (true);
	for (int i = 0; i != 4; ++i)
	{
		if (playerTypes[i] != PLAYERTYPE_NONE)
		{
			okButton->setLocked (false);
			return;
		}
	}
}

//------------------------------------------------------------------------------
void cHotSeatMenu::onClanClicked (void* parent)
{
	cHotSeatMenu* menu = static_cast<cHotSeatMenu*> (parent);

	const auto& mousePosition = cMouse::getInstance().getPosition();
	int player = 0;
	for (int i = 0; i != 4; ++i)
	{
		if(menu->clanImages[i]->overItem(mousePosition.x(), mousePosition.y()))
		{
			player = i;
			break;
		}
	}

	cClanSelectionMenu clanSelectionMenu (menu->clans[player], false);

	if (menu->switchTo(clanSelectionMenu) != 1)
	{
		menu->setClan (player, clanSelectionMenu.getClan());
	}
	menu->draw();
}

//------------------------------------------------------------------------------
void cHotSeatMenu::onPlayerTypeClicked (void* parent)
{
	cHotSeatMenu* menu = static_cast<cHotSeatMenu*> (parent);

	const auto& mousePosition = cMouse::getInstance().getPosition();
	for (int player = 0; player != 4; ++player)
	{
		for(int i = 0; i != 3; ++i)
		{
			if(menu->playerTypeImages[player][i]->overItem(mousePosition.x(), mousePosition.y()))
			{
				menu->choosePlayerType(player, ePlayerType(i));
				menu->draw();
				return;
			}
		}
	}
}

//------------------------------------------------------------------------------
// cNetworkMenu implementation
//------------------------------------------------------------------------------

cNetworkMenu::cNetworkMenu() :
	cMenu (LoadPCX (GFXOD_MULT)),
	eventHandler (new cEventHandling),
	savegameNum (-1)
{
	ip = cSettings::getInstance().getIP();
	port = cSettings::getInstance().getPort();

	playersBox = new cMenuPlayersBox (position.x + 465, position.y + 284, 167, 124, this);
	menuItems.push_back (playersBox.get());

	actPlayer = new sPlayer (cSettings::getInstance().getPlayerName(), cSettings::getInstance().getPlayerColor(), 0, MAX_CLIENTS);
	players.push_back (actPlayer);
	playersBox->setPlayers (&players);

	backButton = new cMenuButton (position.x + 50, position.y + 450, lngPack.i18n ("Text~Others~Back"));
	backButton->setReleasedFunction (&backReleased);
	menuItems.push_back (backButton.get());

	sendButton = new cMenuButton (position.x + 470, position.y + 416, lngPack.i18n ("Text~Title~Send"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	sendButton->setReleasedFunction (&sendReleased);
	menuItems.push_back (sendButton.get());

	mapImage = new cMenuImage (position.x + 33, position.y + 106);
	menuItems.push_back (mapImage.get());

	mapLabel = new cMenuLabel (position.x + 90, position.y + 65);
	mapLabel->setCentered (true);
	menuItems.push_back (mapLabel.get());

	settingsText = new cMenuLabel (position.x + 192, position.y + 52);
	settingsText->setBox (246, 146);
	menuItems.push_back (settingsText.get());
	showSettingsText();

	chatBox = new cMenuListBox (position.x + 14, position.y + 284, 439, 124, 50, this);
	menuItems.push_back (chatBox.get());

	chatLine = new cMenuLineEdit (position.x + 15, position.y + 420, 438, 17, this);
	chatLine->setReturnPressedFunc (&sendReleased);
	menuItems.push_back (chatLine.get());

	ipLabel = new cMenuLabel (position.x + 20, position.y + 245, lngPack.i18n ("Text~Title~IP"));
	menuItems.push_back (ipLabel.get());
	portLabel = new cMenuLabel (position.x + 228, position.y + 245, lngPack.i18n ("Text~Title~Port"));
	menuItems.push_back (portLabel.get());

	nameLabel = new cMenuLabel (position.x + 352, position.y + 245, lngPack.i18n ("Text~Title~Player_Name"));
	menuItems.push_back (nameLabel.get());
	colorLabel = new cMenuLabel (position.x + 500, position.y + 245, lngPack.i18n ("Text~Title~Color"));
	menuItems.push_back (colorLabel.get());

	ipLine = new cMenuLineEdit (position.x + 15, position.y + 256, 188, 17, this);
	ipLine->setWasKeyInputFunction (&portIpChanged);
	menuItems.push_back (ipLine.get());

	portLine = new cMenuLineEdit (position.x + 224, position.y + 256, 84, 17, this);
	portLine->setWasKeyInputFunction (&portIpChanged);
	portLine->setText (iToStr (port));
	portLine->setTaking (false, true);
	menuItems.push_back (portLine.get());

	// little icon that restores our default port on click.
	// TODO: find a proper gfx for this or change menu style to dropdown
	setDefaultPortImage = new cMenuImage (position.x + 224 + 85, position.y + 253);
	setDefaultPortImage->setImage (GraphicsData.gfx_Cpfeil2);
	setDefaultPortImage->setClickedFunction (&setDefaultPort);
	menuItems.push_back (setDefaultPortImage.get());

	nameLine = new cMenuLineEdit (position.x + 347, position.y + 256, 90, 17, this);
	nameLine->setText (actPlayer->getName());
	nameLine->setWasKeyInputFunction (&wasNameImput);
	menuItems.push_back (nameLine.get());

	nextColorButton = new cMenuButton (position.x + 596, position.y + 256, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	nextColorButton->setReleasedFunction (&nextColorReleased);
	menuItems.push_back (nextColorButton.get());
	prevColorButton = new cMenuButton (position.x + 478, position.y + 256, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu);
	prevColorButton->setReleasedFunction (&prevColorReleased);
	menuItems.push_back (prevColorButton.get());
	colorImage = new cMenuImage (position.x + 505, position.y + 260);
	setColor (actPlayer->getColorIndex());
	menuItems.push_back (colorImage.get());

	network = new cTCP;
	network->setMessageReceiver (eventHandler.get());
	triedLoadMap = "";

	Log.write (string (PACKAGE_NAME) + " " + PACKAGE_VERSION + " " + PACKAGE_REV, cLog::eLOG_TYPE_NET_DEBUG);
}

//------------------------------------------------------------------------------
cNetworkMenu::~cNetworkMenu()
{
	delete network;
}

//------------------------------------------------------------------------------
/*virtual*/ void cNetworkMenu::handleNetMessages()
{
	eventHandler->handleNetMessages (NULL, this);
}

//------------------------------------------------------------------------------
void cNetworkMenu::showSettingsText()
{
	string text = lngPack.i18n ("Text~Main~Version", PACKAGE_VERSION) + "\n";
	//text += "Checksum: " + iToStr (cSettings::getInstance().Checksum) + "\n\n";

	if (!saveGameString.empty()) text += lngPack.i18n ("Text~Title~Savegame") + ":\n  " + saveGameString + "\n";

	if (map != NULL)
	{
		text += lngPack.i18n ("Text~Title~Map") + ": " + map->getName();
		text += " (" + iToStr (map->getSize()) + "x" + iToStr (map->getSize()) + ")\n";
	}
	else if (savegame.empty()) text += lngPack.i18n ("Text~Multiplayer~Map_NoSet") + "\n";

	text += "\n";

	if (savegame.empty() && saveGameString.empty())
	{
		if (settings != NULL)
		{
			text += lngPack.i18n ("Text~Comp~GameEndsAt") + " " + settings->getVictoryConditionString() + "\n";
			text += lngPack.i18n ("Text~Title~Metal") + ": " + settings->getResValString (settings->metal) + "\n";
			text += lngPack.i18n ("Text~Title~Oil") + ": " + settings->getResValString (settings->oil) + "\n";
			text += lngPack.i18n ("Text~Title~Gold") + ": " + settings->getResValString (settings->gold) + "\n";
			text += lngPack.i18n ("Text~Title~Resource_Density") + ": " + settings->getResFreqString() + "\n";
			text += lngPack.i18n ("Text~Title~Credits")  + ": " + iToStr (settings->credits) + "\n";
			text += lngPack.i18n ("Text~Title~BridgeHead") + ": " + (settings->bridgeHead == SETTING_BRIDGEHEAD_DEFINITE ? lngPack.i18n ("Text~Option~Definite") : lngPack.i18n ("Text~Option~Mobile")) + "\n";
			//text += lngPack.i18n ("Text~Title~Alien_Tech") + ": " + (settings->alienTech == SETTING_ALIENTECH_ON ? lngPack.i18n ("Text~Option~On") : lngPack.i18n ("Text~Option~Off")) + "\n";
			text += string ("Clans") + ": " + (settings->clans == SETTING_CLANS_ON ? lngPack.i18n ("Text~Option~On") : lngPack.i18n ("Text~Option~Off")) + "\n";
			text += lngPack.i18n ("Text~Title~Game_Type") + ": " + (settings->gameType == SETTINGS_GAMETYPE_TURNS ? lngPack.i18n ("Text~Option~Type_Turns") : lngPack.i18n ("Text~Option~Type_Simu")) + "\n";
		}
		else text += lngPack.i18n ("Text~Multiplayer~Option_NoSet") + "\n";
	}
	settingsText->setText (text);
}

//------------------------------------------------------------------------------
void cNetworkMenu::showMap()
{
	if (map == NULL || !map->isValid())
	{
		mapImage->setImage (NULL);
		mapLabel->setText ("");
		return;
	}

	AutoSurface surface (cStaticMap::loadMapPreview (map->getName()));
	if (surface != NULL)
	{
		mapImage->setImage (surface);
	}

	string mapName = map->getName();
	int size = map->getSize();

	if (font->getTextWide (">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
	{
		while (font->getTextWide (">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
		{
			mapName.erase (mapName.length() - 1, mapName.length());
		}
		mapName = mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")";
	}
	else mapName = mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")";

	mapLabel->setText (mapName);
}

void cNetworkMenu::setColor (int color)
{
	SDL_Rect src = { 0, 0, 83, 10 };
	AutoSurface colorSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_BlitSurface (OtherData.colors[color], &src, colorSurface, NULL);
	colorImage->setImage (colorSurface);
}

//------------------------------------------------------------------------------
void cNetworkMenu::saveOptions()
{
	cSettings::getInstance().setPlayerName (actPlayer->getName().c_str());
	cSettings::getInstance().setPort (port);
	cSettings::getInstance().setPlayerColor (actPlayer->getColorIndex());
	if (ip.compare ("-") != 0)
	{
		cSettings::getInstance().setIP (ip.c_str());
	}
}

//------------------------------------------------------------------------------
void cNetworkMenu::changePlayerReadyState (sPlayer* player)
{
	if (player != actPlayer) return;
	if (!map && !triedLoadMap.empty())
	{
		if (!player->isReady()) chatBox->addLine (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", triedLoadMap));
		player->setReady (false);
	}
	else player->setReady (!player->isReady());
	playerSettingsChanged();
}

//------------------------------------------------------------------------------
bool cNetworkMenu::enteredCommand (const string& text)
{
	if (!text.empty() && text[0] == '/')
	{
		if (text.compare (1, text.length(), "ready") == 0) changePlayerReadyState (actPlayer);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cNetworkMenu::playerReadyClicked (sPlayer* player)
{
	if (player != actPlayer) return;
	PlayFX (SoundData.SNDHudButton);
	changePlayerReadyState (player);
}

//------------------------------------------------------------------------------
void cNetworkMenu::backReleased (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	menu->saveOptions();
	menu->terminate = true;
}

//------------------------------------------------------------------------------
void cNetworkMenu::sendReleased (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	string chatText = menu->chatLine->getText();
	if (!chatText.empty() && !menu->enteredCommand (chatText))
	{
		chatText = menu->nameLine->getText() + ": " + chatText;
		menu->chatBox->addLine (chatText);
		sendMenuChatMessage (*menu->network, chatText, NULL, menu->actPlayer->getNr());
	}
	menu->chatLine->setText ("");
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkMenu::nextColorReleased (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	menu->actPlayer->setToNextColorIndex();
	menu->setColor (menu->actPlayer->getColorIndex());
	menu->playerSettingsChanged();
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkMenu::prevColorReleased (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	menu->actPlayer->setToPrevColorIndex();
	menu->setColor (menu->actPlayer->getColorIndex());
	menu->playerSettingsChanged();
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkMenu::wasNameImput (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	menu->actPlayer->setName (menu->nameLine->getText());
	menu->playerSettingsChanged();
}

//------------------------------------------------------------------------------
void cNetworkMenu::portIpChanged (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	menu->port = atoi (menu->portLine->getText().c_str());
	if (menu->ipLine->getText().compare ("-") != 0) menu->ip = menu->ipLine->getText();
}

//------------------------------------------------------------------------------
void cNetworkMenu::setDefaultPort (void* parent)
{
	cNetworkMenu* menu = reinterpret_cast<cNetworkMenu*> (parent);
	menu->portLine->setText (iToStr (DEFAULTPORT));
	menu->draw();
	menu->portLine->setClickedFunction (&portIpChanged);
	menu->port = atoi (menu->portLine->getText().c_str());
}

//------------------------------------------------------------------------------
// cNetworkHostMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cNetworkHostMenu::cNetworkHostMenu()
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 11, lngPack.i18n ("Text~Others~TCPIP_Host"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	okButton = new cMenuButton (position.x + 390, position.y + 450, lngPack.i18n ("Text~Others~OK"));
	okButton->setReleasedFunction (&okReleased);
	menuItems.push_back (okButton.get());

	mapButton = new cMenuButton (position.x + 470, position.y + 42, lngPack.i18n ("Text~Title~Choose_Planet"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	mapButton->setReleasedFunction (&mapReleased);
	menuItems.push_back (mapButton.get());

	settingsButton = new cMenuButton (position.x + 470, position.y + 77, lngPack.i18n ("Text~Title~Options"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	settingsButton->setReleasedFunction (&settingsReleased);
	menuItems.push_back (settingsButton.get());

	loadButton = new cMenuButton (position.x + 470, position.y + 120, lngPack.i18n ("Text~Others~Game_Load"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	loadButton->setReleasedFunction (&loadReleased);
	menuItems.push_back (loadButton.get());

	startButton = new cMenuButton (position.x + 470, position.y + 200, lngPack.i18n ("Text~Others~Host_Start"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	startButton->setReleasedFunction (&startReleased);
	menuItems.push_back (startButton.get());

	ipLine->setText ("-");
	ipLine->setReadOnly (true);
}

//------------------------------------------------------------------------------
cNetworkHostMenu::~cNetworkHostMenu()
{
	for (size_t i = 0; i != mapSenders.size(); ++i)
	{
		delete mapSenders[i];
	}
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::checkTakenPlayerAttr (sPlayer* player)
{
	if (player->isReady() == false) return;

	for (size_t i = 0; i != players.size(); ++i)
	{
		if (static_cast<int> (i) == player->getNr()) continue;
		if (players[i]->getName() == player->getName())
		{
			if (player->getNr() != actPlayer->getNr()) sendMenuChatMessage (*network, "Text~Multiplayer~Player_Name_Taken", player, actPlayer->getNr(), true);
			else chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Name_Taken"));
			player->setReady (false);
			break;
		}
		if (players[i]->getColorIndex() == player->getColorIndex())
		{
			if (player->getNr() != actPlayer->getNr()) sendMenuChatMessage (*network, "Text~Multiplayer~Player_Color_Taken", player, actPlayer->getNr(), true);
			else chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Color_Taken"));
			player->setReady (false);
			break;
		}
	}
}

//------------------------------------------------------------------------------
int cNetworkHostMenu::checkAllPlayersReady() const
{
	for (size_t i = 0; i != players.size(); ++i)
	{
		if (!players[i]->isReady()) return i;
	}
	return -1;
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::okReleased (void* parent)
{
	cNetworkHostMenu* menu = reinterpret_cast<cNetworkHostMenu*> (parent);

	int playerNr;
	if ((!menu->settings || !menu->map) && menu->savegame.empty())
	{
		menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Missing_Settings"));
		menu->draw();
		return;
	}
	else if ((playerNr = menu->checkAllPlayersReady()) != -1)
	{
		menu->chatBox->addLine (menu->players[playerNr]->getName() + " " + lngPack.i18n ("Text~Multiplayer~Not_Ready"));
		menu->draw();
		return;
	}
	else if (menu->network->getConnectionStatus() == 0)
	{
		menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Server_Not_Running"));
		menu->draw();
		return;
	}
	menu->saveOptions();
	if (!menu->savegame.empty())
	{
		if (!menu->runSavedGame())
		{
			return;
		}
	}
	else
	{
		cServer server (menu->network);

		for (size_t i = 0; i != menu->players.size(); ++i)
		{
			server.addPlayer (new cPlayer (*menu->players[i]));
		}
		cClient client (&server, NULL, *menu->eventHandler);
		client.setPlayers (menu->players, *menu->players[0]);

		server.setMap (menu->map);
		server.setGameSettings (*menu->settings);
		server.changeStateToInitGame();
		client.setMap (menu->map);
		client.setGameSetting (*menu->settings);

		sendGo (server);

		runGamePreparation (*menu, client, *menu->map, true);

		menu->switchTo(client.getGameGUI(), &client);
		server.stop();
	}
	menu->end = true;
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::mapReleased (void* parent)
{
	cNetworkHostMenu* menu = reinterpret_cast<cNetworkHostMenu*> (parent);
	cPlanetsSelectionMenu planetsSelectionMenu;

	if (menu->switchTo(planetsSelectionMenu) == 1)
	{
		menu->draw();
		return;
	}
	menu->map = planetsSelectionMenu.getStaticMap();
	menu->showSettingsText();
	menu->showMap();
	sendGameData (*menu->network, menu->map.get(), menu->settings.get(), menu->saveGameString);
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::settingsReleased (void* parent)
{
	cNetworkHostMenu* menu = reinterpret_cast<cNetworkHostMenu*> (parent);
	if (menu->settings == NULL) menu->settings = new sSettings;

	cSettingsMenu settingsMenu (*menu->settings);
	if (menu->switchTo(settingsMenu) == 0) *menu->settings = settingsMenu.getSettings();
	else
	{
		menu->settings = NULL;
	}
	menu->showSettingsText();
	sendGameData (*menu->network, menu->map.get(), menu->settings.get(), menu->saveGameString);
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::loadReleased (void* parent)
{
	cNetworkHostMenu* menu = reinterpret_cast<cNetworkHostMenu*> (parent);
	cLoadMenu loadMenu;
	if (menu->switchTo(loadMenu) == 1)
	{
		menu->draw();
		return;
	}
	menu->savegame = loadMenu.getFilename();
	menu->savegameNum = loadMenu.getLoadingSlotNumber();

	if (!menu->savegame.empty())
	{
		menu->settings = NULL;
		cSavegame savegame (menu->savegameNum);
		savegame.loadHeader (&menu->saveGameString, NULL, NULL);
		menu->saveGameString += "\n\n" + lngPack.i18n ("Text~Title~Players") + "\n" + savegame.getPlayerNames();

		menu->map = std::make_shared<cStaticMap> ();
		menu->map->loadMap (savegame.getMapName());

		sendGameData (*menu->network, menu->map.get(), menu->settings.get(), menu->saveGameString);
		menu->showSettingsText();
		menu->showMap();
	}
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::startReleased (void* parent)
{
	cNetworkHostMenu* menu = reinterpret_cast<cNetworkHostMenu*> (parent);
	// Connect only if there isn't a connection yet
	if (menu->network->getConnectionStatus() != 0) return;

	if (menu->network->create (menu->port) == -1)
	{
		menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Network_Error_Socket"));
		Log.write ("Error opening socket", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Network_Open") + " (" + lngPack.i18n ("Text~Title~Port") + ": "  + iToStr (menu->port) + ")");
		Log.write ("Game open (Port: " + iToStr (menu->port) + ")", cLog::eLOG_TYPE_INFO);
		menu->portLine->setReadOnly (true);
	}
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::playerSettingsChanged()
{
	checkTakenPlayerAttr (actPlayer);
	sendPlayerList (*network, players);
}

void cNetworkHostMenu::handleNetMessage_MU_MSG_CHAT (cNetMessage* message)
{
	assert (message->iType == MU_MSG_CHAT);

	bool translationText = message->popBool();
	string chatText = message->popString();
	if (translationText) chatBox->addLine (lngPack.i18n (chatText));
	else
	{
		chatBox->addLine (chatText);
		// play some chattersound if we got a player message
		PlayFX (SoundData.SNDChat);
	}
	// send to other clients
	for (size_t i = 1; i != players.size(); ++i)
	{
		if (players[i]->getNr() == message->iPlayerNr) continue;
		sendMenuChatMessage (*network, chatText, players[i], -1, translationText);
	}
	draw();
}

#define UNIDENTIFIED_PLAYER_NAME "unidentified"
void cNetworkHostMenu::handleNetMessage_TCP_ACCEPT (cNetMessage* message)
{
	assert (message->iType == TCP_ACCEPT);

	sPlayer* player = new sPlayer (UNIDENTIFIED_PLAYER_NAME, 0, (int) players.size(), message->popInt16());
	players.push_back (player);
	sendRequestIdentification (*network, *player);
	playersBox->setPlayers (&players);
	draw();
}

void cNetworkHostMenu::handleNetMessage_TCP_CLOSE (cNetMessage* message)
{
	assert (message->iType == TCP_CLOSE);

	int socket = message->popInt16();
	network->close (socket);
	string playerName;

	// delete player
	for (size_t i = 0; i != players.size(); ++i)
	{
		if (players[i]->getSocketIndex() == socket)
		{
			playerName = players[i]->getName();
			players.erase (players.begin() + i);
			break;
		}
	}

	// resort socket numbers
	for (size_t i = 0; i != players.size(); ++i)
	{
		players[i]->onSocketIndexDisconnected (socket);
	}

	// resort player numbers
	for (size_t i = 0; i != players.size(); ++i)
	{
		players[i]->setNr (i);
		sendRequestIdentification (*network, *players[i]);
	}

	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Left", playerName));

	draw();
	sendPlayerList (*network, players);

	playersBox->setPlayers (&players);
}

void cNetworkHostMenu::handleNetMessage_MU_MSG_IDENTIFIKATION (cNetMessage* message)
{
	assert (message->iType == MU_MSG_IDENTIFIKATION);

	int playerNr = message->popInt16();
	if (playerNr < 0 || playerNr >= (int) players.size())
	{
		sendPlayerList (*network, players);
		return;
	}
	sPlayer* player = players[playerNr];

	bool freshJoined = (player->getName().compare (UNIDENTIFIED_PLAYER_NAME) == 0);
	player->setColorIndex (message->popInt16());
	player->setName (message->popString());
	player->setReady (message->popBool());

	Log.write ("game version of client " + iToStr (playerNr) + " is: " + message->popString(), cLog::eLOG_TYPE_NET_DEBUG);

	if (freshJoined) chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Joined", player->getName()));

	// search double taken name or color
	checkTakenPlayerAttr (player);

	draw();
	sendPlayerList (*network, players);
	sendGameData (*network, map.get(), settings.get(), saveGameString, player);
}

void cNetworkHostMenu::handleNetMessage_MU_MSG_REQUEST_MAP (cNetMessage* message)
{
	assert (message->iType == MU_MSG_REQUEST_MAP);

	if (map == NULL || MapDownload::isMapOriginal (map->getName())) return;

	const size_t receiverNr = message->popInt16();
	if (receiverNr >= players.size()) return;
	int socketNr = players[receiverNr]->getSocketIndex();
	// check, if there is already a map sender,
	// that uploads to the same socketNr.
	// If yes, terminate the old map sender.
	for (int i = (int) mapSenders.size() - 1; i >= 0; i--)
	{
		if (mapSenders[i] != 0 && mapSenders[i]->getToSocket() == socketNr)
		{
			delete mapSenders[i];
			mapSenders.erase (mapSenders.begin() + i);
		}
	}
	cMapSender* mapSender = new cMapSender (*network, socketNr, eventHandler.get(), map->getName(), players[receiverNr]->getName());
	mapSenders.push_back (mapSender);
	mapSender->runInThread();
	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_Upload", players[receiverNr]->getName()));
	draw();
}

void cNetworkHostMenu::handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (cNetMessage* message)
{
	assert (message->iType == MU_MSG_FINISHED_MAP_DOWNLOAD);

	string receivingPlayerName = message->popString();
	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_UploadFinished", receivingPlayerName));
	draw();
}

//------------------------------------------------------------------------------
void cNetworkHostMenu::handleNetMessage (cNetMessage* message)
{
	Log.write ("Menu: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message->iType)
	{
		case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
		case TCP_ACCEPT: handleNetMessage_TCP_ACCEPT (message); break;
		case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
		case MU_MSG_IDENTIFIKATION: handleNetMessage_MU_MSG_IDENTIFIKATION (message); break;
		case MU_MSG_REQUEST_MAP: handleNetMessage_MU_MSG_REQUEST_MAP (message); break;
		case MU_MSG_FINISHED_MAP_DOWNLOAD: handleNetMessage_MU_MSG_FINISHED_MAP_DOWNLOAD (message); break;
		default: break;
	}
}

//------------------------------------------------------------------------------
bool cNetworkHostMenu::runSavedGame()
{
	cServer server (network);
	cSavegame savegame (savegameNum);
	if (savegame.load (server) == false) return false;
	const std::vector<cPlayer*>& serverPlayerList = server.PlayerList;
	// first we check whether all necessary players are connected
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		for (size_t j = 0; j != players.size(); ++j)
		{
			if (serverPlayerList[i]->getName() == players[j]->getName()) break;
			// stop when a player is missing
			if (j == players.size() - 1)
			{
				chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Player_Wrong"));
				draw();
				return false;
			}
		}
	}
	// then remove all players that do not belong to the save
	for (size_t i = 0; i != players.size(); ++i)
	{
		for (size_t j = 0; j != serverPlayerList.size(); ++j)
		{
			if (players[i]->getName() == serverPlayerList[j]->getName()) break;

			// the player isn't in the list
			// when the loop has gone trough all players
			// and no match was found
			if (j == serverPlayerList.size() - 1)
			{
				sendMenuChatMessage (*network, "Text~Multiplayer~Disconnect_Not_In_Save", players[i], -1, true);
				const int socketIndex = players[i]->getSocketIndex();
				network->close (socketIndex);
				for (size_t k = 0; k != players.size(); ++k)
				{
					players[k]->onSocketIndexDisconnected (socketIndex);
				}
				delete players[i];
				players.erase (players.begin() + i);
				--i;
			}
		}
	}
	// and now set sockets, playernumbers and colors
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		for (size_t j = 0; j != players.size(); ++j)
		{
			if (serverPlayerList[i]->getName() == players[j]->getName())
			{
				// set the sockets in the servers PlayerList
				if (serverPlayerList[i]->getName() == actPlayer->getName()) serverPlayerList[i]->setLocal();
				else serverPlayerList[i]->setSocketIndex (players[j]->getSocketIndex());
				// set the numbers and colors in the menues players-list
				players[j]->setNr (serverPlayerList[i]->getNr());
				players[j]->setColorIndex (serverPlayerList[i]->getColor());
				break;
			}
		}
	}

	// init client and his player
	AutoPtr<cClient> client (new cClient (&server, network, *eventHandler));
	client->setMap (server.Map->staticMap);
	client->setPlayers (players, *actPlayer);

	// send the correct player numbers to client
	for (size_t i = 0; i != players.size(); ++i)
		sendRequestIdentification (*network, *players[i]);

	// now we can send the menus players-list with the right numbers
	// and colors of each player.
	sendPlayerList (*network, players);

	// send client that the game has to be started
	sendGo (server);

	// send data to all players
	for (size_t i = 0; i != serverPlayerList.size(); ++i)
	{
		sendRequestResync (*client, serverPlayerList[i]->getNr());
		sendHudSettings (server, *serverPlayerList[i]);
		std::vector<sSavedReportMessage>& reportList = serverPlayerList[i]->savedReportsList;
		for (size_t j = 0; j != reportList.size(); ++j)
		{
			sendSavedReport (server, reportList[j], serverPlayerList[i]->getNr());
		}
		reportList.clear();
	}

	// exit menu and start game
	server.serverState = SERVER_STATE_INGAME;
	switchTo(client->getGameGUI(), client.get());

	server.stop();

	reloadUnitValues();

	return true;
}

//------------------------------------------------------------------------------
// cNetworkClientMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cNetworkClientMenu::cNetworkClientMenu()
	: mapReceiver (0)
	, lastRequestedMap ("")
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 11, lngPack.i18n ("Text~Others~TCPIP_Client"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	connectButton = new cMenuButton (position.x + 470, position.y + 200, lngPack.i18n ("Text~Title~Connect"), cMenuButton::BUTTON_TYPE_STANDARD_SMALL);
	connectButton->setReleasedFunction (&connectReleased);
	menuItems.push_back (connectButton.get());

	ipLine->setText (ip);
}

cNetworkClientMenu::~cNetworkClientMenu()
{
	delete mapReceiver;
}

void cNetworkClientMenu::connectReleased (void* parent)
{
	cNetworkClientMenu* menu = reinterpret_cast<cNetworkClientMenu*> (parent);

	// Connect only if there isn't a connection yet
	if (menu->network->getConnectionStatus() != 0) return;

	menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Network_Connecting") + menu->ip + ":" + iToStr (menu->port));    // e.g. Connecting to 127.0.0.1:55800
	Log.write (("Connecting to " + menu->ip + ":" + iToStr (menu->port)), cLog::eLOG_TYPE_INFO);

	if (menu->network->connect (menu->ip, menu->port) == -1)
	{
		menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Network_Error_Connect") + menu->ip + ":" + iToStr (menu->port));
		Log.write ("Error on connecting " + menu->ip + ":" + iToStr (menu->port), cLog::eLOG_TYPE_WARNING);
	}
	else
	{
		menu->chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Network_Connected"));
		Log.write ("Connected", cLog::eLOG_TYPE_INFO);
		menu->portLine->setReadOnly (true);
		menu->ipLine->setReadOnly (true);
	}
	menu->draw();
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::playerSettingsChanged()
{
	sendIdentification (*network, *actPlayer);
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_MU_MSG_CHAT (cNetMessage* message)
{
	assert (message->iType == MU_MSG_CHAT);

	bool translationText = message->popBool();
	string chatText = message->popString();
	if (translationText) chatBox->addLine (lngPack.i18n (chatText));
	else chatBox->addLine (chatText);
	draw();
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_TCP_CLOSE (cNetMessage* message)
{
	assert (message->iType == TCP_CLOSE);

	for (size_t i = 0; i != players.size(); ++i)
	{
		if (players[i]->getNr() == actPlayer->getNr()) continue;
		players.erase (players.begin() + i);
		// memleak ?
		break;
	}
	actPlayer->setReady (false);
	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Lost_Connection", "server"));
	playersBox->setPlayers (&players);
	draw();
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (cNetMessage* message)
{
	assert (message->iType == MU_MSG_REQ_IDENTIFIKATION);

	Log.write ("game version of server is: " + message->popString(), cLog::eLOG_TYPE_NET_DEBUG);
	actPlayer->setNr (message->popInt16());
	sendIdentification (*network, *actPlayer);
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_MU_MSG_PLAYERLIST (cNetMessage* message)
{
	assert (message->iType == MU_MSG_PLAYERLIST);

	int playerCount = message->popInt16();
	int actPlayerNr = actPlayer->getNr();
	for (size_t i = 0; i != players.size(); ++i)
	{
		delete players[i];
	}
	players.clear();
	actPlayer = NULL;
	for (int i = 0; i < playerCount; i++)
	{
		string name = message->popString();
		int color = message->popInt16();
		bool ready = message->popBool();
		int nr = message->popInt16();
		sPlayer* player = new sPlayer (name, color, nr);
		player->setReady (ready);
		if (player->getNr() == actPlayerNr) actPlayer = player;
		players.push_back (player);
	}
	playersBox->setPlayers (&players);
	draw();
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_MU_MSG_OPTINS (cNetMessage* message)
{
	assert (message->iType == MU_MSG_OPTINS);

	if (message->popBool())
	{
		if (!settings) settings = new sSettings;
		settings->popFrom (*message);
	}
	else
	{
		settings = NULL;
	}

	if (message->popBool())
	{
		string mapName = message->popString();
		int32_t mapCheckSum = message->popInt32();
		if (!map || map->getName() != mapName)
		{
			bool mapCheckSumsEqual = (MapDownload::calculateCheckSum (mapName) == mapCheckSum);
			map = std::make_shared<cStaticMap>();
			if (mapCheckSumsEqual && map->loadMap (mapName))
			{
				triedLoadMap = "";
			}
			else
			{
				map = NULL;

				if (actPlayer->isReady())
				{
					chatBox->addLine (lngPack.i18n ("Text~Multiplayer~No_Map_No_Ready", mapName));
					actPlayer->setReady (false);
					playerSettingsChanged();
				}
				triedLoadMap = mapName;

				string existingMapFilePath = MapDownload::getExistingMapFilePath (mapName);
				bool existsMap = !existingMapFilePath.empty();
				if (!mapCheckSumsEqual && existsMap)
				{
					chatBox->addLine ("You have an incompatible version of the");
					chatBox->addLine (string ("map \"") + mapName + "\" at");
					chatBox->addLine (string ("\"") + existingMapFilePath + "\" !");
					chatBox->addLine ("Move it away or delete it, then reconnect.");
				}
				else
				{
					if (MapDownload::isMapOriginal (mapName, mapCheckSum) == false)
					{
						if (mapName != lastRequestedMap)
						{
							lastRequestedMap = mapName;
							sendRequestMap (*network, mapName, actPlayer->getNr());
							chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequest"));
							chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_Download", mapName));
						}
					}
					else
					{
						chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadRequestInvalid"));
						chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_DownloadInvalid", mapName));
					}
				}
			}
			showMap();
		}
	}
	else if (map != NULL)
	{
		map = NULL;
		showMap();
	}

	saveGameString = message->popString();

	showSettingsText();
	draw();
}
//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_MU_MSG_GO (cNetMessage* message)
{
	assert (message->iType == MU_MSG_GO);

	saveOptions();

	cServer* const server = NULL;
	cClient client (server, network, *eventHandler);
	client.setPlayers (players, *actPlayer);

	client.setMap (map);
	client.setGameSetting (*settings);

	if (settings->gameType == SETTINGS_GAMETYPE_TURNS && actPlayer->getNr() != 0) client.enableFreezeMode (FREEZE_WAIT_FOR_OTHERS);

	//if (reconnect) sendReconnectionSuccess (client);

	if (saveGameString.empty())
	{
		runGamePreparation (*this, client, *map, true);
	}
	switchTo(client.getGameGUI(), &client);

	end = true;
}
//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage_GAME_EV_REQ_RECON_IDENT (cNetMessage* message)
{
	assert (message->iType == GAME_EV_REQ_RECON_IDENT);

	cDialogYesNo yesNoDialog (lngPack.i18n ("Text~Multiplayer~Reconnect"));
	if (switchTo(yesNoDialog) == 0)
	{
		sendGameIdentification (*network, *actPlayer, message->popInt16());
	}
	else
	{
		chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Connection_Terminated"));
		network->close (0);
	}
	draw();
}
//------------------------------------------------------------------------------

void cNetworkClientMenu::handleNetMessage_GAME_EV_RECONNECT_ANSWER (cNetMessage* message)
{
	assert (message->iType == GAME_EV_RECONNECT_ANSWER);

	if (message->popBool())
	{
		actPlayer->setNr (message->popInt16());
		actPlayer->setColorIndex (message->popInt16());
		map = std::make_shared<cStaticMap>();
		if (!map->loadMap (message->popString())) return;

		int playerCount = message->popInt16();

		std::vector<sPlayer*> clientPlayers;
		sPlayer* localPlayer = new sPlayer (*actPlayer);

		clientPlayers.push_back (localPlayer);
		while (playerCount > 1)
		{
			std::string playername = message->popString();
			int playercolor = message->popInt16();
			int playernr = message->popInt16();
			clientPlayers.push_back (new sPlayer (playername, playercolor, playernr));
			playerCount--;
		}

		cServer* const server = NULL;
		cClient client (server, network, *eventHandler);
		client.setPlayers (clientPlayers, *localPlayer);
		for (size_t i = 0; i != clientPlayers.size(); ++i)
		{
			delete clientPlayers[i];
		}
		clientPlayers.clear();

		client.setMap (map);
		//client.setGameSetting (*settings);

		sendReconnectionSuccess (client);

		switchTo(client.getGameGUI(), &client);


		end = true;
	}
	else
	{
		chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Reconnect_Forbidden"));
		chatBox->addLine (lngPack.i18n ("Text~Multiplayer~Connection_Terminated"));
		network->close (0);
		draw();
	}
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::handleNetMessage (cNetMessage* message)
{
	Log.write ("Menu: --> " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG);

	switch (message->iType)
	{
		case MU_MSG_CHAT: handleNetMessage_MU_MSG_CHAT (message); break;
		case TCP_CLOSE: handleNetMessage_TCP_CLOSE (message); break;
		case MU_MSG_REQ_IDENTIFIKATION: handleNetMessage_MU_MSG_REQ_IDENTIFIKATION (message); break;
		case MU_MSG_PLAYERLIST: handleNetMessage_MU_MSG_PLAYERLIST (message); break;
		case MU_MSG_OPTINS: handleNetMessage_MU_MSG_OPTINS (message); break;
		case MU_MSG_START_MAP_DOWNLOAD: initMapDownload (message); break;
		case MU_MSG_MAP_DOWNLOAD_DATA: receiveMapData (message); break;
		case MU_MSG_CANCELED_MAP_DOWNLOAD: canceledMapDownload (message); break;
		case MU_MSG_FINISHED_MAP_DOWNLOAD: finishedMapDownload (message); break;
		case MU_MSG_GO: handleNetMessage_MU_MSG_GO (message); break;
		case GAME_EV_REQ_RECON_IDENT: handleNetMessage_GAME_EV_REQ_RECON_IDENT (message); break;
		case GAME_EV_RECONNECT_ANSWER: handleNetMessage_GAME_EV_RECONNECT_ANSWER (message); break;
		default: break;
	}
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::initMapDownload (cNetMessage* message)
{
	int mapSize = message->popInt32();
	string mapName = message->popString();

	delete mapReceiver;
	mapReceiver = new cMapReceiver (mapName, mapSize);
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::receiveMapData (cNetMessage* message)
{
	if (mapReceiver == 0)
		return;

	mapReceiver->receiveData (*message);

	int size = mapReceiver->getMapSize();
	int received = mapReceiver->getBytesReceived();
	int finished = (received * 100) / size;
	ostringstream os;

	os << lngPack.i18n ("Text~Multiplayer~MapDL_Percent", iToStr (finished));
	mapLabel->setText (os.str());
	draw();
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::canceledMapDownload (cNetMessage* message)
{
	if (mapReceiver == 0)
		return;

	delete mapReceiver;
	mapReceiver = 0;

	mapLabel->setText (lngPack.i18n ("Text~Multiplayer~MapDL_Cancel"));
	draw();
}

//------------------------------------------------------------------------------
void cNetworkClientMenu::finishedMapDownload (cNetMessage* message)
{
	if (mapReceiver == 0)
		return;

	mapReceiver->finished();

	map = std::make_shared<cStaticMap>();
	if (!map->loadMap (mapReceiver->getMapName())) map = NULL;

	showSettingsText();
	showMap();
	chatBox->addLine (lngPack.i18n ("Text~Multiplayer~MapDL_Finished"));
	draw();

	delete mapReceiver;
	mapReceiver = 0;
}

//------------------------------------------------------------------------------
// cLoadMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cLoadMenu::cLoadMenu (eMenuBackgrounds backgroundType_) :
	cMenu (LoadPCX (GFXOD_SAVELOAD), backgroundType_)
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 12, lngPack.i18n ("Text~Title~Load"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	backButton = new cMenuButton (position.x + 353, position.y + 438, lngPack.i18n ("Text~Others~Back"), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton);
	backButton->setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (backButton.get());

	loadButton = new cMenuButton (position.x + 514, position.y + 438, lngPack.i18n ("Text~Others~Load"), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton);
	loadButton->setReleasedFunction (&loadReleased);
	menuItems.push_back (loadButton.get());

	upButton = new cMenuButton (position.x + 33, position.y + 438, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG);
	upButton->setReleasedFunction (&upReleased);
	menuItems.push_back (upButton.get());

	downButton = new cMenuButton (position.x + 63, position.y + 438, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG);
	downButton->setReleasedFunction (&downReleased);
	menuItems.push_back (downButton.get());

	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 5; y++)
		{
			saveSlots[5 * x + y] = new cMenuSaveSlot (position.x + 17 + 402 * x, position.y + 45 + 76 * y, this);
			saveSlots[5 * x + y]->setReleasedFunction (&slotClicked);
			saveSlots[5 * x + y]->setClickSound (SoundData.SNDObjectMenu);
			menuItems.push_back (saveSlots[5 * x + y].get());
		}
	}

	offset = 0;
	selected = -1;

	files = getFilesOfDirectory (cSettings::getInstance().getSavesPath());

	loadSaves();
	displaySaves();
}

//------------------------------------------------------------------------------
cLoadMenu::~cLoadMenu()
{
	for (size_t i = 0; i != savefiles.size(); ++i)
	{
		delete savefiles[i];
	}
}

//------------------------------------------------------------------------------
void cLoadMenu::loadSaves()
{
	for (size_t i = 0; i != files.size(); ++i)
	{
		// only check for xml files and numbers for this offset
		string const& file = files[i];
		if (file.length() < 4 || file.compare (file.length() - 3, 3, "xml") != 0)
		{
			files.erase (files.begin() + i);
			i--;
			continue;
		}
		int number;
		if (file.length() < 8 || (number = atoi (file.substr (file.length() - 7, 3).c_str())) < offset || number > offset + 10) continue;
		// don't add files twice
		bool found = false;
		for (unsigned int j = 0; j < savefiles.size(); j++)
		{
			if (savefiles[j]->number == number)
			{
				found = true;
				break;
			}
		}
		if (found) continue;
		// read the information and add it to the saveslist
		sSaveFile* savefile = new sSaveFile;
		savefile->number = number;
		savefile->filename = file;
		cSavegame Savegame (number);
		Savegame.loadHeader (&savefile->gamename, &savefile->type, &savefile->time);
		savefiles.push_back (savefile);
	}
}

//------------------------------------------------------------------------------
void cLoadMenu::displaySaves()
{
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 5; y++)
		{
			int filenum = x * 5 + y;
			cMenuSaveSlot* slot = saveSlots[filenum].get();
			sSaveFile* savefile = NULL;
			for (unsigned int i = 0; i < savefiles.size(); i++)
			{
				if (savefiles[i]->number == offset + filenum + 1)
				{
					savefile = savefiles[i];
				}
			}
			if (savefile)
			{
				slot->setSaveData (*savefile, selected == filenum + offset);
			}
			else slot->reset (offset + filenum + 1, selected == filenum + offset);
		}
	}
}

//------------------------------------------------------------------------------
void cLoadMenu::loadReleased (void* parent)
{
	cLoadMenu* menu = reinterpret_cast<cLoadMenu*> (parent);
	sSaveFile* savefile = NULL;
	for (size_t i = 0; i != menu->savefiles.size(); ++i)
	{
		if (menu->savefiles[i]->number == menu->selected + 1)
		{
			savefile = menu->savefiles[i];
		}
	}
	if (savefile)
	{
		menu->selectedFilename = savefile->filename;
		menu->end = true;
	}
	else
	{
		menu->selectedFilename.clear();
		menu->terminate = true;
	}
}

//------------------------------------------------------------------------------
void cLoadMenu::upReleased (void* parent)
{
	cLoadMenu* menu = reinterpret_cast<cLoadMenu*> (parent);
	if (menu->offset > 0)
	{
		menu->offset -= 10;
		menu->loadSaves();
		menu->displaySaves();
		menu->draw();
	}
}

//------------------------------------------------------------------------------
void cLoadMenu::downReleased (void* parent)
{
	cLoadMenu* menu = reinterpret_cast<cLoadMenu*> (parent);
	if (menu->offset < 90)
	{
		menu->offset += 10;
		menu->loadSaves();
		menu->displaySaves();
		menu->draw();
	}
}

//------------------------------------------------------------------------------
void cLoadMenu::slotClicked (void* parent)
{
	cLoadMenu* menu = reinterpret_cast<cLoadMenu*> (parent);
	const auto& mousePosition = cMouse::getInstance().getPosition();

	int x = mousePosition.x() > menu->position.x + menu->position.w / 2 ? 1 : 0;
	int y = 0;
	if(mousePosition.y() > menu->position.y + 118) y++;
	if(mousePosition.y() > menu->position.y + 118 + 77) y++;
	if(mousePosition.y() > menu->position.y + 118 + 77 * 2) y++;
	if(mousePosition.y() > menu->position.y + 118 + 77 * 3) y++;

	int oldSelection = menu->selected;
	menu->selected = 5 * x + y + menu->offset;
	menu->displaySaves();
	menu->extendedSlotClicked (oldSelection);
	menu->draw();

	if (cMouse::getInstance().getButtonClickCount(eMouseButtonType::Left) == 2 && !menu->loadButton->isLocked())
	{
		loadReleased (parent);
	}
}

//------------------------------------------------------------------------------
// cLoadSaveMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cLoadSaveMenu::cLoadSaveMenu (cClient& client_, cServer* server_) :
	cLoadMenu (MNU_BG_ALPHA),
	client (&client_),
	server (server_)
{
	exitButton = new cMenuButton (position.x + 246, position.y + 438, lngPack.i18n ("Text~Others~Exit"), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton);
	exitButton->setReleasedFunction (&exitReleased);
	menuItems.push_back (exitButton.get());

	saveButton = new cMenuButton (position.x + 132, position.y + 438, lngPack.i18n ("Text~Others~Save"), cMenuButton::BUTTON_TYPE_HUGE, FONT_LATIN_BIG, SoundData.SNDMenuButton);
	saveButton->setReleasedFunction (&saveReleased);
	menuItems.push_back (saveButton.get());

	loadButton->setLocked (true);
}

//------------------------------------------------------------------------------
void cLoadSaveMenu::exitReleased (void* parent)
{
	cLoadSaveMenu* menu = reinterpret_cast<cLoadSaveMenu*> (parent);

	cDialogYesNo yesNoDialog (lngPack.i18n ("Text~Comp~End_Game"));
	if (menu->switchTo(yesNoDialog, menu->client) == 0)
	{
		menu->end = true;
	}
	else
	{
		menu->draw();
	}
	//menu->end = true;
}

//------------------------------------------------------------------------------
void cLoadSaveMenu::saveReleased (void* parent)
{
	cLoadSaveMenu* menu = reinterpret_cast<cLoadSaveMenu*> (parent);
	if (menu->selected < 0 || menu->selected > 99) return;
	if (!menu->server)
	{
		cDialogOK okDialog (lngPack.i18n ("Text~Multiplayer~Save_Only_Host"));
		menu->switchTo(okDialog, menu->client);
		menu->draw();
		return;
	}

	cSavegame savegame (menu->selected + 1);
	savegame.save (*menu->server, menu->saveSlots[menu->selected - menu->offset]->getNameEdit()->getText());
	menu->server->makeAdditionalSaveRequest (menu->selected + 1);

	PlayVoice (VoiceData.VOISaved);

	menu->files = getFilesOfDirectory (cSettings::getInstance().getSavesPath());
	for (size_t i = 0; i != menu->savefiles.size(); ++i)
	{
		if (menu->savefiles[i]->number == menu->selected + 1)
		{
			delete menu->savefiles[i];
			menu->savefiles.erase (menu->savefiles.begin() + i);
			break;
		}
	}
	menu->loadSaves();
	menu->displaySaves();
	menu->draw();
}

//------------------------------------------------------------------------------
void cLoadSaveMenu::extendedSlotClicked (int oldSelection)
{
	if (oldSelection != -1 && oldSelection - offset >= 0 && oldSelection - offset < 10)
	{
		saveSlots[oldSelection - offset]->setActivity (false);
		saveSlots[oldSelection - offset]->getNameEdit()->setActivity (false);
		saveSlots[oldSelection - offset]->getNameEdit()->setReadOnly (true);
	}
	saveSlots[selected - offset]->setActivity (true);
	saveSlots[selected - offset]->getNameEdit()->setActivity (true);
	activeItem = saveSlots[selected - offset]->getNameEdit();
	saveSlots[selected - offset]->getNameEdit()->setReadOnly (false);
}

//------------------------------------------------------------------------------
// cBuildsBuildMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cBuildingsBuildMenu::cBuildingsBuildMenu (cClient& client_, cPlayer* player_, cVehicle* vehicle_) :
	cHangarMenu (LoadPCX (GFXOD_BUILD_SCREEN), player_, MNU_BG_ALPHA),
	client (&client_)
{
	vehicle = vehicle_;

	titleLabel = new cMenuLabel (position.x + 405, position.y + 11, lngPack.i18n ("Text~Title~Build_Vehicle"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	doneButton->move (position.x + 387, position.y + 452);
	doneButton->setReleasedFunction (&doneReleased);
	backButton->move (position.x + 300, position.y + 452);
	backButton->setReleasedFunction (&cMenu::cancelReleased);

	selectionList->move (position.x + 477, position.y + 48);
	selectionList->resize (154, 390);
	selListUpButton->move (position.x + 471, position.y + 440);
	selListDownButton->move (position.x + 491, position.y + 440);
	selectionList->setDoubleClickedFunction (&selListDoubleClicked);

	if (vehicle->data.canBuildPath)
	{
		pathButton = new cMenuButton (position.x + 338, position.y + 428, lngPack.i18n ("Text~Others~Path"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
		pathButton->setReleasedFunction (&pathReleased);
		menuItems.push_back (pathButton.get());
	}

	speedHandler = new cMenuBuildSpeedHandler (position.x + 292, position.y + 345);
	menuItems.push_back (speedHandler.get());

	selectionChangedFunc = &selectionChanged;

	generateSelectionList();

	selectionChanged (this);
}

//------------------------------------------------------------------------------
void cBuildingsBuildMenu::generateSelectionList()
{
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); ++i)
	{
		if (UnitsData.sbuildings[i].explodesOnContact) continue;

		if (vehicle->data.canBuild != UnitsData.sbuildings[i].buildAs) continue;

		selectionList->addUnit (&vehicle->owner->BuildingData[i], player);

		if (vehicle->data.storageResCur < player->BuildingData[i].buildCosts) selectionList->getItem (selectionList->getSize() - 1)->setMarked (true);
	}

	if (selectionList->getSize() > 0) selectionList->setSelection (selectionList->getItem (0));
}

//------------------------------------------------------------------------------
void cBuildingsBuildMenu::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (&destroyedUnit == vehicle) terminate = true;
}

//------------------------------------------------------------------------------
void cBuildingsBuildMenu::doneReleased (void* parent)
{
	cBuildingsBuildMenu* menu = reinterpret_cast<cBuildingsBuildMenu*> (parent);
	if (!menu->selectedUnit->getUnitData()->isBig)
	{
		sendWantBuild (*menu->client, menu->vehicle->iID, menu->selectedUnit->getUnitID(), menu->speedHandler->getBuildSpeed(), menu->client->getMap()->getOffset (menu->vehicle->PosX, menu->vehicle->PosY), false, 0);
	}
	else
	{
		menu->client->getGameGUI().mouseInputMode = placeBand;
		menu->vehicle->BuildBigSavedPos = menu->client->getMap()->getOffset (menu->vehicle->PosX, menu->vehicle->PosY);

		// save building information temporary to have them when placing band is finished
		menu->vehicle->BuildingTyp = menu->selectedUnit->getUnitID();
		menu->vehicle->BuildRounds = menu->speedHandler->getBuildSpeed();

		menu->vehicle->FindNextband (menu->client->getGameGUI());
	}
	menu->end = true;
}

//------------------------------------------------------------------------------
void cBuildingsBuildMenu::pathReleased (void* parent)
{
	cBuildingsBuildMenu* menu = reinterpret_cast<cBuildingsBuildMenu*> (parent);

	menu->vehicle->BuildingTyp = menu->selectedUnit->getUnitID();
	menu->vehicle->BuildRounds = menu->speedHandler->getBuildSpeed();

	menu->client->getGameGUI().mouseInputMode = placeBand;
	menu->end = true;
}

//------------------------------------------------------------------------------
void cBuildingsBuildMenu::selectionChanged (void* parent)
{
	cBuildingsBuildMenu* menu = dynamic_cast<cBuildingsBuildMenu*> ((cHangarMenu*) parent);
	if (!menu) menu = dynamic_cast<cBuildingsBuildMenu*> ((cBuildingsBuildMenu*) parent);
	if (!menu->selectedUnit) return;

	const sUnitData* buildingData = menu->player->getUnitDataCurrentVersion (menu->selectedUnit->getUnitID());
	int turboBuildTurns[3], turboBuildCosts[3];
	menu->vehicle->calcTurboBuild (turboBuildTurns, turboBuildCosts, buildingData->buildCosts);

	if (turboBuildTurns[0] == 0)
	{
		turboBuildCosts[0] = buildingData->buildCosts;
		turboBuildTurns[0] = buildingData->buildCosts / menu->vehicle->data.needsMetal;
	}
	menu->speedHandler->setValues (turboBuildTurns, turboBuildCosts);
}

//------------------------------------------------------------------------------
bool cBuildingsBuildMenu::selListDoubleClicked (cMenuUnitsList* list, void* parent)
{
	cBuildingsBuildMenu* menu = reinterpret_cast<cBuildingsBuildMenu*> (parent);
	menu->doneReleased (menu);
	return true;
}

//------------------------------------------------------------------------------
// cVehiclesBuildMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cVehiclesBuildMenu::cVehiclesBuildMenu (const cGameGUI& gameGUI_, cPlayer* player_, cBuilding* building_) :
	cAdvListHangarMenu (LoadPCX (GFXOD_FAC_BUILD_SCREEN), player_, MNU_BG_ALPHA),
	gameGUI (&gameGUI_)
{
	building = building_;

	titleLabel = new cMenuLabel (position.x + 405, position.y + 11, lngPack.i18n ("Text~Title~Build_Factory"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	doneButton->move (position.x + 387, position.y + 452);
	doneButton->setReleasedFunction (&doneReleased);
	backButton->move (position.x + 300, position.y + 452);
	backButton->setReleasedFunction (&cMenu::cancelReleased);

	selectionList->move (position.x + 477,  position.y + 48);
	selectionList->resize (154, 390);
	selListUpButton->move (position.x + 471, position.y + 440);
	selListDownButton->move (position.x + 491, position.y + 440);

	secondList->setDisplayType (MUL_DIS_TYPE_NOEXTRA);
	secondList->move (position.x + 330, position.y + 50);
	secondList->resize (130, 232);
	secondListUpButton->move (position.x + 327, position.y + 293);
	secondListDownButton->move (position.x + 348, position.y + 293);

	speedHandler = new cMenuBuildSpeedHandler (position.x + 292, position.y + 345);
	speedHandler->setBuildSpeed (building->BuildSpeed);
	menuItems.push_back (speedHandler.get());

	repeatButton = new cMenuCheckButton (position.x + 447, position.y + 322, lngPack.i18n ("Text~Comp~Repeat"), building->RepeatBuild, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD, cMenuCheckButton::TEXT_ORIENT_LEFT);
	menuItems.push_back (repeatButton.get());

	selectionChangedFunc = &selectionChanged;

	generateSelectionList();
	createBuildList();

	selectionChanged (this);
}

//------------------------------------------------------------------------------
void cVehiclesBuildMenu::generateSelectionList()
{
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
	{
		sUnitData& unitData = player->VehicleData[i];
		bool land = false;
		bool water = false;
		int x = building->PosX - 2;
		int y = building->PosY - 1;

		for (int j = 0; j < 12; j++)
		{
			if (j == 4 || j == 6 || j == 8)
			{
				x -= 3;
				y += 1;
			}
			else if (j == 5 || j == 7) x += 3;
			else x++;
			const cMap& map = *gameGUI->getClient()->getMap();

			if (map.isValidPos (x, y) == false) continue;

			int off = map.getOffset (x, y);
			std::vector<cBuilding*>& buildings = map.fields[off].getBuildings();
			std::vector<cBuilding*>::iterator b_it = buildings.begin();
			std::vector<cBuilding*>::iterator b_end = buildings.end();

			while (b_it != b_end && ((*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE || (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE)) ++b_it;

			if (!map.isWaterOrCoast (x, y) || (b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_BASE)) land = true;
			else if (map.isWaterOrCoast (x, y) && b_it != b_end && (*b_it)->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)
			{
				land = true;
				water = true;
				break;
			}
			else if (map.isWaterOrCoast (x, y)) water = true;
		}

		if (unitData.factorSea > 0 && unitData.factorGround == 0 && !water) continue;
		else if (unitData.factorGround > 0 && unitData.factorSea == 0 && !land) continue;

		if (building->data.canBuild != unitData.buildAs) continue;

		selectionList->addUnit (&unitData, player, NULL, false, false);
		selectionList->getItem (selectionList->getSize() - 1)->setResValue (-1, false);
	}

	if (selectionList->getSize() > 0) selectionList->setSelection (selectionList->getItem (0));
}

//------------------------------------------------------------------------------
void cVehiclesBuildMenu::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (&destroyedUnit == building) terminate = true;
}

//------------------------------------------------------------------------------
void cVehiclesBuildMenu::createBuildList()
{
	for (size_t i = 0; i != building->BuildList.size(); ++i)
	{
		secondList->addUnit (building->BuildList[i].type, building->owner, NULL, true, false);
		secondList->getItem (secondList->getSize() - 1)->setResValue (building->BuildList[i].metall_remaining, false);
	}
	if (secondList->getSize() > 0)
	{
		setSelectedUnit (secondList->getItem (0));
		secondList->setSelection (secondList->getItem (0));
	}
}

//------------------------------------------------------------------------------
void cVehiclesBuildMenu::doneReleased (void* parent)
{
	cVehiclesBuildMenu* menu = dynamic_cast<cVehiclesBuildMenu*> ((cMenu*) parent);
	if (!menu) return;
	std::vector<sBuildList> buildList;
	for (int i = 0; i < menu->secondList->getSize(); i++)
	{
		sBuildList buildItem;
		buildItem.type = menu->secondList->getItem (i)->getUnitID();
		buildItem.metall_remaining = menu->secondList->getItem (i)->getResValue();
		buildList.push_back (buildItem);
	}
	//menu->building->BuildSpeed = menu->speedHandler->getBuildSpeed();	//TODO: setting buildspeed here is probably an error
	sendWantBuildList (*menu->gameGUI->getClient(), *menu->building, buildList, menu->repeatButton->isChecked(), menu->speedHandler->getBuildSpeed());
	menu->end = true;
}

//------------------------------------------------------------------------------
void cVehiclesBuildMenu::selectionChanged (void* parent)
{
	cVehiclesBuildMenu* menu = dynamic_cast<cVehiclesBuildMenu*> ((cHangarMenu*) parent);
	if (!menu) menu = dynamic_cast<cVehiclesBuildMenu*> ((cVehiclesBuildMenu*) parent);
	if (!menu) return;

	if (!menu->selectedUnit) return;

	const sUnitData* vehicleData = menu->player->getUnitDataCurrentVersion (menu->selectedUnit->getUnitID());
	int turboBuildTurns[3], turboBuildCosts[3];
	menu->building->CalcTurboBuild (turboBuildTurns, turboBuildCosts, vehicleData->buildCosts, menu->selectedUnit->getResValue());

	menu->speedHandler->setValues (turboBuildTurns, turboBuildCosts);
}

//------------------------------------------------------------------------------
// cUpgradeHangarMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cUpgradeHangarContainer::cUpgradeHangarContainer (cHangarMenu* parentMenu, cPlayer* owner)
{
	const SDL_Rect& position = parentMenu->getPosition();
	upgradeFilter = new cMenuUpgradeFilter (position.x + 467, position.y + 411, parentMenu);
	upgradeFilter->setTankChecked (true);
	upgradeFilter->setPlaneChecked (true);
	upgradeFilter->setShipChecked (true);
	upgradeFilter->setBuildingChecked (true);
	parentMenu->addItem (upgradeFilter.get());

	goldBar = new cMenuMaterialBar (position.x + 372, position.y + 301, position.x + 381, position.y + 275, 0, cMenuMaterialBar::MAT_BAR_TYPE_GOLD);
	parentMenu->addItem (goldBar.get());
	goldBarLabel = new cMenuLabel (position.x + 381, position.y + 285, lngPack.i18n ("Text~Title~Credits"));
	goldBarLabel->setCentered (true);
	parentMenu->addItem (goldBarLabel.get());

	upgradeButtons = new cMenuUpgradeHandler (position.x + 283, position.y + 293, parentMenu, this->goldBar.get());
	parentMenu->addItem (upgradeButtons.get());

	initUpgrades (*owner);
}

//------------------------------------------------------------------------------
void cUpgradeHangarContainer::initUpgrades (const cPlayer& player)
{
	unitUpgrades.resize (UnitsData.getNrVehicles() + UnitsData.getNrBuildings());

	for (unsigned int i = 0; i != UnitsData.getNrVehicles(); ++i)
	{
		const sUnitData& oriData = UnitsData.getVehicle (i, player.getClan());
		const sUnitData& data = player.VehicleData[i];

		cUnitUpgrade& unitUpgrade = unitUpgrades[i];
		unitUpgrade.init (oriData, data, player.researchLevel);
	}
	const int offset = UnitsData.getNrVehicles();
	for (unsigned int i = 0; i != UnitsData.getNrBuildings(); ++i)
	{
		const sUnitData& oriData = UnitsData.getBuilding (i, player.getClan());
		const sUnitData& data = player.BuildingData[i];

		cUnitUpgrade& unitUpgrade = unitUpgrades[offset + i];
		unitUpgrade.init (oriData, data, player.researchLevel);
	}
}

//------------------------------------------------------------------------------
void cUpgradeHangarContainer::computePurchased (const cPlayer& player)
{
	for (size_t i = 0; i != unitUpgrades.size(); ++i)
	{
		cUnitUpgrade& unitUpgrade = unitUpgrades[i];
		const int cost = unitUpgrade.computedPurchasedCount (player.researchLevel);
		goldBar->increaseCurrentValue (-cost);
	}
}

//------------------------------------------------------------------------------
// cUpgradeMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cUpgradeMenu::cUpgradeMenu (cClient& client_) :
	cHangarMenu (LoadPCX (GFXOD_UPGRADE), &client_.getActivePlayer(), MNU_BG_ALPHA),
	client (&client_),
	upgradeHangarContainer (this, &client_.getActivePlayer())
{
	titleLabel = new cMenuLabel (position.x + 405, position.y + 11, lngPack.i18n ("Text~Title~Upgrades_Menu"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	doneButton->setReleasedFunction (&doneReleased);
	backButton->setReleasedFunction (&cMenu::cancelReleased);

	const int credits = client_.getActivePlayer().Credits;
	upgradeHangarContainer.getGoldBar().setMaximalValue (credits);
	upgradeHangarContainer.getGoldBar().setCurrentValue (credits);

	upgradeHangarContainer.getUpgradeFilter().setPlaneChecked (plane);
	upgradeHangarContainer.getUpgradeFilter().setTankChecked (tank);
	upgradeHangarContainer.getUpgradeFilter().setShipChecked (ship);
	upgradeHangarContainer.getUpgradeFilter().setBuildingChecked (build);
	upgradeHangarContainer.getUpgradeFilter().setTNTChecked (tnt);

	generateSelectionList();
	if (selectedUnit != 0)
	{
		unitDetails->setSelection (selectedUnit);
		upgradeHangarContainer.getUpgradeButtons().setSelection (selectedUnit);
	}

	selectionChangedFunc = &selectionChanged;
}

//------------------------------------------------------------------------------
void cUpgradeMenu::doneReleased (void* parent)
{
	cUpgradeMenu* menu = dynamic_cast<cUpgradeMenu*> ((cMenu*) parent);
	if (!menu) return;
	sendTakenUpgrades (*menu->client, menu->upgradeHangarContainer.getUnitUpgrades());
	menu->end = true;
}

//------------------------------------------------------------------------------
void cUpgradeMenu::selectionChanged (void* parent)
{
	cUpgradeMenu* menu = dynamic_cast<cUpgradeMenu*> ((cHangarMenu*) parent);
	if (!menu) menu = dynamic_cast<cUpgradeMenu*> ((cUpgradeMenu*) parent);
	if (!menu) return;
	menu->upgradeHangarContainer.getUpgradeButtons().setSelection (menu->selectedUnit);
	menu->draw();
}

//------------------------------------------------------------------------------
bool cUpgradeMenu::tank = true;
bool cUpgradeMenu::plane = false;
bool cUpgradeMenu::ship = false;
bool cUpgradeMenu::build = false;
bool cUpgradeMenu::tnt = false;

void cUpgradeMenu::generateSelectionList()
{
	sID oldSelectedUnit;
	bool selectOldSelectedUnit = false;
	if (selectionList->getSelectedUnit())
	{
		oldSelectedUnit = selectionList->getSelectedUnit()->getUnitID();
		selectOldSelectedUnit = true;
	}

	selectionList->clear();
	tank = upgradeHangarContainer.getUpgradeFilter().TankIsChecked();
	plane = upgradeHangarContainer.getUpgradeFilter().PlaneIsChecked();
	ship = upgradeHangarContainer.getUpgradeFilter().ShipIsChecked();
	build = upgradeHangarContainer.getUpgradeFilter().BuildingIsChecked();
	tnt = upgradeHangarContainer.getUpgradeFilter().TNTIsChecked();

	if (tank || ship || plane)
	{
		for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
		{
			const sUnitData& data = UnitsData.getVehicle (i, player->getClan());
			if (tnt && !data.canAttack) continue;
			if (data.factorAir > 0 && !plane) continue;
			if (data.factorSea > 0 && data.factorGround == 0 && !ship) continue;
			if (data.factorGround > 0 && !tank) continue;
			selectionList->addUnit (data.ID, player, &upgradeHangarContainer.getUnitUpgrade (i));
		}
	}

	if (build)
	{
		for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
		{
			const sUnitData& data = UnitsData.getBuilding (i, player->getClan());
			if (tnt && !data.canAttack) continue;
			selectionList->addUnit (data.ID, player, &upgradeHangarContainer.getUnitUpgrade (UnitsData.getNrVehicles() + i));
		}
	}

	if (selectOldSelectedUnit)
	{
		for (int i = 0; i < selectionList->getSize(); i++)
		{
			if (oldSelectedUnit == selectionList->getItem (i)->getUnitID())
			{
				selectionList->setSelection (selectionList->getItem (i));
				break;
			}
		}
	}
	if (!selectOldSelectedUnit && selectionList->getSize() > 0)
		selectionList->setSelection (selectionList->getItem (0));
}

//------------------------------------------------------------------------------
// cUnitHelpMenu
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cUnitHelpMenu::cUnitHelpMenu (sID unitID, cPlayer* owner)
	: cMenu (LoadPCX (GFXOD_HELP), MNU_BG_ALPHA)
{
	unit = new cMenuUnitListItem (unitID, owner, NULL, MUL_DIS_TYPE_NOEXTRA, NULL, false);
	init (unitID);
}

//------------------------------------------------------------------------------
cUnitHelpMenu::cUnitHelpMenu (sUnitData* unitData, cPlayer* owner) : cMenu (LoadPCX (GFXOD_HELP), MNU_BG_ALPHA)
{
	unit = new cMenuUnitListItem (unitData, owner, NULL, MUL_DIS_TYPE_NOEXTRA, NULL, false);
	init (unitData->ID);
}

//------------------------------------------------------------------------------
void cUnitHelpMenu::init (sID unitID)
{
	titleLabel = new cMenuLabel (position.x + 406, position.y + 11, lngPack.i18n ("Text~Title~Unitinfo"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	doneButton = new cMenuButton (position.x + 474, position.y + 452, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	doneButton->setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (doneButton.get());

	infoImage = new cMenuImage (position.x + 11, position.y + 13);
	menuItems.push_back (infoImage.get());

	infoText = new cMenuLabel (position.x + 354, position.y + 67);
	infoText->setBox (269, 176);
	menuItems.push_back (infoText.get());

	unitDetails = new cMenuUnitDetailsBig (position.x + 16, position.y + 297);
	unitDetails->setSelection (unit.get());
	menuItems.push_back (unitDetails.get());

	if (unitID.isAVehicle())
	{
		infoImage->setImage (UnitsData.getVehicleUI (unitID)->info);
		infoText->setText (unitID.getUnitDataOriginalVersion()->description);
	}
	else if (unitID.isABuilding())
	{
		infoImage->setImage (UnitsData.getBuildingUI (unitID)->info);
		infoText->setText (unitID.getUnitDataOriginalVersion()->description);
	}
}

//------------------------------------------------------------------------------
void cUnitHelpMenu::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (&destroyedUnit.data == unit->getUnitData()) terminate = true;
}

//------------------------------------------------------------------------------
// cStorageMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cStorageMenu::cStorageMenu (cClient& client_, std::vector<cVehicle*>& storageList_, cUnit& unit) :
	cMenu (LoadPCX (GFXOD_STORAGE), MNU_BG_ALPHA),
	client (&client_),
	ownerVehicle (NULL),
	ownerBuilding (NULL),
	storageList (storageList_),
	voiceTypeAll (false),
	voicePlayed (false)
{
	if (unit.isAVehicle()) ownerVehicle = static_cast<cVehicle*> (&unit);
	else ownerBuilding = static_cast<cBuilding*> (&unit);
	unitData = unit.data;
	if (ownerBuilding)
	{
		subBase = ownerBuilding->SubBase;
	}

	offset = 0;
	canStorePlanes = unitData.storeUnitsImageType == sUnitData::STORE_UNIT_IMG_PLANE;
	canStoreShips = unitData.storeUnitsImageType == sUnitData::STORE_UNIT_IMG_SHIP;
	canRepairReloadUpgrade = ownerBuilding != NULL;

	metalValue = ownerBuilding ? subBase->Metal : 0;

	if (!canStorePlanes)
	{
		SDL_Surface* surface = LoadPCX (GFXOD_STORAGE_GROUND);
		SDL_BlitSurface (surface, NULL, background, NULL);
	}

	doneButton = new cMenuButton (position.x + 518, position.y + 371, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	doneButton->setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (doneButton.get());

	upButton = new cMenuButton (position.x + 504, position.y + 426, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG, FONT_LATIN_NORMAL);
	upButton->setReleasedFunction (&upReleased);
	menuItems.push_back (upButton.get());

	downButton = new cMenuButton (position.x + 532, position.y + 426, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG, FONT_LATIN_NORMAL);
	downButton->setReleasedFunction (&downReleased);
	menuItems.push_back (downButton.get());

	activateAllButton = new cMenuButton (position.x + 518, position.y + 246, lngPack.i18n ("Text~Others~Active"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	activateAllButton->setReleasedFunction (&activateAllReleased);
	menuItems.push_back (activateAllButton.get());

	reloadAllButton = new cMenuButton (position.x + 518, position.y + 246 + 25, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Reload") : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	reloadAllButton->setReleasedFunction (&reloadAllReleased);
	menuItems.push_back (reloadAllButton.get());

	repairAllButton = new cMenuButton (position.x + 518, position.y + 246 + 25 * 2, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Repair") : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	repairAllButton->setReleasedFunction (&repairAllReleased);
	menuItems.push_back (repairAllButton.get());

	upgradeAllButton = new cMenuButton (position.x + 518, position.y + 246 + 25 * 3, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Upgrade") : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	upgradeAllButton->setReleasedFunction (&upgradeAllReleased);
	menuItems.push_back (upgradeAllButton.get());

	metalBar = new cMenuMaterialBar (position.x + 546, position.y + 106, position.x + 557, position.y + 86, metalValue, cMenuMaterialBar::MAT_BAR_TYPE_METAL);
	metalBar->setCurrentValue (metalValue);
	menuItems.push_back (metalBar.get());

	generateItems();

	resetInfos();
}

//------------------------------------------------------------------------------
void cStorageMenu::generateItems()
{
	int maxX = canStorePlanes ? 2 : 3;
	int xStep = canStorePlanes ? 227 : 155;
	int xStepImage = canStorePlanes ? 227 : 155;
	int startX = canStorePlanes ? 42 : 8;

	for (int x = 0; x < maxX; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			int index = x + y * maxX;
			activateButtons[index] = new cMenuButton (position.x + startX + x * xStep, position.y + 191 + y * 236, lngPack.i18n ("Text~Others~Active"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
			activateButtons[index]->setReleasedFunction (&activateReleased);
			reloadButtons[index] = new cMenuButton (position.x + startX + x * xStep, position.y + 191 + 25 + y * 236, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Reload") : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
			reloadButtons[index]->setReleasedFunction (&reloadReleased);
			repairButtons[index] = new cMenuButton (position.x + startX + 75 + x * xStep, position.y + 191 + y * 236, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Repair") : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
			repairButtons[index]->setReleasedFunction (&repairReleased);
			upgradeButtons[index] = new cMenuButton (position.x + startX + 75 + x * xStep, position.y + 191 + 25 + y * 236, canRepairReloadUpgrade ? lngPack.i18n ("Text~Others~Upgrade") : "", cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
			upgradeButtons[index]->setReleasedFunction (&upgradeReleased);

			unitImages[index] = new cMenuImage (position.x + 17 + x * xStepImage, position.y + 9 + y * 236);
			unitNames[index] = new cMenuLabel (position.x + 17 + x * xStepImage + 5, position.y + 9 + y * 236 + 5);
			unitNames[index]->setBox (canStorePlanes ? 190 : 118, 118);
			unitInfo[index] = new cMenuStoredUnitDetails (position.x + startX + 17 + x * xStepImage, position.y + 143 + y * 236);

			menuItems.push_back (activateButtons[index].get());
			menuItems.push_back (reloadButtons[index].get());
			menuItems.push_back (repairButtons[index].get());
			menuItems.push_back (upgradeButtons[index].get());

			menuItems.push_back (unitImages[index].get());
			menuItems.push_back (unitNames[index].get());
			menuItems.push_back (unitInfo[index].get());
		}
	}
}

//------------------------------------------------------------------------------
void cStorageMenu::resetInfos()
{
	int maxX = canStorePlanes ? 2 : 3;

	for (int x = 0; x < maxX; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			int pos = x + y * maxX;
			int index = offset * maxX * 2 + pos;

			SDL_Surface* srcSurface;
			string name;

			if (index < (int) storageList.size())
			{
				cVehicle* vehicle = storageList[index];
				srcSurface = vehicle->uiData->storage;
				name = vehicle->getDisplayName();
				const sUnitData& upgraded = *vehicle->owner->getUnitDataCurrentVersion (vehicle->data.ID);
				if (vehicle->data.version != upgraded.version)
				{
					name += "\n(" + lngPack.i18n ("Text~Comp~Dated") + ")";
				}
				unitInfo[pos]->setUnitData (&vehicle->data);

				activateButtons[pos]->setLocked (false);
				if (vehicle->data.ammoCur != vehicle->data.ammoMax && metalValue >= 1) reloadButtons[pos]->setLocked (false);
				else reloadButtons[pos]->setLocked (true);
				if (vehicle->data.hitpointsCur != vehicle->data.hitpointsMax && metalValue >= 1) repairButtons[pos]->setLocked (false);
				else repairButtons[pos]->setLocked (true);
				if (vehicle->data.version != upgraded.version && metalValue >= 1) upgradeButtons[pos]->setLocked (false);
				else upgradeButtons[pos]->setLocked (true);
			}
			else
			{
				name = "";
				if (canStoreShips) srcSurface = GraphicsData.gfx_edock;
				else if (canStorePlanes) srcSurface = GraphicsData.gfx_ehangar;
				else srcSurface = GraphicsData.gfx_edepot;
				unitInfo[pos]->setUnitData (NULL);

				activateButtons[pos]->setLocked (true);
				reloadButtons[pos]->setLocked (true);
				repairButtons[pos]->setLocked (true);
				upgradeButtons[pos]->setLocked (true);
			}

			SDL_Surface* surface = SDL_CreateRGBSurface (0, srcSurface->w, srcSurface->h, Video.getColDepth(), 0, 0, 0, 0);
			SDL_BlitSurface (srcSurface, NULL, surface, NULL);
			unitImages[pos]->setImage (surface);

			unitNames[pos]->setText (name);
		}
	}

	activateAllButton->setLocked (storageList.empty());

	reloadAllButton->setLocked (true);
	repairAllButton->setLocked (true);
	upgradeAllButton->setLocked (true);
	if (canRepairReloadUpgrade && metalValue >= 1)
	{
		for (size_t i = 0; i != storageList.size(); ++i)
		{
			const cVehicle& vehicle = *storageList[i];
			const sUnitData& upgraded = *vehicle.owner->getUnitDataCurrentVersion (vehicle.data.ID);
			if (vehicle.data.ammoCur != vehicle.data.ammoMax) reloadAllButton->setLocked (false);
			if (vehicle.data.hitpointsCur != vehicle.data.hitpointsMax) repairAllButton->setLocked (false);
			if (vehicle.data.version != upgraded.version) upgradeAllButton->setLocked (false);
		}
	}

	if (ownerBuilding)
	{
		metalValue = ownerBuilding->SubBase->Metal;
		metalBar->setCurrentValue (metalValue);
	}

	if ((offset + 1) * maxX * 2 < unitData.storageUnitsMax && (offset + 1) * maxX * 2 < (int) storageList.size()) downButton->setLocked (false);
	else downButton->setLocked (true);

	upButton->setLocked (offset <= 0);
}

//------------------------------------------------------------------------------
void cStorageMenu::upReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	menu->offset--;
	menu->resetInfos();
	menu->draw();
}

//------------------------------------------------------------------------------
void cStorageMenu::downReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	menu->offset++;
	menu->resetInfos();
	menu->draw();
}

//------------------------------------------------------------------------------
int cStorageMenu::getClickedButtonVehIndex (AutoPtr<cMenuButton> (&buttons) [6])
{
	int maxX = canStorePlanes ? 2 : 3;

	const auto& mousePosition = cMouse::getInstance().getPosition();
	for (int x = 0; x < maxX; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			int index = offset * maxX * 2 + x + y * maxX;
			if (index >= (int) storageList.size()) break;

			if(buttons[x + y * maxX]->overItem(mousePosition.x(), mousePosition.y())) return index;
		}
	}
	return -1;
}

//------------------------------------------------------------------------------
void cStorageMenu::activateReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	int index = menu->getClickedButtonVehIndex (menu->activateButtons);
	if (index == -1) return;

	if (menu->ownerVehicle)
	{
		menu->client->getGameGUI().vehicleToActivate = index;
		if (menu->unitData.factorAir > 0) sendWantActivate (*menu->client, menu->ownerVehicle->iID, true, menu->storageList[index]->iID, menu->ownerVehicle->PosX, menu->ownerVehicle->PosY);
		else menu->client->getGameGUI().mouseInputMode = activateVehicle;
	}
	else if (menu->ownerBuilding)
	{
		menu->client->getGameGUI().vehicleToActivate = index;
		menu->client->getGameGUI().mouseInputMode = activateVehicle;
	}
	menu->end = true;
}

//------------------------------------------------------------------------------
void cStorageMenu::reloadReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	int index = menu->getClickedButtonVehIndex (menu->reloadButtons);
	if (index == -1 || !menu->ownerBuilding) return;

	sendWantSupply (*menu->client, menu->storageList[index]->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REARM);
	menu->voiceTypeAll = false;
	menu->voicePlayed = false;
}

//------------------------------------------------------------------------------
void cStorageMenu::repairReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	int index = menu->getClickedButtonVehIndex (menu->repairButtons);
	if (index == -1 || !menu->ownerBuilding) return;

	sendWantSupply (*menu->client, menu->storageList[index]->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REPAIR);
	menu->voiceTypeAll = false;
	menu->voicePlayed = false;
}

//------------------------------------------------------------------------------
void cStorageMenu::upgradeReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	int index = menu->getClickedButtonVehIndex (menu->upgradeButtons);
	if (index == -1 || !menu->ownerBuilding) return;

	sendWantUpgrade (*menu->client, menu->ownerBuilding->iID, index, false);
}

//------------------------------------------------------------------------------
void cStorageMenu::activateAllReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	bool hasCheckedPlace[16];
	fill_n (hasCheckedPlace, 16, false);

	int unitXPos = menu->ownerBuilding ? menu->ownerBuilding->PosX : menu->ownerVehicle->PosX;
	int unitYPos = menu->ownerBuilding ? menu->ownerBuilding->PosY : menu->ownerVehicle->PosY;
	int id = menu->ownerBuilding ? menu->ownerBuilding->iID : menu->ownerVehicle->iID;
	bool isBig = menu->unitData.isBig;
	const cMap& map = *menu->client->getMap();

	for (size_t i = 0; i != menu->storageList.size(); ++i)
	{
		cVehicle* vehicle = menu->storageList[i];
		bool activated = false;
		for (int ypos = unitYPos - 1, poscount = 0; ypos <= unitYPos + (isBig ? 2 : 1); ypos++)
		{
			if (ypos < 0 || ypos >= map.getSize()) continue;
			for (int xpos = unitXPos - 1; xpos <= unitXPos + (isBig ? 2 : 1); xpos++, poscount++)
			{
				if (xpos < 0 || xpos >= map.getSize()) continue;
				if (((ypos == unitYPos && menu->unitData.factorAir == 0) || (ypos == unitYPos + 1 && isBig)) &&
					((xpos == unitXPos && menu->unitData.factorAir == 0) || (xpos == unitXPos + 1 && isBig))) continue;
				if (((menu->ownerBuilding && menu->ownerBuilding->canExitTo (xpos, ypos, map, vehicle->data)) ||
					 (menu->ownerVehicle && menu->ownerVehicle->canExitTo (xpos, ypos, map, vehicle->data)))
					&& !hasCheckedPlace[poscount])
				{
					sendWantActivate (*menu->client, id, menu->ownerVehicle != NULL, vehicle->iID, xpos, ypos);
					hasCheckedPlace[poscount] = true;
					activated = true;
					break;
				}
			}
			if (activated) break;
		}
	}
	menu->end = true;
}

//------------------------------------------------------------------------------
void cStorageMenu::reloadAllReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	if (!menu->ownerBuilding) return;

	menu->voiceTypeAll = true;
	menu->voicePlayed = false;
	int resources = menu->metalValue;
	for (size_t i = 0; i != menu->storageList.size(); ++i)
	{
		if (resources < 1) break;
		cVehicle* vehicle = menu->storageList[i];
		if (vehicle->data.ammoCur != vehicle->data.ammoMax)
		{
			sendWantSupply (*menu->client, vehicle->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REARM);
			resources--;
		}
	}
}

//------------------------------------------------------------------------------
void cStorageMenu::repairAllReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	if (!menu->ownerBuilding) return;

	menu->voiceTypeAll = true;
	menu->voicePlayed = false;
	int resources = menu->metalValue;
	for (size_t i = 0; i != menu->storageList.size(); ++i)
	{
		if (resources < 1) break;
		cVehicle* vehicle = menu->storageList[i];
		if (vehicle->data.hitpointsCur != vehicle->data.hitpointsMax)
		{
			sendWantSupply (*menu->client, vehicle->iID, true, menu->ownerBuilding->iID, false, SUPPLY_TYPE_REPAIR);
			int value = vehicle->data.hitpointsCur;
			while (value < vehicle->data.hitpointsMax)
			{
				value += Round (((float) vehicle->data.hitpointsMax / vehicle->data.buildCosts) * 4);
				resources--;
			}
		}
	}
}

//------------------------------------------------------------------------------
void cStorageMenu::upgradeAllReleased (void* parent)
{
	cStorageMenu* menu = reinterpret_cast<cStorageMenu*> (parent);
	if (!menu->ownerBuilding) return;

	sendWantUpgrade (*menu->client, menu->ownerBuilding->iID, 0, true);
}

//------------------------------------------------------------------------------
void cStorageMenu::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (&destroyedUnit == ownerBuilding || &destroyedUnit == ownerVehicle) terminate = true;
}
//------------------------------------------------------------------------------
void cStorageMenu::playVoice (int Type)
{
	if (voicePlayed) return;

	voicePlayed = true;

	if (Type == SUPPLY_TYPE_REARM)
	{
		PlayFX (SoundData.SNDReload);
		if (voiceTypeAll == true)
		{
			PlayVoice (VoiceData.VOIReammoAll);
		}
		else
		{
			PlayVoice (VoiceData.VOIReammo);
		}
	}
	else
	{
		PlayFX (SoundData.SNDRepair);
		if (voiceTypeAll == true)
		{
			PlayRandomVoice (VoiceData.VOIRepairedAll);
		}
		else
		{
			PlayRandomVoice (VoiceData.VOIRepaired);
		}
	}
}

//------------------------------------------------------------------------------
// cMineManagerMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cMineManagerMenu::cMineManagerMenu (const cClient& client_, cBuilding* building_) :
	cMenu (LoadPCX (GFXOD_MINEMANAGER), MNU_BG_ALPHA),
	client (&client_),
	building (building_),
	subBase (*building_->SubBase)
{
	titleLabel = new cMenuLabel (position.x + position.w / 2, position.y + 11, lngPack.i18n ("Text~Title~Mine"));
	titleLabel->setCentered (true);
	menuItems.push_back (titleLabel.get());

	doneButton = new cMenuButton (position.x + 514, position.y + 430, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_HUGE);
	doneButton->setReleasedFunction (doneReleased);
	menuItems.push_back (doneButton.get());

	// add the bars before the labels
	// so that the bars will be drawn under the labels
	for (int i = 0; i < 3; i++)
	{
		metalBars[i] = new cMenuMaterialBar (position.x + 174, position.y + 70 + 37 * i, position.x + 174 + 120, position.y + 70 + 15 + 37 * i, 0, cMenuMaterialBar::MAT_BAR_TYPE_METAL_HORI_BIG, false, false);
		menuItems.push_back (metalBars[i].get());
		oilBars[i] = new cMenuMaterialBar (position.x + 174, position.y + 190 + 37 * i, position.x + 174 + 120, position.y + 190 + 15 + 37 * i, 0, cMenuMaterialBar::MAT_BAR_TYPE_OIL_HORI_BIG, false, false);
		menuItems.push_back (oilBars[i].get());
		goldBars[i] = new cMenuMaterialBar (position.x + 174, position.y + 310 + 37 * i, position.x + 174 + 120, position.y + 310 + 15 + 37 * i, 0, cMenuMaterialBar::MAT_BAR_TYPE_GOLD_HORI_BIG, false, false);
		menuItems.push_back (goldBars[i].get());

		noneBars[i] = new cMenuMaterialBar (position.x + 174, position.y + 70 + 120 * i, position.x + 174 + 120, position.y + 310 + 15 + 37 * i, 0, cMenuMaterialBar::MAT_BAR_TYPE_NONE_HORI_BIG, true, false);
		menuItems.push_back (noneBars[i].get());
	}

	for (int i = 0; i < 3; i++)
	{
		incButtons[i] = new cMenuButton (position.x + 421, position.y + 70 + 120 * i, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_BIG);
		incButtons[i]->setReleasedFunction (&increaseReleased);
		menuItems.push_back (incButtons[i].get());
		decButtons[i] = new cMenuButton (position.x + 139, position.y + 70 + 120 * i, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_BIG);
		decButtons[i]->setReleasedFunction (&decreseReleased);
		menuItems.push_back (decButtons[i].get());

		if (i == 0) resourceLabels[i] = new cMenuLabel (position.x + 81, position.y + 78, lngPack.i18n ("Text~Title~Metal"));
		else if (i == 1) resourceLabels[i] = new cMenuLabel (position.x + 81, position.y + 78 + 121, lngPack.i18n ("Text~Title~Oil"));
		else  resourceLabels[i] = new cMenuLabel (position.x + 81, position.y + 78 + 121 * 2, lngPack.i18n ("Text~Title~Gold"));
		resourceLabels[i]->setCentered (true);
		menuItems.push_back (resourceLabels[i].get());

		usageLabels[i] = new cMenuLabel (position.x + 81, position.y + 78 + 37 + 121 * i, lngPack.i18n ("Text~Others~Usage_7"));
		usageLabels[i]->setCentered (true);
		menuItems.push_back (usageLabels[i].get());

		reserveLabels[i] = new cMenuLabel (position.x + 81, position.y + 78 + 37 * 2 + 121 * i, lngPack.i18n ("Text~Comp~Reserve"));
		reserveLabels[i]->setCentered (true);
		menuItems.push_back (reserveLabels[i].get());

		metalBarLabels[i] = new cMenuLabel (position.x + 174 + 120, position.y + 70 + 8 + 37 * i, "", FONT_LATIN_BIG);
		metalBarLabels[i]->setCentered (true);
		menuItems.push_back (metalBarLabels[i].get());

		oilBarLabels[i] = new cMenuLabel (position.x + 174 + 120, position.y + 190 + 8 + 37 * i, "", FONT_LATIN_BIG);
		oilBarLabels[i]->setCentered (true);
		menuItems.push_back (oilBarLabels[i].get());

		goldBarLabels[i] = new cMenuLabel (position.x + 174 + 120, position.y + 310 + 8 + 37 * i, "", FONT_LATIN_BIG);
		goldBarLabels[i]->setCentered (true);
		menuItems.push_back (goldBarLabels[i].get());
	}
	metalBars[0]->setClickedFunction (&barReleased);
	oilBars[0]->setClickedFunction (&barReleased);
	goldBars[0]->setClickedFunction (&barReleased);

	setBarValues();
	setBarLabels();
}

//------------------------------------------------------------------------------
void cMineManagerMenu::setBarValues()
{
	metalBars[0]->setMaximalValue (subBase.getMaxMetalProd());
	metalBars[0]->setCurrentValue (subBase.getMetalProd());
	metalBars[1]->setMaximalValue (subBase.MaxMetalNeed);
	metalBars[1]->setCurrentValue (subBase.MetalNeed);
	metalBars[2]->setMaximalValue (subBase.MaxMetal);
	metalBars[2]->setCurrentValue (subBase.Metal);
	if (subBase.getMaxMetalProd() == 0)
	{
		noneBars[0]->setMaximalValue (1);
		noneBars[0]->setCurrentValue (1);
	}
	else
	{
		noneBars[0]->setMaximalValue (subBase.getMaxMetalProd());
		noneBars[0]->setCurrentValue (subBase.getMaxMetalProd() - subBase.getMaxAllowedMetalProd());
	}

	oilBars[0]->setMaximalValue (subBase.getMaxOilProd());
	oilBars[0]->setCurrentValue (subBase.getOilProd());
	oilBars[1]->setMaximalValue (subBase.MaxOilNeed);
	oilBars[1]->setCurrentValue (subBase.OilNeed);
	oilBars[2]->setMaximalValue (subBase.MaxOil);
	oilBars[2]->setCurrentValue (subBase.Oil);
	if (subBase.getMaxOilProd() == 0)
	{
		noneBars[1]->setMaximalValue (1);
		noneBars[1]->setCurrentValue (1);
	}
	else
	{
		noneBars[1]->setMaximalValue (subBase.getMaxOilProd());
		noneBars[1]->setCurrentValue (subBase.getMaxOilProd() - subBase.getMaxAllowedOilProd());
	}

	goldBars[0]->setMaximalValue (subBase.getMaxGoldProd());
	goldBars[0]->setCurrentValue (subBase.getGoldProd());
	goldBars[1]->setMaximalValue (subBase.MaxGoldNeed);
	goldBars[1]->setCurrentValue (subBase.GoldNeed);
	goldBars[2]->setMaximalValue (subBase.MaxGold);
	goldBars[2]->setCurrentValue (subBase.Gold);
	if (subBase.getMaxGoldProd() == 0)
	{
		noneBars[2]->setMaximalValue (1);
		noneBars[2]->setCurrentValue (1);
	}
	else
	{
		noneBars[2]->setMaximalValue (subBase.getMaxGoldProd());
		noneBars[2]->setCurrentValue (subBase.getMaxGoldProd() - subBase.getMaxAllowedGoldProd());
	}
}

//------------------------------------------------------------------------------
void cMineManagerMenu::setBarLabels()
{
	metalBarLabels[0]->setText (iToStr (subBase.getMetalProd()));
	metalBarLabels[1]->setText (secondBarText (subBase.getMetalProd(), subBase.MetalNeed));
	metalBarLabels[2]->setText (iToStr (subBase.Metal));

	oilBarLabels[0]->setText (iToStr (subBase.getOilProd()));
	oilBarLabels[1]->setText (secondBarText (subBase.getOilProd(), subBase.OilNeed));
	oilBarLabels[2]->setText (iToStr (subBase.Oil));

	goldBarLabels[0]->setText (iToStr (subBase.getGoldProd()));
	goldBarLabels[1]->setText (secondBarText (subBase.getGoldProd(), subBase.GoldNeed));
	goldBarLabels[2]->setText (iToStr (subBase.Gold));
}

//------------------------------------------------------------------------------
string cMineManagerMenu::secondBarText (int prod, int need)
{
	int perTurn = prod - need;
	string text = iToStr (need) + " (";
	if (perTurn > 0) text += "+";
	text += iToStr (perTurn) + " / " + lngPack.i18n ("Text~Comp~Turn") + ")";
	return text;
}

//------------------------------------------------------------------------------
void cMineManagerMenu::doneReleased (void* parent)
{
	cMineManagerMenu* menu = reinterpret_cast<cMineManagerMenu*> (parent);
	sendChangeResources (*menu->client, *menu->building, menu->subBase.getMetalProd(), menu->subBase.getOilProd(), menu->subBase.getGoldProd());
	menu->end = true;
}

//------------------------------------------------------------------------------
void cMineManagerMenu::increaseReleased (void* parent)
{
	cMineManagerMenu* menu = reinterpret_cast<cMineManagerMenu*> (parent);
	const auto& mousePosition = cMouse::getInstance().getPosition();
	if(menu->incButtons[0]->overItem(mousePosition.x(), mousePosition.y()) && menu->subBase.getMaxAllowedMetalProd() - menu->subBase.getMetalProd() > 0)
	{
		menu->subBase.changeMetalProd (+1);
	}
	else if(menu->incButtons[1]->overItem(mousePosition.x(), mousePosition.y()) && menu->subBase.getMaxAllowedOilProd() - menu->subBase.getOilProd() > 0)
	{
		menu->subBase.changeOilProd (+1);
	}
	else if(menu->incButtons[2]->overItem(mousePosition.x(), mousePosition.y()) && menu->subBase.getMaxAllowedGoldProd() - menu->subBase.getGoldProd() > 0)
	{
		menu->subBase.changeGoldProd (+1);
	}
	menu->setBarValues();
	menu->setBarLabels();
	menu->draw();
}

//------------------------------------------------------------------------------
void cMineManagerMenu::decreseReleased (void* parent)
{
	cMineManagerMenu* menu = reinterpret_cast<cMineManagerMenu*> (parent);
	const auto& mousePosition = cMouse::getInstance().getPosition();
	if(menu->decButtons[0]->overItem(mousePosition.x(), mousePosition.y()) && menu->subBase.getMetalProd() > 0)
	{
		menu->subBase.changeMetalProd (-1);
	}
	else if(menu->decButtons[1]->overItem(mousePosition.x(), mousePosition.y()) && menu->subBase.getOilProd() > 0)
	{
		menu->subBase.changeOilProd (-1);
	}
	else if(menu->decButtons[2]->overItem(mousePosition.x(), mousePosition.y()) && menu->subBase.getGoldProd() > 0)
	{
		menu->subBase.changeGoldProd (-1);
	}
	menu->setBarValues();
	menu->setBarLabels();
	menu->draw();
}

//------------------------------------------------------------------------------
void cMineManagerMenu::barReleased (void* parent)
{
	cMineManagerMenu* menu = reinterpret_cast<cMineManagerMenu*> (parent);
	const auto& mousePosition = cMouse::getInstance().getPosition();
	if(menu->metalBars[0]->overItem(mousePosition.x(), mousePosition.y()))
	{
		int metal = Round((mousePosition.x() - menu->metalBars[0]->getPosition().x) * (menu->subBase.getMaxMetalProd() / (float)menu->metalBars[0]->getPosition().w));
		menu->subBase.setMetalProd (metal);
	}
	else if(menu->oilBars[0]->overItem(mousePosition.x(), mousePosition.y()))
	{
		int oil = Round((mousePosition.x() - menu->oilBars[0]->getPosition().x) * (menu->subBase.getMaxOilProd() / (float)menu->oilBars[0]->getPosition().w));
		menu->subBase.setOilProd (oil);
	}
	else if(menu->goldBars[0]->overItem(mousePosition.x(), mousePosition.y()))
	{
		int gold = Round((mousePosition.x() - menu->goldBars[0]->getPosition().x) * (menu->subBase.getMaxGoldProd() / (float)menu->goldBars[0]->getPosition().w));
		menu->subBase.setGoldProd (gold);
	}
	menu->setBarValues();
	menu->setBarLabels();
	menu->draw();
}

//------------------------------------------------------------------------------
void cMineManagerMenu::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (destroyedUnit.isAVehicle()) return;
	cBuilding& destroyedBuilding = static_cast<cBuilding&> (destroyedUnit);
	if (destroyedBuilding.SubBase == building->SubBase) terminate = true;
}

//------------------------------------------------------------------------------
// cReportsMenu implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
cReportsMenu::cReportsMenu (cClient& client_)
	: cMenu (LoadPCX (GFXOD_REPORTS), MNU_BG_ALPHA)
	, client (&client_)
{
	typeButtonGroup = new cMenuRadioGroup();
	menuItems.push_back (typeButtonGroup.get());

	typeButtonGroup->addButton (new cMenuCheckButton (position.x + 524, position.y + 71, lngPack.i18n ("Text~Others~Units"), true, false, cMenuCheckButton::RADIOBTN_TYPE_ANGULAR_BUTTON));
	typeButtonGroup->addButton (new cMenuCheckButton (position.x + 524, position.y + 71 + 29, lngPack.i18n ("Text~Others~Disadvantages"), false, false, cMenuCheckButton::RADIOBTN_TYPE_ANGULAR_BUTTON));
	typeButtonGroup->addButton (new cMenuCheckButton (position.x + 524, position.y + 71 + 29 * 2, lngPack.i18n ("Text~Others~Score"), false, false, cMenuCheckButton::RADIOBTN_TYPE_ANGULAR_BUTTON));
	typeButtonGroup->addButton (new cMenuCheckButton (position.x + 524, position.y + 71 + 29 * 3, lngPack.i18n ("Text~Others~Reports"), false, false, cMenuCheckButton::RADIOBTN_TYPE_ANGULAR_BUTTON));
	typeButtonGroup->setClickedFunction (&typeChanged);

	includedLabel = new cMenuLabel (position.x + 497, position.y + 207, lngPack.i18n ("Text~Others~Included") + ":");
	menuItems.push_back (includedLabel.get());

	planesCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 218, lngPack.i18n ("Text~Others~Air_Units"), true, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	planesCheckBtn->setClickedFunction (&filterClicked);
	planesCheckBtn->limitTextSize (123);
	menuItems.push_back (planesCheckBtn.get());
	groundCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 218 + 18, lngPack.i18n ("Text~Others~Ground_Units"), true, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	groundCheckBtn->setClickedFunction (&filterClicked);
	groundCheckBtn->limitTextSize (123);
	menuItems.push_back (groundCheckBtn.get());
	seaCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 218 + 18 * 2, lngPack.i18n ("Text~Others~Sea_Units"), true, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	seaCheckBtn->setClickedFunction (&filterClicked);
	seaCheckBtn->limitTextSize (123);
	menuItems.push_back (seaCheckBtn.get());
	stationaryCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 218 + 18 * 3, lngPack.i18n ("Text~Others~Stationary_Units"), true, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	stationaryCheckBtn->setClickedFunction (&filterClicked);
	stationaryCheckBtn->limitTextSize (123);
	menuItems.push_back (stationaryCheckBtn.get());

	borderedLabel = new cMenuLabel (position.x + 497, position.y + 299, lngPack.i18n ("Text~Others~Limited_To") + ":");
	menuItems.push_back (borderedLabel.get());

	buildCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 312, lngPack.i18n ("Text~Others~Produce_Units"), false, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	buildCheckBtn->setClickedFunction (&filterClicked);
	buildCheckBtn->limitTextSize (123);
	menuItems.push_back (buildCheckBtn.get());
	fightCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 312 + 18, lngPack.i18n ("Text~Others~Fight_Units"), false, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	fightCheckBtn->setClickedFunction (&filterClicked);
	fightCheckBtn->limitTextSize (123);
	menuItems.push_back (fightCheckBtn.get());
	damagedCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 312 + 18 * 2, lngPack.i18n ("Text~Others~Damaged_Units"), false, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	damagedCheckBtn->setClickedFunction (&filterClicked);
	damagedCheckBtn->limitTextSize (123);
	menuItems.push_back (damagedCheckBtn.get());
	stealthCheckBtn = new cMenuCheckButton (position.x + 496, position.y + 312 + 18 * 3, lngPack.i18n ("Text~Others~Stealth_Units"), false, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
	stealthCheckBtn->setClickedFunction (&filterClicked);
	stealthCheckBtn->limitTextSize (123);
	menuItems.push_back (stealthCheckBtn.get());

	doneButton = new cMenuButton (position.x + 524, position.y + 398, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL);
	doneButton->setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (doneButton.get());

	// it's important that the screen will be added
	// before the up and down buttons
	dataScreen = new cMenuReportsScreen (position.x + 7, position.y + 6, 479, 467, *client, this);
	menuItems.push_back (dataScreen.get());

	upButton = new cMenuButton (position.x + 492, position.y + 426, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BIG);
	upButton->setReleasedFunction (&upReleased);
	menuItems.push_back (upButton.get());

	downButton = new cMenuButton (position.x + 525, position.y + 426, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BIG);
	downButton->setReleasedFunction (&downReleased);
	menuItems.push_back (downButton.get());

	filterClicked (this);
}

//------------------------------------------------------------------------------
void cReportsMenu::upReleased (void* parent)
{
	cReportsMenu* menu = reinterpret_cast<cReportsMenu*> (parent);
	menu->dataScreen->scrollUp();
	menu->draw();
}

//------------------------------------------------------------------------------
void cReportsMenu::downReleased (void* parent)
{
	cReportsMenu* menu = reinterpret_cast<cReportsMenu*> (parent);
	menu->dataScreen->scrollDown();
	menu->draw();
}

//------------------------------------------------------------------------------
void cReportsMenu::typeChanged (void* parent)
{
	cReportsMenu* menu = reinterpret_cast<cReportsMenu*> (parent);
	menu->dataScreen->setType (menu->typeButtonGroup->buttonIsChecked (0), menu->typeButtonGroup->buttonIsChecked (1), menu->typeButtonGroup->buttonIsChecked (2), menu->typeButtonGroup->buttonIsChecked (3));
}

//------------------------------------------------------------------------------
void cReportsMenu::filterClicked (void* parent)
{
	cReportsMenu* menu = reinterpret_cast<cReportsMenu*> (parent);

	menu->dataScreen->setIncludeFilter (menu->planesCheckBtn->isChecked(), menu->groundCheckBtn->isChecked(), menu->seaCheckBtn->isChecked(), menu->stationaryCheckBtn->isChecked());
	menu->dataScreen->setBorderedFilter (menu->buildCheckBtn->isChecked(), menu->fightCheckBtn->isChecked(), menu->damagedCheckBtn->isChecked(), menu->stealthCheckBtn->isChecked());
}

//------------------------------------------------------------------------------
void cReportsMenu::scrollCallback (bool upPossible, bool downPossible)
{
	upButton->setLocked (!upPossible);
	downButton->setLocked (!downPossible);
}

//------------------------------------------------------------------------------
void cReportsMenu::doubleClicked (cUnit* unit)
{
	if (!unit) return;

	end = true;
	cVehicle* vehicle = unit->isAVehicle() ? static_cast<cVehicle*> (unit) : NULL;
	if (vehicle && vehicle->Loaded)
	{
		// find storing unit
		cUnit* storingUnit = vehicle->getContainerVehicle();
		if (storingUnit == NULL) storingUnit = vehicle->getContainerBuilding();
		doubleClicked (storingUnit);
		return;
	}
	client->getGameGUI().selectUnit (*unit);
	client->getGameGUI().center (*unit);
}
