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

#include "checkbox.h"

#include "../../../settings.h"
#include "../../../video.h"
#include "../../../main.h"

//------------------------------------------------------------------------------
cCheckBox::cCheckBox (const cPosition& position, eCheckBoxType type_, bool centered, sSOUND* clickSound_) :
	cClickableWidget (position),
	type (type_),
	fontType (FONT_LATIN_NORMAL),
	textAnchor (eCheckBoxTextAnchor::Right),
	textLimitWidth (-1),
	clickSound (clickSound_),
	checked (false)
{
	renewSurface ();
	if (centered) move (cPosition (-getSize ().x () / 2, 0));
}

cCheckBox::cCheckBox (const cPosition& position, const std::string& text_, eUnicodeFontType fontType_, eCheckBoxTextAnchor textAnchor_, eCheckBoxType type_, bool centered, sSOUND* clickSound_) :
	cClickableWidget (position),
	type (type_),
	text (text_),
	fontType (fontType_),
	textAnchor (textAnchor_),
	textLimitWidth (-1),
	clickSound (clickSound_),
	checked (false)
{
	renewSurface ();
	if (centered) move (cPosition (-getSize ().x () / 2, 0));
}

//------------------------------------------------------------------------------
void cCheckBox::draw ()
{
	auto position = getArea ().toSdlRect ();
	int textDestx = -1;
	int textDesty = -1;
	if (surface)
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
		SDL_BlitSurface (surface, NULL, cVideo::buffer, &position);
		font->showText (textDestx, textDesty, text, fontType);
		break;
	case eCheckBoxType::Tank:
	case eCheckBoxType::Plane:
	case eCheckBoxType::Ship:
	case eCheckBoxType::Building:
	case eCheckBoxType::Tnt:
		SDL_BlitSurface (surface, NULL, cVideo::buffer, &position);
		break;
	case eCheckBoxType::TextOnly:
		font->showText (position.x, position.y, text, fontType);
		if (checked)
		{
			const Uint32 selection_color (0xFFE3DACF);
			auto dest = getArea ();
			dest.getMinCorner ().x () -= 3;
			dest.getMinCorner ().y () -= 2;
			dest.getMaxCorner ().x () += 3;
			dest.getMaxCorner ().y () += 1;
			drawRectangle (cVideo::buffer, dest, selection_color);
		}
		break;
	case eCheckBoxType::Angular:
		SDL_BlitSurface (surface, NULL, cVideo::buffer, &position);
		if (checked) font->showTextCentered (position.x + position.w / 2, position.y + 5, text, fontType);
		else font->showTextCentered (position.x + position.w / 2, position.y + 4, text, fontType);
		break;
	case eCheckBoxType::HudIndex_00:
	case eCheckBoxType::HudIndex_01:
	case eCheckBoxType::HudIndex_02:
		textDesty = 7;
	case eCheckBoxType::HudIndex_10:
	case eCheckBoxType::HudIndex_11:
	case eCheckBoxType::HudIndex_12:
		if (textDesty != 7) textDesty = 6;
	case eCheckBoxType::HudIndex_20:
	case eCheckBoxType::HudIndex_21:
	case eCheckBoxType::HudIndex_22:
		if (textDesty != 6 && textDesty != 7) textDesty = 5;
		SDL_BlitSurface (surface, NULL, cVideo::buffer, &position);
		if (checked) font->showTextCentered (position.x + position.w / 2, position.y + textDesty, text, FONT_LATIN_SMALL_GREEN);
		else font->showTextCentered (position.x + position.w / 2, position.y + textDesty - 1, text, FONT_LATIN_SMALL_RED);
		font->showTextCentered (position.x + position.w / 2 - 1, position.y + textDesty - 1 + (checked ? 1 : 0), text, FONT_LATIN_SMALL_WHITE);
		break;
	}

	cClickableWidget::draw ();
}

//------------------------------------------------------------------------------
void cCheckBox::setChecked (bool checked_)
{
	if (checked != checked_) toggle ();
}

//------------------------------------------------------------------------------
bool cCheckBox::isChecked () const
{
	return checked;
}

//------------------------------------------------------------------------------
void cCheckBox::toggle ()
{
	checked = !checked;

	renewSurface ();
	toggled ();
}

//------------------------------------------------------------------------------
void cCheckBox::setPressed (bool pressed)
{
	cClickableWidget::setPressed (pressed);
	renewSurface ();
}

