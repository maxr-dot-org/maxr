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

#ifndef gui_game_widgets_minimapwidgetH
#define gui_game_widgets_minimapwidgetH

#include "../../widget.h"
#include "../../../maxrconfig.h"
#include "../../../utility/signal/signal.h"
#include "../../../utility/signal/signalconnectionmanager.h"

struct SDL_Surface;

class cStaticMap;
class cMap;
class cPlayer;

class cMiniMapWidget : public cWidget
{
public:
	cMiniMapWidget (const cBox<cPosition>& area, std::shared_ptr<const cStaticMap> staticMap);

	void setDynamicMap (std::shared_ptr<const cMap> dynamicMap);
	void setPlayer (std::shared_ptr<const cPlayer> player);

	void setViewWindow (const cBox<cPosition>& viewWindow);

	void setAttackUnitsUnly (bool attackUnitsOnly);

	void setZoomFactor (int zoomFactor);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

	cSignal<void (const cPosition&)> focus;
protected:

private:
	cSignalConnectionManager dynamicMapSignalConnectionManager;

	AutoSurface surface;
	bool surfaceOutdated;
	AutoSurface viewWindowSurface;
	bool viewWindowSurfaeOutdated;

	std::shared_ptr<const cStaticMap> staticMap;
	std::shared_ptr<const cMap> dynamicMap; // may be null
	std::shared_ptr<const cPlayer> player; // may be null

	int zoomFactor; // TODO: may use floating value here
	cPosition offset;
	bool attackUnitsOnly;
	cBox<cPosition> mapViewWindow;

	bool updateOffset ();

	void renewSurface ();
	void drawLandscape ();
	void drawFog ();
	void drawUnits ();

	void renewViewWindowSurface ();

	cPosition computeMapPosition (const cPosition& screenPosition);
};


#endif // gui_game_widgets_minimapwidgetH
