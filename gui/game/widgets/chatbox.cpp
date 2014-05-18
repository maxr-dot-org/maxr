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

#include "chatbox.h"

#include "../../menu/widgets/lineedit.h"
#include "../../menu/widgets/listview.h"
#include "../../menu/widgets/special/lobbychatboxlistviewitem.h"
#include "chatboxplayerlistviewitem.h"

#include "../../../player.h"

//------------------------------------------------------------------------------
cChatBox::cChatBox (const cBox<cPosition>& area)
{
    setArea (area);

    chatList = addChild (std::make_unique<cListView<cLobbyChatBoxListViewItem>> (cBox<cPosition> (getPosition (), cPosition (getEndPosition ().x () - 162, getEndPosition ().y () - 16))));
    chatList->disableSelectable ();
    chatList->setBeginMargin (cPosition (2, 2));
    chatList->setEndMargin (cPosition (2, 2));

    chatLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (cPosition (getPosition ().x () + 2, getEndPosition ().y () - 12), cPosition (getEndPosition ().x () - 164, getEndPosition ().y () - 2))));
    signalConnectionManager.connect (chatLineEdit->returnPressed, std::bind (&cChatBox::sendCommand, this));

    playersList = addChild (std::make_unique<cListView<cChatBoxPlayerListViewItem>> (cBox<cPosition> (cPosition (getEndPosition ().x () - 158, getPosition ().y ()), getEndPosition ())));
    playersList->disableSelectable ();
    playersList->setBeginMargin (cPosition (2, 2));
    playersList->setEndMargin (cPosition (2, 2));
    playersList->setItemDistance (cPosition (0, 4));
}

//------------------------------------------------------------------------------
void cChatBox::draw ()
{
    auto area1 = chatList->getArea ();
    auto rect1 = area1.toSdlRect ();
    Video.applyShadow (&rect1);

    auto area2 = chatLineEdit->getArea ();
    area2.getMinCorner () -= 2;
    area2.getMaxCorner () += 2;
    auto rect2 = area2.toSdlRect ();
    Video.applyShadow (&rect2);

    auto area3 = playersList->getArea ();
    auto rect3 = area3.toSdlRect ();
    Video.applyShadow (&rect3);

    cWidget::draw ();
}

//------------------------------------------------------------------------------
void cChatBox::clearPlayers ()
{
    playersList->clearItems ();
}

//------------------------------------------------------------------------------
void cChatBox::addPlayer (const cPlayer& player)
{
    playersList->addItem (std::make_unique<cChatBoxPlayerListViewItem> (player, playersList->getSize ().x () - playersList->getBeginMargin ().x () - playersList->getEndMargin ().x ()));
}

//------------------------------------------------------------------------------
const cPlayer* cChatBox::getPlayerFromNumber (int playerNumber)
{
	for (size_t i = 0; i < playersList->getItemsCount (); ++i)
	{
		if (playersList->getItem (i).getPlayer ().getNr () == playerNumber)
		{
			return &playersList->getItem (i).getPlayer ();
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
void cChatBox::addChatMessage (const cPlayer& player, const std::string& message)
{
    auto newItem = chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (player.getName(), message, chatList->getSize ().x () - chatList->getBeginMargin ().x () - chatList->getEndMargin ().x ()));
	chatList->scroolToItem (newItem);
}

//------------------------------------------------------------------------------
void cChatBox::sendCommand ()
{
    commandEntered (chatLineEdit->getText ());
    chatLineEdit->setText ("");
}
