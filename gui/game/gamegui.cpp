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

#include <sstream>
#include <iomanip>

#include "gamegui.h"
#include "hud.h"
#include "gamemapwidget.h"
#include "minimapwidget.h"

#include "../application.h"
#include "../menu/dialogs/dialogpreferences.h"

#include "../../keys.h"
#include "../../player.h"
#include "../../map.h"
#include "../../vehicles.h"
#include "../../buildings.h"
#include "../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cNewGameGUI::cNewGameGUI (std::shared_ptr<const cStaticMap> staticMap_) :
	cWindow (nullptr),
	staticMap (std::move (staticMap_)),
	dynamicMap (nullptr)
{
	auto hudOwning = std::make_unique<cHud> ();

	resize (hudOwning->getSize ());

	gameMap = addChild (std::make_unique<cGameMapWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap));

	hud = addChild (std::move (hudOwning));

	miniMap = addChild (std::make_unique<cMiniMapWidget> (cBox<cPosition> (cPosition (15, 356), cPosition (15 + 112, 356 + 112)), staticMap));

	signalConnectionManager.connect (hud->preferencesClicked, std::bind (&cNewGameGUI::showPreferencesDialog, this));
	signalConnectionManager.connect (hud->filesClicked, std::bind (&cNewGameGUI::showFilesMenu, this));

	signalConnectionManager.connect (hud->zoomChanged, [&](){ gameMap->setZoomFactor (hud->getZoomFactor (), true); });

	signalConnectionManager.connect (hud->surveyToggled, [&](){ gameMap->setDrawSurvey (hud->getSurveyActive ()); });
	signalConnectionManager.connect (hud->hitsToggled, [&](){ gameMap->setDrawHits (hud->getHitsActive ()); });
	signalConnectionManager.connect (hud->scanToggled, [&](){ gameMap->setDrawScan (hud->getScanActive ()); });
	signalConnectionManager.connect (hud->statusToggled, [&](){ gameMap->setDrawStatus (hud->getStatusActive ()); });
	signalConnectionManager.connect (hud->ammoToggled, [&](){ gameMap->setDrawAmmo (hud->getAmmoActive ()); });
	signalConnectionManager.connect (hud->gridToggled, [&](){ gameMap->setDrawGrid (hud->getGridActive ()); });
	signalConnectionManager.connect (hud->colorToggled, [&](){ gameMap->setDrawColor (hud->getColorActive ()); });
	signalConnectionManager.connect (hud->rangeToggled, [&](){ gameMap->setDrawRange (hud->getRangeActive ()); });
	signalConnectionManager.connect (hud->fogToggled, [&](){ gameMap->setDrawFog (hud->getFogActive ()); });

	signalConnectionManager.connect (hud->miniMapZoomFactorToggled, [&](){ miniMap->setZoomFactor (hud->getMiniMapZoomFactorActive () ? 2 : 1); });

	using namespace std::placeholders;

	signalConnectionManager.connect (gameMap->scrolled, std::bind(&cNewGameGUI::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->zoomFactorChanged, std::bind (&cNewGameGUI::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->tileClicked, std::bind (&cNewGameGUI::handleTileClicked, this, _1));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cNewGameGUI::updateHudCoordinates, this, _1));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cNewGameGUI::updateHudUnitName, this, _1));

	signalConnectionManager.connect (miniMap->focus, [&](const cPosition& position){ gameMap->centerAt(position); });
}

//------------------------------------------------------------------------------
void cNewGameGUI::updateHudCoordinates (const cPosition& tilePosition)
{
	std::stringstream coordsString;
	coordsString << std::setw (3) << std::setfill ('0') << tilePosition.x () << "-" << std::setw (3) << std::setfill ('0') << tilePosition.y ();
	hud->setCoordinatesText (coordsString.str ());
}

//------------------------------------------------------------------------------
void cNewGameGUI::updateHudUnitName (const cPosition& tilePosition)
{
	std::string unitNameString;
	if (dynamicMap && (!player || player->canSeeAt (tilePosition)))
	{
		const auto field = dynamicMap->getField (tilePosition);

		cUnit* unit = nullptr;
		if (field.getVehicle () != nullptr) unit = field.getVehicle ();
		else if (field.getPlane () != nullptr) unit = field.getPlane ();
		else if (field.getTopBuilding () != nullptr) unit = field.getTopBuilding ();
		else if (field.getBaseBuilding () != nullptr && field.getBaseBuilding ()->owner) unit = field.getBaseBuilding ();

		if (unit != nullptr)
		{
			// FIXME: string may be to long.
			unitNameString = unit->getDisplayName () + " (" + unit->owner->getName () + ")";
		}
	}
	hud->setUnitNameText (unitNameString);
}

