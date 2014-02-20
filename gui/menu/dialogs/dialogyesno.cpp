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
cDialogNewYesNo::cDialogNewYesNo (const std::string& text) :
	cWindow (LoadPCX (GFXOD_DIALOG2), eWindowBackgrounds::Alpha),
	result (eYesNoDialogResultType::None)
{
	const auto& menuPosition = getArea ().getMinCorner ();

	auto textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (40, 40), menuPosition + cPosition (135, 267)), text, FONT_LATIN_NORMAL, toEnumFlag(eAlignmentType::CenterHorizontal) | eAlignmentType::Top));
	textLabel->setWordWrap (true);

	auto yesButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (155, 185), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Yes"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (yesButton->clicked, std::bind (&cDialogNewYesNo::yesClicked, this));
	// FIXME: add hot key RETURN to button

	auto noButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (67, 185), ePushButtonType::Angular, lngPack.i18n ("Text~Others~No"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (noButton->clicked, std::bind (&cDialogNewYesNo::noClicked, this));
	// FIXME: add hot key ESCAPE to button
}

//------------------------------------------------------------------------------
eYesNoDialogResultType cDialogNewYesNo::getResult () const
{
	return result;
}

//------------------------------------------------------------------------------
cDialogNewYesNo::~cDialogNewYesNo ()
{}

//------------------------------------------------------------------------------
void cDialogNewYesNo::yesClicked ()
{
	result = eYesNoDialogResultType::Yes;
	close ();
}

//------------------------------------------------------------------------------
void cDialogNewYesNo::noClicked ()
{
	result = eYesNoDialogResultType::No;
	close ();
}
