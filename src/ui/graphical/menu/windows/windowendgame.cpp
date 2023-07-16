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

#include "windowendgame.h"

#include "game/data/player/player.h"
#include "resources/pcx.h"
#include "settings.h"
#include "ui/graphical/game/gamegui.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/widgets/application.h"
#include "ui/widgets/label.h"
#include "utility/language.h"
#include "utility/os.h"
#include "utility/random.h"

#include <functional>

namespace
{
	//--------------------------------------------------------------------------
	std::filesystem::path getRandomEndGamePath()
	{
		const auto directory = cSettings::getInstance().getGfxPath() / "endgame";
		const auto filename = getRandom (os::getFilesOfDirectory (directory));
		return directory / filename;
	}
} // namespace

//------------------------------------------------------------------------------
cWindowEndGame::cWindowEndGame (const std::vector<std::shared_ptr<const cPlayer>>& players) :
	cWindow (LoadPCX (getRandomEndGamePath()))
{
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 10), cPosition (640, 25)), lngPack.i18n ("Title~GameOver"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);

	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 130), cPosition (160, 145)), lngPack.i18n ("Comp~Points"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 180), cPosition (160, 195)), lngPack.i18n ("GameOver~Builtfactories"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 210), cPosition (160, 225)), lngPack.i18n ("GameOver~BuiltMineStations"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 240), cPosition (160, 255)), lngPack.i18n ("GameOver~BuiltBuildings"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 260), cPosition (160, 275)), lngPack.i18n ("GameOver~LostBuildings"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 290), cPosition (160, 305)), lngPack.i18n ("GameOver~BuiltUnits"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 310), cPosition (160, 325)), lngPack.i18n ("GameOver~LostUnits"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 340), cPosition (160, 355)), lngPack.i18n ("Others~Upgrade"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);
	emplaceChild<cLabel> (cBox<cPosition> (cPosition (0, 360), cPosition (160, 375)), lngPack.i18n ("Title~Credits"), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::Right) | eAlignmentType::Top);

	int playerIndex = 0;
	for (const auto& player : players)
	{
		const auto posx = playerIndex < 4 ? 130 : 190;
		const auto i = playerIndex % 4;

		const auto& stat = player->getGameOverStat();
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, playerIndex < 4 ? 85 : 60), cPosition (posx + 120 + 120 * i, playerIndex < 4 ? 100 : 75)), player->getName(), eUnicodeFontType::LatinBigGold, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);

		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 130), cPosition (posx + 120 + 120 * i, 145)), std::to_string (player->getScore()), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);

#if 0 // TODO: final player position
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 150), cPosition (posx + 120  + 120 * i, 165)), "n th place", eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
#endif
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 180), cPosition (posx + 120 + 120 * i, 195)), std::to_string (stat.builtFactoriesCount), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 210), cPosition (posx + 120 + 120 * i, 225)), std::to_string (stat.builtMineStationCount), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 240), cPosition (posx + 120 + 120 * i, 255)), std::to_string (stat.builtBuildingsCount), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 260), cPosition (posx + 120 + 120 * i, 275)), std::to_string (stat.lostBuildingsCount), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 290), cPosition (posx + 120 + 120 * i, 305)), std::to_string (stat.builtVehiclesCount), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 310), cPosition (posx + 120 + 120 * i, 325)), std::to_string (stat.lostVehiclesCount), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 340), cPosition (posx + 120 + 120 * i, 355)), std::to_string (stat.totalUpgradeCost), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
		emplaceChild<cLabel> (cBox<cPosition> (cPosition (posx + 120 * i, 360), cPosition (posx + 120 + 120 * i, 375)), std::to_string (player->getCredits()), eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);

		++playerIndex;
	}
	exitButton = emplaceChild<cPushButton> (getPosition() + cPosition (250, 445), ePushButtonType::StandardSmall, &SoundData.SNDMenuButton, lngPack.i18n ("Others~Exit"));
	exitButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_RETURN)));
	exitButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_ESCAPE)));

	signalConnectionManager.connect (exitButton->clicked, [this]() { close(); closed(); });
}

//------------------------------------------------------------------------------
void cWindowEndGame::retranslate()
{
	cWindow::retranslate();

	exitButton->setText (lngPack.i18n ("Others~Exit"));
}
