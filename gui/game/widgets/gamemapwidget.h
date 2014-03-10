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

#ifndef gui_game_widgets_gamemapwidgetH
#define gui_game_widgets_gamemapwidgetH

#include "../unitselection.h"
#include "../unitselectionbox.h"
#include "../mouseinputmode.h"
#include "../temp/unitdrawingengine.h"
#include "../../menu/widgets/clickablewidget.h"
#include "../../../maxrconfig.h"
#include "../../../utility/signal/signal.h"
#include "../../../fxeffects.h"

struct SDL_Surface;

class cStaticMap;
class cMap;
class cPlayer;
class cUnitSelection;
class cUnitContextMenuWidget;
class cFx;

enum class eMouseClickAction
{
	SelectBuildPosition,
	SelectBuildPathDestintaion,
	Transfer,
	Help,
	Attack,
	Disable,
	Steal,
	SupplyAmmo,
	Repair,
	Select,
	Load,
	ActivateStored,
	ActivateFinished,
	Move,
	None,
	Unknown
};

class cGameMapWidget : public cClickableWidget
{
public:
	cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap, std::shared_ptr<cAnimationTimer> animationTimer);

	void setDynamicMap (const cMap* dynamicMap);
	void setPlayer (const cPlayer* player);
	void setUnitSelection (const cUnitSelection* unitSelection);

	void setZoomFactor (double zoomFactor, bool center);
	double getZoomFactor () const;

	cUnitSelection& getUnitSelection ();
	const cUnitSelection& getUnitSelection () const;

	/**
	 * Scrolls the map by a given offset.
	 *
	 * The scrolling is save, which means if the offset is to large
	 * the new offset position will be adjusted in such a way that the
	 * displayed area will not go out of the map range.
	 *
	 * @param offset The offset in pixels to scroll. Can be negative.
	 */
	void scroll (const cPosition& offset);
	const cPosition& getPixelOffset () const;

	void centerAt (const cPosition& position);

	void startFindBuildPosition (const sID& buildId);
	void startFindPathBuildPosition ();

	void addEffect (std::shared_ptr<cFx> effect);

	void setDrawSurvey (bool drawSurvey);
	void setDrawHits (bool drawHits);
	void setDrawScan (bool drawScan);
	void setDrawStatus (bool drawStatus);
	void setDrawAmmo (bool drawAmmo);
	void setDrawGrid (bool drawGrid);
	void setDrawColor (bool drawColor);
	void setDrawRange (bool drawRange);
	void setDrawFog (bool drawFog);

	void toggleHelpMode ();

	cBox<cPosition> getDisplayedMapArea () const;

	void updateMouseCursor ();
	void updateMouseCursor (cMouse& mouse);

	cSignal<void ()> scrolled;
	cSignal<void ()> zoomFactorChanged;

	cSignal<void (const cPosition&)> tileClicked;
	cSignal<void (const cPosition&)> tileUnderMouseChanged;

	cSignal<void ()> mouseInputModeChanged;

	//
	// Game commands
	//
	cSignal<void (const cUnit&)> triggeredUnitHelp;
	cSignal<void (const cUnit&, const cUnit&)> triggeredTransfer;
	cSignal<void (cVehicle&, const cPosition&)> triggeredEndBuilding;
	cSignal<void (cVehicle&, const cPosition&)> triggeredMoveSingle;
	cSignal<void (const std::vector<cVehicle*>&, const cPosition&)> triggeredMoveGroup;
	cSignal<void (const cVehicle&, const cPosition&)> selectedBuildPosition;
	cSignal<void (const cVehicle&, const cPosition&)> selectedBuildPathDestination;
	cSignal<void (const cUnit&, const cPosition&)> triggeredActivateAt;
	cSignal<void (const cBuilding&, const cPosition&)> triggeredExitFinishedUnit;
	cSignal<void (const cUnit&, const cPosition&)> triggeredLoadAt;
	cSignal<void (const cUnit&, const cUnit&)> triggeredSupplyAmmo;
	cSignal<void (const cUnit&, const cUnit&)> triggeredRepair;
	cSignal<void (const cUnit&, const cPosition&)> triggeredAttack;
	cSignal<void (const cUnit&, const cUnit&)> triggeredSteal;
	cSignal<void (const cUnit&, const cUnit&)> triggeredDisable;

	cSignal<void (const cPosition&)> triggeredToggleUnitLock;

	cSignal<void (const cUnit&)> triggeredBuild;
	cSignal<void (const cUnit&)> triggeredResourceDistribution;
	cSignal<void (const cUnit&)> triggeredStartWork;
	cSignal<void (const cUnit&)> triggeredStopWork;
	cSignal<void (const cUnit&)> triggeredAutoMoveJob;
	cSignal<void (const cUnit&)> triggeredStartClear;
	cSignal<void (const cUnit&)> triggeredManualFire;
	cSignal<void (const cUnit&)> triggeredSentry;
	cSignal<void (const cUnit&)> triggeredActivate;
	cSignal<void (const cUnit&)> triggeredResearchMenu;
	cSignal<void (const cUnit&)> triggeredUpgradesMenu;
	cSignal<void (const cUnit&)> triggeredUpgradeThis;
	cSignal<void (const cUnit&)> triggeredUpgradeAll;
	cSignal<void (const cUnit&)> triggeredSelfDestruction;
	cSignal<void (const cUnit&)> triggeredLayMines;
	cSignal<void (const cUnit&)> triggeredCollectMines;
	cSignal<void (const cUnit&)> triggeredUnitDone;

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
protected:
	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

	virtual bool acceptButton (eMouseButtonType button) const MAXR_OVERRIDE_FUNCTION;
