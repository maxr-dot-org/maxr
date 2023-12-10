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

#include "widget.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "output/video/video.h"
#include "ui/widgets/application.h"
#include "utility/color.h"

/*static*/ bool cWidget::drawDebugFrames = false;
/*static*/ cSignal<void()> cWidget::drawDebugFramesChanged;

//------------------------------------------------------------------------------
/*static*/ void cWidget::toggleDrawDebugFrames()
{
	drawDebugFrames = !drawDebugFrames;
	drawDebugFramesChanged();
}

//------------------------------------------------------------------------------
cWidget::cWidget() :
	cWidget (cPosition (0, 0))
{
}

//------------------------------------------------------------------------------
cWidget::cWidget (const cPosition& position) :
	cWidget ({position, position})
{
}

//------------------------------------------------------------------------------
cWidget::cWidget (const cBox<cPosition>& area_) :
	area (area_)
{
	createFrameSurface();

	signalConnectionManager.connect (drawDebugFramesChanged, [this]() { createFrameSurface(); });
}

//------------------------------------------------------------------------------
cWidget::~cWidget()
{
	auto application = getActiveApplication();

	if (application)
	{
		// release the mouse or key focus of this widget or any of its children.
		// We have to release the focus of the children as well because after the parent is deleted
		// the children have no possibility to access the application anymore
		releaseFocusRecursive (*application);
	}

	// just to make sure the children will not access the parent during their clean up.
	for (auto& child : children)
	{
		child->setParent (nullptr);
	}
}

//------------------------------------------------------------------------------
const cPosition& cWidget::getPosition() const
{
	return area.getMinCorner();
}

//------------------------------------------------------------------------------
const cPosition& cWidget::getEndPosition() const
{
	return area.getMaxCorner();
}

//------------------------------------------------------------------------------
void cWidget::moveTo (const cPosition& newPosition)
{
	auto offset = newPosition - area.getMinCorner();
	move (offset);
}

//------------------------------------------------------------------------------
void cWidget::move (const cPosition& offset)
{
	area.getMinCorner() += offset;
	area.getMaxCorner() += offset;
	handleMoved (offset);
}

//------------------------------------------------------------------------------
cPosition cWidget::getSize() const
{
	return area.getSize();
}

//------------------------------------------------------------------------------
void cWidget::resize (const cPosition& newSize)
{
	const auto oldSize = getSize();

	if (oldSize != newSize)
	{
		area.resize (newSize);
		handleResized (oldSize);
	}
}

//------------------------------------------------------------------------------
void cWidget::fitToChildren()
{
	if (children.empty())
	{
		resize (cPosition (0, 0));
	}
	else
	{
		cBox<cPosition> newArea (children[0]->getPosition(), children[0]->getPosition());

		for (auto& child : children)
		{
			newArea.add (child->getArea());
		}

		setArea (newArea);
	}
}

//------------------------------------------------------------------------------
void cWidget::setArea (const cBox<cPosition>& area_)
{
	const cPosition newSize = area_.getSize();
	const cPosition offset = area_.getMinCorner() - getArea().getMinCorner();

	move (offset);
	resize (newSize);
}

//------------------------------------------------------------------------------
cShortcut* cWidget::addShortcut (std::unique_ptr<cShortcut> shortcut)
{
	if (shortcut == nullptr) return nullptr;

	shortcuts.push_back (std::move (shortcut));
	return shortcuts.back().get();
}

