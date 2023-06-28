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

#include "windowstart.h"

#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "settings.h"
#include "ui/graphical/intro.h"
#include "ui/graphical/menu/dialogs/dialoglicense.h"
#include "ui/graphical/menu/dialogs/dialogpreferences.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/windows/windowmultiplayer.h"
#include "ui/graphical/menu/windows/windowsingleplayer.h"
#include "ui/widgets/application.h"
#include "utility/language.h"

#include <functional>

//------------------------------------------------------------------------------
cWindowStart::cWindowStart() :
	cWindowMain (lngPack.i18n ("Title~MainMenu"))
{
	singlePlayerButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Others~Single_Player"));
	signalConnectionManager.connect (singlePlayerButton->clicked, [this]() { singlePlayerClicked(); });

	multiPlayerButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Others~Multi_Player"));
	signalConnectionManager.connect (multiPlayerButton->clicked, [this]() { multiPlayerClicked(); });

	preferencesButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace * 2), ePushButtonType::StandardBig, lngPack.i18n ("Settings~Preferences"));
	signalConnectionManager.connect (preferencesButton->clicked, [this]() { preferencesClicked(); });

	introButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace * 3), ePushButtonType::StandardBig, lngPack.i18n ("Settings~Intro"));
	signalConnectionManager.connect (introButton->clicked, [this]() { showIntro(); });
	if (!hasIntro())
	{
		introButton->lock();
	}

	licenseButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace * 4), ePushButtonType::StandardBig, lngPack.i18n ("Others~Mani"));
	signalConnectionManager.connect (licenseButton->clicked, [this]() { licenceClicked(); });

	exitButton = emplaceChild<cPushButton> (getPosition() + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, &SoundData.SNDMenuButton, lngPack.i18n ("Others~Exit"));
	signalConnectionManager.connect (exitButton->clicked, [this]() { exitClicked(); });
}

//------------------------------------------------------------------------------
cWindowStart::~cWindowStart()
{}

//------------------------------------------------------------------------------
void cWindowStart::retranslate()
{
	cWindowMain::retranslate();
	setTitle (lngPack.i18n ("Title~MainMenu"));

	singlePlayerButton->setText (lngPack.i18n ("Others~Single_Player"));
	multiPlayerButton->setText (lngPack.i18n ("Others~Multi_Player"));
	preferencesButton->setText (lngPack.i18n ("Settings~Preferences"));
	introButton->setText (lngPack.i18n ("Settings~Intro"));
	licenseButton->setText (lngPack.i18n ("Others~Mani"));
	exitButton->setText (lngPack.i18n ("Others~Exit"));
}

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
