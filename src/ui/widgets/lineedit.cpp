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

#include "lineedit.h"

#include "SDLutility/tosdl.h"
#include "input/keyboard/keyboard.h"
#include "input/mouse/mouse.h"
#include "output/video/video.h"
#include "resources/keys.h"
#include "ui/widgets/application.h"
#include "ui/widgets/validators/validator.h"
#include "ui/widgets/window.h"
#include "utility/log.h"
#include "utility/string/utf-8.h"

#include <algorithm>
#include <cassert>

//------------------------------------------------------------------------------
cLineEdit::cLineEdit (const cBox<cPosition>& area, eLineEditFrameType frameType_, eUnicodeFontType fontType_) :
	cClickableWidget (area),
	cursorVisibleTime (800),
	cursorInvisibleTime (500),
	fontType (fontType_),
	frameType (frameType_)
{
	createBackground();
}

//------------------------------------------------------------------------------
cLineEdit::~cLineEdit()
{}

//------------------------------------------------------------------------------
const std::string& cLineEdit::getText()
{
	return text;
}

//------------------------------------------------------------------------------
void cLineEdit::setText (std::string text_)
{
	std::swap (text, text_);
	if (validator)
	{
		const auto state = validator->validate (text);
		if (state != eValidatorState::Valid)
		{
			validator->fixup (text);
		}
	}
	resetTextPosition();
	textEdited(text);
	textSet();
}

//------------------------------------------------------------------------------
void cLineEdit::setReadOnly (bool readOnly_)
{
	readOnly = readOnly_;
	setConsumeClick (!readOnly);
}

//------------------------------------------------------------------------------
void cLineEdit::setValidator (std::unique_ptr<cValidator> validator_)
{
	validator = std::move (validator_);
}

//------------------------------------------------------------------------------
void cLineEdit::finishEditing()
{
	auto application = getActiveApplication();
	if (application)
	{
		application->releaseKeyFocus (*this);
	}
	else
	{
		finishEditingInternal();
	}
}

//------------------------------------------------------------------------------
void cLineEdit::finishEditingInternal()
{
	SDL_StopTextInput();

	if (validator)
	{
		const auto state = validator->validate (text);
		if (state != eValidatorState::Valid)
		{
			validator->fixup (text);
			resetTextPosition();
		}
		editingFinished (state);
	}
	else
	{
		editingFinished (eValidatorState::Valid);
	}
}
//------------------------------------------------------------------------------
void cLineEdit::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		SDL_Rect position = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
	}

	const auto offsetRect = getTextDrawOffset();
	const cPosition textPosition = getPosition() + offsetRect;
	const auto cursorXOffset = cUnicodeFont::getFontSize (fontType) == eUnicodeFontSize::Small ? -1 : 0;

	auto font = cUnicodeFont::font.get();
	font->showText (textPosition.x(), textPosition.y(), text.substr (startOffset, endOffset - startOffset), fontType);
	if (hasKeyFocus && !readOnly)
	{
		const auto now = std::chrono::steady_clock::now();

		if (now - lastCursorBlinkTime > cursorVisibleTime)
		{
			showCursor = !showCursor;
			lastCursorBlinkTime = now;
		}

		if (showCursor) font->showText (textPosition.x() + cursorXOffset + font->getTextWide (text.substr (startOffset, cursorPos - startOffset), fontType), textPosition.y(), "|", fontType);
	}

	cClickableWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
