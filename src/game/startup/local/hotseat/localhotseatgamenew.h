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

#ifndef game_startup_local_hotseat_localhotseatgamenewH
#define game_startup_local_hotseat_localhotseatgamenewH

#include <memory>
#include <vector>
#include <utility>

#include "game/startup/local/hotseat/localhotseatgame.h"
#include "game/data/player/playerbasicdata.h"
#include "maxrconfig.h"
#include "utility/position.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cApplication;
class cStaticMap;
class cGameSettings;
class cPlayerColor;
class cPosition;
class cUnitUpgrade;

struct sLandingUnit;
struct sID;

class cLocalHotSeatGameNew : public cLocalHotSeatGame
{
	struct sPlayerData
	{
		cPlayerBasicData basicData;
		int clan;
		std::vector<sLandingUnit> landingUnits;
		std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades;
		cPosition landingPosition;
	};
public:
	cLocalHotSeatGameNew ();

	void start (cApplication& application);

	void setGameSettings (std::shared_ptr<cGameSettings> gameSettings);

	void setStaticMap (std::shared_ptr<cStaticMap> staticMap);

	void setPlayers (const std::vector<cPlayerBasicData>& players);

	void setPlayerClan (size_t playerIndex, int clan);

	void setLandingUnits (size_t playerIndex, std::vector<sLandingUnit> landingUnits);

	void setUnitUpgrades (size_t playerIndex, std::vector<std::pair<sID, cUnitUpgrade>> unitUpgrades);

	void setLandingPosition (size_t playerIndex, const cPosition& landingPosition);

	const std::shared_ptr<cStaticMap>& getStaticMap ();
	const std::shared_ptr<cGameSettings>& getGameSettings ();

	size_t getPlayerCount () const;
	const cPlayerBasicData& getPlayer (size_t playerIndex) const;
	int getPlayerClan (size_t playerIndex) const;
private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;

	std::vector<sPlayerData> playersData;
};

#endif // game_startup_local_hotseat_localhotseatgamenewH
