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

#include "ui/graphical/menu/dialogs/dialogselfdestruction.h"

#include "game/data/units/unit.h"
#include "resources/pcx.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/protectionglass.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cDialogSelfDestruction::cDialogSelfDestruction (const cUnit& unit, std::shared_ptr<cAnimationTimer> animationTimer) :
	cWindow (LoadPCX (GFXOD_DESTRUCTION), eWindowBackgrounds::Alpha)
{
	armButton = emplaceChild<cPushButton> (getPosition() + cPosition (88, 14), ePushButtonType::Angular, lngPack.i18n ("Others~Hot"), eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (armButton->clicked, [this]() { armcClicked(); });

	destroyButton = emplaceChild<cPushButton> (getPosition() + cPosition (15, 13), ePushButtonType::Destroy);
	destroyButton->lock();
	signalConnectionManager.connect (destroyButton->clicked, [this]() { triggeredDestruction(); });

	cancelButton = emplaceChild<cPushButton> (getPosition() + cPosition (88, 46), ePushButtonType::Angular, lngPack.i18n ("Others~Cancel"), eUnicodeFontType::LatinNormal);
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, [this]() { close(); });

	protectionGlass = emplaceChild<cProtectionGlass> (getPosition() + cPosition (15, 13), std::move (animationTimer));
	signalConnectionManager.connect (protectionGlass->opened, [this]() {
		destroyButton->unlock();
	});

	signalConnectionManager.connect (unit.destroyed, [this]() { closeOnUnitDestruction(); });
}

//------------------------------------------------------------------------------
cDialogSelfDestruction::~cDialogSelfDestruction()
{}

//------------------------------------------------------------------------------
void cDialogSelfDestruction::retranslate()
{
	cWindow::retranslate();

	armButton->setText (lngPack.i18n ("Others~Hot"));
	cancelButton->setText (lngPack.i18n ("Others~Cancel"));
}

//------------------------------------------------------------------------------
void cDialogSelfDestruction::armcClicked()
{
	protectionGlass->open();
	armButton->lock();
}

//------------------------------------------------------------------------------
void cDialogSelfDestruction::closeOnUnitDestruction()
{
	if (isClosing()) return;

	close();
	auto application = getActiveApplication();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Others~Unit_destroyed")));
	}
}
