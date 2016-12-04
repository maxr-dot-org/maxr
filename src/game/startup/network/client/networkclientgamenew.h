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

#ifndef game_startup_network_client_networkclientgamenewH
#define game_startup_network_client_networkclientgamenewH

#include <memory>
#include <vector>
#include <utility>

#include "game/startup/network/client/networkclientgame.h"
#include "maxrconfig.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/position.h"

class cApplication;
class cStaticMap;
class cGameSettings;
class cPlayerBasicData;
class cPlayer;
class cPosition;
class cUnitUpgrade;

struct sLandingUnit;
struct sID;

class cNetworkClientGameNew : public cNetworkClientGame
{
public:
	cNetworkClientGameNew();

	void start (cApplication& application);

	void setPlayers (std::vector<cPlayerBasicData> players, const cPlayerBasicData& localPlayer);

	void setGameSettings (std::shared_ptr<cGameSettings> gameSettings);

	void setStaticMap (std::shared_ptr<cStaticMap> staticMap);

	void setLocalPlayerClan (int clan);

	void setLocalPlayerLandingUnits (std::vector<sLandingUnit> landingUnits);

	void setLocalPlayerUnitUpgrades (std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades);

	void setLocalPlayerLandingPosition (const cPosition& landingPosition);

	const std::shared_ptr<cGameSettings>& getGameSettings();
	const std::shared_ptr<cStaticMap>& getStaticMap();
	const std::vector<cPlayerBasicData>& getPlayers();
	const std::vector<sLandingUnit>& getLandingUnits();
	const cPlayerBasicData& getLocalPlayer();

	int getLocalPlayerClan() const;
private:
	cSignalConnectionManager signalConnectionManager;

	size_t localPlayerNr;
	std::vector<cPlayerBasicData> players;

	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;

	int localPlayerClan;

	std::vector<sLandingUnit> localPlayerLandingUnits;
	std::vector<std::pair<sID, cUnitUpgrade>> localPlayerUnitUpgrades;
	cPosition localPlayerLandingPosition;
};

#endif // game_startup_network_client_networkclientgamenewH
