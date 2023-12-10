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

#ifndef ui_widgets_widgetH
#define ui_widgets_widgetH

#include "SDLutility/autosurface.h"
#include "input/mouse/mousebuttontype.h"
#include "ui/widgets/shortcut.h"
#include "utility/box.h"
#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"

#include <SDL.h>
#include <memory>
#include <type_traits>
#include <vector>

class cMouse;
class cKeyboard;
class cApplication;
class cWindow;
class cShortcut;

class cWidget
{
public:
	static void toggleDrawDebugFrames();

	cWidget();
	explicit cWidget (const cPosition&);
	explicit cWidget (const cBox<cPosition>& area);

	virtual ~cWidget();

	/**
	 * Returns the parent widget of the current one.
	 *
	 * @return The parent widget or null if there is no parent.
	 */
	cWidget* getParent() const { return parent; }

	/**
	 * If a widget is disabled it will not receive any input events
	 * for the mouse or keyboard anymore.
	 *
	 * @return True if the widget is enabled. False if it is disabled.
	 */
	bool isEnabled() const { return enabled; }
	/**
	 * Disables the widget. See @ref isEnabled() for the effects.
	 */
	void disable() { enabled = false; }
	/**
	 * Enables the widget. See @ref isEnabled() for the effects.
	 */
	void enable() { enabled = true; }

	/**
	 * A hidden widget will not be drawn to the screen anymore.
	 *
	 * @return True if the widget is hidden. False if not.
	 */
	bool isHidden() const { return hidden; }
	/**
	 * Hides the widget. See @ref isHidden for the effects.
	 * You may want to @ref disable the widget as well when
	 * it is hidden.
	 */
	void hide() { hidden = true; }
	/**
	 * Sets the widget as not hidden. See @ref isHidden for the effects.
	 */
	void show() { hidden = false; }

	/**
	 * Returns the position (upper left corner) of the widget
	 * in absolute screen coordinates.
	 *
	 * This is the first pixel that is overdrawn with the widgets image.
	 */
	const cPosition& getPosition() const;
	/**
	 * Returns the end position (lower right corner) of the widget
	 * in absolute screen coordinates.
	 *
	 * This is the last pixel that is overdrawn with the widgets image.
	 */
	const cPosition& getEndPosition() const;

	/**
	 * Moves the widget and all its children to a new position.
	 *
	 * @param newPosition The new position (upper left corner) to move the widget to.
	 */
	void moveTo (const cPosition& newPosition);
	/**
	 * Moves the widget and all its children by a given offset.
	 *
	 * @param offset The offset by which to move the widget. Can have negative values.
	 */
	void move (const cPosition& offset);

	/**
	 * Returns the size of the widget.
	 */
	cPosition getSize() const;
	/**
	 * Resizes the widget.
	 */
	void resize (const cPosition& newSize);

	/**
	 * Adjusts the size and position of the widget to fit exactly all its children.
	 * If the widget does not have any children its new size will be (0,0)
	 */
	void fitToChildren();

	/**
	 * Returns the area of the widget in absolute screen coordinates.
	 */
	const cBox<cPosition>& getArea() const { return area; }
	/*
	 * Sets a new area for the widget.
	 *
	 * This implies changes in the position and the size of the widget.
	 *
	 * If setting the new area requires to move the widget, all children will be moved with it.
	 *
	 * @param area The new area to set.
	 */
	void setArea (const cBox<cPosition>& area);

	/**
	 * Adds a shortcut to the widget.
	 *
	 * Shortcuts added to widgets will be executed when the the shortcut is hit will the window that is containing the widget is active.
	 *
	 * @param shortcut The shortcut to add.
	 * @return A non-owning pointer to the shortcut that has been added.
	 */
	cShortcut* addShortcut (std::unique_ptr<cShortcut> shortcut);

	/**
	 * Returns all shortcuts that have been added to this widget.
	 */
	const std::vector<std::unique_ptr<cShortcut>>& getShortcuts() const { return shortcuts; }

	/**
	 * Returns the most deep child (meaning possible a child of a child and so on) at the given
	 * position or null if there is no child at the given position.
	 *
	 * If there are multiple children on the same hierarchy the one that has last been added will
	 * taken into account.
	 *
	 * Children that are disabled or hidden will be ignored and never returned.
	 *
	 * @param position The position in absolute screen coordinates to test for.
	 */
	virtual cWidget* getChildAt (const cPosition& position) const;

	/**
	 * Returns whether the widget is at the given position.
	 *
	 * This is true if the point is within the area of the widget.
	 *
	 * If the widget is hidden this will always return false.
	 *
	 * @param position The position in absolute screen coordinates to test for.
	 */
	virtual bool isAt (const cPosition& position) const;

