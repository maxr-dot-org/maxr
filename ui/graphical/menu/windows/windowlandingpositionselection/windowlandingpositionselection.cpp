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

#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/landingpositionselectionmap.h"
#include "ui/graphical/game/hud.h"
#include "sound.h"
#include "video.h"
#include "main.h"
#include "map.h"
#include "input/mouse/mouse.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "game/logic/landingpositionmanager.h"
#include "output/sound/sounddevice.h"
#include "output/sound/soundchannel.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
cWindowLandingPositionSelection::cWindowLandingPositionSelection (std::shared_ptr<cStaticMap> staticMap_) :
	cWindow (nullptr),
	staticMap (std::move(staticMap_)),
	selectionAllowed (true),
	lastSelectedPosition (0, 0)
{
	using namespace std::placeholders;

    auto hudImageOwned = std::make_unique<cImage> (cPosition (0, 0), createHudSurface ().get ());
	hudImageOwned->disableAtTransparent ();

	mapWidget = addChild (std::make_unique<cLandingPositionSelectionMap> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), hudImageOwned->getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap));
	signalConnectionManager.connect (mapWidget->clickedTile, std::bind (&cWindowLandingPositionSelection::mapClicked, this, _1));

	circlesImage = addChild (std::make_unique<cImage> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight)));
	circlesImage->disable ();

	auto hudImage = addChild (std::move (hudImageOwned));

	backButton = addChild (std::make_unique<cPushButton> (cPosition (35, hudImage->getEndPosition ().y () - 40), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Back"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowLandingPositionSelection::backClicked, this));

	infoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), hudImage->getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), "", FONT_LATIN_BIG, toEnumFlag (eAlignmentType::Center)));
	infoLabel->setWordWrap (true);
	infoLabel->disable ();

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
void cWindowLandingPositionSelection::applyReselectionState (eLandingPositionState state)
{
	if (state == eLandingPositionState::Clear || state == eLandingPositionState::Confirmed) lockBack ();
	else unlockBack ();

	if (state == eLandingPositionState::Warning || state == eLandingPositionState::TooClose) allowSelection ();
	else disallowSelection ();

	if (state == eLandingPositionState::Warning) setInfoMessage (lngPack.i18n ("Text~Comp~Landing_Warning"));
	else if (state == eLandingPositionState::TooClose) setInfoMessage (lngPack.i18n ("Text~Comp~Landing_Too_Close"));
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::setInfoMessage (const std::string& message)
{
	infoLabel->setText (message);
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::allowSelection ()
{
	selectionAllowed = true;
	auto application = getActiveApplication ();
	auto mouse = getActiveMouse ();
	if (application && mouse)
	{
		handleMouseMoved (*application, *mouse, mouse->getPosition ());
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::disallowSelection ()
{
	selectionAllowed = false;
	auto application = getActiveApplication ();
	auto mouse = getActiveMouse ();
	if (application && mouse)
	{
		handleMouseMoved (*application, *mouse, mouse->getPosition ());
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::lockBack ()
{
	backButton->lock ();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::unlockBack ()
{
	backButton->unlock ();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::handleActivated (cApplication& application, bool firstTime)
{
    if (firstTime) cSoundDevice::getInstance ().getFreeVoiceChannel().play (getRandom (VoiceData.VOILanding));
	cWindow::handleActivated (application, firstTime);
}

//------------------------------------------------------------------------------
bool cWindowLandingPositionSelection::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (!selectionAllowed)
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
	else if (mapWidget->isAt (mouse.getPosition ()))
	{
		return mapWidget->handleMouseMoved (application, mouse, offset);
	}
	else
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	}
	return false;
}

//------------------------------------------------------------------------------
AutoSurface cWindowLandingPositionSelection::createHudSurface ()
{
	AutoSurface hudSurface (cHud::generateSurface ());

	SDL_Rect top, bottom;
	top.x = 0;
	top.y = (Video.getResolutionY () / 2) - 479;

	bottom.x = 0;
	bottom.y = (Video.getResolutionY () / 2);

	SDL_BlitSurface (GraphicsData.gfx_panel_top.get (), NULL, hudSurface.get (), &top);
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom.get (), NULL, hudSurface.get (), &bottom);

	return std::move(hudSurface);
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::backClicked ()
{
	canceled ();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::mapClicked (const cPosition& tilePosition)
{
	if (!selectionAllowed) return;

	updateLandingPositionCircles (tilePosition);

	selectedPosition (tilePosition);
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::updateLandingPositionCircles (const cPosition& tilePosition)
{
	AutoSurface circleSurface (SDL_CreateRGBSurface (0, mapWidget->getSize ().x (), mapWidget->getSize ().y(), Video.getColDepth (), 0, 0, 0, 0));
	SDL_FillRect (circleSurface.get (), NULL, 0xFF00FF);
	SDL_SetColorKey (circleSurface.get (), SDL_TRUE, 0xFF00FF);

	const cPosition pixelPosition = tilePosition * mapWidget->getSize () / staticMap->getSize ();

	// for non 4:3 screen resolutions, the size of the circles is
	// only correct in x dimension, because I don't draw an ellipse
	const int warningRadius = static_cast<int>((cLandingPositionManager::warningDistance / 2) * mapWidget->getSize ().x () / staticMap->getSize ().x ());
	const int tooCloseRadius = static_cast<int>((cLandingPositionManager::tooCloseDistance / 2) * mapWidget->getSize ().x () / staticMap->getSize ().x ());
	drawCircle (pixelPosition.x (), pixelPosition.y(), warningRadius, SCAN_COLOR, *circleSurface);
	drawCircle (pixelPosition.x (), pixelPosition.y(), tooCloseRadius, RANGE_GROUND_COLOR, *circleSurface);

	circlesImage->setImage (circleSurface.get());
}
