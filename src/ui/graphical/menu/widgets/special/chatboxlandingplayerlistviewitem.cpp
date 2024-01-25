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

#include "chatboxlandingplayerlistviewitem.h"

#include "SDLutility/tosdl.h"
#include "game/data/player/player.h"
#include "game/logic/landingpositionmanager.h"
#include "resources/playercolor.h"
#include "resources/uidata.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"

//------------------------------------------------------------------------------
cPlayerLandingStatus::cPlayerLandingStatus (const cPlayerBasicData& player) :
	player (player)
{}

//------------------------------------------------------------------------------
const cPlayerBasicData& cPlayerLandingStatus::getPlayer() const
{
	return player;
}

//------------------------------------------------------------------------------
bool cPlayerLandingStatus::hasSelectedPosition() const
{
	return selectedPosition;
}

//------------------------------------------------------------------------------
void cPlayerLandingStatus::setHasSelectedPosition (bool value)
{
	std::swap (selectedPosition, value);
	if (value != selectedPosition) hasSelectedPositionChanged();
}

//------------------------------------------------------------------------------
cChatBoxLandingPlayerListViewItem::cChatBoxLandingPlayerListViewItem (const cPlayerLandingStatus& playerLandingStatus_) :
	cAbstractListViewItem (cPosition (50, 0)),
	playerLandingStatus (playerLandingStatus_)
{
	const auto& player = playerLandingStatus.getPlayer();

	readyImage = emplaceChild<cImage> (getPosition() + cPosition (getSize().x() - 10, 0));

	colorImage = emplaceChild<cImage> (getPosition());

	updatePlayerColor();
	updatePlayerHasSelectedPosition();

	nameLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (colorImage->getEndPosition().x() + 4, 0), getPosition() + cPosition (getSize().x() - readyImage->getSize().x(), readyImage->getSize().y())), player.getName());

	fitToChildren();

	signalConnectionManager.connect (player.nameChanged, [this]() { updatePlayerName(); });
	signalConnectionManager.connect (player.colorChanged, [this]() { updatePlayerColor(); });
	signalConnectionManager.connect (playerLandingStatus.hasSelectedPositionChanged, [this]() { updatePlayerHasSelectedPosition(); });
}

//------------------------------------------------------------------------------
int cChatBoxLandingPlayerListViewItem::getPlayerNumber() const
{
	return playerLandingStatus.getPlayer().getNr();
}

//------------------------------------------------------------------------------
void cChatBoxLandingPlayerListViewItem::setLandingPositionManager (const cLandingPositionManager* landingPositionManager_)
{
	landingPositionManager = landingPositionManager_;

	managerSignalConnectionManager.disconnectAll();

	updatePlayerName();

	managerSignalConnectionManager.connect (landingPositionManager->landingPositionStateChanged, [this] (const cPlayerBasicData& player, eLandingPositionState state) {
		if (player.getNr() == playerLandingStatus.getPlayer().getNr())
		{
			updatePlayerName();
		}
	});
}

//------------------------------------------------------------------------------
void cChatBoxLandingPlayerListViewItem::updatePlayerName()
{
	if (landingPositionManager == nullptr)
	{
		nameLabel->setText (playerLandingStatus.getPlayer().getName());
	}
	else
	{
		const auto state = landingPositionManager->getPlayerState (playerLandingStatus.getPlayer());
		std::string stateName;
		switch (state)
		{
			case eLandingPositionState::Unknown:
				stateName = "unknown";
				break;
			case eLandingPositionState::Clear:
				stateName = "clear";
				break;
			case eLandingPositionState::Warning:
				stateName = "warning";
				break;
			case eLandingPositionState::TooClose:
				stateName = "too close";
				break;
			case eLandingPositionState::Confirmed:
				stateName = "confirmed";
				break;
		}
		nameLabel->setText (playerLandingStatus.getPlayer().getName() + " (" + stateName + ")");
	}
}

//------------------------------------------------------------------------------
void cChatBoxLandingPlayerListViewItem::updatePlayerColor()
{
	SDL_Rect src = {0, 0, 10, 10};

	UniqueSurface colorSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_BlitSurface (cPlayerColor::getTexture (playerLandingStatus.getPlayer().getColor()), &src, colorSurface.get(), nullptr);

	colorImage->setImage (colorSurface.get());
}

//------------------------------------------------------------------------------
void cChatBoxLandingPlayerListViewItem::updatePlayerHasSelectedPosition()
{
	SDL_Rect src = {playerLandingStatus.hasSelectedPosition() ? 10 : 0, 0, 10, 10};

	UniqueSurface readySurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (readySurface.get(), SDL_TRUE, toSdlAlphaColor (cRgbColor (0, 1, 0), *readySurface));
	SDL_BlitSurface (GraphicsData.gfx_player_ready.get(), &src, readySurface.get(), nullptr);

	readyImage->setImage (readySurface.get());
}

//------------------------------------------------------------------------------
void cChatBoxLandingPlayerListViewItem::handleResized (const cPosition& oldSize)
{
	cAbstractListViewItem::handleResized (oldSize);

	if (oldSize.x() == getSize().x()) return;

	readyImage->moveTo (getPosition() + cPosition (getSize().x() - 10, 0));

	nameLabel->resize (cPosition (getSize().x() - readyImage->getSize().x() - colorImage->getSize().x() - 4, readyImage->getSize().y()));

	fitToChildren();
}
