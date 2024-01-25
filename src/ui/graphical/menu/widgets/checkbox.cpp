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
bool cCheckBox::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (clickSound) cSoundDevice::getInstance().playSoundEffect (*clickSound);

	toggle();

	return true;
}

//------------------------------------------------------------------------------
void cCheckBox::renewSurface()
{
	cPosition size;
	SDL_Rect src = {0, 0, 0, 0};
	auto font = cUnicodeFont::font.get();
	int textLimitWidth = -1;

	if (type >= eCheckBoxType::HudIndex_00 && type <= eCheckBoxType::HudIndex_22)
	{
		src.y = 44;
		size = cPosition (55, 0);
	}
	switch (type)
	{
		case eCheckBoxType::TextOnly:
			surface = nullptr;
			size = cPosition (font->getTextWide (text, fontType), font->getFontHeight (fontType));
			break;
		default:
		case eCheckBoxType::Round:
			size = cPosition (18, 17);
			src.x = (checked || isPressed) ? 151 + size.x() : 151;
			src.y = 93;
			break;
		case eCheckBoxType::Angular:
			size = cPosition (78, 23);
			src.x = (checked || isPressed) ? size.x() : 0;
			src.y = 196;
			textLimitWidth = size.x() - 16;
			break;
		case eCheckBoxType::Standard:
			size = cPosition (18, 17);
			src.x = (checked || isPressed) ? 187 + size.x() : 187;
			src.y = 93;
			break;
		case eCheckBoxType::Tank:
			size = cPosition (32, 31);
			src.x = (checked || isPressed) ? size.x() : 0;
			src.y = 219;
			break;
		case eCheckBoxType::Plane:
			size = cPosition (32, 31);
			src.x = (checked || isPressed) ? 32 * 2 + size.x() : 32 * 2;
			src.y = 219;
			break;
		case eCheckBoxType::Ship:
			size = cPosition (32, 31);
			src.x = (checked || isPressed) ? 32 * 4 + size.x() : 32 * 4;
			src.y = 219;
			break;
		case eCheckBoxType::Building:
			size = cPosition (32, 31);
			src.x = (checked || isPressed) ? 32 * 6 + size.x() : 32 * 6;
			src.y = 219;
			break;
		case eCheckBoxType::Tnt:
			size = cPosition (32, 31);
			src.x = (checked || isPressed) ? 32 * 8 + size.x() : 32 * 8;
			src.y = 219;
			break;
		case eCheckBoxType::HudIndex_00:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex00Pressed() : cGraphicsData::getRect_CheckBox_HudIndex00();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_01:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex01Pressed() : cGraphicsData::getRect_CheckBox_HudIndex01();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_02:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex02Pressed() : cGraphicsData::getRect_CheckBox_HudIndex02();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_10:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex10Pressed() : cGraphicsData::getRect_CheckBox_HudIndex10();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_11:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex11Pressed() : cGraphicsData::getRect_CheckBox_HudIndex11();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_12:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex12Pressed() : cGraphicsData::getRect_CheckBox_HudIndex12();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_20:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex20Pressed() : cGraphicsData::getRect_CheckBox_HudIndex20();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_21:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex21Pressed() : cGraphicsData::getRect_CheckBox_HudIndex21();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudIndex_22:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudIndex22Pressed() : cGraphicsData::getRect_CheckBox_HudIndex22();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudChat:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudChatPressed() : cGraphicsData::getRect_CheckBox_HudChat();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudLock:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudLock() : cGraphicsData::getRect_CheckBox_HudUnlock();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudTnt:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudTntPressed() : cGraphicsData::getRect_CheckBox_HudTnt();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::Hud2x:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_Hud2xPressed() : cGraphicsData::getRect_CheckBox_Hud2x();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::HudPlayers:
			src = (checked || isPressed) ? cGraphicsData::getRect_CheckBox_HudPlayerPressed() : cGraphicsData::getRect_CheckBox_HudPlayer();
			size = cPosition (src.w, src.h);
			break;
		case eCheckBoxType::UnitContextMenu:
			size = cPosition (42, 21);
			src.x = 0;
			src.y = (checked || isPressed) ? 21 : 0;
			break;
		case eCheckBoxType::ArrowDownSmall:
			size = cPosition (18, 17);
			src.x = (checked || isPressed) ? 187 + size.x() : 187;
			src.y = 59;
			break;
	}
	resize (size);
	src.w = size.x();
	src.h = size.y();

	if (src.w > 0)
	{
		surface = UniqueSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
		SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
		SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

		SDL_Surface* srcSurface = nullptr;

		if (type >= eCheckBoxType::HudIndex_00 && type <= eCheckBoxType::HudPlayers)
			srcSurface = GraphicsData.gfx_hud_stuff.get();
		else if (type == eCheckBoxType::UnitContextMenu)
			srcSurface = GraphicsData.gfx_context_menu.get();
		else
			srcSurface = GraphicsData.gfx_menu_stuff.get();

		assert (srcSurface != nullptr);

		SDL_BlitSurface (srcSurface, &src, surface.get(), nullptr);
	}

	if (shorten && textLimitWidth != -1) text = font->shortenStringToSize (text, textLimitWidth, fontType);
}
