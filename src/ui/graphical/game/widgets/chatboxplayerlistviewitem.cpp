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

#include "ui/graphical/game/widgets/chatboxplayerlistviewitem.h"

#include "SDLutility/tosdl.h"
#include "game/data/player/player.h"
#include "resources/playercolor.h"
#include "resources/uidata.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"

//------------------------------------------------------------------------------
cChatBoxPlayerListViewItem::cChatBoxPlayerListViewItem (const cPlayer& player_) :
	cAbstractListViewItem (cPosition (138, 0)),
	player (&player_)
{
	readyImage = emplaceChild<cImage> (getPosition() + cPosition (getSize().x() - 10, 0));

	colorImage = emplaceChild<cImage> (getPosition());

	updatePlayerColor();
	updatePlayerFinishedTurn();

	nameLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (colorImage->getEndPosition().x() + 4, 0), getPosition() + cPosition (getSize().x() - readyImage->getSize().x(), readyImage->getSize().y())), player->getName());
	nameLabel->setConsumeClick (false);

	fitToChildren();

	signalConnectionManager.connect (player->hasFinishedTurnChanged, [this]() { updatePlayerFinishedTurn(); });
}

//------------------------------------------------------------------------------
const cPlayer& cChatBoxPlayerListViewItem::getPlayer() const
{
	return *player;
}

//------------------------------------------------------------------------------
int cChatBoxPlayerListViewItem::getPlayerNumber() const
{
	return player->getId();
}

//------------------------------------------------------------------------------
void cChatBoxPlayerListViewItem::updatePlayerName()
{
	// TODO: show player status (removed/lost/may be waiting/...)?
	nameLabel->setText (player->getName());
}

//------------------------------------------------------------------------------
void cChatBoxPlayerListViewItem::updatePlayerColor()
{
	SDL_Rect src = {0, 0, 10, 10};

	AutoSurface colorSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_BlitSurface (cPlayerColor::getTexture (player->getColor()), &src, colorSurface.get(), nullptr);

	colorImage->setImage (colorSurface.get());
}

//------------------------------------------------------------------------------
void cChatBoxPlayerListViewItem::updatePlayerFinishedTurn()
{
	SDL_Rect src = {player->getHasFinishedTurn() ? 10 : 0, 0, 10, 10};

	AutoSurface readySurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (readySurface.get(), SDL_TRUE, toSdlAlphaColor (cRgbColor (0, 1, 0), *readySurface));
	SDL_BlitSurface (GraphicsData.gfx_player_ready.get(), &src, readySurface.get(), nullptr);

	readyImage->setImage (readySurface.get());
}

//------------------------------------------------------------------------------
void cChatBoxPlayerListViewItem::handleResized (const cPosition& oldSize)
{
	cAbstractListViewItem::handleResized (oldSize);

	if (oldSize.x() == getSize().x()) return;

	readyImage->moveTo (getPosition() + cPosition (getSize().x() - 10, 0));

	nameLabel->resize (cPosition (getSize().x() - readyImage->getSize().x() - colorImage->getSize().x() - 4, readyImage->getSize().y()));

	fitToChildren();
}