//------------------------------------------------------------------------------
void cNewGameGUI::handleTileClicked (const cPosition& tilePosition)
{

}

//------------------------------------------------------------------------------
void cNewGameGUI::setDynamicMap (const cMap* dynamicMap_)
{
	dynamicMap = dynamicMap_;
	gameMap->setDynamicMap (dynamicMap);
	miniMap->setDynamicMap (dynamicMap);
}

//------------------------------------------------------------------------------
void cNewGameGUI::setPlayer (const cPlayer* player_)
{
	player = player_;
	gameMap->setPlayer (player);
	miniMap->setPlayer (player);
}

//------------------------------------------------------------------------------
bool cNewGameGUI::handleMouseMoved (cApplication& application, cMouse& mouse)
{
	return cWindow::handleMouseMoved (application, mouse);
}

//------------------------------------------------------------------------------
bool cNewGameGUI::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
{
	const auto oldZoomFactor = gameMap->getZoomFactor ();
	bool consumed = false;
	if (amount.y () > 0)
	{
		hud->decreaseZoomFactor (0.05);
		consumed = true;
	}
	else if (amount.y () < 0)
	{
		hud->increaseZoomFactor (0.05);
		consumed = true;
	}

	// scroll so that the zoom has been performed to center the mouse
	const auto newZoomFactor = gameMap->getZoomFactor ();
	if (newZoomFactor != oldZoomFactor)
	{
		cPosition scrollOffset;

		const auto oldScreenPixelX = gameMap->getSize ().x () / oldZoomFactor;
		const auto newScreenPixelX = gameMap->getSize ().x () / newZoomFactor;
		scrollOffset.x () = (int)((oldScreenPixelX - newScreenPixelX) * (mouse.getPosition ().x () - gameMap->getPosition ().x ()) / gameMap->getSize ().x () - (oldScreenPixelX - newScreenPixelX) / 2);

		const auto oldScreenPixelY = gameMap->getSize ().y () / oldZoomFactor;
		const auto newScreenPixelY = gameMap->getSize ().y () / newZoomFactor;
		scrollOffset.y () = (int)((oldScreenPixelY - newScreenPixelY) * (mouse.getPosition ().y () - gameMap->getPosition ().y ()) / gameMap->getSize ().y () - (oldScreenPixelY - newScreenPixelY) / 2);

		gameMap->scroll (scrollOffset);
	}

	return consumed;
}

//------------------------------------------------------------------------------
bool cNewGameGUI::handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	if (key == KeysList.KeyScroll1)
	{
		gameMap->scroll (cPosition (-cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll3)
	{
		gameMap->scroll (cPosition (+cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll7)
	{
		gameMap->scroll (cPosition (-cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll9)
	{
		gameMap->scroll (cPosition (+cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll2a || key == KeysList.KeyScroll2b)
	{
		gameMap->scroll (cPosition (0, +cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	if (key == KeysList.KeyScroll4a || key == KeysList.KeyScroll4b)
	{
		gameMap->scroll (cPosition (-cSettings::getInstance ().getScrollSpeed (), 0));
		return true;
	}
	if (key == KeysList.KeyScroll6a || key == KeysList.KeyScroll6b)
	{
		gameMap->scroll (cPosition (+cSettings::getInstance ().getScrollSpeed (), 0));
		return true;
	}
	if (key == KeysList.KeyScroll8a || key == KeysList.KeyScroll8b)
	{
		gameMap->scroll (cPosition (0, -cSettings::getInstance ().getScrollSpeed ()));
		return true;
	}
	return cWindow::handleKeyPressed(application, keyboard, key);
}

//------------------------------------------------------------------------------
void cNewGameGUI::handleLooseMouseFocus (cApplication& application)
{

}

//------------------------------------------------------------------------------
void cNewGameGUI::resetMiniMapViewWindow ()
{
	miniMap->setViewWindow (gameMap->getDisplayedMapArea ());
}

//------------------------------------------------------------------------------
void cNewGameGUI::showFilesMenu ()
{

}

//------------------------------------------------------------------------------
void cNewGameGUI::showPreferencesDialog ()
{
	auto application = getActiveApplication ();
	if (!application) return;

	auto preferencesDialog = std::make_shared<cDialogNewPreferences> ();
	application->show (preferencesDialog);
}
