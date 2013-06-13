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

#ifndef JOBS_H
#define JOBS_H

class cGameTimer;
class cJobContainer;
class cUnit;
class cVehicle;

#include <vector>

/**
* little helper jobs for game time synchonous actions,
* like rotating a unit to a spezific direction or landing/takeoff
*/
class cJob
{
	friend class cJobContainer;
protected:
	explicit cJob (cVehicle& vehicle_);
public:
	virtual ~cJob() {}
	virtual void run (const cGameTimer& gameTimer) = 0;

protected:
	bool finished;
	cVehicle* vehicle;
};

class cJobContainer
{
public:
	void addJob (cJob& job);
	void onRemoveUnit (cUnit* unit);
	void run (cGameTimer& gameTimer);
private:
	std::vector<cJob*>::iterator releaseJob (std::vector<cJob*>::iterator it);
private:
	std::vector<cJob*> jobs;
};


class cStartBuildJob : public cJob
{
private:
	int orgX;
	int orgY;
	bool big;
public:
	cStartBuildJob (cVehicle& vehicle_, int orgX_, int orgY_, bool big_);
	virtual void run (const cGameTimer& gameTimer);
};


class cPlaneTakeoffJob : public cJob
{
private:
	bool takeoff;
public:
	cPlaneTakeoffJob (cVehicle& vehicle_, bool takeoff_);
	virtual void run (const cGameTimer& gameTimer);
};

#endif // JOBS_H
