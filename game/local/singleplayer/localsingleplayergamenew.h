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

#ifndef game_local_singleplayer_localsingleplayergamenewH
#define game_local_singleplayer_localsingleplayergamenewH

#include <memory>
#include <vector>
#include <utility>

#include "localsingleplayergame.h"
#include "../../../maxrconfig.h"
#include "../../../utility/signal/signalconnectionmanager.h"
#include "../../../utility/position.h"

class cApplication;
class cStaticMap;
class cGameSettings;
class sPlayer;
class cPlayer;
class cPosition;
class cUnitUpgrade;

struct sLandingUnit;
struct sID;

class cLocalSingleplayerGameNew : public cLocalSingleplayerGame
{
public:
	cLocalSingleplayerGameNew ();

	void start (cApplication& application);

	void setGameSettings (std::shared_ptr<cGameSettings> gameSettings);

	void setStaticMap (std::shared_ptr<cStaticMap> staticMap);

	void setPlayerClan (int clan);

	void setLandingUnits (std::vector<sLandingUnit> landingUnits);

	void setUnitUpgrades (std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades);

	void setLandingPosition (const cPosition& landingPosition);

	sPlayer createPlayer ();
private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;

	int playerClan;

	std::vector<sLandingUnit> landingUnits;
	std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades;
	cPosition landingPosition;
};

#endif // game_local_singleplayer_localsingleplayergamenewH
