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

#include "ui/graphical/game/gameguistate.h"
#include "ui/graphical/game/unitselection.h"
#include "ui/graphical/game/unitlocklist.h"
#include "game/data/units/unit.h"
#include "netmessage.h"
#include "game/logic/clientevents.h"

//------------------------------------------------------------------------------
cGameGuiState::cGameGuiState() :
	mapPosition (0, 0),
	mapZoomFactor (1.),
	surveyActive (false),
	hitsActive (false),
	scanActive (false),
	statusActive (false),
	ammoActive (false),
	gridActive (false),
	colorActive (false),
	rangeActive (false),
	fogActive (false),
	lockActive (false),
	miniMapZoomFactorActive (false),
	miniMapAttackUnitsOnly (false),
	unitVideoPlaying (true),
	chatActive (true)
{}

//------------------------------------------------------------------------------
void cGameGuiState::setMapPosition (const cPosition& position)
{
	mapPosition = position;
}

//------------------------------------------------------------------------------
const cPosition& cGameGuiState::getMapPosition() const
{
	return mapPosition;
}

//------------------------------------------------------------------------------
void cGameGuiState::setMapZoomFactor (float zoom)
{
	mapZoomFactor = zoom;
}

//------------------------------------------------------------------------------
float cGameGuiState::getMapZoomFactor() const
{
	return mapZoomFactor;
}

//------------------------------------------------------------------------------
void cGameGuiState::setSurveyActive (bool value)
{
	surveyActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getSurveyActive() const
{
	return surveyActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setHitsActive (bool value)
{
	hitsActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getHitsActive() const
{
	return hitsActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setScanActive (bool value)
{
	scanActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getScanActive() const
{
	return scanActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setStatusActive (bool value)
{
	statusActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getStatusActive() const
{
	return statusActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setAmmoActive (bool value)
{
	ammoActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getAmmoActive() const
{
	return ammoActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setGridActive (bool value)
{
	gridActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getGridActive() const
{
	return gridActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setColorActive (bool value)
{
	colorActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getColorActive() const
{
	return colorActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setRangeActive (bool value)
{
	rangeActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getRangeActive() const
{
	return rangeActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setFogActive (bool value)
{
	fogActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getFogActive() const
{
	return fogActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setLockActive (bool value)
{
	lockActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getLockActive() const
{
	return lockActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setMiniMapZoomFactorActive (bool value)
{
	miniMapZoomFactorActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getMiniMapZoomFactorActive() const
{
	return miniMapZoomFactorActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setMiniMapAttackUnitsOnly (bool value)
{
	miniMapAttackUnitsOnly = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getMiniMapAttackUnitsOnly() const
{
	return miniMapAttackUnitsOnly;
}

//------------------------------------------------------------------------------
void cGameGuiState::setUnitVideoPlaying (bool value)
{
	unitVideoPlaying = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getUnitVideoPlaying() const
{
	return unitVideoPlaying;
}

//------------------------------------------------------------------------------
void cGameGuiState::setChatActive (bool value)
{
	chatActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getChatActive() const
{
	return chatActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setSelectedUnits (const cUnitSelection& unitSelection)
{
	selectedUnitIds.clear();
	const auto selectedUnits = unitSelection.getSelectedUnits();
	for (size_t i = 0; i < selectedUnits.size(); ++i)
	{
		selectedUnitIds.push_back (selectedUnits[i]->iID);
	}
}

//------------------------------------------------------------------------------
const std::vector<unsigned int>& cGameGuiState::getSelectedUnitIds() const
{
	return selectedUnitIds;
}

//------------------------------------------------------------------------------
void cGameGuiState::setLockedUnits (const cUnitLockList& unitLockList)
{
	lockedUnitIds.clear();
	for (size_t i = 0; i < unitLockList.getLockedUnitsCount(); ++i)
	{
		lockedUnitIds.push_back (unitLockList.getLockedUnit (i)->iID);
	}
}

//------------------------------------------------------------------------------
const std::vector<unsigned int>& cGameGuiState::getLockedUnitIds() const
{
	return lockedUnitIds;
}
