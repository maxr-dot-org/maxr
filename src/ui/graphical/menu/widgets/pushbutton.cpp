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

#include "ui/graphical/menu/widgets/pushbutton.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "input/mouse/mouse.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/unifonts.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "ui/widgets/application.h"

#include <cassert>

//------------------------------------------------------------------------------
/* static */ cPosition cPushButton::getButtonSize (ePushButtonType buttonType)
{
	switch (buttonType)
	{
		default:
		case ePushButtonType::StandardBig: return cPosition (200, 29);
		case ePushButtonType::StandardSmall: return cPosition (150, 29);
		case ePushButtonType::Huge: return cPosition (109, 40);
		case ePushButtonType::ArrowUpBig: return cPosition (28, 29);
		case ePushButtonType::ArrowDownBig: return cPosition (28, 29);
		case ePushButtonType::ArrowLeftBig: return cPosition (28, 29);
		case ePushButtonType::ArrowRightBig: return cPosition (28, 29);
		case ePushButtonType::ArrowUpSmall: return cPosition (18, 17);
		case ePushButtonType::ArrowDownSmall: return cPosition (18, 17);
		case ePushButtonType::ArrowLeftSmall: return cPosition (18, 17);
		case ePushButtonType::ArrowRightSmall: return cPosition (18, 17);
		case ePushButtonType::ArrowUpBar: return cPosition (17, 17);
		case ePushButtonType::ArrowDownBar: return cPosition (17, 17);
		case ePushButtonType::Angular: return cPosition (78, 23);
		case ePushButtonType::HudHelp: return cPosition (26, 24);
		case ePushButtonType::HudCenter: return cPosition (21, 22);
		case ePushButtonType::HudReport: return cPosition (49, 20);
		case ePushButtonType::HudChat: return cPosition (49, 20);
		case ePushButtonType::HudNext: return cPosition (39, 23);
		case ePushButtonType::HudPrev: return cPosition (38, 23);
		case ePushButtonType::HudDone: return cPosition (26, 24);
		case ePushButtonType::HudEnd: return cPosition (70, 17);
		case ePushButtonType::HudPreferences: return cPosition (67, 20);
		case ePushButtonType::HudFiles: return cPosition (67, 20);
		case ePushButtonType::HudPlay: return cPosition (19, 18);
		case ePushButtonType::HudStop: return cPosition (19, 19);
		case ePushButtonType::UnitContextMenu: return cPosition (42, 21);
		case ePushButtonType::Destroy: return cPosition (59, 56);
		case ePushButtonType::ArrowUpSmallModern: return cPosition (16, 8);
		case ePushButtonType::ArrowDownSmallModern: return cPosition (16, 8);
		case ePushButtonType::ArrowLeftSmallModern: return cPosition (8, 16);
		case ePushButtonType::ArrowRightSmallModern: return cPosition (8, 16);
	}
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cBox<cPosition>& area) :
	cClickableWidget (area),
	buttonType (ePushButtonType::Invisible),
	fontType (eUnicodeFontType::LatinBig),
	text (""),
	clickSound (&SoundData.SNDHudButton),
	isLocked (false)

