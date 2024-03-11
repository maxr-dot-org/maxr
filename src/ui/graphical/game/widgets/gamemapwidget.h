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

#ifndef ui_graphical_game_widgets_gamemapwidgetH
#define ui_graphical_game_widgets_gamemapwidgetH

#include "game/data/gui/unitlocklist.h"
#include "game/data/gui/unitselection.h"
#include "game/logic/fxeffects.h"
#include "game/logic/upgradecalculator.h"
#include "ui/graphical/game/control/mousemode/mousemodetype.h"
#include "ui/graphical/game/temp/unitdrawingengine.h"
#include "ui/graphical/game/unitselectionbox.h"
#include "ui/widgets/clickablewidget.h"
#include "utility/signal/signal.h"

#include <map>
#include <set>

struct SDL_Surface;

class cStaticMap;
class cMapView;
class cPlayer;
class cUnitSelection;
class cUnitContextMenuWidget;
class cFx;
class cMouseMode;
class cSoundManager;
class cDrawingCache;
class cAnimation;
class cRightMouseButtonScrollerWidget;
class cFrameCounter;

enum class eStart;

class cGameMapWidget : public cClickableWidget
{
	friend class cDebugOutputWidget;

public:
	cGameMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap>, std::shared_ptr<cAnimationTimer>, std::shared_ptr<cSoundManager>, std::shared_ptr<const cFrameCounter>);
	~cGameMapWidget();
	void setPlayer (std::shared_ptr<const cPlayer>);
	void setUnitsData (std::shared_ptr<const cUnitsData>);

	void setZoomFactor (float zoomFactor, bool center);
	float getZoomFactor() const;

	float computeMinimalZoomFactor() const;

	void setMapView (std::shared_ptr<const cMapView>);
	const cMapView& getMapView() const;

	cUnitSelection& getUnitSelection();
	const cUnitSelection& getUnitSelection() const;

	cUnitLockList& getUnitLockList();
	const cUnitLockList& getUnitLockList() const;

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
	const cPosition& getPixelOffset() const;

	void centerAt (const cPosition&);
	cPosition getMapCenterOffset();

	bool startFindBuildPosition (const sID& buildId);
	void startFindPathBuildPosition();
	void startActivateVehicle (size_t index);

	void addEffect (std::shared_ptr<cFx>, bool playSound = true);

	void setDrawSurvey (bool drawSurvey);
	void setDrawHits (bool drawHits);
	void setDrawScan (bool drawScan);
	void setDrawStatus (bool drawStatus);
	void setDrawAmmo (bool drawAmmo);
	void setDrawGrid (bool drawGrid);
	void setDrawColor (bool drawColor);
	void setDrawRange (bool drawRange);
	void setDrawFog (bool drawFog);
	void setLockActive (bool lockActive);

	void toggleHelpMode();

	cBox<cPosition> getDisplayedMapArea() const;

	void updateMouseCursor();
	void updateMouseCursor (cMouse&);

	void setChangeAllowed (bool value);
	bool isChangeAllowed() const { return changeAllowed; }

	cDrawingCache& getDrawingCache();
	const cDrawingCache& getDrawingCache() const;

	void updateActiveUnitCommandShortcuts();
	void deactivateUnitCommandShortcuts();

	cSignal<void()> scrolled;
	cSignal<void()> zoomFactorChanged;

	cSignal<void (const cPosition&)> tileClicked;
	cSignal<void (const cPosition&)> tileUnderMouseChanged;

	cSignal<void()> mouseInputModeChanged;

	cSignal<void()> mouseFocusReleased;

	//
	// Game commands
	//
	cSignal<void (const cUnit&)> triggeredUnitHelp;
	cSignal<void (const cUnit&, const cUnit&)> triggeredTransfer;
	cSignal<void (const cVehicle&, const cPosition&)> triggeredEndBuilding;
	cSignal<void (const cVehicle&, const cPosition&, eStart)> triggeredMoveSingle;
	cSignal<void (const std::vector<cVehicle*>&, const cPosition&, eStart)> triggeredMoveGroup;
	cSignal<void (const cVehicle&, const cPosition&)> selectedBuildPosition;
	cSignal<void (const cVehicle&, const cPosition&)> selectedBuildPathDestination;
	cSignal<void (const cUnit&, size_t, const cPosition&)> triggeredActivateAt;
	cSignal<void (const cBuilding&, const cPosition&)> triggeredExitFinishedUnit;
	cSignal<void (const cUnit&, const cPosition&)> triggeredLoadAt;
	cSignal<void (const cUnit&, const cUnit&)> triggeredSupplyAmmo;
	cSignal<void (const cUnit&, const cUnit&)> triggeredRepair;
	cSignal<void (const cUnit&, const cPosition&)> triggeredAttack;
	cSignal<void (const cVehicle&, const cUnit&)> triggeredSteal;
	cSignal<void (const cVehicle&, const cUnit&)> triggeredDisable;

	cSignal<void (const cUnit&)> triggeredBuild;
	cSignal<void (const cUnit&)> triggeredResourceDistribution;
	cSignal<void (const cBuilding&)> triggeredStartWork;
	cSignal<void (const cUnit&)> triggeredStopWork;
	cSignal<void (const cUnit&)> triggeredAutoMoveJob;
	cSignal<void (const cVehicle&)> triggeredStartClear;
	cSignal<void (const cUnit&)> triggeredManualFire;
	cSignal<void (const cUnit&)> triggeredSentry;
	cSignal<void (const cUnit&)> triggeredActivate;
	cSignal<void (const cUnit&)> triggeredResearchMenu;
	cSignal<void (const cUnit&)> triggeredUpgradesMenu;
	cSignal<void (const cBuilding&)> triggeredUpgradeThis;
	cSignal<void (const cBuilding&)> triggeredUpgradeAll;
	cSignal<void (const cBuilding&)> triggeredSelfDestruction;
	cSignal<void (const cVehicle&)> triggeredLayMines;
	cSignal<void (const cVehicle&)> triggeredCollectMines;
	cSignal<void (const cUnit&)> triggeredUnitDone;

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;
	bool handleMousePressed (cApplication&, cMouse&, eMouseButtonType) override;
	bool handleMouseReleased (cApplication&, cMouse&, eMouseButtonType) override;
	void handleLooseMouseFocus (cApplication&) override;
	void handleResized (const cPosition& oldSize) override;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;
	bool acceptButton (eMouseButtonType) const override;

	//
	// draw methods
	//
	void drawTerrain();
	void drawGrid();
	void drawEffects (bool bottom);

	void drawBaseUnits();
	void drawTopBuildings();
	void drawShips();
	void drawAboveSeaBaseUnits();
	void drawVehicles();
	void drawConnectors();
	void drawPlanes();

	void drawResources();

	void drawPath (const cVehicle&);
	void drawPathArrow (SDL_Rect dest, const SDL_Rect& lastDest, bool spezialColor) const;
	void drawBuildPath (const cVehicle&);

	void drawSelectionBox();

	void drawUnitCircles();
	void drawLockList();

	void drawExitPoints();
	void drawExitPoint (const cPosition&);
	void drawExitPointsIf (const cUnit&, const std::function<bool (const cPosition&)>& predicate);
	void drawBuildBand();

	bool shouldDrawUnit (const cUnit&, const cPosition& visitingPosition, const std::pair<cPosition, cPosition>& tileDrawingRange);

	//
	// position handling methods
	//
	cPosition computeMaximalPixelOffset() const;

	//
	// drawing helper methods
	//
	cPosition zoomSize (const cPosition& size, float zoomFactor) const;
	cPosition getZoomedTileSize() const;
	cPosition getZoomedStartTilePixelOffset() const;
	std::pair<cPosition, cPosition> computeTileDrawingRange() const;

	SDL_Rect computeTileDrawingArea (const cPosition& zoomedTileSize, const cPosition& zoomedStartTilePixelOffset, const cPosition& tileStartIndex, const cPosition& tileIndex) const;

	cPosition getMapTilePosition (const cPosition& pixelPosition) const;
	cPosition getScreenPosition (const cUnit&, bool movementOffset = true) const;

	void updateActiveAnimations();
	void updateActiveAnimations (const std::pair<cPosition, cPosition>& oldTileDrawingRange);
	void addAnimationsForUnit (const cUnit&);

	void updateUnitMenuPosition();

	void toggleUnitContextMenu (const cUnit*);

	void setMouseInputMode (std::unique_ptr<cMouseMode> newMouseMode);
	void toggleMouseInputMode (eMouseModeType mouseInputMode);

	void runOwnedEffects();

	void renewDamageEffects();
	void renewDamageEffect (const cBuilding&);
	void renewDamageEffect (const cVehicle&);

	void setWindDirection (int direction);
	void changeWindDirection();

	void buildCollidingShortcutsMap();
	void activateShortcutConditional (cShortcut&, std::set<const cShortcut*>& blockedShortcuts, const std::set<const cShortcut*>& collidingShortcuts);

