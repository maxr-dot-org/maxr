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

#include "ui/graphical/menu/dialogs/dialogresearch.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/slider.h"
#include "ui/graphical/menu/widgets/image.h"
#include "pcx.h"
#include "main.h"
#include "game/data/player/player.h"
#include "video.h"

//------------------------------------------------------------------------------
cDialogResearch::cDialogResearch (const cPlayer& player_) :
	cWindow (LoadPCX (GFXOD_DIALOG_RESEARCH), eWindowBackgrounds::Alpha),
	player (player_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 19), getPosition () + cPosition (getArea ().getMaxCorner ().x (), 19 + 10)), lngPack.i18n ("Text~Title~Load"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (23, 52), getPosition () + cPosition (23 + 40, 52 + 10)), lngPack.i18n ("Text~Comp~Labs"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (160, 52), getPosition () + cPosition (160 + 75, 52 + 10)), lngPack.i18n ("Text~Comp~Themes"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (291, 52), getPosition () + cPosition (291 + 44, 52 + 10)), lngPack.i18n ("Text~Comp~Turns"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	auto doneButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (193, 294), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [&](){ done (); });

	auto cancelButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (91, 294), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Cancel"), FONT_LATIN_NORMAL));
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, [&](){ close (); });

	const std::string themeNames[rows] =
	{
		lngPack.i18n ("Text~Others~Attack"),
		lngPack.i18n ("Text~Others~Shots_7"),
		lngPack.i18n ("Text~Others~Range"),
		lngPack.i18n ("Text~Others~Armor_7"),
		lngPack.i18n ("Text~Others~Hitpoints_7"),
		lngPack.i18n ("Text~Others~Speed"),
		lngPack.i18n ("Text~Others~Scan"),
		lngPack.i18n ("Text~Others~Costs")
	};

	const SDL_Rect themeImageSrcs[rows] =
	{
		{27, 109, 10, 14},
		{37, 109, 15, 7},
		{52, 109, 13, 13},
		{65, 109, 11, 14},
		{11, 109, 7, 11},
		{0, 109, 11, 12},
		{76, 109, 13, 13},
		{112, 109, 13, 10}
	};

	for (size_t i = 0; i < rows; ++i)
	{
		addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (183, 72 + 28 * i), getPosition () + cPosition (183 + 50, 72 + 28 * i + 10)), themeNames[i], FONT_LATIN_NORMAL, eAlignmentType::Left));

		auto src = themeImageSrcs[i];

		AutoSurface image (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth (), 0, 0, 0, 0));
		SDL_FillRect (image.get (), NULL, 0x00FF00FF);
		SDL_SetColorKey (image.get (), SDL_TRUE, 0x00FF00FF);
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &src, image.get (), nullptr);

		addChild (std::make_unique<cImage> (getPosition () + cPosition (172 - src.w / 2, 78 - src.h / 2 + 28 * i), image.get()));

		researchCenterCountLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (24, 72 + 28 * i), getPosition () + cPosition (24 + 38, 72 + 28 * i + 10)), "0", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

		percentageLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (236, 72 + 28 * i), getPosition () + cPosition (236 + 44, 72 + 28 * i + 10)), "+" + iToStr (player.researchLevel.getCurResearchLevel (i)) + "%", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

		turnsLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (291, 72 + 28 * i), getPosition () + cPosition (291 + 44, 72 + 28 * i + 10)), "0", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

		sliders[i] = addChild (std::make_unique<cSlider> (cBox<cPosition> (cPosition (90, 70 + 28 * i), cPosition (90 + 51, 70 + 28 * i + 15)), 0, player.workingResearchCenterCount, eOrientationType::Horizontal, eSliderHandleType::Horizontal, eSliderType::Invisible));
		signalConnectionManager.connect (sliders[i]->valueChanged, std::bind (&cDialogResearch::handleSliderValueChanged, this, i));

		decreaseButtons[i] = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (71, 70 + 28 * i), ePushButtonType::ArrowLeftSmall));
		signalConnectionManager.connect (decreaseButtons[i]->clicked, [&, i](){ sliders[i]->decrease (1);  });
		
		increaseButtons[i] = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (143, 70 + 28 * i), ePushButtonType::ArrowRightSmall));
		signalConnectionManager.connect (increaseButtons[i]->clicked, [&, i](){ sliders[i]->increase (1);  });
	}

	unusedResearchCenters = player.workingResearchCenterCount;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		researchSettings[i] = player.researchCentersWorkingOnArea[i];
		unusedResearchCenters -= researchSettings[i];
	}

	updateWidgets ();
}


//------------------------------------------------------------------------------
const std::array<int, cResearch::kNrResearchAreas>& cDialogResearch::getResearchSettings () const
{
	return researchSettings;
}

//------------------------------------------------------------------------------
void cDialogResearch::updateWidgets ()
{
	for (size_t i = 0; i < rows; ++i)
	{
		researchCenterCountLabels[i]->setText (iToStr (researchSettings[i]));
		sliders[i]->setValue (researchSettings[i]);

		turnsLabels[i]->setText (iToStr (player.researchLevel.getRemainingTurns (i, researchSettings[i])));

		if (unusedResearchCenters <= 0) increaseButtons[i]->lock ();
		else increaseButtons[i]->unlock ();

		if (researchSettings[i] <= 0) decreaseButtons[i]->lock ();
		else decreaseButtons[i]->unlock ();
	}
}

//------------------------------------------------------------------------------
void cDialogResearch::handleSliderValueChanged (size_t index)
{
	const auto wantResearchCenters = sliders[index]->getValue ();

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
	updateWidgets ();
}
