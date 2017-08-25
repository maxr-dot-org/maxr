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

#include "utility/position.h"

class cMap;
class cPlayer;
class cFx;
class cUnit;

class cAttackJob
{
public:
	/**
	* selects a target unit from a map field, depending on the attack mode.
	*/
	static cUnit* selectTarget (const cPosition& position, char attackMode, const cMap& map, const cPlayer* owner);

	cAttackJob (cUnit& aggressor, const cPosition& targetPosition, const cModel& model);
	cAttackJob ();

	void run(cModel& model);
	bool finished() const;

	uint32_t getChecksum(uint32_t crc) const;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(aggressor);
		archive & NVP(targetPosition);
		archive & NVP(lockedTargets);
		archive & NVP(fireDir);
		archive & NVP(counter);
		archive & NVP(state);
	}

private:
	cUnit* aggressor;
	cPosition targetPosition;
	std::vector<int> lockedTargets;
	int fireDir;
	int counter;
	enum eAJStates { S_ROTATING, S_PLAYING_MUZZLE, S_FIRING, S_FINISHED };
	eAJStates state;


	int calcFireDir();
	void lockTarget(const cMap& map);
	void releaseTargets(const cModel& model);
	void fire(cModel& model);
	std::unique_ptr<cFx> createMuzzleFx ();
	void impact(cModel& model);
	void impactCluster(cModel& model);
	void impactSingle (const cPosition& position, int attackPoints, cModel& model, std::vector<cUnit*>* avoidTargets = nullptr);

};

#endif // game_logic_attackjobH
