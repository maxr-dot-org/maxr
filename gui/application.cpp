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

#include "application.h"

#include "../input/mouse/mouse.h"
#include "../input/keyboard/keyboard.h"
#include "../events/eventmanager.h"
#include "../settings.h"
#include "../video.h"
#include "../main.h"
#include "../unifonts.h"
#include "../netmessage.h"
#include "../game/game.h"

#include "widget.h"
#include "window.h"

//------------------------------------------------------------------------------
cApplication::cApplication () :
	keyFocusWidget (nullptr),
	mouseFocusWidget (nullptr),
	activeMouse (nullptr),
	activeKeyboard (nullptr)
	//underMouseWidget (nullptr)
{}

//------------------------------------------------------------------------------
cApplication::~cApplication ()
{}

// TODO: find nice place for this class
class cFrameCounter
{
public:
	cFrameCounter () :
		frames (0),
		lastFrames (0),
		lastCheckTime (),
		framesPerSecond (0)
	{}

	void frameDrawn ()
	{
		++frames;
	}

	unsigned int getFramesPerSecond ()
	{
		const auto now = std::chrono::steady_clock::now ();

		const auto timeSinceLastCheck = now - lastCheckTime;

		if (timeSinceLastCheck > std::chrono::seconds (1))
		{
			double passedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(timeSinceLastCheck).count ();
			
			const auto framesSinceLastCheck = frames - lastFrames;

			framesPerSecond = static_cast<unsigned int>(std::round((double)framesSinceLastCheck / passedSeconds));

			lastFrames = frames;
			lastCheckTime = now;
		}

		return framesPerSecond;
	}
private:
	unsigned long long frames;
	unsigned long long lastFrames;

	unsigned int framesPerSecond;

	std::chrono::steady_clock::time_point lastCheckTime;
};

//------------------------------------------------------------------------------
void cApplication::execute ()
{
	cFrameCounter frameCounter;
	cWindow* lastActiveWindow = nullptr;
	while (!modalWindows.empty())
	{
		cEventManager::getInstance ().run ();

		if (game) game->run ();

		std::unique_ptr<cNetMessage> message;
		while (messageQueue.try_pop (message))
		{
			handleNetMessage (*message);
		}

		const auto activeWindow = getActiveWindow ();

		if (activeWindow)
		{
			if (activeWindow != lastActiveWindow)
			{
				if (lastActiveWindow != nullptr) lastActiveWindow->handleDeactivated (*this);
				activeWindow->handleActivated (*this);
			}

			if (activeWindow->isClosing ())
			{
				auto activeWindowOwned = modalWindows.back ();
				modalWindows.pop_back ();
				activeWindowOwned->handleDeactivated (*this);
				activeWindowOwned->handleRemoved (*this);
				lastActiveWindow = nullptr;
			}
			else
			{
				// TODO: long term task: remove this.
				//       Instead: - redraw only if necessary
				//                - do actions only if necessary.
				//                - use non-busy waiting if there is nothing to be done
				if (!cSettings::getInstance ().shouldUseFastMode ()) SDL_Delay (1);

				activeWindow->draw ();
				lastActiveWindow = activeWindow;

				SDL_Rect dest = {0, 0, 55, 10};
				SDL_FillRect (cVideo::buffer, &dest, 0);
				font->showText (0, 0, "FPS: " + iToStr (frameCounter.getFramesPerSecond ()));

				Video.draw ();
				frameCounter.frameDrawn ();
			}
		}
	}
}

//------------------------------------------------------------------------------
void cApplication::center (cWindow& window)
{
	cPosition position;
	position.x () = (Video.getResolutionX () / 2 - window.getSize ().x () / 2);
	position.y () = (Video.getResolutionY () / 2 - window.getSize ().y () / 2);
	window.moveTo (position);
}