bool cLineEdit::handleGetKeyFocus (cApplication& application)
{
	if (readOnly) return false;

	const auto hadKeyFocus = hasKeyFocus;

	hasKeyFocus = true;

	showCursor = true;
	lastCursorBlinkTime = std::chrono::steady_clock::now();

	if (!hadKeyFocus)
	{
		auto font = cUnicodeFont::font.get();
		cursorPos = (int) text.length();
		while (cursorPos > endOffset)
			doPosIncrease (endOffset, endOffset);
		while (font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
			doPosIncrease (startOffset, startOffset);
	}

	SDL_StartTextInput();

	return true;
}

//------------------------------------------------------------------------------
void cLineEdit::handleLooseKeyFocus (cApplication& application)
{
	if (hasKeyFocus)
	{
		hasKeyFocus = false;
		finishEditingInternal();
	}
}

//------------------------------------------------------------------------------
bool cLineEdit::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	clicked();

	if (readOnly) return false;

	auto font = cUnicodeFont::font.get();
	int x = mouse.getPosition().x() - (getPosition().x() + getTextDrawOffset().x());
	int cursor = startOffset;
	while (font->getTextWide (text.substr (startOffset, cursor - startOffset), fontType) < x)
	{
		doPosIncrease (cursor, cursor);
		if (cursor >= endOffset)
		{
			cursor = endOffset;
			break;
		}
	}
	cursorPos = cursor;

	return true;
}

//------------------------------------------------------------------------------
void cLineEdit::createBackground()
{
	if (frameType == eLineEditFrameType::Box)
	{
		//surface = SDL_CreateRGBSurface (0, getSize().x(), getSize().y(), Video.getColDepth(), 0, 0, 0, 0);

		//SDL_FillRect (surface, nullptr, 0xFF00FF);
		//SDL_SetColorKey (surface, SDL_TRUE, 0xFF00FF);
	}
	else
	{
		surface = nullptr;
	}
}

//------------------------------------------------------------------------------
cPosition cLineEdit::getTextDrawOffset() const
{
	switch (frameType)
	{
		default:
		case eLineEditFrameType::None:
			return cPosition (0, 0);
		case eLineEditFrameType::Box:
			return cPosition (6, 3);
	}
}

//------------------------------------------------------------------------------
int cLineEdit::getBorderSize() const
{
	switch (frameType)
	{
		default:
		case eLineEditFrameType::None:
			return 0;
		case eLineEditFrameType::Box:
			return 12;
	}
}

