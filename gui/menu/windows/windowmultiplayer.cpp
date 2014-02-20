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

#include "windowmultiplayer.h"
#include "../../../main.h"
#include "../widgets/pushbutton.h"

//------------------------------------------------------------------------------
cWindowMultiPlayer::cWindowMultiPlayer () :
	cWindowMain (lngPack.i18n ("Text~Others~Multi_Player"))
{
	using namespace std::placeholders;

	const auto& menuPosition = getArea ().getMinCorner ();

	auto hastButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~TCPIP_Host")));
	signalConnectionManager.connect (hastButton->clicked, std::bind (&cWindowMultiPlayer::tcpHostClicked, this));

	auto clientButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~TCPIP_Client")));
	signalConnectionManager.connect (clientButton->clicked, std::bind (&cWindowMultiPlayer::tcpClientClicked, this));

#ifndef RELEASE
	auto newHotSeatButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 190 + buttonSpace * 2), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~HotSeat_New")));
	signalConnectionManager.connect (newHotSeatButton->clicked, std::bind (&cWindowMultiPlayer::newHotSeatClicked, this));

	auto loadHotSeatButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 190 + buttonSpace * 3), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~HotSeat_Load")));
	signalConnectionManager.connect (loadHotSeatButton->clicked, std::bind (&cWindowMultiPlayer::loadHotSeatClicked, this));
#endif

	auto backButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowMultiPlayer::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowMultiPlayer::~cWindowMultiPlayer ()
{}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::tcpHostClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::tcpClientClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::newHotSeatClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::loadHotSeatClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowMultiPlayer::backClicked ()
{
	close ();
}