//------------------------------------------------------------------------------
void cApplication::grapMouseFocus (cWidget& widget)
{
	if (mouseFocusWidget != &widget)
	{
		if (mouseFocusWidget != nullptr) mouseFocusWidget->handleLooseMouseFocus (*this);
		widget.handleGetMouseFocus (*this);
		mouseFocusWidget = &widget;
	}
}

//------------------------------------------------------------------------------
void cApplication::releaseMouseFocus (const cWidget& widget)
{
	if (mouseFocusWidget == &widget)
	{
		mouseFocusWidget->handleLooseMouseFocus (*this);
		mouseFocusWidget = nullptr;
	}
}

//------------------------------------------------------------------------------
bool cApplication::hasMouseFocus (const cWidget& widget) const
{
	return mouseFocusWidget == &widget;
}

//------------------------------------------------------------------------------
void cApplication::grapKeyFocus (cWidget& widget)
{
	assignKeyFocus (&widget);
}

//------------------------------------------------------------------------------
void cApplication::releaseKeyFocus (const cWidget& widget)
{
	if (keyFocusWidget == &widget)
	{
		assignKeyFocus (nullptr);
	}
}

//------------------------------------------------------------------------------
cMouse* cApplication::getActiveMouse ()
{
	return activeMouse;
}

//------------------------------------------------------------------------------
cKeyboard* cApplication::getActiveKeyboard ()
{
	return activeKeyboard;
}

//------------------------------------------------------------------------------
void cApplication::setGame (std::shared_ptr<cGame> game_)
{
	game = game_;
}

//------------------------------------------------------------------------------
const std::shared_ptr<cGame>& cApplication::getGame () const
{
	return game;
}

//------------------------------------------------------------------------------
cWindow* cApplication::getActiveWindow ()
{
	// remove null widgets on the top if there are any
	while (!modalWindows.empty () && modalWindows.back () == nullptr) modalWindows.pop_back ();

	return modalWindows.empty () ? nullptr : modalWindows.back ().get ();
}

//------------------------------------------------------------------------------
cWidget* cApplication::getKeyFocusWidget () const
{
	return keyFocusWidget;
}

//------------------------------------------------------------------------------
cWidget* cApplication::getMouseEventFirstTarget (const cPosition& position)
{
	auto window = getActiveWindow ();
	if (!window) return nullptr;

	auto child = window->getChildAt (position);

	if (child) return child;

	if (window->isAt (position)) return window;

	return nullptr;
}

//------------------------------------------------------------------------------
void cApplication::registerMouse (cMouse& mouse)
{
	using namespace std::placeholders;

	signalConnectionManager.connect (mouse.pressed, std::bind (&cApplication::mousePressed, this, _1, _2));
	signalConnectionManager.connect (mouse.released, std::bind (&cApplication::mouseReleased, this, _1, _2));
	signalConnectionManager.connect (mouse.wheelMoved, std::bind (&cApplication::mouseWheelMoved, this, _1, _2));
	signalConnectionManager.connect (mouse.moved, std::bind (&cApplication::mouseMoved, this, _1, _2));

	if (activeMouse == nullptr) activeMouse = &mouse;
}

//------------------------------------------------------------------------------
void cApplication::registerKeyboard (cKeyboard& keyboard)
{
	using namespace std::placeholders;

	signalConnectionManager.connect (keyboard.keyPressed, std::bind (&cApplication::keyPressed, this, _1, _2));
	signalConnectionManager.connect (keyboard.keyReleased, std::bind (&cApplication::keyReleased, this, _1, _2));
	signalConnectionManager.connect (keyboard.textEntered, std::bind (&cApplication::textEntered, this, _1, _2));

	if (activeKeyboard == nullptr) activeKeyboard = &keyboard;
}

