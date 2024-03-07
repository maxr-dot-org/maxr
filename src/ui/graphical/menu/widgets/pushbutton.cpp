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
cPushButton::cPushButton (const cBox<cPosition>& area) :
	cClickableWidget (area),
	buttonType (ePushButtonType::Invisible),
	fontType (eUnicodeFontType::LatinBig),
	text (""),
	clickSound (&SoundData.SNDHudButton)
{
	if (buttonType >= ePushButtonType::HudNext && buttonType <= ePushButtonType::HudFiles)
		fontType = eUnicodeFontType::LatinSmallWhite;
	renewSurface();
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_) :
	cPushButton (position, buttonType_, &SoundData.SNDHudButton)
{
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, cSoundChunk* clickSound_) :
	cPushButton (position, buttonType_, clickSound_, "", eUnicodeFontType::LatinBig)
{
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, const std::string& text_, eUnicodeFontType fontType_) :
	cPushButton (position, buttonType_, &SoundData.SNDHudButton, text_, fontType_)
{
}

//------------------------------------------------------------------------------
cPushButton::cPushButton (const cPosition& position, ePushButtonType buttonType_, cSoundChunk* clickSound_, const std::string& text_, eUnicodeFontType fontType_) :
	cClickableWidget (position),
	buttonType (buttonType_),
	fontType (fontType_),
	text (text_),
	clickSound (clickSound_)
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
bool cPushButton::handleClicked (cApplication&, cMouse&, eMouseButtonType button) /* override */
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

	sPartialSurface partialSurface{};
	switch (buttonType)
	{
		default:
		case ePushButtonType::StandardBig:
			partialSurface = GraphicsData.get_PushButton_StandardBig (isPressed || isLocked);
			break;
		case ePushButtonType::StandardSmall:
			partialSurface = GraphicsData.get_PushButton_StandardSmall (isPressed || isLocked);
			break;
		case ePushButtonType::Huge:
			partialSurface = GraphicsData.get_PushButton_Huge (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowUpBig:
			partialSurface = GraphicsData.get_PushButton_ArrowUpBig (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowDownBig:
			partialSurface = GraphicsData.get_PushButton_ArrowDownBig (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowLeftBig:
			partialSurface = GraphicsData.get_PushButton_ArrowLeftBig (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowRightBig:
			partialSurface = GraphicsData.get_PushButton_ArrowRightBig (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowUpSmall:
			partialSurface = GraphicsData.get_PushButton_ArrowUpSmall (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowDownSmall:
			partialSurface = GraphicsData.get_PushButton_ArrowDownSmall (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowLeftSmall:
			partialSurface = GraphicsData.get_PushButton_ArrowLeftSmall (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowRightSmall:
			partialSurface = GraphicsData.get_PushButton_ArrowRightSmall (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowUpBar:
			partialSurface = GraphicsData.get_PushButton_ArrowUpBar (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowDownBar:
			partialSurface = GraphicsData.get_PushButton_ArrowDownBar (isPressed || isLocked);
			break;
		case ePushButtonType::Angular:
			partialSurface = GraphicsData.get_PushButton_Angular (isPressed || isLocked);
			break;
		case ePushButtonType::HudHelp:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudHelpPressed() : GraphicsData.get_PushButton_HudHelp();
			break;
		case ePushButtonType::HudCenter:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudCenterPressed() : GraphicsData.get_PushButton_HudCenter();
			break;
		case ePushButtonType::HudNext:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudNextPressed() : GraphicsData.get_PushButton_HudNext();
			break;
		case ePushButtonType::HudPrev:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudPrevPressed() : GraphicsData.get_PushButton_HudPrev();
			break;
		case ePushButtonType::HudDone:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudDonePressed() : GraphicsData.get_PushButton_HudDone();
			break;
		case ePushButtonType::HudReport:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudReportPressed() : GraphicsData.get_PushButton_HudReport();
			break;
		case ePushButtonType::HudChat:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudChatPressed() : GraphicsData.get_PushButton_HudChat();
			break;
		case ePushButtonType::HudPreferences:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudPreferencesPressed() : GraphicsData.get_PushButton_HudPreferences();
			break;
		case ePushButtonType::HudFiles:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudFilesPressed() : GraphicsData.get_PushButton_HudFiles();
			break;
		case ePushButtonType::HudEnd:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudEndPressed() : GraphicsData.get_PushButton_HudEnd();
			break;
		case ePushButtonType::HudPlay:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudPlayPressed() : GraphicsData.get_PushButton_HudPlay();
			break;
		case ePushButtonType::HudStop:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_HudStopPressed() : GraphicsData.get_PushButton_HudStop();
			break;
		case ePushButtonType::UnitContextMenu:
			partialSurface = GraphicsData.get_PushButton_UnitContextMenu (isPressed || isLocked);
			break;
		case ePushButtonType::Destroy:
			partialSurface = (isPressed || isLocked) ? GraphicsData.get_PushButton_DestroyPressed()
			                                         : GraphicsData.get_PushButton_Destroy();
			break;
		case ePushButtonType::ArrowUpSmallModern:
			partialSurface = GraphicsData.get_PushButton_ArrowUpSmallModern (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowDownSmallModern:
			partialSurface = GraphicsData.get_PushButton_ArrowDownSmallModern (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowLeftSmallModern:
			partialSurface = GraphicsData.get_PushButton_ArrowLeftSmallModern (isPressed || isLocked);
			break;
		case ePushButtonType::ArrowRightSmallModern:
			partialSurface = GraphicsData.get_PushButton_ArrowRightSmallModern (isPressed || isLocked);
			break;
	}
	resize ({partialSurface.rect.w, partialSurface.rect.h});

	surface = UniqueSurface (SDL_CreateRGBSurface (0, partialSurface.rect.w, partialSurface.rect.h, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

	assert (partialSurface.surface != nullptr);

	SDL_BlitSurface (partialSurface.surface, &partialSurface.rect, surface.get(), nullptr);

	if (shorten) {
		text = cUnicodeFont::font->shortenStringToSize (text, partialSurface.rect.w - getBordersSize(), fontType);
	}
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
