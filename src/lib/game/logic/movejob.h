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

#ifndef game_logic_movejobsH
#define game_logic_movejobsH

#include "game/logic/endmoveaction.h"
#include "utility/position.h"

#include <forward_list>
#include <memory>
#include <optional>

#define MOVE_SPEED 4 // maximum speed (pixel per gametime tick) of vehicle movements

class cMap;
class cVehicle;

enum class eStopOn
{
	Never,
	DetectResource
};

class cMoveJob
{
public:
	cMoveJob();
	cMoveJob (const std::forward_list<cPosition>& path, cVehicle&);
	/**
	* gets the list of position that make up the path. First element is the position,
	* the unit will drive to when starting the next movement step.
	*/
	const std::forward_list<cPosition>& getPath() const { return path; }
	/**
	* return the moved vehicle
	*/
	std::optional<int> getVehicleId() const { return vehicleId; }
	/**
	* return the amount of saved movement points. These are saved at turn end and added to the
	* vehicles movement points in the next turn. This is done to prevent that the player looses
	* movement points due to rounding issues.
	*/
	unsigned int getSavedSpeed() const { return savedSpeed; }
	/**
	* returns true, if the move job is finished (reached destination or was stopped)
	*/
	bool isFinished() const;
	/**
	* return true, if the movejob is waiting to be resumed
	*/
	bool isWaiting() const;
	/**
	* returns true, if the vehicle is currently moving (or rotating).
	*/
	bool isActive() const;

	/**
	* must be called, when the vehicle is destroyed/removed
	*/
	void removeVehicle();
	/**
	* Execute the movement for the next game time tick
	*/
	void run (cModel&);
	/**
	* Stop the movejob. If the job is not active, it is stopped immediately.
	* If the job is active, the state of the job is set to eMoveJobState::Stopping
	* and the job will be halted when the unit reaches the next field.
	*/
	void stop (cVehicle&);
	/**
	* Resume execution of a waiting movejob.
	*/
	void resume();
	/**
	* defines an action to be executed at the end of the path
	*/
	void setEndMoveAction (const cEndMoveAction&);
	const cEndMoveAction& getEndMoveAction() const { return endMoveAction; }

	/** used for the surveyor ai, so it can recalculate its steps, when resources are detected */
	void setStopOn (eStopOn value) { stopOn = value; }

	uint32_t getChecksum (uint32_t crc) const;

	template <typename Archive>
	static std::unique_ptr<cMoveJob> createFrom (Archive& archive)
	{
		auto res = std::make_unique<cMoveJob>();
		res->serialize (archive);
		return res;
	}

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (vehicleId);
		archive & NVP (path);
		archive & NVP (state);
		archive & NVP (savedSpeed);
		archive & NVP (nextDir);
		archive & NVP (timer100ms);
		archive & NVP (timer50ms);
		archive & NVP (currentSpeed);
		archive & NVP (pixelToMove);
		archive & NVP (endMoveAction);
		archive & NVP (stopOn);
		// clang-format on
	}

private:
	enum class eMoveJobState
	{
		Active,
		Waiting,
		Stopping,
		Finished
	};

	/**
	* triggers all actions, that need to be done before starting a movement step
	*/
	void startMove (cModel&, cVehicle&);

	/**
	* check, weather the next field is free and make the necessary actions if it is not.
	* Return true, if the movement can be continued.
	*/
	bool handleCollision (cModel&, cVehicle&);

	bool recalculatePath (cModel&, cVehicle&);

	/**
	* actually execute the movement
	*/
	void moveVehicle (cModel&, cVehicle&);
	/**
	* updates the current speed of the vehicle (for accelerating and breaking)
	*/
	void updateSpeed (cVehicle&, const cMap&);
	/**
	* triggers all actions, that need to be done after finishing a movement step
	*/
	void endMove (cModel&, cVehicle&);

private:
	/** the vehicle to move */
	std::optional<int> vehicleId;
	/** list of positions. First element is the next field, that the unit will drive to after the current one. */
	std::forward_list<cPosition> path;
	eMoveJobState state = eMoveJobState::Active;

	/** movement points, that are taken to the next turn, to prevent that the player looses movement points due to rounding issues */
	unsigned int savedSpeed = 0;
	/** direction the vehicle must be rotated to, before moving */
	std::optional<unsigned int> nextDir;
	/** 100 ms timer tick */
	unsigned int timer100ms = 1;
	/** 50 ms timer tick */
	unsigned int timer50ms = 1;
	/** speed of the vehicle in 100th pixel per game time tick */
	int currentSpeed = 0;

	int pixelToMove = 0;

	cEndMoveAction endMoveAction;

	/** give the surveyor ai the chance to calc a new path, when resources are found. */
	eStopOn stopOn = eStopOn::Never;
};

#endif
