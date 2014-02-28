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

#ifndef gui_menu_widgets_special_landingpositionselectionmapH
#define gui_menu_widgets_special_landingpositionselectionmapH

#include "../../../../maxrconfig.h"
#include "../clickablewidget.h"
#include "../../../../autosurface.h"
#include "../../../../utility/signal/signal.h"

class cStaticMap;
struct sTerrain;

class cLandingPositionSelectionMap : public cClickableWidget
{
public:
	cLandingPositionSelectionMap (const cBox<cPosition>& area, std::shared_ptr<cStaticMap> map);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;

	cSignal<void (const cPosition&)> clickedTile;
protected:
	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

private:
	AutoSurface mapSurface;

	std::shared_ptr<cStaticMap> map;

	const sTerrain* getMapTile (const cPosition& position, cPosition& tilePosition);
	bool isAllowedTerrain (const sTerrain& terrain);
};

#endif // gui_menu_widgets_special_landingpositionselectionmapH
