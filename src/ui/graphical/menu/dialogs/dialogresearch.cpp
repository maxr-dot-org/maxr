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

#include "dialogresearch.h"

#include "game/data/player/player.h"
#include "output/video/video.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/slider.h"
#include "ui/uidefines.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cDialogResearch::cDialogResearch (const cPlayer& player_) :
	cWindow (LoadPCX (GFXOD_DIALOG_RESEARCH), eWindowBackgrounds::Alpha),
	player (player_)
{
	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 19), getPosition() + cPosition (getArea().getMaxCorner().x(), 19 + 10)), lngPack.i18n ("Title~Labs"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (23, 52), getPosition() + cPosition (23 + 40, 52 + 10)), lngPack.i18n ("Comp~Labs"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (160, 52), getPosition() + cPosition (160 + 75, 52 + 10)), lngPack.i18n ("Comp~Themes"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (291, 52), getPosition() + cPosition (291 + 44, 52 + 10)), lngPack.i18n ("Comp~Turns"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	auto doneButton = emplaceChild<cPushButton> (getPosition() + cPosition (193, 294), ePushButtonType::Angular, lngPack.i18n ("Others~Done"), eUnicodeFontType::LatinNormal);
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [this]() { done(); });

	auto cancelButton = emplaceChild<cPushButton> (getPosition() + cPosition (91, 294), ePushButtonType::Angular, lngPack.i18n ("Others~Cancel"), eUnicodeFontType::LatinNormal);
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, [this]() { close(); });

	const std::string themeNames[rows] = {
		lngPack.i18n ("Others~Attack"),
		lngPack.i18n ("Others~Shots_7"),
		lngPack.i18n ("Others~Range"),
		lngPack.i18n ("Others~Armor_7"),
		lngPack.i18n ("Others~Hitpoints_7"),
		lngPack.i18n ("Others~Speed"),
		lngPack.i18n ("Others~Scan"),
		lngPack.i18n ("Others~Costs")};

	const SDL_Rect themeImageSrcs[rows] = {
		cGraphicsData::getRect_BigSymbol_Attack(),
		cGraphicsData::getRect_BigSymbol_Shots(),
		cGraphicsData::getRect_BigSymbol_Range(),
		cGraphicsData::getRect_BigSymbol_Armor(),
		cGraphicsData::getRect_BigSymbol_Hitpoints(),
		cGraphicsData::getRect_BigSymbol_Speed(),
		cGraphicsData::getRect_BigSymbol_Scan(),
		cGraphicsData::getRect_BigSymbol_Costs()};

	for (size_t i = 0; i < rows; ++i)
	{
		emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (183, 72 + 28 * i), getPosition() + cPosition (183 + 50, 72 + 28 * i + 10)), themeNames[i], eUnicodeFontType::LatinNormal, eAlignmentType::Left);

		auto src = themeImageSrcs[i];

		UniqueSurface image (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
		SDL_FillRect (image.get(), nullptr, 0x00FF00FF);
		SDL_SetColorKey (image.get(), SDL_TRUE, 0x00FF00FF);
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &src, image.get(), nullptr);

		emplaceChild<cImage> (getPosition() + cPosition (172 - src.w / 2, 78 - src.h / 2 + 28 * i), image.get());

		researchCenterCountLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (24, 72 + 28 * i), getPosition() + cPosition (24 + 38, 72 + 28 * i + 10)), "0", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

		percentageLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (236, 72 + 28 * i), getPosition() + cPosition (236 + 44, 72 + 28 * i + 10)), "+" + std::to_string (player.getResearchState().getCurResearchLevel (static_cast<cResearch::eResearchArea> (i))) + "%", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

		turnsLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (291, 72 + 28 * i), getPosition() + cPosition (291 + 44, 72 + 28 * i + 10)), "0", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

		sliders[i] = emplaceChild<cSlider> (cBox<cPosition> (cPosition (90, 70 + 28 * i), cPosition (90 + 51, 70 + 28 * i + 15)), 0, player.getResearchCentersWorkingTotal(), eOrientationType::Horizontal, eSliderHandleType::Horizontal, eSliderType::Invisible);
		signalConnectionManager.connect (sliders[i]->valueChanged, [this, i]() { handleSliderValueChanged (i); });

		decreaseButtons[i] = emplaceChild<cPushButton> (getPosition() + cPosition (71, 70 + 28 * i), ePushButtonType::ArrowLeftSmall);
		signalConnectionManager.connect (decreaseButtons[i]->clicked, [this, i]() { sliders[i]->decrease (1); });

		increaseButtons[i] = emplaceChild<cPushButton> (getPosition() + cPosition (143, 70 + 28 * i), ePushButtonType::ArrowRightSmall);
		signalConnectionManager.connect (increaseButtons[i]->clicked, [this, i]() { sliders[i]->increase (1); });
	}

	unusedResearchCenters = player.getResearchCentersWorkingTotal();
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		researchSettings[i] = player.getResearchCentersWorkingOnArea ((cResearch::eResearchArea) i);
		unusedResearchCenters -= researchSettings[i];
	}

	updateWidgets();
}

//------------------------------------------------------------------------------
const std::array<int, cResearch::kNrResearchAreas>& cDialogResearch::getResearchSettings() const
{
	return researchSettings;
}

//------------------------------------------------------------------------------
void cDialogResearch::updateWidgets()
{
	for (size_t i = 0; i < rows; ++i)
	{
		researchCenterCountLabels[i]->setText (std::to_string (researchSettings[i]));
		sliders[i]->setValue (researchSettings[i]);

		turnsLabels[i]->setText (std::to_string (player.getResearchState().getRemainingTurns (static_cast<cResearch::eResearchArea> (i), researchSettings[i])));

		if (unusedResearchCenters <= 0)
			increaseButtons[i]->lock();
		else
			increaseButtons[i]->unlock();

		if (researchSettings[i] <= 0)
			decreaseButtons[i]->lock();
		else
			decreaseButtons[i]->unlock();
	}
}

//------------------------------------------------------------------------------
void cDialogResearch::handleSliderValueChanged (size_t index)
{
	const auto wantResearchCenters = sliders[index]->getValue();

	if (wantResearchCenters <= researchSettings[index])
	{
		unusedResearchCenters += researchSettings[index] - wantResearchCenters;
		researchSettings[index] = wantResearchCenters;
	}
	else
	{
		const auto wantIncrement = wantResearchCenters - researchSettings[index];
		const auto possibleIncrement = (wantIncrement >= unusedResearchCenters) ? unusedResearchCenters : wantIncrement;
		researchSettings[index] += possibleIncrement;
		unusedResearchCenters -= possibleIncrement;
	}
	updateWidgets();
}