//------------------------------------------------------------------------------
void cLineEdit::resetTextPosition()
{
	startOffset = 0;
	endOffset = (int) text.length();
	cursorPos = endOffset;
	auto font = cUnicodeFont::font.get();

	while (endOffset > 0 && font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
		doPosDecrease (endOffset);
}

//------------------------------------------------------------------------------
void cLineEdit::doPosIncrease (int& value, int pos)
{
	std::size_t index = pos;
	utf8::increasePos (text, index);
	value += index - pos;
}

//------------------------------------------------------------------------------
void cLineEdit::doPosDecrease (int& pos)
{
	std::size_t index = pos;
	utf8::decreasePos (text, index);
	pos = index;
}

//------------------------------------------------------------------------------
void cLineEdit::scrollLeft (bool changeCursor)
{
	// makes the cursor go left
	if (changeCursor && cursorPos > 0) doPosDecrease (cursorPos);

	auto font = cUnicodeFont::font.get();

	if (cursorPos > 0)
		while (cursorPos - 1 < startOffset)
			doPosDecrease (startOffset);
	else
		while (cursorPos < startOffset)
			doPosDecrease (startOffset);

	if (font->getTextWide (text.substr (startOffset, text.length() - startOffset), fontType) > getSize().x() - getBorderSize())
	{
		endOffset = (int) text.length();
		while (endOffset > 0 && font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
			doPosDecrease (endOffset);
	}
}

//------------------------------------------------------------------------------
void cLineEdit::scrollRight()
{
	// makes the cursor go right
	if (cursorPos < (int) text.length()) doPosIncrease (cursorPos, cursorPos);
	assert (cursorPos <= (int) text.length());

	auto font = cUnicodeFont::font.get();
	while (cursorPos > endOffset)
		doPosIncrease (endOffset, endOffset);
	while (font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
		doPosIncrease (startOffset, startOffset);
}

//------------------------------------------------------------------------------
void cLineEdit::deleteLeft()
{
	// deletes the first character left from the cursor
	if (cursorPos > 0)
	{
		std::size_t index = cursorPos;
		utf8::decreasePos (text, index);
		text.erase (index, cursorPos - index);
		endOffset -= cursorPos - index;
		doPosIncrease (endOffset, endOffset);
		cursorPos = index;
		scrollLeft (false);
		textEdited (text);
	}
}

//------------------------------------------------------------------------------
void cLineEdit::deleteRight()
{
	// deletes the first character right from the cursor
	if (cursorPos < (int) text.length())
	{
		std::size_t index = cursorPos;
		utf8::increasePos (text, index);
		text.erase (cursorPos, index - cursorPos);
		endOffset -= index - cursorPos;
		doPosIncrease(endOffset, endOffset);
		textEdited (text);
	}
}

//------------------------------------------------------------------------------
bool cLineEdit::handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	if (readOnly || !hasKeyFocus) return false;

	auto font = cUnicodeFont::font.get();

	switch (key)
	{
		case SDLK_ESCAPE:
			escapePressed();
			application.releaseKeyFocus (*this);
			break;
		case SDLK_KP_ENTER: // fall through
		case SDLK_RETURN:
			returnPressed();
			break;
		case SDLK_LEFT:
			scrollLeft();
			break;
		case SDLK_RIGHT:
			scrollRight();
			break;
		case SDLK_HOME:
			cursorPos = 0;
			startOffset = 0;
			endOffset = (int) text.length();
			while (endOffset > 0 && font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
				doPosDecrease (endOffset);
			break;
		case SDLK_END:
			cursorPos = (int) text.length();
			startOffset = 0;
			endOffset = (int) text.length();
			while (font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
				doPosIncrease (startOffset, startOffset);
			break;
		case SDLK_BACKSPACE:
			deleteLeft();
			break;
		case SDLK_DELETE:
			deleteRight();
			break;
		case SDLK_c:
			if (keyboard.isAnyModifierActive (toEnumFlag (eKeyModifierType::Ctrl)))
			{
				SDL_SetClipboardText (text.c_str());
			}
			break;
		case SDLK_x:
			if (keyboard.isAnyModifierActive (toEnumFlag (eKeyModifierType::Ctrl)))
			{
				SDL_SetClipboardText (text.c_str());
				text.clear();
				resetTextPosition();
				textEdited (text);
			}
			break;
		case SDLK_v:
			if (keyboard.isAnyModifierActive (toEnumFlag (eKeyModifierType::Ctrl)) && SDL_HasClipboardText())
			{
				const auto clipboardText = SDL_GetClipboardText();

				if (clipboardText == nullptr) break;

				const auto clipboardFree = makeScopedOperation ([clipboardText]() { SDL_free (clipboardText); });

				std::string insertText (clipboardText);

				if (validator)
				{
					const auto state = validator->validate (insertText);
					if (state == eValidatorState::Invalid) break;
				}

				text.insert (cursorPos, insertText);

				cursorPos += insertText.size();

				endOffset += insertText.size();

				while (font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
					doPosIncrease (startOffset, startOffset);
				textEdited (text);
			}
			break;
		default: // normal characters are handled as textInput:
			break;
	}
	return true;
}

//------------------------------------------------------------------------------
void cLineEdit::handleTextEntered (cApplication& application, cKeyboard& keyboard, const char* inputText)
{
	if (readOnly || !hasKeyFocus) return;

	text.insert (cursorPos, inputText);

	auto font = cUnicodeFont::font.get();

	if (validator)
	{
		const auto state = validator->validate (text);
		if (state == eValidatorState::Invalid)
		{
			validator->fixup (text);
			resetTextPosition();
		}
	}
	if (cursorPos < (int) text.length())
	{
		doPosIncrease (endOffset, cursorPos);
		doPosIncrease (cursorPos, cursorPos);
	}
	if (cursorPos >= endOffset)
	{
		while (font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
			doPosIncrease (startOffset, startOffset);
	}
	else
	{
		if (font->getTextWide (text.substr (startOffset, endOffset - startOffset), fontType) > getSize().x() - getBorderSize())
			doPosDecrease (endOffset);
	}
	textEdited (text);
}
