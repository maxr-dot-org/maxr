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

class cChatBox : public cWidget
{
public:
	cChatBox (const cBox<cPosition>& area);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual void handleResized (const cPosition& oldSize) MAXR_OVERRIDE_FUNCTION;

	void clearPlayers ();

	void addPlayer (const cPlayer& player);

	const cPlayer* getPlayerFromNumber (int playerNumber);

	void addChatMessage (const cPlayer& player, const std::string& message);
	void addChatMessage (const std::string& playerName, const std::string& message);

	void focus ();

	cSignal<void (const std::string)> commandEntered;
private:
	cSignalConnectionManager signalConnectionManager;

	AutoSurface nonFocusBackground;
	AutoSurface focusBackground;

	cLineEdit* chatLineEdit;

	cListView<cLobbyChatBoxListViewItem>* chatList;

	cListView<cChatBoxPlayerListViewItem>* playersList;

	void sendCommand ();

	void createBackground ();
};

#endif // ui_graphical_game_widgets_chatboxH
