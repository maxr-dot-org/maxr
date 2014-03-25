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

#include "windowstart.h"
#include "windowsingleplayer.h"
#include "windowmultiplayer.h"
#include "../dialogs/dialogpreferences.h"
#include "../../../main.h"
#include "../widgets/pushbutton.h"
#include "../../application.h"

//------------------------------------------------------------------------------
cWindowStart::cWindowStart () :
	cWindowMain (lngPack.i18n ("Text~Title~MainMenu")),
	firstActivate (true)
{
	using namespace std::placeholders;

	auto singlePlayerButton = std::make_unique<cPushButton> (getPosition () + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Single_Player"));
	signalConnectionManager.connect (singlePlayerButton->clicked, std::bind (&cWindowStart::singlePlayerClicked, this));
	addChild (std::move (singlePlayerButton));

	auto multiPlayerButton = std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Multi_Player"));
	signalConnectionManager.connect (multiPlayerButton->clicked, std::bind (&cWindowStart::multiPlayerClicked, this));
	addChild (std::move (multiPlayerButton));

	auto preferencesButton = std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace * 2), ePushButtonType::StandardBig, lngPack.i18n ("Text~Settings~Preferences"));
	signalConnectionManager.connect (preferencesButton->clicked, std::bind (&cWindowStart::preferencesClicked, this));
	addChild (std::move (preferencesButton));

	auto licenceButton = std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace * 3), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Mani"));
	signalConnectionManager.connect (licenceButton->clicked, std::bind (&cWindowStart::licenceClicked, this));
	addChild (std::move (licenceButton));

	auto exitButton = std::make_unique<cPushButton> (getPosition () + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, SoundData.SNDMenuButton, lngPack.i18n ("Text~Others~Exit"));
	signalConnectionManager.connect (exitButton->clicked, std::bind (&cWindowStart::exitClicked, this));
	addChild (std::move (exitButton));
}

//------------------------------------------------------------------------------
cWindowStart::~cWindowStart ()
{}

//------------------------------------------------------------------------------
void cWindowStart::handleActivated (cApplication& application)
{
	cWindow::handleActivated (application);

	if(firstActivate) PlayMusic ((cSettings::getInstance ().getMusicPath () + PATH_DELIMITER + "main.ogg").c_str ());
	firstActivate = false;
}

//------------------------------------------------------------------------------
void cWindowStart::singlePlayerClicked ()
{
	if (!getActiveApplication()) return;

	auto window = std::make_shared<cWindowSinglePlayer> ();
	getActiveApplication ()->show (window);
}

//------------------------------------------------------------------------------
void cWindowStart::multiPlayerClicked ()
{
	if (!getActiveApplication ()) return;

	auto window = std::make_shared<cWindowMultiPlayer> ();
	getActiveApplication ()->show (window);
}

//------------------------------------------------------------------------------
void cWindowStart::preferencesClicked ()
{
	if (!getActiveApplication ()) return;

	auto dialog = std::make_shared<cDialogPreferences> ();
	getActiveApplication ()->show (dialog);
}

//------------------------------------------------------------------------------
void cWindowStart::licenceClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowStart::exitClicked ()
{
	close ();
}
