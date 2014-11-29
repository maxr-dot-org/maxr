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

#ifndef ui_graphical_game_widgets_chatboxH
#define ui_graphical_game_widgets_chatboxH

#include <memory>

#include "ui/graphical/widget.h"

#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/listview.h"

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cPosition;

template<typename T>
class cBox;

class cLineEdit;

template<typename T> class cListView;
class cLobbyChatBoxListViewItem;
class cChatBoxPlayerListViewItem;

class cPlayer;

template<typename ChatListItemType, typename PlayerListItemType>
class cChatBox : public cWidget
{
public:
	cChatBox (const cBox<cPosition>& area);

	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;

	virtual void handleResized (const cPosition& oldSize) MAXR_OVERRIDE_FUNCTION;

	void clearPlayers ();

	void addPlayerEntry (std::unique_ptr<PlayerListItemType> player);

	const PlayerListItemType* getPlayerEntryFromNumber (int playerNumber) const;
	PlayerListItemType* getPlayerEntryFromNumber (int playerNumber);

	void removePlayerEntry (int playerNumber);

	void addChatEntry (std::unique_ptr<ChatListItemType> entry);

	void focus ();

	cSignal<void (const std::string&)> commandEntered;
private:
	cSignalConnectionManager signalConnectionManager;

	AutoSurface nonFocusBackground;
	AutoSurface focusBackground;

	cLineEdit* chatLineEdit;

	cListView<ChatListItemType>* chatList;

	cListView<PlayerListItemType>* playersList;

	void sendCommand ();

	void createBackground ();
};

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
cChatBox<ChatListItemType, PlayerListItemType>::cChatBox (const cBox<cPosition>& area) :
	cWidget (area)
{
	chatList = addChild (std::make_unique<cListView<ChatListItemType>> (cBox<cPosition> (getPosition (), cPosition (getEndPosition ().x () - 162, getEndPosition ().y () - 16)), eScrollBarStyle::Modern));
	chatList->disableSelectable ();
	chatList->setBeginMargin (cPosition (2, 2));
	chatList->setEndMargin (cPosition (2, 2));
	chatList->setScrollOffset (font->getFontHeight () + 3);

	chatLineEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (cPosition (getPosition ().x () + 2, getEndPosition ().y () - 12), cPosition (getEndPosition ().x () - 164, getEndPosition ().y () - 2))));
	signalConnectionManager.connect (chatLineEdit->returnPressed, std::bind (&cChatBox::sendCommand, this));

	playersList = addChild (std::make_unique<cListView<PlayerListItemType>> (cBox<cPosition> (cPosition (getEndPosition ().x () - 158, getPosition ().y ()), getEndPosition ()), eScrollBarStyle::Modern));
	playersList->disableSelectable ();
	playersList->setBeginMargin (cPosition (2, 2));
	playersList->setEndMargin (cPosition (2, 2));
	playersList->setItemDistance (4);

	createBackground ();
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	auto application = getActiveApplication ();

	const bool hasKeyFocus = application && (application->hasKeyFocus (*chatLineEdit)/* ||
																					 application->hasKeyFocus (*chatList) ||
																					 application->hasKeyFocus (*playersList)*/);

	auto background = hasKeyFocus ? focusBackground.get () : nonFocusBackground.get ();

	if (background != nullptr)
	{
		auto positionRect = getArea ().toSdlRect ();
		SDL_BlitSurface (background, nullptr, &destination, &positionRect);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::handleResized (const cPosition& oldSize)
{
	cWidget::handleResized (oldSize);

	chatList->setArea (cBox<cPosition> (getPosition (), cPosition (getEndPosition ().x () - 162, getEndPosition ().y () - 16)));

	chatLineEdit->setArea (cBox<cPosition> (cPosition (getPosition ().x () + 2, getEndPosition ().y () - 12), cPosition (getEndPosition ().x () - 164, getEndPosition ().y () - 2)));

	playersList->setArea (cBox<cPosition> (cPosition (getEndPosition ().x () - 158, getPosition ().y ()), getEndPosition ()));

	createBackground ();
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::clearPlayers ()
{
	playersList->clearItems ();
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::addPlayerEntry (std::unique_ptr<PlayerListItemType> player)
{
	playersList->addItem (std::move(player));
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
PlayerListItemType* cChatBox<ChatListItemType, PlayerListItemType>::getPlayerEntryFromNumber (int playerNumber)
{
	const cChatBox<ChatListItemType, PlayerListItemType>& constMe = *this;
	return const_cast<PlayerListItemType*>(constMe.getPlayerEntryFromNumber(playerNumber));
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
const PlayerListItemType* cChatBox<ChatListItemType, PlayerListItemType>::getPlayerEntryFromNumber (int playerNumber) const
{
	for (size_t i = 0; i < playersList->getItemsCount (); ++i)
	{
		if (playersList->getItem (i).getPlayerNumber () == playerNumber)
		{
			return &playersList->getItem (i);
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::removePlayerEntry (int playerNumber)
{
	for (size_t i = 0; i < playersList->getItemsCount (); ++i)
	{
		if (playersList->getItem (i).getPlayerNumber () == playerNumber)
		{
			playersList->removeItem (playersList->getItem (i));
		}
	}
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::addChatEntry (std::unique_ptr<ChatListItemType> entry)
{
	auto newItem = chatList->addItem (std::move (entry));
	chatList->scrollToItem (newItem);
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::focus ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	application->grapKeyFocus (*chatLineEdit);
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::sendCommand ()
{
	commandEntered (chatLineEdit->getText ());
	chatLineEdit->setText ("");
}

//------------------------------------------------------------------------------
template<typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::createBackground ()
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

#endif // ui_graphical_game_widgets_chatboxH
