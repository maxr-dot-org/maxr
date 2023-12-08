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

#include "ui/graphical/menu/widgets/colorpicker.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "input/mouse/mouse.h"
#include "ui/widgets/application.h"
#include "ui/widgets/image.h"

//------------------------------------------------------------------------------
cRgbColorPicker::cRgbColorPicker (const cBox<cPosition>& area, const cRgbColor& color) :
	cWidget (area),
	currentColor (color.toHsv()),
	startedPressInColor (false),
	startedPressInBar (false)
{
	colorsImage = emplaceChild<cImage> (getPosition());
	colorsImage->setImage (createColorsSurface().get());
	colorsImage->setConsumeClick (false);

	colorBarImage = emplaceChild<cImage> (cPosition (colorsImage->getEndPosition().x() + 10, getPosition().y()));
	colorBarImage->setImage (createColorBarSurface().get());
	colorBarImage->setConsumeClick (false);

	selectedColorMarker = emplaceChild<cImage> (getPosition());
	selectedColorMarker->setImage (createColorMarkerSurface().get());
	selectedColorMarker->setConsumeClick (false);

	selectedColorHueMarker = emplaceChild<cImage> (getPosition());
	selectedColorHueMarker->setImage (createColorHueMarkerSurface().get());
	selectedColorHueMarker->setConsumeClick (false);

	updateMarkers();
}

//------------------------------------------------------------------------------
void cRgbColorPicker::setSelectedColor (const cRgbColor& color)
{
	setSelectedColor (color.toHsv());
}

//------------------------------------------------------------------------------
void cRgbColorPicker::setSelectedColor (const cHsvColor& color)
{
	const auto oldColor = currentColor;

	currentColor = color;

	if (currentColor.h != oldColor.h)
	{
		colorsImage->setImage (createColorsSurface().get());
	}

	if (currentColor != oldColor)
	{
		updateMarkers();
		selectedColorChanged();
	}
}

//------------------------------------------------------------------------------
cRgbColor cRgbColorPicker::getSelectedColor() const
{
	return currentColor.toRgb();
}

//------------------------------------------------------------------------------
AutoSurface cRgbColorPicker::createColorsSurface()
{
	const cPosition size (getSize().x() - 15 - 10, getSize().y());

	AutoSurface surface (SDL_CreateRGBSurface (0, size.x(), size.y(), 32, 0, 0, 0, 0));

	auto color = currentColor;
	for (int y = 0; y < size.y(); ++y)
	{
		for (int x = 0; x < size.x(); ++x)
		{
			color.s = x * 100 / (size.x() - 1);
			color.v = 100 - (y * 100 / (size.y() - 1));

			putPixel (*surface, cPosition (x, y), toSdlAlphaColor (color.toRgb(), surface->format));
		}
	}

	return surface;
}

//------------------------------------------------------------------------------
AutoSurface cRgbColorPicker::createColorBarSurface()
{
	const cPosition size (15, getSize().y());

	AutoSurface surface (SDL_CreateRGBSurface (0, size.x(), size.y(), 32, 0, 0, 0, 0));

	for (int y = 0; y < size.y(); ++y)
	{
		const auto color = cHsvColor (y * 360 / size.y(), 100, 100);
		for (int x = 0; x < size.x(); ++x)
		{
			const cPosition position (x, y);

			putPixel (*surface, position, toSdlAlphaColor (color.toRgb(), surface->format));
		}
	}

	return surface;
}

//------------------------------------------------------------------------------
AutoSurface cRgbColorPicker::createColorMarkerSurface()
{
	AutoSurface surface (SDL_CreateRGBSurface (0, 3, 3, 32, 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, toSdlColor (cRgbColor::white(), *surface));
	drawPoint (*surface, cPosition (1, 1), cRgbColor (0xFF, 0, 0xFF));

	return surface;
}

//------------------------------------------------------------------------------
AutoSurface cRgbColorPicker::createColorHueMarkerSurface()
{
	AutoSurface surface (SDL_CreateRGBSurface (0, 15 + 2, 3, 32, 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, toSdlColor (cRgbColor::white(), *surface));

	drawLine (*surface, cPosition (1, 1), cPosition (16, 1), cRgbColor (0xFF, 0, 0xFF));

	return surface;
}

//------------------------------------------------------------------------------
void cRgbColorPicker::updateMarkers()
{
	const auto relativeHueMarkerPosition = currentColor.h * colorBarImage->getSize().y() / 360;
	selectedColorHueMarker->moveTo (cPosition (colorBarImage->getPosition().x() - 1, colorBarImage->getPosition().y() + relativeHueMarkerPosition - 1));

	const auto relativeMarkerPositionX = currentColor.s * (colorsImage->getSize().x() - 1) / 100;
	const auto relativeMarkerPositionY = (100 - currentColor.v) * (colorsImage->getSize().y() - 1) / 100;

	selectedColorMarker->moveTo (cPosition (colorsImage->getPosition().x() + relativeMarkerPositionX - 1, colorsImage->getPosition().y() + relativeMarkerPositionY - 1));
}

//------------------------------------------------------------------------------
bool cRgbColorPicker::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	if (!application.hasMouseFocus (*this)) return false;

	if (startedPressInColor)
	{
		updateColorByMousePosition (mouse.getPosition());
	}
	else if (startedPressInBar)
	{
		updateColorHueByMousePosition (mouse.getPosition());
	}

	return true;
}

//------------------------------------------------------------------------------
bool cRgbColorPicker::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button != eMouseButtonType::Left) return false;

	if (colorsImage->isAt (mouse.getPosition()))
	{
		startedPressInColor = true;
		application.grapMouseFocus (*this);
		updateColorByMousePosition (mouse.getPosition());
	}
	else if (colorBarImage->isAt (mouse.getPosition()))
	{
		startedPressInBar = true;
		application.grapMouseFocus (*this);
		updateColorHueByMousePosition (mouse.getPosition());
	}
	return false;
}

//------------------------------------------------------------------------------
bool cRgbColorPicker::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	application.releaseMouseFocus (*this);
	return false;
}

//------------------------------------------------------------------------------
void cRgbColorPicker::handleLooseMouseFocus (cApplication& application)
{
	startedPressInBar = false;
	startedPressInColor = false;
}

//------------------------------------------------------------------------------
void cRgbColorPicker::updateColorByMousePosition (const cPosition& mousePosition)
{
	const cPosition relativePosition = mousePosition - colorsImage->getPosition();

	const auto relativeX = std::max (std::min (relativePosition.x(), colorsImage->getSize().x() - 1), 0);
	const auto relativeY = std::max (std::min (relativePosition.y(), colorsImage->getSize().y() - 1), 0);

	auto newColor = currentColor;

	newColor.s = relativeX * 100 / (colorsImage->getSize().x() - 1);
	newColor.v = 100 - (relativeY * 100 / (colorsImage->getSize().y() - 1));

	setSelectedColor (newColor);
}

//------------------------------------------------------------------------------
void cRgbColorPicker::updateColorHueByMousePosition (const cPosition& mousePosition)
{
	const cPosition relativePosition = mousePosition - colorBarImage->getPosition();

	const auto relativeY = std::max (std::min (relativePosition.y(), colorBarImage->getSize().y() - 1), 0);

	auto newColor = currentColor;

	newColor.h = relativeY * 360 / colorBarImage->getSize().y();

	setSelectedColor (newColor);
}
