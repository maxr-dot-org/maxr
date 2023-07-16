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

#include "dialogok.h"

#include "resources/pcx.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/uidefines.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cDialogOk::cDialogOk (const std::string& text, eWindowBackgrounds backgroundType) :
	cWindow (LoadPCX (GFXOD_DIALOG2), backgroundType)
{
	auto textLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 35), getPosition() + cPosition (267, 173)), text, eUnicodeFontType::LatinNormal, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
	textLabel->setWordWrap (true);

	auto okButton = emplaceChild<cPushButton> (getPosition() + cPosition (111, 185), ePushButtonType::Angular, lngPack.i18n ("Others~OK"), eUnicodeFontType::LatinNormal);
	okButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_RETURN)));
	signalConnectionManager.connect (okButton->clicked, [this]() { okClicked(); });
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
