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

#ifndef ui_graphical_menu_widgets_special_landingpositionselectionmapH
#define ui_graphical_menu_widgets_special_landingpositionselectionmapH

#include "SDLutility/uniquesurface.h"
#include "ui/widgets/clickablewidget.h"
#include "utility/signal/signal.h"

class cStaticMap;
struct sLandingUnit;
class cUnitsData;

class cLandingPositionSelectionMap : public cClickableWidget
{
public:
	cLandingPositionSelectionMap (const cBox<cPosition>& area, std::shared_ptr<cStaticMap>, bool fixedBridgeHead, const std::vector<sLandingUnit>&, std::shared_ptr<const cUnitsData>);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;

	cSignal<void (const cPosition&)> clickedTile;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	UniqueSurface mapSurface;

	std::shared_ptr<cStaticMap> map;
	bool fixedBridgeHead;
	const std::vector<sLandingUnit> landingUnits;
	std::shared_ptr<const cUnitsData> unitsData;

	bool isValidLandingLocation (const cPosition&);
};

#endif // ui_graphical_menu_widgets_special_landingpositionselectionmapH
