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

#include "game/data/map/map.h"
#include "game/data/units/landingunit.h"
#include "game/logic/landingpositionmanager.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/sound.h"
#include "resources/uidata.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/chatboxlandingplayerlistviewitem.h"
#include "ui/graphical/menu/widgets/special/landingpositionselectionmap.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
cWindowLandingPositionSelection::cWindowLandingPositionSelection (std::shared_ptr<cStaticMap> staticMap_, bool fixedBridgeHead, const std::vector<sLandingUnit>& landingUnits, std::shared_ptr<const cUnitsData> unitsData, bool withChatBox) :
	cWindow (nullptr),
	staticMap (std::move (staticMap_)),
	animationTimer (std::make_shared<cAnimationTimer>()),
	lastSelectedPosition (0, 0)
{
	auto hudImageOwned = std::make_unique<cImage> (cPosition (0, 0), createHudSurface().get());
	hudImageOwned->disableAtTransparent();

	mapWidget = emplaceChild<cLandingPositionSelectionMap> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), hudImageOwned->getEndPosition() - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap, fixedBridgeHead, landingUnits, unitsData);
	signalConnectionManager.connect (mapWidget->clickedTile, [this] (const cPosition& tilePos) { mapClicked (tilePos); });

	circlesImage = emplaceChild<cImage> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight));
	circlesImage->disable();

	auto hudImage = addChild (std::move (hudImageOwned));

	backButton = emplaceChild<cPushButton> (cPosition (35, hudImage->getEndPosition().y() - 40), ePushButtonType::Angular, lngPack.i18n ("Others~Back"), eUnicodeFontType::LatinNormal);
	signalConnectionManager.connect (backButton->clicked, [this]() { backClicked(); });

	infoLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), hudImage->getEndPosition() - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), "", eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Center));
	infoLabel->setWordWrap (true);
	infoLabel->disable();

	if (withChatBox)
	{
		chatBox = emplaceChild<cChatBox<cLobbyChatBoxListViewItem, cChatBoxLandingPlayerListViewItem>> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, hudImage->getEndPosition().y() - cHud::panelBottomHeight - 12 - 100), hudImage->getEndPosition() - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12)));

		signalConnectionManager.connect (chatBox->commandEntered, [this] (const std::string& message) {
			onCommandEntered (message);
		});

		toggleChatBoxButton = emplaceChild<cCheckBox> (cPosition (35, hudImage->getEndPosition().y() - 65), lngPack.i18n ("Others~Chat"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left, eCheckBoxType::Angular);
		toggleChatBoxButton->setChecked (true);
		signalConnectionManager.connect (toggleChatBoxButton->toggled, [this]() {
			if (toggleChatBoxButton->isChecked())
			{
				chatBox->enable();
				chatBox->show();
			}
			else
			{
				chatBox->disable();
				chatBox->hide();
			}
		});
	}
	else
	{
		chatBox = nullptr;
	}

	signalConnectionManager.connect (selectedPosition, [this] (const cPosition& position) { lastSelectedPosition = position; });

	resize (hudImage->getSize());
}

