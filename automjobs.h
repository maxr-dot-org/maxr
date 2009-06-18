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
#ifndef automjobsH
#define automjobsH

#define FIELD_BLOCKED -10000
#define WAIT_FRAMES 4

//main tuning knobs of the AI:
#define A 1.0	//how important is it, to survey as much fields as possible with each move
#define B 1.49  //how important is it, to stay near the operation point
#define C 9.0	//how important is it, to hold a distance to other surveyors
#define EXP -2	//an negative integer; the influence of other surveyors is falling over the distance with x^EXP

				//when there are no fields to survey next to the surveyor, where should the surveyor resume?
				//if the surveyor seems to plan long senseless moves, rebalancing the following factors might help
#define D 1		//more likely near the operation point
#define E 3		//more likely near his position
#define EXP2 -2
#define F 100	//more likely far away from other surveyors
#define G 1.8	// how important is to go to directions where resources has been found already

#define MAX_DISTANCE_OP 19 //when the distance to the OP exceeds this value, the OP is changed
#define DISTANCE_NEW_OP 7 //the new OP will be between the surveyor and the old OP and has distance of DISTANCE_NEW_OP to the surveyor



class cClientMoveJob;
class cVehicle;

class cAutoMJob {
	cClientMoveJob *lastMoveJob;	//pointer to the last mjob, generated by the AI.
							//needed to check if the move job was changed from outside the AI (i. e. by the player)
	cVehicle *vehicle;		//the vehicle the auto move job belongs to
	int iNumber;			//index of the AutoMJob in autoMJobs[]
	int n;					//frame delay counter
	bool finished;			//true when the job can be deleted


	void DoAutoMove();
	float CalcFactor(int x, int y);
	void PlanNextMove();
	void PlanLongMove();
	void changeOP();


public:

	int OPX, OPY;		//the operation point of the surveyor
						//the surveyor tries to stay near this coordinates
	bool playerMJob;	//the player has changed the move job

	static void handleAutoMoveJobs();
	cAutoMJob(cVehicle *vehicle);
	~cAutoMJob(void);

};

#endif  //automjobsH