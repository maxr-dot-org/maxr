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

#ifndef ui_graphical_game_gameguistateH
#define ui_graphical_game_gameguistateH

#include <vector>

#include "utility/serialization/serialization.h"
#include "utility/position.h"

class cUnitSelection;
class cUnitLockList;

class cGameGuiState
{
public:
	void setSelectedUnits (const cUnitSelection& unitSelection);
	const std::vector<unsigned int>& getSelectedUnitIds() const;

	void setLockedUnits (const cUnitLockList& unitLockList);
	const std::vector<unsigned int>& getLockedUnitIds() const;

	template <typename T>
	void serialize(T& archive)
	{
		archive & NVP(mapPosition);
		archive & NVP(mapZoomFactor);
		archive & NVP(surveyActive);
		archive & NVP(hitsActive);
		archive & NVP(scanActive);
		archive & NVP(statusActive);
		archive & NVP(ammoActive);
		archive & NVP(gridActive);
		archive & NVP(colorActive);
		archive & NVP(rangeActive);
		archive & NVP(fogActive);
		archive & NVP(lockActive);
		archive & NVP(miniMapZoomFactorActive);
		archive & NVP(miniMapAttackUnitsOnly);
		archive & NVP(unitVideoPlaying);
		archive & NVP(chatActive);
		archive & NVP(selectedUnitIds);
		archive & NVP(lockedUnitIds);
	}
public:
	cPosition mapPosition;
	float mapZoomFactor = 1.f;

	bool surveyActive = false;
	bool hitsActive = false;
	bool scanActive = false;
	bool statusActive = false;
	bool ammoActive = false;
	bool gridActive = false;
	bool colorActive = false;
	bool rangeActive = false;
	bool fogActive = false;
	bool lockActive = false;
	bool miniMapZoomFactorActive = false;
	bool miniMapAttackUnitsOnly = false;
	bool unitVideoPlaying = true;
	bool chatActive = true;
private:
	std::vector<unsigned int> selectedUnitIds;
	std::vector<unsigned int> lockedUnitIds;
};

#endif
