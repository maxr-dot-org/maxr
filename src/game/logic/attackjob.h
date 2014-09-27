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

#ifndef game_logic_attackjobH
#define game_logic_attackjobH

#include <vector>
#include <memory>

#include "utility/position.h"

class cMap;
class cPlayer;
class cNetMessage;
class cFx;
class cMenu;
class cServer;
class cClient;
class cUnit;

class cAttackJob
{
private:
	static const int ROTATION_SPEED = 10; //rotate aggressor every X game time ticks
	static const int FIRE_DELAY = 10;
	static const int IMPACT_DELAY = 10;
	static const int DESTROY_DELAY = 30;


	int aggressorID;
	int aggressorPlayerNr;
	cPosition aggressorPosition;
	int attackMode;
	int muzzleType;
	int attackPoints;
	cPosition targetPosition;

	cServer *const server;
	cClient *const client;
	std::vector<int> destroyedTargets; //not synced. only needed on server
	std::vector<int> lockedTargets;    //not synced. TODO: maybe nessesary

	int fireDir;
	
	int counter;
	enum eAJStates { S_ROTATING, S_PLAYING_MUZZLE, S_FIRING, S_EXPLODING, S_FINISHED };
	eAJStates state;

	int calcFireDir();
	int calcTimeForRotation();
	cUnit* getAggressor();

	void lockTarget();
	void fire();
	std::unique_ptr<cFx> createMuzzleFx();
	bool impact();
	bool impactCluster();
	bool impactSingle (const cPosition& position, std::vector<cUnit*>* avoidTargets = NULL);
	void destroyTarget();

public:
	/**
	* selects a target unit from a map field, depending on the attack mode.
	*/
	static cUnit* selectTarget(const cPosition& position, char attackMode, const cMap& map, cPlayer* owner);
	static void runAttackJobs(std::vector<cAttackJob*>& attackJobs);

	cAttackJob(cServer* server, cUnit* aggressor, const cPosition& targetPosition);
	cAttackJob(cClient* client, cNetMessage& message);

	std::unique_ptr<cNetMessage> serialize() const;
	void run();
	bool finished() const;
};
      
#endif // game_logic_attackjobH
