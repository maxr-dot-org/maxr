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
#include "game/logic/savegame.h"
#include "netmessage.h"
#include "game/logic/clientevents.h"

//------------------------------------------------------------------------------
cGameGuiState::cGameGuiState () :
	mapPosition (0,0),
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
const cPosition& cGameGuiState::getMapPosition () const
{
	return mapPosition;
}

//------------------------------------------------------------------------------
void cGameGuiState::setMapZoomFactor (float zoom)
{
	mapZoomFactor = zoom;
}

//------------------------------------------------------------------------------
float cGameGuiState::getMapZoomFactor () const
{
	return mapZoomFactor;
}

//------------------------------------------------------------------------------
void cGameGuiState::setSurveyActive (bool value)
{
	surveyActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getSurveyActive () const
{
	return surveyActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setHitsActive (bool value)
{
	hitsActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getHitsActive () const
{
	return hitsActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setScanActive (bool value)
{
	scanActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getScanActive () const
{
	return scanActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setStatusActive (bool value)
{
	statusActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getStatusActive () const
{
	return statusActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setAmmoActive (bool value)
{
	ammoActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getAmmoActive () const
{
	return ammoActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setGridActive (bool value)
{
	gridActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getGridActive () const
{
	return gridActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setColorActive (bool value)
{
	colorActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getColorActive () const
{
	return colorActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setRangeActive (bool value)
{
	rangeActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getRangeActive () const
{
	return rangeActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setFogActive (bool value)
{
	fogActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getFogActive () const
{
	return fogActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setLockActive (bool value)
{
	lockActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getLockActive () const
{
	return lockActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setMiniMapZoomFactorActive (bool value)
{
	miniMapZoomFactorActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getMiniMapZoomFactorActive () const
{
	return miniMapZoomFactorActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setMiniMapAttackUnitsOnly (bool value)
{
	miniMapAttackUnitsOnly = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getMiniMapAttackUnitsOnly () const
{
	return miniMapAttackUnitsOnly;
}

//------------------------------------------------------------------------------
void cGameGuiState::setUnitVideoPlaying (bool value)
{
	unitVideoPlaying = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getUnitVideoPlaying () const
{
	return unitVideoPlaying;
}

//------------------------------------------------------------------------------
void cGameGuiState::setChatActive (bool value)
{
	chatActive = value;
}

//------------------------------------------------------------------------------
bool cGameGuiState::getChatActive () const
{
	return chatActive;
}

//------------------------------------------------------------------------------
void cGameGuiState::setSelectedUnits (const cUnitSelection& unitSelection)
{
	selectedUnitIds.clear ();
	const auto selectedUnits = unitSelection.getSelectedUnits ();
	for (size_t i = 0; i < selectedUnits.size (); ++i)
	{
		selectedUnitIds.push_back (selectedUnits[i]->iID);
	}
}

//------------------------------------------------------------------------------
const std::vector<unsigned int>& cGameGuiState::getSelectedUnitIds () const
{
	return selectedUnitIds;
}

//------------------------------------------------------------------------------
void cGameGuiState::setLockedUnits (const cUnitLockList& unitLockList)
{
	lockedUnitIds.clear ();
	for (size_t i = 0; i < unitLockList.getLockedUnitsCount (); ++i)
	{
		lockedUnitIds.push_back (unitLockList.getLockedUnit(i)->iID);
	}
}

//------------------------------------------------------------------------------
const std::vector<unsigned int>& cGameGuiState::getLockedUnitIds () const
{
	return lockedUnitIds;
}

//------------------------------------------------------------------------------
void cGameGuiState::pushInto (cNetMessage& message) const
{
	message.pushPosition (mapPosition);
	message.pushFloat (mapZoomFactor);

	message.pushBool (surveyActive);
	message.pushBool (hitsActive);
	message.pushBool (scanActive);
	message.pushBool (statusActive);
	message.pushBool (ammoActive);
	message.pushBool (gridActive);
	message.pushBool (colorActive);
	message.pushBool (rangeActive);
	message.pushBool (fogActive);
	message.pushBool (lockActive);
	message.pushBool (miniMapZoomFactorActive);
	message.pushBool (miniMapAttackUnitsOnly);
	message.pushBool (unitVideoPlaying);
	message.pushBool (chatActive);

	for (size_t i = 0; i < selectedUnitIds.size (); ++i)
	{
		message.pushInt32 (selectedUnitIds[i]);
	}
	message.pushInt32 (selectedUnitIds.size ());

	for (size_t i = 0; i < lockedUnitIds.size (); ++i)
	{
		message.pushInt32 (lockedUnitIds[i]);
	}
	message.pushInt32 (lockedUnitIds.size ());
}

//------------------------------------------------------------------------------
void cGameGuiState::popFrom (cNetMessage& message)
{
	lockedUnitIds.resize (message.popInt32 ());
	for (size_t i = 0; i < lockedUnitIds.size (); ++i)
	{
		lockedUnitIds[i] = message.popInt32 ();
	}

	selectedUnitIds.resize (message.popInt32 ());
	for (size_t i = 0; i < selectedUnitIds.size (); ++i)
	{
		selectedUnitIds[i] = message.popInt32 ();
	}

	chatActive = message.popBool ();
	unitVideoPlaying = message.popBool ();
	miniMapAttackUnitsOnly = message.popBool ();
	miniMapZoomFactorActive = message.popBool ();
	lockActive = message.popBool ();
	fogActive = message.popBool ();
	rangeActive = message.popBool ();
	colorActive = message.popBool ();
	gridActive = message.popBool ();
	ammoActive = message.popBool ();
	statusActive = message.popBool ();
	scanActive = message.popBool ();
	hitsActive = message.popBool ();
	surveyActive = message.popBool ();

	mapZoomFactor = message.popFloat ();
	mapPosition = message.popPosition ();
}

//------------------------------------------------------------------------------
void cGameGuiState::pushInto (tinyxml2::XMLElement& element) const
{
	cSavegame::addAttributeElement (&element, "Offset", "x", iToStr (mapPosition.x ()), "y", iToStr (mapPosition.y()));
	cSavegame::addAttributeElement (&element, "Zoom", "num", fToStr (mapZoomFactor));
	if (colorActive) cSavegame::addMainElement (&element, "Colors");
	if (gridActive) cSavegame::addMainElement (&element, "Grid");
	if (ammoActive) cSavegame::addMainElement (&element, "Ammo");
	if (fogActive) cSavegame::addMainElement (&element, "Fog");
	if (miniMapZoomFactorActive) cSavegame::addMainElement (&element, "MinimapZoom");
	if (rangeActive) cSavegame::addMainElement (&element, "Range");
	if (scanActive) cSavegame::addMainElement (&element, "Scan");
	if (statusActive) cSavegame::addMainElement (&element, "Status");
	if (surveyActive) cSavegame::addMainElement (&element, "Survey");
	if (lockActive) cSavegame::addMainElement (&element, "Lock");
	if (hitsActive) cSavegame::addMainElement (&element, "Hitpoints");
	if (miniMapAttackUnitsOnly) cSavegame::addMainElement (&element, "TNT");
	if (unitVideoPlaying) cSavegame::addMainElement (&element, "UnitVideoPlaying");
	if (chatActive) cSavegame::addMainElement (&element, "ChatActive");

	if (!selectedUnitIds.empty ())
	{
		const auto selectedUnitsElement = cSavegame::addMainElement (&element, "SelectedUnits");
		for (size_t i = 0; i < selectedUnitIds.size (); ++i)
		{
			cSavegame::addAttributeElement (selectedUnitsElement, "SelectedUnit", "num", iToStr (selectedUnitIds[i]));
		}
	}
	if (!lockedUnitIds.empty ())
	{
		const auto lockedUnitsElement = cSavegame::addMainElement (&element, "LockedUnits");
		for (size_t i = 0; i < lockedUnitIds.size (); ++i)
		{
			cSavegame::addAttributeElement (lockedUnitsElement, "LockedUnit", "num", iToStr (lockedUnitIds[i]));
		}
	}
}

//------------------------------------------------------------------------------
void cGameGuiState::popFrom (const tinyxml2::XMLElement& element, const cVersion& saveVersion)
{
	mapPosition.x () = element.FirstChildElement ("Offset")->IntAttribute ("x");
	mapPosition.y () = element.FirstChildElement ("Offset")->IntAttribute ("y");
	mapZoomFactor = element.FirstChildElement ("Zoom")->FloatAttribute ("num");
	colorActive = element.FirstChildElement ("Colors") != nullptr;
	gridActive = element.FirstChildElement ("Grid") != nullptr;
	ammoActive = element.FirstChildElement ("Ammo") != nullptr;
	fogActive = element.FirstChildElement ("Fog") != nullptr;
	miniMapZoomFactorActive = element.FirstChildElement ("MinimapZoom") != nullptr;
	rangeActive = element.FirstChildElement ("Range") != nullptr;
	scanActive = element.FirstChildElement ("Scan") != nullptr;
	statusActive = element.FirstChildElement ("Status") != nullptr;
	surveyActive = element.FirstChildElement ("Survey") != nullptr;
	lockActive = element.FirstChildElement ("Lock") != nullptr;
	hitsActive = element.FirstChildElement ("Hitpoints") != nullptr;
	miniMapAttackUnitsOnly = element.FirstChildElement ("TNT") != nullptr;
	chatActive = element.FirstChildElement ("ChatActive") != nullptr;

	selectedUnitIds.clear ();
	lockedUnitIds.clear ();
	if (saveVersion >= cVersion (0, 6))
	{
		unitVideoPlaying = element.FirstChildElement ("UnitVideoPlaying") != nullptr;

		const auto selectedUnitsElement = element.FirstChildElement ("SelectedUnits");
		if (selectedUnitsElement != nullptr)
		{
			auto selectedUnitElement = selectedUnitsElement->FirstChildElement ("SelectedUnit");
			while (selectedUnitElement != nullptr)
			{
				selectedUnitIds.push_back (selectedUnitElement->IntAttribute ("num"));
				selectedUnitElement = selectedUnitElement->NextSiblingElement ("SelectedUnit");
			}
		}

		const auto lockedUnitsElement = element.FirstChildElement ("LockedUnits");
		if (lockedUnitsElement != nullptr)
		{
			auto lockedUnitElement = lockedUnitsElement->FirstChildElement ("LockedUnit");
			while (lockedUnitElement != nullptr)
			{
				lockedUnitIds.push_back (lockedUnitElement->IntAttribute ("num"));
				lockedUnitElement = lockedUnitElement->NextSiblingElement ("LockedUnit");
			}
		}
	}
	else
	{
		unitVideoPlaying = true;

		auto selectedUnitElement = element.FirstChildElement ("SelectedUnit");
		if (selectedUnitElement)
		{
			selectedUnitIds.push_back (selectedUnitElement->IntAttribute ("num"));
		}
	}
}