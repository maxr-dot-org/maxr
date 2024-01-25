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

#ifndef ui_graphical_menu_control_menucontrollermultiplayerhotseatH
#define ui_graphical_menu_control_menucontrollermultiplayerhotseatH

#include "game/logic/landingpositionmanager.h"
#include "ui/widgets/runnable.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>
#include <vector>

class cApplication;
class cLocalHotSeatGameNew;
class cWindowPlayerSelection;
class cWindowLandingPositionSelection;
class cWindow;
class cLandingPositionManager;
class cPlayerBasicData;

class cMenuControllerMultiplayerHotSeat : public cRunnable, public std::enable_shared_from_this<cMenuControllerMultiplayerHotSeat>
{
public:
	cMenuControllerMultiplayerHotSeat (cApplication&);

	void start();
	void run() override {}

private:
	cSignalConnectionManager signalConnectionManager;

	cApplication& application;

	cWindowPlayerSelection* windowPlayerSelection = nullptr;
	cWindow* firstWindow = nullptr;

	std::shared_ptr<cLocalHotSeatGameNew> game;

	std::unique_ptr<cLandingPositionManager> landingPositionManager;

	std::vector<std::pair<size_t, eLandingPositionState>> invalidLandingPositionPlayers;
	std::vector<std::pair<size_t, eLandingPositionState>> nextInvalidLandingPositionPlayers;

	std::vector<std::shared_ptr<cWindowLandingPositionSelection>> playerLandingSelectionWindows;
	cSignalConnectionManager landingSelectionWindowConnections;

	void reset();

	void selectGameSettings();
	void selectMap();
	void selectPlayers();
	void startNextPlayerGamePreperation (size_t playerIndex);
	void selectClan (size_t playerIndex, bool firstForPlayer);
	void selectLandingUnits (size_t playerIndex, bool firstForPlayer);
	void selectLandingPosition (size_t playerIndex);
	void reselectLandingPosition (size_t playerIndex);
	void checkAllLandingPositions();
};

#endif // ui_graphical_menu_control_menucontrollermultiplayerhotseatH
