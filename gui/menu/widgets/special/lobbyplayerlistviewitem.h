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

#ifndef gui_menu_widgets_special_lobbyplayerlistviewitemH
#define gui_menu_widgets_special_lobbyplayerlistviewitemH

#include <memory>

#include "../abstractlistviewitem.h"
#include "../../../../main.h"
#include "../../../../utility/signal/signal.h"
#include "../../../../utility/signal/signalconnectionmanager.h"

class cImage;
class cLabel;
class sPlayer;

class cLobbyPlayerListViewItem : public cAbstractListViewItem
{
public:
	cLobbyPlayerListViewItem (std::shared_ptr<sPlayer> player, int width);

	const std::shared_ptr<sPlayer>& getPlayer () const;

	cSignal<void ()> readyClicked;
private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* nameLabel;
	cImage* colorImage;
	cImage* readyImage;

	std::shared_ptr<sPlayer> player;

	void updatePlayerName ();
	void updatePlayerColor ();
	void updatePlayerReady ();
};

#endif // gui_menu_widgets_special_lobbyplayerlistviewitemH