//------------------------------------------------------------------------------
void cApplication::mousePressed (cMouse& mouse, eMouseButtonType button)
{
	auto target = getMouseEventFirstTarget (mouse.getPosition ());
	assignKeyFocus (target);

	if (mouseFocusWidget && mouseFocusWidget->handleMousePressed (*this, mouse, button)) return;

	while (target && !target->handleMousePressed (*this, mouse, button))
	{
		target = target->getParent ();
	}
}

//------------------------------------------------------------------------------
void cApplication::mouseReleased (cMouse& mouse, eMouseButtonType button)
{
	auto target = getMouseEventFirstTarget (mouse.getPosition ());
	assignKeyFocus (target);

	if (mouseFocusWidget && mouseFocusWidget->handleMouseReleased (*this, mouse, button)) return;

	while (target && !target->handleMouseReleased (*this, mouse, button))
	{
		target = target->getParent ();
	}
}

//------------------------------------------------------------------------------
void cApplication::mouseWheelMoved (cMouse& mouse, const cPosition& amount)
{
	if (mouseFocusWidget && mouseFocusWidget->handleMouseWheelMoved (*this, mouse, amount)) return;

	auto target = getMouseEventFirstTarget (mouse.getPosition ());
	while (target && !target->handleMouseWheelMoved (*this, mouse, amount))
	{
		target = target->getParent ();
	}
}

//------------------------------------------------------------------------------
void cApplication::mouseMoved (cMouse& mouse, const cPosition& offset)
{
	auto target = getMouseEventFirstTarget (mouse.getPosition ());

	if (mouseFocusWidget && mouseFocusWidget->handleMouseMoved (*this, mouse, offset)) return;

	//if (underMouseWidget != target)
	//{
	//	if (underMouseWidget) underMouseWidget->handleHoveredAway (*this, mouse);
	//	if (target) target->handleHoveredOn (*this, mouse);
	//	underMouseWidget = target;
	//}

	while (target && !target->handleMouseMoved (*this, mouse, offset))
	{
		target = target->getParent ();
	}
}

//------------------------------------------------------------------------------
void cApplication::keyPressed (cKeyboard& keyboard, SDL_Keycode key)
{
	auto widget = getKeyFocusWidget ();
	if (!widget || !widget->handleKeyPressed (*this, keyboard, key))
	{
		auto window = getActiveWindow ();
		if (window)
		{
			window->handleKeyPressed (*this, keyboard, key);
		}
	}
}

//------------------------------------------------------------------------------
void cApplication::keyReleased (cKeyboard& keyboard, SDL_Keycode key)
{
	auto widget = getKeyFocusWidget ();

	// TODO: catch TAB event and may switch key focus widget

	if (!widget || !widget->handleKeyReleased (*this, keyboard, key))
	{
		auto window = getActiveWindow ();
		if (window)
		{
			window->handleKeyReleased (*this, keyboard, key);
		}
	}
}

//------------------------------------------------------------------------------
void cApplication::textEntered (cKeyboard& keyboard, const char* text)
{
	auto widget = getKeyFocusWidget ();
	if (widget) widget->handleTextEntered (*this, keyboard, text);
}

//------------------------------------------------------------------------------
void cApplication::assignKeyFocus (cWidget* widget)
{
	while (widget)
	{
		if (widget->isEnabled () && widget->handleGetKeyFocus (*this))
		{
			break;
		}
		widget = widget->getParent ();
	}
	if (keyFocusWidget && keyFocusWidget != widget) keyFocusWidget->handleLooseKeyFocus (*this);
	keyFocusWidget = widget;
}

//------------------------------------------------------------------------------
void cApplication::pushEvent (std::unique_ptr<cNetMessage> message)
{
	messageQueue.push (std::move (message));
}

//------------------------------------------------------------------------------
void cApplication::handleNetMessage (cNetMessage& message)
{
	for (auto iter = modalWindows.rbegin (); iter != modalWindows.rend (); ++iter)
	{
		if (*iter == nullptr) continue;

		if ((*iter)->handleNetMessage (message)) break;
	}
}