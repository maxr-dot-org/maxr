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

#include "utility/position.h"
#include "tinyxml2.h"

class cUnitSelection;
class cUnitLockList;
class cNetMessage;
class cVersion;

class cGameGuiState
{
public:
	cGameGuiState();

	void setMapPosition (const cPosition& position);
	const cPosition& getMapPosition() const;

	void setMapZoomFactor (float zoomFactor);
	float getMapZoomFactor() const;

	void setSurveyActive (bool value);
	bool getSurveyActive() const;

	void setHitsActive (bool value);
	bool getHitsActive() const;

	void setScanActive (bool value);
	bool getScanActive() const;

	void setStatusActive (bool value);
	bool getStatusActive() const;

	void setAmmoActive (bool value);
	bool getAmmoActive() const;

	void setGridActive (bool value);
	bool getGridActive() const;

	void setColorActive (bool value);
	bool getColorActive() const;

	void setRangeActive (bool value);
	bool getRangeActive() const;

	void setFogActive (bool value);
	bool getFogActive() const;

	void setLockActive (bool value);
	bool getLockActive() const;

	void setMiniMapZoomFactorActive (bool value);
	bool getMiniMapZoomFactorActive() const;

	void setMiniMapAttackUnitsOnly (bool value);
	bool getMiniMapAttackUnitsOnly() const;

	void setUnitVideoPlaying (bool value);
	bool getUnitVideoPlaying() const;

	void setChatActive (bool value);
	bool getChatActive() const;

	void setSelectedUnits (const cUnitSelection& unitSelection);
	const std::vector<unsigned int>& getSelectedUnitIds() const;

	void setLockedUnits (const cUnitLockList& unitLockList);
	const std::vector<unsigned int>& getLockedUnitIds() const;

	void pushInto (cNetMessage& message) const;
	void popFrom (cNetMessage& message);

	void pushInto (tinyxml2::XMLElement& element) const;
	void popFrom (const tinyxml2::XMLElement& element, const cVersion& saveVersion);
private:
	cPosition mapPosition;
	float mapZoomFactor;

	bool surveyActive;
	bool hitsActive;
	bool scanActive;
	bool statusActive;
	bool ammoActive;
	bool gridActive;
	bool colorActive;
	bool rangeActive;
	bool fogActive;
	bool lockActive;
	bool miniMapZoomFactorActive;
	bool miniMapAttackUnitsOnly;
	bool unitVideoPlaying;
	bool chatActive;

	std::vector<unsigned int> selectedUnitIds;
	std::vector<unsigned int> lockedUnitIds;
};

#endif // ui_graphical_game_gameguistateH
