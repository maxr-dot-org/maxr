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

#include "dialogyesno.h"

#include "../widgets/label.h"
#include "../widgets/pushbutton.h"
#include "../../../pcx.h"
#include "../../../main.h"

//------------------------------------------------------------------------------
cDialogYesNo::cDialogYesNo (const std::string& text) :
	cWindow (LoadPCX (GFXOD_DIALOG2), eWindowBackgrounds::Alpha)
{
	auto textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (35, 35), getPosition () + cPosition (267, 173)), text, FONT_LATIN_NORMAL, toEnumFlag(eAlignmentType::CenterHorizontal) | eAlignmentType::Top));
	textLabel->setWordWrap (true);

	auto yesButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (155, 185), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Yes"), FONT_LATIN_NORMAL));
	yesButton->addClickShortcut (cKeySequence ("Return"));
	signalConnectionManager.connect (yesButton->clicked, [&]()
	{
		yesClicked ();
		close ();
	});

	auto noButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (67, 185), ePushButtonType::Angular, lngPack.i18n ("Text~Others~No"), FONT_LATIN_NORMAL));
	noButton->addClickShortcut (cKeySequence ("Esc"));
	signalConnectionManager.connect (noButton->clicked, [&]()
	{
		noClicked ();
		close ();
	});
}

//------------------------------------------------------------------------------
cDialogYesNo::~cDialogYesNo ()
{}
