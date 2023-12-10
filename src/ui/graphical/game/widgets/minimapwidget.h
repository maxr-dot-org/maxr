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

#ifndef ui_graphical_game_widgets_minimapwidgetH
#define ui_graphical_game_widgets_minimapwidgetH

#include "ui/widgets/clickablewidget.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

struct SDL_Surface;

class cStaticMap;
class cMapView;

class cMiniMapWidget : public cClickableWidget
{
public:
	cMiniMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap>);

	//	void setDynamicMap (std::shared_ptr<const cMap> dynamicMap);
	//	void setPlayer (std::shared_ptr<const cPlayer>);
	void setMapView (std::shared_ptr<const cMapView>);

	void setViewWindow (const cBox<cPosition>& viewWindow);

	void setAttackUnitsUnly (bool attackUnitsOnly);

	void setZoomFactor (int zoomFactor);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;
	bool handleMousePressed (cApplication&, cMouse&, eMouseButtonType) override;
	bool handleMouseReleased (cApplication&, cMouse&, eMouseButtonType) override;
	void handleLooseMouseFocus (cApplication&) override;

	cSignal<void (const cPosition&)> focus;
	cSignal<void (const cPosition&)> triggeredMove;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;
	bool acceptButton (eMouseButtonType) const override;

private:
	cSignalConnectionManager dynamicMapSignalConnectionManager;

	UniqueSurface surface;
	bool surfaceOutdated;
	UniqueSurface viewWindowSurface;
	bool viewWindowSurfaeOutdated;

	bool startedMoving;

	std::shared_ptr<const cStaticMap> staticMap;
	std::shared_ptr<const cMapView> mapView; // may be null
	//	std::shared_ptr<const cMap> dynamicMap; // may be null
	//	std::shared_ptr<const cPlayer> player; // may be null

	int zoomFactor; // TODO: may use floating value here
	cPosition offset;
	bool attackUnitsOnly;
	cBox<cPosition> mapViewWindow;

	bool updateOffset();

	void renewSurface();
	void drawLandscape();
	void drawFog();
	void drawUnits();

	void renewViewWindowSurface();

	cPosition computeMapPosition (const cPosition& screenPosition);
};

#endif // ui_graphical_game_widgets_minimapwidgetH
