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

#include "windowlandingpositionselection.h"
#include "../../widgets/image.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/special/landingpositionselectionmap.h"
#include "../../../game/hud.h"
#include "../../../../sound.h"
#include "../../../../video.h"
#include "../../../../main.h"
#include "../../../../input/mouse/mouse.h"
#include "../../../../input/mouse/cursor/mousecursorsimple.h"

//------------------------------------------------------------------------------
cWindowLandingPositionSelection::cWindowLandingPositionSelection (std::shared_ptr<cStaticMap> staticMap) :
	cWindow (nullptr),
	lastSelectedPosition (0, 0),
	firstActivate (true)
{
	using namespace std::placeholders;

	auto hudImageOwned = std::make_unique<cImage> (cPosition (0, 0), createHudSurface ());
	hudImageOwned->disableAtTransparent ();

	map = addChild (std::make_unique<cLandingPositionSelectionMap> (cBox<cPosition> (cPosition (180, 18), hudImageOwned->getEndPosition () - cPosition (12, 14)), std::move (staticMap)));
	signalConnectionManager.connect (map->clickedTile, std::bind (&cWindowLandingPositionSelection::mapClicked, this, _1));

	auto hudImage = addChild (std::move (hudImageOwned));

	auto backButton = addChild (std::make_unique<cPushButton> (cPosition (35, hudImage->getEndPosition ().y () - 40), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Back"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowLandingPositionSelection::backClicked, this));

	signalConnectionManager.connect (selectedPosition, [&](const cPosition& position){ lastSelectedPosition = position; });

	resize (hudImage->getSize ());
}

//------------------------------------------------------------------------------
cWindowLandingPositionSelection::~cWindowLandingPositionSelection ()
{}

//------------------------------------------------------------------------------
const cPosition& cWindowLandingPositionSelection::getSelectedPosition () const
{
	return lastSelectedPosition;
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::handleActivated (cApplication& application)
{
	if (firstActivate) PlayRandomVoice (VoiceData.VOILanding);
	firstActivate = false;
	cWindow::handleActivated (application);
}

//------------------------------------------------------------------------------
bool cWindowLandingPositionSelection::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (map->isAt (mouse.getPosition ()))
	{
		return map->handleMouseMoved (application, mouse, offset);
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	}
	return false;
}

//------------------------------------------------------------------------------
SDL_Surface* cWindowLandingPositionSelection::createHudSurface ()
{
	AutoSurface hudSurface (cHud::generateSurface ());

	SDL_Rect top, bottom;
	top.x = 0;
	top.y = (Video.getResolutionY () / 2) - 479;

	bottom.x = 0;
	bottom.y = (Video.getResolutionY () / 2);

	SDL_BlitSurface (GraphicsData.gfx_panel_top.get (), NULL, hudSurface.get (), &top);
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom.get (), NULL, hudSurface.get (), &bottom);

	return hudSurface.Release ();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::backClicked ()
{
	close ();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::mapClicked (const cPosition& tilePosition)
{
	selectedPosition (tilePosition);
}
