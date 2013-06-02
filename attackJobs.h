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

class cBuilding;
class cClient;
class cMap;
class cMenu;
class cNetMessage;
class cPlayer;
class cServer;
class cUnit;
class cVehicle;


/**
* selects a target unit from a map field, depending on the attack mode.
*/
void selectTarget (cVehicle*& targetVehicle, cBuilding*& targetBuilding, int x, int y, char attackMode, cMap* map);

//--------------------------------------------------------------------------
class cServerAttackJob
{
public:
	cServerAttackJob (cServer& server_, cUnit* _unit, int targetOff, bool sentry);
	~cServerAttackJob();

	void sendFireCommand (cPlayer* player);
	void clientFinished (int playerNr);

	//--------------------------------------------------------------------------
private:
	/** syncronizes positions of target, locks target and suspents move job if nessesary
	* @author Eiko
	*/
	void lockTarget (int offset);
	void lockTargetCluster();
	void sendFireCommand();
	void makeImpact (int x, int y);
	void makeImpactCluster();
	void sendAttackJobImpact (int offset, int remainingHP, int id);

	bool isMuzzleTypeRocket() const;

public:
	std::vector<cPlayer*> executingClients; /** the clients on which the attack job is currently running */
	cUnit* unit;
	int iID;

private:
	static int iNextID;
	cServer* server;
	bool sentryFire;
	int iAgressorOff;
	int iMuzzleType;
	bool bMuzzlePlayed;
	int iTargetOff;
	int damage;
	char attackMode;
	std::vector<cVehicle*> vehicleTargets; /** these lists are only used to sort out duplicate targets, when making a cluster impact */
	std::vector<cBuilding*> buildingTargets;
};


//--------------------------------------------------------------------------
class cClientAttackJob
{
public:
	int iID;
	cVehicle* vehicle;
	cBuilding* building;
	int iFireDir;
	int iMuzzleType;
	int iAgressorOffset;
	int iTargetOffset;
	int wait;
	int length;

	enum eAJStates { ROTATING, PLAYING_MUZZLE, FINISHED };
	eAJStates state;

	/** prepares a mapsquare to be attacked
	* @author Eiko
	*/
	static void lockTarget (cClient& client, cNetMessage* message);
	static void handleAttackJobs(cClient& client, cMenu* activeMenu);
	static void makeImpact (cClient& client, int offset, int remainingHP, int id);

	cClientAttackJob (cClient* client, cNetMessage* message);

	void rotate();
	void playMuzzle(cClient& client, cMenu* activeMenu);
	void sendFinishMessage(cClient& client);
};


#endif //#ifndef attackjobsH
