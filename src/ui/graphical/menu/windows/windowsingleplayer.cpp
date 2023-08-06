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

#include "windowsingleplayer.h"

#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/data/savegameinfo.h"
#include "game/data/units/landingunit.h"
#include "game/data/units/unitdata.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/startup/gamepreparation.h"
#include "resources/uidata.h"
#include "ui/graphical/game/gamegui.h"
#include "ui/graphical/menu/control/local/singleplayer/localsingleplayergamenew.h"
#include "ui/graphical/menu/control/local/singleplayer/localsingleplayergamesaved.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/widgets/application.h"
#include "utility/language.h"
#include "utility/log.h"

#include <functional>

//------------------------------------------------------------------------------
cWindowSinglePlayer::cWindowSinglePlayer() :
	cWindowMain (lngPack.i18n ("Others~Single_Player"))
{
	newGameButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Others~Game_New"));
	signalConnectionManager.connect (newGameButton->clicked, [this]() { newGameClicked(); });

	loadGameButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Others~Game_Load"));
	signalConnectionManager.connect (loadGameButton->clicked, [this]() { loadGameClicked(); });

	backButton = emplaceChild<cPushButton> (getPosition() + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, lngPack.i18n ("Others~Back"));
	signalConnectionManager.connect (backButton->clicked, [this]() { backClicked(); });
}

//------------------------------------------------------------------------------
cWindowSinglePlayer::~cWindowSinglePlayer()
{}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::retranslate()
{
	cWindowMain::retranslate();
	setTitle (lngPack.i18n ("Others~Single_Player"));

	newGameButton->setText (lngPack.i18n ("Others~Game_New"));
	loadGameButton->setText (lngPack.i18n ("Others~Game_Load"));
	backButton->setText (lngPack.i18n ("Others~Back"));
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::newGameClicked()
{
	if (!getActiveApplication()) return;

	auto application = getActiveApplication();

	auto game = std::make_shared<cLocalSingleplayerGameNew>();

	auto windowGameSettings = getActiveApplication()->show (std::make_shared<cWindowGameSettings>());
	windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->done.connect ([=] (const cGameSettings& gameSettings) {
		game->setGameSettings (gameSettings);

		auto windowMapSelection = application->show (std::make_shared<cWindowMapSelection>());

		windowMapSelection->done.connect ([=] (const std::filesystem::path& mapFilename) {
			game->selectMapFilename (mapFilename);
			game->runGamePreparation (*application);
			windowMapSelection->close();
			windowGameSettings->close();
		});
	});
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::loadGameClicked()
{
	if (!getActiveApplication()) return;

	auto application = getActiveApplication();

	auto windowLoad = getActiveApplication()->show (std::make_shared<cWindowLoad>());
	windowLoad->load.connect ([=] (const cSaveGameInfo& saveInfo) {
		auto game = std::make_shared<cLocalSingleplayerGameSaved>();
		game->setSaveGameNumber (saveInfo.number);
		try
		{
			game->start (*application);
		}
		catch (const std::runtime_error& e)
		{
			Log.error ("Could not start saved game.");
			Log.error (e.what());
			application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Error_Messages~ERROR_Save_Loading")));
			return;
		}

		windowLoad->close();
	});
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::backClicked()
{
	close();
}
