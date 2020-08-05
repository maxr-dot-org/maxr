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

#include "ui/graphical/menu/windows/windowplayerselection/windowplayerselection.h"

#include "ui/graphical/application.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/image.h"
#include "utility/language.h"
#include "utility/pcx.h"
#include "defines.h"
#include "video.h"

//------------------------------------------------------------------------------
cWindowPlayerSelection::cWindowPlayerSelection() :
	cWindow (LoadPCX (GFXOD_PLAYER_SELECT)) // 4 players
	// cWindow (LoadPCX (GFXOD_HOTSEAT)) // 8 players
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Text~Title~HotSeat"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (65, 35), getPosition() + cPosition (65 + 70, 35 + 10)), lngPack.i18n ("Text~Title~Team"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (165, 35), getPosition() + cPosition (165 + 70, 35 + 10)), lngPack.i18n ("Text~Title~Human"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (275, 35), getPosition() + cPosition (275 + 70, 35 + 10)), lngPack.i18n ("Text~Title~Computer"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (385, 35), getPosition() + cPosition (385 + 70, 35 + 10)), lngPack.i18n ("Text~Title~Nobody"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	//addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (500, 35), getPosition () + cPosition (500 + 70, 35 + 10)), lngPack.i18n ("Text~Title~Clan"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, [&]() { done(); });

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, [&]() { close(); });

	humanPlayerSurface = AutoSurface (LoadPCX (GFXOD_PLAYER_HUMAN));
	aiPlayerSurface = AutoSurface (LoadPCX (GFXOD_PLAYER_PC));
	noPlayerSurface = AutoSurface (LoadPCX (GFXOD_PLAYER_NONE));
	dummySurface = AutoSurface (SDL_CreateRGBSurface (0, 55, 71, Video.getColDepth(), 0, 0, 0, 0));
	SDL_FillRect (dummySurface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (dummySurface.get(), SDL_TRUE, 0xFF00FF);

	for (size_t i = 0; i < maxPlayers; ++i)
	{
		humanPlayerImages[i] = addChild (std::make_unique<cImage> (getPosition() + cPosition (175, 67 + 92 * i), nullptr, &SoundData.SNDHudButton));
		signalConnectionManager.connect (humanPlayerImages[i]->clicked, std::bind (&cWindowPlayerSelection::setPlayerType, this, i, ePlayerType::Human));
		aiPlayerImages[i] = addChild (std::make_unique<cImage> (getPosition() + cPosition (175 + 109, 67 + 92 * i), nullptr, &SoundData.SNDHudButton));
		//signalConnectionManager.connect (aiPlayerImages[i]->clicked, std::bind (&cWindowHotSeat::setPlayerType, this, i, ePlayerType::Ai));
		signalConnectionManager.connect (aiPlayerImages[i]->clicked, [this]()
		{
			const auto application = getActiveApplication();
			if (!application) return;

			application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Error_Messages~INFO_Not_Implemented")));
		});
		noPlayerImages[i] = addChild (std::make_unique<cImage> (getPosition() + cPosition (175 + 109 * 2, 67 + 92 * i), nullptr, &SoundData.SNDHudButton));
		signalConnectionManager.connect (noPlayerImages[i]->clicked, std::bind (&cWindowPlayerSelection::setPlayerType, this, i, ePlayerType::None));

		setPlayerType (i, i == 0 || i == 1 ? ePlayerType::Human : ePlayerType::None);
	}
}

//------------------------------------------------------------------------------
const std::array<ePlayerType, cWindowPlayerSelection::maxPlayers>& cWindowPlayerSelection::getPlayerTypes() const
{
	return playerTypes;
}

//------------------------------------------------------------------------------
void cWindowPlayerSelection::setPlayerType (size_t playerIndex, ePlayerType playerType)
{
	playerTypes[playerIndex] = playerType;

	humanPlayerImages[playerIndex]->setImage (playerType == ePlayerType::Human ? humanPlayerSurface.get() : dummySurface.get());
	aiPlayerImages[playerIndex]->setImage (playerType == ePlayerType::Ai ? aiPlayerSurface.get() : dummySurface.get());
	noPlayerImages[playerIndex]->setImage (playerType == ePlayerType::None ? noPlayerSurface.get() : dummySurface.get());

	okButton->lock();
	for (size_t i = 0; i < maxPlayers; ++i)
	{
		if (playerTypes[i] == ePlayerType::Human)
		{
			okButton->unlock();
			break;
		}
	}
}
