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

#include "events/eventmanager.h"
#include "input/keyboard/keyboard.h"
#include "input/mouse/mouse.h"
#include "output/video/unifonts.h"
#include "output/video/video.h"
#include "ui/widgets/framecounter.h"
#include "ui/widgets/widget.h"
#include "ui/widgets/window.h"
#include "utility/listhelpers.h"
#include "utility/runnable.h"

//------------------------------------------------------------------------------
cApplication::cApplication() :
	frameCounter (std::make_shared<cFrameCounter>())
{
	signalConnectionManager.connect (Video.resolutionChanged, [this]() {
		for (auto rit = modalWindows.rbegin(); rit != modalWindows.rend(); ++rit)
		{
			const auto& modalWindow = *rit;
			if (modalWindow->wantsCentered())
			{
				center (*modalWindow);
			}
		}
	});

	addShortcut (cKeySequence (cKeyCombination (toEnumFlag (eKeyModifierType::Ctrl) | eKeyModifierType::Alt, SDLK_f)), [this]() {
		shouldDrawFramesPerSecond = !shouldDrawFramesPerSecond;
	});

	addShortcut (cKeySequence (cKeyCombination (toEnumFlag (eKeyModifierType::Ctrl) | eKeyModifierType::Alt, SDLK_w)), [this]() {
		cWidget::toggleDrawDebugFrames();
	});
}

//------------------------------------------------------------------------------
cApplication::~cApplication()
{
	// Move it to temporary container.
	// It allows to break all shared_ptr cross-references broken before
	// cApplication is destroyed
	{
		auto runnables_tmp = std::move (runnables);
	}
}

//------------------------------------------------------------------------------
void cApplication::retranslate()
{
	for (auto& window : modalWindows)
	{
		window->retranslate();
	}
}

//------------------------------------------------------------------------------
void cApplication::execute()
{
	cWindow* lastActiveWindow = nullptr;
	bool lastClosed = false;

	cEventManager& eventManager = cEventManager::getInstance();

	while (!modalWindows.empty() && !eventManager.shouldExit())
	{
		eventManager.run();

		EraseIf (runnables, [] (const auto& runnable) { return runnable->wantsToTerminate(); });
		for (const auto& runnable : runnables)
		{
			runnable->run();
		}

		const auto activeWindow = getActiveWindow();

		if (activeWindow)
		{
			if (activeWindow != lastActiveWindow)
			{
				if (lastActiveWindow != nullptr) lastActiveWindow->handleDeactivated (*this, false);
				activeWindow->handleActivated (*this, !lastClosed);
				lastClosed = false;
			}

			if (activeWindow->isClosing())
			{
				auto activeWindowOwned = modalWindows.back();
				modalWindows.pop_back();
				activeWindowOwned->handleDeactivated (*this, true);
				lastActiveWindow = nullptr;
				lastClosed = true;
				keyFocusWidget = nullptr;
				mouseFocusWidget = nullptr;
			}
			else
			{
				// TODO: long term task: remove this.
				//       Instead: - redraw only if necessary
				//                - do actions only if necessary.
				//                - use non-busy waiting if there is nothing to be done
				if (!cSettings::getInstance().shouldUseFastMode()) SDL_Delay (1);

				activeWindow->draw (*cVideo::buffer, activeWindow->getArea());
				lastActiveWindow = activeWindow;

				if (shouldDrawFramesPerSecond) drawFramesPerSecond (frameCounter->getFramesPerSecond());

				Video.draw();
				frameCounter->frameDrawn();
			}
		}
	}
}

//------------------------------------------------------------------------------
void cApplication::closeTill (const cWindow& window)
{
	for (auto rit = modalWindows.rbegin(); rit != modalWindows.rend(); ++rit)
	{
		if (rit->get() == &window) break;

		(*rit)->close();
	}
}