//------------------------------------------------------------------------------
cWindowLandingPositionSelection::~cWindowLandingPositionSelection()
{}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::retranslate()
{
	cWindow::retranslate();

	backButton->setText (lngPack.i18n ("Others~Back"));
	if (toggleChatBoxButton)
	{
		toggleChatBoxButton->setText (lngPack.i18n ("Others~Chat"));
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::applyReselectionState (eLandingPositionState state)
{
	reselectionState = state;

	if (state == eLandingPositionState::Clear || state == eLandingPositionState::Confirmed)
		lockBack();
	else
		unlockBack();

	if (state == eLandingPositionState::Warning || state == eLandingPositionState::TooClose)
		allowSelection();
	else
		disallowSelection();

	if (state == eLandingPositionState::Warning)
		setInfoMessage (lngPack.i18n ("Comp~Landing_Warning"));
	else if (state == eLandingPositionState::TooClose)
		setInfoMessage (lngPack.i18n ("Comp~Landing_Too_Close"));
	else
		setInfoMessage ("");
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::setInfoMessage (const std::string& message)
{
	infoLabel->setText (message);
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::addChatPlayerEntry (const cPlayerLandingStatus& playerLandingStatus)
{
	if (chatBox) chatBox->addPlayerEntry (std::make_unique<cChatBoxLandingPlayerListViewItem> (playerLandingStatus));
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::removeChatPlayerEntry (int playerId)
{
	if (chatBox) chatBox->removePlayerEntry (playerId);
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::addChatEntry (const std::string& playerName, const std::string& message)
{
	if (!chatBox) return;
	chatBox->addChatEntry (std::make_unique<cLobbyChatBoxListViewItem> (playerName, message));
	cSoundDevice::getInstance().playSoundEffect (SoundData.SNDChat);
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::allowSelection()
{
	selectionAllowed = true;
	auto application = getActiveApplication();
	auto mouse = getActiveMouse();
	if (application && mouse)
	{
		handleMouseMoved (*application, *mouse, mouse->getPosition());
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::disallowSelection()
{
	selectionAllowed = false;
	auto application = getActiveApplication();
	auto mouse = getActiveMouse();
	if (application && mouse)
	{
		handleMouseMoved (*application, *mouse, mouse->getPosition());
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::lockBack()
{
	backButton->lock();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::unlockBack()
{
	backButton->unlock();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::handleActivated (cApplication& application, bool firstTime)
{
	if (firstTime)
	{
		cSoundDevice::getInstance().playVoice (getRandom (VoiceData.VOILanding));
		setInfoMessage (lngPack.i18n ("Comp~Landing_Select"));
	}
	application.addRunnable (animationTimer);
	cWindow::handleActivated (application, firstTime);
	if (firstTime) opened();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::handleDeactivated (cApplication& application, bool removed)
{
	application.removeRunnable (animationTimer);
	cWindow::handleDeactivated (application, removed);
	if (removed) closed();
}

//------------------------------------------------------------------------------
bool cWindowLandingPositionSelection::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (chatBox && !chatBox->isHidden() && chatBox->isAt (mouse.getPosition()))
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	}
	else if (!selectionAllowed)
	{
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::No));
	}
	else if (mapWidget->isAt (mouse.getPosition()))
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
AutoSurface cWindowLandingPositionSelection::createHudSurface()
{
	AutoSurface hudSurface (cHud::generateSurface());

	SDL_Rect top, bottom;
	top.x = 0;
	top.y = (Video.getResolutionY() / 2) - 479;

	bottom.x = 0;
	bottom.y = (Video.getResolutionY() / 2);

	SDL_BlitSurface (GraphicsData.gfx_panel_top.get(), nullptr, hudSurface.get(), &top);
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom.get(), nullptr, hudSurface.get(), &bottom);

	return hudSurface;
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::backClicked()
{
	canceled();
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::mapClicked (const cPosition& tilePosition)
{
	if (!selectionAllowed) return;

	cSoundDevice::getInstance().playSoundEffect (SoundData.SNDMenuButton);

	if (reselectionState == eLandingPositionState::Warning && (tilePosition - lastSelectedPosition).l2Norm() < cLandingPositionManager::tooCloseDistance)
	{
		selectedPosition (lastSelectedPosition);
	}
	else
	{
		startCircleAnimation (tilePosition);
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::startCircleAnimation (const cPosition& tilePosition)
{
	circleAnimationConnectionManager.disconnectAll();
	circleAnimationState = 0.;

	circleAnimationConnectionManager.connect (animationTimer->triggered10msCatchUp, [this, tilePosition]() { runCircleAnimation (tilePosition); });
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::runCircleAnimation (const cPosition& tilePosition)
{
	const float circleAnimationStep = 0.02f;

	circleAnimationState += circleAnimationStep;

	updateLandingPositionCircles (tilePosition, std::min (circleAnimationState, 1.0f));

	if (circleAnimationState >= 1.0)
	{
		circleAnimationConnectionManager.disconnectAll();

		selectedPosition (tilePosition);
	}
}

//------------------------------------------------------------------------------
void cWindowLandingPositionSelection::updateLandingPositionCircles (const cPosition& tilePosition, float radiusFactor)
{
	AutoSurface circleSurface (SDL_CreateRGBSurface (0, mapWidget->getSize().x(), mapWidget->getSize().y(), Video.getColDepth(), 0, 0, 0, 0));
	SDL_FillRect (circleSurface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (circleSurface.get(), SDL_TRUE, 0xFF00FF);

	const cPosition pixelPosition = tilePosition * mapWidget->getSize() / staticMap->getSize();

	// for non 4:3 screen resolutions, the size of the circles is
	// only correct in x dimension, because I don't draw an ellipse
	const int fullWarningRadius = static_cast<int> (cLandingPositionManager::warningDistance * mapWidget->getSize().x() / staticMap->getSize().x());
	const int fullTooCloseRadius = static_cast<int> (cLandingPositionManager::tooCloseDistance * mapWidget->getSize().x() / staticMap->getSize().x());
	const int warningRadius = (int) (fullWarningRadius * radiusFactor);
	const int tooCloseRadius = std::min (warningRadius - 1, fullTooCloseRadius);
	drawCircle (pixelPosition.x(), pixelPosition.y(), warningRadius, SCAN_COLOR, *circleSurface);
	drawCircle (pixelPosition.x(), pixelPosition.y(), tooCloseRadius, RANGE_GROUND_COLOR, *circleSurface);

	circlesImage->setImage (circleSurface.get());
}
