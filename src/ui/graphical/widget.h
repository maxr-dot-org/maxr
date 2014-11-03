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

#ifndef ui_graphical_widgetH
#define ui_graphical_widgetH

#include <vector>
#include <memory>
#include <type_traits>

#include <SDL.h>

#include "input/mouse/mousebuttontype.h"
#include "utility/box.h"
#include "utility/position.h"
#include "utility/autosurface.h"
#include "maxrconfig.h"
#include "ui/graphical/shortcut.h"
#include "utility/signal/signalconnectionmanager.h"

class cMouse;
class cKeyboard;
class cApplication;
class cWindow;
class cShortcut;

class cWidget
{
public:
	static void toggleDrawDebugFrames ();

	cWidget ();
	explicit cWidget (const cPosition& position);
	explicit cWidget (const cBox<cPosition>& area);

	virtual ~cWidget ();

	/**
	 * Returns the parent widget of the current one.
	 *
	 * @return The parent widget or null if there is no parent.
	 */
	cWidget* getParent () const;

	/**
	 * If a widget is disabled it will not receive any input events
	 * for the mouse or keyboard anymore.
	 *
	 * @return True if the widget is enabled. False if it is disabled.
	 */
	bool isEnabled () const;
	/**
	 * Disables the widget. See @ref isEnabled() for the effects.
	 */
	void disable ();
	/**
	 * Enables the widget. See @ref isEnabled() for the effects.
	 */
	void enable ();

	/**
	 * A hidden widget will not be drawn to the screen anymore.
	 *
	 * @return True if the widget is hidden. False if not.
	 */
	bool isHidden () const;
	/**
	 * Hides the widget. See @ref isHidden for the effects.
	 * You may want to @ref disable the widget as well when
	 * it is hidden.
	 */
	void hide ();
	/**
	 * Sets the widget as not hidden. See @ref isHidden for the effects.
	 */
	void show ();

	const cPosition& getPosition () const;
	const cPosition& getEndPosition () const;
	void moveTo (const cPosition& newPosition);
	void move (const cPosition& offset);

	cPosition getSize () const;
	void resize (const cPosition& newSize);

	void fitToChildren ();

	const cBox<cPosition>& getArea () const;
	void setArea (const cBox<cPosition>& area);

	cShortcut* addShortcut (std::unique_ptr<cShortcut> shortcut);

	const std::vector<std::unique_ptr<cShortcut>>& getShortcuts () const;

	virtual cWidget* getChildAt (const cPosition& position) const;

	virtual bool isAt (const cPosition& position) const;

	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect);

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset);
	virtual bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button);
	virtual bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button);
	virtual bool handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount);

	virtual bool handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key);
	virtual bool handleKeyReleased (cApplication& application, cKeyboard& keyboard, SDL_Keycode key);
	virtual bool handleTextEntered (cApplication& application, cKeyboard& keyboard, const char* text);

	//virtual void handleHoveredOn (cApplication& application, cMouse& mouse);
	//virtual void handleHoveredAway (cApplication& application, cMouse& mouse);

	virtual bool handleGetKeyFocus (cApplication& application);
	virtual void handleLooseKeyFocus (cApplication& application);

	virtual void handleGetMouseFocus (cApplication& application);
	virtual void handleLooseMouseFocus (cApplication& application);

	virtual void handleMoved (const cPosition& offset);

	virtual void handleResized (const cPosition& oldSize);

	virtual bool hitShortcuts (const cKeySequence& keySequence);

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
	template<typename WidgetType>
	WidgetType* addChild (std::unique_ptr<WidgetType> child);

	void setParent (cWidget* parent);

	/**
	 * Removes all children.
	 *
	 * See @ref addChild for restrictions on when this method is allowed to be used!
	 */
	void removeChildren ();

	bool hasChildren () const;

	virtual cMouse* getActiveMouse () const;
	virtual cKeyboard* getActiveKeyboard () const;

	virtual cApplication* getActiveApplication () const;
private:
	static bool drawDebugFrames;
	static cSignal<void ()> drawDebugFramesChanged;

	cWidget (const cWidget& other) MAXR_DELETE_FUNCTION;
	cWidget& operator=(const cWidget& other) MAXR_DELETE_FUNCTION;

	cSignalConnectionManager signalConnectionManager;

	cWidget* parent;

	bool enabled;
	bool hidden;

	cBox<cPosition> area;

	AutoSurface frameSurface;

	std::vector<std::unique_ptr<cWidget>> children;

	std::vector<std::unique_ptr<cShortcut>> shortcuts;

	void createFrameSurface ();

	void releaseFocusRecursive (cApplication& application);
};

//------------------------------------------------------------------------------
template<typename WidgetType>
WidgetType* cWidget::addChild (std::unique_ptr<WidgetType> child)
{
	static_assert(std::is_base_of<cWidget, WidgetType>::value, "Only widgets can be added as children of widgets");

	if (child == nullptr) return nullptr;

	child->setParent(this);
	children.push_back (std::move (child));

	return static_cast<WidgetType*>(children.back ().get ());
}

#endif // ui_graphical_widgetH