	/**
	 * Draws the widget and all its children to the destination surface.
	 *
	 * Children will be drawn in the order they have been added.
	 * This means children added last will be drawn on top of the ones added first.
	 *
	 * @param destination The surface to draw the widget onto
	 * @param clipRect An area with which all area to be drawn will be clipped before the drawing is performed.
	 *                 This means all drawing takes place only within this area.
	 */
	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect);

	/**
	 * Will be called when ever the mouse has been moved above the widget.
	 *
	 * If the widget has mouse focus this method will be called for the widget even
	 * if the current mouse position is not above the widget.
	 *
	 * @param application The application associated with the mouse that has triggered the event.
	 * @param mouse The mouse that has been moved.
	 * @param offset The offset by which the mouse has been moved.
	 * @return True if the widget wants to consume the mouse moved event.
	 *         This means this method will not be called for any other widget after this one.
	 */
	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset);
	/**
	 * Will be called when ever a mouse button has been pressed above the widget.
	 *
	 * If the widget has mouse focus this method will be called for the widget even
	 * if the current mouse position is not above the widget.
	 *
	 * @param application The application associated with the mouse that has triggered the event.
	 * @param mouse The mouse whose button has been pressed.
	 * @param button The button that has been pressed.
	 * @return True if the widget wants to consume the mouse pressed event.
	 *         This means this method will not be called for any other widget after this one.
	 */
	virtual bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button);
	/**
	 * Will be called when ever a mouse button has been released above the widget.
	 *
	 * If the widget has mouse focus this method will be called for the widget even
	 * if the current mouse position is not above the widget.
	 *
	 * @param application The application associated with the mouse that has triggered the event.
	 * @param mouse The mouse whose button has been released.
	 * @param button The button that has been released.
	 * @return True if the widget wants to consume the mouse release event.
	 *         This means this method will not be called for any other widget after this one.
	 */
	virtual bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button);
	/**
	 * Will be called when ever the mouse wheel has been moved above the widget.
	 *
	 * If the widget has mouse focus this method will be called for the widget even
	 * if the current mouse position is not above the widget.
	 *
	 * @param application The application associated with the mouse that has triggered the event.
	 * @param mouse The mouse whose wheel has been moved.
	 * @param amount The amount by which the mouse wheel has been moved.
	 * @return True if the widget wants to consume the mouse wheel moved event.
	 *         This means this method will not be called for any other widget after this one.
	 */
	virtual bool handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount);

	/**
	 * Will be called when a keyboard key has been pressed.
	 *
	 * Derived classes should always call this method after processing the
	 * event itself and return true if the base class call has returned true.
	 *
	 * @param application The application associated with the keyboard that has triggered the event.
	 * @param keyboard The keyboard whose key has been pressed.
	 * @param key The key that has been pressed.
	 * @return True if the widget wants to consume the key pressed event.
	 *         This means this method will not be called for any other widget after this one.
	 *         Additionally returning true here prohibits any shortcuts to be executed for this key press.
	 */
	virtual bool handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key);
	/**
	 * Will be called when a keyboard key has been released.
	 *
	 * Derived classes should always call this method after processing the
	 * event itself and return true if the base class call has returned true.
	 *
	 * @param application The application associated with the keyboard that has triggered the event.
	 * @param keyboard The keyboard whose key has been released.
	 * @param key The key that has been released.
	 * @return True if the widget wants to consume the key release event.
	 *         This means this method will not be called for any other widget after this one.
	 */
	virtual bool handleKeyReleased (cApplication& application, cKeyboard& keyboard, SDL_Keycode key);
	/**
	 * Will be called when this widget has key focus and text has been entered via the keyboard.
	 *
	 * @param application The application associated with the keyboard that has triggered the event.
	 * @param keyboard The keyboard on that the text has been entered.
	 * @param text The text that has been entered (UTF-8 encoded).
	 */
	virtual void handleTextEntered (cApplication& application, cKeyboard& keyboard, const char* text);

	//virtual void handleHoveredOn (cApplication& application, cMouse& mouse);
	//virtual void handleHoveredAway (cApplication& application, cMouse& mouse);

	/**
	 * Will be called when ever the widget is going to get the key focus.
	 *
	 * At the time of the call the widget does not yet have the key focus.
	 *
	 * @param application The application associated with the keyboard that has triggered the event.
	 * @return return True if the widget accepts the key focus. In this case the key focus will be assigned to the widget.
	 *                False if the widget does not accept the key focus. In this case the key focus will be assigned
	 *                to the parent widget if there is any and if the parent accepts the focus.
	 */
	virtual bool handleGetKeyFocus (cApplication& application);

	/**
	 * Will be called when ever the widget is losing the key focus.
	 *
	 * At the time of the call the widget does not yet have the key focus.
	 *
	 * @param application The application associated with the keyboard that has triggered the event.
	 */
	virtual void handleLooseKeyFocus (cApplication& application);

	/**
	 * Will be called when ever the widget is getting the mouse focus.
	 *
	 * At the time of the call the widget does not yet have the mouse focus.
	 *
	 * @param application The application associated with the mouse that has triggered the event.
	 */
	virtual void handleGetMouseFocus (cApplication& application);
	/**
	 * Will be called when ever the widget is loosing the mouse focus.
	 *
	 * At the time of the call the widget still has the mouse focus.
	 *
	 * @param application The application associated with the mouse that has triggered the event.
	 */
	virtual void handleLooseMouseFocus (cApplication& application);

	/**
	 * Will be called when ever the widget has been moved.
	 *
	 * Derived classes should always call this method.
	 *
	 * @param offset The offset by which the widget has been moved.
	 */
	virtual void handleMoved (const cPosition& offset);

	/**
	 * Will be called when ever the widget has been resized.
	 *
	 * Derived classes should always call this method.
	 *
	 * @param oldSize The size of the widget before the resize took place
	 */
	virtual void handleResized (const cPosition& oldSize);

	/**
	 * Triggers all active shortcuts that match the end of the given key sequence.
	 *
	 * Will be called recursively for all children of the widget.
	 *
	 * @return True if any shortcut (including the ones of the children) has been triggered.
	 */
	virtual bool hitShortcuts (const cKeySequence& keySequence);

	/**
	 * Called to retranslate to current language.
	 */
	virtual void retranslate();

