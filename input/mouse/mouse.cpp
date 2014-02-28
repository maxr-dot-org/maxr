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

#include <algorithm>

#include "mouse.h"

#include "../../main.h"
#include "../../events/eventmanager.h"
#include "../../events/mouseevents.h"

//------------------------------------------------------------------------------
cMouse::cMouse() :
	sdlCursor(nullptr, SDL_FreeCursor),
	doubleClickTime(500)
{
	setCursorType(eMouseCursorType::Hand, true);

	using namespace std::placeholders;

	signalConnectionManager.connect(cEventManager::getInstance().mouseMotionEvent, std::bind(&cMouse::handleMouseMotionEvent, this, _1));
	signalConnectionManager.connect(cEventManager::getInstance().mouseButtonEvent, std::bind(&cMouse::handleMouseButtonEvent, this, _1));
	signalConnectionManager.connect(cEventManager::getInstance().mouseWheelEvent, std::bind(&cMouse::handleMouseWheelEvent, this, _1));
}

//------------------------------------------------------------------------------
cMouse& cMouse::getInstance()
{
	static cMouse instance;
	return instance;
}

//------------------------------------------------------------------------------
void cMouse::handleMouseMotionEvent(const cEventMouseMotion& event)
{
	position = event.getNewPosition();
	moved(*this, event.getOffset());
}

//------------------------------------------------------------------------------
void cMouse::handleMouseButtonEvent(const cEventMouseButton& event)
{
	assert(event.getType() == cEventMouseButton::Down || event.getType() == cEventMouseButton::Up);

	auto button = event.getButton();

	if(event.getType() == cEventMouseButton::Down)
	{
		buttonPressedState[button] = true;

		const auto currentClickTime = std::chrono::steady_clock::now();
		auto& lastClickTime = getLastClickTime(button);
		buttonClickCount[button] = (currentClickTime - lastClickTime) <= doubleClickTime ? getButtonClickCount(button) + 1 : 1;
		lastClickTime = currentClickTime;

		pressed(*this, button);
	}
	else if(event.getType() == cEventMouseButton::Up)
	{
		buttonPressedState[button] = false;
		released(*this, button);
	}
}

//------------------------------------------------------------------------------
void cMouse::handleMouseWheelEvent(const cEventMouseWheel& event)
{
	wheelMoved(*this, event.getAmount());
}

//------------------------------------------------------------------------------
const cPosition& cMouse::getPosition() const
{
	return position;
}

//------------------------------------------------------------------------------
bool cMouse::setCursorType(eMouseCursorType type, bool force)
{
	if(!force && cursorType == type) return false;

	cursorType = type;

	auto cursorSurface = getCursorSurface(type);
	auto hotPoint = getCursorHotPoint(type);

	auto newSdlCursor = SdlCursorPtrType(SDL_CreateColorCursor(cursorSurface, hotPoint.x(), hotPoint.y()), SDL_FreeCursor);

	SDL_SetCursor(newSdlCursor.get());

	sdlCursor = std::move(newSdlCursor);

	return true;
}

//------------------------------------------------------------------------------
void cMouse::show()
{
	SDL_ShowCursor(true);
}

//------------------------------------------------------------------------------
void cMouse::hide()
{
	SDL_ShowCursor(false);
}

//------------------------------------------------------------------------------
eMouseCursorType cMouse::getCursorType() const
{
	return cursorType;
}

//------------------------------------------------------------------------------
bool cMouse::isButtonPressed(eMouseButtonType button) const
{
	auto iter = buttonPressedState.find(button);
	if(iter == buttonPressedState.end())
	{
		return buttonPressedState[button] = false; // initialize as unpressed
	}
	else return iter->second;
}

//------------------------------------------------------------------------------
unsigned int cMouse::getButtonClickCount(eMouseButtonType button) const
{
	auto iter = buttonClickCount.find(button);
	if(iter == buttonClickCount.end())
	{
		return buttonClickCount[button] = 0; // initialize with zero
	}
	else return iter->second;
}

