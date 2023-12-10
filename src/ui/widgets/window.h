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

#ifndef ui_widgets_windowH
#define ui_widgets_windowH

#include "SDLutility/autosurface.h"
#include "ui/widgets/widget.h"
#include "utility/signal/signal.h"

#include <utility>

class cMouseCursor;

/**
 * Different methods what to do with the background
 * behind menus that are smalled than the applications
 * full resolution.
 */
enum class eWindowBackgrounds
{
	/**
	 * Will everything with black.
	 */
	Black,
	/**
	 * Draw an alpha effect above what ever has been displayed before.
	 */
	Alpha,
	/**
	 * Do nothing with the background, just leave it there.
	 */
	Transparent
};

/**
 * A basic window.
 *
 * This is a special widget that is handled directly by the an @ref cApplication.
 * In general this is a menu that handles other basic widgets (buttons, labels, ...)
 * as its items.
 */
class cWindow : public cWidget
{
public:
	/**
	 * Creates a new window.
	 *
	 * @param surface The background surface of the window. The window will be resized according to the
	 *                size of this image. If it is null, the window will have an size of (0,0).
	 * @param backgroundType Background type to apply when drawing the window.
	 */
	explicit cWindow (UniqueSurface surface, eWindowBackgrounds backgroundType = eWindowBackgrounds::Black);

	/**
	 * Returns whether the window wants to be closed.
	 *
	 * @return True if the window wants to be closed.
	 */
	bool isClosing() const { return closing; }

	/**
	 * Marks the window as to be closed.
	 */
	void close();

	/**
	 * Draws the window, its background and all children.
	 *
	 * @see cWidget::draw
	 */
	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

	/**
	 * Gets called when the window is activated.
	 *
	 * This means it now is on top of the window stack of the application
	 * and is the window that will be drawn.
	 *
	 * Inheriting classes that are overwriting this function should
	 * absolutely call this function of the base class!
	 *
	 * @param application The application that activated this window.
	 * @param firstTime Will be true when the window is activated the first time.
	 *                  This means it is new on the window stack and has just been put on the very top of it.
	 *                  When the window gets activated because all other windows that have been on above
	 *                  this one have been closed, the flag will be false.
	 */
	virtual void handleActivated (cApplication& application, bool firstTime);
	/**
	 * Gets called when the window is deactivated.
	 *
	 * This means it is no longer on top of the window stack of the
	 * application. This can be either because the window will be
	 * closed (as requested from the window through @ref isClosing())
	 * or because an other window has been put above the current one
	 * on the window stack.
	 *
	 * Inheriting classes that are overwriting this function should
	 * absolutely call this function of the base class!
	 *
	 * @param application The application that deactivated this window.
	 * @param removed Is true when the window is removed entirely from the
	 *                applications window stack.
	 */
	virtual void handleDeactivated (cApplication& application, bool removed);

	/**
	 * Return true if the window wants to be centered.
	 */
	virtual bool wantsCentered() const { return true; }

	/**
	 * Triggered when @ref handleRemoved is called.
	 */
	cSignal<void()> terminated;

protected:
	/**
	 * Gets the mouse of the of the application the current window is the
	 * active one (on top of the window stack).
	 *
	 * @return The mouse or null if the current window is
	 *         not the active one in any application or the application
	 *         does not have a mouse registered to it.
	 */
	cMouse* getActiveMouse() const override;

	/**
	 * Gets the keyboard of the of the application the current window is the
	 * active one (on top of the window stack).
	 *
	 * @return The keyboard or null if the current window is
	 *         not the active one in any application or the application
	 *         does not have a keyboard registered to it.
	 */
	cKeyboard* getActiveKeyboard() const override;

	/**
	 * Gets the application where the current window is the
	 * active one (on top of the window stack).
	 *
	 * @return The application or null if the current window is
	 *         not the active one in any application.
	 */
	cApplication* getActiveApplication() const override { return activeApplication; }

	/**
	 * Returns the default cursor for this window.
	 * This cursor will be set to the mouse when the window is activated.
	 *
	 * @return The default cursor for the window or null if there is no default cursor
	 *         for the window and the current cursor should stay visible on the activation of the window.
	 */
	virtual std::unique_ptr<cMouseCursor> makeDefaultCursor() const;

	/**
	 * Sets a new background surface image.
	 * The window will be resized to the images size.
	 *
	 * @param surface The surface to set as background.
	 */
	void setSurface (UniqueSurface surface);

	/**
	 * The current background image.
	 *
	 * @return The background image or null if there is non.
	 */
	SDL_Surface* getSurface();

private:
	/**
	 * Background surface of the window.
	 */
	UniqueSurface surface;

	/**
	 * The type of the background behind the background.
	 * This is only relevant if the resolution of the window is smaller
	 * then the applications full resolution.
	 */
	eWindowBackgrounds backgroundType;

	/**
	 * The application where this window is currently active.
	 * Can be null if the window is not active in any application
	 * at the moment.
	 */
	cApplication* activeApplication = nullptr;

	/**
	 * True if the window wants to be closed
	 */
	bool closing = false;

	/**
	 * True if the window has been drawn at least once since it has become active.
	 */
	bool hasBeenDrawnOnce = false;
};

#endif // ui_widgets_windowH