//------------------------------------------------------------------------------
cWidget* cWidget::getChildAt (const cPosition& position) const
{
	// reverse order because the last child is the one that will be drawn last and therefor
	// is visually the one above all the others.
	// We want to find this one first, because we will abort on the first child that
	// intersects the point, regardless of whether there are other overlapping children.
	for (auto rit = children.rbegin(); rit != children.rend(); ++rit)
	{
		auto child = rit->get();

		if (!child->isEnabled()) continue;

		if (child->isAt (position))
		{
			auto childChild = child->getChildAt (position);
			return childChild ? childChild : child;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------
bool cWidget::isAt (const cPosition& position) const
{
	return !isHidden() && area.withinOrTouches (position);
}

//------------------------------------------------------------------------------
void cWidget::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (isHidden()) return;

	if (frameSurface && getArea().intersects (clipRect))
	{
		auto clipedArea = getArea().intersection (clipRect);

		SDL_Rect position = toSdlRect (clipedArea);

		clipedArea.getMinCorner() -= getPosition();
		clipedArea.getMaxCorner() -= getPosition();

		SDL_Rect source = toSdlRect (clipedArea);

		SDL_BlitSurface (frameSurface.get(), &source, &destination, &position);
	}

	for (auto& child : children)
	{
		if (child->isHidden()) continue;

		if (child->getArea().intersects (clipRect))
		{
			child->draw (destination, child->getArea().intersection (clipRect));
		}
	}
}

//------------------------------------------------------------------------------
bool cWidget::handleMouseMoved (cApplication&, cMouse&, const cPosition& offset)
{
	return false;
}

//------------------------------------------------------------------------------
bool cWidget::handleMousePressed (cApplication&, cMouse&, eMouseButtonType)
{
	return false;
}

//------------------------------------------------------------------------------
bool cWidget::handleMouseReleased (cApplication&, cMouse&, eMouseButtonType)
{
	return false;
}

//------------------------------------------------------------------------------
bool cWidget::handleMouseWheelMoved (cApplication&, cMouse&, const cPosition& amount)
{
	return false;
}

//------------------------------------------------------------------------------
bool cWidget::handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	for (auto& child : children)
	{
		if (child->handleKeyPressed (application, keyboard, key))
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
bool cWidget::handleKeyReleased (cApplication& application, cKeyboard& keyboard, SDL_Keycode key)
{
	for (auto& child : children)
	{
		if (child->handleKeyReleased (application, keyboard, key))
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void cWidget::handleTextEntered (cApplication&, cKeyboard&, const char* text)
{}

////------------------------------------------------------------------------------
//void cWidget::handleHoveredOn (cApplication&, cMouse&)
//{}
//
////------------------------------------------------------------------------------
//void  cWidget::handleHoveredAway (cApplication&, cMouse&)
//{}

//------------------------------------------------------------------------------
bool cWidget::handleGetKeyFocus (cApplication&)
{
	return false;
}

//------------------------------------------------------------------------------
void cWidget::handleLooseKeyFocus (cApplication&)
{}

//------------------------------------------------------------------------------
void cWidget::handleGetMouseFocus (cApplication&)
{}

//------------------------------------------------------------------------------
void cWidget::handleLooseMouseFocus (cApplication&)
{}

//------------------------------------------------------------------------------
void cWidget::handleMoved (const cPosition& offset)
{
	for (auto& child : children)
	{
		child->move (offset);
	}
}

//------------------------------------------------------------------------------
void cWidget::handleResized (const cPosition&)
{
	createFrameSurface();
}

//------------------------------------------------------------------------------
void cWidget::setParent (cWidget* parent_)
{
	parent = parent_;
}

//------------------------------------------------------------------------------
void cWidget::removeChildren()
{
	children.clear();
}

//------------------------------------------------------------------------------
bool cWidget::hasChildren() const
{
	return !children.empty();
}

//------------------------------------------------------------------------------
cMouse* cWidget::getActiveMouse() const
{
	return parent ? parent->getActiveMouse() : nullptr;
}

//------------------------------------------------------------------------------
cKeyboard* cWidget::getActiveKeyboard() const
{
	return parent ? parent->getActiveKeyboard() : nullptr;
}

//------------------------------------------------------------------------------
cApplication* cWidget::getActiveApplication() const
{
	return parent ? parent->getActiveApplication() : nullptr;
}

//------------------------------------------------------------------------------
void cWidget::createFrameSurface()
{
	if (drawDebugFrames)
	{
		const auto size = getSize();

		if (size.x() == 0 || size.y() == 0) return;

		frameSurface = UniqueSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));
		if (!frameSurface) return; // can happen when for some reason the size is invalid (e.g. negative)
		SDL_SetColorKey (frameSurface.get(), SDL_TRUE, 0xFF00FF);
		SDL_FillRect (frameSurface.get(), nullptr, 0xFF00FF);

		drawRectangle (*frameSurface, cBox<cPosition> (cPosition (0, 0), size), cRgbColor::red());
	}
	else
	{
		frameSurface = nullptr;
	}
}

//------------------------------------------------------------------------------
bool cWidget::hitShortcuts (const cKeySequence& keySequence)
{
	bool anyMatch = false;

	for (const auto& shortcut : shortcuts)
	{
		anyMatch |= shortcut->hit (keySequence);
	}

	for (const auto& child : children)
	{
		anyMatch |= child->hitShortcuts (keySequence);
	}
	return anyMatch;
}

//------------------------------------------------------------------------------
void cWidget::retranslate()
{
	for (auto& child : children)
	{
		child->retranslate();
	}
}

//------------------------------------------------------------------------------
void cWidget::releaseFocusRecursive (cApplication& application)
{
	if (application.hasKeyFocus (*this)) application.releaseKeyFocus (*this);
	if (application.hasMouseFocus (*this)) application.releaseMouseFocus (*this);

	if (application.hasKeyFocus() || application.hasMouseFocus())
	{
		for (auto& child : children)
		{
			child->releaseFocusRecursive (application);
		}
	}
}
