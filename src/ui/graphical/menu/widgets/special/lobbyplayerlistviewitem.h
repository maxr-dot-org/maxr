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

#ifndef ui_graphical_menu_widgets_special_lobbyplayerlistviewitemH
#define ui_graphical_menu_widgets_special_lobbyplayerlistviewitemH

#include "ui/graphical/menu/widgets/abstractlistviewitem.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cImage;
class cLabel;
class cPlayerBasicData;

class cLobbyPlayerListViewItem : public cAbstractListViewItem
{
public:
	explicit cLobbyPlayerListViewItem (std::shared_ptr<cPlayerBasicData>);

	const std::shared_ptr<cPlayerBasicData>& getPlayer() const;
	void update();

	cSignal<void()> readyClicked;

	void handleResized (const cPosition& oldSize) override;

private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* nameLabel = nullptr;
	cImage* colorImage = nullptr;
	cImage* readyImage = nullptr;

	std::shared_ptr<cPlayerBasicData> player;

	void updatePlayerName();
	void updatePlayerColor();
	void updatePlayerReady();
};

#endif // ui_graphical_menu_widgets_special_lobbyplayerlistviewitemH
