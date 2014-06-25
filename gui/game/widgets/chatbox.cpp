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
cChatBox::cChatBox (const cBox<cPosition>& area) :
	cWidget (area)
{
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

	createBackground ();
}

//------------------------------------------------------------------------------
void cChatBox::draw ()
{
	auto application = getActiveApplication ();

	const bool hasKeyFocus = application && (application->hasKeyFocus (*chatLineEdit)/* ||
											 application->hasKeyFocus (*chatList) ||
											 application->hasKeyFocus (*playersList)*/);

	auto background = hasKeyFocus ? focusBackground.get () : nonFocusBackground.get ();

	if (background != nullptr)
	{
		auto positionRect = getArea ().toSdlRect ();
		SDL_BlitSurface (background, NULL, cVideo::buffer, &positionRect);
	}

	cWidget::draw ();
}

//------------------------------------------------------------------------------
void cChatBox::handleResized (const cPosition& oldSize)
{
	cWidget::handleResized (oldSize);

	chatList->setArea (cBox<cPosition> (getPosition (), cPosition (getEndPosition ().x () - 162, getEndPosition ().y () - 16)));

	chatLineEdit->setArea (cBox<cPosition> (cPosition (getPosition ().x () + 2, getEndPosition ().y () - 12), cPosition (getEndPosition ().x () - 164, getEndPosition ().y () - 2)));

	playersList->setArea(cBox<cPosition> (cPosition (getEndPosition ().x () - 158, getPosition ().y ()), getEndPosition ()));

	createBackground ();
}

//------------------------------------------------------------------------------
void cChatBox::clearPlayers ()
{
	playersList->clearItems ();
}

//------------------------------------------------------------------------------
void cChatBox::addPlayer (const cPlayer& player)
{
	playersList->addItem (std::make_unique<cChatBoxPlayerListViewItem> (player));
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
	auto newItem = chatList->addItem (std::make_unique<cLobbyChatBoxListViewItem> (player.getName (), message));
	chatList->scrollToItem (newItem);
}

//------------------------------------------------------------------------------
void cChatBox::focus ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	application->grapKeyFocus (*chatLineEdit);
}

//------------------------------------------------------------------------------
void cChatBox::sendCommand ()
{
	commandEntered (chatLineEdit->getText ());
	chatLineEdit->setText ("");
}

//------------------------------------------------------------------------------
void cChatBox::createBackground ()
{
	const auto& size = getSize ();
    nonFocusBackground = AutoSurface (SDL_CreateRGBSurface (0, size.x (), size.y (), Video.getColDepth (), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
    focusBackground = AutoSurface (SDL_CreateRGBSurface (0, size.x (), size.y (), Video.getColDepth (), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));

	SDL_Rect chatListBackgroundRect = {chatList->getPosition ().x () - getPosition ().x (), chatList->getPosition ().y () - getPosition ().y (), chatList->getSize ().x (), chatList->getSize ().y ()};
	SDL_FillRect (nonFocusBackground.get (), &chatListBackgroundRect, SDL_MapRGBA (nonFocusBackground->format, 0, 0, 0, 50));
	SDL_FillRect (focusBackground.get (), &chatListBackgroundRect, SDL_MapRGBA (focusBackground->format, 0, 0, 0, 100));

	SDL_Rect chatLineEditBackgroundRect = {chatLineEdit->getPosition ().x () - getPosition ().x () - 2, chatLineEdit->getPosition ().y () - getPosition ().y () - 2, chatLineEdit->getSize ().x () + 4, chatLineEdit->getSize ().y () + 4};
	SDL_FillRect (nonFocusBackground.get (), &chatLineEditBackgroundRect, SDL_MapRGBA (nonFocusBackground->format, 0, 0, 0, 50));
	SDL_FillRect (focusBackground.get (), &chatLineEditBackgroundRect, SDL_MapRGBA (focusBackground->format, 0, 0, 0, 100));

	SDL_Rect playerListBackgroundRect = {playersList->getPosition ().x () - getPosition ().x (), playersList->getPosition ().y () - getPosition ().y (), playersList->getSize ().x (), playersList->getSize ().y ()};
	SDL_FillRect (nonFocusBackground.get (), &playerListBackgroundRect, SDL_MapRGBA (nonFocusBackground->format, 0, 0, 0, 50));
	SDL_FillRect (focusBackground.get (), &playerListBackgroundRect, SDL_MapRGBA (focusBackground->format, 0, 0, 0, 100));
}