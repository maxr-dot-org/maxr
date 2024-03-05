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

#ifndef ui_graphical_menu_control_local_singleplayer_localsingleplayergamesavedH
#define ui_graphical_menu_control_local_singleplayer_localsingleplayergamesavedH

#include "ui/graphical/menu/control/local/singleplayer/localsingleplayergame.h"
#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>
#include <utility>
#include <vector>

class cApplication;

class cLocalSingleplayerGameSaved : public cLocalSingleplayerGame
{
public:
	explicit cLocalSingleplayerGameSaved (int saveGameNumber) :
		saveGameNumber{saveGameNumber}
	{}
	void start (cApplication& application);

private:
	cSignalConnectionManager signalConnectionManager;

	int saveGameNumber;
};

#endif
