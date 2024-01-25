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

#include "SDLutility/tosdl.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/widgets/lineedit.h"
#include "ui/widgets/widget.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cPosition;

template <typename T>
class cBox;

class cLineEdit;

template <typename T>
class cListView;
class cLobbyChatBoxListViewItem;
class cChatBoxPlayerListViewItem;

class cPlayer;

template <typename ChatListItemType, typename PlayerListItemType>
class cChatBox : public cWidget
{
public:
	cChatBox (const cBox<cPosition>& area);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	void handleResized (const cPosition& oldSize) override;

	void clearPlayers();

	PlayerListItemType* addPlayerEntry (std::unique_ptr<PlayerListItemType> player);

	const PlayerListItemType* getPlayerEntryFromNumber (int playerNumber) const;
	PlayerListItemType* getPlayerEntryFromNumber (int playerNumber);

	void removePlayerEntry (int playerNumber);

	ChatListItemType* addChatEntry (std::unique_ptr<ChatListItemType> entry);

	void scrollToItem (ChatListItemType* item) { chatList->scrollToItem (item); }

	void focus();

	cSignal<void (const std::string&)> commandEntered;

private:
	void sendCommand();
	void createBackground();

private:
	cSignalConnectionManager signalConnectionManager;

	UniqueSurface nonFocusBackground;
	UniqueSurface focusBackground;

	cLineEdit* chatLineEdit = nullptr;
	cListView<ChatListItemType>* chatList = nullptr;
	cListView<PlayerListItemType>* playersList = nullptr;
};

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
cChatBox<ChatListItemType, PlayerListItemType>::cChatBox (const cBox<cPosition>& area) :
	cWidget (area)
{
	chatList = emplaceChild<cListView<ChatListItemType>> (cBox<cPosition> (getPosition(), cPosition (getEndPosition().x() - 162, getEndPosition().y() - 16)), eScrollBarStyle::Modern);
	chatList->disableSelectable();
	chatList->setBeginMargin (cPosition (2, 2));
	chatList->setEndMargin (cPosition (2, 2));
	chatList->setScrollOffset (cUnicodeFont::font->getFontHeight() + 3);

	chatLineEdit = emplaceChild<cLineEdit> (cBox<cPosition> (cPosition (getPosition().x() + 2, getEndPosition().y() - 12), cPosition (getEndPosition().x() - 164, getEndPosition().y() - 2)));
	signalConnectionManager.connect (chatLineEdit->returnPressed, [this]() { sendCommand(); });

	playersList = emplaceChild<cListView<PlayerListItemType>> (cBox<cPosition> (cPosition (getEndPosition().x() - 158, getPosition().y()), getEndPosition()), eScrollBarStyle::Modern);
	playersList->disableSelectable();
	playersList->setBeginMargin (cPosition (2, 2));
	playersList->setEndMargin (cPosition (2, 2));
	playersList->setItemDistance (4);

	createBackground();
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	auto application = getActiveApplication();

	const bool hasKeyFocus = application && (application->hasKeyFocus (*chatLineEdit) /* ||
																					 application->hasKeyFocus (*chatList) ||
																					 application->hasKeyFocus (*playersList)*/
	                         );

	auto background = hasKeyFocus ? focusBackground.get() : nonFocusBackground.get();

	if (background != nullptr)
	{
		auto positionRect = toSdlRect (getArea());
		SDL_BlitSurface (background, nullptr, &destination, &positionRect);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::handleResized (const cPosition& oldSize)
{
	cWidget::handleResized (oldSize);

	chatList->setArea (cBox<cPosition> (getPosition(), cPosition (getEndPosition().x() - 162, getEndPosition().y() - 16)));

	chatLineEdit->setArea (cBox<cPosition> (cPosition (getPosition().x() + 2, getEndPosition().y() - 12), cPosition (getEndPosition().x() - 164, getEndPosition().y() - 2)));

	playersList->setArea (cBox<cPosition> (cPosition (getEndPosition().x() - 158, getPosition().y()), getEndPosition()));

	createBackground();
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::clearPlayers()
{
	playersList->clearItems();
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
PlayerListItemType* cChatBox<ChatListItemType, PlayerListItemType>::addPlayerEntry (std::unique_ptr<PlayerListItemType> player)
{
	return playersList->addItem (std::move (player));
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
PlayerListItemType* cChatBox<ChatListItemType, PlayerListItemType>::getPlayerEntryFromNumber (int playerNumber)
{
	const cChatBox<ChatListItemType, PlayerListItemType>& constMe = *this;
	return const_cast<PlayerListItemType*> (constMe.getPlayerEntryFromNumber (playerNumber));
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
const PlayerListItemType* cChatBox<ChatListItemType, PlayerListItemType>::getPlayerEntryFromNumber (int playerNumber) const
{
	for (size_t i = 0; i < playersList->getItemsCount(); ++i)
	{
		if (playersList->getItem (i).getPlayerNumber() == playerNumber)
		{
			return &playersList->getItem (i);
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::removePlayerEntry (int playerNumber)
{
	for (size_t i = 0; i < playersList->getItemsCount(); ++i)
	{
		if (playersList->getItem (i).getPlayerNumber() == playerNumber)
		{
			playersList->removeItem (playersList->getItem (i));
		}
	}
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
ChatListItemType* cChatBox<ChatListItemType, PlayerListItemType>::addChatEntry (std::unique_ptr<ChatListItemType> entry)
{
	return chatList->addItem (std::move (entry), eAddListItemScrollType::IfAtBottom);
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::focus()
{
	auto application = getActiveApplication();
	if (!application) return;

	application->grapKeyFocus (*chatLineEdit);
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::sendCommand()
{
	commandEntered (chatLineEdit->getText());
	chatLineEdit->setText ("");
}

//------------------------------------------------------------------------------
template <typename ChatListItemType, typename PlayerListItemType>
void cChatBox<ChatListItemType, PlayerListItemType>::createBackground()
{
	const auto& size = getSize();
	nonFocusBackground = UniqueSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
	focusBackground = UniqueSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));

	const auto black_50 = toSdlAlphaColor (cRgbColor::black (50), *nonFocusBackground);
	const auto black_100 = toSdlAlphaColor (cRgbColor::black (100), *focusBackground);

	SDL_Rect chatListBackgroundRect = {chatList->getPosition().x() - getPosition().x(), chatList->getPosition().y() - getPosition().y(), chatList->getSize().x(), chatList->getSize().y()};
	SDL_FillRect (nonFocusBackground.get(), &chatListBackgroundRect, black_50);
	SDL_FillRect (focusBackground.get(), &chatListBackgroundRect, black_100);

	SDL_Rect chatLineEditBackgroundRect = {chatLineEdit->getPosition().x() - getPosition().x() - 2, chatLineEdit->getPosition().y() - getPosition().y() - 2, chatLineEdit->getSize().x() + 4, chatLineEdit->getSize().y() + 4};
	SDL_FillRect (nonFocusBackground.get(), &chatLineEditBackgroundRect, black_50);
	SDL_FillRect (focusBackground.get(), &chatLineEditBackgroundRect, black_100);

	SDL_Rect playerListBackgroundRect = {playersList->getPosition().x() - getPosition().x(), playersList->getPosition().y() - getPosition().y(), playersList->getSize().x(), playersList->getSize().y()};
	SDL_FillRect (nonFocusBackground.get(), &playerListBackgroundRect, black_50);
	SDL_FillRect (focusBackground.get(), &playerListBackgroundRect, black_100);
}

#endif // ui_graphical_game_widgets_chatboxH
