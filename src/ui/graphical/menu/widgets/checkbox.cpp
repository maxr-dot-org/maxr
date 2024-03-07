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

#include "ui/graphical/menu/widgets/checkbox.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "utility/color.h"

#include <cassert>

//------------------------------------------------------------------------------
cCheckBox::cCheckBox (const cPosition& position, eCheckBoxType type_, bool centered, cSoundChunk* clickSound_) :
	cCheckBox (position, "", eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Right, type_, centered, clickSound_)
{
}

//------------------------------------------------------------------------------
cCheckBox::cCheckBox (const cPosition& position, const std::string& text_, eUnicodeFontType fontType_, eCheckBoxTextAnchor textAnchor_, eCheckBoxType type_, bool centered, cSoundChunk* clickSound_) :
	cClickableWidget (position),
	type (type_),
	text (text_),
	fontType (fontType_),
	textAnchor (textAnchor_),
	clickSound (clickSound_)
{
	renewSurface();
	if (centered) move (cPosition (-getSize().x() / 2, 0));
}

//------------------------------------------------------------------------------
void cCheckBox::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	auto font = cUnicodeFont::font.get();

	auto position = toSdlRect (getArea());
	int textDestx = -1;
	int textDesty = -1;
	if (surface != nullptr)
	{
		switch (textAnchor)
		{
			case eCheckBoxTextAnchor::Right:
				textDestx = position.x + surface->w + 2;
				textDesty = position.y + (position.h / 2) - (font->getFontHeight (fontType) / 2);
				break;
			case eCheckBoxTextAnchor::Left:
				textDestx = position.x - (font->getTextWide (text, fontType) + 2);
				textDesty = position.y + (position.h / 2) - (font->getFontHeight (fontType) / 2);
				break;
		}
	}

	switch (type)
	{
		default:
		case eCheckBoxType::Standard:
		case eCheckBoxType::Round:
			SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
			font->showText (textDestx, textDesty, text, fontType);
			break;
		case eCheckBoxType::Tank:
		case eCheckBoxType::Plane:
		case eCheckBoxType::Ship:
		case eCheckBoxType::Building:
		case eCheckBoxType::Tnt:
			SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
			break;
		case eCheckBoxType::TextOnly:
			font->showText (position.x, position.y, text, fontType);
			if (checked)
			{
				const cRgbColor selectionColor (0xE3, 0xDA, 0xCF);
				auto dest = getArea();
				dest.getMinCorner().x() -= 3;
				dest.getMinCorner().y() -= 2;
				dest.getMaxCorner().x() += 3;
				dest.getMaxCorner().y() += 1;
				drawRectangle (destination, dest, selectionColor);
			}
			break;
		case eCheckBoxType::Angular:
			SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
			if (checked)
				font->showTextCentered (position.x + position.w / 2, position.y + 5, text, fontType);
			else
				font->showTextCentered (position.x + position.w / 2, position.y + 4, text, fontType);
			break;
		case eCheckBoxType::HudIndex_00:
		case eCheckBoxType::HudIndex_01:
		case eCheckBoxType::HudIndex_02:
			textDesty = 7;
			[[fallthrough]];
		case eCheckBoxType::HudIndex_10:
		case eCheckBoxType::HudIndex_11:
		case eCheckBoxType::HudIndex_12:
		case eCheckBoxType::HudChat:
			if (textDesty != 7) textDesty = 6;
			[[fallthrough]];
		case eCheckBoxType::HudIndex_20:
		case eCheckBoxType::HudIndex_21:
		case eCheckBoxType::HudIndex_22:
			if (textDesty != 6 && textDesty != 7) textDesty = 5;
			SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
			if (checked)
				font->showTextCentered (position.x + position.w / 2, position.y + textDesty, text, eUnicodeFontType::LatinSmallGreen);
			else
				font->showTextCentered (position.x + position.w / 2, position.y + textDesty - 1, text, eUnicodeFontType::LatinSmallRed);
			font->showTextCentered (position.x + position.w / 2 - 1, position.y + textDesty - 1 + (checked ? 1 : 0), text, eUnicodeFontType::LatinSmallWhite);
			break;
		case eCheckBoxType::UnitContextMenu:
			SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
			font->showTextCentered (position.x + position.w / 2, position.y + (position.h / 2 - font->getFontHeight (eUnicodeFontType::LatinSmallWhite) / 2) + 1, text, eUnicodeFontType::LatinSmallWhite);
			break;
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
bool cCheckBox::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (isLocked) return false;
	return cClickableWidget::handleMousePressed (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cCheckBox::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (isLocked)
	{
		cClickableWidget::finishMousePressed (application, mouse, button);
		return false;
	}
	return cClickableWidget::handleMouseReleased (application, mouse, button);
}

//------------------------------------------------------------------------------
void cCheckBox::setChecked (bool checked_)
{
	if (checked != checked_) toggle();
}

//------------------------------------------------------------------------------
bool cCheckBox::isChecked() const
{
	return checked;
}

//------------------------------------------------------------------------------
void cCheckBox::toggle()
{
	checked = !checked;

	renewSurface();
	toggled();
}

//------------------------------------------------------------------------------
void cCheckBox::setText (const std::string& newText)
{
	text = newText;
}

//------------------------------------------------------------------------------
void cCheckBox::lock()
{
	isLocked = true;
	renewSurface();
}

//------------------------------------------------------------------------------
void cCheckBox::unlock()
{
	isLocked = false;
	renewSurface();
}

//------------------------------------------------------------------------------
void cCheckBox::setPressed (bool pressed)
{
	cClickableWidget::setPressed (pressed);
	renewSurface();
}

//------------------------------------------------------------------------------
bool cCheckBox::handleClicked (cApplication&, cMouse&, eMouseButtonType) /* override */
{
	if (clickSound) cSoundDevice::getInstance().playSoundEffect (*clickSound);

	toggle();

	return true;
}

//------------------------------------------------------------------------------
void cCheckBox::renewSurface()
{
	auto font = cUnicodeFont::font.get();
	std::optional<int> textLimitWidth;

	sPartialSurface partialSurface{};
	switch (type)
	{
		case eCheckBoxType::TextOnly:
			surface = nullptr;
			partialSurface.rect.w = font->getTextWide (text, fontType);
			partialSurface.rect.h = font->getFontHeight (fontType);
			break;
		default:
		case eCheckBoxType::Round:
			partialSurface = GraphicsData.get_CheckBox_Round (checked || isPressed);
			break;
		case eCheckBoxType::Angular:
			partialSurface = GraphicsData.get_CheckBox_Angular (checked || isPressed);
			textLimitWidth = partialSurface.rect.w - 16;
			break;
		case eCheckBoxType::Standard:
			partialSurface = GraphicsData.get_CheckBox_Standard (checked || isPressed);
			break;
		case eCheckBoxType::Tank:
			partialSurface = GraphicsData.get_CheckBox_Tank (checked || isPressed);
			break;
		case eCheckBoxType::Plane:
			partialSurface = GraphicsData.get_CheckBox_Plane (checked || isPressed);
			break;
		case eCheckBoxType::Ship:
			partialSurface = GraphicsData.get_CheckBox_Ship (checked || isPressed);
			break;
		case eCheckBoxType::Building:
			partialSurface = GraphicsData.get_CheckBox_Building (checked || isPressed);
			break;
		case eCheckBoxType::Tnt:
			partialSurface = GraphicsData.get_CheckBox_Tnt (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_00:
			partialSurface = GraphicsData.get_CheckBox_HudIndex00 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_01:
			partialSurface = GraphicsData.get_CheckBox_HudIndex01 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_02:
			partialSurface = GraphicsData.get_CheckBox_HudIndex02 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_10:
			partialSurface = GraphicsData.get_CheckBox_HudIndex10 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_11:
			partialSurface = GraphicsData.get_CheckBox_HudIndex11 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_12:
			partialSurface = GraphicsData.get_CheckBox_HudIndex12 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_20:
			partialSurface = GraphicsData.get_CheckBox_HudIndex20 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_21:
			partialSurface = GraphicsData.get_CheckBox_HudIndex21 (checked || isPressed);
			break;
		case eCheckBoxType::HudIndex_22:
			partialSurface = GraphicsData.get_CheckBox_HudIndex22 (checked || isPressed);
			break;
		case eCheckBoxType::HudChat:
			partialSurface = GraphicsData.get_CheckBox_HudChat (checked || isPressed);
			break;
		case eCheckBoxType::HudLock:
			partialSurface = (checked || isPressed) ? GraphicsData.get_CheckBox_HudLock() : GraphicsData.get_CheckBox_HudUnlock();
			break;
		case eCheckBoxType::HudTnt:
			partialSurface = GraphicsData.get_CheckBox_HudTnt (checked || isPressed);
			break;
		case eCheckBoxType::Hud2x:
			partialSurface = GraphicsData.get_CheckBox_Hud2x (checked || isPressed);
			break;
		case eCheckBoxType::HudPlayers:
			partialSurface = GraphicsData.get_CheckBox_HudPlayer (checked || isPressed);
			break;
		case eCheckBoxType::UnitContextMenu:
			partialSurface = GraphicsData.get_CheckBox_UnitContextMenu (checked || isPressed);
			break;
		case eCheckBoxType::ArrowDownSmall:
			partialSurface = GraphicsData.get_CheckBox_ArrowDownSmall (checked || isPressed);
			break;
	}
	resize ({partialSurface.rect.w, partialSurface.rect.h});

	if (partialSurface.surface)
	{
		surface = UniqueSurface (SDL_CreateRGBSurface (0, partialSurface.rect.w, partialSurface.rect.h, Video.getColDepth(), 0, 0, 0, 0));
		SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
		SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

		SDL_BlitSurface (partialSurface.surface, &partialSurface.rect, surface.get(), nullptr);
	}

	if (shorten && textLimitWidth) text = font->shortenStringToSize (text, *textLimitWidth, fontType);
}
