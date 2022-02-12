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

#include "ui/graphical/menu/windows/windowstart.h"

#include "defines.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"
#include "settings.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/dialogs/dialoglicense.h"
#include "ui/graphical/menu/dialogs/dialogpreferences.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/windows/windowsingleplayer.h"
#include "ui/graphical/menu/windows/windowmultiplayer.h"
#include "utility/language.h"

#include <functional>

//------------------------------------------------------------------------------
cWindowStart::cWindowStart() :
	cWindowMain (lngPack.i18n ("Text~Title~MainMenu"))
{
	auto singlePlayerButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Single_Player")));
	signalConnectionManager.connect (singlePlayerButton->clicked, [this]() { singlePlayerClicked(); });

	auto multiPlayerButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Multi_Player")));
	signalConnectionManager.connect (multiPlayerButton->clicked, [this]() { multiPlayerClicked(); });

	auto preferencesButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace * 2), ePushButtonType::StandardBig, lngPack.i18n ("Text~Settings~Preferences")));
	signalConnectionManager.connect (preferencesButton->clicked, [this]() { preferencesClicked(); });

	auto licenseButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace * 3), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Mani")));
	signalConnectionManager.connect (licenseButton->clicked, [this]() { licenceClicked(); });

	auto exitButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, &SoundData.SNDMenuButton, lngPack.i18n ("Text~Others~Exit")));
	signalConnectionManager.connect (exitButton->clicked, [this]() { exitClicked(); });
}

//------------------------------------------------------------------------------
cWindowStart::~cWindowStart()
{}

//------------------------------------------------------------------------------
void cWindowStart::handleActivated (cApplication& application, bool firstTime)
{
	cWindow::handleActivated (application, firstTime);

	if (firstTime) cSoundDevice::getInstance().startMusic (MusicFiles.start);
}

//------------------------------------------------------------------------------
void cWindowStart::singlePlayerClicked()
{
	if (!getActiveApplication()) return;

	auto window = std::make_shared<cWindowSinglePlayer>();
	getActiveApplication()->show (window);
}

//------------------------------------------------------------------------------
void cWindowStart::multiPlayerClicked()
{
	if (!getActiveApplication()) return;

	auto window = std::make_shared<cWindowMultiPlayer>();
	getActiveApplication()->show (window);
}

//------------------------------------------------------------------------------
void cWindowStart::preferencesClicked()
{
	if (!getActiveApplication()) return;

	auto dialog = std::make_shared<cDialogPreferences>();
	getActiveApplication()->show (dialog);
}

//------------------------------------------------------------------------------
void cWindowStart::licenceClicked()
{
	if (!getActiveApplication()) return;

	auto dialog = std::make_shared<cDialogLicense>();
	getActiveApplication()->show (dialog);
}

//------------------------------------------------------------------------------
void cWindowStart::exitClicked()
{
	close();
}
