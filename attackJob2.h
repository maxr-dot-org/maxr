#ifndef ATTACK_JOB_H
#define ATTACK_JOB_H

class cMap;
class cPlayer;
class cNetMessage;
class cFx;
class cMenu;

#include "autoptr.h"

class cAttackJob
{
private:
	static const int ROTATION_SPEED = 10; //rotate aggressor every X game time ticks
	static const int FIRE_DELAY = 10;
	static const int IMPACT_DELAY = 10;
	static const int DESTROY_DELAY = 30;


	int aggressorID;
	int aggressorPlayerNr;
	int aggressorPosX;
	int aggressorPosY;
	int attackMode;
	int muzzleType;
	int attackPoints;
	int targetX;
	int targetY;

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
	cFx* createMuzzleFx();
	bool impact(cMenu* activeMenu);
	bool impactCluster(cMenu* activeMenu);
	bool impactSingle(cMenu* activeMenu, int x, int y, std::vector<cUnit*>* avoidTargets = NULL);
	void destroyTarget();

public:
	/**
	* selects a target unit from a map field, depending on the attack mode.
	*/
	static cUnit* selectTarget(int x, int y, char attackMode, const cMap& map, cPlayer* owner);
	static void runAttackJobs(std::vector<cAttackJob*>& attackJobs, cMenu* activeMenu = NULL);

	cAttackJob(cServer* server, cUnit* aggressor, int targetX, int targetY);
	cAttackJob(cClient* client, cNetMessage& message);

	cNetMessage* serialize() const;
	void run(cMenu* activeMenu);
	bool finished() const;


};
      
#endif
