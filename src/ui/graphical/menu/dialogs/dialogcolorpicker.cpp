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

#include "ui/graphical/menu/dialogs/dialogcolorpicker.h"

#include "SDLutility/tosdl.h"
#include "resources/pcx.h"
#include "ui/graphical/menu/widgets/colorpicker.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "ui/widgets/lineedit.h"
#include "ui/widgets/validators/validatorint.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cDialogColorPicker::cDialogColorPicker (const cRgbColor& color, eWindowBackgrounds backgroundType) :
	cWindow (LoadPCX (GFXOD_DIALOG2), backgroundType)
{
	auto* font = cUnicodeFont::font.get();

	colorPicker = emplaceChild<cRgbColorPicker> (cBox<cPosition> (getPosition() + cPosition (35, 35), getPosition() + cPosition (35 + 160, 35 + 135)), color);

	selectedColorImage = emplaceChild<cImage> (getPosition() + cPosition (210, 35));
	selectedColorImage->setImage (createSelectedColorSurface().get());

	signalConnectionManager.connect (colorPicker->selectedColorChanged, [this]() {
		selectedColorImage->setImage (createSelectedColorSurface().get());

		const auto color = colorPicker->getSelectedColor();
		redValueLineEdit->setText (std::to_string (color.r));
		greenValueLineEdit->setText (std::to_string (color.g));
		blueValueLineEdit->setText (std::to_string (color.b));
	});

	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (210, 100), getPosition() + cPosition (210 + 20, 100 + font->getFontHeight())), "R:");
	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (210, 120), getPosition() + cPosition (210 + 20, 120 + font->getFontHeight())), "G:");
	emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (210, 140), getPosition() + cPosition (210 + 20, 140 + font->getFontHeight())), "B:");

	redValueLineEdit = emplaceChild<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (230, 100), getPosition() + cPosition (230 + 25, 100 + font->getFontHeight())));
	redValueLineEdit->setValidator (std::make_unique<cValidatorInt> (0, 255));
	greenValueLineEdit = emplaceChild<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (230, 120), getPosition() + cPosition (230 + 25, 120 + font->getFontHeight())));
	greenValueLineEdit->setValidator (std::make_unique<cValidatorInt> (0, 255));
	blueValueLineEdit = emplaceChild<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (230, 140), getPosition() + cPosition (230 + 25, 140 + font->getFontHeight())));
	blueValueLineEdit->setValidator (std::make_unique<cValidatorInt> (0, 255));

	redValueLineEdit->setText (std::to_string (color.r));
	greenValueLineEdit->setText (std::to_string (color.g));
	blueValueLineEdit->setText (std::to_string (color.b));

	signalConnectionManager.connect (redValueLineEdit->returnPressed, [this]() {
		auto application = getActiveApplication();
		if (application) application->releaseKeyFocus (*redValueLineEdit);
	});
	signalConnectionManager.connect (greenValueLineEdit->returnPressed, [this]() {
		auto application = getActiveApplication();
		if (application) application->releaseKeyFocus (*redValueLineEdit);
	});
	signalConnectionManager.connect (blueValueLineEdit->returnPressed, [this]() {
		auto application = getActiveApplication();
		if (application) application->releaseKeyFocus (*redValueLineEdit);
	});
	signalConnectionManager.connect (redValueLineEdit->editingFinished, [this] (eValidatorState) {
		const auto color = colorPicker->getSelectedColor();
		const auto newRed = atoi (redValueLineEdit->getText().c_str());
		if (newRed != color.r)
		{
			colorPicker->setSelectedColor (color.exchangeRed (newRed));
		}
	});
	signalConnectionManager.connect (greenValueLineEdit->editingFinished, [this] (eValidatorState) {
		const auto color = colorPicker->getSelectedColor();
		const auto newGreen = atoi (greenValueLineEdit->getText().c_str());
		if (newGreen != color.g)
		{
			colorPicker->setSelectedColor (color.exchangeGreen (newGreen));
		}
	});
	signalConnectionManager.connect (blueValueLineEdit->editingFinished, [this] (eValidatorState) {
		const auto color = colorPicker->getSelectedColor();
		const auto newBlue = atoi (blueValueLineEdit->getText().c_str());
		if (newBlue != color.b)
		{
			colorPicker->setSelectedColor (color.exchangeBlue (newBlue));
		}
	});

	// FIXME: do not disable line edits here.
	//        currently it is disabled because the conversion from RGB to HSV Colors is not stable yet.
	//        This means converting a RGB color to HSV and then back to RGB will not result in the same
	//        RGB color that we started with.
	//        This results in confusing behavior when setting the RGB colors, because the value may will
	//        be changed by the color picking widget (which performs the conversions mentioned above) again.
	redValueLineEdit->disable();
	greenValueLineEdit->disable();
	blueValueLineEdit->disable();

	okButton = emplaceChild<cPushButton> (getPosition() + cPosition (200, 185), ePushButtonType::Angular, lngPack.i18n ("Others~OK"), eUnicodeFontType::LatinNormal);
	okButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_RETURN)));
	signalConnectionManager.connect (okButton->clicked, [this]() { done(); });

	cancelButton = emplaceChild<cPushButton> (getPosition() + cPosition (111, 185), ePushButtonType::Angular, lngPack.i18n ("Others~Cancel"), eUnicodeFontType::LatinNormal);
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, [this]() { canceled(); });
}

//------------------------------------------------------------------------------
cDialogColorPicker::~cDialogColorPicker()
{}

//------------------------------------------------------------------------------
void cDialogColorPicker::retranslate()
{
	okButton->setText (lngPack.i18n ("Others~OK"));
	cancelButton->setText (lngPack.i18n ("Others~Cancel"));
}

//------------------------------------------------------------------------------
cRgbColor cDialogColorPicker::getSelectedColor() const
{
	return colorPicker->getSelectedColor();
}

//------------------------------------------------------------------------------
AutoSurface cDialogColorPicker::createSelectedColorSurface()
{
	AutoSurface surface (SDL_CreateRGBSurface (0, 50, 50, 32, 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, toSdlAlphaColor (colorPicker->getSelectedColor(), *surface));

	return surface;
}