private:
	//
	// data
	//
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager dynamicMapSignalConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;

	std::shared_ptr<const cStaticMap> staticMap;
	const cMap* dynamicMap; // may be null
	const cPlayer* player; // may be null

	cUnitDrawingEngine unitDrawingEngine;

	cUnitSelection unitSelection;
	cUnitSelectionBox unitSelectionBox;

	cUnitContextMenuWidget* unitMenu;

	eNewMouseInputMode mouseInputMode;
	sID currentBuildUnitId;

	std::vector<std::shared_ptr<cFx>> effects;

	//
	// drawing information data
	//
	cPosition pixelOffset;
	double internalZoomFactor; // should not be used directly! use getZoomFactor() instead!

	bool shouldDrawSurvey;
	bool shouldDrawScan;
	bool shouldDrawGrid;
	bool shouldDrawRange;
	bool shouldDrawFog;

	bool lockActive;

	//
	// draw methods
	//
	void drawTerrain ();
	void drawGrid ();
	void drawEffects (bool bottom);

	void drawBaseUnits ();
	void drawTopBuildings ();
	void drawShips ();
	void drawAboveSeaBaseUnits ();
	void drawVehicles ();
	void drawConnectors ();
	void drawPlanes ();

	void drawResources ();

	void drawPath (const cVehicle& vehicle);
	void drawBuildPath (const cVehicle& vehicle);

	void drawSelectionBox ();

	void drawUnitCircles ();
	void drawLockList (const cPlayer& player);

	void drawExitPoints ();
	void drawExitPoint (const cPosition& position);
	void drawExitPointsIf (const cUnit& unit, const std::function<bool (const cPosition&)>& predicate);
	void drawBuildBand ();

	void drawAttackCursor (const cPosition& position) const;
	void drawCommandoCursor (const cPosition& position, const cVehicle& vehicle, bool steal) const;

	void addEffect ();
	//
	// position handling methods
	//
	double computeMinimalZoomFactor () const;
	cPosition computeMaximalPixelOffset () const;

	//
	// drawing helper methods
	//
	cPosition zoomSize (const cPosition& size, double zoomFactor) const;
	cPosition getZoomedTileSize () const;
	cPosition getZoomedStartTilePixelOffset () const;
	std::pair<cPosition, cPosition> computeTileDrawingRange () const;

	SDL_Rect computeTileDrawingArea (const cPosition& zoomedTileSize, const cPosition& zoomedStartTilePixelOffset, const cPosition& tileStartIndex, const cPosition& tileIndex);

	cPosition getMapTilePosition (const cPosition& pixelPosition) const;
	cPosition getScreenPosition (const cUnit& unit, bool movementOffset = true) const;

	void updateUnitMenuPosition ();

	void toggleUnitContextMenu (const cUnit* unit);

	void setMouseInputMode (eNewMouseInputMode mouseInputMode);
	void toggleMouseInputMode (eNewMouseInputMode mouseInputMode);

	eMouseClickAction getMouseClickAction (const cMouse& mouse);

	std::pair<bool, cPosition> findNextBuildPosition (const cPosition& sourcePosition, const cPosition& desiredPosition, const sID& unitId);
};

#endif // gui_game_widgets_gamemapwidgetH