protected:
	/**
	 * Adds a new child
	 *
	 * This method is potentially very dangerous!
	 * Modifying the children while they are accessed will result in undefined behavior.
	 * Hence make sure this method is only called when it is absolutely sure
	 * that the children are not currently worked on.
	 * This especially means during the drawing of the children (or any of their children)
	 * or during the event handling (e.g. clicked event) of the children or any of their children).
	 *
	 * @tparam WidgetType The type of the widget to add. Has to inherit from @ref cWidget.
	 * @param child The child to add.
	 * @return A non owning pointer to the added child.
	 */
	template <typename WidgetType>
	WidgetType* addChild (std::unique_ptr<WidgetType> child);

	template <typename WidgetType, typename... Ts>
	WidgetType* emplaceChild (Ts&&... args)
	{
		return addChild (std::make_unique<WidgetType> (std::forward<Ts> (args)...));
	}

	void setParent (cWidget* parent);

	/**
	 * Removes all children.
	 *
	 * See @ref addChild for restrictions on when this method is allowed to be used!
	 */
	void removeChildren();

	/**
	 * Returns true if there are any children for this widget.
	 */
	bool hasChildren() const;

	/**
	 * Returns the currently active mouse for this widget.
	 *
	 * This may be null if the widget has not been added to a window yet
	 * or if the window does not have an active application.
	 */
	virtual cMouse* getActiveMouse() const;
	/**
	 * Returns the currently active keyboard for this widget.
	 *
	 * This may be null if the widget has not been added to a window yet
	 * or if the window does not have an active application.
	 */
	virtual cKeyboard* getActiveKeyboard() const;

	/**
	 * Returns the currently active application for this widget.
	 *
	 * This may be null if the widget has not been added to a window yet
	 * or if the window does not have an active application.
	 */
	virtual cApplication* getActiveApplication() const;

private:
	static bool drawDebugFrames;
	static cSignal<void()> drawDebugFramesChanged;

	cWidget (const cWidget&) = delete;
	cWidget& operator= (const cWidget&) = delete;

	void createFrameSurface();

	void releaseFocusRecursive (cApplication&);

private:
	cSignalConnectionManager signalConnectionManager;

	cWidget* parent = nullptr;
	bool enabled = true;
	bool hidden = false;
	cBox<cPosition> area;
	UniqueSurface frameSurface;
	std::vector<std::unique_ptr<cWidget>> children;
	std::vector<std::unique_ptr<cShortcut>> shortcuts;
};

//------------------------------------------------------------------------------
template <typename WidgetType>
WidgetType* cWidget::addChild (std::unique_ptr<WidgetType> child)
{
	static_assert (std::is_base_of<cWidget, WidgetType>::value, "Only widgets can be added as children of widgets");

	if (child == nullptr) return nullptr;

	child->setParent (this);
	children.push_back (std::move (child));

	return static_cast<WidgetType*> (children.back().get());
}

#endif // ui_widgets_widgetH
