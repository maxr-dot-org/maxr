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

#include "game/data/player/clans.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/translations.h"
#include "ui/uidefines.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowClanSelection::cWindowClanSelection (std::shared_ptr<const cUnitsData> unitsData, std::shared_ptr<const cClanData> clanData) :
	cWindow (LoadPCX (GFXOD_CLAN_SELECT)),
	unitsData (unitsData),
	clanData (clanData)
{
	assert (unitsData);
	assert (clanData);
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Title~Choose_Clan"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	//
	// Clan Images
	//
	const auto gfxPath = cSettings::getInstance().getGfxPath();
	std::array<std::filesystem::path, clanCount> clanLogoPaths{
		gfxPath / "clanlogo1.pcx",
		gfxPath / "clanlogo2.pcx",
		gfxPath / "clanlogo3.pcx",
		gfxPath / "clanlogo4.pcx",
		gfxPath / "clanlogo5.pcx",
		gfxPath / "clanlogo6.pcx",
		gfxPath / "clanlogo7.pcx",
		gfxPath / "clanlogo8.pcx"};
	for (size_t row = 0; row < clanRows; ++row)
	{
		for (size_t column = 0; column < clanColumns; ++column)
		{
			const auto index = row * clanColumns + column;

			auto image = LoadPCX (clanLogoPaths[index]);
			SDL_SetColorKey (image.get(), SDL_TRUE, 0xFF00FF);
			clanImages[index] = emplaceChild<cImage> (getPosition() + cPosition (88 + 154 * column - (image ? (image->w / 2) : 0), 48 + 150 * row), image.get(), &SoundData.SNDHudButton);
			signalConnectionManager.connect (clanImages[index]->clicked, [this, index]() { clanClicked (clanImages[index]); });

			clanTitles[index] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (37 + 155 * column, 144 + 150 * row), getPosition() + cPosition (135 + 155 * column, 144 + 10 + 150 * row)), getClanName (clanData->getClans()[index]), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
		}
	}
	clanTitles[selectedClan]->setText (">" + getClanName (clanData->getClans()[selectedClan]) + "<");

	//
	// Clan Description
	//
	clanDescription1 = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (47, 362), getPosition() + cPosition (47 + 550, 362 + 50)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	clanDescription2 = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (380, 362), getPosition() + cPosition (380 + 217, 362 + 50)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	clanShortDescription = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (47, 349), getPosition() + cPosition (47 + 550, 349 + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Left);

	//
	// Buttons
	//
	okButton = emplaceChild<cPushButton> (getPosition() + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Others~OK"));
	signalConnectionManager.connect (okButton->clicked, [this]() { okClicked(); });

	backButton = emplaceChild<cPushButton> (getPosition() + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Others~Back"));
	signalConnectionManager.connect (backButton->clicked, [this]() { backClicked(); });

	updateClanDescription();
}

//------------------------------------------------------------------------------
cWindowClanSelection::~cWindowClanSelection()
{}

//------------------------------------------------------------------------------
void cWindowClanSelection::retranslate()
{
	cWindow::retranslate();

	titleLabel->setText (lngPack.i18n ("Title~Choose_Clan"));
	okButton->setText (lngPack.i18n ("Others~OK"));
	backButton->setText (lngPack.i18n ("Others~Back"));
}

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
				clanTitles[selectedClan]->setText (getClanName (clanData->getClans()[selectedClan]));
				clanTitles[i]->setText (">" + getClanName (clanData->getClans()[i]) + "<");
				selectedClan = i;
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
	const auto& clanInfo = clanData->getClans()[selectedClan];

	auto strings = getClanStatsDescription (clanInfo, *unitsData);

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

	clanShortDescription->setText (getClanDescription (clanInfo));
}
