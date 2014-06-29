#ifndef ATTACK_JOB_H
#define ATTACK_JOB_H

class cMap;
class cPlayer;
class cNetMessage;
class cFx;

#include "autoptr.h"

class cAttackJob
{
private:
	static const int ROTATION_SPEED = 10; //rotate aggressor every X game time ticks
	static const int FIRE_DELAY = 10;
	static const int IMPACT_DELAY = 10; //TODO: delay abhängig von Entfernung u. Rocket/Balistic
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
	cUnit* target;

	int fireDir;
	
	cUnit* aggressor;
	int counter;
	enum eAJStates { S_ROTATING, S_PLAYING_MUZZLE, S_FIRING, S_EXPLODING, S_FINISHED };
	eAJStates state;

	int calcFireDir();
	int calcTimeForRotation();
	cUnit* getAggressor();

	void fire();
	cFx* createMuzzleFx();
	bool impact();
	void destroyTarget();

public:
	/**
	* selects a target unit from a map field, depending on the attack mode.
	*/
	static cUnit* selectTarget(int x, int y, char attackMode, const cMap& map, cPlayer* owner);
	static void runAttackJobs(std::vector<cAttackJob*>& attackJobs);
	static void destroyUnit(cUnit& unit, cServer* server, cClient* client);

	cAttackJob(cServer* server, cUnit* aggressor, int targetX, int targetY);
	cAttackJob(cClient* client, cNetMessage& message);

	void onRemoveUnit(cUnit& unit_) { if (aggressor == &unit_) aggressor = NULL;}

	cNetMessage* serialize() const;
	void run();
	bool finished() const;


};
      
#endif
