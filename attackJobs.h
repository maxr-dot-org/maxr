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
#ifndef attackjobsH
#define attackjobsH

#include <vector>
#include "utility/position.h"

class cBuilding;
class cClient;
class cMap;
class cNetMessage;
class cPlayer;
class cServer;
class cUnit;
class cVehicle;


/**
* selects a target unit from a map field, depending on the attack mode.
*/
cUnit* selectTarget (const cPosition& position, char attackMode, const cMap& map);

//--------------------------------------------------------------------------
class cServerAttackJob
{
public:
	cServerAttackJob (cServer& server_, cUnit* _unit, const cPosition& targetPosition, bool sentry);
	~cServerAttackJob();

	void sendFireCommand (cPlayer* player);
	void clientFinished (int playerNr);

	//--------------------------------------------------------------------------
private:
	/** syncronizes positions of target, locks target and suspents move job if necessary
	* @author Eiko
	*/
	void lockTarget (const cPosition& position);
	void lockTargetCluster();
	void sendFireCommand();
	void makeImpact (const cPosition& position);
	void makeImpactCluster();
	void sendAttackJobImpact (const cPosition& position, int remainingHP, int id);

	bool isMuzzleTypeRocket() const;

public:
	/** the clients on which the attack job is currently running */
	std::vector<cPlayer*> executingClients;
	cUnit* unit;
	int iID;

private:
	static int iNextID;
	cServer* server;
	bool sentryFire;
	cPosition aggressorPosition;
	int iMuzzleType;
	bool bMuzzlePlayed;
	cPosition targetPosition;
	int damage;
	char attackMode;
	/** these lists are only used to sort out duplicate targets,
	 * when making a cluster impact */
	std::vector<cUnit*> targets;
};

//--------------------------------------------------------------------------
class cClientAttackJob
{
private:
	int iID;
	cUnit* unit;
	int iFireDir;
	int iMuzzleType;
	cPosition aggressorPosition;
	cPosition targetPosition;
	int wait;
	int length;

	enum eAJStates { ROTATING, PLAYING_MUZZLE, FINISHED };
	eAJStates state;

public:
	/** prepares a mapsquare to be attacked
	* @author Eiko
	*/
	static void lockTarget (cClient& client, cNetMessage& message);
	static void handleAttackJobs (cClient& client);
	static void makeImpact (cClient& client, const cPosition& position, int remainingHP, int id);
public:

	cClientAttackJob (cClient* client, cNetMessage& message);

	void rotate();
	void playMuzzle (cClient& client);
	void sendFinishMessage (cClient& client);
	void onRemoveUnit (cUnit& unit_) { if (unit == &unit_) unit = 0; }
};


#endif //#ifndef attackjobsH
