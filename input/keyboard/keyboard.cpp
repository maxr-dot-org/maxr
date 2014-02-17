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

#include <cassert>

#include "keyboard.h"

#include "../../events/eventmanager.h"
#include "../../events/keyboardevents.h"

//------------------------------------------------------------------------------
cKeyboard::cKeyboard()
{
	using namespace std::placeholders;

	signalConnectionManager.connect(cEventManager::getInstance().keyboardEvent, std::bind(&cKeyboard::handleKeyboardEvent, this, _1));
	signalConnectionManager.connect(cEventManager::getInstance().textInputEvent, std::bind(&cKeyboard::handleTextInputEvent, this, _1));
}

//------------------------------------------------------------------------------
cKeyboard& cKeyboard::getInstance()
{
	static cKeyboard instance;
	return instance;
}

//------------------------------------------------------------------------------
KeyModifierFlags cKeyboard::getCurrentModifiers() const
{
	return currentModifiers;
}

//------------------------------------------------------------------------------
bool cKeyboard::isAnyModifierActive(KeyModifierFlags flags) const
{
	return currentModifiers & flags;
}

//------------------------------------------------------------------------------
bool cKeyboard::isAllModifiersActive(KeyModifierFlags flags) const
{
	return (currentModifiers & flags) == flags;
}

//------------------------------------------------------------------------------
void cKeyboard::handleKeyboardEvent(const cKeyboardEvent& event)
{
	assert(event.getType() == cKeyboardEvent::Down || event.getType() == cKeyboardEvent::Up);

	currentModifiers = event.getModifiers();

	if(event.getType() == cKeyboardEvent::Down)
	{
		keyPressed(*this, event.getKey());
	}
	else if(event.getType() == cKeyboardEvent::Up)
	{
		keyReleased(*this, event.getKey());
	}
}

//------------------------------------------------------------------------------
void cKeyboard::handleTextInputEvent(const cTextInputEvent& event)
{
	textEntered(*this, event.getText());
}
