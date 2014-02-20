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

#include "clickablewidget.h"
#include "../../application.h"
#include "../../../input/mouse/mouse.h"

//------------------------------------------------------------------------------
cClickableWidget::cClickableWidget () :
	isPressed (false),
	mouseWasOver (false),
	consumeClick (true)
{}

//------------------------------------------------------------------------------
cClickableWidget::cClickableWidget (const cPosition& position) :
	cWidget (position),
	isPressed (false),
	mouseWasOver (false),
	consumeClick (true)
{}

//------------------------------------------------------------------------------
cClickableWidget::cClickableWidget (const cBox<cPosition>& area) :
	cWidget (area),
	isPressed (false),
	mouseWasOver (false),
	consumeClick (true)
{}

//------------------------------------------------------------------------------
bool cClickableWidget::handleMouseMoved (cApplication& application, cMouse& mouse)
{
	if (!application.hasMouseFocus (*this)) return false;

	const bool mouseIsOver = isAt (mouse.getPosition ());
	
	if (mouseWasOver && !mouseIsOver)
	{
		if (isPressed)
		{
			setPressed (false);
		}
	}
	else if (!mouseWasOver && mouseIsOver)
	{
		if (application.hasMouseFocus (*this))
		{
			setPressed (true);
		}
	}

	mouseWasOver = mouseIsOver;

	return true;
}

//------------------------------------------------------------------------------
bool cClickableWidget::handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left)
	{
		setPressed (true);
		application.grapMouseFocus (*this);
		return consumeClick;
	}
	return false;
}

//------------------------------------------------------------------------------
bool cClickableWidget::handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	if (button == eMouseButtonType::Left && application.hasMouseFocus(*this))
	{
		setPressed (false);
		application.releaseMouseFocus (*this);

		if (isAt (mouse.getPosition ()))
		{
			if (handleClicked (application, mouse, button)) return consumeClick;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
void cClickableWidget::handleLooseMouseFocus (cApplication& application)
{
	if (isPressed)
	{
		setPressed (false);
	}
	mouseWasOver = false;
}

//------------------------------------------------------------------------------
void cClickableWidget::setConsumeClick (bool consumeClick_)
{
	consumeClick = consumeClick_;
}

//------------------------------------------------------------------------------
void cClickableWidget::setPressed (bool pressed)
{
	isPressed = pressed;
}
