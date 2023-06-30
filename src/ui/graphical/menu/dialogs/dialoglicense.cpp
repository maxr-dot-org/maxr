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

#include "dialoglicense.h"

#include "resources/pcx.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/uidefines.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

#include <fstream>

namespace
{
	//--------------------------------------------------------------------------
	std::string getCompileTimeYear()
	{
		// __DATE__ format is Mmm dd yyyy
		return __DATE__ + 7;
	}
} // namespace

//------------------------------------------------------------------------------
cDialogLicense::cDialogLicense() :
	cWindow (LoadPCX (GFXOD_DIALOG4), eWindowBackgrounds::Alpha),
	currentPage (0),
	maxPage (3)
{
	auto* font = cUnicodeFont::font.get();

	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 30), getPosition() + cPosition (35 + 232, 30 + font->getFontHeight())), "\"M.A.X.R.\"", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	headerLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 30 + font->getFontHeight()), getPosition() + cPosition (35 + 232, 30 + font->getFontHeight() * 2)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	textLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 35 + font->getFontHeight() * 3), getPosition() + cPosition (35 + 232, 30 + font->getFontHeight() * 3 + 142)), "", eUnicodeFontType::LatinNormal, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
	textLabel->setWordWrap (true);

	auto okButton = emplaceChild<cPushButton> (getPosition() + cPosition (111, 185), ePushButtonType::Angular, lngPack.i18n ("Others~OK"), eUnicodeFontType::LatinNormal);
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
	signalConnectionManager.connect (okButton->clicked, [this]() { close(); });

	upButton = emplaceChild<cPushButton> (getPosition() + cPosition (241, 187), ePushButtonType::ArrowUpSmall);
	upButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_PAGEUP)));
	upButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_UP)));
	signalConnectionManager.connect (upButton->clicked, [this]() { pageUp(); });

	downButton = emplaceChild<cPushButton> (getPosition() + cPosition (261, 187), ePushButtonType::ArrowDownSmall);
	downButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_PAGEDOWN)));
	downButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_DOWN)));
	signalConnectionManager.connect (downButton->clicked, [this]() { pageDown(); });

	readAuthors();
	updatePageButtons();
	resetTexts();
}

//------------------------------------------------------------------------------
cDialogLicense::~cDialogLicense()
{}

//------------------------------------------------------------------------------
void cDialogLicense::pageDown()
{
	currentPage = std::min (currentPage + 1, maxPage);
	updatePageButtons();
	resetTexts();
}

//------------------------------------------------------------------------------
void cDialogLicense::pageUp()
{
	currentPage = std::max (currentPage - 1, 0);
	updatePageButtons();
	resetTexts();
}

//------------------------------------------------------------------------------
void cDialogLicense::updatePageButtons()
{
	if (currentPage <= 0)
		upButton->lock();
	else
		upButton->unlock();

	if (currentPage >= maxPage)
		downButton->lock();
	else
		downButton->unlock();
}

//------------------------------------------------------------------------------
void cDialogLicense::readAuthors()
{
	const std::filesystem::path fileName = cSettings::getInstance().getDataDir() / "AUTHORS";
	std::ifstream authorsFile (fileName);

	if (authorsFile.is_open())
	{
		std::string line;
		while (std::getline (authorsFile, line))
		{
			authors += line + "\n";
		}
	}
	else
		authors = "Couldn't read AUTHORS"; //missing file - naughty
}

//------------------------------------------------------------------------------
void cDialogLicense::resetTexts()
{
	static const char* page0Text =
		"  This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.";

	static const char* page1Text =
		"  This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.";

	static const char* page2Text =
		"  You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";

	static const std::string generalHeader = "(C) 2007-" + getCompileTimeYear() + " by its authors";
	static const char* authorsHeader = "AUTHORS:";

	switch (currentPage)
	{
		default:
		case 0:
			textLabel->setText (page0Text);
			textLabel->setFont (eUnicodeFontType::LatinNormal);
			headerLabel->setText (generalHeader);
			break;
		case 1:
			textLabel->setText (page1Text);
			textLabel->setFont (eUnicodeFontType::LatinNormal);
			headerLabel->setText (generalHeader);
			break;
		case 2:
			textLabel->setText (page2Text);
			textLabel->setFont (eUnicodeFontType::LatinNormal);
			headerLabel->setText (generalHeader);
			break;
		case 3:
			textLabel->setText (authors);
			textLabel->setFont (eUnicodeFontType::LatinSmallWhite);
			headerLabel->setText (authorsHeader);
			break;
	}
}