//------------------------------------------------------------------------------
std::chrono::steady_clock::time_point& cMouse::getLastClickTime(eMouseButtonType button)
{
	auto iter = lastClickTime.find(button);
	if(iter == lastClickTime.end())
	{
		return lastClickTime[button]; // use default initialization. Is this really correct?!
	}
	else return iter->second;
}

//------------------------------------------------------------------------------
SDL_Surface* cMouse::getCursorSurface(eMouseCursorType type)
{
	switch(type)
	{
	case eMouseCursorType::Hand:
		return GraphicsData.gfx_Chand;
	case eMouseCursorType::No:
		return GraphicsData.gfx_Cno;
	case eMouseCursorType::Select:
		return GraphicsData.gfx_Cselect;
	case eMouseCursorType::Move:
		return GraphicsData.gfx_Cmove;
	case eMouseCursorType::ArrowLeftDown:
		return GraphicsData.gfx_Cpfeil1;
	case eMouseCursorType::ArrowDown:
		return GraphicsData.gfx_Cpfeil2;
	case eMouseCursorType::ArrowRightDown:
		return GraphicsData.gfx_Cpfeil3;
	case eMouseCursorType::ArrowLeft:
		return GraphicsData.gfx_Cpfeil4;
	case eMouseCursorType::ArrowRight:
		return GraphicsData.gfx_Cpfeil6;
	case eMouseCursorType::ArrowLeftUp:
		return GraphicsData.gfx_Cpfeil7;
	case eMouseCursorType::ArrowUp:
		return GraphicsData.gfx_Cpfeil8;
	case eMouseCursorType::ArrowRightUp:
		return GraphicsData.gfx_Cpfeil9;
	case eMouseCursorType::Help:
		return GraphicsData.gfx_Chelp;
	case eMouseCursorType::Attack:
		return GraphicsData.gfx_Cattack;
	case eMouseCursorType::Band:
		return GraphicsData.gfx_Cband;
	case eMouseCursorType::Transfer:
		return GraphicsData.gfx_Ctransf;
	case eMouseCursorType::Load:
		return GraphicsData.gfx_Cload;
	case eMouseCursorType::Muni:
		return GraphicsData.gfx_Cmuni;
	case eMouseCursorType::Repair:
		return GraphicsData.gfx_Crepair;
	case eMouseCursorType::Steal:
		return GraphicsData.gfx_Csteal;
	case eMouseCursorType::Disable:
		return GraphicsData.gfx_Cdisable;
	case eMouseCursorType::Activate:
		return GraphicsData.gfx_Cactivate;
	case eMouseCursorType::MoveDraft:
		return GraphicsData.gfx_Cmove_draft;
	case eMouseCursorType::AttacOOR:
		return GraphicsData.gfx_Cattackoor;
	default:
		assert(false);
	}
	return GraphicsData.gfx_Chand;
}

//------------------------------------------------------------------------------
cPosition cMouse::getCursorHotPoint(eMouseCursorType type)
{
	switch(type)
	{
	case eMouseCursorType::Select:
	case eMouseCursorType::Help:
	case eMouseCursorType::Move:
	case eMouseCursorType::MoveDraft:
	case eMouseCursorType::No:
	case eMouseCursorType::Transfer:
	case eMouseCursorType::Band:
	case eMouseCursorType::Load:
	case eMouseCursorType::Muni:
	case eMouseCursorType::Repair:
	case eMouseCursorType::Activate:
		return cPosition(12, 12);
	case eMouseCursorType::Attack:
	case eMouseCursorType::Steal:
	case eMouseCursorType::Disable:
	case eMouseCursorType::AttacOOR:
		return cPosition(19, 16);
		break;
	default:
		return cPosition(0, 0);
		break;
	}
	return cPosition(0, 0);
}
