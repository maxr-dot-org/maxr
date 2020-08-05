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

#include "ui/graphical/menu/dialogs/dialogok.h"

#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "utility/language.h"
#include "utility/pcx.h"

//------------------------------------------------------------------------------
cDialogOk::cDialogOk (const std::string& text, eWindowBackgrounds backgroundType) :
	cWindow (LoadPCX (GFXOD_DIALOG2), backgroundType)
{
	auto textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 35), getPosition() + cPosition (267, 173)), text, FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top));
	textLabel->setWordWrap (true);

	auto okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (111, 185), ePushButtonType::Angular, lngPack.i18n ("Text~Others~OK"), FONT_LATIN_NORMAL));
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cDialogOk::okClicked, this));
}

//------------------------------------------------------------------------------
cDialogOk::~cDialogOk()
{}

//------------------------------------------------------------------------------
void cDialogOk::okClicked()
{
	done();
	close();
}