//------------------------------------------------------------------------------
bool cCheckBox::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (clickSound) PlayFX (clickSound);

	toggle ();

	return true;
}

//------------------------------------------------------------------------------
void cCheckBox::renewSurface ()
{
	cPosition size;
	SDL_Rect src;

	if (type >= eCheckBoxType::HudIndex_00 && type <= eCheckBoxType::HudIndex_22)
	{
		src.y = 44;
		size = cPosition (55, 0);
	}
	switch (type)
	{
	default:
	case eCheckBoxType::Round:
		size = cPosition (18, 17);
		src.x = (checked || isPressed) ? 151 + size.x () : 151;
		src.y = 93;
		break;
	case eCheckBoxType::TextOnly:
		surface = nullptr;
		size = cPosition (font->getTextWide (text, fontType), font->getFontHeight (fontType));
		break;
	case eCheckBoxType::Standard:
		size = cPosition (18, 17);
		src.x = (checked || isPressed) ? 187 + size.x () : 187;
		src.y = 93;
		break;
	case eCheckBoxType::Tank:
		size = cPosition (32, 31);
		src.x = (checked || isPressed) ? size.x () : 0;
		src.y = 219;
		break;
	case eCheckBoxType::Plane:
		size = cPosition (32, 31);
		src.x = (checked || isPressed) ? 32 * 2 + size.x () : 32 * 2;
		src.y = 219;
		break;
	case eCheckBoxType::Ship:
		size = cPosition (32, 31);
		src.x = (checked || isPressed) ? 32 * 4 + size.x () : 32 * 4;
		src.y = 219;
		break;
	case eCheckBoxType::Building:
		size = cPosition (32, 31);
		src.x = (checked || isPressed) ? 32 * 6 + size.x () : 32 * 6;
		src.y = 219;
		break;
	case eCheckBoxType::Tnt:
		size = cPosition (32, 31);
		src.x = (checked || isPressed) ? 32 * 8 + size.x () : 32 * 8;
		src.y = 219;
		break;
	case eCheckBoxType::Angular:
		size = cPosition (78, 23);
		src.x = (checked || isPressed) ? size.x () : 0;
		src.y = 196;
		textLimitWidth = size.x() - 16;
		break;
	case eCheckBoxType::HudIndex_22:
		src.x += size.x ();
	case eCheckBoxType::HudIndex_21:
		src.x += size.x ();
	case eCheckBoxType::HudIndex_20:
		size.y () = 18;
		src.y += 18 + 16;
		if (!checked && !isPressed) src.x += 167;
		break;
	case eCheckBoxType::HudIndex_12:
		src.x += size.x ();
	case eCheckBoxType::HudIndex_11:
		src.x += size.x ();
	case eCheckBoxType::HudIndex_10:
		size.y () = 16;
		src.y += 18;
		if (!(checked || isPressed)) src.x += 167;
		break;
	case eCheckBoxType::HudIndex_02:
		src.x += size.x ();
	case eCheckBoxType::HudIndex_01:
		src.x += size.x ();
	case eCheckBoxType::HudIndex_00:
		size.y () = 18;
		if (!checked && !isPressed) src.x += 167;
		break;
	case eCheckBoxType::HudLock:
		size = cPosition (21, 22);
		src.x = 397;
		src.y = (checked || isPressed) ? 298 : 321;
		break;
	case eCheckBoxType::HudTnt:
		size = cPosition (27, 28);
		src.x = (checked || isPressed) ? 362 : 334;
		src.y = 24;
		break;
	case eCheckBoxType::Hud2x:
		size = cPosition (27, 28);
		src.x = (checked || isPressed) ? 362 : 334;
		src.y = 53;
		break;
	case eCheckBoxType::HudPlayers:
		size = cPosition (27, 28);
		src.x = (checked || isPressed) ? (317 + 27) : 317;
		src.y = 479;
		break;
	}
	resize (size);
	src.w = size.x ();
	src.h = size.y ();

	if (src.w > 0)
	{
		surface = SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth (), 0, 0, 0, 0);
		if (type >= eCheckBoxType::HudIndex_00 && type <= eCheckBoxType::HudPlayers) SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &src, surface, NULL);
		else SDL_BlitSurface (GraphicsData.gfx_menu_stuff, &src, surface, NULL);
	}

	if (textLimitWidth != -1) text = font->shortenStringToSize (text, textLimitWidth, fontType);
}
