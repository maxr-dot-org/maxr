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

#include "keyboardevents.h"

//------------------------------------------------------------------------------
cKeyboardEvent::cKeyboardEvent(const SDL_KeyboardEvent& sdlEvent_) :
	sdlEvent(sdlEvent_)
{}

//------------------------------------------------------------------------------
SDL_Keycode cKeyboardEvent::getKey() const
{
	return sdlEvent.keysym.sym;
}

//------------------------------------------------------------------------------
cKeyboardEvent::eType cKeyboardEvent::getType() const
{
	return sdlEvent.state == SDL_PRESSED ? Down : Up;
}

//------------------------------------------------------------------------------
KeyModifierFlags cKeyboardEvent::getModifiers() const
{
	KeyModifierFlags result;

	if(sdlEvent.keysym.mod & KMOD_LSHIFT) result |= eKeyModifierType::ShiftLeft;
	if(sdlEvent.keysym.mod & KMOD_RSHIFT) result |= eKeyModifierType::ShiftRight;
	if(sdlEvent.keysym.mod & KMOD_LCTRL) result |= eKeyModifierType::CtrlLeft;
	if(sdlEvent.keysym.mod & KMOD_RCTRL) result |= eKeyModifierType::CtrlRight;
	if(sdlEvent.keysym.mod & KMOD_LALT) result |= eKeyModifierType::AltLeft;
	if(sdlEvent.keysym.mod & KMOD_RALT) result |= eKeyModifierType::AltRight;
	if(sdlEvent.keysym.mod & KMOD_LGUI) result |= eKeyModifierType::GuiLeft;
	if(sdlEvent.keysym.mod & KMOD_RGUI) result |= eKeyModifierType::GuiRight;
	if(sdlEvent.keysym.mod & KMOD_NUM) result |= eKeyModifierType::Num;
	if(sdlEvent.keysym.mod & KMOD_CAPS) result |= eKeyModifierType::Caps;
	if(sdlEvent.keysym.mod & KMOD_MODE) result |= eKeyModifierType::Mode;

	return result;
}

//------------------------------------------------------------------------------
cTextInputEvent::cTextInputEvent(const SDL_TextInputEvent& sdlEvent_) :
	sdlEvent(sdlEvent_)
{}

//------------------------------------------------------------------------------
const char* cTextInputEvent::getText() const
{
	return sdlEvent.text;
}
