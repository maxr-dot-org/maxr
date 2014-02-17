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

#include "windowclanselection.h"
#include "../../../../main.h"
#include "../../../../pcx.h"
#include "../../widgets/label.h"
#include "../../widgets/pushbutton.h"

//------------------------------------------------------------------------------
cWindowClanSelection::cWindowClanSelection () :
	cWindow (LoadPCX (GFXOD_CLAN_SELECT))
{
	const auto& menuPosition = getArea ().getMinCorner ();

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (0, 13), menuPosition + cPosition (getArea ().getMaxCorner ().x (), 23)), lngPack.i18n ("Text~Button~Game_Options"), FONT_LATIN_NORMAL, eAlignmentType::Center));
	//
	// Buttons
	//
	auto okButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Button~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowClanSelection::okClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Button~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowClanSelection::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowClanSelection::~cWindowClanSelection ()
{}

//------------------------------------------------------------------------------
void cWindowClanSelection::okClicked ()
{
	done ();
}

//------------------------------------------------------------------------------
void cWindowClanSelection::backClicked ()
{
	close ();
}
