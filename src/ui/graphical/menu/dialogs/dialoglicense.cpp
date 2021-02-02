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

#include <fstream>

#include "ui/graphical/menu/dialogs/dialoglicense.h"

#include "resources/pcx.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "utility/language.h"

namespace
{
	//--------------------------------------------------------------------------
	std::string getCompileTimeYear()
	{
		// __DATE__ format is Mmm dd yyyy
		return __DATE__ + 7;
	}
}

//------------------------------------------------------------------------------
cDialogLicense::cDialogLicense() :
	cWindow (LoadPCX (GFXOD_DIALOG4), eWindowBackgrounds::Alpha),
	currentPage (0),
	maxPage (3)
{
	auto* font = cUnicodeFont::font.get();

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 30), getPosition() + cPosition (35 + 232, 30 + font->getFontHeight())), "\"M.A.X.R.\"", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	headerLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 30 + font->getFontHeight()), getPosition() + cPosition (35 + 232, 30 + font->getFontHeight() * 2)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 35 + font->getFontHeight() * 3), getPosition() + cPosition (35 + 232, 30 + font->getFontHeight() * 3 + 142)), "", FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top));
	textLabel->setWordWrap (true);

	auto okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (111, 185), ePushButtonType::Angular, lngPack.i18n ("Text~Others~OK"), FONT_LATIN_NORMAL));
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cDialogLicense::close, this));

	upButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (241, 187), ePushButtonType::ArrowUpSmall));
	signalConnectionManager.connect (upButton->clicked, std::bind (&cDialogLicense::pageUp, this));

	downButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (261, 187), ePushButtonType::ArrowDownSmall));
	signalConnectionManager.connect (downButton->clicked, std::bind (&cDialogLicense::pageDown, this));

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
	if (currentPage <= 0) upButton->lock();
	else upButton->unlock();

	if (currentPage >= maxPage) downButton->lock();
	else downButton->unlock();
}

//------------------------------------------------------------------------------
void cDialogLicense::readAuthors()
{
	std::string fileName;
#ifdef WIN32
	fileName = "AUTHORS";
#elif __amigaos4
	fileName = cSettings::getInstance().getDataDir() + PATH_DELIMITER + "AUTHORS";
#elif MAC
	fileName = "AUTHORS";
#else
	fileName = cSettings::getInstance().getDataDir() + PATH_DELIMITER + "AUTHORS";
#endif

	std::ifstream authorsFile (fileName);

	if (authorsFile.is_open())
	{
		std::string line;
		while (std::getline (authorsFile, line))
		{
			if (!authors.empty()) authors += "\n";
			authors += line;
		}
	}
	else authors = "Couldn't read AUTHORS"; //missing file - naughty
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
			textLabel->setFont (FONT_LATIN_NORMAL);
			headerLabel->setText (generalHeader);
			break;
		case 1:
			textLabel->setText (page1Text);
			textLabel->setFont (FONT_LATIN_NORMAL);
			headerLabel->setText (generalHeader);
			break;
		case 2:
			textLabel->setText (page2Text);
			textLabel->setFont (FONT_LATIN_NORMAL);
			headerLabel->setText (generalHeader);
			break;
		case 3:
			textLabel->setText (authors);
			textLabel->setFont (FONT_LATIN_SMALL_WHITE);
			headerLabel->setText (authorsHeader);
			break;
	}
}
