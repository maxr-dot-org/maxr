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

#ifndef gui_applicationH
#define gui_applicationH

#include <SDL.h>

#include <vector>
#include <memory>

#include "../network.h"
#include "../utility/concurrentqueue.h"
#include "../utility/signal/signalconnectionmanager.h"
#include "../input/mouse/mousebuttontype.h"

class cMouse;
class cKeyboard;
class cWidget;
class cWindow;
class cPosition;
class cGame;

class cApplication : public INetMessageReceiver
{
public:
	cApplication ();
	~cApplication ();

	void execute ();

	template<typename WindowType>
	WindowType* show (std::shared_ptr<WindowType> window, bool centered = true);

	void registerMouse (cMouse& mouse);
	void registerKeyboard (cKeyboard& keyboard);

	void grapMouseFocus (cWidget& widget);
	void releaseMouseFocus (const cWidget& widget);
	bool hasMouseFocus (const cWidget& widget) const;

	void grapKeyFocus (cWidget& widget);
	void releaseKeyFocus (const cWidget& widget);

	void setGame (std::shared_ptr<cGame> game);
	const std::shared_ptr<cGame>& getGame () const;

	cMouse* getActiveMouse ();
	cKeyboard* getActiveKeyboard ();

	virtual void pushEvent (std::unique_ptr<cNetMessage> message) MAXR_OVERRIDE_FUNCTION;
private:
	std::vector<std::shared_ptr<cWindow>> modalWindows;
	cConcurrentQueue<std::unique_ptr<cNetMessage>> messageQueue;

	bool terminate;

	cSignalConnectionManager signalConnectionManager;

	cMouse* activeMouse;
	cKeyboard* activeKeyboard;

	cWidget* keyFocusWidget;
	cWidget* mouseFocusWidget;
	//cWidget* underMouseWidget;

	std::shared_ptr<cGame> game;

	void center (cWindow& window);

	cWindow* getActiveWindow ();

	cWidget* getMouseEventFirstTarget (const cPosition& position);

	cWidget* getKeyFocusWidget () const;

	void mousePressed (cMouse& mouse, eMouseButtonType button);
	void mouseReleased (cMouse& mouse, eMouseButtonType button);
	void mouseWheelMoved (cMouse& mouse, const cPosition& amount);
	void mouseMoved (cMouse& mouse, const cPosition& offset);

	void keyPressed (cKeyboard& keyboard, SDL_Keycode key);
	void keyReleased (cKeyboard& keyboard, SDL_Keycode key);
	void textEntered (cKeyboard& keyboard, const char* text);

	void assignKeyFocus (cWidget* widget);

	void handleNetMessage (cNetMessage& message);
};

//------------------------------------------------------------------------------
template<typename WindowType>
WindowType* cApplication::show (std::shared_ptr<WindowType> window, bool centered)
{
	if (window == nullptr) return nullptr;

	if (centered)
	{
		center (*window);
	}

	modalWindows.push_back (std::move (window));

	return static_cast<WindowType*>(modalWindows.back ().get ());
}

#endif // gui_applicationH
