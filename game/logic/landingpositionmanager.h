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

#ifndef game_logic_landingpositionmanagerH
#define game_logic_landingpositionmanagerH

#include <vector>
#include <memory>

#include "landingpositionstate.h"
#include "../../utility/position.h"
#include "../../utility/signal/signal.h"

class sPlayer;

class cLandingPositionManager
{
	struct sLandingPositionData
	{
		sLandingPositionData (std::shared_ptr<sPlayer> player);

		cPosition landingPosition;
		cPosition lastLandingPosition;

		eLandingPositionState state;

		std::shared_ptr<sPlayer> player;
	};
public:
	cLandingPositionManager (const std::vector<std::shared_ptr<sPlayer>>& players);

	void setLandingPosition (const sPlayer& player, const cPosition& landingPosition);

	cSignal<void (const sPlayer&, eLandingPositionState)> landingPositionStateChanged;
	cSignal<void ()> allPositionsValid;

private:
	static const double warningDistance;
	static const double tooCloseDistance;

	std::vector<sLandingPositionData> landingPositions;

	sLandingPositionData& getLandingPositionData (const sPlayer& player);

	void checkPlayerState (const sPlayer& player);
};

#endif // game_logic_landingpositionmanagerH
