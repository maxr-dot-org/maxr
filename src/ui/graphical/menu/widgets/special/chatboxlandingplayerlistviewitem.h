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

#ifndef ui_graphical_menu_widgets_special_chatboxlandingplayerlistviewitemH
#define ui_graphical_menu_widgets_special_chatboxlandingplayerlistviewitemH

#include "game/data/player/playerbasicdata.h"
#include "ui/graphical/menu/widgets/abstractlistviewitem.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cImage;
class cLabel;
class cPlayerBasicData;
class cLandingPositionManager;

class cPlayerLandingStatus
{
public:
	explicit cPlayerLandingStatus (const cPlayerBasicData&);

	const cPlayerBasicData& getPlayer() const;

	bool hasSelectedPosition() const;
	void setHasSelectedPosition (bool value);

	mutable cSignal<void()> hasSelectedPositionChanged;

private:
	cPlayerBasicData player;
	bool selectedPosition = false;
};

class cChatBoxLandingPlayerListViewItem : public cAbstractListViewItem
{
public:
	explicit cChatBoxLandingPlayerListViewItem (const cPlayerLandingStatus&);

	int getPlayerNumber() const;

	void setLandingPositionManager (const cLandingPositionManager*);

	void handleResized (const cPosition& oldSize) override;

private:
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager managerSignalConnectionManager;

	cLabel* nameLabel;
	cImage* colorImage;
	cImage* readyImage;

	const cPlayerLandingStatus& playerLandingStatus;

	const cLandingPositionManager* landingPositionManager = nullptr;

	void updatePlayerName();
	void updatePlayerColor();
	void updatePlayerHasSelectedPosition();
};

#endif // ui_graphical_menu_widgets_special_chatboxlandingplayerlistviewitemH