{
	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		fontType = eUnicodeFontType::LatinSmallWhite;
	renewSurface();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (eUnicodeFontType::LatinBig),
	text (""),
	clickSound (&SoundData.SNDHudButton),
	isLocked (false)
{
	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		fontType = eUnicodeFontType::LatinSmallWhite;
	renewSurface();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, cSoundChunk* clickSound_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (eUnicodeFontType::LatinBig),
	text (""),
	clickSound (clickSound_),
	isLocked (false)
{
	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		fontType = eUnicodeFontType::LatinSmallWhite;
	renewSurface();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, const std::string& text_, eUnicodeFontType fontType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (fontType_),
	text (text_),
	clickSound (&SoundData.SNDHudButton),
	isLocked (false)
{
	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		fontType = eUnicodeFontType::LatinSmallWhite;
	renewSurface();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, cSoundChunk* clickSound_, const std::string& text_, eUnicodeFontType fontType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (fontType_),
	text (text_),
	clickSound (clickSound_),
	isLocked (false)
{
	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		fontType = eUnicodeFontType::LatinSmallWhite;
	renewSurface();
}

//------------------------------------------------------------------------------
void cPushButton::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	SDL_Rect position = toSdlRect (getArea());
	if (surface != nullptr) SDL_BlitSurface (surface.get(), nullptr, &destination, &position);

	if (!text.empty())
	{
		auto font = cUnicodeFont::font.get();
		if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		{
			if (isPressed || isLocked)
				font->showTextCentered (position.x + position.w / 2, position.y + getTextYOffset(), text, eUnicodeFontType::LatinSmallGreen);
			else
				font->showTextCentered (position.x + position.w / 2, position.y + getTextYOffset() - 1, text, eUnicodeFontType::LatinSmallRed);
			font->showTextCentered (position.x + position.w / 2 - 1, position.y + getTextYOffset() - 1 + (isPressed || isLocked ? 1 : 0), text, eUnicodeFontType::LatinSmallWhite);
		}
		else
			font->showTextCentered (position.x + position.w / 2, position.y + getTextYOffset(), text, fontType);
	}

	cWidget::draw (destination, clipRect);
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
	if (isLocked)
	{
		cClickableWidget::finishMousePressed (application, mouse, button);
		return false;
	}
	return cClickableWidget::handleMouseReleased (application, mouse, button);
}

//------------------------------------------------------------------------------
bool cPushButton::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left && !isLocked)
	{
		if (clickSound) cSoundDevice::getInstance().playSoundEffect (*clickSound);
		clicked();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void cPushButton::setPressed (bool pressed)
{
	cClickableWidget::setPressed (pressed);
	renewSurface();
}

//------------------------------------------------------------------------------
void cPushButton::setText (const std::string& newText)
{
	text = newText;
}

//------------------------------------------------------------------------------
void cPushButton::lock()
{
	isLocked = true;
	renewSurface();
}

//------------------------------------------------------------------------------
void cPushButton::unlock()
{
	isLocked = false;
	renewSurface();
}

//------------------------------------------------------------------------------
void cPushButton::renewSurface()
{
	if (buttonType == ePushButtonType::Invisible)
	{
		surface = nullptr;
		return;
	}

	cPosition size = getButtonSize (buttonType);
	SDL_Rect src;
	switch (buttonType)
	{
		default:
		case ePushButtonType::StandardBig:
			src.x = 0;
			src.y = (isPressed || isLocked) ? 29 : 0;
			break;
		case ePushButtonType::StandardSmall:
			src.x = 0;
			src.y = (isPressed || isLocked) ? 87 : 58;
			break;
		case ePushButtonType::Huge:
			src.x = (isPressed || isLocked) ? 109 : 0;
			src.y = 116;
			break;
		case ePushButtonType::ArrowUpBig:
			src.x = (isPressed || isLocked) ? 125 : 97;
			src.y = 157;
			break;
		case ePushButtonType::ArrowDownBig:
			src.x = (isPressed || isLocked) ? 181 : 153;
			src.y = 157;
			break;
		case ePushButtonType::ArrowLeftBig:
			src.x = (isPressed || isLocked) ? 293 : 265;
			src.y = 157;
			break;
		case ePushButtonType::ArrowRightBig:
			src.x = (isPressed || isLocked) ? 237 : 209;
			src.y = 157;
			break;
		case ePushButtonType::ArrowUpSmall:
			src.x = (isPressed || isLocked) ? 151 + size.x() : 151;
			src.y = 59;
			break;
		case ePushButtonType::ArrowDownSmall:
			src.x = (isPressed || isLocked) ? 187 + size.x() : 187;
			src.y = 59;
			break;
		case ePushButtonType::ArrowLeftSmall:
			src.x = (isPressed || isLocked) ? 151 + size.x() : 151;
			src.y = 76;
			break;
		case ePushButtonType::ArrowRightSmall:
			src.x = (isPressed || isLocked) ? 187 + size.x() : 187;
			src.y = 76;
			break;
		case ePushButtonType::ArrowUpBar:
			src.x = (isPressed || isLocked) ? 201 + size.x() : 201;
			src.y = 1;
			break;
		case ePushButtonType::ArrowDownBar:
			src.x = (isPressed || isLocked) ? 201 + size.x() : 201;
			src.y = 18;
			break;
		case ePushButtonType::Angular:
			src.x = (isPressed || isLocked) ? size.x() : 0;
			src.y = 196;
			break;
		case ePushButtonType::HudHelp:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudHelpPressed() : cGraphicsData::getRect_PushButton_HudHelp();
			break;
		case ePushButtonType::HudCenter:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudCenterPressed() : cGraphicsData::getRect_PushButton_HudCenter();
			break;
		case ePushButtonType::HudNext:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudNextPressed() : cGraphicsData::getRect_PushButton_HudNext();
			break;
		case ePushButtonType::HudPrev:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudPrevPressed() : cGraphicsData::getRect_PushButton_HudPrev();
			break;
		case ePushButtonType::HudDone:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudDonePressed() : cGraphicsData::getRect_PushButton_HudDone();
			break;
		case ePushButtonType::HudReport:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudReportPressed() : cGraphicsData::getRect_PushButton_HudReport();
			break;
		case ePushButtonType::HudChat:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudChatPressed() : cGraphicsData::getRect_PushButton_HudChat();
			break;
		case ePushButtonType::HudPreferences:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudPreferencesPressed() : cGraphicsData::getRect_PushButton_HudPreferences();
			break;
		case ePushButtonType::HudFiles:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudFilesPressed() : cGraphicsData::getRect_PushButton_HudFiles();
			break;
		case ePushButtonType::HudEnd:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudEndPressed() : cGraphicsData::getRect_PushButton_HudEnd();
			break;
		case ePushButtonType::HudPlay:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudPlayPressed() : cGraphicsData::getRect_PushButton_HudPlay();
			break;
		case ePushButtonType::HudStop:
			src = (isPressed || isLocked) ? cGraphicsData::getRect_PushButton_HudStopPressed() : cGraphicsData::getRect_PushButton_HudStop();
			break;
		case ePushButtonType::UnitContextMenu:
			src.x = 0;
			src.y = (isPressed || isLocked) ? 21 : 0;
			break;
		case ePushButtonType::Destroy:
			src.x = (isPressed || isLocked) ? 6 : 15;
			src.y = (isPressed || isLocked) ? 269 : 13;
			break;
		case ePushButtonType::ArrowUpSmallModern:
			src.x = 224;
			src.y = (isPressed || isLocked) ? 75 : 83;
			break;
		case ePushButtonType::ArrowDownSmallModern:
			src.x = 224;
			src.y = (isPressed || isLocked) ? 59 : 67;
			break;
		case ePushButtonType::ArrowLeftSmallModern:
			src.x = (isPressed || isLocked) ? 272 : 264;
			src.y = 59;
			break;
		case ePushButtonType::ArrowRightSmallModern:
			src.x = (isPressed || isLocked) ? 256 : 248;
			src.y = 59;
			break;
	}
	resize (size);

	src.w = size.x();
	src.h = size.y();

	surface = UniqueSurface (SDL_CreateRGBSurface (0, src.w, src.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

	SDL_Surface* srcSurface = nullptr;

	if (buttonType >= ePushButtonType::HudHelp && buttonType <= ePushButtonType::HudStop)
		srcSurface = GraphicsData.gfx_hud_stuff.get();
	else if (buttonType == ePushButtonType::UnitContextMenu)
		srcSurface = GraphicsData.gfx_context_menu.get();
	else if (buttonType == ePushButtonType::Destroy)
		srcSurface = (isPressed || isLocked) ? GraphicsData.gfx_hud_stuff.get() : GraphicsData.gfx_destruction.get();
	else
		srcSurface = GraphicsData.gfx_menu_stuff.get();

	assert (srcSurface != nullptr);

	SDL_BlitSurface (srcSurface, &src, surface.get(), nullptr);

	text = cUnicodeFont::font->shortenStringToSize (text, size.x() - getBordersSize(), fontType);
}

//------------------------------------------------------------------------------
int cPushButton::getTextYOffset() const
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
			if (isPressed || isLocked)
				return 5;
			else
				return 4;
		case ePushButtonType::HudNext:
		case ePushButtonType::HudPrev:
		case ePushButtonType::HudDone:
			return 9;
		case ePushButtonType::HudEnd:
			if (isPressed || isLocked)
				return 3;
			else
				return 2;
		case ePushButtonType::HudReport:
		case ePushButtonType::HudChat:
		case ePushButtonType::HudPreferences:
		case ePushButtonType::HudFiles:
			return 6;
		case ePushButtonType::UnitContextMenu:
			return 7;
	}
}

//------------------------------------------------------------------------------
int cPushButton::getBordersSize() const
{
	switch (buttonType)
	{
		case ePushButtonType::UnitContextMenu:
		case ePushButtonType::HudDone:
			return 0;
		default:
			return 12;
	}
}
