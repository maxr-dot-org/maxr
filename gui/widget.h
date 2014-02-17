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

#ifndef gui_widgetH
#define gui_widgetH

#include <vector>
#include <memory>
#include <type_traits>

#include <SDL.h>

#include "../input/mouse/mousebuttontype.h"
#include "../utility/box.h"
#include "../utility/position.h"
#include "../autosurface.h"

class cMouse;
class cKeyboard;
class cApplication;

class cWidget
{
public:
	cWidget ();
	explicit cWidget (const cPosition& position);
	explicit cWidget (const cBox<cPosition>& area);

	virtual ~cWidget ();

	cWidget* getParent () const;

	bool isEnabled () const;
	void disable ();
	void enable ();

	const cPosition& getPosition () const;
	const cPosition& getEndPosition () const;
	void moveTo (const cPosition& newPosition);
	void move (const cPosition& offset);

	cPosition getSize () const;
	void resize (const cPosition& newSize);

	const cBox<cPosition>& getArea () const;
	void setArea (const cBox<cPosition>& area);

	cWidget* getChildAt (const cPosition& position) const;

	virtual bool isAt (const cPosition& position) const;

	virtual void draw ();

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse);
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

protected:
	template<typename WidgetType>
	WidgetType* addChild (std::unique_ptr<WidgetType> child);

	const std::vector<std::unique_ptr<cWidget>>& getChildren () const;

	// TODO: find some suitable place for this function!
	static void drawRectangle (SDL_Surface* surface, const cBox<cPosition>& rectangle, Uint32 color);
private:
	cWidget* parent;

	bool enabled;

	cBox<cPosition> area;

	AutoSurface frameSurface;

	std::vector<std::unique_ptr<cWidget>> children;

	void createFrameSurface ();
};

//------------------------------------------------------------------------------
template<typename WidgetType>
WidgetType* cWidget::addChild (std::unique_ptr<WidgetType> child)
{
	static_assert(std::is_base_of<cWidget, WidgetType>::value, "Only widgets can be added as children of widgets");

	if (child == nullptr) return nullptr;

	child->parent = this;
	children.push_back (std::move (child));

	return static_cast<WidgetType*>(children.back ().get ());
}

#endif // gui_widgetH
