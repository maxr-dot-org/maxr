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

#include "pushbutton.h"

#include "../../application.h"

#include "../../../settings.h"
#include "../../../video.h"
#include "../../../unifonts.h"
#include "../../../main.h"
#include "../../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (FONT_LATIN_BIG),
	text (""),
	clickSound (SoundData.SNDHudButton),
	isLocked (false)
{
	renewSurface ();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, sSOUND* clickSound_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (FONT_LATIN_BIG),
	text (""),
	clickSound (clickSound_),
	isLocked (false)
{
	renewSurface ();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, const std::string& text_, eUnicodeFontType fontType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (fontType_),
	text (text_),
	clickSound (SoundData.SNDHudButton),
	isLocked (false)
{
	renewSurface ();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, sSOUND* clickSound_, const std::string& text_, eUnicodeFontType fontType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (fontType_),
	text (text_),
	clickSound (clickSound_),
	isLocked (false)
{
	renewSurface ();
}

//------------------------------------------------------------------------------
void cPushButton::draw ()
{
	SDL_Rect position = getArea ().toSdlRect ();
	if (surface) SDL_BlitSurface (surface, NULL, cVideo::buffer, &position);

	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
	{
		if (isPressed || isLocked) font->showTextCentered (position.x + position.w / 2, position.y + getTextYOffset (), text, FONT_LATIN_SMALL_GREEN);
		else font->showTextCentered (position.x + position.w / 2, position.y + getTextYOffset () - 1, text, FONT_LATIN_SMALL_RED);
		font->showTextCentered (position.x + position.w / 2 - 1, position.y + getTextYOffset () - 1 + (isPressed || isLocked ? 1 : 0), text, FONT_LATIN_SMALL_WHITE);
	}
	else font->showTextCentered (position.x + position.w / 2, position.y + getTextYOffset (), text, fontType);

	cWidget::draw ();
}

//------------------------------------------------------------------------------
bool cPushButton::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (isLocked) return false;
	return cClickableWidget::handleMousePressed (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cPushButton::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (isLocked) return false;
	return cClickableWidget::handleMouseReleased (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cPushButton::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		if (clickSound) PlayFX (clickSound);
		clicked ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cPushButton::setPressed (bool pressed)
{
	cClickableWidget::setPressed (pressed);
	renewSurface ();
}

//------------------------------------------------------------------------------
void cPushButton::lock ()
{
	isLocked = true;
	renewSurface ();
}

//------------------------------------------------------------------------------
void cPushButton::unlock ()
{
	isLocked = false;
	renewSurface ();
}

//------------------------------------------------------------------------------
void cPushButton::renewSurface ()
{
	cPosition size;
	SDL_Rect src;
	switch (buttonType)
	{
	default:
	case ePushButtonType::StandardBig:
		src.x = 0;
		src.y = (isPressed || isLocked) ? 29 : 0;
		size = cPosition (200, 29);
		break;
	case ePushButtonType::StandardSmall:
		src.x = 0;
		src.y = (isPressed || isLocked) ? 87 : 58;
		size = cPosition (150, 29);
		break;
	case ePushButtonType::Huge:
		src.x = (isPressed || isLocked) ? 109 : 0;
		src.y = 116;
		size = cPosition (109, 40);
		break;
	case ePushButtonType::ArrowUpBig:
		src.x = (isPressed || isLocked) ? 125 : 97;
		src.y = 157;
		size = cPosition (28, 29);
		break;
	case ePushButtonType::ArrowDownBig:
		src.x = (isPressed || isLocked) ? 181 : 153;
		src.y = 157;
		size = cPosition (28, 29);
		break;
	case ePushButtonType::ArrowLeftBig:
		src.x = (isPressed || isLocked) ? 293 : 265;
		src.y = 157;
		size = cPosition (28, 29);
		break;
	case ePushButtonType::ArrowRightBig:
		src.x = (isPressed || isLocked) ? 237 : 209;
		src.y = 157;
		size = cPosition (28, 29);
		break;
	case ePushButtonType::ArrowUpSmall:
		size = cPosition (18, 17);
		src.x = (isPressed || isLocked) ? 151 + size.x () : 151;
		src.y = 59;
		break;
	case ePushButtonType::ArrowDownSmall:
		size = cPosition (18, 17);
		src.x = (isPressed || isLocked) ? 187 + size.x () : 187;
		src.y = 59;
		break;
	case ePushButtonType::ArrowLeftSmall:
		size = cPosition (18, 17);
		src.x = (isPressed || isLocked) ? 151 + size.x () : 151;
		src.y = 76;
		break;
	case ePushButtonType::ArrowRightSmall:
		size = cPosition (18, 17);
		src.x = (isPressed || isLocked) ? 187 + size.x () : 187;
		src.y = 76;
		break;
	case ePushButtonType::ArrowUpBar:
		size = cPosition (17, 17);
		src.x = (isPressed || isLocked) ? 201 + size.x () : 201;
		src.y = 1;
		break;
	case ePushButtonType::ArrowDownBar:
		size = cPosition (17, 17);
		src.x = (isPressed || isLocked) ? 201 + size.x () : 201;
		src.y = 18;
		break;
	case ePushButtonType::Angular:
		size = cPosition (78, 23);
		src.x = (isPressed || isLocked) ? size.x () : 0;
		src.y = 196;
		break;
	case ePushButtonType::HudHelp:
		src.x = (isPressed || isLocked) ? 366 : 268;
		src.y = (isPressed || isLocked) ? 0 : 151;
		size = cPosition (26, 24);
		break;
	case ePushButtonType::HudCenter:
		src.x = (isPressed || isLocked) ? 0 : 139;
		src.y = (isPressed || isLocked) ? 21 : 149;
		size = cPosition (21, 22);
		break;
	case ePushButtonType::HudReport:
		src.x = (isPressed || isLocked) ? 210 : 245;
		src.y = (isPressed || isLocked) ? 21 : 130;
		size = cPosition (49, 20);
		break;
	case ePushButtonType::HudChat:
		src.x = (isPressed || isLocked) ? 160 : 196;
		src.y = (isPressed || isLocked) ? 21 : 129;
		size = cPosition (49, 20);
		break;
	case ePushButtonType::HudNext:
		src.x = (isPressed || isLocked) ? 288 : 158;
		src.y = (isPressed || isLocked) ? 0 : 172;
		size = cPosition (39, 23);
		break;
	case ePushButtonType::HudPrev:
		src.x = (isPressed || isLocked) ? 327 : 198;
		src.y = (isPressed || isLocked) ? 0 : 172;
		size = cPosition (38, 23);
		break;
	case ePushButtonType::HudDone:
		src.x = (isPressed || isLocked) ? 262 : 132;
		src.y = (isPressed || isLocked) ? 0 : 172;
		size = cPosition (26, 24);
		break;
	case ePushButtonType::HudEnd:
		src.x = (isPressed || isLocked) ? 22 : 0;
		src.y = (isPressed || isLocked) ? 21 : 151;
		size = cPosition (70, 17);
		break;
	case ePushButtonType::HudPreferences:
		src.x = (isPressed || isLocked) ? 195 : 0;
		src.y = (isPressed || isLocked) ? 0 : 169;
		size = cPosition (67, 20);
		break;
	case ePushButtonType::HudFiles:
		src.x = (isPressed || isLocked) ? 93 : 71;
		src.y = (isPressed || isLocked) ? 21 : 151;
		size = cPosition (67, 20);
		break;
	case ePushButtonType::HudPlay:
		src.x = (isPressed || isLocked) ? 157 : 0;
		src.y = (isPressed || isLocked) ? 0 : 132;
		size = cPosition (19, 18);
		break;
	case ePushButtonType::HudStop:
		src.x = (isPressed || isLocked) ? 176 : 19;
		src.y = (isPressed || isLocked) ? 0 : 132;
		size = cPosition (19, 19);
		break;
	}
	resize (size);
	src.w = size.x ();
	src.h = size.y ();
	surface = SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth (), 0, 0, 0, 0);
	SDL_SetColorKey (surface, SDL_TRUE, 0xFF00FF);
	SDL_FillRect (surface, NULL, 0xFF00FF);
	if (buttonType >= ePushButtonType::HudHelp && buttonType <= ePushButtonType::HudStop) SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &src, surface, NULL);
	else SDL_BlitSurface (GraphicsData.gfx_menu_stuff, &src, surface, NULL);

	text = font->shortenStringToSize (text, size.x() - getBordersSize (), fontType);
}

//------------------------------------------------------------------------------
int cPushButton::getTextYOffset () const
{
	switch (buttonType)
	{
	default:
	case ePushButtonType::StandardBig:
	case ePushButtonType::StandardSmall:
		return 7;
	case ePushButtonType::Huge:
		return 11;
	case ePushButtonType::ArrowUpBig:
	case ePushButtonType::ArrowDownBig:
	case ePushButtonType::ArrowLeftBig:
	case ePushButtonType::ArrowRightBig:
	case ePushButtonType::ArrowUpSmall:
	case ePushButtonType::ArrowDownSmall:
	case ePushButtonType::ArrowLeftSmall:
	case ePushButtonType::ArrowRightSmall:
	case ePushButtonType::ArrowUpBar:
	case ePushButtonType::ArrowDownBar:
		return -1;
	case ePushButtonType::Angular:
		if (isPressed || isLocked) return 5;
		else return 4;
	case ePushButtonType::HudNext:
	case ePushButtonType::HudPrev:
	case ePushButtonType::HudDone:
		return 9;
	case ePushButtonType::HudEnd:
		if (isPressed || isLocked) return 3;
		else return 2;
	case ePushButtonType::HudReport:
	case ePushButtonType::HudChat:
	case ePushButtonType::HudPreferences:
	case ePushButtonType::HudFiles:
		return 6;
	}
}

//------------------------------------------------------------------------------
int cPushButton::getBordersSize () const
{
	switch (buttonType)
	{
	case ePushButtonType::HudDone:
		return 0;
	default:
		return 12;
	}
}
