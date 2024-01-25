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

#ifndef game_logic_surveyoraiH
#define game_logic_surveyoraiH

#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"

#include <forward_list>
#include <memory>

class cMap;
class cVehicle;
class cClient;

class cSurveyorAi
{
public:
	explicit cSurveyorAi (const cVehicle&);

	void run (cClient&, const std::vector<std::unique_ptr<cSurveyorAi>>&);
	bool isFinished() const { return finished; }
	const cVehicle& getVehicle() { return vehicle; }

private:
	void planMove (std::forward_list<cPosition>& path, int remainingMovePoints, const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, const cMap&) const;
	void planLongMove (const std::vector<std::unique_ptr<cSurveyorAi>>&, cClient&);

	float calcFactor (const cPosition&, const std::forward_list<cPosition>& path, const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, const cMap&) const;
	float calcScoreDistToOtherSurveyor (const std::vector<std::unique_ptr<cSurveyorAi>>& jobs, const cPosition&, float e) const;

	bool positionHasBeenSurveyedByPath (const cPosition&, const std::forward_list<cPosition>& path) const;
	bool hasAdjacentResources (const cPosition&, const cMap&) const;

	void changeOP();

private:
	const cVehicle& vehicle; // the vehicle the auto move job belongs to
	bool finished = false; // true when the job can be deleted
	int counter = 0;

	// the operation point of the surveyor
	// the surveyor tries to stay near this coordinates
	cPosition operationPoint;

	cSignalConnectionManager connectionManager;
};

#endif // game_logic_surveyoraiH
