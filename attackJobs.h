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

#include "player.h"

class cServerAttackJob
{
public:

	static int iNextID;
	int iID;
	int iAgressorOff;
	int iMuzzleType;
	bool bMuzzlePlayed;
	int iTargetOff;
	int damage;
	int attackMode;

	cList<cPlayer*> executingClients; /** the clients on which the attack job is currently running */
	cBuilding* building;
	cVehicle* vehicle;

	cServerAttackJob( cBuilding* building, int targetOff );
	cServerAttackJob( cVehicle* vehicle, int targetOff );
	~cServerAttackJob();
	/** syncronizes positions of target, locks target and suspents move job if nessesary
	* @author Eiko
	*/
	void lockTarget( int offset );
	void sendFireCommand();
	void clientFinished( int playerNr );
	void makeImpact();
	void sendAttackJobImpact(int offset, int damage, int attackMode );

};


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
	bool mineExplosion;

	enum eAJStates { UPDATE_AGRESSOR_DATA, ROTATING, PLAYING_MUZZLE, FINISHED };
	eAJStates state;

	/** prepares a mapspuare to be attacked
	* @author Eiko
	*/
	static void clientLockTarget( cNetMessage* message );
	static void handleAttackJobs();
	static void makeImpact(int offset, int damage, int attackMode);

	cClientAttackJob( cNetMessage* message );

	void rotate();
	void playMuzzle();
	void sendFinishMessage();
	void updateAgressorData();
};


#endif //#ifndef attackjobsH
