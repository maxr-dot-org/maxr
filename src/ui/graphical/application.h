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

#ifndef ui_graphical_applicationH
#define ui_graphical_applicationH

#include <SDL.h>

#include <vector>
#include <list>
#include <memory>

#include "input/keyboard/keysequence.h"
#include "utility/thread/concurrentqueue.h"
#include "utility/signal/signalconnectionmanager.h"
#include "input/mouse/mousebuttontype.h"

class cMouse;
class cKeyboard;
class cWidget;
class cWindow;
class cPosition;
class cRunnable;
class cShortcut;
class cFrameCounter;

class cApplication
{
public:
	cApplication();
	~cApplication();

	void execute();

	template<typename WindowType>
	WindowType* show (std::shared_ptr<WindowType> window);

	/**
	 * Marks all windows to be closed that are above the passed one
	 * on the window stack.
	 *
	 * If the passed window is not on the applications window stack
	 * all windows will be marked to be closed.
	 *
	 * @param window The reference window to search for.
	 *               This window will not be marked to be closed.
	 */
	void closeTill (const cWindow& window);

	void registerMouse (cMouse& mouse);
	void registerKeyboard (cKeyboard& keyboard);

	void grapMouseFocus (cWidget& widget);
	void releaseMouseFocus (const cWidget& widget);
	bool hasMouseFocus (const cWidget& widget) const;
	bool hasMouseFocus() const;

	void grapKeyFocus (cWidget& widget);
	void releaseKeyFocus (const cWidget& widget);
	bool hasKeyFocus (const cWidget& widget) const;
	bool hasKeyFocus() const;

	void addRunnable (std::shared_ptr<cRunnable> runnable);
	std::shared_ptr<cRunnable> removeRunnable (std::shared_ptr<cRunnable> runnable);

	cMouse* getActiveMouse();
	cKeyboard* getActiveKeyboard();

private:
	template <typename Action> void addShortcut (cKeySequence, Action);

	void center (cWindow& window);

	cWindow* getActiveWindow();

	cWidget* getMouseEventFirstTarget (const cPosition& position);

	cWidget* getKeyFocusWidget() const;

	void mousePressed (cMouse& mouse, eMouseButtonType button);
	void mouseReleased (cMouse& mouse, eMouseButtonType button);
	void mouseWheelMoved (cMouse& mouse, const cPosition& amount);
	void mouseMoved (cMouse& mouse, const cPosition& offset);

	void keyPressed (cKeyboard& keyboard, SDL_Keycode key);
	void keyReleased (cKeyboard& keyboard, SDL_Keycode key);
	void textEntered (cKeyboard& keyboard, const char* text);

	void assignKeyFocus (cWidget* widget);

	bool hitShortcuts (const cKeySequence& keySequence);

	void drawFramesPerSecond (unsigned int fps, bool draw = true);

public:
	std::shared_ptr<cFrameCounter> frameCounter;
private:
	static const size_t maximalShortcutSequenceLength = 4;

	std::vector<std::shared_ptr<cWindow>> modalWindows;

	cSignalConnectionManager signalConnectionManager;

	cKeySequence currentKeySequence;

	std::vector<std::unique_ptr<cShortcut>> shortcuts;

	cMouse* activeMouse;
	cKeyboard* activeKeyboard;

	cWidget* keyFocusWidget;
	cWidget* mouseFocusWidget;
	//cWidget* underMouseWidget;

	std::list<std::shared_ptr<cRunnable>> runnables;

	bool shouldDrawFramesPerSecond;
};

//------------------------------------------------------------------------------
template<typename WindowType>
WindowType* cApplication::show (std::shared_ptr<WindowType> window)
{
	if (window == nullptr) return nullptr;

	if (window->wantsCentered())
	{
		center (*window);
	}

	modalWindows.push_back (std::move (window));

	return static_cast<WindowType*> (modalWindows.back().get());
}

#endif // ui_graphical_applicationH
