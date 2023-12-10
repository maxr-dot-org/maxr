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

#include "colorselector.h"

#include "output/video/video.h"
#include "resources/playercolor.h"
#include "ui/graphical/menu/dialogs/dialogcolorpicker.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/widgets/application.h"
#include "ui/widgets/image.h"

//------------------------------------------------------------------------------
cColorSelector::cColorSelector (const cPosition& pos, const cRgbColor& color) :
	cWidget (cBox<cPosition> (pos, pos + cPosition (140, 20))), color (color)
{
	auto prevColorButton = emplaceChild<cPushButton> (getPosition(), ePushButtonType::ArrowLeftSmall, &SoundData.SNDObjectMenu);
	colorImage = emplaceChild<cImage> (getPosition() + cPosition (27, 4));
	auto nextColorButton = emplaceChild<cPushButton> (getPosition() + cPosition (118, 0), ePushButtonType::ArrowRightSmall, &SoundData.SNDObjectMenu);
	setColor (color);
	signalConnectionManager.connect (colorImage->clicked, [this]() {
		auto application = getActiveApplication();

		if (!application) return;

		auto dialog = application->show (std::make_shared<cDialogColorPicker> (this->color));
		dialog->done.connect ([this, dialog]() {
			setColor (dialog->getSelectedColor());
			dialog->close();
		});
		dialog->canceled.connect ([dialog]() { dialog->close(); });
	});
	signalConnectionManager.connect (prevColorButton->clicked, [this]() {
		const auto localPlayerColorIndex = (cPlayerColor::findClosestPredefinedColor (this->color) + cPlayerColor::predefinedColorsCount - 1) % cPlayerColor::predefinedColorsCount;
		setColor (cPlayerColor::predefinedColors[localPlayerColorIndex]);
	});
	signalConnectionManager.connect (nextColorButton->clicked, [this]() {
		const auto localPlayerColorIndex = (cPlayerColor::findClosestPredefinedColor (this->color) + 1) % cPlayerColor::predefinedColorsCount;
		setColor (cPlayerColor::predefinedColors[localPlayerColorIndex]);
	});
	signalConnectionManager.connect (onColorChanged, [this] (const cRgbColor& color) { setColor (color); });
}

//------------------------------------------------------------------------------
void cColorSelector::setColor (const cRgbColor& color)
{
	SDL_Rect src = {0, 0, 83, 10};
	UniqueSurface colorSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_BlitSurface (cPlayerColor::getTexture (color), &src, colorSurface.get(), nullptr);
	colorImage->setImage (colorSurface.get());

	if (this->color != color)
	{
		this->color = color;
		onColorChanged (this->color);
	}
}