public:
	std::vector<cResearch::eResearchArea> currentTurnResearchAreasFinished;

private:
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager mapViewSignalConnectionManager;
	cSignalConnectionManager mouseModeSignalConnectionManager;
	cSignalConnectionManager unitContextMenuSignalConnectionManager;
	cSignalConnectionManager selectedUnitSignalConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;
	std::shared_ptr<cSoundManager> soundManager;

	std::shared_ptr<const cStaticMap> staticMap;
	std::shared_ptr<const cMapView> mapView; // may be null
	std::shared_ptr<const cPlayer> player; // may be null
	std::shared_ptr<const cUnitsData> unitsData;

	cUnitDrawingEngine unitDrawingEngine;

	cUnitSelection unitSelection;
	cUnitSelectionBox unitSelectionBox;

	cUnitLockList unitLockList;

	cUnitContextMenuWidget* unitMenu = nullptr;

	std::unique_ptr<cMouseMode> mouseMode;

	bool changeAllowed = true;

	std::vector<std::shared_ptr<cFx>> effects;

	std::vector<std::unique_ptr<cAnimation>> animations;

	//
	// unit command shortcuts
	//
	cShortcut* attackShortcut = nullptr;
	cShortcut* buildShortcut = nullptr;
	cShortcut* transferShortcut = nullptr;
	cShortcut* automoveShortcut = nullptr;
	cShortcut* startShortcut = nullptr;
	cShortcut* stopShortcut = nullptr;
	cShortcut* clearShortcut = nullptr;
	cShortcut* sentryShortcut = nullptr;
	cShortcut* manualFireShortcut = nullptr;
	cShortcut* activateShortcut = nullptr;
	cShortcut* loadShortcut = nullptr;
	cShortcut* enterShortcut = nullptr;
	cShortcut* relaodShortcut = nullptr;
	cShortcut* repairShortcut = nullptr;
	cShortcut* layMineShortcut = nullptr;
	cShortcut* clearMineShortcut = nullptr;
	cShortcut* disableShortcut = nullptr;
	cShortcut* stealShortcut = nullptr;
	cShortcut* infoShortcut = nullptr;
	cShortcut* distributeShortcut = nullptr;
	cShortcut* researchShortcut = nullptr;
	cShortcut* upgradeShortcut = nullptr;
	cShortcut* destroyShortcut = nullptr;

	std::map<const cShortcut*, std::set<const cShortcut*>> collidingUnitCommandShortcuts;

	//
	// drawing information data
	//
	cPosition pixelOffset;
	float internalZoomFactor = 1.f; // should not be used directly! use getZoomFactor() instead!

	bool shouldDrawSurvey = false;
	bool shouldDrawScan = false;
	bool shouldDrawGrid = false;
	bool shouldDrawRange = false;
	bool shouldDrawFog = false;

	bool lockActive = false;

	float windDirection;

	cRightMouseButtonScrollerWidget* rightMouseButtonScrollerWidget = nullptr;
};

#endif // ui_graphical_game_widgets_gamemapwidgetH
