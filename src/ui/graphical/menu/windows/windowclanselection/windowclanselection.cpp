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

#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "main.h"
#include "pcx.h"
#include "game/data/player/clans.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/image.h"

//------------------------------------------------------------------------------
cWindowClanSelection::cWindowClanSelection() :
	cWindow (LoadPCX (GFXOD_CLAN_SELECT)),
	selectedClan (0)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Text~Title~Choose_Clan"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	//
	// Clan Images
	//
	std::array<std::string, clanCount> clanLogoPaths;
	const auto gfxPath = cSettings::getInstance().getGfxPath() + PATH_DELIMITER;
	clanLogoPaths[0] = gfxPath + "clanlogo1.pcx";
	clanLogoPaths[1] = gfxPath + "clanlogo2.pcx";
	clanLogoPaths[2] = gfxPath + "clanlogo3.pcx";
	clanLogoPaths[3] = gfxPath + "clanlogo4.pcx";
	clanLogoPaths[4] = gfxPath + "clanlogo5.pcx";
	clanLogoPaths[5] = gfxPath + "clanlogo6.pcx";
	clanLogoPaths[6] = gfxPath + "clanlogo7.pcx";
	clanLogoPaths[7] = gfxPath + "clanlogo8.pcx";

	for (size_t row = 0; row < clanRows; ++row)
	{
		for (size_t column = 0; column < clanColumns; ++column)
		{
			const auto index = row * clanColumns + column;

			auto image = LoadPCX (clanLogoPaths[index].c_str());
			SDL_SetColorKey (image.get(), SDL_TRUE, 0xFF00FF);
			clanImages[index] = addChild (std::make_unique<cImage> (getPosition() + cPosition (88 + 154 * column - (image ? (image->w / 2) : 0), 48 + 150 * row), image.get(), &SoundData.SNDHudButton));
			signalConnectionManager.connect (clanImages[index]->clicked, std::bind (&cWindowClanSelection::clanClicked, this, clanImages[index]));

			clanTitles[index] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (37 + 155 * column, 144 + 150 * row), getPosition() + cPosition (135 + 155 * column, 144 + 10 + 150 * row)), cClanData::instance().getClan (index)->getName(), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
		}
	}
	clanTitles[selectedClan]->setText (">" + cClanData::instance().getClan (selectedClan)->getName() + "<");

	//
	// Clan Description
	//
	clanDescription1 = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (47, 362), getPosition() + cPosition (47 + 550, 362 + 50)), "", FONT_LATIN_NORMAL, eAlignmentType::Left));
	clanDescription2 = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (380, 362), getPosition() + cPosition (380 + 217, 362 + 50)), "", FONT_LATIN_NORMAL, eAlignmentType::Left));
	clanShortDescription = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (47, 349), getPosition() + cPosition (47 + 550, 349 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::Left));

	//
	// Buttons
	//
	auto okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowClanSelection::okClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowClanSelection::backClicked, this));

	updateClanDescription();
}

//------------------------------------------------------------------------------
cWindowClanSelection::~cWindowClanSelection()
{}

//------------------------------------------------------------------------------
unsigned int cWindowClanSelection::getSelectedClan() const
{
	return selectedClan;
}

//------------------------------------------------------------------------------
void cWindowClanSelection::clanClicked (const cImage* clanImage)
{
	for (size_t i = 0; i < clanImages.size(); ++i)
	{
		if (clanImage == clanImages[i])
		{
			if (i != selectedClan)
			{
				clanTitles[selectedClan]->setText (cClanData::instance().getClan (selectedClan)->getName());
				selectedClan = i;
				clanTitles[selectedClan]->setText (">" + cClanData::instance().getClan (selectedClan)->getName() + "<");
				updateClanDescription();
			}
			break;
		}
	}
}

//------------------------------------------------------------------------------
void cWindowClanSelection::okClicked()
{
	done();
}

//------------------------------------------------------------------------------
void cWindowClanSelection::backClicked()
{
	canceled();
}

//------------------------------------------------------------------------------
void cWindowClanSelection::updateClanDescription()
{
	auto clanInfo = cClanData::instance().getClan (selectedClan);
	if (clanInfo)
	{
		auto strings = clanInfo->getClanStatsDescription();

		std::string desc1;
		for (size_t i = 0; i < 4 && i < strings.size(); ++i)
		{
			desc1.append (strings[i]);
			desc1.append ("\n");
		}
		clanDescription1->setText (desc1);

		std::string desc2;
		for (size_t i = 4; i < strings.size(); ++i)
		{
			desc2.append (strings[i]);
			desc2.append ("\n");
		}
		clanDescription2->setText (desc2);

		clanShortDescription->setText (clanInfo->getDescription());
	}
	else
	{
		clanDescription1->setText ("Unknown");
		clanDescription1->setText ("");
	}
}