//------------------------------------------------------------------------------
void cApplication::center (cWindow& window)
{
	cPosition position;
	position.x() = (Video.getResolutionX() / 2 - window.getSize().x() / 2);
	position.y() = (Video.getResolutionY() / 2 - window.getSize().y() / 2);
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
bool cApplication::hasMouseFocus() const
{
	return mouseFocusWidget != nullptr;
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
bool cApplication::hasKeyFocus (const cWidget& widget) const
{
	return keyFocusWidget == &widget;
}

//------------------------------------------------------------------------------
bool cApplication::hasKeyFocus() const
{
	return keyFocusWidget != nullptr;
}

//------------------------------------------------------------------------------
void cApplication::addRunnable (std::shared_ptr<cRunnable> runnable)
{
	runnables.push_back (std::move (runnable));
}

//------------------------------------------------------------------------------
std::shared_ptr<cRunnable> cApplication::removeRunnable (std::shared_ptr<cRunnable> runnable)
{
	const auto it = ranges::find (runnables, runnable);

	if (it != runnables.end())
	{
		auto result = std::move (*it);
		runnables.erase (it);
		return result;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cWindow* cApplication::getActiveWindow()
{
	// remove null widgets on the top if there are any
	while (!modalWindows.empty() && modalWindows.back() == nullptr)
		modalWindows.pop_back();

	return modalWindows.empty() ? nullptr : modalWindows.back().get();
}

//------------------------------------------------------------------------------
cWidget* cApplication::getMouseEventFirstTarget (const cPosition& position)
{
	auto window = getActiveWindow();
	if (!window) return nullptr;

	auto child = window->getChildAt (position);

	if (child) return child;

	if (window->isAt (position)) return window;

	return nullptr;
}

//------------------------------------------------------------------------------
void cApplication::registerMouse (cMouse& mouse)
{
	signalConnectionManager.connect (mouse.pressed, [this] (cMouse& mouse, eMouseButtonType button) { mousePressed (mouse, button); });
	signalConnectionManager.connect (mouse.released, [this] (cMouse& mouse, eMouseButtonType button) { mouseReleased (mouse, button); });
	signalConnectionManager.connect (mouse.wheelMoved, [this] (cMouse& mouse, const cPosition& amount) { mouseWheelMoved (mouse, amount); });
	signalConnectionManager.connect (mouse.moved, [this] (cMouse& mouse, const cPosition& offset) { mouseMoved (mouse, offset); });

	if (activeMouse == nullptr) activeMouse = &mouse;
}

//------------------------------------------------------------------------------
void cApplication::registerKeyboard (cKeyboard& keyboard)
{
	signalConnectionManager.connect (keyboard.keyPressed, [this] (cKeyboard& keyboard, SDL_Keycode key) { keyPressed (keyboard, key); });
	signalConnectionManager.connect (keyboard.keyReleased, [this] (cKeyboard& keyboard, SDL_Keycode key) { keyReleased (keyboard, key); });
	signalConnectionManager.connect (keyboard.textEntered, [this] (cKeyboard& keyboard, const char* text) { textEntered (keyboard, text); });

	if (activeKeyboard == nullptr) activeKeyboard = &keyboard;
}

//------------------------------------------------------------------------------
void cApplication::mousePressed (cMouse& mouse, eMouseButtonType button)
{
	auto target = getMouseEventFirstTarget (mouse.getPosition());
	assignKeyFocus (target);

	if (mouseFocusWidget && mouseFocusWidget->handleMousePressed (*this, mouse, button)) return;

	while (target && !target->handleMousePressed (*this, mouse, button))
	{
		target = target->getParent();
	}
}

//------------------------------------------------------------------------------
void cApplication::mouseReleased (cMouse& mouse, eMouseButtonType button)
{
	auto target = getMouseEventFirstTarget (mouse.getPosition());
	assignKeyFocus (target);

	if (mouseFocusWidget && mouseFocusWidget->handleMouseReleased (*this, mouse, button)) return;

	while (target && !target->handleMouseReleased (*this, mouse, button))
	{
		target = target->getParent();
	}
}

//------------------------------------------------------------------------------
void cApplication::mouseWheelMoved (cMouse& mouse, const cPosition& amount)
{
	if (mouseFocusWidget && mouseFocusWidget->handleMouseWheelMoved (*this, mouse, amount)) return;

	auto target = getMouseEventFirstTarget (mouse.getPosition());
	while (target && !target->handleMouseWheelMoved (*this, mouse, amount))
	{
		target = target->getParent();
	}
}

//------------------------------------------------------------------------------
void cApplication::mouseMoved (cMouse& mouse, const cPosition& offset)
{
	auto target = getMouseEventFirstTarget (mouse.getPosition());

	if (mouseFocusWidget && mouseFocusWidget->handleMouseMoved (*this, mouse, offset)) return;

	//if (underMouseWidget != target)
	//{
	//	if (underMouseWidget) underMouseWidget->handleHoveredAway (*this, mouse);
	//	if (target) target->handleHoveredOn (*this, mouse);
	//	underMouseWidget = target;
	//}

	while (target && !target->handleMouseMoved (*this, mouse, offset))
	{
		target = target->getParent();
	}
}

//------------------------------------------------------------------------------
void cApplication::keyPressed (cKeyboard& keyboard, SDL_Keycode key)
{
	const auto widget = getKeyFocusWidget();

	// TODO: catch TAB event and may switch key focus widget

	const bool isShortcutKey = cKeyCombination::isRepresentableKey (key);

	bool eventHandled = false;
	if (isShortcutKey)
	{
		currentKeySequence.addKeyCombination (cKeyCombination (keyboard.getCurrentModifiers(), key));

		eventHandled = hitShortcuts (currentKeySequence);
	}

	if (widget && !eventHandled)
	{
		eventHandled = widget->handleKeyPressed (*this, keyboard, key);
	}

	const auto window = getActiveWindow();
	if (window)
	{
		if (!eventHandled)
		{
			eventHandled = window->handleKeyPressed (*this, keyboard, key);
		}

		if (isShortcutKey && !eventHandled)
		{
			window->hitShortcuts (currentKeySequence);
		}
	}

	if (isShortcutKey)
	{
		while (currentKeySequence.length() >= maximalShortcutSequenceLength)
		{
			currentKeySequence.removeFirst();
		}
	}
}

//------------------------------------------------------------------------------
void cApplication::keyReleased (cKeyboard& keyboard, SDL_Keycode key)
{
	auto widget = getKeyFocusWidget();
	if (!widget || !widget->handleKeyReleased (*this, keyboard, key))
	{
		auto window = getActiveWindow();
		if (window)
		{
			window->handleKeyReleased (*this, keyboard, key);
		}
	}
}

//------------------------------------------------------------------------------
void cApplication::textEntered (cKeyboard& keyboard, const char* text)
{
	auto widget = getKeyFocusWidget();
	if (widget) widget->handleTextEntered (*this, keyboard, text);
}

//------------------------------------------------------------------------------
void cApplication::assignKeyFocus (cWidget* widget)
{
	while (widget)
	{
		if (widget->isEnabled() && widget->handleGetKeyFocus (*this))
		{
			break;
		}
		widget = widget->getParent();
	}
	if (keyFocusWidget && keyFocusWidget != widget) keyFocusWidget->handleLooseKeyFocus (*this);
	keyFocusWidget = widget;
}

//------------------------------------------------------------------------------
template <typename Action>
void cApplication::addShortcut (cKeySequence key, Action action)
{
	auto shortcut = std::make_unique<cShortcut> (key);
	signalConnectionManager.connect (shortcut->triggered, action);
	shortcuts.push_back (std::move (shortcut));
}

//------------------------------------------------------------------------------
bool cApplication::hitShortcuts (const cKeySequence& keySequence)
{
	bool anyMatch = false;

	for (const auto& shortcut : shortcuts)
	{
		anyMatch |= shortcut->hit (keySequence);
	}
	return anyMatch;
}

//------------------------------------------------------------------------------
void cApplication::drawFramesPerSecond (unsigned int fps)
{
	SDL_Rect dest = {0, 0, 55, 10};
	SDL_FillRect (cVideo::buffer, &dest, 0);
	cUnicodeFont::font->showText (0, 0, "FPS: " + std::to_string (fps));
}
